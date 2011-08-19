#include "ScriptEditor.h"
#include "ScriptEditorManager.h"
#include "IntelliSense.h"
#include "RefactorTools.h"

#include "[Common]\HandShakeStructs.h"
#include "[Common]\ListViewUtilities.h"
#include "[Common]\NativeWrapper.h"
#include "[Common]\CustomInputBox.h"

using namespace IntelliSense;

namespace ScriptEditor
{
	void GlobalInputMonitor_MouseUp(Object^ Sender, MouseEventArgs^ E)
	{
		switch (E->Button)
		{
		case MouseButtons::Left:
			{
				if (SEMGR->TornWorkspace != nullptr)
				{
					IntPtr Wnd = NativeWrapper::WindowFromPoint(E->Location);
					if (Wnd == IntPtr::Zero)
					{
						ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
						Parameters->EditorHandleIndex = 0;
						Parameters->ParameterList->Add(ScriptEditorManager::TabTearOpType::e_NewContainer);
						Parameters->ParameterList->Add(SEMGR->TornWorkspace);
						Parameters->ParameterList->Add(nullptr);
						Parameters->ParameterList->Add(E->Location);
						SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_TabTearOp, Parameters);

						HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
						SEMGR->TornWorkspace = nullptr;
					}

					DotNetBar::SuperTabStrip^ Strip = nullptr;
					try
					{
						Strip = dynamic_cast<DotNetBar::SuperTabStrip^>(Control::FromHandle(Wnd));
					}
					catch (Exception^ E)
					{
						DebugPrint("An exception was raised during a tab tearing operation!\n\tError Message: " + E->Message);
						Strip = nullptr;
					}
					if (Strip != nullptr)
					{
						if (SEMGR->TornWorkspace->GetIsTabStripParent(Strip))		// not a tearing op , the strip's the same
						{
							HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
							SEMGR->TornWorkspace = nullptr;
						}
						else
						{
							ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
							Parameters->EditorHandleIndex = 0;
							Parameters->ParameterList->Add(ScriptEditorManager::TabTearOpType::e_RelocateToContainer);
							Parameters->ParameterList->Add(SEMGR->TornWorkspace);
							Parameters->ParameterList->Add(dynamic_cast<TabContainer^>(Strip->Tag));
							Parameters->ParameterList->Add(E->Location);
							SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_TabTearOp, Parameters);

							HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
							SEMGR->TornWorkspace = nullptr;
						}
					}
					else
					{
						ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
						Parameters->EditorHandleIndex = 0;
						Parameters->ParameterList->Add(ScriptEditorManager::TabTearOpType::e_NewContainer);
						Parameters->ParameterList->Add(SEMGR->TornWorkspace);
						Parameters->ParameterList->Add(nullptr);
						Parameters->ParameterList->Add(E->Location);
						SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_TabTearOp, Parameters);

						HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
						SEMGR->TornWorkspace = nullptr;
					}
				}
				else
				{
					DebugPrint("Global tab tear hook called out of turn! Expecting an unresolved operation.");
					HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
				}
				break;
			}
		default:
			if (SEMGR->TornWorkspace != nullptr)
			{
				HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
				SEMGR->TornWorkspace = nullptr;
			}
			break;
		}
	}

	#pragma region TabContainer
		TabContainer::TabContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height)
		{
			Application::EnableVisualStyles();
			InitializedFlag = false;

			EditorForm = gcnew AnimatedForm(0.15);
			EditorForm->SuspendLayout();

			EditorForm->FormBorderStyle = FormBorderStyle::Sizable;
			EditorForm->Closing += gcnew CancelEventHandler(this, &TabContainer::EditorForm_Cancel);
			EditorForm->KeyDown += gcnew KeyEventHandler(this, &TabContainer::EditorForm_KeyDown);
			EditorForm->Move += gcnew EventHandler(this, &TabContainer::EditorForm_PositionChanged);
			EditorForm->SizeChanged += gcnew EventHandler(this, &TabContainer::EditorForm_SizeChanged);
			EditorForm->MaximizedBoundsChanged += gcnew EventHandler(this, &TabContainer::EditorForm_SizeChanged);
			EditorForm->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			EditorForm->AutoScaleMode = AutoScaleMode::Font;
			EditorForm->Size = Size(Width, Height);
			EditorForm->KeyPreview = true;

			if (!ScriptModifiedIcons->Images->Count)
			{
				ScriptModifiedIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOff"));		// unmodified
				ScriptModifiedIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOn"));		// modified
				ScriptModifiedIcons->ImageSize = Size(12, 12);
				ScriptModifiedIcons->TransparentColor = Color::White;
			}

			EditorTabStrip = gcnew DotNetBar::SuperTabControl();
			EditorTabStrip->SuspendLayout();
			EditorTabStrip->Dock = DockStyle::Fill;
			EditorTabStrip->Location = Point(0, 0);
			EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Top;
			EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLineFit;
			EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Far;
			EditorTabStrip->Font = gcnew Font("Segoe UI", 10, FontStyle::Regular);
			EditorTabStrip->TabFont = gcnew Font("Segoe UI", 9, FontStyle::Bold);
			EditorTabStrip->SelectedTabFont = gcnew Font("Segoe UI", 9, FontStyle::Bold);
			EditorTabStrip->FixedTabSize = Size(0, 23);

			if (!OPTIONS->FetchSettingAsInt("TabsOnTop"))
			{
				EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Left;
				EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
				EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Near;
				EditorTabStrip->FixedTabSize = Size(150, 40);
			}

			EditorTabStrip->TabItemClose += gcnew EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>(this, &TabContainer::ScriptStrip_TabItemClose);
			EditorTabStrip->SelectedTabChanged += gcnew EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>(this, &TabContainer::ScriptStrip_SelectedTabChanged);
			EditorTabStrip->TabRemoved += gcnew EventHandler<DotNetBar::SuperTabStripTabRemovedEventArgs^>(this, &TabContainer::ScriptStrip_TabRemoved);
			EditorTabStrip->TabStripMouseClick += gcnew EventHandler<MouseEventArgs^>(this, &TabContainer::ScriptStrip_MouseClick);
			EditorTabStrip->TabMoving += gcnew EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>(this, &TabContainer::ScriptStrip_TabMoving);

			EditorTabStrip->CloseButtonOnTabsVisible = true;
			EditorTabStrip->CloseButtonOnTabsAlwaysDisplayed = false;
			EditorTabStrip->AntiAlias = true;
			EditorTabStrip->TabStop = false;
			EditorTabStrip->ImageList = ScriptModifiedIcons;
			EditorTabStrip->ReorderTabsEnabled = true;
			EditorTabStrip->TabStyle = DotNetBar::eSuperTabStyle::Office2010BackstageBlue;

			EditorTabStrip->TabStrip->Tag = this;

			NewTabButton = gcnew DotNetBar::ButtonItem();
			NewTabButton->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource("NewTabButton");
			NewTabButton->ButtonStyle = DotNetBar::eButtonStyle::Default;
			NewTabButton->Style = DotNetBar::eDotNetBarStyle::Office2007;
			NewTabButton->ColorTable = DotNetBar::eButtonColor::BlueOrb;
			NewTabButton->Text = "New Tab";
			NewTabButton->Tooltip = "New Tab";
			NewTabButton->Click += gcnew EventHandler(this, &TabContainer::NewTabButton_Click);

			SortTabsButton = gcnew DotNetBar::ButtonItem();
			SortTabsButton->ButtonStyle = DotNetBar::eButtonStyle::TextOnlyAlways;
			SortTabsButton->Text = "Sort Tabs";
			SortTabsButton->Tooltip = "Sort Tabs";
			SortTabsButton->Click += gcnew EventHandler(this, &TabContainer::SortTabsButton_Click);

			NewTabButton->SubItems->Add(SortTabsButton);
			EditorTabStrip->ControlBox->SubItems->Add(NewTabButton);
			EditorTabStrip->ControlBox->Visible = true;

			EditorForm->HelpButton = false;
			EditorForm->Text = "CSE Script Editor";

			EditorForm->Controls->Add(EditorTabStrip);

			if (OPTIONS->FetchSettingAsInt("UseCSParent"))
			{
				EditorForm->ShowInTaskbar = false;
				EditorForm->Show(gcnew WindowHandleWrapper((IntPtr)g_CSEInterface->CSEEditorAPI.GetCSMainWindowHandle()));
			}
			else
			{
				EditorForm->Show();
			}

			EditorForm->Location = Point(PosX, PosY);

			DestructionFlag = false;
			BackJumpStack = gcnew Stack<UInt32>();
			ForwardJumpStack = gcnew Stack<UInt32>();

			DotNetBar::RibbonPredefinedColorSchemes::ChangeOffice2010ColorTable(EditorForm, DotNetBar::Rendering::eOffice2010ColorScheme::Black);
			EditorTabStrip->TabStripColor->Background = gcnew DotNetBar::Rendering::SuperTabLinearGradientColorTable(Color::FromArgb(255, 48, 59, 61),
																													Color::FromArgb(255, 61, 76, 106));

			EditorTabStrip->ResumeLayout();
			EditorForm->ResumeLayout();

			InstantiateNewWorkspace(InitScript);

			InitializedFlag = true;
		}

		void TabContainer::EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			if (DestructionFlag)
			{
				return;
			}

			E->Cancel = true;
			if (EditorTabStrip->Tabs->Count > 1)
			{
				if (MessageBox::Show("Are you sure you want to close all open tabs?",
									 "CSE Script Editor",
									 MessageBoxButtons::YesNo,
									 MessageBoxIcon::Question,
									 MessageBoxDefaultButton::Button2) == DialogResult::No)
				{
					return;
				}
			}

			DestructionFlag = true;
			CloseAllOpenWorkspaces();
			DestructionFlag = false;
		}

		void TabContainer::EditorForm_PositionChanged(Object^ Sender, EventArgs^ E)
		{
			if (!InitializedFlag)
				return;

			Workspace^ Itr = dynamic_cast<Workspace^>(EditorTabStrip->SelectedTab->Tag);
			if (Itr != nullptr)
				Itr->HandlePositionSizeChange();
		}

		void TabContainer::EditorForm_SizeChanged(Object^ Sender, EventArgs^ E)
		{
			if (!InitializedFlag)
				return;

			Workspace^ Itr = dynamic_cast<Workspace^>(EditorTabStrip->SelectedTab->Tag);
			if (Itr != nullptr)
				Itr->HandlePositionSizeChange();
		}

		void TabContainer::ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs^ E)
		{
			Workspace^ Itr = dynamic_cast<Workspace^>(GetMouseOverTab()->Tag);		// clicking on the close button doesn't change the active tab

			E->Cancel = true;

			Itr->CloseScript();
		}

		void TabContainer::ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs ^ E)
		{
			if (E->OldValue == nullptr || E->NewValue == nullptr)
				return;

			Workspace^ OldWorkspace = dynamic_cast<Workspace^>(E->OldValue->Tag);
			Workspace^ NewWorkspace = dynamic_cast<Workspace^>(E->NewValue->Tag);

			if (OldWorkspace != nullptr && !OldWorkspace->GetIsFirstRun())
				OldWorkspace->DisableControls();
			if (!NewWorkspace->GetIsFirstRun())
				NewWorkspace->EnableControls();

			EditorForm->Text = NewWorkspace->GetScriptDescription() + " - CSE Script Editor";
			EditorForm->Focus();

			if (OldWorkspace != nullptr)
				OldWorkspace->HandleFocus(false);

			NewWorkspace->HandleFocus(true);
		}

		void TabContainer::ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E)
		{
			if (EditorTabStrip->Tabs->Count == 0)
			{
				if (!DestructionFlag && OPTIONS->FetchSettingAsInt("DestroyOnLastTabClose") == 0)
					InstantiateNewWorkspace(0);
				else
				{
					ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
					Parameters->ParameterList->Add(this);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_ReleaseTabContainer, Parameters);
				}
			}
		}

		Workspace^ TabContainer::InstantiateNewWorkspace(ComponentDLLInterface::ScriptData* InitScript)
		{
			EditorForm->SuspendLayout();

			ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
			Parameters->EditorHandleIndex = 0;
			Parameters->ParameterList->Add(this);
			Parameters->ParameterList->Add((UInt32)InitScript);

			Workspace^ NewWorkspace = dynamic_cast<Workspace^>(SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateWorkspace, Parameters));
			Trace::Assert(NewWorkspace != nullptr, "NewWorkspace == nullptr");
			NewWorkspace->BringToFront();

			EditorForm->ResumeLayout();

			return NewWorkspace;
		}

		void TabContainer::PerformRemoteTabOperation(RemoteOperation Operation, Object^ Arbitrary)
		{
			ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();

			switch (Operation)
			{
			case RemoteOperation::e_New:
				{
					Workspace^ NewWorkspace = InstantiateNewWorkspace(0);
					NewWorkspace->NewScript();
					break;
				}
			case RemoteOperation::e_Open:
				{
					Workspace^ NewWorkspace = InstantiateNewWorkspace(0);
					NewWorkspace->OpenScript();
					break;
				}
			case RemoteOperation::e_LoadNew:
				{
					String^ FilePath = dynamic_cast<String^>(Arbitrary);
					Workspace^ NewWorkspace = nullptr;

					if (OPTIONS->FetchSettingAsInt("LoadScriptUpdateExistingScripts") == 0)
					{
						NewWorkspace = InstantiateNewWorkspace(0);
						NewWorkspace->NewScript();
					}
					else
					{
						try
						{
							StreamReader^ TextParser = gcnew StreamReader(FilePath);
							String^ FileContents = TextParser->ReadToEnd();
							TextParser->Close();

							String^ ScriptName = ((FileContents->Split('\n', 1))[0]->Split(' '))[1];
							CString CEID(ScriptName);
							ComponentDLLInterface::ScriptData* Data = g_CSEInterface->CSEEditorAPI.LookupScriptableFormByEditorID(CEID.c_str());
							if (Data)
								NewWorkspace = InstantiateNewWorkspace(Data);
							else
							{
								NewWorkspace = InstantiateNewWorkspace(0);
								NewWorkspace->NewScript();
							}

							g_CSEInterface->DeleteNativeHeapPointer(Data, false);
						}
						catch (Exception^ E)
						{
							DebugPrint("Couldn't read script name from file " + FilePath + " for script updating!\n\tException: " + E->Message);
							NewWorkspace = InstantiateNewWorkspace(0);
						}
					}

					NewWorkspace->LoadFileFromDisk(FilePath);
					break;
				}
			case RemoteOperation::e_NewText:
				{
					Workspace^ NewWorkspace = InstantiateNewWorkspace(0);
					NewWorkspace->NewScript();
					NewWorkspace->SetScriptText(dynamic_cast<String^>(Arbitrary));
					break;
				}
			}
		}

		void TabContainer::Destroy()
		{
			FlagDestruction(true);

			EditorForm->Close();
			BackJumpStack->Clear();
			ForwardJumpStack->Clear();

			delete EditorForm;
		}

		void TabContainer::JumpToWorkspace(UInt32 AllocatedIndex, String^% ScriptName)
		{
			UInt32 Count = 0;
			DotNetBar::SuperTabItem^ OpenedWorkspace = nullptr;

			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);

				if (Editor != nullptr && !String::Compare(Editor->GetScriptID(), ScriptName, true))
				{
					Count++;
					OpenedWorkspace = Itr;
				}
			}

			if (Count == 1)
				EditorTabStrip->SelectedTab = OpenedWorkspace;
			else
			{
				CString CEID(ScriptName);
				ComponentDLLInterface::ScriptData* Data = g_CSEInterface->CSEEditorAPI.LookupScriptableFormByEditorID(CEID.c_str());
				if (Data)
					InstantiateNewWorkspace(Data);
				else
					InstantiateNewWorkspace(0);

				g_CSEInterface->DeleteNativeHeapPointer(Data, false);
			}

			BackJumpStack->Push(AllocatedIndex);
			ForwardJumpStack->Clear();
		}

		void TabContainer::NavigateJumpStack(UInt32 AllocatedIndex, TabContainer::NavigationDirection Direction)
		{
			UInt32 JumpIndex = 0;
			switch (Direction)
			{
			case NavigationDirection::e_Back:
				if (BackJumpStack->Count < 1)		return;
				JumpIndex = BackJumpStack->Pop();
				break;
			case NavigationDirection::e_Forward:
				if (ForwardJumpStack->Count < 1)	return;
				JumpIndex = ForwardJumpStack->Pop();
				break;
			}

			Workspace^ Itr = SEMGR->GetAllocatedWorkspace(JumpIndex);

			if (Itr == nullptr)
			{
				NavigateJumpStack(AllocatedIndex, Direction);
			}
			else
			{
				switch (Direction)
				{
				case NavigationDirection::e_Back:
					ForwardJumpStack->Push(AllocatedIndex);
					break;
				case NavigationDirection::e_Forward:
					BackJumpStack->Push(AllocatedIndex);
					break;
				}

				Itr->BringToFront();
				DebugPrint("Jumping from index " + AllocatedIndex + " to " + JumpIndex);
			}
		}

		void TabContainer::NewTabButton_Click(Object^ Sender, EventArgs^ E)
		{
			InstantiateNewWorkspace(0);
		}

		void TabContainer::EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::T:									// New tab
			case Keys::Tab:									// Switch tab
			case Keys::O:									// Open script
			case Keys::Left:								// Previous script
			case Keys::Right:								// Next script
			case Keys::N:									// New script
				Workspace^ Itr = dynamic_cast<Workspace^>(EditorTabStrip->SelectedTab->Tag);
				if (Itr != nullptr)
					Itr->TunnelKeyDownEvent(E);
				break;
			}
		}

		DotNetBar::SuperTabItem^ TabContainer::GetMouseOverTab()
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				if (Itr->IsMouseOver)
					return Itr;
			}

			return nullptr;
		}

		void TabContainer::ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E)
		{
			switch (E->Button)
			{
			case MouseButtons::Middle:
				{
					DotNetBar::SuperTabItem^ MouseOverTab = GetMouseOverTab();
					if (MouseOverTab != nullptr)
					{
						Workspace^ Itr = dynamic_cast<Workspace^>(MouseOverTab->Tag);
						Itr->CloseScript();
					}
					break;
				}
			}
		}

		void TabContainer::ScriptStrip_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E)
		{
			if (SEMGR->TornWorkspace != nullptr)
				return;			// wait until the global hook clears the handle, as the TabMoving event gets called for every frame of mouse movement

			DotNetBar::SuperTabItem^ MouseOverTab = GetMouseOverTab();
			if (MouseOverTab != nullptr)
			{
				SEMGR->TornWorkspace = dynamic_cast<Workspace^>(MouseOverTab->Tag);;
				HookManager::MouseUp += GlobalMouseHook_MouseUpHandler;
			}
		}

		void TabContainer::SaveAllOpenWorkspaces()
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);
				Editor->SaveScript(Workspace::SaveScriptOperation::e_SaveAndCompile);
			}
		}

		void TabContainer::CloseAllOpenWorkspaces()
		{
			List<Workspace^>^ ActiveWorkspaces = gcnew List<Workspace^>();
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
				ActiveWorkspaces->Add(dynamic_cast<Workspace^>(Itr->Tag));

			for each (Workspace^ Editor in ActiveWorkspaces)
				Editor->CloseScript();
		}

		void TabContainer::SaveAllTabsToFolder(String^ FolderPath)
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);

				if (Editor->GetIsCurrentScriptNew() || Editor->GetIsFirstRun())
					continue;

				Editor->SaveScriptToDisk(FolderPath, false);
			}
		}

		void TabContainer::LoadFileToNewTab(String^ FileName)
		{
			PerformRemoteTabOperation(RemoteOperation::e_LoadNew, FileName);
		}

		Rectangle TabContainer::GetEditorFormRect()
		{
			if (GetEditorFormWindowState() == FormWindowState::Normal)
				return EditorForm->Bounds;
			else
				return EditorForm->RestoreBounds;
		}

		Workspace^ TabContainer::LookupWorkspaceByTabIndex(UInt32 TabIndex)
		{
			if (TabIndex >= EditorTabStrip->Tabs->Count)
				return nullptr;
			else
				return dynamic_cast<Workspace^>(EditorTabStrip->Tabs[TabIndex]->Tag);
		}

		void TabContainer::AddTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->Tabs->Add(Tab);
		}

		void TabContainer::RemoveTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->Tabs->Remove(Tab);
		}

		void TabContainer::AddTabControlBox(DotNetBar::SuperTabControlPanel^ Box)
		{
			EditorTabStrip->Controls->Add(Box);
		}

		void TabContainer::RemoveTabControlBox(DotNetBar::SuperTabControlPanel^ Box)
		{
			EditorTabStrip->Controls->Remove(Box);
		}

		void TabContainer::SelectTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->SelectedTab = Tab;
			EditorTabStrip->TabStrip->EnsureVisible(Tab);
		}

		void TabContainer::SortTabsButton_Click( Object^ Sender, EventArgs^ E )
		{
			array<String^>^ Keys = gcnew array<String^>(EditorTabStrip->Tabs->Count);
			array<DotNetBar::BaseItem^>^ Tabs = gcnew array<DotNetBar::BaseItem^>(EditorTabStrip->Tabs->Count);

			for (int i = 0; i < EditorTabStrip->Tabs->Count; i++)
			{
				Tabs[i] = EditorTabStrip->Tabs[i];
				Keys[i] = EditorTabStrip->Tabs[i]->PlainText;
			}

			Array::Sort(Keys, Tabs);

			for (int i = 0; i < EditorTabStrip->Tabs->Count; i++)
				EditorTabStrip->Tabs[i] = Tabs[i];

			EditorTabStrip->RecalcLayout();
		}

		void TabContainer::SelectPreviousTab()
		{
			if (EditorTabStrip->Tabs->Count < 2)
				return;

			EditorTabStrip->SelectPreviousTab();
		}

		void TabContainer::SelectNextTab()
		{
			if (EditorTabStrip->Tabs->Count < 2)
				return;

			EditorTabStrip->SelectNextTab();
		}

	#pragma endregion

	#pragma region Workspace
		Workspace::Workspace(UInt32 Index, TabContainer^ Parent, ComponentDLLInterface::ScriptData* InitScript)
		{
			ParentContainer = Parent;
			EditorTab = gcnew DotNetBar::SuperTabItem();
			EditorControlBox = gcnew DotNetBar::SuperTabControlPanel();

			WorkspaceSplitter = gcnew SplitContainer();

			MessageList = gcnew ListView();
			FindList = gcnew ListView();
			BookmarkList = gcnew ListView();
			VariableIndexList = gcnew ListView();
			VariableIndexEditBox = gcnew TextBox();
			SpoilerText = gcnew Label();

			WorkspaceMainToolBar = gcnew ToolStrip();
			ToolBarNewScript = gcnew ToolStripButton();
			ToolBarOpenScript = gcnew ToolStripButton();
			ToolBarPreviousScript = gcnew ToolStripButton();
			ToolBarNextScript = gcnew ToolStripButton();
			ToolBarSaveScript = gcnew ToolStripSplitButton();
			ToolBarSaveScriptDropDown = gcnew ToolStripDropDown();
			ToolBarSaveScriptNoCompile = gcnew ToolStripButton();
			ToolBarSaveScriptAndPlugin = gcnew ToolStripButton();
			ToolBarRecompileScripts = gcnew ToolStripButton();
			ToolBarCompileDependencies = gcnew ToolStripButton();
			ToolBarDeleteScript = gcnew ToolStripButton();
			ToolBarNavigationBack = gcnew ToolStripButton();
			ToolBarNavigationForward = gcnew ToolStripButton();
			ToolBarSaveAll = gcnew ToolStripButton();
			ToolBarOptions = gcnew ToolStripButton();

			ToolBarScriptType = gcnew ToolStripDropDownButton();
			ToolBarScriptTypeContents = gcnew ToolStripDropDown();
			ToolBarScriptTypeContentsObject = gcnew ToolStripButton();
			ToolBarScriptTypeContentsQuest = gcnew ToolStripButton();
			ToolBarScriptTypeContentsMagicEffect = gcnew ToolStripButton();

			WorkspaceSecondaryToolBar = gcnew ToolStrip();
			ToolBarCommonTextBox = gcnew ToolStripTextBox();
			ToolBarEditMenu = gcnew ToolStripDropDownButton();
			ToolBarEditMenuContents = gcnew ToolStripDropDown();
			ToolBarEditMenuContentsFind = gcnew ToolStripButton();
			ToolBarEditMenuContentsReplace = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoLine = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoOffset = gcnew ToolStripButton();
			ToolBarMessageList = gcnew ToolStripButton();
			ToolBarFindList = gcnew ToolStripButton();
			ToolBarBookmarkList = gcnew ToolStripButton();
			ToolBarDumpScript = gcnew ToolStripSplitButton();
			ToolBarDumpScriptDropDown = gcnew ToolStripDropDown();
			ToolBarDumpAllScripts = gcnew ToolStripButton();
			ToolBarLoadScript = gcnew ToolStripSplitButton();
			ToolBarLoadScriptDropDown = gcnew ToolStripDropDown();
			ToolBarLoadScriptsToTabs = gcnew ToolStripButton();
			ToolBarGetVarIndices = gcnew ToolStripButton();
			ToolBarUpdateVarIndices = gcnew ToolStripButton();
			ToolBarShowOffsets = gcnew ToolStripButton();
			ToolBarShowPreprocessedText = gcnew ToolStripButton();
			ToolBarSanitizeScriptText = gcnew ToolStripButton();
			ToolBarBindScript = gcnew ToolStripButton();
			ToolBarByteCodeSize = gcnew ToolStripProgressBar();

			TextEditorContextMenu = gcnew ContextMenuStrip();
			ContextMenuCopy = gcnew ToolStripMenuItem();
			ContextMenuPaste = gcnew ToolStripMenuItem();
			ContextMenuFind = gcnew ToolStripMenuItem();
			ContextMenuToggleComment = gcnew ToolStripMenuItem();
			ContextMenuToggleBookmark = gcnew ToolStripMenuItem();
			ContextMenuAddMessage = gcnew ToolStripMenuItem();
			ContextMenuWord = gcnew ToolStripMenuItem();
			ContextMenuWikiLookup = gcnew ToolStripMenuItem();
			ContextMenuOBSEDocLookup = gcnew ToolStripMenuItem();
			ContextMenuCopyToCTB = gcnew ToolStripMenuItem();
			ContextMenuDirectLink = gcnew ToolStripMenuItem();
			ContextMenuJumpToScript = gcnew ToolStripMenuItem();
			ContextMenuGoogleLookup = gcnew ToolStripMenuItem();
			ContextMenuRefactorMenu = gcnew ToolStripMenuItem();
			ContextMenuRefactorAddVariable = gcnew ToolStripMenuItem();
			ContextMenuRefactorAddVariableInt = gcnew ToolStripMenuItem();
			ContextMenuRefactorAddVariableFloat = gcnew ToolStripMenuItem();
			ContextMenuRefactorAddVariableRef = gcnew ToolStripMenuItem();
			ContextMenuRefactorAddVariableString = gcnew ToolStripMenuItem();
			ContextMenuRefactorAddVariableArray = gcnew ToolStripMenuItem();
			ContextMenuRefactorDocumentScript = gcnew ToolStripMenuItem();
			ContextMenuRefactorCreateUDFImplementation = gcnew ToolStripMenuItem();
			ContextMenuRefactorRenameVariables = gcnew ToolStripMenuItem();

			ScriptListBox = gcnew ScriptListDialog(Index);

			SetupControlImage(ToolBarNewScript);
			SetupControlImage(ToolBarOpenScript);
			SetupControlImage(ToolBarPreviousScript);
			SetupControlImage(ToolBarNextScript);
			SetupControlImage(ToolBarSaveScript);
			SetupControlImage(ToolBarSaveScriptNoCompile);
			SetupControlImage(ToolBarSaveScriptAndPlugin);
			SetupControlImage(ToolBarRecompileScripts);
			SetupControlImage(ToolBarCompileDependencies);
			SetupControlImage(ToolBarDeleteScript);
			SetupControlImage(ToolBarNavigationBack);
			SetupControlImage(ToolBarNavigationForward);
			SetupControlImage(ToolBarSaveAll);
			SetupControlImage(ToolBarOptions);

			SetupControlImage(ToolBarEditMenu);
			SetupControlImage(ToolBarEditMenuContentsFind);
			SetupControlImage(ToolBarEditMenuContentsReplace);
			SetupControlImage(ToolBarEditMenuContentsGotoLine);
			SetupControlImage(ToolBarEditMenuContentsGotoOffset);

			SetupControlImage(ToolBarMessageList);
			SetupControlImage(ToolBarFindList);
			SetupControlImage(ToolBarBookmarkList);
			SetupControlImage(ToolBarDumpScript);
			SetupControlImage(ToolBarDumpAllScripts);
			SetupControlImage(ToolBarLoadScript);
			SetupControlImage(ToolBarLoadScriptsToTabs);
			SetupControlImage(ToolBarGetVarIndices);
			SetupControlImage(ToolBarUpdateVarIndices);
			SetupControlImage(ToolBarShowOffsets);
			SetupControlImage(ToolBarShowPreprocessedText);
			SetupControlImage(ToolBarSanitizeScriptText);
			SetupControlImage(ToolBarBindScript);

			SetupControlImage(ToolBarScriptTypeContentsObject);
			SetupControlImage(ToolBarScriptTypeContentsQuest);
			SetupControlImage(ToolBarScriptTypeContentsMagicEffect);

			SetupControlImage(ContextMenuCopy);
			SetupControlImage(ContextMenuPaste);
			SetupControlImage(ContextMenuFind);
			SetupControlImage(ContextMenuToggleComment);
			SetupControlImage(ContextMenuToggleBookmark);
			SetupControlImage(ContextMenuAddMessage);
			SetupControlImage(ContextMenuWikiLookup);
			SetupControlImage(ContextMenuOBSEDocLookup);
			SetupControlImage(ContextMenuCopyToCTB);
			SetupControlImage(ContextMenuDirectLink);
			SetupControlImage(ContextMenuJumpToScript);
			SetupControlImage(ContextMenuGoogleLookup);
//			SetupControlImage(ContextMenuRefactorMenu);
			SetupControlImage(ContextMenuRefactorAddVariable);
			SetupControlImage(ContextMenuRefactorDocumentScript);
			SetupControlImage(ContextMenuRefactorCreateUDFImplementation);
			SetupControlImage(ContextMenuRefactorRenameVariables);

			if ( MessageListIcons->Images->Count == 0)
			{
				 MessageListIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListWarning"));
				 MessageListIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListError"));
				 MessageListIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListMessage"));
				 MessageListIcons->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListWarning"));
			}

			EditorControlBox->Dock = DockStyle::Fill;
			EditorControlBox->Location = Point(0, 26);
			EditorControlBox->Padding = Padding(1);
			EditorControlBox->TabItem = EditorTab;

			EditorTab->AttachedControl = EditorControlBox;
			EditorTab->Tooltip = "New Workspace";
			EditorTab->Text = " " + EditorTab->Tooltip;
			EditorTab->Tag = this;

			Padding PrimaryButtonPad = Padding(0);
			PrimaryButtonPad.Right = 5;
			Padding SecondaryButtonPad = Padding(0);
			SecondaryButtonPad.Right = 20;

			ToolStripStatusLabel^ ToolBarSpacerA = gcnew ToolStripStatusLabel();
			ToolBarSpacerA->Spring = true;
			ToolStripStatusLabel^ ToolBarSpacerB = gcnew ToolStripStatusLabel();
			ToolBarSpacerB->Spring = true;

			Color ForeColor = Color::Black;
			Color BackColor = Color::White;
			Color HighlightColor = Color::Maroon;

			Font^ CustomFont = gcnew Font(OPTIONS->FetchSettingAsString("Font"), OPTIONS->FetchSettingAsInt("FontSize"), (FontStyle)OPTIONS->FetchSettingAsInt("FontStyle"));

			TextEditor = gcnew AvalonEditTextEditor(CustomFont, this);
			OffsetViewer = gcnew ScriptOffsetViewer(CustomFont, ForeColor, BackColor, HighlightColor, WorkspaceSplitter->Panel1);
			PreprocessedTextViewer = gcnew SimpleTextViewer(CustomFont, ForeColor, BackColor, HighlightColor, WorkspaceSplitter->Panel1);

			int TabSize = Decimal::ToInt32(OPTIONS->FetchSettingAsInt("TabSize"));
			if (TabSize)
				TextEditor->SetTabCharacterSize(TabSize);

			TextEditor->KeyDown += gcnew KeyEventHandler(this, &Workspace::TextEditor_KeyDown);
			TextEditor->ScriptModified += gcnew ScriptModifiedEventHandler(this, &Workspace::TextEditor_ScriptModified);

			WorkspaceSplitter->Dock = DockStyle::Fill;
			WorkspaceSplitter->SplitterWidth = 3;
			WorkspaceSplitter->Orientation = Orientation::Horizontal;

			WorkspaceMainToolBar->GripStyle = ToolStripGripStyle::Hidden;

			ToolBarCommonTextBox->MaxLength = 500;
			ToolBarCommonTextBox->Width = 300;
			ToolBarCommonTextBox->AutoSize = false;
			ToolBarCommonTextBox->LostFocus += gcnew EventHandler(this, &Workspace::ToolBarCommonTextBox_LostFocus);
			ToolBarCommonTextBox->KeyDown += gcnew KeyEventHandler(this, &Workspace::ToolBarCommonTextBox_KeyDown);
			ToolBarCommonTextBox->Tag = "";
			ToolBarCommonTextBox->Padding = SecondaryButtonPad;
			ToolBarCommonTextBox->ForeColor = Color::Black;
			ToolBarCommonTextBox->BackColor = Color::FromArgb(255, 215, 235, 255);

			ToolStripSeparator^ ToolBarSeparatorA = gcnew ToolStripSeparator();
			ToolStripSeparator^ ToolBarSeparatorB = gcnew ToolStripSeparator();

			ToolBarEditMenuContentsFind->Text = "Find";
			ToolBarEditMenuContentsFind->ToolTipText = "Enter your search string in the adjacent textbox.";
			ToolBarEditMenuContentsFind->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsFind_Click);
			ToolBarEditMenuContentsReplace->Text = "Replace";
			ToolBarEditMenuContentsReplace->ToolTipText = "Enter your search string in the adjacent textbox and the replacement string in the messagebox that pops up.";
			ToolBarEditMenuContentsReplace->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsReplace_Click);
			ToolBarEditMenuContentsGotoLine->Text = "Goto Line";
			ToolBarEditMenuContentsGotoLine->ToolTipText = "Enter the line number to jump to in the adjacent textbox.";
			ToolBarEditMenuContentsGotoLine->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsGotoLine_Click);
			ToolBarEditMenuContentsGotoOffset->Text = "Goto Offset";
			ToolBarEditMenuContentsGotoOffset->ToolTipText = "Enter the offset to jump to in the adjacent textbox, without the hex specifier.";
			ToolBarEditMenuContentsGotoOffset->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsGotoOffset_Click);

			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsFind);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsReplace);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoLine);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoOffset);

			ToolBarEditMenu->Text = "Edit";
			ToolBarEditMenu->DropDown = ToolBarEditMenuContents;

			ToolBarScriptTypeContentsObject->Text = "Object                   ";
			ToolBarScriptTypeContentsObject->ToolTipText = "Object";
			ToolBarScriptTypeContentsObject->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsObject_Click);
			ToolBarScriptTypeContentsQuest->Text = "Quest                    ";
			ToolBarScriptTypeContentsQuest->ToolTipText = "Quest";
			ToolBarScriptTypeContentsQuest->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsQuest_Click);
			ToolBarScriptTypeContentsMagicEffect->Text = "Magic Effect         ";
			ToolBarScriptTypeContentsMagicEffect->ToolTipText = "Magic Effect";
			ToolBarScriptTypeContentsMagicEffect->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsMagicEffect_Click);

			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsObject);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsQuest);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsMagicEffect);

			ToolBarScriptType->ImageTransparentColor = Color::White;
			ToolBarScriptType->Text = "Object Script";
			ToolBarScriptType->DropDown = ToolBarScriptTypeContents;
			Padding TypePad = Padding(0);
			TypePad.Left = 100;
			TypePad.Right = 50;
			ToolBarScriptType->Margin = TypePad;

			ToolBarMessageList->ToolTipText = "Message List";
			ToolBarMessageList->AutoSize = true;
			ToolBarMessageList->Click += gcnew EventHandler(this, &Workspace::ToolBarMessageList_Click);
			ToolBarMessageList->Margin = PrimaryButtonPad;

			ToolBarFindList->ToolTipText = "Find/Replace Results";
			ToolBarFindList->AutoSize = true;
			ToolBarFindList->Click += gcnew EventHandler(this, &Workspace::ToolBarFindList_Click);
			ToolBarFindList->Margin = PrimaryButtonPad;

			ToolBarBookmarkList->ToolTipText = "Bookmarks";
			ToolBarBookmarkList->AutoSize = true;
			ToolBarBookmarkList->Click += gcnew EventHandler(this, &Workspace::ToolBarBookmarkList_Click);
			ToolBarBookmarkList->Margin = SecondaryButtonPad;

			ToolBarDumpScript->ToolTipText = "Dump Script";
			ToolBarDumpScript->AutoSize = true;
			ToolBarDumpScript->ButtonClick += gcnew EventHandler(this, &Workspace::ToolBarDumpScript_Click);
			ToolBarDumpScript->Margin = PrimaryButtonPad;

			ToolBarDumpAllScripts->ToolTipText = "Dump All Tabs";
			ToolBarDumpAllScripts->AutoSize = true;
			ToolBarDumpAllScripts->Click += gcnew EventHandler(this, &Workspace::ToolBarDumpAllScripts_Click);

			ToolBarDumpScriptDropDown->Items->Add(ToolBarDumpAllScripts);
			ToolBarDumpScript->DropDown = ToolBarDumpScriptDropDown;

			ToolBarLoadScript->ToolTipText = "Load Script";
			ToolBarLoadScript->AutoSize = true;
			ToolBarLoadScript->ButtonClick += gcnew EventHandler(this, &Workspace::ToolBarLoadScript_Click);
			ToolBarLoadScript->Margin = SecondaryButtonPad;

			ToolBarLoadScriptsToTabs->ToolTipText = "Load Multiple Scripts Into Tabs";
			ToolBarLoadScriptsToTabs->AutoSize = true;
			ToolBarLoadScriptsToTabs->Click += gcnew EventHandler(this, &Workspace::ToolBarLoadScriptsToTabs_Click);

			ToolBarLoadScriptDropDown->Items->Add(ToolBarLoadScriptsToTabs);
			ToolBarLoadScript->DropDown = ToolBarLoadScriptDropDown;

			ToolBarOptions->ToolTipText = "Preferences";
			ToolBarOptions->Click += gcnew EventHandler(this, &Workspace::ToolBarOptions_Click);
			ToolBarOptions->Alignment = ToolStripItemAlignment::Right;

			ToolBarNewScript->ToolTipText = "New Script";
			ToolBarNewScript->AutoSize = true;
			ToolBarNewScript->Click += gcnew EventHandler(this, &Workspace::ToolBarNewScript_Click);
			ToolBarNewScript->Margin = SecondaryButtonPad;

			ToolBarOpenScript->ToolTipText = "Open Script";
			ToolBarOpenScript->AutoSize = true;
			ToolBarOpenScript->Click += gcnew EventHandler(this, &Workspace::ToolBarOpenScript_Click);
			ToolBarOpenScript->Margin = SecondaryButtonPad;

			ToolBarPreviousScript->ToolTipText = "Previous Script";
			ToolBarPreviousScript->AutoSize = true;
			ToolBarPreviousScript->Click += gcnew EventHandler(this, &Workspace::ToolBarPreviousScript_Click);
			ToolBarPreviousScript->Margin = PrimaryButtonPad;

			ToolBarNextScript->ToolTipText = "Next Script";
			ToolBarNextScript->AutoSize = true;
			ToolBarNextScript->Click += gcnew EventHandler(this, &Workspace::ToolBarNextScript_Click);
			ToolBarNextScript->Margin = SecondaryButtonPad;

			ToolBarSaveScript->ToolTipText = "Compile and Save Script";
			ToolBarSaveScript->AutoSize = true;
			ToolBarSaveScript->ButtonClick += gcnew EventHandler(this, &Workspace::ToolBarSaveScript_Click);
			ToolBarSaveScript->Margin = SecondaryButtonPad;

			ToolBarSaveScriptNoCompile->ToolTipText = "Save But Don't Compile Script";
			ToolBarSaveScriptNoCompile->AutoSize = true;
			ToolBarSaveScriptNoCompile->Click += gcnew EventHandler(this, &Workspace::ToolBarSaveScriptNoCompile_Click);

			ToolBarSaveScriptAndPlugin->ToolTipText = "Save Script and Active Plugin";
			ToolBarSaveScriptAndPlugin->AutoSize = true;
			ToolBarSaveScriptAndPlugin->Click += gcnew EventHandler(this, &Workspace::ToolBarSaveScriptAndPlugin_Click);

			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptNoCompile);
			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptAndPlugin);
			ToolBarSaveScript->DropDown = ToolBarSaveScriptDropDown;

			ToolBarRecompileScripts->ToolTipText = "Recompile Active Scripts";
			ToolBarRecompileScripts->AutoSize = true;
			ToolBarRecompileScripts->Click += gcnew EventHandler(this, &Workspace::ToolBarRecompileScripts_Click);
			ToolBarRecompileScripts->Margin = SecondaryButtonPad;

			ToolBarDeleteScript->ToolTipText = "Delete Script";
			ToolBarDeleteScript->AutoSize = true;
			ToolBarDeleteScript->Click += gcnew EventHandler(this, &Workspace::ToolBarDeleteScript_Click);
			ToolBarDeleteScript->Margin = PrimaryButtonPad;

			ToolBarShowOffsets->ToolTipText = "Toggle Offset Viewer";
			ToolBarShowOffsets->AutoSize = true;
			ToolBarShowOffsets->Click += gcnew EventHandler(this, &Workspace::ToolBarShowOffsets_Click);

			Padding OffsetPad = Padding(0);
			OffsetPad.Left = 17;
			OffsetPad.Right = 5;
			ToolBarShowOffsets->Margin = OffsetPad;

			ToolBarShowPreprocessedText->ToolTipText = "Toggle Preprocessed Text Viewer";
			ToolBarShowPreprocessedText->AutoSize = true;
			ToolBarShowPreprocessedText->Click += gcnew EventHandler(this, &Workspace::ToolBarShowPreprocessedText_Click);

			ToolBarSanitizeScriptText->ToolTipText = "Sanitize Script Text";
			ToolBarSanitizeScriptText->AutoSize = true;
			ToolBarSanitizeScriptText->Click += gcnew EventHandler(this, &Workspace::ToolBarSanitizeScriptText_Click);

			Padding SanitizePad = Padding(0);
			SanitizePad.Left = 17;
			SanitizePad.Right = 5;
			ToolBarSanitizeScriptText->Margin = SanitizePad;

			ToolBarBindScript->ToolTipText = "Bind Script";
			ToolBarBindScript->AutoSize = true;
			ToolBarBindScript->Click += gcnew EventHandler(this, &Workspace::ToolBarBindScript_Click);
			ToolBarBindScript->Margin = SecondaryButtonPad;

			ToolBarNavigationBack->ToolTipText = "Navigate Back";
			ToolBarNavigationBack->AutoSize = true;
			ToolBarNavigationBack->Click += gcnew EventHandler(this, &Workspace::ToolBarNavigationBack_Click);
			ToolBarNavigationBack->Margin = PrimaryButtonPad;
			ToolBarNavigationBack->Alignment = ToolStripItemAlignment::Right;

			ToolBarNavigationForward->ToolTipText = "Navigate Forward";
			ToolBarNavigationForward->AutoSize = true;
			ToolBarNavigationForward->Click += gcnew EventHandler(this, &Workspace::ToolBarNavigationForward_Click);
			ToolBarNavigationForward->Margin = SecondaryButtonPad;
			ToolBarNavigationForward->Alignment = ToolStripItemAlignment::Right;

			ToolBarByteCodeSize->Minimum = 0;
			ToolBarByteCodeSize->Maximum = 0x8000;
			ToolBarByteCodeSize->AutoSize = false;
			ToolBarByteCodeSize->Size = Size(85, 13);
			ToolBarByteCodeSize->ToolTipText = "Compiled Script Size";
			ToolBarByteCodeSize->Alignment = ToolStripItemAlignment::Right;
			ToolBarByteCodeSize->Margin = SecondaryButtonPad;

			ToolBarGetVarIndices->ToolTipText = "Fetch Variable Indices";
			ToolBarGetVarIndices->AutoSize = true;
			ToolBarGetVarIndices->Click += gcnew EventHandler(this, &Workspace::ToolBarGetVarIndices_Click);
			ToolBarGetVarIndices->Margin = PrimaryButtonPad;

			ToolBarUpdateVarIndices->ToolTipText = "Update Variable Indices";
			ToolBarUpdateVarIndices->AutoSize = true;
			ToolBarUpdateVarIndices->Click += gcnew EventHandler(this, &Workspace::ToolBarUpdateVarIndices_Click);
			ToolBarUpdateVarIndices->Margin = PrimaryButtonPad;

			ToolBarSaveAll->ToolTipText = "Save All Open Scripts";
			ToolBarSaveAll->AutoSize = true;
			ToolBarSaveAll->Click += gcnew EventHandler(this, &Workspace::ToolBarSaveAll_Click);
			ToolBarSaveAll->Margin = SecondaryButtonPad;
			ToolBarSaveAll->Alignment = ToolStripItemAlignment::Right;

			ToolBarCompileDependencies->ToolTipText = "Compile Dependencies";
			ToolBarCompileDependencies->AutoSize = true;
			ToolBarCompileDependencies->Click += gcnew EventHandler(this, &Workspace::ToolBarCompileDependencies_Click);
			ToolBarCompileDependencies->Margin = SecondaryButtonPad;

			WorkspaceMainToolBar->Dock = DockStyle::Top;
			WorkspaceMainToolBar->Items->Add(ToolBarNewScript);
			WorkspaceMainToolBar->Items->Add(ToolBarOpenScript);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveScript);
			WorkspaceMainToolBar->Items->Add(ToolBarPreviousScript);
			WorkspaceMainToolBar->Items->Add(ToolBarNextScript);
			WorkspaceMainToolBar->Items->Add(ToolBarRecompileScripts);
			WorkspaceMainToolBar->Items->Add(ToolBarCompileDependencies);
			WorkspaceMainToolBar->Items->Add(ToolBarDeleteScript);
			WorkspaceMainToolBar->Items->Add(ToolBarScriptType);
			WorkspaceMainToolBar->Items->Add(ToolBarSpacerA);
			WorkspaceMainToolBar->Items->Add(ToolBarOptions);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationForward);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationBack);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveAll);
			WorkspaceMainToolBar->ShowItemToolTips = true;

			WorkspaceSecondaryToolBar->GripStyle = ToolStripGripStyle::Hidden;
			WorkspaceSecondaryToolBar->Dock = DockStyle::Top;
			WorkspaceSecondaryToolBar->Items->Add(ToolBarEditMenu);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarCommonTextBox);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarMessageList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarFindList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBookmarkList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarDumpScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarLoadScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarGetVarIndices);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarUpdateVarIndices);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSpacerB);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowOffsets);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowPreprocessedText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSanitizeScriptText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBindScript);

			WorkspaceSecondaryToolBar->Items->Add(ToolBarByteCodeSize);
			WorkspaceSecondaryToolBar->ShowItemToolTips = true;

			ContextMenuCopy->Click += gcnew EventHandler(this, &Workspace::ContextMenuCopy_Click);
			ContextMenuCopy->Text = "Copy";

			ContextMenuPaste->Click += gcnew EventHandler(this, &Workspace::ContextMenuPaste_Click);
			ContextMenuPaste->Text = "Paste";

			ContextMenuWord->Enabled = false;

			ContextMenuWikiLookup->Click += gcnew EventHandler(this, &Workspace::ContextMenuWikiLookup_Click);
			ContextMenuWikiLookup->Text = "Look up on the Wiki";

			ContextMenuOBSEDocLookup->Click += gcnew EventHandler(this, &Workspace::ContextMenuOBSEDocLookup_Click);
			ContextMenuOBSEDocLookup->Text = "Look up on the OBSE Doc";

			ContextMenuCopyToCTB->Click += gcnew EventHandler(this, &Workspace::ContextMenuCopyToCTB_Click);
			ContextMenuCopyToCTB->Text = "Copy to Edit Box";

			ContextMenuFind->Click += gcnew EventHandler(this, &Workspace::ContextMenuFind_Click);
			ContextMenuFind->Text = "Find";

			ContextMenuToggleComment->Click += gcnew EventHandler(this, &Workspace::ContextMenuToggleComment_Click);
			ContextMenuToggleComment->Text = "Toggle Comment";

			ContextMenuToggleBookmark->Click += gcnew EventHandler(this, &Workspace::ContextMenuToggleBookmark_Click);
			ContextMenuToggleBookmark->Text = "Toggle Bookmark";

			ContextMenuDirectLink->Click += gcnew EventHandler(this, &Workspace::ContextMenuDirectLink_Click);
			ContextMenuDirectLink->Text = "Developer Page";

			ContextMenuJumpToScript->Click += gcnew EventHandler(this, &Workspace::ContextMenuJumpToScript_Click);
			ContextMenuJumpToScript->Text = "Jump into Script";

			ContextMenuAddMessage->Click += gcnew EventHandler(this, &Workspace::ContextMenuAddMessage_Click);
			ContextMenuAddMessage->Text = "Add Message";

			ContextMenuGoogleLookup->Click += gcnew EventHandler(this, &Workspace::ContextMenuGoogleLookup_Click);
			ContextMenuGoogleLookup->Text = "Look up on Google";

			ContextMenuRefactorMenu->Text = "Refactor ...";
			ContextMenuRefactorMenu->DropDownItems->Add(ContextMenuRefactorAddVariable);
			ContextMenuRefactorMenu->DropDownItems->Add(ContextMenuRefactorDocumentScript);
			ContextMenuRefactorMenu->DropDownItems->Add(ContextMenuRefactorCreateUDFImplementation);
			ContextMenuRefactorMenu->DropDownItems->Add(ContextMenuRefactorRenameVariables);

			ContextMenuRefactorAddVariable->Text = "Add Variable ...";
			ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableInt);
			ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableFloat);
			ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableRef);
			ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableString);
			ContextMenuRefactorAddVariable->DropDownItems->Add(ContextMenuRefactorAddVariableArray);

			ContextMenuRefactorAddVariableInt->Text = "Integer";
			ContextMenuRefactorAddVariableInt->Tag = ScriptParser::VariableType::e_Integer;
			ContextMenuRefactorAddVariableInt->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorAddVariable_Click);

			ContextMenuRefactorAddVariableFloat->Text = "Float";
			ContextMenuRefactorAddVariableFloat->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorAddVariable_Click);
			ContextMenuRefactorAddVariableFloat->Tag = ScriptParser::VariableType::e_Float;

			ContextMenuRefactorAddVariableRef->Text = "Reference";
			ContextMenuRefactorAddVariableRef->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorAddVariable_Click);
			ContextMenuRefactorAddVariableRef->Tag = ScriptParser::VariableType::e_Ref;

			ContextMenuRefactorAddVariableString->Text = "String";
			ContextMenuRefactorAddVariableString->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorAddVariable_Click);
			ContextMenuRefactorAddVariableString->Tag = ScriptParser::VariableType::e_String;

			ContextMenuRefactorAddVariableArray->Text = "Array";
			ContextMenuRefactorAddVariableArray->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorAddVariable_Click);
			ContextMenuRefactorAddVariableArray->Tag = ScriptParser::VariableType::e_Array;

			ContextMenuRefactorDocumentScript->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorDocumentScript_Click);
			ContextMenuRefactorDocumentScript->Text = "Document Script";

			ContextMenuRefactorCreateUDFImplementation->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorCreateUDFImplementation_Click);
			ContextMenuRefactorCreateUDFImplementation->Text = "Create UFD Implementation";

			ContextMenuRefactorRenameVariables->Click += gcnew EventHandler(this, &Workspace::ContextMenuRefactorRenameVariables_Click);
			ContextMenuRefactorRenameVariables->Text = "Rename Variables";

			TextEditorContextMenu->Items->Add(ContextMenuRefactorMenu);
			TextEditorContextMenu->Items->Add(ContextMenuCopy);
			TextEditorContextMenu->Items->Add(ContextMenuPaste);
			TextEditorContextMenu->Items->Add(ContextMenuFind);
			TextEditorContextMenu->Items->Add(ContextMenuToggleComment);
			TextEditorContextMenu->Items->Add(ContextMenuToggleBookmark);
			TextEditorContextMenu->Items->Add(ContextMenuAddMessage);
			TextEditorContextMenu->Items->Add(ToolBarSeparatorA);
			TextEditorContextMenu->Items->Add(ContextMenuWord);
			TextEditorContextMenu->Items->Add(ContextMenuCopyToCTB);
			TextEditorContextMenu->Items->Add(ContextMenuWikiLookup);
			TextEditorContextMenu->Items->Add(ContextMenuOBSEDocLookup);
			TextEditorContextMenu->Items->Add(ContextMenuGoogleLookup);
			TextEditorContextMenu->Items->Add(ContextMenuDirectLink);
			TextEditorContextMenu->Items->Add(ContextMenuJumpToScript);

			TextEditorContextMenu->Opening += gcnew CancelEventHandler(this, &Workspace::TextEditorContextMenu_Opening);
			TextEditorContextMenu->Renderer = gcnew ToolStripProfessionalRenderer();

			MessageList->Font = CustomFont;
			MessageList->Dock = DockStyle::Fill;
			MessageList->BorderStyle = BorderStyle::Fixed3D;
			MessageList->BackColor = BackColor;
			MessageList->ForeColor = ForeColor;
			MessageList->DoubleClick += gcnew EventHandler(this, &Workspace::MessageList_DoubleClick);
			MessageList->Visible = false;
			MessageList->View = View::Details;
			MessageList->MultiSelect = false;
			MessageList->CheckBoxes = false;
			MessageList->FullRowSelect = true;
			MessageList->HideSelection = false;
			MessageList->SmallImageList =  MessageListIcons;

			ColumnHeader^ MessageListType = gcnew ColumnHeader();
			MessageListType->Text = " ";
			MessageListType->Width = 25;
			ColumnHeader^ MessageListLine = gcnew ColumnHeader();
			MessageListLine->Text = "Line";
			MessageListLine->Width = 50;
			ColumnHeader^ MessageListMessage = gcnew ColumnHeader();
			MessageListMessage->Text = "Message";
			MessageListMessage->Width = 750;
			MessageList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(3) {MessageListType,
																				 MessageListLine,
																				 MessageListMessage});
			MessageList->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::MessageList_ColumnClick);
			MessageList->Tag = (int)1;

			FindList->Font = CustomFont;
			FindList->Dock = DockStyle::Fill;
			FindList->BorderStyle = BorderStyle::Fixed3D;
			FindList->BackColor = BackColor;
			FindList->ForeColor = ForeColor;
			FindList->DoubleClick += gcnew EventHandler(this, &Workspace::FindList_DoubleClick);
			FindList->Visible = false;
			FindList->View = View::Details;
			FindList->MultiSelect = false;
			FindList->CheckBoxes = false;
			FindList->FullRowSelect = true;
			FindList->HideSelection = false;

			ColumnHeader^ FindListLine = gcnew ColumnHeader();
			FindListLine->Text = "Line";
			FindListLine->Width = 50;
			ColumnHeader^ FindListLineContent = gcnew ColumnHeader();
			FindListLineContent->Text = "Text";
			FindListLineContent->Width = 300;
			FindList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(2) {FindListLine,
																			  FindListLineContent});
			FindList->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::FindList_ColumnClick);
			FindList->Tag = (int)1;

			BookmarkList->Font = CustomFont;
			BookmarkList->Dock = DockStyle::Fill;
			BookmarkList->BorderStyle = BorderStyle::Fixed3D;
			BookmarkList->BackColor = BackColor;
			BookmarkList->ForeColor = ForeColor;
			BookmarkList->DoubleClick += gcnew EventHandler(this, &Workspace::BookmarkList_DoubleClick);
			BookmarkList->Visible = false;
			BookmarkList->View = View::Details;
			BookmarkList->MultiSelect = false;
			BookmarkList->CheckBoxes = false;
			BookmarkList->FullRowSelect = true;
			BookmarkList->HideSelection = false;
			BookmarkList->Tag = (int)1;

			ColumnHeader^ BookmarkListLine = gcnew ColumnHeader();
			BookmarkListLine->Text = "Line";
			BookmarkListLine->Width = 50;
			ColumnHeader^ BookmarkListDesc = gcnew ColumnHeader();
			BookmarkListDesc->Text = "Description";
			BookmarkListDesc->Width = 300;
			BookmarkList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(2) {BookmarkListLine,
																				 BookmarkListDesc});
			BookmarkList->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::BookmarkList_ColumnClick);

			VariableIndexList->Font = CustomFont;
			VariableIndexList->Dock = DockStyle::Fill;
			VariableIndexList->BorderStyle = BorderStyle::Fixed3D;
			VariableIndexList->BackColor = BackColor;
			VariableIndexList->ForeColor = ForeColor;
			VariableIndexList->DoubleClick += gcnew EventHandler(this, &Workspace::VariableIndexList_DoubleClick);
			VariableIndexList->Visible = false;
			VariableIndexList->View = View::Details;
			VariableIndexList->MultiSelect = false;
			VariableIndexList->CheckBoxes = false;
			VariableIndexList->FullRowSelect = true;
			VariableIndexList->HideSelection = false;
			VariableIndexList->Tag = (int)1;

			ColumnHeader^ VariableIndexListName = gcnew ColumnHeader();
			VariableIndexListName->Text = "Variable Name";
			VariableIndexListName->Width = 300;
			ColumnHeader^ VariableIndexListType = gcnew ColumnHeader();
			VariableIndexListType->Text = "Type";
			VariableIndexListType->Width = 300;
			ColumnHeader^ VariableIndexListIndex = gcnew ColumnHeader();
			VariableIndexListIndex->Text = "Index";
			VariableIndexListIndex->Width = 100;
			VariableIndexList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(3) {VariableIndexListName,
																				 VariableIndexListType,
																				 VariableIndexListIndex});
			VariableIndexList->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::VariableIndexList_ColumnClick);

			VariableIndexEditBox->Font = CustomFont;
			VariableIndexEditBox->Multiline = true;
			VariableIndexEditBox->BorderStyle = BorderStyle::FixedSingle;
			VariableIndexEditBox->Visible = false;
			VariableIndexEditBox->AcceptsReturn = true;
			VariableIndexEditBox->LostFocus += gcnew EventHandler(this, &Workspace::VariableIndexEditBox_LostFocus);
			VariableIndexEditBox->KeyDown += gcnew KeyEventHandler(this, &Workspace::VariableIndexEditBox_KeyDown);

			VariableIndexList->Controls->Add(VariableIndexEditBox);

			SpoilerText->Dock = DockStyle::Fill;
			SpoilerText->TextAlign = ContentAlignment::MiddleCenter;
			SpoilerText->Text = "Right, everybody out! Smash the Spinning Jenny! Burn the rolling Rosalind! Destroy the going-up-and-down-a-bit-and-then-moving-along Gertrude! And death to the stupid Prince who grows fat on the profits!";

			WorkspaceSplitter->Panel1->Controls->Add(TextEditor->GetContainer());
			WorkspaceSplitter->Panel2->Controls->Add(WorkspaceSecondaryToolBar);
			WorkspaceSplitter->Panel2->Controls->Add(MessageList);
			WorkspaceSplitter->Panel2->Controls->Add(FindList);
			WorkspaceSplitter->Panel2->Controls->Add(BookmarkList);
			WorkspaceSplitter->Panel2->Controls->Add(VariableIndexList);
			WorkspaceSplitter->Panel2->Controls->Add(SpoilerText);

			EditorControlBox->Controls->Add(WorkspaceSplitter);
			EditorControlBox->Controls->Add(WorkspaceMainToolBar);

			Parent->AddTab(EditorTab);
			Parent->AddTabControlBox(EditorControlBox);

			try { WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height; }
			catch (...) {}

			DisableControls();
			ToolBarUpdateVarIndices->Enabled = false;
			TextEditor->SetContextMenu(TextEditorContextMenu);

			AllocatedIndex = Index;
			DestructionFlag = false;
			CurrentScriptEditorID = FIRSTRUNSCRIPTID;
			HandlingKeyDownEvent = false;
			NewScriptFlag = false;

			SetScriptType(ScriptType::e_Object);
			SetModifiedStatus(false);

			if (InitScript && InitScript->ParentForm)
				UpdateEnvironment(InitScript, true);
			else
				CurrentScript = 0;

			g_CSEInterface->DeleteNativeHeapPointer(InitScript, false);
		}

		#pragma region Methods
			void Workspace::DisableControls()
			{
				WorkspaceSplitter->Panel1->Enabled = false;
				WorkspaceSplitter->Panel2->Enabled = false;
				TextEditor->SetEnabledState(false);
			}
			void Workspace::EnableControls()
			{
				WorkspaceSplitter->Panel1->Enabled = true;
				WorkspaceSplitter->Panel2->Enabled = true;
				TextEditor->SetEnabledState(true);
			}
			void Workspace::ClearErrorsItemsFromMessagePool(void)
			{
				LinkedList<ListViewItem^>^ InvalidItems = gcnew LinkedList<ListViewItem^>();

				for each (ListViewItem^ Itr in MessageList->Items)
				{
					if (Itr->ImageIndex < (int)MessageType::e_Message)
						InvalidItems->AddLast(Itr);
				}

				for each (ListViewItem^ Itr in InvalidItems)
					MessageList->Items->Remove(Itr);

				TextEditor->ClearScriptErrorHighlights();
			}
			void Workspace::FindReplaceOutput(String^ Line, String^ Text)
			{
				ListViewItem^ Item = gcnew ListViewItem(Line);
				Item->SubItems->Add(Text);
				FindList->Items->Add(Item);
			}
			void Workspace::FindReplaceWrapper(IScriptTextEditor::FindReplaceOperation Operation)
			{
				String^ SearchString = ToolBarCommonTextBox->Text;
				String^ ReplaceString = "";

				if (SearchString == "")
				{
					MessageBox::Show("Enter a valid search string.", "Find and Replace - CSE Script Editor");
					return;
				}

				if (Operation == IScriptTextEditor::FindReplaceOperation::e_Replace)
				{
					InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter Replacement String", "Find and Replace - CSE Script Editor");
					if (Result->ReturnCode == DialogResult::Cancel)
						return;
					else
						ReplaceString = Result->Text;
				}

				FindList->Items->Clear();
				UInt32 Hits = TextEditor->FindReplace(Operation,
											SearchString,
											ReplaceString,
											gcnew IScriptTextEditor::FindReplaceOutput(this, &ScriptEditor::Workspace::FindReplaceOutput));

				if (Hits > 0 && FindList->Visible == false)
					ToolBarFindList->PerformClick();
			}
			void Workspace::ToggleBookmark(int CaretPos)
			{
				int LineNo = TextEditor->GetLineNumberFromCharIndex(CaretPos) + 1, Count = 0;
				for each (ListViewItem^ Itr in BookmarkList->Items)
				{
					if (int::Parse(Itr->SubItems[0]->Text) == LineNo)
					{
						BookmarkList->Items->RemoveAt(Count);
						return;
					}
					Count++;
				}

				String^ BookmarkDesc = "";
				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter A Description For The Bookmark", "Place Bookmark - CSE Script Editor");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;
				else
					BookmarkDesc = Result->Text;

				ListViewItem^ Item = gcnew ListViewItem(LineNo.ToString());
				Item->SubItems->Add(BookmarkDesc);
				BookmarkList->Items->Add(Item);

				if (!BookmarkList->Visible)
					ToolBarBookmarkList->PerformClick();
			}
			void Workspace::SetScriptType(Workspace::ScriptType Type)
			{
				switch (Type)
				{
				case ScriptType::e_Object:
					ToolBarScriptType->Text = "Object Script";
					ToolBarScriptType->Image = ToolBarScriptTypeContentsObject->Image;
					break;
				case ScriptType::e_Quest:
					ToolBarScriptType->Text = "Quest Script";
					ToolBarScriptType->Image = ToolBarScriptTypeContentsQuest->Image;
					break;
				case ScriptType::e_MagicEffect:
					ToolBarScriptType->Text = "Magic Effect Script";
					ToolBarScriptType->Image = ToolBarScriptTypeContentsMagicEffect->Image;
					break;
				}

				CurrentScriptType = Type;
			}
			String^ Workspace::SerializeCSEBlock(void)
			{
				String^ Block = "";
				String^ Result = "";

				SerializeCaretPos(Block);
				SerializeBookmarks(Block);
				SerializeMessages(Block);

				if (Block != "")
				{
					Result += "\n;<CSEBlock>\n";
					Result += Block;
					Result += ";</CSEBlock>";
				}

				return Result;
			}
			void Workspace::SerializeCaretPos(String^% Result)
			{
				if (OPTIONS->FetchSettingAsInt("SaveLastKnownPos"))
				{
					Result += String::Format(";<CSECaretPos> {0} </CSECaretPos>\n", TextEditor->GetCaretPos());
				}
			}
			void Workspace::SerializeBookmarks(String^% Result)
			{
				String^ BookmarkSegment = "";

				for each (ListViewItem^ Itr in BookmarkList->Items)
				{
					BookmarkSegment += ";<CSEBookmark>\t" + Itr->SubItems[0]->Text + "\t" + Itr->SubItems[1]->Text + "\t</CSEBookmark>\n";
				}
				Result += BookmarkSegment;
			}
			void Workspace::SerializeMessages(String^% Result)
			{
				for each (ListViewItem^ Itr in MessageList->Items)
				{
					if (Itr->ImageIndex > (int)MessageType::e_Error)
					switch ((MessageType)Itr->ImageIndex)
					{
					case MessageType::e_CSEMessage:
						Result += ";<CSEMessageEditor> " + Itr->SubItems[2]->Text + " </CSEMessageEditor>\n";
						break;
					case MessageType::e_Message:
						Result += ";<CSEMessageRegular> " + Itr->SubItems[2]->Text + " </CSEMessageRegular>\n";
						break;
					}
				}
			}
			String^ Workspace::DeserializeCSEBlock(String^% Source, String^% ExtractedBlock)
			{
				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ StringParser = gcnew StringReader(Source);
				String^ ReadLine = StringParser->ReadLine();
				String^ Block = "";
				String^ Result = "";
				bool ExtractingBlock = false;

				while (ReadLine != nullptr)
				{
					TextParser->Tokenize(ReadLine, false);

					if (ExtractingBlock)
					{
						if (!TextParser->GetTokenIndex(";</CSEBlock>"))
							ExtractingBlock = false;
						else
							Block += ReadLine + "\n";

						ReadLine = StringParser->ReadLine();
						continue;
					}

					if (!TextParser->Valid)
					{
						Result += "\n" + ReadLine;
						ReadLine = StringParser->ReadLine();
						continue;
					}
					else if (!TextParser->GetTokenIndex(";<CSEBlock>"))
					{
						ExtractingBlock = true;
						ReadLine = StringParser->ReadLine();
						continue;
					}

					Result += "\n" + ReadLine;
					ReadLine = StringParser->ReadLine();
				}

				ExtractedBlock = Block;
				if (Result == "")
					return Result;
				else
					return Result->Substring(1);
			}
			void Workspace::DeserializeCaretPos(String^% ExtractedBlock)
			{
				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ StringParser = gcnew StringReader(ExtractedBlock);
				String^ ReadLine = StringParser->ReadLine();
				int CaretPos = 0;

				while (ReadLine != nullptr)
				{
					TextParser->Tokenize(ReadLine, false);
					if (!TextParser->Valid)
					{
						ReadLine = StringParser->ReadLine();
						continue;
					}

					if (!TextParser->GetTokenIndex(";<CSECaretPos>"))
					{
						try { CaretPos = int::Parse(TextParser->Tokens[1]); } catch (...) { CaretPos = -1; }
						break;
					}

					ReadLine = StringParser->ReadLine();
				}

				TextEditor->SetCaretPos(CaretPos);
				TextEditor->ScrollToCaret();
			}
			void Workspace::DeserializeBookmarks(String^% ExtractedBlock)
			{
				BookmarkList->Items->Clear();
				BookmarkList->BeginUpdate();

				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ StringParser = gcnew StringReader(ExtractedBlock);
				String^ ReadLine = StringParser->ReadLine();
				int LineNo = 0;

				while (ReadLine != nullptr)
				{
					TextParser->Tokenize(ReadLine, false);
					if (!TextParser->Valid)
					{
						ReadLine = StringParser->ReadLine();
						continue;
					}

					if (!TextParser->GetTokenIndex(";<CSEBookmark>"))
					{
						array<String^>^ Splits = ReadLine->Substring(TextParser->Indices[0])->Split((String("\t")).ToCharArray());
						try
						{
							LineNo = int::Parse(Splits[1]);
						}
						catch (...)
						{
							LineNo = 1;
						}

						ListViewItem^ Item = gcnew ListViewItem(LineNo.ToString());
						Item->SubItems->Add(Splits[2]);
						BookmarkList->Items->Add(Item);
					}

					ReadLine = StringParser->ReadLine();
				}
				BookmarkList->EndUpdate();
			}
			void Workspace::DeserializeMessages(String^% ExtractedBlock)
			{
				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ StringParser = gcnew StringReader(ExtractedBlock);
				String^ ReadLine = StringParser->ReadLine();

				while (ReadLine != nullptr)
				{
					TextParser->Tokenize(ReadLine, false);
					if (!TextParser->Valid) {
						ReadLine = StringParser->ReadLine();
						continue;
					}

					String^ Message = "";
					if (!TextParser->GetTokenIndex(";<CSEMessageEditor>"))
					{
						Message = ReadLine->Substring(TextParser->Indices[1])->Replace(" </CSEMessageEditor>", "");
						AddMessageToMessagePool(MessageType::e_CSEMessage, -1, Message);
					}
					else if (!TextParser->GetTokenIndex(";<CSEMessageRegular>"))
					{
						Message = ReadLine->Substring(TextParser->Indices[1])->Replace(" </CSEMessageRegular>", "");
						AddMessageToMessagePool(MessageType::e_Message, -1, Message);
					}

					ReadLine = StringParser->ReadLine();
				}
			}
			void Workspace::PreprocessorErrorOutputWrapper(String^ Message)
			{
				AddMessageToMessagePool(MessageType::e_Error, -1, Message);
			}
			void Workspace::SanitizeScriptText(SanitizeOperation Operation)
			{
				String^ SanitizedScriptText = "";

				StringReader^ TextReader = gcnew StringReader(TextEditor->GetText());
				ScriptParser^ LocalParser = gcnew ScriptParser();

				int IndentCount = 0;
				for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
				{
					switch (Operation)
					{
					case SanitizeOperation::e_Indent:
						{
							int IndentMode = -1;		// 0 = Decrement
														// 1 = Post-Increment
														// 2 = Decrement&Increment

							LocalParser->Tokenize(ReadLine, false);
							if (LocalParser->Valid)
							{
								String^ Token = LocalParser->Tokens[0];
								if (Token[0] != ';')
								{
									if (!String::Compare(Token, "begin", true) ||
										!String::Compare(Token, "if", true) ||
										!String::Compare(Token, "while", true) ||
										!String::Compare(Token, "forEach", true))
									{
										IndentMode = 1;
									}
									else if	(!String::Compare(Token, "loop", true) ||
											!String::Compare(Token, "endIf", true) ||
											!String::Compare(Token, "end", true))
									{
										IndentMode = 0;
									}
									else if	(!String::Compare(Token, "else", true) ||
											!String::Compare(Token, "elseIf", true))
									{
										IndentMode = 2;
									}

									if (IndentMode == 0 || IndentMode == 2)
										IndentCount--;

									for (int i = 0; i < IndentCount; i++)
										SanitizedScriptText += "\t";
									SanitizedScriptText += ReadLine->Substring(LocalParser->Indices[0]) + "\n";

									if (IndentMode == 1 || IndentMode == 2)
										IndentCount++;
								}
								else
									SanitizedScriptText += ReadLine + "\n";
							}
							else
								SanitizedScriptText += ReadLine + "\n";

							break;
						}
					case SanitizeOperation::e_AnnealCasing:
						{
							LocalParser->Tokenize(ReadLine, true);
							if (LocalParser->Valid)
							{
								for (int i = 0; i < LocalParser->GetCurrentTokenCount(); i++)
								{
									String^ Token = LocalParser->Tokens[i];
									String^ Delimiter = "" + LocalParser->Delimiters[i];

									if (LocalParser->GetCommentTokenIndex(i) == -1 && ISDB->GetIsIdentifierScriptCommand(Token))
										SanitizedScriptText += ISDB->SanitizeCommandIdentifier(Token);
									else
										SanitizedScriptText += Token;

									SanitizedScriptText += Delimiter;
								}
							}
							else
								SanitizedScriptText += ReadLine + "\n";

							break;
						}
					}
				}

				if (SanitizedScriptText->Length > 0 && SanitizedScriptText[SanitizedScriptText->Length - 1] == '\n')
					SanitizedScriptText = SanitizedScriptText->Substring(0, SanitizedScriptText->Length - 1);

				TextEditor->SetText(SanitizedScriptText, true);
			}
			void Workspace::UpdateEnvironment(ComponentDLLInterface::ScriptData* Data, bool Initializing)
			{
				String^ ScriptText = gcnew String(Data->Text);
				UInt16 ScriptType = Data->Type;
				String^ ScriptName = gcnew String(Data->EditorID);
				UInt32 ByteCode = (UInt32)Data->ByteCode;
				UInt32 ByteCodeLength = Data->Length;
				UInt32 FormID = Data->FormID;

				if (ScriptName->Length == 0)
					ScriptName = NEWSCRIPTID;

				if (Initializing)
				{
					CurrentScript = Data->ParentForm;
					NewScriptFlag = false;

					if (ScriptName != NEWSCRIPTID)
						TextEditor->SetInitializingStatus(true);

					TextEditor->ClearScriptErrorHighlights();
					MessageList->Items->Clear();
					VariableIndexList->Items->Clear();

					String^ CSEBlock = "";
					String^ DeserializedText = DeserializeCSEBlock(ScriptText, CSEBlock);
					TextEditor->SetText(DeserializedText, false);

					DeserializeCaretPos(CSEBlock);
					DeserializeBookmarks(CSEBlock);
					DeserializeMessages(CSEBlock);

					EnableControls();
				}

				CurrentScriptEditorID = ScriptName;
				EditorTab->Tooltip = ScriptName + " [" + FormID.ToString("X8") + "]";
				EditorTab->Text = " " + ScriptName;
				ParentContainer->SetWindowTitle(EditorTab->Tooltip + " - CSE Script Editor");
				SetScriptType((Workspace::ScriptType)ScriptType);

				SetModifiedStatus(false);

				ToolBarByteCodeSize->Value = ByteCodeLength;
				ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(ByteCodeLength / 1024.0));

				OffsetViewer->InitializeViewer(ScriptText, ByteCode, ByteCodeLength);
				TextEditor->UpdateIntelliSenseLocalDatabase();
			}
			void Workspace::LoadFileFromDisk(String^ Path)
			{
				TextEditor->LoadFileFromDisk(Path, AllocatedIndex);
			}
			void Workspace::SaveScriptToDisk(String^ Path, bool PathIncludesFileName)
			{
				TextEditor->SaveScriptToDisk(Path, PathIncludesFileName, EditorTab->Tooltip, AllocatedIndex);
			}
			bool Workspace::ValidateScript(String^% PreprocessedScriptText)
			{
				StringReader^ ValidateParser = gcnew StringReader(PreprocessedScriptText);
				String^ ReadLine = ValidateParser->ReadLine();
				ScriptParser^ ScriptTextParser = gcnew ScriptParser();
				UInt32 ScriptType = (UInt32)GetScriptType();
				int CurrentLineNo = 0, PreviousLineNo = 0;
				String^ ScriptName = "";

				ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Invalid);
				ClearErrorsItemsFromMessagePool();

				bool Result = true;

				while (ReadLine != nullptr)
				{
					ScriptTextParser->Tokenize(ReadLine, false);

					if (!ScriptTextParser->Valid || ScriptTextParser->Tokens[0][0] == ';')
					{
						CurrentLineNo++;
						ReadLine = ValidateParser->ReadLine();
						continue;
					}

					CurrentLineNo++;

					String^ FirstToken = ScriptTextParser->Tokens[0];
					String^ SecondToken = (ScriptTextParser->Tokens->Count > 1)?ScriptTextParser->Tokens[1]:"";
					ScriptParser::TokenType TokenType = ScriptTextParser->GetTokenType(FirstToken);

					switch (TokenType)
					{
					case ScriptParser::TokenType::e_ScriptName:
						if (ScriptTextParser->GetContainsIllegalChar(SecondToken, "_", ""))
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Identifier '" + SecondToken + "' contains an invalid character."), Result = false;
						if (ScriptName == "")
							ScriptName = SecondToken;
						else
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Redeclaration of script name."), Result = false;
						break;
					case ScriptParser::TokenType::e_Variable:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_Invalid)
						{
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Variable '" + SecondToken + "' declared inside a script block.");
							Result = false;
						}
						if (ScriptTextParser->LookupVariableByName(SecondToken) != nullptr)
							AddMessageToMessagePool(MessageType::e_Warning, CurrentLineNo, "Redeclaration of variable '" + SecondToken + "'."), Result = false;
						else
							ScriptTextParser->Variables->AddLast(gcnew ScriptParser::VariableRefCountData(SecondToken, 0));
						break;
					case ScriptParser::TokenType::e_Begin:
						if (!ScriptTextParser->GetIsBlockValidForScriptType(SecondToken, (ScriptParser::ScriptType)ScriptType))
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid script block '" + SecondToken + "' for script type."), Result = false;
						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_ScriptBlock);
						break;
					case ScriptParser::TokenType::e_End:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_ScriptBlock)
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid block structure. Command 'End' has no matching 'Begin'."), Result = false;
						else
							ScriptTextParser->BlockStack->Pop();
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToMessagePool(MessageType::e_Warning, CurrentLineNo, "Command 'End' has an otiose expression following it.");
						break;
					case ScriptParser::TokenType::e_While:
						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
						break;
					case ScriptParser::TokenType::e_ForEach:
						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
						break;
					case ScriptParser::TokenType::e_Loop:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_Loop)
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid block structure. Command 'Loop' has no matching 'While' or 'ForEach'."), Result = false;
						else
							ScriptTextParser->BlockStack->Pop();
						break;
					case ScriptParser::TokenType::e_If:
						if (ScriptTextParser->GetCurrentTokenCount() < 2 || ScriptTextParser->Tokens[1][0] == ';')
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid condition."), Result = false;

						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_If);
						break;
					case ScriptParser::TokenType::e_ElseIf:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid block structure. Command 'ElseIf' has no matching 'If'."), Result = false;
						else if (ScriptTextParser->GetCurrentTokenCount() < 2 || ScriptTextParser->Tokens[1][0] == ';')
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid condition."), Result = false;
						break;
					case ScriptParser::TokenType::e_Else:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid block structure. Command 'Else' has no matching 'If'."), Result = false;
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToMessagePool(MessageType::e_Warning, CurrentLineNo, "Command 'Else' has an otiose expression following it.");
						break;
					case ScriptParser::TokenType::e_EndIf:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
							AddMessageToMessagePool(MessageType::e_Error, CurrentLineNo, "Invalid block structure. Command 'EndIf' has no matching 'If'."), Result = false;
						else
							ScriptTextParser->BlockStack->Pop();
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToMessagePool(MessageType::e_Warning, CurrentLineNo, "Command 'EndIf' has an otiose expression following it.");
						break;
					case ScriptParser::TokenType::e_Return:
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToMessagePool(MessageType::e_Warning, CurrentLineNo, "Command 'Return' has an otiose expression following it.");
						break;
					}

																							// increment variable ref count
					UInt32 Pos = 0;
					if (ScriptTextParser->GetTokenType(FirstToken) != ScriptParser::TokenType::e_Variable)
					{
						for each (String^% Itr in ScriptTextParser->Tokens)
						{
							if (ScriptTextParser->LookupVariableByName(Itr) != nullptr)
							{
								if (Pos == 0 || ScriptTextParser->Delimiters[Pos - 1] != '.')
								{
									if (ScriptTextParser->GetCommentTokenIndex(Pos) == -1)
										ScriptTextParser->LookupVariableByName(Itr)->RefCount++;
								}
							}
							Pos++;
						}
					}

					ReadLine = ValidateParser->ReadLine();
				}

				for each (ScriptParser::VariableRefCountData^% Itr in ScriptTextParser->Variables)
				{
					if (Itr->RefCount == 0)
					{
						if ((ScriptParser::ScriptType)ScriptType != ScriptParser::ScriptType::e_Quest || OPTIONS->FetchSettingAsInt("SuppressRefCountForQuestScripts") == 0)
							AddMessageToMessagePool(MessageType::e_Warning, 1, "Variable '" + Itr->Name + "' unreferenced in local context.");
					}

					bool InvalidVarName = false;
					try
					{
						UInt32 Temp = int::Parse(Itr->Name);
						InvalidVarName = true;
					}
					catch (...) { }

					if (InvalidVarName)
					{
						AddMessageToMessagePool(MessageType::e_Error, 1, "Variable '" + Itr->Name + "' has an all-numeric identifier.");
						Result = false;
					}
				}

				if (!Result)
					AddMessageToMessagePool(MessageType::e_Warning, -1, "Compilation of script '" + ScriptName + "' halted - Couldn't recover from previous errors.");

				ComponentDLLInterface::FormData* Data = g_CSEInterface->CSEEditorAPI.LookupFormByEditorID((CString(ScriptName)).c_str());
				if (Data && String::Compare(CurrentScriptEditorID, ScriptName, true) != 0)
				{
					if (MessageBox::Show("Script name '" + ScriptName + "' is already used by another form. Proceeding with compilation will modify the script's editorID.\n\nDo you want to proceed?",
						"CSE Script Editor",
						MessageBoxButtons::YesNo,
						MessageBoxIcon::Exclamation) == DialogResult::No)
					{
						Result = false;
					}
				}
				g_CSEInterface->DeleteNativeHeapPointer(Data, false);

				if (MessageList->Items->Count && MessageList->Visible == false)
					ToolBarMessageList->PerformClick();

				return Result;
			}
			void Workspace::Destroy()
			{
				DestructionFlag = true;

				delete TextEditor;
				delete OffsetViewer;
				delete PreprocessedTextViewer;
				delete ScriptListBox;

				delete WorkspaceMainToolBar;
				delete WorkspaceSplitter;

				delete WorkspaceSecondaryToolBar;
				delete MessageList;
				delete FindList;
				delete BookmarkList;
				delete VariableIndexList;
				delete SpoilerText;
				delete TextEditorContextMenu;
				delete VariableIndexEditBox;

				ParentContainer->RemoveTab(EditorTab);
				ParentContainer->RemoveTabControlBox(EditorControlBox);

				delete EditorTab;
				delete EditorControlBox;

				ParentContainer->Redraw();
			}
			Workspace::ScriptType Workspace::GetScriptType()
			{
				return CurrentScriptType;
			}
			bool Workspace::PreprocessScriptText(String^% PreprocessorResult)
			{
				bool Result = Preprocessor::GetSingleton()->PreprocessScript(TextEditor->GetText(),
														PreprocessorResult,
														gcnew ScriptPreprocessor::StandardOutputError(this, &ScriptEditor::Workspace::PreprocessorErrorOutputWrapper),
														gcnew ScriptEditorPreprocessorData(Globals::AppPath,
														OPTIONS->FetchSettingAsInt("AllowRedefinitions"),
														OPTIONS->FetchSettingAsInt("NoOfPasses")));
				return Result;
			}
			void Workspace::AddMessageToMessagePool(MessageType Type, int Line, String^ Message)
			{
				ListViewItem^ Item = gcnew ListViewItem(" ", (int)Type);
				if (Line != -1)
					Item->SubItems->Add(Line.ToString());
				else
					Item->SubItems->Add("");
				Item->SubItems->Add(Message);
				if (Type == MessageType::e_Message)
					Item->ToolTipText = "Double click to remove message";

				MessageList->Items->Add(Item);
				if (MessageList->Visible == false)
					ToolBarMessageList->PerformClick();

				if (Type == MessageType::e_Error)
					TextEditor->HighlightScriptError(Line);
			}
			void Workspace::ClearCSEMessagesFromMessagePool(void)
			{
				LinkedList<ListViewItem^>^ InvalidItems = gcnew LinkedList<ListViewItem^>();

				for each (ListViewItem^ Itr in MessageList->Items)
				{
					if (Itr->ImageIndex == (int)MessageType::e_CSEMessage)
						InvalidItems->AddLast(Itr);
				}

				for each (ListViewItem^ Itr in InvalidItems)
					MessageList->Items->Remove(Itr);
			}
			void Workspace::Relocate(TabContainer^ Destination)
			{
				EditorControlBox->TabItem = nullptr;		// reset to prevent disposal
				EditorTab->AttachedControl = nullptr;

				ParentContainer->FlagDestruction(true);
				ParentContainer->RemoveTabControlBox(EditorControlBox);
				ParentContainer->FlagDestruction(false);

				ParentContainer = Destination;
				EditorControlBox->TabItem = EditorTab;
				EditorTab->AttachedControl = EditorControlBox;

				Destination->AddTab(EditorTab);
				Destination->AddTabControlBox(EditorControlBox);
			}
			void Workspace::HandleFocus( bool GotFocus )
			{
				if (GotFocus)
					TextEditor->OnGotFocus();
				else
					TextEditor->OnLostFocus();
			}
			bool Workspace::PerformScriptHouseKeeping()
			{
				if (GetModifiedStatus())
				{
					DialogResult Result = MessageBox::Show("The current script '" + CurrentScriptEditorID + "' has unsaved changes. Do you wish to save them ?",
															"CSE Script Editor",
															MessageBoxButtons::YesNoCancel, MessageBoxIcon::Exclamation);
					if (Result == DialogResult::Yes)
						return SaveScript(SaveScriptOperation::e_SaveAndCompile);
					else if (Result == DialogResult::No)
					{
						if (NewScriptFlag)
						{
							g_CSEInterface->ScriptEditor.DestroyScriptInstance(CurrentScript);
							CurrentScript = 0;
						}

						return true;
					}
					else
						return false;
				}
				return true;
			}
			void Workspace::NewScript()
			{
				if (PerformScriptHouseKeeping())
				{
					ComponentDLLInterface::ScriptData* Data = g_CSEInterface->ScriptEditor.CreateNewScript();
					UpdateEnvironment(Data, true);
					g_CSEInterface->DeleteNativeHeapPointer(Data, false);

					NewScriptFlag = true;
					SetModifiedStatus(true);
				}
			}
			void Workspace::OpenScript()
			{
				ComponentDLLInterface::ScriptData* Data = ScriptListBox->Show(ScriptListDialog::Operation::e_Open, CurrentScriptEditorID);
				if (Data && PerformScriptHouseKeeping())
				{
					UpdateEnvironment(Data, true);
				}
				g_CSEInterface->DeleteNativeHeapPointer(Data, false);
			}
			bool Workspace::SaveScript(SaveScriptOperation Operation)
			{
				bool Result = false;
				String^ PreprocessedScriptResult = "";

				if (PreprocessScriptText(PreprocessedScriptResult) && ValidateScript(PreprocessedScriptResult))
				{
					if (CurrentScript)
					{
						if (Operation == SaveScriptOperation::e_SaveButDontCompile)
						{
							g_CSEInterface->ScriptEditor.ToggleScriptCompilation(false);
							AddMessageToMessagePool(MessageType::e_CSEMessage, -1, "This is an uncompiled script. Expect weird behavior during runtime execution");
						}

						ComponentDLLInterface::ScriptCompileData CompileData;

						CString ScriptText(PreprocessedScriptResult->Replace("\n", "\r\n"));
						CompileData.Script.Text = ScriptText.c_str();
						CompileData.Script.Type = (int)GetScriptType();
						CompileData.Script.ParentForm = (TESForm*)CurrentScript;

						if (g_CSEInterface->ScriptEditor.CompileScript(&CompileData))
						{
							UpdateEnvironment(&CompileData.Script, false);

							String^ OriginalText = GetScriptText() + SerializeCSEBlock();
							CString OrgScriptText(OriginalText);
							g_CSEInterface->ScriptEditor.SetScriptText(CurrentScript, OrgScriptText.c_str());
							Result = true;
						}
						else
						{
							for (int i = 0; i < CompileData.CompileErrorData.Count; i++)
							{
								AddMessageToMessagePool(MessageType::e_Error,
														CompileData.CompileErrorData.ErrorListHead[i].Line,
														gcnew String(CompileData.CompileErrorData.ErrorListHead[i].Message));
							}

							g_CSEInterface->DeleteNativeHeapPointer(CompileData.CompileErrorData.ErrorListHead, true);
						}

						if (Operation == SaveScriptOperation::e_SaveButDontCompile)
							g_CSEInterface->ScriptEditor.ToggleScriptCompilation(true);
						else if (Operation == SaveScriptOperation::e_SaveActivePluginToo)
							g_CSEInterface->CSEEditorAPI.SaveActivePlugin();
					}
					else
						Result = true;
				}

				return Result;
			}
			void Workspace::DeleteScript()
			{
				if (PerformScriptHouseKeeping())
				{
					ComponentDLLInterface::ScriptData* Data = ScriptListBox->Show(ScriptListDialog::Operation::e_Delete, CurrentScriptEditorID);
					if (Data)
					{
						g_CSEInterface->ScriptEditor.DeleteScript(Data->EditorID);
					}
					g_CSEInterface->DeleteNativeHeapPointer(Data, false);
				}
			}
			void Workspace::RecompileScripts()
			{
				if (MessageBox::Show("Are you sure you want to recompile all the scripts in the active plugin?",
															"CSE Script Editor",
															MessageBoxButtons::YesNo, MessageBoxIcon::Exclamation) == DialogResult::Yes)
				{
					g_CSEInterface->ScriptEditor.RecompileScripts();
					MessageBox::Show("All active scripts recompiled. Results have been logged to the console.",
															"CSE Script Editor",
															MessageBoxButtons::OK, MessageBoxIcon::Information);
				}
			}
			void Workspace::PreviousScript()
			{
				if (PerformScriptHouseKeeping())
				{
					ComponentDLLInterface::ScriptData* Data = g_CSEInterface->ScriptEditor.GetPreviousScriptInList(CurrentScript);
					if (Data)
					{
						UpdateEnvironment(Data, true);
					}
					g_CSEInterface->DeleteNativeHeapPointer(Data, false);
				}
			}
			void Workspace::NextScript()
			{
				if (PerformScriptHouseKeeping())
				{
					ComponentDLLInterface::ScriptData* Data = g_CSEInterface->ScriptEditor.GetNextScriptInList(CurrentScript);
					if (Data)
					{
						UpdateEnvironment(Data, true);
					}
					g_CSEInterface->DeleteNativeHeapPointer(Data, false);
				}
			}
			void Workspace::CloseScript()
			{
				if (PerformScriptHouseKeeping())
				{
					Rectangle Bounds = GetParentContainer()->GetEditorFormRect();
					g_CSEInterface->ScriptEditor.SaveEditorBoundsToINI(Bounds.Left, Bounds.Top, Bounds.Width, Bounds.Height);

					ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
					Parameters->ParameterList->Add(this);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_ReleaseWorkspace, Parameters);
				}
			}
		#pragma endregion

		#pragma region Event Handlers
			void Workspace::TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E)
			{
				HandlingKeyDownEvent = true;

				switch (E->KeyCode)								// ### TabContainer::EditorForm_KeyDown refers to some keys directly
				{
				case Keys::T:
					if (E->Modifiers == Keys::Control)
						GetParentContainer()->InstantiateNewWorkspace(0);
				case Keys::Tab:
					if (E->Control == true && E->Shift == false)
						GetParentContainer()->SelectNextTab();
					else if (E->Control == true && E->Shift == true)
						GetParentContainer()->SelectPreviousTab();
					break;
				case Keys::O:									// Open script
					if (E->Modifiers == Keys::Control)
						ToolBarOpenScript->PerformClick();
					break;
				case Keys::S:									// Save script
					if (E->Modifiers == Keys::Control)
						SaveScript(SaveScriptOperation::e_SaveAndCompile);
					break;
				case Keys::D:									// Delete script
					if (E->Modifiers == Keys::Control)
						ToolBarDeleteScript->PerformClick();
					break;
				case Keys::Left:								// Previous script
					if (E->Control && E->Alt)
					{
						ToolBarPreviousScript->PerformClick();
						TextEditor->FocusTextArea();
					}
					break;
				case Keys::Right:								// Next script
					if (E->Control && E->Alt)
					{
						ToolBarNextScript->PerformClick();
						TextEditor->FocusTextArea();
					}
					break;
				case Keys::N:									// New script
					if (E->Modifiers == Keys::Control)
						ToolBarNewScript->PerformClick();
					break;
				case Keys::B:									// Toggle bookmark
					if (E->Modifiers == Keys::Control)
						ContextMenuToggleBookmark->PerformClick();
					break;
				case Keys::F:									// Find
					if (E->Modifiers == Keys::Control)
					{
						ToolBarCommonTextBox->Tag = "Find";
						ToolBarCommonTextBox->Focus();
					}
					break;
				case Keys::H:									// Replace
					if (E->Modifiers == Keys::Control)
					{
						ToolBarCommonTextBox->Tag = "Replace";
						ToolBarCommonTextBox->Focus();
					}
					break;
				case Keys::G:									// Goto Line
					if (E->Modifiers == Keys::Control)
					{
						ToolBarCommonTextBox->Tag = "Goto Line";
						ToolBarCommonTextBox->Focus();
					}
					break;
				case Keys::E:									// Goto Offset
					if (E->Modifiers == Keys::Control)
					{
						ToolBarCommonTextBox->Tag = "Goto Offset";
						ToolBarCommonTextBox->Focus();
					}
					break;
				}

				HandlingKeyDownEvent = false;
			}
			void Workspace::TextEditor_ScriptModified(Object^ Sender, ScriptModifiedEventArgs^ E)
			{
				EditorTab->ImageIndex = (int)E->ModifiedStatus;
			}

			void Workspace::MessageList_DoubleClick(Object^ Sender, EventArgs^ E)
			{
				if (GetListViewSelectedItem(MessageList) != nullptr)
				{
					if (GetListViewSelectedItem(MessageList)->ImageIndex == (int)MessageType::e_Message)
						MessageList->Items->Remove(GetListViewSelectedItem(MessageList));
					else
						TextEditor->ScrollToLine(GetListViewSelectedItem(MessageList)->SubItems[1]->Text);
				}
			}
			void Workspace::MessageList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
			{
				if (E->Column != (int)MessageList->Tag)
				{
					MessageList->Tag = E->Column;
					MessageList->Sorting = SortOrder::Ascending;
				}
				else
				{
					if (MessageList->Sorting == SortOrder::Ascending)
						MessageList->Sorting = SortOrder::Descending;
					else
						MessageList->Sorting = SortOrder::Ascending;
				}

				MessageList->Sort();
				System::Collections::IComparer^ Sorter;
				switch (E->Column)
				{
				case 0:
					Sorter = gcnew CSEListViewImgSorter(E->Column, MessageList->Sorting);
					break;
				case 1:
					Sorter = gcnew CSEListViewIntSorter(E->Column, MessageList->Sorting, false);
					break;
				default:
					Sorter = gcnew CSEListViewStringSorter(E->Column, MessageList->Sorting);
					break;
				}
				MessageList->ListViewItemSorter = Sorter;
			}
			void Workspace::FindList_DoubleClick(Object^ Sender, EventArgs^ E)
			{
				if (GetListViewSelectedItem(FindList) != nullptr)
				{
					TextEditor->ScrollToLine(GetListViewSelectedItem(FindList)->SubItems[0]->Text);
				}
			}
			void Workspace::FindList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
			{
				if (E->Column != (int)FindList->Tag)
				{
					FindList->Tag = E->Column;
					FindList->Sorting = SortOrder::Ascending;
				}
				else
				{
					if (FindList->Sorting == SortOrder::Ascending)
						FindList->Sorting = SortOrder::Descending;
					else
						FindList->Sorting = SortOrder::Ascending;
				}

				FindList->Sort();
				System::Collections::IComparer^ Sorter;
				switch (E->Column)
				{
				case 0:
					Sorter = gcnew CSEListViewIntSorter(E->Column, FindList->Sorting, false);
					break;
				default:
					Sorter = gcnew CSEListViewStringSorter(E->Column, FindList->Sorting);
					break;
				}
				FindList->ListViewItemSorter = Sorter;
			}
			void Workspace::BookmarkList_DoubleClick(Object^ Sender, EventArgs^ E)
			{
				if (GetListViewSelectedItem(BookmarkList) != nullptr)
				{
					TextEditor->ScrollToLine(GetListViewSelectedItem(BookmarkList)->SubItems[0]->Text);
				}
			}
			void Workspace::BookmarkList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
			{
				if (E->Column != (int)BookmarkList->Tag)
				{
					BookmarkList->Tag = E->Column;
					BookmarkList->Sorting = SortOrder::Ascending;
				}
				else
				{
					if (BookmarkList->Sorting == SortOrder::Ascending)
						BookmarkList->Sorting = SortOrder::Descending;
					else
						BookmarkList->Sorting = SortOrder::Ascending;
				}

				BookmarkList->Sort();
				System::Collections::IComparer^ Sorter;
				switch (E->Column)
				{
				case 0:
					Sorter = gcnew CSEListViewIntSorter(E->Column, BookmarkList->Sorting, false);
					break;
				default:
					Sorter = gcnew CSEListViewStringSorter(E->Column, BookmarkList->Sorting);
					break;
				}
				BookmarkList->ListViewItemSorter = Sorter;
			}
			void Workspace::VariableIndexList_DoubleClick(Object^ Sender, EventArgs^ E)
			{
				if (GetListViewSelectedItem(VariableIndexList) != nullptr)
				{
					ListViewItem^ Item = GetListViewSelectedItem(VariableIndexList);
					Rectangle Bounds = Item->SubItems[2]->Bounds;
					if (Bounds.Width > 35)
					{
						VariableIndexEditBox->SetBounds(Bounds.X, Bounds.Y, Bounds.Width, Bounds.Height, BoundsSpecified::All);
						VariableIndexEditBox->Show();
						VariableIndexEditBox->BringToFront();
						VariableIndexEditBox->Focus();
					}
					else
					{
						MessageBox::Show("Please expand the Index column sufficiently to allow the editing of its contents", "CSE Script Editor",
										MessageBoxButtons::OK, MessageBoxIcon::Information);
					}
				}
			}
			void Workspace::VariableIndexList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
			{
				if (E->Column != (int)VariableIndexList->Tag)
				{
					VariableIndexList->Tag = E->Column;
					VariableIndexList->Sorting = SortOrder::Ascending;
				}
				else
				{
					if (VariableIndexList->Sorting == SortOrder::Ascending)
						VariableIndexList->Sorting = SortOrder::Descending;
					else
						VariableIndexList->Sorting = SortOrder::Ascending;
				}

				VariableIndexList->Sort();
				System::Collections::IComparer^ Sorter;
				switch (E->Column)
				{
				case 2:
					Sorter = gcnew CSEListViewIntSorter(E->Column, VariableIndexList->Sorting, false);
					break;
				default:
					Sorter = gcnew CSEListViewStringSorter(E->Column, VariableIndexList->Sorting);
					break;
				}
				VariableIndexList->ListViewItemSorter = Sorter;
			}
			void Workspace::VariableIndexEditBox_LostFocus(Object^ Sender, EventArgs^ E)
			{
				VariableIndexEditBox->Hide();

				UInt32 Index = 0;
				try
				{
					Index = UInt32::Parse(VariableIndexEditBox->Text);
				}
				catch (...)
				{
					VariableIndexEditBox->Text = "";
					return;
				}

				VariableIndexEditBox->Text = "";
				if (GetListViewSelectedItem(VariableIndexList) != nullptr)
				{
					ListViewItem^ Item = GetListViewSelectedItem(VariableIndexList);
					Item->SubItems[2]->Text = Index.ToString();
					Item->Tag = (int)1;		// flag modification
					DebugPrint("Set the index of variable '" + Item->Text + "' in script '" + CurrentScriptEditorID + "' to " + Index.ToString());
				}
			}
			void Workspace::VariableIndexEditBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
			{
				if (E->KeyCode == Keys::Enter)
					Workspace::VariableIndexEditBox_LostFocus(nullptr, nullptr);
			}

			void Workspace::ToolBarNewScript_Click(Object^ Sender, EventArgs^ E)
			{
				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->EditorHandleIndex = AllocatedIndex;

				if (HandlingKeyDownEvent)
				{
					NewScript();
				}
				else
				{
					switch (Control::ModifierKeys)
					{
					case Keys::Control:
						ParentContainer->PerformRemoteTabOperation(TabContainer::RemoteOperation::e_New, nullptr);
						break;
					case Keys::Shift:
						Parameters->EditorHandleIndex = 0;
						Parameters->ParameterList->Add((UInt32)0);
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().X);
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().Y);
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().Width);
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().Height);

						SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateTabContainer, Parameters);
						break;
					default:
						NewScript();
						break;
					}
				}
			}
			void Workspace::ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E)
			{
				if (Control::ModifierKeys == Keys::Control && !HandlingKeyDownEvent)
					ParentContainer->PerformRemoteTabOperation(TabContainer::RemoteOperation::e_Open, nullptr);
				else
					OpenScript();
			}
			void Workspace::ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E)
			{
				PreviousScript();
			}
			void Workspace::ToolBarNextScript_Click(Object^ Sender, EventArgs^ E)
			{
				NextScript();
			}
			void Workspace::ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E)
			{
				SaveScript(SaveScriptOperation::e_SaveAndCompile);
			}
			void Workspace::ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E)
			{
				if (GetIsCurrentScriptNew() || GetIsFirstRun())
				{
					MessageBox::Show("You may only perform this operation on an existing script.", "CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Information);
					return;
				}

				SaveScript(SaveScriptOperation::e_SaveButDontCompile);
			}
			void Workspace::ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E)
			{
				SaveScript(SaveScriptOperation::e_SaveActivePluginToo);
			}
			void Workspace::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
			{
				RecompileScripts();
			}
			void Workspace::ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E)
			{
				if (!GetIsCurrentScriptNew() && !GetIsFirstRun())
				{
					CString CEID(CurrentScriptEditorID);
					g_CSEInterface->ScriptEditor.CompileDependencies(CEID.c_str());
					MessageBox::Show("Operation complete! Script variables used as condition parameters will need to be corrected manually. The results have been logged to the console.",
									"CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Information);
				}
				else
				{
					MessageBox::Show("The current script needs to be compiled before its dependencies can be updated.",
									"CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				}
			}
			void Workspace::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
			{
				DeleteScript();
			}
			void Workspace::ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E)
			{
				ParentContainer->NavigateJumpStack(AllocatedIndex, TabContainer::NavigationDirection::e_Back);
			}

			void Workspace::ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E)
			{
				ParentContainer->NavigateJumpStack(AllocatedIndex, TabContainer::NavigationDirection::e_Forward);
			}

			void Workspace::ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E)
			{
				ParentContainer->SaveAllOpenWorkspaces();
			}
			void Workspace::ToolBarOptions_Click(Object^ Sender, EventArgs^ E)
			{
				OPTIONS->LoadINI();
				OPTIONS->Show();
			}

			void Workspace::ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E)
			{
				SetScriptType(ScriptType::e_Object);
				if (!GetIsFirstRun())			SetModifiedStatus(true);
			}
			void Workspace::ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E)
			{
				SetScriptType(ScriptType::e_Quest);
				if (!GetIsFirstRun())			SetModifiedStatus(true);
			}
			void Workspace::ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E)
			{
				SetScriptType(ScriptType::e_MagicEffect);
				if (!GetIsFirstRun())			SetModifiedStatus(true);
			}

			void Workspace::TextEditorContextMenu_Opening(Object^ Sender, CancelEventArgs^ E)
			{
				array<String^>^ Tokens = TextEditor->GetTokensAtMouseLocation();
				String^ MidToken = Tokens[1]->Replace("\n", "")->Replace("\t", " ")->Replace("\r", "");

				if (MidToken->Length > 20)
					ContextMenuWord->Text = MidToken->Substring(0, 17) + gcnew String("...");
				else
					ContextMenuWord->Text = MidToken;

				ContextMenuDirectLink->Tag = nullptr;
				if (ISDB->GetIsIdentifierScriptCommand(MidToken))
					ContextMenuDirectLink->Tag = ISDB->LookupDeveloperURLByCommand(MidToken);

				if (ContextMenuDirectLink->Tag == nullptr)
					ContextMenuDirectLink->Visible = false;
				else
					ContextMenuDirectLink->Visible = true;

				ContextMenuJumpToScript->Visible = true;

				CString CTUM(MidToken);
				ComponentDLLInterface::ScriptData* Data = g_CSEInterface->CSEEditorAPI.LookupScriptableFormByEditorID(CTUM.c_str());
				if (Data && Data->IsValid())
				{
					switch (Data->Type)
					{
					case 0:
						ContextMenuJumpToScript->Text = "Jump to Object script";
						break;
					case 1:
						ContextMenuJumpToScript->Text = "Jump to Quest script";
						break;
					}

					ContextMenuJumpToScript->Tag = gcnew String(Data->EditorID);
				}
				else if (ISDB->GetIsIdentifierUserFunction(MidToken))
				{
					ContextMenuJumpToScript->Text = "Jump to Function script";
					ContextMenuJumpToScript->Tag = MidToken;
				}
				else
					ContextMenuJumpToScript->Visible = false;

				g_CSEInterface->DeleteNativeHeapPointer(Data, false);

				ContextMenuRefactorCreateUDFImplementation->Visible = false;
				if (!String::Compare(Tokens[0], "call", true) &&
					!TextEditor->GetCharIndexInsideCommentSegment(TextEditor->GetCharIndexFromPosition(TextEditor->GetLastKnownMouseClickLocation())))
				{
					if (!ISDB->GetIsIdentifierUserFunction(MidToken))
					{
						ContextMenuRefactorCreateUDFImplementation->Visible = true;
						ContextMenuRefactorCreateUDFImplementation->Tag = MidToken;
					}
				}
			}
			void Workspace::ContextMenuCopy_Click(Object^ Sender, EventArgs^ E)
			{
				try
				{
					Clipboard::Clear();

					String^ CopiedText = TextEditor->GetSelectedText();
					if (CopiedText == "")
						CopiedText = TextEditor->GetTokenAtMouseLocation();

					if (CopiedText != "")
						Clipboard::SetText(CopiedText->Replace("\n", "\r\n"));
				}
				catch (Exception^ E)
				{
					DebugPrint("Exception raised while accessing the clipboard.\n\tException: " + E->Message, true);
				}
			}
			void Workspace::ContextMenuPaste_Click(Object^ Sender, EventArgs^ E)
			{
				try
				{
					if (Clipboard::GetText() != "")
						TextEditor->SetSelectedText(Clipboard::GetText(), false);
				}
				catch (Exception^ E)
				{
					DebugPrint("Exception raised while accessing the clipboard.\n\tException: " + E->Message, true);
				}
			}
			void Workspace::ContextMenuFind_Click(Object^ Sender, EventArgs^ E)
			{
				if (TextEditor->GetSelectedText() != "")
					ToolBarCommonTextBox->Text = TextEditor->GetSelectedText();
				else
					ToolBarCommonTextBox->Text = TextEditor->GetTokenAtMouseLocation();

				ToolBarEditMenuContentsFind->PerformClick();
			}
			void Workspace::ContextMenuToggleComment_Click(Object^ Sender, EventArgs^ E)
			{
				TextEditor->ToggleComment(TextEditor->GetCharIndexFromPosition(TextEditor->GetLastKnownMouseClickLocation()));
			}
			void Workspace::ContextMenuToggleBookmark_Click(Object^ Sender, EventArgs^ E)
			{
				ToggleBookmark(TextEditor->GetCharIndexFromPosition(TextEditor->GetLastKnownMouseClickLocation()));
			}
			void Workspace::ContextMenuAddMessage_Click(Object^ Sender, EventArgs^ E)
			{
				String^ Message = "";

				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter The Message String", "Add Message - CSE Script Editor");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;
				else
					Message = Result->Text;

				AddMessageToMessagePool(MessageType::e_Message, -1, Message);
			}
			void Workspace::ContextMenuWikiLookup_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start("http://cs.elderscrolls.com/constwiki/index.php/Special:Search?search=" + TextEditor->GetTokenAtMouseLocation() + "&fulltext=Search");
			}
			void Workspace::ContextMenuOBSEDocLookup_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start("http://obse.silverlock.org/obse_command_doc.html#" + TextEditor->GetTokenAtMouseLocation());
			}
			void Workspace::ContextMenuCopyToCTB_Click(Object^ Sender, EventArgs^ E)
			{
				ToolBarCommonTextBox->Text = TextEditor->GetTokenAtMouseLocation();
				ToolBarCommonTextBox->Focus();
			}
			void Workspace::ContextMenuDirectLink_Click(Object^ Sender, EventArgs^ E)
			{
				try
				{
					Process::Start(dynamic_cast<String^>(ContextMenuDirectLink->Tag));
				}
				catch (Exception^ E)
				{
					DebugPrint("Exception raised while opening internet page.\n\tException: " + E->Message);
					MessageBox::Show("Couldn't open internet page. Mostly likely caused by an improperly formatted URL.", "CSE Script Editor");
				}
			}
			void Workspace::ContextMenuJumpToScript_Click(Object^ Sender, EventArgs^ E)
			{
				ParentContainer->JumpToWorkspace(AllocatedIndex, dynamic_cast<String^>(ContextMenuJumpToScript->Tag));
			}
			void Workspace::ContextMenuGoogleLookup_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start("http://www.google.com/search?hl=en&source=hp&q=" + TextEditor->GetTokenAtMouseLocation());
			}
			void Workspace::ContextMenuRefactorAddVariable_Click( Object^ Sender, EventArgs^ E )
			{
				ToolStripMenuItem^ MenuItem = dynamic_cast<ToolStripMenuItem^>(Sender);
				ScriptParser::VariableType VarType = (ScriptParser::VariableType)MenuItem->Tag;
				String^ VarName = "";
				String^ ScriptText = TextEditor->GetText()->Replace("\r", "");

				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter Variable Name", "Add Variable - CSE Script Editor");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;
				else
					VarName = Result->Text;

				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ TextReader = gcnew StringReader(ScriptText);
				int LastVarOffset = 0, InsertOffset = 0;

				for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
				{
					TextParser->Tokenize(ReadLine, false);
					InsertOffset += ReadLine->Length + 1;

					if (!TextParser->Valid)
					{
						continue;
					}

					bool ExitLoop = false;
					switch (TextParser->GetTokenType(TextParser->Tokens[0]))
					{
					case ScriptParser::TokenType::e_Variable:
						LastVarOffset = InsertOffset;
						break;
					case ScriptParser::TokenType::e_Comment:
					case ScriptParser::TokenType::e_ScriptName:
						break;
					default:
						ExitLoop = true;
						break;
					}

					if (ExitLoop)
						break;
				}

				if (LastVarOffset)
					InsertOffset = LastVarOffset;

				String^ VarText = "";
				if (InsertOffset > ScriptText->Length)
					VarText += "\n";

				switch (VarType)
				{
				case ScriptParser::VariableType::e_Integer:
					VarText += "int " + VarName;
					break;
				case ScriptParser::VariableType::e_Float:
					VarText += "float " + VarName;
					break;
				case ScriptParser::VariableType::e_Ref:
					VarText += "ref " + VarName;
					break;
				case ScriptParser::VariableType::e_String:
					VarText += "string_var " + VarName;
					break;
				case ScriptParser::VariableType::e_Array:
					VarText += "array_var " + VarName;
					break;
				}
				VarText += "\n";

				TextEditor->InsertText(VarText, InsertOffset);
			}
			void Workspace::ContextMenuRefactorDocumentScript_Click( Object^ Sender, EventArgs^ E )
			{
				if (GetModifiedStatus())
				{
					MessageBox::Show("The current script needs to be compiled before it can be documented.",
									"CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
					return;
				}

				Refactoring::EditScriptComponentDialog DocumentScriptData(GetParentContainer()->GetEditorFormHandle(),
																			CurrentScriptEditorID,
																			Refactoring::EditScriptComponentDialog::OperationType::e_DocumentScript,
																			"Script Description");

				if (DocumentScriptData.HasResult)
				{
					ScriptParser^ ScriptTextParser = gcnew ScriptParser();
					StringReader^ TextParser = gcnew StringReader(GetScriptText());

					String^ DocumentedScript = "";
					String^ Description = "";
					String^ FixedDescription = "";
					String^ ScriptName = "";
					DocumentScriptData.ResultData->LookupEditDataByName("Script Description", Description);
					StringReader^ DescriptionParser = gcnew StringReader(Description);

					for (String^ ReadLine = DescriptionParser->ReadLine(); ReadLine != nullptr; ReadLine = DescriptionParser->ReadLine())
					{
						if (ReadLine != "")
							FixedDescription += "; " + ReadLine + "\n";
						else
							FixedDescription += "\n";
					}

					bool SkippedDescription = false;
					bool DoneDocumenting = false;

					for (String^ ReadLine = TextParser->ReadLine(); ReadLine != nullptr; ReadLine = TextParser->ReadLine())
					{
						ScriptTextParser->Tokenize(ReadLine, false);

						if (DoneDocumenting || !ScriptTextParser->Valid)
						{
							DocumentedScript += ReadLine + "\n";
							continue;
						}

						String^ FirstToken = ScriptTextParser->Tokens[0];
						String^ SecondToken = (ScriptTextParser->Tokens->Count > 1)?ScriptTextParser->Tokens[1]:"";
						String^ EditData = "";

						ScriptParser::TokenType Type = ScriptTextParser->GetTokenType(FirstToken);

						switch (Type)
						{
						case ScriptParser::TokenType::e_Variable:
							if (!SkippedDescription)
							{
								SkippedDescription = true;
								DocumentedScript = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";
							}

							if (DocumentScriptData.ResultData->LookupEditDataByName(SecondToken, EditData) && EditData != "")
							{
								DocumentedScript += FirstToken + " " + SecondToken + "\t ; " + EditData->Replace("\n", "")->Replace("\r", "") + "\n";
								continue;
							}
							else
								DocumentedScript += ReadLine + "\n";
							break;
						case ScriptParser::TokenType::e_ScriptName:
							ScriptName = SecondToken;
							break;
						case ScriptParser::TokenType::e_Comment:
							if (SkippedDescription)
								DocumentedScript += ReadLine + "\n";
							break;
						case ScriptParser::TokenType::e_Begin:
							if (!SkippedDescription)
							{
								SkippedDescription = true;
								DocumentedScript = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";
							}

							DoneDocumenting = true;
							DocumentedScript += ReadLine + "\n";
							break;
						default:
							if (!SkippedDescription)
							{
								SkippedDescription = true;
								DocumentedScript = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";
							}

							DocumentedScript += ReadLine + "\n";
							break;
						}
					}

					if (!SkippedDescription)
						DocumentedScript = "scn " + ScriptName + "\n\n" + FixedDescription + "\n";

					TextEditor->SetSelectionStart(0);
					TextEditor->SetSelectionLength(TextEditor->GetTextLength());
					TextEditor->SetSelectedText(DocumentedScript->Substring(0, DocumentedScript->Length - 1), false);
					TextEditor->SetSelectionLength(0);
				}
			}
			void Workspace::ContextMenuRefactorCreateUDFImplementation_Click( Object^ Sender, EventArgs^ E )
			{
				String^ UDFName = dynamic_cast<String^>(ContextMenuRefactorCreateUDFImplementation->Tag);

				Refactoring::CreateUDFImplementationDialog UDFData(GetParentContainer()->GetEditorFormHandle());

				if (UDFData.HasResult)
				{
					String^ UDFScriptText = "scn " + UDFName + "\n\n";
					String^ ParamList = "{ ";
					for each (Refactoring::CreateUDFImplementationData::ParameterData^ Itr in UDFData.ResultData->ParameterList)
					{
						UDFScriptText += Itr->Type + " " + Itr->Name + "\n";
						ParamList += Itr->Name + " ";
					}
					ParamList += "}";
					if (UDFData.ResultData->ParameterList->Count)
						UDFScriptText += "\n";

					UDFScriptText += "begin function " + ParamList + "\n\n\n\nend\n";

					GetParentContainer()->PerformRemoteTabOperation(TabContainer::RemoteOperation::e_NewText, UDFScriptText);
				}
			}
			void Workspace::ContextMenuRefactorRenameVariables_Click( Object^ Sender, EventArgs^ E )
			{
				if (GetModifiedStatus())
				{
					MessageBox::Show("The current script needs to be compiled before its variables can be renamed.",
									"CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
					return;
				}

				Refactoring::EditScriptComponentDialog RenameVariablesData(GetParentContainer()->GetEditorFormHandle(),
																			CurrentScriptEditorID,
																			Refactoring::EditScriptComponentDialog::OperationType::e_RenameVariables,
																			"");

				if (RenameVariablesData.HasResult)
				{
					List<CString^>^ StringAllocations = gcnew List<CString^>();
					List<Refactoring::EditScriptComponentData::ScriptComponent^>^ RenameEntries = gcnew List<Refactoring::EditScriptComponentData::ScriptComponent^>();
					ComponentDLLInterface::ScriptVarRenameData RenameData;

					for each (Refactoring::EditScriptComponentData::ScriptComponent^ Itr in RenameVariablesData.ResultData->ScriptComponentList)
					{
						if (Itr->EditData != "")
							RenameEntries->Add(Itr);
					}

					if (RenameEntries->Count)
					{
						RenameData.ScriptVarListCount = RenameEntries->Count;
						RenameData.ScriptVarListHead = new ComponentDLLInterface::ScriptVarRenameData::ScriptVarInfo[RenameData.ScriptVarListCount];

						for (int i = 0; i < RenameData.ScriptVarListCount; i++)
						{
							ComponentDLLInterface::ScriptVarRenameData::ScriptVarInfo* Data = &RenameData.ScriptVarListHead[i];
							CString^ OldID = gcnew CString(RenameEntries[i]->ElementName);
							CString^ NewID = gcnew CString(RenameEntries[i]->EditData);

							Data->OldName = OldID->c_str();
							Data->NewName = NewID->c_str();

							StringAllocations->Add(OldID);
							StringAllocations->Add(NewID);
						}

						CString CEID(CurrentScriptEditorID);
						g_CSEInterface->ScriptEditor.UpdateScriptVarNames(CEID.c_str(), &RenameData);

						MessageBox::Show("Variables have been renamed. Scripts referencing them (current script included) will have to be manually updated with the new identifiers.",
										"CSE Script Editor",
										MessageBoxButtons::OK,
										MessageBoxIcon::Information);

						for each (CString^ Itr in StringAllocations)
							delete Itr;
					}

					StringAllocations->Clear();
					RenameEntries->Clear();
				}
			}

			void Workspace::ToolBarCommonTextBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
			{
				switch (E->KeyCode)
				{
				case Keys::Enter:
					if (ToolBarCommonTextBox->Tag->ToString() != "")
					{
						if	(ToolBarCommonTextBox->Tag->ToString() == "Find")
							ToolBarEditMenuContentsFind->PerformClick();
						else if (ToolBarCommonTextBox->Tag->ToString() == "Replace")
							ToolBarEditMenuContentsReplace->PerformClick();
						else if (ToolBarCommonTextBox->Tag->ToString() == "Goto Line")
							ToolBarEditMenuContentsGotoLine->PerformClick();
						else if (ToolBarCommonTextBox->Tag->ToString() == "Goto Offset")
							ToolBarEditMenuContentsGotoOffset->PerformClick();
					}

					E->Handled = true;
					break;
				}
			}
			void Workspace::ToolBarCommonTextBox_LostFocus(Object^ Sender, EventArgs^ E)
			{
				ToolBarCommonTextBox->Tag = "";
			}

			void Workspace::ToolBarEditMenuContentsFind_Click(Object^ Sender, EventArgs^ E)
			{
				if ( ToolBarCommonTextBox->Text != "")
					FindReplaceWrapper(IScriptTextEditor::FindReplaceOperation::e_Find);
			}
			void Workspace::ToolBarEditMenuContentsReplace_Click(Object^ Sender, EventArgs^ E)
			{
				if (ToolBarCommonTextBox->Text != "")
					FindReplaceWrapper(IScriptTextEditor::FindReplaceOperation::e_Replace);
			}
			void Workspace::ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E)
			{
				if (ToolBarCommonTextBox->Text != "")
				{
					if (ToolBarShowPreprocessedText->Checked)
						PreprocessedTextViewer->JumpToLine(ToolBarCommonTextBox->Text);
					else if (ToolBarShowOffsets->Checked)
						MessageBox::Show("This operation can only be performed in the text editor and the preprocesed text viewer", "CSE Script Editor");
					else
						TextEditor->ScrollToLine(ToolBarCommonTextBox->Text);
				}
			}
			void Workspace::ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E)
			{
				if (ToolBarCommonTextBox->Text != "")
				{
					if (ToolBarShowOffsets->Checked)
						OffsetViewer->JumpToLine(ToolBarCommonTextBox->Text);
					else
						MessageBox::Show("This operation can only be performed in the offset viewer", "CSE Script Editor");
				}
			}

			void Workspace::ToolBarMessageList_Click(Object^ Sender, EventArgs^ E)
			{
				if (FindList->Visible)
					ToolBarFindList->PerformClick();
				else if (BookmarkList->Visible)
					ToolBarBookmarkList->PerformClick();
				else if (VariableIndexList->Visible)
					ToolBarGetVarIndices->PerformClick();

				if (!MessageList->Visible)
				{
					MessageList->Show();
					MessageList->BringToFront();
					ToolBarMessageList->Checked = true;
					WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height / 2;
				}
				else
				{
					MessageList->Hide();
					ToolBarMessageList->Checked = false;
					WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height;
				}
			}
			void Workspace::ToolBarFindList_Click(Object^ Sender, EventArgs^ E)
			{
				if (MessageList->Visible)
					ToolBarMessageList->PerformClick();
				else if (BookmarkList->Visible)
					ToolBarBookmarkList->PerformClick();
				else if (VariableIndexList->Visible)
					ToolBarGetVarIndices->PerformClick();

				if (!FindList->Visible)
				{
					FindList->Show();
					FindList->BringToFront();
					ToolBarFindList->Checked = true;
					WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height / 2;
				}
				else
				{
					FindList->Hide();
					TextEditor->ClearFindResultIndicators();
					ToolBarFindList->Checked = false;
					WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height;
				}
			}
			void Workspace::ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E)
			{
				if (MessageList->Visible)
					ToolBarMessageList->PerformClick();
				else if (FindList->Visible)
					ToolBarFindList->PerformClick();
				else if (VariableIndexList->Visible)
					ToolBarGetVarIndices->PerformClick();

				if (!BookmarkList->Visible)
				{
					BookmarkList->Show();
					BookmarkList->BringToFront();
					ToolBarBookmarkList->Checked = true;
					WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height / 2;
				}
				else
				{
					BookmarkList->Hide();
					ToolBarBookmarkList->Checked = false;
					WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height;
				}
			}
			void Workspace::ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E)
			{
				SaveFileDialog^ SaveManager = gcnew SaveFileDialog();

				SaveManager->DefaultExt = "*.txt";
				SaveManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
				SaveManager->FileName = EditorTab->Tooltip;
				SaveManager->RestoreDirectory = true;

				if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->FileName->Length > 0)
				{
					SaveScriptToDisk(SaveManager->FileName, true);
				}
			}
			void Workspace::ToolBarDumpAllScripts_Click(Object^ Sender, EventArgs^ E)
			{
				FolderBrowserDialog^ SaveManager = gcnew FolderBrowserDialog();

				SaveManager->Description = "All open scripts in this window will be dumped to the selected folder.";
				SaveManager->ShowNewFolderButton = true;
				SaveManager->SelectedPath = Globals::AppPath + "\\Data\\Scripts";

				if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->SelectedPath->Length > 0)
				{
					ParentContainer->SaveAllTabsToFolder(SaveManager->SelectedPath);
					DebugPrint("Dumped all open scripts to " + SaveManager->SelectedPath);
				}
			}
			void Workspace::ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E)
			{
				OpenFileDialog^ LoadManager = gcnew OpenFileDialog();

				LoadManager->DefaultExt = "*.txt";
				LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
				LoadManager->RestoreDirectory = true;

				if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileName->Length > 0)
				{
					LoadFileFromDisk(LoadManager->FileName);
				}
			}
			void Workspace::ToolBarLoadScriptsToTabs_Click(Object^ Sender, EventArgs^ E)
			{
				OpenFileDialog^ LoadManager = gcnew OpenFileDialog();

				LoadManager->DefaultExt = "*.txt";
				LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
				LoadManager->Multiselect = true;
				LoadManager->RestoreDirectory = true;

				if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileNames->Length > 0)
				{
					for each (String^ Itr in LoadManager->FileNames)
					{
						ParentContainer->LoadFileToNewTab(Itr);
					}
				}
			}
			void Workspace::ToolBarGetVarIndices_Click(Object^ Sender, EventArgs^ E)
			{
				if (!VariableIndexList->Visible)
				{
					if (TextEditor->GetModifiedStatus())
						MessageBox::Show("The current script needs to be compiled before its variable indices can be updated.", "CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
					else
					{
						ToolBarUpdateVarIndices->Enabled = false;

						if (MessageList->Visible)
							ToolBarMessageList->PerformClick();
						else if (FindList->Visible)
							ToolBarFindList->PerformClick();
						else if (BookmarkList->Visible)
							ToolBarBookmarkList->PerformClick();

						VariableIndexList->Items->Clear();
						ComponentDLLInterface::ScriptVarListData* Data = g_CSEInterface->ScriptEditor.GetScriptVarList((CString(CurrentScriptEditorID)).c_str());
						if (Data)
						{
							for (int i = 0; i < Data->ScriptVarListCount; i++)
							{
								ComponentDLLInterface::ScriptVarListData::ScriptVarInfo* VarInfo = &Data->ScriptVarListHead[i];
								String^ VarType;
								switch (VarInfo->Type)
								{
								case 0:
									VarType = "Float";
									break;
								case 1:
									VarType = "Integer";
									break;
								case 2:
									VarType = "Reference";
									break;
								}

								ListViewItem^ Item = gcnew ListViewItem(gcnew String(VarInfo->Name));
								Item->SubItems->Add(VarType);
								Item->SubItems->Add(VarInfo->Index.ToString());
								VariableIndexList->Items->Add(Item);
							}
						}
						g_CSEInterface->DeleteNativeHeapPointer(Data, false);

						VariableIndexList->Show();
						VariableIndexList->BringToFront();

						if (VariableIndexList->Items->Count)
							ToolBarUpdateVarIndices->Enabled = true;

						WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height / 2;
						ToolBarGetVarIndices->Checked = true;
					}
				}
				else
				{
					VariableIndexList->Hide();

					ToolBarGetVarIndices->Checked = false;
					ToolBarUpdateVarIndices->Enabled = false;
					WorkspaceSplitter->SplitterDistance = ParentContainer->GetEditorFormRect().Height;
				}
			}
			void Workspace::ToolBarUpdateVarIndices_Click(Object^ Sender, EventArgs^ E)
			{
				CString CScriptName(CurrentScriptEditorID);
				CString^ Buffer = nullptr;

				List<CString^>^ StringAllocations = gcnew List<CString^>();
				int Count = 0;
				for each (ListViewItem^ Itr in VariableIndexList->Items)
				{
					if (Itr->Tag != nullptr)
						Count++;
				}

				ComponentDLLInterface::ScriptVarListData Data;
				Data.ScriptVarListCount = Count;
				Data.ScriptVarListHead = new ComponentDLLInterface::ScriptVarListData::ScriptVarInfo[Data.ScriptVarListCount];

				int i = 0;
				for each (ListViewItem^ Itr in VariableIndexList->Items)
				{
					if (Itr->Tag != nullptr)
					{
						Buffer = gcnew CString(Itr->Text);
						StringAllocations->Add(Buffer);

						UInt32 Index = UInt32::Parse(Itr->SubItems[2]->Text);

						Data.ScriptVarListHead[i].Index = Index;
						if	(!String::Compare(Itr->SubItems[1]->Text, "Integer", true))
							Data.ScriptVarListHead[i].Type = 1;
						else if (!String::Compare(Itr->SubItems[1]->Text, "Float", true))
							Data.ScriptVarListHead[i].Type = 0;
						else
							Data.ScriptVarListHead[i].Type = 2;
						Data.ScriptVarListHead[i].Name = Buffer->c_str();
						i++;
					}
				}

				if (!g_CSEInterface->ScriptEditor.UpdateScriptVarIndices(CScriptName.c_str(), &Data))
					DebugPrint("Couldn't successfully update all variable indices of script '" + CurrentScriptEditorID + "'");
				else
				{
					if (OPTIONS->FetchSettingAsInt("RecompileVarIdx"))
						ToolBarCompileDependencies->PerformClick();
				}

				if (VariableIndexList->Visible)
					ToolBarGetVarIndices->PerformClick();

				for each (CString^ Itr in StringAllocations)
					delete Itr;

				StringAllocations->Clear();
			}
			void Workspace::ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E)
			{
				if (ToolBarShowOffsets->Checked)
				{
					OffsetViewer->Hide();
					TextEditor->FocusTextArea();
					ToolBarShowOffsets->Checked = false;
				}
				else
				{
					if (OffsetViewer->Show())
						ToolBarShowOffsets->Checked = true;
				}
			}
			void Workspace::ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E)
			{
				if (ToolBarShowPreprocessedText->Checked)
				{
					PreprocessedTextViewer->Hide();
					TextEditor->FocusTextArea();
					ToolBarShowPreprocessedText->Checked = false;
				}
				else
				{
					ClearErrorsItemsFromMessagePool();
					String^ PreprocessedText = "";
					if (PreprocessScriptText(PreprocessedText))
					{
						PreprocessedTextViewer->Show(PreprocessedText);
						ToolBarShowPreprocessedText->Checked = true;
					}
					else
						MessageBox::Show("The preprocessing operation was unsuccessful.", "CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Error);
				}
			}
			void Workspace::ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E)
			{
				try
				{
					NativeWrapper::LockWindowUpdate(TextEditor->GetHandle());
					if (OPTIONS->FetchSettingAsInt("AnnealCasing"))
						SanitizeScriptText(SanitizeOperation::e_AnnealCasing);

					if (OPTIONS->FetchSettingAsInt("IndentLines"))
						SanitizeScriptText(SanitizeOperation::e_Indent);
				}
				finally
				{
					NativeWrapper::LockWindowUpdate(IntPtr::Zero);
				}
			}

			void Workspace::ToolBarBindScript_Click(Object^ Sender, EventArgs^ E)
			{
				if (GetIsCurrentScriptNew() || GetIsFirstRun())
				{
					MessageBox::Show("You may only perform this operation on an existing script.", "Message - CSE Script Editor", MessageBoxButtons::OK, MessageBoxIcon::Information);
					return;
				}
				else
				{
					g_CSEInterface->ScriptEditor.BindScript((CString(CurrentScriptEditorID)).c_str(), (HWND)GetParentContainer()->GetEditorFormHandle());
				}
			}
		#pragma endregion
	#pragma endregion
}