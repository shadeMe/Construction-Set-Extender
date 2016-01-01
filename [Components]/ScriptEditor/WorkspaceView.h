#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "AuxiliaryTextEditor.h"
#include "WorkspaceViewInterface.h"
#include "WorkspaceModelInterface.h"
#include "ScriptListDialog.h"
#include "FindReplaceDialog.h"

using namespace DevComponents;
using namespace DevComponents::DotNetBar::Events;

#define ConcreteWorkspaceViewDeclareClickHandler(Name)				EventHandler^ Name##ClickHandler; \
																	void ConcreteWorkspaceView::##Name##_Click(Object^ Sender, EventArgs^ E)
#define ConcreteWorkspaceViewDefineClickHandler(Name)				Name##ClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::##Name##_Click)
#define ConcreteWorkspaceViewSubscribeClickEvent(Name)				Name##->Click += Name##ClickHandler
#define ConcreteWorkspaceViewUnsubscribeClickEvent(Name)			Name##->Click -= Name##ClickHandler
#define ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(Name)		Name##->Click -= Name##ClickHandler; delete Name##ClickHandler; Name##ClickHandler = nullptr

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
			virtual void	DetachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);

			virtual void	BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);

			virtual void	Jump(IWorkspaceView^ View, IWorkspaceModel^ From, String^ ScriptEditorID);
			virtual int		FindReplace(IWorkspaceView^ View, TextEditors::IScriptTextEditor::FindReplaceOperation Operation,
										String^ Query, String^ Replacement, UInt32 Options, bool Global);
			virtual void	ShowOutline(IWorkspaceView^ View, ObScriptParsing::Structurizer^ Data, IWorkspaceModel^ Model);

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

		ref class WorkspaceViewTabFilter
		{
			ConcreteWorkspaceView^						Parent;

			AnimatedForm^								Form;
			BrightIdeasSoftware::ObjectListView^		ListView;
			TextBox^									SearchBox;

			List<DotNetBar::SuperTabItem^>^				FilterResults;

			void										ListView_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void										ListView_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
			void										ListView_ItemActivate(Object^ Sender, EventArgs^ E);

			void										SearchBox_TextChanged(Object^ Sender, EventArgs^ E);
			void										SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

			void										Form_Deactivate(Object^ Sender, EventArgs^ E);

			KeyEventHandler^							ListViewKeyDownHandler;
			KeyPressEventHandler^						ListViewKeyPressHandler;
			EventHandler^								ListViewItemActivateHandler;
			EventHandler^								SearchBoxTextChangedHandler;
			KeyEventHandler^							SearchBoxKeyDownHandler;
			EventHandler^								FormDeactivateHandler;

			void										UpdateFilterResults();
			void										CompleteSelection();

			static Object^								ListViewAspectGetter(Object^ RowObject);
			static Object^								ListViewImageGetter(Object^ RowObject);
		public:
			WorkspaceViewTabFilter(ConcreteWorkspaceView^ ParentView);
			~WorkspaceViewTabFilter();

			void										Show();
			void										Hide();
		};

		ref class WorkspaceViewOutlineView
		{
			ConcreteWorkspaceView^						Parent;

			AnimatedForm^								Form;
			BrightIdeasSoftware::TreeListView^			ListView;
			ObScriptParsing::Structurizer^				StructureData;
			IWorkspaceModel^							AssociatedModel;

			void										ListView_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void										ListView_ItemActivate(Object^ Sender, EventArgs^ E);

			void										Form_Deactivate(Object^ Sender, EventArgs^ E);

			KeyEventHandler^							ListViewKeyDownHandler;
			EventHandler^								ListViewItemActivateHandler;
			EventHandler^								FormDeactivateHandler;

			void										JumpToLine(UInt32 Line);
			void										ResetState();

			static Object^								ListViewAspectGetter(Object^ RowObject);
			static Object^								ListViewImageGetter(Object^ RowObject);
			static bool									ListViewCanExpandGetter(Object^ E);
			static Collections::IEnumerable^			ListViewChildrenGetter(Object^ E);
		public:
			WorkspaceViewOutlineView(ConcreteWorkspaceView^ ParentView);
			~WorkspaceViewOutlineView();

			void										Show(ObScriptParsing::Structurizer^ Data, IWorkspaceModel^ Model);
			void										Hide();
		};

		ref struct FindReplaceAllResults
		{
			ref struct HitData
			{
				IWorkspaceModel^										Parent;
				String^													ParentDescription;
				TextEditors::IScriptTextEditor::FindReplaceResult^		Hits;

				HitData(IWorkspaceModel^ Parent, TextEditors::IScriptTextEditor::FindReplaceResult^ Data) :
						Parent(Parent),
						ParentDescription(Parent->LongDescription),
						Hits(Data) {}
			};

			TextEditors::IScriptTextEditor::FindReplaceOperation		Operation;
			String^														Query;
			String^														Replacement;
			UInt32														Options;

			List<HitData^>^												Hits;

			FindReplaceAllResults() : Operation(TextEditors::IScriptTextEditor::FindReplaceOperation::CountMatches),
				Query(""), Replacement(""), Options(0), Hits(gcnew List<HitData^>) {}

			void Add(IWorkspaceModel^ Model, TextEditors::IScriptTextEditor::FindReplaceResult^ Data)
			{
				Debug::Assert(Data->HasError == false);
				Hits->Add(gcnew HitData(Model, Data));
			}

			property int TotalHitCount
			{
				int get()
				{
					int Count = 0;

					for each (auto Instance in Hits)
						Count += Instance->Hits->TotalHitCount;

					return Count;
				}
			}

			static bool							GenericCanExpandGetter(Object^ E);
			static Collections::IEnumerable^	GenericChildrenGetter(Object^ E);
			static Object^						GenericAspectGetter(Object^ E);
		};

		ref class ConcreteWorkspaceView : public IWorkspaceView
		{
			static Rectangle LastUsedBounds = Rectangle(100, 100, 100, 100);

			CancelEventHandler^						EditorFormCancelHandler;
			KeyEventHandler^						EditorFormKeyDownHandler;
			EventHandler^							EditorFormPositionChangedHandler;
			EventHandler^							EditorFormSizeChangedHandler;
			EventHandler^							EditorFormActivated;

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
			void									EditorForm_Activated(Object^ Sender, EventArgs^ E);

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
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarGlobalFindList);
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

			void											ModelStateChangeHandler_Dirty(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E);
			void											ModelStateChangeHandler_ByteCodeSize(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E);
			void											ModelStateChangeHandler_Type(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E);
			void											ModelStateChangeHandler_Description(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E);

			IWorkspaceModel::StateChangeEventHandler^		ModelStateChangedDirty;
			IWorkspaceModel::StateChangeEventHandler^		ModelStateChangedByteCodeSize;
			IWorkspaceModel::StateChangeEventHandler^		ModelStateChangedType;
			IWorkspaceModel::StateChangeEventHandler^		ModelStateChangedDescription;

			void									ModelSubscribeEvents(IWorkspaceModel^ Model);
			void									ModelUnsubscribeEvents(IWorkspaceModel^ Model);

			Stack<IWorkspaceModel^>^				NavigationStackBackward;
			Stack<IWorkspaceModel^>^				NavigationStackForward;
			bool									FreezeNavigationStacks;

			void									RemoveFromNavigationStacks(IWorkspaceModel^ Model);
			void									RemoveFromFindReplaceAllResultCache(IWorkspaceModel^ Model);

			EventHandler^							GlobalFindListItemActivate;
			void									GlobalFindList_ItemActivate(Object^ Sender, EventArgs^ E);
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
			ToolStripButton^						ToolBarGlobalFindList;
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
			Panel^									AttachPanel;
			ListView^								MessageList;
			ListView^								FindList;
			ListView^								BookmarkList;
			BrightIdeasSoftware::TreeListView^		GlobalFindList;
			Label^									SpoilerText;

			TextEditors::ScriptOffsetViewer^		OffsetTextViewer;
			TextEditors::SimpleTextViewer^			PreprocessorTextViewer;

			ScriptListDialog^						ScriptListBox;
			FindReplaceDialog^						FindReplaceBox;

			IntelliSense::IIntelliSenseInterfaceView^	IntelliSenseView;

			WorkspaceViewTabFilter^					TabStripFilter;
			List<FindReplaceAllResults^>^			CachedFindReplaceAllResults;
			WorkspaceViewOutlineView^				OutlineView;

			ConcreteWorkspaceViewController^		ViewController;
			ConcreteWorkspaceViewFactory^			ViewFactory;

			typedef Dictionary<IWorkspaceModel^, DotNetBar::SuperTabItem^>	ModelTabTableT;
			ModelTabTableT^							AssociatedModels;

			bool									AllowDisposal;								// when false, the form's closing is canceled
			bool									DisallowBinding;							// when true, prevents models from binding to the view

			IWorkspaceModelController^				ModelController();							// yeah, not nice
			IWorkspaceModelFactory^					ModelFactory();								// not nice at all...

			void									AssociateModel(IWorkspaceModel^ Model, bool Bind);			// assign model a tab item and add it to the table
			void									DissociateModel(IWorkspaceModel^ Model, bool Destroy);		// remove model from table and dissociate from the view
			IWorkspaceModel^						GetModel(DotNetBar::SuperTabItem^ Tab);
			bool									IsModelAssociated(IWorkspaceModel^ Model);
			IWorkspaceModel^						GetActiveModel();
			IWorkspaceModel^						GetModel(String^ Description);

			DotNetBar::SuperTabItem^				GetTab(IWorkspaceModel^ Model);
			DotNetBar::SuperTabItem^				GetMouseOverTab();
			DotNetBar::SuperTabItem^				GetActiveTab();

			Rectangle								GetBounds(bool UseRestoreBounds);
			void									ToggleSecondaryPanel(bool State);
			void									UpdateScriptTypeControls(IWorkspaceModel::ScriptType Type);

			void									ShowMessageList();
			void									ShowFindResultList();
			void									ShowBookmarkList();
			void									ShowGlobalFindResultList();
			void									ToggleMessageList(bool State);
			void									ToggleBookmarkList(bool State);
			void									ToggleFindResultList(bool State);
			void									ToggleGlobalFindResultList(bool State);
			void									HideAllLists();

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
			void									Focus();

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
			property IntelliSense::IIntelliSenseInterfaceView^ IntelliSenseInterfaceView
			{
				virtual IntelliSense::IIntelliSenseInterfaceView^ get() { return IntelliSenseView; }
				virtual void set(IntelliSense::IIntelliSenseInterfaceView^ e) {}
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
			property bool							Enabled
			{
				virtual bool get() { return WorkspaceSplitter->Panel1->Enabled; }
				virtual void set(bool e)
				{
					AttachPanel->Enabled = e;
					WorkspaceSplitter->Panel2->Enabled = e;
				}
			}
#pragma endregion
		};
	}
}