#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;


namespace UIComponents {

	/// <summary>
	/// Summary for TagBrowser
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class TagBrowser : public System::Windows::Forms::Form
	{
	public:
		TagBrowser(void)
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
		~TagBrowser()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::TextBox^  SearchBox;
	protected: 


	private: System::Windows::Forms::ListView^  FormList;
	private: System::Windows::Forms::ColumnHeader^  FormListHEditorID;
	private: System::Windows::Forms::ColumnHeader^  FormListHRefID;
	private: System::Windows::Forms::ColumnHeader^  FormListHType;




	private: DevComponents::AdvTree::AdvTree^  TagTree;


	private: DevComponents::AdvTree::NodeConnector^  TagTreeNodeConnector;
	private: DevComponents::DotNetBar::ElementStyle^  TagTreeElementStyle1;





















	private: DevComponents::DotNetBar::ElementStyle^  TagTreeElementStyle2;

	private: System::Windows::Forms::ContextMenuStrip^  TagTreeContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  TagTreeContextMenuAdd;
	private: System::Windows::Forms::ToolStripMenuItem^  TagTreeContextMenuRemove;

	private: System::Windows::Forms::ContextMenuStrip^  FormListContextMenu;
	private: System::Windows::Forms::ToolStripMenuItem^  FormListContextMenuRemove;





	private: System::Windows::Forms::Button^  SaveTags;
	private: System::Windows::Forms::Button^  LoadTags;













	private: System::ComponentModel::IContainer^  components;
















	protected: 

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
			this->SearchBox = (gcnew System::Windows::Forms::TextBox());
			this->FormList = (gcnew System::Windows::Forms::ListView());
			this->FormListHEditorID = (gcnew System::Windows::Forms::ColumnHeader());
			this->FormListHRefID = (gcnew System::Windows::Forms::ColumnHeader());
			this->FormListHType = (gcnew System::Windows::Forms::ColumnHeader());
			this->FormListContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->FormListContextMenuRemove = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->TagTree = (gcnew DevComponents::AdvTree::AdvTree());
			this->TagTreeContextMenu = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->TagTreeContextMenuAdd = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->TagTreeContextMenuRemove = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->TagTreeNodeConnector = (gcnew DevComponents::AdvTree::NodeConnector());
			this->TagTreeElementStyle2 = (gcnew DevComponents::DotNetBar::ElementStyle());
			this->TagTreeElementStyle1 = (gcnew DevComponents::DotNetBar::ElementStyle());
			this->SaveTags = (gcnew System::Windows::Forms::Button());
			this->LoadTags = (gcnew System::Windows::Forms::Button());
			this->FormListContextMenu->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->TagTree))->BeginInit();
			this->TagTreeContextMenu->SuspendLayout();
			this->SuspendLayout();
			// 
			// SearchBox
			// 
			this->SearchBox->Location = System::Drawing::Point(310, 624);
			this->SearchBox->Multiline = true;
			this->SearchBox->Name = L"SearchBox";
			this->SearchBox->Size = System::Drawing::Size(554, 33);
			this->SearchBox->TabIndex = 1;
			// 
			// FormList
			// 
			this->FormList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {this->FormListHEditorID, 
				this->FormListHRefID, this->FormListHType});
			this->FormList->ContextMenuStrip = this->FormListContextMenu;
			this->FormList->FullRowSelect = true;
			this->FormList->Location = System::Drawing::Point(310, 12);
			this->FormList->Name = L"FormList";
			this->FormList->Size = System::Drawing::Size(646, 606);
			this->FormList->TabIndex = 2;
			this->FormList->UseCompatibleStateImageBehavior = false;
			this->FormList->View = System::Windows::Forms::View::Details;
			// 
			// FormListHEditorID
			// 
			this->FormListHEditorID->Text = L"EditorID";
			this->FormListHEditorID->Width = 482;
			// 
			// FormListHRefID
			// 
			this->FormListHRefID->Text = L"FormID";
			this->FormListHRefID->Width = 64;
			// 
			// FormListHType
			// 
			this->FormListHType->Text = L"Type";
			this->FormListHType->Width = 77;
			// 
			// FormListContextMenu
			// 
			this->FormListContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->FormListContextMenuRemove});
			this->FormListContextMenu->Name = L"FormListContextMenu";
			this->FormListContextMenu->Size = System::Drawing::Size(165, 26);
			// 
			// FormListContextMenuRemove
			// 
			this->FormListContextMenuRemove->Name = L"FormListContextMenuRemove";
			this->FormListContextMenuRemove->RightToLeft = System::Windows::Forms::RightToLeft::Yes;
			this->FormListContextMenuRemove->Size = System::Drawing::Size(164, 22);
			this->FormListContextMenuRemove->Text = L"Remove From Tag";
			// 
			// TagTree
			// 
			this->TagTree->AccessibleRole = System::Windows::Forms::AccessibleRole::Outline;
			this->TagTree->AllowDrop = true;
			this->TagTree->BackColor = System::Drawing::SystemColors::Window;
			// 
			// 
			// 
			this->TagTree->BackgroundStyle->Class = L"TreeBorderKey";
			this->TagTree->BackgroundStyle->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->TagTree->CellEdit = true;
			this->TagTree->ColorSchemeStyle = DevComponents::AdvTree::eColorSchemeStyle::VS2005;
			this->TagTree->ColumnsVisible = false;
			this->TagTree->ContextMenuStrip = this->TagTreeContextMenu;
			this->TagTree->Cursor = System::Windows::Forms::Cursors::Arrow;
			this->TagTree->GridRowLines = true;
			this->TagTree->HideSelection = true;
			this->TagTree->HotTracking = true;
			this->TagTree->Location = System::Drawing::Point(9, 12);
			this->TagTree->Name = L"TagTree";
			this->TagTree->NodesConnector = this->TagTreeNodeConnector;
			this->TagTree->NodeSpacing = 4;
			this->TagTree->NodeStyle = this->TagTreeElementStyle2;
			this->TagTree->PathSeparator = L";";
			this->TagTree->SelectionBoxStyle = DevComponents::AdvTree::eSelectionStyle::NodeMarker;
			this->TagTree->SelectionPerCell = true;
			this->TagTree->Size = System::Drawing::Size(295, 645);
			this->TagTree->Styles->Add(this->TagTreeElementStyle1);
			this->TagTree->Styles->Add(this->TagTreeElementStyle2);
			this->TagTree->TabIndex = 4;
			this->TagTree->Text = L"Tags";
			// 
			// TagTreeContextMenu
			// 
			this->TagTreeContextMenu->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->TagTreeContextMenuAdd, 
				this->TagTreeContextMenuRemove});
			this->TagTreeContextMenu->Name = L"TagTreeContextMenu";
			this->TagTreeContextMenu->Size = System::Drawing::Size(141, 48);
			// 
			// TagTreeContextMenuAdd
			// 
			this->TagTreeContextMenuAdd->Name = L"TagTreeContextMenuAdd";
			this->TagTreeContextMenuAdd->Size = System::Drawing::Size(140, 22);
			this->TagTreeContextMenuAdd->Text = L"Add Tag";
			// 
			// TagTreeContextMenuRemove
			// 
			this->TagTreeContextMenuRemove->Name = L"TagTreeContextMenuRemove";
			this->TagTreeContextMenuRemove->Size = System::Drawing::Size(140, 22);
			this->TagTreeContextMenuRemove->Text = L"Remove Tag";
			// 
			// TagTreeNodeConnector
			// 
			this->TagTreeNodeConnector->DashStyle = System::Drawing::Drawing2D::DashStyle::Solid;
			this->TagTreeNodeConnector->LineColor = System::Drawing::SystemColors::ControlText;
			// 
			// TagTreeElementStyle2
			// 
			this->TagTreeElementStyle2->BackColor = System::Drawing::Color::White;
			this->TagTreeElementStyle2->BackColor2 = System::Drawing::Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(228)), 
				static_cast<System::Int32>(static_cast<System::Byte>(228)), static_cast<System::Int32>(static_cast<System::Byte>(240)));
			this->TagTreeElementStyle2->BackColorGradientAngle = 90;
			this->TagTreeElementStyle2->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
			this->TagTreeElementStyle2->BorderBottomWidth = 1;
			this->TagTreeElementStyle2->BorderColor = System::Drawing::Color::DarkGray;
			this->TagTreeElementStyle2->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
			this->TagTreeElementStyle2->BorderLeftWidth = 1;
			this->TagTreeElementStyle2->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
			this->TagTreeElementStyle2->BorderRightWidth = 1;
			this->TagTreeElementStyle2->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
			this->TagTreeElementStyle2->BorderTopWidth = 1;
			this->TagTreeElementStyle2->Class = L"";
			this->TagTreeElementStyle2->CornerDiameter = 4;
			this->TagTreeElementStyle2->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->TagTreeElementStyle2->Description = L"Gray";
			this->TagTreeElementStyle2->Name = L"TagTreeElementStyle2";
			this->TagTreeElementStyle2->PaddingBottom = 1;
			this->TagTreeElementStyle2->PaddingLeft = 1;
			this->TagTreeElementStyle2->PaddingRight = 1;
			this->TagTreeElementStyle2->PaddingTop = 1;
			this->TagTreeElementStyle2->TextColor = System::Drawing::Color::Black;
			// 
			// TagTreeElementStyle1
			// 
			this->TagTreeElementStyle1->Class = L"";
			this->TagTreeElementStyle1->CornerType = DevComponents::DotNetBar::eCornerType::Square;
			this->TagTreeElementStyle1->Name = L"TagTreeElementStyle1";
			this->TagTreeElementStyle1->TextColor = System::Drawing::SystemColors::ControlText;
			// 
			// SaveTags
			// 
			this->SaveTags->BackColor = System::Drawing::SystemColors::Control;
			this->SaveTags->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->SaveTags->ForeColor = System::Drawing::Color::Black;
			this->SaveTags->Location = System::Drawing::Point(870, 624);
			this->SaveTags->Name = L"SaveTags";
			this->SaveTags->Size = System::Drawing::Size(40, 33);
			this->SaveTags->TabIndex = 19;
			this->SaveTags->Text = L"Save";
			this->SaveTags->UseVisualStyleBackColor = false;
			// 
			// LoadTags
			// 
			this->LoadTags->BackColor = System::Drawing::SystemColors::Control;
			this->LoadTags->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->LoadTags->ForeColor = System::Drawing::Color::Black;
			this->LoadTags->Location = System::Drawing::Point(916, 624);
			this->LoadTags->Name = L"LoadTags";
			this->LoadTags->Size = System::Drawing::Size(40, 33);
			this->LoadTags->TabIndex = 20;
			this->LoadTags->Text = L"Load";
			this->LoadTags->UseVisualStyleBackColor = false;
			this->LoadTags->Click += gcnew System::EventHandler(this, &TagBrowser::button1_Click);
			// 
			// TagBrowser
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(968, 669);
			this->Controls->Add(this->TagTree);
			this->Controls->Add(this->FormList);
			this->Controls->Add(this->SearchBox);
			this->Controls->Add(this->SaveTags);
			this->Controls->Add(this->LoadTags);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedSingle;
			this->MaximizeBox = false;
			this->Name = L"TagBrowser";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterScreen;
			this->Text = L"Tag Browser";
			this->FormListContextMenu->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->TagTree))->EndInit();
			this->TagTreeContextMenu->ResumeLayout(false);
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			 }
};
}
