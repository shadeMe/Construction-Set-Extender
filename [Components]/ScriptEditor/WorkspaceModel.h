#pragma once

#include "WorkspaceModelInterface.h"
#include "WorkspaceViewInterface.h"
#include "BackgroundAnalysis.h"
#include "AvalonEditTextEditor.h"

#define NEWSCRIPTID					"New Script"
#define FIRSTRUNSCRIPTID			"New Workspace"

namespace cse
{
	namespace scriptEditor
	{
		ref class ConcreteWorkspaceModel;

		ref class ConcreteWorkspaceModelFactory : public IWorkspaceModelFactory
		{
			List<ConcreteWorkspaceModel^>^				Allocations;

			ConcreteWorkspaceModelFactory();
		public:
			~ConcreteWorkspaceModelFactory();

			property UInt32 	Count
			{
				virtual UInt32 get() { return Allocations->Count; }
				virtual void set(UInt32 e) {}
			}

			static ConcreteWorkspaceModelFactory^		Instance = gcnew ConcreteWorkspaceModelFactory;

			void 			Remove(ConcreteWorkspaceModel^ Allocation);

			// IWorkspaceModelFactory
			virtual IWorkspaceModel^				CreateModel(componentDLLInterface::ScriptData* Data);
			virtual IWorkspaceModelController^		CreateController();
		};

		ref class ConcreteWorkspaceModelController : public IWorkspaceModelController
		{
		public:
			virtual void	Bind(IWorkspaceModel^ Model, IWorkspaceView^ To);
			virtual void	Unbind(IWorkspaceModel^ Model);

			virtual void	SetText(IWorkspaceModel^ Model, String^ Text, bool ResetUndoStack);
			virtual String^	GetText(IWorkspaceModel^ Model, bool Preprocess, bool% PreprocessResult, bool SuppressPreprocessorErrors);

			virtual int		GetCaret(IWorkspaceModel^ Model);
			virtual void	SetCaret(IWorkspaceModel^ Model, int Index);

			virtual String^	GetSelection(IWorkspaceModel^ Model);
			virtual String^	GetCaretToken(IWorkspaceModel^ Model);

			virtual void	AcquireInputFocus(IWorkspaceModel^ Model);

			virtual void	New(IWorkspaceModel^ Model);
			virtual void	Open(IWorkspaceModel^ Model, componentDLLInterface::ScriptData* Data);
			virtual bool	Save(IWorkspaceModel^ Model, IWorkspaceModel::SaveOperation Operation, bool% HasWarnings);
			virtual bool	Close(IWorkspaceModel^ Model, bool% OperationCancelled);
			virtual void	Next(IWorkspaceModel^ Model);
			virtual void	Previous(IWorkspaceModel^ Model);
			virtual void	CompileDepends(IWorkspaceModel^ Model);

			virtual void	SetType(IWorkspaceModel^ Model, IWorkspaceModel::ScriptType New);
			virtual void	GotoLine(IWorkspaceModel^ Model, UInt32 Line);
			virtual UInt32	GetLineCount(IWorkspaceModel^ Model);
			virtual bool	Sanitize(IWorkspaceModel^ Model);
			virtual void	BindToForm(IWorkspaceModel^ Model);

			virtual void	LoadFromDisk(IWorkspaceModel^ Model, String^ PathToFile);
			virtual void	SaveToDisk(IWorkspaceModel^ Model, String^ PathToFile, bool PathIncludesFileName, String^ Extension);

			virtual textEditor::FindReplaceResult^
							FindReplace(IWorkspaceModel^ Model, textEditor::eFindReplaceOperation Operation,
										String^ Query, String^ Replacement, textEditor::eFindReplaceOptions Options);

			virtual bool	GetOffsetViewerData(IWorkspaceModel^ Model, String^% OutText, void** OutBytecode, UInt32% OutLength);
			virtual bool	ApplyRefactor(IWorkspaceModel^ Model, IWorkspaceModel::RefactorOperation Operation, Object^ Arg);
			virtual void	JumpToScript(IWorkspaceModel^ Model, String^ ScriptEditorID);

			virtual void	AddMessage(IWorkspaceModel^ Model, UInt32 Line, String^ Text, ScriptDiagnosticMessage::MessageType Type, ScriptDiagnosticMessage::MessageSource Source);
			virtual void	ClearMessages(IWorkspaceModel^ Model, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter);
			virtual bool	GetMessages(IWorkspaceModel^ Model, UInt32 Line, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter, List<ScriptDiagnosticMessage^>^% OutMessages);
			virtual UInt32	GetErrorCount(IWorkspaceModel^ Model, UInt32 Line);
			virtual UInt32	GetWarningCount(IWorkspaceModel^ Model, UInt32 Line);
			virtual void	BeginUpdateMessages(IWorkspaceModel^ Model);
			virtual void	EndUpdateMessages(IWorkspaceModel^ Model);

			virtual void	AddBookmark(IWorkspaceModel^ Model, UInt32 Line, String^ BookmarkText);
			virtual void	RemoveBookmark(IWorkspaceModel^ Model, UInt32 Line, String^ BookmarkText);
			virtual void	ClearBookmarks(IWorkspaceModel^ Model);
			virtual bool	GetBookmarks(IWorkspaceModel^ Model, UInt32 Line, List<ScriptBookmark^>^% OutBookmarks);
			virtual UInt32	GetBookmarkCount(IWorkspaceModel^ Model, UInt32 Line);
			virtual void	BeginUpdateBookmarks(IWorkspaceModel^ Model);
			virtual void	EndUpdateBookmarks(IWorkspaceModel^ Model);

			virtual void	AddFindResult(IWorkspaceModel^ Model, UInt32 Line, String^ PreviewText, UInt32 Hits);
			virtual void	ClearFindResults(IWorkspaceModel^ Model);
			virtual void	BeginUpdateFindResults(IWorkspaceModel^ Model);
			virtual void	EndUpdateFindResults(IWorkspaceModel^ Model);
		};

		ref class ConcreteWorkspaceModel : public IWorkspaceModel
		{
			KeyEventHandler^ TextEditorKeyDownHandler;
			EventHandler^	 AutoSaveTimerTickHandler;
			EventHandler^	 ScriptEditorPreferencesSavedHandler;
			textEditor::TextEditorScriptModifiedEventHandler^ TextEditorScriptModifiedHandler;
			textEditor::TextEditorMouseClickEventHandler^ TextEditorMouseClickHandler;
			EventHandler^	 TextEditorLineAnchorInvalidatedHandler;
			SemanticAnalysisCompleteEventHandler^
							BackgroundAnalyzerAnalysisCompleteHandler;

			void	TextEditor_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void	TextEditor_ScriptModified(Object^ Sender, textEditor::TextEditorScriptModifiedEventArgs^ E);
			void	TextEditor_MouseClick(Object^ Sender, textEditor::TextEditorMouseClickEventArgs^ E);
			void	TextEditor_LineAnchorInvalidated(Object^ Sender, EventArgs^ E);
			void	BackgroundAnalysis_AnalysisComplete(Object^ Sender, scriptEditor::SemanticAnalysisCompleteEventArgs^ E);
			void	ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);
			void	AutoSaveTimer_Tick(Object^ Sender, EventArgs^ E);

			void 	CheckAutoRecovery();
			void 	ClearAutoRecovery();
			String^ GetLineText(UInt32 Line);
			void 	ShowSyncedScriptWarning();

			void 	OnStateChangedDirty(bool Modified);
			void 	OnStateChangedByteCodeSize(UInt32 Size);
			void 	OnStateChangedType(IWorkspaceModel::ScriptType Type);
			void 	OnStateChangedDescription();
			void	OnStateChangedMessages();
			void	OnStateChangedBookmarks();
			void	OnStateChangedFindResults();

			String^ PreprocessScriptText(String^ ScriptText, bool SuppressErrors, bool% OutPreprocessResult, bool% OutContainsDirectives);
			void	TrackPreprocessorMessage(int Line, String^ Message);

			CompilationData^
					BeginScriptCompilation();
			void	EndScriptCompilation(CompilationData^ Data);

			void	InitializeTextEditor(String^ RawScriptText);
			ScriptBookmark^ LookupBookmark(UInt32 Line, String^ Text);
		public:
			textEditor::ITextEditor^
							TextEditor;
			intellisense::IIntelliSenseInterfaceModel^
							IntelliSenseModel;
			ScriptBackgroundAnalysis^
							BackgroundAnalysis;
			Timer^ 			AutoSaveTimer;

			void* 			ScriptNativeObject;
			IWorkspaceModel::ScriptType
							ScriptType;
			String^ 		EditorID;
			UInt32 			FormID;
			void* 			Bytecode;
			UInt32 			BytecodeLength;
			bool 			NewScriptFlag;
			bool			CompilationInProgress;

			List<ScriptDiagnosticMessage^>^
							Messages;
			List<ScriptBookmark^>^
							Bookmarks;
			List<ScriptFindResult^>^
							FindResults;

			IWorkspaceView^ BoundParent;
			ConcreteWorkspaceModelController^
							ModelController;
			ConcreteWorkspaceModelFactory^
							ModelFactory;

			static enum class BatchUpdateSource
			{
				None,
				Messages,
				Bookmarks,
				FindResults
			};

			BatchUpdateSource
							ActiveBatchUpdateSource;
			int				ActiveBatchUpdateSourceCounter;

			void InitializeState(componentDLLInterface::ScriptData* ScriptData, bool NewScript);

			bool PerformHouseKeeping();
			bool PerformHouseKeeping(bool% OperationCancelled);

			void Bind(IWorkspaceView^ To);
			void Unbind();

			void NewScript();
			void OpenScript(componentDLLInterface::ScriptData* Data);
			bool SaveScript(IWorkspaceModel::SaveOperation Operation, bool% HasWarnings);
			bool CloseScript(bool% OperationCancelled);
			void NextScript();
			void PreviousScript();

			String^ GetText(bool Preprocess, bool% PreprocessResult, bool SuppressPreprocessorErrors);
			void SetType(IWorkspaceModel::ScriptType New);
			bool Sanitize();

			void AddMessage(UInt32 Line, String^ Text, ScriptDiagnosticMessage::MessageType Type, ScriptDiagnosticMessage::MessageSource Source);
			void ClearMessages(ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter);
			bool GetMessages(UInt32 Line, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter, List<ScriptDiagnosticMessage^>^% OutMessages); // returns false when there are no messages
			UInt32 GetErrorCount(UInt32 Line);
			UInt32 GetWarningCount(UInt32 Line);

			void AddBookmark(UInt32 Line, String^ BookmarkText);
			void RemoveBookmark(UInt32 Line, String^ BookmarkText);
			void ClearBookmarks();
			bool GetBookmarks(UInt32 Line, List<ScriptBookmark^>^% OutBookmarks);
			UInt32 GetBookmarkCount(UInt32 Line);

			void AddFindResult(UInt32 Line, String^ PreviewText, UInt32 Hits);
			void ClearFindResults();

			void BeginBatchUpdate(BatchUpdateSource Source);
			void EndBatchUpdate(BatchUpdateSource Source);

			ConcreteWorkspaceModel(ConcreteWorkspaceModelController^ Controller,
								   ConcreteWorkspaceModelFactory^ Factory,
								   componentDLLInterface::ScriptData* Data);
			~ConcreteWorkspaceModel();

			virtual event IWorkspaceModel::StateChangeEventHandler^	StateChanged;

			virtual property IWorkspaceModelFactory^ Factory
			{
				IWorkspaceModelFactory^ get() { return ModelFactory; }
				void set(IWorkspaceModelFactory^ e) {}
			}
			virtual property IWorkspaceModelController^ Controller
			{
				IWorkspaceModelController^ get() { return ModelController; }
				void set(IWorkspaceModelController^ e) {}
			}
			virtual property bool Initialized
			{
				bool get() { return ScriptNativeObject != nullptr; }
				void set(bool e) {}
			}
			virtual property bool New
			{
				bool get() { return NewScriptFlag; }
				void set(bool e) {}
			}
			virtual property bool Dirty
			{
				bool get() { return TextEditor->Modified; }
				void set(bool e) {}
			}
			virtual property IWorkspaceModel::ScriptType	Type
			{
				IWorkspaceModel::ScriptType get() { return ScriptType; }
				void set(IWorkspaceModel::ScriptType e) {}
			}
			virtual property String^ ShortDescription
			{
				String^ get() { return EditorID; }
				void set(String^ e) {}
			}
			virtual property String^ LongDescription
			{
				String^ get() { return EditorID + " [" + FormID.ToString("X8") + "]"; }
				void set(String^ e) {}
			}
			virtual property IBackgroundSemanticAnalyzer^ BackgroundSemanticAnalyzer
			{
				IBackgroundSemanticAnalyzer^ get() { return BackgroundAnalysis; }
				void set(IBackgroundSemanticAnalyzer^ e) {}
			}
			virtual property Control^ InternalView
			{
				Control^ get() { return TextEditor->Container; }
				void set(Control^ e) {}
			}
			virtual property bool Bound
			{
				bool get() { return BoundParent != nullptr; }
				void set(bool e) {}
			}
		};
	}
}