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
	private: System::Windows::Forms::TreeView^  SnippetTree;
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
	private: System::Windows::Forms::ContextMenuStrip^  SnippetTreeContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  addSnippetToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  removeSnippetToolStripMenuItem;
	private: System::Windows::Forms::ContextMenuStrip^  VarListContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  addVariableToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  removeVariableToolStripMenuItem;
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
			this->SnippetTree = (gcnew System::Windows::Forms::TreeView());
			this->GroupSnippetData = (gcnew System::Windows::Forms::GroupBox());
			this->LabelName = (gcnew System::Windows::Forms::Label());
			this->LabelShorthand = (gcnew System::Windows::Forms::Label());
			this->LabelDescription = (gcnew System::Windows::Forms::Label());
			this->LabelVariables = (gcnew System::Windows::Forms::Label());
			this->LabelCode = (gcnew System::Windows::Forms::Label());
			this->SnippetNameBox = (gcnew System::Windows::Forms::TextBox());
			this->SnippetShorthandBox = (gcnew System::Windows::Forms::TextBox());
			this->SnippetDescBox = (gcnew System::Windows::Forms::TextBox());
			this->SnippetCodeBox = (gcnew System::Windows::Forms::TextBox());
			this->ButtonApply = (gcnew System::Windows::Forms::Button());
			this->SnippetVarList = (gcnew System::Windows::Forms::ListView());
			this->SnippetVarListCHName = (gcnew System::Windows::Forms::ColumnHeader());
			this->SnippetVarListCHType = (gcnew System::Windows::Forms::ColumnHeader());
			this->VarListContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->addVariableToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->removeVariableToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->SnippetTreeContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->addSnippetToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->removeSnippetToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->GroupSnippetData->SuspendLayout();
			this->VarListContextMenu->SuspendLayout();
			this->SnippetTreeContextMenu->SuspendLayout();
			this->SuspendLayout();
			// 
			// SnippetTree
			// 
			this->SnippetTree->ContextMenuStrip = this->SnippetTreeContextMenu;
			this->SnippetTree->HotTracking = true;
			this->SnippetTree->Indent = 20;
			this->SnippetTree->Location = System::Drawing::Point(12, 12);
			this->SnippetTree->Name = L"SnippetTree";
			this->SnippetTree->Size = System::Drawing::Size(217, 411);
			this->SnippetTree->TabIndex = 0;
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
			this->GroupSnippetData->Location = System::Drawing::Point(240, 12);
			this->GroupSnippetData->Name = L"GroupSnippetData";
			this->GroupSnippetData->Size = System::Drawing::Size(437, 411);
			this->GroupSnippetData->TabIndex = 1;
			this->GroupSnippetData->TabStop = false;
			this->GroupSnippetData->Text = L"Snippet Data";
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
			// LabelShorthand
			// 
			this->LabelShorthand->AutoSize = true;
			this->LabelShorthand->Location = System::Drawing::Point(9, 57);
			this->LabelShorthand->Name = L"LabelShorthand";
			this->LabelShorthand->Size = System::Drawing::Size(56, 13);
			this->LabelShorthand->TabIndex = 1;
			this->LabelShorthand->Text = L"Shorthand";
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
			// LabelVariables
			// 
			this->LabelVariables->AutoSize = true;
			this->LabelVariables->Location = System::Drawing::Point(9, 114);
			this->LabelVariables->Name = L"LabelVariables";
			this->LabelVariables->Size = System::Drawing::Size(50, 13);
			this->LabelVariables->TabIndex = 3;
			this->LabelVariables->Text = L"Variables";
			// 
			// LabelCode
			// 
			this->LabelCode->AutoSize = true;
			this->LabelCode->Location = System::Drawing::Point(9, 216);
			this->LabelCode->Name = L"LabelCode";
			this->LabelCode->Size = System::Drawing::Size(32, 13);
			this->LabelCode->TabIndex = 4;
			this->LabelCode->Text = L"Code";
			// 
			// SnippetNameBox
			// 
			this->SnippetNameBox->Location = System::Drawing::Point(72, 27);
			this->SnippetNameBox->Name = L"SnippetNameBox";
			this->SnippetNameBox->Size = System::Drawing::Size(349, 20);
			this->SnippetNameBox->TabIndex = 5;
			// 
			// SnippetShorthandBox
			// 
			this->SnippetShorthandBox->Location = System::Drawing::Point(72, 54);
			this->SnippetShorthandBox->Name = L"SnippetShorthandBox";
			this->SnippetShorthandBox->Size = System::Drawing::Size(349, 20);
			this->SnippetShorthandBox->TabIndex = 6;
			// 
			// SnippetDescBox
			// 
			this->SnippetDescBox->Location = System::Drawing::Point(72, 83);
			this->SnippetDescBox->Name = L"SnippetDescBox";
			this->SnippetDescBox->Size = System::Drawing::Size(349, 20);
			this->SnippetDescBox->TabIndex = 7;
			// 
			// SnippetCodeBox
			// 
			this->SnippetCodeBox->AcceptsReturn = true;
			this->SnippetCodeBox->Location = System::Drawing::Point(12, 232);
			this->SnippetCodeBox->MaxLength = 150994943;
			this->SnippetCodeBox->Multiline = true;
			this->SnippetCodeBox->Name = L"SnippetCodeBox";
			this->SnippetCodeBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->SnippetCodeBox->Size = System::Drawing::Size(409, 136);
			this->SnippetCodeBox->TabIndex = 8;
			// 
			// ButtonApply
			// 
			this->ButtonApply->Location = System::Drawing::Point(327, 374);
			this->ButtonApply->Name = L"ButtonApply";
			this->ButtonApply->Size = System::Drawing::Size(94, 25);
			this->ButtonApply->TabIndex = 9;
			this->ButtonApply->Text = L"Apply Changes";
			this->ButtonApply->UseVisualStyleBackColor = true;
			// 
			// SnippetVarList
			// 
			this->SnippetVarList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(2) {this->SnippetVarListCHName, 
				this->SnippetVarListCHType});
			this->SnippetVarList->ContextMenuStrip = this->VarListContextMenu;
			this->SnippetVarList->Location = System::Drawing::Point(12, 130);
			this->SnippetVarList->Name = L"SnippetVarList";
			this->SnippetVarList->Size = System::Drawing::Size(409, 83);
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
			this->SnippetVarListCHType->Width = 82;
			// 
			// VarListContextMenu
			// 
			this->VarListContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->addVariableToolStripMenuItem, 
				this->removeVariableToolStripMenuItem});
			this->VarListContextMenu->Name = L"VarListContextMenu";
			this->VarListContextMenu->ShowImageMargin = false;
			this->VarListContextMenu->Size = System::Drawing::Size(138, 48);
			// 
			// addVariableToolStripMenuItem
			// 
			this->addVariableToolStripMenuItem->Name = L"addVariableToolStripMenuItem";
			this->addVariableToolStripMenuItem->Size = System::Drawing::Size(137, 22);
			this->addVariableToolStripMenuItem->Text = L"Add Variable";
			// 
			// removeVariableToolStripMenuItem
			// 
			this->removeVariableToolStripMenuItem->Name = L"removeVariableToolStripMenuItem";
			this->removeVariableToolStripMenuItem->Size = System::Drawing::Size(137, 22);
			this->removeVariableToolStripMenuItem->Text = L"Remove Variable";
			// 
			// SnippetTreeContextMenu
			// 
			this->SnippetTreeContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->addSnippetToolStripMenuItem, 
				this->removeSnippetToolStripMenuItem});
			this->SnippetTreeContextMenu->Name = L"SnippetTreeContextMenu";
			this->SnippetTreeContextMenu->ShowImageMargin = false;
			this->SnippetTreeContextMenu->Size = System::Drawing::Size(136, 48);
			// 
			// addSnippetToolStripMenuItem
			// 
			this->addSnippetToolStripMenuItem->Name = L"addSnippetToolStripMenuItem";
			this->addSnippetToolStripMenuItem->Size = System::Drawing::Size(135, 22);
			this->addSnippetToolStripMenuItem->Text = L"Add Snippet";
			// 
			// removeSnippetToolStripMenuItem
			// 
			this->removeSnippetToolStripMenuItem->Name = L"removeSnippetToolStripMenuItem";
			this->removeSnippetToolStripMenuItem->Size = System::Drawing::Size(135, 22);
			this->removeSnippetToolStripMenuItem->Text = L"Remove Snippet";
			// 
			// SESnippetManager
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(689, 435);
			this->Controls->Add(this->GroupSnippetData);
			this->Controls->Add(this->SnippetTree);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"SESnippetManager";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Code Snippet Manager";
			this->GroupSnippetData->ResumeLayout(false);
			this->GroupSnippetData->PerformLayout();
			this->VarListContextMenu->ResumeLayout(false);
			this->SnippetTreeContextMenu->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
