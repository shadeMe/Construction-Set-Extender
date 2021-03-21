#include "UseInfoList.h"
#include "[Common]\ListViewUtilities.h"
#include "[Common]\HandshakeStructs.h"
#include "[Common]\NativeWrapper.h"

namespace cse
{
	void CentralizedUseInfoList::ListViewForms_SelectionChanged(Object^ Sender, EventArgs^ E)
	{
		if (ListViewForms->SelectedObject == nullptr)
		{
			ListViewObjectUsage->ClearObjects();
			ListViewCellUsage->ClearObjects();
			return;
		}

		NativeFormWrapper^ Data = (NativeFormWrapper^)ListViewForms->SelectedObject;
		PopulateUsageInfo(Data);
	}

	void CentralizedUseInfoList::ListViewForms_ItemActivate(Object^ Sender, EventArgs^ E)
	{
		if (ListViewForms->SelectedObject == nullptr)
			return;

		NativeFormWrapper^ Data = (NativeFormWrapper^)ListViewForms->SelectedObject;
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.LoadFormForEditByEditorID(CString(Data->EditorID).c_str());

	}

	void CentralizedUseInfoList::ListViewForms_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E)
	{
		if (E->Model == nullptr)
			return;

		NativeFormWrapper^ Data = (NativeFormWrapper^)E->Model;
		if (Data->Active)
			HandleActiveFormHighlighting(E);
	}

	void CentralizedUseInfoList::ListViewObjectUsage_ItemActivate(Object^ Sender, EventArgs^ E)
	{
		if (ListViewForms->SelectedObject == nullptr)
			return;

		NativeFormWrapper^ Data = (NativeFormWrapper^)ListViewObjectUsage->SelectedObject;
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.LoadFormForEditByEditorID(CString(Data->EditorID).c_str());
	}

	void CentralizedUseInfoList::ListViewObjectUsage_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E)
	{
		if (E->Model == nullptr)
			return;

		NativeFormWrapper^ Data = (NativeFormWrapper^)E->Model;
		if (Data->Active)
			HandleActiveFormHighlighting(E);
	}

	void CentralizedUseInfoList::ListViewCellUsage_ItemActivate(Object^ Sender, EventArgs^ E)
	{
		if (ListViewForms->SelectedObject == nullptr)
			return;

		NativeCellUsageWrapper^ Data = (NativeCellUsageWrapper^)ListViewCellUsage->SelectedObject;
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.LoadFormForEditByFormID(Data->FirstRefFormID);
	}

	void CentralizedUseInfoList::ListViewCellUsage_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E)
	{
		if (E->Model == nullptr)
			return;

		NativeCellUsageWrapper^ Data = (NativeCellUsageWrapper^)E->Model;
		if (Data->Active)
			HandleActiveFormHighlighting(E);
	}

	void CentralizedUseInfoList::TextBoxFilter_TextChanged(Object^ Sender, EventArgs^ E)
	{
		PopulateLoadedForms(TextBoxFilter->Text);
	}

	void CentralizedUseInfoList::Control_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
	{
		switch (E->KeyChar)
		{
		case 'F':
			if (Sender != TextBoxFilter)
			{
				E->Handled = true;
				TextBoxFilter->Select();
			}
			break;
		case (char)27:		// Escape key
			E->Handled = true;
			this->Close();
			break;
		}
	}

	void CentralizedUseInfoList::Dialog_Cancel(Object^ Sender, CancelEventArgs^ E)
	{
		if (LoadedForms)
		{
			nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(LoadedForms, false);
			LoadedForms = nullptr;
		}
	}

	void CentralizedUseInfoList::InitializeComponent(void)
	{
		this->TextBoxFilter = (gcnew System::Windows::Forms::TextBox());
		this->ListViewForms = (gcnew BrightIdeasSoftware::FastObjectListView());
		this->LVFormCType = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVFormCEditorID = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVFormCFormID = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVFormCPlugin = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LabelFilter = (gcnew System::Windows::Forms::Label());
		this->LabelObjects = (gcnew System::Windows::Forms::Label());
		this->LabelCells = (gcnew System::Windows::Forms::Label());
		this->ListViewObjectUsage = (gcnew BrightIdeasSoftware::FastObjectListView());
		this->LVObjectsCType = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVObjectsCEditorID = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVObjectsCFormID = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVObjectsCPlugin = (gcnew BrightIdeasSoftware::OLVColumn());
		this->ListViewCellUsage = (gcnew BrightIdeasSoftware::FastObjectListView());
		this->LVCellsCWorldEditorID = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVCellsCCellEditorID = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVCellsCGrid = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVCellsCUseCount = (gcnew BrightIdeasSoftware::OLVColumn());
		this->LVCellsCRef = (gcnew BrightIdeasSoftware::OLVColumn());
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewForms))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewObjectUsage))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewCellUsage))->BeginInit();
		this->SuspendLayout();
		//
		// TextBoxFilter
		//
		this->TextBoxFilter->Location = System::Drawing::Point(44, 665);
		this->TextBoxFilter->MaxLength = 100;
		this->TextBoxFilter->Multiline = false;
		this->TextBoxFilter->Name = L"TextBoxFilter";
		this->TextBoxFilter->Size = System::Drawing::Size(396, 23);
		this->TextBoxFilter->TabIndex = 0;
		//
		// ListViewForms
		//
		this->ListViewForms->AllColumns->Add(this->LVFormCType);
		this->ListViewForms->AllColumns->Add(this->LVFormCEditorID);
		this->ListViewForms->AllColumns->Add(this->LVFormCFormID);
		this->ListViewForms->AllColumns->Add(this->LVFormCPlugin);
		this->ListViewForms->CellEditUseWholeCell = false;
		this->ListViewForms->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(4) {
			this->LVFormCType,
				this->LVFormCEditorID, this->LVFormCFormID, this->LVFormCPlugin
		});
		this->ListViewForms->Cursor = System::Windows::Forms::Cursors::Default;
		this->ListViewForms->HideSelection = false;
		this->ListViewForms->Location = System::Drawing::Point(12, 12);
		this->ListViewForms->MultiSelect = false;
		this->ListViewForms->Name = L"ListViewForms";
		this->ListViewForms->ShowGroups = false;
		this->ListViewForms->Size = System::Drawing::Size(428, 647);
		this->ListViewForms->TabIndex = 1;
		this->ListViewForms->UseCompatibleStateImageBehavior = false;
		this->ListViewForms->View = System::Windows::Forms::View::Details;
		this->ListViewForms->VirtualMode = true;
		this->ListViewForms->FullRowSelect = true;
		//
		// LVFormCType
		//
		this->LVFormCType->Text = L"Type";
		this->LVFormCType->Width = 65;
		this->LVFormCType->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterType);
		//
		// LVFormCEditorID
		//
		this->LVFormCEditorID->Text = L"Editor ID";
		this->LVFormCEditorID->Width = 165;
		this->LVFormCEditorID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterEditorID);
		//
		// LVFormCFormID
		//
		this->LVFormCFormID->Text = L"Form ID";
		this->LVFormCFormID->Width = 70;
		this->LVFormCFormID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterFormID);
		this->LVFormCFormID->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&CentralizedUseInfoList::LVFormAspectToStringFormID);
		//
		// LVFormCPlugin
		//
		this->LVFormCPlugin->Text = L"Parent Plugin";
		this->LVFormCPlugin->Width = 140;
		this->LVFormCPlugin->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterParentPlugin);
		//
		// LabelFilter
		//
		this->LabelFilter->AutoSize = true;
		this->LabelFilter->Location = System::Drawing::Point(12, 668);
		this->LabelFilter->Name = L"LabelFilter";
		this->LabelFilter->Size = System::Drawing::Size(29, 13);
		this->LabelFilter->TabIndex = 6;
		this->LabelFilter->Text = L"Filter";
		//
		// LabelObjects
		//
		this->LabelObjects->AutoSize = true;
		this->LabelObjects->Location = System::Drawing::Point(443, 14);
		this->LabelObjects->Name = L"LabelObjects";
		this->LabelObjects->Size = System::Drawing::Size(119, 13);
		this->LabelObjects->TabIndex = 7;
		this->LabelObjects->Text = L"Used By These Objects";
		//
		// LabelCells
		//
		this->LabelCells->AutoSize = true;
		this->LabelCells->Location = System::Drawing::Point(443, 354);
		this->LabelCells->Name = L"LabelCells";
		this->LabelCells->Size = System::Drawing::Size(102, 13);
		this->LabelCells->TabIndex = 8;
		this->LabelCells->Text = L"Used In These Cells";
		//
		// ListViewObjectUsage
		//
		this->ListViewObjectUsage->AllColumns->Add(this->LVObjectsCType);
		this->ListViewObjectUsage->AllColumns->Add(this->LVObjectsCEditorID);
		this->ListViewObjectUsage->AllColumns->Add(this->LVObjectsCFormID);
		this->ListViewObjectUsage->AllColumns->Add(this->LVObjectsCPlugin);
		this->ListViewObjectUsage->CellEditUseWholeCell = false;
		this->ListViewObjectUsage->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(4) {
			this->LVObjectsCType,
				this->LVObjectsCEditorID, this->LVObjectsCFormID, this->LVObjectsCPlugin
		});
		this->ListViewObjectUsage->Cursor = System::Windows::Forms::Cursors::Default;
		this->ListViewObjectUsage->HideSelection = false;
		this->ListViewObjectUsage->Location = System::Drawing::Point(446, 30);
		this->ListViewObjectUsage->MultiSelect = false;
		this->ListViewObjectUsage->Name = L"ListViewObjectUsage";
		this->ListViewObjectUsage->ShowGroups = false;
		this->ListViewObjectUsage->Size = System::Drawing::Size(549, 318);
		this->ListViewObjectUsage->TabIndex = 2;
		this->ListViewObjectUsage->UseCompatibleStateImageBehavior = false;
		this->ListViewObjectUsage->View = System::Windows::Forms::View::Details;
		this->ListViewObjectUsage->VirtualMode = true;
		this->ListViewObjectUsage->FullRowSelect = true;
		//
		// LVObjectsCType
		//
		this->LVObjectsCType->Text = L"Type";
		this->LVObjectsCType->Width = 65;
		this->LVObjectsCType->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterType);
		//
		// LVObjectsCEditorID
		//
		this->LVObjectsCEditorID->Text = L"Editor ID";
		this->LVObjectsCEditorID->Width = 165;
		this->LVObjectsCEditorID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterEditorID);
		//
		// LVObjectsCFormID
		//
		this->LVObjectsCFormID->Text = L"Form ID";
		this->LVObjectsCFormID->Width = 65;
		this->LVObjectsCFormID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterFormID);
		this->LVObjectsCFormID->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&CentralizedUseInfoList::LVFormAspectToStringFormID);
		//
		// LVObjectsCPlugin
		//
		this->LVObjectsCPlugin->Text = L"Parent Plugin";
		this->LVObjectsCPlugin->Width = 165;
		this->LVObjectsCPlugin->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVFormAspectGetterParentPlugin);
		//
		// ListViewCellUsage
		//
		this->ListViewCellUsage->AllColumns->Add(this->LVCellsCWorldEditorID);
		this->ListViewCellUsage->AllColumns->Add(this->LVCellsCCellEditorID);
		this->ListViewCellUsage->AllColumns->Add(this->LVCellsCGrid);
		this->ListViewCellUsage->AllColumns->Add(this->LVCellsCUseCount);
		this->ListViewCellUsage->AllColumns->Add(this->LVCellsCRef);
		this->ListViewCellUsage->CellEditUseWholeCell = false;
		this->ListViewCellUsage->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(5) {
			this->LVCellsCWorldEditorID,
				this->LVCellsCCellEditorID, this->LVCellsCGrid, this->LVCellsCUseCount, this->LVCellsCRef
		});
		this->ListViewCellUsage->Cursor = System::Windows::Forms::Cursors::Default;
		this->ListViewCellUsage->HideSelection = false;
		this->ListViewCellUsage->Location = System::Drawing::Point(446, 370);
		this->ListViewCellUsage->MultiSelect = false;
		this->ListViewCellUsage->Name = L"ListViewCellUsage";
		this->ListViewCellUsage->ShowGroups = false;
		this->ListViewCellUsage->Size = System::Drawing::Size(549, 318);
		this->ListViewCellUsage->TabIndex = 3;
		this->ListViewCellUsage->UseCompatibleStateImageBehavior = false;
		this->ListViewCellUsage->View = System::Windows::Forms::View::Details;
		this->ListViewCellUsage->VirtualMode = true;
		this->ListViewCellUsage->FullRowSelect = true;
		//
		// LVCellsCWorldEditorID
		//
		this->LVCellsCWorldEditorID->Text = L"World";
		this->LVCellsCWorldEditorID->Width = 100;
		this->LVCellsCWorldEditorID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVCellAspectGetterWorldEditorID);
		//
		// LVCellsCCellEditorID
		//
		this->LVCellsCCellEditorID->Text = L"Cell";
		this->LVCellsCCellEditorID->Width = 150;
		this->LVCellsCCellEditorID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVCellAspectGetterCellEditorID);
		//
		// LVCellsCGrid
		//
		this->LVCellsCGrid->Text = L"Grid";
		this->LVCellsCGrid->Width = 50;
		this->LVCellsCGrid->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVCellAspectGetterGrid);
		//
		// LVCellsCUseCount
		//
		this->LVCellsCUseCount->Text = L"Use Count";
		this->LVCellsCUseCount->Width = 70;
		this->LVCellsCUseCount->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVCellAspectGetterUseCount);
		//
		// LVCellsCRef
		//
		this->LVCellsCRef->Text = L"First Reference";
		this->LVCellsCRef->Width = 165;
		this->LVCellsCRef->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&CentralizedUseInfoList::LVCellAspectGetterRef);
		this->LVCellsCRef->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&CentralizedUseInfoList::LVFormAspectToStringFormID);
		//
		// UseInfoList
		//
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->ClientSize = System::Drawing::Size(1007, 700);
		this->Controls->Add(this->ListViewCellUsage);
		this->Controls->Add(this->ListViewObjectUsage);
		this->Controls->Add(this->LabelCells);
		this->Controls->Add(this->LabelObjects);
		this->Controls->Add(this->LabelFilter);
		this->Controls->Add(this->ListViewForms);
		this->Controls->Add(this->TextBoxFilter);
		this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
		this->KeyPreview = false;
		this->MaximizeBox = false;
		this->Name = L"UseInfoList";
		this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
		this->Text = L"Centralised Use Info Listing";
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewForms))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewObjectUsage))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewCellUsage))->EndInit();
		this->ResumeLayout(false);
		this->PerformLayout();

		// ### The invocation of the SelectionChanged event apparently is tied to the parent thread's idle message loop
		// ### This breaks event handling for this event when the idle loop isn't active (which are usually active when thread-modal dialog is being shown)
		// ### We need to use SelectedIndexChanged instead.
		ListViewForms->SelectedIndexChanged += gcnew EventHandler(this, &CentralizedUseInfoList::ListViewForms_SelectionChanged);
		ListViewForms->ItemActivate += gcnew EventHandler(this, &CentralizedUseInfoList::ListViewForms_ItemActivate);
		ListViewForms->FormatRow += gcnew EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>(this, &CentralizedUseInfoList::ListViewForms_FormatRow);
		ListViewForms->KeyPress += gcnew KeyPressEventHandler(this, &CentralizedUseInfoList::Control_KeyPress);
		ListViewObjectUsage->ItemActivate += gcnew EventHandler(this, &CentralizedUseInfoList::ListViewObjectUsage_ItemActivate);
		ListViewObjectUsage->FormatRow += gcnew EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>(this, &CentralizedUseInfoList::ListViewObjectUsage_FormatRow);
		ListViewObjectUsage->KeyPress += gcnew KeyPressEventHandler(this, &CentralizedUseInfoList::Control_KeyPress);
		ListViewCellUsage->ItemActivate += gcnew EventHandler(this, &CentralizedUseInfoList::ListViewCellUsage_ItemActivate);
		ListViewCellUsage->FormatRow += gcnew EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>(this, &CentralizedUseInfoList::ListViewCellUsage_FormatRow);
		ListViewCellUsage->KeyPress += gcnew KeyPressEventHandler(this, &CentralizedUseInfoList::Control_KeyPress);

		TextBoxFilter->TextChanged += gcnew EventHandler(this, &CentralizedUseInfoList::TextBoxFilter_TextChanged);
		TextBoxFilter->KeyPress += gcnew KeyPressEventHandler(this, &CentralizedUseInfoList::Control_KeyPress);
		this->Closing += gcnew CancelEventHandler(this, &CentralizedUseInfoList::Dialog_Cancel);
	}

	void CentralizedUseInfoList::PopulateLoadedForms(String^ FilterString)
	{
		ListViewForms->DeselectAll();
		ListViewForms->ClearObjects();

		ListViewObjectUsage->DeselectAll();
		ListViewCellUsage->DeselectAll();

		ListViewObjectUsage->ClearObjects();
		ListViewCellUsage->ClearObjects();

		if (LoadedForms == nullptr)
			return;

		List<NativeFormWrapper^>^ Model = gcnew List<NativeFormWrapper ^>;
		for (int i = 0; i < LoadedForms->FormCount; i++)
		{
			componentDLLInterface::FormData* ThisForm = &LoadedForms->FormListHead[i];
			NativeFormWrapper^ NewItem = gcnew NativeFormWrapper(ThisForm);

			if (FilterString->Length > 0)
			{
				if (NewItem->EditorID->IndexOf(FilterString, StringComparison::CurrentCultureIgnoreCase) == -1)
					continue;
			}

			Model->Add(NewItem);
		}

		ListViewForms->SetObjects(Model);
		if (ListViewForms->GetItemCount())
			ListViewForms->SelectedIndex = 0;
	}

	void CentralizedUseInfoList::PopulateUsageInfo(NativeFormWrapper^ BaseForm)
	{
		ListViewObjectUsage->DeselectAll();
		ListViewCellUsage->DeselectAll();

		ListViewObjectUsage->ClearObjects();
		ListViewCellUsage->ClearObjects();

		CString BaseFormEID(BaseForm->EditorID);
		List<NativeFormWrapper^>^ CrossRefModel = gcnew List<NativeFormWrapper^>;
		DisposibleDataAutoPtr<componentDLLInterface::UseInfoListCrossRefData> CrossRefs
			(nativeWrapper::g_CSEInterfaceTable->UseInfoList.GetCrossRefDataForForm(BaseFormEID.c_str()));

		if (CrossRefs)
		{
			for (int i = 0; i < CrossRefs->FormCount; i++)
			{
				componentDLLInterface::FormData* ThisForm = &CrossRefs->FormListHead[i];
				CrossRefModel->Add(gcnew NativeFormWrapper(ThisForm));
			}
		}

		List<NativeCellUsageWrapper^>^ CellUsageModel = gcnew List<NativeCellUsageWrapper^>;
		DisposibleDataAutoPtr<componentDLLInterface::UseInfoListCellItemListData> CellUsage
			(nativeWrapper::g_CSEInterfaceTable->UseInfoList.GetCellRefDataForForm(BaseFormEID.c_str()));

		if (CellUsage)
		{
			for (int i = 0; i < CellUsage->UseInfoListCellItemListCount; i++)
			{
				componentDLLInterface::UseInfoListCellItemData* ThisForm = &CellUsage->UseInfoListCellItemListHead[i];
				CellUsageModel->Add(gcnew NativeCellUsageWrapper(ThisForm));
			}
		}

		ListViewObjectUsage->SetObjects(CrossRefModel);
		if (ListViewObjectUsage->GetItemCount())
			ListViewObjectUsage->SelectedIndex = 0;

		ListViewCellUsage->SetObjects(CellUsageModel);
		if (ListViewCellUsage->GetItemCount())
			ListViewCellUsage->SelectedIndex = 0;
	}

	void CentralizedUseInfoList::HandleActiveFormHighlighting(BrightIdeasSoftware::FormatRowEventArgs^ E)
	{
		bool ColorizeActiveForms = nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetShouldColorizeActiveForms();
		if (ColorizeActiveForms == false)
			return;

		UInt32 ActiveForeColor = nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormListActiveItemForegroundColor();
		UInt32 ActiveBackColor = nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormListActiveItemBackgroundColor();

		E->Item->ForeColor = System::Drawing::Color::FromArgb(ActiveForeColor & 0xFF,
			(ActiveForeColor >> 8) & 0xFF,
			(ActiveForeColor >> 16) & 0xFF);
		E->Item->BackColor = System::Drawing::Color::FromArgb(ActiveBackColor & 0xFF,
			(ActiveBackColor >> 8) & 0xFF,
			(ActiveBackColor >> 16) & 0xFF);
	}

	System::Object^ CentralizedUseInfoList::LVFormAspectGetterEditorID(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeFormWrapper^ Data = (NativeFormWrapper^)Model;
		return Data->EditorID;
	}

	System::Object^ CentralizedUseInfoList::LVFormAspectGetterFormID(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeFormWrapper^ Data = (NativeFormWrapper^)Model;
		return Data->FormID;
	}

	System::String^ CentralizedUseInfoList::LVFormAspectToStringFormID(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		return ((UInt32)Model).ToString("X8");
	}

	System::Object^ CentralizedUseInfoList::LVFormAspectGetterType(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeFormWrapper^ Data = (NativeFormWrapper^)Model;
		return Data->Type;
	}

	System::Object^ CentralizedUseInfoList::LVFormAspectGetterParentPlugin(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeFormWrapper^ Data = (NativeFormWrapper^)Model;
		return Data->ParentPlugin;
	}

	System::Object^ CentralizedUseInfoList::LVCellAspectGetterWorldEditorID(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeCellUsageWrapper^ Data = (NativeCellUsageWrapper^)Model;
		return Data->WorldEditorID;
	}

	System::Object^ CentralizedUseInfoList::LVCellAspectGetterCellEditorID(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeCellUsageWrapper^ Data = (NativeCellUsageWrapper^)Model;
		return Data->CellEditorID;
	}

	System::Object^ CentralizedUseInfoList::LVCellAspectGetterGrid(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeCellUsageWrapper^ Data = (NativeCellUsageWrapper^)Model;
		if (Data->Interior)
			return String::Empty;

		return String::Format("{0},{1}", Data->ExteriorGrid.X, Data->ExteriorGrid.Y);
	}

	System::Object^ CentralizedUseInfoList::LVCellAspectGetterUseCount(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeCellUsageWrapper^ Data = (NativeCellUsageWrapper^)Model;
		return Data->UseCount;
	}

	System::Object^ CentralizedUseInfoList::LVCellAspectGetterRef(Object^ Model)
	{
		if (Model == nullptr)
			return nullptr;

		NativeCellUsageWrapper^ Data = (NativeCellUsageWrapper^)Model;
		return Data->FirstRefFormID;
	}

	CentralizedUseInfoList::CentralizedUseInfoList(String^ FilterString)
	{
		InitializeComponent();

		TextBoxFilter->Text = FilterString;
		LoadedForms = nativeWrapper::g_CSEInterfaceTable->UseInfoList.GetLoadedForms();

		PopulateLoadedForms(FilterString);
		this->Show();
	}

	CentralizedUseInfoList::~CentralizedUseInfoList()
	{
		if (LoadedForms)
		{
			nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(LoadedForms, false);
			LoadedForms = nullptr;
		}

		if (components)
		{
			delete components;
		}

		Singleton = nullptr;
	}

	void CentralizedUseInfoList::Open(String^ FilterString)
	{
		if (Singleton)
		{
			Singleton->PopulateLoadedForms(FilterString);
			Singleton->BringToFront();
			return;
		}

		Singleton = gcnew CentralizedUseInfoList(FilterString);
	}

	void CentralizedUseInfoList::Close()
	{
		if (Singleton == nullptr)
			return;

		Singleton->Form::Close();
	}
}