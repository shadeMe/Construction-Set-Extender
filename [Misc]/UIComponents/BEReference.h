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
	private: System::Windows::Forms::CheckBox^  UsePosX;

	private: System::Windows::Forms::CheckBox^  FlagVWD;
	private: System::Windows::Forms::CheckBox^  FlagDisabled;


	private: System::Windows::Forms::CheckBox^  FlagPersistent;
	private: System::Windows::Forms::CheckBox^  UsePersistent;

	private: System::Windows::Forms::TabPage^  DataBox_TBExtra;
	private: System::Windows::Forms::CheckBox^  UseCount;

	private: System::Windows::Forms::Label^  LabelCount;
	private: System::Windows::Forms::TextBox^  Count;
	private: System::Windows::Forms::Label^  LabelHealth;
	private: System::Windows::Forms::TextBox^  Health;



	private: System::Windows::Forms::Label^  LabelCharge;
	private: System::Windows::Forms::TextBox^  Charge;


	private: System::Windows::Forms::Label^  LabelTimeLeft;
	private: System::Windows::Forms::TextBox^  TimeLeft;
	private: System::Windows::Forms::ComboBox^  SoulLevel;

	private: System::Windows::Forms::Label^  LabelSoul;
	private: System::Windows::Forms::CheckBox^  UseRotX;
	private: System::Windows::Forms::CheckBox^  UseScale;



	private: System::Windows::Forms::CheckBox^  UseRotZ;

	private: System::Windows::Forms::CheckBox^  UseRotY;

	private: System::Windows::Forms::CheckBox^  UsePosZ;

	private: System::Windows::Forms::CheckBox^  UsePosY;
	private: System::Windows::Forms::CheckBox^  UseDisabled;
	private: System::Windows::Forms::CheckBox^  UseVWD;
	private: System::Windows::Forms::CheckBox^  UseHealth;
	private: System::Windows::Forms::CheckBox^  UseTimeLeft;
private: System::Windows::Forms::CheckBox^  UseSoulLevel;
private: System::Windows::Forms::CheckBox^  UseCharge;




private: System::Windows::Forms::CheckBox^  UseEnableParent;


private: System::Windows::Forms::CheckBox^  OppositeState;
private: System::Windows::Forms::CheckBox^  UseOwnership;
private: System::Windows::Forms::Label^  LabelFaction;
private: System::Windows::Forms::Label^  LabelNPC;
private: System::Windows::Forms::ComboBox^  NPCList;
private: System::Windows::Forms::Label^  LabelGlobal;

private: System::Windows::Forms::ComboBox^  FactionList;
private: System::Windows::Forms::Label^  LabelRank;

private: System::Windows::Forms::ComboBox^  GlobalList;
private: System::Windows::Forms::TextBox^  Rank;
private: System::Windows::Forms::RadioButton^  UseNPCOwner;
private: System::Windows::Forms::RadioButton^  UseFactionOwner;
private: System::Windows::Forms::Button^  SetParent;































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
			this->UseScale = (gcnew System::Windows::Forms::CheckBox());
			this->LabelScale = (gcnew System::Windows::Forms::Label());
			this->RotationBox = (gcnew System::Windows::Forms::GroupBox());
			this->LabelZEx = (gcnew System::Windows::Forms::Label());
			this->UseRotZ = (gcnew System::Windows::Forms::CheckBox());
			this->RotX = (gcnew System::Windows::Forms::TextBox());
			this->UseRotY = (gcnew System::Windows::Forms::CheckBox());
			this->LabelYEx = (gcnew System::Windows::Forms::Label());
			this->UseRotX = (gcnew System::Windows::Forms::CheckBox());
			this->RotY = (gcnew System::Windows::Forms::TextBox());
			this->LabelXEx = (gcnew System::Windows::Forms::Label());
			this->RotZ = (gcnew System::Windows::Forms::TextBox());
			this->Scale = (gcnew System::Windows::Forms::TextBox());
			this->PositionBox = (gcnew System::Windows::Forms::GroupBox());
			this->UsePosZ = (gcnew System::Windows::Forms::CheckBox());
			this->UsePosY = (gcnew System::Windows::Forms::CheckBox());
			this->UsePosX = (gcnew System::Windows::Forms::CheckBox());
			this->LabelZ = (gcnew System::Windows::Forms::Label());
			this->LabelY = (gcnew System::Windows::Forms::Label());
			this->LabelX = (gcnew System::Windows::Forms::Label());
			this->PosZ = (gcnew System::Windows::Forms::TextBox());
			this->PosY = (gcnew System::Windows::Forms::TextBox());
			this->PosX = (gcnew System::Windows::Forms::TextBox());
			this->DataBox_TBFlags = (gcnew System::Windows::Forms::TabPage());
			this->UseDisabled = (gcnew System::Windows::Forms::CheckBox());
			this->UseVWD = (gcnew System::Windows::Forms::CheckBox());
			this->UsePersistent = (gcnew System::Windows::Forms::CheckBox());
			this->FlagVWD = (gcnew System::Windows::Forms::CheckBox());
			this->FlagDisabled = (gcnew System::Windows::Forms::CheckBox());
			this->FlagPersistent = (gcnew System::Windows::Forms::CheckBox());
			this->DataBox_TBEnableParent = (gcnew System::Windows::Forms::TabPage());
			this->OppositeState = (gcnew System::Windows::Forms::CheckBox());
			this->UseEnableParent = (gcnew System::Windows::Forms::CheckBox());
			this->DataBox_TBOwnership = (gcnew System::Windows::Forms::TabPage());
			this->UseFactionOwner = (gcnew System::Windows::Forms::RadioButton());
			this->UseNPCOwner = (gcnew System::Windows::Forms::RadioButton());
			this->Rank = (gcnew System::Windows::Forms::TextBox());
			this->LabelRank = (gcnew System::Windows::Forms::Label());
			this->GlobalList = (gcnew System::Windows::Forms::ComboBox());
			this->LabelGlobal = (gcnew System::Windows::Forms::Label());
			this->FactionList = (gcnew System::Windows::Forms::ComboBox());
			this->NPCList = (gcnew System::Windows::Forms::ComboBox());
			this->LabelFaction = (gcnew System::Windows::Forms::Label());
			this->LabelNPC = (gcnew System::Windows::Forms::Label());
			this->UseOwnership = (gcnew System::Windows::Forms::CheckBox());
			this->DataBox_TBExtra = (gcnew System::Windows::Forms::TabPage());
			this->UseHealth = (gcnew System::Windows::Forms::CheckBox());
			this->UseTimeLeft = (gcnew System::Windows::Forms::CheckBox());
			this->UseSoulLevel = (gcnew System::Windows::Forms::CheckBox());
			this->UseCharge = (gcnew System::Windows::Forms::CheckBox());
			this->LabelSoul = (gcnew System::Windows::Forms::Label());
			this->SoulLevel = (gcnew System::Windows::Forms::ComboBox());
			this->LabelHealth = (gcnew System::Windows::Forms::Label());
			this->Health = (gcnew System::Windows::Forms::TextBox());
			this->LabelCharge = (gcnew System::Windows::Forms::Label());
			this->Charge = (gcnew System::Windows::Forms::TextBox());
			this->LabelTimeLeft = (gcnew System::Windows::Forms::Label());
			this->TimeLeft = (gcnew System::Windows::Forms::TextBox());
			this->LabelCount = (gcnew System::Windows::Forms::Label());
			this->Count = (gcnew System::Windows::Forms::TextBox());
			this->UseCount = (gcnew System::Windows::Forms::CheckBox());
			this->SetParent = (gcnew System::Windows::Forms::Button());
			this->DataBox->SuspendLayout();
			this->DataBox_TB3DData->SuspendLayout();
			this->RotationBox->SuspendLayout();
			this->PositionBox->SuspendLayout();
			this->DataBox_TBFlags->SuspendLayout();
			this->DataBox_TBEnableParent->SuspendLayout();
			this->DataBox_TBOwnership->SuspendLayout();
			this->DataBox_TBExtra->SuspendLayout();
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
			this->ObjectList->Size = System::Drawing::Size(318, 419);
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
			this->DataBox->Controls->Add(this->DataBox_TBExtra);
			this->DataBox->HotTrack = true;
			this->DataBox->Location = System::Drawing::Point(336, 12);
			this->DataBox->Name = L"DataBox";
			this->DataBox->SelectedIndex = 0;
			this->DataBox->Size = System::Drawing::Size(337, 419);
			this->DataBox->TabIndex = 3;
			// 
			// DataBox_TB3DData
			// 
			this->DataBox_TB3DData->Controls->Add(this->UseScale);
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
			// UseScale
			// 
			this->UseScale->AutoSize = true;
			this->UseScale->Location = System::Drawing::Point(14, 359);
			this->UseScale->Name = L"UseScale";
			this->UseScale->Size = System::Drawing::Size(15, 14);
			this->UseScale->TabIndex = 20;
			this->UseScale->UseVisualStyleBackColor = true;
			// 
			// LabelScale
			// 
			this->LabelScale->AutoSize = true;
			this->LabelScale->Location = System::Drawing::Point(267, 359);
			this->LabelScale->Name = L"LabelScale";
			this->LabelScale->Size = System::Drawing::Size(34, 13);
			this->LabelScale->TabIndex = 13;
			this->LabelScale->Text = L"Scale";
			// 
			// RotationBox
			// 
			this->RotationBox->Controls->Add(this->LabelZEx);
			this->RotationBox->Controls->Add(this->UseRotZ);
			this->RotationBox->Controls->Add(this->RotX);
			this->RotationBox->Controls->Add(this->UseRotY);
			this->RotationBox->Controls->Add(this->LabelYEx);
			this->RotationBox->Controls->Add(this->UseRotX);
			this->RotationBox->Controls->Add(this->RotY);
			this->RotationBox->Controls->Add(this->LabelXEx);
			this->RotationBox->Controls->Add(this->RotZ);
			this->RotationBox->Location = System::Drawing::Point(6, 181);
			this->RotationBox->Name = L"RotationBox";
			this->RotationBox->Size = System::Drawing::Size(317, 150);
			this->RotationBox->TabIndex = 1;
			this->RotationBox->TabStop = false;
			this->RotationBox->Text = L"Rotation";
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
			// UseRotZ
			// 
			this->UseRotZ->AutoSize = true;
			this->UseRotZ->Location = System::Drawing::Point(8, 122);
			this->UseRotZ->Name = L"UseRotZ";
			this->UseRotZ->Size = System::Drawing::Size(15, 14);
			this->UseRotZ->TabIndex = 19;
			this->UseRotZ->UseVisualStyleBackColor = true;
			this->UseRotZ->CheckedChanged += gcnew System::EventHandler(this, &BatchEditor_Reference::checkBox5_CheckedChanged);
			// 
			// RotX
			// 
			this->RotX->Location = System::Drawing::Point(29, 19);
			this->RotX->Name = L"RotX";
			this->RotX->Size = System::Drawing::Size(215, 20);
			this->RotX->TabIndex = 6;
			// 
			// UseRotY
			// 
			this->UseRotY->AutoSize = true;
			this->UseRotY->Location = System::Drawing::Point(8, 73);
			this->UseRotY->Name = L"UseRotY";
			this->UseRotY->Size = System::Drawing::Size(15, 14);
			this->UseRotY->TabIndex = 18;
			this->UseRotY->UseVisualStyleBackColor = true;
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
			// UseRotX
			// 
			this->UseRotX->AutoSize = true;
			this->UseRotX->Location = System::Drawing::Point(8, 22);
			this->UseRotX->Name = L"UseRotX";
			this->UseRotX->Size = System::Drawing::Size(15, 14);
			this->UseRotX->TabIndex = 17;
			this->UseRotX->UseVisualStyleBackColor = true;
			this->UseRotX->CheckedChanged += gcnew System::EventHandler(this, &BatchEditor_Reference::checkBox3_CheckedChanged);
			// 
			// RotY
			// 
			this->RotY->Location = System::Drawing::Point(29, 70);
			this->RotY->Name = L"RotY";
			this->RotY->Size = System::Drawing::Size(215, 20);
			this->RotY->TabIndex = 7;
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
			// Scale
			// 
			this->Scale->Location = System::Drawing::Point(35, 356);
			this->Scale->Name = L"Scale";
			this->Scale->Size = System::Drawing::Size(215, 20);
			this->Scale->TabIndex = 12;
			// 
			// PositionBox
			// 
			this->PositionBox->Controls->Add(this->UsePosZ);
			this->PositionBox->Controls->Add(this->UsePosY);
			this->PositionBox->Controls->Add(this->UsePosX);
			this->PositionBox->Controls->Add(this->LabelZ);
			this->PositionBox->Controls->Add(this->LabelY);
			this->PositionBox->Controls->Add(this->LabelX);
			this->PositionBox->Controls->Add(this->PosZ);
			this->PositionBox->Controls->Add(this->PosY);
			this->PositionBox->Controls->Add(this->PosX);
			this->PositionBox->Location = System::Drawing::Point(6, 15);
			this->PositionBox->Name = L"PositionBox";
			this->PositionBox->Size = System::Drawing::Size(317, 150);
			this->PositionBox->TabIndex = 0;
			this->PositionBox->TabStop = false;
			this->PositionBox->Text = L"Position";
			// 
			// UsePosZ
			// 
			this->UsePosZ->AutoSize = true;
			this->UsePosZ->Location = System::Drawing::Point(8, 122);
			this->UsePosZ->Name = L"UsePosZ";
			this->UsePosZ->Size = System::Drawing::Size(15, 14);
			this->UsePosZ->TabIndex = 16;
			this->UsePosZ->UseVisualStyleBackColor = true;
			// 
			// UsePosY
			// 
			this->UsePosY->AutoSize = true;
			this->UsePosY->Location = System::Drawing::Point(8, 73);
			this->UsePosY->Name = L"UsePosY";
			this->UsePosY->Size = System::Drawing::Size(15, 14);
			this->UsePosY->TabIndex = 15;
			this->UsePosY->UseVisualStyleBackColor = true;
			// 
			// UsePosX
			// 
			this->UsePosX->AutoSize = true;
			this->UsePosX->Location = System::Drawing::Point(8, 22);
			this->UsePosX->Name = L"UsePosX";
			this->UsePosX->Size = System::Drawing::Size(15, 14);
			this->UsePosX->TabIndex = 14;
			this->UsePosX->UseVisualStyleBackColor = true;
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
			// LabelY
			// 
			this->LabelY->AutoSize = true;
			this->LabelY->Location = System::Drawing::Point(261, 73);
			this->LabelY->Name = L"LabelY";
			this->LabelY->Size = System::Drawing::Size(14, 13);
			this->LabelY->TabIndex = 4;
			this->LabelY->Text = L"Y";
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
			// PosZ
			// 
			this->PosZ->Location = System::Drawing::Point(29, 119);
			this->PosZ->Name = L"PosZ";
			this->PosZ->Size = System::Drawing::Size(215, 20);
			this->PosZ->TabIndex = 2;
			// 
			// PosY
			// 
			this->PosY->Location = System::Drawing::Point(29, 70);
			this->PosY->Name = L"PosY";
			this->PosY->Size = System::Drawing::Size(215, 20);
			this->PosY->TabIndex = 1;
			// 
			// PosX
			// 
			this->PosX->Location = System::Drawing::Point(29, 19);
			this->PosX->Name = L"PosX";
			this->PosX->Size = System::Drawing::Size(215, 20);
			this->PosX->TabIndex = 0;
			// 
			// DataBox_TBFlags
			// 
			this->DataBox_TBFlags->Controls->Add(this->UseDisabled);
			this->DataBox_TBFlags->Controls->Add(this->UseVWD);
			this->DataBox_TBFlags->Controls->Add(this->UsePersistent);
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
			// UseDisabled
			// 
			this->UseDisabled->AutoSize = true;
			this->UseDisabled->Location = System::Drawing::Point(81, 147);
			this->UseDisabled->Name = L"UseDisabled";
			this->UseDisabled->Size = System::Drawing::Size(15, 14);
			this->UseDisabled->TabIndex = 17;
			this->UseDisabled->UseVisualStyleBackColor = true;
			this->UseDisabled->CheckedChanged += gcnew System::EventHandler(this, &BatchEditor_Reference::checkBox2_CheckedChanged);
			// 
			// UseVWD
			// 
			this->UseVWD->AutoSize = true;
			this->UseVWD->Location = System::Drawing::Point(81, 180);
			this->UseVWD->Name = L"UseVWD";
			this->UseVWD->Size = System::Drawing::Size(15, 14);
			this->UseVWD->TabIndex = 16;
			this->UseVWD->UseVisualStyleBackColor = true;
			// 
			// UsePersistent
			// 
			this->UsePersistent->AutoSize = true;
			this->UsePersistent->Location = System::Drawing::Point(81, 115);
			this->UsePersistent->Name = L"UsePersistent";
			this->UsePersistent->Size = System::Drawing::Size(15, 14);
			this->UsePersistent->TabIndex = 15;
			this->UsePersistent->UseVisualStyleBackColor = true;
			// 
			// FlagVWD
			// 
			this->FlagVWD->AutoSize = true;
			this->FlagVWD->Location = System::Drawing::Point(111, 179);
			this->FlagVWD->Name = L"FlagVWD";
			this->FlagVWD->Size = System::Drawing::Size(124, 17);
			this->FlagVWD->TabIndex = 2;
			this->FlagVWD->Text = L"Visible When Distant";
			this->FlagVWD->UseVisualStyleBackColor = true;
			// 
			// FlagDisabled
			// 
			this->FlagDisabled->AutoSize = true;
			this->FlagDisabled->Location = System::Drawing::Point(111, 146);
			this->FlagDisabled->Name = L"FlagDisabled";
			this->FlagDisabled->Size = System::Drawing::Size(101, 17);
			this->FlagDisabled->TabIndex = 1;
			this->FlagDisabled->Text = L"Initially Disabled";
			this->FlagDisabled->UseVisualStyleBackColor = true;
			// 
			// FlagPersistent
			// 
			this->FlagPersistent->AutoSize = true;
			this->FlagPersistent->Location = System::Drawing::Point(111, 114);
			this->FlagPersistent->Name = L"FlagPersistent";
			this->FlagPersistent->Size = System::Drawing::Size(72, 17);
			this->FlagPersistent->TabIndex = 0;
			this->FlagPersistent->Text = L"Persistent";
			this->FlagPersistent->UseVisualStyleBackColor = true;
			// 
			// DataBox_TBEnableParent
			// 
			this->DataBox_TBEnableParent->Controls->Add(this->SetParent);
			this->DataBox_TBEnableParent->Controls->Add(this->OppositeState);
			this->DataBox_TBEnableParent->Controls->Add(this->UseEnableParent);
			this->DataBox_TBEnableParent->Location = System::Drawing::Point(4, 22);
			this->DataBox_TBEnableParent->Name = L"DataBox_TBEnableParent";
			this->DataBox_TBEnableParent->Size = System::Drawing::Size(329, 393);
			this->DataBox_TBEnableParent->TabIndex = 2;
			this->DataBox_TBEnableParent->Text = L"Enable Parent";
			this->DataBox_TBEnableParent->UseVisualStyleBackColor = true;
			// 
			// OppositeState
			// 
			this->OppositeState->AutoSize = true;
			this->OppositeState->Location = System::Drawing::Point(63, 179);
			this->OppositeState->Name = L"OppositeState";
			this->OppositeState->Size = System::Drawing::Size(203, 17);
			this->OppositeState->TabIndex = 3;
			this->OppositeState->Text = L"Set enable state to opposite of parent";
			this->OppositeState->UseVisualStyleBackColor = true;
			// 
			// UseEnableParent
			// 
			this->UseEnableParent->AutoSize = true;
			this->UseEnableParent->Location = System::Drawing::Point(63, 106);
			this->UseEnableParent->Name = L"UseEnableParent";
			this->UseEnableParent->Size = System::Drawing::Size(15, 14);
			this->UseEnableParent->TabIndex = 0;
			this->UseEnableParent->UseVisualStyleBackColor = true;
			// 
			// DataBox_TBOwnership
			// 
			this->DataBox_TBOwnership->Controls->Add(this->UseFactionOwner);
			this->DataBox_TBOwnership->Controls->Add(this->UseNPCOwner);
			this->DataBox_TBOwnership->Controls->Add(this->Rank);
			this->DataBox_TBOwnership->Controls->Add(this->LabelRank);
			this->DataBox_TBOwnership->Controls->Add(this->GlobalList);
			this->DataBox_TBOwnership->Controls->Add(this->LabelGlobal);
			this->DataBox_TBOwnership->Controls->Add(this->FactionList);
			this->DataBox_TBOwnership->Controls->Add(this->NPCList);
			this->DataBox_TBOwnership->Controls->Add(this->LabelFaction);
			this->DataBox_TBOwnership->Controls->Add(this->LabelNPC);
			this->DataBox_TBOwnership->Controls->Add(this->UseOwnership);
			this->DataBox_TBOwnership->Location = System::Drawing::Point(4, 22);
			this->DataBox_TBOwnership->Name = L"DataBox_TBOwnership";
			this->DataBox_TBOwnership->Size = System::Drawing::Size(329, 393);
			this->DataBox_TBOwnership->TabIndex = 3;
			this->DataBox_TBOwnership->Text = L"Ownership";
			this->DataBox_TBOwnership->UseVisualStyleBackColor = true;
			// 
			// UseFactionOwner
			// 
			this->UseFactionOwner->AutoSize = true;
			this->UseFactionOwner->Location = System::Drawing::Point(27, 137);
			this->UseFactionOwner->Name = L"UseFactionOwner";
			this->UseFactionOwner->Size = System::Drawing::Size(14, 13);
			this->UseFactionOwner->TabIndex = 10;
			this->UseFactionOwner->UseVisualStyleBackColor = true;
			// 
			// UseNPCOwner
			// 
			this->UseNPCOwner->AutoSize = true;
			this->UseNPCOwner->Checked = true;
			this->UseNPCOwner->Location = System::Drawing::Point(27, 71);
			this->UseNPCOwner->Name = L"UseNPCOwner";
			this->UseNPCOwner->Size = System::Drawing::Size(14, 13);
			this->UseNPCOwner->TabIndex = 9;
			this->UseNPCOwner->TabStop = true;
			this->UseNPCOwner->UseVisualStyleBackColor = true;
			// 
			// Rank
			// 
			this->Rank->Location = System::Drawing::Point(211, 207);
			this->Rank->Name = L"Rank";
			this->Rank->Size = System::Drawing::Size(82, 20);
			this->Rank->TabIndex = 8;
			// 
			// LabelRank
			// 
			this->LabelRank->AutoSize = true;
			this->LabelRank->Location = System::Drawing::Point(24, 210);
			this->LabelRank->Name = L"LabelRank";
			this->LabelRank->Size = System::Drawing::Size(181, 13);
			this->LabelRank->TabIndex = 7;
			this->LabelRank->Text = L"Required Rank Index ( -1 for NONE )";
			// 
			// GlobalList
			// 
			this->GlobalList->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->GlobalList->FormattingEnabled = true;
			this->GlobalList->Location = System::Drawing::Point(176, 97);
			this->GlobalList->Name = L"GlobalList";
			this->GlobalList->Size = System::Drawing::Size(138, 21);
			this->GlobalList->TabIndex = 6;
			// 
			// LabelGlobal
			// 
			this->LabelGlobal->AutoSize = true;
			this->LabelGlobal->Location = System::Drawing::Point(203, 71);
			this->LabelGlobal->Name = L"LabelGlobal";
			this->LabelGlobal->Size = System::Drawing::Size(78, 13);
			this->LabelGlobal->TabIndex = 5;
			this->LabelGlobal->Text = L"Global Variable";
			// 
			// FactionList
			// 
			this->FactionList->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->FactionList->FormattingEnabled = true;
			this->FactionList->Location = System::Drawing::Point(27, 162);
			this->FactionList->Name = L"FactionList";
			this->FactionList->Size = System::Drawing::Size(138, 21);
			this->FactionList->TabIndex = 4;
			// 
			// NPCList
			// 
			this->NPCList->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->NPCList->FormattingEnabled = true;
			this->NPCList->Location = System::Drawing::Point(27, 98);
			this->NPCList->Name = L"NPCList";
			this->NPCList->Size = System::Drawing::Size(138, 21);
			this->NPCList->TabIndex = 3;
			// 
			// LabelFaction
			// 
			this->LabelFaction->AutoSize = true;
			this->LabelFaction->Location = System::Drawing::Point(80, 137);
			this->LabelFaction->Name = L"LabelFaction";
			this->LabelFaction->Size = System::Drawing::Size(42, 13);
			this->LabelFaction->TabIndex = 2;
			this->LabelFaction->Text = L"Faction";
			// 
			// LabelNPC
			// 
			this->LabelNPC->AutoSize = true;
			this->LabelNPC->Location = System::Drawing::Point(80, 71);
			this->LabelNPC->Name = L"LabelNPC";
			this->LabelNPC->Size = System::Drawing::Size(29, 13);
			this->LabelNPC->TabIndex = 1;
			this->LabelNPC->Text = L"NPC";
			// 
			// UseOwnership
			// 
			this->UseOwnership->AutoSize = true;
			this->UseOwnership->Location = System::Drawing::Point(27, 23);
			this->UseOwnership->Name = L"UseOwnership";
			this->UseOwnership->Size = System::Drawing::Size(15, 14);
			this->UseOwnership->TabIndex = 0;
			this->UseOwnership->UseVisualStyleBackColor = true;
			// 
			// DataBox_TBExtra
			// 
			this->DataBox_TBExtra->Controls->Add(this->UseHealth);
			this->DataBox_TBExtra->Controls->Add(this->UseTimeLeft);
			this->DataBox_TBExtra->Controls->Add(this->UseSoulLevel);
			this->DataBox_TBExtra->Controls->Add(this->UseCharge);
			this->DataBox_TBExtra->Controls->Add(this->LabelSoul);
			this->DataBox_TBExtra->Controls->Add(this->SoulLevel);
			this->DataBox_TBExtra->Controls->Add(this->LabelHealth);
			this->DataBox_TBExtra->Controls->Add(this->Health);
			this->DataBox_TBExtra->Controls->Add(this->LabelCharge);
			this->DataBox_TBExtra->Controls->Add(this->Charge);
			this->DataBox_TBExtra->Controls->Add(this->LabelTimeLeft);
			this->DataBox_TBExtra->Controls->Add(this->TimeLeft);
			this->DataBox_TBExtra->Controls->Add(this->LabelCount);
			this->DataBox_TBExtra->Controls->Add(this->Count);
			this->DataBox_TBExtra->Controls->Add(this->UseCount);
			this->DataBox_TBExtra->Location = System::Drawing::Point(4, 22);
			this->DataBox_TBExtra->Name = L"DataBox_TBExtra";
			this->DataBox_TBExtra->Padding = System::Windows::Forms::Padding(3);
			this->DataBox_TBExtra->Size = System::Drawing::Size(329, 393);
			this->DataBox_TBExtra->TabIndex = 4;
			this->DataBox_TBExtra->Text = L"Extra";
			this->DataBox_TBExtra->UseVisualStyleBackColor = true;
			// 
			// UseHealth
			// 
			this->UseHealth->AutoSize = true;
			this->UseHealth->Location = System::Drawing::Point(19, 129);
			this->UseHealth->Name = L"UseHealth";
			this->UseHealth->Size = System::Drawing::Size(15, 14);
			this->UseHealth->TabIndex = 14;
			this->UseHealth->UseVisualStyleBackColor = true;
			// 
			// UseTimeLeft
			// 
			this->UseTimeLeft->AutoSize = true;
			this->UseTimeLeft->Location = System::Drawing::Point(19, 216);
			this->UseTimeLeft->Name = L"UseTimeLeft";
			this->UseTimeLeft->Size = System::Drawing::Size(15, 14);
			this->UseTimeLeft->TabIndex = 13;
			this->UseTimeLeft->UseVisualStyleBackColor = true;
			// 
			// UseSoulLevel
			// 
			this->UseSoulLevel->AutoSize = true;
			this->UseSoulLevel->Location = System::Drawing::Point(19, 259);
			this->UseSoulLevel->Name = L"UseSoulLevel";
			this->UseSoulLevel->Size = System::Drawing::Size(15, 14);
			this->UseSoulLevel->TabIndex = 12;
			this->UseSoulLevel->UseVisualStyleBackColor = true;
			// 
			// UseCharge
			// 
			this->UseCharge->AutoSize = true;
			this->UseCharge->Location = System::Drawing::Point(19, 174);
			this->UseCharge->Name = L"UseCharge";
			this->UseCharge->Size = System::Drawing::Size(15, 14);
			this->UseCharge->TabIndex = 11;
			this->UseCharge->UseVisualStyleBackColor = true;
			// 
			// LabelSoul
			// 
			this->LabelSoul->AutoSize = true;
			this->LabelSoul->Location = System::Drawing::Point(264, 256);
			this->LabelSoul->Name = L"LabelSoul";
			this->LabelSoul->Size = System::Drawing::Size(57, 13);
			this->LabelSoul->TabIndex = 10;
			this->LabelSoul->Text = L"Soul Level";
			// 
			// SoulLevel
			// 
			this->SoulLevel->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->SoulLevel->FormattingEnabled = true;
			this->SoulLevel->Items->AddRange(gcnew cli::array< System::Object^  >(6) {L"None", L"Petty", L"Lesser", L"Common", L"Greater", 
				L"Grand"});
			this->SoulLevel->Location = System::Drawing::Point(40, 256);
			this->SoulLevel->Name = L"SoulLevel";
			this->SoulLevel->Size = System::Drawing::Size(208, 21);
			this->SoulLevel->TabIndex = 9;
			// 
			// LabelHealth
			// 
			this->LabelHealth->AutoSize = true;
			this->LabelHealth->Location = System::Drawing::Point(264, 126);
			this->LabelHealth->Name = L"LabelHealth";
			this->LabelHealth->Size = System::Drawing::Size(38, 13);
			this->LabelHealth->TabIndex = 8;
			this->LabelHealth->Text = L"Health";
			// 
			// Health
			// 
			this->Health->Location = System::Drawing::Point(40, 126);
			this->Health->Name = L"Health";
			this->Health->Size = System::Drawing::Size(208, 20);
			this->Health->TabIndex = 7;
			// 
			// LabelCharge
			// 
			this->LabelCharge->AutoSize = true;
			this->LabelCharge->Location = System::Drawing::Point(264, 171);
			this->LabelCharge->Name = L"LabelCharge";
			this->LabelCharge->Size = System::Drawing::Size(41, 13);
			this->LabelCharge->TabIndex = 6;
			this->LabelCharge->Text = L"Charge";
			// 
			// Charge
			// 
			this->Charge->Location = System::Drawing::Point(40, 171);
			this->Charge->Name = L"Charge";
			this->Charge->Size = System::Drawing::Size(208, 20);
			this->Charge->TabIndex = 5;
			// 
			// LabelTimeLeft
			// 
			this->LabelTimeLeft->AutoSize = true;
			this->LabelTimeLeft->Location = System::Drawing::Point(264, 213);
			this->LabelTimeLeft->Name = L"LabelTimeLeft";
			this->LabelTimeLeft->Size = System::Drawing::Size(51, 13);
			this->LabelTimeLeft->TabIndex = 4;
			this->LabelTimeLeft->Text = L"Time Left";
			// 
			// TimeLeft
			// 
			this->TimeLeft->Location = System::Drawing::Point(40, 213);
			this->TimeLeft->Name = L"TimeLeft";
			this->TimeLeft->Size = System::Drawing::Size(208, 20);
			this->TimeLeft->TabIndex = 3;
			// 
			// LabelCount
			// 
			this->LabelCount->AutoSize = true;
			this->LabelCount->Location = System::Drawing::Point(264, 84);
			this->LabelCount->Name = L"LabelCount";
			this->LabelCount->Size = System::Drawing::Size(35, 13);
			this->LabelCount->TabIndex = 2;
			this->LabelCount->Text = L"Count";
			this->LabelCount->Click += gcnew System::EventHandler(this, &BatchEditor_Reference::LabelCount_Click);
			// 
			// Count
			// 
			this->Count->Location = System::Drawing::Point(40, 84);
			this->Count->Name = L"Count";
			this->Count->Size = System::Drawing::Size(208, 20);
			this->Count->TabIndex = 1;
			this->Count->TextChanged += gcnew System::EventHandler(this, &BatchEditor_Reference::Count_TextChanged);
			// 
			// UseCount
			// 
			this->UseCount->AutoSize = true;
			this->UseCount->Location = System::Drawing::Point(19, 87);
			this->UseCount->Name = L"UseCount";
			this->UseCount->Size = System::Drawing::Size(15, 14);
			this->UseCount->TabIndex = 0;
			this->UseCount->UseVisualStyleBackColor = true;
			// 
			// SetParent
			// 
			this->SetParent->Location = System::Drawing::Point(63, 145);
			this->SetParent->Name = L"SetParent";
			this->SetParent->Size = System::Drawing::Size(202, 28);
			this->SetParent->TabIndex = 4;
			this->SetParent->Text = L"Set Parent to ";
			this->SetParent->UseVisualStyleBackColor = true;
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
			this->DataBox_TBEnableParent->ResumeLayout(false);
			this->DataBox_TBEnableParent->PerformLayout();
			this->DataBox_TBOwnership->ResumeLayout(false);
			this->DataBox_TBOwnership->PerformLayout();
			this->DataBox_TBExtra->ResumeLayout(false);
			this->DataBox_TBExtra->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void Count_TextChanged(System::Object^  sender, System::EventArgs^  e) {
			 }
private: System::Void LabelCount_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
private: System::Void checkBox3_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void checkBox5_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void checkBox2_CheckedChanged(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}
