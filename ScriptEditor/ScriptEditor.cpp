#include "ScriptEditor.h"
#include "Common\NativeWrapper.h"

#include "Globals.h"
#include "IntelliSense.h"
#include "ScriptParser.h"
#include "ScriptEditorManager.h"
#include "ScriptListDialog.h"
#include "Common\HandShakeStructs.h"
#include "Common\ListViewUtilities.h"

#using "Microsoft.VisualBasic.dll"
using namespace System::Text::RegularExpressions;

namespace ScriptEditor
{
TabContainer::TabContainer(UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height)
{
	Application::EnableVisualStyles();
	EditorForm = gcnew Form();
	EditorForm->FormBorderStyle = ::FormBorderStyle::Sizable;
	EditorForm->Closing += gcnew CancelEventHandler(this, &TabContainer::EditorForm_Cancel);
	EditorForm->AutoScaleMode = AutoScaleMode::Font;
	EditorForm->Size = Size(Width, Height);	
	

	if (!FileFlags->Images->Count) {
		FileFlags->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(GLOB->ImageResources->GetObject("SEModifiedFlagEx"))));	// unmodified
		FileFlags->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(GLOB->ImageResources->GetObject("SEModifiedFlag"))));		// modified
		FileFlags->ImageSize = Size(12, 12);
	}

	ScriptStrip = gcnew DotNetBar::TabControl();
	ScriptStrip->CanReorderTabs = true;
	ScriptStrip->CloseButtonOnTabsAlwaysDisplayed = false;
	ScriptStrip->CloseButtonOnTabsVisible = true;
	ScriptStrip->Dock = DockStyle::Fill;
	ScriptStrip->Location = Point(0, 0);
	ScriptStrip->Font = gcnew Font("Segoe UI", 7.75F, FontStyle::Regular);
	ScriptStrip->SelectedTabFont = gcnew Font("Segoe UI", 7.75F, FontStyle::Italic);
	ScriptStrip->TabLayoutType = DotNetBar::eTabLayoutType::FixedWithNavigationBox;
	ScriptStrip->TabItemClose += gcnew DotNetBar::TabStrip::UserActionEventHandler(this, &TabContainer::ScriptStrip_TabItemClose);
	ScriptStrip->SelectedTabChanged += gcnew DotNetBar::TabStrip::SelectedTabChangedEventHandler(this, &TabContainer::ScriptStrip_SelectedTabChanged);
	ScriptStrip->SelectedTabChanging += gcnew DotNetBar::TabStrip::SelectedTabChangingEventHandler(this, &TabContainer::ScriptStrip_SelectedTabChanging);
	ScriptStrip->TabRemoved += gcnew EventHandler(this, &TabContainer::ScriptStrip_TabRemoved);
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

	NewTabButton = gcnew DotNetBar::TabItem;
	NewTabButton->Image = GLOB->SENewTabImg;
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
	EditorForm->Show();


	EditorForm->Location = Point(PosX, PosY);

	Destroying = false;
	BackStack = gcnew Stack<UInt32>();
	ForwardStack = gcnew Stack<UInt32>();
	RemovingTab = false;


	const char* EditorID = NativeWrapper::ScriptEditor_GetAuxScriptName();
	if (EditorID)			CreateNewTab(gcnew String(EditorID));
	else					CreateNewTab(nullptr);
}

void TabContainer::EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E)
{	
	if (Destroying)		return;

	E->Cancel = true;
	if (ScriptStrip->Tabs->Count > 2) {
		if (MessageBox::Show("Are you sure you want to close all open tabs?", 
							 "CSE Script Editor",
							 MessageBoxButtons::YesNo, 
							 MessageBoxIcon::Question, 
							 MessageBoxDefaultButton::Button2) == DialogResult::No) {
			return;
		}
	}

	Destroying = true;
	CloseAllTabs();
	Destroying = false;
}

void TabContainer::ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::TabStripActionEventArgs^ E)
{
	Workspace^% Itr = dynamic_cast<Workspace^>(ScriptStrip->SelectedTab->Tag);

	E->Cancel = true;
	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
	Parameters->VanillaHandleIndex = Itr->AllocatedIndex;
	Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Close);
	SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
}

void TabContainer::ScriptStrip_SelectedTabChanging(Object^ Sender, DotNetBar::TabStripTabChangingEventArgs^ E)
{
	if (!RemovingTab && E->NewTab == NewTabButton) {
		E->Cancel = true;
	}
	RemovingTab = false;
}

void TabContainer::ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::TabStripTabChangedEventArgs^ E)
{
	if (ScriptStrip->SelectedTab == nullptr)	return;
	else if (ScriptStrip->SelectedTab == NewTabButton)	{
		if (!ScriptStrip->SelectNextTab())		ScriptStrip->SelectPreviousTab();
		return;
	}

	Workspace^ Itr = dynamic_cast<Workspace^>(ScriptStrip->SelectedTab->Tag);
	EditorForm->Text = Itr->EditorTab->Text + " - CSE Script Editor";
	EditorForm->Focus();
}

void TabContainer::ScriptStrip_TabRemoved(Object^ Sender, EventArgs^ E)
{
	RemovingTab = true;
	if (ScriptStrip->Tabs->Count == 1) {
		if (!Destroying)		CreateNewTab(nullptr);
		else					SEMGR->DestroyTabContainer(this);
	}	
}


UInt32 TabContainer::CreateNewTab(String^ ScriptName)
{
	UInt32 AllocatedIndex = 0;
	if (ScriptName != "") {
		CStringWrapper^ CEID = gcnew CStringWrapper(ScriptName);
		AllocatedIndex = NativeWrapper::ScriptEditor_InstantiateCustomEditor(CEID->String());
	}
	else
		AllocatedIndex = NativeWrapper::ScriptEditor_InstantiateCustomEditor(0);

	if (!AllocatedIndex) {
		DebugPrint("Fatal error occured when allocating a custom editor!", true);
		return AllocatedIndex;
	}

	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
	Parameters->VanillaHandleIndex = 0;
	Parameters->ParameterList->Add(AllocatedIndex);
	Parameters->ParameterList->Add(this);
	SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateWorkspace, Parameters);

	NativeWrapper::ScriptEditor_PostProcessEditorInit(AllocatedIndex);
	ScriptStrip->SelectedTab = (SEMGR->GetAllocatedWorkspace(AllocatedIndex))->EditorTab;
	return AllocatedIndex;
}

void TabContainer::PerformRemoteOperation(RemoteOperation Operation)
{
	UInt32 AllocatedIndex = CreateNewTab(nullptr);
	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();

	switch (Operation)
	{
	case RemoteOperation::e_New:
		Parameters->VanillaHandleIndex = AllocatedIndex;
		Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_New);
		SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
		break;
	case RemoteOperation::e_Open:
		SEMGR->GetAllocatedWorkspace(AllocatedIndex)->ScriptListBox->Show(ScriptListDialog::Operation::e_Open);
		break;
	}
}

void TabContainer::Destroy()
{
	Destroying = true;
	EditorForm->Close();
	BackStack->Clear();
	ForwardStack->Clear();
}

void TabContainer::JumpToScript(UInt32 AllocatedIndex, String^% ScriptName)
{
	UInt32 Count = 0;
	DotNetBar::TabItem^ OpenedWorkspace = nullptr;

	for each (DotNetBar::TabItem^ Itr in ScriptStrip->Tabs) {
		if (!String::Compare(Itr->Text, ScriptName, true)) {
			Count++;
			OpenedWorkspace = Itr;
		}
	}

	if (Count == 1) {
		ScriptStrip->SelectedTab = OpenedWorkspace;
	} else {
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
	if (Itr == Workspace::NullSE) {
		NavigateStack(AllocatedIndex, Direction);
	} else {
		switch (Direction)
		{
		case NavigationDirection::e_Back:
			ForwardStack->Push(AllocatedIndex);
			break;
		case NavigationDirection::e_Forward:
			BackStack->Push(AllocatedIndex);
			break;
		}	
		ScriptStrip->SelectedTab = Itr->EditorTab;
		DebugPrint("Jumping from index " + AllocatedIndex + " to " + JumpIndex);
	}
}

void TabContainer::NewTabButton_Click(Object^ Sender, EventArgs^ E)
{
	CreateNewTab(nullptr);
}

/*void TabContainer::ScriptStrip_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::T:
		if (E->Modifiers == Keys::Control) {
			CreateNewTab(nullptr);
		}
		break;
	}
}*/

void TabContainer::SaveAllTabs()
{
	Workspace^ Editor;

	for each (DotNetBar::TabItem^ Itr in ScriptStrip->Tabs) {
		if (Itr == NewTabButton)	continue;
		Editor = dynamic_cast<Workspace^>(Itr->Tag);
		Editor->ToolBarSaveScript_Click(nullptr, nullptr);
	}
}

void TabContainer::CloseAllTabs()
{
	array<DotNetBar::TabItem^, 1>^ Tabs = gcnew array<DotNetBar::TabItem^, 1>(ScriptStrip->Tabs->Count);
	ScriptStrip->Tabs->CopyTo(Tabs, 0);

	for each (DotNetBar::TabItem^ Itr in Tabs) {
		if (Itr == NewTabButton)	continue;
		Workspace^ Editor = dynamic_cast<Workspace^>(Itr->Tag);

		ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
		Parameters->VanillaHandleIndex = Editor->AllocatedIndex;
		Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Close);
		SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
	}
}





Workspace::Workspace(UInt32 Index, TabContainer^% Parent)
{
	ParentStrip = Parent;
	EditorTab = gcnew DotNetBar::TabItem();
	EditorControlBox = gcnew DotNetBar::TabControlPanel();

	EditorControlBox->Dock = DockStyle::Fill;
	EditorControlBox->Location = Point(0, 26);
	EditorControlBox->Padding = Padding(1);
	EditorControlBox->TabItem = EditorTab;

	EditorTab->AttachedControl = EditorControlBox;
	EditorTab->Text = "New Workspace";
	EditorTab->Tag = this;
	EditorTab->ImageIndex = 0;

	EditorSplitter = gcnew SplitContainer();

	Padding PrimaryButtonPad = Padding(0);
	PrimaryButtonPad.Right = 5;
	Padding SecondaryButtonPad = Padding(0);
	SecondaryButtonPad.Right = 20;
	
	EditorSplitter->Dock = DockStyle::Fill;
	EditorSplitter->FixedPanel = FixedPanel::Panel1;
    EditorSplitter->IsSplitterFixed = true;
    EditorSplitter->SplitterDistance = 48;
    EditorSplitter->SplitterWidth = 1;

	EditorBox = gcnew RichTextBox();

	EditorBox->Font = Globals::GetSingleton()->ScriptEditorOptions->FontSelection->Font;
	EditorBox->Dock = DockStyle::Fill;
	EditorBox->Multiline = true;
	EditorBox->WordWrap = false;
	EditorBox->BorderStyle = BorderStyle::Fixed3D;
	EditorBox->AutoWordSelection = false;

	int TabSize = Decimal::ToInt32(GLOB->ScriptEditorOptions->TabSize->Value);
	if (TabSize) {
		Array^ TabStops = Array::CreateInstance(int::typeid, 32);
		for (int i = 0; i < 32; i++) {
			TabStops->SetValue(TabSize * i, i);
		}
		EditorBox->SelectionTabs = static_cast<array<int>^>(TabStops);
	}

	if (Globals::GetSingleton()->ScriptEditorOptions->ColorEditorBox->Checked) {
		EditorBox->BackColor = Globals::GetSingleton()->ScriptEditorOptions->BCDialog->Color;
		EditorBox->ForeColor = Globals::GetSingleton()->ScriptEditorOptions->FCDialog->Color;
	}		

	EditorBox->TextChanged += gcnew EventHandler(this, &Workspace::EditorBox_TextChanged);
	EditorBox->VScroll += gcnew EventHandler(this, &Workspace::EditorBox_VScroll);
	EditorBox->Resize += gcnew EventHandler(this, &Workspace::EditorBox_Resize);
	EditorBox->KeyUp += gcnew KeyEventHandler(this, &Workspace::EditorBox_KeyUp);
	EditorBox->MouseDown += gcnew MouseEventHandler(this, &Workspace::EditorBox_MouseDown);
	EditorBox->KeyDown += gcnew KeyEventHandler(this, &Workspace::EditorBox_KeyDown);
	EditorBox->KeyPress += gcnew KeyPressEventHandler(this, &Workspace::EditorBox_KeyPress);
	EditorBox->MouseUp += gcnew MouseEventHandler(this, &Workspace::EditorBox_MouseUp);
	EditorBox->MouseDoubleClick += gcnew MouseEventHandler(this, &Workspace::EditorBox_MouseDoubleClick);
	EditorBox->HScroll += gcnew EventHandler(this, &Workspace::EditorBox_HScroll);

	ScriptLineLimitIndicator = gcnew PictureBox();
	ScriptLineLimitIndicator->Image = GLOB->SELineLimitImg;
	ScriptLineLimitIndicator->Visible = false;
	ScriptLineLimitIndicator->BorderStyle = BorderStyle::None;
	ScriptLineLimitIndicator->SizeMode = PictureBoxSizeMode::AutoSize;
	EditorBox->Controls->Add(ScriptLineLimitIndicator);

	EditorLineNo = gcnew RichTextBox();
	EditorLineNo->Font = EditorBox->Font;
	EditorLineNo->Multiline = true;
	EditorLineNo->WordWrap = false;
	EditorLineNo->ReadOnly = true;
	EditorLineNo->Cursor = Cursors::Arrow;
    EditorLineNo->Text = "";
	EditorLineNo->ScrollBars = RichTextBoxScrollBars::None;
	EditorLineNo->BorderStyle = BorderStyle::Fixed3D;
	EditorLineNo->BackColor = Globals::GetSingleton()->ScriptEditorOptions->BCDialog->Color;
	EditorLineNo->ForeColor = Globals::GetSingleton()->ScriptEditorOptions->FCDialog->Color;
	EditorLineNo->MouseDown += gcnew MouseEventHandler(this, &Workspace::EditorLineNo_MouseDown);
	EditorLineNo->SelectionAlignment = HorizontalAlignment::Right;

	EditorBoxSplitter = gcnew SplitContainer();
	
	EditorBoxSplitter->Dock = DockStyle::Fill;
    EditorBoxSplitter->SplitterWidth = 3;
	EditorBoxSplitter->Orientation = Orientation::Horizontal;
	


	EditorToolBar = gcnew ToolStrip(); 
	EditorToolBar->GripStyle = ToolStripGripStyle::Hidden;

	ToolBarCommonTextBox = gcnew ToolStripTextBox();
	ToolBarCommonTextBox->MaxLength = 500;
	ToolBarCommonTextBox->Width = 300;
	ToolBarCommonTextBox->AutoSize = false;
	ToolBarCommonTextBox->LostFocus += gcnew EventHandler(this, &Workspace::ToolBarCommonTextBox_LostFocus);
	ToolBarCommonTextBox->KeyDown += gcnew KeyEventHandler(this, &Workspace::ToolBarCommonTextBox_KeyDown);
	ToolBarCommonTextBox->KeyUp += gcnew KeyEventHandler(this, &Workspace::ToolBarCommonTextBox_KeyUp);
	ToolBarCommonTextBox->KeyPress += gcnew KeyPressEventHandler(this, &Workspace::ToolBarCommonTextBox_KeyPress);
	ToolBarCommonTextBox->Tag = "";

	ToolStripSeparator^ ToolBarSeparatorA = gcnew ToolStripSeparator();
	ToolStripSeparator^ ToolBarSeparatorB = gcnew ToolStripSeparator();

	ToolBarEditMenu = gcnew ToolStripDropDownButton();
		ToolBarEditMenuContents = gcnew ToolStripDropDown();
			ToolBarEditMenuContentsFind = gcnew ToolStripButton();
				ToolBarEditMenuContentsFind->Text = "Find     ";
				ToolBarEditMenuContentsFind->ToolTipText = "Enter your search string in the adjacent textbox.";
				ToolBarEditMenuContentsFind->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsFind_Click);
			ToolBarEditMenuContentsReplace = gcnew ToolStripButton();
				ToolBarEditMenuContentsReplace->Text = "Replace  ";
				ToolBarEditMenuContentsReplace->ToolTipText = "Enter your search string in the adjacent textbox and the replacement string in the messagebox that pops up.";
				ToolBarEditMenuContentsReplace->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsReplace_Click);
			ToolBarEditMenuContentsGotoLine = gcnew ToolStripButton();
				ToolBarEditMenuContentsGotoLine->Text = "Goto Line";
				ToolBarEditMenuContentsGotoLine->ToolTipText = "Enter the line number to jump to in the adjacent textbox.";
				ToolBarEditMenuContentsGotoLine->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsGotoLine_Click);
			ToolBarEditMenuContentsGotoOffset = gcnew ToolStripButton();
				ToolBarEditMenuContentsGotoOffset->Text = "Goto Offset";
				ToolBarEditMenuContentsGotoOffset->ToolTipText = "Enter the offset to jump to in the adjacent textbox, without the hex specifier.";
				ToolBarEditMenuContentsGotoOffset->Click += gcnew EventHandler(this, &Workspace::ToolBarEditMenuContentsGotoOffset_Click);
				
				
		ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsFind);
		ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsReplace);
		ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoLine);
		ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoOffset);
    ToolBarEditMenu->Text = "Edit";
	ToolBarEditMenu->DropDown = ToolBarEditMenuContents;
	ToolBarEditMenu->Margin = SecondaryButtonPad;
	
	ToolBarScriptType = gcnew ToolStripDropDownButton();
		ToolBarScriptTypeContents = gcnew ToolStripDropDown();
			ToolBarScriptTypeContentsObject = gcnew ToolStripButton();
				ToolBarScriptTypeContentsObject->Text = "Object                  ";
				ToolBarScriptTypeContentsObject->ToolTipText = "Object";
				ToolBarScriptTypeContentsObject->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsObject_Click);
			ToolBarScriptTypeContentsQuest = gcnew ToolStripButton();
				ToolBarScriptTypeContentsQuest->Text = "Quest                    ";
				ToolBarScriptTypeContentsQuest->ToolTipText = "Quest";
				ToolBarScriptTypeContentsQuest->Click += gcnew EventHandler(this, &Workspace::ToolBarScriptTypeContentsQuest_Click);
			ToolBarScriptTypeContentsMagicEffect = gcnew ToolStripButton();
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

	ToolStripStatusLabel^ ToolBarSpacerA = gcnew ToolStripStatusLabel();
	ToolBarSpacerA->Spring = true;
	ToolStripStatusLabel^ ToolBarSpacerB = gcnew ToolStripStatusLabel();
	ToolBarSpacerB->Spring = true;

	ToolBarErrorList = gcnew ToolStripButton();
	ToolBarErrorList->ToolTipText = "Error List";
	ToolBarErrorList->Image = Globals::GetSingleton()->SEErrorListImg;
	ToolBarErrorList->AutoSize = true;
	ToolBarErrorList->Click += gcnew EventHandler(this, &Workspace::ToolBarErrorList_Click);
	ToolBarErrorList->Margin = PrimaryButtonPad;

	ToolBarFindList = gcnew ToolStripButton();
	ToolBarFindList->ToolTipText = "Find/Replace Results";
	ToolBarFindList->Image = Globals::GetSingleton()->SEFindListImg;
	ToolBarFindList->AutoSize = true;
	ToolBarFindList->Click += gcnew EventHandler(this, &Workspace::ToolBarFindList_Click);
	ToolBarFindList->Margin = PrimaryButtonPad;

	ToolBarBookmarkList = gcnew ToolStripButton();
	ToolBarBookmarkList->ToolTipText = "Bookmarks";
	ToolBarBookmarkList->Image = Globals::GetSingleton()->SEBookmarkListImg;
	ToolBarBookmarkList->AutoSize = true;
	ToolBarBookmarkList->Click += gcnew EventHandler(this, &Workspace::ToolBarBookmarkList_Click);
	ToolBarBookmarkList->Margin = PrimaryButtonPad;

	ToolBarDumpScript = gcnew ToolStripButton();
	ToolBarDumpScript->ToolTipText = "Dump Script";
	ToolBarDumpScript->Image = Globals::GetSingleton()->SEDumpScriptImg;
	ToolBarDumpScript->AutoSize = true;
	ToolBarDumpScript->Click += gcnew EventHandler(this, &Workspace::ToolBarDumpScript_Click);
	ToolBarDumpScript->Margin = PrimaryButtonPad;
	
	ToolBarLoadScript = gcnew ToolStripButton();
	ToolBarLoadScript->ToolTipText = "Load Script";
	ToolBarLoadScript->Image = Globals::GetSingleton()->SELoadScriptImg;
	ToolBarLoadScript->AutoSize = true;
	ToolBarLoadScript->Click += gcnew EventHandler(this, &Workspace::ToolBarLoadScript_Click);
	ToolBarLoadScript->Margin = SecondaryButtonPad;

	ToolBarOptions = gcnew ToolStripButton();
	ToolBarOptions->ToolTipText = "Preferences";
	ToolBarOptions->Image = Globals::GetSingleton()->SEOptionsImg;
	ToolBarOptions->Click += gcnew EventHandler(this, &Workspace::ToolBarOptions_Click);
	ToolBarOptions->Alignment = ToolStripItemAlignment::Right;


	ToolBarNewScript = gcnew ToolStripButton();
	ToolBarNewScript->ToolTipText = "New Script";
	ToolBarNewScript->Image = Globals::GetSingleton()->SENewImg;
	ToolBarNewScript->AutoSize = true;
	ToolBarNewScript->Click += gcnew EventHandler(this, &Workspace::ToolBarNewScript_Click);
	ToolBarNewScript->Margin = SecondaryButtonPad;

	ToolBarOpenScript = gcnew ToolStripButton();
	ToolBarOpenScript->ToolTipText = "Open Script";
	ToolBarOpenScript->Image = Globals::GetSingleton()->SEOpenImg;
	ToolBarOpenScript->AutoSize = true;
	ToolBarOpenScript->Click += gcnew EventHandler(this, &Workspace::ToolBarOpenScript_Click);
	ToolBarOpenScript->Margin = SecondaryButtonPad;

	ToolBarPreviousScript = gcnew ToolStripButton();
	ToolBarPreviousScript->ToolTipText = "Previous Script";
	ToolBarPreviousScript->Image = Globals::GetSingleton()->SEPreviousImg;
	ToolBarPreviousScript->AutoSize = true;
	ToolBarPreviousScript->Click += gcnew EventHandler(this, &Workspace::ToolBarPreviousScript_Click);
	ToolBarPreviousScript->Margin = PrimaryButtonPad;

	ToolBarNextScript = gcnew ToolStripButton();
	ToolBarNextScript->ToolTipText = "Next Script";
	ToolBarNextScript->Image = Globals::GetSingleton()->SENextImg;
	ToolBarNextScript->AutoSize = true;
	ToolBarNextScript->Click += gcnew EventHandler(this, &Workspace::ToolBarNextScript_Click);
	ToolBarNextScript->Margin = SecondaryButtonPad;

	ToolBarSaveScript = gcnew ToolStripButton();
	ToolBarSaveScript->ToolTipText = "Save Script";
	ToolBarSaveScript->Image = Globals::GetSingleton()->SESaveImg;
	ToolBarSaveScript->AutoSize = true;
	ToolBarSaveScript->Click += gcnew EventHandler(this, &Workspace::ToolBarSaveScript_Click);
	ToolBarSaveScript->Margin = SecondaryButtonPad;

	ToolBarRecompileScripts = gcnew ToolStripButton();
	ToolBarRecompileScripts->ToolTipText = "Recompile Scripts";
	ToolBarRecompileScripts->Image = Globals::GetSingleton()->SERecompileImg;
	ToolBarRecompileScripts->AutoSize = true;
	ToolBarRecompileScripts->Click += gcnew EventHandler(this, &Workspace::ToolBarRecompileScripts_Click);
	ToolBarRecompileScripts->Margin = SecondaryButtonPad;

	ToolBarDeleteScript = gcnew ToolStripButton();
	ToolBarDeleteScript->ToolTipText = "Delete Script";
	ToolBarDeleteScript->Image = Globals::GetSingleton()->SEDeleteImg;
	ToolBarDeleteScript->AutoSize = true;
	ToolBarDeleteScript->Click += gcnew EventHandler(this, &Workspace::ToolBarDeleteScript_Click);
	ToolBarDeleteScript->Margin = PrimaryButtonPad;

	ToolBarOffsetToggle = gcnew ToolStripButton();
	ToolBarOffsetToggle->ToolTipText = "Toggle Offsets";
	ToolBarOffsetToggle->Image = Globals::GetSingleton()->SEOffsetImg;
	ToolBarOffsetToggle->AutoSize = true;
	ToolBarOffsetToggle->Click += gcnew EventHandler(this, &Workspace::ToolBarOffsetToggle_Click);
	Padding OffsetPad = Padding(0);
	OffsetPad.Left = 17;
	OffsetPad.Right = 17;
	ToolBarOffsetToggle->Margin = OffsetPad;
	ToolBarOffsetToggle->Enabled = false;


	ToolBarNavigationBack = gcnew ToolStripButton();
	ToolBarNavigationBack->ToolTipText = "Navigate Back";
	ToolBarNavigationBack->Image = Globals::GetSingleton()->SENavBackImg;
	ToolBarNavigationBack->AutoSize = true;
	ToolBarNavigationBack->Click += gcnew EventHandler(this, &Workspace::ToolBarNavBack_Click);
	ToolBarNavigationBack->Margin = PrimaryButtonPad;
	ToolBarNavigationBack->Alignment = ToolStripItemAlignment::Right;

	ToolBarNavigationForward = gcnew ToolStripButton();
	ToolBarNavigationForward->ToolTipText = "Navigate Forward";
	ToolBarNavigationForward->Image = Globals::GetSingleton()->SENavForwardImg;
	ToolBarNavigationForward->AutoSize = true;
	ToolBarNavigationForward->Click += gcnew EventHandler(this, &Workspace::ToolBarNavForward_Click);
	ToolBarNavigationForward->Margin = SecondaryButtonPad;
	ToolBarNavigationForward->Alignment = ToolStripItemAlignment::Right;

	ToolBarByteCodeSize = gcnew ToolStripProgressBar();
	ToolBarByteCodeSize->Minimum = 0;
	ToolBarByteCodeSize->Maximum = 0x4000;
	ToolBarByteCodeSize->AutoSize = false;
	ToolBarByteCodeSize->Size = Size(85, 13);
	ToolBarByteCodeSize->ToolTipText = "Compiled Script Size";
	ToolBarByteCodeSize->Alignment = ToolStripItemAlignment::Right;
	ToolBarByteCodeSize->Margin = SecondaryButtonPad;

	
	ToolBarGetVarIndices = gcnew ToolStripButton();
	ToolBarGetVarIndices->ToolTipText = "Get Variable Indices";
	ToolBarGetVarIndices->Image = Globals::GetSingleton()->SEVarIdxListImg;
	ToolBarGetVarIndices->AutoSize = true;
	ToolBarGetVarIndices->Click += gcnew EventHandler(this, &Workspace::ToolBarGetVarIndices_Click);
	ToolBarGetVarIndices->Margin = PrimaryButtonPad;

	ToolBarUpdateVarIndices = gcnew ToolStripButton();
	ToolBarUpdateVarIndices->ToolTipText = "Update Variable Indices";
	ToolBarUpdateVarIndices->Image = Globals::GetSingleton()->SEVarIdxUpdateImg;
	ToolBarUpdateVarIndices->AutoSize = true;
	ToolBarUpdateVarIndices->Click += gcnew EventHandler(this, &Workspace::ToolBarUpdateVarIndices_Click);
	ToolBarUpdateVarIndices->Margin = PrimaryButtonPad;

	ToolBarSaveAll = gcnew ToolStripButton();
	ToolBarSaveAll->ToolTipText = "Save all open scripts";
	ToolBarSaveAll->Image = Globals::GetSingleton()->SESaveAllImg;
	ToolBarSaveAll->AutoSize = true;
	ToolBarSaveAll->Click += gcnew EventHandler(this, &Workspace::ToolBarSaveAll_Click);
	ToolBarSaveAll->Margin = SecondaryButtonPad;
	ToolBarSaveAll->Alignment = ToolStripItemAlignment::Right;
	
	EditorToolBar->Dock = DockStyle::Top;
	EditorToolBar->Items->Add(ToolBarNewScript);
	EditorToolBar->Items->Add(ToolBarOpenScript);
	EditorToolBar->Items->Add(ToolBarSaveScript);
	EditorToolBar->Items->Add(ToolBarPreviousScript);
	EditorToolBar->Items->Add(ToolBarNextScript);
	EditorToolBar->Items->Add(ToolBarRecompileScripts);
	EditorToolBar->Items->Add(ToolBarDeleteScript);
	EditorToolBar->Items->Add(ToolBarScriptType);
	EditorToolBar->Items->Add(ToolBarSpacerA);
	EditorToolBar->Items->Add(ToolBarOptions);
	EditorToolBar->Items->Add(ToolBarNavigationForward);
	EditorToolBar->Items->Add(ToolBarNavigationBack);
	EditorToolBar->Items->Add(ToolBarSaveAll);
	EditorToolBar->ShowItemToolTips = true;

	BoxToolBar = gcnew ToolStrip(); 
	BoxToolBar->GripStyle = ToolStripGripStyle::Hidden;
	BoxToolBar->Dock = DockStyle::Top;
	BoxToolBar->Items->Add(ToolBarOffsetToggle);
	BoxToolBar->Items->Add(ToolBarCommonTextBox);
	BoxToolBar->Items->Add(ToolBarEditMenu);
	BoxToolBar->Items->Add(ToolBarErrorList);
	BoxToolBar->Items->Add(ToolBarFindList);
	BoxToolBar->Items->Add(ToolBarBookmarkList);
	BoxToolBar->Items->Add(ToolBarDumpScript);
	BoxToolBar->Items->Add(ToolBarLoadScript);
	BoxToolBar->Items->Add(ToolBarGetVarIndices);
	BoxToolBar->Items->Add(ToolBarUpdateVarIndices);
	BoxToolBar->Items->Add(ToolBarSpacerB);
	BoxToolBar->Items->Add(ToolBarByteCodeSize);	
	BoxToolBar->ShowItemToolTips = true;
	

	EditorContextMenu = gcnew ContextMenuStrip();
		ContextMenuCopy = gcnew ToolStripMenuItem();
			ContextMenuCopy->Click += gcnew EventHandler(this, &Workspace::ContextMenuCopy_Click);
			ContextMenuCopy->Text = "Copy";
			ContextMenuCopy->Image = GLOB->SEContextCopyImg;
		ContextMenuPaste = gcnew ToolStripMenuItem();
			ContextMenuPaste->Click += gcnew EventHandler(this, &Workspace::ContextMenuPaste_Click);
			ContextMenuPaste->Text = "Paste";
			ContextMenuPaste->Image = GLOB->SEContextPasteImg;
		ContextMenuWord = gcnew ToolStripMenuItem();
			ContextMenuWord->Enabled = false;
		ContextMenuWikiLookup = gcnew ToolStripMenuItem();
			ContextMenuWikiLookup->Click += gcnew EventHandler(this, &Workspace::ContextMenuWikiLookup_Click);
			ContextMenuWikiLookup->Text = "Look up on the Wiki";
			ContextMenuWikiLookup->Image = GLOB->SEContextLookupImg;
		ContextMenuOBSEDocLookup = gcnew ToolStripMenuItem();
			ContextMenuOBSEDocLookup->Click += gcnew EventHandler(this, &Workspace::ContextMenuOBSEDocLookup_Click);
			ContextMenuOBSEDocLookup->Text = "Look up on the OBSE Doc";
		ContextMenuCopyToCTB = gcnew ToolStripMenuItem();
			ContextMenuCopyToCTB->Click += gcnew EventHandler(this, &Workspace::ContextMenuCopyToCTB_Click);
			ContextMenuCopyToCTB->Text = "Copy to Edit Box";
			ContextMenuCopyToCTB->Image = GLOB->SEContextCTBImg;
		ContextMenuFind = gcnew ToolStripMenuItem();
			ContextMenuFind->Click += gcnew EventHandler(this, &Workspace::ContextMenuFind_Click);
			ContextMenuFind->Text = "Find";
			ContextMenuFind->Image = GLOB->SEContextFindImg;
		ContextMenuToggleComment = gcnew ToolStripMenuItem();
			ContextMenuToggleComment->Click += gcnew EventHandler(this, &Workspace::ContextMenuToggleComment_Click);
			ContextMenuToggleComment->Text = "Toggle Comment";
			ContextMenuToggleComment->Image = GLOB->SEContextCommentImg;
		ContextMenuToggleBookmark = gcnew ToolStripMenuItem();
			ContextMenuToggleBookmark->Click += gcnew EventHandler(this, &Workspace::ContextMenuToggleBookmark_Click);
			ContextMenuToggleBookmark->Text = "Toggle Bookmark";
			ContextMenuToggleBookmark->Image = GLOB->SEContextBookmarkImg;
		ContextMenuDirectLink = gcnew ToolStripMenuItem();
			ContextMenuDirectLink->Click += gcnew EventHandler(this, &Workspace::ContextMenuDirectLink_Click);
			ContextMenuDirectLink->Text = "Developer Page";
			ContextMenuDirectLink->Image = GLOB->SEContextDevLinkImg;
		ContextMenuJumpToScript = gcnew ToolStripMenuItem();
			ContextMenuJumpToScript->Click += gcnew EventHandler(this, &Workspace::ContextMenuJumpToScript_Click);
			ContextMenuJumpToScript->Text = "Jump into Script";
			ContextMenuJumpToScript->Image = GLOB->SEContextJumpImg;
		
			

	EditorContextMenu->Items->Add(ContextMenuCopy);
	EditorContextMenu->Items->Add(ContextMenuPaste);
	EditorContextMenu->Items->Add(ContextMenuFind);
	EditorContextMenu->Items->Add(ContextMenuToggleComment);
	EditorContextMenu->Items->Add(ContextMenuToggleBookmark);
	EditorContextMenu->Items->Add(ToolBarSeparatorA);
	EditorContextMenu->Items->Add(ContextMenuWord);
	EditorContextMenu->Items->Add(ContextMenuCopyToCTB);
	EditorContextMenu->Items->Add(ContextMenuWikiLookup);
	EditorContextMenu->Items->Add(ContextMenuOBSEDocLookup);
	EditorContextMenu->Items->Add(ContextMenuDirectLink);
	EditorContextMenu->Items->Add(ContextMenuJumpToScript);

	EditorContextMenu->Opening += gcnew CancelEventHandler(this, &Workspace::EditorContextMenu_Opening);

	if (!MessageIcon->Images->Count) {
		MessageIcon->TransparentColor = Color::White;
		MessageIcon->Images->Add(GLOB->SEWarningImg);
		MessageIcon->Images->Add(GLOB->SEErrorImg);
	}

	ErrorBox = gcnew ListView();
	ErrorBox->Font = Globals::GetSingleton()->ScriptEditorOptions->FontSelection->Font;
	ErrorBox->Dock = DockStyle::Fill;
	ErrorBox->BorderStyle = BorderStyle::Fixed3D;
	ErrorBox->BackColor = Globals::GetSingleton()->ScriptEditorOptions->BCDialog->Color;
	ErrorBox->ForeColor = Globals::GetSingleton()->ScriptEditorOptions->FCDialog->Color;
	ErrorBox->DoubleClick += gcnew EventHandler(this, &Workspace::ErrorBox_DoubleClick);
	ErrorBox->Visible = false;
	ErrorBox->View = View::Details;
	ErrorBox->MultiSelect = false;
	ErrorBox->CheckBoxes = false;
	ErrorBox->FullRowSelect = true;
	ErrorBox->HideSelection = false;
	ErrorBox->SmallImageList = MessageIcon;

	ColumnHeader^ ErrorBoxType = gcnew ColumnHeader();
	ErrorBoxType->Text = " ";
	ErrorBoxType->Width = 25;
	ColumnHeader^ ErrorBoxLine = gcnew ColumnHeader();
	ErrorBoxLine->Text = "Line";
	ErrorBoxLine->Width = 50;
	ColumnHeader^ ErrorBoxMessage = gcnew ColumnHeader();
	ErrorBoxMessage->Text = "Message";
	ErrorBoxMessage->Width = 300;
	ErrorBox->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(3) {ErrorBoxType, 
																		 ErrorBoxLine,
																		 ErrorBoxMessage});
	ErrorBox->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::ErrorBox_ColumnClick);
	ErrorBox->Tag = (int)1;


	FindBox = gcnew ListView();
	FindBox->Font = Globals::GetSingleton()->ScriptEditorOptions->FontSelection->Font;
	FindBox->Dock = DockStyle::Fill;
	FindBox->BorderStyle = BorderStyle::Fixed3D;
	FindBox->BackColor = Globals::GetSingleton()->ScriptEditorOptions->BCDialog->Color;
	FindBox->ForeColor = Globals::GetSingleton()->ScriptEditorOptions->FCDialog->Color;
	FindBox->DoubleClick += gcnew EventHandler(this, &Workspace::FindBox_DoubleClick);	
	FindBox->Visible = false;
	FindBox->View = View::Details;
	FindBox->MultiSelect = false;
	FindBox->CheckBoxes = false;
	FindBox->FullRowSelect = true;
	FindBox->HideSelection = false;

	ColumnHeader^ FindBoxLine = gcnew ColumnHeader();
	FindBoxLine->Text = "Line";
	FindBoxLine->Width = 50;
	ColumnHeader^ FindBoxLineContent = gcnew ColumnHeader();
	FindBoxLineContent->Text = "Text";
	FindBoxLineContent->Width = 300;
	FindBox->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(2) {FindBoxLine,
																		 FindBoxLineContent});
	FindBox->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::FindBox_ColumnClick);
	FindBox->Tag = (int)1;

	BookmarkBox = gcnew ListView();
	BookmarkBox->Font = Globals::GetSingleton()->ScriptEditorOptions->FontSelection->Font;
	BookmarkBox->Dock = DockStyle::Fill;
	BookmarkBox->BorderStyle = BorderStyle::Fixed3D;
	BookmarkBox->BackColor = Globals::GetSingleton()->ScriptEditorOptions->BCDialog->Color;
	BookmarkBox->ForeColor = Globals::GetSingleton()->ScriptEditorOptions->FCDialog->Color;
	BookmarkBox->DoubleClick += gcnew EventHandler(this, &Workspace::BookmarkBox_DoubleClick);	
	BookmarkBox->Visible = false;
	BookmarkBox->View = View::Details;
	BookmarkBox->MultiSelect = false;
	BookmarkBox->CheckBoxes = false;
	BookmarkBox->FullRowSelect = true;
	BookmarkBox->HideSelection = false;
	BookmarkBox->Tag = (int)1;

	ColumnHeader^ BookmarkBoxLine = gcnew ColumnHeader();
	BookmarkBoxLine->Text = "Line";
	BookmarkBoxLine->Width = 50;
	ColumnHeader^ BookmarkBoxDesc = gcnew ColumnHeader();
	BookmarkBoxDesc->Text = "Description";
	BookmarkBoxDesc->Width = 300;
	BookmarkBox->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(2) {BookmarkBoxLine,
																		 BookmarkBoxDesc});
	BookmarkBox->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::BookmarkBox_ColumnClick);

	VariableBox = gcnew ListView();
	VariableBox->Font = Globals::GetSingleton()->ScriptEditorOptions->FontSelection->Font;
	VariableBox->Dock = DockStyle::Fill;
	VariableBox->BorderStyle = BorderStyle::Fixed3D;
	VariableBox->BackColor = Globals::GetSingleton()->ScriptEditorOptions->BCDialog->Color;
	VariableBox->ForeColor = Globals::GetSingleton()->ScriptEditorOptions->FCDialog->Color;
	VariableBox->DoubleClick += gcnew EventHandler(this, &Workspace::VariableBox_DoubleClick);	
	VariableBox->Visible = false;
	VariableBox->View = View::Details;
	VariableBox->MultiSelect = false;
	VariableBox->CheckBoxes = false;
	VariableBox->FullRowSelect = true;
	VariableBox->HideSelection = false;
	VariableBox->Tag = (int)1;

	ColumnHeader^ VariableBoxName = gcnew ColumnHeader();
	VariableBoxName->Text = "Variable Name";
	VariableBoxName->Width = 300;
	ColumnHeader^ VariableBoxType = gcnew ColumnHeader();
	VariableBoxType->Text = "Type";
	VariableBoxType->Width = 300;
	ColumnHeader^ VariableBoxIndex = gcnew ColumnHeader();
	VariableBoxIndex->Text = "Index";
	VariableBoxIndex->Width = 100;
	VariableBox->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(3) {VariableBoxName,
																		 VariableBoxType,
																		 VariableBoxIndex});
	VariableBox->ColumnClick += gcnew ColumnClickEventHandler(this, &Workspace::VariableBox_ColumnClick);

	IndexEditBox = gcnew TextBox();
	IndexEditBox->Font = Globals::GetSingleton()->ScriptEditorOptions->FontSelection->Font;
	IndexEditBox->Multiline = true;
	IndexEditBox->BorderStyle = BorderStyle::FixedSingle;
	IndexEditBox->Visible = false;
	IndexEditBox->AcceptsReturn = true;
	IndexEditBox->LostFocus += gcnew EventHandler(this, &Workspace::IndexEditBox_LostFocus);
	IndexEditBox->KeyDown += gcnew KeyEventHandler(this, &Workspace::IndexEditBox_KeyDown);

	VariableBox->Controls->Add(IndexEditBox);

	EditorSplitter->Panel1->Controls->Add(EditorLineNo);
	EditorSplitter->Panel1->BorderStyle = BorderStyle::Fixed3D;
	EditorSplitter->Panel2->Controls->Add(EditorBox);

	EditorBoxSplitter->Panel1->Controls->Add(EditorSplitter);
	EditorBoxSplitter->Panel2->Controls->Add(BoxToolBar);
	EditorBoxSplitter->Panel2->Controls->Add(ErrorBox);
	EditorBoxSplitter->Panel2->Controls->Add(FindBox);
	EditorBoxSplitter->Panel2->Controls->Add(BookmarkBox);
	EditorBoxSplitter->Panel2->Controls->Add(VariableBox);

	ISBox = gcnew SyntaxBox(const_cast<ScriptEditor::Workspace^>(this));

	EditorControlBox->Controls->Add(EditorBoxSplitter);
	EditorControlBox->Controls->Add(EditorToolBar);

	Parent->ScriptStrip->Tabs->Add(EditorTab);
	Parent->ScriptStrip->Controls->Add(EditorControlBox);

	EditorBox->Controls->Add(ISBox->IntelliSenseList);
	EditorBox->ContextMenuStrip = EditorContextMenu;

	ISBox->Hide();
	EditorBoxSplitter->SplitterDistance = ((ParentStrip->EditorForm->Height - 65 < 0)?500 : ParentStrip->EditorForm->Height - 65);

	EditorBoxSplitter->Enabled = false;
	ToolBarUpdateVarIndices->Enabled = false;

	AllocatedIndex = Index;
	HasChanged = false;
	TextSet = false;
	Destroying = false;
	HandleKeyEditorBox = Keys::None;
	HandleKeyCTB = Keys::None;
	HandleTextChanged = true;
	Indents = 0;
	ScriptTextParser = gcnew ScriptParser();
	IndexPointers = gcnew List<PictureBox^>();
	CurrentLineNo = 0;
	LineOffsets = gcnew List<UInt16>();
	GetVariableData = false;

	ScriptListBox = gcnew ScriptListDialog(AllocatedIndex);
}

Workspace::Workspace(UInt32 Index)
{
	AllocatedIndex = Index;
}



void Workspace::Destroy()
{
	Destroying = true;
	ScriptListBox->ScriptBox->Close();
	EditorControlBox->Controls->Clear();
	ParentStrip->ScriptStrip->Tabs->Remove(EditorTab);
}

void Workspace::EnableControls()
{
	EditorBoxSplitter->Enabled = true;
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

int Workspace::FindLineNumberInLineBox(UInt32 Line)		// Parameter is one-based, as opposed to a zero-based index
{
	if (ToolBarOffsetToggle->Checked && Line - 1 < EditorBox->Lines->Length) {
		UInt32 Offset = LineOffsets[Line - 1];
		if (Offset == 0xFFFF)	return -1;
		else {
			return EditorLineNo->Find(Offset.ToString("X4"), 0, RichTextBoxFinds::WholeWord);
		}
	} else
		return EditorLineNo->Find(Line.ToString(), 0, EditorLineNo->Text->Length, RichTextBoxFinds::WholeWord);
}


void Workspace::PerformLineNumberHighlights(void)
{
	if (EditorBox->Text != "") {
		int LineField = 0, CurrentLine = 0;
		Font^ BoldStyle = gcnew Font(EditorLineNo->Font->FontFamily, EditorLineNo->Font->Size, FontStyle::Bold);

		for each (ListViewItem^% Itr in BookmarkBox->Items) {
			if (FindLineNumberInLineBox(int::Parse(Itr->SubItems[0]->Text)) != -1) {
				EditorLineNo->SelectionColor = Globals::GetSingleton()->ScriptEditorOptions->BMCDialog->Color;
				EditorLineNo->SelectionFont = BoldStyle;
			}			
		}

		CurrentLine = EditorBox->GetLineFromCharIndex(EditorBox->SelectionStart) + 1;
		if (FindLineNumberInLineBox(CurrentLine) != -1) {
			EditorLineNo->SelectionColor = Globals::GetSingleton()->ScriptEditorOptions->HCDialog->Color;
			EditorLineNo->SelectionFont = BoldStyle;
		}
	}
}

void Workspace::UpdateLineNumbers(void)
{
    int FirstLine = EditorBox->GetLineFromCharIndex(EditorBox->GetCharIndexFromPosition(Point(0, 0)));
    int LastLine = EditorBox->GetLineFromCharIndex(EditorBox->GetCharIndexFromPosition(Point(EditorBox->Width, EditorBox->Height)));

	EditorLineNo->Clear();
	EditorLineNo->SelectionColor = Globals::GetSingleton()->ScriptEditorOptions->FCDialog->Color;
	EditorLineNo->SelectionFont = EditorLineNo->Font;

	for (int i = FirstLine + 1; i <= LastLine + 2; i++) {
		if (i <= EditorBox->Lines->Length) {
			if (ToolBarOffsetToggle->Checked) {
				try {
					UInt32 Offset = LineOffsets[i - 1];
					EditorLineNo->Text += ((Offset == 0xFFFF)? "":Offset.ToString("X4")) + "\n";
				} catch (...) {
					EditorLineNo->Text += "----" + "\n";
				}
			} else
				EditorLineNo->Text += i + "\n";
		}
    }

	PerformLineNumberHighlights();
}


String^ Workspace::GetTextAtLoc(Point Loc, bool FromMouse, bool SelectText, int Index, bool ReplaceLineBreaks)
{
	if (EditorBox->SelectedText != "" && FromMouse) {
		if (ReplaceLineBreaks)		return EditorBox->SelectedText->Replace("\n", "");
		else						return EditorBox->SelectedText;
	}

	Index =	(Index == -1) ? EditorBox->GetCharIndexFromPosition(Loc) : Index;

	String^% Source = EditorBox->Text;
	int SearchIndex = Source->Length, SubStrStart = 0, SubStrEnd = SearchIndex;

	for (int i = Index; i > 0; i--) {
		if (Globals::GetSingleton()->Delimiters->IndexOf(Source[i]) != -1) {
			SubStrStart = i + 1;
			break;
		}
	}

	for (int i = Index; i < SearchIndex; i++) {
		if (Globals::GetSingleton()->Delimiters->IndexOf(Source[i]) != -1) {
			SubStrEnd = i;
			break;
		}
	}

	if (SubStrStart > SubStrEnd) return "";
	else { 
		if (SelectText) {
			EditorBox->SelectionStart = SubStrStart;
			EditorBox->SelectionLength = SubStrEnd - SubStrStart;
		}

		if (ReplaceLineBreaks)	return Source->Substring(SubStrStart, SubStrEnd - SubStrStart)->Replace("\n", "");		
		else					return Source->Substring(SubStrStart, SubStrEnd - SubStrStart);
	}
}

void Workspace::ClearFindImagePointers(void)
{
	for each (PictureBox^% Itr in IndexPointers) {
		Itr->Hide();
	}
	IndexPointers->Clear();
}

void Workspace::PlaceFindImagePointer(int Index)
{
	IndexPointers->Add(gcnew PictureBox());
	PictureBox^% IP = IndexPointers[IndexPointers->Count - 1];
	IP->BorderStyle = BorderStyle::None;
	IP->SizeMode = PictureBoxSizeMode::AutoSize;
	IP->Location = Point(EditorBox->GetPositionFromCharIndex(Index).X, EditorBox->GetPositionFromCharIndex(Index).Y - 25);
	IP->Tag = String::Format("{0}", Index);
	IP->Image = dynamic_cast<Image^>(Globals::GetSingleton()->PosPointerImg);
	EditorBox->Controls->Add(IP);
}

void Workspace::FindAndReplace(bool Replace)
{
	try {
		NativeWrapper::LockWindowUpdate(EditorBox->Handle);
		ClearFindImagePointers();
		String^ SearchString = ToolBarCommonTextBox->Text;
		String^ ReplaceString = "NULL";
		FindBox->Items->Clear();
		bool UseRegEx = Globals::GetSingleton()->ScriptEditorOptions->UseRegEx->Checked;

		if (Replace)
			ReplaceString = Microsoft::VisualBasic::Interaction::InputBox("Enter replace string.", "Find and Replace", "", EditorBox->Location.X + EditorBox->Width / 2, EditorBox->Location.Y + EditorBox->Height / 2);

		if (SearchString == "" || ReplaceString == "") {
			MessageBox::Show("Enter a valid search/replace string.", "Find and Replace");
			return;
		}

		int Hits = 0, SelStart = EditorBox->SelectionStart, SelLength = SearchString->Length, Pos = -1;
		UseRegEx = false;

		if (UseRegEx) {
			;//
		} else {
			Pos = EditorBox->Find(SearchString, 0, EditorBox->Text->Length - 1, RichTextBoxFinds::NoHighlight);

			while (Pos != -1) {
				PlaceFindImagePointer(Pos);

				if (Replace) {
					HandleTextChanged = false;
					EditorBox->SelectionStart = Pos;
					EditorBox->SelectionLength = SearchString->Length;
					EditorBox->SelectedText = ReplaceString;
				}

				ListViewItem^ Item = gcnew ListViewItem((EditorBox->GetLineFromCharIndex(Pos) + 1).ToString());
				Item->SubItems->Add(EditorBox->Lines[EditorBox->GetLineFromCharIndex(Pos)]->Replace("\t", ""));
				FindBox->Items->Add(Item);

				Pos = EditorBox->Find(SearchString, Pos + SearchString->Length, EditorBox->Text->Length - 1, RichTextBoxFinds::NoHighlight);
				++Hits;
			}
		}

		if (Hits > 0 && FindBox->Visible == false)
			ToolBarFindList_Click(nullptr, nullptr);

		MessageBox::Show(String::Format("{0} hits in the current script.", Hits), "Find and Replace");
	} finally {
		NativeWrapper::LockWindowUpdate(IntPtr::Zero);
	}
}

void Workspace::JumpToLine(String^ LineStr, bool OffsetSearch)
{
	int LineNo = 0, Count = 0;
	try { 
		if (!OffsetSearch)		LineNo = Int32::Parse(LineStr); 
		else {
			UInt16 Offset = UInt16::Parse(LineStr, System::Globalization::NumberStyles::HexNumber);
			for each (UInt16 Itr in LineOffsets) {
				if (Itr == Offset) {
					LineNo = Count + 1;
					break;
				}
				++Count;
			}
		}
	} catch (...) { return;}


	if (LineNo > EditorBox->Lines->Length || !LineNo) {
		if (!OffsetSearch)
			MessageBox::Show("The line to jump to must be less than or equal to " + EditorBox->Lines->Length + ".", "Goto Line");
		else
			MessageBox::Show("Unknown offset.", "Goto Line");
	}
	else {
		--LineNo;
		EditorBox->Focus();
		EditorBox->SelectionStart = EditorBox->GetFirstCharIndexFromLine(LineNo);
		EditorBox->SelectionLength = EditorBox->Lines[LineNo]->Length + 1;
		EditorBox->ScrollToCaret();
	}
}

int Workspace::CalculateIndents(int EndPos, bool& ExdentLine, bool CullEmptyLines)
{
	int Indents = 0;

	if (!Globals::GetSingleton()->ScriptEditorOptions->AutoIndent->Checked)
		return Indents;

	ExdentLine = false;
	StringReader^ IndentParser = gcnew StringReader(EditorBox->Text->Substring(0, EndPos));
	String^ ReadLine = IndentParser->ReadLine(), ^ReadLineEx;

	while (ReadLine != nullptr) {
		ScriptTextParser->Tokenize(ReadLine, false);
		if (!ScriptTextParser->Valid) {
			ReadLineEx = ReadLine;
			ReadLine = IndentParser->ReadLine();
			continue;
		}

		if (!String::Compare(ScriptTextParser->Tokens[0], "begin", true) || 
			!String::Compare(ScriptTextParser->Tokens[0], "if", true) || 
			!String::Compare(ScriptTextParser->Tokens[0], "while", true) || 
			!String::Compare(ScriptTextParser->Tokens[0], "forEach", true)) {
			++Indents;
		}
		else if	(!String::Compare(ScriptTextParser->Tokens[0], "loop", true) || 
				!String::Compare(ScriptTextParser->Tokens[0], "endIf", true) || 
				!String::Compare(ScriptTextParser->Tokens[0], "end", true)) {
			--Indents;
		}

		ReadLineEx = ReadLine;
		ReadLine = IndentParser->ReadLine();
	}

	if (ReadLineEx != nullptr) {		// last line needs to be checked separately for exdents
		ReadLine = ReadLineEx;
		ScriptTextParser->Tokenize(ReadLine, false);
		if (ScriptTextParser->Valid) {
			if (!String::Compare(ScriptTextParser->Tokens[0], "loop", true) || 
				!String::Compare(ScriptTextParser->Tokens[0], "endIf", true) || 
				!String::Compare(ScriptTextParser->Tokens[0], "end", true)) {
				ExdentLine = true;
			}
			else if (!String::Compare(ScriptTextParser->Tokens[0], "elseIf", true) || 
					!String::Compare(ScriptTextParser->Tokens[0], "else", true)) {
				ExdentLine = true;
			}
		}
		else if (ReadLine->Replace("\t", "")->Length == 0 && CullEmptyLines) {
			int CaretPos = EditorBox->SelectionStart - 1, LineStart = ScriptTextParser->GetLineStartIndex(CaretPos, EditorBox->Text);

			if (LineStart > -1 && LineStart < CaretPos) {
				EditorBox->SelectionStart = LineStart;
				EditorBox->SelectionLength = CaretPos - LineStart;
				EditorBox->SelectedText = "";
			}
		}

	}

	if (Indents < 0)	Indents = 0;
	return Indents;
}

void Workspace::ExdentLine(void)
{
	try {
		NativeWrapper::LockWindowUpdate(EditorBox->Handle);
		int SelStart = EditorBox->SelectionStart - 1, Index = 0;
		for (int i = SelStart; i > Index; --i) {
			if (EditorBox->Text[i] == '\n') {
				Index = i;
				break;
			}
		}

		UInt32 Count = ScriptTextParser->GetTrailingTabCount(Index + 1, EditorBox->Text), Exs = 0;

		EditorBox->SelectionStart = Index + 1;
		EditorBox->SelectionLength = 1;

		if (!String::Compare(ScriptTextParser->Tokens[0], "loop", true) || 
			!String::Compare(ScriptTextParser->Tokens[0], "endIf", true) || 
			!String::Compare(ScriptTextParser->Tokens[0], "end", true)) {
				Exs = Indents;
		}
		else	Exs = Indents - 1;

		if (Count > Exs)	EditorBox->SelectedText = "";

		if (SelStart + 1 < EditorBox->Text->Length  && Count > Exs)	EditorBox->SelectionStart = SelStart;
		else														EditorBox->SelectionStart = SelStart + 1;
		EditorBox->SelectionLength = 0;
	} finally {
		NativeWrapper::LockWindowUpdate(IntPtr::Zero);
	}
}

void Workspace::AddMessageToPool(MessageType Type, UInt32 Line, String^ Message)
{
	ListViewItem^ Item = gcnew ListViewItem(" ", (int)Type);
	Item->SubItems->Add(Line.ToString());
	Item->SubItems->Add(Message);
	ErrorBox->Items->Add(Item);
	return;
}

void Workspace::ValidateScript(UInt32 ScriptType)
{
	StringReader^ ValidateParser = gcnew StringReader(PreProcessedText);	
	String^ ReadLine = ValidateParser->ReadLine();
	ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_Invalid);
	ErrorBox->Items->Clear();
	ScriptTextParser->Variables->Clear();

	while (ReadLine != nullptr) {
		ScriptTextParser->Tokenize(ReadLine, false);

		if (!ScriptTextParser->Valid) {
			++ScriptTextParser->CurrentLineNo;
			ReadLine = ValidateParser->ReadLine();
			continue;
		} else if (ScriptTextParser->Tokens[0][0] == ';') {
			if (!ScriptTextParser->HasToken(";<CSEBlock>")) {
				do {
					ReadLine = ValidateParser->ReadLine();
					ScriptTextParser->Tokenize(ReadLine, false);
					if (!ScriptTextParser->HasToken(";</CSEBlock>"))	break;
				} while (ReadLine != nullptr);
				continue;
			} else
				++ScriptTextParser->CurrentLineNo;

			ReadLine = ValidateParser->ReadLine();
			continue;
		}

		++ScriptTextParser->CurrentLineNo;

															// parse first token
		String^ FirstToken = ScriptTextParser->Tokens[0];
		String^ SecondToken = (ScriptTextParser->Tokens->Count > 1)?ScriptTextParser->Tokens[1]:"";
		ScriptParser::TokenType TokenType = ScriptTextParser->GetTokenType(FirstToken);


		switch (TokenType)
		{
		case ScriptParser::TokenType::e_ScriptName: 
			if (Char::IsDigit(SecondToken[0]))
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Identifier '" + SecondToken + "' starts with an integer. Can cause BadThings™.");
			if (ScriptTextParser->HasStringGotIllegalChar(SecondToken, "_", ""))
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Identifier '" + SecondToken + "' contains an invalid character.");
			if (ScriptTextParser->ScriptName == nullptr)
				ScriptTextParser->ScriptName = SecondToken;
			else
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Redeclaration of script name.");
			break;
		case ScriptParser::TokenType::e_Variable:
			if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_Invalid)
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Variable '" + SecondToken + "' declared inside a script block.");
			if (ScriptTextParser->FindVariable(SecondToken)->VarName != "@nullptr")
				AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Redeclaration of variable '" + SecondToken + "'.");
			else
				ScriptTextParser->Variables->AddLast(gcnew ScriptParser::VariableInfo(SecondToken, 0));
			break;
		case ScriptParser::TokenType::e_Begin: 
			if (!ScriptTextParser->IsValidBlock(SecondToken, (ScriptParser::ScriptType)ScriptType))
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid script block '" + SecondToken + "' for script type.");
			ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_ScriptBlock);
			break;
		case ScriptParser::TokenType::e_End:
			if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_ScriptBlock)
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'End' has no matching 'Begin'.");
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
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'Loop' has no matching 'While' or 'ForEach'.");
			else
				ScriptTextParser->BlockStack->Pop();
			break;
		case ScriptParser::TokenType::e_If:
			ScriptTextParser->BlockStack->Push(ScriptParser::BlockType::e_If);
			break;
		case ScriptParser::TokenType::e_ElseIf:
			if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'ElseIf' has no matching 'If'.");
			break;
		case ScriptParser::TokenType::e_Else:
			if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'Else' has no matching 'If'.");
			if (ScriptTextParser->Tokens->Count > 1 && ScriptTextParser->Tokens[1][0] != ';')
				AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Command 'Else' has an otiose expression following it.");
			break;
		case ScriptParser::TokenType::e_EndIf:
			if (ScriptTextParser->BlockStack->Peek() != ScriptParser::BlockType::e_If)
				AddMessageToPool(MessageType::e_Error, ScriptTextParser->CurrentLineNo, "Invalid block structure. Command 'EndIf' has no matching 'If'.");
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
		if (ScriptTextParser->GetTokenType(FirstToken) != ScriptParser::TokenType::e_Variable) {
			for each (String^% Itr in ScriptTextParser->Tokens) {
				if (ScriptTextParser->FindVariable(Itr)->VarName != "@nullptr") {
					if (Pos == 0 || ScriptTextParser->Delimiters[Pos - 1] != '.') {
						if (ScriptTextParser->IsComment(Pos) == -1) {
							ScriptTextParser->FindVariable(Itr)->RefCount++;
						}
					}
				}
				++Pos;
			}
		}

		Pos = 0;
		String^ Token;
		for each (Char Itr in ScriptTextParser->Delimiters) {
		//	if (Itr == '.') {
				Token = ScriptTextParser->Tokens[Pos];

				if (Char::IsDigit(Token[0]) && ScriptTextParser->HasAlpha(Pos))
					AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Identifier '" + Token + "' starts with an integer. Can cause BadThings™.");
				if (ScriptTextParser->FindVariable(Token)->VarName == "@nullptr" && 
					ScriptTextParser->IsComment(Pos + 1) == -1 && 
					ScriptTextParser->IsLiteral(Token) == 0 && 
					ScriptTextParser->GetTokenType(Token) != ScriptParser::TokenType::e_Variable &&
					ISDB->IsCommand(Token) == 0 &&
					ScriptTextParser->HasStringGotIllegalChar(Token, "_", "$#!=<>:|+-*"))
					AddMessageToPool(MessageType::e_Warning, ScriptTextParser->CurrentLineNo, "Identifier '" + Token + "' contains an invalid character.");
		//	}
			++Pos;
		}
		ReadLine = ValidateParser->ReadLine();
	}

	for each (ScriptParser::VariableInfo^% Itr in ScriptTextParser->Variables) {
		if (!Itr->RefCount)
			AddMessageToPool(MessageType::e_Warning, 1, "Variable '" + Itr->VarName + "' unreferenced in local context.");
	}

	ScriptTextParser->Reset();
	if (ErrorBox->Items->Count && ErrorBox->Visible == false) {
		ToolBarErrorList_Click(nullptr, nullptr);
	}
}

bool Workspace::TabIndent()
{
	int SelStart = EditorBox->SelectionStart, Operation = 0;
	String^ Source = EditorBox->SelectedText, ^Result;
//	if (Control::ModifierKeys == Keys::Control)		Operation = 1;			// indent
//	else if (Control::ModifierKeys == Keys::Shift)	Operation = -1;			// exdent
	if (Control::ModifierKeys == Keys::Shift)	Operation = -1;				// exdent
	else										Operation = 1;				// indent

	switch (Operation)
	{
	case 0:
		return false;
	case 1:
		if (Source == "")	return false;
		break;
	case -1:
		if (Source == "") {
			int LineStart = ScriptTextParser->GetLineStartIndex(SelStart - 1, EditorBox->Text);	

			if (LineStart > -1) {
				if (ScriptTextParser->GetTrailingTabCount(LineStart, EditorBox->Text) > 0) {
					EditorBox->SelectionStart = LineStart;
					EditorBox->SelectionLength = 1;
					EditorBox->SelectedText = "";
					EditorBox->SelectionStart = SelStart;
					EditorBox->SelectionLength = 0;
				}
			}
			return true;
		}
		break;
	}


	StringReader^ TabIndentParser = gcnew StringReader(Source);
	String^ ReadLine = TabIndentParser->ReadLine();

	while (ReadLine != nullptr) {
		if (ReadLine != "") {
			if (Operation == -1) {
				Char Itr = ReadLine[0];
				if (Itr == '\t')		Result += ReadLine->Substring(1);
				else					Result += ReadLine;
			}
			else {
				Result += "\t" + ReadLine;
			}
		}

		ReadLine = TabIndentParser->ReadLine();
		if (ReadLine != nullptr)
			Result += "\n";
	}

	EditorBox->SelectedText = Result;
	EditorBox->SelectionStart = SelStart;
	EditorBox->SelectionLength = Result->Length;
	return true;
}

bool Workspace::IsDelimiterKey(Keys KeyCode)
{
	bool Result = false;

	for each (Keys Itr in Globals::GetSingleton()->DelimiterKeys) {
		if (Itr == KeyCode) {
			Result = true;
			break;
		}
	}
	return Result;
}


void Workspace::ToggleComment(int CaretPos)
{
	int LineStartIdx = ScriptTextParser->GetLineStartIndex(CaretPos - 1, EditorBox->Text), SelLength = EditorBox->SelectionLength;
	if (LineStartIdx < 0)	LineStartIdx = 0;
	int LineEndIdx = ScriptTextParser->GetLineEndIndex(CaretPos, EditorBox->Text);
	if (LineEndIdx < 0)		LineEndIdx = EditorBox->Text->Length;

	if (SelLength == 0) {
		String^ Line = EditorBox->Text->Substring(LineStartIdx, LineEndIdx - LineStartIdx);
		ScriptTextParser->Tokenize(Line, false);
		if (!ScriptTextParser->Valid)		return;

		HandleTextChanged = false;
		EditorBox->SelectionStart = LineStartIdx + ScriptTextParser->Indices[0];
		if (Line->IndexOf(";") == ScriptTextParser->Indices[0]) {		// is a comment
			EditorBox->SelectionLength = 1;
			EditorBox->SelectedText = "";
		}
		else {															// not a comment
			EditorBox->SelectionStart = LineStartIdx;
			EditorBox->SelectionLength = 0;
			EditorBox->SelectedText = ";";
		}
	}
	else {																// comment each line
		CaretPos = EditorBox->SelectionStart;
		LineStartIdx = ScriptTextParser->GetLineStartIndex(CaretPos - 1, EditorBox->Text);
		if (LineStartIdx < 0)	LineStartIdx = 0;
		LineEndIdx = ScriptTextParser->GetLineEndIndex(CaretPos + SelLength, EditorBox->Text);
		if (LineEndIdx < 0)		LineEndIdx = EditorBox->Text->Length;
		
		String^ Lines = EditorBox->Text->Substring(LineStartIdx, LineEndIdx - LineStartIdx), 
				^Result, 
				^ReadLine;
		StringReader^ CommentParser = gcnew StringReader(Lines); ReadLine = CommentParser->ReadLine();
		UInt32 Count = 0, ToggleType = 9;								// 0 - off, 1 - on
		while (ReadLine != nullptr) {
			ScriptTextParser->Tokenize(ReadLine, false);
			if (!ScriptTextParser->Valid) {
				Result += ReadLine + "\n";
				ReadLine = CommentParser->ReadLine();
				continue;
			}
		
			if (ReadLine->IndexOf(";") == ScriptTextParser->Indices[0] && (!Count || !ToggleType)) {
				Result += ReadLine->Substring(0, ScriptTextParser->Indices[0]);
				if (!Count)		ToggleType = 0;
				Result += ReadLine->Substring(ReadLine->IndexOf(";") + 1);
				
			}
			else if (ReadLine->IndexOf(";") != ScriptTextParser->Indices[0] && (!Count || ToggleType)) {
				if (!Count)		ToggleType = 1;
				Result += ";" + ReadLine;
			}
			else
				Result += ReadLine;

			ReadLine = CommentParser->ReadLine();
			if (ReadLine != nullptr)	Result += "\n";
			Count++;
		}

		HandleTextChanged = false;
		EditorBox->SelectionStart = LineStartIdx;
		EditorBox->SelectionLength = LineEndIdx - LineStartIdx;
		EditorBox->SelectedText = Result;
	}

	EditorBox->SelectionStart = CaretPos;
	EditorBox->SelectionLength = 0;
	UpdateLineNumbers();
}

void Workspace::ToggleBookmark(int CaretPos)
{
	int LineNo = EditorBox->GetLineFromCharIndex(CaretPos) + 1, Count = 0;
	for each (ListViewItem^% Itr in BookmarkBox->Items) {
		if (int::Parse(Itr->SubItems[0]->Text) == LineNo) {
			BookmarkBox->Items->RemoveAt(Count);
			return;
		}
		Count++;
	}

	String^ BookmarkDesc = Microsoft::VisualBasic::Interaction::InputBox("Enter a description for the bookmark.", "Place Bookmark", "", EditorBox->Location.X + EditorBox->Width / 2, EditorBox->Location.Y + EditorBox->Height / 2);

	if (BookmarkDesc == "")		return;

	ListViewItem^ Item = gcnew ListViewItem(LineNo.ToString());
	Item->SubItems->Add(BookmarkDesc);
	BookmarkBox->Items->Add(Item);

	UpdateLineNumbers();

	if (!BookmarkBox->Visible)		ToolBarBookmarkList_Click(nullptr, nullptr);
}

void Workspace::UpdateImagePointers(void)
{
	if (IndexPointers != nullptr) {
		for each (PictureBox^% Itr in IndexPointers) {
			Point Loc = EditorBox->GetPositionFromCharIndex(Int32::Parse(Itr->Tag->ToString()));
			if (Loc.Y < EditorBox->Height && Loc.Y > 0 && Loc.X > 0 && Loc.Y < EditorBox->Width) {
				Loc.Y -= 8;
				Itr->Location = Loc;
				if (!Itr->Visible)
					Itr->Visible = true;
			} else if (Itr->Visible)
				Itr->Visible = false;
		}
	}

	if (ScriptLineLimitIndicator->Visible) {
		Point Loc = EditorBox->GetPositionFromCharIndex(Int32::Parse(ScriptLineLimitIndicator->Tag->ToString()));
		Loc.Y -= 15;
		ScriptLineLimitIndicator->Location = Loc;
	}
}

void Workspace::MoveCaretToValidHome(void)
{
	int CaretPos = EditorBox->SelectionStart, LineStartIdx = ScriptTextParser->GetLineStartIndex(CaretPos - 1, EditorBox->Text);
	if (LineStartIdx < 0)	LineStartIdx = 0;
	int LineEndIdx = ScriptTextParser->GetLineEndIndex(CaretPos, EditorBox->Text);
	if (LineEndIdx < 0)		LineEndIdx = EditorBox->Text->Length;

	String^ Line = EditorBox->Text->Substring(LineStartIdx, LineEndIdx - LineStartIdx);
	ScriptTextParser->Tokenize(Line, false);
	if (!ScriptTextParser->Valid)		return;

	int HomeIdx = LineStartIdx + ScriptTextParser->Indices[0];
	if (HomeIdx == CaretPos)
		EditorBox->SelectionStart = LineStartIdx;
	else
		EditorBox->SelectionStart = HomeIdx;

	if (Control::ModifierKeys == Keys::Shift)
		EditorBox->SelectionLength = LineEndIdx - LineStartIdx;
	else
		EditorBox->SelectionLength = 0;
}

void Workspace::PreProcessScriptText(PreProcessor::PreProcessOp Operation, String^ ScriptText)
{
	String^ ExtractedBlock = "";
	switch (Operation)
	{
	case PreProcessor::PreProcessOp::e_Collapse:	// before setting editor text
		EditorBox->Text = PREPROC->PreProcess(ScriptText, Operation, false, ExtractedBlock);
		ReadBookmarks(ExtractedBlock);
		LoadSavedCaretPos(ExtractedBlock);
		break;
	case PreProcessor::PreProcessOp::e_Expand:		// before moving editor text
		PreProcessedText = PREPROC->PreProcess(EditorBox->Text, Operation, false, ExtractedBlock);
		PreProcessedText += ";<CSEBlock>\n";
		PreProcessedText += ";<CSEWarning>This script may contain preprocessor directives parsed by the CSE Script Editor. Refrain from modifying it in the vanilla editor.</CSEWarning>\n";
		SaveCaretPos();
		DumpBookmarks();
		PreProcessedText += ";</CSEBlock>\n";
		break;
	}
}

void Workspace::DumpBookmarks()
{
	String^ BookmarkSegment = "";

	for each (ListViewItem^ Itr in BookmarkBox->Items) {
		BookmarkSegment += ";<CSEBookmark>\t" + Itr->SubItems[0]->Text + "\t" + Itr->SubItems[1]->Text + "\t</CSEBookmark>\n";
	}
	PreProcessedText += BookmarkSegment;
}


void Workspace::ReadBookmarks(String^ ExtractedBlock)
{
	BookmarkBox->Items->Clear();
	BookmarkBox->BeginUpdate();

	ScriptParser^ TextParser = gcnew ScriptParser();
	StringReader^ StringParser = gcnew StringReader(ExtractedBlock);
	String^ ReadLine = StringParser->ReadLine();
	int LineNo = 0;

	while (ReadLine != nullptr) {
		TextParser->Tokenize(ReadLine, false);
		if (!TextParser->Valid) {
			ReadLine = StringParser->ReadLine();
			continue;
		}

		if (!TextParser->HasToken(";<CSEBookmark>")) {
			array<String^>^ Splits = ReadLine->Substring(TextParser->Indices[0])->Split(static_cast<array<Char>^>(Globals::GetSingleton()->TabDelimit));
			try { LineNo = int::Parse(Splits[1]); } catch (...) { LineNo = 1; }

			ListViewItem^ Item = gcnew ListViewItem(LineNo.ToString());
			Item->SubItems->Add(Splits[2]);
			BookmarkBox->Items->Add(Item);
		}

		ReadLine = StringParser->ReadLine();
	}
	BookmarkBox->EndUpdate();
}

void Workspace::SaveCaretPos()
{
	if (Globals::GetSingleton()->ScriptEditorOptions->SaveLastKnownPos->Checked) {
		PreProcessedText += String::Format(";<CSECaretPos> {0} </CSECaretPos>\n", EditorBox->SelectionStart);
	}
}

void Workspace::LoadSavedCaretPos(String^ ExtractedBlock)
{
	ScriptParser^ TextParser = gcnew ScriptParser();
	StringReader^ StringParser = gcnew StringReader(ExtractedBlock);
	String^ ReadLine = StringParser->ReadLine();
	int CaretPos = -1;
	EditorBox->Focus();

	while (ReadLine != nullptr) {
		TextParser->Tokenize(ReadLine, false);
		if (!TextParser->Valid) {
			ReadLine = StringParser->ReadLine();
			continue;
		}

		if (!TextParser->HasToken(";<CSECaretPos>")) {
			try { CaretPos = int::Parse(TextParser->Tokens[1]); } catch (...) { CaretPos = -1; }
			break;
		}

		ReadLine = StringParser->ReadLine();
	}

	if (CaretPos > -1)		EditorBox->SelectionStart = CaretPos;
	else					EditorBox->SelectionStart = 0;
	EditorBox->ScrollToCaret();

}

bool Workspace::IsCursorInsideCommentSeg(bool OneLessIdx)
{
	bool Result = true;
	Point Loc = EditorBox->GetPositionFromCharIndex(EditorBox->SelectionStart);
	int LineNo = EditorBox->GetLineFromCharIndex(EditorBox->SelectionStart);
	if (LineNo < EditorBox->Lines->Length) {
		ScriptTextParser->Tokenize(EditorBox->Lines[LineNo], false);
		if (ScriptTextParser->IsComment(ScriptTextParser->HasToken(GetTextAtLoc(Loc, false, false, (OneLessIdx)?(EditorBox->SelectionStart - 1):(EditorBox->SelectionStart), true))) == -1)
			Result = false;
	}
	return Result;
}

bool Workspace::HasLineChanged()
{
	bool Result = false;
	int LineNo = EditorBox->GetLineFromCharIndex(EditorBox->SelectionStart);
	if (LineNo != CurrentLineNo) {
		Result = true;
		CurrentLineNo = LineNo;
	}
	return Result;
}

void Workspace::CalculateLineOffsets(UInt32 Data, UInt32 Length, String^% ScriptText)
{
	LineOffsets->Clear();
	if (Data) {
		UInt8* DataPtr = (UInt8*)Data;
		Array^ ByteCode = Array::CreateInstance(Byte::typeid, Length);

		for (UInt32 i = 0; i < Length; i++) {
			ByteCode->SetValue(*(DataPtr + i), (int)i);
		}

		UInt32 ScriptOffset = 0, CurrentOffset = 0, SkipOffset = 0;

		array<String^>^ Lines = ScriptText->Split('\n');
		ScriptParser^ TextParser = gcnew ScriptParser();
		for each (String^% Itr in Lines) {
			TextParser->Tokenize(Itr, false);

			if (!TextParser->HasToken(";<CSEImportSeg>")) {
				SkipOffset++;
				continue;
			} else if (!TextParser->HasToken(";</CSEImportSeg>")) {			
				if (!--SkipOffset)	LineOffsets->Add(0xffff);
				continue;
			} else if (!TextParser->HasToken(";<CSEMacroDef>") || !TextParser->HasToken(";<CSEEnum>")) {
				if (!SkipOffset)	LineOffsets->Add(0xffff);
				continue;
			} else if (!Itr->IndexOf(";<CSE") || !Itr->IndexOf(";</CSE")) {
				continue;
			}

			
			CurrentOffset = ByteCodeParser::GetOffsetForLine(Itr, ByteCode, ScriptOffset);
			if (!SkipOffset)			LineOffsets->Add(CurrentOffset);
		}
	}

	if (ToolBarOffsetToggle->Checked) ToolBarOffsetToggle_Click(nullptr, nullptr);
	if (LineOffsets->Count < 1)			ToolBarOffsetToggle->Enabled = false;
	else								ToolBarOffsetToggle->Enabled = true;
}

void Workspace::ValidateLineLimit(void)
{
	ScriptLineLimitIndicator->Hide();
	UInt32 LineNo = EditorBox->GetLineFromCharIndex(EditorBox->GetFirstCharIndexOfCurrentLine()), Length = 0, Index = 0;
	if (LineNo < EditorBox->Lines->Length) {
		Length = EditorBox->Lines[LineNo]->Length;
		if (Length > 512) {
			Index = EditorBox->GetFirstCharIndexOfCurrentLine() + 511;
			ScriptLineLimitIndicator->Location = Point(EditorBox->GetPositionFromCharIndex(Index).X, EditorBox->GetPositionFromCharIndex(Index).Y - 15);
			ScriptLineLimitIndicator->Tag = String::Format("{0}", Index);
			ScriptLineLimitIndicator->Show();
			ScriptLineLimitIndicator->BringToFront();
		}
	}
}


void Workspace::GetVariableIndices(bool SetFlag)
{
	if (GetVariableData) {
		VariableBox->Items->Clear();
		NativeWrapper::ScriptEditor_GetScriptVariableIndices(AllocatedIndex, g_ScriptDataPackage->EditorID);
		VariableBox->Show();
		VariableBox->BringToFront();
		if (VariableBox->Items->Count) {
			ToolBarUpdateVarIndices->Enabled = true;
		}
		GetVariableData = false;
	} else if (SetFlag) {
		GetVariableData = true;
	}
}

void Workspace::SetVariableIndices(void)
{
	ScriptVarIndexData::ScriptVarInfo Data;

	for each (ListViewItem^ Itr in VariableBox->Items) {
		try {
			if (Itr->Tag != nullptr) {
				CStringWrapper^ CEID = gcnew CStringWrapper(Itr->Text);
				UInt32 Index = 0;

				try {
					Index = UInt32::Parse(Itr->SubItems[2]->Text);
				} catch (Exception^ E) {
					throw gcnew CSEGeneralException("Couldn't parse index of variable  '" + Itr->Text + "' in script '" + EditorTab->Text + "'\n\tError Message: " + E->Message);
				}
				Data.Index = Index;
				if		(!String::Compare(Itr->SubItems[1]->Text, "Integer", true))			Data.Type = 1;
				else if (!String::Compare(Itr->SubItems[1]->Text, "Float", true))			Data.Type = 0;
				else																		Data.Type = 2;
				Data.Name = CEID->String();		

				if (!NativeWrapper::ScriptEditor_SetScriptVariableIndex(g_ScriptDataPackage->EditorID, &Data)) {	// shaky code, need to be certain of g_ScriptDataPackage's state
					throw gcnew CSEGeneralException("Couldn't update the index of variable '" + Itr->Text + "' in script '" + EditorTab->Text + "'");
				}
			}
		} catch (CSEGeneralException^ E) {
			DebugPrint(E->Message, true);
		}
	}
}













//  EVENT HANDLERS

//	STATUS BAR CONTENTS 
void Workspace::ToolBarEditMenuContentsFind_Click(Object^ Sender, EventArgs^ E)
{
	if (EditorBox->Enabled && ToolBarCommonTextBox->Text != "") {
		FindAndReplace(false);
	}
}

void Workspace::ToolBarEditMenuContentsReplace_Click(Object^ Sender, EventArgs^ E)
{
	if (EditorBox->Enabled && ToolBarCommonTextBox->Text != "")
		FindAndReplace(true);
}

void Workspace::ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E)
{
	if (EditorBox->Enabled && ToolBarCommonTextBox->Text != "")
		JumpToLine(ToolBarCommonTextBox->Text, false);
}

void Workspace::ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E)
{
	if (EditorBox->Enabled && ToolBarCommonTextBox->Text != "")
		JumpToLine(ToolBarCommonTextBox->Text, true);
}

void Workspace::ToolBarErrorList_Click(Object^ Sender, EventArgs^ E)
{
	if (FindBox->Visible)
		ToolBarFindList_Click(nullptr, nullptr);
	else if (BookmarkBox->Visible)
		ToolBarBookmarkList_Click(nullptr, nullptr);

	if (!ErrorBox->Visible) {
		ErrorBox->Show();
		ErrorBox->BringToFront();
		ToolBarErrorList->Checked = true;
	}
	else {
		ErrorBox->Hide();
		ToolBarErrorList->Checked = false;
	}
}

void Workspace::ToolBarFindList_Click(Object^ Sender, EventArgs^ E)
{
	if (ErrorBox->Visible)
		ToolBarErrorList_Click(nullptr, nullptr);
	else if (BookmarkBox->Visible)
		ToolBarBookmarkList_Click(nullptr, nullptr);

	if (!FindBox->Visible) {
		FindBox->Show();
		FindBox->BringToFront();
		ToolBarFindList->Checked = true;
	}
	else {
		FindBox->Hide();
		ClearFindImagePointers();
		ToolBarFindList->Checked = false;
	}
}

void Workspace::ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E)
{
	if (ErrorBox->Visible)
		ToolBarErrorList_Click(nullptr, nullptr);
	else if (FindBox->Visible)
		ToolBarFindList_Click(nullptr, nullptr);

	if (!BookmarkBox->Visible) {
		BookmarkBox->Show();
		BookmarkBox->BringToFront();
		ToolBarBookmarkList->Checked = true;
	}
	else {
		BookmarkBox->Hide();
		ToolBarBookmarkList->Checked = false;
	}
}

void Workspace::ToolBarCommonTextBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::Enter:
		if (ToolBarCommonTextBox->Tag->ToString() != "") {
			if		(ToolBarCommonTextBox->Tag->ToString() == "Find")				ToolBarEditMenuContentsFind_Click(nullptr, nullptr);
			else if (ToolBarCommonTextBox->Tag->ToString() == "Replace")			ToolBarEditMenuContentsReplace_Click(nullptr, nullptr);
			else if (ToolBarCommonTextBox->Tag->ToString() == "Goto Line")			ToolBarEditMenuContentsGotoLine_Click(nullptr, nullptr);
			else if (ToolBarCommonTextBox->Tag->ToString() == "Goto Offset")		ToolBarEditMenuContentsGotoOffset_Click(nullptr, nullptr);
		}	

		E->Handled = true;
		HandleKeyCTB = E->KeyCode;
		break;
	}
}

void Workspace::ToolBarCommonTextBox_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
{
	if (HandleKeyCTB != Keys::None) {
		E->Handled = true;
		HandleKeyCTB = Keys::None;
		return;
	}
}

void Workspace::ToolBarCommonTextBox_KeyUp(Object^ Sender, KeyEventArgs^ E)
{
	if (E->KeyCode == HandleKeyCTB) {
		E->Handled = true;
		HandleKeyCTB = Keys::None;
		return;
	}
}

void Workspace::ToolBarCommonTextBox_LostFocus(Object^ Sender, EventArgs^ E)
{
	ToolBarCommonTextBox->Tag = "";
}

void Workspace::ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E)
{
	SaveFileDialog^ SaveManager = gcnew SaveFileDialog();

	SaveManager->DefaultExt = "*.txt";
	SaveManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
	SaveManager->FileName = EditorTab->Text;

	if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->FileName->Length > 0) {
		EditorBox->SaveFile(SaveManager->FileName, RichTextBoxStreamType::PlainText);
		DebugPrint("Dumped editor " + AllocatedIndex + "'s text to " + SaveManager->FileName);
	}
}

void Workspace::ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E)
{
	OpenFileDialog^ LoadManager = gcnew OpenFileDialog();

	LoadManager->DefaultExt = "*.txt";
	LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";

	if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileName->Length > 0) {
		HandleTextChanged = false;
		EditorBox->LoadFile(LoadManager->FileName, RichTextBoxStreamType::PlainText);
		DebugPrint("Loaded text from " + LoadManager->FileName + " to editor " + AllocatedIndex);
	}
}

void Workspace::ToolBarOptions_Click(Object^ Sender, EventArgs^ E)
{
	Globals::GetSingleton()->ScriptEditorOptions->LoadINI();
	Globals::GetSingleton()->ScriptEditorOptions->OptionsBox->ShowDialog();
}

void Workspace::ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E)
{
	ToolBarScriptType->Text = ToolBarScriptTypeContentsObject->ToolTipText + " Script";
	ScriptType = 0;
}
void Workspace::ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E)
{
	ToolBarScriptType->Text = ToolBarScriptTypeContentsQuest->ToolTipText + " Script";
	ScriptType = 1;
}
void Workspace::ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E)
{
	ToolBarScriptType->Text = ToolBarScriptTypeContentsMagicEffect->ToolTipText + " Script";
	ScriptType = 2;
}


void Workspace::ToolBarNewScript_Click(Object^ Sender, EventArgs^ E)
{
	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
	Parameters->VanillaHandleIndex = AllocatedIndex;

	switch (Control::ModifierKeys)
	{
	case Keys::Control:
		ParentStrip->PerformRemoteOperation(TabContainer::RemoteOperation::e_New);
		break;
	case Keys::Shift:
		Parameters->VanillaHandleIndex = 0;
		Parameters->ParameterList->Add((UInt32)ParentStrip->EditorForm->Location.X);
		Parameters->ParameterList->Add((UInt32)ParentStrip->EditorForm->Location.Y);
		Parameters->ParameterList->Add((UInt32)ParentStrip->EditorForm->Width);
		Parameters->ParameterList->Add((UInt32)ParentStrip->EditorForm->Height);

		SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_AllocateTabContainer, Parameters);
		break;
	default:
		Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_New);
		SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
		break;
	}
}
void Workspace::ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E)
{
	if (Control::ModifierKeys == Keys::Control) {
		ParentStrip->PerformRemoteOperation(TabContainer::RemoteOperation::e_Open);
	} else {
		ScriptListBox->Show(ScriptListDialog::Operation::e_Open);
	}
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

	SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
}
void Workspace::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
{
	ScriptEditorManager::OperationParams^ Parameters = gcnew ScriptEditorManager::OperationParams();
	Parameters->VanillaHandleIndex = AllocatedIndex;
	Parameters->ParameterList->Add(ScriptEditorManager::SendReceiveMessageType::e_Recompile);

	SEMGR->PerformOperation(ScriptEditorManager::OperationType::e_SendMessage, Parameters);
}
void Workspace::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
{
	ScriptListBox->Show(ScriptListDialog::Operation::e_Delete);
}


void Workspace::ToolBarOffsetToggle_Click(Object^ Sender, EventArgs^ E)
{
	if (ToolBarOffsetToggle->Checked)		ToolBarOffsetToggle->Checked = false;
	else									ToolBarOffsetToggle->Checked = true;
	UpdateLineNumbers();
}

void Workspace::ToolBarNavBack_Click(Object^ Sender, EventArgs^ E)
{
	ParentStrip->NavigateStack(AllocatedIndex, TabContainer::NavigationDirection::e_Back);
}

void Workspace::ToolBarNavForward_Click(Object^ Sender, EventArgs^ E)
{
	ParentStrip->NavigateStack(AllocatedIndex, TabContainer::NavigationDirection::e_Forward);
}

void Workspace::ToolBarGetVarIndices_Click(Object^ Sender, EventArgs^ E)
{
	GetVariableIndices(true);
	Workspace::ToolBarSaveScript_Click(nullptr, nullptr);	
}

void Workspace::ToolBarUpdateVarIndices_Click(Object^ Sender, EventArgs^ E)
{
	SetVariableIndices();
}

void Workspace::ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E)
{
	ParentStrip->SaveAllTabs();
}



//	CONTEXT MENU  
void Workspace::ContextMenuCopy_Click(Object^ Sender, EventArgs^ E)
{
	try {
		Clipboard::Clear();
		Clipboard::SetText(GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, false, -1, false));
	} catch (Exception^ E) {
		DebugPrint("Exception raised while accessing the clipboard.\n\tException: " + E->Message, true);
	}
}

void Workspace::ContextMenuPaste_Click(Object^ Sender, EventArgs^ E)
{
	try {
		EditorBox->SelectedText = Clipboard::GetText();
	} catch (Exception^ E) {
		DebugPrint("Exception raised while accessing the clipboard.\n\tException: " + E->Message, true);
	}

}

void Workspace::EditorContextMenu_Opening(Object^ Sender, CancelEventArgs^ E)
{
	EditorContextMenu->Items[6]->Text = GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, false, -1, true);
	if (EditorContextMenu->Items[6]->Text->Length > 20)
		EditorContextMenu->Items[6]->Text = EditorContextMenu->Items[6]->Text->Substring(0, 17) + gcnew String("...");

	String^ TextUnderMouse = GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, false, -1, true);

	ContextMenuDirectLink->Tag = nullptr;
	if (ISDB->IsCommand(TextUnderMouse))			ContextMenuDirectLink->Tag = ISDB->GetCommandURL(TextUnderMouse);

	if (ContextMenuDirectLink->Tag == nullptr)		ContextMenuDirectLink->Visible = false;
	else											ContextMenuDirectLink->Visible = true;

	CStringWrapper^ CTUM = gcnew CStringWrapper(TextUnderMouse);
	ScriptData* Data = NativeWrapper::FetchScriptFromForm(CTUM->String());
	ContextMenuJumpToScript->Visible = true;

	if (Data->IsValid()) {
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
	} else if (ISDB->IsUDF(TextUnderMouse)) {
		ContextMenuJumpToScript->Text = "Jump to Function script";
		ContextMenuJumpToScript->Tag = TextUnderMouse;
	} else
		ContextMenuJumpToScript->Visible = false;
}

void Workspace::ContextMenuWikiLookup_Click(Object^ Sender, EventArgs^ E)
{
	Process::Start("http://cs.elderscrolls.com/constwiki/index.php/Special:Search?search=" + GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, false, -1, true) + "&fulltext=Search");
}

void Workspace::ContextMenuOBSEDocLookup_Click(Object^ Sender, EventArgs^ E)
{
	Process::Start("http://obse.silverlock.org/obse_command_doc.html#" + GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, false, -1, true));
}

void Workspace::ContextMenuDirectLink_Click(Object^ Sender, EventArgs^ E)
{
	try { Process::Start(dynamic_cast<String^>(ContextMenuDirectLink->Tag)); }
	catch (Exception^ E) {
		DebugPrint("Exception raised while opening internet page.\n\tException: " + E->Message);
		MessageBox::Show("Couldn't open internet page. Mostly likely caused by an improperly formatted URL.", "CSE Script Editor");
	}
}


void Workspace::ContextMenuCopyToCTB_Click(Object^ Sender, EventArgs^ E)
{
	ToolBarCommonTextBox->Text = GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, false, -1, true);
	ToolBarCommonTextBox->Focus();
}

void Workspace::ContextMenuFind_Click(Object^ Sender, EventArgs^ E)
{
	ToolBarCommonTextBox->Text = GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, false, -1, true);
	ToolBarEditMenuContentsFind_Click(nullptr, nullptr);
}

void Workspace::ContextMenuToggleComment_Click(Object^ Sender, EventArgs^ E)
{
	ToggleComment(EditorBox->GetCharIndexFromPosition(Globals::GetSingleton()->MouseLocation));
}

void Workspace::ContextMenuToggleBookmark_Click(Object^ Sender, EventArgs^ E)
{
	ToggleBookmark(EditorBox->GetCharIndexFromPosition(Globals::GetSingleton()->MouseLocation));
}

void Workspace::ContextMenuJumpToScript_Click(Object^ Sender, EventArgs^ E)
{
	ParentStrip->JumpToScript(AllocatedIndex, dynamic_cast<String^>(ContextMenuJumpToScript->Tag));
}



//	EDITOR BOX	
void Workspace::EditorBox_TextChanged(Object^ Sender, EventArgs^ E)
{
	if (TextSet) {
		TextSet = false;
		HasChanged = false;
		EditorTab->ImageIndex = 0;
		UpdateLineNumbers();
		if (IndexPointers != nullptr)	UpdateImagePointers();
		return;
	}
	else {
		HasChanged = true;
		EditorTab->ImageIndex = 1;

		if (ToolBarOffsetToggle->Checked)		ToolBarOffsetToggle_Click(nullptr, nullptr);
		ToolBarOffsetToggle->Enabled = false;

		ClearFindImagePointers();
	}

	if (!HandleTextChanged) {
		HandleTextChanged = true;
		return;
	}

	if (!IsCursorInsideCommentSeg(true)) {
		ISBox->Initialize(ISBox->LastOperation, false, false);
	}
	ValidateLineLimit();
}

void Workspace::EditorBox_VScroll(Object^ Sender, EventArgs^ E)
{
	EditorLineNo->Location = Point(0, EditorBox->GetPositionFromCharIndex(0).Y % (EditorBox->Font->Height - 1));
	UpdateLineNumbers();
	UpdateImagePointers();
}

void Workspace::EditorBox_Resize(Object^ Sender, EventArgs^ E)
{
	EditorLineNo->Size = Size(48, EditorBoxSplitter->SplitterDistance + EditorBox->Font->Height);
	EditorBox_VScroll(nullptr, nullptr);
}

void Workspace::EditorBox_MouseDown(Object^ Sender, MouseEventArgs^ E)
{

	if (HasLineChanged()) {
		UpdateLineNumbers();
		ISBox->CanShow = true;
		ISBox->LastOperation = SyntaxBox::Operation::e_Default;
		ValidateLineLimit();
	}
	Globals::GetSingleton()->MouseLocation = E->Location;

	if (Control::ModifierKeys == Keys::Control) {
		GetTextAtLoc(Globals::GetSingleton()->MouseLocation, true, true, -1, false);
	}

	if (ISBox->IsVisible()) {
		ISBox->Hide();
		ISBox->CanShow = false;
		ISBox->LastOperation = SyntaxBox::Operation::e_Default;
	}
}

void Workspace::EditorBox_MouseUp(Object^ Sender, MouseEventArgs^ E)
{
	;//
}

void Workspace::EditorBox_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E)
{
	if (E->Button == MouseButtons::Left && EditorBox->Text->Length > 0) {
		String^ TextUnderMouse = GetTextAtLoc(Globals::GetSingleton()->MouseLocation, false, false, EditorBox->SelectionStart, true);
		if (!IsCursorInsideCommentSeg(false)) {
			if (ISBox->QuickView(TextUnderMouse)) {
				return;
			}
		}
	}
}

void Workspace::EditorBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	int SelStart = EditorBox->SelectionStart, SelLength = EditorBox->SelectionLength;
	bool Exdent = false;

	if (IsDelimiterKey(E->KeyCode)) {
		ISBox->UpdateLocalVars();
		ISBox->CanShow = true;

		Indents = CalculateIndents(EditorBox->SelectionStart, Exdent, false);
		if (Exdent && E->KeyCode != Keys::Enter)		ExdentLine();

		if (!IsCursorInsideCommentSeg(true)) {
			try {
				if (E->KeyCode == Keys::OemPeriod) {
					ISBox->Initialize(SyntaxBox::Operation::e_Dot, false, true);
					HandleTextChanged = false;
				} 
				else if (E->KeyCode == Keys::Space) {
					String^ CommandName = GetTextAtLoc(Globals::GetSingleton()->MouseLocation, false, false, EditorBox->SelectionStart - 1, true);

					if (!String::Compare(CommandName, "call", true)) {
						ISBox->Initialize(SyntaxBox::Operation::e_Call, false, true);
						HandleTextChanged = false;
					} else if (!String::Compare(CommandName, "let", true) || !String::Compare(CommandName, "set", true)) {
						ISBox->Initialize(SyntaxBox::Operation::e_Assign, false, true);
						HandleTextChanged = false;
					}
					else ISBox->LastOperation = SyntaxBox::Operation::e_Default;
				} 
				else ISBox->LastOperation = SyntaxBox::Operation::e_Default;
			} catch (Exception^ E) {
				DebugPrint("IntelliSense threw an exception while initializing.\n\tException: " + E->Message, true);
			}
		}
	}


	switch (E->KeyCode) 
	{
	case Keys::V:									// Strips rtf elements
		if (E->Modifiers == Keys::Control) {
			try {
				if (Clipboard::ContainsText()) {
					String^ CText = Clipboard::GetText(TextDataFormat::Text);
					Clipboard::SetText(CText);
				} else {
					HandleKeyEditorBox = E->KeyCode;	
					E->Handled = true;
				}
			} catch (Exception^ E) {
				DebugPrint("Had trouble stripping RTF elements in editor " + AllocatedIndex + ".\n\tException: " + E->Message, true);
			}
			HandleTextChanged = false;
		}
		break;	
	case Keys::Q:									// Toggle comment
		if (E->Modifiers == Keys::Control) {
			ToggleComment(EditorBox->SelectionStart);
			HandleKeyEditorBox = E->KeyCode;
		}
		break;
	case Keys::O:									// Open script
		if (E->Modifiers == Keys::Control) {
			ToolBarOpenScript_Click(nullptr, nullptr);
		}
		break;
	case Keys::S:									// Save script
		if (E->Modifiers == Keys::Control) {
			ToolBarSaveScript_Click(nullptr, nullptr);
		}
		break;
	case Keys::D:									// Delete script
		if (E->Modifiers == Keys::Control) {
			ToolBarDeleteScript_Click(nullptr, nullptr);
		}
		break;
	case Keys::Left:								// Previous script
		if (E->Control && E->Alt) {
			ToolBarPreviousScript_Click(nullptr, nullptr);
		}
		break;
	case Keys::Right:								// Next script
		if (E->Control && E->Alt) {
			ToolBarNextScript_Click(nullptr, nullptr);
		}
		break;
	case Keys::N:									// New script
		if (E->Modifiers == Keys::Control) {
			ToolBarNewScript_Click(nullptr, nullptr);
		}
		break;
	case Keys::B:									// Toggle bookmark
		if (E->Modifiers == Keys::Control) {
			ContextMenuToggleBookmark_Click(nullptr, nullptr);
		}
		break;
	case Keys::Enter:								// Show syntax box
		if (E->Modifiers == Keys::Control) {
			if (!ISBox->IsVisible())					
				ISBox->Initialize(SyntaxBox::Operation::e_Default, true, false);

			HandleKeyEditorBox = E->KeyCode;
			E->Handled = true;
		} else {
			Indents = CalculateIndents(EditorBox->SelectionStart, Exdent, true);
			if (Exdent) ExdentLine();

			if (E->Modifiers == Keys::Shift) {
				HandleKeyEditorBox = E->KeyCode;
				E->Handled = true;			
			}
		}
		break;
	case Keys::Escape:
		if (ISBox->IsVisible()) {
			ISBox->Hide();
			ISBox->CanShow = false;
			ISBox->LastOperation = SyntaxBox::Operation::e_Default;
			E->Handled = true;
			HandleKeyEditorBox = E->KeyCode;
		}
		break;
	case Keys::Tab:
		if (ISBox->IsVisible()) {
			ISBox->PickIdentifier();
			ISBox->LastOperation = SyntaxBox::Operation::e_Default;
			E->Handled = true;
			HandleKeyEditorBox = E->KeyCode;
		} 
		else {
			if (TabIndent()) {	
				E->Handled = true;
				HandleKeyEditorBox = E->KeyCode;
			}
		}
		break;
	case Keys::Up:
		if (ISBox->IsVisible()) {
			ISBox->MoveIndex(SyntaxBox::Direction::e_Up);
			E->Handled = true;
			HandleKeyEditorBox = E->KeyCode;
		}
		break;
	case Keys::Down:
		if (ISBox->IsVisible()) {
			ISBox->MoveIndex(SyntaxBox::Direction::e_Down);
			E->Handled = true;
			HandleKeyEditorBox = E->KeyCode;
		}
		break;																														
	case Keys::Home:
		if (!E->Control) {
			MoveCaretToValidHome();
			HandleKeyEditorBox = E->KeyCode;
			E->Handled = true;
		}
		break;
	case Keys::Z:
	case Keys::Y:
		if (E->Modifiers == Keys::Control) {
			HandleTextChanged = false;	
		}
		break;
	case Keys::F:									// Find
		if (E->Modifiers == Keys::Control) {
			ToolBarCommonTextBox->Tag = "Find";
			ToolBarCommonTextBox->Text = "";
			ToolBarCommonTextBox->Focus();
			HandleKeyEditorBox = E->KeyCode;
			E->Handled = true;
		}
		break;
	case Keys::H:									// Replace
		if (E->Modifiers == Keys::Control) {
			ToolBarCommonTextBox->Tag = "Replace";
			ToolBarCommonTextBox->Text = "";
			ToolBarCommonTextBox->Focus();
			HandleKeyEditorBox = E->KeyCode;
			E->Handled = true;
		}
		break;
	case Keys::G:									// Goto Line
		if (E->Modifiers == Keys::Control) {
			ToolBarCommonTextBox->Tag = "Goto Line";
			ToolBarCommonTextBox->Text = "";
			ToolBarCommonTextBox->Focus();
			HandleKeyEditorBox = E->KeyCode;
			E->Handled = true;
		}
		break;
	case Keys::T:									// Goto Offset
		if (E->Modifiers == Keys::Control) {
			ToolBarCommonTextBox->Tag = "Goto Offset";
			ToolBarCommonTextBox->Text = "";
			ToolBarCommonTextBox->Focus();
			HandleKeyEditorBox = E->KeyCode;
			E->Handled = true;
		}
		break;
	}
}

void Workspace::EditorBox_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
{
	if (HandleKeyEditorBox != Keys::None) {
		E->Handled = true;
		HandleKeyEditorBox = Keys::None;
		return;
	}

	switch (E->KeyChar)
	{
	case Keys::Enter:
		if (Indents) {
			try {
				String^ IndentStr;
				for (int i = 0; i < Indents; i++) {
					IndentStr += "\t";
				}
				NativeWrapper::LockWindowUpdate(EditorBox->Handle);
				EditorBox->SelectedText = IndentStr;
			} finally {
				NativeWrapper::LockWindowUpdate(IntPtr::Zero);
			}
		}
	case Keys::Delete:
		UpdateLineNumbers();
		break;
	}
}

void Workspace::EditorBox_KeyUp(Object^ Sender, KeyEventArgs^ E)
{
	if (E->KeyCode == HandleKeyEditorBox) {
		E->Handled = true;
		HandleKeyEditorBox = Keys::None;
		return;
	}

	if (E->KeyCode == Keys::Back || E->KeyCode == Keys::Enter || E->KeyCode == Keys::Delete)
		UpdateLineNumbers();
	else if (EditorBox->SelectedText != "")
		UpdateLineNumbers();
	else if (E->KeyCode == Keys::Up || E->KeyCode == Keys::Down) {
		if (HasLineChanged()) {
			UpdateLineNumbers();
			ISBox->CanShow = true;
			ISBox->LastOperation = SyntaxBox::Operation::e_Default;
			ValidateLineLimit();
		}
	}
}

void Workspace::EditorBox_HScroll(Object^ Sender, EventArgs^ E)
{
	UpdateImagePointers();
}


//	EDITOR LINE NO	
void Workspace::EditorLineNo_MouseDown(Object^ Sender, MouseEventArgs^ E)
{
	EditorBox->Focus();
	int LineNo = 0, SelStart = EditorLineNo->SelectionStart; 
	try { 
		if (SelStart != -1 && EditorLineNo->GetLineFromCharIndex(SelStart) < EditorLineNo->Lines->Length && EditorLineNo->Lines->Length > 0) {
			if (ToolBarOffsetToggle->Checked) {
				LineNo = int::Parse(EditorLineNo->Lines[EditorLineNo->GetLineFromCharIndex(SelStart)], System::Globalization::NumberStyles::AllowHexSpecifier);
				UInt32 Count = 0;
				for each (UInt32 Itr in LineOffsets) {
					if (Itr == LineNo)	break;
					Count++;
				}
				LineNo = Count;
			} else LineNo = int::Parse(EditorLineNo->Lines[EditorLineNo->GetLineFromCharIndex(SelStart)]) - 1;
		} else	return;

		EditorBox->SelectionStart = EditorBox->GetFirstCharIndexFromLine(LineNo);
		EditorBox->SelectionLength = EditorBox->Lines[LineNo]->Length + 1;
	}
	catch (...) {
		return;
	}
}


//	ERROR BOX 
void Workspace::ErrorBox_DoubleClick(Object^ Sender, EventArgs^ E)
{
	if (GetListViewSelectedItem(ErrorBox) != nullptr) {
		JumpToLine(GetListViewSelectedItem(ErrorBox)->SubItems[1]->Text, false);
	}
}

void Workspace::ErrorBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
{
	if (E->Column != (int)ErrorBox->Tag) {
		ErrorBox->Tag = E->Column;
		ErrorBox->Sorting = SortOrder::Ascending;
	} else {
		if (ErrorBox->Sorting == SortOrder::Ascending)
			ErrorBox->Sorting = SortOrder::Descending;
		else
			ErrorBox->Sorting = SortOrder::Ascending;
	}

	ErrorBox->Sort();
	System::Collections::IComparer^ Sorter;
	switch (E->Column)
	{
	case 0:							
		Sorter = gcnew CSEListViewImgSorter(E->Column, ErrorBox->Sorting);
		break;
	case 1:
		Sorter = gcnew CSEListViewIntSorter(E->Column, ErrorBox->Sorting, false);
		break;
	default:
		Sorter = gcnew CSEListViewStringSorter(E->Column, ErrorBox->Sorting);
		break;
	}
	ErrorBox->ListViewItemSorter = Sorter;
}


//	FIND BOX 
void Workspace::FindBox_DoubleClick(Object^ Sender, EventArgs^ E)
{
	if (GetListViewSelectedItem(FindBox) != nullptr) {
		JumpToLine(GetListViewSelectedItem(FindBox)->SubItems[0]->Text, false);
	}
}

void Workspace::FindBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
{
	if (E->Column != (int)FindBox->Tag) {
		FindBox->Tag = E->Column;
		FindBox->Sorting = SortOrder::Ascending;
	} else {
		if (FindBox->Sorting == SortOrder::Ascending)
			FindBox->Sorting = SortOrder::Descending;
		else
			FindBox->Sorting = SortOrder::Ascending;
	}

	FindBox->Sort();
	System::Collections::IComparer^ Sorter;
	switch (E->Column)
	{
	case 0:
		Sorter = gcnew CSEListViewIntSorter(E->Column, FindBox->Sorting, false);
		break;
	default:
		Sorter = gcnew CSEListViewStringSorter(E->Column, FindBox->Sorting);
		break;
	}
	FindBox->ListViewItemSorter = Sorter;
}


//	BOOKMARK BOX 
void Workspace::BookmarkBox_DoubleClick(Object^ Sender, EventArgs^ E)
{
	if (GetListViewSelectedItem(BookmarkBox) != nullptr) {
		JumpToLine(GetListViewSelectedItem(BookmarkBox)->SubItems[0]->Text, false);
	}
}

void Workspace::BookmarkBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
{
	if (E->Column != (int)BookmarkBox->Tag) {
		BookmarkBox->Tag = E->Column;
		BookmarkBox->Sorting = SortOrder::Ascending;
	} else {
		if (BookmarkBox->Sorting == SortOrder::Ascending)
			BookmarkBox->Sorting = SortOrder::Descending;
		else
			BookmarkBox->Sorting = SortOrder::Ascending;
	}

	BookmarkBox->Sort();
	System::Collections::IComparer^ Sorter;
	switch (E->Column)
	{
	case 0:
		Sorter = gcnew CSEListViewIntSorter(E->Column, BookmarkBox->Sorting, false);
		break;
	default:
		Sorter = gcnew CSEListViewStringSorter(E->Column, BookmarkBox->Sorting);
		break;
	}
	BookmarkBox->ListViewItemSorter = Sorter;
}

// VARIABLE BOX

void Workspace::VariableBox_DoubleClick(Object^ Sender, EventArgs^ E)
{
	if (GetListViewSelectedItem(VariableBox) != nullptr) {
		ListViewItem^ Item = GetListViewSelectedItem(VariableBox);
		Rectangle Bounds = Item->SubItems[2]->Bounds;
		if (Bounds.Width > 35) {
			IndexEditBox->SetBounds(Bounds.X, Bounds.Y, Bounds.Width, Bounds.Height, BoundsSpecified::All);
			IndexEditBox->Show();
			IndexEditBox->BringToFront();
			IndexEditBox->Focus();
		} else {
			MessageBox::Show("Please expand the Index column sufficiently to allow the editing of its contents", "CSE Script Editor", 
							MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
	}		
}

void Workspace::VariableBox_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
{
	if (E->Column != (int)VariableBox->Tag) {
		VariableBox->Tag = E->Column;
		VariableBox->Sorting = SortOrder::Ascending;
	} else {
		if (VariableBox->Sorting == SortOrder::Ascending)
			VariableBox->Sorting = SortOrder::Descending;
		else
			VariableBox->Sorting = SortOrder::Ascending;
	}

	VariableBox->Sort();
	System::Collections::IComparer^ Sorter;
	switch (E->Column)
	{
	case 2:
		Sorter = gcnew CSEListViewIntSorter(E->Column, VariableBox->Sorting, false);
		break;
	default:
		Sorter = gcnew CSEListViewStringSorter(E->Column, VariableBox->Sorting);
		break;
	}
	VariableBox->ListViewItemSorter = Sorter;
}

void Workspace::IndexEditBox_LostFocus(Object^ Sender, EventArgs^ E)
{
	IndexEditBox->Hide();

	UInt32 Index = 0;
	try {
		Index = UInt32::Parse(IndexEditBox->Text);
	} catch (...) {
		IndexEditBox->Text = "";
		return;
	}

	IndexEditBox->Text = "";
	if (GetListViewSelectedItem(VariableBox) != nullptr) {
		ListViewItem^ Item = GetListViewSelectedItem(VariableBox);
		Item->SubItems[2]->Text = Index.ToString();
		Item->Tag = (int)1;
		DebugPrint("Set the index of variable '" + Item->Text + "' in script '" + EditorTab->Text + "' to " + Index.ToString());
	}
}

void Workspace::IndexEditBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	if (E->KeyCode == Keys::Enter) {
		Workspace::IndexEditBox_LostFocus(nullptr, nullptr);
	}
}

}