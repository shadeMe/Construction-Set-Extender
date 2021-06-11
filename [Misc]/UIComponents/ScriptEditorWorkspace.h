#pragma once
#include "ScriptEditorSelectScripts.h"
#include "ScriptEditorScriptSync.h"
#include "ScriptEditorPreferences.h"

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace DevComponents::DotNetBar;

	/// <summary>
	/// Summary for ScriptEditorWorkspace
	/// </summary>
	public ref class ScriptEditorWorkspace : public Metro::MetroForm
	{
	public:
		ScriptEditorWorkspace(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~ScriptEditorWorkspace()
		{
			if (components)
			{
				delete components;
			}
		}
	private: DevComponents::DotNetBar::StyleManager^ StyleManager;
	protected:
	private: DevComponents::DotNetBar::SuperTabControl^ MainTabStrip;


	private: DevComponents::DotNetBar::Bar^ ContainerMainToolbar;

	private: DevComponents::DotNetBar::ButtonItem^ ToolbarNewScript;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarOpenScript;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarSaveScript;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarSaveScriptAndActivePlugin;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarSaveScriptNoCompile;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarPreviousScript;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarNextScript;

	private: DevComponents::DotNetBar::ButtonItem^ ToolbarSaveAllScripts;
	private: DevComponents::DotNetBar::LabelItem^ ToolbarLabelScriptType;
	private: DevComponents::DotNetBar::ComboBoxItem^ ToolbarScriptTypeDropdown;
	private: DevComponents::DotNetBar::DotNetBarManager^ DockManager;
	private: DevComponents::DotNetBar::DockSite^ DockSiteBottom;
	private: DevComponents::DotNetBar::DockSite^ DockSiteLeftEx;
	private: DevComponents::DotNetBar::DockSite^ DockSiteRightEx;



	private: DevComponents::DotNetBar::DockSite^ DockSiteTop;
	private: DevComponents::DotNetBar::DockSite^ DockSiteLeft;
	private: DevComponents::DotNetBar::DockSite^ DockSiteRight;
	private: DevComponents::DotNetBar::DockSite^ DockSiteTopEx;
	private: DevComponents::DotNetBar::DockSite^ DockSiteBottomEx;








	private: DevComponents::Editors::ComboItem^ DropdownScriptTypeObject;
	private: DevComponents::Editors::ComboItem^ DropdownScriptTypeQuest;
	private: DevComponents::Editors::ComboItem^ DropdownScriptTypeMagicEffect;



	private: DevComponents::DotNetBar::ButtonItem^ ToolbarMenuEdit;
	private: DevComponents::DotNetBar::ButtonItem^ EditMenuFindReplace;
	private: DevComponents::DotNetBar::ButtonItem^ EditMenuGoToLine;
	private: DevComponents::DotNetBar::ButtonItem^ EditMenuGoToOffset;
	private: DevComponents::DotNetBar::ButtonItem^ EditMenuAddBookmark;
	private: DevComponents::DotNetBar::ButtonItem^ EditMenuToggleComment;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarMenuView;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuPreprocessorOutput;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuBytecodeOffsets;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuIconMargin;
	private: DevComponents::DotNetBar::ButtonItem^ ToolbarMenuTools;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuSanitiseScript;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuAttachScript;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuCompileDepends;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuDocumentScript;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuRenameVars;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuModifyVarIndices;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuAddVar;
	private: DevComponents::DotNetBar::ButtonItem^ MenuAddVarInteger;
	private: DevComponents::DotNetBar::ButtonItem^ MenuAddVarFloat;
	private: DevComponents::DotNetBar::ButtonItem^ MenuAddVarReference;
	private: DevComponents::DotNetBar::ButtonItem^ MenuAddVarString;
	private: DevComponents::DotNetBar::ButtonItem^ MenuAddVarArray;





	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuImport;
	private: DevComponents::DotNetBar::ButtonItem^ MenuImportCurrentTab;
	private: DevComponents::DotNetBar::ButtonItem^ MenuImportMultipleTabs;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuExport;
	private: DevComponents::DotNetBar::ButtonItem^ MenuExportCurrentTab;
	private: DevComponents::DotNetBar::ButtonItem^ MenuExportAllTabs;
	private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuCompileActiveFileScripts;


	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuDarkMode;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuMessages;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuFindResults;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuBookmarks;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuGlobalFindResults;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuOutline;
	private: DevComponents::DotNetBar::ButtonItem^ ViewMenuNavBreadcrumb;
private: DevComponents::DotNetBar::Bar^ BottomDockBar;
private: DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerBookmarks;
private: BrightIdeasSoftware::FastObjectListView^ BookmarksList;
private: BrightIdeasSoftware::OLVColumn^ BookmarksColumnLine;
private: BrightIdeasSoftware::OLVColumn^ BookmarksColumnText;
private: DevComponents::DotNetBar::Bar^ BookmarksToolbar;
private: DevComponents::DotNetBar::ButtonItem^ BookmarksToolbarAdd;
private: DevComponents::DotNetBar::ButtonItem^ BookmarksToolbarRemove;
private: DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerMessageList;
private: BrightIdeasSoftware::FastObjectListView^ MessagesList;
private: BrightIdeasSoftware::OLVColumn^ MessageListColumnType;
private: BrightIdeasSoftware::OLVColumn^ MessageListColumnLine;
private: BrightIdeasSoftware::OLVColumn^ MessageListColumnText;
private: BrightIdeasSoftware::OLVColumn^ MessageListColumnSource;
private: DevComponents::DotNetBar::DockContainerItem^ DockContainerItemMessageList;
private: DevComponents::DotNetBar::DockContainerItem^ DockContainerItemBookmarks;
private: DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerFindResults;
private: BrightIdeasSoftware::FastObjectListView^ FindResultsList;
private: BrightIdeasSoftware::OLVColumn^ FindResultsListColumnLine;
private: BrightIdeasSoftware::OLVColumn^ FindResultsListColumnText;
private: BrightIdeasSoftware::OLVColumn^ FindResultsListColumnHits;
private: DevComponents::DotNetBar::Bar^ FindResultsToolbar;
private: DevComponents::DotNetBar::LabelItem^ FindResultsToolbarLabel;
private: DevComponents::DotNetBar::LabelItem^ FindResultsListToolbarLabelQuery;
private: DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerGlobalFindResults;
private: BrightIdeasSoftware::TreeListView^ GlobalFindResultsList;
private: BrightIdeasSoftware::OLVColumn^ GlobalFindResultsListColumnCode;
private: BrightIdeasSoftware::OLVColumn^ GlobalFindResultsListColumnLine;
private: BrightIdeasSoftware::OLVColumn^ GlobalFindResultsListColumnHits;
private: DevComponents::DotNetBar::DockContainerItem^ DockContainerItemFindResults;
private: DevComponents::DotNetBar::DockContainerItem^ DockContainerItemGlobalFindResults;
private: DevComponents::DotNetBar::Bar^ RightDockBar;



private: DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerOutline;
private: DevComponents::DotNetBar::DockContainerItem^ DockContainerItemOutline;

private: DevComponents::DotNetBar::PanelDockContainer^ PanelDockContainerFindReplace;
private: DevComponents::DotNetBar::DockContainerItem^ DockContainerItemFindReplace;



















private: BrightIdeasSoftware::TreeListView^ OutlineTreeView;
private: DevComponents::DotNetBar::Layout::LayoutControl^ FindWindowLayoutControl;

private: DevComponents::DotNetBar::Controls::TextBoxDropDown^ FindWindowDropdownFind;
private: DevComponents::DotNetBar::Controls::TextBoxDropDown^ FindWindowDropdownReplace;
private: DevComponents::DotNetBar::Controls::CheckBoxX^ FindWindowCheckboxMatchCase;
private: DevComponents::DotNetBar::Controls::CheckBoxX^ FindWindowCheckboxUseRegEx;
private: DevComponents::DotNetBar::Controls::CheckBoxX^ FindWindowCheckBoxMatchWholeWord;
private: DevComponents::DotNetBar::Controls::ComboBoxEx^ FindWindowComboLookIn;
private: DevComponents::Editors::ComboItem^ FindWindowDropdownLookInSelection;
private: DevComponents::Editors::ComboItem^ FindWindowDropdownLookInCurrentScript;
private: DevComponents::Editors::ComboItem^ FindWindowDropdownLookInAllOpenScripts;
private: DevComponents::DotNetBar::ButtonX^ FindWindowButtonFind;
private: DevComponents::DotNetBar::ButtonX^ FindWindowButtonReplace;
private: DevComponents::DotNetBar::ButtonX^ FindWindowButtonCountMatches;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIFindDropdown;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIReplaceDropdown;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCILookIn;
private: DevComponents::DotNetBar::Layout::LayoutGroup^ FindWindowLayoutGroupSettings;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIMatchCase;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIMatchWholeWord;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIUseRegEx;
private: DevComponents::DotNetBar::Layout::LayoutGroup^ FindWindowLayouyGroupButtons;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIFindButton;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCIReplaceButton;
private: DevComponents::DotNetBar::Layout::LayoutControlItem^ FindWindowLCICountMatches;
private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuDeleteScripts;
private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuScriptSync;
private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuCodeSnippets;
private: DevComponents::DotNetBar::ButtonItem^ ToolsMenuPreferences;
private: DevComponents::DotNetBar::Bar^ StatusBar;
private: DevComponents::DotNetBar::LabelItem^ StatusBarPreprocessorOutputFlag;
private: DevComponents::DotNetBar::DockSite^ DockSiteCenter;
private: DevComponents::DotNetBar::Bar^ CenterDockBar;
private: System::Windows::Forms::Panel^ PanelDocumentContent;
private: DevComponents::DotNetBar::CrumbBar^ NavigationBar;
private: DevComponents::DotNetBar::SuperTabControlPanel^ superTabControlPanel2;
private: DevComponents::DotNetBar::SuperTabItem^ AnotherDummyTabItem;
private: DevComponents::DotNetBar::SuperTabControlPanel^ superTabControlPanel1;
private: DevComponents::DotNetBar::SuperTabItem^ DummyTabItem;
private: DevComponents::DotNetBar::Bar^ LeftDockBar;

private: DevComponents::DotNetBar::LabelItem^ StatusBarLineNumber;
private: DevComponents::DotNetBar::LabelItem^ StatusBarColumnNumber;
private: DevComponents::DotNetBar::CircularProgressItem^ StatusBarScriptBytecodeLength;
private: DevComponents::DotNetBar::ContextMenuBar^ ContextMenuProvider;
private: DevComponents::DotNetBar::ButtonItem^ ContextMenuTextEditor;
private: DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuCopy;
private: DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuPaste;
private: DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuToggleComment;
private: DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuAddBookmark;





private: DevComponents::DotNetBar::ButtonItem^ ToolbarMenuHelp;
private: DevComponents::DotNetBar::ButtonItem^ HelpMenuWiki;
private: DevComponents::DotNetBar::ButtonItem^ HelpMenuObseDocs;
private: DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuOpenPreprocessorImport;
private: DevComponents::DotNetBar::ButtonItem^ TextEditorContextMenuJumpToScript;
private: DevComponents::DotNetBar::StyleManagerAmbient^ StyleManagerAmbient;
















	protected:

	protected:
	private: System::ComponentModel::IContainer^ components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		//System::Resources::ResourceManager^ resources = gcnew System::Resources::ResourceManager("UIComponents.ScriptEditor-Workspace", System::Reflection::Assembly::GetExecutingAssembly());
		//

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(ScriptEditorWorkspace::typeid));
			this->StyleManager = (gcnew DevComponents::DotNetBar::StyleManager(this->components));
			this->MainTabStrip = (gcnew DevComponents::DotNetBar::SuperTabControl());
			this->superTabControlPanel1 = (gcnew DevComponents::DotNetBar::SuperTabControlPanel());
			this->DummyTabItem = (gcnew DevComponents::DotNetBar::SuperTabItem());
			this->superTabControlPanel2 = (gcnew DevComponents::DotNetBar::SuperTabControlPanel());
			this->AnotherDummyTabItem = (gcnew DevComponents::DotNetBar::SuperTabItem());
			this->ContainerMainToolbar = (gcnew DevComponents::DotNetBar::Bar());
			this->ToolbarNewScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarOpenScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarSaveScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarSaveScriptAndActivePlugin = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarSaveScriptNoCompile = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarSaveAllScripts = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarPreviousScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarNextScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarLabelScriptType = (gcnew DevComponents::DotNetBar::LabelItem());
			this->ToolbarScriptTypeDropdown = (gcnew DevComponents::DotNetBar::ComboBoxItem());
			this->DropdownScriptTypeObject = (gcnew DevComponents::Editors::ComboItem());
			this->DropdownScriptTypeQuest = (gcnew DevComponents::Editors::ComboItem());
			this->DropdownScriptTypeMagicEffect = (gcnew DevComponents::Editors::ComboItem());
			this->ToolbarMenuEdit = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->EditMenuFindReplace = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->EditMenuGoToLine = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->EditMenuGoToOffset = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->EditMenuAddBookmark = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->EditMenuToggleComment = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarMenuView = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuPreprocessorOutput = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuBytecodeOffsets = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuIconMargin = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuMessages = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuBookmarks = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuOutline = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuFindResults = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuGlobalFindResults = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuNavBreadcrumb = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ViewMenuDarkMode = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolbarMenuTools = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuSanitiseScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuAttachScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuCompileDepends = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuDocumentScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuRenameVars = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuModifyVarIndices = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuAddVar = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuAddVarInteger = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuAddVarFloat = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuAddVarReference = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuAddVarString = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuAddVarArray = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuImport = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuImportCurrentTab = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuImportMultipleTabs = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuExport = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuExportCurrentTab = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->MenuExportAllTabs = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuCompileActiveFileScripts = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuDeleteScripts = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuScriptSync = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuCodeSnippets = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->ToolsMenuPreferences = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->DockManager = (gcnew DevComponents::DotNetBar::DotNetBarManager(this->components));
			this->DockSiteBottom = (gcnew DevComponents::DotNetBar::DockSite());
			this->BottomDockBar = (gcnew DevComponents::DotNetBar::Bar());
			this->PanelDockContainerGlobalFindResults = (gcnew DevComponents::DotNetBar::PanelDockContainer());
			this->GlobalFindResultsList = (gcnew BrightIdeasSoftware::TreeListView());
			this->GlobalFindResultsListColumnCode = (gcnew BrightIdeasSoftware::OLVColumn());
			this->GlobalFindResultsListColumnLine = (gcnew BrightIdeasSoftware::OLVColumn());
			this->GlobalFindResultsListColumnHits = (gcnew BrightIdeasSoftware::OLVColumn());
			this->PanelDockContainerMessageList = (gcnew DevComponents::DotNetBar::PanelDockContainer());
			this->MessagesList = (gcnew BrightIdeasSoftware::FastObjectListView());
			this->MessageListColumnType = (gcnew BrightIdeasSoftware::OLVColumn());
			this->MessageListColumnLine = (gcnew BrightIdeasSoftware::OLVColumn());
			this->MessageListColumnText = (gcnew BrightIdeasSoftware::OLVColumn());
			this->MessageListColumnSource = (gcnew BrightIdeasSoftware::OLVColumn());
			this->PanelDockContainerBookmarks = (gcnew DevComponents::DotNetBar::PanelDockContainer());
			this->BookmarksList = (gcnew BrightIdeasSoftware::FastObjectListView());
			this->BookmarksColumnLine = (gcnew BrightIdeasSoftware::OLVColumn());
			this->BookmarksColumnText = (gcnew BrightIdeasSoftware::OLVColumn());
			this->BookmarksToolbar = (gcnew DevComponents::DotNetBar::Bar());
			this->BookmarksToolbarAdd = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->BookmarksToolbarRemove = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->PanelDockContainerFindResults = (gcnew DevComponents::DotNetBar::PanelDockContainer());
			this->FindResultsList = (gcnew BrightIdeasSoftware::FastObjectListView());
			this->FindResultsListColumnLine = (gcnew BrightIdeasSoftware::OLVColumn());
			this->FindResultsListColumnText = (gcnew BrightIdeasSoftware::OLVColumn());
			this->FindResultsListColumnHits = (gcnew BrightIdeasSoftware::OLVColumn());
			this->FindResultsToolbar = (gcnew DevComponents::DotNetBar::Bar());
			this->FindResultsToolbarLabel = (gcnew DevComponents::DotNetBar::LabelItem());
			this->FindResultsListToolbarLabelQuery = (gcnew DevComponents::DotNetBar::LabelItem());
			this->DockContainerItemMessageList = (gcnew DevComponents::DotNetBar::DockContainerItem());
			this->DockContainerItemBookmarks = (gcnew DevComponents::DotNetBar::DockContainerItem());
			this->DockContainerItemFindResults = (gcnew DevComponents::DotNetBar::DockContainerItem());
			this->DockContainerItemGlobalFindResults = (gcnew DevComponents::DotNetBar::DockContainerItem());
			this->DockSiteLeftEx = (gcnew DevComponents::DotNetBar::DockSite());
			this->LeftDockBar = (gcnew DevComponents::DotNetBar::Bar());
			this->PanelDockContainerOutline = (gcnew DevComponents::DotNetBar::PanelDockContainer());
			this->OutlineTreeView = (gcnew BrightIdeasSoftware::TreeListView());
			this->DockContainerItemOutline = (gcnew DevComponents::DotNetBar::DockContainerItem());
			this->DockSiteRightEx = (gcnew DevComponents::DotNetBar::DockSite());
			this->RightDockBar = (gcnew DevComponents::DotNetBar::Bar());
			this->PanelDockContainerFindReplace = (gcnew DevComponents::DotNetBar::PanelDockContainer());
			this->FindWindowLayoutControl = (gcnew DevComponents::DotNetBar::Layout::LayoutControl());
			this->FindWindowDropdownFind = (gcnew DevComponents::DotNetBar::Controls::TextBoxDropDown());
			this->FindWindowDropdownReplace = (gcnew DevComponents::DotNetBar::Controls::TextBoxDropDown());
			this->FindWindowCheckboxMatchCase = (gcnew DevComponents::DotNetBar::Controls::CheckBoxX());
			this->FindWindowCheckboxUseRegEx = (gcnew DevComponents::DotNetBar::Controls::CheckBoxX());
			this->FindWindowCheckBoxMatchWholeWord = (gcnew DevComponents::DotNetBar::Controls::CheckBoxX());
			this->FindWindowComboLookIn = (gcnew DevComponents::DotNetBar::Controls::ComboBoxEx());
			this->FindWindowDropdownLookInSelection = (gcnew DevComponents::Editors::ComboItem());
			this->FindWindowDropdownLookInCurrentScript = (gcnew DevComponents::Editors::ComboItem());
			this->FindWindowDropdownLookInAllOpenScripts = (gcnew DevComponents::Editors::ComboItem());
			this->FindWindowButtonFind = (gcnew DevComponents::DotNetBar::ButtonX());
			this->FindWindowButtonReplace = (gcnew DevComponents::DotNetBar::ButtonX());
			this->FindWindowButtonCountMatches = (gcnew DevComponents::DotNetBar::ButtonX());
			this->FindWindowLCIFindDropdown = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLCIReplaceDropdown = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLCILookIn = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLayoutGroupSettings = (gcnew DevComponents::DotNetBar::Layout::LayoutGroup());
			this->FindWindowLCIMatchCase = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLCIMatchWholeWord = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLCIUseRegEx = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLayouyGroupButtons = (gcnew DevComponents::DotNetBar::Layout::LayoutGroup());
			this->FindWindowLCIFindButton = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLCIReplaceButton = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->FindWindowLCICountMatches = (gcnew DevComponents::DotNetBar::Layout::LayoutControlItem());
			this->DockContainerItemFindReplace = (gcnew DevComponents::DotNetBar::DockContainerItem());
			this->DockSiteBottomEx = (gcnew DevComponents::DotNetBar::DockSite());
			this->DockSiteLeft = (gcnew DevComponents::DotNetBar::DockSite());
			this->DockSiteRight = (gcnew DevComponents::DotNetBar::DockSite());
			this->DockSiteTopEx = (gcnew DevComponents::DotNetBar::DockSite());
			this->DockSiteTop = (gcnew DevComponents::DotNetBar::DockSite());
			this->DockSiteCenter = (gcnew DevComponents::DotNetBar::DockSite());
			this->CenterDockBar = (gcnew DevComponents::DotNetBar::Bar());
			this->PanelDocumentContent = (gcnew System::Windows::Forms::Panel());
			this->ContextMenuProvider = (gcnew DevComponents::DotNetBar::ContextMenuBar());
			this->ContextMenuTextEditor = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->TextEditorContextMenuCopy = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->TextEditorContextMenuPaste = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->TextEditorContextMenuToggleComment = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->TextEditorContextMenuAddBookmark = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->NavigationBar = (gcnew DevComponents::DotNetBar::CrumbBar());
			this->StatusBar = (gcnew DevComponents::DotNetBar::Bar());
			this->StatusBarLineNumber = (gcnew DevComponents::DotNetBar::LabelItem());
			this->StatusBarColumnNumber = (gcnew DevComponents::DotNetBar::LabelItem());
			this->StatusBarPreprocessorOutputFlag = (gcnew DevComponents::DotNetBar::LabelItem());
			this->StatusBarScriptBytecodeLength = (gcnew DevComponents::DotNetBar::CircularProgressItem());
			this->ToolbarMenuHelp = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->HelpMenuWiki = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->HelpMenuObseDocs = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->TextEditorContextMenuOpenPreprocessorImport = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->TextEditorContextMenuJumpToScript = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->StyleManagerAmbient = (gcnew DevComponents::DotNetBar::StyleManagerAmbient(this->components));
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->MainTabStrip))->BeginInit();
			this->MainTabStrip->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ContainerMainToolbar))->BeginInit();
			this->DockSiteBottom->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomDockBar))->BeginInit();
			this->BottomDockBar->SuspendLayout();
			this->PanelDockContainerGlobalFindResults->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->GlobalFindResultsList))->BeginInit();
			this->PanelDockContainerMessageList->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->MessagesList))->BeginInit();
			this->PanelDockContainerBookmarks->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BookmarksList))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BookmarksToolbar))->BeginInit();
			this->PanelDockContainerFindResults->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->FindResultsList))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->FindResultsToolbar))->BeginInit();
			this->DockSiteLeftEx->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->LeftDockBar))->BeginInit();
			this->LeftDockBar->SuspendLayout();
			this->PanelDockContainerOutline->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->OutlineTreeView))->BeginInit();
			this->DockSiteRightEx->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->RightDockBar))->BeginInit();
			this->RightDockBar->SuspendLayout();
			this->PanelDockContainerFindReplace->SuspendLayout();
			this->FindWindowLayoutControl->SuspendLayout();
			this->DockSiteCenter->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->CenterDockBar))->BeginInit();
			this->CenterDockBar->SuspendLayout();
			this->PanelDocumentContent->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ContextMenuProvider))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->StatusBar))->BeginInit();
			this->SuspendLayout();
			//
			// StyleManager
			//
			this->StyleManager->ManagerStyle = DevComponents::DotNetBar::eStyle::VisualStudio2012Dark;
			this->StyleManager->MetroColorParameters = DevComponents::DotNetBar::Metro::ColorTables::MetroColorGeneratorParameters(System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48))), System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(128)),
					static_cast<System::Int32>(static_cast<System::Byte>(64)), static_cast<System::Int32>(static_cast<System::Byte>(0))));
			//
			// MainTabStrip
			//
			this->MainTabStrip->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->MainTabStrip->CloseButtonOnTabsVisible = true;
			//
			//
			//
			//
			//
			//
			this->MainTabStrip->ControlBox->CloseBox->Name = L"";
			//
			//
			//
			this->MainTabStrip->ControlBox->MenuBox->Name = L"";
			this->MainTabStrip->ControlBox->Name = L"";
			this->MainTabStrip->ControlBox->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->MainTabStrip->ControlBox->MenuBox,
					this->MainTabStrip->ControlBox->CloseBox
			});
			this->MainTabStrip->Controls->Add(this->superTabControlPanel1);
			this->MainTabStrip->Controls->Add(this->superTabControlPanel2);
			this->MainTabStrip->Dock = System::Windows::Forms::DockStyle::Top;
			this->MainTabStrip->ForeColor = System::Drawing::Color::White;
			this->MainTabStrip->ItemPadding->Bottom = 5;
			this->MainTabStrip->ItemPadding->Left = 5;
			this->MainTabStrip->ItemPadding->Right = 5;
			this->MainTabStrip->ItemPadding->Top = 5;
			this->MainTabStrip->Location = System::Drawing::Point(0, 0);
			this->MainTabStrip->Name = L"MainTabStrip";
			this->MainTabStrip->ReorderTabsEnabled = true;
			this->MainTabStrip->SelectedTabFont = (gcnew System::Drawing::Font(L"Segoe UI", 9, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->MainTabStrip->SelectedTabIndex = 0;
			this->MainTabStrip->Size = System::Drawing::Size(895, 27);
			this->MainTabStrip->TabFont = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->MainTabStrip->TabIndex = 1;
			this->MainTabStrip->Tabs->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) { this->DummyTabItem, this->AnotherDummyTabItem });
			this->MainTabStrip->TabStyle = DevComponents::DotNetBar::eSuperTabStyle::Office2010BackstageBlue;
			this->MainTabStrip->TextAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
			//
			// superTabControlPanel1
			//
			this->superTabControlPanel1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->superTabControlPanel1->Location = System::Drawing::Point(0, 30);
			this->superTabControlPanel1->Name = L"superTabControlPanel1";
			this->superTabControlPanel1->Size = System::Drawing::Size(895, 0);
			this->superTabControlPanel1->TabIndex = 0;
			this->superTabControlPanel1->TabItem = this->DummyTabItem;
			//
			// DummyTabItem
			//
			this->DummyTabItem->AttachedControl = this->superTabControlPanel1;
			this->DummyTabItem->GlobalItem = false;
			this->DummyTabItem->ImageAlignment = DevComponents::DotNetBar::ImageAlignment::MiddleCenter;
			this->DummyTabItem->Name = L"DummyTabItem";
			this->DummyTabItem->Text = L"Dummy Tab";
			//
			// superTabControlPanel2
			//
			this->superTabControlPanel2->Dock = System::Windows::Forms::DockStyle::Fill;
			this->superTabControlPanel2->Location = System::Drawing::Point(0, 30);
			this->superTabControlPanel2->Name = L"superTabControlPanel2";
			this->superTabControlPanel2->Size = System::Drawing::Size(737, 0);
			this->superTabControlPanel2->TabIndex = 0;
			this->superTabControlPanel2->TabItem = this->AnotherDummyTabItem;
			//
			// AnotherDummyTabItem
			//
			this->AnotherDummyTabItem->AttachedControl = this->superTabControlPanel2;
			this->AnotherDummyTabItem->GlobalItem = false;
			this->AnotherDummyTabItem->Name = L"AnotherDummyTabItem";
			this->AnotherDummyTabItem->Text = L"Yet Another Dummy Tab";
			//
			// ContainerMainToolbar
			//
			this->ContainerMainToolbar->AccessibleDescription = L"DotNetBar Bar (ContainerMainToolbar)";
			this->ContainerMainToolbar->AccessibleName = L"DotNetBar Bar";
			this->ContainerMainToolbar->AccessibleRole = System::Windows::Forms::AccessibleRole::ToolBar;
			this->ContainerMainToolbar->AntiAlias = true;
			this->ContainerMainToolbar->CanDockBottom = false;
			this->ContainerMainToolbar->CanDockLeft = false;
			this->ContainerMainToolbar->CanDockRight = false;
			this->ContainerMainToolbar->CanDockTab = false;
			this->ContainerMainToolbar->CanMove = false;
			this->ContainerMainToolbar->CanUndock = false;
			this->ContainerMainToolbar->Dock = System::Windows::Forms::DockStyle::Top;
			this->ContainerMainToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->ContainerMainToolbar->IsMaximized = false;
			this->ContainerMainToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(12) {
				this->ToolbarNewScript,
					this->ToolbarOpenScript, this->ToolbarSaveScript, this->ToolbarSaveAllScripts, this->ToolbarPreviousScript, this->ToolbarNextScript,
					this->ToolbarLabelScriptType, this->ToolbarScriptTypeDropdown, this->ToolbarMenuEdit, this->ToolbarMenuView, this->ToolbarMenuTools,
					this->ToolbarMenuHelp
			});
			this->ContainerMainToolbar->ItemSpacing = 4;
			this->ContainerMainToolbar->Location = System::Drawing::Point(0, 27);
			this->ContainerMainToolbar->Margin = System::Windows::Forms::Padding(5);
			this->ContainerMainToolbar->Name = L"ContainerMainToolbar";
			this->ContainerMainToolbar->PaddingBottom = 5;
			this->ContainerMainToolbar->PaddingLeft = 5;
			this->ContainerMainToolbar->PaddingRight = 5;
			this->ContainerMainToolbar->PaddingTop = 5;
			this->ContainerMainToolbar->Size = System::Drawing::Size(895, 37);
			this->ContainerMainToolbar->Stretch = true;
			this->ContainerMainToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->ContainerMainToolbar->TabIndex = 2;
			this->ContainerMainToolbar->TabStop = false;
			//
			// ToolbarNewScript
			//
			this->ToolbarNewScript->AlternateShortCutText = L"aasds";
			this->ToolbarNewScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarNewScript.Image")));
			this->ToolbarNewScript->ImagePaddingHorizontal = 15;
			this->ToolbarNewScript->ImagePaddingVertical = 10;
			this->ToolbarNewScript->Name = L"ToolbarNewScript";
			this->ToolbarNewScript->Text = L"New Script";
			this->ToolbarNewScript->Tooltip = L"New Script";
			//
			// ToolbarOpenScript
			//
			this->ToolbarOpenScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarOpenScript.Image")));
			this->ToolbarOpenScript->ImagePaddingHorizontal = 15;
			this->ToolbarOpenScript->ImagePaddingVertical = 10;
			this->ToolbarOpenScript->Name = L"ToolbarOpenScript";
			this->ToolbarOpenScript->Text = L"Open Script";
			this->ToolbarOpenScript->Tooltip = L"Open Script";
			this->ToolbarOpenScript->Click += gcnew System::EventHandler(this, &ScriptEditorWorkspace::ToolbarOpenScript_Click);
			//
			// ToolbarSaveScript
			//
			this->ToolbarSaveScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSaveScript.Image")));
			this->ToolbarSaveScript->ImagePaddingHorizontal = 15;
			this->ToolbarSaveScript->ImagePaddingVertical = 10;
			this->ToolbarSaveScript->Name = L"ToolbarSaveScript";
			this->ToolbarSaveScript->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->ToolbarSaveScriptAndActivePlugin,
					this->ToolbarSaveScriptNoCompile
			});
			this->ToolbarSaveScript->Text = L"Save Script";
			this->ToolbarSaveScript->Tooltip = L"Save Script";
			//
			// ToolbarSaveScriptAndActivePlugin
			//
			this->ToolbarSaveScriptAndActivePlugin->Name = L"ToolbarSaveScriptAndActivePlugin";
			this->ToolbarSaveScriptAndActivePlugin->Text = L"Save Script and Active Plugin";
			//
			// ToolbarSaveScriptNoCompile
			//
			this->ToolbarSaveScriptNoCompile->Name = L"ToolbarSaveScriptNoCompile";
			this->ToolbarSaveScriptNoCompile->Text = L"Save Script (Do Not Compile)";
			//
			// ToolbarSaveAllScripts
			//
			this->ToolbarSaveAllScripts->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarSaveAllScripts.Image")));
			this->ToolbarSaveAllScripts->ImagePaddingHorizontal = 15;
			this->ToolbarSaveAllScripts->ImagePaddingVertical = 10;
			this->ToolbarSaveAllScripts->Name = L"ToolbarSaveAllScripts";
			this->ToolbarSaveAllScripts->Text = L"Save All Open Scripts";
			this->ToolbarSaveAllScripts->Tooltip = L"Save All Open Scripts";
			//
			// ToolbarPreviousScript
			//
			this->ToolbarPreviousScript->BeginGroup = true;
			this->ToolbarPreviousScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarPreviousScript.Image")));
			this->ToolbarPreviousScript->ImagePaddingHorizontal = 15;
			this->ToolbarPreviousScript->ImagePaddingVertical = 10;
			this->ToolbarPreviousScript->Name = L"ToolbarPreviousScript";
			this->ToolbarPreviousScript->Text = L"Previous Script";
			this->ToolbarPreviousScript->Tooltip = L"Previous Script";
			//
			// ToolbarNextScript
			//
			this->ToolbarNextScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolbarNextScript.Image")));
			this->ToolbarNextScript->ImagePaddingHorizontal = 15;
			this->ToolbarNextScript->ImagePaddingVertical = 10;
			this->ToolbarNextScript->Name = L"ToolbarNextScript";
			this->ToolbarNextScript->Text = L"Next Script";
			this->ToolbarNextScript->Tooltip = L"Next Script";
			//
			// ToolbarLabelScriptType
			//
			this->ToolbarLabelScriptType->BeginGroup = true;
			this->ToolbarLabelScriptType->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Center;
			this->ToolbarLabelScriptType->Name = L"ToolbarLabelScriptType";
			this->ToolbarLabelScriptType->PaddingLeft = 5;
			this->ToolbarLabelScriptType->Text = L"Script Type";
			//
			// ToolbarScriptTypeDropdown
			//
			this->ToolbarScriptTypeDropdown->ComboWidth = 150;
			this->ToolbarScriptTypeDropdown->DropDownHeight = 106;
			this->ToolbarScriptTypeDropdown->DropDownWidth = 150;
			this->ToolbarScriptTypeDropdown->ItemHeight = 18;
			this->ToolbarScriptTypeDropdown->Items->AddRange(gcnew cli::array< System::Object^  >(3) {
				this->DropdownScriptTypeObject,
					this->DropdownScriptTypeQuest, this->DropdownScriptTypeMagicEffect
			});
			this->ToolbarScriptTypeDropdown->Name = L"ToolbarScriptTypeDropdown";
			//
			// DropdownScriptTypeObject
			//
			this->DropdownScriptTypeObject->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"DropdownScriptTypeObject.Image")));
			this->DropdownScriptTypeObject->Text = L"Object Script";
			//
			// DropdownScriptTypeQuest
			//
			this->DropdownScriptTypeQuest->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"DropdownScriptTypeQuest.Image")));
			this->DropdownScriptTypeQuest->Text = L"Quest Script";
			//
			// DropdownScriptTypeMagicEffect
			//
			this->DropdownScriptTypeMagicEffect->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"DropdownScriptTypeMagicEffect.Image")));
			this->DropdownScriptTypeMagicEffect->Text = L"Magic Effect Script";
			//
			// ToolbarMenuEdit
			//
			this->ToolbarMenuEdit->AutoExpandOnClick = true;
			this->ToolbarMenuEdit->ImageFixedSize = System::Drawing::Size(12, 12);
			this->ToolbarMenuEdit->ImagePaddingHorizontal = 15;
			this->ToolbarMenuEdit->ImagePaddingVertical = 10;
			this->ToolbarMenuEdit->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
			this->ToolbarMenuEdit->Name = L"ToolbarMenuEdit";
			this->ToolbarMenuEdit->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(5) {
				this->EditMenuFindReplace,
					this->EditMenuGoToLine, this->EditMenuGoToOffset, this->EditMenuAddBookmark, this->EditMenuToggleComment
			});
			this->ToolbarMenuEdit->Text = L"&EDIT";
			//
			// EditMenuFindReplace
			//
			this->EditMenuFindReplace->AlternateShortCutText = L"Ctrl+Shift+F";
			this->EditMenuFindReplace->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"EditMenuFindReplace.Image")));
			this->EditMenuFindReplace->Name = L"EditMenuFindReplace";
			this->EditMenuFindReplace->Text = L"&Find and Replace";
			//
			// EditMenuGoToLine
			//
			this->EditMenuGoToLine->AlternateShortCutText = L"Ctrl + G";
			this->EditMenuGoToLine->BeginGroup = true;
			this->EditMenuGoToLine->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"EditMenuGoToLine.Image")));
			this->EditMenuGoToLine->Name = L"EditMenuGoToLine";
			this->EditMenuGoToLine->Text = L"Go To &Line";
			//
			// EditMenuGoToOffset
			//
			this->EditMenuGoToOffset->AlternateShortCutText = L"Ctrl + E";
			this->EditMenuGoToOffset->Name = L"EditMenuGoToOffset";
			this->EditMenuGoToOffset->Text = L"Go To &Offset";
			//
			// EditMenuAddBookmark
			//
			this->EditMenuAddBookmark->AlternateShortCutText = L"Ctrl + B";
			this->EditMenuAddBookmark->BeginGroup = true;
			this->EditMenuAddBookmark->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"EditMenuAddBookmark.Image")));
			this->EditMenuAddBookmark->Name = L"EditMenuAddBookmark";
			this->EditMenuAddBookmark->Text = L"Add &Bookmark";
			//
			// EditMenuToggleComment
			//
			this->EditMenuToggleComment->AlternateShortCutText = L"Ctrl + Q/W";
			this->EditMenuToggleComment->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"EditMenuToggleComment.Image")));
			this->EditMenuToggleComment->Name = L"EditMenuToggleComment";
			this->EditMenuToggleComment->Text = L"Toggle &Comment";
			//
			// ToolbarMenuView
			//
			this->ToolbarMenuView->AutoExpandOnClick = true;
			this->ToolbarMenuView->ImageFixedSize = System::Drawing::Size(12, 12);
			this->ToolbarMenuView->Name = L"ToolbarMenuView";
			this->ToolbarMenuView->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(10) {
				this->ViewMenuPreprocessorOutput,
					this->ViewMenuBytecodeOffsets, this->ViewMenuIconMargin, this->ViewMenuMessages, this->ViewMenuBookmarks, this->ViewMenuOutline,
					this->ViewMenuFindResults, this->ViewMenuGlobalFindResults, this->ViewMenuNavBreadcrumb, this->ViewMenuDarkMode
			});
			this->ToolbarMenuView->Text = L"&VIEW";
			//
			// ViewMenuPreprocessorOutput
			//
			this->ViewMenuPreprocessorOutput->Checked = true;
			this->ViewMenuPreprocessorOutput->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuPreprocessorOutput.Image")));
			this->ViewMenuPreprocessorOutput->Name = L"ViewMenuPreprocessorOutput";
			this->ViewMenuPreprocessorOutput->Text = L"&Preprocessed Script Text";
			//
			// ViewMenuBytecodeOffsets
			//
			this->ViewMenuBytecodeOffsets->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuBytecodeOffsets.Image")));
			this->ViewMenuBytecodeOffsets->Name = L"ViewMenuBytecodeOffsets";
			this->ViewMenuBytecodeOffsets->Text = L"Script B&ytecode Offsets";
			//
			// ViewMenuIconMargin
			//
			this->ViewMenuIconMargin->Name = L"ViewMenuIconMargin";
			this->ViewMenuIconMargin->Text = L"&Icon Margin";
			//
			// ViewMenuMessages
			//
			this->ViewMenuMessages->BeginGroup = true;
			this->ViewMenuMessages->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuMessages.Image")));
			this->ViewMenuMessages->Name = L"ViewMenuMessages";
			this->ViewMenuMessages->Text = L"&Messages";
			//
			// ViewMenuBookmarks
			//
			this->ViewMenuBookmarks->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuBookmarks.Image")));
			this->ViewMenuBookmarks->Name = L"ViewMenuBookmarks";
			this->ViewMenuBookmarks->Text = L"&Bookmarks";
			//
			// ViewMenuOutline
			//
			this->ViewMenuOutline->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuOutline.Image")));
			this->ViewMenuOutline->Name = L"ViewMenuOutline";
			this->ViewMenuOutline->Text = L"&Outline View";
			//
			// ViewMenuFindResults
			//
			this->ViewMenuFindResults->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuFindResults.Image")));
			this->ViewMenuFindResults->Name = L"ViewMenuFindResults";
			this->ViewMenuFindResults->Text = L"&Find and Replace Results";
			//
			// ViewMenuGlobalFindResults
			//
			this->ViewMenuGlobalFindResults->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuGlobalFindResults.Image")));
			this->ViewMenuGlobalFindResults->Name = L"ViewMenuGlobalFindResults";
			this->ViewMenuGlobalFindResults->Text = L"Find in &Tabs Results";
			//
			// ViewMenuNavBreadcrumb
			//
			this->ViewMenuNavBreadcrumb->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuNavBreadcrumb.Image")));
			this->ViewMenuNavBreadcrumb->Name = L"ViewMenuNavBreadcrumb";
			this->ViewMenuNavBreadcrumb->Text = L"&Navigation Bar";
			//
			// ViewMenuDarkMode
			//
			this->ViewMenuDarkMode->BeginGroup = true;
			this->ViewMenuDarkMode->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ViewMenuDarkMode.Image")));
			this->ViewMenuDarkMode->Name = L"ViewMenuDarkMode";
			this->ViewMenuDarkMode->Text = L"&Dark Mode";
			this->ViewMenuDarkMode->Click += gcnew System::EventHandler(this, &ScriptEditorWorkspace::ViewMenuDarkMode_Click);
			//
			// ToolbarMenuTools
			//
			this->ToolbarMenuTools->AutoExpandOnClick = true;
			this->ToolbarMenuTools->Name = L"ToolbarMenuTools";
			this->ToolbarMenuTools->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(14) {
				this->ToolsMenuSanitiseScript,
					this->ToolsMenuAttachScript, this->ToolsMenuCompileDepends, this->ToolsMenuDocumentScript, this->ToolsMenuRenameVars, this->ToolsMenuModifyVarIndices,
					this->ToolsMenuAddVar, this->ToolsMenuImport, this->ToolsMenuExport, this->ToolsMenuCompileActiveFileScripts, this->ToolsMenuDeleteScripts,
					this->ToolsMenuScriptSync, this->ToolsMenuCodeSnippets, this->ToolsMenuPreferences
			});
			this->ToolbarMenuTools->Text = L"&TOOLS";
			//
			// ToolsMenuSanitiseScript
			//
			this->ToolsMenuSanitiseScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuSanitiseScript.Image")));
			this->ToolsMenuSanitiseScript->Name = L"ToolsMenuSanitiseScript";
			this->ToolsMenuSanitiseScript->Text = L"&Sanitise Script";
			//
			// ToolsMenuAttachScript
			//
			this->ToolsMenuAttachScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuAttachScript.Image")));
			this->ToolsMenuAttachScript->Name = L"ToolsMenuAttachScript";
			this->ToolsMenuAttachScript->Text = L"&Attach Script...";
			//
			// ToolsMenuCompileDepends
			//
			this->ToolsMenuCompileDepends->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuCompileDepends.Image")));
			this->ToolsMenuCompileDepends->Name = L"ToolsMenuCompileDepends";
			this->ToolsMenuCompileDepends->Text = L"Recompile Script &Dependencies";
			//
			// ToolsMenuDocumentScript
			//
			this->ToolsMenuDocumentScript->BeginGroup = true;
			this->ToolsMenuDocumentScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuDocumentScript.Image")));
			this->ToolsMenuDocumentScript->Name = L"ToolsMenuDocumentScript";
			this->ToolsMenuDocumentScript->Text = L"Doc&ument Script...";
			//
			// ToolsMenuRenameVars
			//
			this->ToolsMenuRenameVars->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuRenameVars.Image")));
			this->ToolsMenuRenameVars->Name = L"ToolsMenuRenameVars";
			this->ToolsMenuRenameVars->Text = L"Re&name Variables...";
			//
			// ToolsMenuModifyVarIndices
			//
			this->ToolsMenuModifyVarIndices->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuModifyVarIndices.Image")));
			this->ToolsMenuModifyVarIndices->Name = L"ToolsMenuModifyVarIndices";
			this->ToolsMenuModifyVarIndices->Text = L"Modif&y Variable Indices...";
			//
			// ToolsMenuAddVar
			//
			this->ToolsMenuAddVar->AutoExpandOnClick = true;
			this->ToolsMenuAddVar->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuAddVar.Image")));
			this->ToolsMenuAddVar->Name = L"ToolsMenuAddVar";
			this->ToolsMenuAddVar->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(5) {
				this->MenuAddVarInteger,
					this->MenuAddVarFloat, this->MenuAddVarReference, this->MenuAddVarString, this->MenuAddVarArray
			});
			this->ToolsMenuAddVar->Text = L"Add &Variable";
			//
			// MenuAddVarInteger
			//
			this->MenuAddVarInteger->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::TextOnlyAlways;
			this->MenuAddVarInteger->Name = L"MenuAddVarInteger";
			this->MenuAddVarInteger->Text = L"&Integer";
			//
			// MenuAddVarFloat
			//
			this->MenuAddVarFloat->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::TextOnlyAlways;
			this->MenuAddVarFloat->Name = L"MenuAddVarFloat";
			this->MenuAddVarFloat->Text = L"&Float";
			//
			// MenuAddVarReference
			//
			this->MenuAddVarReference->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::TextOnlyAlways;
			this->MenuAddVarReference->Name = L"MenuAddVarReference";
			this->MenuAddVarReference->Text = L"&Reference";
			//
			// MenuAddVarString
			//
			this->MenuAddVarString->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::TextOnlyAlways;
			this->MenuAddVarString->Name = L"MenuAddVarString";
			this->MenuAddVarString->Text = L"&String";
			//
			// MenuAddVarArray
			//
			this->MenuAddVarArray->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::TextOnlyAlways;
			this->MenuAddVarArray->Name = L"MenuAddVarArray";
			this->MenuAddVarArray->Text = L"&Array";
			//
			// ToolsMenuImport
			//
			this->ToolsMenuImport->BeginGroup = true;
			this->ToolsMenuImport->Name = L"ToolsMenuImport";
			this->ToolsMenuImport->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->MenuImportCurrentTab,
					this->MenuImportMultipleTabs
			});
			this->ToolsMenuImport->Text = L"&Import";
			//
			// MenuImportCurrentTab
			//
			this->MenuImportCurrentTab->Name = L"MenuImportCurrentTab";
			this->MenuImportCurrentTab->Text = L"Import File into &Current Script";
			//
			// MenuImportMultipleTabs
			//
			this->MenuImportMultipleTabs->Name = L"MenuImportMultipleTabs";
			this->MenuImportMultipleTabs->Text = L"Import &Multiple Files...";
			//
			// ToolsMenuExport
			//
			this->ToolsMenuExport->Name = L"ToolsMenuExport";
			this->ToolsMenuExport->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->MenuExportCurrentTab,
					this->MenuExportAllTabs
			});
			this->ToolsMenuExport->Text = L"&Export";
			//
			// MenuExportCurrentTab
			//
			this->MenuExportCurrentTab->Name = L"MenuExportCurrentTab";
			this->MenuExportCurrentTab->Text = L"Save &Current Script to Disk";
			//
			// MenuExportAllTabs
			//
			this->MenuExportAllTabs->Name = L"MenuExportAllTabs";
			this->MenuExportAllTabs->Text = L"Save &All Open Scripts to Disk";
			//
			// ToolsMenuCompileActiveFileScripts
			//
			this->ToolsMenuCompileActiveFileScripts->BeginGroup = true;
			this->ToolsMenuCompileActiveFileScripts->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuCompileActiveFileScripts.Image")));
			this->ToolsMenuCompileActiveFileScripts->Name = L"ToolsMenuCompileActiveFileScripts";
			this->ToolsMenuCompileActiveFileScripts->Text = L"Recompile All Scripts in Active &File";
			//
			// ToolsMenuDeleteScripts
			//
			this->ToolsMenuDeleteScripts->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuDeleteScripts.Image")));
			this->ToolsMenuDeleteScripts->Name = L"ToolsMenuDeleteScripts";
			this->ToolsMenuDeleteScripts->Text = L"&Delete Scripts...";
			//
			// ToolsMenuScriptSync
			//
			this->ToolsMenuScriptSync->BeginGroup = true;
			this->ToolsMenuScriptSync->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuScriptSync.Image")));
			this->ToolsMenuScriptSync->Name = L"ToolsMenuScriptSync";
			this->ToolsMenuScriptSync->Text = L"S&ync Scripts to Disk...";
			this->ToolsMenuScriptSync->Click += gcnew System::EventHandler(this, &ScriptEditorWorkspace::ToolsMenuScriptSync_Click);
			//
			// ToolsMenuCodeSnippets
			//
			this->ToolsMenuCodeSnippets->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuCodeSnippets.Image")));
			this->ToolsMenuCodeSnippets->Name = L"ToolsMenuCodeSnippets";
			this->ToolsMenuCodeSnippets->Text = L"Code &Snippets...";
			//
			// ToolsMenuPreferences
			//
			this->ToolsMenuPreferences->BeginGroup = true;
			this->ToolsMenuPreferences->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"ToolsMenuPreferences.Image")));
			this->ToolsMenuPreferences->Name = L"ToolsMenuPreferences";
			this->ToolsMenuPreferences->Text = L"Script Editor &Preferences...";
			this->ToolsMenuPreferences->Click += gcnew System::EventHandler(this, &ScriptEditorWorkspace::ToolsMenuPreferences_Click);
			//
			// DockManager
			//
			this->DockManager->AllowUserBarCustomize = false;
			this->DockManager->AlwaysShowFullMenus = true;
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::F1);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::CtrlC);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::CtrlA);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::CtrlV);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::CtrlX);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::CtrlZ);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::CtrlY);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::Del);
			this->DockManager->AutoDispatchShortcuts->Add(DevComponents::DotNetBar::eShortcut::Ins);
			this->DockManager->BottomDockSite = this->DockSiteBottom;
			this->DockManager->DockSplitterSize = 5;
			this->DockManager->EnableFullSizeDock = false;
			this->DockManager->LeftDockSite = this->DockSiteLeftEx;
			this->DockManager->ParentForm = this;
			this->DockManager->PopupAnimation = DevComponents::DotNetBar::ePopupAnimation::Fade;
			this->DockManager->RightDockSite = this->DockSiteRightEx;
			this->DockManager->ShowCustomizeContextMenu = false;
			this->DockManager->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->DockManager->ToolbarBottomDockSite = this->DockSiteBottomEx;
			this->DockManager->ToolbarLeftDockSite = this->DockSiteLeft;
			this->DockManager->ToolbarRightDockSite = this->DockSiteRight;
			this->DockManager->ToolbarTopDockSite = this->DockSiteTopEx;
			this->DockManager->TopDockSite = this->DockSiteTop;
			//
			// DockSiteBottom
			//
			this->DockSiteBottom->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteBottom->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteBottom->Controls->Add(this->BottomDockBar);
			this->DockSiteBottom->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->DockSiteBottom->DocumentDockContainer = (gcnew DevComponents::DotNetBar::DocumentDockContainer(gcnew cli::array< DevComponents::DotNetBar::DocumentBaseContainer^  >(1) {
				(cli::safe_cast<DevComponents::DotNetBar::DocumentBaseContainer^>((gcnew DevComponents::DotNetBar::DocumentBarContainer(this->BottomDockBar,
					895, 195))))
			}, DevComponents::DotNetBar::eOrientation::Vertical));
			this->DockSiteBottom->ForeColor = System::Drawing::Color::White;
			this->DockSiteBottom->Location = System::Drawing::Point(0, 636);
			this->DockSiteBottom->Name = L"DockSiteBottom";
			this->DockSiteBottom->Size = System::Drawing::Size(895, 200);
			this->DockSiteBottom->TabIndex = 7;
			this->DockSiteBottom->TabStop = false;
			//
			// BottomDockBar
			//
			this->BottomDockBar->AccessibleDescription = L"DotNetBar Bar (BottomDockBar)";
			this->BottomDockBar->AccessibleName = L"DotNetBar Bar";
			this->BottomDockBar->AccessibleRole = System::Windows::Forms::AccessibleRole::Grouping;
			this->BottomDockBar->AutoSyncBarCaption = true;
			this->BottomDockBar->BarType = DevComponents::DotNetBar::eBarType::DockWindow;
			this->BottomDockBar->CanDockTop = false;
			this->BottomDockBar->CanHide = true;
			this->BottomDockBar->CloseSingleTab = true;
			this->BottomDockBar->Controls->Add(this->PanelDockContainerBookmarks);
			this->BottomDockBar->Controls->Add(this->PanelDockContainerMessageList);
			this->BottomDockBar->Controls->Add(this->PanelDockContainerFindResults);
			this->BottomDockBar->Controls->Add(this->PanelDockContainerGlobalFindResults);
			this->BottomDockBar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->BottomDockBar->GrabHandleStyle = DevComponents::DotNetBar::eGrabHandleStyle::Caption;
			this->BottomDockBar->IsMaximized = false;
			this->BottomDockBar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(4) {
				this->DockContainerItemMessageList,
					this->DockContainerItemBookmarks, this->DockContainerItemFindResults, this->DockContainerItemGlobalFindResults
			});
			this->BottomDockBar->LayoutType = DevComponents::DotNetBar::eLayoutType::DockContainer;
			this->BottomDockBar->Location = System::Drawing::Point(0, 5);
			this->BottomDockBar->Name = L"BottomDockBar";
			this->BottomDockBar->SelectedDockTab = 1;
			this->BottomDockBar->Size = System::Drawing::Size(895, 195);
			this->BottomDockBar->Stretch = true;
			this->BottomDockBar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BottomDockBar->TabIndex = 0;
			this->BottomDockBar->TabStop = false;
			this->BottomDockBar->Text = L"Bookmarks";
			//
			// PanelDockContainerGlobalFindResults
			//
			this->PanelDockContainerGlobalFindResults->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->PanelDockContainerGlobalFindResults->Controls->Add(this->GlobalFindResultsList);
			this->PanelDockContainerGlobalFindResults->DisabledBackColor = System::Drawing::Color::Empty;
			this->PanelDockContainerGlobalFindResults->Location = System::Drawing::Point(3, 23);
			this->PanelDockContainerGlobalFindResults->Name = L"PanelDockContainerGlobalFindResults";
			this->PanelDockContainerGlobalFindResults->Size = System::Drawing::Size(889, 144);
			this->PanelDockContainerGlobalFindResults->Style->Alignment = System::Drawing::StringAlignment::Center;
			this->PanelDockContainerGlobalFindResults->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
			this->PanelDockContainerGlobalFindResults->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::ItemText;
			this->PanelDockContainerGlobalFindResults->Style->GradientAngle = 90;
			this->PanelDockContainerGlobalFindResults->TabIndex = 13;
			//
			// GlobalFindResultsList
			//
			this->GlobalFindResultsList->AllColumns->Add(this->GlobalFindResultsListColumnCode);
			this->GlobalFindResultsList->AllColumns->Add(this->GlobalFindResultsListColumnLine);
			this->GlobalFindResultsList->AllColumns->Add(this->GlobalFindResultsListColumnHits);
			this->GlobalFindResultsList->BackColor = System::Drawing::Color::Black;
			this->GlobalFindResultsList->CellEditUseWholeCell = false;
			this->GlobalFindResultsList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {
				this->GlobalFindResultsListColumnCode,
					this->GlobalFindResultsListColumnLine, this->GlobalFindResultsListColumnHits
			});
			this->GlobalFindResultsList->Cursor = System::Windows::Forms::Cursors::Default;
			this->GlobalFindResultsList->Dock = System::Windows::Forms::DockStyle::Fill;
			this->GlobalFindResultsList->EmptyListMsg = L"Doesn\'t look like anything to me...";
			this->GlobalFindResultsList->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->GlobalFindResultsList->ForeColor = System::Drawing::Color::White;
			this->GlobalFindResultsList->GridLines = true;
			this->GlobalFindResultsList->HideSelection = false;
			this->GlobalFindResultsList->Location = System::Drawing::Point(0, 0);
			this->GlobalFindResultsList->MultiSelect = false;
			this->GlobalFindResultsList->Name = L"GlobalFindResultsList";
			this->GlobalFindResultsList->ShowGroups = false;
			this->GlobalFindResultsList->Size = System::Drawing::Size(889, 144);
			this->GlobalFindResultsList->TabIndex = 0;
			this->GlobalFindResultsList->UseCompatibleStateImageBehavior = false;
			this->GlobalFindResultsList->View = System::Windows::Forms::View::Details;
			this->GlobalFindResultsList->VirtualMode = true;
			//
			// GlobalFindResultsListColumnCode
			//
			this->GlobalFindResultsListColumnCode->MaximumWidth = 500;
			this->GlobalFindResultsListColumnCode->Text = L"Code";
			this->GlobalFindResultsListColumnCode->Width = 500;
			//
			// GlobalFindResultsListColumnLine
			//
			this->GlobalFindResultsListColumnLine->MaximumWidth = 40;
			this->GlobalFindResultsListColumnLine->MinimumWidth = 40;
			this->GlobalFindResultsListColumnLine->Text = L"Line";
			this->GlobalFindResultsListColumnLine->Width = 40;
			//
			// GlobalFindResultsListColumnHits
			//
			this->GlobalFindResultsListColumnHits->MaximumWidth = 40;
			this->GlobalFindResultsListColumnHits->MinimumWidth = 40;
			this->GlobalFindResultsListColumnHits->Text = L"Hits";
			this->GlobalFindResultsListColumnHits->Width = 40;
			//
			// PanelDockContainerMessageList
			//
			this->PanelDockContainerMessageList->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->PanelDockContainerMessageList->Controls->Add(this->MessagesList);
			this->PanelDockContainerMessageList->DisabledBackColor = System::Drawing::Color::Empty;
			this->PanelDockContainerMessageList->Location = System::Drawing::Point(3, 23);
			this->PanelDockContainerMessageList->Name = L"PanelDockContainerMessageList";
			this->PanelDockContainerMessageList->Size = System::Drawing::Size(889, 144);
			this->PanelDockContainerMessageList->Style->Alignment = System::Drawing::StringAlignment::Center;
			this->PanelDockContainerMessageList->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
			this->PanelDockContainerMessageList->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::ItemText;
			this->PanelDockContainerMessageList->Style->GradientAngle = 90;
			this->PanelDockContainerMessageList->TabIndex = 0;
			//
			// MessagesList
			//
			this->MessagesList->AllColumns->Add(this->MessageListColumnType);
			this->MessagesList->AllColumns->Add(this->MessageListColumnLine);
			this->MessagesList->AllColumns->Add(this->MessageListColumnText);
			this->MessagesList->AllColumns->Add(this->MessageListColumnSource);
			this->MessagesList->BackColor = System::Drawing::Color::Black;
			this->MessagesList->CellEditUseWholeCell = false;
			this->MessagesList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(4) {
				this->MessageListColumnType,
					this->MessageListColumnLine, this->MessageListColumnText, this->MessageListColumnSource
			});
			this->MessagesList->Cursor = System::Windows::Forms::Cursors::Default;
			this->MessagesList->Dock = System::Windows::Forms::DockStyle::Fill;
			this->MessagesList->EmptyListMsg = L"Doesn\'t look like anything to me...";
			this->MessagesList->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->MessagesList->ForeColor = System::Drawing::Color::White;
			this->MessagesList->FullRowSelect = true;
			this->MessagesList->GridLines = true;
			this->MessagesList->HideSelection = false;
			this->MessagesList->Location = System::Drawing::Point(0, 0);
			this->MessagesList->MultiSelect = false;
			this->MessagesList->Name = L"MessagesList";
			this->MessagesList->ShowGroups = false;
			this->MessagesList->Size = System::Drawing::Size(889, 144);
			this->MessagesList->TabIndex = 0;
			this->MessagesList->UseCompatibleStateImageBehavior = false;
			this->MessagesList->View = System::Windows::Forms::View::Details;
			this->MessagesList->VirtualMode = true;
			//
			// MessageListColumnType
			//
			this->MessageListColumnType->MaximumWidth = 40;
			this->MessageListColumnType->MinimumWidth = 40;
			this->MessageListColumnType->Text = L"Type";
			this->MessageListColumnType->Width = 40;
			//
			// MessageListColumnLine
			//
			this->MessageListColumnLine->MaximumWidth = 40;
			this->MessageListColumnLine->MinimumWidth = 50;
			this->MessageListColumnLine->Text = L"Line";
			this->MessageListColumnLine->Width = 40;
			//
			// MessageListColumnText
			//
			this->MessageListColumnText->MinimumWidth = 500;
			this->MessageListColumnText->Text = L"Message";
			this->MessageListColumnText->Width = 682;
			//
			// MessageListColumnSource
			//
			this->MessageListColumnSource->MinimumWidth = 100;
			this->MessageListColumnSource->Text = L"Source";
			this->MessageListColumnSource->Width = 100;
			//
			// PanelDockContainerBookmarks
			//
			this->PanelDockContainerBookmarks->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->PanelDockContainerBookmarks->Controls->Add(this->BookmarksList);
			this->PanelDockContainerBookmarks->Controls->Add(this->BookmarksToolbar);
			this->PanelDockContainerBookmarks->DisabledBackColor = System::Drawing::Color::Empty;
			this->PanelDockContainerBookmarks->Location = System::Drawing::Point(3, 23);
			this->PanelDockContainerBookmarks->Name = L"PanelDockContainerBookmarks";
			this->PanelDockContainerBookmarks->Size = System::Drawing::Size(889, 144);
			this->PanelDockContainerBookmarks->Style->Alignment = System::Drawing::StringAlignment::Center;
			this->PanelDockContainerBookmarks->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
			this->PanelDockContainerBookmarks->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::ItemText;
			this->PanelDockContainerBookmarks->Style->GradientAngle = 90;
			this->PanelDockContainerBookmarks->TabIndex = 5;
			//
			// BookmarksList
			//
			this->BookmarksList->AllColumns->Add(this->BookmarksColumnLine);
			this->BookmarksList->AllColumns->Add(this->BookmarksColumnText);
			this->BookmarksList->BackColor = System::Drawing::Color::Black;
			this->BookmarksList->CellEditUseWholeCell = false;
			this->BookmarksList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {
				this->BookmarksColumnLine,
					this->BookmarksColumnText
			});
			this->BookmarksList->Cursor = System::Windows::Forms::Cursors::Default;
			this->BookmarksList->Dock = System::Windows::Forms::DockStyle::Fill;
			this->BookmarksList->EmptyListMsg = L"Doesn\'t look like anything to me...";
			this->BookmarksList->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->BookmarksList->ForeColor = System::Drawing::Color::White;
			this->BookmarksList->FullRowSelect = true;
			this->BookmarksList->GridLines = true;
			this->BookmarksList->HideSelection = false;
			this->BookmarksList->Location = System::Drawing::Point(28, 0);
			this->BookmarksList->MultiSelect = false;
			this->BookmarksList->Name = L"BookmarksList";
			this->BookmarksList->ShowGroups = false;
			this->BookmarksList->Size = System::Drawing::Size(861, 144);
			this->BookmarksList->TabIndex = 1;
			this->BookmarksList->UseCompatibleStateImageBehavior = false;
			this->BookmarksList->View = System::Windows::Forms::View::Details;
			this->BookmarksList->VirtualMode = true;
			//
			// BookmarksColumnLine
			//
			this->BookmarksColumnLine->MaximumWidth = 40;
			this->BookmarksColumnLine->MinimumWidth = 50;
			this->BookmarksColumnLine->Text = L"Line";
			this->BookmarksColumnLine->Width = 50;
			//
			// BookmarksColumnText
			//
			this->BookmarksColumnText->MinimumWidth = 500;
			this->BookmarksColumnText->Text = L"Text";
			this->BookmarksColumnText->Width = 757;
			//
			// BookmarksToolbar
			//
			this->BookmarksToolbar->AntiAlias = true;
			this->BookmarksToolbar->CanMove = false;
			this->BookmarksToolbar->CanUndock = false;
			this->BookmarksToolbar->Dock = System::Windows::Forms::DockStyle::Left;
			this->BookmarksToolbar->DockOrientation = DevComponents::DotNetBar::eOrientation::Vertical;
			this->BookmarksToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->BookmarksToolbar->IsMaximized = false;
			this->BookmarksToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->BookmarksToolbarAdd,
					this->BookmarksToolbarRemove
			});
			this->BookmarksToolbar->ItemSpacing = 5;
			this->BookmarksToolbar->Location = System::Drawing::Point(0, 0);
			this->BookmarksToolbar->Name = L"BookmarksToolbar";
			this->BookmarksToolbar->Size = System::Drawing::Size(28, 144);
			this->BookmarksToolbar->Stretch = true;
			this->BookmarksToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BookmarksToolbar->TabIndex = 0;
			this->BookmarksToolbar->TabStop = false;
			this->BookmarksToolbar->Text = L"bar1";
			//
			// BookmarksToolbarAdd
			//
			this->BookmarksToolbarAdd->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"BookmarksToolbarAdd.Image")));
			this->BookmarksToolbarAdd->ImagePaddingHorizontal = 15;
			this->BookmarksToolbarAdd->ImagePaddingVertical = 10;
			this->BookmarksToolbarAdd->Name = L"BookmarksToolbarAdd";
			this->BookmarksToolbarAdd->Text = L"Add Bookmark";
			this->BookmarksToolbarAdd->Tooltip = L"Add Bookmark";
			//
			// BookmarksToolbarRemove
			//
			this->BookmarksToolbarRemove->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"BookmarksToolbarRemove.Image")));
			this->BookmarksToolbarRemove->ImagePaddingHorizontal = 15;
			this->BookmarksToolbarRemove->ImagePaddingVertical = 10;
			this->BookmarksToolbarRemove->Name = L"BookmarksToolbarRemove";
			this->BookmarksToolbarRemove->Text = L"Remove Bookmark";
			this->BookmarksToolbarRemove->Tooltip = L"Remove Bookmark";
			//
			// PanelDockContainerFindResults
			//
			this->PanelDockContainerFindResults->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->PanelDockContainerFindResults->Controls->Add(this->FindResultsList);
			this->PanelDockContainerFindResults->Controls->Add(this->FindResultsToolbar);
			this->PanelDockContainerFindResults->DisabledBackColor = System::Drawing::Color::Empty;
			this->PanelDockContainerFindResults->Location = System::Drawing::Point(3, 23);
			this->PanelDockContainerFindResults->Name = L"PanelDockContainerFindResults";
			this->PanelDockContainerFindResults->Size = System::Drawing::Size(889, 144);
			this->PanelDockContainerFindResults->Style->Alignment = System::Drawing::StringAlignment::Center;
			this->PanelDockContainerFindResults->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
			this->PanelDockContainerFindResults->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::ItemText;
			this->PanelDockContainerFindResults->Style->GradientAngle = 90;
			this->PanelDockContainerFindResults->TabIndex = 9;
			//
			// FindResultsList
			//
			this->FindResultsList->AllColumns->Add(this->FindResultsListColumnLine);
			this->FindResultsList->AllColumns->Add(this->FindResultsListColumnText);
			this->FindResultsList->AllColumns->Add(this->FindResultsListColumnHits);
			this->FindResultsList->BackColor = System::Drawing::Color::Black;
			this->FindResultsList->CellEditUseWholeCell = false;
			this->FindResultsList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {
				this->FindResultsListColumnLine,
					this->FindResultsListColumnText, this->FindResultsListColumnHits
			});
			this->FindResultsList->Cursor = System::Windows::Forms::Cursors::Default;
			this->FindResultsList->Dock = System::Windows::Forms::DockStyle::Fill;
			this->FindResultsList->EmptyListMsg = L"Doesn\'t look like anything to me...";
			this->FindResultsList->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->FindResultsList->ForeColor = System::Drawing::Color::White;
			this->FindResultsList->FullRowSelect = true;
			this->FindResultsList->GridLines = true;
			this->FindResultsList->HideSelection = false;
			this->FindResultsList->Location = System::Drawing::Point(0, 19);
			this->FindResultsList->MultiSelect = false;
			this->FindResultsList->Name = L"FindResultsList";
			this->FindResultsList->ShowGroups = false;
			this->FindResultsList->Size = System::Drawing::Size(889, 125);
			this->FindResultsList->TabIndex = 2;
			this->FindResultsList->UseCompatibleStateImageBehavior = false;
			this->FindResultsList->View = System::Windows::Forms::View::Details;
			this->FindResultsList->VirtualMode = true;
			//
			// FindResultsListColumnLine
			//
			this->FindResultsListColumnLine->MaximumWidth = 40;
			this->FindResultsListColumnLine->MinimumWidth = 50;
			this->FindResultsListColumnLine->Text = L"Line";
			this->FindResultsListColumnLine->Width = 40;
			//
			// FindResultsListColumnText
			//
			this->FindResultsListColumnText->MinimumWidth = 500;
			this->FindResultsListColumnText->Text = L"Code";
			this->FindResultsListColumnText->Width = 779;
			//
			// FindResultsListColumnHits
			//
			this->FindResultsListColumnHits->MaximumWidth = 40;
			this->FindResultsListColumnHits->Text = L"Hits";
			this->FindResultsListColumnHits->Width = 40;
			//
			// FindResultsToolbar
			//
			this->FindResultsToolbar->AntiAlias = true;
			this->FindResultsToolbar->Dock = System::Windows::Forms::DockStyle::Top;
			this->FindResultsToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->FindResultsToolbar->IsMaximized = false;
			this->FindResultsToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->FindResultsToolbarLabel,
					this->FindResultsListToolbarLabelQuery
			});
			this->FindResultsToolbar->Location = System::Drawing::Point(0, 0);
			this->FindResultsToolbar->Name = L"FindResultsToolbar";
			this->FindResultsToolbar->Size = System::Drawing::Size(889, 19);
			this->FindResultsToolbar->Stretch = true;
			this->FindResultsToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindResultsToolbar->TabIndex = 0;
			this->FindResultsToolbar->TabStop = false;
			this->FindResultsToolbar->Text = L"bar1";
			//
			// FindResultsToolbarLabel
			//
			this->FindResultsToolbarLabel->Name = L"FindResultsToolbarLabel";
			this->FindResultsToolbarLabel->Text = L"Query :";
			//
			// FindResultsListToolbarLabelQuery
			//
			this->FindResultsListToolbarLabelQuery->Name = L"FindResultsListToolbarLabelQuery";
			this->FindResultsListToolbarLabelQuery->Text = L"Last Find/Replace Operation";
			//
			// DockContainerItemMessageList
			//
			this->DockContainerItemMessageList->Control = this->PanelDockContainerMessageList;
			this->DockContainerItemMessageList->Name = L"DockContainerItemMessageList";
			this->DockContainerItemMessageList->Text = L"Messages";
			//
			// DockContainerItemBookmarks
			//
			this->DockContainerItemBookmarks->Control = this->PanelDockContainerBookmarks;
			this->DockContainerItemBookmarks->Name = L"DockContainerItemBookmarks";
			this->DockContainerItemBookmarks->Text = L"Bookmarks";
			//
			// DockContainerItemFindResults
			//
			this->DockContainerItemFindResults->Control = this->PanelDockContainerFindResults;
			this->DockContainerItemFindResults->Name = L"DockContainerItemFindResults";
			this->DockContainerItemFindResults->Text = L"Find and Replace Results";
			//
			// DockContainerItemGlobalFindResults
			//
			this->DockContainerItemGlobalFindResults->Control = this->PanelDockContainerGlobalFindResults;
			this->DockContainerItemGlobalFindResults->Name = L"DockContainerItemGlobalFindResults";
			this->DockContainerItemGlobalFindResults->Text = L"Find in Tabs Results";
			//
			// DockSiteLeftEx
			//
			this->DockSiteLeftEx->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteLeftEx->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteLeftEx->Controls->Add(this->LeftDockBar);
			this->DockSiteLeftEx->Dock = System::Windows::Forms::DockStyle::Left;
			this->DockSiteLeftEx->DocumentDockContainer = (gcnew DevComponents::DotNetBar::DocumentDockContainer(gcnew cli::array< DevComponents::DotNetBar::DocumentBaseContainer^  >(1) {
				(cli::safe_cast<DevComponents::DotNetBar::DocumentBaseContainer^>((gcnew DevComponents::DotNetBar::DocumentBarContainer(this->LeftDockBar,
					88, 572))))
			}, DevComponents::DotNetBar::eOrientation::Horizontal));
			this->DockSiteLeftEx->ForeColor = System::Drawing::Color::White;
			this->DockSiteLeftEx->Location = System::Drawing::Point(0, 64);
			this->DockSiteLeftEx->Name = L"DockSiteLeftEx";
			this->DockSiteLeftEx->Size = System::Drawing::Size(93, 572);
			this->DockSiteLeftEx->TabIndex = 4;
			this->DockSiteLeftEx->TabStop = false;
			//
			// LeftDockBar
			//
			this->LeftDockBar->AccessibleDescription = L"DotNetBar Bar (LeftDockBar)";
			this->LeftDockBar->AccessibleName = L"DotNetBar Bar";
			this->LeftDockBar->AccessibleRole = System::Windows::Forms::AccessibleRole::Grouping;
			this->LeftDockBar->AutoSyncBarCaption = true;
			this->LeftDockBar->CanDockTop = false;
			this->LeftDockBar->CanHide = true;
			this->LeftDockBar->CloseSingleTab = true;
			this->LeftDockBar->Controls->Add(this->PanelDockContainerOutline);
			this->LeftDockBar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->LeftDockBar->GrabHandleStyle = DevComponents::DotNetBar::eGrabHandleStyle::Caption;
			this->LeftDockBar->IsMaximized = false;
			this->LeftDockBar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(1) { this->DockContainerItemOutline });
			this->LeftDockBar->LayoutType = DevComponents::DotNetBar::eLayoutType::DockContainer;
			this->LeftDockBar->Location = System::Drawing::Point(0, 0);
			this->LeftDockBar->Name = L"LeftDockBar";
			this->LeftDockBar->Size = System::Drawing::Size(88, 572);
			this->LeftDockBar->Stretch = true;
			this->LeftDockBar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->LeftDockBar->TabIndex = 1;
			this->LeftDockBar->TabStop = false;
			this->LeftDockBar->Text = L"Outline View";
			//
			// PanelDockContainerOutline
			//
			this->PanelDockContainerOutline->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->PanelDockContainerOutline->Controls->Add(this->OutlineTreeView);
			this->PanelDockContainerOutline->DisabledBackColor = System::Drawing::Color::Empty;
			this->PanelDockContainerOutline->Location = System::Drawing::Point(3, 23);
			this->PanelDockContainerOutline->Name = L"PanelDockContainerOutline";
			this->PanelDockContainerOutline->Size = System::Drawing::Size(82, 546);
			this->PanelDockContainerOutline->Style->Alignment = System::Drawing::StringAlignment::Center;
			this->PanelDockContainerOutline->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
			this->PanelDockContainerOutline->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::ItemText;
			this->PanelDockContainerOutline->Style->GradientAngle = 90;
			this->PanelDockContainerOutline->TabIndex = 0;
			//
			// OutlineTreeView
			//
			this->OutlineTreeView->BackColor = System::Drawing::Color::Black;
			this->OutlineTreeView->CellEditUseWholeCell = false;
			this->OutlineTreeView->Dock = System::Windows::Forms::DockStyle::Fill;
			this->OutlineTreeView->ForeColor = System::Drawing::Color::White;
			this->OutlineTreeView->HideSelection = false;
			this->OutlineTreeView->Location = System::Drawing::Point(0, 0);
			this->OutlineTreeView->Name = L"OutlineTreeView";
			this->OutlineTreeView->ShowGroups = false;
			this->OutlineTreeView->Size = System::Drawing::Size(82, 546);
			this->OutlineTreeView->TabIndex = 0;
			this->OutlineTreeView->UseCompatibleStateImageBehavior = false;
			this->OutlineTreeView->View = System::Windows::Forms::View::Details;
			this->OutlineTreeView->VirtualMode = true;
			//
			// DockContainerItemOutline
			//
			this->DockContainerItemOutline->Control = this->PanelDockContainerOutline;
			this->DockContainerItemOutline->Name = L"DockContainerItemOutline";
			this->DockContainerItemOutline->Text = L"Outline View";
			//
			// DockSiteRightEx
			//
			this->DockSiteRightEx->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteRightEx->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteRightEx->Controls->Add(this->RightDockBar);
			this->DockSiteRightEx->Dock = System::Windows::Forms::DockStyle::Right;
			this->DockSiteRightEx->DocumentDockContainer = (gcnew DevComponents::DotNetBar::DocumentDockContainer(gcnew cli::array< DevComponents::DotNetBar::DocumentBaseContainer^  >(1) {
				(cli::safe_cast<DevComponents::DotNetBar::DocumentBaseContainer^>((gcnew DevComponents::DotNetBar::DocumentBarContainer(this->RightDockBar,
					172, 572))))
			}, DevComponents::DotNetBar::eOrientation::Vertical));
			this->DockSiteRightEx->ForeColor = System::Drawing::Color::White;
			this->DockSiteRightEx->Location = System::Drawing::Point(718, 64);
			this->DockSiteRightEx->Name = L"DockSiteRightEx";
			this->DockSiteRightEx->Size = System::Drawing::Size(177, 572);
			this->DockSiteRightEx->TabIndex = 5;
			this->DockSiteRightEx->TabStop = false;
			//
			// RightDockBar
			//
			this->RightDockBar->AccessibleDescription = L"DotNetBar Bar (RightDockBar)";
			this->RightDockBar->AccessibleName = L"DotNetBar Bar";
			this->RightDockBar->AccessibleRole = System::Windows::Forms::AccessibleRole::Grouping;
			this->RightDockBar->AutoSyncBarCaption = true;
			this->RightDockBar->CanDockTop = false;
			this->RightDockBar->CanHide = true;
			this->RightDockBar->CloseSingleTab = true;
			this->RightDockBar->Controls->Add(this->PanelDockContainerFindReplace);
			this->RightDockBar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->RightDockBar->GrabHandleStyle = DevComponents::DotNetBar::eGrabHandleStyle::Caption;
			this->RightDockBar->IsMaximized = false;
			this->RightDockBar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(1) { this->DockContainerItemFindReplace });
			this->RightDockBar->LayoutType = DevComponents::DotNetBar::eLayoutType::DockContainer;
			this->RightDockBar->Location = System::Drawing::Point(5, 0);
			this->RightDockBar->Name = L"RightDockBar";
			this->RightDockBar->Size = System::Drawing::Size(172, 572);
			this->RightDockBar->Stretch = true;
			this->RightDockBar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->RightDockBar->TabIndex = 0;
			this->RightDockBar->TabStop = false;
			this->RightDockBar->Text = L"Find and Replace";
			//
			// PanelDockContainerFindReplace
			//
			this->PanelDockContainerFindReplace->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->PanelDockContainerFindReplace->Controls->Add(this->FindWindowLayoutControl);
			this->PanelDockContainerFindReplace->DisabledBackColor = System::Drawing::Color::Empty;
			this->PanelDockContainerFindReplace->Location = System::Drawing::Point(3, 23);
			this->PanelDockContainerFindReplace->Name = L"PanelDockContainerFindReplace";
			this->PanelDockContainerFindReplace->Size = System::Drawing::Size(166, 546);
			this->PanelDockContainerFindReplace->Style->Alignment = System::Drawing::StringAlignment::Center;
			this->PanelDockContainerFindReplace->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
			this->PanelDockContainerFindReplace->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::ItemText;
			this->PanelDockContainerFindReplace->Style->GradientAngle = 90;
			this->PanelDockContainerFindReplace->TabIndex = 0;
			//
			// FindWindowLayoutControl
			//
			this->FindWindowLayoutControl->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowDropdownFind);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowDropdownReplace);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowCheckboxMatchCase);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowCheckboxUseRegEx);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowCheckBoxMatchWholeWord);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowComboLookIn);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowButtonFind);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowButtonReplace);
			this->FindWindowLayoutControl->Controls->Add(this->FindWindowButtonCountMatches);
			this->FindWindowLayoutControl->Dock = System::Windows::Forms::DockStyle::Fill;
			this->FindWindowLayoutControl->ForeColor = System::Drawing::Color::White;
			this->FindWindowLayoutControl->Location = System::Drawing::Point(0, 0);
			this->FindWindowLayoutControl->Name = L"FindWindowLayoutControl";
			//
			//
			//
			this->FindWindowLayoutControl->RootGroup->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::Layout::LayoutItemBase^  >(5) {
				this->FindWindowLCIFindDropdown,
					this->FindWindowLCIReplaceDropdown, this->FindWindowLCILookIn, this->FindWindowLayoutGroupSettings, this->FindWindowLayouyGroupButtons
			});
			this->FindWindowLayoutControl->Size = System::Drawing::Size(166, 546);
			this->FindWindowLayoutControl->TabIndex = 19;
			//
			// FindWindowDropdownFind
			//
			this->FindWindowDropdownFind->BackColor = System::Drawing::Color::Black;
			//
			//
			//
			this->FindWindowDropdownFind->BackgroundStyle->Class = L"TextBoxBorder";
			this->FindWindowDropdownFind->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->FindWindowDropdownFind->ButtonDropDown->Visible = true;
			this->FindWindowDropdownFind->ForeColor = System::Drawing::Color::White;
			this->FindWindowDropdownFind->Location = System::Drawing::Point(50, 4);
			this->FindWindowDropdownFind->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowDropdownFind->Name = L"FindWindowDropdownFind";
			this->FindWindowDropdownFind->Size = System::Drawing::Size(112, 22);
			this->FindWindowDropdownFind->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowDropdownFind->TabIndex = 0;
			this->FindWindowDropdownFind->Text = L"";
			//
			// FindWindowDropdownReplace
			//
			this->FindWindowDropdownReplace->BackColor = System::Drawing::Color::Black;
			//
			//
			//
			this->FindWindowDropdownReplace->BackgroundStyle->Class = L"TextBoxBorder";
			this->FindWindowDropdownReplace->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->FindWindowDropdownReplace->ButtonDropDown->Visible = true;
			this->FindWindowDropdownReplace->ForeColor = System::Drawing::Color::White;
			this->FindWindowDropdownReplace->Location = System::Drawing::Point(50, 34);
			this->FindWindowDropdownReplace->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowDropdownReplace->Name = L"FindWindowDropdownReplace";
			this->FindWindowDropdownReplace->Size = System::Drawing::Size(112, 22);
			this->FindWindowDropdownReplace->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowDropdownReplace->TabIndex = 1;
			this->FindWindowDropdownReplace->Text = L"";
			//
			// FindWindowCheckboxMatchCase
			//
			this->FindWindowCheckboxMatchCase->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
			//
			//
			//
			this->FindWindowCheckboxMatchCase->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->FindWindowCheckboxMatchCase->ForeColor = System::Drawing::Color::White;
			this->FindWindowCheckboxMatchCase->Location = System::Drawing::Point(8, 98);
			this->FindWindowCheckboxMatchCase->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowCheckboxMatchCase->Name = L"FindWindowCheckboxMatchCase";
			this->FindWindowCheckboxMatchCase->Size = System::Drawing::Size(150, 22);
			this->FindWindowCheckboxMatchCase->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowCheckboxMatchCase->TabIndex = 3;
			this->FindWindowCheckboxMatchCase->Text = L"Match case";
			//
			// FindWindowCheckboxUseRegEx
			//
			this->FindWindowCheckboxUseRegEx->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
			//
			//
			//
			this->FindWindowCheckboxUseRegEx->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->FindWindowCheckboxUseRegEx->ForeColor = System::Drawing::Color::White;
			this->FindWindowCheckboxUseRegEx->Location = System::Drawing::Point(8, 158);
			this->FindWindowCheckboxUseRegEx->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowCheckboxUseRegEx->Name = L"FindWindowCheckboxUseRegEx";
			this->FindWindowCheckboxUseRegEx->Size = System::Drawing::Size(150, 22);
			this->FindWindowCheckboxUseRegEx->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowCheckboxUseRegEx->TabIndex = 5;
			this->FindWindowCheckboxUseRegEx->Text = L"Use regular expressions";
			//
			// FindWindowCheckBoxMatchWholeWord
			//
			this->FindWindowCheckBoxMatchWholeWord->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(48)));
			//
			//
			//
			this->FindWindowCheckBoxMatchWholeWord->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->FindWindowCheckBoxMatchWholeWord->ForeColor = System::Drawing::Color::White;
			this->FindWindowCheckBoxMatchWholeWord->Location = System::Drawing::Point(8, 128);
			this->FindWindowCheckBoxMatchWholeWord->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowCheckBoxMatchWholeWord->Name = L"FindWindowCheckBoxMatchWholeWord";
			this->FindWindowCheckBoxMatchWholeWord->Size = System::Drawing::Size(150, 22);
			this->FindWindowCheckBoxMatchWholeWord->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowCheckBoxMatchWholeWord->TabIndex = 4;
			this->FindWindowCheckBoxMatchWholeWord->Text = L"Match whole word";
			//
			// FindWindowComboLookIn
			//
			this->FindWindowComboLookIn->DisplayMember = L"Text";
			this->FindWindowComboLookIn->DrawMode = System::Windows::Forms::DrawMode::OwnerDrawFixed;
			this->FindWindowComboLookIn->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->FindWindowComboLookIn->ForeColor = System::Drawing::Color::White;
			this->FindWindowComboLookIn->FormattingEnabled = true;
			this->FindWindowComboLookIn->ItemHeight = 17;
			this->FindWindowComboLookIn->Items->AddRange(gcnew cli::array< System::Object^  >(3) {
				this->FindWindowDropdownLookInSelection,
					this->FindWindowDropdownLookInCurrentScript, this->FindWindowDropdownLookInAllOpenScripts
			});
			this->FindWindowComboLookIn->Location = System::Drawing::Point(50, 64);
			this->FindWindowComboLookIn->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowComboLookIn->Name = L"FindWindowComboLookIn";
			this->FindWindowComboLookIn->Size = System::Drawing::Size(112, 23);
			this->FindWindowComboLookIn->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowComboLookIn->TabIndex = 2;
			//
			// FindWindowDropdownLookInSelection
			//
			this->FindWindowDropdownLookInSelection->Text = L"Selection (Current Script)";
			//
			// FindWindowDropdownLookInCurrentScript
			//
			this->FindWindowDropdownLookInCurrentScript->Text = L"Entire Text (Current Script)";
			//
			// FindWindowDropdownLookInAllOpenScripts
			//
			this->FindWindowDropdownLookInAllOpenScripts->Text = L"Entire Text (All Open Scripts)";
			//
			// FindWindowButtonFind
			//
			this->FindWindowButtonFind->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->FindWindowButtonFind->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->FindWindowButtonFind->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"FindWindowButtonFind.Image")));
			this->FindWindowButtonFind->ImageTextSpacing = 5;
			this->FindWindowButtonFind->Location = System::Drawing::Point(8, 198);
			this->FindWindowButtonFind->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowButtonFind->Name = L"FindWindowButtonFind";
			this->FindWindowButtonFind->Size = System::Drawing::Size(71, 28);
			this->FindWindowButtonFind->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowButtonFind->TabIndex = 7;
			this->FindWindowButtonFind->Text = L"Find";
			//
			// FindWindowButtonReplace
			//
			this->FindWindowButtonReplace->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->FindWindowButtonReplace->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->FindWindowButtonReplace->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"FindWindowButtonReplace.Image")));
			this->FindWindowButtonReplace->ImageTextSpacing = 5;
			this->FindWindowButtonReplace->Location = System::Drawing::Point(87, 198);
			this->FindWindowButtonReplace->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowButtonReplace->Name = L"FindWindowButtonReplace";
			this->FindWindowButtonReplace->Size = System::Drawing::Size(71, 28);
			this->FindWindowButtonReplace->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowButtonReplace->TabIndex = 8;
			this->FindWindowButtonReplace->Text = L"Replace";
			//
			// FindWindowButtonCountMatches
			//
			this->FindWindowButtonCountMatches->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->FindWindowButtonCountMatches->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->FindWindowButtonCountMatches->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"FindWindowButtonCountMatches.Image")));
			this->FindWindowButtonCountMatches->ImageTextSpacing = 5;
			this->FindWindowButtonCountMatches->Location = System::Drawing::Point(8, 234);
			this->FindWindowButtonCountMatches->Margin = System::Windows::Forms::Padding(0);
			this->FindWindowButtonCountMatches->Name = L"FindWindowButtonCountMatches";
			this->FindWindowButtonCountMatches->Size = System::Drawing::Size(150, 28);
			this->FindWindowButtonCountMatches->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->FindWindowButtonCountMatches->TabIndex = 9;
			this->FindWindowButtonCountMatches->Text = L"Count Matches";
			//
			// FindWindowLCIFindDropdown
			//
			this->FindWindowLCIFindDropdown->Control = this->FindWindowDropdownFind;
			this->FindWindowLCIFindDropdown->Height = 30;
			this->FindWindowLCIFindDropdown->MinSize = System::Drawing::Size(64, 18);
			this->FindWindowLCIFindDropdown->Name = L"FindWindowLCIFindDropdown";
			this->FindWindowLCIFindDropdown->Text = L"Find";
			this->FindWindowLCIFindDropdown->Width = 100;
			this->FindWindowLCIFindDropdown->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCIReplaceDropdown
			//
			this->FindWindowLCIReplaceDropdown->Control = this->FindWindowDropdownReplace;
			this->FindWindowLCIReplaceDropdown->Height = 30;
			this->FindWindowLCIReplaceDropdown->MinSize = System::Drawing::Size(64, 18);
			this->FindWindowLCIReplaceDropdown->Name = L"FindWindowLCIReplaceDropdown";
			this->FindWindowLCIReplaceDropdown->Text = L"Replace";
			this->FindWindowLCIReplaceDropdown->Width = 100;
			this->FindWindowLCIReplaceDropdown->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCILookIn
			//
			this->FindWindowLCILookIn->Control = this->FindWindowComboLookIn;
			this->FindWindowLCILookIn->Height = 30;
			this->FindWindowLCILookIn->MinSize = System::Drawing::Size(64, 18);
			this->FindWindowLCILookIn->Name = L"FindWindowLCILookIn";
			this->FindWindowLCILookIn->Text = L"Look in";
			this->FindWindowLCILookIn->Width = 100;
			this->FindWindowLCILookIn->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLayoutGroupSettings
			//
			this->FindWindowLayoutGroupSettings->Height = 100;
			this->FindWindowLayoutGroupSettings->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::Layout::LayoutItemBase^  >(3) {
				this->FindWindowLCIMatchCase,
					this->FindWindowLCIMatchWholeWord, this->FindWindowLCIUseRegEx
			});
			this->FindWindowLayoutGroupSettings->MinSize = System::Drawing::Size(120, 32);
			this->FindWindowLayoutGroupSettings->Name = L"FindWindowLayoutGroupSettings";
			this->FindWindowLayoutGroupSettings->TextPosition = DevComponents::DotNetBar::Layout::eLayoutPosition::Top;
			this->FindWindowLayoutGroupSettings->Width = 100;
			this->FindWindowLayoutGroupSettings->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCIMatchCase
			//
			this->FindWindowLCIMatchCase->Control = this->FindWindowCheckboxMatchCase;
			this->FindWindowLCIMatchCase->Height = 30;
			this->FindWindowLCIMatchCase->MinSize = System::Drawing::Size(64, 18);
			this->FindWindowLCIMatchCase->Name = L"FindWindowLCIMatchCase";
			this->FindWindowLCIMatchCase->Width = 100;
			this->FindWindowLCIMatchCase->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCIMatchWholeWord
			//
			this->FindWindowLCIMatchWholeWord->Control = this->FindWindowCheckBoxMatchWholeWord;
			this->FindWindowLCIMatchWholeWord->Height = 30;
			this->FindWindowLCIMatchWholeWord->MinSize = System::Drawing::Size(64, 18);
			this->FindWindowLCIMatchWholeWord->Name = L"FindWindowLCIMatchWholeWord";
			this->FindWindowLCIMatchWholeWord->Width = 100;
			this->FindWindowLCIMatchWholeWord->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCIUseRegEx
			//
			this->FindWindowLCIUseRegEx->Control = this->FindWindowCheckboxUseRegEx;
			this->FindWindowLCIUseRegEx->Height = 30;
			this->FindWindowLCIUseRegEx->MinSize = System::Drawing::Size(64, 18);
			this->FindWindowLCIUseRegEx->Name = L"FindWindowLCIUseRegEx";
			this->FindWindowLCIUseRegEx->Width = 100;
			this->FindWindowLCIUseRegEx->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLayouyGroupButtons
			//
			this->FindWindowLayouyGroupButtons->Height = 100;
			this->FindWindowLayouyGroupButtons->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::Layout::LayoutItemBase^  >(3) {
				this->FindWindowLCIFindButton,
					this->FindWindowLCIReplaceButton, this->FindWindowLCICountMatches
			});
			this->FindWindowLayouyGroupButtons->MinSize = System::Drawing::Size(120, 32);
			this->FindWindowLayouyGroupButtons->Name = L"FindWindowLayouyGroupButtons";
			this->FindWindowLayouyGroupButtons->TextPosition = DevComponents::DotNetBar::Layout::eLayoutPosition::Top;
			this->FindWindowLayouyGroupButtons->Width = 100;
			this->FindWindowLayouyGroupButtons->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCIFindButton
			//
			this->FindWindowLCIFindButton->Control = this->FindWindowButtonFind;
			this->FindWindowLCIFindButton->Height = 36;
			this->FindWindowLCIFindButton->MinSize = System::Drawing::Size(32, 20);
			this->FindWindowLCIFindButton->Name = L"FindWindowLCIFindButton";
			this->FindWindowLCIFindButton->Width = 50;
			this->FindWindowLCIFindButton->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCIReplaceButton
			//
			this->FindWindowLCIReplaceButton->Control = this->FindWindowButtonReplace;
			this->FindWindowLCIReplaceButton->Height = 36;
			this->FindWindowLCIReplaceButton->MinSize = System::Drawing::Size(32, 20);
			this->FindWindowLCIReplaceButton->Name = L"FindWindowLCIReplaceButton";
			this->FindWindowLCIReplaceButton->Width = 50;
			this->FindWindowLCIReplaceButton->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// FindWindowLCICountMatches
			//
			this->FindWindowLCICountMatches->Control = this->FindWindowButtonCountMatches;
			this->FindWindowLCICountMatches->Height = 36;
			this->FindWindowLCICountMatches->MinSize = System::Drawing::Size(32, 20);
			this->FindWindowLCICountMatches->Name = L"FindWindowLCICountMatches";
			this->FindWindowLCICountMatches->Width = 100;
			this->FindWindowLCICountMatches->WidthType = DevComponents::DotNetBar::Layout::eLayoutSizeType::Percent;
			//
			// DockContainerItemFindReplace
			//
			this->DockContainerItemFindReplace->Control = this->PanelDockContainerFindReplace;
			this->DockContainerItemFindReplace->Name = L"DockContainerItemFindReplace";
			this->DockContainerItemFindReplace->Text = L"Find and Replace";
			//
			// DockSiteBottomEx
			//
			this->DockSiteBottomEx->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteBottomEx->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteBottomEx->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->DockSiteBottomEx->ForeColor = System::Drawing::Color::White;
			this->DockSiteBottomEx->Location = System::Drawing::Point(0, 836);
			this->DockSiteBottomEx->Name = L"DockSiteBottomEx";
			this->DockSiteBottomEx->Size = System::Drawing::Size(895, 0);
			this->DockSiteBottomEx->TabIndex = 11;
			this->DockSiteBottomEx->TabStop = false;
			//
			// DockSiteLeft
			//
			this->DockSiteLeft->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteLeft->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteLeft->Dock = System::Windows::Forms::DockStyle::Left;
			this->DockSiteLeft->ForeColor = System::Drawing::Color::White;
			this->DockSiteLeft->Location = System::Drawing::Point(0, 64);
			this->DockSiteLeft->Name = L"DockSiteLeft";
			this->DockSiteLeft->Size = System::Drawing::Size(0, 772);
			this->DockSiteLeft->TabIndex = 8;
			this->DockSiteLeft->TabStop = false;
			//
			// DockSiteRight
			//
			this->DockSiteRight->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteRight->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteRight->Dock = System::Windows::Forms::DockStyle::Right;
			this->DockSiteRight->ForeColor = System::Drawing::Color::White;
			this->DockSiteRight->Location = System::Drawing::Point(895, 64);
			this->DockSiteRight->Name = L"DockSiteRight";
			this->DockSiteRight->Size = System::Drawing::Size(0, 772);
			this->DockSiteRight->TabIndex = 9;
			this->DockSiteRight->TabStop = false;
			//
			// DockSiteTopEx
			//
			this->DockSiteTopEx->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteTopEx->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteTopEx->Dock = System::Windows::Forms::DockStyle::Top;
			this->DockSiteTopEx->Enabled = false;
			this->DockSiteTopEx->ForeColor = System::Drawing::Color::White;
			this->DockSiteTopEx->Location = System::Drawing::Point(0, 64);
			this->DockSiteTopEx->Name = L"DockSiteTopEx";
			this->DockSiteTopEx->Size = System::Drawing::Size(895, 0);
			this->DockSiteTopEx->TabIndex = 10;
			this->DockSiteTopEx->TabStop = false;
			//
			// DockSiteTop
			//
			this->DockSiteTop->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteTop->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteTop->Dock = System::Windows::Forms::DockStyle::Top;
			this->DockSiteTop->DocumentDockContainer = (gcnew DevComponents::DotNetBar::DocumentDockContainer());
			this->DockSiteTop->Enabled = false;
			this->DockSiteTop->ForeColor = System::Drawing::Color::White;
			this->DockSiteTop->Location = System::Drawing::Point(0, 64);
			this->DockSiteTop->Name = L"DockSiteTop";
			this->DockSiteTop->Size = System::Drawing::Size(895, 0);
			this->DockSiteTop->TabIndex = 6;
			this->DockSiteTop->TabStop = false;
			//
			// DockSiteCenter
			//
			this->DockSiteCenter->AccessibleRole = System::Windows::Forms::AccessibleRole::Window;
			this->DockSiteCenter->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			this->DockSiteCenter->Controls->Add(this->CenterDockBar);
			this->DockSiteCenter->Dock = System::Windows::Forms::DockStyle::Fill;
			this->DockSiteCenter->DocumentDockContainer = (gcnew DevComponents::DotNetBar::DocumentDockContainer(gcnew cli::array< DevComponents::DotNetBar::DocumentBaseContainer^  >(1) {
				(cli::safe_cast<DevComponents::DotNetBar::DocumentBaseContainer^>((gcnew DevComponents::DotNetBar::DocumentBarContainer(this->CenterDockBar,
					625, 572))))
			}, DevComponents::DotNetBar::eOrientation::Horizontal));
			this->DockSiteCenter->ForeColor = System::Drawing::Color::White;
			this->DockSiteCenter->Location = System::Drawing::Point(93, 64);
			this->DockSiteCenter->Name = L"DockSiteCenter";
			this->DockSiteCenter->Size = System::Drawing::Size(625, 572);
			this->DockSiteCenter->TabIndex = 12;
			this->DockSiteCenter->TabStop = false;
			//
			// CenterDockBar
			//
			this->CenterDockBar->AccessibleDescription = L"DotNetBar Bar (CenterDockBar)";
			this->CenterDockBar->AccessibleName = L"DotNetBar Bar";
			this->CenterDockBar->AccessibleRole = System::Windows::Forms::AccessibleRole::Grouping;
			this->CenterDockBar->BarType = DevComponents::DotNetBar::eBarType::DockWindow;
			this->CenterDockBar->CanCustomize = false;
			this->CenterDockBar->CanDockBottom = false;
			this->CenterDockBar->CanDockDocument = true;
			this->CenterDockBar->CanDockLeft = false;
			this->CenterDockBar->CanDockRight = false;
			this->CenterDockBar->CanDockTop = false;
			this->CenterDockBar->CanHide = true;
			this->CenterDockBar->CanMove = false;
			this->CenterDockBar->CanReorderTabs = false;
			this->CenterDockBar->CanUndock = false;
			this->CenterDockBar->Controls->Add(this->PanelDocumentContent);
			this->CenterDockBar->Controls->Add(this->NavigationBar);
			this->CenterDockBar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->CenterDockBar->GrabHandleStyle = DevComponents::DotNetBar::eGrabHandleStyle::Caption;
			this->CenterDockBar->IsMaximized = false;
			this->CenterDockBar->LayoutType = DevComponents::DotNetBar::eLayoutType::DockContainer;
			this->CenterDockBar->Location = System::Drawing::Point(0, 0);
			this->CenterDockBar->Name = L"CenterDockBar";
			this->CenterDockBar->Size = System::Drawing::Size(625, 572);
			this->CenterDockBar->Stretch = true;
			this->CenterDockBar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->CenterDockBar->TabIndex = 0;
			this->CenterDockBar->TabStop = false;
			//
			// PanelDocumentContent
			//
			this->PanelDocumentContent->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(85)),
				static_cast<System::Int32>(static_cast<System::Byte>(85)), static_cast<System::Int32>(static_cast<System::Byte>(91)));
			this->ContextMenuProvider->SetContextMenuEx(this->PanelDocumentContent, this->ContextMenuTextEditor);
			this->PanelDocumentContent->Controls->Add(this->ContextMenuProvider);
			this->PanelDocumentContent->Dock = System::Windows::Forms::DockStyle::Fill;
			this->PanelDocumentContent->ForeColor = System::Drawing::Color::White;
			this->PanelDocumentContent->Location = System::Drawing::Point(0, 22);
			this->PanelDocumentContent->Name = L"PanelDocumentContent";
			this->PanelDocumentContent->Size = System::Drawing::Size(625, 550);
			this->PanelDocumentContent->TabIndex = 10;
			//
			// ContextMenuProvider
			//
			this->ContextMenuProvider->AntiAlias = true;
			this->ContextMenuProvider->DockSide = DevComponents::DotNetBar::eDockSide::Document;
			this->ContextMenuProvider->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->ContextMenuProvider->IsMaximized = false;
			this->ContextMenuProvider->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(1) { this->ContextMenuTextEditor });
			this->ContextMenuProvider->Location = System::Drawing::Point(6, 6);
			this->ContextMenuProvider->Name = L"ContextMenuProvider";
			this->ContextMenuProvider->Size = System::Drawing::Size(113, 25);
			this->ContextMenuProvider->Stretch = true;
			this->ContextMenuProvider->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->ContextMenuProvider->TabIndex = 0;
			this->ContextMenuProvider->TabStop = false;
			//
			// ContextMenuTextEditor
			//
			this->ContextMenuTextEditor->AutoExpandOnClick = true;
			this->ContextMenuTextEditor->Name = L"ContextMenuTextEditor";
			this->ContextMenuTextEditor->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(6) {
				this->TextEditorContextMenuCopy,
					this->TextEditorContextMenuPaste, this->TextEditorContextMenuToggleComment, this->TextEditorContextMenuAddBookmark, this->TextEditorContextMenuOpenPreprocessorImport,
					this->TextEditorContextMenuJumpToScript
			});
			this->ContextMenuTextEditor->Text = L"TE";
			//
			// TextEditorContextMenuCopy
			//
			this->TextEditorContextMenuCopy->AlternateShortCutText = L"Ctrl + C";
			this->TextEditorContextMenuCopy->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"TextEditorContextMenuCopy.Image")));
			this->TextEditorContextMenuCopy->Name = L"TextEditorContextMenuCopy";
			this->TextEditorContextMenuCopy->Text = L"Copy";
			//
			// TextEditorContextMenuPaste
			//
			this->TextEditorContextMenuPaste->AlternateShortCutText = L"Ctrl + V";
			this->TextEditorContextMenuPaste->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"TextEditorContextMenuPaste.Image")));
			this->TextEditorContextMenuPaste->Name = L"TextEditorContextMenuPaste";
			this->TextEditorContextMenuPaste->Text = L"Paste";
			//
			// TextEditorContextMenuToggleComment
			//
			this->TextEditorContextMenuToggleComment->AlternateShortCutText = L"Ctrl + Q/W";
			this->TextEditorContextMenuToggleComment->BeginGroup = true;
			this->TextEditorContextMenuToggleComment->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"TextEditorContextMenuToggleComment.Image")));
			this->TextEditorContextMenuToggleComment->Name = L"TextEditorContextMenuToggleComment";
			this->TextEditorContextMenuToggleComment->Text = L"Toggle Comment";
			//
			// TextEditorContextMenuAddBookmark
			//
			this->TextEditorContextMenuAddBookmark->AlternateShortCutText = L"Ctrl + B";
			this->TextEditorContextMenuAddBookmark->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"TextEditorContextMenuAddBookmark.Image")));
			this->TextEditorContextMenuAddBookmark->Name = L"TextEditorContextMenuAddBookmark";
			this->TextEditorContextMenuAddBookmark->Text = L"Add Bookmark";
			//
			// NavigationBar
			//
			this->NavigationBar->AutoSize = true;
			this->NavigationBar->BackColor = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(45)), static_cast<System::Int32>(static_cast<System::Byte>(45)),
				static_cast<System::Int32>(static_cast<System::Byte>(48)));
			//
			//
			//
			this->NavigationBar->BackgroundStyle->Class = L"CrumbBarBackgroundKey";
			this->NavigationBar->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->NavigationBar->ContainerControlProcessDialogKey = true;
			this->NavigationBar->Dock = System::Windows::Forms::DockStyle::Top;
			this->NavigationBar->ForeColor = System::Drawing::Color::White;
			this->NavigationBar->Location = System::Drawing::Point(0, 0);
			this->NavigationBar->Name = L"NavigationBar";
			this->NavigationBar->PathSeparator = L";";
			this->NavigationBar->Size = System::Drawing::Size(625, 22);
			this->NavigationBar->Style = DevComponents::DotNetBar::eCrumbBarStyle::Office2007;
			this->NavigationBar->TabIndex = 9;
			//
			// StatusBar
			//
			this->StatusBar->AntiAlias = true;
			this->StatusBar->BarType = DevComponents::DotNetBar::eBarType::StatusBar;
			this->StatusBar->Dock = System::Windows::Forms::DockStyle::Bottom;
			this->StatusBar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->StatusBar->ForeColor = System::Drawing::Color::Black;
			this->StatusBar->IsMaximized = false;
			this->StatusBar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(4) {
				this->StatusBarLineNumber,
					this->StatusBarColumnNumber, this->StatusBarPreprocessorOutputFlag, this->StatusBarScriptBytecodeLength
			});
			this->StatusBar->ItemSpacing = 7;
			this->StatusBar->Location = System::Drawing::Point(0, 836);
			this->StatusBar->Name = L"StatusBar";
			this->StatusBar->PaddingBottom = 2;
			this->StatusBar->PaddingLeft = 15;
			this->StatusBar->PaddingRight = 15;
			this->StatusBar->PaddingTop = 2;
			this->StatusBar->Size = System::Drawing::Size(895, 30);
			this->StatusBar->Stretch = true;
			this->StatusBar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->StatusBar->TabIndex = 13;
			this->StatusBar->TabStop = false;
			//
			// StatusBarLineNumber
			//
			this->StatusBarLineNumber->Name = L"StatusBarLineNumber";
			this->StatusBarLineNumber->Text = L"Line 0";
			//
			// StatusBarColumnNumber
			//
			this->StatusBarColumnNumber->Name = L"StatusBarColumnNumber";
			this->StatusBarColumnNumber->Text = L"Column 0";
			//
			// StatusBarPreprocessorOutputFlag
			//
			this->StatusBarPreprocessorOutputFlag->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
			this->StatusBarPreprocessorOutputFlag->Name = L"StatusBarPreprocessorOutputFlag";
			this->StatusBarPreprocessorOutputFlag->Text = L"Preprocessor Output ON";
			//
			// StatusBarScriptBytecodeLength
			//
			this->StatusBarScriptBytecodeLength->BeginGroup = true;
			this->StatusBarScriptBytecodeLength->Maximum = 32768;
			this->StatusBarScriptBytecodeLength->Name = L"StatusBarScriptBytecodeLength";
			this->StatusBarScriptBytecodeLength->ProgressBarType = DevComponents::DotNetBar::eCircularProgressType::Donut;
			this->StatusBarScriptBytecodeLength->ProgressColor = System::Drawing::Color::MintCream;
			this->StatusBarScriptBytecodeLength->Text = L"Compiled Script Size:";
			this->StatusBarScriptBytecodeLength->TextPadding->Left = 5;
			this->StatusBarScriptBytecodeLength->TextPadding->Right = 5;
			//
			// ToolbarMenuHelp
			//
			this->ToolbarMenuHelp->AutoExpandOnClick = true;
			this->ToolbarMenuHelp->Name = L"ToolbarMenuHelp";
			this->ToolbarMenuHelp->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->HelpMenuWiki,
					this->HelpMenuObseDocs
			});
			this->ToolbarMenuHelp->Text = L"&HELP";
			//
			// HelpMenuWiki
			//
			this->HelpMenuWiki->Name = L"HelpMenuWiki";
			this->HelpMenuWiki->Text = L"Wiki";
			//
			// HelpMenuObseDocs
			//
			this->HelpMenuObseDocs->Name = L"HelpMenuObseDocs";
			this->HelpMenuObseDocs->Text = L"OBSE Manual";
			//
			// TextEditorContextMenuOpenPreprocessorImport
			//
			this->TextEditorContextMenuOpenPreprocessorImport->BeginGroup = true;
			this->TextEditorContextMenuOpenPreprocessorImport->Name = L"TextEditorContextMenuOpenPreprocessorImport";
			this->TextEditorContextMenuOpenPreprocessorImport->Text = L"Open Preprocessor Import File";
			//
			// TextEditorContextMenuJumpToScript
			//
			this->TextEditorContextMenuJumpToScript->AlternateShortCutText = L"Ctrl + |";
			this->TextEditorContextMenuJumpToScript->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"TextEditorContextMenuJumpToScript.Image")));
			this->TextEditorContextMenuJumpToScript->Name = L"TextEditorContextMenuJumpToScript";
			this->TextEditorContextMenuJumpToScript->Text = L"Jump to Attached Script";
			//
			// ScriptEditorWorkspace
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(895, 866);
			this->Controls->Add(this->DockSiteCenter);
			this->Controls->Add(this->DockSiteRightEx);
			this->Controls->Add(this->DockSiteLeftEx);
			this->Controls->Add(this->DockSiteTop);
			this->Controls->Add(this->DockSiteBottom);
			this->Controls->Add(this->DockSiteLeft);
			this->Controls->Add(this->DockSiteRight);
			this->Controls->Add(this->DockSiteTopEx);
			this->Controls->Add(this->DockSiteBottomEx);
			this->Controls->Add(this->ContainerMainToolbar);
			this->Controls->Add(this->MainTabStrip);
			this->Controls->Add(this->StatusBar);
			this->DoubleBuffered = true;
			this->KeyPreview = true;
			this->Name = L"ScriptEditorWorkspace";
			this->Text = L"CSE Script Editor";
			this->Load += gcnew System::EventHandler(this, &ScriptEditorWorkspace::ScriptEditorWorkspace_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->MainTabStrip))->EndInit();
			this->MainTabStrip->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ContainerMainToolbar))->EndInit();
			this->DockSiteBottom->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomDockBar))->EndInit();
			this->BottomDockBar->ResumeLayout(false);
			this->PanelDockContainerGlobalFindResults->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->GlobalFindResultsList))->EndInit();
			this->PanelDockContainerMessageList->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->MessagesList))->EndInit();
			this->PanelDockContainerBookmarks->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BookmarksList))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BookmarksToolbar))->EndInit();
			this->PanelDockContainerFindResults->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->FindResultsList))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->FindResultsToolbar))->EndInit();
			this->DockSiteLeftEx->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->LeftDockBar))->EndInit();
			this->LeftDockBar->ResumeLayout(false);
			this->PanelDockContainerOutline->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->OutlineTreeView))->EndInit();
			this->DockSiteRightEx->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->RightDockBar))->EndInit();
			this->RightDockBar->ResumeLayout(false);
			this->PanelDockContainerFindReplace->ResumeLayout(false);
			this->FindWindowLayoutControl->ResumeLayout(false);
			this->DockSiteCenter->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->CenterDockBar))->EndInit();
			this->CenterDockBar->ResumeLayout(false);
			this->CenterDockBar->PerformLayout();
			this->PanelDocumentContent->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ContextMenuProvider))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->StatusBar))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void ScriptEditorWorkspace_Load(System::Object^ sender, System::EventArgs^ e) {
	}
private: System::Void ViewMenuDarkMode_Click(System::Object^ sender, System::EventArgs^ e) {
	if (StyleManager->ManagerStyle == DevComponents::DotNetBar::eStyle::VisualStudio2012Dark)
		StyleManager->ManagerStyle = DevComponents::DotNetBar::eStyle::VisualStudio2012Light;
	else
		StyleManager->ManagerStyle = DevComponents::DotNetBar::eStyle::VisualStudio2012Dark;
}
private: System::Void ToolbarOpenScript_Click(System::Object^ sender, System::EventArgs^ e) {
	auto dlg = gcnew ScriptEditorSelectScripts();
	dlg->ShowDialog(this);
}
private: System::Void ToolsMenuScriptSync_Click(System::Object^ sender, System::EventArgs^ e) {
	auto dlg = gcnew ScriptEditorScriptSync();
	dlg->ShowDialog(this);
}
private: System::Void ToolsMenuPreferences_Click(System::Object^ sender, System::EventArgs^ e) {
	auto dlg = gcnew ScriptEditorPreferences();
	dlg->ShowDialog(this);
}
};
}
