#include "ScriptEditor.h"
#include "ScriptEditorManager.h"
#include "IntelliSense.h"

#include "[Common]\HandShakeStructs.h"
#include "[Common]\ListViewUtilities.h"
#include "[Common]\NativeWrapper.h"

#using "Microsoft.VisualBasic.dll"

namespace ScriptEditor
{
	void GlobalInputMonitor_MouseUp(Object^ Sender, MouseEventArgs^ E)
	{
		switch (E->Button)
		{
		case MouseButtons::Right:
			if (SEMGR->TornWorkspace != nullptr)
			{
				DebugPrint("Tab Tear Operation interrupted by right mouse button");
				HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
				SEMGR->TornWorkspace = nullptr;
			}
			break;
		case MouseButtons::Left:
		{
			if (SEMGR->TornWorkspace != nullptr)
			{
				IntPtr Wnd = NativeWrapper::WindowFromPoint(E->Location);
				if (Wnd == IntPtr::Zero)
				{
					ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
					Parameters->VanillaHandleIndex = 0;
					Parameters->ParameterList->Add(ScriptEditorManager::TabTearOpType::e_NewContainer);
					Parameters->ParameterList->Add(SEMGR->TornWorkspace);
					Parameters->ParameterList->Add(nullptr);
					Parameters->ParameterList->Add(E->Location);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_TabTearOp, Parameters);

					HookManager::MouseUp -= TabContainer::GlobalMouseHook_MouseUpHandler;
					SEMGR->TornWorkspace = nullptr;
				}

				DotNetBar::TabStrip^ Strip = nullptr;
				try
				{
					Strip = dynamic_cast<DotNetBar::TabStrip^>(Control::FromHandle(Wnd));
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
						Parameters->VanillaHandleIndex = 0;
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
					Parameters->VanillaHandleIndex = 0;
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
		}
	}

	Assembly^ ResolveMissingAssemblies(Object^ Sender, ResolveEventArgs^ E)
	{
		Assembly^ PreprocAssembly, ^ExecutingAssemblies;
		String^ TempPath = "";

		ExecutingAssemblies = Assembly::GetExecutingAssembly();

		for each(AssemblyName^ AssmbName in ExecutingAssemblies->GetReferencedAssemblies())
		{
			if (AssmbName->FullName->Substring(0, AssmbName->FullName->IndexOf(",")) == E->Name->Substring(0, E->Name->IndexOf(",")))
			{
				TempPath = Globals::AppPath + "Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\" + E->Name->Substring(0, E->Name->IndexOf(",")) + ".dll";
				PreprocAssembly = Assembly::LoadFrom(TempPath);
				return PreprocAssembly;
			}
		}

		return nullptr;
	}

	#pragma region TabContainer
		TabContainer::TabContainer(UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height)
		{
			Application::EnableVisualStyles();
			EditorForm = gcnew Form();
			EditorForm->SuspendLayout();

			EditorForm->FormBorderStyle = ::FormBorderStyle::Sizable;
			EditorForm->Closing += gcnew CancelEventHandler(this, &TabContainer::EditorForm_Cancel);
			EditorForm->KeyDown += gcnew KeyEventHandler(this, &TabContainer::EditorForm_KeyDown);
			EditorForm->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			EditorForm->AutoScaleMode = AutoScaleMode::Font;
			EditorForm->Size = Size(Width, Height);
			EditorForm->KeyPreview = true;

			if (!FileFlags->Images->Count)
			{
				FileFlags->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOff"));		// unmodified
				FileFlags->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ModifiedFlagOn"));		// modified
				FileFlags->ImageSize = Size(12, 12);
			}

			ScriptStrip = gcnew DotNetBar::TabControl();
			ScriptStrip->CanReorderTabs = true;
			ScriptStrip->TabStrip->CloseButtonOnTabsAlwaysDisplayed = true;
			ScriptStrip->TabStrip->CloseButtonOnTabsVisible = true;
			ScriptStrip->CloseButtonVisible = false;
			ScriptStrip->Dock = DockStyle::Fill;
			ScriptStrip->Location = Point(0, 0);
			ScriptStrip->Font = gcnew Font("Segoe UI", 7.75F, FontStyle::Regular);
			ScriptStrip->SelectedTabFont = gcnew Font("Segoe UI", 7.75F, FontStyle::Italic);
			ScriptStrip->TabLayoutType = DotNetBar::eTabLayoutType::FixedWithNavigationBox;
			ScriptStrip->TabItemClose += gcnew DotNetBar::TabStrip::UserActionEventHandler(this, &TabContainer::ScriptStrip_TabItemClose);
			ScriptStrip->SelectedTabChanged += gcnew DotNetBar::TabStrip::SelectedTabChangedEventHandler(this, &TabContainer::ScriptStrip_SelectedTabChanged);
			ScriptStrip->SelectedTabChanging += gcnew DotNetBar::TabStrip::SelectedTabChangingEventHandler(this, &TabContainer::ScriptStrip_SelectedTabChanging);
			ScriptStrip->TabRemoved += gcnew EventHandler(this, &TabContainer::ScriptStrip_TabRemoved);
			ScriptStrip->TabStrip->MouseClick += gcnew MouseEventHandler(this, &TabContainer::ScriptStrip_MouseClick);
			ScriptStrip->TabStrip->MouseDown += gcnew MouseEventHandler(this, &TabContainer::ScriptStrip_MouseDown);
			ScriptStrip->TabStrip->MouseUp += gcnew MouseEventHandler(this, &TabContainer::ScriptStrip_MouseUp);
			ScriptStrip->AntiAlias = true;
			ScriptStrip->CloseButtonPosition = DotNetBar::eTabCloseButtonPosition::Right;
			ScriptStrip->KeyboardNavigationEnabled = false;
			ScriptStrip->TabLayoutType = DotNetBar::eTabLayoutType::FixedWithNavigationBox;
			ScriptStrip->TabScrollAutoRepeat = true;
			ScriptStrip->TabStop = false;
			ScriptStrip->ImageList = FileFlags;
			ScriptStrip->Style = DotNetBar::eTabStripStyle::VS2005Dock;
			DotNetBar::TabColorScheme^ TabItemColorScheme = gcnew DotNetBar::TabColorScheme(DotNetBar::eTabStripStyle::SimulatedTheme);
			ScriptStrip->ColorScheme = TabItemColorScheme;
			ScriptStrip->TabStrip->Tag = this;

			NewTabButton = gcnew DotNetBar::TabItem;
			NewTabButton->Name = "NewTabButton";
			NewTabButton->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource("NewTabButton");
			NewTabButton->CloseButtonVisible = false;
			NewTabButton->BackColor = Color::AliceBlue;
			NewTabButton->BackColor2 = Color::BurlyWood;
			NewTabButton->BackColorGradientAngle = 40;
			NewTabButton->LightBorderColor = Color::BurlyWood;
			NewTabButton->BorderColor = Color::BurlyWood;
			NewTabButton->DarkBorderColor = Color::Bisque;
			NewTabButton->Tooltip = "New Tab";
			NewTabButton->Click += gcnew EventHandler(this, &TabContainer::NewTabButton_Click);

			ScriptStrip->Tabs->Add(NewTabButton);

			EditorForm->HelpButton = false;
			EditorForm->Text = "CSE Script Editor";

			EditorForm->Controls->Add(ScriptStrip);

			if (OPTIONS->FetchSettingAsInt("UseCSParent")) 
			{
				EditorForm->ShowInTaskbar = false;
				EditorForm->Show(gcnew WindowHandleWrapper(NativeWrapper::GetCSMainWindowHandle()));
			}
			else
			{
				EditorForm->Show();
			}

			EditorForm->Location = Point(PosX, PosY);

			DestructionFlag = false;
			BackStack = gcnew Stack<UInt32>();
			ForwardStack = gcnew Stack<UInt32>();
			RemovingTab = false;

			EditorForm->ResumeLayout();

			const char* EditorID = NativeWrapper::ScriptEditor_GetAuxScriptName();
			if (EditorID)			CreateNewTab(gcnew String(EditorID));
			else					CreateNewTab(nullptr);
		}

		void TabContainer::EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			if (DestructionFlag)		return;

			E->Cancel = true;
			if (ScriptStrip->Tabs->Count > 2)
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
			CloseAllTabs();
			DestructionFlag = false;
		}

		void TabContainer::ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::TabStripActionEventArgs^ E)
		{
			Workspace^% Itr = dynamic_cast<Workspace^>(ScriptStrip->SelectedTab->Tag);

			E->Cancel = true;
			ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
			Parameters->VanillaHandleIndex = Itr->GetAllocatedIndex();
			Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Close);
			SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
		}

		void TabContainer::ScriptStrip_SelectedTabChanging(Object^ Sender, DotNetBar::TabStripTabChangingEventArgs^ E)
		{
			if (!RemovingTab && E->NewTab == NewTabButton)
			{
				E->Cancel = true;
			}
			RemovingTab = false;
		}

		void TabContainer::ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::TabStripTabChangedEventArgs^ E)
		{
			if (ScriptStrip->SelectedTab == nullptr)	return;
			else if (ScriptStrip->SelectedTab == NewTabButton)
			{
				if (!ScriptStrip->SelectNextTab())		ScriptStrip->SelectPreviousTab();
				return;
			}

			Workspace^ Itr = dynamic_cast<Workspace^>(ScriptStrip->SelectedTab->Tag);
			EditorForm->Text = Itr->GetScriptDescription() + " - CSE Editor";
			EditorForm->Focus();
			Itr->HandleWorkspaceFocus();
		}

		void TabContainer::ScriptStrip_TabRemoved(Object^ Sender, EventArgs^ E)
		{
			RemovingTab = true;

			if (ScriptStrip->Tabs->Count == 1)
			{
				if (!DestructionFlag && OPTIONS->FetchSettingAsInt("DestroyOnLastTabClose") == 0)
					CreateNewTab(nullptr);
				else
				{
					ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
					Parameters->VanillaHandleIndex = 0;
					Parameters->ParameterList->Add(this);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_DestroyTabContainer, Parameters);
				}
			}
		}

		UInt32 TabContainer::CreateNewTab(String^ ScriptName)
		{
			EditorForm->SuspendLayout();

			UInt32 AllocatedIndex = 0;
			if (ScriptName != nullptr)
			{
				CStringWrapper^ CEID = gcnew CStringWrapper(ScriptName);
				AllocatedIndex = NativeWrapper::ScriptEditor_InstantiateCustomEditor(CEID->String());
			}
			else
				AllocatedIndex = NativeWrapper::ScriptEditor_InstantiateCustomEditor(0);

			if (!AllocatedIndex) 
			{
				DebugPrint("Fatal error occured when allocating a custom editor! Instantiating an empty workspace...", true);
				AllocatedIndex = NativeWrapper::ScriptEditor_InstantiateCustomEditor(0);
			}

			ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
			Parameters->VanillaHandleIndex = 0;
			Parameters->ParameterList->Add(AllocatedIndex);
			Parameters->ParameterList->Add(this);
			SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateWorkspace, Parameters);

			NativeWrapper::ScriptEditor_PostProcessEditorInit(AllocatedIndex);
			SEMGR->GetAllocatedWorkspace(AllocatedIndex)->MakeActiveInParentContainer();

			EditorForm->ResumeLayout();

			return AllocatedIndex;
		}

		void TabContainer::PerformRemoteOperation(RemoteOperation Operation, Object^ Arbitrary)
		{
			ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();

			switch (Operation)
			{
			case RemoteOperation::e_New:
				{
					UInt32 AllocatedIndex = CreateNewTab(nullptr);
					Parameters->VanillaHandleIndex = AllocatedIndex;
					Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_New);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
					break;
				}
			case RemoteOperation::e_Open:
				{
					UInt32 AllocatedIndex = CreateNewTab(nullptr);
					SEMGR->GetAllocatedWorkspace(AllocatedIndex)->ShowScriptListBox(ScriptListDialog::Operation::e_Open);
					break;
				}
			case RemoteOperation::e_LoadNew:
				{
					String^ FilePath = dynamic_cast<String^>(Arbitrary);
					UInt32 AllocatedIndex = 0;

					if (OPTIONS->FetchSettingAsInt("LoadScriptUpdateExistingScripts") == 0)
						AllocatedIndex = CreateNewTab(nullptr);
					else
					{
						try
						{
							StreamReader^ TextParser = gcnew StreamReader(FilePath);
							String^ FileContents = TextParser->ReadToEnd();
							TextParser->Close();

							String^ ScriptName = ((FileContents->Split('\n', 1))[0]->Split(' '))[1];
							CStringWrapper^ CEID = gcnew CStringWrapper(ScriptName);
							if (NativeWrapper::LookupFormByEditorID(CEID->String()))
							{
								AllocatedIndex = CreateNewTab(ScriptName);
								SEMGR->GetAllocatedWorkspace(AllocatedIndex)->LoadFileFromDisk(FilePath);
								break;
							}
							else
								AllocatedIndex = CreateNewTab(nullptr);
						}
						catch (Exception^ E)
						{
							DebugPrint("Couldn't read script name from file " + FilePath + " for script updating!\n\tException: " + E->Message);
							AllocatedIndex = CreateNewTab(nullptr);
						}
					}

					Parameters->VanillaHandleIndex = AllocatedIndex;
					Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_New);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);

					SEMGR->GetAllocatedWorkspace(AllocatedIndex)->LoadFileFromDisk(FilePath);
					break;
				}
			}
		}

		void TabContainer::Destroy()
		{
			FlagDestruction(true);
			EditorForm->Close();
			BackStack->Clear();
			ForwardStack->Clear();
		}

		void TabContainer::JumpToScript(UInt32 AllocatedIndex, String^% ScriptName)
		{
			UInt32 Count = 0;
			DotNetBar::TabItem^ OpenedWorkspace = nullptr;

			for each (DotNetBar::TabItem^ Itr in ScriptStrip->Tabs)
			{
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);

				if (Editor != nullptr && !String::Compare(const_cast<String^>(Editor->GetScriptID()), ScriptName, true))
				{
					Count++;
					OpenedWorkspace = Itr;
				}
			}

			if (Count == 1) 
			{
				ScriptStrip->SelectedTab = OpenedWorkspace;
			}
			else
			{
				CreateNewTab(ScriptName);
			}
			BackStack->Push(AllocatedIndex);
			ForwardStack->Clear();
		}

		void TabContainer::NavigateStack(UInt32 AllocatedIndex, TabContainer::NavigationDirection Direction)
		{
			UInt32 JumpIndex = 0;
			switch (Direction)
			{
			case NavigationDirection::e_Back:
				if (BackStack->Count < 1)		return;
				JumpIndex = BackStack->Pop();
				break;
			case NavigationDirection::e_Forward:
				if (ForwardStack->Count < 1)	return;
				JumpIndex = ForwardStack->Pop();
				break;
			}

			Workspace^ Itr = SEMGR->GetAllocatedWorkspace(JumpIndex);
			if (Itr->IsValid() == 0) 
			{
				NavigateStack(AllocatedIndex, Direction);
			}
			else
			{
				switch (Direction)
				{
				case NavigationDirection::e_Back:
					ForwardStack->Push(AllocatedIndex);
					break;
				case NavigationDirection::e_Forward:
					BackStack->Push(AllocatedIndex);
					break;
				}
				Itr->MakeActiveInParentContainer();
				DebugPrint("Jumping from index " + AllocatedIndex + " to " + JumpIndex);
			}
		}

		void TabContainer::NewTabButton_Click(Object^ Sender, EventArgs^ E)
		{
			CreateNewTab(nullptr);
		}

		void TabContainer::EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::T:
				if (E->Modifiers == Keys::Control)
				{
					CreateNewTab(nullptr);
				}
				break;
			case Keys::Tab:
				if (ScriptStrip->Tabs->Count < 2)	break;

				if (E->Control == true && E->Shift == false)
				{
					if (ScriptStrip->SelectedTabIndex == ScriptStrip->Tabs->Count - 1)
					{
						ScriptStrip->SelectedTab = ScriptStrip->Tabs[1];
					} 
					else
					{
						ScriptStrip->SelectNextTab();
					}
					E->Handled = true;
				}
				else if (E->Control == true && E->Shift == true)
				{
					if (ScriptStrip->SelectedTabIndex == 1)
					{
						ScriptStrip->SelectedTab = ScriptStrip->Tabs[ScriptStrip->Tabs->Count - 1];
					} 
					else
					{
						ScriptStrip->SelectPreviousTab();
					}
					E->Handled = true;
				}
				break;
			}
		}

		DotNetBar::TabItem^ TabContainer::GetMouseOverTab()
		{
			for each (DotNetBar::TabItem^ Itr in ScriptStrip->Tabs)
			{
				if (Itr->IsMouseOver)	return Itr;
			}
			return nullptr;
		}

		void TabContainer::ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E)
		{
			switch (E->Button)
			{
			case MouseButtons::Middle:
			{
				DotNetBar::TabItem^ MouseOverTab = GetMouseOverTab();
				if (MouseOverTab != nullptr && MouseOverTab != NewTabButton)
				{
					Workspace^% Itr = dynamic_cast<Workspace^>(MouseOverTab->Tag);

					ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
					Parameters->VanillaHandleIndex = Itr->GetAllocatedIndex();
					Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Close);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
				}
				break;
			}
			}
		}

		void TabContainer::ScriptStrip_MouseDown(Object^ Sender, MouseEventArgs^ E)
		{
			switch (E->Button)
			{
			case MouseButtons::Left:
			{
				if (SEMGR->TornWorkspace != nullptr)
				{
					DebugPrint("A previous tab tearing operation did not complete successfully!");
					SEMGR->TornWorkspace = nullptr;
				}

				DotNetBar::TabItem^ MouseOverTab = GetMouseOverTab();
				if (MouseOverTab != nullptr && MouseOverTab != NewTabButton)
				{
					SEMGR->TornWorkspace = dynamic_cast<Workspace^>(MouseOverTab->Tag);;
					HookManager::MouseUp += GlobalMouseHook_MouseUpHandler;
				}
				break;
			}
			}
		}

		void TabContainer::ScriptStrip_MouseUp(Object^ Sender, MouseEventArgs^ E)
		{
			switch (E->Button)
			{
			case MouseButtons::Left:
				break;
			}
		}

		void TabContainer::SaveAllTabs()
		{
			for each (DotNetBar::TabItem^ Itr in ScriptStrip->Tabs)
			{
				if (Itr == NewTabButton)	continue;
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);
				Editor->PerformCompileAndSave();
			}
		}

		void TabContainer::CloseAllTabs()
		{
			array<DotNetBar::TabItem^, 1>^ Tabs = gcnew array<DotNetBar::TabItem^, 1>(ScriptStrip->Tabs->Count);
			ScriptStrip->Tabs->CopyTo(Tabs, 0);

			for each (DotNetBar::TabItem^ Itr in Tabs) 
			{
				if (Itr == NewTabButton)	continue;
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);

				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->VanillaHandleIndex = Editor->GetAllocatedIndex();
				Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Close);
				SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
			}
		}

		void TabContainer::DumpAllTabs(String^ FolderPath)
		{
			for each (DotNetBar::TabItem^ Itr in ScriptStrip->Tabs) 
			{
				if (Itr == NewTabButton)	continue;
				Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);
				if (Editor->GetIsCurrentScriptNew())	continue;

				Editor->SaveScriptToDisk(FolderPath, false);
			}
		}

		void TabContainer::LoadToTab(String^ FileName)
		{
			PerformRemoteOperation(RemoteOperation::e_LoadNew, FileName);
		}

		Rectangle TabContainer::GetEditorFormRect()
		{
			if (GetEditorFormWindowState() == FormWindowState::Normal)
				return EditorForm->Bounds;
			else
				return EditorForm->RestoreBounds;
		}

		Workspace^ TabContainer::LookupWorkspaceByTab(UInt32 TabIndex)
		{
			if (TabIndex >= ScriptStrip->Tabs->Count)
				return Workspace::NullWorkspace;
			else
				return dynamic_cast<Workspace^>(ScriptStrip->Tabs[TabIndex]->Tag);
		}

		void TabContainer::AddTab(DotNetBar::TabItem^ Tab)
		{
			ScriptStrip->Tabs->Add(Tab);
		}

		void TabContainer::RemoveTab(DotNetBar::TabItem^ Tab)
		{
			ScriptStrip->Tabs->Remove(Tab);
		}

		void TabContainer::AddTabControlBox(DotNetBar::TabControlPanel^ Box)
		{
			ScriptStrip->Controls->Add(Box);
		}

		void TabContainer::RemoveTabControlBox(DotNetBar::TabControlPanel^ Box)
		{
			ScriptStrip->Controls->Remove(Box);
		}

		void TabContainer::SelectTab(DotNetBar::TabItem^ Tab)
		{
			ScriptStrip->SelectedTab = Tab;
			ScriptStrip->TabStrip->EnsureVisible(Tab);
		}
	#pragma endregion

	#pragma region Workspace
		Workspace::Workspace(UInt32 Index, TabContainer^ Parent)
		{
			ParentContainer = Parent;
			EditorTab = gcnew DotNetBar::TabItem();
			EditorControlBox = gcnew DotNetBar::TabControlPanel();

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

			ScriptListingDialog = gcnew ScriptListDialog(Index);

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

			if (MessageIcon->Images->Count == 0)
			{
				MessageIcon->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListWarning"));
				MessageIcon->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListError"));
				MessageIcon->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListMessage"));
				MessageIcon->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListWarning"));
			}

			EditorControlBox->Dock = DockStyle::Fill;
			EditorControlBox->Location = Point(0, 26);
			EditorControlBox->Padding = Padding(1);
			EditorControlBox->TabItem = EditorTab;

			EditorTab->AttachedControl = EditorControlBox;
			EditorTab->Text = "New Workspace";
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
			WorkspaceSplitter->SplitterWidth = 5;
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

			ToolBarEditMenuContentsFind->Text = "Find     ";
			ToolBarEditMenuContentsFind->ToolTipText = "Enter your search string in the adjacent textbox.";
			ToolBarEditMenuContentsFind->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsFind_Click);
			ToolBarEditMenuContentsReplace->Text = "Replace  ";
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

			ToolBarScriptTypeContentsObject->Text = "Object                  ";
			ToolBarScriptTypeContentsObject->ToolTipText = "Object";
			ToolBarScriptTypeContentsObject->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsObject_Click);
			ToolBarScriptTypeContentsQuest->Text = "Quest                    ";
			ToolBarScriptTypeContentsQuest->ToolTipText = "Quest";
			ToolBarScriptTypeContentsQuest->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsQuest_Click);
			ToolBarScriptTypeContentsMagicEffect->Text = "Magic Effect     ";
			ToolBarScriptTypeContentsMagicEffect->ToolTipText = "Magic Effect";
			ToolBarScriptTypeContentsMagicEffect->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsMagicEffect_Click);

			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsObject);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsQuest);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsMagicEffect);

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
			MessageList->SmallImageList = MessageIcon;

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
			catch (...){}

			WorkspaceSplitter->Enabled = false;
			ToolBarUpdateVarIndices->Enabled = false;

			AllocatedIndex = Index;
			DestructionFlag = false;
			ScriptType = 0;
			ScriptEditorID = "";
			HandlingKeyDownEvent = false;

			SetModifiedStatus(false);
			TextEditor->SetContextMenu(TextEditorContextMenu);
		}
		Workspace::Workspace(UInt32 Index)
		{
			AllocatedIndex = Index;
		}

		#pragma region Methods
			void Workspace::EnableControls()
			{
				WorkspaceSplitter->Enabled = true;
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
			void Workspace::FindReplaceWrapper(ScriptTextEditorInterface::FindReplaceOperation Operation)
			{
				String^ SearchString = ToolBarCommonTextBox->Text;
				String^ ReplaceString = "qqq";

				if (Operation == ScriptTextEditorInterface::FindReplaceOperation::e_Replace)
				{
					ReplaceString = Microsoft::VisualBasic::Interaction::InputBox("Enter replace string",
									"Find and Replace - CSE Editor",
									"",
									SystemInformation::PrimaryMonitorSize.Width / 2,
									SystemInformation::PrimaryMonitorSize.Height / 2);
				}

				if (SearchString == "")
				{
					MessageBox::Show("Enter a valid search string.", "Find and Replace - CSE Editor");
					return;
				}
				else if (ReplaceString->Length == 0)
					return;

				FindList->Items->Clear();
				UInt32 Hits = TextEditor->FindReplace(Operation,
											SearchString,
											ReplaceString,
											gcnew ScriptTextEditorInterface::FindReplaceOutput(this, &ScriptEditor::Workspace::FindReplaceOutput));

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

				String^ BookmarkDesc = Microsoft::VisualBasic::Interaction::InputBox("Enter a description for the bookmark",
										"Place Bookmark",
										"",
										SystemInformation::PrimaryMonitorSize.Width / 2,
										SystemInformation::PrimaryMonitorSize.Height / 2);

				if (BookmarkDesc == "")		return;

				ListViewItem^ Item = gcnew ListViewItem(LineNo.ToString());
				Item->SubItems->Add(BookmarkDesc);
				BookmarkList->Items->Add(Item);

				if (!BookmarkList->Visible)
					ToolBarBookmarkList->PerformClick();
			}
			void Workspace::SetScriptType(UInt16 ScriptType)
			{
				switch (ScriptType)
				{
				case 0:
					ToolBarScriptType->Text = "Object Script";
					break;
				case 1:
					ToolBarScriptType->Text = "Quest Script";
					break;
				case 2:
					ToolBarScriptType->Text = "Magic Effect Script";
					break;
				}
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
			//		Result += ";<CSEStatutoryWarning> This script may contain preprocessor directives parsed by the CSE Script Editor. Refrain from modifying it in the vanilla editor. </CSEStatutoryWarning>\n";
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
						if (!TextParser->HasToken(";</CSEBlock>"))
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
					else if (!TextParser->HasToken(";<CSEBlock>"))
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
				int CaretPos = -1;

				while (ReadLine != nullptr)
				{
					TextParser->Tokenize(ReadLine, false);
					if (!TextParser->Valid)
					{
						ReadLine = StringParser->ReadLine();
						continue;
					}

					if (!TextParser->HasToken(";<CSECaretPos>"))
					{
						try { CaretPos = int::Parse(TextParser->Tokens[1]); } catch (...) { CaretPos = -1; }
						break;
					}

					ReadLine = StringParser->ReadLine();
				}

				if (CaretPos > -1)
				{
					TextEditor->SetCaretPos(CaretPos);
					TextEditor->ScrollToCaret();
				}
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

					if (!TextParser->HasToken(";<CSEBookmark>"))
					{
						array<String^>^ Splits = ReadLine->Substring(TextParser->Indices[0])->Split(Globals::TabDelimit);
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
					if (!TextParser->HasToken(";<CSEMessageEditor>"))
					{
						Message = ReadLine->Substring(TextParser->Indices[1])->Replace(" </CSEMessageEditor>", "");
						AddMessageToPool(MessageType::e_CSEMessage, -1, Message);
					}
					else if (!TextParser->HasToken(";<CSEMessageRegular>"))
					{
						Message = ReadLine->Substring(TextParser->Indices[1])->Replace(" </CSEMessageRegular>", "");
						AddMessageToPool(MessageType::e_Message, -1, Message);
					}

					ReadLine = StringParser->ReadLine();
				}
			}
			void Workspace::PreprocessorErrorOutputWrapper(String^ Message)
			{
				AddMessageToPool(MessageType::e_Error, -1, Message);
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

									if (LocalParser->IsComment(i) == -1 && ISDB->IsCommand(Token))
										SanitizedScriptText += ISDB->SanitizeCommandName(Token);
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

			void Workspace::InitializeScript(String^ ScriptText, UInt16 ScriptType, String^ ScriptName, UInt32 Data, UInt32 DataLength, UInt32 FormID)
			{
				if (ScriptName != "New Script")
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

				ScriptEditorID = ScriptName;
				EditorTab->Text = ScriptName + " [" + FormID.ToString("X8") + "]";
				ParentContainer->SetWindowTitle(EditorTab->Text + " - CSE Editor");
				SetScriptType(ScriptType);

				EnableControls();
				SetModifiedStatus(false);

				ToolBarByteCodeSize->Value = DataLength;
				ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(DataLength / (float)1024));

				OffsetViewer->InitializeViewer(ScriptText, Data, DataLength);
				TextEditor->UpdateIntelliSenseLocalDatabase();
			}

			void Workspace::UpdateScriptFromDataPackage(ScriptData* Package)
			{
				switch (Package->Type)
				{
				case 9:									// Function script
					Package->Type = 0;
					break;
				case 99:
					DebugPrint("Couldn't fetch script data from the vanilla editor!", true);
					return;
				}

				ScriptEditorID = gcnew String(Package->EditorID);
				EditorTab->Text = ScriptEditorID + " [" + Package->FormID.ToString("X8") + "]";
				ParentContainer->SetWindowTitle(EditorTab->Text + " - CSE Editor");

				SetModifiedStatus(false);

				ToolBarByteCodeSize->Value = Package->Length;
				ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(Package->Length / (float)1024));

				OffsetViewer->InitializeViewer(gcnew String(Package->Text), (UInt32)Package->ByteCode, Package->Length);
			}
			void Workspace::AddItemToScriptListDialog(String^% ScriptName, UInt32 FormID, UInt16 Type, UInt32 Flags)
			{
				String^ ScriptType;
				switch (Type)
				{
				case 0:
					ScriptType = "Object";
					break;
				case 1:
					ScriptType = "Quest";
					break;
				case 2:
					ScriptType = "Magic Effect";
					break;
				case 9:
					ScriptType = "Function";
					break;
				}
				ScriptListingDialog->AddScript(ScriptName, FormID.ToString("X8"), ScriptType, Flags);
			}
			void Workspace::AddItemToVariableIndexList(String^% Name, UInt32 Type, UInt32 Index)
			{
				String^ VarType;
				switch (Type)
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

				ListViewItem^ Item = gcnew ListViewItem(Name);
				Item->SubItems->Add(VarType);
				Item->SubItems->Add(Index.ToString());
				VariableIndexList->Items->Add(Item);
			}
			void Workspace::LoadFileFromDisk(String^ Path)
			{
				TextEditor->LoadFileFromDisk(Path, AllocatedIndex);
			}
			void Workspace::SaveScriptToDisk(String^ Path, bool PathIncludesFileName)
			{
				TextEditor->SaveScriptToDisk(Path, PathIncludesFileName, EditorTab->Text, AllocatedIndex);
			}
			bool Workspace::ValidateScript(String^% PreprocessedScriptText)
			{
				StringReader^ ValidateParser = gcnew StringReader(PreprocessedScriptText);
				String^ ReadLine = ValidateParser->ReadLine();
				ScriptParser^ ScriptTextParser = gcnew ScriptParser();
				UInt32 ScriptType = GetScriptType();

				ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Invalid);
				ClearErrorsItemsFromMessagePool();

				bool Result = true;

				while (ReadLine != nullptr)
				{
					ScriptTextParser->Tokenize(ReadLine, false);

					if (!ScriptTextParser->Valid || ScriptTextParser->Tokens[0][0] == ';')
					{
						ScriptTextParser->CurrentLineNo++;
						ReadLine = ValidateParser->ReadLine();
						continue;
					}

					ScriptTextParser->CurrentLineNo++;

					String^ FirstToken = ScriptTextParser->Tokens[0];
					String^ SecondToken = (ScriptTextParser->Tokens->Count > 1)?ScriptTextParser->Tokens[1]:"";
					ScriptParser::TokenType TokenType = ScriptTextParser->GetTokenType(FirstToken);

					switch (TokenType)
					{
					case ScriptParser::TokenType::e_ScriptName:
						if (ScriptTextParser->HasStringGotIllegalChar(SecondToken, "_", ""))
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Identifier '" + SecondToken + "' contains an invalid character."), Result = false;
						if (ScriptTextParser->ScriptName == "")
							ScriptTextParser->ScriptName = SecondToken;
						else
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Redeclaration of script name."), Result = false;
						break;
					case ScriptParser::TokenType::e_Variable:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_Invalid)
						{
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Variable '" + SecondToken + "' declared inside a script block.");
							Result = false;
						}
						if (ScriptTextParser->FindVariable(SecondToken)->IsValid())
							AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Redeclaration of variable '" + SecondToken + "'."), Result = false;
						else
							ScriptTextParser->Variables->AddLast(gcnew ScriptParser::VariableInfo(SecondToken, 0));
						break;
					case ScriptParser::TokenType::e_Begin:
						if (!ScriptTextParser->IsValidBlock(SecondToken, (ScriptParser::ScriptType)ScriptType))
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid script block '" + SecondToken + "' for script type."), Result = false;
						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_ScriptBlock);
						break;
					case ScriptParser::TokenType::e_End:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_ScriptBlock)
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'End' has no matching 'Begin'."), Result = false;
						else
							ScriptTextParser->BlockStack->Pop();
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Command 'End' has an otiose expression following it.");
						break;
					case ScriptParser::TokenType::e_While:
						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
						break;
					case ScriptParser::TokenType::e_ForEach:
						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
						break;
					case ScriptParser::TokenType::e_Loop:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_Loop)
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'Loop' has no matching 'While' or 'ForEach'."), Result = false;
						else
							ScriptTextParser->BlockStack->Pop();
						break;
					case ScriptParser::TokenType::e_If:
						ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_If);
						break;
					case ScriptParser::TokenType::e_ElseIf:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'ElseIf' has no matching 'If'."), Result = false;
						break;
					case ScriptParser::TokenType::e_Else:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'Else' has no matching 'If'."), Result = false;
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Command 'Else' has an otiose expression following it.");
						break;
					case ScriptParser::TokenType::e_EndIf:
						if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
							AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'EndIf' has no matching 'If'."), Result = false;
						else
							ScriptTextParser->BlockStack->Pop();
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Command 'EndIf' has an otiose expression following it.");
						break;
					case ScriptParser::TokenType::e_Return:
						if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
							AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Command 'Return' has an otiose expression following it.");
						break;
					}

																							// increment variable ref count
					UInt32 Pos = 0;
					if (ScriptTextParser->GetTokenType(FirstToken) != ScriptParser::TokenType::e_Variable)
					{
						for each (String^% Itr in ScriptTextParser->Tokens)
						{
							if (ScriptTextParser->FindVariable(Itr)->IsValid())
							{
								if (Pos == 0 || ScriptTextParser->Delimiters[Pos - 1] != '.')
								{
									if (ScriptTextParser->IsComment(Pos) == -1)
										ScriptTextParser->FindVariable(Itr)->RefCount++;
								}
							}
							Pos++;
						}
					}

					ReadLine = ValidateParser->ReadLine();
				}

				for each (ScriptParser::VariableInfo^% Itr in ScriptTextParser->Variables)
				{
					if (Itr->RefCount == 0)
					{
						if ((ScriptParser::ScriptType)ScriptType != ScriptParser::ScriptType::e_Quest || OPTIONS->FetchSettingAsInt("SuppressRefCountForQuestScripts") == 0)
							AddMessageToPool(MessageType::e_Warning, 1, "Variable '" + Itr->VarName + "' unreferenced in local context.");
					}
				}

				if (!Result)
					AddMessageToPool(MessageType::e_Warning, -1, "Compilation of script '" + ScriptTextParser->ScriptName + "' halted - Couldn't recover from previous errors.");

				if (NativeWrapper::LookupFormByEditorID((gcnew CStringWrapper(ScriptTextParser->ScriptName))->String()) && String::Compare(ScriptEditorID, ScriptTextParser->ScriptName, true) != 0)
				{
					if (MessageBox::Show("Script name '" + ScriptTextParser->ScriptName + "' is already used by another form. Proceeding with compilation will modify the script's editorID.\n\nDo you want to proceed?",
						"CSE Editor",
						MessageBoxButtons::YesNo,
						MessageBoxIcon::Exclamation) == DialogResult::No)
					{
						Result = false;
					}
				}

				if (MessageList->Items->Count && MessageList->Visible == false)
					ToolBarMessageList->PerformClick();

				return Result;
			}
			void Workspace::Destroy()
			{
				DestructionFlag = true;
				TextEditor->Destroy();
				ScriptListingDialog->Destroy();
				EditorControlBox->Controls->Clear();
				ParentContainer->RemoveTab(EditorTab);
				ParentContainer->RemoveTabControlBox(EditorControlBox);
				ParentContainer->RedrawContainer();
			}
			UInt16 Workspace::GetScriptType()
			{
				if (ToolBarScriptType->Text == "Object Script")
					return 0;
				else if (ToolBarScriptType->Text == "Quest Script")
					return 1;
				else
					return 2;
			}
			bool Workspace::PreprocessScriptText(String^% PreprocessorResult)
			{
				bool Result = Preprocessor::GetSingleton()->PreprocessScript(TextEditor->GetText(),
														PreprocessorResult,
														gcnew ScriptPreprocessor::StandardOutputError(this, &ScriptEditor::Workspace::PreprocessorErrorOutputWrapper),
														gcnew ScriptEditorPreprocessorData(Globals::AppPath, OPTIONS->FetchSettingAsInt("AllowRedefinitions"), 1));		// OPTIONS->FetchSettingAsInt("NoOfPasses")
				return Result;
			}
			void Workspace::AddMessageToPool(MessageType Type, int Line, String^ Message)
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
				ParentContainer->FlagDestruction(true);
				ParentContainer->RemoveTab(EditorTab);
				ParentContainer->FlagDestruction(false);

				ParentContainer = Destination;
				Destination->AddTab(EditorTab);
				Destination->AddTabControlBox(EditorControlBox);
			}
		#pragma endregion

		#pragma region Event Handlers
			void Workspace::TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E)
			{
				HandlingKeyDownEvent = true;

				switch (E->KeyCode)
				{
				case Keys::O:									// Open script
					if (E->Modifiers == Keys::Control)
						ToolBarOpenScript->PerformClick();
					break;
				case Keys::S:									// Save script
					if (E->Modifiers == Keys::Control)
						PerformCompileAndSave();
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
				case Keys::T:									// Goto Offset
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
					Item->Tag = (int)1;
					DebugPrint("Set the index of variable '" + Item->Text + "' in script '" + EditorTab->Text + "' to " + Index.ToString());
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
				Parameters->VanillaHandleIndex = AllocatedIndex;

				if (HandlingKeyDownEvent)
				{
					Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_New);
					SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
				}
				else
				{
					switch (Control::ModifierKeys)
					{
					case Keys::Control:
						ParentContainer->PerformRemoteOperation(TabContainer::RemoteOperation::e_New, nullptr);
						break;
					case Keys::Shift:
						Parameters->VanillaHandleIndex = 0;
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().X);
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().Y);
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().Width);
						Parameters->ParameterList->Add((UInt32)ParentContainer->GetEditorFormRect().Height);

						SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateTabContainer, Parameters);
						break;
					default:
						Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_New);
						SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
						break;
					}
				}
			}
			void Workspace::ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E)
			{
				if (Control::ModifierKeys == Keys::Control && !HandlingKeyDownEvent)
					ParentContainer->PerformRemoteOperation(TabContainer::RemoteOperation::e_Open, nullptr);
				else
					ScriptListingDialog->Show(ScriptListDialog::Operation::e_Open);
			}
			void Workspace::ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E)
			{
				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->VanillaHandleIndex = AllocatedIndex;
				Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Previous);

				SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
			}
			void Workspace::ToolBarNextScript_Click(Object^ Sender, EventArgs^ E)
			{
				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->VanillaHandleIndex = AllocatedIndex;
				Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Next);

				SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
			}
			void Workspace::ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E)
			{
				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->VanillaHandleIndex = AllocatedIndex;
				Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Save);
				Parameters->ParameterList->Add(ScriptEditorManager::SaveWorkspaceOpType::e_SaveAndCompile);

				SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
			}
			void Workspace::ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E)
			{
				if (GetIsCurrentScriptNew())
				{
					MessageBox::Show("You may only perform this operation on an existing script.", "CSE Editor", MessageBoxButtons::OK, MessageBoxIcon::Information);
					return;
				}

				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->VanillaHandleIndex = AllocatedIndex;
				Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Save);
				Parameters->ParameterList->Add(ScriptEditorManager::SaveWorkspaceOpType::e_SaveButDontCompile);

				SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
			}
			void Workspace::ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E)
			{
				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->VanillaHandleIndex = AllocatedIndex;
				Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Save);
				Parameters->ParameterList->Add(ScriptEditorManager::SaveWorkspaceOpType::e_SaveActivePluginToo);

				SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
			}
			void Workspace::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
			{
				ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
				Parameters->VanillaHandleIndex = AllocatedIndex;
				Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Recompile);

				SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
			}
			void Workspace::ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E)
			{
				if (ScriptEditorID != "" && !GetIsCurrentScriptNew())
				{
					CStringWrapper^ CEID = gcnew CStringWrapper(ScriptEditorID);
					NativeWrapper::ScriptEditor_CompileDependencies(CEID->String());
					MessageBox::Show("Operation complete! Script variables used as condition parameters will need to be corrected manually. The results have been logged to the console.", "CSE Editor", MessageBoxButtons::OK, MessageBoxIcon::Information);
				}
				else
				{
					MessageBox::Show("The current script needs to be compiled before its dependencies can be updated.", "CSE Editor", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				}
			}
			void Workspace::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
			{
				ScriptListingDialog->Show(ScriptListDialog::Operation::e_Delete);
			}
			void Workspace::ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E)
			{
				ParentContainer->NavigateStack(AllocatedIndex, TabContainer::NavigationDirection::e_Back);
			}

			void Workspace::ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E)
			{
				ParentContainer->NavigateStack(AllocatedIndex, TabContainer::NavigationDirection::e_Forward);
			}

			void Workspace::ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E)
			{
				ParentContainer->SaveAllTabs();
			}
			void Workspace::ToolBarOptions_Click(Object^ Sender, EventArgs^ E)
			{
				OPTIONS->LoadINI();
				OPTIONS->Show();
			}

			void Workspace::ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E)
			{
				ToolBarScriptType->Text = ToolBarScriptTypeContentsObject->ToolTipText + " Script";
				ScriptType = 0;
				SetModifiedStatus(true);
			}
			void Workspace::ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E)
			{
				ToolBarScriptType->Text = ToolBarScriptTypeContentsQuest->ToolTipText + " Script";
				ScriptType = 1;
				SetModifiedStatus(true);
			}
			void Workspace::ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E)
			{
				ToolBarScriptType->Text = ToolBarScriptTypeContentsMagicEffect->ToolTipText + " Script";
				ScriptType = 2;
				SetModifiedStatus(true);
			}

			void Workspace::TextEditorContextMenu_Opening(Object^ Sender, CancelEventArgs^ E)
			{
				String^ Token = TextEditor->GetTokenAtMouseLocation()->Replace("\n", "")->Replace("\r\n", "")->Replace("\t", " ");

				if (Token->Length > 20)
					TextEditorContextMenu->Items[7]->Text = Token->Substring(0, 17) + gcnew String("...");
				else
					TextEditorContextMenu->Items[7]->Text = Token;

				ContextMenuDirectLink->Tag = nullptr;
				if (ISDB->IsCommand(Token))						ContextMenuDirectLink->Tag = ISDB->GetCommandURL(Token);

				if (ContextMenuDirectLink->Tag == nullptr)		ContextMenuDirectLink->Visible = false;
				else											ContextMenuDirectLink->Visible = true;

				CStringWrapper^ CTUM = gcnew CStringWrapper(Token);
				ScriptData* Data = NativeWrapper::FetchScriptFromForm(CTUM->String());
				ContextMenuJumpToScript->Visible = true;

				if (Data->IsValid())
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
				else if (ISDB->IsUDF(Token))
				{
					ContextMenuJumpToScript->Text = "Jump to Function script";
					ContextMenuJumpToScript->Tag = Token;
				}
				else
					ContextMenuJumpToScript->Visible = false;
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
				String^ Message = Microsoft::VisualBasic::Interaction::InputBox("Enter the message string",
																				"Add Message", "",
																				SystemInformation::PrimaryMonitorSize.Width / 2,
																				SystemInformation::PrimaryMonitorSize.Height / 2);
				if (Message == "")		return;
				AddMessageToPool(MessageType::e_Message, -1, Message);
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
				ParentContainer->JumpToScript(AllocatedIndex, dynamic_cast<String^>(ContextMenuJumpToScript->Tag));
			}
			void Workspace::ContextMenuGoogleLookup_Click(Object^ Sender, EventArgs^ E)
			{
				Process::Start("http://www.google.com/search?hl=en&source=hp&q=" + TextEditor->GetTokenAtMouseLocation());
			}

			void Workspace::ToolBarCommonTextBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
			{
				switch (E->KeyCode)
				{
				case Keys::Enter:
					if (ToolBarCommonTextBox->Tag->ToString() != "")
					{
						if		(ToolBarCommonTextBox->Tag->ToString() == "Find")				ToolBarEditMenuContentsFind->PerformClick();
						else if (ToolBarCommonTextBox->Tag->ToString() == "Replace")			ToolBarEditMenuContentsReplace->PerformClick();
						else if (ToolBarCommonTextBox->Tag->ToString() == "Goto Line")			ToolBarEditMenuContentsGotoLine->PerformClick();
						else if (ToolBarCommonTextBox->Tag->ToString() == "Goto Offset")		ToolBarEditMenuContentsGotoOffset->PerformClick();
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
					FindReplaceWrapper(ScriptTextEditorInterface::FindReplaceOperation::e_Find);
			}
			void Workspace::ToolBarEditMenuContentsReplace_Click(Object^ Sender, EventArgs^ E)
			{
				if (ToolBarCommonTextBox->Text != "")
					FindReplaceWrapper(ScriptTextEditorInterface::FindReplaceOperation::e_Replace);
			}
			void Workspace::ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E)
			{
				if (ToolBarCommonTextBox->Text != "")
				{
					if (ToolBarShowPreprocessedText->Checked)
						PreprocessedTextViewer->JumpToLine(ToolBarCommonTextBox->Text);
					else if (ToolBarShowOffsets->Checked)
						MessageBox::Show("This operation can only be performed in the text editor and the preprocesed text viewer", "CSE Editor");
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
						MessageBox::Show("This operation can only be performed in the offset viewer", "CSE Editor");
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
				SaveManager->FileName = EditorTab->Text;
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
					ParentContainer->DumpAllTabs(SaveManager->SelectedPath);
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
						ParentContainer->LoadToTab(Itr);
					}
				}
			}
			void Workspace::ToolBarGetVarIndices_Click(Object^ Sender, EventArgs^ E)
			{
				if (!VariableIndexList->Visible)
				{
					if (TextEditor->GetModifiedStatus())
						MessageBox::Show("The current script needs to be compiled before its variable indices can be updated.", "CSE Editor", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
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

						NativeWrapper::ScriptEditor_GetScriptVariableIndices(AllocatedIndex, (gcnew CStringWrapper(ScriptEditorID))->String());
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
				ScriptVarIndexData::ScriptVarInfo Data;
				CStringWrapper^ CScriptName = gcnew CStringWrapper(ScriptEditorID);

				for each (ListViewItem^ Itr in VariableIndexList->Items)
				{
					try
					{
						if (Itr->Tag != nullptr) 
						{
							CStringWrapper^ CEID = gcnew CStringWrapper(Itr->Text);
							UInt32 Index = 0;

							try
							{
								Index = UInt32::Parse(Itr->SubItems[2]->Text);
							} 
							catch (Exception^ E)
							{
								throw gcnew CSEGeneralException("Couldn't parse index of variable  '" + Itr->Text + "' in script '" + EditorTab->Text + "'\n\tError Message: " + E->Message);
							}

							Data.Index = Index;
							if		(!String::Compare(Itr->SubItems[1]->Text, "Integer", true))			Data.Type = 1;
							else if (!String::Compare(Itr->SubItems[1]->Text, "Float", true))			Data.Type = 0;
							else																		Data.Type = 2;
							Data.Name = CEID->String();

							if (!NativeWrapper::ScriptEditor_SetScriptVariableIndex(CScriptName->String(), &Data)) {
								throw gcnew CSEGeneralException("Couldn't update the index of variable '" + Itr->Text + "' in script '" + EditorTab->Text + "'");
							}
						}
					}
					catch (Exception^ E)
					{
						DebugPrint(E->Message, true);
					}
				}

				if (OPTIONS->FetchSettingAsInt("RecompileVarIdx"))
					ToolBarCompileDependencies->PerformClick();

				if (VariableIndexList->Visible)
					ToolBarGetVarIndices->PerformClick();
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
						MessageBox::Show("The preprocessing operation was unsuccessful.", "CSE Editor", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
				if (GetIsCurrentScriptNew())
				{
					MessageBox::Show("You may only perform this operation on an existing script.", "Message - CSE Editor", MessageBoxButtons::OK, MessageBoxIcon::Information);
					return;
				}
				else
				{
					NativeWrapper::ScriptEditor_BindScript((gcnew CStringWrapper(ScriptEditorID))->String(), GetParentContainer()->GetEditorFormHandle());
				}
			}
		#pragma endregion
	#pragma endregion
}