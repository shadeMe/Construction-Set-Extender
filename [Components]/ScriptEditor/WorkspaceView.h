#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "AuxiliaryTextEditor.h"
#include "WorkspaceViewInterface.h"
#include "WorkspaceModelInterface.h"
#include "ScriptListDialog.h"
#include "FindReplaceDialog.h"

using namespace DevComponents;
using namespace DevComponents::DotNetBar::Events;

#define ConcreteWorkspaceViewDeclareClickHandler(Name)		EventHandler^ Name##ClickHandler; \
															void ConcreteWorkspaceView::##Name##_Click(Object^ Sender, EventArgs^ E)
#define ConcreteWorkspaceViewDefineClickHandler(Name)		Name##ClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::##Name##_Click)
#define ConcreteWorkspaceViewSubscribeClickEvent(Name)		Name##->Click += Name##ClickHandler
#define ConcreteWorkspaceViewUnsubscribeClickEvent(Name)	Name##->Click -= Name##ClickHandler

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ref class ConcreteWorkspaceView;

		ref class ConcreteWorkspaceViewFactory : public IWorkspaceViewFactory
		{
			List<ConcreteWorkspaceView^>^				Allocations;

			ConcreteWorkspaceViewFactory();
		public:
			~ConcreteWorkspaceViewFactory();

			property UInt32								Count
			{
				virtual UInt32 get() { return Allocations->Count; }
				virtual void set(UInt32 e) {}
			}

			static ConcreteWorkspaceViewFactory^		Instance = gcnew ConcreteWorkspaceViewFactory;

			void										Remove(ConcreteWorkspaceView^ Allocation);
			void										Clear();

			// IWorkspaceViewFactory
			virtual IWorkspaceView^						CreateView(int X, int Y, int Width, int Height);
			virtual IWorkspaceViewController^			CreateController();
		};

		ref class ConcreteWorkspaceViewController : public IWorkspaceViewController
		{
		public:
			virtual void	AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);
			virtual void	DettachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);

			virtual void	SetModifiedIndicator(IWorkspaceView^ View, IWorkspaceModel^ Model, bool Modified);
			virtual void	SetByteCodeSize(IWorkspaceView^ View, UInt32 Size);
			virtual void	UpdateType(IWorkspaceView^ View, IWorkspaceModel^ Model);

			virtual void	BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);

			virtual void	Jump(IWorkspaceView^ View, IWorkspaceModel^ From, String^ ScriptEditorID);
			virtual int		FindReplace(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
										String^ Query, String^ Replacement, UInt32 Options, bool Global);

			virtual void	Redraw(IWorkspaceView^ View);

			virtual void	NewTab(IWorkspaceView^ View, NewTabOperationArgs^ E);

			virtual DialogResult	MessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon);
		};

		ref class WorkspaceViewTabTearing
		{
			static void							TearingEventHandler(Object^ Sender, MouseEventArgs^ E);

			static MouseEventHandler^			TearingEventDelegate = gcnew MouseEventHandler(&TearingEventHandler);

			static IWorkspaceModel^				Torn = nullptr;
			static ConcreteWorkspaceView^		Source = nullptr;

			static void							End();
		public:
			static bool							InProgress = false;

			static void							Begin(IWorkspaceModel^ Tearing, ConcreteWorkspaceView^ From);
		};

		ref class ConcreteWorkspaceView : public IWorkspaceView
		{
			static Rectangle LastUsedBounds = Rectangle(100, 100, 100, 100);

			CancelEventHandler^						EditorFormCancelHandler;
			KeyEventHandler^						EditorFormKeyDownHandler;
			EventHandler^							EditorFormPositionChangedHandler;
			EventHandler^							EditorFormSizeChangedHandler;

			EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>^			ScriptStripTabItemCloseHandler;
			EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>^		ScriptStripSelectedTabChangedHandler;
			EventHandler<DotNetBar::SuperTabStripTabRemovedEventArgs^>^             ScriptStripTabRemovedHandler;
			EventHandler<MouseEventArgs^>^											ScriptStripMouseClickHandler;
			EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>^              ScriptStripTabMovingHandler;

			EventHandler^							ScriptEditorPreferencesSavedHandler;

			void									EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E);
			void									EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void									EditorForm_SizeChanged(Object^ Sender, EventArgs^ E);
			void									EditorForm_PositionChanged(Object^ Sender, EventArgs^ E);

			void									ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs ^ E);
			void									ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E);
			void									ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E);
			void									ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E);
			void									ScriptStrip_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E);

			void									ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

			ConcreteWorkspaceViewDeclareClickHandler(NewTabButton);
			ConcreteWorkspaceViewDeclareClickHandler(SortTabsButton);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarNavigationBack);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarNavigationForward);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarSaveAll);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarOptions);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarDumpAllScripts);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarLoadScriptsToTabs);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarMessageList);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarFindList);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarBookmarkList);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarNewScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarOpenScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarPreviousScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarNextScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarSaveScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarSaveScriptNoCompile);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarSaveScriptAndPlugin);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarRecompileScripts);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarCompileDependencies);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarDeleteScript);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarScriptTypeContentsObject);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarScriptTypeContentsQuest);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarScriptTypeContentsMagicEffect);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarEditMenuContentsFindReplace);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarEditMenuContentsGotoLine);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarEditMenuContentsGotoOffset);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarRefactorMenuContentsDocumentScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarRefactorMenuContentsRenameVariables);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarRefactorMenuContentsModifyVariableIndices);

			ConcreteWorkspaceViewDeclareClickHandler(ToolBarDumpScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarLoadScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarShowOffsets);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarShowPreprocessedText);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarSanitizeScriptText);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarBindScript);
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarSnippetManager);
		public:
			AnimatedForm^							EditorForm;
			DotNetBar::SuperTabControl^				EditorTabStrip;
			DotNetBar::ButtonItem^					NewTabButton;
			DotNetBar::ButtonItem^					SortTabsButton;

			ToolStrip^								WorkspaceMainToolBar;
			ToolStripButton^						ToolBarNewScript;
			ToolStripButton^						ToolBarOpenScript;
			ToolStripButton^						ToolBarPreviousScript;
			ToolStripButton^						ToolBarNextScript;
			ToolStripSplitButton^					ToolBarSaveScript;
			ToolStripDropDown^						ToolBarSaveScriptDropDown;
			ToolStripButton^						ToolBarSaveScriptNoCompile;
			ToolStripButton^						ToolBarSaveScriptAndPlugin;
			ToolStripButton^						ToolBarRecompileScripts;
			ToolStripButton^						ToolBarCompileDependencies;
			ToolStripButton^						ToolBarDeleteScript;
			ToolStripButton^						ToolBarNavigationBack;
			ToolStripButton^						ToolBarNavigationForward;
			ToolStripButton^						ToolBarSaveAll;
			ToolStripButton^						ToolBarOptions;

			ToolStrip^								WorkspaceSecondaryToolBar;

			ToolStripDropDownButton^				ToolBarEditMenu;
			ToolStripDropDown^						ToolBarEditMenuContents;
			ToolStripButton^						ToolBarEditMenuContentsFindReplace;
			ToolStripButton^						ToolBarEditMenuContentsGotoLine;
			ToolStripButton^						ToolBarEditMenuContentsGotoOffset;

			ToolStripDropDownButton^				ToolBarRefactorMenu;
			ToolStripDropDown^						ToolBarRefactorMenuContents;
			ToolStripButton^						ToolBarRefactorMenuContentsDocumentScript;
			ToolStripButton^						ToolBarRefactorMenuContentsRenameVariables;
			ToolStripButton^						ToolBarRefactorMenuContentsModifyVariableIndices;

			ToolStripDropDownButton^				ToolBarScriptType;
			ToolStripDropDown^						ToolBarScriptTypeContents;
			ToolStripButton^						ToolBarScriptTypeContentsObject;
			ToolStripButton^						ToolBarScriptTypeContentsQuest;
			ToolStripButton^						ToolBarScriptTypeContentsMagicEffect;

			ToolStripButton^						ToolBarMessageList;
			ToolStripButton^						ToolBarFindList;
			ToolStripButton^						ToolBarBookmarkList;
			ToolStripSplitButton^					ToolBarDumpScript;
			ToolStripDropDown^						ToolBarDumpScriptDropDown;
			ToolStripButton^						ToolBarDumpAllScripts;
			ToolStripSplitButton^					ToolBarLoadScript;
			ToolStripDropDown^						ToolBarLoadScriptDropDown;
			ToolStripButton^						ToolBarLoadScriptsToTabs;
			ToolStripButton^						ToolBarShowOffsets;
			ToolStripButton^						ToolBarShowPreprocessedText;
			ToolStripButton^						ToolBarSanitizeScriptText;
			ToolStripButton^						ToolBarBindScript;
			ToolStripButton^						ToolBarSnippetManager;
			ToolStripProgressBar^					ToolBarByteCodeSize;

			SplitContainer^							WorkspaceSplitter;
			ListView^								MessageList;
			ListView^								FindList;
			ListView^								BookmarkList;
			Label^									SpoilerText;

			TextEditors::ScriptOffsetViewer^		OffsetTextViewer;
			TextEditors::SimpleTextViewer^			PreprocessorTextViewer;

			ScriptListDialog^						ScriptListBox;
			FindReplaceDialog^						FindReplaceBox;

			ConcreteWorkspaceViewController^		ViewController;
			ConcreteWorkspaceViewFactory^			ViewFactory;

			typedef Dictionary<IWorkspaceModel^, DotNetBar::SuperTabItem^>	ModelTabTableT;
			ModelTabTableT^							AssociatedModels;

			property bool							AllowDisposal;												// when false, the form's closing is canceled

			IWorkspaceModelController^				ModelController();
			IWorkspaceModelFactory^					ModelFactory();

			void									AssociateModel(IWorkspaceModel^ Model, bool Bind);			// assign model a tab item and add it to the table
			void									DissociateModel(IWorkspaceModel^ Model, bool Destroy);		// remove model from table and dissociate from the view
			IWorkspaceModel^						GetModel(DotNetBar::SuperTabItem^ Tab);
			bool									IsModelAssociated(IWorkspaceModel^ Model);
			IWorkspaceModel^						GetActiveModel();

			DotNetBar::SuperTabItem^				GetTab(IWorkspaceModel^ Model);
			DotNetBar::SuperTabItem^				GetMouseOverTab();
			DotNetBar::SuperTabItem^				GetActiveTab();

			Rectangle								GetBounds(bool UseRestoreBounds);
			void									ToggleSecondaryPanel(bool State);
			void									UpdateScriptTypeControls();

			void									ShowOpenDialog();
			void									ShowDeleteDialog();
			void									ShowFindReplaceDialog(bool PerformSearch);

			void									GotoLine();
			void									GotoOffset();

			void									SaveAll();
			void									CloseAll();

			void									DumpAllToDisk(String^ OutputDirectory, String^ FileExtension);
			void									LoadAllFromDisk(array<String^>^ FilePaths);

			void									BeginUpdate();
			void									EndUpdate();
			void									Redraw();

			int										GetTabCount();
			void									SelectTab(DotNetBar::SuperTabItem^ Tab);
			void									SelectTab(Keys Index);
			void									SelectTab(int Index);
			void									SelectNextTab();
			void									SelectPreviousTab();

			void									NewTab(NewTabOperationArgs^ E);

			ConcreteWorkspaceView(ConcreteWorkspaceViewController^ Controller, ConcreteWorkspaceViewFactory^ Factory, Rectangle Bounds);
			~ConcreteWorkspaceView();

#pragma region Interfaces
			// IWorkspaceViewBindableListView
			property ListView^						ListViewMessages
			{
				virtual ListView^ get() { return MessageList; }
				virtual void set(ListView^ e) {}
			}
			property ListView^						ListViewBookmarks
			{
				virtual ListView^ get() { return BookmarkList; }
				virtual void set(ListView^ e) {}
			}
			property ListView^						ListViewFindResults
			{
				virtual ListView^ get() { return FindList; }
				virtual void set(ListView^ e) {}
			}

			property IWorkspaceViewController^		Controller
			{
				virtual IWorkspaceViewController^ get() { return ViewController; }
				virtual void set(IWorkspaceViewController^ e) {}
			}
			property IntPtr							WindowHandle
			{
				virtual IntPtr get() { return EditorForm->Handle; }
				virtual void set(IntPtr e) {}
			}
			property String^						Description
			{
				virtual String^ get() { return String::Empty; }
				virtual void set(String^ e)
				{
					EditorForm->Text = e + " - " + SCRIPTEDITOR_TITLE;
					GetActiveTab()->Tooltip = e;
				}
			}
			property bool							Enabled
			{
				virtual bool get() { return WorkspaceSplitter->Panel1->Enabled; }
				virtual void set(bool e)
				{
					WorkspaceSplitter->Panel1->Enabled = e;
					WorkspaceSplitter->Panel2->Enabled = e;
				}
			}
#pragma endregion
		};
	}
}