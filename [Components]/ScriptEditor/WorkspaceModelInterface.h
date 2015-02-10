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
			property IWorkspaceModelFactory^		Factory;
			property IWorkspaceModelController^		Controller;

			property bool							Initialized;		// has a valid native script object
			property bool							New;				// is a new script
		};

		interface class IWorkspaceModelController
		{
		public:
			virtual void					Bind(IWorkspaceModel^ Model, IWorkspaceView^ To);		// attaches to the view for display
			virtual void					Unbind(IWorkspaceModel^ Model);							// detaches from the view

			virtual void					TunnelKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);

			virtual void					SetText(IWorkspaceModel^ Model, String^ Text, bool ResetUndoStack);

			virtual void					New(IWorkspaceModel^ Model);
			virtual void					Open(IWorkspaceModel^ Model);							// opens the script list dialog
			virtual void					Save(IWorkspaceModel^ Model);
			virtual bool					Close(IWorkspaceModel^ Model);							// returns true if successful
			TODO("freeze the model's after close - can only be destroyed");

			virtual void					LoadFromDisk(IWorkspaceModel^ Model, String^ PathToFile);
			virtual void					SaveToDisk(IWorkspaceModel^ Model, String^ PathToFile, bool PathIncludesFileName, String^ Extension);

			virtual int						FindReplace(IWorkspaceModel^ Model, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
														String^ Query, String^ Replacement, UInt32 Options);
		};

		interface class IWorkspaceModelFactory
		{
		public:
			virtual IWorkspaceModel^				CreateModel(ComponentDLLInterface::ScriptData* Data);
			virtual IWorkspaceModelController^		CreateController();
		};
	}
}