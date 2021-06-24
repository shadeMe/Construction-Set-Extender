#pragma once
#include "IIntelliSenseInterface.h"

namespace cse
{


namespace scriptEditor
{


namespace view
{


enum class eViewRole;
interface class IScriptEditorView;


namespace components
{


// the access specifier is to disambiguate CLR enums from C++11 enums
// https://docs.microsoft.com/en-us/cpp/dotnet/how-to-define-and-consume-enums-in-cpp-cli?view=msvc-160#operators-and-enums
public enum class eComponentType
{
	None,
	Form,
	TabStrip,
	TabStripItem,
	Button,
	ComboBox,
	Label,
	ObjectListView,
	ObjectListViewColumn,
	CircularProgress,
	DockablePane,
	CrumbBar,
	CrumbBarItem,
	Container
};


interface class IForm;
interface class IButton;
interface class IComboBox;
interface class ILabel;
interface class ITabStrip;
interface class ITabStripItem;
interface class IObjectListView;
interface class IObjectListViewColumn;
interface class ICircularProgress;
interface class IDockablePane;
interface class ICrumbBar;
interface class ICrumbBarItem;
interface class IContainer;


interface class IViewComponent
{
	property eComponentType Type;
	property eViewRole Role;

	IForm^ AsForm();
	IButton^ AsButton();
	IComboBox^ AsComboBox();
	ILabel^ AsLabel();
	ITabStrip^ AsTabStrip();
	ITabStripItem^ AsTabStripItem();
	IObjectListView^ AsObjectListView();
	IObjectListViewColumn^ AsObjectListViewColumn();
	ICircularProgress^ AsCircularProgress();
	IDockablePane^ AsDockablePane();
	ICrumbBar^ AsCrumbBar();
	ICrumbBarItem^ AsCrumbBarItem();
	IContainer^ AsContainer();
};


ref struct ViewComponentEvent
{
	property IViewComponent^ Component;
	property Object^ EventType;
	property Object^ EventArgs;
};


interface class IForm : public IViewComponent
{
	property String^ Text;
	property Rectangle Bounds;

	void BeginUpdate();
	void EndUpdate();
	void Redraw();
	void Focus();
	void Close(bool Force);

	static enum class eEvent
	{
		Closing,
		KeyDown,
	};

	ref struct ClosingEventArgs
	{
		property bool Cancel;
	};

	ref struct KeyDownEventArgs
	{
		property KeyEventArgs^ KeyEvent;
	};
};


interface class IButton : public IViewComponent
{
	property String^ Text;
	property String^ ShortcutKey;
	property String^ Tooltip;
	property bool Checked;
	property bool Visible;
	property bool Enabled;

	void PerformClick();

	static enum class eEvent
	{
		Click,
		PopupOpening	// raised for each button in the popup menu
	};

	ref struct ClickEventArgs {};

	ref struct PopupOpeningEventArgs {};
};


interface class IComboBox : public IViewComponent
{
	property Object^ Selection;
	property String^ Text;
	property IEnumerable<Object^>^ Items;
	property bool Enabled;

	void AddDropdownItem(Object^ NewItem, bool AtFirstPosition);
	void ClearDropdownItems();
	Object^ LookupDropdownItem(String^ DropdownItemText);

	static enum class eEvent
	{
		SelectionChanged,
		KeyDown,
	};

	ref struct SelectionChangedEventArgs
	{
		property Object^ SelectedItem;
	};

	ref struct KeyDownEventArgs
	{
		property KeyEventArgs^ KeyEvent;
	};
};


interface class ILabel : public IViewComponent
{
	property String^ Text;
	property bool Visible;
};


interface class ITabStripItem : public IViewComponent
{
	property String^ Text;
	property String^ Tooltip;
	property Image^ Image;
	property Object^ Tag;
	property ITabStrip^ Parent;
};


interface class ITabStrip
{
	property ITabStripItem^ ActiveTab;
	property UInt32 TabCount;

	ITabStripItem^ AllocateNewTab();
	void AddTab(ITabStripItem^ Tab);
	void RemoveTab(ITabStripItem^ Tab);
	void SelectNextTab();
	void SelectPreviousTab();
	ITabStripItem^ LookupTabByTag(Object^ Tag);

	static enum class eEvent
	{
		TabClick,
		TabMoving,
		TabClosing,
		ActiveTabChanged,
	};

	ref struct TabClickEventArgs
	{
		property MouseEventArgs^ MouseEvent;
		property ITabStripItem^ MouseOverTab;
	};

	ref struct TabMovingEventArgs
	{
		property ITabStripItem^ Tab;
	};

	ref struct TabClosingEventArgs
	{
		property bool Cancel;
		property ITabStripItem^ Tab;
	};

	ref struct ActiveTabChangedEventArgs
	{
		property ITabStripItem^ OldValue;
		property ITabStripItem^ NewValue;
	};
};


interface class IObjectListViewColumn : public IViewComponent
{
	delegate Object^ AspectGetter(Object^ Model);
	delegate String^ AspectToStringGetter(Object^ Aspect);
	delegate Object^ ImageGetter(Object^ Model);

	property int Index;
	property String^ Text;
	property IObjectListView^ Parent;
	property int MinimumWidth;
	property int MaximumWidth;
	property int Width;
	property bool FillsFreeSpace;

	void SetAspectGetter(AspectGetter^ Delegate);
	void SetAspectToStringGetter(AspectToStringGetter^ Delegate);
	void SetImageGetter(ImageGetter^ Delegate);
};


interface class IObjectListView : public IViewComponent
{
	delegate bool CanExpandGetter(Object^ Model);
	delegate Collections::IEnumerable^ ChildrenGetter(Object^ Model);

	property Object^ SelectedObject;
	property Collections::IList^ SelectedObjects;

	void SetObjects(Collections::IEnumerable^ Collection, bool PreserveState);
	IObjectListViewColumn^ AllocateNewColumn();
	void AddColumn(IObjectListViewColumn^ Column);
	List<IObjectListViewColumn^>^ GetColumns();
	void SetCanExpandGetter(CanExpandGetter^ Delegate);
	void SetChildrenGetter(ChildrenGetter^ Delegate);
	void EnsureItemVisible(Object^ Item);
	void ExpandAll();
	void CollapseAll();

	static enum class eEvent
	{
		ItemActivate,
	};

	ref struct ItemActivateEventArgs
	{
		property Object^ ItemModel;
		property Object^ ParentItemModel;
	};
};


interface class ICircularProgress : public IViewComponent
{
	property String^ Text;
	property int Value;
	property int Minimum;
	property int Maximum;
};


interface class IDockablePane : public IViewComponent
{
	property bool Visible;

	void Focus();
};


interface class ICrumbBarItem : public IViewComponent
{
	property String^ Text;
	property Object^ Tag;
	property String^ Tooltip;
	property Image^ Image;

	void AddChild(ICrumbBarItem^ Child);
	void ClearChildren();
};


interface class ICrumbBar : public IViewComponent
{
	property ICrumbBarItem^ SelectedItem;
	property bool Visible;

	ICrumbBarItem^ AllocateNewItem();
	void ClearItems();
	void AddItem(ICrumbBarItem^ Item);

	static enum class eEvent
	{
		ItemClick,
	};

	ref struct ItemClickEventArgs
	{
		property ICrumbBarItem^ Item;
	};
};


interface class IContainer : public IViewComponent
{
	void AddControl(Control^ Control);
	void RemoveControl(Control^ Control);
};


interface class IScriptSelectionDialog
{
	ref struct Params
	{
		property String^ SelectedScriptEditorID;
		property String^ FilterString;
		property bool ShowDeletedScripts;
		property bool PreventSyncedScriptSelection;

		Params();
	};

	ref struct Result
	{
		property List<String^>^ SelectedScriptEditorIDs;
		property UInt32 SelectionCount
		{
			UInt32 get() { return SelectedScriptEditorIDs->Count; }
		}

		Result();
	};

	Result^ SelectScripts(Params^ SelectionParams);
};


ref class CommonIcons
{
	static CommonIcons^ Singleton = gcnew CommonIcons();

	ImageResourceManager^ IconResources;

	CommonIcons();
public:
	property Image^ Transparent;
	property Image^ UnsavedChanges;
	property Image^ Info;
	property Image^ Warning;
	property Image^ Error;
	property Image^ Invalid;
	property Image^ Success;

	static CommonIcons^ Get();
};


} // namespace components


public enum class eViewRole
{
	None,
	MainWindow,

	MainTabStrip,
	MainTabStrip_NewTab,
	MainTabStrip_NewTab_NewScript,
	MainTabStrip_NewTab_ExistingScript,

	MainToolbar_NewScript,
	MainToolbar_OpenScript,
	MainToolbar_SaveScript,
	MainToolbar_SaveScriptAndActivePlugin,
	MainToolbar_SaveScriptNoCompile,
	MainToolbar_PreviousScript,
	MainToolbar_NextScript,
	MainToolbar_SaveAllScripts,
	MainToolbar_ScriptTypeDropdown,

	MainToolbar_Edit,
	MainToolbar_Edit_FindReplace,
	MainToolbar_Edit_GoToLine,
	MainToolbar_Edit_AddBookmark,
	MainToolbar_Edit_Comment,
	MainToolbar_Edit_Uncomment,

	MainToolbar_View,
	MainToolbar_View_PreprocessorOutput,
	MainToolbar_View_BytecodeOffsets,
	MainToolbar_View_IconMargin,
	MainToolbar_View_Messages,
	MainToolbar_View_Bookmarks,
	MainToolbar_View_OutlineView,
	MainToolbar_View_FindReplaceResults,
	MainToolbar_View_FindInTabsResults,
	MainToolbar_View_NavigationBar,
	MainToolbar_View_DarkMode,

	MainToolbar_Tools,
	MainToolbar_Tools_SanitiseScript,
	MainToolbar_Tools_AttachScript,
	MainToolbar_Tools_RecompileScriptDependencies,
	MainToolbar_Tools_DocumentScript,
	MainToolbar_Tools_ModifyVariableIndices,

	MainToolbar_Tools_Import_IntoCurrentScript,
	MainToolbar_Tools_Import_IntoTabs,

	MainToolbar_Tools_Export_CurrentScript,
	MainToolbar_Tools_Export_AllOpenScripts,

	MainToolbar_Tools_RecompileAllActiveScripts,
	MainToolbar_Tools_DeleteScripts,
	MainToolbar_Tools_SyncScripts,
	MainToolbar_Tools_CodeSnippets,
	MainToolbar_Tools_Preferences,

	MainToolbar_Help_Wiki,
	MainToolbar_Help_OBSE,

	StatusBar_CurrentMessage,
	StatusBar_LineNumber,
	StatusBar_ColumnNumber,
	StatusBar_PreprocessorOutput,
	StatusBar_CompiledScriptSize,

	FindReplace_DockPanel,
	FindReplace_FindDropdown,
	FindReplace_ReplaceDropdown,
	FindReplace_LookInDropdown,
	FindReplace_MatchCase,
	FindReplace_MatchWholeWord,
	FindReplace_UseRegEx,
	FindReplace_IgnoreComments,
	FindReplace_FindButton,
	FindReplace_ReplaceButton,
	FindReplace_CountMatchesButton,

	Messages_DockPanel,
	Messages_ListView,

	Bookmarks_DockPanel,
	Bookmarks_ListView,
	Bookmarks_Toolbar_AddBookmark,
	Bookmarks_Toolbar_RemoveBookmark,

	FindReplaceResults_DockPanel,
	FindReplaceResults_Query,
	FindReplaceResults_ListView,

	GlobalFindReplaceResults_DockPanel,
	GlobalFindReplaceResults_TreeView,

	OutlineView_DockPanel,
	OutlineView_TreeView,

	NavigationBar,
	TextEditor_ViewPortContainer,

	TextEditor_ContextMenu,
	TextEditor_ContextMenu_Copy,
	TextEditor_ContextMenu_Paste,

	TextEditor_ContextMenu_AddVar_Integer,
	TextEditor_ContextMenu_AddVar_Float,
	TextEditor_ContextMenu_AddVar_Reference,
	TextEditor_ContextMenu_AddVar_String,
	TextEditor_ContextMenu_AddVar_Array,

	TextEditor_ContextMenu_OpenPreprocessorImportFile,
	TextEditor_ContextMenu_JumpToAttachedScript,
};


using namespace components;

interface class IScriptEditorView
{
	property intellisense::IIntelliSenseInterfaceView^ IntelliSenseView;
	property IntPtr WindowHandle;

	delegate void EventHandler(IScriptEditorView^ Sender, ViewComponentEvent^ E);

	event EventHandler^ ComponentEvent;

	IViewComponent^ GetComponentByRole(eViewRole Role);
	void ShowNotification(String^ Message, Image^ Image, int DurationInMs);
	DialogResult ShowMessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon);
	DialogResult ShowInputPrompt(String^ Prompt, String^ Title, String^% OutText);
	List<String^>^ SelectExistingScripts(String^ DefaultSelectionEditorId);	// returns a list of script editorIDs

	void Reveal();
	void BeginUpdate();
	void EndUpdate();
};


interface class IFactory
{
	IScriptEditorView^ NewView(Rectangle InitialBounds);
};


} // namespace view


} // namespace scriptEditor


} // namespace cse