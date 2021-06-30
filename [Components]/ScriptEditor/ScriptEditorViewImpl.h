#pragma once
#include "ScriptEditorViewImplComponents.h"
#include "IntelliSenseInterfaceView.h"

namespace cse
{


namespace scriptEditor
{


namespace viewImpl
{


using namespace cse::scriptEditor::view;
using namespace DevComponents;
using namespace DevComponents::DotNetBar;
using namespace DevComponents::DotNetBar::Controls;
using namespace DevComponents::DotNetBar::Metro;
using namespace DevComponents::DotNetBar::Events;
using namespace System::Windows;
using namespace System::Drawing;
using namespace System;


ref class ScriptEditorWorkspace : public MetroForm, IScriptEditorView
{
	DevComponents::DotNetBar::StyleManager^ StyleManager;
	DevComponents::DotNetBar::SuperTabControl^ MainTabStrip;
	DevComponents::DotNetBar::Bar^ ContainerMainToolbar;
	DevComponents::DotNetBar::ButtonItem^ ToolbarNewScript;
	DevComponents::DotNetBar::ButtonItem^ ToolbarOpenScript;
	DevComponents::DotNetBar::ButtonItem^ ToolbarSaveScript;
	DevComponents::DotNetBar::ButtonItem^ ToolbarSaveScriptAndActivePlugin;
	DevComponents::DotNetBar::ButtonItem^ ToolbarSaveScriptNoCompile;
	DevComponents::DotNetBar::ButtonItem^ ToolbarPreviousScript;
	DevComponents::DotNetBar::ButtonItem^ ToolbarNextScript;
	DevComponents::DotNetBar::ButtonItem^ ToolbarSaveAllScripts;
	DevComponents::DotNetBar::LabelItem^ ToolbarLabelScriptType;
	DevComponents::DotNetBar::ComboBoxItem^ ToolbarScriptTypeDropdown;
	DevComponents::DotNetBar::DotNetBarManager^ DockManager;
	DevComponents::DotNetBar::DockSite^ DockSiteBottom;
	DevComponents::DotNetBar::DockSite^ DockSiteLeftEx;
	DevComponents::DotNetBar::DockSite^ DockSiteRightEx;
	DevComponents::DotNetBar::DockSite^ DockSiteTop;
	DevComponents::DotNetBar::DockSite^ DockSiteLeft;
	DevComponents::DotNetBar::DockSite^ DockSiteRight;
	DevComponents::DotNetBar::DockSite^ DockSiteTopEx;
	DevComponents::DotNetBar::DockSite^ DockSiteBottomEx;
	DevComponents::Editors::ComboItem^ DropdownScriptTypeObject;
	DevComponents::Editors::ComboItem^ DropdownScriptTypeQuest;
	DevComponents::Editors::ComboItem^ DropdownScriptTypeMagicEffect;
	DevComponents::DotNetBar::ButtonItem^ ToolbarMenuEdit;
	DevComponents::DotNetBar::ButtonItem^ EditMenuFindReplace;
	DevComponents::DotNetBar::ButtonItem^ EditMenuGoToLine;
	DevComponents::DotNetBar::ButtonItem^ EditMenuAddBookmark;
	DevComponents::DotNetBar::ButtonItem^ ToolbarMenuView;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuPreprocessorOutput;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuBytecodeOffsets;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuIconMargin;
	DevComponents::DotNetBar::ButtonItem^ ToolbarMenuTools;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuSanitiseScript;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuAttachScript;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuCompileDepends;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuDocumentScript;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuModifyVarIndices;
	DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuAddVar;
	DevComponents::DotNetBar::ButtonItem^ MenuAddVarInteger;
	DevComponents::DotNetBar::ButtonItem^ MenuAddVarFloat;
	DevComponents::DotNetBar::ButtonItem^ MenuAddVarReference;
	DevComponents::DotNetBar::ButtonItem^ MenuAddVarString;
	DevComponents::DotNetBar::ButtonItem^ MenuAddVarArray;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuImport;
	DevComponents::DotNetBar::ButtonItem^ MenuImportCurrentTab;
	DevComponents::DotNetBar::ButtonItem^ MenuImportMultipleTabs;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuExport;
	DevComponents::DotNetBar::ButtonItem^ MenuExportCurrentTab;
	DevComponents::DotNetBar::ButtonItem^ MenuExportAllTabs;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuCompileActiveFileScripts;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuDarkMode;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuMessages;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuFindResults;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuBookmarks;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuGlobalFindResults;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuOutline;
	DevComponents::DotNetBar::ButtonItem^ ViewMenuNavBreadcrumb;
	DevComponents::DotNetBar::Bar^ DockableBarFindInTabsResults;
	DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerBookmarks;
	BrightIdeasSoftware::FastObjectListView^ BookmarksList;
	DevComponents::DotNetBar::Bar^ BookmarksToolbar;
	DevComponents::DotNetBar::ButtonItem^ BookmarksToolbarAdd;
	DevComponents::DotNetBar::ButtonItem^ BookmarksToolbarRemove;
	DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerMessageList;
	BrightIdeasSoftware::FastObjectListView^ MessagesList;
	DevComponents::DotNetBar::DockContainerItem^ DockContainerItemMessageList;
	DevComponents::DotNetBar::DockContainerItem^ DockContainerItemBookmarks;
	DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerFindResults;
	BrightIdeasSoftware::FastObjectListView^ FindResultsList;
	DevComponents::DotNetBar::Bar^ FindResultsToolbar;
	DevComponents::DotNetBar::LabelItem^ FindResultsToolbarLabel;
	DevComponents::DotNetBar::LabelItem^ FindResultsListToolbarLabelQuery;
	DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerGlobalFindResults;
	BrightIdeasSoftware::TreeListView^ GlobalFindResultsList;
	DevComponents::DotNetBar::DockContainerItem^ DockContainerItemFindResults;
	DevComponents::DotNetBar::DockContainerItem^ DockContainerItemGlobalFindResults;
	DevComponents::DotNetBar::Bar^ DockableBarFindReplace;
	DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerOutline;
	DevComponents::DotNetBar::DockContainerItem^ DockContainerItemOutline;
	DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerFindReplace;
	DevComponents::DotNetBar::DockContainerItem^ DockContainerItemFindReplace;
	BrightIdeasSoftware::TreeListView^ OutlineTreeView;
	DevComponents::DotNetBar::Layout::LayoutControl^ FindWindowLayoutControl;
	DevComponents::DotNetBar::Controls::ComboBoxEx^ FindWindowDropdownFind;
	DevComponents::DotNetBar::Controls::ComboBoxEx^ FindWindowDropdownReplace;
	DevComponents::DotNetBar::Controls::CheckBoxX^ FindWindowCheckboxMatchCase;
	DevComponents::DotNetBar::Controls::CheckBoxX^ FindWindowCheckboxUseRegEx;
	DevComponents::DotNetBar::Controls::CheckBoxX^ FindWindowCheckBoxMatchWholeWord;
	DevComponents::DotNetBar::Controls::ComboBoxEx^ FindWindowComboLookIn;
	DevComponents::DotNetBar::ButtonX^ FindWindowButtonFind;
	DevComponents::DotNetBar::ButtonX^ FindWindowButtonReplace;
	DevComponents::DotNetBar::ButtonX^ FindWindowButtonCountMatches;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIFindDropdown;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIReplaceDropdown;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCILookIn;
	DevComponents::DotNetBar::Layout::LayoutGroup^ FindWindowLayoutGroupSettings;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIMatchCase;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIMatchWholeWord;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIUseRegEx;
	DevComponents::DotNetBar::Layout::LayoutGroup^ FindWindowLayouyGroupButtons;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIFindButton;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIReplaceButton;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCICountMatches;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuDeleteScripts;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuScriptSync;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuCodeSnippets;
	DevComponents::DotNetBar::ButtonItem^ ToolsMenuPreferences;
	DevComponents::DotNetBar::Bar^ StatusBar;
	DevComponents::DotNetBar::LabelItem^ StatusBarPreprocessorOutputFlag;
	DevComponents::DotNetBar::DockSite^ DockSiteCenter;
	DevComponents::DotNetBar::Bar^ CenterDockBar;
	System::Windows::Forms::Panel^ PanelDocumentContent;
	DevComponents::DotNetBar::CrumbBar^ NavigationBar;
	DevComponents::DotNetBar::SuperTabControlPanel^ DummySuperTabControlPanel2;
	DevComponents::DotNetBar::SuperTabItem^ AnotherDummyTabItem;
	DevComponents::DotNetBar::SuperTabControlPanel^ DummySuperTabControlPanel1;
	DevComponents::DotNetBar::SuperTabItem^ DummyTabItem;
	DevComponents::DotNetBar::LabelItem^ StatusBarLineNumber;
	DevComponents::DotNetBar::LabelItem^ StatusBarColumnNumber;
	DevComponents::DotNetBar::CircularProgressItem^ StatusBarScriptBytecodeLength;
	DevComponents::DotNetBar::ContextMenuBar^ ContextMenuProvider;
	DevComponents::DotNetBar::ButtonItem^ ContextMenuTextEditor;
	DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuCopy;
	DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuPaste;
	DevComponents::DotNetBar::ButtonItem^ ToolbarMenuHelp;
	DevComponents::DotNetBar::ButtonItem^ HelpMenuWiki;
	DevComponents::DotNetBar::ButtonItem^ HelpMenuObseDocs;
	DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuJumpToScript;
	DevComponents::DotNetBar::Bar^ DockableBarOutlineView;
	DevComponents::DotNetBar::Bar^ DockableBarMessages;
	DevComponents::DotNetBar::Bar^ DockableBarBookmarks;
	DevComponents::DotNetBar::Bar^ DockableBarFindReplaceResults;
	DevComponents::DotNetBar::Controls::CheckBoxX^ FindWindowCheckBoxIgnoreComments;
	DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIIgnoreComments;
	DevComponents::DotNetBar::ButtonItem^ EditMenuComment;
	DevComponents::DotNetBar::ButtonItem^ EditMenuUncomment;
	System::ComponentModel::IContainer^ components;

	ref struct ViewComponentData
	{
		Object^ Source;
		components::ViewComponent^ ViewComponent;
	};

	Dictionary<eViewRole, ViewComponentData^>^ ViewComponents;
	components::ViewComponentEventRaiser^ DelegateViewComponentEventRouter;
	EventHandler^ DelegatePreferencesChanged;
	intellisense::IIntelliSenseInterfaceView^ IntelliSenseInterface;
	bool SkipViewComponentEventProcessing;

	void HandleViewComponentEvent(ViewComponentEvent^ E);
	void HandlePreferencesChanged(Object^ Sender, EventArgs^ E);

	void InitializeComponents();
	void FinalizeComponents();
	void InitializeViewComponents();
	void DeinitializeViewComponents();

	void RegisterViewComponent(components::ViewComponent^ ViewComponent, Object^ Source, eViewRole Role);
	void SetupViewComponentForm(Forms::Form^ Source);
	void SetupViewComponentTabStrip(SuperTabControl^ Source);
	void SetupViewComponentButton(ButtonItem^ Source, eViewRole Role);
	void SetupViewComponentButton(ButtonX^ Source, eViewRole Role);
	void SetupViewComponentButton(CheckBoxX^ Source, eViewRole Role);
	void SetupViewComponentComboBox(ComboBoxItem^ Source, eViewRole Role);
	void SetupViewComponentComboBox(ComboBoxEx^ Source, eViewRole Role);
	void SetupViewComponentLabel(LabelItem^ Source, eViewRole Role);
	void SetupViewComponentObjectListView(BrightIdeasSoftware::ObjectListView^ Source, eViewRole Role);
	void SetupViewComponentProgressBar(DotNetBar::CircularProgressItem^ Source, eViewRole Role);
	void SetupViewComponentDockablePane(DockContainerItem^ Source, eViewRole Role);
	void SetupViewComponentCrumbBar(DotNetBar::CrumbBar^ Source);
	void SetupViewComponentContainer(Control^ Source, eViewRole Role);
public:
	ScriptEditorWorkspace();
	virtual ~ScriptEditorWorkspace();

	ImplPropertyGetOnly(intellisense::IIntelliSenseInterfaceView^, IntelliSenseView, IntelliSenseInterface);
	ImplPropertyGetOnly(IntPtr, WindowHandle, this->Handle);

	virtual event IScriptEditorView::EventHandler^ ComponentEvent;

	virtual IViewComponent^ GetComponentByRole(eViewRole Role);
	virtual void ShowNotification(String^ Message, Image^ Image, int DurationInMs);
	virtual Forms::DialogResult ShowMessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon);
	virtual Forms::DialogResult ShowInputPrompt(String^ Prompt, String^ Title, String^% OutText);
	virtual Forms::DialogResult ShowInputPrompt(String^ Prompt, String^ Title, String^ DefaultValue, String^% OutText);
	virtual List<String^>^ SelectExistingScripts(String^ DefaultSelectionEditorId);
	virtual void Reveal(Rectangle InitialBounds);
	virtual void BeginUpdate();
	virtual void EndUpdate();
};


ref struct ScriptEditorViewFactory : public view::IFactory
{
public:
	virtual view::IScriptEditorView^ NewView();

	static ScriptEditorViewFactory^ NewFactory();
};


} // namespace viewImpl


} // namespace scriptEditor


} // namespace cse