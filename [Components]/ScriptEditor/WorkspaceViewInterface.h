#pragma once
#include "Globals.h"
#include "AuxiliaryTextEditor.h"
#include "ScriptTextEditorInterface.h"

using namespace DevComponents;

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{	
		ref class ScriptListDialog;
		ref class FindReplaceDialog;

		// not a real/proper MVC implementation, just less ugly than what came before it
		interface class IWorkspaceModel;
		interface class IWorkspaceModelController;
		interface class IWorkspaceModelFactory;
		interface class IWorkspaceViewController;

		interface class IWorkspaceViewBindableToolbar
		{
		public:
			property ToolStripButton^					ButtonNew;
			property ToolStripButton^					ButtonOpen;
			property ToolStripButton^					ButtonPrevious;
			property ToolStripButton^					ButtonNext;
			property ToolStripSplitButton^				ButtonSave;
			property ToolStripButton^					ButtonSaveNoCompile;
			property ToolStripButton^					ButtonSaveAndPlugin;
			property ToolStripButton^					ButtonRecompile;
			property ToolStripButton^					ButtonCompileDepends;
			property ToolStripButton^					ButtonDelete;

			property ToolStripButton^					ButtonFind;
			property ToolStripButton^					ButtonReplace;
			property ToolStripButton^					ButtonGotoLine;
			property ToolStripButton^					ButtonGotoOffset;

			property ToolStripButton^					ButtonMessages;
			property ToolStripButton^					ButtonBookmarks;
			property ToolStripButton^					ButtonFindResults;

			property ToolStripSplitButton^				ButtonSaveToDisk;
			property ToolStripSplitButton^				ButtonLoadFromDisk;

			property ToolStripButton^					ButtonOffsetViewer;
			property ToolStripButton^					ButtonPreprocessorViewer;

			property ToolStripButton^					ButtonSanitize;
			property ToolStripButton^					ButtonBind;
			property ToolStripButton^					ButtonSnippets;

			property ToolStripProgressBar^				ProgressBarByteCodeSize;

			property ToolStripButton^					ButtonTypeObject;
			property ToolStripButton^					ButtonTypeQuest;
			property ToolStripButton^					ButtonTypeMagicEffect;
		};

		interface class IWorkspaceViewBindableListView
		{
		public:
			property ListView^							ListViewMessages;
			property ListView^							ListViewBookmarks;
			property ListView^							ListViewFindResults;
		};

		interface class IWorkspaceViewBindableBase
		{
		public:
			property Control^								TextEditorContainer;

			property TextEditors::ScriptOffsetViewer^		OffsetViewer;
			property TextEditors::SimpleTextViewer^			PreprocessedTextViewer;

			property ScriptListDialog^						ScriptListWindow;
			property FindReplaceDialog^						FindReplaceWindow;
		};

		interface class IWorkspaceView : public	IWorkspaceViewBindableBase,
												IWorkspaceViewBindableToolbar,
												IWorkspaceViewBindableListView
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
			virtual void								SetModifiedIndicator(IWorkspaceView^ View, IWorkspaceModel^ Model, bool Modified);
			virtual void								BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);
			
			virtual void								Jump(IWorkspaceView^ View, String^ ScriptEditorID);
			virtual void								FindReplaceAll(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
																	String^ Query, String^ Replacement, UInt32 Options);

			virtual void								Redraw(IWorkspaceView^ View);
		};

		interface class IWorkspaceViewFactory
		{
		public:
			virtual IWorkspaceView^						CreateView(int X, int Y, int Width, int Height);
			virtual IWorkspaceViewController^			CreateController();
		};
	}
}