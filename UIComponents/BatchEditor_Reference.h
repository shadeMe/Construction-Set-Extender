#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace UIComponents {

	/// <summary>
	/// Summary for BatchEditor_Reference
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class BatchEditor_Reference : public System::Windows::Forms::Form
	{
	public:
		BatchEditor_Reference(void)
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
		~BatchEditor_Reference()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::ListView^  ObjectList;
	private: System::Windows::Forms::ColumnHeader^  ObjectList_CHEditorID;
	private: System::Windows::Forms::ColumnHeader^  ObjectList_CHFormID;
	private: System::Windows::Forms::ColumnHeader^  ObjectList_CHType;
	private: System::Windows::Forms::Button^  ApplyButton;
	private: System::Windows::Forms::Button^  CancelButton;
	private: System::Windows::Forms::TabControl^  DataBox;
	private: System::Windows::Forms::TabPage^  DataBox_TB3DData;
	private: System::Windows::Forms::TabPage^  DataBox_TBFlags;
	private: System::Windows::Forms::TabPage^  DataBox_TBEnableParent;
	private: System::Windows::Forms::TabPage^  DataBox_TBOwnership;
	private: System::Windows::Forms::GroupBox^  PositionBox;
	private: System::Windows::Forms::GroupBox^  RotationBox;
	private: System::Windows::Forms::Label^  LabelZ;

	private: System::Windows::Forms::Label^  LabelY;

	private: System::Windows::Forms::Label^  LabelX;
	private: System::Windows::Forms::TextBox^  PosZ;


	private: System::Windows::Forms::TextBox^  PosY;

	private: System::Windows::Forms::TextBox^  PosX;
	private: System::Windows::Forms::Label^  LabelZEx;
	private: System::Windows::Forms::TextBox^  RotX;

	private: System::Windows::Forms::Label^  LabelYEx;
	private: System::Windows::Forms::TextBox^  RotY;



	private: System::Windows::Forms::Label^  LabelXEx;
	private: System::Windows::Forms::TextBox^  RotZ;
	private: System::Windows::Forms::Label^  LabelScale;
	private: System::Windows::Forms::TextBox^  Scale;
	private: System::Windows::Forms::CheckBox^  Use3DData;
	private: System::Windows::Forms::CheckBox^  FlagVWD;
	private: System::Windows::Forms::CheckBox^  FlagDisabled;


	private: System::Windows::Forms::CheckBox^  FlagPersistent;
	private: System::Windows::Forms::CheckBox^  UseFlags;












	protected: 



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
			this->ObjectList = (gcnew System::Windows::Forms::ListView());
			this->ObjectList_CHEditorID = (gcnew System::Windows::Forms::ColumnHeader());
			this->ObjectList_CHFormID = (gcnew System::Windows::Forms::ColumnHeader());
			this->ObjectList_CHType = (gcnew System::Windows::Forms::ColumnHeader());
			this->ApplyButton = (gcnew System::Windows::Forms::Button());
			this->CancelButton = (gcnew System::Windows::Forms::Button());
			this->DataBox = (gcnew System::Windows::Forms::TabControl());
			this->DataBox_TB3DData = (gcnew System::Windows::Forms::TabPage());
			this->RotationBox = (gcnew System::Windows::Forms::GroupBox());
			this->PositionBox = (gcnew System::Windows::Forms::GroupBox());
			this->DataBox_TBFlags = (gcnew System::Windows::Forms::TabPage());
			this->DataBox_TBEnableParent = (gcnew System::Windows::Forms::TabPage());
			this->DataBox_TBOwnership = (gcnew System::Windows::Forms::TabPage());
			this->PosX = (gcnew System::Windows::Forms::TextBox());
			this->PosY = (gcnew System::Windows::Forms::TextBox());
			this->PosZ = (gcnew System::Windows::Forms::TextBox());
			this->LabelX = (gcnew System::Windows::Forms::Label());
			this->LabelY = (gcnew System::Windows::Forms::Label());
			this->LabelZ = (gcnew System::Windows::Forms::Label());
			this->LabelZEx = (gcnew System::Windows::Forms::Label());
			this->LabelYEx = (gcnew System::Windows::Forms::Label());
			this->LabelXEx = (gcnew System::Windows::Forms::Label());
			this->RotZ = (gcnew System::Windows::Forms::TextBox());
			this->RotY = (gcnew System::Windows::Forms::TextBox());
			this->RotX = (gcnew System::Windows::Forms::TextBox());
			this->LabelScale = (gcnew System::Windows::Forms::Label());
			this->Scale = (gcnew System::Windows::Forms::TextBox());
			this->Use3DData = (gcnew System::Windows::Forms::CheckBox());
			this->FlagPersistent = (gcnew System::Windows::Forms::CheckBox());
			this->FlagDisabled = (gcnew System::Windows::Forms::CheckBox());
			this->FlagVWD = (gcnew System::Windows::Forms::CheckBox());
			this->UseFlags = (gcnew System::Windows::Forms::CheckBox());
			this->DataBox->SuspendLayout();
			this->DataBox_TB3DData->SuspendLayout();
			this->RotationBox->SuspendLayout();
			this->PositionBox->SuspendLayout();
			this->DataBox_TBFlags->SuspendLayout();
			this->SuspendLayout();
			// 
			// ObjectList
			// 
			this->ObjectList->CheckBoxes = true;
			this->ObjectList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->ObjectList_CHEditorID, 
				this->ObjectList_CHFormID, this->ObjectList_CHType});
			this->ObjectList->FullRowSelect = true;
			this->ObjectList->GridLines = true;
			this->ObjectList->HideSelection = false;
			this->ObjectList->Location = System::Drawing::Point(12, 12);
			this->ObjectList->Name = L"ObjectList";
			this->ObjectList->Size = System::Drawing::Size(306, 419);
			this->ObjectList->TabIndex = 0;
			this->ObjectList->UseCompatibleStateImageBehavior = false;
			this->ObjectList->View = System::Windows::Forms::View::Details;
			// 
			// ObjectList_CHEditorID
			// 
			this->ObjectList_CHEditorID->Text = L"Editor ID";
			this->ObjectList_CHEditorID->Width = 142;
			// 
			// ObjectList_CHFormID
			// 
			this->ObjectList_CHFormID->Text = L"Form ID";
			this->ObjectList_CHFormID->Width = 76;
			// 
			// ObjectList_CHType
			// 
			this->ObjectList_CHType->Text = L" Type";
			this->ObjectList_CHType->Width = 83;
			// 
			// ApplyButton
			// 
			this->ApplyButton->Location = System::Drawing::Point(214, 437);
			this->ApplyButton->Name = L"ApplyButton";
			this->ApplyButton->Size = System::Drawing::Size(103, 39);
			this->ApplyButton->TabIndex = 1;
			this->ApplyButton->Text = L"Apply Changes";
			this->ApplyButton->UseVisualStyleBackColor = true;
			// 
			// CancelButton
			// 
			this->CancelButton->Location = System::Drawing::Point(323, 437);
			this->CancelButton->Name = L"CancelButton";
			this->CancelButton->Size = System::Drawing::Size(103, 39);
			this->CancelButton->TabIndex = 2;
			this->CancelButton->Text = L"Discard Changes";
			this->CancelButton->UseVisualStyleBackColor = true;
			// 
			// DataBox
			// 
			this->DataBox->Controls->Add(this->DataBox_TB3DData);
			this->DataBox->Controls->Add(this->DataBox_TBFlags);
			this->DataBox->Controls->Add(this->DataBox_TBEnableParent);
			this->DataBox->Controls->Add(this->DataBox_TBOwnership);
			this->DataBox->HotTrack = true;
			this->DataBox->Location = System::Drawing::Point(336, 12);
			this->DataBox->Name = L"DataBox";
			this->DataBox->SelectedIndex = 0;
			this->DataBox->Size = System::Drawing::Size(337, 419);
			this->DataBox->TabIndex = 3;
			// 
			// DataBox_TB3DData
			// 
			this->DataBox_TB3DData->Controls->Add(this->Use3DData);
			this->DataBox_TB3DData->Controls->Add(this->LabelScale);
			this->DataBox_TB3DData->Controls->Add(this->RotationBox);
			this->DataBox_TB3DData->Controls->Add(this->Scale);
			this->DataBox_TB3DData->Controls->Add(this->PositionBox);
			this->DataBox_TB3DData->Location = System::Drawing::Point(4, 22);
			this->DataBox_TB3DData->Name = L"DataBox_TB3DData";
			this->DataBox_TB3DData->Padding = System::Windows::Forms::Padding(3);
			this->DataBox_TB3DData->Size = System::Drawing::Size(329, 393);
			this->DataBox_TB3DData->TabIndex = 0;
			this->DataBox_TB3DData->Text = L"3D Data";
			this->DataBox_TB3DData->UseVisualStyleBackColor = true;
			// 
			// RotationBox
			// 
			this->RotationBox->Controls->Add(this->LabelZEx);
			this->RotationBox->Controls->Add(this->RotX);
			this->RotationBox->Controls->Add(this->LabelYEx);
			this->RotationBox->Controls->Add(this->RotY);
			this->RotationBox->Controls->Add(this->LabelXEx);
			this->RotationBox->Controls->Add(this->RotZ);
			this->RotationBox->Location = System::Drawing::Point(6, 211);
			this->RotationBox->Name = L"RotationBox";
			this->RotationBox->Size = System::Drawing::Size(317, 150);
			this->RotationBox->TabIndex = 1;
			this->RotationBox->TabStop = false;
			this->RotationBox->Text = L"Rotation";
			// 
			// PositionBox
			// 
			this->PositionBox->Controls->Add(this->LabelZ);
			this->PositionBox->Controls->Add(this->LabelY);
			this->PositionBox->Controls->Add(this->LabelX);
			this->PositionBox->Controls->Add(this->PosZ);
			this->PositionBox->Controls->Add(this->PosY);
			this->PositionBox->Controls->Add(this->PosX);
			this->PositionBox->Location = System::Drawing::Point(6, 36);
			this->PositionBox->Name = L"PositionBox";
			this->PositionBox->Size = System::Drawing::Size(317, 150);
			this->PositionBox->TabIndex = 0;
			this->PositionBox->TabStop = false;
			this->PositionBox->Text = L"Position";
			// 
			// DataBox_TBFlags
			// 
			this->DataBox_TBFlags->Controls->Add(this->UseFlags);
			this->DataBox_TBFlags->Controls->Add(this->FlagVWD);
			this->DataBox_TBFlags->Controls->Add(this->FlagDisabled);
			this->DataBox_TBFlags->Controls->Add(this->FlagPersistent);
			this->DataBox_TBFlags->Location = System::Drawing::Point(4, 22);
			this->DataBox_TBFlags->Name = L"DataBox_TBFlags";
			this->DataBox_TBFlags->Padding = System::Windows::Forms::Padding(3);
			this->DataBox_TBFlags->Size = System::Drawing::Size(329, 393);
			this->DataBox_TBFlags->TabIndex = 1;
			this->DataBox_TBFlags->Text = L"Flags";
			this->DataBox_TBFlags->UseVisualStyleBackColor = true;
			// 
			// DataBox_TBEnableParent
			// 
			this->DataBox_TBEnableParent->Location = System::Drawing::Point(4, 22);
			this->DataBox_TBEnableParent->Name = L"DataBox_TBEnableParent";
			this->DataBox_TBEnableParent->Size = System::Drawing::Size(329, 393);
			this->DataBox_TBEnableParent->TabIndex = 2;
			this->DataBox_TBEnableParent->Text = L"Enable Parent";
			this->DataBox_TBEnableParent->UseVisualStyleBackColor = true;
			// 
			// DataBox_TBOwnership
			// 
			this->DataBox_TBOwnership->Location = System::Drawing::Point(4, 22);
			this->DataBox_TBOwnership->Name = L"DataBox_TBOwnership";
			this->DataBox_TBOwnership->Size = System::Drawing::Size(329, 393);
			this->DataBox_TBOwnership->TabIndex = 3;
			this->DataBox_TBOwnership->Text = L"Ownership";
			this->DataBox_TBOwnership->UseVisualStyleBackColor = true;
			// 
			// PosX
			// 
			this->PosX->Location = System::Drawing::Point(29, 19);
			this->PosX->Name = L"PosX";
			this->PosX->Size = System::Drawing::Size(215, 20);
			this->PosX->TabIndex = 0;
			// 
			// PosY
			// 
			this->PosY->Location = System::Drawing::Point(29, 70);
			this->PosY->Name = L"PosY";
			this->PosY->Size = System::Drawing::Size(215, 20);
			this->PosY->TabIndex = 1;
			// 
			// PosZ
			// 
			this->PosZ->Location = System::Drawing::Point(29, 119);
			this->PosZ->Name = L"PosZ";
			this->PosZ->Size = System::Drawing::Size(215, 20);
			this->PosZ->TabIndex = 2;
			// 
			// LabelX
			// 
			this->LabelX->AutoSize = true;
			this->LabelX->Location = System::Drawing::Point(261, 22);
			this->LabelX->Name = L"LabelX";
			this->LabelX->Size = System::Drawing::Size(14, 13);
			this->LabelX->TabIndex = 3;
			this->LabelX->Text = L"X";
			// 
			// LabelY
			// 
			this->LabelY->AutoSize = true;
			this->LabelY->Location = System::Drawing::Point(261, 73);
			this->LabelY->Name = L"LabelY";
			this->LabelY->Size = System::Drawing::Size(14, 13);
			this->LabelY->TabIndex = 4;
			this->LabelY->Text = L"Y";
			// 
			// LabelZ
			// 
			this->LabelZ->AutoSize = true;
			this->LabelZ->Location = System::Drawing::Point(261, 122);
			this->LabelZ->Name = L"LabelZ";
			this->LabelZ->Size = System::Drawing::Size(14, 13);
			this->LabelZ->TabIndex = 5;
			this->LabelZ->Text = L"Z";
			// 
			// LabelZEx
			// 
			this->LabelZEx->AutoSize = true;
			this->LabelZEx->Location = System::Drawing::Point(261, 122);
			this->LabelZEx->Name = L"LabelZEx";
			this->LabelZEx->Size = System::Drawing::Size(14, 13);
			this->LabelZEx->TabIndex = 11;
			this->LabelZEx->Text = L"Z";
			// 
			// LabelYEx
			// 
			this->LabelYEx->AutoSize = true;
			this->LabelYEx->Location = System::Drawing::Point(261, 73);
			this->LabelYEx->Name = L"LabelYEx";
			this->LabelYEx->Size = System::Drawing::Size(14, 13);
			this->LabelYEx->TabIndex = 10;
			this->LabelYEx->Text = L"Y";
			// 
			// LabelXEx
			// 
			this->LabelXEx->AutoSize = true;
			this->LabelXEx->Location = System::Drawing::Point(261, 22);
			this->LabelXEx->Name = L"LabelXEx";
			this->LabelXEx->Size = System::Drawing::Size(14, 13);
			this->LabelXEx->TabIndex = 9;
			this->LabelXEx->Text = L"X";
			// 
			// RotZ
			// 
			this->RotZ->Location = System::Drawing::Point(29, 119);
			this->RotZ->Name = L"RotZ";
			this->RotZ->Size = System::Drawing::Size(215, 20);
			this->RotZ->TabIndex = 8;
			// 
			// RotY
			// 
			this->RotY->Location = System::Drawing::Point(29, 70);
			this->RotY->Name = L"RotY";
			this->RotY->Size = System::Drawing::Size(215, 20);
			this->RotY->TabIndex = 7;
			// 
			// RotX
			// 
			this->RotX->Location = System::Drawing::Point(29, 19);
			this->RotX->Name = L"RotX";
			this->RotX->Size = System::Drawing::Size(215, 20);
			this->RotX->TabIndex = 6;
			// 
			// LabelScale
			// 
			this->LabelScale->AutoSize = true;
			this->LabelScale->Location = System::Drawing::Point(267, 370);
			this->LabelScale->Name = L"LabelScale";
			this->LabelScale->Size = System::Drawing::Size(34, 13);
			this->LabelScale->TabIndex = 13;
			this->LabelScale->Text = L"Scale";
			// 
			// Scale
			// 
			this->Scale->Location = System::Drawing::Point(35, 367);
			this->Scale->Name = L"Scale";
			this->Scale->Size = System::Drawing::Size(215, 20);
			this->Scale->TabIndex = 12;
			// 
			// Use3DData
			// 
			this->Use3DData->AutoSize = true;
			this->Use3DData->Location = System::Drawing::Point(6, 13);
			this->Use3DData->Name = L"Use3DData";
			this->Use3DData->Size = System::Drawing::Size(88, 17);
			this->Use3DData->TabIndex = 14;
			this->Use3DData->Text = L"Use 3D Data";
			this->Use3DData->UseVisualStyleBackColor = true;
			// 
			// FlagPersistent
			// 
			this->FlagPersistent->AutoSize = true;
			this->FlagPersistent->Location = System::Drawing::Point(15, 68);
			this->FlagPersistent->Name = L"FlagPersistent";
			this->FlagPersistent->Size = System::Drawing::Size(72, 17);
			this->FlagPersistent->TabIndex = 0;
			this->FlagPersistent->Text = L"Persistent";
			this->FlagPersistent->UseVisualStyleBackColor = true;
			// 
			// FlagDisabled
			// 
			this->FlagDisabled->AutoSize = true;
			this->FlagDisabled->Location = System::Drawing::Point(15, 110);
			this->FlagDisabled->Name = L"FlagDisabled";
			this->FlagDisabled->Size = System::Drawing::Size(101, 17);
			this->FlagDisabled->TabIndex = 1;
			this->FlagDisabled->Text = L"Initially Disabled";
			this->FlagDisabled->UseVisualStyleBackColor = true;
			// 
			// FlagVWD
			// 
			this->FlagVWD->AutoSize = true;
			this->FlagVWD->Location = System::Drawing::Point(186, 68);
			this->FlagVWD->Name = L"FlagVWD";
			this->FlagVWD->Size = System::Drawing::Size(124, 17);
			this->FlagVWD->TabIndex = 2;
			this->FlagVWD->Text = L"Visible When Distant";
			this->FlagVWD->UseVisualStyleBackColor = true;
			// 
			// UseFlags
			// 
			this->UseFlags->AutoSize = true;
			this->UseFlags->Location = System::Drawing::Point(6, 13);
			this->UseFlags->Name = L"UseFlags";
			this->UseFlags->Size = System::Drawing::Size(73, 17);
			this->UseFlags->TabIndex = 15;
			this->UseFlags->Text = L"Use Flags";
			this->UseFlags->UseVisualStyleBackColor = true;
			// 
			// BatchEditor_Reference
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(686, 482);
			this->ControlBox = false;
			this->Controls->Add(this->DataBox);
			this->Controls->Add(this->CancelButton);
			this->Controls->Add(this->ApplyButton);
			this->Controls->Add(this->ObjectList);
			this->Name = L"BatchEditor_Reference";
			this->Text = L"Batch Editor - References";
			this->DataBox->ResumeLayout(false);
			this->DataBox_TB3DData->ResumeLayout(false);
			this->DataBox_TB3DData->PerformLayout();
			this->RotationBox->ResumeLayout(false);
			this->RotationBox->PerformLayout();
			this->PositionBox->ResumeLayout(false);
			this->PositionBox->PerformLayout();
			this->DataBox_TBFlags->ResumeLayout(false);
			this->DataBox_TBFlags->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
