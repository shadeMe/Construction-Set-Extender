#pragma once

#include "Globals.h"
#include "ScriptTextEditorInterface.h"
#include "TextEditorAdornments.h"

namespace cse
{
	namespace scriptEditor
	{
		// not a real/proper MVC implementation, obviously
		interface class IWorkspaceModel;
		interface class IWorkspaceViewController;

		interface class IWorkspaceView
		{
		public:
			property IWorkspaceViewController^
								Controller;
			property intellisense::IIntelliSenseInterfaceView^
								IntelliSenseInterfaceView;
			property textEditors::ScopeBreadcrumbManager^
								BreadcrumbManager;
			property IntPtr		WindowHandle;
			property bool		Enabled;
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

			PostNewTabOperation					PostCreationOperation;
			String^								PathToFile;
			componentDLLInterface::ScriptData* 	OpenArgs;
			String^								NewText;
			bool								BindAfterCreation;

			NewTabOperationArgs()
			{
				PostCreationOperation = PostNewTabOperation::None;
				PathToFile = "";
				OpenArgs = nullptr;
				NewText = "";
				BindAfterCreation = false;
			}
		};

		interface class IWorkspaceViewController
		{
		public:
			void AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);
			void DetachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);
			void BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);
			void Jump(IWorkspaceView^ View, IWorkspaceModel^ From, String^ ScriptEditorID);
			int	 FindReplace(IWorkspaceView^ View, textEditors::IScriptTextEditor::FindReplaceOperation Operation,
				 			 String^ Query, String^ Replacement, textEditors::IScriptTextEditor::FindReplaceOptions Options, bool Global);
			void ShowOutline(IWorkspaceView^ View, obScriptParsing::Structurizer^ Data, IWorkspaceModel^ Model);
			void Redraw(IWorkspaceView^ View);
			void NewTab(IWorkspaceView^ View, NewTabOperationArgs^ E);
			DialogResult MessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon);
		};

		interface class IWorkspaceViewFactory
		{
		public:
			IWorkspaceView^ CreateView(int X, int Y, int Width, int Height);
			IWorkspaceViewController^ CreateController();
		};
	}
}