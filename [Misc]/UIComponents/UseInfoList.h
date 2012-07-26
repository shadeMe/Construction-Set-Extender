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
	private: System::Windows::Forms::ListView^  FormList;
	protected: 
	private: System::Windows::Forms::ColumnHeader^  FormListCType;
	private: System::Windows::Forms::ColumnHeader^  FormListCEditorID;
	private: System::Windows::Forms::ColumnHeader^  FormListCFormID;
	private: System::Windows::Forms::ListView^  UseListObject;
	private: System::Windows::Forms::ColumnHeader^  UseListObjectCType;
	private: System::Windows::Forms::ColumnHeader^  UseListObjectCEditorID;
	private: System::Windows::Forms::ColumnHeader^  UseListObjectCFormID;
	private: System::Windows::Forms::GroupBox^  UseListObjectGroup;
	private: System::Windows::Forms::GroupBox^  UseListCellGroup;
	private: System::Windows::Forms::ListView^  UseListCell;
	private: System::Windows::Forms::ColumnHeader^  UseListCellCWorldEditorID;
	private: System::Windows::Forms::ColumnHeader^  UseListCellCCellFormID;
	private: System::Windows::Forms::ColumnHeader^  UseListCellCCellEditorID;
	private: System::Windows::Forms::ColumnHeader^  UseListCellCCellGrid;
	private: System::Windows::Forms::ColumnHeader^  UseListCellCUseCount;
	private: System::Windows::Forms::TextBox^  SearchBox;

	private: System::Windows::Forms::Button^  FilterLabel;
















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
			this->FormList = (gcnew System::Windows::Forms::ListView());
			this->FormListCType = (gcnew System::Windows::Forms::ColumnHeader());
			this->FormListCEditorID = (gcnew System::Windows::Forms::ColumnHeader());
			this->FormListCFormID = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListObject = (gcnew System::Windows::Forms::ListView());
			this->UseListObjectCType = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListObjectCEditorID = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListObjectCFormID = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListObjectGroup = (gcnew System::Windows::Forms::GroupBox());
			this->UseListCellGroup = (gcnew System::Windows::Forms::GroupBox());
			this->UseListCell = (gcnew System::Windows::Forms::ListView());
			this->UseListCellCWorldEditorID = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListCellCCellFormID = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListCellCCellEditorID = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListCellCCellGrid = (gcnew System::Windows::Forms::ColumnHeader());
			this->UseListCellCUseCount = (gcnew System::Windows::Forms::ColumnHeader());
			this->SearchBox = (gcnew System::Windows::Forms::TextBox());
			this->FilterLabel = (gcnew System::Windows::Forms::Button());
			this->UseListObjectGroup->SuspendLayout();
			this->UseListCellGroup->SuspendLayout();
			this->SuspendLayout();
			// 
			// FormList
			// 
			this->FormList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->FormListCType, this->FormListCEditorID, 
				this->FormListCFormID});
			this->FormList->Location = System::Drawing::Point(12, 12);
			this->FormList->Name = L"FormList";
			this->FormList->Size = System::Drawing::Size(475, 638);
			this->FormList->TabIndex = 0;
			this->FormList->UseCompatibleStateImageBehavior = false;
			this->FormList->View = System::Windows::Forms::View::Details;
			// 
			// FormListCType
			// 
			this->FormListCType->Text = L"Type";
			this->FormListCType->Width = 46;
			// 
			// FormListCEditorID
			// 
			this->FormListCEditorID->Text = L"Editor ID";
			this->FormListCEditorID->Width = 167;
			// 
			// FormListCFormID
			// 
			this->FormListCFormID->Text = L"Form ID";
			this->FormListCFormID->Width = 121;
			// 
			// UseListObject
			// 
			this->UseListObject->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->UseListObjectCType, 
				this->UseListObjectCEditorID, this->UseListObjectCFormID});
			this->UseListObject->Dock = System::Windows::Forms::DockStyle::Fill;
			this->UseListObject->Location = System::Drawing::Point(3, 16);
			this->UseListObject->Name = L"UseListObject";
			this->UseListObject->Size = System::Drawing::Size(463, 303);
			this->UseListObject->TabIndex = 1;
			this->UseListObject->UseCompatibleStateImageBehavior = false;
			this->UseListObject->View = System::Windows::Forms::View::Details;
			// 
			// UseListObjectCType
			// 
			this->UseListObjectCType->Text = L"Type";
			// 
			// UseListObjectCEditorID
			// 
			this->UseListObjectCEditorID->Text = L"Editor ID";
			// 
			// UseListObjectCFormID
			// 
			this->UseListObjectCFormID->Text = L"Form ID";
			// 
			// UseListObjectGroup
			// 
			this->UseListObjectGroup->Controls->Add(this->UseListObject);
			this->UseListObjectGroup->Location = System::Drawing::Point(493, 12);
			this->UseListObjectGroup->Name = L"UseListObjectGroup";
			this->UseListObjectGroup->Size = System::Drawing::Size(469, 322);
			this->UseListObjectGroup->TabIndex = 2;
			this->UseListObjectGroup->TabStop = false;
			this->UseListObjectGroup->Text = L"Used by these objects";
			// 
			// UseListCellGroup
			// 
			this->UseListCellGroup->Controls->Add(this->UseListCell);
			this->UseListCellGroup->Location = System::Drawing::Point(493, 340);
			this->UseListCellGroup->Name = L"UseListCellGroup";
			this->UseListCellGroup->Size = System::Drawing::Size(469, 345);
			this->UseListCellGroup->TabIndex = 3;
			this->UseListCellGroup->TabStop = false;
			this->UseListCellGroup->Text = L"Used in these cells";
			// 
			// UseListCell
			// 
			this->UseListCell->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(5) {this->UseListCellCWorldEditorID, 
				this->UseListCellCCellFormID, this->UseListCellCCellEditorID, this->UseListCellCCellGrid, this->UseListCellCUseCount});
			this->UseListCell->Dock = System::Windows::Forms::DockStyle::Fill;
			this->UseListCell->Location = System::Drawing::Point(3, 16);
			this->UseListCell->Name = L"UseListCell";
			this->UseListCell->Size = System::Drawing::Size(463, 326);
			this->UseListCell->TabIndex = 1;
			this->UseListCell->UseCompatibleStateImageBehavior = false;
			this->UseListCell->View = System::Windows::Forms::View::Details;
			// 
			// UseListCellCWorldEditorID
			// 
			this->UseListCellCWorldEditorID->Text = L"World Editor ID";
			this->UseListCellCWorldEditorID->Width = 104;
			// 
			// UseListCellCCellFormID
			// 
			this->UseListCellCCellFormID->Text = L"Cell Form ID";
			this->UseListCellCCellFormID->Width = 5;
			// 
			// UseListCellCCellEditorID
			// 
			this->UseListCellCCellEditorID->Text = L"Cell Editor ID";
			this->UseListCellCCellEditorID->Width = 112;
			// 
			// UseListCellCCellGrid
			// 
			this->UseListCellCCellGrid->Text = L"Cell Grid";
			// 
			// UseListCellCUseCount
			// 
			this->UseListCellCUseCount->Text = L"Use Count";
			this->UseListCellCUseCount->Width = 69;
			// 
			// SearchBox
			// 
			this->SearchBox->Location = System::Drawing::Point(12, 656);
			this->SearchBox->MaxLength = 100;
			this->SearchBox->Multiline = true;
			this->SearchBox->Name = L"SearchBox";
			this->SearchBox->Size = System::Drawing::Size(371, 29);
			this->SearchBox->TabIndex = 4;
			// 
			// FilterLabel
			// 
			this->FilterLabel->Enabled = false;
			this->FilterLabel->Location = System::Drawing::Point(389, 656);
			this->FilterLabel->Name = L"FilterLabel";
			this->FilterLabel->Size = System::Drawing::Size(98, 29);
			this->FilterLabel->TabIndex = 5;
			this->FilterLabel->Text = L"<< Filter String";
			this->FilterLabel->UseVisualStyleBackColor = true;
			// 
			// UseInfoList
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(974, 697);
			this->Controls->Add(this->FilterLabel);
			this->Controls->Add(this->SearchBox);
			this->Controls->Add(this->UseListCellGroup);
			this->Controls->Add(this->FormList);
			this->Controls->Add(this->UseListObjectGroup);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->Name = L"UseInfoList";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Use Info List";
			this->UseListObjectGroup->ResumeLayout(false);
			this->UseListCellGroup->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	};
}
