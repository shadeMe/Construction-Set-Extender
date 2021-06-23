#include "ScriptEditorModelImpl.h"
#include "Preferences.h"
#include "IntelliSenseInterfaceModel.h"
#include "IntelliSenseDatabase.h"
#include "TextEditorFactory.h"
#include "ScriptPreprocessor.h"

namespace cse
{


namespace scriptEditor
{


namespace modelImpl
{


void ScriptDocument::Editor_ScriptModified(Object^ Sender, textEditor::TextEditorScriptModifiedEventArgs^ E)
{
	OnStateChangedDirty(E->ModifiedStatus);
}

generic <typename T> where T : textEditor::ILineAnchor
bool RemoveInvalidatedAnchors(List<T>^ Source)
{
	auto Invalidated = gcnew List<T>;
	for each (auto Itr in Source)
	{
		if (!Itr->Valid)
			Invalidated->Add(Itr);
	}

	for each (auto Itr in Invalidated)
		Source->Remove(Itr);

	return Invalidated->Count != 0;
}

void ScriptDocument::Editor_LineAnchorInvalidated(Object^ Sender, EventArgs^ E)
{
	if (RemoveInvalidatedAnchors<ScriptDiagnosticMessage^>(Messages))
		OnStateChangedMessages();

	if (RemoveInvalidatedAnchors<ScriptBookmark^>(Bookmarks))
		OnStateChangedBookmarks();

	if (RemoveInvalidatedAnchors<ScriptFindResult^>(FindResults))
		OnStateChangedFindResults();
}

void ScriptDocument::BgAnalyzer_AnalysisComplete(Object^ Sender, IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs^ E)
{
	BeginBatchUpdate(eBatchUpdateSource::Messages);
	{
		ClearMessages(ScriptDiagnosticMessage::eMessageSource::Validator, ScriptDiagnosticMessage::eMessageType::All);
		for each (auto Itr in E->Result->AnalysisMessages)
		{
			AddMessage(Itr->Line, Itr->Message,
					   Itr->Critical ? ScriptDiagnosticMessage::eMessageType::Error : ScriptDiagnosticMessage::eMessageType::Warning,
					   ScriptDiagnosticMessage::eMessageSource::Validator);
		}
	}
	EndBatchUpdate(eBatchUpdateSource::Messages);
}

void ScriptDocument::AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E)
{
	if (!preferences::SettingsHolder::Get()->Backup->UseAutoRecovery)
		return;
	else if (!Valid)
		return;
	else if (EditorID->Length == 0)
		return;
	else if (!Dirty)
		return;

	SaveAutoRecoveryCache();
}

void ScriptDocument::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
{
	AutoSaveTimer->Stop();
	AutoSaveTimer->Interval = preferences::SettingsHolder::Get()->Backup->AutoRecoveryInterval * 1000 * 60;
	AutoSaveTimer->Start();
}

void ScriptDocument::OnStateChangedDirty(bool Modified)
{
	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::Dirty;
	E->Dirty = Modified;
	StateChanged(this, E);
}

void ScriptDocument::OnStateChangedBytecodeLength(UInt16 Size)
{
	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::BytecodeLength;
	E->BytecodeLength = Size;
	StateChanged(this, E);
}

void ScriptDocument::OnStateChangedType(IScriptDocument::eScriptType Type)
{
	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::ScriptType;
	E->ScriptType = Type;
	StateChanged(this, E);
}

void ScriptDocument::OnStateChangedEditorIdAndFormId(String^ EditorId, UInt32 FormId)
{
	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::EditorIdAndFormId;
	E->EditorId = EditorId;
	E->FormId = FormId;
	StateChanged(this, E);
}

void ScriptDocument::OnStateChangedMessages()
{
	if (ActiveBatchUpdateSource == eBatchUpdateSource::Messages)
		return;

	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::Messages;
	E->Messages = Messages;
	StateChanged(this, E);
}

void ScriptDocument::OnStateChangedBookmarks()
{
	if (ActiveBatchUpdateSource == eBatchUpdateSource::Bookmarks)
		return;

	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::Bookmarks;
	E->Bookmarks = Bookmarks;
	StateChanged(this, E);
}

void ScriptDocument::OnStateChangedFindResults()
{
	if (ActiveBatchUpdateSource == eBatchUpdateSource::FindResults)
		return;

	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::FindResults;
	E->FindResults = FindResults;
	StateChanged(this, E);
}

void DummyOutputWrapper(int Line, String^ Message) {}

System::String^ ScriptDocument::PreprocessScriptText(String^ ScriptText, bool SuppressErrors, bool% OutPreprocessResult, bool% OutContainsDirectives)
{
	String^ Preprocessed = "";
	auto ErrorOutput = gcnew preprocessor::StandardOutputError(&DummyOutputWrapper);
	if (SuppressErrors == false)
	{
		ErrorOutput = gcnew preprocessor::StandardOutputError(this, &ScriptDocument::TrackPreprocessorMessage);
		BeginBatchUpdate(eBatchUpdateSource::Messages);
		ClearMessages(ScriptDiagnosticMessage::eMessageSource::Preprocessor, ScriptDiagnosticMessage::eMessageType::All);
	}


	auto PreprocessorParams = gcnew preprocessor::PreprocessorParams(gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorBasePath()),
		gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetPreprocessorStandardPath()),
		preferences::SettingsHolder::Get()->Preprocessor->AllowMacroRedefs,
		preferences::SettingsHolder::Get()->Preprocessor->NumPasses);

	bool Result = preprocessor::Preprocessor::Get()->PreprocessScript(ScriptText, Preprocessed, ErrorOutput, PreprocessorParams);

	if (SuppressErrors == false)
		EndBatchUpdate(eBatchUpdateSource::Messages);

	OutPreprocessResult = Result;
	return Preprocessed;
}

void ScriptDocument::TrackPreprocessorMessage(int Line, String^ Message)
{
	AddMessage(Line, Message, ScriptDiagnosticMessage::eMessageType::Error, ScriptDiagnosticMessage::eMessageSource::Preprocessor);
}

ScriptCompilationData^ ScriptDocument::BeginScriptCompilation()
{
	Debug::Assert(CompilationInProgress == false);
	CompilationInProgress = true;

	BeginBatchUpdate(eBatchUpdateSource::Messages);

	auto Result = gcnew ScriptCompilationData;
	Result->UnpreprocessedScriptText = TextEditor->GetText();

	auto SemanticAnalysisData = BgAnalyzer->DoSynchronousAnalysis(false);
	ClearMessages(ScriptDiagnosticMessage::eMessageSource::Validator, ScriptDiagnosticMessage::eMessageType::All);

	for each (auto Itr in SemanticAnalysisData->AnalysisMessages)
	{
		AddMessage(Itr->Line, Itr->Message,
				   Itr->Critical ? ScriptDiagnosticMessage::eMessageType::Error : ScriptDiagnosticMessage::eMessageType::Warning,
				   ScriptDiagnosticMessage::eMessageSource::Validator);
	}

	if (!SemanticAnalysisData->HasCriticalMessages && !SemanticAnalysisData->MalformedStructure)
	{
		Result->PreprocessedScriptText = PreprocessScriptText(Result->UnpreprocessedScriptText, false, Result->CanCompile, Result->HasPreprocessorDirectives);
		if (Result->CanCompile)
		{
			auto Metadata = PrepareMetadataForSerialization(Result->HasPreprocessorDirectives);
			Result->SerializedMetadata = ScriptTextMetadataHelper::SerializeMetadata(Metadata);
			ClearMessages(ScriptDiagnosticMessage::eMessageSource::Compiler, ScriptDiagnosticMessage::eMessageType::All);
		}
	}

	// doesn't include compiler warnings for obvious reasons but it's okay since all compiler messages are errors
	Result->HasWarnings = GetWarningCount(0);
	return Result;
}

void ScriptDocument::EndScriptCompilation(ScriptCompilationData^ Data)
{
	Debug::Assert(CompilationInProgress == true);
	CompilationInProgress = false;

	String^ kRepeatedString = "Compiled script not saved!";

	if (Data->CanCompile)
	{
		if (Data->CompileResult->CompilationSuccessful)
		{
			String^ OriginalText = Data->UnpreprocessedScriptText + Data->SerializedMetadata;
			CString OrgScriptText(OriginalText);
			nativeWrapper::g_CSEInterfaceTable->ScriptEditor.SetScriptText(ScriptNativeObject, OrgScriptText.c_str());

			// update model local state
			EditorID = gcnew String(Data->CompileResult->Script.EditorID);
			FormID = Data->CompileResult->Script.FormID;
			Bytecode = reinterpret_cast<UInt8*>(Data->CompileResult->Script.ByteCode);
			BytecodeLength = Data->CompileResult->Script.Length;

			ScriptType = safe_cast<IScriptDocument::eScriptType>(Data->CompileResult->Script.Type);
			OnStateChangedEditorIdAndFormId(EditorID, FormID);
			OnStateChangedBytecodeLength(BytecodeLength);
			TextEditor->Modified = false;
		}
		else
		{
			for (int i = 0; i < Data->CompileResult->CompileErrorData.Count; i++)
			{
				String^ Message = gcnew String(Data->CompileResult->CompileErrorData.ErrorListHead[i].Message);
				Message = Message->Replace(kRepeatedString, String::Empty);

				int Line = Data->CompileResult->CompileErrorData.ErrorListHead[i].Line;
				if (Line < 1)
					Line = 1;

				AddMessage(Line, Message, ScriptDiagnosticMessage::eMessageType::Error, ScriptDiagnosticMessage::eMessageSource::Compiler);
			}
		}
	}

	EndBatchUpdate(eBatchUpdateSource::Messages);
}

ScriptBookmark^ ScriptDocument::LookupBookmark(UInt32 Line, String^ Text)
{
	for each (auto Itr in Bookmarks)
	{
		if (Itr->Line == Line && Itr->Text->Equals(Text))
			return Itr;
	}

	return nullptr;
}

System::String^ ScriptDocument::GetPrettyName()
{
	return EditorID + " (" + FormID.ToString("X8") + ")";
}

void ScriptDocument::BeginBatchUpdate(eBatchUpdateSource Source)
{
	Debug::Assert(ActiveBatchUpdateSource == eBatchUpdateSource::None || ActiveBatchUpdateSource == Source);
	Debug::Assert(Source != eBatchUpdateSource::None);

	++ActiveBatchUpdateCounter;
	ActiveBatchUpdateSource = Source;
}

void ScriptDocument::EndBatchUpdate(eBatchUpdateSource Source)
{
	Debug::Assert(ActiveBatchUpdateSource != eBatchUpdateSource::None);

	ActiveBatchUpdateCounter--;
	Debug::Assert(ActiveBatchUpdateCounter >= 0);

	if (ActiveBatchUpdateCounter == 0)
	{
		auto CompletedBatchUpdateSource = ActiveBatchUpdateSource;
		ActiveBatchUpdateSource = eBatchUpdateSource::None;

		switch (CompletedBatchUpdateSource)
		{
		case eBatchUpdateSource::Messages:
			OnStateChangedMessages();
			break;
		case eBatchUpdateSource::Bookmarks:
			OnStateChangedBookmarks();
			break;
		case eBatchUpdateSource::FindResults:
			OnStateChangedFindResults();
			break;
		}
	}
}

void ScriptDocument::AddMessage(UInt32 Line, String^ Text, ScriptDiagnosticMessage::eMessageType Type, ScriptDiagnosticMessage::eMessageSource Source)
{
	if (Line > TextEditor->LineCount)
		Line = TextEditor->LineCount;

	auto LineAnchor = TextEditor->CreateLineAnchor(Line);
	auto NewMessage = gcnew ScriptDiagnosticMessage(LineAnchor,
													Text->Replace("\t", "")->Replace("\r", "")->Replace("\n", ""),
													Type, Source);
	Messages->Add(NewMessage);
	OnStateChangedMessages();
}

void ScriptDocument::ClearMessages(ScriptDiagnosticMessage::eMessageSource SourceFilter, ScriptDiagnosticMessage::eMessageType TypeFilter)
{
	auto Buffer = gcnew List<ScriptDiagnosticMessage^> ;

	for each (auto Itr in Messages)
	{
		bool MatchedSource = SourceFilter == ScriptDiagnosticMessage::eMessageSource::All || Itr->Source == SourceFilter;
		bool MatchedType = TypeFilter == ScriptDiagnosticMessage::eMessageType::All || Itr->Type == TypeFilter;

		if (MatchedSource && MatchedType)
			Buffer->Add(Itr);
	}

	if (Buffer->Count)
	{
		for each (auto Itr in Buffer)
			Messages->Remove(Itr);

		OnStateChangedBookmarks();
	}
}

void ScriptDocument::ClearBookmarks()
{
	Bookmarks->Clear();
	OnStateChangedBookmarks();
}

void ScriptDocument::AddFindResult(String^ Query, UInt32 Line, String^ PreviewText, UInt32 Hits)
{
	if (Line > TextEditor->LineCount)
		Line = TextEditor->LineCount;

	auto LineAnchor = TextEditor->CreateLineAnchor(Line);
	auto NewFindResult= gcnew ScriptFindResult(LineAnchor, PreviewText->Replace("\t", ""), Hits, Query);
	FindResults->Add(NewFindResult);
	OnStateChangedFindResults();
}

void ScriptDocument::ClearFindResults()
{
	FindResults->Clear();
	OnStateChangedFindResults();
}

System::String^ ScriptDocument::LoadAutoRecoveryCache()
{
	auto LoadPath = GetAutoRecoveryCacheFilePath();
	if (!System::IO::File::Exists(LoadPath))
		return String::Empty;

	try
	{
		auto RecoveredText = System::IO::File::ReadAllText(LoadPath);
		Microsoft::VisualBasic::FileIO::FileSystem::DeleteFile(LoadPath,
															   Microsoft::VisualBasic::FileIO::UIOption::OnlyErrorDialogs,
															   Microsoft::VisualBasic::FileIO::RecycleOption::SendToRecycleBin);

		return RecoveredText;
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't load auto-recovery file from '" + LoadPath + "'!\n\tException: " + E->Message, true);
	}

	return String::Empty;
}

void ScriptDocument::SaveAutoRecoveryCache()
{
	auto Metadata = PrepareMetadataForSerialization(false);
	auto TextToSave = ScriptText + ScriptTextMetadataHelper::SerializeMetadata(Metadata);
	auto SavePath = GetAutoRecoveryCacheFilePath();

	try
	{
		System::IO::File::WriteAllText(SavePath, TextToSave);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't save auto-recovery file to '" + SavePath + "'!\n\tException: " + E->Message, true);
	}
}

System::String^ ScriptDocument::GetAutoRecoveryCacheFilePath()
{
	auto AutoRecoveryDir = gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetAutoRecoveryCachePath());
	return AutoRecoveryDir + "\\" + GetPrettyName() + ".txt";
}

void ScriptDocument::ClearAutoRecoveryCache()
{
	try
	{
		auto Filepath = GetAutoRecoveryCacheFilePath();
		System::IO::File::Delete(Filepath);
	}
	catch (...) {}
}

ScriptTextMetadata^ ScriptDocument::PrepareMetadataForSerialization(bool HasPreprocessorDirectives)
{
	auto Metadata = gcnew ScriptTextMetadata;
	Metadata->CaretPos = Editor->Caret;
	Metadata->HasPreprocessorDirectives = HasPreprocessorDirectives;
	for each (auto Itr in Bookmarks)
		Metadata->Bookmarks->Add(gcnew ScriptTextMetadata::Bookmark(Itr->Line, Itr->Text));

	return Metadata;
}

ScriptDocument::ScriptDocument()
{
	Type = IScriptDocument::eScriptType::Object;
	EditorID = String::Empty;
	FormID = 0;
	Bytecode = nullptr;
	BytecodeLength = 0;
	NativeObject = NativeObject;
	CompilationInProgress = false;

	Messages = gcnew List<ScriptDiagnosticMessage^>;
	Bookmarks = gcnew List<ScriptBookmark^>;
	FindResults = gcnew List<ScriptFindResult^>;
	ActiveBatchUpdateSource = eBatchUpdateSource::None;
	ActiveBatchUpdateCounter = 0;

	Editor = textEditor::Factory::Create(textEditor::Factory::eTextEditor::AvalonEdit, this);
	IntelliSense = gcnew intellisense::IntelliSenseInterfaceModel(Editor);
	BgAnalyzer = gcnew BackgroundSemanticAnalyzer(this);
	NavHelper = gcnew components::NavigationHelper(this);

	AutoSaveTimer = gcnew Timer();
	AutoSaveTimer->Interval = preferences::SettingsHolder::Get()->Backup->AutoRecoveryInterval * 1000 * 60;

	EditorScriptModifiedHandler = gcnew textEditor::TextEditorScriptModifiedEventHandler(this, &ScriptDocument::Editor_ScriptModified);
	EditorLineAnchorInvalidatedHandler = gcnew EventHandler(this, &ScriptDocument::Editor_LineAnchorInvalidated);
	BgAnalysisCompleteHandler = gcnew IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler(this, &ScriptDocument::BgAnalyzer_AnalysisComplete);
	AutoSaveTimerTickHandler = gcnew EventHandler(this, &ScriptDocument::AutoSaveTimer_Tick);
	ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ScriptDocument::ScriptEditorPreferences_Saved);

	Editor->ScriptModified += EditorScriptModifiedHandler;
	Editor->LineAnchorInvalidated += EditorLineAnchorInvalidatedHandler;
	BgAnalyzer->SemanticAnalysisComplete += BgAnalysisCompleteHandler;
	preferences::SettingsHolder::Get()->SavedToDisk += ScriptEditorPreferencesSavedHandler;
	AutoSaveTimer->Tick += AutoSaveTimerTickHandler;

	AutoSaveTimer->Start();
}

ScriptDocument::~ScriptDocument()
{
	AutoSaveTimer->Stop();

	Editor->ScriptModified -= EditorScriptModifiedHandler;
	Editor->LineAnchorInvalidated -= EditorLineAnchorInvalidatedHandler;
	BgAnalyzer->SemanticAnalysisComplete -= BgAnalysisCompleteHandler;
	preferences::SettingsHolder::Get()->SavedToDisk -= ScriptEditorPreferencesSavedHandler;
	AutoSaveTimer->Tick -= AutoSaveTimerTickHandler;

	SAFEDELETE_CLR(EditorScriptModifiedHandler);
	SAFEDELETE_CLR(EditorLineAnchorInvalidatedHandler);
	SAFEDELETE_CLR(BgAnalysisCompleteHandler);
	SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);
	SAFEDELETE_CLR(AutoSaveTimerTickHandler);

	SAFEDELETE_CLR(Editor);
	SAFEDELETE_CLR(BgAnalyzer);
	SAFEDELETE_CLR(IntelliSense);
	SAFEDELETE_CLR(AutoSaveTimer);

	Messages->Clear();
	Bookmarks->Clear();
	FindResults->Clear();
}

System::String^ ScriptDocument::PreprocessedScriptText::get()
{
	bool PreprocessResult = false, Throwaway;
	auto PreprocessedText = PreprocessScriptText(ScriptText, true, PreprocessResult, Throwaway);

	return PreprocessResult ? PreprocessedText : ScriptText;
}

void ScriptDocument::ScriptType::set(IScriptDocument::eScriptType v)
{
	Type= v;
	OnStateChangedType(v);
}

void ScriptDocument::Initialize(componentDLLInterface::ScriptData* ScriptData, bool UseAutoRecoveryFile)
{
	Debug::Assert(ScriptData != nullptr);

	EditorID = gcnew String(ScriptData->EditorID);
	FormID = ScriptData->FormID;
	Bytecode = reinterpret_cast<UInt8*>(ScriptData->ByteCode);
	BytecodeLength = ScriptData->Length;
	ScriptNativeObject = ScriptData->ParentForm;
	ScriptType = safe_cast<IScriptDocument::eScriptType>(ScriptData->Type);

	OnStateChangedEditorIdAndFormId(EditorID, FormID);
	OnStateChangedBytecodeLength(BytecodeLength);

	TextEditor->Modified = false;


	ClearMessages(ScriptDiagnosticMessage::eMessageSource::All, ScriptDiagnosticMessage::eMessageType::All);
	ClearBookmarks();
	ClearFindResults();

	auto RawScriptText = gcnew String(ScriptData->Text);
	if (UseAutoRecoveryFile)
		RawScriptText = LoadAutoRecoveryCache();

	String^ ExtractedScriptText = "";
	auto ExtractedMetadata = gcnew ScriptTextMetadata();
	ScriptTextMetadataHelper::DeserializeRawScriptText(RawScriptText, ExtractedScriptText, ExtractedMetadata);

	if (ExtractedMetadata->CaretPos > ExtractedScriptText->Length)
		ExtractedMetadata->CaretPos = ExtractedScriptText->Length;
	else if (ExtractedMetadata->CaretPos < 0)
		ExtractedMetadata->CaretPos = 0;

	TextEditor->InitializeState(ExtractedScriptText, ExtractedMetadata->CaretPos);

	BeginBatchUpdate(eBatchUpdateSource::Bookmarks);
	{
		for each (auto Itr in ExtractedMetadata->Bookmarks)
		{
			if (Itr->Line > 0 && Itr->Line <= TextEditor->LineCount)
				AddBookmark(Itr->Line, Itr->Text);
		}
	}
	EndBatchUpdate(eBatchUpdateSource::Bookmarks);

	intellisense::IntelliSenseBackend::Get()->Refresh(false);
}

bool ScriptDocument::Save(IScriptDocument::eSaveOperation SaveOperation, bool% OutHasWarning)
{
	if (ScriptNativeObject == nullptr)
		return false;

	bool Result = false;
	DisposibleDataAutoPtr<componentDLLInterface::ScriptCompileData> CompileInteropData(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData());
	auto Data = BeginScriptCompilation();
	{
		if (Data->CanCompile)
		{
			Data->CompileResult = CompileInteropData.get();

			if (SaveOperation == IScriptDocument::eSaveOperation::DontCompile)
				nativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(false);

			CString ScriptText(Data->PreprocessedScriptText->Replace("\n", "\r\n"));
			CompileInteropData->Script.Text = ScriptText.c_str();
			CompileInteropData->Script.Type = (int)Type;
			CompileInteropData->Script.ParentForm = (TESForm*)ScriptNativeObject;
			CompileInteropData->PrintErrorsToConsole = false;

			nativeWrapper::g_CSEInterfaceTable->ScriptEditor.CompileScript(CompileInteropData.get());
			Result = CompileInteropData->CompilationSuccessful;

			if (SaveOperation == IScriptDocument::eSaveOperation::DontCompile)
			{
				nativeWrapper::g_CSEInterfaceTable->ScriptEditor.ToggleScriptCompilation(true);
				nativeWrapper::g_CSEInterfaceTable->ScriptEditor.RemoveScriptBytecode(ScriptNativeObject);
			}
			else if (SaveOperation == IScriptDocument::eSaveOperation::AlsoSaveActiveFile)
				nativeWrapper::g_CSEInterfaceTable->EditorAPI.SaveActivePlugin();
		}
	}
	EndScriptCompilation(Data);
	OutHasWarning = Data->HasWarnings;

	if (Result)
		ClearAutoRecoveryCache();

	return Result;
}

List<ScriptDiagnosticMessage^>^ ScriptDocument::GetMessages(UInt32 Line, ScriptDiagnosticMessage::eMessageSource SourceFilter, ScriptDiagnosticMessage::eMessageType TypeFilter)
{
	auto OutMessages = gcnew List<ScriptDiagnosticMessage^>;

	for each (auto Itr in Messages)
	{
		if (Itr->Line == Line)
		{
			bool MatchedSource = SourceFilter == ScriptDiagnosticMessage::eMessageSource::All || Itr->Source == SourceFilter;
			bool MatchedType = TypeFilter == ScriptDiagnosticMessage::eMessageType::All || Itr->Type == TypeFilter;

			if (MatchedSource && MatchedType)
				OutMessages->Add(Itr);
		}
	}

	return OutMessages;
}

UInt32 ScriptDocument::GetErrorCount(UInt32 Line)
{
	UInt32 Count = 0;
	for each (auto Itr in Messages)
	{
		if ((Itr->Line == 0 || Itr->Line == Line) && Itr->Type == ScriptDiagnosticMessage::eMessageType::Error)
			++Count;
	}

	return Count;
}

UInt32 ScriptDocument::GetWarningCount(UInt32 Line)
{
	UInt32 Count = 0;
	for each (auto Itr in Messages)
	{
		if ((Itr->Line == 0 || Itr->Line == Line) && Itr->Type == ScriptDiagnosticMessage::eMessageType::Warning)
			++Count;
	}

	return Count;
}

void ScriptDocument::AddBookmark(UInt32 Line, String^ BookmarkText)
{
	if (Line > TextEditor->LineCount)
		Line = TextEditor->LineCount;

	if (LookupBookmark(Line, BookmarkText) != nullptr)
		return;

	auto LineAnchor = TextEditor->CreateLineAnchor(Line);
	auto NewBookmark= gcnew ScriptBookmark(LineAnchor, BookmarkText->Replace("\t", ""));
	Bookmarks->Add(NewBookmark);
	OnStateChangedBookmarks();
}

void ScriptDocument::RemoveBookmark(UInt32 Line, String^ BookmarkText)
{
	auto ToRemove = LookupBookmark(Line, BookmarkText);
	if (ToRemove)
	{
		Bookmarks->Remove(ToRemove);
		OnStateChangedBookmarks();
	}
}

void ScriptDocument::RemoveBookmark(ScriptBookmark^ Bookmark)
{
	RemoveBookmark(Bookmark->Line, Bookmark->Text);
}

List<ScriptBookmark^>^ ScriptDocument::GetBookmarks(UInt32 Line)
{
	auto OutBookmarks = gcnew List<ScriptBookmark^>;
	for each (auto Itr in Bookmarks)
	{
		if (Itr->Line == Line)
			OutBookmarks->Add(Itr);
	}

	return OutBookmarks;
}

UInt32 ScriptDocument::GetBookmarkCount(UInt32 Line)
{
	UInt32 Count = 0;
	for each (auto Itr in Bookmarks)
	{
		if (Itr->Line == Line)
			++Count;
	}

	return Count;
}

bool ScriptDocument::HasAutoRecoveryFile()
{
	return System::IO::File::Exists(GetAutoRecoveryCacheFilePath());
}

System::DateTime ScriptDocument::GetAutoRecoveryFileLastWriteTimestamp()
{
	if (HasAutoRecoveryFile())
		return System::IO::File::GetLastWriteTime(GetAutoRecoveryCacheFilePath());

	return DateTime::MaxValue;
}


void ScriptEditorDocumentModel::SetActiveScriptDocument(ScriptDocument^ New)
{
	if (SettingActiveScriptDocument)
	{
		// the only type of reentrant call we expect is one where the new active document is the same as the current
		// this can potentially arise if there is a loop in the event chain connecting the model and the view
		// only such cases can be safely ignored
		Debug::Assert(New == ActiveScriptDocument);
		return;
	}

	SettingActiveScriptDocument = true;
	auto EventArgs = gcnew IScriptEditorModel::ActiveDocumentChangedEventArgs(ActiveScriptDocument, New);

	ActiveScriptDocument = New;
	ActiveDocumentChanged(this, EventArgs);
	SettingActiveScriptDocument = false;
}

bool ScriptEditorDocumentModel::ValidateCommonActionPreconditions()
{
	if (ActiveDocument == nullptr)
		return false;
}

void ScriptEditorDocumentModel::ActiveAction_Copy()
{
	if (!ValidateCommonActionPreconditions())
		return;

	ActiveDocument->TextEditor->InvokeDefaultCopy();
}

void ScriptEditorDocumentModel::ActiveAction_Paste()
{
	if (!ValidateCommonActionPreconditions())
		return;

	ActiveDocument->TextEditor->InvokeDefaultPaste();
}

void ScriptEditorDocumentModel::ActiveAction_Comment()
{
	if (!ValidateCommonActionPreconditions())
		return;

	ActiveDocument->TextEditor->CommentSelection();
}

void ScriptEditorDocumentModel::ActiveAction_Uncomment()
{
	if (!ValidateCommonActionPreconditions())
		return;

	ActiveDocument->TextEditor->UncommentSelection();
}

void ScriptEditorDocumentModel::ActiveAction_AddBookmark(Object^ Params)
{
	if (!ValidateCommonActionPreconditions())
		return;

	auto ActionParam = safe_cast<IScriptEditorModel::ActiveDocumentActionCollection::AddBookmarkParams^>(Params);
	ActiveDocument->AddBookmark(ActiveDocument->TextEditor->CurrentLine, ActionParam->BookmarkDescription);
}

void ScriptEditorDocumentModel::ActiveAction_GoToLine(Object^ Params)
{
	if (!ValidateCommonActionPreconditions())
		return;

	auto ActionParam = safe_cast<IScriptEditorModel::ActiveDocumentActionCollection::GoToLineParams^>(Params);
	ActiveDocument->TextEditor->ScrollToLine(ActionParam->Line);
}


ScriptEditorDocumentModel::ScriptEditorDocumentModel()
{
	ScriptDocuments = gcnew List<ScriptDocument^>;
	ActiveScriptDocument = nullptr;
	SettingActiveScriptDocument = false;

	ActiveActions = gcnew IScriptEditorModel::ActiveDocumentActionCollection;
	ActiveActions->Copy->Delegate = gcnew ActionDelegate(this, &ScriptEditorDocumentModel::ActiveAction_Copy);
	ActiveActions->Paste->Delegate = gcnew ActionDelegate(this, &ScriptEditorDocumentModel::ActiveAction_Paste);
	ActiveActions->Comment->Delegate = gcnew ActionDelegate(this, &ScriptEditorDocumentModel::ActiveAction_Comment);
	ActiveActions->Uncomment->Delegate = gcnew ActionDelegate(this, &ScriptEditorDocumentModel::ActiveAction_Uncomment);
	ActiveActions->AddBookmark->Delegate = gcnew ParameterizedActionDelegate(this, &ScriptEditorDocumentModel::ActiveAction_AddBookmark);
	ActiveActions->GoToLine->Delegate = gcnew ParameterizedActionDelegate(this, &ScriptEditorDocumentModel::ActiveAction_GoToLine);
}

ScriptEditorDocumentModel::~ScriptEditorDocumentModel()
{
	for each (auto Itr in ScriptDocuments)
		delete Itr;

	ActiveScriptDocument = nullptr;
	ScriptDocuments->Clear();
	SAFEDELETE_CLR(ActiveActions);
}

IScriptDocument^ ScriptEditorDocumentModel::AllocateNewDocument()
{
	return gcnew ScriptDocument();
}

void ScriptEditorDocumentModel::AddDocument(IScriptDocument^ Document)
{
	if (ContainsDocument(Document))
		throw gcnew ArgumentException("Document is already part of the model");

	auto ScriptDoc = safe_cast<ScriptDocument^>(Document);
	ScriptDocuments->Add(ScriptDoc);
}

void ScriptEditorDocumentModel::RemoveDocument(IScriptDocument^ Document)
{
	if (!ContainsDocument(Document))
		throw gcnew ArgumentException("Document is not part of the model");

	auto ScriptDoc = safe_cast<ScriptDocument^>(Document);
	ScriptDocuments->Remove(ScriptDoc);
}

bool ScriptEditorDocumentModel::ContainsDocument(IScriptDocument^ Document)
{
	auto ScriptDoc = safe_cast<ScriptDocument^>(Document);

	for each (auto Itr in ScriptDocuments)
	{
		if (Itr == ScriptDoc)
			return true;
	}

	return false;
}

bool ScriptEditorDocumentModel::ContainsDocument(String^ EditorId)
{
	auto ScriptDoc = safe_cast<ScriptDocument^>(LookupDocument(EditorId));
	if (ScriptDoc == nullptr)
		return false;

	return ContainsDocument(ScriptDoc);
}

IScriptDocument^ ScriptEditorDocumentModel::LookupDocument(String^ EditorId)
{
	for each (auto Itr in ScriptDocuments)
	{
		if (Itr->ScriptEditorID == EditorId)
			return Itr;
	}

	return nullptr;
}

IScriptDocument^ ScriptEditorDocumentModel::ActiveDocument::get()
{
	return ActiveScriptDocument;
}

void ScriptEditorDocumentModel::ActiveDocument::set(IScriptDocument^ v)
{
	SetActiveScriptDocument(safe_cast<ScriptDocument^>(v));
}

} // namespace modelImpl


} // namespace scriptEditor


} // namespace cse