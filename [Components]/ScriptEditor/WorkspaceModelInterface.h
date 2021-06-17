#pragma once

#include "ITextEditor.h"
#include "RefactorTools.h"
#include "WorkspaceModelComponents.h"

namespace cse
{
	namespace scriptEditor
	{
		interface class IWorkspaceView;
		interface class IWorkspaceModelFactory;
		interface class IWorkspaceModelController;
		interface class IBackgroundSemanticAnalyzer;

		interface class IWorkspaceModel
		{
		public:
			static enum class ScriptType
			{
				Object = 0,
				Quest = 1,
				MagicEffect = 0x100
			};

			static enum class SaveOperation
			{
				Default,
				NoCompile,
				SavePlugin,
			};

			static enum class RefactorOperation
			{
				DocumentScript,
				RenameVariables,
				ModifyVariableIndices,
				CreateUDF,
			};

			ref struct StateChangeEventArgs
			{
				static enum class Type
				{
					None,
					Dirty,
					ByteCodeSize,
					ScriptType,
					Description,
					Messages,
					Bookmarks,
					FindResults
				};

				Type		EventType;

				bool		Dirty;
				UInt32		ByteCodeSize;		// in bytes
				ScriptType	ScriptType;
				String^		ShortDescription;
				String^		LongDescription;
				List<ScriptDiagnosticMessage^>^
							Messages;
				List<ScriptBookmark^>^
							Bookmarks;
				List<ScriptFindResult^>^
							FindResults;

				StateChangeEventArgs()
				{
					EventType = Type::None;

					Dirty = false;
					ByteCodeSize = 0;
					ScriptType = ScriptType::Object;
					ShortDescription = "";
					LongDescription = "";
					Messages = gcnew List<ScriptDiagnosticMessage^>;
					Bookmarks = gcnew List<ScriptBookmark^>;
					FindResults = gcnew List<ScriptFindResult^>;
				}
			};

			delegate void StateChangeEventHandler(IWorkspaceModel^ Sender, StateChangeEventArgs^ E);

			event StateChangeEventHandler^			StateChanged;

			property IWorkspaceModelFactory^		Factory;
			property IWorkspaceModelController^		Controller;

			property bool							Initialized;		// has a valid native script object
			property bool							New;				// is a new script
			property bool							Dirty;
			property ScriptType						Type;
			property String^						ShortDescription;	// EditorID
			property String^						LongDescription;	// EditorID [FormID]
			property IBackgroundSemanticAnalyzer^	BackgroundSemanticAnalyzer;

			property Control^						InternalView;		// the text editor
			property bool							Bound;
		};

		interface class IWorkspaceModelController
		{
		public:
			void	Bind(IWorkspaceModel^ Model, IWorkspaceView^ To);		// attaches to the view for display
			void	Unbind(IWorkspaceModel^ Model);							// detaches from the view

			void	SetText(IWorkspaceModel^ Model, String^ Text, bool ResetUndoStack);
			String^	GetText(IWorkspaceModel^ Model, bool Preprocess, bool% PreprocessResult, bool SuppressPreprocessorErrors);

			int		GetCaret(IWorkspaceModel^ Model);
			void	SetCaret(IWorkspaceModel^ Model, int Index);

			String^	GetSelection(IWorkspaceModel^ Model);
			String^	GetCaretToken(IWorkspaceModel^ Model);

			void	AcquireInputFocus(IWorkspaceModel^ Model);

			void	New(IWorkspaceModel^ Model);
			void	Open(IWorkspaceModel^ Model, componentDLLInterface::ScriptData* Data);
			bool	Save(IWorkspaceModel^ Model, IWorkspaceModel::SaveOperation Operation, bool% HasWarnings);
			bool	Close(IWorkspaceModel^ Model, bool% OperationCancelled);							// returns true if successful
			void	Next(IWorkspaceModel^ Model);
			void	Previous(IWorkspaceModel^ Model);
			void	CompileDepends(IWorkspaceModel^ Model);

			void	SetType(IWorkspaceModel^ Model, IWorkspaceModel::ScriptType New);
			void	GotoLine(IWorkspaceModel^ Model, UInt32 Line);
			UInt32	GetLineCount(IWorkspaceModel^ Model);
			bool	Sanitize(IWorkspaceModel^ Model);						// returns true if successful
			void	BindToForm(IWorkspaceModel^ Model);

			void	LoadFromDisk(IWorkspaceModel^ Model, String^ PathToFile);
			void	SaveToDisk(IWorkspaceModel^ Model, String^ PathToFile, bool PathIncludesFileName, String^ Extension);

			textEditor::ITextEditor::FindReplaceResult^
					FindReplace(IWorkspaceModel^ Model, textEditor::ITextEditor::eFindReplaceOperation Operation,
								String^ Query, String^ Replacement, textEditor::ITextEditor::FindReplaceOptions Options);


			bool	GetOffsetViewerData(IWorkspaceModel^ Model, String^% OutText, void** OutBytecode, UInt32% OutLength); // returns false if the operation's invalid (unsaved changes)
			bool	ApplyRefactor(IWorkspaceModel^ Model, IWorkspaceModel::RefactorOperation Operation, Object^ Arg); // returns true on success
			void	JumpToScript(IWorkspaceModel^ Model, String^ ScriptEditorID);

			void	AddMessage(IWorkspaceModel^ Model, UInt32 Line, String^ Text, ScriptDiagnosticMessage::MessageType Type, ScriptDiagnosticMessage::MessageSource Source);
			void	ClearMessages(IWorkspaceModel^ Model, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter);
			bool	GetMessages(IWorkspaceModel^ Model, UInt32 Line, ScriptDiagnosticMessage::MessageSource SourceFilter, ScriptDiagnosticMessage::MessageType TypeFilter, List<ScriptDiagnosticMessage^>^% OutMessages); // returns false when there are no messages
			UInt32	GetErrorCount(IWorkspaceModel^ Model, UInt32 Line);
			UInt32	GetWarningCount(IWorkspaceModel^ Model, UInt32 Line);
			void	BeginUpdateMessages(IWorkspaceModel^ Model);
			void	EndUpdateMessages(IWorkspaceModel^ Model);

			void	AddBookmark(IWorkspaceModel^ Model, UInt32 Line, String^ BookmarkText);
			void	RemoveBookmark(IWorkspaceModel^ Model, UInt32 Line, String^ BookmarkText);
			void	ClearBookmarks(IWorkspaceModel^ Model);
			bool	GetBookmarks(IWorkspaceModel^ Model, UInt32 Line, List<ScriptBookmark^>^% OutBookmarks);
			UInt32	GetBookmarkCount(IWorkspaceModel^ Model, UInt32 Line);
			void	BeginUpdateBookmarks(IWorkspaceModel^ Model);
			void	EndUpdateBookmarks(IWorkspaceModel^ Model);

			void	AddFindResult(IWorkspaceModel^ Model, UInt32 Line, String^ PreviewText, UInt32 Hits);
			void	ClearFindResults(IWorkspaceModel^ Model);
			void	BeginUpdateFindResults(IWorkspaceModel^ Model);
			void	EndUpdateFindResults(IWorkspaceModel^ Model);
		};

		interface class IWorkspaceModelFactory
		{
		public:
			IWorkspaceModel^				CreateModel(componentDLLInterface::ScriptData* Data);
			IWorkspaceModelController^		CreateController();
		};
	}
}