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
	Copy = gcnew utilities::BasicAction("Copy", "Copies the current selection or line");
	Paste = gcnew utilities::BasicAction("Paste", "Pastes the contents of the clipboard at the caret location");
	Comment = gcnew utilities::BasicAction("Comment", "Comments the current selection or line");
	Uncomment = gcnew utilities::BasicAction("Uncomment", "Uncomments the current selection or line");
	AddBookmark = gcnew utilities::BasicAction("Add Bookmark", "Adds a new bookmark at the current line");
	GoToLine = gcnew utilities::BasicAction("Go to Line", "Jumps to a given line in the document");
	JumpToScriptAtCaret = gcnew utilities::BasicAction("Jump to Attached Script", "Opens the script (attached to the form) at the caret location");

	AddVarInteger = gcnew utilities::BasicAction("Add New Variable (Integer)", "Adds a new integer variable to the script's variable declaration");
	AddVarFloat = gcnew utilities::BasicAction("Add New Variable (Float)", "Adds a new float variable to the script's variable declaration");
	AddVarReference = gcnew utilities::BasicAction("Add New Variable (Reference)", "Adds a new reference variable to the script's variable declaration");
	AddVarString = gcnew utilities::BasicAction("Add New Variable (String)", "Adds a new string variable to the script's variable declaration");
	AddVarArray = gcnew utilities::BasicAction("Add New Variable (Array)", "Adds a new array variable to the script's variable declaration");
}

ActiveDocumentActionCollection::~ActiveDocumentActionCollection()
{
	SAFEDELETE_CLR(Copy);
	SAFEDELETE_CLR(Paste);
	SAFEDELETE_CLR(Comment);
	SAFEDELETE_CLR(Uncomment);
	SAFEDELETE_CLR(AddBookmark);
	SAFEDELETE_CLR(GoToLine);
	SAFEDELETE_CLR(JumpToScriptAtCaret);

	SAFEDELETE_CLR(AddVarInteger);
	SAFEDELETE_CLR(AddVarFloat);
	SAFEDELETE_CLR(AddVarReference);
	SAFEDELETE_CLR(AddVarString);
	SAFEDELETE_CLR(AddVarArray);
}

void ActiveDocumentActionCollection::CreateDefaultKeyBindings(components::InputManager^ InputManager)
{
	InputManager->AddKeyChordCommand(Copy, utilities::KeyCombo::New(Keys::Control, Keys::C), false, view::eViewRole::TextEditor_ContextMenu_Copy);
	InputManager->AddKeyChordCommand(Paste, utilities::KeyCombo::New(Keys::Control, Keys::V), false, view::eViewRole::TextEditor_ContextMenu_Paste);
	InputManager->AddKeyChordCommand(Comment, utilities::KeyCombo::New(Keys::Control, Keys::K), utilities::KeyCombo::New(Keys::Control, Keys::C), false, view::eViewRole::MainToolbar_Edit_Comment);
	InputManager->AddKeyChordCommand(Uncomment, utilities::KeyCombo::New(Keys::Control, Keys::K), utilities::KeyCombo::New(Keys::Control, Keys::U), false, view::eViewRole::MainToolbar_Edit_Uncomment);
	InputManager->AddKeyChordCommand(AddBookmark, utilities::KeyCombo::New(Keys::Control, Keys::B), false, view::eViewRole::MainToolbar_Edit_AddBookmark, view::eViewRole::MainToolbar_Edit_AddBookmark);
	InputManager->AddKeyChordCommand(GoToLine, utilities::KeyCombo::New(Keys::Control, Keys::G), false, view::eViewRole::MainToolbar_Edit_GoToLine);
	InputManager->AddKeyChordCommand(JumpToScriptAtCaret, utilities::KeyCombo::New(Keys::Control, Keys::OemPipe), false, view::eViewRole::TextEditor_ContextMenu_JumpToAttachedScript);

	InputManager->AddKeyChordCommand(AddVarInteger, utilities::KeyCombo::New(Keys::Control, Keys::L), utilities::KeyCombo::New(Keys::Control, Keys::I), false, view::eViewRole::TextEditor_ContextMenu_AddVar_Integer);
	InputManager->AddKeyChordCommand(AddVarFloat, utilities::KeyCombo::New(Keys::Control, Keys::L), utilities::KeyCombo::New(Keys::Control, Keys::F), false, view::eViewRole::TextEditor_ContextMenu_AddVar_Float);
	InputManager->AddKeyChordCommand(AddVarReference, utilities::KeyCombo::New(Keys::Control, Keys::L), utilities::KeyCombo::New(Keys::Control, Keys::R), false, view::eViewRole::TextEditor_ContextMenu_AddVar_Reference);
	InputManager->AddKeyChordCommand(AddVarString, utilities::KeyCombo::New(Keys::Control, Keys::L), utilities::KeyCombo::New(Keys::Control, Keys::S), false, view::eViewRole::TextEditor_ContextMenu_AddVar_String);
	InputManager->AddKeyChordCommand(AddVarArray, utilities::KeyCombo::New(Keys::Control, Keys::L), utilities::KeyCombo::New(Keys::Control, Keys::A), false, view::eViewRole::TextEditor_ContextMenu_AddVar_Array);
}

ViewActionCollection::ViewActionCollection()
{
	CurrentTabNewScript = gcnew utilities::BasicAction("New Script (Current Tab)", "Create a new script in the current tab");
	CurrentTabOpenScript = gcnew utilities::BasicAction("Open Script (Current Tab)", "Open an existing script in the current tab");
	CurrentTabSaveScript = gcnew utilities::BasicAction("Save Script (Current Tab)", "Compile and save the script open in the current tab");
	CurrentTabPreviousScript = gcnew utilities::BasicAction("Previous Script (Current Tab)", "Open the previous script relative to the script in the current tab");
	CurrentTabNextScript = gcnew utilities::BasicAction("Next Script (Current Tab)", "Open the next script relative to the script in the current tab");
	SaveAllTabs = gcnew utilities::BasicAction("Save All Open Scripts", "Compile and save all scripts in all open tabs");
	NewTabWithNewScript = gcnew utilities::BasicAction("Open New Tab with New Script", "Open a new tab and initialize it with a new script");
	NewTabWithExistingScript = gcnew utilities::BasicAction("Open New Tab with Existing Script", "Open a new tab and load an existing script into it");
	PreviousTab = gcnew utilities::BasicAction("Previous Tab", "Switch to the previous tab");
	NextTab = gcnew utilities::BasicAction("Next Tab", "Switch to the next tab");
	CloseCurrentTab = gcnew utilities::BasicAction("Close Current Tab", "Attempt to close the current tab (unsaved changes can be saved)");
	ShowFindReplacePane = gcnew utilities::BasicAction("Show Find/Replace Window", "Display the find/replace window");
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
	InputManager->AddKeyChordCommand(CurrentTabNewScript, utilities::KeyCombo::New(Keys::Control, Keys::N), false, view::eViewRole::MainToolbar_NewScript);
	InputManager->AddKeyChordCommand(CurrentTabOpenScript, utilities::KeyCombo::New(Keys::Control, Keys::O), false, view::eViewRole::MainToolbar_OpenScript);
	InputManager->AddKeyChordCommand(CurrentTabSaveScript, utilities::KeyCombo::New(Keys::Control, Keys::S), false, view::eViewRole::MainToolbar_SaveScript);
	InputManager->AddKeyChordCommand(CurrentTabPreviousScript, utilities::KeyCombo::New(Keys::Control | Keys::Alt, Keys::Left), false, view::eViewRole::MainToolbar_PreviousScript);
	InputManager->AddKeyChordCommand(CurrentTabNextScript, utilities::KeyCombo::New(Keys::Control | Keys::Alt, Keys::Right), false, view::eViewRole::MainToolbar_NextScript);
	InputManager->AddKeyChordCommand(SaveAllTabs, utilities::KeyCombo::New(Keys::Control | Keys::Shift, Keys::S), false, view::eViewRole::MainToolbar_SaveAllScripts);
	InputManager->AddKeyChordCommand(NewTabWithNewScript, utilities::KeyCombo::New(Keys::Control, Keys::T), utilities::KeyCombo::New(Keys::Control, Keys::N), false, view::eViewRole::MainTabStrip_NewTab_NewScript, view::eViewRole::EmptyWorkspacePanel_NewScript);
	InputManager->AddKeyChordCommand(NewTabWithExistingScript, utilities::KeyCombo::New(Keys::Control, Keys::T), utilities::KeyCombo::New(Keys::Control, Keys::O), false, view::eViewRole::MainTabStrip_NewTab_ExistingScript, view::eViewRole::EmptyWorkspacePanel_OpenScript);
	InputManager->AddKeyChordCommand(PreviousTab, utilities::KeyCombo::New(Keys::Control | Keys::Shift, Keys::Tab), false);
	InputManager->AddKeyChordCommand(NextTab, utilities::KeyCombo::New(Keys::Control, Keys::Tab), false);
	InputManager->AddKeyChordCommand(CloseCurrentTab, utilities::KeyCombo::New(Keys::Control, Keys::F4), false);
	InputManager->AddKeyChordCommand(ShowFindReplacePane, utilities::KeyCombo::New(Keys::Control | Keys::Shift, Keys::F), false, view::eViewRole::MainToolbar_Edit_FindReplace);
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

	if (!Document->TextEditor->SelectionEmpty)
		Document->TextEditor->CommentSelection();
	else
		Document->TextEditor->CommentLine(Document->TextEditor->CurrentLine);
}

void ScriptEditorController::ActiveDocumentAction_Uncomment()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	if (!Document->TextEditor->SelectionEmpty)
		Document->TextEditor->UncommentSelection();
	else
		Document->TextEditor->UncommentLine(Document->TextEditor->CurrentLine);
}

void ScriptEditorController::ActiveDocumentAction_AddBookmark()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;
	else if (Document->IsPreprocessorOutputEnabled())
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
	int::TryParse(LineNumberStr, LineNumber);

	if (LineNumber >= 1 && LineNumber <= LineCount)
		Document->TextEditor->ScrollToLine(LineNumber);
	else
	{
		View->ShowNotification("Line number must be between 1 and " + LineCount + ".",
							   view::components::CommonIcons::Get()->ErrorLarge, 3500);
	}
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
		View->ShowNotification("Could not insert new variable '" + VariableName + "'!\nEnsure that there are no other variables with the same name.",
							   view::components::CommonIcons::Get()->BlockedLarge, 3500);
	}
}

void ScriptEditorController::ActiveDocumentAction_JumpToScriptAtCaret()
{
	auto Document = BoundDocument;
	if (Document == nullptr)
		return;

	auto TokenAtCaret = Document->TextEditor->GetTokenAtCaretPos();
	auto AttachedScript = intellisense::IntelliSenseBackend::Get()->GetAttachedScript(TokenAtCaret);

	if (AttachedScript)
		ActivateOrCreateNewDocument(AttachedScript->GetIdentifier());
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
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::Reference, BoundDocument, View);
}

void ScriptEditorController::ActiveDocumentAction_AddVarString()
{
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::String, BoundDocument, View);
}

void ScriptEditorController::ActiveDocumentAction_AddVarArray()
{
	AddNewVariableToDocument(obScriptParsing::Variable::eDataType::Array, BoundDocument, View);
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
	BoundDocument->TextEditor->Container->Visible = true;
	BoundDocument->TextEditor->Container->BringToFront();
	BoundDocument->TextEditor->Container->Focus();

	BoundDocument->TextEditor->KeyDown += DelegateModelKeyDown;
	BoundDocument->TextEditor->MouseClick += DelegateModelMouseClick;
	BoundDocument->NavigationHelper->NavigationChanged += DelegateModelNavigationChanged;

	BoundDocument->BackgroundAnalyzer->Resume();
	BoundDocument->IntelliSenseModel->Bind(View->IntelliSenseView);
	BoundDocument->TextEditor->Bind();
;
	SetDocumentDependentViewComponentsEnabled(true);
	if (BoundDocument->IsPreprocessorOutputEnabled())
		SetDocumentPreprocessorOutputDisplayDependentViewComponentsEnabled(false);

	BoundDocument->PushStateToSubscribers();
	ValidateDocumentSyncingStatus(BoundDocument);
	DocumentNavigationHelper->SyncWithDocument(BoundDocument);
}

void ScriptEditorController::UnbindBoundDocument()
{
	Debug::Assert(BoundDocument != nullptr);

	BoundDocument->BackgroundAnalyzer->Pause();
	BoundDocument->TextEditor->Unbind();
	BoundDocument->IntelliSenseModel->Unbind();

	BoundDocument->TextEditor->KeyDown -= DelegateModelKeyDown;
	BoundDocument->TextEditor->MouseClick -= DelegateModelMouseClick;
	BoundDocument->NavigationHelper->NavigationChanged -= DelegateModelNavigationChanged;

	BoundDocument->TextEditor->Container->SendToBack();
	BoundDocument->TextEditor->Container->Visible = false;
	View->GetComponentByRole(view::eViewRole::TextEditor_ViewPortContainer)->AsContainer()->RemoveControl(BoundDocument->TextEditor->Container);

	ResetViewComponentsToUnboundState();

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
		view::eViewRole::MainToolbar_Edit_FindReplace,
		view::eViewRole::MainToolbar_Edit_GoToLine,
		view::eViewRole::MainToolbar_Edit_AddBookmark,
		view::eViewRole::MainToolbar_Edit_Comment,
		view::eViewRole::MainToolbar_Edit_Uncomment,
		view::eViewRole::MainToolbar_View_PreprocessorOutputAndBytecodeOffsets,
		view::eViewRole::MainToolbar_View_IconMargin,
		view::eViewRole::MainToolbar_Tools_SanitiseScript,
		view::eViewRole::MainToolbar_Tools_AttachScript,
		view::eViewRole::MainToolbar_Tools_RecompileScriptDependencies,
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

	// ### TODO This causes a huge spike in CPU activity; investigate!
	//View->GetComponentByRole(view::eViewRole::NavigationBar)->AsCrumbBar()->Visible = Enabled ? preferences::SettingsHolder::Get()->Appearance->ShowScopeBar : false;
	View->GetComponentByRole(view::eViewRole::StatusBar_DocumentDescription)->AsLabel()->Visible = Enabled;
	View->GetComponentByRole(view::eViewRole::StatusBar_LineNumber)->AsLabel()->Visible = Enabled;
	View->GetComponentByRole(view::eViewRole::StatusBar_ColumnNumber)->AsLabel()->Visible = Enabled;
	View->GetComponentByRole(view::eViewRole::StatusBar_ErrorCount)->AsButton()->Visible = Enabled;
	View->GetComponentByRole(view::eViewRole::StatusBar_WarningCount)->AsButton()->Visible = Enabled;
	View->GetComponentByRole(view::eViewRole::StatusBar_NoIssuesIndicator)->AsButton()->Visible = Enabled;
	View->GetComponentByRole(view::eViewRole::StatusBar_CompiledScriptSize)->AsProgressBar()->Visible = Enabled;
	View->GetComponentByRole(view::eViewRole::StatusBar)->AsContainer()->Invalidate();
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

	View->GetComponentByRole(view::eViewRole::StatusBar)->AsContainer()->Invalidate();
}

void ScriptEditorController::ResetViewComponentsToUnboundState()
{
	SetDocumentDependentViewComponentsEnabled(false);
	DocumentNavigationHelper->SyncWithDocument(nullptr);

	View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm()->Text = view::IScriptEditorView::MainWindowDefaultTitle;
	View->GetComponentByRole(view::eViewRole::Messages_ListView)->AsObjectListView()->ClearObjects();
	View->GetComponentByRole(view::eViewRole::Bookmarks_ListView)->AsObjectListView()->ClearObjects();
	View->GetComponentByRole(view::eViewRole::FindReplaceResults_ListView)->AsObjectListView()->ClearObjects();
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

	auto PromptScriptName = Document->ScriptEditorID != "" ? "'" + Document->ScriptEditorID + "' " : "";
	auto PromptResult = View->ShowMessageBox("The current script " + PromptScriptName + "has unsaved changes.\n\nDo you wish to save them?",
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
	NewTabItem->Tooltip = NewTabItem->Text + " (" + Document->ScriptFormID.ToString("X8") + ")";
	NewTabItem->Image = Document->Dirty ? view::components::CommonIcons::Get()->UnsavedChanges : nullptr;

	TabStrip->AddTab(NewTabItem);

	// we need to start subscribe to certain changes in state even when the document isn't active
	Document->StateChanged += DelegateModelDocumentStateChanged;
	View->GetComponentByRole(view::eViewRole::EmptyWorkspacePanel)->AsContainer()->Visible = false;
}

void ScriptEditorController::DetachDocumentFromView(model::IScriptDocument^ Document)
{
	Debug::Assert(BoundDocument != Document);

	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto TabItem = TabStrip->LookupTabByTag(Document);

	Debug::Assert(TabItem != nullptr);

	TabStrip->RemoveTab(TabItem);
	delete TabItem;

	FindReplaceHelper->InvalidateScriptDocumentInGlobalResultsCache(Document);

	Document->StateChanged -= DelegateModelDocumentStateChanged;
	View->GetComponentByRole(view::eViewRole::EmptyWorkspacePanel)->AsContainer()->Visible = TabStrip->TabCount == 0;
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
	Debug::Assert(ChildView != nullptr);
	Debug::Assert(BoundDocument == nullptr);

	ChildView->ComponentEvent -= DelegateViewComponentEvent;
	components::ViewTabTearingHelper::Get()->DeregisterTabStrip(ChildView->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip());
	ChildView = nullptr;

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

void ScriptEditorController::BatchCreateNewTabs(NewTabCreationParams::eInitOperation Operation, ICollection<String^>^ EditorIdsOrPaths, bool ActivateFirstNewTab)
{
	Debug::Assert(Operation != NewTabCreationParams::eInitOperation::NewScript);
	if (EditorIdsOrPaths->Count == 0)
		return;

	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto Form = View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm();

	//View->ShowNotification("Please Wait...", view::CommonIcons::Get()->InfoLarge, 3000);
	//System::Threading::Thread::CurrentThread->Sleep(32);

	Form->BeginUpdate();
	TabStrip->BeginUpdate();
	{
		auto Params = gcnew NewTabCreationParams;
		Params->InitOperation = Operation;
		bool FirstTabCreated = false;
		for each (auto EditorIdOrPath in EditorIdsOrPaths)
		{
			Params->BindAfterInit = !FirstTabCreated && ActivateFirstNewTab;
			if (!FirstTabCreated)
				FirstTabCreated = true;

			if (Operation == NewTabCreationParams::eInitOperation::LoadExistingScript)
				Params->ExistingScriptEditorId = EditorIdOrPath;
			else
				Params->DiskFilePath = EditorIdOrPath;

			CreateNewTab(Params);
		}
	}
	TabStrip->EndUpdate();
	Form->EndUpdate();
}

model::IScriptDocument^ ScriptEditorController::ImportDocumentFromDisk(String^ DiskFilePath, bool ImportAsExistingScript)
{
	Debug::Assert(System::IO::File::Exists(DiskFilePath));

	model::IScriptDocument^ NewOrExisting = nullptr;
	String^ FileContents = "";
	try
	{
		auto FileParser = gcnew StreamReader(DiskFilePath);
		FileContents = FileParser->ReadToEnd()->Replace("\r\n", "\n");
		FileParser->Close();

		auto AnalysisParams = gcnew obScriptParsing::AnalysisData::Params;
		AnalysisParams->ScriptText = FileContents;

		auto AnalysisData = gcnew obScriptParsing::AnalysisData(AnalysisParams);
		if (AnalysisData->Name != "")
		{
			nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ExistingScriptData(
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
		NewOrExisting->TextEditor->SetText(FileContents, true);
	}

	NewOrExisting->Dirty = true;

	return NewOrExisting;
}

void ScriptEditorController::HandleViewClosureRequest(view::components::IForm^ Form, view::components::IForm::ClosingEventArgs^ E)
{
	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto OpenTabs = TabStrip->TabCount;
	Debug::Assert(OpenTabs == Model->Documents->Count);

	if (OpenTabs == 0)
	{
		DisposeSelfOnViewClosure();
		return;
	}

	if (OpenTabs > 1)
	{
		auto DialogResult = View->ShowMessageBox("Are you sure you want to close all open scripts?",
												 MessageBoxButtons::YesNo, MessageBoxIcon::Question);
		if (DialogResult == DialogResult::No)
		{
			E->Cancel = true;
			return;
		}
	}

	auto ModelDocumentsCopy = gcnew List<model::IScriptDocument^>(Model->Documents);
	bool CloseOperationCancelled = false;
	bool StopClosure = false;
	Form->BeginUpdate();
	TabStrip->BeginUpdate();
	{
		for each (auto Doc in ModelDocumentsCopy)
		{
			// switch to dirty models to enable user interaction
			if (Doc->Dirty)
				ActivateDocumentInView(Doc);

			// we need to disable the default tab switching behaviour since removing
			// a tab will cause the next tab in the tabstrip to automatically get activated
			DisableDocumentActivationOnTabSwitch = true;
			bool CloseSuccess = false;
			{
				CloseSuccess = CloseAndRemoveDocument(Doc, CloseOperationCancelled);
			}
			DisableDocumentActivationOnTabSwitch = false;

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
	TabStrip->EndUpdate();
	Form->EndUpdate();

	if (StopClosure)
	{
		E->Cancel = true;
		return;
	}

	Debug::Assert(Model->Documents->Count == 0);
	Debug::Assert(Model->Documents->Count == TabStrip->TabCount);
	DisposeSelfOnViewClosure();
}

void ScriptEditorController::AllowDocumentBindingAfterTabMove()
{
	Debug::Assert(BoundDocument != nullptr);
	Debug::Assert(DisableDocumentActivationOnTabSwitch);

	DisableDocumentActivationOnTabSwitch = false;
}

void ScriptEditorController::LoadNewUnsavedScriptIntoDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NewScriptInstance(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CreateNewScript());
	Document->Initialize(NewScriptInstance.get(), false);
}

void ScriptEditorController::LoadExistingScriptIntoDocument(model::IScriptDocument^ Document, String^ ExistingScriptEditorId)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ExistingScriptData(
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CString(ExistingScriptEditorId).c_str())
	);
	Debug::Assert(ExistingScriptData);

	Document->Initialize(ExistingScriptData.get(), ShouldUseAutoRecoveryFile(ExistingScriptEditorId));
}

void ScriptEditorController::LoadExistingScriptIntoDocument(model::IScriptDocument^ Document, nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> ExistingScriptData)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	Document->Initialize(ExistingScriptData.get(), ShouldUseAutoRecoveryFile(gcnew String(ExistingScriptData->EditorID)));
	ValidateDocumentSyncingStatus(Document);
}

bool ScriptEditorController::SaveDocument(model::IScriptDocument^ Document, model::IScriptDocument::eSaveOperation SaveOperation)
{
	Debug::Assert(Document != nullptr);

	if (!Document->Dirty)
		return true;
	else if (Document->UnsavedNewScript && SaveOperation == model::IScriptDocument::eSaveOperation::DontCompile)
	{
		View->ShowNotification("This operation can only be performed on scripts that have been compiled at least once.",
							   view::components::CommonIcons::Get()->BlockedLarge,
							   3000);
		return false;
	}

	bool SaveResult = Document->Save(SaveOperation);
	if (!SaveResult && BoundDocument == Document)
	{
		View->GetComponentByRole(view::eViewRole::Messages_DockPanel)->AsDockablePane()->Focus();
		View->ShowNotification("Script compilation failed.\nCheck the message log for more information.",
							   view::components::CommonIcons::Get()->ErrorLarge,
							   3000);
	}

	ValidateDocumentSyncingStatus(Document);
	return SaveResult;
}

void ScriptEditorController::LoadNextScriptIntoDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetNextScriptInList(Document->ScriptNativeObject));
	if (Data)
	{
		Document->Initialize(Data.get(), ShouldUseAutoRecoveryFile(gcnew String(Data->EditorID)));
		ValidateDocumentSyncingStatus(Document);
	}
}

void ScriptEditorController::LoadPreviousScriptIntoDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!HandleVolatileDocumentStateBeforeDestructiveOperation(Document))
		return;

	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreviousScriptInList(Document->ScriptNativeObject));
	if (Data)
	{
		Document->Initialize(Data.get(), ShouldUseAutoRecoveryFile(gcnew String(Data->EditorID)));
		ValidateDocumentSyncingStatus(Document);
	}
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

	View->ShowNotification("The current script is actively being synced from/to disk.\nModifying it inside the script editor will result in inconsistent and unexpected behaviour.",
						   view::components::CommonIcons::Get()->WarningLarge,
						   5000);
}

bool ScriptEditorController::ShouldUseAutoRecoveryFile(String^ ScriptEditorId)
{
	if (!preferences::SettingsHolder::Get()->Backup->UseAutoRecovery)
		return false;
	else if (BoundDocument && !String::Equals(BoundDocument->ScriptEditorID, ScriptEditorId, StringComparison::CurrentCultureIgnoreCase))
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

bool ScriptEditorController::ShouldFilterScriptDiagnosticMessage(Object^ Message)
{
	auto DiagnosticMessage = safe_cast<model::components::ScriptDiagnosticMessage^>(Message);

	switch (DiagnosticMessage->Type)
	{
	case model::components::ScriptDiagnosticMessage::eMessageType::Error:
		return View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleErrors)->AsButton()->Checked;
	case model::components::ScriptDiagnosticMessage::eMessageType::Warning:
		return View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleWarnings)->AsButton()->Checked;
	case model::components::ScriptDiagnosticMessage::eMessageType::Info:
		return View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleInfos)->AsButton()->Checked;
	default:
		throw gcnew NotImplementedException;
	}
}

void ScriptEditorController::DeferredUiActionTimer_Tick(Object^ Sender, EventArgs^ E)
{
	Debug::Assert(DelegateDeferredUiAction != nullptr);

	DeferredUiActionTimer->Enabled = false;
	DelegateDeferredUiAction();

	DelegateDeferredUiAction = nullptr;
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
	case view::eViewRole::MainToolbar:
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
	case view::eViewRole::MainToolbar_View_PreprocessorOutputAndBytecodeOffsets:
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
	case view::eViewRole::Messages_Toolbar_ToggleErrors:
	case view::eViewRole::Messages_Toolbar_ToggleWarnings:
	case view::eViewRole::Messages_Toolbar_ToggleInfos:
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
	case view::eViewRole::MainTabStrip_ContextMenu:
	case view::eViewRole::MainTabStrip_ContextMenu_Close:
	case view::eViewRole::MainTabStrip_ContextMenu_CloseOthers:
	case view::eViewRole::MainTabStrip_ContextMenu_CloseSaved:
	case view::eViewRole::MainTabStrip_ContextMenu_CloseAll:
	case view::eViewRole::MainTabStrip_ContextMenu_PopOut:
		ViewEventHandler_TabStripContextMenu(E);
		break;
	case view::eViewRole::EmptyWorkspacePanel:
	case view::eViewRole::EmptyWorkspacePanel_NewScript:
	case view::eViewRole::EmptyWorkspacePanel_OpenScript:
		ViewEventHandler_EmptyWorkspacePanel(E);
		break;
	case view::eViewRole::StatusBar:
	case view::eViewRole::StatusBar_ErrorCount:
	case view::eViewRole::StatusBar_WarningCount:
	case view::eViewRole::StatusBar_NoIssuesIndicator:
		ViewEventHandler_StatusBar(E);
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
	case view::components::IForm::eEvent::FocusEnter:
	case view::components::IForm::eEvent::FocusLeave:
		InputManager->HandleInputFocusChange(E, this);
		break;
	case view::components::IForm::eEvent::Closing:
		HandleViewClosureRequest(Form, safe_cast<view::components::IForm::ClosingEventArgs^>(E->EventArgs));
		break;
	}
}

void ScriptEditorController::ViewEventHandler_MainTabStrip(view::ViewComponentEvent^ E)
{
	if (E->Component->Type == view::eComponentType::Button)
	{
		auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
		if (EventType == view::components::IButton::eEvent::PopupOpening)
			return;

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
			auto ExistingScriptEditorIds = View->SelectExistingScripts(BoundDocument ? BoundDocument->ScriptEditorID : "");
			BatchCreateNewTabs(NewTabCreationParams::eInitOperation::LoadExistingScript, ExistingScriptEditorIds, true);

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
	case view::components::ITabStrip::eEvent::TabMoved:
	{
		// on a successful tab moved operation, the tab collection on the tabstrip is reordered by removing
		// and inserting the existing tabs in the new order, leading to each of them being activated in turn
		// this causes each of their attached parent to get bound/unbound, resulting in a drop in UI responsiveness

		// we have to, therefore, resort to this hack - temporarily disable document binding after the completion of
		// the tab moved event
		auto Args = safe_cast<view::components::ITabStrip::TabMovedEventArgs^>(E->EventArgs);
		Debug::Assert(Args->Tab != nullptr);

		auto MovedDocument = safe_cast<model::IScriptDocument^>(Args->Tab->Tag);
		Debug::Assert(BoundDocument == MovedDocument);

		auto DeferredUiAction = gcnew DeferredUiActionDelegate(this, &ScriptEditorController::AllowDocumentBindingAfterTabMove);
		DisableDocumentActivationOnTabSwitch = true;
		QueueDeferredUiAction(DeferredUiAction, 64);

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

		auto OldDocument = Args->OldValue ? safe_cast<model::IScriptDocument^>(Args->OldValue->Tag) : nullptr;
		auto NewDocument = safe_cast<model::IScriptDocument^>(Args->NewValue->Tag);

		View->BeginUpdate();
		{
			if (OldDocument && OldDocument == BoundDocument)
				UnbindBoundDocument();

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
		LoadNewUnsavedScriptIntoDocument(BoundDocument);
		break;
	case view::eViewRole::MainToolbar_OpenScript:
	{
		auto ExistingScriptEditorIds = View->SelectExistingScripts(BoundDocument->ScriptEditorID);
		if (ExistingScriptEditorIds->Count == 0)
			break;

		View->BeginUpdate();
		{
			// load the first one into the current doc and the rest in new tab
			LoadExistingScriptIntoDocument(BoundDocument, ExistingScriptEditorIds[0]);

			ExistingScriptEditorIds->RemoveAt(0);
			BatchCreateNewTabs(NewTabCreationParams::eInitOperation::LoadExistingScript, ExistingScriptEditorIds, false);
		}
		View->EndUpdate();

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
		FindReplaceHelper->ShowFindReplacePane(this);
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
	case view::eViewRole::MainToolbar_View_PreprocessorOutputAndBytecodeOffsets:
		if (BoundDocument == nullptr)
			break;

		if (OnPopupOpening)
			Button->Checked = BoundDocument->IsPreprocessorOutputEnabled();
		else if (OnClick)
		{
			bool WasTurnedOn = BoundDocument->IsPreprocessorOutputEnabled();
			bool Success = BoundDocument->TogglePreprocessorOutput(!WasTurnedOn);
			bool GotTurnedOn = BoundDocument->IsPreprocessorOutputEnabled();
			SetDocumentPreprocessorOutputDisplayDependentViewComponentsEnabled(!GotTurnedOn);

			if (GotTurnedOn)
			{
				View->ShowNotification("Displaying script preprocessor output.\nCertain actions are disabled in this mode.",
									   view::components::CommonIcons::Get()->InfoLarge, 4000);
			}
			else if (!Success)
				View->ShowNotification("This operation cannot be performed until the unsaved changes are saved.", view::components::CommonIcons::Get()->BlockedLarge, 4000);
		}

		break;
	case view::eViewRole::MainToolbar_View_IconMargin:
		if (OnPopupOpening)
			Button->Checked = preferences::SettingsHolder::Get()->Appearance->ShowIconMargin;
		else if (OnClick)
		{
			preferences::SettingsHolder::Get()->Appearance->ShowIconMargin = preferences::SettingsHolder::Get()->Appearance->ShowIconMargin == false;
			preferences::SettingsHolder::Get()->RaisePreferencesChangedEvent();
		}
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
		if (OnPopupOpening)
			Button->Checked = preferences::SettingsHolder::Get()->Appearance->ShowScopeBar;
		else if (OnClick)
		{
			preferences::SettingsHolder::Get()->Appearance->ShowScopeBar = preferences::SettingsHolder::Get()->Appearance->ShowScopeBar == false;
			preferences::SettingsHolder::Get()->RaisePreferencesChangedEvent();
		}
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
			View->ShowNotification("Operation failed!\nPlease fix all structural errors in the script before trying again.",
								   view::components::CommonIcons::Get()->ErrorLarge, 5000);
		}

		break;
	case view::eViewRole::MainToolbar_Tools_AttachScript:
		if (BoundDocument->UnsavedNewScript)
		{
			View->ShowNotification("This operation cannot be performed until the unsaved changes are saved.", view::components::CommonIcons::Get()->BlockedLarge, 4000);
			break;
		}

		nativeWrapper::g_CSEInterfaceTable->ScriptEditor.BindScript(CString(BoundDocument->ScriptEditorID).c_str(),
																	safe_cast<HWND>(View->WindowHandle));
		break;
	case view::eViewRole::MainToolbar_Tools_RecompileScriptDependencies:
		if (BoundDocument->UnsavedNewScript)
		{
			View->ShowNotification("This operation cannot be performed until the unsaved changes are saved.", view::components::CommonIcons::Get()->BlockedLarge, 4000);
			break;
		}

		nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileDependencies(CString(BoundDocument->ScriptEditorID).c_str());
		View->ShowNotification("Operation complete!\nScript variables used as condition parameters will need to be corrected manually.\nThe results have been logged to the console.",
							   view::components::CommonIcons::Get()->SuccessLarge, 6000);
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
			BatchCreateNewTabs(NewTabCreationParams::eInitOperation::LoadFileFromDisk, gcnew List<String^>(LoadManager->FileNames), true);

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
		View->ShowNotification("All scripts in the active plugin file have been recompiled.\nResults have been logged to the console.",
							   view::components::CommonIcons::Get()->SuccessLarge, 4000);
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
								   view::components::CommonIcons::Get()->SuccessLarge, 4000);
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
	case view::eViewRole::Messages_Toolbar_ToggleErrors:
	case view::eViewRole::Messages_Toolbar_ToggleWarnings:
	case view::eViewRole::Messages_Toolbar_ToggleInfos:
	{
		if (safe_cast<view::components::IButton::eEvent>(E->EventType) == view::components::IButton::eEvent::Click)
		{
			auto Button = E->Component->AsButton();
			Button->Checked = !Button->Checked;

			// refresh the filter
			auto ListView = View->GetComponentByRole(view::eViewRole::Messages_ListView)->AsObjectListView();
			ListView->UseFiltering = false;
			ListView->UseFiltering = true;
		}

		break;
	}
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
			auto CursorPosition = Cursor::Position;
			auto RelativeCoords = BoundDocument->TextEditor->ScreenToClient(CursorPosition);
			auto TextIndex = BoundDocument->TextEditor->GetCharIndexFromPosition(RelativeCoords);
			auto TokenAtPopup = BoundDocument->TextEditor->GetTokenAtCharIndex(TextIndex);
			auto AttachedScript = intellisense::IntelliSenseBackend::Get()->GetAttachedScript(TokenAtPopup);

			if (AttachedScript)
			{
				Button->Text = "Open Attached Script";
				Button->Enabled = true;
				Button->Tag = AttachedScript->GetIdentifier();
			}
			else
				Button->Enabled = false;
		}

		break;
	}
	}
}

void ScriptEditorController::ViewEventHandler_TabStripContextMenu(view::ViewComponentEvent^ E)
{
	auto EventType = safe_cast<view::components::IButton::eEvent>(E->EventType);
	bool OnClick = EventType == view::components::IButton::eEvent::Click;
	auto OnPopup = EventType == view::components::IButton::eEvent::PopupOpening;

	auto Form = View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm();
	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto Button = E->Component->AsButton();

	if (OnPopup)
		Button->Tag = TabStrip->MouseOverTab;

	switch (E->Component->Role)
	{
	case view::eViewRole::MainTabStrip_ContextMenu_Close:
		if (OnPopup)
			Button->Enabled = Button->Tag != nullptr;
		else if (OnClick)
			safe_cast<view::components::ITabStripItem^>(Button->Tag)->Close();

		break;
	case view::eViewRole::MainTabStrip_ContextMenu_CloseOthers:
		if (OnPopup)
			Button->Enabled = Button->Tag != nullptr && TabStrip->TabCount > 1;
		else if (OnClick)
		{
			Form->BeginUpdate();
			TabStrip->BeginUpdate();
			{
				for each (auto Tab in TabStrip->Tabs)
				{
					if (Tab != Button->Tag)
						Tab->Close();
				}
			}
			TabStrip->EndUpdate();
			Form->EndUpdate();
		}

		break;
	case view::eViewRole::MainTabStrip_ContextMenu_CloseSaved:
		if (OnPopup)
		{
			bool DocWithNoModifications = false;
			for each (auto Doc in Model->Documents)
			{
				if (!Doc->Dirty)
				{
					DocWithNoModifications = true;
					break;
				}
			}

			Button->Enabled = DocWithNoModifications;
		}
		else if (OnClick)
		{
			Form->BeginUpdate();
			TabStrip->BeginUpdate();
			{
				for each (auto Tab in TabStrip->Tabs)
				{
					auto Doc = safe_cast<model::IScriptDocument^>(Tab->Tag);
					if (!Doc->Dirty)
						Tab->Close();
				}
			}
			TabStrip->EndUpdate();
			Form->EndUpdate();
		}

		break;
	case view::eViewRole::MainTabStrip_ContextMenu_CloseAll:
		if (OnPopup)
			Button->Enabled = TabStrip->TabCount > 0;
		else if (OnClick)
		{
			Form->BeginUpdate();
			TabStrip->BeginUpdate();
			{
				for each (auto Tab in TabStrip->Tabs)
					Tab->Close();
			}
			TabStrip->EndUpdate();
			Form->EndUpdate();
		}

		break;
	case view::eViewRole::MainTabStrip_ContextMenu_PopOut:
		if (OnPopup)
			Button->Enabled = TabStrip->MouseOverTab != nullptr;
		else if (OnClick)
		{
			auto Document = safe_cast<model::IScriptDocument^>(safe_cast<view::components::ITabStripItem^>(Button->Tag)->Tag);
			auto NewBounds = View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm()->Bounds;
			NewBounds.X = NewBounds.X + 25;
			if (NewBounds.X >= Screen::PrimaryScreen->Bounds.Width)
				NewBounds.X = 50;

			NewBounds.Y = NewBounds.Y + 25;
			if (NewBounds.Y >= Screen::PrimaryScreen->Bounds.Height)
				NewBounds.Y = 50;

			auto NewController = New();
			NewController->RelocateDocument(Document, this);
			NewController->View->Reveal(NewBounds);
		}

		break;
	}
}

void ScriptEditorController::ViewEventHandler_EmptyWorkspacePanel(view::ViewComponentEvent^ E)
{
	switch (E->Component->Role)
	{
	case view::eViewRole::EmptyWorkspacePanel_NewScript:
		ViewActions->NewTabWithNewScript->Invoke();
		break;
	case view::eViewRole::EmptyWorkspacePanel_OpenScript:
		ViewActions->NewTabWithExistingScript->Invoke();
		break;
	}
}

void ScriptEditorController::ViewEventHandler_StatusBar(view::ViewComponentEvent^ E)
{
	switch (E->Component->Role)
	{
	case view::eViewRole::StatusBar_ErrorCount:
	case view::eViewRole::StatusBar_WarningCount:
		View->GetComponentByRole(view::eViewRole::Messages_DockPanel)->AsDockablePane()->Focus();
		break;
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

		if (Document == BoundDocument)
		{
			auto StatusBarDocDesc = View->GetComponentByRole(view::eViewRole::StatusBar_DocumentDescription)->AsLabel();
			StatusBarDocDesc->Image = TabItem->Image;
		}

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Bytecode:
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
		}
		else
		{
			BytecodeProgress->Text = String::Format("Compiled Script Size: {0:F2} KB", static_cast<float>(E->BytecodeLength / 1024.0));
			BytecodeProgress->Value = E->BytecodeLength;
			BytecodeProgress->Minimum = 0;
			BytecodeProgress->Maximum = model::IScriptDocument::MaximumScriptBytecodeLength;
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
		TabItem->Tooltip = TabItem->Text + "  (" + E->FormId.ToString("X8") + ")";

		if (Document == BoundDocument)
		{
			auto Form = View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm();
			Form->Text = TabItem->Tooltip + " -- " + view::IScriptEditorView::MainWindowDefaultTitle;

			auto StatusBarDocDesc = View->GetComponentByRole(view::eViewRole::StatusBar_DocumentDescription)->AsLabel();
			StatusBarDocDesc->Text = TabItem->Text + "  (" + E->FormId.ToString("X8") + ")";
		}

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Messages:
	{
		if (Document != BoundDocument)
			break;

		auto ListView = View->GetComponentByRole(view::eViewRole::Messages_ListView)->AsObjectListView();
		ListView->SetObjects(E->Messages, true);

		auto ErrorCount = Document->GetMessageCountErrors(0);
		auto WarningCount = Document->GetMessageCountWarnings(0);
		auto InfosCount = Document->GetMessageCountInfos(0);

		View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleErrors)->AsButton()->Text = ErrorCount + " " + (ErrorCount == 1 ? "Error" : "Errors");
		View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleWarnings)->AsButton()->Text = WarningCount + " " + (WarningCount == 1 ? "Warning" : "Warnings");
		View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleInfos)->AsButton()->Text = InfosCount + " " + (InfosCount == 1 ? "Message" : "Messages");

		auto StatusBarErrorCount = View->GetComponentByRole(view::eViewRole::StatusBar_ErrorCount)->AsButton();
		auto StatusBarWarningCount = View->GetComponentByRole(view::eViewRole::StatusBar_WarningCount)->AsButton();
		auto StatusBarNoIssuesIndicator = View->GetComponentByRole(view::eViewRole::StatusBar_NoIssuesIndicator)->AsButton();

		StatusBarErrorCount->Text = ErrorCount.ToString();
		StatusBarErrorCount->Visible = ErrorCount > 0;
		StatusBarWarningCount->Text = WarningCount.ToString();
		StatusBarWarningCount->Visible = WarningCount > 0;
		StatusBarNoIssuesIndicator->Visible = ErrorCount == 0 && WarningCount == 0;
		View->GetComponentByRole(view::eViewRole::StatusBar)->AsContainer()->Invalidate();

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
			QueryLabel->Text = "-";

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::DisplayingPreprocessorOutput:
	{
		if (Document != BoundDocument)
			break;

		auto StatusBarLabel = View->GetComponentByRole(view::eViewRole::StatusBar_PreprocessorOutput)->AsLabel();
		StatusBarLabel->Visible = E->DisplayingPreprocessorOutput;

		View->GetComponentByRole(view::eViewRole::StatusBar)->AsContainer()->Invalidate();

		break;
	}
	case model::IScriptDocument::StateChangeEventArgs::eEventType::LineOrColumn:
	{
		if (Document != BoundDocument)
			break;

		auto StatusBarLabelLine = View->GetComponentByRole(view::eViewRole::StatusBar_LineNumber)->AsLabel();
		StatusBarLabelLine->Text = "Ln " + E->Line.ToString();

		auto StatusBarLabelCol = View->GetComponentByRole(view::eViewRole::StatusBar_ColumnNumber)->AsLabel();
		StatusBarLabelCol->Text = "Col " + E->Column.ToString();

		break;
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
	InputManager->HandleTextEditorMouseClick(E, this);
}

void ScriptEditorController::ModelEventHandler_NavigationChanged(Object^ Sender, model::components::INavigationHelper::NavigationChangedEventArgs^ E)
{
	auto Document = safe_cast<model::IScriptDocument^>(Sender);
	Debug::Assert(Document == BoundDocument);

	DocumentNavigationHelper->HandleNavigationChangedEvent(E, Document);
}

System::Object^ ScriptTextAnnotationListLineNumberAspectGetter(Object^ E)
{
	auto Model = safe_cast<model::components::ScriptLineAnnotation^>(E);
	if (Model == nullptr)
		return nullptr;
	else if (!Model->Valid)
		return nullptr;

	return Model->Line;
}

System::Object^ ScriptTextAnnotationListTextAspectGetter(Object^ E)
{
	auto Model = safe_cast<model::components::ScriptLineAnnotation^>(E);
	if (Model == nullptr)
		return nullptr;

	return Model->Text;
}

System::Object^ MessageListTypeAspectGetter(Object^ E)
{
	auto Model = safe_cast<model::components::ScriptDiagnosticMessage^>(E);
	if (Model == nullptr)
		return nullptr;

	return Model->Type;
}

System::Object^ MessageListTypeImageGetter(Object^ RowObject)
{
	auto Model = safe_cast<model::components::ScriptDiagnosticMessage^>(RowObject);
	if (Model == nullptr)
		return nullptr;

	switch (Model->Type)
	{
	case model::components::ScriptDiagnosticMessage::eMessageType::Info:
		return view::components::CommonIcons::Get()->Info;
	case model::components::ScriptDiagnosticMessage::eMessageType::Warning:
		return view::components::CommonIcons::Get()->Warning;
	case model::components::ScriptDiagnosticMessage::eMessageType::Error:
		return view::components::CommonIcons::Get()->Error;
	}

	return nullptr;
}

System::String^ MessageListTypeAspectToStringConverter(Object^ E)
{
	return String::Empty;
}

System::Object^ MessageListSourceAspectGetter(Object^ E)
{
	auto Model = safe_cast<model::components::ScriptDiagnosticMessage^>(E);
	if (Model == nullptr)
		return nullptr;

	return Model->Source;
}

System::String^ MessageListSourceAspectToStringConverter(Object^ E)
{
	auto Key = safe_cast<model::components::ScriptDiagnosticMessage::eMessageSource>(E);
	return Key.ToString();
}

void ScriptEditorController::InitViewComponents()
{
	{
		auto MessagesListView = View->GetComponentByRole(view::eViewRole::Messages_ListView)->AsObjectListView();

		auto ColumnType = MessagesListView->AllocateNewColumn();
		auto ColumnLine = MessagesListView->AllocateNewColumn();
		auto ColumnText = MessagesListView->AllocateNewColumn();
		auto ColumnSource = MessagesListView->AllocateNewColumn();

		ColumnType->Text = "Type";
		ColumnType->MinimumWidth = 40;
		ColumnType->MaximumWidth = 40;
		ColumnType->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&MessageListTypeAspectGetter));
		ColumnType->SetAspectToStringGetter(gcnew view::components::IObjectListViewColumn::AspectToStringGetter(&MessageListTypeAspectToStringConverter));
		ColumnType->SetImageGetter(gcnew view::components::IObjectListViewColumn::ImageGetter(&MessageListTypeImageGetter));

		ColumnLine->Text = "Line";
		ColumnLine->MinimumWidth = 40;
		ColumnLine->MaximumWidth = 40;
		ColumnLine->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&ScriptTextAnnotationListLineNumberAspectGetter));

		ColumnText->Text = "Message";
		ColumnText->MinimumWidth = 600;
		ColumnText->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&ScriptTextAnnotationListTextAspectGetter));

		ColumnSource->Text = "Message Source";
		ColumnSource->MinimumWidth = 75;
		ColumnSource->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&MessageListSourceAspectGetter));
		ColumnSource->SetAspectToStringGetter(gcnew view::components::IObjectListViewColumn::AspectToStringGetter(&MessageListSourceAspectToStringConverter));

		MessagesListView->AddColumn(ColumnType);
		MessagesListView->AddColumn(ColumnLine);
		MessagesListView->AddColumn(ColumnText);
		MessagesListView->AddColumn(ColumnSource);

		MessagesListView->SetModelFilter(gcnew Predicate<Object^>(this, &ScriptEditorController::ShouldFilterScriptDiagnosticMessage));

		auto ToggleErrors = View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleErrors)->AsButton();
		auto ToggleWarnings = View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleWarnings)->AsButton();
		auto ToggleInfo = View->GetComponentByRole(view::eViewRole::Messages_Toolbar_ToggleInfos)->AsButton();

		ToggleErrors->Checked = true;
		ToggleErrors->Tag = model::components::ScriptDiagnosticMessage::eMessageType::Error;
		ToggleWarnings->Checked = true;
		ToggleWarnings->Tag = model::components::ScriptDiagnosticMessage::eMessageType::Warning;
		ToggleInfo->Checked = true;
		ToggleInfo->Tag = model::components::ScriptDiagnosticMessage::eMessageType::Info;
	}

	{
		auto BookmarksListView = View->GetComponentByRole(view::eViewRole::Bookmarks_ListView)->AsObjectListView();

		auto ColumnLine = BookmarksListView->AllocateNewColumn();
		auto ColumnText = BookmarksListView->AllocateNewColumn();

		ColumnLine->Text = "Line";
		ColumnLine->MinimumWidth = 40;
		ColumnLine->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&ScriptTextAnnotationListLineNumberAspectGetter));

		ColumnText->Text = "Description";
		ColumnText->MinimumWidth = 600;
		ColumnText->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&ScriptTextAnnotationListTextAspectGetter));

		BookmarksListView->AddColumn(ColumnLine);
		BookmarksListView->AddColumn(ColumnText);
	}

	InputManager->RefreshViewComponentShortcutTexts();
	ResetViewComponentsToUnboundState();
}

void ScriptEditorController::ProcessInstantiationParameters(IScriptEditorController::InstantiationParams^ Params)
{
	if (Params->Operations == IScriptEditorController::InstantiationParams::eInitOperation::None)
	{
		View->Reveal(Params->InitialBounds);
		return;
	}

	bool IsSingleDocument = Params->ExistingScriptEditorIds->Count == 1 || Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::CreateNewScript);
	auto NewTabParams = gcnew NewTabCreationParams;

	if (Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::CreateNewScript))
	{
		NewTabParams->InitOperation = NewTabCreationParams::eInitOperation::NewScript;
		NewTabParams->BindAfterInit = true;
		CreateNewTab(NewTabParams);
	}
	else if (Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::LoadExistingScript))
	{
		NewTabParams->InitOperation = NewTabCreationParams::eInitOperation::LoadExistingScript;

		for (int i = 0; i < Params->ExistingScriptEditorIds->Count; ++i)
		{
			NewTabParams->ExistingScriptEditorId = Params->ExistingScriptEditorIds[i];
			Debug::Assert(NewTabParams->ExistingScriptEditorId->Length > 0);
			NewTabParams->BindAfterInit = i == 0;
			CreateNewTab(NewTabParams);
		}
	}
	else
		throw gcnew ArgumentException("Unknown controller instantiation operation!");

	if (Params->Operations.HasFlag(IScriptEditorController::InstantiationParams::eInitOperation::PerformFind))
	{
		bool DoGlobalFind = !IsSingleDocument;
		FindReplaceHelper->DoOperation(textEditor::eFindReplaceOperation::Find, Params->FindQuery, "", textEditor::eFindReplaceOptions::CaseInsensitive, DoGlobalFind, this);
	}

	View->Reveal(Params->InitialBounds);
}

void ScriptEditorController::QueueDeferredUiAction(DeferredUiActionDelegate^ Action, UInt32 TimeoutInMs)
{
	Debug::Assert(!DeferredUiActionTimer->Enabled);
	Debug::Assert(DelegateDeferredUiAction == nullptr);

	DelegateDeferredUiAction = Action;
	DeferredUiActionTimer->Interval = TimeoutInMs;
	DeferredUiActionTimer->Start();
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
	InputManager = gcnew components::InputManager(ChildView);

	ActiveDocumentActions = gcnew ActiveDocumentActionCollection;
	ActiveDocumentActions->Copy->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Copy);
	ActiveDocumentActions->Paste->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Paste);
	ActiveDocumentActions->Comment->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Comment);
	ActiveDocumentActions->Uncomment->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_Uncomment);
	ActiveDocumentActions->AddBookmark->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddBookmark);
	ActiveDocumentActions->GoToLine->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_GoToLine);
	ActiveDocumentActions->JumpToScriptAtCaret->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_JumpToScriptAtCaret);
	ActiveDocumentActions->AddVarInteger->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarInteger);
	ActiveDocumentActions->AddVarFloat->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarFloat);
	ActiveDocumentActions->AddVarReference->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarReference);
	ActiveDocumentActions->AddVarString->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarString);
	ActiveDocumentActions->AddVarArray->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ActiveDocumentAction_AddVarArray);
	ActiveDocumentActions->CreateDefaultKeyBindings(InputManager);

	ViewActions = gcnew ViewActionCollection;
	ViewActions->CurrentTabNewScript->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabNewScript);
	ViewActions->CurrentTabOpenScript->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabOpenScript);
	ViewActions->CurrentTabSaveScript->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabSaveScript);
	ViewActions->CurrentTabPreviousScript->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabPreviousScript);
	ViewActions->CurrentTabNextScript->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CurrentTabNextScript);
	ViewActions->SaveAllTabs->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_SaveAllTabs);
	ViewActions->NewTabWithNewScript->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_NewTabWithNewScript);
	ViewActions->NewTabWithExistingScript->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_NewTabWithExistingScript);
	ViewActions->PreviousTab->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_PreviousTab);
	ViewActions->NextTab->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_NextTab);
	ViewActions->CloseCurrentTab->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_CloseCurrentTab);
	ViewActions->ShowFindReplacePane->InvokeDelegate = gcnew utilities::BasicAction::InvokationDelegate(this, &ScriptEditorController::ViewAction_ShowFindReplacePane);
	ViewActions->CreateDefaultKeyBindings(InputManager);

	DeferredUiActionTimer = gcnew Timer;
	DeferredUiActionTimer->Interval = 100;
	DeferredUiActionTimer->Enabled = false;

	DelegateViewComponentEvent = gcnew view::IScriptEditorView::EventHandler(this, &ScriptEditorController::ViewEventHandler_ComponentEvent);
	DelegateModelDocumentStateChanged = gcnew model::IScriptDocument::StateChangeEventHandler(this, &ScriptEditorController::ModelEventHandler_DocumentStateChanged);
	DelegateModelKeyDown = gcnew KeyEventHandler(this, &ScriptEditorController::ModelEventHandler_KeyDown);
	DelegateModelMouseClick = gcnew textEditor::TextEditorMouseClickEventHandler(this, &ScriptEditorController::ModelEventHandler_MouseClick);
	DelegateModelNavigationChanged = gcnew model::components::INavigationHelper::NavigationChangedEventHandler(this, &ScriptEditorController::ModelEventHandler_NavigationChanged);
	DelegateDeferredUiActionTimerTick = gcnew EventHandler(this, &ScriptEditorController::DeferredUiActionTimer_Tick);
	DelegateDeferredUiAction = nullptr;

	DisableDocumentActivationOnTabSwitch = false;

	ChildView->ComponentEvent += DelegateViewComponentEvent;
	DeferredUiActionTimer->Tick += DelegateDeferredUiActionTimerTick;

	InitViewComponents();

	components::ViewTabTearingHelper::Get()->RegisterTabStrip(View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip(), this);
	ScriptEditorInstanceManager::Get()->RegisterController(this);
}

ScriptEditorController::ScriptEditorController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory, IScriptEditorController::InstantiationParams^ InitParams)
	: ScriptEditorController(ModelFactory, ViewFactory)
{
	ProcessInstantiationParameters(InitParams);
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

	DeferredUiActionTimer->Tick -= DelegateDeferredUiActionTimerTick;
	SAFEDELETE_CLR(DeferredUiActionTimer);

	SAFEDELETE_CLR(ChildModel);
	if (ChildView)
	{
		ChildView->ComponentEvent -= DelegateViewComponentEvent;
		components::ViewTabTearingHelper::Get()->DeregisterTabStrip(View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip());

		SAFEDELETE_CLR(ChildView);
	}

	SAFEDELETE_CLR(ChildView);
	SAFEDELETE_CLR(ChildModel);

	SAFEDELETE_CLR(DelegateViewComponentEvent);
	SAFEDELETE_CLR(DelegateModelDocumentStateChanged);
	SAFEDELETE_CLR(DelegateModelKeyDown);
	SAFEDELETE_CLR(DelegateModelMouseClick);
	SAFEDELETE_CLR(DelegateModelNavigationChanged);
	SAFEDELETE_CLR(DelegateDeferredUiActionTimerTick);
	SAFEDELETE_CLR(DelegateDeferredUiAction);

	ScriptEditorInstanceManager::Get()->DeregisterController(this);
}

void ScriptEditorController::ActiveDocument::set(model::IScriptDocument^ v)
{
	if (BoundDocument == v)
		return;

	ActivateDocumentInView(v);
}

IScriptEditorController^ ScriptEditorController::New()
{
	return ControllerFactory->NewController(ModelFactory, ViewFactory);
}

IScriptEditorController^ ScriptEditorController::New(IScriptEditorController::InstantiationParams^ Params)
{
	return ControllerFactory->NewController(ModelFactory, ViewFactory, Params);
}

void ScriptEditorController::RelocateDocument(model::IScriptDocument^ Document, IScriptEditorController^ Source)
{
	auto OtherController = safe_cast<ScriptEditorController^>(Source);

	OtherController->View->BeginUpdate();
	this->View->BeginUpdate();
	{
		if (OtherController->ActiveDocument == Document)
			OtherController->UnbindBoundDocument();

		OtherController->DetachDocumentFromView(Document);
		OtherController->Model->RemoveDocument(Document);

		this->Model->AddDocument(Document);
		this->AttachDocumentToView(Document);
		this->ActivateDocumentInView(Document);
	}
	this->View->EndUpdate();
	OtherController->View->EndUpdate();

	// dispose the source controller if it has no other documents
	// as we don't want to have empty views sticking around
	if (OtherController->Model->Documents->Count == 0)
		delete OtherController;
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

IScriptEditorController^ ScriptEditorControllerFactory::NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory)
{
	return gcnew controllerImpl::ScriptEditorController(ModelFactory, ViewFactory);
}

IScriptEditorController^ ScriptEditorControllerFactory::NewController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory, IScriptEditorController::InstantiationParams^ InitParams)
{
	return gcnew controllerImpl::ScriptEditorController(ModelFactory, ViewFactory, InitParams);
}

ScriptEditorControllerFactory^ ScriptEditorControllerFactory::NewFactory()
{
	return gcnew ScriptEditorControllerFactory;
}


} // namespace controllerImpl


} // namespace scriptEditor


} // namespace cse