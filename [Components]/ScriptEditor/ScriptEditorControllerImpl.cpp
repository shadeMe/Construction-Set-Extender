#pragma once

#include "ScriptEditorControllerImpl.h"
#include "ScriptSync.h"
#include "Preferences.h"

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
}

ActiveDocumentActionCollection::~ActiveDocumentActionCollection()
{
	SAFEDELETE_CLR(Copy);
	SAFEDELETE_CLR(Paste);
	SAFEDELETE_CLR(Comment);
	SAFEDELETE_CLR(Uncomment);
	SAFEDELETE_CLR(AddBookmark);
	SAFEDELETE_CLR(GoToLine);
}

void ActiveDocumentActionCollection::CreateDefaultKeyBindings(components::InputManager^ InputManager)
{
	InputManager->AddKeyChordCommand(Copy, KeyCombo::New(Keys::Control, Keys::C), false);
	InputManager->AddKeyChordCommand(Paste, KeyCombo::New(Keys::Control, Keys::V), false);
	InputManager->AddKeyChordCommand(Comment, KeyCombo::New(Keys::Control, Keys::K), KeyCombo::New(Keys::Control, Keys::C), false);
	InputManager->AddKeyChordCommand(Uncomment, KeyCombo::New(Keys::Control, Keys::K), KeyCombo::New(Keys::Control, Keys::U), false);
	InputManager->AddKeyChordCommand(AddBookmark, KeyCombo::New(Keys::Control, Keys::B), false);
	InputManager->AddKeyChordCommand(GoToLine, KeyCombo::New(Keys::Control, Keys::G), false);
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

	BoundDocument->StateChanged += DelegateModelDocumentStateChanged;
	BoundDocument->TextEditor->KeyDown += DelegateModelKeyDown;
	BoundDocument->TextEditor->MouseClick += DelegateModelMouseClick;;
	BoundDocument->NavigationHelper->NavigationChanged += DelegateModelNavigationChanged;

	BoundDocument->IntelliSenseModel->Bind(View->IntelliSenseView);
	BoundDocument->TextEditor->Bind();
	BoundDocument->BackgroundAnalyzer->Resume();

	BoundDocument->PushStateToSubscribers();

	ValidateDocumentSyncingStatus(BoundDocument);
}

void ScriptEditorController::UnbindBoundDocument()
{
	Debug::Assert(BoundDocument != nullptr);

	BoundDocument->BackgroundAnalyzer->Pause();
	BoundDocument->TextEditor->Unbind();
	BoundDocument->IntelliSenseModel->Unbind();

	BoundDocument->StateChanged -= DelegateModelDocumentStateChanged;
	BoundDocument->TextEditor->KeyDown -= DelegateModelKeyDown;
	BoundDocument->TextEditor->MouseClick -= DelegateModelMouseClick;;
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
		view::eViewRole::TextEditor_ContextMenu_OpenPreprocessorImportFile,
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

bool ScriptEditorController::HandleUnsavedChangesBeforeDestructiveOperation(model::IScriptDocument^ Document)
{
	bool Throwaway;
	return HandleUnsavedChangesBeforeDestructiveOperation(Document, Throwaway);
}

bool ScriptEditorController::HandleUnsavedChangesBeforeDestructiveOperation(model::IScriptDocument^ Document, bool% OutOperationCancelled)
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

	OutOperationCancelled = true;
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
		NewTabItem->Text = "Unsaved Script";
	NewTabItem->Tooltip = NewTabItem->Text + "(" + Document->ScriptFormID.ToString("X8") + ")";
	NewTabItem->Image = Document->Dirty ? view::components::CommonIcons::Get()->UnsavedChanges : nullptr;

	TabStrip->AddTab(NewTabItem);
}

void ScriptEditorController::DetachDocumentFromView(model::IScriptDocument^ Document)
{
	Debug::Assert(BoundDocument != Document);

	auto TabStrip = View->GetComponentByRole(view::eViewRole::MainTabStrip)->AsTabStrip();
	auto TabItem = TabStrip->LookupTabByTag(Document);

	Debug::Assert(TabItem != nullptr);

	TabStrip->RemoveTab(TabItem);
	delete TabItem;
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

	Rectangle Bounds = View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm()->Bounds;
	nativeWrapper::g_CSEInterfaceTable->ScriptEditor.SaveEditorBoundsToINI(Bounds.Left, Bounds.Top, Bounds.Width, Bounds.Height);

	delete this;
}

void ScriptEditorController::CreateNewTab(NewTabCreationParams^ Params)
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

	if (!HandleUnsavedChangesBeforeDestructiveOperation(Document))
		return;

	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> NewScriptInstance(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CreateNewScript());
	Document->Initialize(NewScriptInstance.get(), false);
}

void ScriptEditorController::LoadExistingScriptIntoDocument(model::IScriptDocument^ Document, String^ ExistingScriptEditorId)
{
	Debug::Assert(Document != nullptr);

	if (!HandleUnsavedChangesBeforeDestructiveOperation(Document))
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

	if (!HandleUnsavedChangesBeforeDestructiveOperation(Document))
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

	if (!HandleUnsavedChangesBeforeDestructiveOperation(Document))
		return;

	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetNextScriptInList(Document->ScriptNativeObject));
	if (Data)
		Document->Initialize(Data.get(), ShouldUseAutoRecoveryFile(gcnew String(Data->EditorID)));
}

void ScriptEditorController::LoadPreviousScriptIntoDocument(model::IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!HandleUnsavedChangesBeforeDestructiveOperation(Document))
		return;

	DisposibleDataAutoPtr<componentDLLInterface::ScriptData> Data(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreviousScriptInList(Document->ScriptNativeObject));
	if (Data)
		Document->Initialize(Data.get(), ShouldUseAutoRecoveryFile(gcnew String(Data->EditorID)));
}

bool ScriptEditorController::CloseAndRemoveDocument(model::IScriptDocument^ Document, bool% OutOperationCancelled)
{
	Debug::Assert(Document != nullptr);

	bool ScriptSavedOrIsUnmodified = HandleUnsavedChangesBeforeDestructiveOperation(Document, OutOperationCancelled);
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

		auto ExistingScriptEditorIds = View->SelectExistingScripts(BoundDocument ? BoundDocument->ScriptEditorID : "");
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

		auto SelectionText = Selection->ToString();
		if (String::Equals(SelectionText, "Object Script"))
			BoundDocument->ScriptType = model::IScriptDocument::eScriptType::Object;
		else if (String::Equals(SelectionText, "Quest Script"))
			BoundDocument->ScriptType = model::IScriptDocument::eScriptType::Quest;
		else if (String::Equals(SelectionText, "Magic Effect Script"))
			BoundDocument->ScriptType = model::IScriptDocument::eScriptType::MagicEffect;
		else
			throw gcnew ArgumentException("Unexpected script type " + SelectionText);

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
			BoundDocument->TogglePreprocessorOutput(BoundDocument->IsPreprocessorOutputEnabled() == false);
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

		break;
	case view::eViewRole::MainToolbar_Tools_ModifyVariableIndices:

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


		break;
	}
	case view::eViewRole::MainToolbar_Tools_DeleteScripts:

		break;
	case view::eViewRole::MainToolbar_Tools_SyncScripts:

		break;
	case view::eViewRole::MainToolbar_Tools_CodeSnippets:

		break;
	case view::eViewRole::MainToolbar_Tools_Preferences:

		break;
	}
}

void ScriptEditorController::ViewEventHandler_MainToolbarMenuHelp(view::ViewComponentEvent^ E)
{

}

void ScriptEditorController::ViewEventHandler_MessagesPanel(view::ViewComponentEvent^ E)
{

}

void ScriptEditorController::ViewEventHandler_BookmarksPanel(view::ViewComponentEvent^ E)
{

}

void ScriptEditorController::ViewEventHandler_OutlineViewPanel(view::ViewComponentEvent^ E)
{

}

void ScriptEditorController::ViewEventHandler_TextEditorContextMenu(view::ViewComponentEvent^ E)
{

}

void ScriptEditorController::ModelEventHandler_DocumentStateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E)
{

}

void ScriptEditorController::ModelEventHandler_KeyDown(Object^ Sender, KeyEventArgs^ E)
{

}

void ScriptEditorController::ModelEventHandler_MouseClick(Object^ Sender, textEditor::TextEditorMouseClickEventArgs^ E)
{

}

void ScriptEditorController::ModelEventHandler_NavigationChanged(Object^ Sender, model::components::INavigationHelper::NavigationChangedEventArgs^ E)
{

}

ScriptEditorController::ScriptEditorController(model::IFactory^ ModelFactory, view::IFactory^ ViewFactory)
{
	// inti the script ttpy combobo items with tags
}

ScriptEditorController::~ScriptEditorController()
{
	if (ChildView)
		SAFEDELETE_CLR(ChildView);
}

IScriptEditorController^ ScriptEditorController::New()
{

}

IScriptEditorController^ ScriptEditorController::New(Rectangle ViewInitialBounds)
{

}

void ScriptEditorController::RelocateDocument(model::IScriptDocument^ Document, IScriptEditorController^ Source)
{

}

void ScriptEditorController::ActivateOrCreateNewDocument(String^ ScriptEditorId)
{

}



} // namespace controllerImpl


} // namespace scriptEditor


} // namespace cse