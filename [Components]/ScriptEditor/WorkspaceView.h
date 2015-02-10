#pragma once

#include "WorkspaceViewInterface.h"
#include "WorkspaceModelInterface.h"

using namespace DevComponents;
using namespace DevComponents::DotNetBar::Events;

namespace ConstructionSetExtender
{
	ref class AnimatedForm;
	TODO("remove the destroyonlasttab setting");

	namespace ScriptEditor
	{
		ref class ConcreteWorkspaceView;
		ref class ConcreteWorkspaceViewFactory;

		ref class WorkspaceViewTabTearing
		{
			void								TearingEventHandler(Object^ Sender, MouseEventArgs^ E);

			static MouseEventHandler^			TearingEventDelegate = gcnew MouseEventHandler(&WorkspaceViewTabTearing::TearingEventHandler);

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

			EventHandler^							ToolBarNewScriptClickHandler;
			EventHandler^							ToolBarOpenScriptClickHandler;

			void									ToolBarNewScript_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E);

			EventHandler^							ToolBarMessageListClickHandler;
			EventHandler^							ToolBarFindListClickHandler;
			EventHandler^							ToolBarBookmarkListClickHandler;

			void									ToolBarMessageList_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarFindList_Click(Object^ Sender, EventArgs^ E);
			void									ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E);
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
			// IWorkspaceViewBindableToolbar
			virtual property ToolStripButton^		ButtonNew
			{
				virtual ToolStripButton^ get() { return ToolBarNewScript; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonOpen
			{
				virtual ToolStripButton^ get() { return ToolBarOpenScript; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonPrevious
			{
				virtual ToolStripButton^ get() { return ToolBarPreviousScript; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonNext
			{
				virtual ToolStripButton^ get() { return ToolBarNextScript; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripSplitButton^	ButtonSave
			{
				virtual ToolStripSplitButton^ get() { return ToolBarSaveScript; }
				virtual void set(ToolStripSplitButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonSaveNoCompile
			{
				virtual ToolStripButton^ get() { return ToolBarSaveScriptNoCompile; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonSaveAndPlugin
			{
				virtual ToolStripButton^ get() { return ToolBarSaveScriptAndPlugin; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonRecompile
			{
				virtual ToolStripButton^ get() { return ToolBarRecompileScripts; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonCompileDepends
			{
				virtual ToolStripButton^ get() { return ToolBarCompileDependencies; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonDelete
			{
				virtual ToolStripButton^ get() { return ToolBarDeleteScript; }
				virtual void set(ToolStripButton^ e) {}
			}

			virtual property ToolStripButton^		ButtonFind
			{
				virtual ToolStripButton^ get() { return ToolBarEditMenuContentsFind; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonReplace
			{
				virtual ToolStripButton^ get() { return ToolBarEditMenuContentsReplace; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonGotoLine
			{
				virtual ToolStripButton^ get() { return ToolBarEditMenuContentsGotoLine; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonGotoOffset
			{
				virtual ToolStripButton^ get() { return ToolBarEditMenuContentsGotoOffset; }
				virtual void set(ToolStripButton^ e) {}
			}

			virtual property ToolStripSplitButton^	ButtonSaveToDisk
			{
				virtual ToolStripSplitButton^ get() { return ToolBarDumpScript; }
				virtual void set(ToolStripSplitButton^ e) {}
			}
			virtual property ToolStripSplitButton^	ButtonLoadFromDisk
			{
				virtual ToolStripSplitButton^ get() { return ToolBarLoadScript; }
				virtual void set(ToolStripSplitButton^ e) {}
			}

			virtual property ToolStripButton^		ButtonOffsetViewer
			{
				virtual ToolStripButton^ get() { return ToolBarShowOffsets; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonPreprocessorViewer
			{
				virtual ToolStripButton^ get() { return ToolBarShowPreprocessedText; }
				virtual void set(ToolStripButton^ e) {}
			}

			virtual property ToolStripButton^		ButtonSanitize
			{
				virtual ToolStripButton^ get() { return ToolBarSanitizeScriptText; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonBind
			{
				virtual ToolStripButton^ get() { return ToolBarBindScript; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonSnippets
			{
				virtual ToolStripButton^ get() { return ToolBarSnippetManager; }
				virtual void set(ToolStripButton^ e) {}
			}

			virtual property ToolStripProgressBar^	ProgressBarByteCodeSize
			{
				virtual ToolStripProgressBar^ get() { return ToolBarByteCodeSize; }
				virtual void set(ToolStripProgressBar^ e) {}
			}

			virtual property ToolStripButton^		ButtonTypeObject
			{
				virtual ToolStripButton^ get() { return ToolBarScriptTypeContentsObject; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonTypeQuest
			{
				virtual ToolStripButton^ get() { return ToolBarScriptTypeContentsQuest; }
				virtual void set(ToolStripButton^ e) {}
			}
			virtual property ToolStripButton^		ButtonTypeMagicEffect
			{
				virtual ToolStripButton^ get() { return ToolBarScriptTypeContentsMagicEffect; }
				virtual void set(ToolStripButton^ e) {}
			}

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

			// IWorkspaceViewBindableBase
			property Control^								TextEditorContainer
			{
				virtual Control^ get() { return WorkspaceSplitter->Panel1; }
				virtual void set(Control^ e) {}
			}

			property TextEditors::ScriptOffsetViewer^		OffsetViewer
			{
				virtual TextEditors::ScriptOffsetViewer^ get() { return OffsetTextViewer; }
				virtual void set(TextEditors::ScriptOffsetViewer^ e) {}
			}
			property TextEditors::SimpleTextViewer^			PreprocessedTextViewer
			{
				virtual TextEditors::SimpleTextViewer^ get() { return PreprocessorTextViewer; }
				virtual void set(TextEditors::SimpleTextViewer^ e) {}
			}

			property ScriptListDialog^						ScriptListWindow
			{
				virtual ScriptListDialog^ get() { return ScriptListWindow; }
				virtual void set(ScriptListDialog^ e) {}
			}
			property FindReplaceDialog^						FindReplaceWindow
			{
				virtual FindReplaceDialog^ get() { return FindReplaceWindow; }
				virtual void set(FindReplaceDialog^ e) {}
			}

			property IWorkspaceViewController^			Controller
			{
				virtual IWorkspaceViewController^ get() { return ViewController; }
				virtual void set(IWorkspaceViewController^ e) {}
			}

			property IntPtr								WindowHandle
			{
				virtual IntPtr get() { return EditorForm->Handle; }
				virtual void set(IntPtr e) {}
			}

			property String^							Description
			{
				virtual String^ get() { return String::Empty; }
				virtual void set(String^ e)
				{
					EditorForm->Text = e + " - " + SCRIPTEDITOR_TITLE;
					GetActiveTab()->Tooltip = e;
				}
			}
			property bool								Enabled
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
			virtual void	SetModifiedIndicator(IWorkspaceView^ View, IWorkspaceModel^ Model, bool Modified) override;
			virtual void	BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E) override;

			virtual void	Jump(IWorkspaceView^ View, String^ ScriptEditorID) override;
			virtual void	FindReplaceAll(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
										String^ Query, String^ Replacement, UInt32 Options) override;

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