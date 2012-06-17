#include "ScriptEditor.h"
#include "ScriptEditorManager.h"
#include "RefactorTools.h"
#include "AvalonEditTextEditor.h"
#include "IntelliSense\IntelliSenseDatabase.h"
#include "IntelliSense\IntelliSenseItem.h"

#include "[Common]\HandShakeStructs.h"
#include "[Common]\ListViewUtilities.h"
#include "[Common]\NativeWrapper.h"
#include "[Common]\CustomInputBox.h"

namespace ConstructionSetExtender
{
	using namespace IntelliSense;

	namespace ScriptEditor
	{
#pragma region WorkspaceContainer
		void WorkspaceTearingEventHandler(Object^ Sender, MouseEventArgs^ E)
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
							Parameters->ParameterList->Add(ScriptEditorManager::WorkspaceTearOpType::e_NewContainer);
							Parameters->ParameterList->Add(SEMGR->TornWorkspace);
							Parameters->ParameterList->Add(nullptr);
							Parameters->ParameterList->Add(E->Location);
							SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_WorkspaceTearing, Parameters);

							HookManager::MouseUp -= WorkspaceContainer::WorkspaceTearingEventDelegate;
							SEMGR->TornWorkspace = nullptr;
						}

						DotNetBar::SuperTabStrip^ Strip = nullptr;
						try
						{
							Strip = dynamic_cast<DotNetBar::SuperTabStrip^>(Control::FromHandle(Wnd));
						}
						catch (Exception^ E)
						{
							DebugPrint("An exception was raised during a tab tearing operation!\n\tError Message: " + E->Message, true);
							Strip = nullptr;
						}
						if (Strip != nullptr)
						{
							if (SEMGR->TornWorkspace->GetIsTabStripParent(Strip))		// not a tearing op , the strip's the same
							{
								HookManager::MouseUp -= WorkspaceContainer::WorkspaceTearingEventDelegate;
								SEMGR->TornWorkspace = nullptr;
							}
							else
							{
								ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
								Parameters->EditorHandleIndex = 0;
								Parameters->ParameterList->Add(ScriptEditorManager::WorkspaceTearOpType::e_RelocateToContainer);
								Parameters->ParameterList->Add(SEMGR->TornWorkspace);
								Parameters->ParameterList->Add(dynamic_cast<WorkspaceContainer^>(Strip->Tag));
								Parameters->ParameterList->Add(E->Location);
								SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_WorkspaceTearing, Parameters);

								HookManager::MouseUp -= WorkspaceContainer::WorkspaceTearingEventDelegate;
								SEMGR->TornWorkspace = nullptr;
							}
						}
						else
						{
							ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
							Parameters->EditorHandleIndex = 0;
							Parameters->ParameterList->Add(ScriptEditorManager::WorkspaceTearOpType::e_NewContainer);
							Parameters->ParameterList->Add(SEMGR->TornWorkspace);
							Parameters->ParameterList->Add(nullptr);
							Parameters->ParameterList->Add(E->Location);
							SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_WorkspaceTearing, Parameters);

							HookManager::MouseUp -= WorkspaceContainer::WorkspaceTearingEventDelegate;
							SEMGR->TornWorkspace = nullptr;
						}
					}
					else
					{
						DebugPrint("Global tab tear hook called out of turn! Expecting an unresolved operation.");
						HookManager::MouseUp -= WorkspaceContainer::WorkspaceTearingEventDelegate;
					}
					break;
				}
			default:
				if (SEMGR->TornWorkspace != nullptr)
				{
					HookManager::MouseUp -= WorkspaceContainer::WorkspaceTearingEventDelegate;
					SEMGR->TornWorkspace = nullptr;
				}
				break;
			}
		}

		WorkspaceContainer::WorkspaceContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height)
		{
			Application::EnableVisualStyles();
			InitializedFlag = false;

			EditorFormCancelHandler =  gcnew CancelEventHandler(this, &WorkspaceContainer::EditorForm_Cancel);
			EditorFormKeyDownHandler = gcnew KeyEventHandler(this, &WorkspaceContainer::EditorForm_KeyDown);
			EditorFormPositionChangedHandler = gcnew EventHandler(this, &WorkspaceContainer::EditorForm_PositionChanged);
			EditorFormSizeChangedHandler = gcnew EventHandler(this, &WorkspaceContainer::EditorForm_SizeChanged);

			ScriptStripTabItemCloseHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>(this, &WorkspaceContainer::ScriptStrip_TabItemClose);
			ScriptStripSelectedTabChangedHandler = gcnew EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>(this, &WorkspaceContainer::ScriptStrip_SelectedTabChanged);
			ScriptStripTabRemovedHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabRemovedEventArgs^>(this, &WorkspaceContainer::ScriptStrip_TabRemoved);
			ScriptStripMouseClickHandler = gcnew EventHandler<MouseEventArgs^>(this, &WorkspaceContainer::ScriptStrip_MouseClick);
			ScriptStripTabMovingHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>(this, &WorkspaceContainer::ScriptStrip_TabMoving);

			NewTabButtonClickHandler = gcnew EventHandler(this, &WorkspaceContainer::NewTabButton_Click);
			SortTabsButtonClickHandler = gcnew EventHandler(this, &WorkspaceContainer::SortTabsButton_Click);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &WorkspaceContainer::ScriptEditorPreferences_Saved);

			EditorForm = gcnew AnimatedForm(0.10);
			EditorForm->SuspendLayout();

			EditorForm->FormBorderStyle = FormBorderStyle::Sizable;
			EditorForm->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			EditorForm->AutoScaleMode = AutoScaleMode::Font;
			EditorForm->Size = Size(Width, Height);
			EditorForm->KeyPreview = true;

			EditorTabStrip = gcnew DotNetBar::SuperTabControl();
			EditorTabStrip->SuspendLayout();
			EditorTabStrip->Dock = DockStyle::Fill;
			EditorTabStrip->Location = Point(0, 0);
			EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Top;
			EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
			EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Far;
			EditorTabStrip->Font = gcnew Font("Segoe UI", 10, FontStyle::Regular);
			EditorTabStrip->TabFont = gcnew Font("Segoe UI", 9, FontStyle::Bold);
			EditorTabStrip->SelectedTabFont = gcnew Font("Segoe UI", 9, FontStyle::Bold);
			EditorTabStrip->FixedTabSize = Size(0, 23);

			if (!PREFERENCES->FetchSettingAsInt("TabsOnTop", "Appearance"))
			{
				EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Left;
				EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
				EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Near;
				EditorTabStrip->FixedTabSize = Size(150, 40);
			}

			EditorTabStrip->CloseButtonOnTabsVisible = true;
			EditorTabStrip->CloseButtonOnTabsAlwaysDisplayed = false;
			EditorTabStrip->AntiAlias = true;
			EditorTabStrip->TabStop = false;
			EditorTabStrip->ImageList = gcnew ImageList;
			EditorTabStrip->ImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOff"));
			EditorTabStrip->ImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOn"));
			EditorTabStrip->ImageList->ImageSize = Size(12, 12);
			EditorTabStrip->ImageList->TransparentColor = Color::White;
			EditorTabStrip->ReorderTabsEnabled = true;
			EditorTabStrip->TabStyle = DotNetBar::eSuperTabStyle::Office2010BackstageBlue;
			EditorTabStrip->TabStripColor->OuterBorder = Color::FromArgb(255, 59, 59, 59);
			EditorTabStrip->TabStripColor->InnerBorder = Color::FromArgb(255, 59, 59, 59);

			EditorTabStrip->TabStrip->Tag = this;

			NewTabButton = gcnew DotNetBar::ButtonItem();
			NewTabButton->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource("NewTabButton");
			NewTabButton->ButtonStyle = DotNetBar::eButtonStyle::Default;
			NewTabButton->Style = DotNetBar::eDotNetBarStyle::Office2007;
			NewTabButton->ColorTable = DotNetBar::eButtonColor::Office2007WithBackground;
			NewTabButton->Text = "New Tab";
			NewTabButton->Tooltip = "New Tab";

			SortTabsButton = gcnew DotNetBar::ButtonItem();
			SortTabsButton->ButtonStyle = DotNetBar::eButtonStyle::TextOnlyAlways;
			SortTabsButton->Text = "Sort Tabs";
			SortTabsButton->Tooltip = "Sort Tabs";

			NewTabButton->SubItems->Add(SortTabsButton);
			EditorTabStrip->ControlBox->SubItems->Add(NewTabButton);
			EditorTabStrip->ControlBox->Visible = true;

			EditorForm->HelpButton = false;
			EditorForm->Text = SCRIPTEDITOR_TITLE;

			EditorForm->Controls->Add(EditorTabStrip);

			if (PREFERENCES->FetchSettingAsInt("UseCSParent", "General"))
			{
				EditorForm->ShowInTaskbar = false;
				EditorForm->Show(gcnew WindowHandleWrapper((IntPtr)NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetMainWindowHandle()));
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
			// 			EditorTabStrip->TabStripColor->Background = gcnew DotNetBar::Rendering::SuperTabLinearGradientColorTable(Color::FromArgb(255, 48, 59, 61),
			// 																													Color::FromArgb(255, 61, 76, 106));
			EditorTabStrip->TabStripColor->Background = gcnew DotNetBar::Rendering::SuperTabLinearGradientColorTable(Color::FromArgb(255, 59, 59, 59),
				Color::FromArgb(255, 60, 60, 60));

			EditorTabStrip->ResumeLayout();
			EditorForm->ResumeLayout();

			EditorForm->Closing += EditorFormCancelHandler;
			EditorForm->KeyDown += EditorFormKeyDownHandler;
			EditorForm->Move += EditorFormPositionChangedHandler;
			EditorForm->SizeChanged += EditorFormSizeChangedHandler;
			EditorForm->MaximizedBoundsChanged += EditorFormSizeChangedHandler;
			EditorTabStrip->TabItemClose += ScriptStripTabItemCloseHandler;
			EditorTabStrip->SelectedTabChanged += ScriptStripSelectedTabChangedHandler;
			EditorTabStrip->TabRemoved += ScriptStripTabRemovedHandler;
			EditorTabStrip->TabStripMouseClick += ScriptStripMouseClickHandler;
			EditorTabStrip->TabMoving += ScriptStripTabMovingHandler;
			NewTabButton->Click += NewTabButtonClickHandler;
			SortTabsButton->Click += SortTabsButtonClickHandler;
			PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;
			InitializedFlag = true;

			InstantiateNewWorkspace(InitScript);
		}

		void WorkspaceContainer::EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			if (DestructionFlag)
			{
				return;
			}

			E->Cancel = true;
			if (EditorTabStrip->Tabs->Count > 1)
			{
				if (MessageBox::Show("Are you sure you want to close all open tabs?",
									SCRIPTEDITOR_TITLE,
									MessageBoxButtons::YesNo,
									MessageBoxIcon::Question,
									MessageBoxDefaultButton::Button2) == DialogResult::No)
				{
					return;
				}
			}

			FlagDestruction(true);
			CloseAllOpenWorkspaces();
			FlagDestruction(false);
		}

		void WorkspaceContainer::EditorForm_PositionChanged(Object^ Sender, EventArgs^ E)
		{
			if (!InitializedFlag)
				return;

			Workspace^ Itr = dynamic_cast<Workspace^>(EditorTabStrip->SelectedTab->Tag);
			if (Itr != nullptr)
				Itr->HandlePositionSizeChange();
		}

		void WorkspaceContainer::EditorForm_SizeChanged(Object^ Sender, EventArgs^ E)
		{
			if (!InitializedFlag)
				return;

			Workspace^ Itr = dynamic_cast<Workspace^>(EditorTabStrip->SelectedTab->Tag);
			if (Itr != nullptr)
				Itr->HandlePositionSizeChange();
		}

		void WorkspaceContainer::ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs^ E)
		{
			Workspace^ Itr = dynamic_cast<Workspace^>(E->Tab->Tag);		// clicking on the close button doesn't change the active tab

			E->Cancel = true;

			Itr->CloseScript();
		}

		void WorkspaceContainer::ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs ^ E)
		{
			if (E->OldValue == nullptr || E->NewValue == nullptr)
				return;

			Workspace^ OldWorkspace = dynamic_cast<Workspace^>(E->OldValue->Tag);
			Workspace^ NewWorkspace = dynamic_cast<Workspace^>(E->NewValue->Tag);

			if (OldWorkspace != nullptr && !OldWorkspace->GetIsUninitialized())
				OldWorkspace->DisableControls();
			if (!NewWorkspace->GetIsUninitialized())
				NewWorkspace->EnableControls();

			EditorForm->Text = NewWorkspace->GetScriptDescription() + " - " + SCRIPTEDITOR_TITLE;
			EditorForm->Focus();

			if (OldWorkspace != nullptr)
				OldWorkspace->HandleFocus(false);

			NewWorkspace->HandleFocus(true);
		}

		void WorkspaceContainer::ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E)
		{
			if (EditorTabStrip->Tabs->Count == 0)
			{
				if (!DestructionFlag && PREFERENCES->FetchSettingAsInt("DestroyOnLastTabClose", "General") == 0)
				{
					InstantiateNewWorkspace(0);
				}
				else
				{
					ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
					Parameters->ParameterList->Add(this);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_ReleaseWorkspaceContainer, Parameters);
				}
			}
		}

		Workspace^ WorkspaceContainer::InstantiateNewWorkspace(ComponentDLLInterface::ScriptData* InitScript)
		{
			BeginUpdate();

			ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
			Parameters->EditorHandleIndex = 0;
			Parameters->ParameterList->Add(this);
			Parameters->ParameterList->Add((UInt32)InitScript);

			Workspace^ NewWorkspace = dynamic_cast<Workspace^>(SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateWorkspace, Parameters));
			Trace::Assert(NewWorkspace != nullptr, "NewWorkspace == nullptr");
			NewWorkspace->BringToFront();

			EndUpdate();

			return NewWorkspace;
		}

		void WorkspaceContainer::PerformRemoteWorkspaceOperation(RemoteWorkspaceOperation Operation, List<Object^>^ Parameters)
		{
			switch (Operation)
			{
			case RemoteWorkspaceOperation::e_CreateNewWorkspaceAndScript:
				{
					Workspace^ NewWorkspace = InstantiateNewWorkspace(0);
					NewWorkspace->NewScript();
					break;
				}
			case RemoteWorkspaceOperation::e_CreateNewWorkspaceAndSelectScript:
				{
					Workspace^ NewWorkspace = InstantiateNewWorkspace(0);
					NewWorkspace->OpenScript();
					break;
				}
			case RemoteWorkspaceOperation::e_LoadFileIntoNewWorkspace:
				{
					String^ FilePath = dynamic_cast<String^>(Parameters[0]);
					Workspace^ NewWorkspace = nullptr;

					if (PREFERENCES->FetchSettingAsInt("LoadScriptUpdateExistingScripts", "General") == 0)
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
							ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CEID.c_str());
							if (Data)
								NewWorkspace = InstantiateNewWorkspace(Data);
							else
							{
								NewWorkspace = InstantiateNewWorkspace(0);
								NewWorkspace->NewScript();
							}
						}
						catch (Exception^ E)
						{
							DebugPrint("Couldn't read script name from file " + FilePath + " for script updating!\n\tException: " + E->Message, true);
							NewWorkspace = InstantiateNewWorkspace(0);
							NewWorkspace->NewScript();
						}
					}

					NewWorkspace->LoadFileFromDisk(FilePath);
					break;
				}
			case RemoteWorkspaceOperation::e_CreateNewWorkspaceAndScriptAndSetText:
				{
					Workspace^ NewWorkspace = InstantiateNewWorkspace(0);
					NewWorkspace->NewScript();
					NewWorkspace->SetScriptText(dynamic_cast<String^>(Parameters[0]), true);
					break;
				}
			case RemoteWorkspaceOperation::e_FindReplaceInOpenWorkspaces:
				{
					for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
					{
						Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);
						if (Editor != nullptr)
						{
							Editor->PerformFindReplace((TextEditors::IScriptTextEditor::FindReplaceOperation)((int)Parameters[0]),
														dynamic_cast<String^>(Parameters[1]),
														dynamic_cast<String^>(Parameters[2]),
														(UInt32)(Parameters[3]));
						}
					}
					break;
				}
			}
		}

		WorkspaceContainer::~WorkspaceContainer()
		{
			WorkspaceContainer::Destroy();
		}

		void WorkspaceContainer::Destroy()
		{
			FlagDestruction(true);

			BackJumpStack->Clear();
			ForwardJumpStack->Clear();
			for each (Image^ Itr in EditorTabStrip->ImageList->Images)
				delete Itr;
			EditorTabStrip->ImageList->Images->Clear();
			EditorTabStrip->ImageList = nullptr;
			EditorTabStrip->TabStrip->Tag = nullptr;
			EditorTabStrip->Tabs->Clear();

			EditorForm->Closing -= EditorFormCancelHandler;
			EditorForm->KeyDown -= EditorFormKeyDownHandler;
			EditorForm->Move -= EditorFormPositionChangedHandler;
			EditorForm->SizeChanged -= EditorFormSizeChangedHandler;
			EditorForm->MaximizedBoundsChanged -= EditorFormSizeChangedHandler;
			EditorTabStrip->TabItemClose -= ScriptStripTabItemCloseHandler;
			EditorTabStrip->SelectedTabChanged -= ScriptStripSelectedTabChangedHandler;
			EditorTabStrip->TabRemoved -= ScriptStripTabRemovedHandler;
			EditorTabStrip->TabStripMouseClick -= ScriptStripMouseClickHandler;
			EditorTabStrip->TabMoving -= ScriptStripTabMovingHandler;
			NewTabButton->Click -= NewTabButtonClickHandler;
			SortTabsButton->Click -= SortTabsButtonClickHandler;
			PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;

			delete EditorTabStrip;
			delete NewTabButton->Image;
			delete NewTabButton;
			delete SortTabsButton;

			EditorForm->ForceClose();

			PREFERENCES->SaveINI();
		}

		void WorkspaceContainer::JumpToWorkspace(UInt32 AllocatedIndex, String^% ScriptName)
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
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CEID.c_str());
				if (Data)
					InstantiateNewWorkspace(Data);
				else
					InstantiateNewWorkspace(0);
			}

			BackJumpStack->Push(AllocatedIndex);
			ForwardJumpStack->Clear();
		}

		void WorkspaceContainer::NavigateJumpStack(UInt32 AllocatedIndex, WorkspaceContainer::JumpStackNavigationDirection Direction)
		{
			UInt32 JumpIndex = 0;
			switch (Direction)
			{
			case JumpStackNavigationDirection::e_Back:
				if (BackJumpStack->Count < 1)
					return;

				JumpIndex = BackJumpStack->Pop();
				break;
			case JumpStackNavigationDirection::e_Forward:
				if (ForwardJumpStack->Count < 1)
					return;

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
				case JumpStackNavigationDirection::e_Back:
					ForwardJumpStack->Push(AllocatedIndex);
					break;
				case JumpStackNavigationDirection::e_Forward:
					BackJumpStack->Push(AllocatedIndex);
					break;
				}

				Itr->BringToFront();
			}
		}

		void WorkspaceContainer::NewTabButton_Click(Object^ Sender, EventArgs^ E)
		{
			InstantiateNewWorkspace(0);
		}

		void WorkspaceContainer::EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E)
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

		DotNetBar::SuperTabItem^ WorkspaceContainer::GetMouseOverTab()
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				if (Itr->IsMouseOver)
					return Itr;
			}

			return nullptr;
		}

		void WorkspaceContainer::ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E)
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

		void WorkspaceContainer::ScriptStrip_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E)
		{
			if (SEMGR->TornWorkspace != nullptr)
				return;			// wait until the global hook clears the handle, as the TabMoving event gets called for every frame of mouse movement

			DotNetBar::SuperTabItem^ MouseOverTab = GetMouseOverTab();
			if (MouseOverTab != nullptr)
			{
				SEMGR->TornWorkspace = dynamic_cast<Workspace^>(MouseOverTab->Tag);;
				HookManager::MouseUp += WorkspaceTearingEventDelegate;
			}
		}

		void WorkspaceContainer::SaveAllOpenWorkspaces()
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);
				Editor->SaveScript(Workspace::ScriptSaveOperation::e_SaveAndCompile);
			}
		}

		void WorkspaceContainer::CloseAllOpenWorkspaces()
		{
			List<Workspace^>^ ActiveWorkspaces = gcnew List<Workspace^>();
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
				ActiveWorkspaces->Add(dynamic_cast<Workspace^>(Itr->Tag));

			for each (Workspace^ Editor in ActiveWorkspaces)
				Editor->CloseScript();

			ActiveWorkspaces->Clear();
		}

		void WorkspaceContainer::DumpAllOpenWorkspacesToFolder(String^ FolderPath)
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);

				if (Editor->GetIsScriptNew() || Editor->GetIsUninitialized())
					continue;

				Editor->SaveScriptToDisk(FolderPath, false);
			}
		}

		void WorkspaceContainer::LoadFileIntoNewWorkspace(String^ FileName)
		{
			List<Object^>^ RemoteOpParameters = gcnew List<Object^>();
			RemoteOpParameters->Add(FileName);
			PerformRemoteWorkspaceOperation(RemoteWorkspaceOperation::e_LoadFileIntoNewWorkspace, RemoteOpParameters);
		}

		Rectangle WorkspaceContainer::GetBounds()
		{
			if (GetWindowState() == FormWindowState::Normal)
				return EditorForm->Bounds;
			else
				return EditorForm->RestoreBounds;
		}

		Workspace^ WorkspaceContainer::LookupWorkspaceByTabIndex(UInt32 TabIndex)
		{
			if (TabIndex >= EditorTabStrip->Tabs->Count)
				return nullptr;
			else
				return dynamic_cast<Workspace^>(EditorTabStrip->Tabs[TabIndex]->Tag);
		}

		void WorkspaceContainer::AddTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->Tabs->Add(Tab);
		}

		void WorkspaceContainer::RemoveTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->Tabs->Remove(Tab);
		}

		void WorkspaceContainer::AddTabControlBox(DotNetBar::SuperTabControlPanel^ Box)
		{
			EditorTabStrip->Controls->Add(Box);
		}

		void WorkspaceContainer::RemoveTabControlBox(DotNetBar::SuperTabControlPanel^ Box)
		{
			EditorTabStrip->Controls->Remove(Box);
		}

		void WorkspaceContainer::SelectTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->SelectedTab = Tab;
			EditorTabStrip->TabStrip->EnsureVisible(Tab);
		}

		void WorkspaceContainer::SelectTab( Keys Index )
		{
			int TabIndex = -1;

			switch (Index)
			{
			case Keys::D1:
				TabIndex = 0;
				break;
			case Keys::D2:
				TabIndex = 1;
				break;
			case Keys::D3:
				TabIndex = 2;
				break;
			case Keys::D4:
				TabIndex = 3;
				break;
			case Keys::D5:
				TabIndex = 4;
				break;
			case Keys::D6:
				TabIndex = 5;
				break;
			case Keys::D7:
				TabIndex = 6;
				break;
			case Keys::D8:
				TabIndex = 7;
				break;
			case Keys::D9:
				TabIndex = 8;
				break;
			default:
				break;
			}

			if (TabIndex != -1)
			{
				SelectTab(TabIndex);
			}
		}

		void WorkspaceContainer::SelectTab( int Index )
		{
			if (Index == -1)
				SelectTab(dynamic_cast<DotNetBar::SuperTabItem^>(EditorTabStrip->Tabs[EditorTabStrip->Tabs->Count - 1]));
			else if (Index < EditorTabStrip->Tabs->Count)
				SelectTab(dynamic_cast<DotNetBar::SuperTabItem^>(EditorTabStrip->Tabs[Index]));
		}

		void WorkspaceContainer::SortTabsButton_Click( Object^ Sender, EventArgs^ E )
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

		void WorkspaceContainer::SelectPreviousTab()
		{
			if (EditorTabStrip->Tabs->Count < 2)
				return;

			if (EditorTabStrip->SelectPreviousTab() == false)
				SelectTab(-1);
		}

		void WorkspaceContainer::SelectNextTab()
		{
			if (EditorTabStrip->Tabs->Count < 2)
				return;

			if (EditorTabStrip->SelectNextTab() == false)
				SelectTab(0);
		}

		void WorkspaceContainer::DisableControls( void )
		{
			EditorForm->Enabled = false;
		}

		void WorkspaceContainer::EnableControls( void )
		{
			EditorForm->Enabled = true;
		}

		void WorkspaceContainer::ScriptEditorPreferences_Saved( Object^ Sender, EventArgs^ E )
		{
			EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Top;
			EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
			EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Far;
			EditorTabStrip->FixedTabSize = Size(0, 23);

			if (!PREFERENCES->FetchSettingAsInt("TabsOnTop", "Appearance"))
			{
				EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Left;
				EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
				EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Near;
				EditorTabStrip->FixedTabSize = Size(150, 40);
			}

			Redraw();
		}

		void WorkspaceContainer::SetCursor( Cursor^ NewCursor )
		{
			EditorForm->Cursor = NewCursor;
		}

		void WorkspaceContainer::FlagDestruction( bool Destroying )
		{
			this->DestructionFlag = Destroying;
		}

		void WorkspaceContainer::Redraw()
		{
			EditorForm->Invalidate(true);
		}

		void WorkspaceContainer::SetWindowTitle( String^ Title )
		{
			EditorForm->Text = Title;
		}

		System::IntPtr WorkspaceContainer::GetHandle()
		{
			return EditorForm->Handle;
		}

		System::Windows::Forms::FormWindowState WorkspaceContainer::GetWindowState()
		{
			return EditorForm->WindowState;
		}

		void WorkspaceContainer::SetWindowState( FormWindowState State )
		{
			EditorForm->WindowState = State;
		}

		UInt32 WorkspaceContainer::GetTabCount()
		{
			return EditorTabStrip->Tabs->Count;
		}

		void WorkspaceContainer::BeginUpdate( void )
		{
			EditorForm->SuspendLayout();
		}

		void WorkspaceContainer::EndUpdate( void )
		{
			EditorForm->ResumeLayout();
		}

#pragma endregion

#pragma region Workspace
		Workspace::Workspace(UInt32 Index, WorkspaceContainer^ Parent, ComponentDLLInterface::ScriptData* InitScript)
		{
			ParentContainer = Parent;
			WorkspaceHandleIndex = Index;
			DestructionFlag = false;
			CurrentScriptEditorID = FIRSTRUNSCRIPTID;
			HandlingKeyDownEvent = false;
			NewScriptFlag = false;

			WorkspaceTabItem = gcnew DotNetBar::SuperTabItem();
			WorkspaceControlBox = gcnew DotNetBar::SuperTabControlPanel();
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

			ScriptListBox = gcnew ScriptListDialog(WorkspaceHandleIndex);
			FindReplaceBox = gcnew FindReplaceDialog(WorkspaceHandleIndex);

			AutoSaveTimer = gcnew Timer();
			AutoSaveTimer->Interval = PREFERENCES->FetchSettingAsInt("AutoRecoverySavePeriod", "Backup") * 1000 * 60;

			Color ForeColor = Color::Black;
			Color BackColor = Color::White;
			Color HighlightColor = Color::Maroon;
			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"), PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"), (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));

			TextEditor = gcnew TextEditors::AvalonEditor::AvalonEditTextEditor(CustomFont, WorkspaceHandleIndex);
			OffsetViewer = gcnew TextEditors::ScriptOffsetViewer(CustomFont, ForeColor, BackColor, HighlightColor, WorkspaceSplitter->Panel1);
			PreprocessedTextViewer = gcnew TextEditors::SimpleTextViewer(CustomFont, ForeColor, BackColor, HighlightColor, WorkspaceSplitter->Panel1);

			int TabSize = Decimal::ToInt32(PREFERENCES->FetchSettingAsInt("TabSize", "Appearance"));
			if (TabSize)
				TextEditor->SetTabCharacterSize(TabSize);

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
			SetupControlImage(ContextMenuDirectLink);
			SetupControlImage(ContextMenuJumpToScript);
			SetupControlImage(ContextMenuGoogleLookup);
			SetupControlImage(ContextMenuRefactorMenu);
			SetupControlImage(ContextMenuRefactorAddVariable);
			SetupControlImage(ContextMenuRefactorDocumentScript);
			SetupControlImage(ContextMenuRefactorCreateUDFImplementation);
			SetupControlImage(ContextMenuRefactorRenameVariables);

			TextEditorKeyDownHandler = gcnew KeyEventHandler(this, &Workspace::TextEditor_KeyDown);
			TextEditorScriptModifiedHandler = gcnew TextEditors::ScriptModifiedEventHandler(this, &Workspace::TextEditor_ScriptModified);
			MessageListDoubleClickHandler = gcnew EventHandler(this, &Workspace::MessageList_DoubleClick);
			MessageListColumnClickHandler = gcnew ColumnClickEventHandler(this, &Workspace::MessageList_ColumnClick);
			FindListDoubleClickHandler = gcnew EventHandler(this, &Workspace::FindList_DoubleClick);
			FindListColumnClickHandler = gcnew ColumnClickEventHandler(this, &Workspace::FindList_ColumnClick);
			BookmarkListDoubleClickHandler = gcnew EventHandler(this, &Workspace::BookmarkList_DoubleClick);
			BookmarkListColumnClickHandler = gcnew ColumnClickEventHandler(this, &Workspace::BookmarkList_ColumnClick);
			VariableIndexListDoubleClickHandler = gcnew EventHandler(this, &Workspace::VariableIndexList_DoubleClick);
			VariableIndexListColumnClickHandler = gcnew ColumnClickEventHandler(this, &Workspace::VariableIndexList_ColumnClick);
			VariableIndexEditBoxLostFocusHandler = gcnew EventHandler(this, &Workspace::VariableIndexEditBox_LostFocus);
			VariableIndexEditBoxKeyDownHandler = gcnew KeyEventHandler(this, &Workspace::VariableIndexEditBox_KeyDown);
			ToolBarNewScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarNewScript_Click);
			ToolBarOpenScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarOpenScript_Click);
			ToolBarPreviousScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarPreviousScript_Click);
			ToolBarNextScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarNextScript_Click);
			ToolBarSaveScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarSaveScript_Click);
			ToolBarSaveScriptNoCompileClickHandler = gcnew EventHandler(this, &Workspace::ToolBarSaveScriptNoCompile_Click);
			ToolBarSaveScriptAndPluginClickHandler = gcnew EventHandler(this, &Workspace::ToolBarSaveScriptAndPlugin_Click);
			ToolBarRecompileScriptsClickHandler = gcnew EventHandler(this, &Workspace::ToolBarRecompileScripts_Click);
			ToolBarCompileDependenciesClickHandler = gcnew EventHandler(this, &Workspace::ToolBarCompileDependencies_Click);
			ToolBarDeleteScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarDeleteScript_Click);
			ToolBarNavigationBackClickHandler = gcnew EventHandler(this, &Workspace::ToolBarNavigationBack_Click);
			ToolBarNavigationForwardClickHandler = gcnew EventHandler(this, &Workspace::ToolBarNavigationForward_Click);
			ToolBarSaveAllClickHandler = gcnew EventHandler(this, &Workspace::ToolBarSaveAll_Click);
			ToolBarOptionsClickHandler = gcnew EventHandler(this, &Workspace::ToolBarOptions_Click);
			ToolBarScriptTypeContentsObjectClickHandler = gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsObject_Click);
			ToolBarScriptTypeContentsQuestClickHandler = gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsQuest_Click);
			ToolBarScriptTypeContentsMagicEffectClickHandler = gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsMagicEffect_Click);
			TextEditorContextMenuOpeningHandler = gcnew CancelEventHandler(this, &Workspace::TextEditorContextMenu_Opening);
			ContextMenuCopyClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuCopy_Click);
			ContextMenuPasteClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuPaste_Click);
			ContextMenuFindClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuFind_Click);
			ContextMenuToggleCommentClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuToggleComment_Click);
			ContextMenuToggleBookmarkClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuToggleBookmark_Click);
			ContextMenuAddMessageClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuAddMessage_Click);
			ContextMenuWikiLookupClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuWikiLookup_Click);
			ContextMenuOBSEDocLookupClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuOBSEDocLookup_Click);
			ContextMenuDirectLinkClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuDirectLink_Click);
			ContextMenuJumpToScriptClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuJumpToScript_Click);
			ContextMenuGoogleLookupClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuGoogleLookup_Click);
			ContextMenuRefactorAddVariableClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuRefactorAddVariable_Click);
			ContextMenuRefactorDocumentScriptClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuRefactorDocumentScript_Click);
			ContextMenuRefactorCreateUDFImplementationClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuRefactorCreateUDFImplementation_Click);
			ContextMenuRefactorRenameVariablesClickHandler = gcnew EventHandler(this, &Workspace::ContextMenuRefactorRenameVariables_Click);
			ToolBarEditMenuContentsFindReplaceClickHandler = gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsFindReplace_Click);
			ToolBarEditMenuContentsGotoLineClickHandler = gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsGotoLine_Click);
			ToolBarEditMenuContentsGotoOffsetClickHandler = gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsGotoOffset_Click);
			ToolBarMessageListClickHandler = gcnew EventHandler(this, &Workspace::ToolBarMessageList_Click);
			ToolBarFindListClickHandler = gcnew EventHandler(this, &Workspace::ToolBarFindList_Click);
			ToolBarBookmarkListClickHandler = gcnew EventHandler(this, &Workspace::ToolBarBookmarkList_Click);
			ToolBarDumpScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarDumpScript_Click);
			ToolBarDumpAllScriptsClickHandler = gcnew EventHandler(this, &Workspace::ToolBarDumpAllScripts_Click);
			ToolBarLoadScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarLoadScript_Click);
			ToolBarLoadScriptsToTabsClickHandler = gcnew EventHandler(this, &Workspace::ToolBarLoadScriptsToTabs_Click);
			ToolBarGetVarIndicesClickHandler = gcnew EventHandler(this, &Workspace::ToolBarGetVarIndices_Click);
			ToolBarUpdateVarIndicesClickHandler = gcnew EventHandler(this, &Workspace::ToolBarUpdateVarIndices_Click);
			ToolBarShowOffsetsClickHandler = gcnew EventHandler(this, &Workspace::ToolBarShowOffsets_Click);
			ToolBarShowPreprocessedTextClickHandler = gcnew EventHandler(this, &Workspace::ToolBarShowPreprocessedText_Click);
			ToolBarSanitizeScriptTextClickHandler = gcnew EventHandler(this, &Workspace::ToolBarSanitizeScriptText_Click);
			ToolBarBindScriptClickHandler = gcnew EventHandler(this, &Workspace::ToolBarBindScript_Click);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &Workspace::ScriptEditorPreferences_Saved);
			AutoSaveTimerTickHandler = gcnew EventHandler(this, &Workspace::AutoSaveTimer_Tick);

			WorkspaceControlBox->Dock = DockStyle::Fill;
			WorkspaceControlBox->Location = Point(0, 26);
			WorkspaceControlBox->Padding = Padding(1);
			WorkspaceControlBox->TabItem = WorkspaceTabItem;

			WorkspaceTabItem->AttachedControl = WorkspaceControlBox;
			WorkspaceTabItem->Tooltip = "New Workspace";
			WorkspaceTabItem->Text = " " + WorkspaceTabItem->Tooltip;
			WorkspaceTabItem->Tag = this;

			Padding ToolBarButtonPaddingLarge = Padding(16, 0, 16, 0);
			Padding ToolBarButtonPaddingRegular = Padding(10, 0, 10, 0);

			ToolStripStatusLabel^ ToolBarSpacerA = gcnew ToolStripStatusLabel();
			ToolBarSpacerA->Spring = true;

			WorkspaceSplitter->Dock = DockStyle::Fill;
			WorkspaceSplitter->SplitterWidth = 2;
			WorkspaceSplitter->Orientation = Orientation::Horizontal;

			WorkspaceMainToolBar->GripStyle = ToolStripGripStyle::Hidden;

			ToolBarNewScript->ToolTipText = "New Script";
			ToolBarNewScript->AutoSize = true;
			ToolBarNewScript->Margin = Padding(0, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarOpenScript->ToolTipText = "Open Script";
			ToolBarOpenScript->AutoSize = true;
			ToolBarOpenScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarSaveScript->ToolTipText = "Save And Compile Script";
			ToolBarSaveScript->AutoSize = true;
			ToolBarSaveScript->Margin = ToolBarButtonPaddingLarge;
			ToolBarSaveScriptNoCompile->ToolTipText = "Save But Do Not Compile Script";
			ToolBarSaveScriptNoCompile->Text = "Save But Do Not Compile Script";
			ToolBarSaveScriptNoCompile->AutoSize = true;
			ToolBarSaveScriptAndPlugin->Text = "Save Script And Active Plugin";
			ToolBarSaveScriptAndPlugin->ToolTipText = "Save Script And Active Plugin";
			ToolBarSaveScriptAndPlugin->AutoSize = true;
			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptNoCompile);
			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptAndPlugin);
			ToolBarSaveScript->DropDown = ToolBarSaveScriptDropDown;

			ToolBarPreviousScript->ToolTipText = "Previous Script";
			ToolBarPreviousScript->AutoSize = true;
			ToolBarPreviousScript->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarNextScript->ToolTipText = "Next Script";
			ToolBarNextScript->AutoSize = true;
			ToolBarNextScript->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarRecompileScripts->ToolTipText = "Recompile Active Scripts";
			ToolBarRecompileScripts->AutoSize = true;
			ToolBarRecompileScripts->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarCompileDependencies->ToolTipText = "Recompile Script Dependencies";
			ToolBarCompileDependencies->AutoSize = true;
			ToolBarCompileDependencies->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarDeleteScript->ToolTipText = "Delete Script";
			ToolBarDeleteScript->AutoSize = true;
			ToolBarDeleteScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarSaveAll->ToolTipText = "Save All Open Scripts";
			ToolBarSaveAll->AutoSize = true;
			ToolBarSaveAll->Margin = ToolBarButtonPaddingLarge;
			ToolBarSaveAll->Alignment = ToolStripItemAlignment::Right;

			ToolBarNavigationBack->ToolTipText = "Navigate Back";
			ToolBarNavigationBack->AutoSize = true;
			ToolBarNavigationBack->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);
			ToolBarNavigationBack->Alignment = ToolStripItemAlignment::Right;

			ToolBarNavigationForward->ToolTipText = "Navigate Forward";
			ToolBarNavigationForward->AutoSize = true;
			ToolBarNavigationForward->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);
			ToolBarNavigationForward->Alignment = ToolStripItemAlignment::Right;

			ToolBarOptions->ToolTipText = "Preferences";
			ToolBarOptions->Alignment = ToolStripItemAlignment::Right;
			ToolBarOptions->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, 0, 0);

			WorkspaceMainToolBar->Dock = DockStyle::Top;
			WorkspaceMainToolBar->Items->Add(ToolBarNewScript);
			WorkspaceMainToolBar->Items->Add(ToolBarOpenScript);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveScript);
			WorkspaceMainToolBar->Items->Add(ToolBarPreviousScript);
			WorkspaceMainToolBar->Items->Add(ToolBarNextScript);
			WorkspaceMainToolBar->Items->Add(ToolBarRecompileScripts);
			WorkspaceMainToolBar->Items->Add(ToolBarCompileDependencies);
			WorkspaceMainToolBar->Items->Add(ToolBarDeleteScript);
			WorkspaceMainToolBar->Items->Add(ToolBarSpacerA);
			WorkspaceMainToolBar->Items->Add(ToolBarOptions);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationForward);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationBack);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveAll);
			WorkspaceMainToolBar->ShowItemToolTips = true;

			ToolBarMessageList->ToolTipText = "Messages";
			ToolBarMessageList->AutoSize = true;
			ToolBarMessageList->Margin = Padding(0, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarFindList->ToolTipText = "Find/Replace Results";
			ToolBarFindList->AutoSize = true;
			ToolBarFindList->Margin = ToolBarButtonPaddingRegular;

			ToolBarBookmarkList->ToolTipText = "Bookmarks";
			ToolBarBookmarkList->AutoSize = true;
			ToolBarBookmarkList->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarDumpScript->ToolTipText = "Dump Script";
			ToolBarDumpScript->AutoSize = true;
			ToolBarDumpScript->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);
			ToolBarDumpAllScripts->ToolTipText = "Dump All Tabs";
			ToolBarDumpAllScripts->Text = "Dump All Tabs";
			ToolBarDumpAllScripts->AutoSize = true;
			ToolBarDumpScriptDropDown->Items->Add(ToolBarDumpAllScripts);
			ToolBarDumpScript->DropDown = ToolBarDumpScriptDropDown;

			ToolBarLoadScript->ToolTipText = "Load Script";
			ToolBarLoadScript->AutoSize = true;
			ToolBarLoadScript->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);
			ToolBarLoadScriptsToTabs->Text = "Load Multiple Scripts Into Tabs";
			ToolBarLoadScriptsToTabs->ToolTipText = "Load Multiple Scripts Into Tabs";
			ToolBarLoadScriptsToTabs->AutoSize = true;
			ToolBarLoadScriptDropDown->Items->Add(ToolBarLoadScriptsToTabs);
			ToolBarLoadScript->DropDown = ToolBarLoadScriptDropDown;

			ToolBarGetVarIndices->ToolTipText = "Fetch Variable Indices";
			ToolBarGetVarIndices->AutoSize = true;
			ToolBarGetVarIndices->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarUpdateVarIndices->ToolTipText = "Update Variable Indices";
			ToolBarUpdateVarIndices->AutoSize = true;
			ToolBarUpdateVarIndices->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarShowOffsets->ToolTipText = "Toggle Offset Viewer";
			ToolBarShowOffsets->AutoSize = true;
			ToolBarShowOffsets->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarShowPreprocessedText->ToolTipText = "Toggle Preprocessed Text Viewer";
			ToolBarShowPreprocessedText->AutoSize = true;
			ToolBarShowPreprocessedText->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarSanitizeScriptText->ToolTipText = "Sanitize Script Text";
			ToolBarSanitizeScriptText->AutoSize = true;
			ToolBarSanitizeScriptText->Margin = ToolBarButtonPaddingLarge;

			ToolBarBindScript->ToolTipText = "Bind Script";
			ToolBarBindScript->AutoSize = true;
			ToolBarBindScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarByteCodeSize->Minimum = 0;
			ToolBarByteCodeSize->Maximum = 0x8000;
			ToolBarByteCodeSize->AutoSize = false;
			ToolBarByteCodeSize->Size = Size(125, 14);
			ToolBarByteCodeSize->ToolTipText = "Compiled Script Size";
			ToolBarByteCodeSize->Alignment = ToolStripItemAlignment::Left;
			ToolBarByteCodeSize->Margin = Padding(8, 0, 8, 0);

			ToolBarScriptTypeContentsObject->Text =					"Object                   ";
			ToolBarScriptTypeContentsObject->ToolTipText =			"Object";
			ToolBarScriptTypeContentsQuest->Text =					"Quest                    ";
			ToolBarScriptTypeContentsQuest->ToolTipText =			"Quest";
			ToolBarScriptTypeContentsMagicEffect->Text =			"Magic Effect        ";
			ToolBarScriptTypeContentsMagicEffect->ToolTipText =		"Magic Effect";
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsObject);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsQuest);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsMagicEffect);
			ToolBarScriptType->ImageTransparentColor = Color::White;
			ToolBarScriptType->DropDown = ToolBarScriptTypeContents;
			ToolBarScriptType->Alignment = ToolStripItemAlignment::Right;

			ToolBarEditMenuContentsFind->Text =			"Find";
			ToolBarEditMenuContentsReplace->Text =		"Replace";
			ToolBarEditMenuContentsGotoLine->Text =		"Goto Line";
			ToolBarEditMenuContentsGotoOffset->Text =	"Goto Offset";
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsFind);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsReplace);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoLine);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoOffset);
			ToolBarEditMenu->Text = "Edit";
			ToolBarEditMenu->DropDown = ToolBarEditMenuContents;
			ToolBarEditMenu->Padding = Padding(0);
			ToolBarEditMenu->Alignment = ToolStripItemAlignment::Right;

			WorkspaceSecondaryToolBar->GripStyle = ToolStripGripStyle::Hidden;
			WorkspaceSecondaryToolBar->Dock = DockStyle::Bottom;
			WorkspaceSecondaryToolBar->Items->Add(ToolBarMessageList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarFindList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBookmarkList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarDumpScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarLoadScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarGetVarIndices);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarUpdateVarIndices);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowOffsets);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowPreprocessedText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSanitizeScriptText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBindScript);
			WorkspaceSecondaryToolBar->Items->Add(gcnew ToolStripSeparator());
			WorkspaceSecondaryToolBar->Items->Add(ToolBarByteCodeSize);
			WorkspaceSecondaryToolBar->Items->Add(gcnew ToolStripSeparator());
			WorkspaceSecondaryToolBar->Items->Add(ToolBarEditMenu);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarScriptType);
			WorkspaceSecondaryToolBar->ShowItemToolTips = true;

			ContextMenuCopy->Text = "Copy";
			ContextMenuPaste->Text = "Paste";
			ContextMenuWord->Enabled = false;
			ContextMenuWikiLookup->Text = "Look up on the Wiki";
			ContextMenuOBSEDocLookup->Text = "Look up on the OBSE Doc";
			ContextMenuFind->Text = "Find";
			ContextMenuToggleComment->Text = "Toggle Comment";
			ContextMenuToggleBookmark->Text = "Toggle Bookmark";
			ContextMenuDirectLink->Text = "Developer Page";
			ContextMenuJumpToScript->Text = "Jump into Script";
			ContextMenuAddMessage->Text = "Add Message";
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
			ContextMenuRefactorAddVariableFloat->Text = "Float";
			ContextMenuRefactorAddVariableFloat->Tag = ScriptParser::VariableType::e_Float;
			ContextMenuRefactorAddVariableRef->Text = "Reference";
			ContextMenuRefactorAddVariableRef->Tag = ScriptParser::VariableType::e_Ref;
			ContextMenuRefactorAddVariableString->Text = "String";
			ContextMenuRefactorAddVariableString->Tag = ScriptParser::VariableType::e_String;
			ContextMenuRefactorAddVariableArray->Text = "Array";
			ContextMenuRefactorAddVariableArray->Tag = ScriptParser::VariableType::e_Array;
			ContextMenuRefactorDocumentScript->Text = "Document Script";
			ContextMenuRefactorCreateUDFImplementation->Text = "Create UFD Implementation";
			ContextMenuRefactorRenameVariables->Text = "Rename Variables";

			TextEditorContextMenu->Items->Add(ContextMenuRefactorMenu);
			TextEditorContextMenu->Items->Add(ContextMenuCopy);
			TextEditorContextMenu->Items->Add(ContextMenuPaste);
			TextEditorContextMenu->Items->Add(ContextMenuFind);
			TextEditorContextMenu->Items->Add(ContextMenuToggleComment);
			TextEditorContextMenu->Items->Add(ContextMenuToggleBookmark);
			TextEditorContextMenu->Items->Add(ContextMenuAddMessage);
			TextEditorContextMenu->Items->Add(gcnew ToolStripSeparator());
			TextEditorContextMenu->Items->Add(ContextMenuWord);
			TextEditorContextMenu->Items->Add(ContextMenuWikiLookup);
			TextEditorContextMenu->Items->Add(ContextMenuOBSEDocLookup);
			TextEditorContextMenu->Items->Add(ContextMenuGoogleLookup);
			TextEditorContextMenu->Items->Add(ContextMenuDirectLink);
			TextEditorContextMenu->Items->Add(ContextMenuJumpToScript);

			MessageList->Dock = DockStyle::Fill;
			MessageList->BorderStyle = BorderStyle::Fixed3D;
			MessageList->BackColor = BackColor;
			MessageList->ForeColor = ForeColor;
			MessageList->Visible = false;
			MessageList->View = View::Details;
			MessageList->MultiSelect = false;
			MessageList->CheckBoxes = false;
			MessageList->FullRowSelect = true;
			MessageList->HideSelection = false;
			MessageList->SmallImageList =  gcnew ImageList();
			MessageList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListWarning"));
			MessageList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListError"));
			MessageList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListMessage"));
			MessageList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListWarning"));

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
			MessageList->Tag = (int)1;

			FindList->Dock = DockStyle::Fill;
			FindList->BorderStyle = BorderStyle::Fixed3D;
			FindList->BackColor = BackColor;
			FindList->ForeColor = ForeColor;
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
			FindList->Tag = (int)1;

			BookmarkList->Dock = DockStyle::Fill;
			BookmarkList->BorderStyle = BorderStyle::Fixed3D;
			BookmarkList->BackColor = BackColor;
			BookmarkList->ForeColor = ForeColor;
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

			VariableIndexList->Dock = DockStyle::Fill;
			VariableIndexList->BorderStyle = BorderStyle::Fixed3D;
			VariableIndexList->BackColor = BackColor;
			VariableIndexList->ForeColor = ForeColor;
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

			VariableIndexEditBox->Multiline = true;
			VariableIndexEditBox->BorderStyle = BorderStyle::FixedSingle;
			VariableIndexEditBox->Visible = false;
			VariableIndexEditBox->AcceptsReturn = true;

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

			WorkspaceControlBox->Controls->Add(WorkspaceSplitter);
			WorkspaceControlBox->Controls->Add(WorkspaceMainToolBar);

 			Parent->AddTab(WorkspaceTabItem);
 			Parent->AddTabControlBox(WorkspaceControlBox);

			try { WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height; }
			catch (...) {}

			TextEditor->KeyDown += TextEditorKeyDownHandler;
			TextEditor->ScriptModified += TextEditorScriptModifiedHandler;
			ToolBarEditMenuContentsFind->Click += ToolBarEditMenuContentsFindReplaceClickHandler;
			ToolBarEditMenuContentsReplace->Click += ToolBarEditMenuContentsFindReplaceClickHandler;
			ToolBarEditMenuContentsGotoLine->Click += ToolBarEditMenuContentsGotoLineClickHandler;
			ToolBarEditMenuContentsGotoOffset->Click += ToolBarEditMenuContentsGotoOffsetClickHandler;
			ToolBarScriptTypeContentsObject->Click += ToolBarScriptTypeContentsObjectClickHandler;
			ToolBarScriptTypeContentsQuest->Click += ToolBarScriptTypeContentsQuestClickHandler;
			ToolBarScriptTypeContentsMagicEffect->Click += ToolBarScriptTypeContentsMagicEffectClickHandler;
			ToolBarMessageList->Click += ToolBarMessageListClickHandler;
			ToolBarFindList->Click += ToolBarFindListClickHandler;
			ToolBarBookmarkList->Click += ToolBarBookmarkListClickHandler;
			ToolBarDumpScript->ButtonClick += ToolBarDumpScriptClickHandler;
			ToolBarDumpAllScripts->Click += ToolBarDumpAllScriptsClickHandler;
			ToolBarLoadScript->ButtonClick += ToolBarLoadScriptClickHandler;
			ToolBarLoadScriptsToTabs->Click += ToolBarLoadScriptsToTabsClickHandler;
			ToolBarOptions->Click += ToolBarOptionsClickHandler;
			ToolBarNewScript->Click += ToolBarNewScriptClickHandler;
			ToolBarOpenScript->Click += ToolBarOpenScriptClickHandler;
			ToolBarPreviousScript->Click += ToolBarPreviousScriptClickHandler;
			ToolBarNextScript->Click += ToolBarNextScriptClickHandler;
			ToolBarSaveScript->ButtonClick += ToolBarSaveScriptClickHandler;
			ToolBarSaveScriptNoCompile->Click += ToolBarSaveScriptNoCompileClickHandler;
			ToolBarSaveScriptAndPlugin->Click += ToolBarSaveScriptAndPluginClickHandler;
			ToolBarRecompileScripts->Click += ToolBarRecompileScriptsClickHandler;
			ToolBarDeleteScript->Click += ToolBarDeleteScriptClickHandler;
			ToolBarShowOffsets->Click += ToolBarShowOffsetsClickHandler;
			ToolBarShowPreprocessedText->Click += ToolBarShowPreprocessedTextClickHandler;
			ToolBarSanitizeScriptText->Click += ToolBarSanitizeScriptTextClickHandler;
			ToolBarBindScript->Click += ToolBarBindScriptClickHandler;
			ToolBarNavigationBack->Click += ToolBarNavigationBackClickHandler;
			ToolBarNavigationForward->Click += ToolBarNavigationForwardClickHandler;
			ToolBarGetVarIndices->Click += ToolBarGetVarIndicesClickHandler;
			ToolBarUpdateVarIndices->Click += ToolBarUpdateVarIndicesClickHandler;
			ToolBarSaveAll->Click += ToolBarSaveAllClickHandler;
			ToolBarCompileDependencies->Click += ToolBarCompileDependenciesClickHandler;
			ContextMenuCopy->Click += ContextMenuCopyClickHandler;
			ContextMenuPaste->Click += ContextMenuPasteClickHandler;
			ContextMenuWikiLookup->Click += ContextMenuWikiLookupClickHandler;
			ContextMenuOBSEDocLookup->Click += ContextMenuOBSEDocLookupClickHandler;
			ContextMenuFind->Click += ContextMenuFindClickHandler;
			ContextMenuToggleComment->Click += ContextMenuToggleCommentClickHandler;
			ContextMenuToggleBookmark->Click += ContextMenuToggleBookmarkClickHandler;
			ContextMenuDirectLink->Click += ContextMenuDirectLinkClickHandler;
			ContextMenuJumpToScript->Click += ContextMenuJumpToScriptClickHandler;
			ContextMenuAddMessage->Click += ContextMenuAddMessageClickHandler;
			ContextMenuGoogleLookup->Click += ContextMenuGoogleLookupClickHandler;
			ContextMenuRefactorAddVariableInt->Click += ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableFloat->Click += ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableRef->Click += ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableString->Click += ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableArray->Click += ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorDocumentScript->Click += ContextMenuRefactorDocumentScriptClickHandler;
			ContextMenuRefactorCreateUDFImplementation->Click += ContextMenuRefactorCreateUDFImplementationClickHandler;
			ContextMenuRefactorRenameVariables->Click += ContextMenuRefactorRenameVariablesClickHandler;
			TextEditorContextMenu->Opening += TextEditorContextMenuOpeningHandler;
			MessageList->DoubleClick += MessageListDoubleClickHandler;
			MessageList->ColumnClick += MessageListColumnClickHandler;
			FindList->DoubleClick += FindListDoubleClickHandler;
			FindList->ColumnClick += FindListColumnClickHandler;
			BookmarkList->DoubleClick += BookmarkListDoubleClickHandler;
			BookmarkList->ColumnClick += BookmarkListColumnClickHandler;
			VariableIndexList->DoubleClick += VariableIndexListDoubleClickHandler;
			VariableIndexList->ColumnClick += VariableIndexListColumnClickHandler;
			VariableIndexEditBox->LostFocus += VariableIndexEditBoxLostFocusHandler;
			VariableIndexEditBox->KeyDown += VariableIndexEditBoxKeyDownHandler;
			PREFERENCES->PreferencesSaved += ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick += AutoSaveTimerTickHandler;

			AutoSaveTimer->Start();
			DisableControls();
			ToolBarUpdateVarIndices->Enabled = false;
			TextEditor->SetContextMenu(TextEditorContextMenu);

			SetScriptType(ScriptType::e_Object);
			SetModifiedStatus(false);

			if (InitScript && InitScript->ParentForm)
				UpdateEnvironment(InitScript, true);
			else
				CurrentScript = 0;

			NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(InitScript, false);
		}

		Workspace::~Workspace()
		{
			Workspace::Destroy();
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
		void Workspace::ClearErrorMessagesFromMessagePool(void)
		{
			LinkedList<ListViewItem^>^ InvalidItems = gcnew LinkedList<ListViewItem^>();

			for each (ListViewItem^ Itr in MessageList->Items)
			{
				if (Itr->ImageIndex < (int)MessageListItemType::e_RegularMessage)
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
		int Workspace::PerformFindReplace(TextEditors::IScriptTextEditor::FindReplaceOperation Operation, String^ Query, String^ Replacement, UInt32 Options)
		{
			FindList->Items->Clear();
			int Hits = TextEditor->FindReplace(Operation,
				Query,
				Replacement,
				gcnew TextEditors::IScriptTextEditor::FindReplaceOutput(this, &ScriptEditor::Workspace::FindReplaceOutput), Options);

			if (Hits > 0 && FindList->Visible == false)
				ToolBarFindList->PerformClick();

			return Hits;
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
			InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter A Description For The Bookmark", "Place Bookmark");
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

		void Workspace::InsertVariable( String^ VariableName, ScriptParser::VariableType VariableType )
		{
			String^ ScriptText = TextEditor->GetText()->Replace("\r", "");
			ScriptParser^ TextParser = gcnew ScriptParser();
			StringReader^ TextReader = gcnew StringReader(ScriptText);
			int LastVarOffset = 0, InsertOffset = 0;

			for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
			{
				TextParser->Tokenize(ReadLine, false);

				if (!TextParser->Valid)
				{
					InsertOffset += ReadLine->Length + 1;
					continue;
				}

				bool ExitLoop = false, SaveOffset = false;
				switch (TextParser->GetTokenType(TextParser->Tokens[0]))
				{
				case ScriptParser::TokenType::e_Variable:
					SaveOffset = true;
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

				InsertOffset += ReadLine->Length + 1;
				if (SaveOffset)
					LastVarOffset = InsertOffset;
			}

			if (LastVarOffset)
				InsertOffset = LastVarOffset;

			String^ VarText = "";
			if (InsertOffset > ScriptText->Length)
				VarText += "\n";

			switch (VariableType)
			{
			case ScriptParser::VariableType::e_Integer:
				VarText += "int " + VariableName;
				break;
			case ScriptParser::VariableType::e_Float:
				VarText += "float " + VariableName;
				break;
			case ScriptParser::VariableType::e_Ref:
				VarText += "ref " + VariableName;
				break;
			case ScriptParser::VariableType::e_String:
				VarText += "string_var " + VariableName;
				break;
			case ScriptParser::VariableType::e_Array:
				VarText += "array_var " + VariableName;
				break;
			}
			VarText += "\n";

			TextEditor->InsertText(VarText, InsertOffset, true);
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
			if (PREFERENCES->FetchSettingAsInt("SaveLastKnownPos", "General"))
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
				if (Itr->ImageIndex > (int)MessageListItemType::e_Error)
					switch ((MessageListItemType)Itr->ImageIndex)
				{
					case MessageListItemType::e_EditorMessage:
						Result += ";<CSEMessageEditor> " + Itr->SubItems[2]->Text + " </CSEMessageEditor>\n";
						break;
					case MessageListItemType::e_RegularMessage:
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

			if (CaretPos >= TextEditor->GetTextLength())
				CaretPos = TextEditor->GetTextLength() - 1;
			else if (CaretPos < 0)
				CaretPos = 0;

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
				if (!TextParser->Valid)
				{
					ReadLine = StringParser->ReadLine();
					continue;
				}

				String^ Message = "";
				if (!TextParser->GetTokenIndex(";<CSEMessageEditor>"))
				{
					Message = ReadLine->Substring(TextParser->Indices[1])->Replace(" </CSEMessageEditor>", "");
					AddMessageToMessagePool(MessageListItemType::e_EditorMessage, -1, Message);
				}
				else if (!TextParser->GetTokenIndex(";<CSEMessageRegular>"))
				{
					Message = ReadLine->Substring(TextParser->Indices[1])->Replace(" </CSEMessageRegular>", "");
					AddMessageToMessagePool(MessageListItemType::e_RegularMessage, -1, Message);
				}

				ReadLine = StringParser->ReadLine();
			}
		}
		void Workspace::PreprocessorErrorOutputWrapper(String^ Message)
		{
			AddMessageToMessagePool(MessageListItemType::e_Error, -1, Message);
		}
		String^ Workspace::SanitizeScriptText(SanitizeOperation Operation, String^ ScriptText)
		{
			String^ SanitizedScriptText = "";

			StringReader^ TextReader = gcnew StringReader(ScriptText);
			ScriptParser^ LocalParser = gcnew ScriptParser();

			int IndentCount = 0;
			for (String^ ReadLine = TextReader->ReadLine(); ReadLine != nullptr; ReadLine = TextReader->ReadLine())
			{
				switch (Operation)
				{
				case SanitizeOperation::e_Indent:
					{
						int IndentMode = -1;
						// 0 = Decrement
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

								if (LocalParser->GetCommentTokenIndex(i) == -1)
								{
									IntelliSenseItemVariable^ LocalVar = TextEditor->GetIntelliSenseInterface()->LookupLocalVariableByIdentifier(Token);
									if (LocalVar != nullptr)
										SanitizedScriptText += LocalVar->GetIdentifier();
									else
										SanitizedScriptText += ISDB->SanitizeIdentifier(Token);
								}
								else
									SanitizedScriptText += Token;

								SanitizedScriptText += Delimiter;
							}
						}
						else
							SanitizedScriptText += ReadLine + "\n";

						break;
					}
				case SanitizeOperation::e_EvalifyIfs:
					{
						LocalParser->Tokenize(ReadLine, true);
						if (LocalParser->Valid)
						{
							for (int i = 0; i < LocalParser->GetCurrentTokenCount(); i++)
							{
								String^ Token = LocalParser->Tokens[i];
								String^ Delimiter = "" + LocalParser->Delimiters[i];

								if (i == 0 &&
									LocalParser->GetCommentTokenIndex(i) == -1 &&
									(LocalParser->GetTokenType(Token) == ScriptParser::TokenType::e_ElseIf ||
									LocalParser->GetTokenType(Token) == ScriptParser::TokenType::e_If) &&
									LocalParser->GetCurrentTokenCount() > 1 &&
									String::Compare(LocalParser->Tokens[i + 1], "eval", true))
								{
									SanitizedScriptText += Token + " eval";
								}
								else
									SanitizedScriptText += Token;

								SanitizedScriptText += Delimiter;
							}
						}
						else
							SanitizedScriptText += ReadLine + "\n";

						break;
					}
				case SanitizeOperation::e_CompilerOverrideBlocks:
					{
						LocalParser->Tokenize(ReadLine, true);
						if (LocalParser->Valid)
						{
							for (int i = 0; i < LocalParser->GetCurrentTokenCount(); i++)
							{
								String^ Token = LocalParser->Tokens[i];
								String^ Delimiter = "" + LocalParser->Delimiters[i];

								if (i == 0 &&
									LocalParser->GetCommentTokenIndex(i) == -1 &&
									LocalParser->GetTokenType(Token) == ScriptParser::TokenType::e_Begin &&
									LocalParser->GetCurrentTokenCount() > 1 &&
									LocalParser->Tokens[i + 1]->Length > 0 &&
									LocalParser->Tokens[i + 1][0] != '_')
								{
									SanitizedScriptText += Token + " _";
									continue;
								}
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

			return SanitizedScriptText;
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

				if (MessageList->Visible)
				{
					MessageList->Items->Clear();
					ToolBarMessageList->PerformClick();
				}

				if (FindList->Visible)
				{
					FindList->Items->Clear();
					ToolBarFindList->PerformClick();
				}

				if (VariableIndexList->Visible)
				{
					VariableIndexList->Items->Clear();
					ToolBarGetVarIndices->PerformClick();
				}

				if (BookmarkList->Visible)
				{
					BookmarkList->Items->Clear();
					ToolBarBookmarkList->PerformClick();
				}

				String^ CSEBlock = "";
				String^ DeserializedText = DeserializeCSEBlock(ScriptText, CSEBlock);
				TextEditor->SetText(DeserializedText, false, true);

				DeserializeCaretPos(CSEBlock);
				DeserializeBookmarks(CSEBlock);
				DeserializeMessages(CSEBlock);

				EnableControls();
			}

			CurrentScriptEditorID = ScriptName;
			WorkspaceTabItem->Tooltip = ScriptName + " [" + FormID.ToString("X8") + "]";
			WorkspaceTabItem->Text = " " + ScriptName;
			ParentContainer->SetWindowTitle(GetScriptDescription() + " - " + SCRIPTEDITOR_TITLE);
			SetScriptType((Workspace::ScriptType)ScriptType);

			SetModifiedStatus(false);

			ToolBarByteCodeSize->Value = ByteCodeLength;
			ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(ByteCodeLength / 1024.0));

			String^ PreprocessedText = "";
			if (PreprocessScriptText(PreprocessedText))
				OffsetViewer->InitializeViewer(PreprocessedText, ByteCode, ByteCodeLength);
			else
				OffsetViewer->Reset();

			TextEditor->UpdateIntelliSenseLocalDatabase();

			if (PREFERENCES->FetchSettingAsInt("UseAutoRecovery", "Backup") && Initializing)
			{
				String^ CachePath = gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + GetScriptDescription() + ".txt";
				if (System::IO::File::Exists(CachePath))
				{
					try
					{
						System::DateTime LastWriteTime = System::IO::File::GetLastWriteTime(CachePath);
						if (MessageBox::Show("An auto-recovery cache for the script '" + GetScriptDescription() + "' was found, dated " + LastWriteTime.ToShortDateString() +  " " + LastWriteTime.ToLongTimeString() + ".\n\nWould you like to load it instead?",
											SCRIPTEDITOR_TITLE,
											MessageBoxButtons::YesNo,
											MessageBoxIcon::Information) == DialogResult::Yes)
						{
							LoadFileFromDisk(CachePath);
						}

						System::IO::File::Delete(CachePath);
					}
					catch (Exception^ E)
					{
						DebugPrint("Couldn't access auto-recovery cache '" + GetScriptDescription() + "'!\n\tException: " + E->Message, true);
					}
				}
			}
		}
		void Workspace::LoadFileFromDisk(String^ Path)
		{
			TextEditor->LoadFileFromDisk(Path);
		}
		void Workspace::SaveScriptToDisk(String^ Path, bool PathIncludesFileName)
		{
			TextEditor->SaveScriptToDisk(Path, PathIncludesFileName, GetScriptDescription());
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
			ClearErrorMessagesFromMessagePool();

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
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Identifier '" + SecondToken + "' contains an invalid character."), Result = false;
					if (ScriptName == "")
						ScriptName = SecondToken;
					else
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Redeclaration of script name."), Result = false;
					break;
				case ScriptParser::TokenType::e_Variable:
					if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_Invalid)
					{
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Variable '" + SecondToken + "' declared inside a script block.");
						Result = false;
					}
					if (ScriptTextParser->LookupVariableByName(SecondToken) != nullptr)
						AddMessageToMessagePool(MessageListItemType::e_Warning, CurrentLineNo, "Redeclaration of variable '" + SecondToken + "'."), Result = false;
					else
						ScriptTextParser->Variables->AddLast(gcnew ScriptParser::VariableRefCountData(SecondToken, 0));
					break;
				case ScriptParser::TokenType::e_Begin:
					if (!ScriptTextParser->GetIsBlockValidForScriptType(SecondToken, (ScriptParser::ScriptType)ScriptType))
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid script block '" + SecondToken + "' for script type."), Result = false;
					ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_ScriptBlock);
					break;
				case ScriptParser::TokenType::e_End:
					if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_ScriptBlock)
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid block structure. Command 'End' has no matching 'Begin'."), Result = false;
					else
						ScriptTextParser->BlockStack->Pop();
					if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
						AddMessageToMessagePool(MessageListItemType::e_Warning, CurrentLineNo, "Command 'End' has an otiose expression following it.");
					break;
				case ScriptParser::TokenType::e_While:
					ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
					break;
				case ScriptParser::TokenType::e_ForEach:
					ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
					break;
				case ScriptParser::TokenType::e_Loop:
					if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_Loop)
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid block structure. Command 'Loop' has no matching 'While' or 'ForEach'."), Result = false;
					else
						ScriptTextParser->BlockStack->Pop();
					break;
				case ScriptParser::TokenType::e_If:
					if (ScriptTextParser->GetCurrentTokenCount() < 2 || ScriptTextParser->Tokens[1][0] == ';')
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid condition."), Result = false;

					ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_If);
					break;
				case ScriptParser::TokenType::e_ElseIf:
					if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid block structure. Command 'ElseIf' has no matching 'If'."), Result = false;
					else if (ScriptTextParser->GetCurrentTokenCount() < 2 || ScriptTextParser->Tokens[1][0] == ';')
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid condition."), Result = false;
					break;
				case ScriptParser::TokenType::e_Else:
					if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid block structure. Command 'Else' has no matching 'If'."), Result = false;
					if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
						AddMessageToMessagePool(MessageListItemType::e_Warning, CurrentLineNo, "Command 'Else' has an otiose expression following it.");
					break;
				case ScriptParser::TokenType::e_EndIf:
					if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
						AddMessageToMessagePool(MessageListItemType::e_Error, CurrentLineNo, "Invalid block structure. Command 'EndIf' has no matching 'If'."), Result = false;
					else
						ScriptTextParser->BlockStack->Pop();
					if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
						AddMessageToMessagePool(MessageListItemType::e_Warning, CurrentLineNo, "Command 'EndIf' has an otiose expression following it.");
					break;
				case ScriptParser::TokenType::e_Return:
					if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
						AddMessageToMessagePool(MessageListItemType::e_Warning, CurrentLineNo, "Command 'Return' has an otiose expression following it.");
					break;
				}

				// increment variable ref count
				UInt32 Pos = 0;
				if (ScriptTextParser->GetTokenType(FirstToken) != ScriptParser::TokenType::e_Variable)
				{
					for each (String^ Itr in ScriptTextParser->Tokens)
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

			for each (ScriptParser::VariableRefCountData^ Itr in ScriptTextParser->Variables)
			{
				if (Itr->RefCount == 0)
				{
					if ((ScriptParser::ScriptType)ScriptType != ScriptParser::ScriptType::e_Quest ||
						PREFERENCES->FetchSettingAsInt("SuppressRefCountForQuestScripts", "General") == 0)
					{
						AddMessageToMessagePool(MessageListItemType::e_Warning, 1, "Variable '" + Itr->Name + "' unreferenced in local context.");
					}
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
					AddMessageToMessagePool(MessageListItemType::e_Error, 1, "Variable '" + Itr->Name + "' has an all-numeric identifier.");
					Result = false;
				}
			}

			if (Result == false)
				AddMessageToMessagePool(MessageListItemType::e_Warning, -1, "Compilation of script '" + ScriptName + "' halted - Couldn't recover from previous errors.");

			ComponentDLLInterface::FormData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupFormByEditorID((CString(ScriptName)).c_str());
			if (Data && String::Compare(CurrentScriptEditorID, ScriptName, true) != 0)
			{
				if (MessageBox::Show("Script name '" + ScriptName + "' is already used by another form. Proceeding with compilation will modify the script's editorID.\n\nDo you want to proceed?",
									SCRIPTEDITOR_TITLE,
									MessageBoxButtons::YesNo,
									MessageBoxIcon::Exclamation) == DialogResult::No)
				{
					Result = false;
				}
			}
			NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);

			if (MessageList->Items->Count && MessageList->Visible == false)
				ToolBarMessageList->PerformClick();

			return Result;
		}
		void Workspace::Destroy()
		{
			DestructionFlag = true;
			AutoSaveTimer->Stop();

			TextEditor->KeyDown -= TextEditorKeyDownHandler;
			TextEditor->ScriptModified -= TextEditorScriptModifiedHandler;
			ToolBarEditMenuContentsFind->Click -= ToolBarEditMenuContentsFindReplaceClickHandler;
			ToolBarEditMenuContentsReplace->Click -= ToolBarEditMenuContentsFindReplaceClickHandler;
			ToolBarEditMenuContentsGotoLine->Click -= ToolBarEditMenuContentsGotoLineClickHandler;
			ToolBarEditMenuContentsGotoOffset->Click -= ToolBarEditMenuContentsGotoOffsetClickHandler;
			ToolBarScriptTypeContentsObject->Click -= ToolBarScriptTypeContentsObjectClickHandler;
			ToolBarScriptTypeContentsQuest->Click -= ToolBarScriptTypeContentsQuestClickHandler;
			ToolBarScriptTypeContentsMagicEffect->Click -= ToolBarScriptTypeContentsMagicEffectClickHandler;
			ToolBarMessageList->Click -= ToolBarMessageListClickHandler;
			ToolBarFindList->Click -= ToolBarFindListClickHandler;
			ToolBarBookmarkList->Click -= ToolBarBookmarkListClickHandler;
			ToolBarDumpScript->ButtonClick -= ToolBarDumpScriptClickHandler;
			ToolBarDumpAllScripts->Click -= ToolBarDumpAllScriptsClickHandler;
			ToolBarLoadScript->ButtonClick -= ToolBarLoadScriptClickHandler;
			ToolBarLoadScriptsToTabs->Click -= ToolBarLoadScriptsToTabsClickHandler;
			ToolBarOptions->Click -= ToolBarOptionsClickHandler;
			ToolBarNewScript->Click -= ToolBarNewScriptClickHandler;
			ToolBarOpenScript->Click -= ToolBarOpenScriptClickHandler;
			ToolBarPreviousScript->Click -= ToolBarPreviousScriptClickHandler;
			ToolBarNextScript->Click -= ToolBarNextScriptClickHandler;
			ToolBarSaveScript->ButtonClick -= ToolBarSaveScriptClickHandler;
			ToolBarSaveScriptNoCompile->Click -= ToolBarSaveScriptNoCompileClickHandler;
			ToolBarSaveScriptAndPlugin->Click -= ToolBarSaveScriptAndPluginClickHandler;
			ToolBarRecompileScripts->Click -= ToolBarRecompileScriptsClickHandler;
			ToolBarDeleteScript->Click -= ToolBarDeleteScriptClickHandler;
			ToolBarShowOffsets->Click -= ToolBarShowOffsetsClickHandler;
			ToolBarShowPreprocessedText->Click -= ToolBarShowPreprocessedTextClickHandler;
			ToolBarSanitizeScriptText->Click -= ToolBarSanitizeScriptTextClickHandler;
			ToolBarBindScript->Click -= ToolBarBindScriptClickHandler;
			ToolBarNavigationBack->Click -= ToolBarNavigationBackClickHandler;
			ToolBarNavigationForward->Click -= ToolBarNavigationForwardClickHandler;
			ToolBarGetVarIndices->Click -= ToolBarGetVarIndicesClickHandler;
			ToolBarUpdateVarIndices->Click -= ToolBarUpdateVarIndicesClickHandler;
			ToolBarSaveAll->Click -= ToolBarSaveAllClickHandler;
			ToolBarCompileDependencies->Click -= ToolBarCompileDependenciesClickHandler;
			ContextMenuCopy->Click -= ContextMenuCopyClickHandler;
			ContextMenuPaste->Click -= ContextMenuPasteClickHandler;
			ContextMenuWikiLookup->Click -= ContextMenuWikiLookupClickHandler;
			ContextMenuOBSEDocLookup->Click -= ContextMenuOBSEDocLookupClickHandler;
			ContextMenuFind->Click -= ContextMenuFindClickHandler;
			ContextMenuToggleComment->Click -= ContextMenuToggleCommentClickHandler;
			ContextMenuToggleBookmark->Click -= ContextMenuToggleBookmarkClickHandler;
			ContextMenuDirectLink->Click -= ContextMenuDirectLinkClickHandler;
			ContextMenuJumpToScript->Click -= ContextMenuJumpToScriptClickHandler;
			ContextMenuAddMessage->Click -= ContextMenuAddMessageClickHandler;
			ContextMenuGoogleLookup->Click -= ContextMenuGoogleLookupClickHandler;
			ContextMenuRefactorAddVariableInt->Click -= ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableFloat->Click -= ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableRef->Click -= ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableString->Click -= ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorAddVariableArray->Click -= ContextMenuRefactorAddVariableClickHandler;
			ContextMenuRefactorDocumentScript->Click -= ContextMenuRefactorDocumentScriptClickHandler;
			ContextMenuRefactorCreateUDFImplementation->Click -= ContextMenuRefactorDocumentScriptClickHandler;
			ContextMenuRefactorRenameVariables->Click -= ContextMenuRefactorRenameVariablesClickHandler;
			TextEditorContextMenu->Opening -= TextEditorContextMenuOpeningHandler;
			MessageList->DoubleClick -= MessageListDoubleClickHandler;
			MessageList->ColumnClick -= MessageListColumnClickHandler;
			FindList->DoubleClick -= FindListDoubleClickHandler;
			FindList->ColumnClick -= FindListColumnClickHandler;
			BookmarkList->DoubleClick -= BookmarkListDoubleClickHandler;
			BookmarkList->ColumnClick -= BookmarkListColumnClickHandler;
			VariableIndexList->DoubleClick -= VariableIndexListDoubleClickHandler;
			VariableIndexList->ColumnClick -= VariableIndexListColumnClickHandler;
			VariableIndexEditBox->LostFocus -= VariableIndexEditBoxLostFocusHandler;
			VariableIndexEditBox->KeyDown -= VariableIndexEditBoxKeyDownHandler;
			PREFERENCES->PreferencesSaved -= ScriptEditorPreferencesSavedHandler;
			AutoSaveTimer->Tick -= AutoSaveTimerTickHandler;

			for each (Image^ Itr in MessageList->SmallImageList->Images)
				delete Itr;

			MessageList->SmallImageList->Images->Clear();
			MessageList->SmallImageList =  nullptr;
			WorkspaceTabItem->Tag = nullptr;

			delete TextEditor;
			delete OffsetViewer;
			delete PreprocessedTextViewer;
			delete ScriptListBox;

			ParentContainer->RemoveTab(WorkspaceTabItem);
			ParentContainer->RemoveTabControlBox(WorkspaceControlBox);

			WorkspaceControlBox->Controls->Clear();
			WorkspaceSplitter->Panel1->Controls->Clear();
			WorkspaceSplitter->Panel2->Controls->Clear();
			VariableIndexList->Controls->Clear();

			DisposeControlImage(ToolBarNewScript);
			DisposeControlImage(ToolBarOpenScript);
			DisposeControlImage(ToolBarPreviousScript);
			DisposeControlImage(ToolBarNextScript);
			DisposeControlImage(ToolBarSaveScript);
			DisposeControlImage(ToolBarSaveScriptNoCompile);
			DisposeControlImage(ToolBarSaveScriptAndPlugin);
			DisposeControlImage(ToolBarRecompileScripts);
			DisposeControlImage(ToolBarCompileDependencies);
			DisposeControlImage(ToolBarDeleteScript);
			DisposeControlImage(ToolBarNavigationBack);
			DisposeControlImage(ToolBarNavigationForward);
			DisposeControlImage(ToolBarSaveAll);
			DisposeControlImage(ToolBarOptions);

			DisposeControlImage(ToolBarEditMenu);
			DisposeControlImage(ToolBarEditMenuContentsFind);
			DisposeControlImage(ToolBarEditMenuContentsReplace);
			DisposeControlImage(ToolBarEditMenuContentsGotoLine);
			DisposeControlImage(ToolBarEditMenuContentsGotoOffset);

			DisposeControlImage(ToolBarMessageList);
			DisposeControlImage(ToolBarFindList);
			DisposeControlImage(ToolBarBookmarkList);
			DisposeControlImage(ToolBarDumpScript);
			DisposeControlImage(ToolBarDumpAllScripts);
			DisposeControlImage(ToolBarLoadScript);
			DisposeControlImage(ToolBarLoadScriptsToTabs);
			DisposeControlImage(ToolBarGetVarIndices);
			DisposeControlImage(ToolBarUpdateVarIndices);
			DisposeControlImage(ToolBarShowOffsets);
			DisposeControlImage(ToolBarShowPreprocessedText);
			DisposeControlImage(ToolBarSanitizeScriptText);
			DisposeControlImage(ToolBarBindScript);

			DisposeControlImage(ToolBarScriptTypeContentsObject);
			DisposeControlImage(ToolBarScriptTypeContentsQuest);
			DisposeControlImage(ToolBarScriptTypeContentsMagicEffect);

			DisposeControlImage(ContextMenuCopy);
			DisposeControlImage(ContextMenuPaste);
			DisposeControlImage(ContextMenuFind);
			DisposeControlImage(ContextMenuToggleComment);
			DisposeControlImage(ContextMenuToggleBookmark);
			DisposeControlImage(ContextMenuAddMessage);
			DisposeControlImage(ContextMenuWikiLookup);
			DisposeControlImage(ContextMenuOBSEDocLookup);
			DisposeControlImage(ContextMenuDirectLink);
			DisposeControlImage(ContextMenuJumpToScript);
			DisposeControlImage(ContextMenuGoogleLookup);
			DisposeControlImage(ContextMenuRefactorMenu);
			DisposeControlImage(ContextMenuRefactorAddVariable);
			DisposeControlImage(ContextMenuRefactorDocumentScript);
			DisposeControlImage(ContextMenuRefactorCreateUDFImplementation);
			DisposeControlImage(ContextMenuRefactorRenameVariables);

			delete WorkspaceTabItem;
			delete WorkspaceControlBox;
			delete WorkspaceSplitter;
			delete MessageList;
			delete FindList;
			delete BookmarkList;
			delete VariableIndexList;
			delete VariableIndexEditBox;
			delete SpoilerText;

			delete WorkspaceMainToolBar;
			delete ToolBarNewScript;
			delete ToolBarOpenScript;
			delete ToolBarPreviousScript;
			delete ToolBarNextScript;
			delete ToolBarSaveScript;
			delete ToolBarSaveScriptDropDown;
			delete ToolBarSaveScriptNoCompile;
			delete ToolBarSaveScriptAndPlugin;
			delete ToolBarRecompileScripts;
			delete ToolBarCompileDependencies;
			delete ToolBarDeleteScript;
			delete ToolBarNavigationBack;
			delete ToolBarNavigationForward;
			delete ToolBarSaveAll;
			delete ToolBarOptions;

			delete ToolBarScriptType;
			delete ToolBarScriptTypeContents;
			delete ToolBarScriptTypeContentsObject;
			delete ToolBarScriptTypeContentsQuest;
			delete ToolBarScriptTypeContentsMagicEffect;

			delete WorkspaceSecondaryToolBar;
			delete ToolBarEditMenu;
			delete ToolBarEditMenuContents;
			delete ToolBarEditMenuContentsFind;
			delete ToolBarEditMenuContentsReplace;
			delete ToolBarEditMenuContentsGotoLine;
			delete ToolBarEditMenuContentsGotoOffset;
			delete ToolBarMessageList;
			delete ToolBarFindList;
			delete ToolBarBookmarkList;
			delete ToolBarDumpScript;
			delete ToolBarDumpScriptDropDown;
			delete ToolBarDumpAllScripts;
			delete ToolBarLoadScript;
			delete ToolBarLoadScriptDropDown;
			delete ToolBarLoadScriptsToTabs;
			delete ToolBarGetVarIndices;
			delete ToolBarUpdateVarIndices;
			delete ToolBarShowOffsets;
			delete ToolBarShowPreprocessedText;
			delete ToolBarSanitizeScriptText;
			delete ToolBarBindScript;
			delete ToolBarByteCodeSize;

			delete TextEditorContextMenu;
			delete ContextMenuCopy;
			delete ContextMenuPaste;
			delete ContextMenuFind;
			delete ContextMenuToggleComment;
			delete ContextMenuToggleBookmark;
			delete ContextMenuAddMessage;
			delete ContextMenuWord;
			delete ContextMenuWikiLookup;
			delete ContextMenuOBSEDocLookup;
			delete ContextMenuDirectLink;
			delete ContextMenuJumpToScript;
			delete ContextMenuGoogleLookup;
			delete ContextMenuRefactorMenu;
			delete ContextMenuRefactorAddVariable;
			delete ContextMenuRefactorAddVariableInt;
			delete ContextMenuRefactorAddVariableFloat;
			delete ContextMenuRefactorAddVariableRef;
			delete ContextMenuRefactorAddVariableString;
			delete ContextMenuRefactorAddVariableArray;
			delete ContextMenuRefactorDocumentScript;
			delete ContextMenuRefactorCreateUDFImplementation;
			delete ContextMenuRefactorRenameVariables;
			delete AutoSaveTimer;

			ParentContainer->Redraw();
			ParentContainer = nullptr;
			ScriptListBox = nullptr;
			TextEditor = nullptr;
			OffsetViewer = nullptr;
			PreprocessedTextViewer = nullptr;
		}
		Point Workspace::GetCaretLocation( bool AbsoluteValue )
		{
			if (AbsoluteValue == false)
				return TextEditor->GetPositionFromCharIndex(TextEditor->GetCaretPos());
			else
				return TextEditor->GetAbsolutePositionFromCharIndex(TextEditor->GetCaretPos());
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
								gcnew ScriptEditorPreprocessorData(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
																gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
																PREFERENCES->FetchSettingAsInt("AllowRedefinitions", "Preprocessor"),
																PREFERENCES->FetchSettingAsInt("NoOfPasses", "Preprocessor")));
			return Result;
		}
		void Workspace::AddMessageToMessagePool(MessageListItemType Type, int Line, String^ Message)
		{
			ListViewItem^ Item = gcnew ListViewItem(" ", (int)Type);
			if (Line != -1)
				Item->SubItems->Add(Line.ToString());
			else
				Item->SubItems->Add("");
			Item->SubItems->Add(Message);
			if (Type == MessageListItemType::e_RegularMessage)
				Item->ToolTipText = "Double click to remove message";

			MessageList->Items->Add(Item);
			if (MessageList->Visible == false)
				ToolBarMessageList->PerformClick();

			if (Type == MessageListItemType::e_Error)
				TextEditor->HighlightScriptError(Line);
		}
		void Workspace::ClearEditorMessagesFromMessagePool(void)
		{
			LinkedList<ListViewItem^>^ InvalidItems = gcnew LinkedList<ListViewItem^>();

			for each (ListViewItem^ Itr in MessageList->Items)
			{
				if (Itr->ImageIndex == (int)MessageListItemType::e_EditorMessage)
					InvalidItems->AddLast(Itr);
			}

			for each (ListViewItem^ Itr in InvalidItems)
				MessageList->Items->Remove(Itr);
		}
		void Workspace::Relocate(WorkspaceContainer^ Destination)
		{
			WorkspaceControlBox->TabItem = nullptr;		// reset to prevent disposal
			WorkspaceTabItem->AttachedControl = nullptr;

			ParentContainer->FlagDestruction(true);
			ParentContainer->RemoveTabControlBox(WorkspaceControlBox);
			ParentContainer->FlagDestruction(false);

			ParentContainer = Destination;
			WorkspaceControlBox->TabItem = WorkspaceTabItem;
			WorkspaceTabItem->AttachedControl = WorkspaceControlBox;

			Destination->AddTab(WorkspaceTabItem);
			Destination->AddTabControlBox(WorkspaceControlBox);
		}
		void Workspace::HandleFocus( bool GotFocus )
		{
			if (GotFocus)
			{
				TextEditor->OnGotFocus();
			}
			else
			{
				FindReplaceBox->Hide();
				TextEditor->OnLostFocus();
			}
		}
		bool Workspace::PerformHouseKeeping()
		{
			if (GetModifiedStatus())
			{
				DialogResult Result = MessageBox::Show("The current script '" + CurrentScriptEditorID + "' has unsaved changes. Do you wish to save them ?",
													SCRIPTEDITOR_TITLE,
													MessageBoxButtons::YesNoCancel,
													MessageBoxIcon::Exclamation);

				if (Result == DialogResult::Yes)
					return SaveScript(ScriptSaveOperation::e_SaveAndCompile);
				else if (Result == DialogResult::No)
				{
					if (NewScriptFlag)
					{
						NativeWrapper::g_CSEInterfaceTable->ScriptEditor.DestroyScriptInstance(CurrentScript);
						CurrentScript = 0;
					}

					try			// delete the script's autorecovery cache, if any
					{
						System::IO::File::Delete(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + GetScriptDescription() + ".txt");
					}
					catch (...) {}

					return true;
				}
				else
					return false;
			}

			return true;
		}
		void Workspace::NewScript()
		{
			if (PerformHouseKeeping())
			{
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CreateNewScript();
				UpdateEnvironment(Data, true);
				NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);

				NewScriptFlag = true;
				SetModifiedStatus(true);
			}
		}
		void Workspace::OpenScript()
		{
			ComponentDLLInterface::ScriptData* Data = ScriptListBox->Show(ScriptListDialog::Operation::e_Open,
																		(GetIsUninitialized() ? "" : CurrentScriptEditorID));
			if (Data && PerformHouseKeeping())
			{
				UpdateEnvironment(Data, true);
			}
			NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);
		}
		bool Workspace::SaveScript(ScriptSaveOperation Operation)
		{
			bool Result = false;
			String^ PreprocessedScriptResult = "";

			if (PreprocessScriptText(PreprocessedScriptResult) && ValidateScript(PreprocessedScriptResult))
			{
				if (CurrentScript)
				{
					ClearEditorMessagesFromMessagePool();

					if (Operation == ScriptSaveOperation::e_SaveButDontCompile)
					{
						NativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(false);
						AddMessageToMessagePool(MessageListItemType::e_EditorMessage,
												-1,
												"This is an uncompiled script. Expect weird behavior during runtime execution.");
					}

					ComponentDLLInterface::ScriptCompileData CompileData;

					CString ScriptText(PreprocessedScriptResult->Replace("\n", "\r\n"));
					CompileData.Script.Text = ScriptText.c_str();
					CompileData.Script.Type = (int)GetScriptType();
					CompileData.Script.ParentForm = (TESForm*)CurrentScript;

					if (NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(&CompileData))
					{
						UpdateEnvironment(&CompileData.Script, false);

						String^ OriginalText = GetScriptText() + SerializeCSEBlock();
						CString OrgScriptText(OriginalText);
						NativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(CurrentScript, OrgScriptText.c_str());
						Result = true;
					}
					else
					{
						for (int i = 0; i < CompileData.CompileErrorData.Count; i++)
						{
							AddMessageToMessagePool(MessageListItemType::e_Error,
								CompileData.CompileErrorData.ErrorListHead[i].Line,
								gcnew String(CompileData.CompileErrorData.ErrorListHead[i].Message));
						}

						NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(CompileData.CompileErrorData.ErrorListHead, true);
					}

					if (Operation == ScriptSaveOperation::e_SaveButDontCompile)
						NativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(true);
					else if (Operation == ScriptSaveOperation::e_SaveActivePluginToo)
						NativeWrapper::g_CSEInterfaceTable->EditorAPI.SaveActivePlugin();
				}
				else
					Result = true;
			}

			if (Result)
			{
				try			// delete the script's autorecovery cache, if any
				{
					System::IO::File::Delete(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()) + GetScriptDescription() + ".txt");
				}
				catch (...) {}
			}

			return Result;
		}
		void Workspace::DeleteScript()
		{
			if (PerformHouseKeeping())
			{
				ComponentDLLInterface::ScriptData* Data = ScriptListBox->Show(ScriptListDialog::Operation::e_Delete,
																		(GetIsUninitialized() ? "" : CurrentScriptEditorID));
				if (Data)
				{
					NativeWrapper::g_CSEInterfaceTable->ScriptEditor.DeleteScript(Data->EditorID);
				}
				NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);
			}
		}
		void Workspace::RecompileScripts()
		{
			if (MessageBox::Show("Are you sure you want to recompile all the scripts in the active plugin?",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::YesNo,
								MessageBoxIcon::Exclamation) == DialogResult::Yes)
			{
				NativeWrapper::g_CSEInterfaceTable->ScriptEditor.RecompileScripts();
				MessageBox::Show("All active scripts recompiled. Results have been logged to the console.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Information);
			}
		}
		void Workspace::PreviousScript()
		{
			if (PerformHouseKeeping())
			{
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreviousScriptInList(CurrentScript);
				if (Data)
				{
					UpdateEnvironment(Data, true);
				}
				NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);
			}
		}
		void Workspace::NextScript()
		{
			if (PerformHouseKeeping())
			{
				ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetNextScriptInList(CurrentScript);
				if (Data)
				{
					UpdateEnvironment(Data, true);
				}
				NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);
			}
		}
		void Workspace::CloseScript()
		{
			if (PerformHouseKeeping())
			{
				Rectangle Bounds = GetParentContainer()->GetBounds();
				NativeWrapper::g_CSEInterfaceTable->ScriptEditor.SaveEditorBoundsToINI(Bounds.Left, Bounds.Top, Bounds.Width, Bounds.Height);

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

			switch (E->KeyCode)								// ### WorkspaceContainer::EditorForm_KeyDown refers to some keys directly
			{
			case Keys::T:
				if (E->Modifiers == Keys::Control)
					GetParentContainer()->InstantiateNewWorkspace(0);
				break;
			case Keys::PageDown:
				if (E->Modifiers == Keys::Control)
					GetParentContainer()->SelectNextTab();
				break;
			case Keys::PageUp:
				if (E->Modifiers == Keys::Control)
					GetParentContainer()->SelectPreviousTab();
				break;
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
					SaveScript(ScriptSaveOperation::e_SaveAndCompile);
				else if (E->Control && E->Shift)
					ParentContainer->SaveAllOpenWorkspaces();
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
			case Keys::H:									// Replace
				if (E->Modifiers == Keys::Control)
				{
					ToolBarEditMenuContentsFind->PerformClick();
				}
				break;
			case Keys::G:									// Goto Line
				if (E->Modifiers == Keys::Control)
				{
					ToolBarEditMenuContentsGotoLine->PerformClick();
				}
				break;
			case Keys::E:									// Goto Offset
				if (E->Modifiers == Keys::Control)
				{
					ToolBarEditMenuContentsGotoOffset->PerformClick();
				}
				break;
			case Keys::F4:
				if (E->Modifiers == Keys::Control)
				{
					CloseScript();
				}
			case Keys::D1:
			case Keys::D2:
			case Keys::D3:
			case Keys::D4:
			case Keys::D5:
			case Keys::D6:
			case Keys::D7:
			case Keys::D8:
			case Keys::D9:
				if (E->Modifiers == Keys::Control)
				{
					ParentContainer->SelectTab(E->KeyCode);
				}
				break;
			}

			HandlingKeyDownEvent = false;
		}
		void Workspace::TextEditor_ScriptModified(Object^ Sender, TextEditors::ScriptModifiedEventArgs^ E)
		{
			WorkspaceTabItem->ImageIndex = (int)E->ModifiedStatus;
		}

		void Workspace::MessageList_DoubleClick(Object^ Sender, EventArgs^ E)
		{
			if (GetListViewSelectedItem(MessageList) != nullptr)
			{
				if (GetListViewSelectedItem(MessageList)->ImageIndex == (int)MessageListItemType::e_RegularMessage)
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
				Sorter = gcnew ListViewImgSorter(E->Column, MessageList->Sorting);
				break;
			case 1:
				Sorter = gcnew ListViewIntSorter(E->Column, MessageList->Sorting, false);
				break;
			default:
				Sorter = gcnew ListViewStringSorter(E->Column, MessageList->Sorting);
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
				Sorter = gcnew ListViewIntSorter(E->Column, FindList->Sorting, false);
				break;
			default:
				Sorter = gcnew ListViewStringSorter(E->Column, FindList->Sorting);
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
				Sorter = gcnew ListViewIntSorter(E->Column, BookmarkList->Sorting, false);
				break;
			default:
				Sorter = gcnew ListViewStringSorter(E->Column, BookmarkList->Sorting);
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
					MessageBox::Show("Please expand the Index column sufficiently to allow the editing of its contents",
									SCRIPTEDITOR_TITLE,
									MessageBoxButtons::OK,
									MessageBoxIcon::Information);
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
				Sorter = gcnew ListViewIntSorter(E->Column, VariableIndexList->Sorting, false);
				break;
			default:
				Sorter = gcnew ListViewStringSorter(E->Column, VariableIndexList->Sorting);
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
			Parameters->EditorHandleIndex = WorkspaceHandleIndex;

			if (HandlingKeyDownEvent)
			{
				NewScript();
			}
			else
			{
				switch (Control::ModifierKeys)
				{
				case Keys::Control:
					ParentContainer->PerformRemoteWorkspaceOperation(WorkspaceContainer::RemoteWorkspaceOperation::e_CreateNewWorkspaceAndScript, nullptr);
					break;
				case Keys::Shift:
					Parameters->EditorHandleIndex = 0;
					Parameters->ParameterList->Add((UInt32)0);
					Parameters->ParameterList->Add((UInt32)ParentContainer->GetBounds().X);
					Parameters->ParameterList->Add((UInt32)ParentContainer->GetBounds().Y);
					Parameters->ParameterList->Add((UInt32)ParentContainer->GetBounds().Width);
					Parameters->ParameterList->Add((UInt32)ParentContainer->GetBounds().Height);

					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateWorkspaceContainer, Parameters);
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
				ParentContainer->PerformRemoteWorkspaceOperation(WorkspaceContainer::RemoteWorkspaceOperation::e_CreateNewWorkspaceAndSelectScript, nullptr);
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
			SaveScript(ScriptSaveOperation::e_SaveAndCompile);
		}
		void Workspace::ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E)
		{
			if (GetIsScriptNew() || GetIsUninitialized())
			{
				MessageBox::Show("You may only perform this operation on an existing script.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);

				return;
			}

			SaveScript(ScriptSaveOperation::e_SaveButDontCompile);
		}
		void Workspace::ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E)
		{
			SaveScript(ScriptSaveOperation::e_SaveActivePluginToo);
		}
		void Workspace::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
		{
			RecompileScripts();
		}
		void Workspace::ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E)
		{
			if (!GetIsScriptNew() && !GetIsUninitialized())
			{
				CString CEID(CurrentScriptEditorID);
				NativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileDependencies(CEID.c_str());
				MessageBox::Show("Operation complete! Script variables used as condition parameters will need to be corrected manually. The results have been logged to the console.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Information);
			}
			else
			{
				MessageBox::Show("The current script needs to be compiled before its dependencies can be updated.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);
			}
		}
		void Workspace::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
		{
			DeleteScript();
		}
		void Workspace::ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E)
		{
			ParentContainer->NavigateJumpStack(WorkspaceHandleIndex, WorkspaceContainer::JumpStackNavigationDirection::e_Back);
		}

		void Workspace::ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E)
		{
			ParentContainer->NavigateJumpStack(WorkspaceHandleIndex, WorkspaceContainer::JumpStackNavigationDirection::e_Forward);
		}

		void Workspace::ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E)
		{
			ParentContainer->SaveAllOpenWorkspaces();
		}
		void Workspace::ToolBarOptions_Click(Object^ Sender, EventArgs^ E)
		{
			PREFERENCES->LoadINI();
			PREFERENCES->Show();
		}

		void Workspace::ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E)
		{
			SetScriptType(ScriptType::e_Object);

			if (!GetIsUninitialized())
				SetModifiedStatus(true);
		}
		void Workspace::ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E)
		{
			SetScriptType(ScriptType::e_Quest);

			if (!GetIsUninitialized())
				SetModifiedStatus(true);
		}
		void Workspace::ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E)
		{
			SetScriptType(ScriptType::e_MagicEffect);

			if (!GetIsUninitialized())
				SetModifiedStatus(true);
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
			ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CTUM.c_str());
			if (Data && Data->IsValid())
			{
				switch (Data->Type)
				{
				case 0:
					if (ISDB->GetIsIdentifierUserFunction(MidToken))
						ContextMenuJumpToScript->Text = "Jump to Function script";
					else
						ContextMenuJumpToScript->Text = "Jump to Object script";
					break;
				case 1:
					ContextMenuJumpToScript->Text = "Jump to Quest script";
					break;
				}

				ContextMenuJumpToScript->Tag = gcnew String(Data->EditorID);
			}
			else
				ContextMenuJumpToScript->Visible = false;

			NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);

			ContextMenuRefactorCreateUDFImplementation->Visible = false;
			if (!String::Compare(Tokens[0], "call", true) &&
				!TextEditor->GetCharIndexInsideCommentSegment(TextEditor->GetLastKnownMouseClickOffset()))
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
				FindReplaceBox->Show(ParentContainer->GetHandle(), TextEditor->GetSelectedText(), false);
			else
				FindReplaceBox->Show(ParentContainer->GetHandle(), TextEditor->GetTokenAtMouseLocation(), false);
		}
		void Workspace::ContextMenuToggleComment_Click(Object^ Sender, EventArgs^ E)
		{
			TextEditor->ToggleComment(TextEditor->GetLastKnownMouseClickOffset());
		}
		void Workspace::ContextMenuToggleBookmark_Click(Object^ Sender, EventArgs^ E)
		{
			ToggleBookmark(TextEditor->GetLastKnownMouseClickOffset());
		}
		void Workspace::ContextMenuAddMessage_Click(Object^ Sender, EventArgs^ E)
		{
			String^ Message = "";

			InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter The Message String", "Add Message");
			if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
				return;
			else
				Message = Result->Text;

			AddMessageToMessagePool(MessageListItemType::e_RegularMessage, -1, Message);
		}
		void Workspace::ContextMenuWikiLookup_Click(Object^ Sender, EventArgs^ E)
		{
			Process::Start("http://cs.elderscrolls.com/constwiki/index.php/Special:Search?search=" + TextEditor->GetTokenAtMouseLocation() + "&fulltext=Search");
		}
		void Workspace::ContextMenuOBSEDocLookup_Click(Object^ Sender, EventArgs^ E)
		{
			Process::Start("http://obse.silverlock.org/obse_command_doc.html#" + TextEditor->GetTokenAtMouseLocation());
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
				MessageBox::Show("Couldn't open internet page. Mostly likely caused by an improperly formatted URL.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Error);
			}
		}
		void Workspace::ContextMenuJumpToScript_Click(Object^ Sender, EventArgs^ E)
		{
			ParentContainer->JumpToWorkspace(WorkspaceHandleIndex, dynamic_cast<String^>(ContextMenuJumpToScript->Tag));
		}
		void Workspace::ContextMenuGoogleLookup_Click(Object^ Sender, EventArgs^ E)
		{
			Process::Start("http://www.google.com/search?hl=en&source=hp&q=" + TextEditor->GetTokenAtMouseLocation());
		}
		void Workspace::ContextMenuRefactorAddVariable_Click( Object^ Sender, EventArgs^ E )
		{
			ToolStripMenuItem^ MenuItem = dynamic_cast<ToolStripMenuItem^>(Sender);
			ScriptParser::VariableType VarType = (ScriptParser::VariableType)MenuItem->Tag;
			String^ VarName = ContextMenuWord->Text;
			String^ ScriptText = TextEditor->GetText()->Replace("\r", "");

			if (VarName->Length == 0)
			{
				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter Variable Name", "Add Variable", VarName);
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;
				else
					VarName = Result->Text;
			}

			InsertVariable(VarName, VarType);
		}
		void Workspace::ContextMenuRefactorDocumentScript_Click( Object^ Sender, EventArgs^ E )
		{
			if (GetModifiedStatus())
			{
				MessageBox::Show("The current script needs to be compiled before it can be documented.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);

				return;
			}

			Refactoring::EditScriptComponentDialog DocumentScriptData(GetParentContainer()->GetHandle(),
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

				TextEditor->SetText(DocumentedScript->Substring(0, DocumentedScript->Length - 1), false, false);
			}
		}
		void Workspace::ContextMenuRefactorCreateUDFImplementation_Click( Object^ Sender, EventArgs^ E )
		{
			String^ UDFName = dynamic_cast<String^>(ContextMenuRefactorCreateUDFImplementation->Tag);

			Refactoring::CreateUDFImplementationDialog UDFData(GetParentContainer()->GetHandle());

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

				UDFScriptText += "begin function " + ParamList + "\n\nend\n";

				List<Object^>^ RemoteOpParameters = gcnew List<Object^>();
				RemoteOpParameters->Add(UDFScriptText);
				GetParentContainer()->PerformRemoteWorkspaceOperation(WorkspaceContainer::RemoteWorkspaceOperation::e_CreateNewWorkspaceAndScriptAndSetText,
																	RemoteOpParameters);
			}
		}
		void Workspace::ContextMenuRefactorRenameVariables_Click( Object^ Sender, EventArgs^ E )
		{
			if (GetModifiedStatus())
			{
				MessageBox::Show("The current script needs to be compiled before its variables can be renamed.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);

				return;
			}

			Refactoring::EditScriptComponentDialog RenameVariablesData(GetParentContainer()->GetHandle(),
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
					NativeWrapper::g_CSEInterfaceTable->ScriptEditor.UpdateScriptVarNames(CEID.c_str(), &RenameData);

					MessageBox::Show("Variables have been renamed. Scripts referencing them (current script included) will have to be manually updated with the new identifiers.",
									SCRIPTEDITOR_TITLE,
									MessageBoxButtons::OK,
									MessageBoxIcon::Information);

					for each (CString^ Itr in StringAllocations)
						delete Itr;
				}

				StringAllocations->Clear();
				RenameEntries->Clear();
			}
		}

		void Workspace::ToolBarEditMenuContentsFindReplace_Click(Object^ Sender, EventArgs^ E)
		{
			if (TextEditor->GetSelectedText() != "")
				FindReplaceBox->Show(ParentContainer->GetHandle(), TextEditor->GetSelectedText(), false);
			else
				FindReplaceBox->Show(ParentContainer->GetHandle(), TextEditor->GetTokenAtCaretPos(), false);
		}
		void Workspace::ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E)
		{
			if (ToolBarShowOffsets->Checked)
			{
				MessageBox::Show("This operation can only be performed in the text editor and the preprocessed text viewer",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);
			}
			else
			{
				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Line Number (1 - " + TextEditor->GetTotalLineCount() + ")", "Go To Line");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;

				if (ToolBarShowPreprocessedText->Checked)
					PreprocessedTextViewer->JumpToLine(Result->Text);
				else
					TextEditor->ScrollToLine(Result->Text);
			}
		}
		void Workspace::ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E)
		{
			if (ToolBarShowOffsets->Checked)
			{
				InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Offset (0000 - " + OffsetViewer->GetLastOffset().ToString("X4") + ")", "Go To Offset");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;

				OffsetViewer->JumpToLine(Result->Text);
			}
			else
			{
				MessageBox::Show("This operation can only be performed in the offset viewer",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);
			}
		}

		void Workspace::ToolBarMessageList_Click(Object^ Sender, EventArgs^ E)
		{
			GetParentContainer()->BeginUpdate();

			if (FindList->Visible)
				ToolBarFindList->PerformClick();
			else if (BookmarkList->Visible)
				ToolBarBookmarkList->PerformClick();
			else if (VariableIndexList->Visible)
				ToolBarGetVarIndices->PerformClick();

			if (MessageList->Visible == false)
			{
				MessageList->Show();
				MessageList->BringToFront();
				ToolBarMessageList->Checked = true;
				WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height / 1.5;
			}
			else
			{
				MessageList->Hide();
				ToolBarMessageList->Checked = false;
				WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height;
			}

			GetParentContainer()->EndUpdate();
		}
		void Workspace::ToolBarFindList_Click(Object^ Sender, EventArgs^ E)
		{
			GetParentContainer()->BeginUpdate();

			if (MessageList->Visible)
				ToolBarMessageList->PerformClick();
			else if (BookmarkList->Visible)
				ToolBarBookmarkList->PerformClick();
			else if (VariableIndexList->Visible)
				ToolBarGetVarIndices->PerformClick();

			if (FindList->Visible == false)
			{
				FindList->Show();
				FindList->BringToFront();
				ToolBarFindList->Checked = true;
				WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height / 1.5;
			}
			else
			{
				FindList->Hide();
				ToolBarFindList->Checked = false;
				WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height;
			}

			GetParentContainer()->EndUpdate();
		}
		void Workspace::ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E)
		{
			GetParentContainer()->BeginUpdate();

			if (MessageList->Visible)
				ToolBarMessageList->PerformClick();
			else if (FindList->Visible)
				ToolBarFindList->PerformClick();
			else if (VariableIndexList->Visible)
				ToolBarGetVarIndices->PerformClick();

			if (BookmarkList->Visible == false)
			{
				BookmarkList->Show();
				BookmarkList->BringToFront();
				ToolBarBookmarkList->Checked = true;
				WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height / 1.5;
			}
			else
			{
				BookmarkList->Hide();
				ToolBarBookmarkList->Checked = false;
				WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height;
			}

			GetParentContainer()->EndUpdate();
		}
		void Workspace::ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E)
		{
			SaveFileDialog^ SaveManager = gcnew SaveFileDialog();

			SaveManager->DefaultExt = "*.txt";
			SaveManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			SaveManager->FileName = GetScriptDescription();
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
			SaveManager->SelectedPath = gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetDefaultCachePath());

			if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->SelectedPath->Length > 0)
			{
				ParentContainer->DumpAllOpenWorkspacesToFolder(SaveManager->SelectedPath);
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
					ParentContainer->LoadFileIntoNewWorkspace(Itr);
				}
			}
		}
		void Workspace::ToolBarGetVarIndices_Click(Object^ Sender, EventArgs^ E)
		{
			GetParentContainer()->BeginUpdate();

			if (VariableIndexList->Visible == false)
			{
				if (TextEditor->GetModifiedStatus())
				{
					MessageBox::Show("The current script needs to be compiled before its variable indices can be updated.",
									SCRIPTEDITOR_TITLE,
									MessageBoxButtons::OK,
									MessageBoxIcon::Exclamation);
				}
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
					ComponentDLLInterface::ScriptVarListData* Data = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetScriptVarList((CString(CurrentScriptEditorID)).c_str());
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
					NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);

					VariableIndexList->Show();
					VariableIndexList->BringToFront();

					if (VariableIndexList->Items->Count)
						ToolBarUpdateVarIndices->Enabled = true;

					WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height / 1.5;
					ToolBarGetVarIndices->Checked = true;
				}
			}
			else
			{
				VariableIndexList->Hide();

				ToolBarGetVarIndices->Checked = false;
				ToolBarUpdateVarIndices->Enabled = false;
				WorkspaceSplitter->SplitterDistance = ParentContainer->GetBounds().Height;
			}

			GetParentContainer()->EndUpdate();
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

			if (!NativeWrapper::g_CSEInterfaceTable->ScriptEditor.UpdateScriptVarIndices(CScriptName.c_str(), &Data))
				DebugPrint("Couldn't successfully update all variable indices of script '" + CurrentScriptEditorID + "'");
			else
			{
				if (PREFERENCES->FetchSettingAsInt("RecompileVarIdx", "General"))
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
				int Caret = OffsetViewer->Hide();
				TextEditor->FocusTextArea();
				TextEditor->SetCaretPos(Caret);
				ToolBarShowOffsets->Checked = false;
			}
			else
			{
				if (OffsetViewer->Show((TextEditor->GetCaretPos() != -1 ? TextEditor->GetCaretPos() : 0)))
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
				ClearErrorMessagesFromMessagePool();
				String^ PreprocessedText = "";
				if (PreprocessScriptText(PreprocessedText))
				{
					PreprocessedTextViewer->Show(PreprocessedText, (TextEditor->GetCaretPos() != -1 ? TextEditor->GetCaretPos() : 0));
					ToolBarShowPreprocessedText->Checked = true;
				}
				else
					MessageBox::Show("The preprocessing operation was unsuccessful.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}
		void Workspace::ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E)
		{
			ParentContainer->SetCursor(Cursors::WaitCursor);

			String^ SanitizedText = TextEditor->GetText();

			if (PREFERENCES->FetchSettingAsInt("AnnealCasing", "Sanitize"))
				SanitizedText = SanitizeScriptText(SanitizeOperation::e_AnnealCasing, SanitizedText);

			if (PREFERENCES->FetchSettingAsInt("EvalifyIfs", "Sanitize"))
				SanitizedText = SanitizeScriptText(SanitizeOperation::e_EvalifyIfs, SanitizedText);

			if (PREFERENCES->FetchSettingAsInt("CompilerOverrideBlocks", "Sanitize"))
				SanitizedText = SanitizeScriptText(SanitizeOperation::e_CompilerOverrideBlocks, SanitizedText);

			if (PREFERENCES->FetchSettingAsInt("IndentLines", "Sanitize"))
				SanitizedText = SanitizeScriptText(SanitizeOperation::e_Indent, SanitizedText);

			TextEditor->SetText(SanitizedText, false, false);

			ParentContainer->SetCursor(Cursors::Default);
		}

		void Workspace::ToolBarBindScript_Click(Object^ Sender, EventArgs^ E)
		{
			if (GetIsScriptNew() || GetIsUninitialized())
			{
				MessageBox::Show("You may only perform this operation on an existing script.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				return;
			}
			else
			{
				NativeWrapper::g_CSEInterfaceTable->ScriptEditor.BindScript((CString(CurrentScriptEditorID)).c_str(), (HWND)GetParentContainer()->GetHandle());
			}
		}

		void Workspace::ScriptEditorPreferences_Saved( Object^ Sender, EventArgs^ E )
		{
			AutoSaveTimer->Stop();

			Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"), PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"), (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));
			TextEditor->SetFont(CustomFont);
			OffsetViewer->SetFont(CustomFont);
			PreprocessedTextViewer->SetFont(CustomFont);

			int TabSize = Decimal::ToInt32(PREFERENCES->FetchSettingAsInt("TabSize", "Appearance"));
			if (TabSize == 0)
				TabSize = 4;

			TextEditor->SetTabCharacterSize(TabSize);
			AutoSaveTimer->Interval = PREFERENCES->FetchSettingAsInt("AutoRecoverySavePeriod", "Backup") * 1000 * 60;
			AutoSaveTimer->Start();
		}

		void Workspace::AutoSaveTimer_Tick( Object^ Sender, EventArgs^ E )
		{
			if (PREFERENCES->FetchSettingAsInt("UseAutoRecovery", "Backup"))
			{
				if (GetIsUninitialized() == false && GetIsScriptNew() == false && TextEditor->GetModifiedStatus() == true)
				{
					SaveScriptToDisk(gcnew String(NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath()), false);
				}
			}
		}
	#pragma endregion
#pragma endregion
	}
}