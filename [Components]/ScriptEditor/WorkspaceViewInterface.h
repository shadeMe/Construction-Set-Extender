#pragma once

#include "Globals.h"
#include "ScriptTextEditorInterface.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		// not a real/proper MVC implementation, obviously
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
			void	AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);
			void	DettachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);

			void	SetModifiedIndicator(IWorkspaceView^ View, IWorkspaceModel^ Model, bool Modified);
			void	SetByteCodeSize(IWorkspaceView^ View, UInt32 Size);
			void	UpdateType(IWorkspaceView^ View, IWorkspaceModel^ Model);

			void	BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);

			void	Jump(IWorkspaceView^ View, IWorkspaceModel^ From, String^ ScriptEditorID);
			int		FindReplace(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
																	String^ Query, String^ Replacement, UInt32 Options, bool Global);

			void	Redraw(IWorkspaceView^ View);

			void	NewTab(IWorkspaceView^ View, NewTabOperationArgs^ E);

			DialogResult	MessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon);
		};

		interface class IWorkspaceViewFactory
		{
		public:
			IWorkspaceView^					CreateView(int X, int Y, int Width, int Height);
			IWorkspaceViewController^		CreateController();
		};
	}
}