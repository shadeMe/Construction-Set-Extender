#pragma once

#include "[Common]\AuxiliaryWindowsForm.h"
#include "AuxiliaryTextEditor.h"
#include "WorkspaceViewInterface.h"
#include "WorkspaceModelInterface.h"
#include "ScriptSelectionDialog.h"
#include "FindReplaceDialog.h"
#include "TextEditorAdornments.h"

using namespace DevComponents;
using namespace DevComponents::DotNetBar::Events;

#define ConcreteWorkspaceViewDeclareClickHandler(Name)				EventHandler^ Name##ClickHandler; \
																	void ConcreteWorkspaceView::##Name##_Click(Object^ Sender, EventArgs^ E)
#define ConcreteWorkspaceViewDefineClickHandler(Name)				Name##ClickHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::##Name##_Click)
#define ConcreteWorkspaceViewSubscribeClickEvent(Name)				Name##->Click += Name##ClickHandler
#define ConcreteWorkspaceViewUnsubscribeClickEvent(Name)			Name##->Click -= Name##ClickHandler
#define ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(Name)		Name##->Click -= Name##ClickHandler; delete Name##ClickHandler; Name##ClickHandler = nullptr

namespace cse
{
	namespace scriptEditor
	{
		using namespace BrightIdeasSoftware;

		ref class ConcreteWorkspaceView;

		ref class ConcreteWorkspaceViewFactory : public IWorkspaceViewFactory
		{
			List<ConcreteWorkspaceView^>^ Allocations;

			ConcreteWorkspaceViewFactory();
		public:
			~ConcreteWorkspaceViewFactory();

			property UInt32	Count
			{
				virtual UInt32 get() { return Allocations->Count; }
				virtual void set(UInt32 e) {}
			}

			void								Remove(ConcreteWorkspaceView^ Allocation);
			void								Clear();

			virtual IWorkspaceView^				CreateView(int X, int Y, int Width, int Height);
			virtual IWorkspaceViewController^	CreateController();

			static ConcreteWorkspaceViewFactory^ Instance = gcnew ConcreteWorkspaceViewFactory;
		};

		ref class ConcreteWorkspaceViewController : public IWorkspaceViewController
		{
		public:
			virtual void	AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);
			virtual void	DetachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model);

			virtual void	BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E);

			virtual void	Jump(IWorkspaceView^ View, IWorkspaceModel^ From, String^ ScriptEditorID);
			virtual int		FindReplace(IWorkspaceView^ View, textEditor::eFindReplaceOperation Operation,
										String^ Query, String^ Replacement, textEditor::eFindReplaceOptions Options, bool Global);
			virtual void	ShowOutline(IWorkspaceView^ View, obScriptParsing::Structurizer^ Data, IWorkspaceModel^ Model);

			virtual void	Redraw(IWorkspaceView^ View);

			virtual void	NewTab(IWorkspaceView^ View, NewTabOperationArgs^ E);

			virtual DialogResult	MessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon);
		};

		ref class WorkspaceViewTabTearingHelper
		{
			static WorkspaceViewTabTearingHelper^	Singleton = nullptr;

			IWorkspaceModel^		Torn;
			ConcreteWorkspaceView^	Source;
			bool					Active;
			bool					ProcessingMouseMessage;
			MouseEventHandler^		TearingEventDelegate;

			void	TearingEventHandler(Object^ Sender, MouseEventArgs^ E);
			void	End();

			WorkspaceViewTabTearingHelper();
		public:
			~WorkspaceViewTabTearingHelper();

			void InitiateHandling(IWorkspaceModel^ Tearing, ConcreteWorkspaceView^ From);

			property bool InProgress
			{
				bool get() { return Active; }
			}

			static WorkspaceViewTabTearingHelper^ Get();
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
			obScriptParsing::Structurizer^				StructureData;
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

			void										Show(obScriptParsing::Structurizer^ Data, IWorkspaceModel^ Model);
			void										Hide();
		};

		ref struct FindReplaceAllResults
		{
			ref struct PerScriptData
			{
				IWorkspaceModel^										ParentModel;
				String^													ParentDescription;
				textEditor::FindReplaceResult^		Results;

				PerScriptData(IWorkspaceModel^ Parent, textEditor::FindReplaceResult^ Data)
				{
					ParentModel = Parent;
					ParentDescription = ParentModel->LongDescription;
					Results = Data;
				}

				property int TotalHitCount
				{
					int get() { return Results->TotalHits; }
				}
			};

			textEditor::eFindReplaceOperation		Operation;
			String^														Query;
			String^														Replacement;
			textEditor::eFindReplaceOptions			Options;

			List<PerScriptData^>^										ScriptsWithHits;

			FindReplaceAllResults()
			{
				Operation = textEditor::eFindReplaceOperation::CountMatches;
				Query = "";
				Replacement = "";
				Options = textEditor::eFindReplaceOptions::None;
				ScriptsWithHits = gcnew List<PerScriptData^>;
			}

			void Add(IWorkspaceModel^ Model, textEditor::FindReplaceResult^ Data)
			{
				Debug::Assert(Data->HasError == false);
				ScriptsWithHits->Add(gcnew PerScriptData(Model, Data));
			}

			property int TotalHitCount
			{
				int get()
				{
					int Count = 0;

					for each (auto Instance in ScriptsWithHits)
						Count += Instance->Results->TotalHits;

					return Count;
				}
			}

			static bool							GenericCanExpandGetter(Object^ E);
			static Collections::IEnumerable^	GenericChildrenGetter(Object^ E);
			static Object^						TextAspectGetter(Object^ E);
			static Object^						LineAspectGetter(Object^ E);
			static Object^						HitsAspectGetter(Object^ E);
		};

		ref class ConcreteWorkspaceView : public IWorkspaceView
		{
			static Rectangle LastUsedBounds = Rectangle(100, 100, 100, 100);

			CancelEventHandler^	EditorFormCancelHandler;
			KeyEventHandler^	EditorFormKeyDownHandler;
			EventHandler^		EditorFormPositionChangedHandler;
			EventHandler^		EditorFormSizeChangedHandler;
			EventHandler^		EditorFormActivated;

			EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>^		ScriptStripTabItemCloseHandler;
			EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>^	ScriptStripSelectedTabChangedHandler;
			EventHandler<DotNetBar::SuperTabStripTabRemovedEventArgs^>^         ScriptStripTabRemovedHandler;
			EventHandler<MouseEventArgs^>^										ScriptStripMouseClickHandler;
			EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>^          ScriptStripTabMovingHandler;
			EventHandler^														ScriptEditorPreferencesSavedHandler;

			void EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E);
			void EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void EditorForm_SizeChanged(Object^ Sender, EventArgs^ E);
			void EditorForm_PositionChanged(Object^ Sender, EventArgs^ E);
			void EditorForm_Activated(Object^ Sender, EventArgs^ E);

			void ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs ^ E);
			void ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E);
			void ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E);
			void ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E);
			void ScriptStrip_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E);

			void ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E);

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
			ConcreteWorkspaceViewDeclareClickHandler(ToolBarSyncScriptsToDisk);

			void Model_StateChangeHandler(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E);

			IWorkspaceModel::StateChangeEventHandler^ ModelStateChangedHandler;


			void ModelSubscribeEvents(IWorkspaceModel^ Model);
			void ModelUnsubscribeEvents(IWorkspaceModel^ Model);

			Stack<IWorkspaceModel^>^ NavigationStackBackward;
			Stack<IWorkspaceModel^>^ NavigationStackForward;
			bool					 FreezeNavigationStacks;

			void RemoveFromNavigationStacks(IWorkspaceModel^ Model);
			void RemoveFromFindReplaceAllResultCache(IWorkspaceModel^ Model);

			EventHandler^	GlobalFindListItemActivate;
			void			GlobalFindList_ItemActivate(Object^ Sender, EventArgs^ E);

			EventHandler^	ScriptAnnotationListItemActivate;
			void			ScriptAnnotationList_ItemActivate(Object^ Sender, EventArgs^ E);


			EventHandler<BrightIdeasSoftware::CellClickEventArgs^>^
							BookmarksListButtonClick;
			void			BookmarksList_ButtonClick(Object^ Sender, BrightIdeasSoftware::CellClickEventArgs^ E);

			using ModelTabTableT = Dictionary<IWorkspaceModel^, DotNetBar::SuperTabItem^>;

			static Object^ ScriptTextAnnotationListLineNumberAspectGetter(Object^ E);
			static Object^ ScriptTextAnnotationListTextAspectGetter(Object^ E);

			static Object^ MessageListTypeAspectGetter(Object^ E);
			static Object^ MessageListTypeImageGetter(Object^ RowObject);
			static String^ MessageListTypeAspectToStringConverter(Object^ E);
			static Object^ MessageListTypeGroupKeyGetter(Object^ RowObject);
			static String^ MessageListTypeGroupKeyToTitleConverter(System::Object^ GroupKey);
			static Object^ MessageListSourceAspectGetter(Object^ E);
			static String^ MessageListSourceAspectToStringConverter(Object^ E);
			static Object^ MessageListSourceGroupKeyGetter(Object^ RowObject);
			static String^ MessageListSourceGroupKeyToTitleConverter(System::Object^ GroupKey);

			static Object^ BookmarksListActionAspectGetter(Object^ E);
			static Object^ FindResultsListHitsAspectGetter(Object^ E);
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
			ToolStripButton^						ToolBarSyncScriptsToDisk;
			ToolStripProgressBar^					ToolBarByteCodeSize;

			SplitContainer^							WorkspaceSplitter;
			Panel^									AttachPanel;
			ObjectListView^							MessageList;
			FastObjectListView^						FindResultsList;
			FastObjectListView^						BookmarkList;
			TreeListView^							GlobalFindList;
			Label^									SpoilerText;

			textEditor::ScriptOffsetViewer^		OffsetTextViewer;
			textEditor::SimpleTextViewer^			PreprocessorTextViewer;

			DotNetBar::CrumbBar^					ScopeCrumbBar;
			textEditor::ScopeBreadcrumbManager^	ScopeCrumbManager;

			FindReplaceDialog^						FindReplaceBox;
			intellisense::IIntelliSenseInterfaceView^
													IntelliSenseView;

			WorkspaceViewTabFilter^					TabStripFilter;
			List<FindReplaceAllResults^>^			CachedFindReplaceAllResults;
			WorkspaceViewOutlineView^				OutlineView;

			ConcreteWorkspaceViewController^		ViewController;
			ConcreteWorkspaceViewFactory^			ViewFactory;
			ModelTabTableT^							AssociatedModels;

			bool									AllowDisposal;								// when false, the form's closing is canceled
			bool									DisallowBinding;							// when true, prevents models from binding to the view

			IWorkspaceModelController^ 	ModelController();							// yeah, not nice
			IWorkspaceModelFactory^		ModelFactory();								// not nice at all...

			void						AssociateModel(IWorkspaceModel^ Model, bool Bind);			// assign model a tab item and add it to the table
			void						DissociateModel(IWorkspaceModel^ Model, bool Destroy);		// remove model from table and dissociate from the view
			IWorkspaceModel^			GetModel(DotNetBar::SuperTabItem^ Tab);
			bool						IsModelAssociated(IWorkspaceModel^ Model);
			IWorkspaceModel^			GetActiveModel();
			IWorkspaceModel^			GetModel(String^ Description);

			DotNetBar::SuperTabItem^ GetTab(IWorkspaceModel^ Model);
			DotNetBar::SuperTabItem^ GetMouseOverTab();
			DotNetBar::SuperTabItem^ GetActiveTab();

			Rectangle GetBounds(bool UseRestoreBounds);
			void ToggleSecondaryPanel(bool State);
			void UpdateScriptTypeControls(IWorkspaceModel::ScriptType Type);

			void ShowMessageList();
			void ShowFindResultList();
			void ShowBookmarkList();
			void ShowGlobalFindResultList();
			void ToggleMessageList(bool State);
			void ToggleBookmarkList(bool State);
			void ToggleFindResultList(bool State);
			void ToggleGlobalFindResultList(bool State);
			void HideAllLists();

			void ShowOpenDialog();
			void ShowDeleteDialog();
			void ShowFindReplaceDialog(bool PerformSearch);

			void GotoLine();
			void GotoOffset();

			void SaveAll();
			void CloseAll();

			void DumpAllToDisk(String^ OutputDirectory, String^ FileExtension);
			void LoadAllFromDisk(array<String^>^ FilePaths);

			void BeginUpdate();
			void EndUpdate();
			void Redraw();
			void Focus();

			int	GetTabCount();
			void SelectTab(DotNetBar::SuperTabItem^ Tab);
			void SelectTab(Keys Index);
			void SelectTab(int Index);
			void SelectNextTab();
			void SelectPreviousTab();

			void NewTab(NewTabOperationArgs^ E);

			ConcreteWorkspaceView(ConcreteWorkspaceViewController^ Controller, ConcreteWorkspaceViewFactory^ Factory, Rectangle Bounds);
			~ConcreteWorkspaceView();

			virtual property intellisense::IIntelliSenseInterfaceView^ IntelliSenseInterfaceView
			{
				intellisense::IIntelliSenseInterfaceView^ get() { return IntelliSenseView; }
				void set(intellisense::IIntelliSenseInterfaceView^ e) {}
			}
			virtual property textEditor::ScopeBreadcrumbManager^ BreadcrumbManager
			{
				textEditor::ScopeBreadcrumbManager^ get() { return ScopeCrumbManager; }
				void set(textEditor::ScopeBreadcrumbManager^ e) {}
			}
			virtual property IWorkspaceViewController^ Controller
			{
				IWorkspaceViewController^ get() { return ViewController; }
				void set(IWorkspaceViewController^ e) {}
			}
			virtual property IntPtr WindowHandle
			{
				IntPtr get() { return EditorForm->Handle; }
				void set(IntPtr e) {}
			}
			virtual property bool Enabled
			{
				bool get() { return WorkspaceSplitter->Panel1->Enabled; }
				void set(bool e)
				{
					AttachPanel->Enabled = e;
					WorkspaceSplitter->Panel2->Enabled = e;
				}
			}
		};
	}
}