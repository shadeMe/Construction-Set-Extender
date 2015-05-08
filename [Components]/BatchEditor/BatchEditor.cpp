#include "BatchEditor.h"
#include "[Common]\ListViewUtilities.h"

using namespace ComponentDLLInterface;

namespace ConstructionSetExtender
{
	void BatchEditor::ApplyButton_Click(Object^ Sender, EventArgs^ E)
	{
		if (ScrubData())
			Close(DialogResult::OK);
	}
	void BatchEditor::CancelButton_Click(Object^ Sender, EventArgs^ E)
	{
		Close(DialogResult::Cancel);
	}

	void BatchEditor::Close(DialogResult Result)
	{
		BatchEditBox->DialogResult = Result;
		Cleanup();
		BatchEditBox->Hide();
	}

	RefBatchEditor^% RefBatchEditor::GetSingleton()
	{
		if (Singleton == nullptr)
		{
			Singleton = gcnew RefBatchEditor();
		}
		return Singleton;
	}

	RefBatchEditor::RefBatchEditor()
	{
		Application::EnableVisualStyles();
		BatchEditBox = gcnew Form();
#pragma region Designer Gunk
		this->ObjectList = (gcnew ListView());
		this->ObjectList_CHEditorID = (gcnew ColumnHeader());
		this->ObjectList_CHFormID = (gcnew ColumnHeader());
		this->ObjectList_CHType = (gcnew ColumnHeader());
		this->ApplyButton = (gcnew Button());
		this->CancelButton = (gcnew Button());
		this->DataBox = (gcnew TabControl());
		this->DataBox_TB3DData = (gcnew TabPage());
		this->UseScale = (gcnew CheckBox());
		this->LabelScale = (gcnew Label());
		this->RotationBox = (gcnew GroupBox());
		this->LabelZEx = (gcnew Label());
		this->UseRotZ = (gcnew CheckBox());
		this->RotX = (gcnew TextBox());
		this->UseRotY = (gcnew CheckBox());
		this->LabelYEx = (gcnew Label());
		this->UseRotX = (gcnew CheckBox());
		this->RotY = (gcnew TextBox());
		this->LabelXEx = (gcnew Label());
		this->RotZ = (gcnew TextBox());
		this->Scale = (gcnew TextBox());
		this->PositionBox = (gcnew GroupBox());
		this->UsePosZ = (gcnew CheckBox());
		this->UsePosY = (gcnew CheckBox());
		this->UsePosX = (gcnew CheckBox());
		this->LabelZ = (gcnew Label());
		this->LabelY = (gcnew Label());
		this->LabelX = (gcnew Label());
		this->PosZ = (gcnew TextBox());
		this->PosY = (gcnew TextBox());
		this->PosX = (gcnew TextBox());
		this->DataBox_TBFlags = (gcnew TabPage());
		this->UseDisabled = (gcnew CheckBox());
		this->UseVWD = (gcnew CheckBox());
		this->UsePersistent = (gcnew CheckBox());
		this->FlagVWD = (gcnew CheckBox());
		this->FlagDisabled = (gcnew CheckBox());
		this->FlagPersistent = (gcnew CheckBox());
		this->DataBox_TBEnableParent = (gcnew TabPage());
		this->DataBox_TBOwnership = (gcnew TabPage());
		this->DataBox_TBExtra = (gcnew TabPage());
		this->UseHealth = (gcnew CheckBox());
		this->UseTimeLeft = (gcnew CheckBox());
		this->UseSoulLevel = (gcnew CheckBox());
		this->UseCharge = (gcnew CheckBox());
		this->LabelSoul = (gcnew Label());
		this->SoulLevel = (gcnew ComboBox());
		this->LabelHealth = (gcnew Label());
		this->Health = (gcnew TextBox());
		this->LabelCharge = (gcnew Label());
		this->Charge = (gcnew TextBox());
		this->LabelTimeLeft = (gcnew Label());
		this->TimeLeft = (gcnew TextBox());
		this->LabelCount = (gcnew Label());
		this->Count = (gcnew TextBox());
		this->UseCount = (gcnew CheckBox());
		this->UseEnableParent = (gcnew CheckBox());
		this->SetParent = (gcnew Button());
		this->OppositeState = (gcnew CheckBox());
		this->UseOwnership = (gcnew CheckBox());
		this->LabelNPC = (gcnew Label());
		this->LabelFaction = (gcnew Label());
		this->NPCList = (gcnew ComboBox());
		this->FactionList = (gcnew ComboBox());
		this->LabelGlobal = (gcnew Label());
		this->GlobalList = (gcnew ComboBox());
		this->LabelRank = (gcnew Label());
		this->Rank = (gcnew TextBox());
		this->UseNPCOwner = (gcnew RadioButton());
		this->UseFactionOwner = (gcnew RadioButton());
		this->DataBox->SuspendLayout();
		this->DataBox_TB3DData->SuspendLayout();
		this->RotationBox->SuspendLayout();
		this->PositionBox->SuspendLayout();
		this->DataBox_TBFlags->SuspendLayout();
		this->DataBox_TBEnableParent->SuspendLayout();
		this->DataBox_TBOwnership->SuspendLayout();
		this->DataBox_TBExtra->SuspendLayout();
		this->BatchEditBox->SuspendLayout();
		//
		// ObjectList
		//
		this->ObjectList->CheckBoxes = true;
		this->ObjectList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(3) {this->ObjectList_CHEditorID,
			this->ObjectList_CHFormID, this->ObjectList_CHType});
		this->ObjectList->FullRowSelect = true;
		this->ObjectList->GridLines = true;
		this->ObjectList->HideSelection = false;
		this->ObjectList->Location = System::Drawing::Point(12, 12);
		this->ObjectList->Name = L"ObjectList";
		this->ObjectList->Size = System::Drawing::Size(318, 419);
		this->ObjectList->TabIndex = 0;
		this->ObjectList->UseCompatibleStateImageBehavior = false;
		this->ObjectList->View = View::Details;
		ObjectList->ColumnClick += gcnew ColumnClickEventHandler(this, &RefBatchEditor::ObjectList_ColumnClick);
		ObjectList->Tag = (int)1;
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
		ApplyButton->Click += gcnew EventHandler(this, &RefBatchEditor::ApplyButton_Click);
		//
		// CancelButton
		//
		this->CancelButton->Location = System::Drawing::Point(323, 437);
		this->CancelButton->Name = L"CancelButton";
		this->CancelButton->Size = System::Drawing::Size(103, 39);
		this->CancelButton->TabIndex = 2;
		this->CancelButton->Text = L"Discard Changes";
		this->CancelButton->UseVisualStyleBackColor = true;
		CancelButton->Click += gcnew EventHandler(this, &RefBatchEditor::CancelButton_Click);
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
		this->DataBox_TB3DData->Padding = Padding(3);
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
		this->DataBox_TBFlags->Padding = Padding(3);
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
		// SetParent
		//
		this->SetParent->Location = System::Drawing::Point(41, 134);
		this->SetParent->Name = L"SetParent";
		this->SetParent->Size = System::Drawing::Size(202, 28);
		this->SetParent->TabIndex = 4;
		this->SetParent->Text = L"Set Parent to NONE";
		this->SetParent->UseVisualStyleBackColor = true;
		SetParent->Click += gcnew EventHandler(this, &RefBatchEditor::SetParent_Click);
		//
		// DataBox_TBEnableParent
		//
		this->DataBox_TBEnableParent->Controls->Add(this->OppositeState);
		this->DataBox_TBEnableParent->Controls->Add(this->SetParent);
		this->DataBox_TBEnableParent->Controls->Add(this->UseEnableParent);
		this->DataBox_TBEnableParent->Location = System::Drawing::Point(4, 22);
		this->DataBox_TBEnableParent->Name = L"DataBox_TBEnableParent";
		this->DataBox_TBEnableParent->Size = System::Drawing::Size(329, 393);
		this->DataBox_TBEnableParent->TabIndex = 2;
		this->DataBox_TBEnableParent->Text = L"Enable Parent";
		this->DataBox_TBEnableParent->UseVisualStyleBackColor = true;
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
		this->DataBox_TBExtra->Padding = Padding(3);
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
		this->SoulLevel->DropDownStyle = ComboBoxStyle::DropDownList;
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
		//
		// Count
		//
		this->Count->Location = System::Drawing::Point(40, 84);
		this->Count->Name = L"Count";
		this->Count->Size = System::Drawing::Size(208, 20);
		this->Count->TabIndex = 1;
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
		// UseEnableParent
		//
		this->UseEnableParent->AutoSize = true;
		this->UseEnableParent->Location = System::Drawing::Point(41, 96);
		this->UseEnableParent->Name = L"UseEnableParent";
		this->UseEnableParent->Size = System::Drawing::Size(15, 14);
		this->UseEnableParent->TabIndex = 0;
		this->UseEnableParent->UseVisualStyleBackColor = true;
		//
		// OppositeState
		//
		this->OppositeState->AutoSize = true;
		this->OppositeState->Location = System::Drawing::Point(41, 169);
		this->OppositeState->Name = L"OppositeState";
		this->OppositeState->Size = System::Drawing::Size(203, 17);
		this->OppositeState->TabIndex = 3;
		this->OppositeState->Text = L"Set enable state to opposite of parent";
		this->OppositeState->UseVisualStyleBackColor = true;
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
		// LabelNPC
		//
		this->LabelNPC->AutoSize = true;
		this->LabelNPC->Location = System::Drawing::Point(80, 71);
		this->LabelNPC->Name = L"LabelNPC";
		this->LabelNPC->Size = System::Drawing::Size(29, 13);
		this->LabelNPC->TabIndex = 1;
		this->LabelNPC->Text = L"NPC";
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
		// NPCList
		//
		this->NPCList->DropDownStyle = ComboBoxStyle::DropDownList;
		this->NPCList->FormattingEnabled = true;
		this->NPCList->Location = System::Drawing::Point(27, 98);
		this->NPCList->Name = L"NPCList";
		this->NPCList->Size = System::Drawing::Size(138, 21);
		this->NPCList->TabIndex = 3;
		NPCList->Sorted = false;
		NPCList->DropDownHeight = 500;
		NPCList->DropDownWidth = 250;
		NPCList->Tag = gcnew List<UInt32>();
		//
		// FactionList
		//
		this->FactionList->DropDownStyle = ComboBoxStyle::DropDownList;
		this->FactionList->FormattingEnabled = true;
		this->FactionList->Location = System::Drawing::Point(27, 162);
		this->FactionList->Name = L"FactionList";
		this->FactionList->Size = System::Drawing::Size(138, 21);
		this->FactionList->TabIndex = 4;
		FactionList->Sorted = false;
		FactionList->DropDownHeight = 500;
		FactionList->DropDownWidth = 250;
		FactionList->Tag = gcnew List<UInt32>();
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
		// GlobalList
		//
		this->GlobalList->DropDownStyle = ComboBoxStyle::DropDownList;
		this->GlobalList->FormattingEnabled = true;
		this->GlobalList->Location = System::Drawing::Point(176, 97);
		this->GlobalList->Name = L"GlobalList";
		this->GlobalList->Size = System::Drawing::Size(138, 21);
		this->GlobalList->TabIndex = 6;
		GlobalList->Sorted = false;
		GlobalList->DropDownHeight = 500;
		GlobalList->DropDownWidth = 250;
		GlobalList->Tag = gcnew List<UInt32>();
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
		// Rank
		//
		this->Rank->Location = System::Drawing::Point(211, 207);
		this->Rank->Name = L"Rank";
		this->Rank->Size = System::Drawing::Size(82, 20);
		this->Rank->TabIndex = 8;
		this->Rank->Text = "-1";
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
		// UseFactionOwner
		//
		this->UseFactionOwner->AutoSize = true;
		this->UseFactionOwner->Location = System::Drawing::Point(27, 137);
		this->UseFactionOwner->Name = L"UseFactionOwner";
		this->UseFactionOwner->Size = System::Drawing::Size(14, 13);
		this->UseFactionOwner->TabIndex = 10;
		this->UseFactionOwner->UseVisualStyleBackColor = true;
		//
		// BatchEditor_Reference
		//
		BatchEditBox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		BatchEditBox->AutoScaleMode = AutoScaleMode::Font;
		BatchEditBox->ClientSize = System::Drawing::Size(686, 482);
		BatchEditBox->ControlBox = false;
		BatchEditBox->FormBorderStyle = FormBorderStyle::FixedDialog;
		BatchEditBox->Controls->Add(this->DataBox);
		BatchEditBox->Controls->Add(this->CancelButton);
		BatchEditBox->Controls->Add(this->ApplyButton);
		BatchEditBox->Controls->Add(this->ObjectList);
		BatchEditBox->Text = L"Batch Editor - References";
		BatchEditBox->StartPosition = FormStartPosition::CenterScreen;
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
		BatchEditBox->ResumeLayout(false);
#pragma endregion

		BatchEditBox->Hide();
	}

	void RefBatchEditor::Cleanup()
	{
		List<UInt32>^ PtrList = nullptr;
		ObjectList->Items->Clear();

		NPCList->Items->Clear();
		PtrList = dynamic_cast< List<UInt32>^ >(NPCList->Tag);
		PtrList->Clear();

		GlobalList->Items->Clear();
		PtrList = dynamic_cast< List<UInt32>^ >(GlobalList->Tag);
		PtrList->Clear();

		FactionList->Items->Clear();
		PtrList = dynamic_cast< List<UInt32>^ >(FactionList->Tag);
		PtrList->Clear();

		SetParent->Text = "Set Parent to NONE";

		for each (TabPage^ Tab in DataBox->TabPages)
		{
			for each (Control^ Itr in Tab->Controls)
			{
				GroupBox^ Box = dynamic_cast<GroupBox^>(Itr);
				if (Box != nullptr)
					SanitizeControls(Box);
			}

			SanitizeControls(Tab);
		}

		Rank->Text = "-1";
	}

	void RefBatchEditor::SanitizeControls(Control^ Container)
	{
		for each (Control^ Itr in Container->Controls)
		{
			TextBox^ TB = dynamic_cast<TextBox^>(Itr);
			if (TB != nullptr)
				TB->Text = "";

			CheckBox^ CB = dynamic_cast<CheckBox^>(Itr);
			if (CB != nullptr)
				CB->Checked = false;
		}
	}

	void RefBatchEditor::PopulateObjectList(BatchRefData* Data)
	{
		ObjectList->BeginUpdate();

		for (CellObjectData* Itr = Data->CellObjectListHead; Itr != Data->CellObjectListHead + Data->ObjectCount; ++Itr)
		{
			if (!Itr->IsValid())
				continue;

			ListViewItem^ NewItem = gcnew ListViewItem(gcnew String(Itr->EditorID));
			NewItem->SubItems->Add(Itr->FormID.ToString("X8"));
			NewItem->SubItems->Add(gcnew String(NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormTypeIDLongName(Itr->TypeID)));
			NewItem->Tag = (UInt32)Itr;

			if (Itr->Selected)
				NewItem->Checked = true;

			ObjectList->Items->Add(NewItem);
		}

		ObjectList->EndUpdate();
	}

	void RefBatchEditor::PopulateFormLists()
	{
		ComboBox^ FormList = nullptr;

		BatchRefOwnerFormData* Data = NativeWrapper::g_CSEInterfaceTable->BatchRefEditor.GetOwnershipData();
		NPCList->BeginUpdate();
		GlobalList->BeginUpdate();
		FactionList->BeginUpdate();

		NPCList->Items->Clear();
		NPCList->Items->Add("NONE");
		((List<UInt32>^)NPCList->Tag)->Add((UInt32)0);

		GlobalList->Items->Clear();
		GlobalList->Items->Add("NONE");
		((List<UInt32>^)GlobalList->Tag)->Add((UInt32)0);

		FactionList->Items->Clear();
		FactionList->Items->Add("NONE");
		((List<UInt32>^)FactionList->Tag)->Add((UInt32)0);

		for (int i = 0; i < Data->FormCount; i++)
		{
			FormData* ThisForm = &Data->FormListHead[i];
			switch (ThisForm->TypeID)
			{
			case 0x23:	// NPC
				FormList = NPCList;
				break;
			case 0x04:	// TESGlobal
				FormList = GlobalList;
				break;
			case 0x6:	// TESFaction
				FormList = FactionList;
				break;
			}

			List<UInt32>^ PtrList = (List<UInt32>^)FormList->Tag;
			FormList->Items->Add(gcnew String(ThisForm->EditorID));
			PtrList->Add((UInt32)ThisForm->ParentForm);
		}

		NPCList->EndUpdate();
		GlobalList->EndUpdate();
		FactionList->EndUpdate();

		NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
	}

	bool RefBatchEditor::InitializeBatchEditor(BatchRefData* Data)
	{
		BatchData = Data;
		PopulateObjectList(Data);
		PopulateFormLists();
		if (BatchEditBox->ShowDialog() == DialogResult::OK)
			return true;
		else
			return false;
	}

	float RefBatchEditor::GetFloatFromString(String^ Text)
	{
		try
		{
			return float::Parse(Text);
		}
		catch (...)
		{
			throw gcnew CSEGeneralException("Invalid numeric data entered in one of the fields.");
			return 0;
		}
	}
	int RefBatchEditor::GetIntFromString(String^ Text)
	{
		try
		{
			return int::Parse(Text);
		}
		catch (...)
		{
			throw gcnew CSEGeneralException("Invalid numeric data entered in one of the fields.");
			return 0;
		}
	}
	UInt32 RefBatchEditor::GetUIntFromString(String^ Text)
	{
		try
		{
			return UInt32::Parse(Text);
		}
		catch (...)
		{
			throw gcnew CSEGeneralException("Invalid numeric data entered in one of the fields.");
			return 0;
		}
	}

	bool RefBatchEditor::ScrubData()
	{
		for each (ListViewItem^ Itr in ObjectList->Items)
		{
			CellObjectData* Tag = (CellObjectData*)((UInt32)Itr->Tag);

			if (Itr->Checked)			Tag->Selected = true;
			else						Tag->Selected = false;
		}

		// update flags
		ToggleFlag(&BatchData->World3DData.Flags, BatchRefData::_3DData::kFlag_UsePosX, UsePosX->Checked);
		ToggleFlag(&BatchData->World3DData.Flags, BatchRefData::_3DData::kFlag_UsePosY, UsePosY->Checked);
		ToggleFlag(&BatchData->World3DData.Flags, BatchRefData::_3DData::kFlag_UsePosZ, UsePosZ->Checked);
		ToggleFlag(&BatchData->World3DData.Flags, BatchRefData::_3DData::kFlag_UseRotX, UseRotX->Checked);
		ToggleFlag(&BatchData->World3DData.Flags, BatchRefData::_3DData::kFlag_UseRotY, UseRotY->Checked);
		ToggleFlag(&BatchData->World3DData.Flags, BatchRefData::_3DData::kFlag_UseRotZ, UseRotZ->Checked);
		ToggleFlag(&BatchData->World3DData.Flags, BatchRefData::_3DData::kFlag_UseScale, UseScale->Checked);

		ToggleFlag(&BatchData->Flags.Flags, BatchRefData::_Flags::kFlag_UsePersistent, UsePersistent->Checked);
		ToggleFlag(&BatchData->Flags.Flags, BatchRefData::_Flags::kFlag_UseDisabled, UseDisabled->Checked);
		ToggleFlag(&BatchData->Flags.Flags, BatchRefData::_Flags::kFlag_UseVWD, UseVWD->Checked);

		ToggleFlag(&BatchData->EnableParent.Flags, BatchRefData::_EnableParent::kFlag_UseEnableParent, UseEnableParent->Checked);

		ToggleFlag(&BatchData->Ownership.Flags, BatchRefData::_Ownership::kFlag_UseOwnership, UseOwnership->Checked);
		ToggleFlag(&BatchData->Ownership.Flags, BatchRefData::_Ownership::kFlag_UseNPCOwner, UseNPCOwner->Checked);

		ToggleFlag(&BatchData->Extra.Flags, BatchRefData::_Extra::kFlag_UseCharge, UseCharge->Checked);
		ToggleFlag(&BatchData->Extra.Flags, BatchRefData::_Extra::kFlag_UseCount, UseCount->Checked);
		ToggleFlag(&BatchData->Extra.Flags, BatchRefData::_Extra::kFlag_UseHealth, UseHealth->Checked);
		ToggleFlag(&BatchData->Extra.Flags, BatchRefData::_Extra::kFlag_UseTimeLeft, UseTimeLeft->Checked);
		ToggleFlag(&BatchData->Extra.Flags, BatchRefData::_Extra::kFlag_UseSoulLevel, UseSoulLevel->Checked);

		// fetch data
		try
		{
			if (BatchData->World3DData.UsePosX())			BatchData->World3DData.PosX = GetFloatFromString(PosX->Text);
			if (BatchData->World3DData.UsePosY())			BatchData->World3DData.PosY = GetFloatFromString(PosY->Text);
			if (BatchData->World3DData.UsePosZ())			BatchData->World3DData.PosZ = GetFloatFromString(PosZ->Text);

			if (BatchData->World3DData.UseRotX())			BatchData->World3DData.RotX = GetFloatFromString(RotX->Text);
			if (BatchData->World3DData.UseRotY())			BatchData->World3DData.RotY = GetFloatFromString(RotY->Text);
			if (BatchData->World3DData.UseRotZ())			BatchData->World3DData.RotZ = GetFloatFromString(RotZ->Text);

			if (BatchData->World3DData.UseScale())			BatchData->World3DData.Scale = GetFloatFromString(Scale->Text);

			BatchData->Flags.Persistent = FlagPersistent->Checked;
			BatchData->Flags.Disabled = FlagDisabled->Checked;
			BatchData->Flags.VWD = FlagVWD->Checked;

			BatchData->EnableParent.OppositeState = OppositeState->Checked;

			if (BatchData->Ownership.UseOwnership())
			{
				List<UInt32>^ PtrList = nullptr;

				if (BatchData->Ownership.UseNPCOwner())
				{
					PtrList = dynamic_cast<List<UInt32>^>(NPCList->Tag);
					if (NPCList->SelectedIndex == -1)
						BatchData->Ownership.Owner = 0;
					else
						BatchData->Ownership.Owner = (void*)PtrList[NPCList->SelectedIndex];

					PtrList = dynamic_cast<List<UInt32>^>(GlobalList->Tag);
					if (GlobalList->SelectedIndex == -1)
						BatchData->Ownership.Global = 0;
					else
						BatchData->Ownership.Global = (void*)PtrList[GlobalList->SelectedIndex];
				}
				else
				{
					PtrList = dynamic_cast<List<UInt32>^>(FactionList->Tag);
					if (FactionList->SelectedIndex == -1)
						BatchData->Ownership.Owner = 0;
					else
						BatchData->Ownership.Owner = (void*)PtrList[FactionList->SelectedIndex];

					BatchData->Ownership.Rank = GetIntFromString(Rank->Text);
					if (BatchData->Ownership.Rank < -1)
						BatchData->Ownership.Rank = -1;
				}
			}

			if (BatchData->Extra.UseCharge())		BatchData->Extra.Charge = GetIntFromString(Charge->Text);
			if (BatchData->Extra.UseCount())		BatchData->Extra.Count = GetIntFromString(Count->Text);
			if (BatchData->Extra.UseHealth())		BatchData->Extra.Health = GetFloatFromString(Health->Text);
			if (BatchData->Extra.UseTimeLeft())		BatchData->Extra.TimeLeft = GetFloatFromString(TimeLeft->Text);
			if (BatchData->Extra.UseSoulLevel())
			{
				if (SoulLevel->SelectedIndex == -1)
					BatchData->Extra.SoulLevel = BatchData->Extra.kSoul_None;
				else
					BatchData->Extra.SoulLevel = SoulLevel->SelectedIndex;
			}
		}
		catch (Exception^ E)
		{
			MessageBox::Show(E->Message, "Batch Editor", MessageBoxButtons::OK, MessageBoxIcon::Error);
			return false;
		}

		return true;
	}

	void RefBatchEditor::ObjectList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
	{
		if (E->Column != (int)ObjectList->Tag)
		{
			ObjectList->Tag = E->Column;
			ObjectList->Sorting = SortOrder::Descending;
		}
		else
		{
			if (ObjectList->Sorting == SortOrder::Ascending)
				ObjectList->Sorting = SortOrder::Descending;
			else
				ObjectList->Sorting = SortOrder::Ascending;
		}

		ObjectList->Sort();
		System::Collections::IComparer^ Sorter;
		switch (E->Column)
		{
		case 1:							// FormID
			Sorter = gcnew ListViewIntSorter(E->Column, ObjectList->Sorting, true);
			break;
		default:
			Sorter = gcnew ListViewStringSorter(E->Column, ObjectList->Sorting);
			break;
		}
		ObjectList->ListViewItemSorter = Sorter;
	}

	void RefBatchEditor::SetParent_Click(Object^ Sender, EventArgs^ E)
	{
		ComponentDLLInterface::FormData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.ShowPickReferenceDialog((HWND)BatchEditBox->Handle);
		BatchData->EnableParent.Parent = 0;

		if (Data)
		{
			BatchData->EnableParent.Parent = Data->ParentForm;
			if (Data->EditorID)
				SetParent->Text = "Set Parent to " + gcnew String(Data->EditorID);
			else
				SetParent->Text = "Set Parent to " + Data->FormID.ToString("X8");
		}
		else
			SetParent->Text = "Set Parent to NONE";

		NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);
	}
}