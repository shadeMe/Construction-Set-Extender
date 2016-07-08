#pragma once

#include "ScriptTextEditorInterface.h"
#include "RefactorTools.h"

namespace cse
{
	namespace scriptEditor
	{
		interface class IWorkspaceView;
		interface class IWorkspaceModelFactory;
		interface class IWorkspaceModelController;

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

			// common for all events, holds the updated state
			ref struct StateChangeEventArgs
			{
				bool								Dirty;
				UInt32								ByteCodeSize;		// in bytes
				ScriptType							Type;
				String^								ShortDescription;
				String^								LongDescription;

				StateChangeEventArgs() : Dirty(false), ByteCodeSize(0), Type(ScriptType::Object), ShortDescription(""), LongDescription("") {}
			};

			delegate void StateChangeEventHandler(IWorkspaceModel^ Sender, StateChangeEventArgs^ E);

			event StateChangeEventHandler^			StateChangedDirty;
			event StateChangeEventHandler^			StateChangedByteCodeSize;
			event StateChangeEventHandler^			StateChangedType;
			event StateChangeEventHandler^			StateChangedDescription;		// raised when either of the descriptions is changed

			property IWorkspaceModelFactory^		Factory;
			property IWorkspaceModelController^		Controller;

			property bool							Initialized;		// has a valid native script object
			property bool							New;				// is a new script
			property bool							Dirty;
			property ScriptType						Type;
			property String^						ShortDescription;	// EditorID
			property String^						LongDescription;	// EditorID [FormID]

			property Control^						InternalView;		// the text editor
			property bool							Bound;
		};

		interface class IWorkspaceModelController
		{
		public:
			void					Bind(IWorkspaceModel^ Model, IWorkspaceView^ To);		// attaches to the view for display
			void					Unbind(IWorkspaceModel^ Model);							// detaches from the view

			void					SetText(IWorkspaceModel^ Model, String^ Text, bool ResetUndoStack);
			String^					GetText(IWorkspaceModel^ Model, bool Preprocess, bool% PreprocessResult);

			int						GetCaret(IWorkspaceModel^ Model);
			void					SetCaret(IWorkspaceModel^ Model, int Index);

			String^					GetSelection(IWorkspaceModel^ Model);
			String^					GetCaretToken(IWorkspaceModel^ Model);

			void					AcquireInputFocus(IWorkspaceModel^ Model);

			void					New(IWorkspaceModel^ Model);
			void					Open(IWorkspaceModel^ Model, componentDLLInterface::ScriptData* Data);
			bool					Save(IWorkspaceModel^ Model, IWorkspaceModel::SaveOperation Operation, bool% HasWarnings);
			bool					Close(IWorkspaceModel^ Model, bool% OperationCancelled);							// returns true if successful
			void					Next(IWorkspaceModel^ Model);
			void					Previous(IWorkspaceModel^ Model);
			void					CompileDepends(IWorkspaceModel^ Model);

			void					SetType(IWorkspaceModel^ Model, IWorkspaceModel::ScriptType New);
			void					GotoLine(IWorkspaceModel^ Model, UInt32 Line);
			UInt32					GetLineCount(IWorkspaceModel^ Model);
			bool					Sanitize(IWorkspaceModel^ Model);						// returns true if successful
			void					BindToForm(IWorkspaceModel^ Model);

			void					LoadFromDisk(IWorkspaceModel^ Model, String^ PathToFile);
			void					SaveToDisk(IWorkspaceModel^ Model, String^ PathToFile, bool PathIncludesFileName, String^ Extension);

			textEditors::IScriptTextEditor::FindReplaceResult^
									FindReplace(IWorkspaceModel^ Model, textEditors::IScriptTextEditor::FindReplaceOperation Operation,
														String^ Query, String^ Replacement, textEditors::IScriptTextEditor::FindReplaceOptions Options);

											// returns false if the operation's invalid (unsaved changes)
			bool					GetOffsetViewerData(IWorkspaceModel^ Model, String^% OutText, UInt32% OutBytecode, UInt32% OutLength);

											// returns true on success
			bool					ApplyRefactor(IWorkspaceModel^ Model, IWorkspaceModel::RefactorOperation Operation, Object^ Arg);
		};

		interface class IWorkspaceModelFactory
		{
		public:
			IWorkspaceModel^				CreateModel(componentDLLInterface::ScriptData* Data);
			IWorkspaceModelController^		CreateController();
		};
	}
}