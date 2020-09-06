#pragma once

#include "[Common]\HandshakeStructs.h"

namespace cse
{
	ref struct NativeFormWrapper
	{
		property String^	EditorID;
		property UInt32		FormID;
		property String^	Type;
		property String^	ParentPlugin;
		property bool		Active;

		NativeFormWrapper(componentDLLInterface::FormData* FormData)
		{
			this->EditorID = gcnew String(FormData->EditorID);
			this->FormID = FormData->FormID;
			this->Type = gcnew String(nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormTypeIDLongName(FormData->TypeID));
			this->ParentPlugin = gcnew String(FormData->ParentPluginName);
			this->Active = FormData->IsActive();
		}
	};

	ref struct NativeCellUsageWrapper
	{
		property String^	CellEditorID;
		property bool		Active;
		property bool		Interior;
		property String^	WorldEditorID;
		property Point		ExteriorGrid;
		property UInt32		UseCount;
		property UInt32		FirstRefFormID;
		property String^	FirstRefEditorID;

		NativeCellUsageWrapper(componentDLLInterface::UseInfoListCellItemData* UsageData)
		{
			this->CellEditorID = gcnew String(UsageData->EditorID);
			this->Active = UsageData->IsActive();
			this->Interior = UsageData->ParentCellInterior;
			this->WorldEditorID = gcnew String(UsageData->WorldEditorID);
			this->ExteriorGrid = Point(UsageData->XCoord, UsageData->YCoord);
			this->UseCount = UsageData->UseCount;
			this->FirstRefFormID = UsageData->RefFormID;
			this->FirstRefEditorID = gcnew String(UsageData->RefEditorID);
		}
	};

	ref class CentralizedUseInfoList : public Form
	{
		System::ComponentModel::Container^			components;

		System::Windows::Forms::TextBox^			TextBoxFilter;
		BrightIdeasSoftware::FastObjectListView^	ListViewForms;
		System::Windows::Forms::Label^				LabelFilter;
		System::Windows::Forms::Label^				LabelObjects;
		System::Windows::Forms::Label^				LabelCells;
		BrightIdeasSoftware::FastObjectListView^	ListViewObjectUsage;
		BrightIdeasSoftware::FastObjectListView^	ListViewCellUsage;
		BrightIdeasSoftware::OLVColumn^				LVFormCType;
		BrightIdeasSoftware::OLVColumn^				LVFormCEditorID;
		BrightIdeasSoftware::OLVColumn^				LVFormCFormID;
		BrightIdeasSoftware::OLVColumn^				LVFormCPlugin;
		BrightIdeasSoftware::OLVColumn^				LVObjectsCType;
		BrightIdeasSoftware::OLVColumn^				LVObjectsCEditorID;
		BrightIdeasSoftware::OLVColumn^				LVObjectsCFormID;
		BrightIdeasSoftware::OLVColumn^				LVObjectsCPlugin;
		BrightIdeasSoftware::OLVColumn^				LVCellsCWorldEditorID;
		BrightIdeasSoftware::OLVColumn^				LVCellsCCellEditorID;
		BrightIdeasSoftware::OLVColumn^				LVCellsCGrid;
		BrightIdeasSoftware::OLVColumn^				LVCellsCUseCount;
		BrightIdeasSoftware::OLVColumn^				LVCellsCRef;


		void			ListViewForms_SelectionChanged(Object^ Sender, EventArgs^ E);
		void			ListViewForms_ItemActivate(Object^ Sender, EventArgs^ E);
		void			ListViewForms_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E);
		void			ListViewObjectUsage_ItemActivate(Object^ Sender, EventArgs^ E);
		void			ListViewObjectUsage_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E);
		void			ListViewCellUsage_ItemActivate(Object^ Sender, EventArgs^ E);
		void			ListViewCellUsage_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E);

		void			TextBoxFilter_TextChanged(Object^ Sender, EventArgs^ E);
		void			Dialog_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
		void			Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);

		void			InitializeComponent();
		void			PopulateLoadedForms(String^ FilterString);
		void			PopulateUsageInfo(NativeFormWrapper^ BaseForm);
		void			HandleActiveFormHighlighting(BrightIdeasSoftware::FormatRowEventArgs^ E);

		static Object^	LVFormAspectGetterEditorID(Object^ Model);
		static Object^	LVFormAspectGetterFormID(Object^ Model);
		static String^	LVFormAspectToStringFormID(Object^ Model);
		static Object^	LVFormAspectGetterType(Object^ Model);
		static Object^	LVFormAspectGetterParentPlugin(Object^ Model);

		static Object^	LVCellAspectGetterWorldEditorID(Object^ Model);
		static Object^	LVCellAspectGetterCellEditorID(Object^ Model);
		static Object^	LVCellAspectGetterGrid(Object^ Model);
		static Object^	LVCellAspectGetterUseCount(Object^ Model);
		static Object^	LVCellAspectGetterRef(Object^ Model);

		componentDLLInterface::UseInfoListFormData*		LoadedForms;

		static CentralizedUseInfoList^					Singleton = nullptr;

		CentralizedUseInfoList(String^ FilterString);
	public:
		~CentralizedUseInfoList();

		static void Open(String^ FilterString);
		static void Close();
	};
}