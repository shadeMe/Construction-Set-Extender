#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace UIComponents {

	/// <summary>
	/// Summary for UseInfoList
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class UseInfoList : public System::Windows::Forms::Form
	{
	public:
		UseInfoList(void)
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
		~UseInfoList()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::TextBox^ TextBoxFilter;
	protected:

	protected: 
















	private: BrightIdeasSoftware::FastObjectListView^ ListViewForms;

	private: System::Windows::Forms::Label^ LabelFilter;
	private: System::Windows::Forms::Label^ LabelObjects;
	private: System::Windows::Forms::Label^ LabelCells;
	private: BrightIdeasSoftware::FastObjectListView^ ListViewObjectUsage;
	private: BrightIdeasSoftware::FastObjectListView^ ListViewCellUsage;
	private: BrightIdeasSoftware::OLVColumn^ LVFormCType;
	private: BrightIdeasSoftware::OLVColumn^ LVFormCEditorID;
	private: BrightIdeasSoftware::OLVColumn^ LVFormCFormID;
	private: BrightIdeasSoftware::OLVColumn^ LVFormCPlugin;
	private: BrightIdeasSoftware::OLVColumn^ LVObjectsCType;
	private: BrightIdeasSoftware::OLVColumn^ LVObjectsCEditorID;
	private: BrightIdeasSoftware::OLVColumn^ LVObjectsCFormID;
	private: BrightIdeasSoftware::OLVColumn^ LVObjectsCPlugin;
	private: BrightIdeasSoftware::OLVColumn^ LVCellsCWorldEditorID;
	private: BrightIdeasSoftware::OLVColumn^ LVCellsCCellEditorID;
	private: BrightIdeasSoftware::OLVColumn^ LVCellsCGrid;
	private: BrightIdeasSoftware::OLVColumn^ LVCellsCUseCount;
	private: BrightIdeasSoftware::OLVColumn^ LVCellsCRef;




















	protected: 











	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
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
			this->TextBoxFilter->Name = L"TextBoxFilter";
			this->TextBoxFilter->Size = System::Drawing::Size(396, 20);
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
			this->ListViewForms->FullRowSelect = true;
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
			// 
			// LVFormCType
			// 
			this->LVFormCType->Text = L"Type";
			this->LVFormCType->Width = 65;
			// 
			// LVFormCEditorID
			// 
			this->LVFormCEditorID->Text = L"Editor ID";
			this->LVFormCEditorID->Width = 165;
			// 
			// LVFormCFormID
			// 
			this->LVFormCFormID->Text = L"Form ID";
			this->LVFormCFormID->Width = 70;
			// 
			// LVFormCPlugin
			// 
			this->LVFormCPlugin->Text = L"Parent Plugin";
			this->LVFormCPlugin->Width = 140;
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
			this->ListViewObjectUsage->FullRowSelect = true;
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
			// 
			// LVObjectsCType
			// 
			this->LVObjectsCType->Text = L"Type";
			this->LVObjectsCType->Width = 65;
			// 
			// LVObjectsCEditorID
			// 
			this->LVObjectsCEditorID->Text = L"Editor ID";
			this->LVObjectsCEditorID->Width = 165;
			// 
			// LVObjectsCFormID
			// 
			this->LVObjectsCFormID->Text = L"Form ID";
			this->LVObjectsCFormID->Width = 65;
			// 
			// LVObjectsCPlugin
			// 
			this->LVObjectsCPlugin->Text = L"Parent Plugin";
			this->LVObjectsCPlugin->Width = 165;
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
			this->ListViewCellUsage->FullRowSelect = true;
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
			// 
			// LVCellsCWorldEditorID
			// 
			this->LVCellsCWorldEditorID->Text = L"World";
			this->LVCellsCWorldEditorID->Width = 100;
			// 
			// LVCellsCCellEditorID
			// 
			this->LVCellsCCellEditorID->Text = L"Cell";
			this->LVCellsCCellEditorID->Width = 150;
			// 
			// LVCellsCGrid
			// 
			this->LVCellsCGrid->Text = L"Grid";
			this->LVCellsCGrid->Width = 50;
			// 
			// LVCellsCUseCount
			// 
			this->LVCellsCUseCount->Text = L"Use Count";
			this->LVCellsCUseCount->Width = 70;
			// 
			// LVCellsCRef
			// 
			this->LVCellsCRef->Text = L"First Reference";
			this->LVCellsCRef->Width = 165;
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
			this->KeyPreview = true;
			this->MaximizeBox = false;
			this->Name = L"UseInfoList";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Centeralized Use Info Listing";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewForms))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewObjectUsage))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ListViewCellUsage))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	};
}
