#pragma once

namespace UIComponents {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for ArchiveBrowser
	/// </summary>
	public ref class ArchiveBrowser : public DevComponents::DotNetBar::Metro::MetroForm
	{
	public:
		ArchiveBrowser(void)
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
		~ArchiveBrowser()
		{
			if (components)
			{
				delete components;
			}
		}
	private: DevComponents::DotNetBar::Bar^ TopToolbar;
	private: DevComponents::DotNetBar::ButtonItem^ TopToolbarOpenArchive;
	protected:

	private: DevComponents::DotNetBar::LabelItem^ TopToolbarCurrentArchiveName;





	private: BrightIdeasSoftware::TreeListView^ DirectoryTreeListView;
	private: DevComponents::DotNetBar::ButtonX^ BottomToolbarOk;

	private: DevComponents::DotNetBar::Bar^ BottomToolbar;
	private: DevComponents::DotNetBar::TextBoxItem^ BottomToolbarTextboxFilter;
	private: DevComponents::DotNetBar::ButtonX^ BottomToolbarCancel;
	private: System::Windows::Forms::Panel^ BottomToolbarContainer;
	private: BrightIdeasSoftware::OLVColumn^ DirectoryTreeListViewColName;
	private: BrightIdeasSoftware::OLVColumn^ DirectoryTreeListViewColSize;
	private: BrightIdeasSoftware::OLVColumn^ DirectoryTreeListViewColType;


	private: System::ComponentModel::IContainer^ components;

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
			System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(ArchiveBrowser::typeid));
			this->TopToolbar = (gcnew DevComponents::DotNetBar::Bar());
			this->TopToolbarOpenArchive = (gcnew DevComponents::DotNetBar::ButtonItem());
			this->TopToolbarCurrentArchiveName = (gcnew DevComponents::DotNetBar::LabelItem());
			this->DirectoryTreeListView = (gcnew BrightIdeasSoftware::TreeListView());
			this->DirectoryTreeListViewColName = (gcnew BrightIdeasSoftware::OLVColumn());
			this->DirectoryTreeListViewColSize = (gcnew BrightIdeasSoftware::OLVColumn());
			this->DirectoryTreeListViewColType = (gcnew BrightIdeasSoftware::OLVColumn());
			this->BottomToolbarOk = (gcnew DevComponents::DotNetBar::ButtonX());
			this->BottomToolbar = (gcnew DevComponents::DotNetBar::Bar());
			this->BottomToolbarTextboxFilter = (gcnew DevComponents::DotNetBar::TextBoxItem());
			this->BottomToolbarCancel = (gcnew DevComponents::DotNetBar::ButtonX());
			this->BottomToolbarContainer = (gcnew System::Windows::Forms::Panel());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->TopToolbar))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->DirectoryTreeListView))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomToolbar))->BeginInit();
			this->BottomToolbarContainer->SuspendLayout();
			this->SuspendLayout();
			// 
			// TopToolbar
			// 
			this->TopToolbar->AntiAlias = true;
			this->TopToolbar->CanAutoHide = false;
			this->TopToolbar->CanMove = false;
			this->TopToolbar->CanUndock = false;
			this->TopToolbar->Dock = System::Windows::Forms::DockStyle::Top;
			this->TopToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->TopToolbar->IsMaximized = false;
			this->TopToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(2) {
				this->TopToolbarOpenArchive,
					this->TopToolbarCurrentArchiveName
			});
			this->TopToolbar->Location = System::Drawing::Point(0, 0);
			this->TopToolbar->Name = L"TopToolbar";
			this->TopToolbar->RoundCorners = false;
			this->TopToolbar->Size = System::Drawing::Size(476, 25);
			this->TopToolbar->Stretch = true;
			this->TopToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->TopToolbar->TabIndex = 0;
			this->TopToolbar->TabStop = false;
			this->TopToolbar->Text = L"bar1";
			// 
			// TopToolbarOpenArchive
			// 
			this->TopToolbarOpenArchive->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::ImageAndText;
			this->TopToolbarOpenArchive->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"TopToolbarOpenArchive.Image")));
			this->TopToolbarOpenArchive->Name = L"TopToolbarOpenArchive";
			this->TopToolbarOpenArchive->Text = L"Open Archive";
			// 
			// TopToolbarCurrentArchiveName
			// 
			this->TopToolbarCurrentArchiveName->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
			this->TopToolbarCurrentArchiveName->Name = L"TopToolbarCurrentArchiveName";
			this->TopToolbarCurrentArchiveName->Text = L"Archive Name";
			// 
			// DirectoryTreeListView
			// 
			this->DirectoryTreeListView->AllColumns->Add(this->DirectoryTreeListViewColName);
			this->DirectoryTreeListView->AllColumns->Add(this->DirectoryTreeListViewColSize);
			this->DirectoryTreeListView->AllColumns->Add(this->DirectoryTreeListViewColType);
			this->DirectoryTreeListView->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->DirectoryTreeListView->CellEditUseWholeCell = false;
			this->DirectoryTreeListView->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(3) {
				this->DirectoryTreeListViewColName,
					this->DirectoryTreeListViewColSize, this->DirectoryTreeListViewColType
			});
			this->DirectoryTreeListView->Cursor = System::Windows::Forms::Cursors::Default;
			this->DirectoryTreeListView->EmptyListMsg = L"Open a BSA archive";
			this->DirectoryTreeListView->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9, System::Drawing::FontStyle::Bold,
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->DirectoryTreeListView->FullRowSelect = true;
			this->DirectoryTreeListView->GridLines = true;
			this->DirectoryTreeListView->HideSelection = false;
			this->DirectoryTreeListView->Location = System::Drawing::Point(0, 31);
			this->DirectoryTreeListView->Name = L"DirectoryTreeListView";
			this->DirectoryTreeListView->ShowGroups = false;
			this->DirectoryTreeListView->Size = System::Drawing::Size(476, 438);
			this->DirectoryTreeListView->TabIndex = 2;
			this->DirectoryTreeListView->UseCompatibleStateImageBehavior = false;
			this->DirectoryTreeListView->UseFiltering = true;
			this->DirectoryTreeListView->View = System::Windows::Forms::View::Details;
			this->DirectoryTreeListView->VirtualMode = true;
			// 
			// DirectoryTreeListViewColName
			// 
			this->DirectoryTreeListViewColName->Text = L"Name";
			this->DirectoryTreeListViewColName->Width = 315;
			// 
			// DirectoryTreeListViewColSize
			// 
			this->DirectoryTreeListViewColSize->Text = L"Size";
			// 
			// DirectoryTreeListViewColType
			// 
			this->DirectoryTreeListViewColType->Text = L"Type";
			// 
			// BottomToolbarOk
			// 
			this->BottomToolbarOk->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->BottomToolbarOk->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->BottomToolbarOk->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->BottomToolbarOk->Location = System::Drawing::Point(352, 475);
			this->BottomToolbarOk->Name = L"BottomToolbarOk";
			this->BottomToolbarOk->Size = System::Drawing::Size(59, 29);
			this->BottomToolbarOk->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BottomToolbarOk->TabIndex = 4;
			this->BottomToolbarOk->Text = L"OK";
			// 
			// BottomToolbar
			// 
			this->BottomToolbar->AntiAlias = true;
			this->BottomToolbar->Dock = System::Windows::Forms::DockStyle::Fill;
			this->BottomToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
			this->BottomToolbar->IsMaximized = false;
			this->BottomToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(1) { this->BottomToolbarTextboxFilter });
			this->BottomToolbar->Location = System::Drawing::Point(0, 0);
			this->BottomToolbar->Margin = System::Windows::Forms::Padding(0);
			this->BottomToolbar->MaximumSize = System::Drawing::Size(0, 29);
			this->BottomToolbar->MinimumSize = System::Drawing::Size(0, 29);
			this->BottomToolbar->Name = L"BottomToolbar";
			this->BottomToolbar->PaddingBottom = 0;
			this->BottomToolbar->PaddingLeft = 0;
			this->BottomToolbar->PaddingRight = 0;
			this->BottomToolbar->PaddingTop = 4;
			this->BottomToolbar->RoundCorners = false;
			this->BottomToolbar->Size = System::Drawing::Size(346, 29);
			this->BottomToolbar->Stretch = true;
			this->BottomToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BottomToolbar->TabIndex = 3;
			this->BottomToolbar->TabStop = false;
			this->BottomToolbar->Text = L"bar1";
			// 
			// BottomToolbarTextboxFilter
			// 
			this->BottomToolbarTextboxFilter->Name = L"BottomToolbarTextboxFilter";
			this->BottomToolbarTextboxFilter->Stretch = true;
			this->BottomToolbarTextboxFilter->TextBoxWidth = 200;
			this->BottomToolbarTextboxFilter->WatermarkColor = System::Drawing::SystemColors::GrayText;
			this->BottomToolbarTextboxFilter->WatermarkText = L"Filter";
			// 
			// BottomToolbarCancel
			// 
			this->BottomToolbarCancel->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
			this->BottomToolbarCancel->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Right));
			this->BottomToolbarCancel->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
			this->BottomToolbarCancel->Location = System::Drawing::Point(417, 475);
			this->BottomToolbarCancel->Name = L"BottomToolbarCancel";
			this->BottomToolbarCancel->Size = System::Drawing::Size(59, 29);
			this->BottomToolbarCancel->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
			this->BottomToolbarCancel->TabIndex = 5;
			this->BottomToolbarCancel->Text = L"Cancel";
			// 
			// BottomToolbarContainer
			// 
			this->BottomToolbarContainer->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->BottomToolbarContainer->Controls->Add(this->BottomToolbar);
			this->BottomToolbarContainer->Location = System::Drawing::Point(0, 475);
			this->BottomToolbarContainer->Name = L"BottomToolbarContainer";
			this->BottomToolbarContainer->Size = System::Drawing::Size(346, 29);
			this->BottomToolbarContainer->TabIndex = 10;
			// 
			// ArchiveBrowser
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(476, 506);
			this->Controls->Add(this->BottomToolbarContainer);
			this->Controls->Add(this->BottomToolbarCancel);
			this->Controls->Add(this->BottomToolbarOk);
			this->Controls->Add(this->DirectoryTreeListView);
			this->Controls->Add(this->TopToolbar);
			this->DoubleBuffered = true;
			this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
			this->Name = L"ArchiveBrowser";
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Archive Asset Selector";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->TopToolbar))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->DirectoryTreeListView))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomToolbar))->EndInit();
			this->BottomToolbarContainer->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
	};
}
