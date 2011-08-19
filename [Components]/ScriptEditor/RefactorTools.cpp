#include "[Common]\CustomInputBox.h"
#include "[Common]\ListViewUtilities.h"
#include "RefactorTools.h"
#include "ScriptEditor.h"
#include "ScriptEditorManager.h"

namespace Refactoring
{
	CreateUDFImplementationDialog::CreateUDFImplementationDialog(IntPtr ParentHandle)
	{
		InitializeComponent();

		HasResult = false;

		this->Hide();
		this->ShowDialog();
	}

	void CreateUDFImplementationDialog::InitializeComponent()
	{
			ListViewItem^  listViewItem1 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"Parameter Name (Double click to edit)",
				L"--"}, -1));
			ListViewItem^  listViewItem2 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem3 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem4 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem5 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem6 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem7 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem8 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem9 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			ListViewItem^  listViewItem10 = (gcnew ListViewItem(gcnew cli::array< System::String^  >(2) {L"",
				L"--"}, -1));
			this->ParameterBox = (gcnew ListView());
			this->ImplementBox = (gcnew Button());
			this->ParameterBoxCHName = (gcnew ColumnHeader());
			this->ParameterBoxCHType = (gcnew ColumnHeader());
			this->SuspendLayout();
			//
			// ParameterBox
			//
			this->ParameterBox->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(2) {this->ParameterBoxCHName,
				this->ParameterBoxCHType});
			this->ParameterBox->Items->AddRange(gcnew cli::array< ListViewItem^  >(10) {listViewItem1, listViewItem2,
				listViewItem3, listViewItem4, listViewItem5, listViewItem6, listViewItem7, listViewItem8, listViewItem9, listViewItem10});
			this->ParameterBox->Location = System::Drawing::Point(12, 12);
			this->ParameterBox->Name = L"ParameterBox";
			this->ParameterBox->Size = System::Drawing::Size(345, 200);
			this->ParameterBox->TabIndex = 0;
			this->ParameterBox->UseCompatibleStateImageBehavior = false;
			this->ParameterBox->View = View::Details;
			this->ParameterBox->FullRowSelect = true;
			ParameterBox->HideSelection = false;
			this->ParameterBox->ItemActivate += gcnew EventHandler(this, &CreateUDFImplementationDialog::ParameterBox_ItemActivate);
			//
			// ImplementBox
			//
			this->ImplementBox->Location = System::Drawing::Point(137, 218);
			this->ImplementBox->Name = L"ImplementBox";
			this->ImplementBox->Size = System::Drawing::Size(95, 23);
			this->ImplementBox->TabIndex = 1;
			this->ImplementBox->Text = L"Implement";
			this->ImplementBox->UseVisualStyleBackColor = true;
			this->ImplementBox->Click += gcnew EventHandler(this, &CreateUDFImplementationDialog::ImplementBox_Click);
			//
			// ParameterBoxCHName
			//
			this->ParameterBoxCHName->Text = L"Parameter Name";
			this->ParameterBoxCHName->Width = 241;
			//
			// ParameterBoxCHType
			//
			this->ParameterBoxCHType->Text = L"Type";
			this->ParameterBoxCHType->Width = 100;
			//
			// SERefactorUDF
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(369, 248);
			this->Controls->Add(this->ImplementBox);
			this->Controls->Add(this->ParameterBox);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
			this->KeyPreview = true;
			this->Name = L"SERefactorUDF";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Create UDF Implementation";
			this->ResumeLayout(false);
	}

	void CreateUDFImplementationDialog::ImplementBox_Click( Object^ Sender, EventArgs^ E )
	{
		HasResult = true;
		ResultData = gcnew CreateUDFImplementationData();

		for each (ListViewItem^ Itr in ParameterBox->Items)
		{
			if (Itr->Text != "" && Itr->SubItems[1]->Text != "--")
				ResultData->ParameterList->Add(gcnew CreateUDFImplementationData::ParameterData(Itr->Text, Itr->SubItems[1]->Text));
		}

		this->Close();
	}

	void CreateUDFImplementationDialog::ParameterBox_ItemActivate( Object^ Sender, EventArgs^ E )
	{
		ListViewItem^ Selection = GetListViewSelectedItem(ParameterBox);

		if (Selection != nullptr)
		{
			InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter Parameter Name", "Create UDF Implementation - CSE Script Editor");
			if (Result->ReturnCode == System::Windows::Forms::DialogResult::Cancel || Result->Text == "")
				return;
			else
				Selection->Text = Result->Text;

			Result = InputBoxes::InputBox::Show("Enter Parameter Type", "Create UDF Implementation - CSE Script Editor");
			if (Result->ReturnCode == System::Windows::Forms::DialogResult::Cancel || Result->Text == "")
				return;
			else
				Selection->SubItems[1]->Text = Result->Text;
		}
	}

	bool EditScriptComponentData::LookupEditDataByName( String^ ElementName, String^% OutEditData )
	{
		bool Result = false;

		for each (ScriptComponent^ Itr in ScriptComponentList)
		{
			if (!String::Compare(Itr->ElementName, ElementName, true))
			{
				Result = true;
				OutEditData = Itr->EditData;
			}
		}

		return Result;
	}

	EditScriptComponentDialog::EditScriptComponentDialog(IntPtr ParentHandle, String^ ScriptEditorID, EditScriptComponentDialog::OperationType Operation, String^ DefaultItemString)
	{
		this->DefaultItemString = DefaultItemString;
		InitializeComponent();

		HasResult = false;

		if (DefaultItemString != "")
		{
			ListViewItem^ Item = gcnew ListViewItem(DefaultItemString);
			Item->SubItems->Add("Double click to edit");
			ElementList->Items->Add(Item);
		}

		switch (Operation)
		{
		case OperationType::e_DocumentScript:
			{
				ElementListCHElementName->Text = "Documentable Element";
				ElementListCHEditData->Text = "Comment";
				this->Text = "Document Script";
				break;
			}
		case OperationType::e_RenameVariables:
			{
				ElementListCHElementName->Text = "Old Identifier";
				ElementListCHEditData->Text = "New Identifier";
				this->Text = "Rename Variables";
				break;
			}
		}

		ComponentDLLInterface::ScriptVarListData* Data = g_CSEInterface->ScriptEditor.GetScriptVarList((CString(ScriptEditorID)).c_str());
		if (Data)
		{
			for (int i = 0; i < Data->ScriptVarListCount; i++)
			{
				ComponentDLLInterface::ScriptVarListData::ScriptVarInfo* VarInfo = &Data->ScriptVarListHead[i];

				ListViewItem^ Item = gcnew ListViewItem(gcnew String(VarInfo->Name));
				Item->SubItems->Add("");
				ElementList->Items->Add(Item);
			}
		}
		g_CSEInterface->DeleteNativeHeapPointer(Data, false);

		this->Hide();
		this->ShowDialog();
	}

	void EditScriptComponentDialog::InitializeComponent()
	{
		this->ElementList = (gcnew System::Windows::Forms::ListView());
		this->ElementListCHElementName = (gcnew System::Windows::Forms::ColumnHeader());
		this->ElementListCHEditData = (gcnew System::Windows::Forms::ColumnHeader());
		this->ImplementBox = (gcnew System::Windows::Forms::Button());
		ElementListEditBox = gcnew TextBox();
		this->SuspendLayout();
		//
		// DocumentationList
		//
		this->ElementList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->ElementListCHElementName,
			this->ElementListCHEditData});
		this->ElementList->Location = System::Drawing::Point(13, 12);
		this->ElementList->Size = System::Drawing::Size(484, 314);
		this->ElementList->TabIndex = 0;
		this->ElementList->UseCompatibleStateImageBehavior = false;
		this->ElementList->View = System::Windows::Forms::View::Details;
		ElementList->FullRowSelect = true;
		this->ElementList->ItemActivate += gcnew EventHandler(this, &EditScriptComponentDialog::ElementList_ItemActivate);

		ElementListEditBox->Multiline = true;
		ElementListEditBox->BorderStyle = BorderStyle::FixedSingle;
		ElementListEditBox->Visible = false;
		ElementListEditBox->AcceptsReturn = true;
		ElementListEditBox->LostFocus += gcnew EventHandler(this, &EditScriptComponentDialog::ElementListEditBox_LostFocus);

		ElementList->Controls->Add(ElementListEditBox);

		//
		// DocumentationListCHDocumentableElement
		//
		this->ElementListCHElementName->Text = L"Element Identifier";
		this->ElementListCHElementName->Width = 151;
		//
		// DocumentationListCHComment
		//
		this->ElementListCHEditData->Text = L"Edit Data";
		this->ElementListCHEditData->Width = 300;
		//
		// ImplementBox
		//
		this->ImplementBox->Location = System::Drawing::Point(199, 332);
		this->ImplementBox->Name = L"ImplementBox";
		this->ImplementBox->Size = System::Drawing::Size(113, 25);
		this->ImplementBox->TabIndex = 1;
		this->ImplementBox->Text = L"Implement";
		this->ImplementBox->UseVisualStyleBackColor = true;
		this->ImplementBox->Click += gcnew EventHandler(this, &EditScriptComponentDialog::ImplementBox_Click);
		//
		// SERefactorDocumentScript
		//
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->ClientSize = System::Drawing::Size(510, 369);
		this->Controls->Add(this->ImplementBox);
		this->Controls->Add(this->ElementList);
		this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
		this->Name = L"SERefactorDocumentScript";
		this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
		this->Text = L"Edit Script Component";
		this->ResumeLayout(false);
	}

	void EditScriptComponentDialog::ImplementBox_Click( Object^ Sender, EventArgs^ E )
	{
		HasResult = true;
		ResultData = gcnew EditScriptComponentData();

		for each (ListViewItem^ Itr in ElementList->Items)
		{
			ResultData->ScriptComponentList->Add(gcnew EditScriptComponentData::ScriptComponent(Itr->Text, Itr->SubItems[1]->Text));
		}

		this->Close();
	}

	void EditScriptComponentDialog::ElementList_ItemActivate( Object^ Sender, EventArgs^ E )
	{
		ListViewItem^ Selection = GetListViewSelectedItem(ElementList);

		if (Selection != nullptr)
		{
			Rectangle Bounds = Selection->SubItems[1]->Bounds;
			if (Bounds.Width > 35)
			{
				if (Selection->Text == DefaultItemString)
					ElementListEditBox->SetBounds(Bounds.X, Bounds.Y, Bounds.Width, ElementList->Height - Bounds.Y - 20, BoundsSpecified::All);
				else
					ElementListEditBox->SetBounds(Bounds.X, Bounds.Y, Bounds.Width, Bounds.Height, BoundsSpecified::All);

				ElementListEditBox->Text = Selection->SubItems[1]->Text;
				ElementListEditBox->Show();
				ElementListEditBox->BringToFront();
				ElementListEditBox->Focus();
			}
			else
			{
				MessageBox::Show("Please expand the second column sufficiently to allow the editing of its contents", "CSE Script Editor",
								MessageBoxButtons::OK, MessageBoxIcon::Information);
			}
		}
	}

	void EditScriptComponentDialog::ElementListEditBox_LostFocus( Object^ Sender, EventArgs^ E )
	{
		String^ Comment = ElementListEditBox->Text;

		ElementListEditBox->Text = "";
		ElementListEditBox->Hide();

		if (GetListViewSelectedItem(ElementList) != nullptr)
		{
			ListViewItem^ Item = GetListViewSelectedItem(ElementList);
			Item->SubItems[1]->Text = Comment;
		}
	}
}