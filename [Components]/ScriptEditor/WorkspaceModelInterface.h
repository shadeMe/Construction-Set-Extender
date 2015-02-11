#pragma once

#include "ScriptTextEditorInterface.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		interface class IWorkspaceView;
		interface class IWorkspaceViewController;

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

			property IWorkspaceModelFactory^		Factory;
			property IWorkspaceModelController^		Controller;

			property bool							Initialized;		// has a valid native script object
			property bool							New;				// is a new script
			property ScriptType						Type;
			property String^						Description;		// EditorID [FormID]

			property Control^						InternalView;		// the text editor
			property bool							Bound;
		};

		interface class IWorkspaceModelController
		{
		public:
			virtual void					Bind(IWorkspaceModel^ Model, IWorkspaceView^ To);		// attaches to the view for display
			virtual void					Unbind(IWorkspaceModel^ Model);							// detaches from the view

			virtual void					SetText(IWorkspaceModel^ Model, String^ Text, bool ResetUndoStack);
			virtual String^					GetText(IWorkspaceModel^ Model, bool Preprocess, bool% PreprocessResult);

			virtual int						GetCaret(IWorkspaceModel^ Model);
			virtual void					SetCaret(IWorkspaceModel^ Model, int Index);

			virtual void					AcquireInputFocus(IWorkspaceModel^ Model);

			virtual void					New(IWorkspaceModel^ Model);
			virtual void					Open(IWorkspaceModel^ Model, ComponentDLLInterface::ScriptData* Data);
			virtual bool					Save(IWorkspaceModel^ Model, IWorkspaceModel::SaveOperation Operation);
			virtual bool					Close(IWorkspaceModel^ Model);							// returns true if successful
			virtual void					Next(IWorkspaceModel^ Model);
			virtual void					Previous(IWorkspaceModel^ Model);
			virtual void					CompileDepends(IWorkspaceModel^ Model);

			virtual void					SetType(IWorkspaceModel^ Model, IWorkspaceModel::ScriptType New);
			virtual void					GotoLine(IWorkspaceModel^ Model, UInt32 Line);
			virtual UInt32					GetLineCount(IWorkspaceModel^ Model);
			virtual bool					Sanitize(IWorkspaceModel^ Model);						// returns true if successful
			virtual void					BindToForm(IWorkspaceModel^ Model);

			virtual void					LoadFromDisk(IWorkspaceModel^ Model, String^ PathToFile);
			virtual void					SaveToDisk(IWorkspaceModel^ Model, String^ PathToFile, bool PathIncludesFileName, String^ Extension);

			virtual int						FindReplace(IWorkspaceModel^ Model, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
														String^ Query, String^ Replacement, UInt32 Options);

											// returns false if the operation's invalid (unsaved changes)
			virtual bool					GetOffsetViewerData(IWorkspaceModel^ Model, String^% OutText, UInt32% OutBytecode, UInt32% OutLength);
		};

		interface class IWorkspaceModelFactory
		{
		public:
			virtual IWorkspaceModel^				CreateModel(ComponentDLLInterface::ScriptData* Data);
			virtual IWorkspaceModelController^		CreateController();
		};
	}
}