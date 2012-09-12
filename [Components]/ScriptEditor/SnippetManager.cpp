#include "SnippetManager.h"

#include "[Common]\NativeWrapper.h"
#include "[Common]\ListViewUtilities.h"
#include "[Common]\CustomInputBox.h"

using namespace System::IO;
using namespace System::Windows::Forms;

namespace ConstructionSetExtender
{
	namespace IntelliSense
	{
		CodeSnippet::VariableInfo::VariableInfo( String^ Name, ScriptParser::VariableType Type )
		{
			this->Name = Name;
			this->Type = Type;
		}

		void CodeSnippet::Save( String^ Path )
		{
			try
			{
				DataContractSerializer^ Serializer = gcnew DataContractSerializer(CodeSnippet::typeid);
				FileStream^ OutStream =  gcnew FileStream(Path + "\\" + Name + ".csesnippet", FileMode::Create);

				Serializer->WriteObject(OutStream, this);
				OutStream->Close();
			}
			catch (Exception^ E)
			{
				DebugPrint("Couldn't serialize code snippet '" + Name + "'!\n\tException: " + E->Message, true);
			}
		}

		CodeSnippet^ CodeSnippet::Load( String^ FullPath )
		{
			CodeSnippet^ Result = nullptr;

			try
			{
				DataContractSerializer^ Serializer = gcnew DataContractSerializer(CodeSnippet::typeid);
				FileStream^ InStream = gcnew FileStream(FullPath, FileMode::Open);

				Result = dynamic_cast<CodeSnippet^>(Serializer->ReadObject(InStream));
				InStream->Close();
			}
			catch (Exception^ E)
			{
				DebugPrint("Couldn't de-serialize code snippet at '" + FullPath + "'!\n\tException: " + E->Message, true);
			}

			return Result;
		}

		void CodeSnippet::AddVariable( String^ Name, ScriptParser::VariableType Type )
		{
			if (LookupVariable(Name) == nullptr)
				Variables->Add(gcnew VariableInfo(Name, Type));
		}

		void CodeSnippet::AddVariable( VariableInfo^ Var )
		{
			if (LookupVariable(Var->Name) == nullptr)
				Variables->Add(Var);
		}

		void CodeSnippet::RemoveVariable( VariableInfo^ Var )
		{
			if (LookupVariable(Var->Name))
				Variables->Remove(Var);
		}

		CodeSnippet::VariableInfo^ CodeSnippet::LookupVariable( String^ Name )
		{
			for each (VariableInfo^ Itr in Variables)
			{
				if (!String::Compare(Itr->Name, Name, true))
					return Itr;
			}

			return nullptr;
		}

		CodeSnippet::CodeSnippet() :
			Variables(gcnew List<VariableInfo^>())
		{
			Name = "";
			Shorthand = "";
			Description = "";
			Code = "";
		}

		CodeSnippet::~CodeSnippet()
		{
			Variables->Clear();
		}

		CodeSnippetCollection::CodeSnippetCollection() :
			LoadedSnippets(gcnew List<CodeSnippet^>())
		{
			;//
		}

		CodeSnippetCollection::CodeSnippetCollection( List<CodeSnippet^>^ Source ) :
			LoadedSnippets(gcnew List<CodeSnippet^>(Source))
		{
			;//
		}

		CodeSnippetCollection::~CodeSnippetCollection()
		{
			LoadedSnippets->Clear();
		}

		CodeSnippet^ CodeSnippetCollection::Lookup( String^ Name )
		{
			for each (CodeSnippet^ Itr in LoadedSnippets)
			{
				if (!String::Compare(Itr->Name, Name, true))
					return Itr;
			}

			return nullptr;
		}

		void CodeSnippetCollection::Save( String^ SnippetDirectory )
		{
			try
			{
				Directory::Delete(SnippetDirectory, true);
				Directory::CreateDirectory(SnippetDirectory);

				for each (CodeSnippet^ Itr in LoadedSnippets)
					Itr->Save(SnippetDirectory);
			}
			catch (Exception^ E)
			{
				DebugPrint("Couldn't save code snippet collection to '" + SnippetDirectory + "'!\n\tException: " + E->Message, true);
			}
		}

		void CodeSnippetCollection::Load( String^ SnippetDirectory )
		{
			try
			{
				if (Directory::Exists(SnippetDirectory) == false)
					Directory::CreateDirectory(SnippetDirectory);

				for each (String^ Itr in Directory::GetFiles(SnippetDirectory))
				{
					CodeSnippet^ New = CodeSnippet::Load(Itr);
					if (New != nullptr)
						Add(New);
				}
			}
			catch (Exception^ E)
			{
				DebugPrint("Couldn't load code snippet collection from '" + SnippetDirectory + "'!\n\tException: " + E->Message, true);
			}
		}

		void CodeSnippetCollection::Add( CodeSnippet^ In )
		{
			if (Lookup(In->Name) == nullptr)
				LoadedSnippets->Add(In);
		}

		void CodeSnippetCollection::Remove( CodeSnippet^ In )
		{
			if (Lookup(In->Name) != nullptr)
				LoadedSnippets->Remove(In);
		}

		void CodeSnippetManagerDialog::InitializeComponent()
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
			this->SnippetListCMAddSnippet->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::SnippetListCMAddSnippet_Click);
			//
			// SnippetListCMRemoveSnippet
			//
			this->SnippetListCMRemoveSnippet->Name = L"SnippetListCMRemoveSnippet";
			this->SnippetListCMRemoveSnippet->Size = System::Drawing::Size(132, 22);
			this->SnippetListCMRemoveSnippet->Text = L"Remove Snippet";
			this->SnippetListCMRemoveSnippet->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::SnippetListCMRemoveSnippet_Click);
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
			this->VarListCMRemoveVariable->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::VarListCMRemoveVariable_Click);
			//
			// ButtonApply
			//
			this->ButtonApply->Location = System::Drawing::Point(377, 470);
			this->ButtonApply->Name = L"ButtonApply";
			this->ButtonApply->Size = System::Drawing::Size(94, 25);
			this->ButtonApply->TabIndex = 9;
			this->ButtonApply->Text = L"Apply Changes";
			this->ButtonApply->UseVisualStyleBackColor = true;
			this->ButtonApply->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::ButtonApply_Click);
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
			this->SnippetList->Tag = (int)1;
			this->SnippetList->Sorting = SortOrder::Ascending;
			this->SnippetList->SelectedIndexChanged += gcnew EventHandler(this, &CodeSnippetManagerDialog::SnippetList_SelectedIndexChanged);
			this->SnippetList->ColumnClick += gcnew ColumnClickEventHandler(this, &CodeSnippetManagerDialog::SnippetList_ColumnClick);
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
			this->VarListCMAddVarInt->Tag = ScriptParser::VariableType::e_Integer;
			this->VarListCMAddVarInt->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			this->VarListCMAddVarInt->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::VarListCMAddVariable_Click);
			//
			// VarListCMAddVarFloat
			//
			this->VarListCMAddVarFloat->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarFloat->Name = L"VarListCMAddVarFloat";
			this->VarListCMAddVarFloat->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarFloat->Text = L"Float";
			this->VarListCMAddVarFloat->Tag = ScriptParser::VariableType::e_Float;
			this->VarListCMAddVarFloat->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			this->VarListCMAddVarFloat->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::VarListCMAddVariable_Click);

			//
			// VarListCMAddVarRef
			//
			this->VarListCMAddVarRef->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarRef->Name = L"VarListCMAddVarRef";
			this->VarListCMAddVarRef->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarRef->Text = L"Reference";
			this->VarListCMAddVarRef->Tag = ScriptParser::VariableType::e_Ref;
			this->VarListCMAddVarRef->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			this->VarListCMAddVarRef->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::VarListCMAddVariable_Click);
			//
			// VarListCMAddVarString
			//
			this->VarListCMAddVarString->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarString->Name = L"VarListCMAddVarString";
			this->VarListCMAddVarString->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarString->Text = L"String";
			this->VarListCMAddVarString->Tag = ScriptParser::VariableType::e_String;
			this->VarListCMAddVarString->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			this->VarListCMAddVarString->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::VarListCMAddVariable_Click);
			//
			// VarListCMAddVarArray
			//
			this->VarListCMAddVarArray->DisplayStyle = System::Windows::Forms::ToolStripItemDisplayStyle::Text;
			this->VarListCMAddVarArray->Name = L"VarListCMAddVarArray";
			this->VarListCMAddVarArray->Size = System::Drawing::Size(152, 22);
			this->VarListCMAddVarArray->Text = L"Array";
			this->VarListCMAddVarArray->Tag = ScriptParser::VariableType::e_Array;
			this->VarListCMAddVarArray->TextImageRelation = System::Windows::Forms::TextImageRelation::TextBeforeImage;
			this->VarListCMAddVarArray->Click += gcnew EventHandler(this, &CodeSnippetManagerDialog::VarListCMAddVariable_Click);
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
			this->KeyPreview = true;
			this->Name = L"SESnippetManager";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Code Snippet Manager";
			this->SnippetListContextMenu->ResumeLayout(false);
			this->GroupSnippetData->ResumeLayout(false);
			this->GroupSnippetData->PerformLayout();
			this->VarListContextMenu->ResumeLayout(false);
			this->ResumeLayout(false);
		}

		CodeSnippetManagerDialog::CodeSnippetManagerDialog( CodeSnippetCollection^ Data )
		{
			if (Data == nullptr)
				Data = gcnew CodeSnippetCollection();

			InitializeComponent();

			WorkingCopy = Data;

			PopulateSnippetList();

			this->Hide();
			this->ShowDialog();
		}

		CodeSnippetManagerDialog::~CodeSnippetManagerDialog()
		{
			if (components)
			{
				delete components;
			}

			WorkingCopy = nullptr;
		}

		void CodeSnippetManagerDialog::PopulateSnippetList()
		{
			if (WorkingCopy)
			{
				SnippetList->BeginUpdate();

				SnippetList->Items->Clear();

				for each (CodeSnippet^ Itr in WorkingCopy->LoadedSnippets)
				{
					ListViewItem^ Item = gcnew ListViewItem(Itr->Name);
					Item->SubItems->Add(Itr->Shorthand);
					Item->Tag = Itr;

					SnippetList->Items->Add(Item);
				}

				SnippetList->EndUpdate();
			}
		}

		void CodeSnippetManagerDialog::SetSnippetData( CodeSnippet^ Snippet, ListViewItem^ LVItem )
		{
			String^ ReservedCharacters = "\\/:*?\"<>|";
			String^ NewName = SnippetNameBox->Text;

			if (NewName->IndexOfAny(ReservedCharacters->ToCharArray()) != -1)
			{
				MessageBox::Show("Snippet name cannot contain any of the following characters: " + ReservedCharacters,
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);
				return;
			}
			else if (String::Compare(NewName, Snippet->Name, true) &&
					WorkingCopy->Lookup(NewName))
			{
				MessageBox::Show("Snippet name must be unique.",
								SCRIPTEDITOR_TITLE,
								MessageBoxButtons::OK,
								MessageBoxIcon::Exclamation);
				return;
			}

			Snippet->Name = NewName;
			Snippet->Shorthand = SnippetShorthandBox->Text;
			Snippet->Description = SnippetDescBox->Text;
			Snippet->Code = SnippetCodeBox->Text;

			Snippet->Variables->Clear();

			for each (ListViewItem^ Itr in SnippetVarList->Items)
				Snippet->AddVariable(dynamic_cast<CodeSnippet::VariableInfo^>(Itr->Tag));

			LVItem->SubItems[0]->Text = Snippet->Name;
			LVItem->SubItems[1]->Text = Snippet->Shorthand;
		}

		void CodeSnippetManagerDialog::GetSnippetData( CodeSnippet^ Snippet )
		{
			if (Snippet == nullptr)
			{
				SnippetNameBox->Text = "";
				SnippetShorthandBox->Text = "";
				SnippetDescBox->Text = "";
				SnippetCodeBox->Text = "";

				SnippetVarList->Items->Clear();
			}
			else
			{
				SnippetNameBox->Text = Snippet->Name;
				SnippetShorthandBox->Text = Snippet->Shorthand;
				SnippetDescBox->Text = Snippet->Description;
				SnippetCodeBox->Text = Snippet->Code;

				SnippetVarList->BeginUpdate();

				SnippetVarList->Items->Clear();

				for each (CodeSnippet::VariableInfo^ Itr in Snippet->Variables)
				{
					ListViewItem^ Item = gcnew ListViewItem(Itr->Name);
					Item->SubItems->Add(ScriptParser::GetVariableID(Itr->Type));
					Item->Tag = Itr;

					SnippetVarList->Items->Add(Item);
				}

				SnippetVarList->EndUpdate();
			}
		}

		void CodeSnippetManagerDialog::ButtonApply_Click( Object^ Sender, EventArgs^ E )
		{
			ListViewItem^ Selection = GetListViewSelectedItem(SnippetList);

			if (Selection)
			{
				CodeSnippet^ Current = dynamic_cast<CodeSnippet^>(Selection->Tag);
				SetSnippetData(Current, Selection);
			}
		}

		void CodeSnippetManagerDialog::SnippetListCMAddSnippet_Click( Object^ Sender, EventArgs^ E )
		{
			InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter Snippet Name", "New Snippet");
			if (Result->ReturnCode == System::Windows::Forms::DialogResult::Cancel || Result->Text == "")
				return;
			else
			{
				String^ Name = Result->Text;

				if (WorkingCopy->Lookup(Name))
				{
					MessageBox::Show("Snippet name must be unique.",
									SCRIPTEDITOR_TITLE,
									MessageBoxButtons::OK,
									MessageBoxIcon::Exclamation);
					return;
				}

				CodeSnippet^ NewSnippet = gcnew CodeSnippet();
				NewSnippet->Name = Name;

				ListViewItem^ Item = gcnew ListViewItem(Name);
				Item->SubItems->Add(NewSnippet->Shorthand);
				Item->Tag = NewSnippet;

				SnippetList->Items->Add(Item);
				WorkingCopy->Add(NewSnippet);

				Item->Selected = true;
			}
		}

		void CodeSnippetManagerDialog::SnippetListCMRemoveSnippet_Click( Object^ Sender, EventArgs^ E )
		{
			ListViewItem^ Selection = GetListViewSelectedItem(SnippetList);

			if (Selection)
			{
				CodeSnippet^ Current = dynamic_cast<CodeSnippet^>(Selection->Tag);
				WorkingCopy->Remove(Current);

				SnippetList->Items->Remove(Selection);
			}
		}

		void CodeSnippetManagerDialog::VarListCMAddVariable_Click( Object^ Sender, EventArgs^ E )
		{
			ToolStripMenuItem^ MenuItem = dynamic_cast<ToolStripMenuItem^>(Sender);
			ScriptParser::VariableType VarType = (ScriptParser::VariableType)MenuItem->Tag;

			InputBoxes::InputBoxResult^ Result = InputBoxes::InputBox::Show("Enter Variable Name", "Add Variable");
			if (Result->ReturnCode == System::Windows::Forms::DialogResult::Cancel || Result->Text == "")
				return;
			else
			{
				String^ VarName = Result->Text;

				CodeSnippet::VariableInfo^ NewVar = gcnew CodeSnippet::VariableInfo(VarName, VarType);

				ListViewItem^ Item = gcnew ListViewItem(VarName);
				Item->SubItems->Add(ScriptParser::GetVariableKeyword(VarType));
				Item->Tag = NewVar;

				SnippetVarList->Items->Add(Item);
			}
		}

		void CodeSnippetManagerDialog::VarListCMRemoveVariable_Click( Object^ Sender, EventArgs^ E )
		{
			ListViewItem^ Selection = GetListViewSelectedItem(SnippetVarList);

			if (Selection)
			{
				SnippetVarList->Items->Remove(Selection);
			}
		}

		void CodeSnippetManagerDialog::SnippetList_SelectedIndexChanged( Object^ Sender, EventArgs^ E )
		{
			ListViewItem^ Selection = GetListViewSelectedItem(SnippetList);

			if (Selection)
			{
				CodeSnippet^ Current = dynamic_cast<CodeSnippet^>(Selection->Tag);
				GetSnippetData(Current);
			}
			else
				GetSnippetData(nullptr);
		}

		void CodeSnippetManagerDialog::SnippetList_ColumnClick( Object^ Sender, ColumnClickEventArgs^ E )
		{
			if (E->Column != (int)SnippetList->Tag)
			{
				SnippetList->Tag = E->Column;
				SnippetList->Sorting = SortOrder::Ascending;
			}
			else
			{
				if (SnippetList->Sorting == SortOrder::Ascending)
					SnippetList->Sorting = SortOrder::Descending;
				else
					SnippetList->Sorting = SortOrder::Ascending;
			}

			SnippetList->Sort();
			SnippetList->ListViewItemSorter = gcnew ListViewStringSorter(E->Column, SnippetList->Sorting);
		}
	}
}