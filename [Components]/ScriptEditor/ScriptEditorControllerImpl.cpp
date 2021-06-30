#pragma once

#include "ScriptEditorControllerImpl.h"
#include "ScriptSync.h"
#include "IntelliSenseBackend.h"
#include "Preferences.h"
#include "ScriptEditorInstanceManager.h"

namespace cse
{


namespace scriptEditor
{


namespace controllerImpl
{


ActiveDocumentActionCollection::ActiveDocumentActionCollection()
{
	Copy = gcnew BasicAction("Copy", "Copies the current selection or line");
	Paste = gcnew BasicAction("Paste", "Pastes the contents of the clipboard at the caret location");
	Comment = gcnew BasicAction("Comment", "Comments the current selection or line");
	Uncomment = gcnew BasicAction("Uncomment", "Uncomments the current selection or line");
	AddBookmark = gcnew BasicAction("Add Bookmark", "Adds a new bookmark at the current line");
	GoToLine = gcnew BasicAction("Go to Line", "Jumps to a given line in the document");

	AddVarInteger = gcnew BasicAction("Add New Variable (Integer)", "Adds a new integer variable to the script's variable declaration");
	AddVarFloat = gcnew BasicAction("Add New Variable (Float)", "Adds a new float variable to the script's variable declaration");
	AddVarReference = gcnew BasicAction("Add New Variable (Reference)", "Adds a new reference variable to the script's variable declaration");
	AddVarString = gcnew BasicAction("Add New Variable (String)", "Adds a new string variable to the script's variable declaration");
	AddVarArray = gcnew BasicAction("Add New Variable (Array)", "Adds a new array variable to the script's variable declaration");
}

ActiveDocumentActionCollection::~ActiveDocumentActionCollection()
{
	SAFEDELETE_CLR(Copy);
	SAFEDELETE_CLR(Paste);
	SAFEDELETE_CLR(Comment);
	SAFEDELETE_CLR(Uncomment);
	SAFEDELETE_CLR(AddBookmark);
	SAFEDELETE_CLR(GoToLine);

	SAFEDELETE_CLR(AddVarInteger);
	SAFEDELETE_CLR(AddVarFloat);
	SAFEDELETE_CLR(AddVarReference);
	SAFEDELETE_CLR(AddVarString);
	SAFEDELETE_CLR(AddVarArray);
}

void ActiveDocumentActionCollection::CreateDefaultKeyBindings(components::InputManager^ InputManager)
{
	InputManager->AddKeyChordCommand(Copy, KeyCombo::New(Keys::Control, Keys::C), false);
	InputManager->AddKeyChordCommand(Paste, KeyCombo::New(Keys::Control, Keys::V), false);
	InputManager->AddKeyChordCommand(Comment, KeyCombo::New(Keys::Control, Keys::K), KeyCombo::New(Keys::Control, Keys::C), false);
	InputManager->AddKeyChordCommand(Uncomment, KeyCombo::New(Keys::Control, Keys::K), KeyCombo::New(Keys::Control, Keys::U), false);
	InputManager->AddKeyChordCommand(AddBookmark, KeyCombo::New(Keys::Control, Keys::B), false);
	InputManager->AddKeyChordCommand(GoToLine, KeyCombo::New(Keys::Control, Keys::G), false);

	InputManager->AddKeyChordCommand(AddVarInteger, KeyCombo::New(Keys::Control, Keys::L), KeyCombo::New(Keys::Control, Keys::I), false);
	InputManager->AddKeyChordCommand(AddVarFloat, KeyCombo::New(Keys::Control, Keys::L), KeyCombo::New(Keys::Control, Keys::F), false);
	InputManager->AddKeyChordCommand(AddVarReference, KeyCombo::New(Keys::Control, Keys::L), KeyCombo::New(Keys::Control, Keys::R), false);
	InputManager->AddKeyChordCommand(AddVarString, KeyCombo::New(Keys::Control, Keys::L), KeyCombo::New(Keys::Control, Keys::S), false);
	InputManager->AddKeyChordCommand(AddVarArray, KeyCombo::New(Keys::Control, Keys::L), KeyCombo::New(Keys::Control, Keys::A), false);

}

ViewActionCollection::ViewActionCollection()
{
	CurrentTabNewScript = gcnew BasicAction("New Script (Current Tab)", "Create a new script in the current tab");
	CurrentTabOpenScript = gcnew BasicAction("Open Script (Current Tab)", "Open an existing script in the current tab");
	CurrentTabSaveScript = gcnew BasicAction("Save Script (Current Tab)", "Compile and save the script open in the current tab");
	CurrentTabPreviousScript = gcnew BasicAction("Previous Script (Current Tab)", "Open the previous script relative to the script in the current tab");
	CurrentTabNextScript = gcnew BasicAction("Next Script (Current Tab)", "Open the next script relative to the script in the current tab");
	SaveAllTabs = gcnew BasicAction("Save All Open Scripts", "Compile and save all scripts in all open tabs");
	NewTabWithNewScript = gcnew BasicAction("Open New Tab with New Script", "Open a new tab and initialize it with a new script");
	NewTabWithExistingScript = gcnew BasicAction("Open New Tab with Existing Script", "Open a new tab and load an existing script into it");
	PreviousTab = gcnew BasicAction("Previous Tab", "Switch to the previous tab");
	NextTab = gcnew BasicAction("Next Tab", "Switch to the next tab");
	CloseCurrentTab = gcnew BasicAction("Close Current Tab", "Attempt to close the current tab (unsaved changes can be saved)");
	ShowFindReplacePane = gcnew BasicAction("Show Find/Replace Window", "Display the find/replace window");
}

ViewActionCollection::~ViewActionCollection()
{
	SAFEDELETE_CLR(CurrentTabNewScript);
	SAFEDELETE_CLR(CurrentTabOpenScript);
	SAFEDELETE_CLR(CurrentTabSaveScript);
	SAFEDELETE_CLR(CurrentTabPreviousScript);
	SAFEDELETE_CLR(CurrentTabNextScript);
	SAFEDELETE_CLR(SaveAllTabs);
	SAFEDELETE_CLR(NewTabWithNewScript);
	SAFEDELETE_CLR(NewTabWithExistingScript);
	SAFEDELETE_CLR(PreviousTab);
	SAFEDELETE_CLR(NextTab);
	SAFEDELETE_CLR(CloseCurrentTab);
	SAFEDELETE_CLR(ShowFindReplacePane);
}

void ViewActionCollection::CreateDefaultKeyBindings(components::InputManager^ InputManager)
{
	InputManager->AddKeyChordCommand(CurrentTabNewScript, KeyCombo::New(Keys::Control, Keys::N), false);
	InputManager->AddKeyChordCommand(CurrentTabOpenScript, KeyCombo::New(Keys::Control, Keys::O), false);
	InputManager->AddKeyChordCommand(CurrentTabSaveScript, KeyCombo::New(Keys::Control, Keys::S), false);
	InputManager->AddKeyChordCommand(CurrentTabPreviousScript, KeyCombo::New(Keys::Control | Keys::Alt, Keys::Left), false);
	InputManager->AddKeyChordCommand(CurrentTabNextScript, KeyCombo::New(Keys::Control | Keys::Alt, Keys::Right), false);
	InputManager->AddKeyChordCommand(SaveAllTabs, KeyCombo::New(Keys::Control | Keys::Shift, Keys::S), false);
	InputManager->AddKeyChordCommand(NewTabWithNewScript, KeyCombo::New(Keys::Control, Keys::T), KeyCombo::New(Keys::Control, Keys::N), false);
	InputManager->AddKeyChordCommand(NewTabWithExistingScript, KeyCombo::New(Keys::Control, Keys::T), KeyCombo::New(Keys::Control, Keys::O), false);
	InputManager->AddKeyChordCommand(PreviousTab, KeyCombo::New(Keys::Control | Keys::Shift, Keys::Tab), false);
	InputManager->AddKeyChordCommand(NextTab, KeyCombo::New(Keys::Control, Keys::Tab), false);
	InputManager->AddKeyChordCommand(CloseCurrentTab, KeyCombo::New(Keys::Control, Keys::F4), false);
	InputManager->AddKeyChordCommand(ShowFindReplacePane, KeyCombo::New(Keys::Control | Keys::Shift, Keys::F), false);
}

NewTabCreationParams::NewTabCreationParams()
{
	InitOperation = eInitOperation::NewScript;
	ExistingScriptEditorId = String::Empty;
	DiskFilePath = String::Empty;
	BindAfterInit = false;
}

void ScriptEditorController::ActiveDocumentAction_Copy()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	Document->TextEditor->InvokeDefaultCopy();
}

void ScriptEditorController::ActiveDocumentAction_Paste()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	Document->TextEditor->InvokeDefaultPaste();
}

void ScriptEditorController::ActiveDocumentAction_Comment()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	Document->TextEditor->CommentSelection();
}

void ScriptEditorController::ActiveDocumentAction_Uncomment()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	Document->TextEditor->UncommentSelection();
}

void ScriptEditorController::ActiveDocumentAction_AddBookmark()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	String^ BookmarkText = "";
	auto DialogResult = View->ShowInputPrompt("Enter Bookmark Description", "Add Bookmark", BookmarkText);
	if (DialogResult == Windows::Forms::DialogResult::Cancel)
		return;

	Document->AddBookmark(Document->TextEditor->CurrentLine, BookmarkText);
}

void ScriptEditorController::ActiveDocumentAction_GoToLine()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	String^ LineNumberStr = "";
	int LineCount = Document->TextEditor->LineCount;

	auto DialogResult = View->ShowInputPrompt("Line Number (1-" + LineCount + ")", "Go To Line", LineNumberStr);
	if (DialogResult == Windows::Forms::DialogResult::Cancel)
		return;

	int LineNumber = 0;
	if (int::TryParse(LineNumberStr, LineNumber))
		Document->TextEditor->ScrollToLine(LineNumber);
}

void AddNewVariableToDocument(obScriptParsing::Variable::eDataType DataType, model::IScriptDocument^ Document, view::IScriptEditorView^ View)
{
	if (Document == nullptr)
		return;

	String^ VariableName = "";
	auto CaretToken = Document->TextEditor->GetTokenAtCaretPos();
	auto DialogResult = View->ShowInputPrompt("Enter Variable Name", "Insert New Variable", CaretToken, VariableName);
	if (DialogResult == Windows::Forms::DialogResult::Cancel || VariableName->Length == 0)
		return;

	if (!Document->TextEditor->InsertVariable(VariableName, DataType))
	{
		View->ShowNotification("Could not insert new variable '" + VariableName + "'! Ensure that there are no other variables with the same name.",
							   view::components::CommonIcons::Get()->Error, 3500);
	}
}

void ScriptEditorController::ActiveDocumentAction_AddVarInteger()
{
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::Integer, BoundDocument, View);
}

void ScriptEditorController::ActiveDocumentAction_AddVarFloat()
{
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::Float, BoundDocument, View);
}

void ScriptEditorController::ActiveDocumentAction_AddVarReference()
{
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::Ref, BoundDocument, View);
}

void ScriptEditorController::ActiveDocumentAction_AddVarString()
{
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::StringVar, BoundDocument, View);
}

void ScriptEditorController::ActiveDocumentAction_AddVarArray()
{
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::ArrayVar, BoundDocument, View);
}

void ScriptEditorController::ViewAction_CurrentTabNewScript()
{
	View->GetComponentByRole(view::eViewRole::MainToolbar_NewScript)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_CurrentTabOpenScript()
{
	View->GetComponentByRole(view::eViewRole::MainToolbar_OpenScript)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_CurrentTabSaveScript()
{
	View->GetComponentByRole(view::eViewRole::MainToolbar_SaveScript)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_CurrentTabPreviousScript()
{
	View->GetComponentByRole(view::eViewRole::MainToolbar_PreviousScript)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_CurrentTabNextScript()
{
	View->GetComponentByRole(view::eViewRole::MainToolbar_NextScript)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_SaveAllTabs()
{
	View->GetComponentByRole(view::eViewRole::MainToolbar_SaveAllScripts)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_NewTabWithNewScript()
{
	View->GetComponentByRole(view::eViewRole::MainTabStrip_NewTab_NewScript)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_NewTabWithExistingScript()
{
	View->GetComponentByRole(view::eViewRole::MainTabStrip_NewTab_ExistingScript)->AsButton()->PerformClick();
}

void ScriptEditorController::ViewAction_PreviousTab()
{
	View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip()->SelectPreviousTab();
}

void ScriptEditorController::ViewAction_NextTab()
{
	View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip()->SelectNextTab();
}

void ScriptEditorController::ViewAction_CloseCurrentTab()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	bool Throwaway;
	CloseAndRemoveDocument(Document, Throwaway);
}

void ScriptEditorController::ViewAction_ShowFindReplacePane()
{
	View->GetComponentByRole(view::eViewRole::MainToolbar_Edit_FindReplace)->AsButton()->PerformClick();
}

void ScriptEditorController::SetBoundDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(BoundDocument == nullptr);
	Debug::Assert(Document != nullptr);
	Debug::Assert(Document->Valid);

	BoundDocument = Document;

	View->GetComponentByRole(view::eViewRole::TextEditor_ViewPortContainer)->AsContainer()->AddControl(BoundDocument->TextEditor->Container);

	BoundDocument->TextEditor->KeyDown += DelegateModelKeyDown;
	BoundDocument->TextEditor->MouseClick += DelegateModelMouseClick;
	BoundDocument->TextEditor->LineChanged += DelegateModelLineChanged;
	BoundDocument->TextEditor->ColumnChanged += DelegateModelColumnChanged;
	BoundDocument->NavigationHelper->NavigationChanged += DelegateModelNavigationChanged;

	BoundDocument->IntelliSenseModel->Bind(View->IntelliSenseView);
	BoundDocument->TextEditor->Bind();
	BoundDocument->BackgroundAnalyzer->Resume();

	BoundDocument->PushStateToSubscribers();

	ValidateDocumentSyncingStatus(BoundDocument);

	SetDocumentDependentViewComponentsEnabled(true);
	if (BoundDocument->IsPreprocessorOutputEnabled())
		SetDocumentPreprocessorOutputDisplayDependentViewComponentsEnabled(false);
}

void ScriptEditorController::UnbindBoundDocument()
{
	Debug::Assert(BoundDocument != nullptr);

	BoundDocument->BackgroundAnalyzer->Pause();
	BoundDocument->TextEditor->Unbind();
	BoundDocument->IntelliSenseModel->Unbind();

	BoundDocument->TextEditor->KeyDown -= DelegateModelKeyDown;
	BoundDocument->TextEditor->MouseClick -= DelegateModelMouseClick;
	BoundDocument->TextEditor->LineChanged -= DelegateModelLineChanged;
	BoundDocument->TextEditor->ColumnChanged -= DelegateModelColumnChanged;
	BoundDocument->NavigationHelper->NavigationChanged -= DelegateModelNavigationChanged;

	View->GetComponentByRole(view::eViewRole::TextEditor_ViewPortContainer)->AsContainer()->RemoveControl(BoundDocument->TextEditor->Container);
	SetDocumentDependentViewComponentsEnabled(false);

	BoundDocument = nullptr;
}

void ScriptEditorController::SetDocumentDependentViewComponentsEnabled(bool Enabled)
{
	array<view::eViewRole>^ AffectedComponents =
	{
		view::eViewRole::MainToolbar_NewScript,
		view::eViewRole::MainToolbar_OpenScript,
		view::eViewRole::MainToolbar_SaveScript,
		view::eViewRole::MainToolbar_SaveScriptAndActivePlugin,
		view::eViewRole::MainToolbar_SaveScriptNoCompile,
		view::eViewRole::MainToolbar_PreviousScript,
		view::eViewRole::MainToolbar_NextScript,
		view::eViewRole::MainToolbar_SaveAllScripts,
		view::eViewRole::MainToolbar_ScriptTypeDropdown,
		view::eViewRole::MainToolbar_Edit_GoToLine,
		view::eViewRole::MainToolbar_Edit_AddBookmark,
		view::eViewRole::MainToolbar_Edit_Comment,
		view::eViewRole::MainToolbar_Edit_Uncomment,
		view::eViewRole::MainToolbar_View_PreprocessorOutput,
		view::eViewRole::MainToolbar_View_BytecodeOffsets,
		view::eViewRole::MainToolbar_View_IconMargin,
		view::eViewRole::MainToolbar_Tools_SanitiseScript,
		view::eViewRole::MainToolbar_Tools_AttachScript,
		view::eViewRole::MainToolbar_Tools_RecompileScriptDependencies,
		view::eViewRole::MainToolbar_Tools_DocumentScript,
		view::eViewRole::MainToolbar_Tools_ModifyVariableIndices,
		view::eViewRole::MainToolbar_Tools_Import_IntoCurrentScript,
		view::eViewRole::MainToolbar_Tools_Import_IntoTabs,
		view::eViewRole::MainToolbar_Tools_Export_CurrentScript,
		view::eViewRole::MainToolbar_Tools_Export_AllOpenScripts,
		view::eViewRole::MainToolbar_Tools_RecompileAllActiveScripts,
		view::eViewRole::FindReplace_FindDropdown,
		view::eViewRole::FindReplace_ReplaceDropdown,
		view::eViewRole::FindReplace_LookInDropdown,
		view::eViewRole::FindReplace_MatchCase,
		view::eViewRole::FindReplace_MatchWholeWord,
		view::eViewRole::FindReplace_UseRegEx,
		view::eViewRole::FindReplace_IgnoreComments,
		view::eViewRole::FindReplace_FindButton,
		view::eViewRole::FindReplace_ReplaceButton,
		view::eViewRole::FindReplace_CountMatchesButton,
		view::eViewRole::Bookmarks_Toolbar_AddBookmark,
		view::eViewRole::Bookmarks_Toolbar_RemoveBookmark,
		view::eViewRole::TextEditor_ContextMenu_Copy,
		view::eViewRole::TextEditor_ContextMenu_Paste,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Integer,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Float,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Reference,
		view::eViewRole::TextEditor_ContextMenu_AddVar_String,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Array,
		view::eViewRole::TextEditor_ContextMenu_JumpToAttachedScript,
	};

	for each (auto Role in AffectedComponents)
	{
		auto Component = View->GetComponentByRole(Role);

		if (Component->Type == view::eComponentType::Button)
			Component->AsButton()->Enabled = Enabled;
		else if (Component->Type == view::eComponentType::ComboBox)
			Component->AsComboBox()->Enabled = Enabled;
		else
			throw gcnew NotImplementedException;
	}
}

void ScriptEditorController::SetDocumentPreprocessorOutputDisplayDependentViewComponentsEnabled(bool Enabled)
{
	array<view::eViewRole>^ AffectedComponents =
	{
		view::eViewRole::MainToolbar_NewScript,
		view::eViewRole::MainToolbar_OpenScript,
		view::eViewRole::MainToolbar_SaveScript,
		view::eViewRole::MainToolbar_SaveScriptAndActivePlugin,
		view::eViewRole::MainToolbar_SaveScriptNoCompile,
		view::eViewRole::MainToolbar_PreviousScript,
		view::eViewRole::MainToolbar_NextScript,
		view::eViewRole::MainToolbar_SaveAllScripts,
		view::eViewRole::MainToolbar_ScriptTypeDropdown,
		view::eViewRole::MainToolbar_Edit_AddBookmark,
		view::eViewRole::MainToolbar_Edit_Comment,
		view::eViewRole::MainToolbar_Edit_Uncomment,
		view::eViewRole::MainToolbar_Tools_SanitiseScript,
		view::eViewRole::MainToolbar_Tools_AttachScript,
		view::eViewRole::MainToolbar_Tools_RecompileScriptDependencies,
		view::eViewRole::MainToolbar_Tools_DocumentScript,
		view::eViewRole::MainToolbar_Tools_ModifyVariableIndices,
		view::eViewRole::MainToolbar_Tools_Import_IntoCurrentScript,
		view::eViewRole::MainToolbar_Tools_Import_IntoTabs,
		view::eViewRole::MainToolbar_Tools_Export_CurrentScript,
		view::eViewRole::MainToolbar_Tools_Export_AllOpenScripts,
		view::eViewRole::MainToolbar_Tools_RecompileAllActiveScripts,
		view::eViewRole::FindReplace_ReplaceDropdown,
		view::eViewRole::FindReplace_ReplaceButton,
		view::eViewRole::Bookmarks_Toolbar_AddBookmark,
		view::eViewRole::Bookmarks_Toolbar_RemoveBookmark,
		view::eViewRole::TextEditor_ContextMenu_Paste,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Integer,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Float,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Reference,
		view::eViewRole::TextEditor_ContextMenu_AddVar_String,
		view::eViewRole::TextEditor_ContextMenu_AddVar_Array,
	};

	for each (auto Role in AffectedComponents)
	{
		auto Component = View->GetComponentByRole(Role);

		if (Component->Type == view::eComponentType::Button)
			Component->AsButton()->Enabled = Enabled;
		else if (Component->Type == view::eComponentType::ComboBox)
			Component->AsComboBox()->Enabled = Enabled;
		else
			throw gcnew NotImplementedException;
	}
}

view::ITabStripItem^ ScriptEditorController::LookupTabStripItem(model::IScriptDocument^ Document)
{
	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto TabItem = TabStrip->LookupTabByTag(Document);

	return TabItem;
}

void ScriptEditorController::SetDocumentScriptTypeFromDropdown(model::IScriptDocument^ Document, view::components::IComboBox^ Dropdown)
{
	auto Selection = Dropdown->Selection;
	if (Selection == nullptr)
		throw gcnew InvalidOperationException("Script type dropdown must always have a selection");

	auto SelectionText = Selection->ToString();
	if (String::Equals(SelectionText, "Object Script"))
		BoundDocument->ScriptType = model::IScriptDocument::eScriptType::Object;
	else if (String::Equals(SelectionText, "Quest Script"))
		BoundDocument->ScriptType = model::IScriptDocument::eScriptType::Quest;
	else if (String::Equals(SelectionText, "Magic Effect Script"))
		BoundDocument->ScriptType = model::IScriptDocument::eScriptType::MagicEffect;
	else
		throw gcnew ArgumentException("Unexpected script type dropdown item " + SelectionText);
}

void ScriptEditorController::SetScriptTypeDropdown(view::components::IComboBox^ Dropdown, model::IScriptDocument::eScriptType ScriptType)
{
	Object^ NewSelection = nullptr;
	for each (auto Item in Dropdown->Items)
	{
		if (String::Equals(Item->ToString(), "Object Script") && ScriptType == model::IScriptDocument::eScriptType::Object)
			NewSelection = Item;
		else if (String::Equals(Item->ToString(), "Quest Script") && ScriptType == model::IScriptDocument::eScriptType::Quest)
			NewSelection = Item;
		else if (String::Equals(Item->ToString(), "Magic Effect Script") && ScriptType == model::IScriptDocument::eScriptType::MagicEffect)
			NewSelection = Item;
	}

	if (NewSelection == nullptr)
		throw gcnew ArgumentException("Couldn't find script type dropdown item for type " + ScriptType.ToString());

	Dropdown->Selection = NewSelection;
}

bool ScriptEditorController::HandleVolatileDocumentStateBeforeDestructiveOperation(model::IScriptDocument^ Document)
{
	bool Throwaway;
	return HandleVolatileDocumentStateBeforeDestructiveOperation(Document, Throwaway);
}

bool ScriptEditorController::HandleVolatileDocumentStateBeforeDestructiveOperation(model::IScriptDocument^ Document, bool% OutSaveOperationCancelled)
{
	if (!Document->Dirty)
		return true;

	// we need to be bound to the view if we want process user input
	Debug::Assert(BoundDocument == Document);

	auto PromptResult = View->ShowMessageBox("The current script '" + Document->ScriptEditorID + "' has unsaved changes.\n\nDo you wish to save them?",
											 MessageBoxButtons::YesNoCancel,
											 MessageBoxIcon::Exclamation);

	if (PromptResult == DialogResult::Yes)
		return SaveDocument(Document, model::IScriptDocument::eSaveOperation::Default);
	else if (PromptResult == DialogResult::No)
	{
		auto AutoRecoveryFile = gcnew model::components::ScriptTextAutoRecoveryCache(Document->ScriptEditorID);
		AutoRecoveryFile->Delete(false);
		return true;
	}

	OutSaveOperationCancelled = true;
	return false;
}

void ScriptEditorController::AttachDocumentToView(model::IScriptDocument^ Document)
{
	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	Debug::Assert(TabStrip->LookupTabByTag(Document) == nullptr);

	auto NewTabItem = TabStrip->AllocateNewTab();
	NewTabItem->Tag = Document;
	NewTabItem->Text = Document->ScriptEditorID;
	if (NewTabItem->Text->Length == 0)
		NewTabItem->Text = UnsavedScriptDisplayText;
	NewTabItem->Tooltip = NewTabItem->Text + "(" + Document->ScriptFormID.ToString("X8") + ")";
	NewTabItem->Image = Document->Dirty ? view::components::CommonIcons::Get()->UnsavedChanges : nullptr;

	TabStrip->AddTab(NewTabItem);

	// we need to start subscribe to certain changes in state even when the document isn't active
	Document->StateChanged += DelegateModelDocumentStateChanged;
}

void ScriptEditorController::DetachDocumentFromView(model::IScriptDocument^ Document)
{
	Debug::Assert(BoundDocument != Document);

	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto TabItem = TabStrip->LookupTabByTag(Document);

	Debug::Assert(TabItem != nullptr);

	TabStrip->RemoveTab(TabItem);
	delete TabItem;

	Document->StateChanged -= DelegateModelDocumentStateChanged;
}

void ScriptEditorController::ActivateDocumentInView(model::IScriptDocument^ Document)
{
	if (BoundDocument == Document)
		return;

	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto TabItem = TabStrip->LookupTabByTag(Document);
	Debug::Assert(TabItem != nullptr);

	TabStrip->ActiveTab = TabItem;
}

void ScriptEditorController::DisposeSelfOnViewClosure()
{
	Debug::Assert(View != nullptr);
	Debug::Assert(BoundDocument == nullptr);

	View->ComponentEvent -= DelegateViewComponentEvent;

	delete this;
}

model::IScriptDocument^ ScriptEditorController::CreateNewTab(NewTabCreationParams^ Params)
{
	model::IScriptDocument^ NewDocument = nullptr;

	switch (Params->InitOperation)
	{
	case NewTabCreationParams::eInitOperation::NewScript:
	{
		NewDocument = Model->AllocateNewDocument();
		LoadNewUnsavedScriptIntoDocument(NewDocument);
		break;
	}
	case NewTabCreationParams::eInitOperation::LoadExistingScript:
	{
		NewDocument = Model->AllocateNewDocument();
		LoadExistingScriptIntoDocument(NewDocument, Params->ExistingScriptEditorId);
		break;
	}
	case NewTabCreationParams::eInitOperation::LoadFileFromDisk:
	{
		NewDocument = ImportDocumentFromDisk(Params->DiskFilePath,
											 preferences::SettingsHolder::Get()->General->LoadScriptUpdatesExistingScripts);
		break;
	}
	}

	Model->AddDocument(NewDocument);
	AttachDocumentToView(NewDocument);
	if (Params->BindAfterInit)
		ActivateDocumentInView(NewDocument);

	return NewDocument;
}

model::IScriptDocument^ ScriptEditorController::ImportDocumentFromDisk(String^ DiskFilePath, bool ImportAsExistingScript)
{
	Debug::Assert(System::IO::File::Exists(DiskFilePath));

	model::IScriptDocument^ NewOrExisting = nullptr;
	String^ FileContents = "";
	try
	{
		auto FileParser = gcnew StreamReader(DiskFilePath);
		auto Contents = FileParser->ReadToEnd()->Replace("\r\n", "\n");
		FileParser->Close();

		auto AnalysisData = (gcnew obScriptParsing::AnalysisData())->PerformAnalysis(gcnew obScriptParsing::AnalysisData::Params);
		if (AnalysisData->Name != "")
		{
			DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ExistingScriptData(
				nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CString(AnalysisData->Name).c_str())
			);

			if (ExistingScriptData)
			{
				NewOrExisting = Model->AllocateNewDocument();
				LoadExistingScriptIntoDocument(NewOrExisting, ExistingScriptData.release());
			}
		}
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't import script from file " + DiskFilePath + "!\n\tException: " + E->Message, true);
	}

	if (NewOrExisting == nullptr)
	{
		NewOrExisting = Model->AllocateNewDocument();
		LoadNewUnsavedScriptIntoDocument(NewOrExisting);
	}

	NewOrExisting->TextEditor->SetText(FileContents, true);
	NewOrExisting->Dirty = true;

	return NewOrExisting;
}

void ScriptEditorController::LoadNewUnsavedScriptIntoDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NewScriptInstance(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CreateNewScript());
	Document->Initialize(NewScriptInstance.get(), false);
}

void ScriptEditorController::LoadExistingScriptIntoDocument(model::IScriptDocument^ Document, String^ ExistingScriptEditorId)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ExistingScriptData(
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CString(ExistingScriptEditorId).c_str())
	);
	Debug::Assert(ExistingScriptData);

	Document->Initialize(ExistingScriptData.get(), ShouldUseAutoRecoveryFile(ExistingScriptEditorId));
}

void ScriptEditorController::LoadExistingScriptIntoDocument(model::IScriptDocument^ Document, DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ExistingScriptData)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	Document->Initialize(ExistingScriptData.get(), ShouldUseAutoRecoveryFile(gcnew String(ExistingScriptData->EditorID)));
}

bool ScriptEditorController::SaveDocument(model::IScriptDocument^ Document, model::IScriptDocument::eSaveOperation SaveOperation)
{
	Debug::Assert(Document != nullptr);

	if (!Document->Dirty)
		return true;

	bool SaveResult = Document->Save(SaveOperation);
	if (!SaveResult && BoundDocument == Document)
	{
		View->ShowNotification("Script compilation failed. Check the message log for more information.",
							   view::components::CommonIcons::Get()->Error,
							   3000);
	}

	return SaveResult;
}

void ScriptEditorController::LoadNextScriptIntoDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetNextScriptInList(Document->ScriptNativeObject));
	if (Data)
		Document->Initialize(Data.get(), ShouldUseAutoRecoveryFile(gcnew String(Data->EditorID)));
}

void ScriptEditorController::LoadPreviousScriptIntoDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreviousScriptInList(Document->ScriptNativeObject));
	if (Data)
		Document->Initialize(Data.get(), ShouldUseAutoRecoveryFile(gcnew String(Data->EditorID)));
}

bool ScriptEditorController::CloseAndRemoveDocument(model::IScriptDocument^ Document, bool% OutOperationCancelled)
{
	Debug::Assert(Document != nullptr);

	bool ScriptSavedOrIsUnmodified = HandleVolatileDocumentStateBeforeDestructiveOperation(Document, OutOperationCancelled);
	if (!ScriptSavedOrIsUnmodified)
		return false;

	if (BoundDocument == Document)
		UnbindBoundDocument();

	DetachDocumentFromView(Document);
	Model->RemoveDocument(Document);
	delete Document;

	return true;
}

bool ScriptEditorController::CloseAndRemoveDocument(model::IScriptDocument^ Document)
{
	bool Throwaway;
	return CloseAndRemoveDocument(Document, Throwaway);
}

void ScriptEditorController::ValidateDocumentSyncingStatus(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!scriptSync::DiskSync::Get()->IsScriptBeingSynced(Document->ScriptEditorID))
		return;
	else if (BoundDocument != Document)
		return;

	View->ShowNotification("The current script is actively being synced from/to disk. Modifying it inside the script editor will cause inconsistent and unexpected behaviour.",
						   view::components::CommonIcons::Get()->Warning,
						   5000);
}

bool ScriptEditorController::ShouldUseAutoRecoveryFile(String^ ScriptEditorId)
{
	if (!preferences::SettingsHolder::Get()->Backup->UseAutoRecovery)
		return false;
	else if (!String::Equals(BoundDocument->ScriptEditorID, ScriptEditorId, StringComparison::CurrentCultureIgnoreCase))
		return false;

	auto AutoRecoveryFile = gcnew model::components::ScriptTextAutoRecoveryCache(ScriptEditorId);
	if (!AutoRecoveryFile->Exists)
		return false;

	auto LastWriteTime = AutoRecoveryFile->LastWriteTime;
	auto PromptResult = View->ShowMessageBox("An auto-recovery cache for the script '" + ScriptEditorId + "' was found, dated " + LastWriteTime.ToShortDateString() + " " + LastWriteTime.ToLongTimeString() + ".\n\nWould you like to load it instead?",
											 MessageBoxButtons::YesNo,
											 MessageBoxIcon::Information);
	if (PromptResult == DialogResult::No)
	{
		AutoRecoveryFile->Delete(true);
		return false;
	}

	return true;
}

void ScriptEditorController::ViewEventHandler_ComponentEvent(Object^ Sender, view::ViewComponentEvent^ E)
{
	Debug::Assert(Sender == View);

	switch (E->Component->Role)
	{
	case view::eViewRole::MainWindow:
		ViewEventHandler_MainWindow(E);
		break;
	case view::eViewRole::MainTabStrip:
	case view::eViewRole::MainTabStrip_NewTab:
	case view::eViewRole::MainTabStrip_NewTab_NewScript:
	case view::eViewRole::MainTabStrip_NewTab_ExistingScript:
		ViewEventHandler_MainTabStrip(E);
		break;
	case view::eViewRole::MainToolbar_NewScript:
	case view::eViewRole::MainToolbar_OpenScript:
	case view::eViewRole::MainToolbar_SaveScript:
	case view::eViewRole::MainToolbar_SaveScriptAndActivePlugin:
	case view::eViewRole::MainToolbar_SaveScriptNoCompile:
	case view::eViewRole::MainToolbar_PreviousScript:
	case view::eViewRole::MainToolbar_NextScript:
	case view::eViewRole::MainToolbar_SaveAllScripts:
	case view::eViewRole::MainToolbar_ScriptTypeDropdown:
		ViewEventHandler_MainToolbar(E);
		break;
	case view::eViewRole::MainToolbar_Edit:
	case view::eViewRole::MainToolbar_Edit_FindReplace:
	case view::eViewRole::MainToolbar_Edit_GoToLine:
	case view::eViewRole::MainToolbar_Edit_AddBookmark:
	case view::eViewRole::MainToolbar_Edit_Comment:
	case view::eViewRole::MainToolbar_Edit_Uncomment:
		ViewEventHandler_MainToolbarMenuEdit(E);
		break;
	case view::eViewRole::MainToolbar_View:
	case view::eViewRole::MainToolbar_View_PreprocessorOutput:
	case view::eViewRole::MainToolbar_View_BytecodeOffsets:
	case view::eViewRole::MainToolbar_View_IconMargin:
	case view::eViewRole::MainToolbar_View_Messages:
	case view::eViewRole::MainToolbar_View_Bookmarks:
	case view::eViewRole::MainToolbar_View_OutlineView:
	case view::eViewRole::MainToolbar_View_FindReplaceResults:
	case view::eViewRole::MainToolbar_View_FindInTabsResults:
	case view::eViewRole::MainToolbar_View_NavigationBar:
	case view::eViewRole::MainToolbar_View_DarkMode:
		ViewEventHandler_MainToolbarMenuView(E);
		break;
	case view::eViewRole::MainToolbar_Tools:
	case view::eViewRole::MainToolbar_Tools_SanitiseScript:
	case view::eViewRole::MainToolbar_Tools_AttachScript:
	case view::eViewRole::MainToolbar_Tools_RecompileScriptDependencies:
	case view::eViewRole::MainToolbar_Tools_DocumentScript:
	case view::eViewRole::MainToolbar_Tools_ModifyVariableIndices:
	case view::eViewRole::MainToolbar_Tools_Import_IntoCurrentScript:
	case view::eViewRole::MainToolbar_Tools_Import_IntoTabs:
	case view::eViewRole::MainToolbar_Tools_Export_CurrentScript:
	case view::eViewRole::MainToolbar_Tools_Export_AllOpenScripts:
	case view::eViewRole::MainToolbar_Tools_RecompileAllActiveScripts:
	case view::eViewRole::MainToolbar_Tools_DeleteScripts:
	case view::eViewRole::MainToolbar_Tools_SyncScripts:
	case view::eViewRole::MainToolbar_Tools_CodeSnippets:
	case view::eViewRole::MainToolbar_Tools_Preferences:
		ViewEventHandler_MainToolbarMenuTools(E);
		break;
	case view::eViewRole::MainToolbar_Help_Wiki:
	case view::eViewRole::MainToolbar_Help_OBSE:
		ViewEventHandler_MainToolbarMenuHelp(E);
		break;
	case view::eViewRole::FindReplace_DockPanel:
	case view::eViewRole::FindReplace_FindDropdown:
	case view::eViewRole::FindReplace_ReplaceDropdown:
	case view::eViewRole::FindReplace_LookInDropdown:
	case view::eViewRole::FindReplace_MatchCase:
	case view::eViewRole::FindReplace_MatchWholeWord:
	case view::eViewRole::FindReplace_UseRegEx:
	case view::eViewRole::FindReplace_IgnoreComments:
	case view::eViewRole::FindReplace_FindButton:
	case view::eViewRole::FindReplace_ReplaceButton:
	case view::eViewRole::FindReplace_CountMatchesButton:
		FindReplaceHelper->HandleFindReplaceEvent(E, this);
		break;
	case view::eViewRole::Messages_DockPanel:
	case view::eViewRole::Messages_ListView:
		ViewEventHandler_MessagesPanel(E);
		break;
	case view::eViewRole::Bookmarks_DockPanel:
	case view::eViewRole::Bookmarks_ListView:
	case view::eViewRole::Bookmarks_Toolbar_AddBookmark:
	case view::eViewRole::Bookmarks_Toolbar_RemoveBookmark:
		ViewEventHandler_BookmarksPanel(E);
		break;
	case view::eViewRole::FindReplaceResults_DockPanel:
	case view::eViewRole::FindReplaceResults_Query:
	case view::eViewRole::FindReplaceResults_ListView:
	case view::eViewRole::GlobalFindReplaceResults_DockPanel:
	case view::eViewRole::GlobalFindReplaceResults_TreeView:
		FindReplaceHelper->HandleResultsViewEvent(E, this);
		break;
	case view::eViewRole::OutlineView_DockPanel:
	case view::eViewRole::OutlineView_TreeView:
		DocumentNavigationHelper->HandleListViewEvent(E, BoundDocument);
		break;
	case view::eViewRole::NavigationBar:
		DocumentNavigationHelper->HandleCrumbBarEvent(E, BoundDocument);
		break;
	case view::eViewRole::TextEditor_ContextMenu:
	case view::eViewRole::TextEditor_ContextMenu_Copy:
	case view::eViewRole::TextEditor_ContextMenu_Paste:
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Integer:
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Float:
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Reference:
	case view::eViewRole::TextEditor_ContextMenu_AddVar_String:
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Array:
	case view::eViewRole::TextEditor_ContextMenu_JumpToAttachedScript:
		ViewEventHandler_TextEditorContextMenu(E);
		break;
	default:
		throw gcnew ArgumentException("Unknown view component role " + E->Component->Role.ToString());
	}
}

void ScriptEditorController::ViewEventHandler_MainWindow(view::ViewComponentEvent^ E)
{
	auto Form = E->Component->AsForm();
	auto EventType = safe_cast<view::components::IForm::eEvent>(E->EventType);
	switch (EventType)
	{
	case view::components::IForm::eEvent::KeyDown:
		InputManager->HandleKeyDown(safe_cast<view::components::IForm::KeyDownEventArgs^>(E->EventArgs)->KeyEvent, this);
		break;
	case view::components::IForm::eEvent::Closing:
	{
		auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
		auto OpenTabs = TabStrip->TabCount;
		Debug::Assert(OpenTabs == Model->Documents->Count);

		if (OpenTabs == 0)
		{
			DisposeSelfOnViewClosure();
			break;
		}

		auto Args = safe_cast<view::components::IForm::ClosingEventArgs^>(E->EventArgs);
		if (OpenTabs > 1)
		{
			auto DialogResult = View->ShowMessageBox("Are you sure you want to close all open scripts?",
													 MessageBoxButtons::YesNo, MessageBoxIcon::Question);
			if (DialogResult == DialogResult::No)
			{
				Args->Cancel = true;
				break;
			}
		}

		auto ModelDocumentsCopy = gcnew List<model::IScriptDocument^>(Model->Documents);
		bool CloseOperationCancelled = false;

		// we need to disable the default tab switching behaviour since removing
		// a tab will cause the next tab in the tabstrip to automatically get activated
		bool StopClosure = false;
		DisableDocumentActivationOnTabSwitch = true;
		Form->BeginUpdate();
		{
			for each (auto Doc in ModelDocumentsCopy)
			{
				// switch to dirty models to enable user interaction
				if (Doc->Dirty)
					ActivateDocumentInView(Doc);

				bool CloseSuccess = CloseAndRemoveDocument(Doc, CloseOperationCancelled);
				if (!CloseSuccess || CloseOperationCancelled)
				{
					// cancel the close operation and let the user handle any errors, etc
					// also, ensure that document in question is the active one
					ActivateDocumentInView(Doc);
					StopClosure = true;
					break;
				}
			}
		}
		Form->EndUpdate();
		DisableDocumentActivationOnTabSwitch = false;

		if (StopClosure)
		{
			Args->Cancel = true;
			break;
		}

		Debug::Assert(Model->Documents->Count == 0);
		Debug::Assert(Model->Documents->Count == TabStrip->TabCount);
		DisposeSelfOnViewClosure();

		break;
	}
	}
}

void ScriptEditorController::ViewEventHandler_MainTabStrip(view::ViewComponentEvent^ E)
{
	if (E->Component->Type == view::eComponentType::Button)
	{
		switch (E->Component->Role)
		{
		case view::eViewRole::MainTabStrip_NewTab_NewScript:
		{
			auto Params = gcnew NewTabCreationParams;
			Params->BindAfterInit = true;
			Params->InitOperation = NewTabCreationParams::eInitOperation::NewScript;
			CreateNewTab(Params);

			break;
		}
		case view::eViewRole::MainTabStrip_NewTab_ExistingScript:
		{
			auto Params = gcnew NewTabCreationParams;
			Params->InitOperation = NewTabCreationParams::eInitOperation::LoadExistingScript;

			auto ExistingScriptEditorIds = View->SelectExistingScripts(BoundDocument ? BoundDocument->ScriptEditorID : "");
			bool FirstScript = true;

			//View->BeginUpdate();
			{
				for each (auto EditorId in ExistingScriptEditorIds)
				{
					if (FirstScript)
					{
						Params->BindAfterInit = true;
						FirstScript = false;
					}

					Params->ExistingScriptEditorId = EditorId;
					CreateNewTab(Params);
				}
			}
			//View->EndUpdate();

			break;
		}
		}

		return;
	}

	auto TabStrip = E->Component->AsTabStrip();
	auto EventType = safe_cast<view::components::ITabStrip::eEvent>(E->EventType);
	switch (EventType)
	{
	case view::components::ITabStrip::eEvent::TabClick:
	{
		auto Args = safe_cast<view::components::ITabStrip::TabClickEventArgs^>(E->EventArgs);
		if (Args->MouseOverTab == nullptr)
			break;

		if (Args->MouseEvent->Button == MouseButtons::Middle)
		{
			auto Document = safe_cast<model::IScriptDocument^>(Args->MouseOverTab->Tag);

			if (Document->Dirty)
				ActivateDocumentInView(Document);
			CloseAndRemoveDocument(Document);
		}

		break;
	}
	case view::components::ITabStrip::eEvent::TabMoving:
	{
		if (components::ViewTabTearingHelper::Get()->InProgress)
			break;

		auto Args = safe_cast<view::components::ITabStrip::TabMovingEventArgs^>(E->EventArgs);
		if (Args->Tab == nullptr)
			break;

		auto Document = safe_cast<model::IScriptDocument^>(Args->Tab->Tag);
		components::ViewTabTearingHelper::Get()->InitiateHandling(Document, this);

		break;
	}
	case view::components::ITabStrip::eEvent::TabClosing:
	{
		auto Args = safe_cast<view::components::ITabStrip::TabClosingEventArgs^>(E->EventArgs);
		auto Document = safe_cast<model::IScriptDocument^>(Args->Tab->Tag);

		// cancel the default handler as we take care of the tab item ourselves
		Args->Cancel = true;

		if (Document->Dirty)
			ActivateDocumentInView(Document);
		CloseAndRemoveDocument(Document);

		break;
	}
	case view::components::ITabStrip::eEvent::ActiveTabChanged:
	{
		if (DisableDocumentActivationOnTabSwitch)
			break;

		auto Args = safe_cast<view::components::ITabStrip::ActiveTabChangedEventArgs^>(E->EventArgs);
		if (Args->NewValue == nullptr)
			break;

		auto OldDocument = safe_cast<model::IScriptDocument^>(Args->OldValue->Tag);
		auto NewDocument = safe_cast<model::IScriptDocument^>(Args->NewValue->Tag);

		View->BeginUpdate();
		{
			if (OldDocument)
			{
				Debug::Assert(OldDocument == BoundDocument);
				UnbindBoundDocument();
			}
			SetBoundDocument(NewDocument);
		}
		View->EndUpdate();

		break;
	}
	}
}

void ScriptEditorController::ViewEventHandler_MainToolbar(view::ViewComponentEvent^ E)
{
	switch (E->Component->Role)
	{
	case view::eViewRole::MainToolbar_NewScript:
		LoadNextScriptIntoDocument(BoundDocument);
		break;
	case view::eViewRole::MainToolbar_OpenScript:
	{
		auto Params = gcnew NewTabCreationParams;
		Params->InitOperation = NewTabCreationParams::eInitOperation::LoadExistingScript;

		auto ExistingScriptEditorIds = View->SelectExistingScripts(BoundDocument->ScriptEditorID);
		bool FirstScript = true;

		//View->BeginUpdate();
		{
			for each (auto EditorId in ExistingScriptEditorIds)
			{
				if (FirstScript)
				{
					LoadExistingScriptIntoDocument(BoundDocument, EditorId);
					FirstScript = false;
				}
				else
				{
					Params->ExistingScriptEditorId = EditorId;
					CreateNewTab(Params);
				}
			}
		}
		//View->EndUpdate();

		break;
	}
	case view::eViewRole::MainToolbar_SaveScript:
	case view::eViewRole::MainToolbar_SaveScriptAndActivePlugin:
	case view::eViewRole::MainToolbar_SaveScriptNoCompile:
	{
		auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
		if (EventType != view::components::IButton::eEvent::Click)
			break;		// need to handle this explicitly as the buttons appear in a popup menu

		if (E->Component->Role == view::eViewRole::MainToolbar_SaveScript)
			SaveDocument(BoundDocument, model::IScriptDocument::eSaveOperation::Default);
		else if (E->Component->Role == view::eViewRole::MainToolbar_SaveScriptAndActivePlugin)
			SaveDocument(BoundDocument, model::IScriptDocument::eSaveOperation::AlsoSaveActiveFile);
		else
			SaveDocument(BoundDocument, model::IScriptDocument::eSaveOperation::DontCompile);

		break;
	}
	case view::eViewRole::MainToolbar_PreviousScript:
		LoadPreviousScriptIntoDocument(BoundDocument);
		break;
	case view::eViewRole::MainToolbar_NextScript:
		LoadNextScriptIntoDocument(BoundDocument);
		break;
	case view::eViewRole::MainToolbar_SaveAllScripts:
	{
		for each (auto Document in Model->Documents)
		{
			if (!Document->Dirty)
				continue;

			SaveDocument(Document, model::IScriptDocument::eSaveOperation::Default);
		}
		break;
	}
	case view::eViewRole::MainToolbar_ScriptTypeDropdown:
	{
		auto EventType = safe_cast<view::components::IComboBox::eEvent>(E->EventType);
		if (EventType != view::components::IComboBox::eEvent::SelectionChanged)
			break;

		auto Selection = E->Component->AsComboBox()->Selection;
		Debug::Assert(Selection != nullptr);

		SetDocumentScriptTypeFromDropdown(BoundDocument, E->Component->AsComboBox());
		break;
	}
	}
}

void ScriptEditorController::ViewEventHandler_MainToolbarMenuEdit(view::ViewComponentEvent^ E)
{
	auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
	if (EventType != view::components::IButton::eEvent::Click)
		return;

	switch (E->Component->Role)
	{
	case view::eViewRole::MainToolbar_Edit_FindReplace:
		ViewActions->ShowFindReplacePane->Invoke();
		break;
	case view::eViewRole::MainToolbar_Edit_GoToLine:
		ActiveDocumentActions->GoToLine->Invoke();
		break;
	case view::eViewRole::MainToolbar_Edit_AddBookmark:
		ActiveDocumentActions->AddBookmark->Invoke();
		break;
	case view::eViewRole::MainToolbar_Edit_Comment:
		ActiveDocumentActions->Comment->Invoke();
		break;
	case view::eViewRole::MainToolbar_Edit_Uncomment:
		ActiveDocumentActions->Uncomment->Invoke();
		break;
	}
}

void HandlePreferencesButtonEvent(view::components::IButton::eEvent EventType, view::components::IButton^ Button, bool% PreferenceValue)
{
	if (EventType == view::components::IButton::eEvent::PopupOpening)
		Button->Checked = PreferenceValue;
	else if (EventType == view::components::IButton::eEvent::Click)
	{
		PreferenceValue = PreferenceValue == false;
		preferences::SettingsHolder::Get()->RaisePreferencesChangedEvent();
	}
	else
		throw gcnew NotImplementedException();
}


void HandlePreferencesDockablePaneEvent(view::components::IButton::eEvent EventType, view::components::IButton^ Button, view::components::IDockablePane^ Pane)
{
	if (EventType == view::components::IButton::eEvent::PopupOpening)
		Button->Checked = Pane->Visible;
	else if (EventType == view::components::IButton::eEvent::Click)
		Pane->Visible = Pane->Visible == false;
	else
		throw gcnew NotImplementedException();
}

void ScriptEditorController::ViewEventHandler_MainToolbarMenuView(view::ViewComponentEvent^ E)
{
	auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
	auto Button = E->Component->AsButton();
	auto OnClick = EventType == view::components::IButton::eEvent::Click;
	auto OnPopupOpening = EventType == view::components::IButton::eEvent::PopupOpening;

	switch (E->Component->Role)
	{
	case view::eViewRole::MainToolbar_View_PreprocessorOutput:
		if (OnPopupOpening)
			Button->Checked = BoundDocument->IsPreprocessorOutputEnabled();
		else if (OnClick)
		{
			bool IsTurnedOn = BoundDocument->IsPreprocessorOutputEnabled();

			SetDocumentPreprocessorOutputDisplayDependentViewComponentsEnabled(!IsTurnedOn);
			BoundDocument->TogglePreprocessorOutput(IsTurnedOn == false);
		}

		break;
	case view::eViewRole::MainToolbar_View_BytecodeOffsets:
		HandlePreferencesButtonEvent(EventType, Button, preferences::SettingsHolder::Get()->Appearance->ShowBytecodeOffsetMargin);
		break;
	case view::eViewRole::MainToolbar_View_IconMargin:
		HandlePreferencesButtonEvent(EventType, Button, preferences::SettingsHolder::Get()->Appearance->ShowIconMargin);
		break;
	case view::eViewRole::MainToolbar_View_Messages:
		HandlePreferencesDockablePaneEvent(EventType, Button,
										   View->GetComponentByRole(view::eViewRole::Messages_DockPanel)->AsDockablePane());
		break;
	case view::eViewRole::MainToolbar_View_Bookmarks:
		HandlePreferencesDockablePaneEvent(EventType, Button,
										   View->GetComponentByRole(view::eViewRole::Bookmarks_DockPanel)->AsDockablePane());
		break;
	case view::eViewRole::MainToolbar_View_OutlineView:
		HandlePreferencesDockablePaneEvent(EventType, Button,
										   View->GetComponentByRole(view::eViewRole::OutlineView_DockPanel)->AsDockablePane());
		break;
	case view::eViewRole::MainToolbar_View_FindReplaceResults:
		HandlePreferencesDockablePaneEvent(EventType, Button,
										   View->GetComponentByRole(view::eViewRole::FindReplaceResults_DockPanel)->AsDockablePane());
		break;
	case view::eViewRole::MainToolbar_View_FindInTabsResults:
		HandlePreferencesDockablePaneEvent(EventType, Button,
										   View->GetComponentByRole(view::eViewRole::GlobalFindReplaceResults_DockPanel)->AsDockablePane());
		break;
	case view::eViewRole::MainToolbar_View_NavigationBar:
		HandlePreferencesButtonEvent(EventType, Button, preferences::SettingsHolder::Get()->Appearance->ShowScopeBar);
		break;
	case view::eViewRole::MainToolbar_View_DarkMode:
		HandlePreferencesButtonEvent(EventType, Button, preferences::SettingsHolder::Get()->Appearance->DarkMode);
		break;
	}
}

void ScriptEditorController::ViewEventHandler_MainToolbarMenuTools(view::ViewComponentEvent^ E)
{
	auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
	if (EventType != view::components::IButton::eEvent::Click)
		return;

	switch (E->Component->Role)
	{
	case view::eViewRole::MainToolbar_Tools_SanitiseScript:
		if (!BoundDocument->SanitizeScriptText())
		{
			View->ShowNotification("Operation failed! Please fix all structural errors in the script before trying again.",
								   view::components::CommonIcons::Get()->Error, 5000);
		}

		break;
	case view::eViewRole::MainToolbar_Tools_AttachScript:
		if (BoundDocument->UnsavedNewScript)
		{
			View->ShowNotification("Please save the current script first.",
								   view::components::CommonIcons::Get()->Info, 3000);
			break;
		}

		nativeWrapper::g_CSEInterfaceTable->ScriptEditor.BindScript(CString(BoundDocument->ScriptEditorID).c_str(),
																	safe_cast<HWND>(View->WindowHandle));
		break;
	case view::eViewRole::MainToolbar_Tools_RecompileScriptDependencies:
		if (BoundDocument->UnsavedNewScript)
		{
			View->ShowNotification("Please save the current script first.",
								   view::components::CommonIcons::Get()->Info, 3000);
			break;
		}

		nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileDependencies(CString(BoundDocument->ScriptEditorID).c_str());
		View->ShowNotification("Operation complete!\n\nScript variables used as condition parameters will need to be corrected manually. The results have been logged to the console.",
							   view::components::CommonIcons::Get()->Success, 6000);
		break;
	case view::eViewRole::MainToolbar_Tools_DocumentScript:
		View->ShowNotification("Implement this, you!",
							   view::components::CommonIcons::Get()->Invalid, 3000);
		break;
	case view::eViewRole::MainToolbar_Tools_ModifyVariableIndices:
		View->ShowNotification("Implement this, you!",
							   view::components::CommonIcons::Get()->Invalid, 3000);
		break;
	case view::eViewRole::MainToolbar_Tools_Import_IntoCurrentScript:
	{
		OpenFileDialog^ LoadManager = gcnew OpenFileDialog();
		LoadManager->Filter = "Oblivion Script Files|*" + preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension + "| All files(*.*) | *.*";
		LoadManager->DefaultExt = "*" + preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension;
		LoadManager->RestoreDirectory = true;

		if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileName->Length > 0)
			BoundDocument->LoadScriptTextFromDisk(LoadManager->FileName);
		break;
	}
	case view::eViewRole::MainToolbar_Tools_Import_IntoTabs:
	{
		OpenFileDialog^ LoadManager = gcnew OpenFileDialog();
		LoadManager->Filter = "Oblivion Script Files|*" + preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension + "| All files(*.*) | *.*";
		LoadManager->DefaultExt = "*" + preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension;
		LoadManager->RestoreDirectory = true;
		LoadManager->Multiselect = true;

		if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileNames->Length > 0)
		{
			auto NewTabParam = gcnew NewTabCreationParams;
			NewTabParam->InitOperation = NewTabCreationParams::eInitOperation::LoadFileFromDisk;

			for each (auto FilePath in LoadManager->FileNames)
			{
				NewTabParam->DiskFilePath = FilePath;
				CreateNewTab(NewTabParam);
			}
		}
		break;
	}
	case view::eViewRole::MainToolbar_Tools_Export_CurrentScript:
	{
		SaveFileDialog^ SaveManager = gcnew SaveFileDialog();
		SaveManager->Filter = "Oblivion Script Files|*" + preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension + "| All files(*.*) | *.*";
		SaveManager->DefaultExt = "*" + preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension;
		SaveManager->FileName = BoundDocument->ScriptEditorID;
		SaveManager->RestoreDirectory = true;

		if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->FileName->Length > 0)
			BoundDocument->SaveScriptTextToDisk(SaveManager->FileName);

		break;
	}
	case view::eViewRole::MainToolbar_Tools_Export_AllOpenScripts:
	{
		auto SaveManager = gcnew FolderBrowserDialog();

		SaveManager->Description = "All open scripts (except unsaved, new scripts) in this window will be exported by their editor IDs to the selected folder.";
		SaveManager->ShowNewFolderButton = true;
		SaveManager->SelectedPath = gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetDefaultCachePath());

		if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->SelectedPath->Length > 0)
		{
			for each (auto Document in Model->Documents)
			{
				if (Document->UnsavedNewScript)
					continue;

				auto FilePath = SaveManager->SelectedPath + "\\" + Document->ScriptEditorID + preferences::SettingsHolder::Get()->General->ExportedScriptFileExtension;
				Document->SaveScriptTextToDisk(FilePath);
			}
		}

		break;
	}
	case view::eViewRole::MainToolbar_Tools_RecompileAllActiveScripts:
	{
		auto DialogResult = View->ShowMessageBox("Are you sure you want to recompile all the scripts in the active plugin file?",
												 MessageBoxButtons::YesNo, MessageBoxIcon::Exclamation);
		if (DialogResult == DialogResult::No)
			break;

		nativeWrapper::g_CSEInterfaceTable->ScriptEditor.RecompileAllScriptsInActiveFile();
		View->ShowNotification("All scripts in the active plugin file have been recompiled. Results have been logged to the console.",
							   view::components::CommonIcons::Get()->Info, 4000);
		break;
	}
	case view::eViewRole::MainToolbar_Tools_DeleteScripts:
	{
		auto ExistingScriptEditorIds = View->SelectExistingScripts(BoundDocument ? BoundDocument->ScriptEditorID : "");
		UInt32 DeletedScripts = 0;

		for each (auto EditorId in ExistingScriptEditorIds)
		{
			if (nativeWrapper::g_CSEInterfaceTable->ScriptEditor.DeleteScript(CString(EditorId).c_str()))
				++DeletedScripts;
		}

		if (DeletedScripts > 0)
		{
			View->ShowNotification("Marked " + DeletedScripts + " script(s) as deleted.",
								   view::components::CommonIcons::Get()->Success, 4000);
		}

		break;
	}
	case view::eViewRole::MainToolbar_Tools_SyncScripts:
		scriptSync::DiskSyncDialog::Show();

		break;
	case view::eViewRole::MainToolbar_Tools_CodeSnippets:
		intellisense::IntelliSenseBackend::Get()->ShowCodeSnippetManager();

		break;
	case view::eViewRole::MainToolbar_Tools_Preferences:
	{
		preferences::PreferencesDialog PreferencesDialog;

		break;
	}
	}
}

void ScriptEditorController::ViewEventHandler_MainToolbarMenuHelp(view::ViewComponentEvent^ E)
{
	auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
	if (EventType != view::components::IButton::eEvent::Click)
		return;

	switch (E->Component->Role)
	{
	case view::eViewRole::MainToolbar_Help_Wiki:
		Process::Start("https://cs.elderscrolls.com/index.php?title=Main_Page");
		break;
	case view::eViewRole::MainToolbar_Help_OBSE:
		Process::Start("https://htmlpreview.github.io/?https://github.com/llde/xOBSE/blob/master/obse_command_doc.html");
		break;
	}
}

void HandleScriptLineAnnotationItemActivation(view::ViewComponentEvent^ E, model::IScriptDocument^ Document)
{
	auto EventType = safe_cast<view::components::IObjectListView::eEvent>(E->EventType);
	if (EventType == view::components::IObjectListView::eEvent::ItemActivate)
	{
		auto EventArgs = safe_cast<view::components::IObjectListView::ItemActivateEventArgs^>(E->EventArgs);
		if (EventArgs->ItemModel == nullptr)
			return;

		auto Annotation = safe_cast<model::components::ScriptLineAnnotation^>(EventArgs->ItemModel);
		Document->TextEditor->ScrollToLine(Annotation->Line);
	}
}

void ScriptEditorController::ViewEventHandler_MessagesPanel(view::ViewComponentEvent^ E)
{
	switch (E->Component->Role)
	{
	case view::eViewRole::Messages_ListView:
		HandleScriptLineAnnotationItemActivation(E, BoundDocument);
		break;
	}
}

void ScriptEditorController::ViewEventHandler_BookmarksPanel(view::ViewComponentEvent^ E)
{
	switch (E->Component->Role)
	{
	case view::eViewRole::Bookmarks_ListView:
		HandleScriptLineAnnotationItemActivation(E, BoundDocument);
		break;
	case view::eViewRole::Bookmarks_Toolbar_AddBookmark:
		ActiveDocumentActions->AddBookmark->Invoke();
		break;
	case view::eViewRole::Bookmarks_Toolbar_RemoveBookmark:
	{
		auto ListView = View->GetComponentByRole(view::eViewRole::Bookmarks_ListView)->AsObjectListView();
		for each (auto Item in ListView->SelectedObjects)
		{
			auto Bookmark = safe_cast<model::components::ScriptBookmark^>(Item);
			BoundDocument->RemoveBookmark(Bookmark);
		}

		break;
	}
	}
}

void ScriptEditorController::ViewEventHandler_TextEditorContextMenu(view::ViewComponentEvent^ E)
{
	auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
	bool OnClick = EventType == view::components::IButton::eEvent::Click;

	switch (E->Component->Role)
	{
	case view::eViewRole::TextEditor_ContextMenu:
	{
		if (EventType == view::components::IButton::eEvent::PopupOpening)
		{
			auto Args = safe_cast<view::components::IButton::PopupOpeningEventArgs^>(E->EventArgs);
			if (BoundDocument == nullptr)
				Args->Cancel = true;
		}

		break;
	}
	case view::eViewRole::TextEditor_ContextMenu_Copy:
		if (OnClick)
			ActiveDocumentActions->Copy->Invoke();

		break;
	case view::eViewRole::TextEditor_ContextMenu_Paste:
		if (OnClick)
			ActiveDocumentActions->Paste->Invoke();

		break;
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Integer:
		if (OnClick)
			ActiveDocumentActions->AddVarInteger->Invoke();

		break;
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Float:
		if (OnClick)
			ActiveDocumentActions->AddVarFloat->Invoke();

		break;
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Reference:
		if (OnClick)
			ActiveDocumentActions->AddVarReference->Invoke();

		break;
	case view::eViewRole::TextEditor_ContextMenu_AddVar_String:
		if (OnClick)
			ActiveDocumentActions->AddVarString->Invoke();

		break;
	case view::eViewRole::TextEditor_ContextMenu_AddVar_Array:
		if (OnClick)
			ActiveDocumentActions->AddVarArray->Invoke();

		break;
	case view::eViewRole::TextEditor_ContextMenu_JumpToAttachedScript:
	{
		auto Button = E->Component->AsButton();
		if (OnClick && Button->Tag)
			ActivateOrCreateNewDocument(Button->Tag->ToString());
		else
		{
			auto RelativeCoords = BoundDocument->TextEditor->ScreenToClient(Cursor::Position);
			auto TextIndex = BoundDocument->TextEditor->GetCharIndexFromPosition(RelativeCoords);
			auto TokenAtPopup = BoundDocument->TextEditor->GetTokenAtCharIndex(TextIndex);
			auto AttachedScript = intellisense::IntelliSenseBackend::Get()->GetAttachedScript(TokenAtPopup);

			if (AttachedScript)
			{
				Button->Text = "Open Attached Script (" + AttachedScript->GetIdentifier() + ")";
				Button->Visible = true;
				Button->Tag = AttachedScript->GetIdentifier();
			}
			else
				Button->Visible = false;
		}

		break;
	}
	}
}

void ScriptEditorController::ModelEventHandler_DocumentStateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E)
{
	auto Document = safe_cast<model::IScriptDocument^>(Sender);

	switch (E->EventType)
	{
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Dirty:
	{
		auto TabItem = LookupTabStripItem(Document);
		Debug::Assert(TabItem != nullptr);

		TabItem->Image = E->Dirty ? view::components::CommonIcons::Get()->UnsavedChanges : nullptr;

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::BytecodeLength:
	{
		if (Document != BoundDocument)
			break;

		auto BytecodeProgress = View->GetComponentByRole(view::eViewRole::StatusBar_CompiledScriptSize)->AsProgressBar();

		if (E->BytecodeLength == 0)
		{
			BytecodeProgress->Text = "Uncompiled Script";
			BytecodeProgress->Value = 0;
			BytecodeProgress->Minimum = 0;
			BytecodeProgress->Maximum = 0;
			BytecodeProgress->Tooltip = "";
		}
		else
		{
			BytecodeProgress->Text = "Compiled Script Size:";
			BytecodeProgress->Value = E->BytecodeLength;
			BytecodeProgress->Minimum = 0;
			BytecodeProgress->Maximum = model::IScriptDocument::MaximumScriptBytecodeLength;
			BytecodeProgress->Tooltip = String::Format("Compiled Script Size: {0:F2} KB", static_cast<float>(E->BytecodeLength / 1024.0));
		}

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::ScriptType:
	{
		if (Document != BoundDocument)
			break;

		SetScriptTypeDropdown(View->GetComponentByRole(view::eViewRole::MainToolbar_ScriptTypeDropdown)->AsComboBox(), E->ScriptType);
		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::EditorIdAndFormId:
	{
		auto TabItem = LookupTabStripItem(Document);
		Debug::Assert(TabItem != nullptr);

		TabItem->Text = E->EditorId;
		if (TabItem->Text->Length == 0)
			TabItem->Text = UnsavedScriptDisplayText;
		TabItem->Tooltip = TabItem->Text + "(" + E->FormId.ToString("X8") + ")";

		if (Document == BoundDocument)
		{
			auto Form = View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm();
			Form->Text = TabItem->Tooltip + " -- " + view::IScriptEditorView::MainWindowDefaultTitle;
		}

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Messages:
	{
		if (Document != BoundDocument)
			break;

		auto ListView = View->GetComponentByRole(view::eViewRole::Messages_ListView)->AsObjectListView();
		ListView->SetObjects(E->Messages, true);

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Bookmarks:
	{
		if (Document != BoundDocument)
			break;

		auto ListView = View->GetComponentByRole(view::eViewRole::Bookmarks_ListView)->AsObjectListView();
		ListView->SetObjects(E->Bookmarks, true);

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::FindResults:
	{
		if (Document != BoundDocument)
			break;

		auto ListView = View->GetComponentByRole(view::eViewRole::FindReplaceResults_ListView)->AsObjectListView();
		ListView->SetObjects(E->FindResults, true);

		auto QueryLabel = View->GetComponentByRole(view::eViewRole::FindReplaceResults_Query)->AsLabel();
		if (E->FindResults->Count > 0)
			QueryLabel->Text = E->FindResults[0]->Query;
		else
			QueryLabel->Text = "--";

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::DisplayingPreprocessorOutput:
	{
		if (Document != BoundDocument)
			break;

		auto StatusBarLabel = View->GetComponentByRole(view::eViewRole::StatusBar_PreprocessorOutput)->AsLabel();
		StatusBarLabel->Visible = E->DisplayingPreprocessorOutput;
	}
	default:
		throw gcnew ArgumentException("Unknown state document change event type " + E->EventType.ToString());
	}
}

void ScriptEditorController::ModelEventHandler_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	InputManager->HandleKeyDown(E, this);
}

void ScriptEditorController::ModelEventHandler_MouseClick(Object^ Sender, textEditor::TextEditorMouseClickEventArgs^ E)
{
	InputManager->HandleMouseClick(E, this);
}

void ScriptEditorController::ModelEventHandler_NavigationChanged(Object^ Sender, model::components::INavigationHelper::NavigationChangedEventArgs^ E)
{
	auto Document = safe_cast<model::IScriptDocument^>(Sender);
	Debug::Assert(Document == BoundDocument);

	DocumentNavigationHelper->HandleNavigationChangedEvent(E, Document);
}

void ScriptEditorController::ModelEventHandler_LineChanged(Object^ Sender, EventArgs^ E)
{
	Debug::Assert(BoundDocument != nullptr);

	auto StatusBarLabelLine = View->GetComponentByRole(view::eViewRole::StatusBar_LineNumber)->AsLabel();
	StatusBarLabelLine->Text = BoundDocument->TextEditor->CurrentLine.ToString();
}

void ScriptEditorController::ModelEventHandler_ColumnChanged(Object^ Sender, EventArgs^ E)
{
	Debug::Assert(BoundDocument != nullptr);

	auto StatusBarLabelCol = View->GetComponentByRole(view::eViewRole::StatusBar_ColumnNumber)->AsLabel();
	StatusBarLabelCol->Text = BoundDocument->TextEditor->CurrentColumn.ToString();
}

ScriptEditorController::ScriptEditorController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory)
{
	this->ViewFactory = ViewFactory;
	this->ModelFactory = ModelFactory;
	this->ControllerFactory = ScriptEditorControllerFactory::NewFactory();

	BoundDocument = nullptr;
	ChildView = ViewFactory->NewView();
	ChildModel = ModelFactory->NewModel();

	DocumentNavigationHelper = gcnew components::DocumentNavigationHelper(ChildView);
	FindReplaceHelper = gcnew components::FindReplaceHelper(ChildView);;
	InputManager = gcnew components::InputManager;

	ActiveDocumentActions = gcnew ActiveDocumentActionCollection;
	ActiveDocumentActions->Copy->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Copy);
	ActiveDocumentActions->Paste->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Paste);
	ActiveDocumentActions->Comment->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Comment);
	ActiveDocumentActions->Uncomment->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Uncomment);
	ActiveDocumentActions->AddBookmark->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddBookmark);
	ActiveDocumentActions->GoToLine->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_GoToLine);
	ActiveDocumentActions->AddVarInteger->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarInteger);
	ActiveDocumentActions->AddVarFloat->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarFloat);
	ActiveDocumentActions->AddVarReference->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarReference);
	ActiveDocumentActions->AddVarString->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarString);
	ActiveDocumentActions->AddVarArray->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarArray);
	ActiveDocumentActions->CreateDefaultKeyBindings(InputManager);

	ViewActions = gcnew ViewActionCollection;
	ViewActions->CurrentTabNewScript->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabNewScript);
	ViewActions->CurrentTabOpenScript->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabOpenScript);
	ViewActions->CurrentTabSaveScript->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabSaveScript);
	ViewActions->CurrentTabPreviousScript->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabPreviousScript);
	ViewActions->CurrentTabNextScript->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabNextScript);
	ViewActions->SaveAllTabs->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_SaveAllTabs);
	ViewActions->NewTabWithNewScript->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_NewTabWithNewScript);
	ViewActions->NewTabWithExistingScript->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_NewTabWithExistingScript);
	ViewActions->PreviousTab->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_PreviousTab);
	ViewActions->NextTab->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_NextTab);
	ViewActions->CloseCurrentTab->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CloseCurrentTab);
	ViewActions->ShowFindReplacePane->InvokeDelegate = gcnew BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_ShowFindReplacePane);
	ViewActions->CreateDefaultKeyBindings(InputManager);

	DelegateViewComponentEvent = gcnew view::IScriptEditorView::EventHandler(this, &ScriptEditorController::ViewEventHandler_ComponentEvent);
	DelegateModelDocumentStateChanged = gcnew model::IScriptDocument::StateChangeEventHandler(this, &ScriptEditorController::ModelEventHandler_DocumentStateChanged);
	DelegateModelKeyDown = gcnew KeyEventHandler(this, &ScriptEditorController::ModelEventHandler_KeyDown);
	DelegateModelMouseClick = gcnew textEditor::TextEditorMouseClickEventHandler(this, &ScriptEditorController::ModelEventHandler_MouseClick);
	DelegateModelNavigationChanged = gcnew model::components::INavigationHelper::NavigationChangedEventHandler(this, &ScriptEditorController::ModelEventHandler_NavigationChanged);
	DelegateModelLineChanged = gcnew EventHandler(this, &ScriptEditorController::ModelEventHandler_LineChanged);
	DelegateModelColumnChanged = gcnew EventHandler(this, &ScriptEditorController::ModelEventHandler_ColumnChanged);

	DisableDocumentActivationOnTabSwitch = false;

	ChildView->ComponentEvent += DelegateViewComponentEvent;

	ScriptEditorInstanceManager::Get()->RegisterController(this);
}

ScriptEditorController::~ScriptEditorController()
{
	if (BoundDocument)
	{
		Debug::Assert(View != nullptr);
		UnbindBoundDocument();
	}

	SAFEDELETE_CLR(DocumentNavigationHelper);
	SAFEDELETE_CLR(FindReplaceHelper);
	SAFEDELETE_CLR(InputManager);
	SAFEDELETE_CLR(ActiveDocumentActions);
	SAFEDELETE_CLR(ViewActions);

	SAFEDELETE_CLR(ChildModel);
	if (ChildView)
	{
		ChildView->ComponentEvent -= DelegateViewComponentEvent;
		SAFEDELETE_CLR(ChildView);
	}

	SAFEDELETE_CLR(ChildView);
	SAFEDELETE_CLR(ChildModel);

	SAFEDELETE_CLR(DelegateViewComponentEvent);
	SAFEDELETE_CLR(DelegateModelDocumentStateChanged);
	SAFEDELETE_CLR(DelegateModelKeyDown);
	SAFEDELETE_CLR(DelegateModelMouseClick);
	SAFEDELETE_CLR(DelegateModelNavigationChanged);
	SAFEDELETE_CLR(DelegateModelLineChanged);
	SAFEDELETE_CLR(DelegateModelColumnChanged);

	ScriptEditorInstanceManager::Get()->DeregisterController(this);
}

IScriptEditorController^ ScriptEditorController::New()
{
	return ControllerFactory->NewController(ModelFactory, ViewFactory);
}

void ScriptEditorController::RelocateDocument(model::IScriptDocument^ Document, IScriptEditorController^ Source)
{
	auto OtherController = safe_cast<ScriptEditorController^>(Source);

	if (OtherController->ActiveDocument == Document)
		OtherController->UnbindBoundDocument();

	OtherController->DetachDocumentFromView(Document);
	OtherController->Model->RemoveDocument(Document);

	this->Model->AddDocument(Document);
	this->AttachDocumentToView(Document);
	this->ActivateDocumentInView(Document);
}

void ScriptEditorController::ActivateOrCreateNewDocument(String^ ScriptEditorId)
{
	auto ExistingDocument = Model->LookupDocument(ScriptEditorId);
	if (ExistingDocument)
	{
		ActivateDocumentInView(ExistingDocument);
		return;
	}

	auto NewTabParams = gcnew NewTabCreationParams;
	NewTabParams->InitOperation = NewTabCreationParams::eInitOperation::LoadExistingScript;
	NewTabParams->ExistingScriptEditorId = ScriptEditorId;
	NewTabParams->BindAfterInit = true;

	CreateNewTab(NewTabParams);
}

void ScriptEditorController::InstantiateEditor(IScriptEditorController::InstantiationParams^ Params)
{
	Debug::Assert(Params->Operations != IScriptEditorController::InstantiationParams::eInitOperation::None);

	bool IsSingleDocument = Params->ExistingScriptEditorIds->Count == 1 || Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::CreateNewScript);

	auto NewController = safe_cast<ScriptEditorController^>(New());
	auto NewTabParams = gcnew NewTabCreationParams;

	if (Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::CreateNewScript))
	{
		NewTabParams->InitOperation = NewTabCreationParams::eInitOperation::NewScript;
		NewTabParams->BindAfterInit = true;
		NewController->CreateNewTab(NewTabParams);
	}
	else if (Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::LoadExistingScript))
	{
		NewTabParams->InitOperation = NewTabCreationParams::eInitOperation::LoadExistingScript;

		for (int i = 0; i < Params->ExistingScriptEditorIds->Count; ++i)
		{
			NewTabParams->ExistingScriptEditorId = Params->ExistingScriptEditorIds[i];
			Debug::Assert(NewTabParams->ExistingScriptEditorId->Length > 0);
			NewTabParams->BindAfterInit = i == 0;
			NewController->CreateNewTab(NewTabParams);
		}
	}
	else
		throw gcnew ArgumentException("Unknown controller instantiation operation!");


	if (Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::PerformFind))
	{
		bool DoGlobalFind = !IsSingleDocument;

		NewController->FindReplaceHelper->DoOperation(textEditor::eFindReplaceOperation::Find, Params->FindQuery, "",
													  textEditor::eFindReplaceOptions::CaseInsensitive, DoGlobalFind, NewController);
	}

	NewController->View->Reveal(Params->InitialBounds);
}

IScriptEditorController^ ScriptEditorControllerFactory::NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory)
{
	return gcnew controllerImpl::ScriptEditorController(ModelFactory, ViewFactory);
}

ScriptEditorControllerFactory^ ScriptEditorControllerFactory::NewFactory()
{
	return gcnew ScriptEditorControllerFactory;
}


} // namespace controllerImpl


} // namespace scriptEditor


} // namespace cse