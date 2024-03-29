#pragma once

#include "IScriptEditorView.h"


namespace cse
{


namespace scriptEditor
{


namespace selectScript
{


ref struct NativeScriptDataWrapper
{
	componentDLLInterface::ScriptData* ScriptData;
	String^ EditorID;
	String^ Type;
	String^ ParentPluginName;

	NativeScriptDataWrapper(componentDLLInterface::ScriptData* Data);
};

ref class ScriptCollectionSorter : public System::Collections::Generic::IComparer<NativeScriptDataWrapper^>, System::Collections::IComparer
{
public:
	static enum class eSortField
	{
		Flags,
		EditorID,
		FormID,
		Type,
		ParentPlugin
	};
private:
	eSortField Field;
	SortOrder Order;

	static int CompareFieldAscending(NativeScriptDataWrapper^ X, NativeScriptDataWrapper^ Y, eSortField Field);
public:
	ScriptCollectionSorter(eSortField Field, SortOrder Order);

	virtual int Compare(NativeScriptDataWrapper^ X, NativeScriptDataWrapper^ Y);
	virtual int Compare(Object^ X, Object^ Y);
};

ref class FastScriptListViewDataSource : public BrightIdeasSoftware::FastObjectListDataSource
{
public:
	FastScriptListViewDataSource(BrightIdeasSoftware::FastObjectListView^ Parent);

	virtual void Sort(BrightIdeasSoftware::OLVColumn^ column, System::Windows::Forms::SortOrder order) override;
};

ref class ScriptSelectionDialog : public DevComponents::DotNetBar::Metro::MetroForm
{
public:
	ref struct Params
	{
		property String^ SelectedScriptEditorID;
		property String^ FilterString;
		property bool ShowDeletedScripts;
		property bool PreventSyncedScriptSelection;
		property IntPtr ParentWindowHandle;

		Params();
	};

	ref struct SelectionResult
	{
		property List<String^>^ SelectedScriptEditorIDs;
		property UInt32 SelectionCount
		{
			UInt32 get() { return SelectedScriptEditorIDs->Count; }
		}

		SelectionResult();
	};

	// Also doubles as precedence
	static enum class eFlagType
	{
		Syncing = 0,
		Deleted,
		Uncompiled,
		Active,
		Default
	};

	static eFlagType GetFlagType(componentDLLInterface::ScriptData* Script);
private:
	static Dictionary<eFlagType, Image^>^ FlagIcons = gcnew Dictionary<eFlagType, Image^>;

	static Image^ GetFlagIcon(eFlagType FlagType);

	BrightIdeasSoftware::FastObjectListView^ ScriptList;
	BrightIdeasSoftware::OLVColumn^ ScriptListCFlags;
	BrightIdeasSoftware::OLVColumn^ ScriptListCScriptName;
	BrightIdeasSoftware::OLVColumn^ ScriptListCFormID;
	BrightIdeasSoftware::OLVColumn^ ScriptListCType;
	BrightIdeasSoftware::OLVColumn^ ScriptListCParentPlugin;
	DevComponents::DotNetBar::Controls::TextBoxX^ ScriptTextPreview;
	DevComponents::DotNetBar::PanelEx^ BottomToolbarContainer;
	DevComponents::DotNetBar::Bar^ BottomToolbar;
	DevComponents::DotNetBar::TextBoxItem^ BottomToolbarTextboxFilter;
	DevComponents::DotNetBar::ButtonItem^ BottomToolbarSelectAll;
	DevComponents::DotNetBar::ButtonItem^ MenuSelectAllActive;
	DevComponents::DotNetBar::ButtonItem^ MenuSelectAllDeleted;
	DevComponents::DotNetBar::ButtonItem^ MenuSelectAllUncompiled;
	DevComponents::DotNetBar::LabelItem^ BottomToolbarSelectionCount;
	DevComponents::DotNetBar::ButtonX^ BottomToolbarCompleteSelection;
	System::Windows::Forms::Timer^ DeferredSelectionUpdateTimer;
	DevComponents::DotNetBar::StyleManagerAmbient^ ColorManager;
	System::ComponentModel::IContainer^ components;
	view::IObjectListView^ ListViewThemeWrapper;

	void ScriptList_SelectionChanged(Object^ Sender, EventArgs^ E);
	void ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
	void ScriptList_ItemActivate(Object^ Sender, EventArgs^ E);

	void BottomToolbarCompleteSelection_Click(Object^ Sender, EventArgs^ E);
	void MenuSelectAllActive_Click(Object^ Sender, EventArgs^ E);
	void MenuSelectAllUncompiled_Click(Object^ Sender, EventArgs^ E);
	void MenuSelectAllDeleted_Click(Object^ Sender, EventArgs^ E);
	void BottomToolbarTextboxFilter_TextChanged(Object^ Sender, EventArgs^ E);
	void BottomToolbarTextboxFilter_KeyDown(Object^ Sender, KeyEventArgs^ E);

	void DeferredSelectionUpdateTimer_Tick(Object^ Sender, EventArgs^ E);
	void Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);
	void Dialog_Load(Object^ Sender, EventArgs^ E);

	void InitializeComponent();
	void FinalizeComponents();
	void ShowUseReport();
	void CompleteSelection();
	void PopulateLoadedScripts(String^ FilterString, bool DefaultSelection, bool SortByFlags);
	void SetDefaultFont(System::Drawing::Font^ DefaultFont);

	static Object^ ScriptListAspectScriptNameGetter(Object^ RowObject);
	static Object^ ScriptListAspectFormIDGetter(Object^ RowObject);
	static String^ ScriptListAspectToStringConverterFormID(Object^ RowObject);
	static Object^ ScriptListAspectTypeGetter(Object^ RowObject);
	static Object^ ScriptListAspectParentPluginGetter(Object^ RowObject);
	static Object^ ScriptListAspectFlagsGetter(Object^ RowObject);
	static String^ ScriptListAspectToStringConverterFlags(Object^ RowObject);
	static Object^ ScriptListImageFlagsGetter(Object^ RowObject);

	Params^ Parameters;
	componentDLLInterface::ScriptListData* LoadedScripts;
	List<NativeScriptDataWrapper^>^ ListDataSource;
	List<NativeScriptDataWrapper^>^ DisabledScripts;
public:
	property SelectionResult^ ResultData;
	property bool HasResult;

	ScriptSelectionDialog(Params^ Parameters);
	~ScriptSelectionDialog();
};


} // namespace selectScript


} // namespace scriptEditor


} // namespace cse