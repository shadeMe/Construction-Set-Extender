#pragma once
#include "Macros.h"
#include "ScriptEditorModelImplComponents.h"
#include "IntelliSenseInterfaceModel.h"
#include "ITextEditor.h"


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

	textEditor::ITextEditor^ Editor;
	intellisense::IntelliSenseInterfaceModel^ IntelliSense;
	BackgroundSemanticAnalyzer^ BgAnalyzer;
	NavigationHelper^ NavHelper;
	Timer^ AutoSaveTimer;

	textEditor::TextEditorScriptModifiedEventHandler^ EditorScriptModifiedHandler;
	EventHandler^ EditorLineAnchorInvalidatedHandler;
	IBackgroundSemanticAnalyzer::AnalysisCompleteEventHandler^ BgAnalysisCompleteHandler;
	EventHandler^ AutoSaveTimerTickHandler;
	EventHandler^ ScriptEditorPreferencesSavedHandler;

	void Editor_ScriptModified(Object^ Sender, textEditor::TextEditorScriptModifiedEventArgs^ E);
	void Editor_LineAnchorInvalidated(Object^ Sender, EventArgs^ E);
	void BgAnalyzer_AnalysisComplete(Object^ Sender, IBackgroundSemanticAnalyzer::AnalysisCompleteEventArgs^ E);
	void AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E);
	void ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

	void OnStateChangedDirty(bool Modified);
	void OnStateChangedBytecodeLength(UInt16 Size);
	void OnStateChangedType(IScriptDocument::eScriptType Type);
	void OnStateChangedEditorIdAndFormId(String^ EditorId, UInt32 FormId);
	void OnStateChangedMessages();
	void OnStateChangedBookmarks();
	void OnStateChangedFindResults();

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

	void ClearBookmarks();

	void AddFindResult(String^ Query, UInt32 Line, String^ PreviewText, UInt32 Hits);
	void ClearFindResults();

	String^ LoadAutoRecoveryCache();
	void SaveAutoRecoveryCache();
	String^ GetAutoRecoveryCacheFilePath();
	void ClearAutoRecoveryCache();

	ScriptTextMetadata^ PrepareMetadataForSerialization(bool HasPreprocessorDirectives);
public:
	ScriptDocument();
	virtual ~ScriptDocument();

	ImplPropertyGetOnly(bool, Valid, NativeObject != nullptr);
	ImplPropertyGetOnly(bool, Dirty, Editor->Modified);
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
	virtual bool Save(IScriptDocument::eSaveOperation SaveOperation, bool% OutHasWarning);

	virtual List<ScriptDiagnosticMessage^>^ GetMessages(UInt32 Line, ScriptDiagnosticMessage::eMessageSource SourceFilter, ScriptDiagnosticMessage::eMessageType TypeFilter);
	virtual UInt32 GetErrorCount(UInt32 Line);
	virtual UInt32 GetWarningCount(UInt32 Line);

	virtual void AddBookmark(UInt32 Line, String^ BookmarkText);
	virtual void RemoveBookmark(UInt32 Line, String^ BookmarkText);
	virtual void RemoveBookmark(ScriptBookmark^ Bookmark);
	virtual List<ScriptBookmark^>^ GetBookmarks(UInt32 Line);
	virtual UInt32 GetBookmarkCount(UInt32 Line);

	virtual bool HasAutoRecoveryFile();
	virtual DateTime GetAutoRecoveryFileLastWriteTimestamp();
};


ref class ScriptEditorDocumentModel : public IScriptEditorModel
{
	List<ScriptDocument^>^ ScriptDocuments;
	ScriptDocument^ ActiveScriptDocument;
	IScriptEditorModel::ActiveDocumentActionCollection^ ActiveActions;
	bool SettingActiveScriptDocument;

	void SetActiveScriptDocument(ScriptDocument^ New);

	bool ValidateCommonActionPreconditions();
	void ActiveAction_Copy();
	void ActiveAction_Paste();
	void ActiveAction_Comment();
	void ActiveAction_Uncomment();
	void ActiveAction_AddBookmark(Object^ Params);
	void ActiveAction_GoToLine(Object^ Params);
public:
	ScriptEditorDocumentModel();
	virtual ~ScriptEditorDocumentModel();

	property List<IScriptDocument^>^ Documents
	{
		virtual List<IScriptDocument^>^ get();
		ImplPropertySetInvalid(List<IScriptDocument^>^);
	}
	property IScriptDocument^ ActiveDocument
	{
		virtual IScriptDocument^ get();
		virtual void set(IScriptDocument^ v);
	}
	ImplPropertyGetOnly(IScriptEditorModel::ActiveDocumentActionCollection^, ActiveDocumentActions, ActiveActions);

	virtual event IScriptEditorModel::ActiveDocumentChangedEventHandler^ ActiveDocumentChanged;

	virtual IScriptDocument^ AllocateNewDocument();
	virtual void AddDocument(IScriptDocument^ Document);
	virtual void RemoveDocument(IScriptDocument^ Document);
	virtual bool ContainsDocument(IScriptDocument^ Document);
	virtual bool ContainsDocument(String^ EditorId);
	virtual IScriptDocument^ LookupDocument(String^ EditorId);
};


} // namespace modelImpl


} // namespace scriptEditor


} // namespace cse