#include "ScriptEditorModelImpl.h"
#include "Preferences.h"
#include "IntelliSenseInterfaceModel.h"
#include "IntelliSenseBackend.h"
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

void ScriptDocument::Editor_StaticTextDisplayChanged(Object^ Sender, EventArgs^ E)
{
	OnStateChangedDisplayingPreprocessorOutput();
}

void ScriptDocument::Editor_LineColumnChanged(Object^ Sender, EventArgs^ E)
{
	OnStateChangedLineColumn();
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

void ScriptDocument::OnStateChangedBytecode(UInt8* Data, UInt16 Size, String^ PreprocessedScriptText)
{
	CalculateLineBytecodeOffsets(PreprocessedScriptText, Data, Size);

	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::Bytecode;
	E->BytecodeData = Data;
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

void ScriptDocument::OnStateChangedDisplayingPreprocessorOutput()
{
	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::DisplayingPreprocessorOutput;
	E->DisplayingPreprocessorOutput = TextEditor->DisplayingStaticText;
	StateChanged(this, E);
}

void ScriptDocument::OnStateChangedLineColumn()
{
	auto E = gcnew IScriptDocument::StateChangeEventArgs;
	E->EventType = IScriptDocument::StateChangeEventArgs::eEventType::LineOrColumn;
	E->Line = TextEditor->CurrentLine;
	E->Column = TextEditor->CurrentColumn;
	StateChanged(this, E);
}

void ScriptDocument::ReleaseNativeObjectIfNewScript()
{
	if (NativeObject == nullptr)
		return;
	else if (!nativeWrapper::g_CSEInterfaceTable->ScriptEditor.IsUnsavedNewScript(NativeObject))
		return;

	nativeWrapper::g_CSEInterfaceTable->ScriptEditor.DestroyScriptInstance(NativeObject);
	NativeObject = nullptr;
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
	ClearMessages(ScriptDiagnosticMessage::eMessageSource::Compiler, ScriptDiagnosticMessage::eMessageType::All);

	for each (auto Itr in SemanticAnalysisData->AnalysisMessages)
	{
		AddMessage(Itr->Line, Itr->Message,
				   Itr->Critical ? ScriptDiagnosticMessage::eMessageType::Error : ScriptDiagnosticMessage::eMessageType::Warning,
				   ScriptDiagnosticMessage::eMessageSource::Validator);
	}

	Result->CanCompile = !SemanticAnalysisData->HasCriticalMessages && !SemanticAnalysisData->MalformedStructure;

	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::FormData> ExistingFormWithScriptName(
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupFormByEditorID(CString(SemanticAnalysisData->Name).c_str()));
	if (ExistingFormWithScriptName)
	{
		bool SameAsCurrentScript = !String::Compare(EditorID, SemanticAnalysisData->Name, true);
		if (!SameAsCurrentScript)
		{
			Result->CanCompile = false;
			AddMessage(1, "Script name must be unique - '" + SemanticAnalysisData->Name + "' is assigned to an existing form",
					   ScriptDiagnosticMessage::eMessageType::Error, ScriptDiagnosticMessage::eMessageSource::Compiler);
		}
	}

	if (!Result->CanCompile)
		return Result;

	Result->PreprocessedScriptText = PreprocessScriptText(Result->UnpreprocessedScriptText, false, Result->CanCompile, Result->HasPreprocessorDirectives);
	if (!Result->CanCompile)
	{
		auto Metadata = PrepareMetadataForSerialization(Result->HasPreprocessorDirectives);
		Result->SerializedMetadata = ScriptTextMetadataHelper::SerializeMetadata(Metadata);
		ClearMessages(ScriptDiagnosticMessage::eMessageSource::Compiler, ScriptDiagnosticMessage::eMessageType::All);
	}

	// doesn't include compiler warnings for obvious reasons but it's okay since all compiler messages are errors
	Result->HasWarnings = GetMessageCountWarnings(0);
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
			OnStateChangedBytecode(Bytecode, BytecodeLength, PreprocessedScriptText);
			Dirty = false;
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

UInt32 ScriptDocument::GetMessageCount(UInt32 Line, ScriptDiagnosticMessage::eMessageType TypeFilter)
{
	UInt32 Count = 0;
	for each (auto Itr in Messages)
	{
		if (!Itr->Valid)
			continue;

		if ((Line == 0 || Itr->Line == Line) && Itr->Type == TypeFilter)
			++Count;
	}

	return Count;
}

void ScriptDocument::ClearBookmarks(bool MarkAsModified)
{
	Bookmarks->Clear();
	if (MarkAsModified)
		Dirty = true;
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


void ScriptDocument::SaveAutoRecoveryCache()
{
	auto Metadata = PrepareMetadataForSerialization(false);
	auto TextToSave = ScriptText + ScriptTextMetadataHelper::SerializeMetadata(Metadata);

	auto AutoRecoveryCache = gcnew model::components::ScriptTextAutoRecoveryCache(EditorID);
	AutoRecoveryCache->Write(TextToSave);
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

bool ScriptDocument::CalculateLineBytecodeOffsets(String^ PreprocessedScriptText, UInt8* BytecodeData, UInt16 BytecodeLength)
{
	try
	{
		LineBytecodeOffsets->Clear();
		LineBytecodeOffsets->Add(0);	// sentinel offset; will be set to 0xFFFF in case of an error

		if (BytecodeData)
		{
			UInt16 BytecodeCursor = 0;
			auto ScriptTextLines = PreprocessedScriptText->Split('\n');

			for each (auto Line in ScriptTextLines)
			{
				auto LineOffset = obScriptParsing::ByteCodeParser::GetOffsetForLine(Line, BytecodeData, BytecodeCursor);
				LineBytecodeOffsets->Add(LineOffset);
			}
		}
	}
	catch (...)
	{
		// exceptions can be raised when bytecode doesn't correspond to script text for whatever reason (external modification, etc)
		LineBytecodeOffsets[0] = 0xFFFF;
	}

	return AreLineBytecodeOffsetsValid();
}

bool ScriptDocument::AreLineBytecodeOffsetsValid()
{
	if (LineBytecodeOffsets->Count == 0)
		return false;
	else if (LineBytecodeOffsets[0] == 0xFFFF)
		return false;

	return true;
}

void ScriptDocument::CacheBookmarksAndReset()
{
	BookmarksBuffer->Clear();

	for each (auto Bookmark in Bookmarks)
	{
		Debug::Assert(Bookmark->Valid);
		BookmarksBuffer->Add(gcnew Tuple<UInt32, String^>(Bookmark->Line, Bookmark->Text));
	}

	ClearBookmarks(false);
}

void ScriptDocument::RestoreCachedBookmarks()
{
	BeginBatchUpdate(eBatchUpdateSource::Bookmarks);
	{
		Debug::Assert(Bookmarks->Count == 0);

		for each (auto Bookmark in BookmarksBuffer)
			AddBookmark(Bookmark->Item1, Bookmark->Item2, false);
	}
	EndBatchUpdate(eBatchUpdateSource::Bookmarks);

	BookmarksBuffer->Clear();
}

ScriptDocument::ScriptDocument()
{
	Type = IScriptDocument::eScriptType::Object;
	EditorID = String::Empty;
	FormID = 0;
	Bytecode = nullptr;
	BytecodeLength = 0;
	NativeObject = nullptr;
	CompilationInProgress = false;

	Messages = gcnew List<ScriptDiagnosticMessage^>;
	Bookmarks = gcnew List<ScriptBookmark^>;
	FindResults = gcnew List<ScriptFindResult^>;
	ActiveBatchUpdateSource = eBatchUpdateSource::None;
	ActiveBatchUpdateCounter = 0;
	LineBytecodeOffsets = gcnew List<UInt16>;
	BookmarksBuffer = gcnew List<Tuple<UInt32, String^>^>;

	BgAnalyzer = gcnew BackgroundSemanticAnalyzer(this);
	Editor = gcnew textEditor::avalonEdit::AvalonEditTextEditor(this);
	IntelliSense = gcnew intellisense::IntelliSenseInterfaceModel(Editor);
	NavHelper = gcnew components::NavigationHelper(this);

	AutoSaveTimer = gcnew Timer();
	AutoSaveTimer->Interval = preferences::SettingsHolder::Get()->Backup->AutoRecoveryInterval * 1000 * 60;

	EditorScriptModifiedHandler = gcnew textEditor::TextEditorScriptModifiedEventHandler(this, &ScriptDocument::Editor_ScriptModified);
	EditorLineAnchorInvalidatedHandler = gcnew EventHandler(this, &ScriptDocument::Editor_LineAnchorInvalidated);
	EditorStaticTextDisplayChangedHandler = gcnew EventHandler(this, &ScriptDocument::Editor_StaticTextDisplayChanged);
	EditorLineColumnChangedHandler = gcnew EventHandler(this, &ScriptDocument::Editor_LineColumnChanged);
	BgAnalysisCompleteHandler = gcnew IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler(this, &ScriptDocument::BgAnalyzer_AnalysisComplete);
	AutoSaveTimerTickHandler = gcnew EventHandler(this, &ScriptDocument::AutoSaveTimer_Tick);
	ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ScriptDocument::ScriptEditorPreferences_Saved);

	Editor->ScriptModified += EditorScriptModifiedHandler;
	Editor->LineAnchorInvalidated += EditorLineAnchorInvalidatedHandler;
	Editor->StaticTextDisplayChanged += EditorStaticTextDisplayChangedHandler;
	Editor->LineChanged += EditorLineColumnChangedHandler;
	Editor->ColumnChanged += EditorLineColumnChangedHandler;
	BgAnalyzer->SemanticAnalysisComplete += BgAnalysisCompleteHandler;
	preferences::SettingsHolder::Get()->PreferencesChanged += ScriptEditorPreferencesSavedHandler;
	AutoSaveTimer->Tick += AutoSaveTimerTickHandler;

	AutoSaveTimer->Start();
}

ScriptDocument::~ScriptDocument()
{
	ReleaseNativeObjectIfNewScript();

	AutoSaveTimer->Stop();

	Editor->ScriptModified -= EditorScriptModifiedHandler;
	Editor->LineAnchorInvalidated -= EditorLineAnchorInvalidatedHandler;
	Editor->StaticTextDisplayChanged -= EditorStaticTextDisplayChangedHandler;
	Editor->LineChanged -= EditorLineColumnChangedHandler;
	Editor->ColumnChanged -= EditorLineColumnChangedHandler;
	BgAnalyzer->SemanticAnalysisComplete -= BgAnalysisCompleteHandler;
	preferences::SettingsHolder::Get()->PreferencesChanged -= ScriptEditorPreferencesSavedHandler;
	AutoSaveTimer->Tick -= AutoSaveTimerTickHandler;

	SAFEDELETE_CLR(EditorScriptModifiedHandler);
	SAFEDELETE_CLR(EditorLineAnchorInvalidatedHandler);
	SAFEDELETE_CLR(EditorStaticTextDisplayChangedHandler);
	SAFEDELETE_CLR(EditorLineColumnChangedHandler);
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


bool ScriptDocument::UnsavedNewScript::get()
{
	return NativeObject && nativeWrapper::g_CSEInterfaceTable->ScriptEditor.IsUnsavedNewScript(NativeObject);
}

System::String^ ScriptDocument::PreprocessedScriptText::get()
{
	bool PreprocessResult = false, Throwaway;
	auto PreprocessedText = PreprocessScriptText(ScriptText, true, PreprocessResult, Throwaway);

	return PreprocessResult ? PreprocessedText : ScriptText;
}

void ScriptDocument::ScriptType::set(IScriptDocument::eScriptType v)
{
	if (Type == v)
		return;

	Type = v;
	OnStateChangedType(v);
	Dirty = true;
}

void ScriptDocument::Initialize(componentDLLInterface::ScriptData* ScriptData, bool UseAutoRecoveryFile)
{
	Debug::Assert(ScriptData != nullptr);
	Debug::Assert(!TextEditor->DisplayingStaticText);

	// release the currently bound native object if it points to an unsaved new script instance
	ReleaseNativeObjectIfNewScript();

	EditorID = gcnew String(ScriptData->EditorID);
	FormID = ScriptData->FormID;
	Bytecode = reinterpret_cast<UInt8*>(ScriptData->ByteCode);
	BytecodeLength = ScriptData->Length;
	NativeObject = ScriptData->ParentForm;
	ScriptType = safe_cast<IScriptDocument::eScriptType>(ScriptData->Type);

	ClearMessages(ScriptDiagnosticMessage::eMessageSource::All, ScriptDiagnosticMessage::eMessageType::All);
	ClearBookmarks(false);
	ClearFindResults();

	auto RawScriptText = gcnew String(ScriptData->Text);
	if (UseAutoRecoveryFile)
	{
		Debug::Assert(!UnsavedNewScript);

		auto AutoRecoveryFile = gcnew ScriptTextAutoRecoveryCache(EditorID);
		RawScriptText = AutoRecoveryFile->Read();
		AutoRecoveryFile->Delete(false);

	}

	String^ ExtractedScriptText = "";
	auto ExtractedMetadata = gcnew ScriptTextMetadata();
	ScriptTextMetadataHelper::DeserializeRawScriptText(RawScriptText, ExtractedScriptText, ExtractedMetadata);

	if (ExtractedMetadata->CaretPos > ExtractedScriptText->Length)
		ExtractedMetadata->CaretPos = ExtractedScriptText->Length;
	else if (ExtractedMetadata->CaretPos < 0)
		ExtractedMetadata->CaretPos = 0;

	TextEditor->InitializeState(ExtractedScriptText, ExtractedMetadata->CaretPos);

	OnStateChangedEditorIdAndFormId(EditorID, FormID);
	OnStateChangedBytecode(Bytecode, BytecodeLength, PreprocessedScriptText);

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
	Dirty = UnsavedNewScript || UseAutoRecoveryFile ? true : false;
}

bool ScriptDocument::Save(IScriptDocument::eSaveOperation SaveOperation)
{
	Debug::Assert(ScriptNativeObject != nullptr);

	bool Result = false;
	nativeWrapper::DisposibleDataAutoPtr<componentDLLInterface::ScriptCompileData> CompileInteropData(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.AllocateCompileData());
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

	if (Result)
	{
		auto AutoRecoveryCache = gcnew model::components::ScriptTextAutoRecoveryCache(EditorID);
		AutoRecoveryCache->Delete(false);
	}

	return Result;
}

List<ScriptDiagnosticMessage^>^ ScriptDocument::GetMessages(UInt32 Line, ScriptDiagnosticMessage::eMessageSource SourceFilter, ScriptDiagnosticMessage::eMessageType TypeFilter)
{
	auto OutMessages = gcnew List<ScriptDiagnosticMessage^>;

	for each (auto Itr in Messages)
	{
		if (!Itr->Valid)
			continue;

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

UInt32 ScriptDocument::GetMessageCountErrors(UInt32 Line)
{
	return GetMessageCount(Line, ScriptDiagnosticMessage::eMessageType::Error);
}

UInt32 ScriptDocument::GetMessageCountWarnings(UInt32 Line)
{
	return GetMessageCount(Line, ScriptDiagnosticMessage::eMessageType::Warning);
}

UInt32 ScriptDocument::GetMessageCountInfos(UInt32 Line)
{
	return GetMessageCount(Line, ScriptDiagnosticMessage::eMessageType::Info);
}

void ScriptDocument::AddBookmark(UInt32 Line, String^ BookmarkText)
{
	AddBookmark(Line, BookmarkText, true);
}

void ScriptDocument::AddBookmark(UInt32 Line, String^ BookmarkText, bool MarkAsModified)
{
	if (Line > TextEditor->LineCount)
		Line = TextEditor->LineCount;

	if (LookupBookmark(Line, BookmarkText) != nullptr)
		return;

	auto LineAnchor = TextEditor->CreateLineAnchor(Line);
	auto NewBookmark= gcnew ScriptBookmark(LineAnchor, BookmarkText->Replace("\t", ""));
	Bookmarks->Add(NewBookmark);

	if (MarkAsModified)
		Dirty = true;

	OnStateChangedBookmarks();
}

void ScriptDocument::RemoveBookmark(UInt32 Line, String^ BookmarkText, bool MarkAsModified)
{
	auto ToRemove = LookupBookmark(Line, BookmarkText);
	if (ToRemove)
	{
		Bookmarks->Remove(ToRemove);
		if (MarkAsModified)
			Dirty = true;

		OnStateChangedBookmarks();
	}
}

void ScriptDocument::RemoveBookmark(ScriptBookmark^ Bookmark)
{
	RemoveBookmark(Bookmark->Line, Bookmark->Text, true);
}

List<ScriptBookmark^>^ ScriptDocument::GetBookmarks(UInt32 Line)
{
	auto OutBookmarks = gcnew List<ScriptBookmark^>;
	for each (auto Itr in Bookmarks)
	{
		if (!Itr->Valid)
			continue;

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
		if (!Itr->Valid)
			continue;

		if (Itr->Line == Line)
			++Count;
	}

	return Count;
}

Dictionary<UInt32, IScriptDocument::PerLineAnnotationCounts^>^ ScriptDocument::CountAnnotationsForLineRange(UInt32 StartLine, UInt32 EndLine)
{
	auto Counts = gcnew Dictionary<UInt32, IScriptDocument::PerLineAnnotationCounts^>;
	for (int i = StartLine; i <= EndLine; ++i)
		Counts->Add(i, gcnew IScriptDocument::PerLineAnnotationCounts);

	for each (auto Itr in Messages)
	{
		if (!Itr->Valid)
			continue;

		auto MessageLine = Itr->Line;
		if (MessageLine >= StartLine && MessageLine <= EndLine)
		{
			auto LineCounts = Counts[MessageLine];
			if (Itr->Type == ScriptDiagnosticMessage::eMessageType::Error)
				LineCounts->ErrorCount += 1;
			else if (Itr->Type == ScriptDiagnosticMessage::eMessageType::Warning)
				LineCounts->WarningCount += 1;
		}
	}

	for each (auto Itr in Bookmarks)
	{
		if (!Itr->Valid)
			continue;

		auto BookMarkLine = Itr->Line;
		if (BookMarkLine >= StartLine && BookMarkLine <= EndLine)
			Counts[BookMarkLine]->BookmarkCount += 1;
	}

	return Counts;
}

void ScriptDocument::PushStateToSubscribers()
{
	OnStateChangedDirty(Dirty);
	OnStateChangedBytecode(Bytecode, BytecodeLength, PreprocessedScriptText);
	OnStateChangedType(Type);
	OnStateChangedEditorIdAndFormId(EditorID, FormID);
	OnStateChangedMessages();
	OnStateChangedBookmarks();
	OnStateChangedFindResults();
	OnStateChangedDisplayingPreprocessorOutput();
	OnStateChangedLineColumn();
}

bool ScriptDocument::TogglePreprocessorOutput(bool Enabled)
{
	if (Enabled && Dirty)
		return false;

	Debug::Assert(TextEditor->DisplayingStaticText == !Enabled);

	if (Enabled)
	{
		// cache bookmarks beforehand to prevent their invalidation
		// due to the deletion of their text anchors
		CacheBookmarksAndReset();
		TextEditor->BeginDisplayingStaticText(PreprocessedScriptText);
	}
	else
	{
		TextEditor->EndDisplayingStaticText();
		RestoreCachedBookmarks();
	}

	Editor->ToggleScriptBytecodeOffsetMargin(Enabled);
	return true;
}

bool ScriptDocument::IsPreprocessorOutputEnabled()
{
	return TextEditor->DisplayingStaticText;
}

String^ GetSanitizedIdentifier(String^ Identifier)
{
	return intellisense::IntelliSenseBackend::Get()->SanitizeIdentifier(Identifier);
}

bool ScriptDocument::SanitizeScriptText()
{
	auto Agent = gcnew obScriptParsing::Sanitizer(TextEditor->GetText());
	obScriptParsing::Sanitizer::eOperation Operation;

	if (preferences::SettingsHolder::Get()->Sanitizer->NormalizeIdentifiers)
		Operation = Operation | obScriptParsing::Sanitizer::eOperation::AnnealCasing;

	if (preferences::SettingsHolder::Get()->Sanitizer->PrefixIfElseIfWithEval)
		Operation = Operation | obScriptParsing::Sanitizer::eOperation::EvalifyIfs;

	if (preferences::SettingsHolder::Get()->Sanitizer->ApplyCompilerOverride)
		Operation = Operation | obScriptParsing::Sanitizer::eOperation::CompilerOverrideBlocks;

	if (preferences::SettingsHolder::Get()->Sanitizer->IndentLines)
		Operation = Operation | obScriptParsing::Sanitizer::eOperation::IndentLines;

	bool Result = Agent->SanitizeScriptText(Operation, gcnew obScriptParsing::Sanitizer::GetSanitizedIdentifier(GetSanitizedIdentifier));
	if (Result)
	{
		CacheBookmarksAndReset();
		TextEditor->SetText(Agent->Output, false);
		RestoreCachedBookmarks();
	}

	return Result;
}

bool ScriptDocument::SaveScriptTextToDisk(String^ DiskFilePath)
{
	try
	{
		File::WriteAllText(DiskFilePath, ScriptText, System::Text::Encoding::UTF8);
		return true;
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't save script text to file @ " + DiskFilePath + "!\nException: " + E->ToString());
		return false;
	}
}

bool ScriptDocument::LoadScriptTextFromDisk(String^ DiskFilePath)
{
	try
	{
		auto FileContents = File::ReadAllText(DiskFilePath);
		TextEditor->SetText(FileContents, true);
		return true;
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't load script text from file @ " + DiskFilePath + "!\nException: " + E->ToString());
		return false;
	}
}

textEditor::FindReplaceResult^ ScriptDocument::FindReplace(textEditor::eFindReplaceOperation Operation, String^ Query, String^ Replacement, textEditor::eFindReplaceOptions Options)
{
	auto Result = TextEditor->FindReplace(Operation, Query, Replacement, Options);
	if (!Result->HasError)
	{
		BeginBatchUpdate(eBatchUpdateSource::FindResults);
		{
			ClearFindResults();
			auto OpQuery = Operation == textEditor::eFindReplaceOperation::Replace ? Query + " => " + Replacement : Query;
			for each (auto Hit in Result->Hits)
				AddFindResult(OpQuery, Hit->Line, Hit->Text, Hit->HitCount);
		}
		EndBatchUpdate(eBatchUpdateSource::FindResults);
	}

	return Result;
}

bool ScriptDocument::GetBytecodeOffsetForScriptLine(UInt32 Line, UInt16% OutOffset)
{
	if (!AreLineBytecodeOffsetsValid())
		return false;
	else if (Line < 1 || Line >= LineBytecodeOffsets->Count)
		return false;

	OutOffset = LineBytecodeOffsets[Line];
	return true;
}

ScriptEditorDocumentModel::ScriptEditorDocumentModel()
{
	ScriptDocuments = gcnew List<ScriptDocument^>;
}

ScriptEditorDocumentModel::~ScriptEditorDocumentModel()
{
	for each (auto Itr in ScriptDocuments)
		delete Itr;

	ScriptDocuments->Clear();
}

System::Collections::Generic::ICollection<IScriptDocument^>^ ScriptEditorDocumentModel::Documents::get()
{
	auto Out = gcnew List<IScriptDocument^>;
	for each (auto Itr in ScriptDocuments)
		Out->Add(Itr);
	return Out;
}

IScriptDocument^ ScriptEditorDocumentModel::AllocateNewDocument()
{
	return gcnew ScriptDocument();
}

void ScriptEditorDocumentModel::AddDocument(IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (ContainsDocument(Document))
		throw gcnew ArgumentException("Document is already part of the model");

	auto ScriptDoc = safe_cast<ScriptDocument^>(Document);
	ScriptDocuments->Add(ScriptDoc);
}

void ScriptEditorDocumentModel::RemoveDocument(IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);

	if (!ContainsDocument(Document))
		throw gcnew ArgumentException("Document is not part of the model");

	auto ScriptDoc = safe_cast<ScriptDocument^>(Document);
	ScriptDocuments->Remove(ScriptDoc);
}

bool ScriptEditorDocumentModel::ContainsDocument(IScriptDocument^ Document)
{
	Debug::Assert(Document != nullptr);
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

model::IScriptEditorModel^ ScriptEditorModelFactory::NewModel()
{
	return gcnew modelImpl::ScriptEditorDocumentModel;
}

ScriptEditorModelFactory^ ScriptEditorModelFactory::NewFactory()
{
	return gcnew ScriptEditorModelFactory;
}


} // namespace modelImpl


} // namespace scriptEditor


} // namespace cse