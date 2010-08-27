#include "BatchEditor.h"
#include "Common\ListViewUtilities.h"
#include "Common\MiscUtilities.h"


RefBatchEditor^% RefBatchEditor::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew RefBatchEditor();
	}
	return Singleton;
}

RefBatchEditor::RefBatchEditor()
{
	BatchEditBox = gcnew Form();
	this->ObjectList = (gcnew ListView());
	this->ObjectList_CHEditorID = (gcnew ColumnHeader());
	this->ObjectList_CHFormID = (gcnew ColumnHeader());
	this->ObjectList_CHType = (gcnew ColumnHeader());
	this->ApplyButton = (gcnew Button());
	this->CancelButton = (gcnew Button());
	this->DataBox = (gcnew TabControl());
	this->DataBox_TB3DData = (gcnew TabPage());
	this->RotationBox = (gcnew GroupBox());
	this->PositionBox = (gcnew GroupBox());
	this->DataBox_TBFlags = (gcnew TabPage());
	this->DataBox_TBEnableParent = (gcnew TabPage());
	this->DataBox_TBOwnership = (gcnew TabPage());
	this->PosX = (gcnew TextBox());
	this->PosY = (gcnew TextBox());
	this->PosZ = (gcnew TextBox());
	this->LabelX = (gcnew Label());
	this->LabelY = (gcnew Label());
	this->LabelZ = (gcnew Label());
	this->LabelZEx = (gcnew Label());
	this->LabelYEx = (gcnew Label());
	this->LabelXEx = (gcnew Label());
	this->RotZ = (gcnew TextBox());
	this->RotY = (gcnew TextBox());
	this->RotX = (gcnew TextBox());
	this->LabelScale = (gcnew Label());
	this->Scale = (gcnew TextBox());
	this->Use3DData = (gcnew CheckBox());
	this->FlagPersistent = (gcnew CheckBox());
	this->FlagDisabled = (gcnew CheckBox());
	this->FlagVWD = (gcnew CheckBox());
	this->UseFlags = (gcnew CheckBox());
	this->DataBox->SuspendLayout();
	this->DataBox_TB3DData->SuspendLayout();
	this->RotationBox->SuspendLayout();
	this->PositionBox->SuspendLayout();
	this->DataBox_TBFlags->SuspendLayout();
	BatchEditBox->SuspendLayout();
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
	this->ObjectList->Size = System::Drawing::Size(306, 419);
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
//	ApplyButton->DialogResult = DialogResult::OK;
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
//	CancelButton->DialogResult = DialogResult::Cancel;
	CancelButton->Click += gcnew EventHandler(this, &RefBatchEditor::CancelButton_Click);
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
	this->DataBox_TB3DData->Padding = Padding(3);
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
	this->DataBox_TBFlags->Padding = Padding(3);
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
	BatchEditBox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	BatchEditBox->AutoScaleMode = AutoScaleMode::Font;
	BatchEditBox->ClientSize = System::Drawing::Size(686, 482);
	BatchEditBox->ControlBox = false;
	BatchEditBox->Controls->Add(this->DataBox);
	BatchEditBox->Controls->Add(this->CancelButton);
	BatchEditBox->Controls->Add(this->ApplyButton);
	BatchEditBox->Controls->Add(this->ObjectList);
	BatchEditBox->Name = L"BatchEditor_Reference";
	BatchEditBox->Text = L"Batch Editor - References";
	BatchEditBox->StartPosition = FormStartPosition::CenterScreen;
	BatchEditBox->FormBorderStyle = FormBorderStyle::FixedDialog;
	DataBox->ResumeLayout(false);
	DataBox_TB3DData->ResumeLayout(false);
	DataBox_TB3DData->PerformLayout();
	RotationBox->ResumeLayout(false);
	RotationBox->PerformLayout();
	PositionBox->ResumeLayout(false);
	PositionBox->PerformLayout();
	DataBox_TBFlags->ResumeLayout(false);
	DataBox_TBFlags->PerformLayout();
	BatchEditBox->ResumeLayout(false);	

	BatchEditBox->Hide();
}

void RefBatchEditor::Close(DialogResult Result)
{
	BatchEditBox->DialogResult = Result;
	Cleanup();
	BatchEditBox->Hide();
}

void RefBatchEditor::Cleanup()
{
	ObjectList->Items->Clear();
}

void RefBatchEditor::PopulateObjectList(BatchRefData* Data)
{
	ObjectList->BeginUpdate();

	for (CellObjectData* Itr = Data->CellObjectListHead; Itr != Data->CellObjectListHead + Data->ObjectCount; ++Itr) {
		if (!Itr->IsValid())		continue;

		ListViewItem^ NewItem = gcnew ListViewItem(gcnew String(Itr->EditorID));
		NewItem->SubItems->Add(Itr->FormID.ToString("X8"));
		NewItem->SubItems->Add(Itr->TypeID.ToString("X2"));
		NewItem->Tag = (UInt32)Itr;

		if (Itr->Selected)
			NewItem->Checked = true;

		ObjectList->Items->Add(NewItem);
	}

	ObjectList->EndUpdate();
}

bool RefBatchEditor::InitializeBatchEditor(BatchRefData* Data)
{
	BatchData = Data;
	PopulateObjectList(Data);
	if (BatchEditBox->ShowDialog() == DialogResult::OK)
		return true;
	else
		return false;
}

bool RefBatchEditor::ScrubData(BatchRefData* Data)
{
	for each (ListViewItem^ Itr in ObjectList->Items) {
		CellObjectData* Tag = (CellObjectData*)((UInt32)Itr->Tag);

		if (Itr->Checked)		Tag->Selected = true;
		else					Tag->Selected = false;
	}

	Data->Use3DData = Use3DData->Checked;
	Data->UseFlags = UseFlags->Checked;

	if (Data->Use3DData) {
		float Buffer = 0;
		try {
			Buffer = float::Parse(PosX->Text);
			Data->World3DData.PosX = Buffer;
			Buffer = float::Parse(PosY->Text);
			Data->World3DData.PosY = Buffer;
			Buffer = float::Parse(PosZ->Text);
			Data->World3DData.PosZ = Buffer;

			Buffer = float::Parse(RotX->Text);
			Data->World3DData.RotX = Buffer;
			Buffer = float::Parse(RotY->Text);
			Data->World3DData.RotY = Buffer;
			Buffer = float::Parse(RotZ->Text);
			Data->World3DData.RotZ = Buffer;

			Buffer = float::Parse(Scale->Text);
			Data->World3DData.Scale = Buffer;
		} catch (...) {
			MessageBox::Show("Invalid numeric data entered in one of the fields! Please correct before committing changes", "Batch Editor", MessageBoxButtons::OK);
			return false;
		}
	}

	Data->Flags.Persistent = FlagPersistent->Checked;
	Data->Flags.Disabled = FlagDisabled->Checked;
	Data->Flags.VWD = FlagVWD->Checked;

	return true;
}

void RefBatchEditor::ApplyButton_Click(Object^ Sender, EventArgs^ E)
{
	if (ScrubData(BatchData))
		Close(DialogResult::OK);
}
void RefBatchEditor::CancelButton_Click(Object^ Sender, EventArgs^ E)
{
	Close(DialogResult::Cancel);
}

void RefBatchEditor::ObjectList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
{
	if (E->Column != (int)ObjectList->Tag) {
		ObjectList->Tag = E->Column;
		ObjectList->Sorting = SortOrder::Descending;
	} else {
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
		Sorter = gcnew CSEListViewIntSorter(E->Column, ObjectList->Sorting, true);
		break;
	default:
		Sorter = gcnew CSEListViewStringSorter(E->Column, ObjectList->Sorting);
		break;
	}
	ObjectList->ListViewItemSorter = Sorter;
}



