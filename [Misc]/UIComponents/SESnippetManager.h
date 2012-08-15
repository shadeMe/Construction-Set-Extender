#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for SESnippetManager
	/// </summary>
	public ref class SESnippetManager : public System::Windows::Forms::Form
	{
	public:
		SESnippetManager(void)
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
		~SESnippetManager()
		{
			if (components)
			{
				delete components;
			}
		}

	protected: 

	private: System::Windows::Forms::GroupBox^  GroupSnippetData;
	private: System::Windows::Forms::Button^  ButtonApply;

	private: System::Windows::Forms::TextBox^  SnippetCodeBox;
	private: System::Windows::Forms::TextBox^  SnippetDescBox;
	private: System::Windows::Forms::TextBox^  SnippetShorthandBox;
	private: System::Windows::Forms::TextBox^  SnippetNameBox;
	private: System::Windows::Forms::Label^  LabelCode;
	private: System::Windows::Forms::Label^  LabelVariables;
	private: System::Windows::Forms::Label^  LabelDescription;
	private: System::Windows::Forms::Label^  LabelShorthand;
	private: System::Windows::Forms::Label^  LabelName;
	private: System::Windows::Forms::ListView^  SnippetVarList;
	private: System::Windows::Forms::ColumnHeader^  SnippetVarListCHName;
	private: System::Windows::Forms::ColumnHeader^  SnippetVarListCHType;
	private: System::Windows::Forms::ContextMenuStrip^  SnippetListContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  SnippetListCMAddSnippet;
	private: System::Windows::Forms::ToolStripMenuItem^  SnippetListCMRemoveSnippet;





	private: System::Windows::Forms::ContextMenuStrip^  VarListContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  VarListCMAddVariable;
	private: System::Windows::Forms::ToolStripMenuItem^  VarListCMRemoveVariable;
	private: System::Windows::Forms::ListView^  SnippetList;
	private: System::Windows::Forms::ColumnHeader^  SnippetListCHName;
	private: System::Windows::Forms::ColumnHeader^  SnippetListCHShorthand;
	private: System::Windows::Forms::ToolStripMenuItem^  VarListCMAddVarInt;
	private: System::Windows::Forms::ToolStripMenuItem^  VarListCMAddVarFloat;
	private: System::Windows::Forms::ToolStripMenuItem^  VarListCMAddVarRef;
	private: System::Windows::Forms::ToolStripMenuItem^  VarListCMAddVarString;
	private: System::Windows::Forms::ToolStripMenuItem^  VarListCMAddVarArray;







	private: System::ComponentModel::IContainer^  components;
	protected: 

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->SnippetListContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->SnippetListCMAddSnippet = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->SnippetListCMRemoveSnippet = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->GroupSnippetData = (gcnew System::Windows::Forms::GroupBox());
			this->SnippetVarList = (gcnew System::Windows::Forms::ListView());
			this->SnippetVarListCHName = (gcnew System::Windows::Forms::ColumnHeader());
			this->SnippetVarListCHType = (gcnew System::Windows::Forms::ColumnHeader());
			this->VarListContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->VarListCMAddVariable = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->VarListCMRemoveVariable = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ButtonApply = (gcnew System::Windows::Forms::Button());
			this->SnippetCodeBox = (gcnew System::Windows::Forms::TextBox());
			this->SnippetDescBox = (gcnew System::Windows::Forms::TextBox());
			this->SnippetShorthandBox = (gcnew System::Windows::Forms::TextBox());
			this->SnippetNameBox = (gcnew System::Windows::Forms::TextBox());
			this->LabelCode = (gcnew System::Windows::Forms::Label());
			this->LabelVariables = (gcnew System::Windows::Forms::Label());
			this->LabelDescription = (gcnew System::Windows::Forms::Label());
			this->LabelShorthand = (gcnew System::Windows::Forms::Label());
			this->LabelName = (gcnew System::Windows::Forms::Label());
			this->SnippetList = (gcnew System::Windows::Forms::ListView());
			this->SnippetListCHName = (gcnew System::Windows::Forms::ColumnHeader());
			this->SnippetListCHShorthand = (gcnew System::Windows::Forms::ColumnHeader());
			this->VarListCMAddVarInt = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->VarListCMAddVarFloat = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->VarListCMAddVarRef = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->VarListCMAddVarString = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->VarListCMAddVarArray = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->SnippetListContextMenu->SuspendLayout();
			this->GroupSnippetData->SuspendLayout();
			this->VarListContextMenu->SuspendLayout();
			this->SuspendLayout();
			// 
			// SnippetListContextMenu
			// 
			this->SnippetListContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->SnippetListCMAddSnippet, 
				this->SnippetListCMRemoveSnippet});
			this->SnippetListContextMenu->Name = L"SnippetTreeContextMenu";
			this->SnippetListContextMenu->ShowImageMargin = false;
			this->SnippetListContextMenu->Size = System::Drawing::Size(133, 48);
			// 
			// SnippetListCMAddSnippet
			// 
			this->SnippetListCMAddSnippet->Name = L"SnippetListCMAddSnippet";
			this->SnippetListCMAddSnippet->Size = System::Drawing::Size(132, 22);
			this->SnippetListCMAddSnippet->Text = L"Add Snippet";
			// 
			// SnippetListCMRemoveSnippet
			// 
			this->SnippetListCMRemoveSnippet->Name = L"SnippetListCMRemoveSnippet";
			this->SnippetListCMRemoveSnippet->Size = System::Drawing::Size(132, 22);
			this->SnippetListCMRemoveSnippet->Text = L"Remove Snippet";
			// 
			// GroupSnippetData
			// 
			this->GroupSnippetData->Controls->Add(this->SnippetVarList);
			this->GroupSnippetData->Controls->Add(this->ButtonApply);
			this->GroupSnippetData->Controls->Add(this->SnippetCodeBox);
			this->GroupSnippetData->Controls->Add(this->SnippetDescBox);
			this->GroupSnippetData->Controls->Add(this->SnippetShorthandBox);
			this->GroupSnippetData->Controls->Add(this->SnippetNameBox);
			this->GroupSnippetData->Controls->Add(this->LabelCode);
			this->GroupSnippetData->Controls->Add(this->LabelVariables);
			this->GroupSnippetData->Controls->Add(this->LabelDescription);
			this->GroupSnippetData->Controls->Add(this->LabelShorthand);
			this->GroupSnippetData->Controls->Add(this->LabelName);
			this->GroupSnippetData->Location = System::Drawing::Point(304, 12);
			this->GroupSnippetData->Name = L"GroupSnippetData";
			this->GroupSnippetData->Size = System::Drawing::Size(477, 501);
			this->GroupSnippetData->TabIndex = 1;
			this->GroupSnippetData->TabStop = false;
			this->GroupSnippetData->Text = L"Snippet Data";
			// 
			// SnippetVarList
			// 
			this->SnippetVarList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->SnippetVarListCHName, 
				this->SnippetVarListCHType});
			this->SnippetVarList->ContextMenuStrip = this->VarListContextMenu;
			this->SnippetVarList->FullRowSelect = true;
			this->SnippetVarList->Location = System::Drawing::Point(12, 136);
			this->SnippetVarList->Name = L"SnippetVarList";
			this->SnippetVarList->Size = System::Drawing::Size(459, 113);
			this->SnippetVarList->TabIndex = 10;
			this->SnippetVarList->UseCompatibleStateImageBehavior = false;
			this->SnippetVarList->View = System::Windows::Forms::View::Details;
			// 
			// SnippetVarListCHName
			// 
			this->SnippetVarListCHName->Text = L"Name";
			this->SnippetVarListCHName->Width = 323;
			// 
			// SnippetVarListCHType
			// 
			this->SnippetVarListCHType->Text = L"Data Type";
			this->SnippetVarListCHType->Width = 106;
			// 
			// VarListContextMenu
			// 
			this->VarListContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->VarListCMAddVariable, 
				this->VarListCMRemoveVariable});
			this->VarListContextMenu->Name = L"VarListContextMenu";
			this->VarListContextMenu->ShowImageMargin = false;
			this->VarListContextMenu->Size = System::Drawing::Size(135, 48);
			// 
			// VarListCMAddVariable
			// 
			this->VarListCMAddVariable->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(5) {this->VarListCMAddVarInt, 
				this->VarListCMAddVarFloat, this->VarListCMAddVarRef, this->VarListCMAddVarString, this->VarListCMAddVarArray});
			this->VarListCMAddVariable->Name = L"VarListCMAddVariable";
			this->VarListCMAddVariable->Size = System::Drawing::Size(134, 22);
			this->VarListCMAddVariable->Text = L"Add Variable...";
			// 
			// VarListCMRemoveVariable
			// 
			this->VarListCMRemoveVariable->Name = L"VarListCMRemoveVariable";
			this->VarListCMRemoveVariable->Size = System::Drawing::Size(134, 22);
			this->VarListCMRemoveVariable->Text = L"Remove Variable";
			// 
			// ButtonApply
			// 
			this->ButtonApply->Location = System::Drawing::Point(377, 470);
			this->ButtonApply->Name = L"ButtonApply";
			this->ButtonApply->Size = System::Drawing::Size(94, 25);
			this->ButtonApply->TabIndex = 9;
			this->ButtonApply->Text = L"Apply Changes";
			this->ButtonApply->UseVisualStyleBackColor = true;
			// 
			// SnippetCodeBox
			// 
			this->SnippetCodeBox->AcceptsReturn = true;
			this->SnippetCodeBox->AcceptsTab = true;
			this->SnippetCodeBox->Location = System::Drawing::Point(12, 281);
			this->SnippetCodeBox->MaxLength = 150994943;
			this->SnippetCodeBox->Multiline = true;
			this->SnippetCodeBox->Name = L"SnippetCodeBox";
			this->SnippetCodeBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->SnippetCodeBox->Size = System::Drawing::Size(459, 183);
			this->SnippetCodeBox->TabIndex = 8;
			this->SnippetCodeBox->WordWrap = false;
			// 
			// SnippetDescBox
			// 
			this->SnippetDescBox->Location = System::Drawing::Point(72, 83);
			this->SnippetDescBox->Name = L"SnippetDescBox";
			this->SnippetDescBox->Size = System::Drawing::Size(399, 20);
			this->SnippetDescBox->TabIndex = 7;
			// 
			// SnippetShorthandBox
			// 
			this->SnippetShorthandBox->Location = System::Drawing::Point(72, 54);
			this->SnippetShorthandBox->Name = L"SnippetShorthandBox";
			this->SnippetShorthandBox->Size = System::Drawing::Size(399, 20);
			this->SnippetShorthandBox->TabIndex = 6;
			// 
			// SnippetNameBox
			// 
			this->SnippetNameBox->Location = System::Drawing::Point(72, 27);
			this->SnippetNameBox->Name = L"SnippetNameBox";
			this->SnippetNameBox->Size = System::Drawing::Size(399, 20);
			this->SnippetNameBox->TabIndex = 5;
			// 
			// LabelCode
			// 
			this->LabelCode->AutoSize = true;
			this->LabelCode->Location = System::Drawing::Point(9, 265);
			this->LabelCode->Name = L"LabelCode";
			this->LabelCode->Size = System::Drawing::Size(32, 13);
			this->LabelCode->TabIndex = 4;
			this->LabelCode->Text = L"Code";
			// 
			// LabelVariables
			// 
			this->LabelVariables->AutoSize = true;
			this->LabelVariables->Location = System::Drawing::Point(9, 120);
			this->LabelVariables->Name = L"LabelVariables";
			this->LabelVariables->Size = System::Drawing::Size(50, 13);
			this->LabelVariables->TabIndex = 3;
			this->LabelVariables->Text = L"Variables";
			// 
			// LabelDescription
			// 
			this->LabelDescription->AutoSize = true;
			this->LabelDescription->Location = System::Drawing::Point(9, 86);
			this->LabelDescription->Name = L"LabelDescription";
			this->LabelDescription->Size = System::Drawing::Size(60, 13);
			this->LabelDescription->TabIndex = 2;
			this->LabelDescription->Text = L"Description";
			// 
			// LabelShorthand
			// 
			this->LabelShorthand->AutoSize = true;
			this->LabelShorthand->Location = System::Drawing::Point(9, 57);
			this->LabelShorthand->Name = L"LabelShorthand";
			this->LabelShorthand->Size = System::Drawing::Size(56, 13);
			this->LabelShorthand->TabIndex = 1;
			this->LabelShorthand->Text = L"Shorthand";
			// 
			// LabelName
			// 
			this->LabelName->AutoSize = true;
			this->LabelName->Location = System::Drawing::Point(9, 30);
			this->LabelName->Name = L"LabelName";
			this->LabelName->Size = System::Drawing::Size(35, 13);
			this->LabelName->TabIndex = 0;
			this->LabelName->Text = L"Name";
			// 
			// SnippetList
			// 
			this->SnippetList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->SnippetListCHName, 
				this->SnippetListCHShorthand});
			this->SnippetList->ContextMenuStrip = this->SnippetListContextMenu;
			this->SnippetList->FullRowSelect = true;
			this->SnippetList->Location = System::Drawing::Point(12, 12);
			this->SnippetList->MultiSelect = false;
			this->SnippetList->Name = L"SnippetList";
			this->SnippetList->Size = System::Drawing::Size(286, 501);
			this->SnippetList->TabIndex = 2;
			this->SnippetList->UseCompatibleStateImageBehavior = false;
			this->SnippetList->View = System::Windows::Forms::View::Details;
			// 
			// SnippetListCHName
			// 
			this->SnippetListCHName->Text = L"Name";
			this->SnippetListCHName->Width = 166;
			// 
			// SnippetListCHShorthand
			// 
			this->SnippetListCHShorthand->Text = L"Shorthand";
			this->SnippetListCHShorthand->Width = 114;
			// 
			// VarListCMAddVarInt
			// 
			this->VarListCMAddVarInt->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarInt->Name = L"VarListCMAddVarInt";
			this->VarListCMAddVarInt->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarInt->Text = L"Integer";
			this->VarListCMAddVarInt->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			// 
			// VarListCMAddVarFloat
			// 
			this->VarListCMAddVarFloat->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarFloat->Name = L"VarListCMAddVarFloat";
			this->VarListCMAddVarFloat->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarFloat->Text = L"Float";
			this->VarListCMAddVarFloat->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			// 
			// VarListCMAddVarRef
			// 
			this->VarListCMAddVarRef->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarRef->Name = L"VarListCMAddVarRef";
			this->VarListCMAddVarRef->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarRef->Text = L"Reference";
			this->VarListCMAddVarRef->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			// 
			// VarListCMAddVarString
			// 
			this->VarListCMAddVarString->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarString->Name = L"VarListCMAddVarString";
			this->VarListCMAddVarString->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarString->Text = L"String";
			this->VarListCMAddVarString->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			// 
			// VarListCMAddVarArray
			// 
			this->VarListCMAddVarArray->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarArray->Name = L"VarListCMAddVarArray";
			this->VarListCMAddVarArray->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarArray->Text = L"Array";
			this->VarListCMAddVarArray->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			// 
			// SESnippetManager
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(793, 525);
			this->Controls->Add(this->GroupSnippetData);
			this->Controls->Add(this->SnippetList);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SESnippetManager";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Code Snippet Manager";
			this->SnippetListContextMenu->ResumeLayout(false);
			this->GroupSnippetData->ResumeLayout(false);
			this->GroupSnippetData->PerformLayout();
			this->VarListContextMenu->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
