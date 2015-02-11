#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "AuxiliaryTextEditor.h"
#include "WorkspaceViewInterface.h"
#include "WorkspaceModelInterface.h"
#include "ScriptListDialog.h"
#include "FindReplaceDialog.h"

using namespace DevComponents;
using namespace DevComponents::DotNetBar::Events;

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ref class ConcreteWorkspaceView;
		ref class ConcreteWorkspaceViewFactory;

		ref class WorkspaceViewTabTearing
		{
			void								TearingEventHandler(Object^ Sender, MouseEventArgs^ E);

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

			EventHandler^							NewTabButtonClickHandler;
			EventHandler^							SortTabsButtonClickHandler;
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

			void									NewTabButton_Click(Object^ Sender, EventArgs^ E);
			void									SortTabsButton_Click(Object^ Sender, EventArgs^ E);
			void									ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarNavigationBackClickHandler;
			EventHandler^							ToolBarNavigationForwardClickHandler;
			EventHandler^							ToolBarSaveAllClickHandler;
			EventHandler^							ToolBarOptionsClickHandler;

			void									ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarOptions_Click(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarDumpAllScriptsClickHandler;
			EventHandler^							ToolBarLoadScriptsToTabsClickHandler;

			void									ToolBarDumpAllScripts_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarLoadScriptsToTabs_Click(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarMessageListClickHandler;
			EventHandler^							ToolBarFindListClickHandler;
			EventHandler^							ToolBarBookmarkListClickHandler;

			void									ToolBarMessageList_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarFindList_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarNewScriptClickHandler;
			EventHandler^							ToolBarOpenScriptClickHandler;
			EventHandler^							ToolBarPreviousScriptClickHandler;
			EventHandler^							ToolBarNextScriptClickHandler;
			EventHandler^							ToolBarSaveScriptClickHandler;
			EventHandler^							ToolBarSaveScriptNoCompileClickHandler;
			EventHandler^							ToolBarSaveScriptAndPluginClickHandler;
			EventHandler^							ToolBarRecompileScriptsClickHandler;
			EventHandler^							ToolBarCompileDependenciesClickHandler;
			EventHandler^							ToolBarDeleteScriptClickHandler;

			void									ToolBarNewScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarNextScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarScriptTypeContentsObjectClickHandler;
			EventHandler^							ToolBarScriptTypeContentsQuestClickHandler;
			EventHandler^							ToolBarScriptTypeContentsMagicEffectClickHandler;

			void									ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarEditMenuContentsFindReplaceClickHandler;
			EventHandler^							ToolBarEditMenuContentsGotoLineClickHandler;
			EventHandler^							ToolBarEditMenuContentsGotoOffsetClickHandler;

			void									ToolBarEditMenuContentsFindReplace_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarDumpScriptClickHandler;
			EventHandler^							ToolBarLoadScriptClickHandler;
			EventHandler^							ToolBarShowOffsetsClickHandler;
			EventHandler^							ToolBarShowPreprocessedTextClickHandler;
			EventHandler^							ToolBarSanitizeScriptTextClickHandler;
			EventHandler^							ToolBarBindScriptClickHandler;
			EventHandler^							ToolBarSnippetManagerClickHandler;

			void									ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarBindScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarSnippetManager_Click(Object^ Sender, EventArgs^ E);

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
			ToolStripButton^						ToolBarEditMenuContentsFind;
			ToolStripButton^						ToolBarEditMenuContentsReplace;
			ToolStripButton^						ToolBarEditMenuContentsGotoLine;
			ToolStripButton^						ToolBarEditMenuContentsGotoOffset;
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

			ToolStripDropDownButton^				ToolBarScriptType;
			ToolStripDropDown^						ToolBarScriptTypeContents;
			ToolStripButton^						ToolBarScriptTypeContentsObject;
			ToolStripButton^						ToolBarScriptTypeContentsQuest;
			ToolStripButton^						ToolBarScriptTypeContentsMagicEffect;

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
			void									ShowFindReplaceDialog();

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

		ref class ConcreteWorkspaceViewController : public IWorkspaceViewController
		{
		public:
			virtual void	AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model) override;
			virtual void	DettachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model) override;

			virtual void	SetModifiedIndicator(IWorkspaceView^ View, IWorkspaceModel^ Model, bool Modified) override;
			virtual void	SetByteCodeSize(IWorkspaceView^ View, UInt32 Size) override;
			virtual void	UpdateType(IWorkspaceView^ View, IWorkspaceModel^ Model) override;

			virtual void	BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E) override;

			virtual void	Jump(IWorkspaceView^ View, String^ ScriptEditorID) override;
			virtual int		FindReplace(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
										String^ Query, String^ Replacement, UInt32 Options, bool Global) override;

			virtual void	Redraw(IWorkspaceView^ View) override;
		};

		ref class ConcreteWorkspaceViewFactory : public IWorkspaceViewFactory
		{
			List<ConcreteWorkspaceView^>^				Allocations;

			ConcreteWorkspaceViewFactory();
			~ConcreteWorkspaceViewFactory();
		public:
			property UInt32								Count
			{
				virtual UInt32 get() { return Allocations->Count; }
				virtual void set(UInt32 e) {}
			}

			static ConcreteWorkspaceViewFactory^		Instance = gcnew ConcreteWorkspaceViewFactory;

			void										Remove(ConcreteWorkspaceView^ Allocation);

			// IWorkspaceViewFactory
			virtual IWorkspaceView^						CreateView(int X, int Y, int Width, int Height) override;
			virtual IWorkspaceViewController^			CreateController() override;
		};
	}
}