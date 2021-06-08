#pragma once
#include "[Common]\HandShakeStructs.h"
#include "[Common]\AuxiliaryWindowsForm.h"

namespace cse
{
	namespace scriptEditor
	{
		ref struct SelectScriptDialogData
		{
			property List<String^>^		SelectedScriptEditorIDs;
			property UInt32				SelectionCount
			{
				UInt32 get() { return SelectedScriptEditorIDs->Count; }
			}

			SelectScriptDialogData()
			{
				this->SelectedScriptEditorIDs = gcnew List<String^>;
			}
		};

		ref struct SelectScriptDialogParams
		{
			property String^	SelectedScriptEditorID;
			property String^	FilterString;
			property bool		ShowDeletedScripts;
			property bool		PreventSyncedScriptSelection;

			SelectScriptDialogParams()
			{
				this->SelectedScriptEditorID = "";
				this->FilterString = "";
				this->ShowDeletedScripts = true;
				this->PreventSyncedScriptSelection = false;
			}
		};

		ref struct NativeScriptDataWrapper
		{
			componentDLLInterface::ScriptData*	ScriptData;
			String^								EditorID;
			String^								Type;
			String^								ParentPluginName;

			NativeScriptDataWrapper(componentDLLInterface::ScriptData* Data);
		};

		ref class ScriptCollectionSorter : public System::Collections::Generic::IComparer<NativeScriptDataWrapper^>, System::Collections::IComparer
		{
		public:
			static enum class SortField
			{
				Flags,
				EditorID,
				FormID,
				Type,
				ParentPlugin
			};
		private:
			SortField	Field;
			SortOrder	Order;

			static int	CompareFieldAscending(NativeScriptDataWrapper^ X, NativeScriptDataWrapper^ Y, SortField Field);
		public:
			ScriptCollectionSorter(SortField Field, SortOrder Order)
				: Field(Field), Order(Order) {}

			virtual int Compare(NativeScriptDataWrapper^ X, NativeScriptDataWrapper^ Y);
			virtual int Compare(Object^ X, Object^ Y);
		};

		ref class FastScriptListViewDataSource : public BrightIdeasSoftware::FastObjectListDataSource
		{
		public:
			FastScriptListViewDataSource(BrightIdeasSoftware::FastObjectListView^ Parent)
				: FastObjectListDataSource(Parent) {}

			virtual void Sort(BrightIdeasSoftware::OLVColumn^ column, System::Windows::Forms::SortOrder order) override;
		};

		ref class SelectScriptDialog : public Form
		{
		public:
			// Also doubles as precedence
			static enum class FlagType
			{
				Syncing = 0,
				Deleted,
				Uncompiled,
				Active,
				Default
			};

			static FlagType GetFlagType(componentDLLInterface::ScriptData* Script);
		private:
			System::ComponentModel::Container^					components;

			System::Windows::Forms::TextBox^					PreviewBox;
			BrightIdeasSoftware::FastObjectListView^			ScriptList;
			BrightIdeasSoftware::OLVColumn^						ScriptListCFlags;
			BrightIdeasSoftware::OLVColumn^						ScriptListCScriptName;
			BrightIdeasSoftware::OLVColumn^						ScriptListCFormID;
			BrightIdeasSoftware::OLVColumn^						ScriptListCType;
			BrightIdeasSoftware::OLVColumn^						ScriptListCParentPlugin;

			System::Windows::Forms::ToolStrip^					BottomToolStrip;
			System::Windows::Forms::ToolStripLabel^				ToolStripLabelFilter;
			System::Windows::Forms::ToolStripTextBox^			ToolStripFilterTextBox;
			System::Windows::Forms::ToolStripDropDownButton^	ToolStripLabelSelect;
			System::Windows::Forms::ToolStripMenuItem^			ActiveScriptsToolStripMenuItem;
			System::Windows::Forms::ToolStripMenuItem^			UncompiledScriptsToolStripMenuItem;
			System::Windows::Forms::ToolStripLabel^				ToolStripLabelSelectionCount;

			System::Windows::Forms::Button^						ButtonCompleteSelection;
			System::Windows::Forms::Timer^						DeferredSelectionUpdateTimer;


			void						ScriptList_SelectionChanged(Object^ Sender, EventArgs^ E);
			void						ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void						ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
			void						ScriptList_ItemActivate(Object^ Sender, EventArgs^ E);

			void						ButtonCompleteSelection_Click(Object^ Sender, EventArgs^ E);
			void						ActiveScriptsToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
			void						UncompiledScriptsToolStripMenuItem_Click(Object^ Sender, EventArgs^ E);
			void						ToolStripFilterTextBox_TextChanged(Object^ Sender, EventArgs^ E);
			void						ToolStripFilterTextBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

			void						DeferredSelectionUpdateTimer_Tick(Object^ Sender, EventArgs^ E);
			void						Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);

			void						InitializeComponent();
			void						ShowUseReport();
			void						SaveBoundsToINI();
			void						LoadBoundsFromINI();
			void						CompleteSelection();
			void						PopulateLoadedScripts(String^ FilterString, bool DefaultSelection, bool SortByFlags);

			static Object^				ScriptListAspectScriptNameGetter(Object^ RowObject);
			static Object^				ScriptListAspectFormIDGetter(Object^ RowObject);
			static String^				ScriptListAspectToStringConverterFormID(Object^ RowObject);
			static Object^				ScriptListAspectTypeGetter(Object^ RowObject);
			static Object^				ScriptListAspectParentPluginGetter(Object^ RowObject);
			static Object^				ScriptListAspectFlagsGetter(Object^ RowObject);
			static String^				ScriptListAspectToStringConverterFlags(Object^ RowObject);
			static Object^				ScriptListImageFlagsGetter(Object^ RowObject);


			SelectScriptDialogParams^				Parameters;
			componentDLLInterface::ScriptListData*	LoadedScripts;
			List<NativeScriptDataWrapper^>^			ListDataSource;
			List<NativeScriptDataWrapper^>^			DisabledScripts;
		public:
			property SelectScriptDialogData^		ResultData;
			property bool							HasResult;

			SelectScriptDialog(SelectScriptDialogParams^ Params);
			~SelectScriptDialog();
		};
	}
}