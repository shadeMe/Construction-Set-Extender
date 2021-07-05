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
	ProgressBar,
	DockablePane,
	CrumbBar,
	CrumbBarItem,
	Container,
	ContextMenu
};


interface class IForm;
interface class IButton;
interface class IComboBox;
interface class ILabel;
interface class ITabStrip;
interface class ITabStripItem;
interface class IObjectListView;
interface class IObjectListViewColumn;
interface class IProgressBar;
interface class IDockablePane;
interface class ICrumbBar;
interface class ICrumbBarItem;
interface class IContainer;
interface class IContextMenu;


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
	IProgressBar^ AsProgressBar();
	IDockablePane^ AsDockablePane();
	ICrumbBar^ AsCrumbBar();
	ICrumbBarItem^ AsCrumbBarItem();
	IContainer^ AsContainer();
	IContextMenu^ AsContextMenu();
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
	property Object^ Tag;
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

	ref struct PopupOpeningEventArgs
	{
		bool Cancel;
	};
};


interface class IComboBox : public IViewComponent
{
	property Object^ Selection;
	property String^ Text;
	property Collections::IEnumerable^ Items;
	property bool Enabled;

	void AddDropdownItem(Object^ NewItem, bool AtFirstPosition);
	void ClearDropdownItems();
	Object^ LookupDropdownItem(String^ DropdownItemText);
	void Focus();

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
		TabMoved,
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

	ref struct TabMovedEventArgs
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
	property bool HeaderVisible;

	void SetObjects(Collections::IEnumerable^ Collection, bool PreserveState);
	void ClearObjects();
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


interface class IProgressBar : public IViewComponent
{
	property String^ Text;
	property String^ Tooltip;
	property int Value;
	property int Minimum;
	property int Maximum;
	property bool Visible;
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
	property bool Visible;

	void AddControl(Control^ Control);
	void RemoveControl(Control^ Control);
	void Invalidate();
};


interface class IContextMenu : public IViewComponent
{
	// context menus will raise the IButton::eEvent::PopupOpening once when they are about to open
	// the event subscriber should - just for this single invocation - interpret the component as an IButton

	void Show(Drawing::Point ScreenCoords);
	void Hide();
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
	property Image^ InfoLarge;
	property Image^ Warning;
	property Image^ WarningLarge;
	property Image^ Error;
	property Image^ ErrorLarge;
	property Image^ BlockedLarge;
	property Image^ Invalid;
	property Image^ Success;
	property Image^ SuccessLarge;
	property Image^ InProgress;

	property ImageResourceManager^ ResourceManager
	{
		ImageResourceManager^ get() { return IconResources; }
	}

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
	MainToolbar_View_PreprocessorOutputAndBytecodeOffsets,
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

	StatusBar,
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

	TextEditor_ContextMenu_JumpToAttachedScript,

	EmptyWorkspacePanel,
	EmptyWorkspacePanel_NewScript,
	EmptyWorkspacePanel_OpenScript,
};


using namespace components;

interface class IScriptEditorView
{
	static property String^ MainWindowDefaultTitle
	{
		String^ get() { return "CSE Script Editor"; }
	}

	property intellisense::IIntelliSenseInterfaceView^ IntelliSenseView;
	property IntPtr WindowHandle;

	delegate void EventHandler(Object^ Sender, ViewComponentEvent^ E);

	event EventHandler^ ComponentEvent;

	IViewComponent^ GetComponentByRole(eViewRole Role);
	void ShowNotification(String^ Message, Image^ Image, int DurationInMs);
	DialogResult ShowMessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon);
	DialogResult ShowInputPrompt(String^ Prompt, String^ Title, String^% OutText);
	DialogResult ShowInputPrompt(String^ Prompt, String^ Title, String^ DefaultValue, String^% OutText);
	List<String^>^ SelectExistingScripts(String^ DefaultSelectionEditorId);	// returns a list of script editorIDs

	void Reveal(Rectangle InitialBounds);
	void BeginUpdate();
	void EndUpdate();
};


interface class IFactory
{
	IScriptEditorView^ NewView();
};


} // namespace view


} // namespace scriptEditor


} // namespace cse