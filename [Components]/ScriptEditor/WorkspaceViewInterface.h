#pragma once

#include "Globals.h"
#include "ScriptTextEditorInterface.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		// not a real/proper MVC implementation, just less ugly than what came before it
		interface class IWorkspaceModel;
		interface class IWorkspaceViewController;

		interface class IWorkspaceViewBindableListView
		{
		public:
			property ListView^		ListViewMessages;
			property ListView^		ListViewBookmarks;
			property ListView^		ListViewFindResults;
		};

		interface class IWorkspaceView : public	IWorkspaceViewBindableListView
		{
		public:
			property IWorkspaceViewController^			Controller;

			property IntPtr								WindowHandle;

			property String^							Description;	// updates the window title and tab item tooltip
			property bool								Enabled;
		};

		ref struct NewTabOperationArgs
		{
			static enum class PostNewTabOperation
			{
				None,
				New,
				Open,
				OpenList,
				LoadFromDisk,
				SetText
			};

			PostNewTabOperation							PostCreationOperation;
			String^										PathToFile;
			ComponentDLLInterface::ScriptData*			OpenArgs;
			String^										NewText;

			NewTabOperationArgs() : PostCreationOperation(PostNewTabOperation::None), PathToFile(""), OpenArgs(nullptr), NewText("") {}
		};

		interface class IWorkspaceViewController
		{
		public:
			virtual void	AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);
			virtual void	DettachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);

			virtual void	SetModifiedIndicator(IWorkspaceView^ View, IWorkspaceModel^ Model, bool Modified);
			virtual void	SetByteCodeSize(IWorkspaceView^ View, UInt32 Size);
			virtual void	UpdateType(IWorkspaceView^ View, IWorkspaceModel^ Model);

			virtual void	BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);

			virtual void	Jump(IWorkspaceView^ View, String^ ScriptEditorID);
			virtual int		FindReplace(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
																	String^ Query, String^ Replacement, UInt32 Options, bool Global);

			virtual void	Redraw(IWorkspaceView^ View);
		};

		interface class IWorkspaceViewFactory
		{
		public:
			virtual IWorkspaceView^					CreateView(int X, int Y, int Width, int Height);
			virtual IWorkspaceViewController^		CreateController();
		};
	}
}