#pragma once

#include "Macros.h"
#include "ScriptEditorModelImplComponents.h"
#include "IntelliSenseInterfaceModel.h"
#include "AvalonEditTextEditor.h"

namespace cse
{


namespace scriptEditor
{


namespace modelImpl
{


using namespace model;
using namespace model::components;
using namespace modelImpl::components;

ref class ScriptDocument : public IScriptDocument
{
	static enum class eBatchUpdateSource
	{
		None,
		Messages,
		Bookmarks,
		FindResults
	};

	IScriptDocument::eScriptType Type;
	String^ EditorID;
	UInt32 FormID;
	UInt8* Bytecode;
	UInt16 BytecodeLength;
	void* NativeObject;
	bool CompilationInProgress;

	List<ScriptDiagnosticMessage^>^ Messages;
	List<ScriptBookmark^>^ Bookmarks;
	List<ScriptFindResult^>^ FindResults;
	eBatchUpdateSource ActiveBatchUpdateSource;
	int ActiveBatchUpdateCounter;
	List<UInt16>^ LineBytecodeOffsets;
	List<Tuple<UInt32, String^>^>^ BookmarksBuffer;

	textEditor::avalonEdit::AvalonEditTextEditor^ Editor;
	intellisense::IntelliSenseInterfaceModel^ IntelliSense;
	BackgroundSemanticAnalyzer^ BgAnalyzer;
	NavigationHelper^ NavHelper;
	Timer^ AutoSaveTimer;

	textEditor::TextEditorScriptModifiedEventHandler^ EditorScriptModifiedHandler;
	EventHandler^ EditorLineAnchorInvalidatedHandler;
	EventHandler^ EditorStaticTextDisplayChangedHandler;
	EventHandler^ EditorLineColumnChangedHandler;
	IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler^ BgAnalysisCompleteHandler;
	EventHandler^ AutoSaveTimerTickHandler;
	EventHandler^ ScriptEditorPreferencesSavedHandler;

	void Editor_ScriptModified(Object^ Sender, textEditor::TextEditorScriptModifiedEventArgs^ E);
	void Editor_LineAnchorInvalidated(Object^ Sender, EventArgs^ E);
	void Editor_StaticTextDisplayChanged(Object^ Sender, EventArgs^ E);
	void Editor_LineColumnChanged(Object^ Sender, EventArgs^ E);
	void BgAnalyzer_AnalysisComplete(Object^ Sender, IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs^ E);
	void AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E);
	void ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

	void OnStateChangedDirty(bool Modified);
	void OnStateChangedBytecode(UInt8* Data, UInt16 Size, String^ PreprocessedScriptText);
	void OnStateChangedType(IScriptDocument::eScriptType Type);
	void OnStateChangedEditorIdAndFormId(String^ EditorId, UInt32 FormId);
	void OnStateChangedMessages();
	void OnStateChangedBookmarks();
	void OnStateChangedFindResults();
	void OnStateChangedDisplayingPreprocessorOutput();
	void OnStateChangedLineColumn();

	void ReleaseNativeObjectIfNewScript();

	String^ PreprocessScriptText(String^ ScriptText, bool SuppressErrors, bool% OutPreprocessResult, bool% OutContainsDirectives);
	void TrackPreprocessorMessage(int Line, String^ Message);

	ScriptCompilationData^ BeginScriptCompilation();
	void EndScriptCompilation(ScriptCompilationData^ Data);

	ScriptBookmark^ LookupBookmark(UInt32 Line, String^ Text);
	String^ GetPrettyName();

	void BeginBatchUpdate(eBatchUpdateSource Source);
	void EndBatchUpdate(eBatchUpdateSource Source);

	void AddMessage(UInt32 Line, String^ Text, ScriptDiagnosticMessage::eMessageType Type, ScriptDiagnosticMessage::eMessageSource Source);
	void ClearMessages(ScriptDiagnosticMessage::eMessageSource SourceFilter, ScriptDiagnosticMessage::eMessageType TypeFilter);

	void AddBookmark(UInt32 Line, String^ BookmarkText, bool MarkAsModified);
	void RemoveBookmark(UInt32 Line, String^ BookmarkText, bool MarkAsModified);
	void ClearBookmarks(bool MarkAsModified);

	void AddFindResult(String^ Query, UInt32 Line, String^ PreviewText, UInt32 Hits);
	void ClearFindResults();

	void SaveAutoRecoveryCache();

	ScriptTextMetadata^ PrepareMetadataForSerialization(bool HasPreprocessorDirectives);
	bool CalculateLineBytecodeOffsets(String^ PreprocessedScriptText, UInt8* BytecodeData, UInt16 BytecodeLength);
	bool AreLineBytecodeOffsetsValid();

	void CacheBookmarksAndReset();
	void RestoreCachedBookmarks();
public:
	ScriptDocument();
	virtual ~ScriptDocument();

	ImplPropertyGetOnly(bool, Valid, NativeObject != nullptr);
	ImplPropertySimple(bool, Dirty, Editor->Modified);
	property bool UnsavedNewScript
	{
		virtual bool get();
		ImplPropertySetInvalid(bool);
	}
	ImplPropertyGetOnly(String^, ScriptEditorID, EditorID);
	ImplPropertyGetOnly(UInt32, ScriptFormID, FormID);
	ImplPropertyGetOnly(String^, ScriptText, Editor->GetText());
	ImplPropertyGetOnly(UInt8*, ScriptBytecode, Bytecode);
	ImplPropertyGetOnly(UInt16, ScriptBytecodeLength, BytecodeLength);
	ImplPropertyGetOnly(void*, ScriptNativeObject, NativeObject);
	property String^ PreprocessedScriptText
	{
		virtual String^ get();
		ImplPropertySetInvalid(String^);
	}
	property IScriptDocument::eScriptType ScriptType
	{
		ImplPropertyGetSimple(IScriptDocument::eScriptType, Type);
		virtual void set(IScriptDocument::eScriptType v);
	}
	ImplPropertyGetOnly(textEditor::ITextEditor^, TextEditor, Editor);
	ImplPropertyGetOnly(intellisense::IIntelliSenseInterfaceModel^, IntelliSenseModel, IntelliSense);
	ImplPropertyGetOnly(IBackgroundSemanticAnalyzer^, BackgroundAnalyzer, BgAnalyzer);
	ImplPropertyGetOnly(INavigationHelper^, NavigationHelper, NavHelper);

	virtual event IScriptDocument::StateChangeEventHandler^ StateChanged;

	virtual void Initialize(componentDLLInterface::ScriptData* ScriptData, bool UseAutoRecoveryFile);
	virtual bool Save(IScriptDocument::eSaveOperation SaveOperation);

	virtual List<ScriptDiagnosticMessage^>^ GetMessages(UInt32 Line, ScriptDiagnosticMessage::eMessageSource SourceFilter, ScriptDiagnosticMessage::eMessageType TypeFilter);
	virtual UInt32 GetErrorCount(UInt32 Line);
	virtual UInt32 GetWarningCount(UInt32 Line);

	virtual void AddBookmark(UInt32 Line, String^ BookmarkText);
	virtual void RemoveBookmark(ScriptBookmark^ Bookmark);
	virtual List<ScriptBookmark^>^ GetBookmarks(UInt32 Line);
	virtual UInt32 GetBookmarkCount(UInt32 Line);

	virtual Dictionary<UInt32, IScriptDocument::PerLineAnnotationCounts^>^ CountAnnotationsForLineRange(UInt32 StartLine, UInt32 EndLine);

	virtual void PushStateToSubscribers();

	virtual bool TogglePreprocessorOutput(bool Enabled);
	virtual bool IsPreprocessorOutputEnabled();

	virtual bool SanitizeScriptText();
	virtual bool SaveScriptTextToDisk(String^ DiskFilePath);
	virtual bool LoadScriptTextFromDisk(String^ DiskFilePath);

	virtual textEditor::FindReplaceResult^ FindReplace(textEditor::eFindReplaceOperation Operation, String^ Query, String^ Replacement, textEditor::eFindReplaceOptions Options);

	virtual bool GetBytecodeOffsetForScriptLine(UInt32 Line, UInt16% OutOffset);
};


ref class ScriptEditorDocumentModel : public IScriptEditorModel
{
	List<ScriptDocument^>^ ScriptDocuments;
public:
	ScriptEditorDocumentModel();
	virtual ~ScriptEditorDocumentModel();

	property System::Collections::Generic::ICollection<IScriptDocument^>^ Documents
	{
		virtual System::Collections::Generic::ICollection<IScriptDocument^>^ get();
		ImplPropertySetInvalid(System::Collections::Generic::ICollection<IScriptDocument^>^);
	}

	virtual IScriptDocument^ AllocateNewDocument();
	virtual void AddDocument(IScriptDocument^ Document);
	virtual void RemoveDocument(IScriptDocument^ Document);
	virtual bool ContainsDocument(IScriptDocument^ Document);
	virtual bool ContainsDocument(String^ EditorId);
	virtual IScriptDocument^ LookupDocument(String^ EditorId);
};


ref struct ScriptEditorModelFactory : public model::IFactory
{
public:
	virtual model::IScriptEditorModel^ NewModel();

	static ScriptEditorModelFactory^ NewFactory();
};


} // namespace modelImpl


} // namespace scriptEditor


} // namespace cse