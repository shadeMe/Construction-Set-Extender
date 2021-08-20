#include "BSAViewer.h"


namespace cse
{


namespace bsaViewer
{


ArchiveFile::ArchiveFile(String^ ParentDirectory, UInt32 SizeInBytes, bool Compressed)
{
	this->Filename = String::Empty;
	this->ParentDirectory = ParentDirectory;
	this->SizeInBytes = SizeInBytes;
	this->Compressed = Compressed;
	this->FileType = eFileType::Unknown;
};

ArchiveFile::eFileType ArchiveFile::GetFileTypeFromExtension(String^ Extension)
{
	if (FileTypeExtensions->Count == 0)
	{
		FileTypeExtensions->Add("dds", eFileType::Dds);
		FileTypeExtensions->Add("nif", eFileType::Nif);
		FileTypeExtensions->Add("mp3", eFileType::Mp3);
		FileTypeExtensions->Add("wav", eFileType::Wav);
		FileTypeExtensions->Add("lip", eFileType::Lip);
		FileTypeExtensions->Add("Kf", eFileType::Kf);
		FileTypeExtensions->Add("spt", eFileType::Spt);
	}

	auto Out = eFileType::Unknown;
	FileTypeExtensions->TryGetValue(Extension, Out);
	return Out;
}

System::String^ ArchiveFile::Extension::get()
{
	auto Ext = Path::GetExtension(Filename);
	if (Ext->Length != 0)
		Ext = Ext->Substring(1);	// remove the period
	return Ext;
}

Archive::Archive(String^ FilePath)
{
	this->FilePath = FilePath;
	this->Valid = false;
	this->ArchiveFlags = eArchiveFlags::None;
	this->ContentFlags = eContentFlags::None;
	this->Files = gcnew List<ArchiveFile^>;
}


System::Collections::Generic::ICollection<DirectoryTreeNode^>^ Archive::GenerateDirectoryTree(String^ PathFilter, ArchiveFile::eFileType FileTypeFilter)
{
	auto Root = gcnew DirectoryTreeNode();

	for each (auto File in Files)
	{
		DirectoryTreeNode^ WorkingDir = Root;

		if (FileTypeFilter != ArchiveFile::eFileType::Unknown && File->FileType != FileTypeFilter)
			continue;
		else if (PathFilter->Length > 0 && File->FullPath->IndexOf(PathFilter, StringComparison::CurrentCultureIgnoreCase) == -1)
			continue;

		auto Splits = File->FullPath->Split('\\');
		String^ PathCrumb = "";

		for (int i = 0; i < Splits->Length; ++i)
		{
			auto CurrentSplit = Splits[i];
			PathCrumb += (i != 0 ? "\\" : "") + CurrentSplit;

			DirectoryTreeNode^ ExistingNode = nullptr;
			if (WorkingDir->Children->TryGetValue(CurrentSplit, ExistingNode))
				WorkingDir = ExistingNode;
			else
			{
				auto NewNode = gcnew DirectoryTreeNode(PathCrumb);
				if (i == Splits->Length - 1)
					NewNode->SourceFile = File;

				WorkingDir->Children->Add(CurrentSplit, NewNode);
				WorkingDir = NewNode;
			}
		}
	}

	return Root->Children->Values;
}

DirectoryTreeNode::DirectoryTreeNode(String^ FullPath)
{
	this->Name = Path::GetFileName(FullPath);
	this->ParentPath = Path::GetDirectoryName(FullPath);
	this->FullPath = FullPath;
	this->Children = gcnew Dictionary<String^, DirectoryTreeNode^>(StringComparer::CurrentCultureIgnoreCase);
	this->SourceFile = nullptr;
}

DirectoryTreeNode::DirectoryTreeNode()
{
	this->Name = String::Empty;
	this->ParentPath = String::Empty;
	this->FullPath = String::Empty;
	this->Children = gcnew Dictionary<String^, DirectoryTreeNode^>(StringComparer::CurrentCultureIgnoreCase);
	this->SourceFile = nullptr;
}

System::String^ ArchiveReader::ReadNullTerminatedString(BinaryReader^ Stream)
{
	auto Sb = gcnew System::Text::StringBuilder;
	Char NextChar;
	while ((NextChar = Stream->ReadByte()) != '\0')
		Sb->Append(NextChar);

	return Sb->ToString();
}

System::String^ ArchiveReader::ReadString(BinaryReader^ Stream, UInt32 Length)
{
	auto Sb = gcnew System::Text::StringBuilder;

	for (int i = 0; i < Length; ++i)
		Sb->Append(static_cast<Char>(Stream->ReadByte()));

	return Sb->ToString();
}

Archive^ ArchiveReader::ParseArchive(String^ ArchiveFilePath)
{
	auto Out =gcnew Archive(ArchiveFilePath);

	BinaryReader^ Stream = nullptr;
	try
	{
		auto FileInfo = gcnew IO::FileInfo(ArchiveFilePath);
		auto Stream = gcnew BinaryReader(FileInfo->OpenRead());

		// file format - https://en.uesp.net/wiki/Oblivion_Mod:BSA_File_Format
		auto Magic = ReadNullTerminatedString(Stream);
		auto Version = Stream->ReadUInt32();

		if (Magic != "BSA" || Version != 0x67)
			throw gcnew ArgumentException("Not a valid TES4 BSA archive");

		// offset to the folder records
		Stream->ReadUInt32();

		Out->ArchiveFlags = static_cast<Archive::eArchiveFlags>(Stream->ReadUInt32());

		auto ArchiveFolderCount = Stream->ReadUInt32();
		auto ArchiveFileCount = Stream->ReadUInt32();

		// total folder stringtable length
		Stream->ReadUInt32();

		// total file stringstable length
		Stream->ReadUInt32();

		Out->ContentFlags = static_cast<Archive::eContentFlags>(Stream->ReadUInt32());

		// folder records start here
		// save the number of files for each folder to read them out later (in the same order)
		auto FolderFileCounts = gcnew List<UInt32>;
		for (int i = 0; i < ArchiveFolderCount; ++i)
		{
			// folder name hash
			Stream->ReadUInt64();

			// folder file count
			FolderFileCounts->Add(Stream->ReadUInt32());

			// offset to folder name/file records
			Stream->ReadUInt32();
		}

		// folder names and files start here
		for (int i = 0; i < ArchiveFolderCount; ++i)
		{
			auto FolderNameLength = Stream->ReadByte();	// including null-terminator
			auto FolderName = ReadString(Stream, FolderNameLength - 1);

			// ### what's this for?
			Stream->ReadByte();

			for (int j = 0, FileCount = FolderFileCounts[i]; j < FileCount; ++j)
			{
				// file name hash
				Stream->ReadUInt64();

				auto FileSize = Stream->ReadUInt32();
				auto FileCompressed = Out->ArchiveFlags.HasFlag(Archive::eArchiveFlags::Compressed);
				const UInt32 CompressionInvertedFlag = 1 << 30;

				if (FileSize & CompressionInvertedFlag)
				{
					FileCompressed = !FileCompressed;
					FileSize ^= ~CompressionInvertedFlag;
				}

				// file offset
				Stream->ReadUInt32();

				auto NewFile = gcnew ArchiveFile(FolderName, FileSize, FileCompressed);
				Out->Files->Add(NewFile);
			}
		}

		// filenames start here
		for (int i = 0; i < ArchiveFileCount; ++i)
		{
			auto ArchiveFile = Out->Files[i];
			ArchiveFile->Filename = ReadNullTerminatedString(Stream);
			ArchiveFile->FileType = ArchiveFile::GetFileTypeFromExtension(ArchiveFile->Extension);
		}

		Out->Valid = true;
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't parse BSA archive @ '" + ArchiveFilePath + "'!\n\tException: " + E->Message, true);
	}
	finally
	{
		if (Stream)
			Stream->Close();
	}

	return Out;
}

void ArchiveBrowser::DirectoryTreeListView_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::Escape:
		this->Close();
		E->Handled = true;
		E->SuppressKeyPress = true;
		break;
	case Keys::Back:
		if (BottomToolbarTextboxFilter->Text->Length >= 1)
		{
			BottomToolbarTextboxFilter->Text = BottomToolbarTextboxFilter->Text->Remove(BottomToolbarTextboxFilter->Text->Length - 1);
			DirectoryTreeListView->Focus();
		}

		E->Handled = true;
		break;
	}
}

void ArchiveBrowser::DirectoryTreeListView_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
{
	if ((E->KeyChar > 0x29 && E->KeyChar < 0x3A) ||
		(E->KeyChar > 0x60 && E->KeyChar < 0x7B))
	{
		BottomToolbarTextboxFilter->Text += E->KeyChar.ToString();
		DirectoryTreeListView->Focus();
		E->Handled = true;
	}
}

void ArchiveBrowser::DirectoryTreeListView_ItemActivate(Object^ Sender, EventArgs^ E)
{
	auto Selection = dynamic_cast<DirectoryTreeNode^>(DirectoryTreeListView->SelectedObject);
	if (Selection == nullptr)
		return;

	if (!Selection->IsFile)
		DirectoryTreeListView->Expand(Selection);
	else
		CompleteSelection();
}

void ArchiveBrowser::TopToolbarOpenArchive_Click(Object^ Sender, EventArgs^ E)
{
	auto OpenDlg = gcnew OpenFileDialog;
	OpenDlg->DefaultExt = "bsa";
	OpenDlg->Filter = "Bethesda Archive|*.bsa";
	OpenDlg->Title = "Select a TES 4 Oblivion BSA Archive";
	OpenDlg->RestoreDirectory = true;
	OpenDlg->ShowHelp = false;
	OpenDlg->InitialDirectory = DefaultDirectory;

	if (OpenDlg->ShowDialog() == Windows::Forms::DialogResult::OK && OpenDlg->FileName->Length > 0)
	{
		if (!LoadArchive(OpenDlg->FileName))
		{
			MessageBox::Show("Couldn't load the archive! Check the console for more information.",
							 this->Text, MessageBoxButtons::OK, MessageBoxIcon::Error);
		}
	}
}

void ArchiveBrowser::BottomToolbarOk_Click(Object^ Sender, EventArgs^ E)
{
	CompleteSelection();
}

void ArchiveBrowser::BottomToolbarCancel_Click(Object^ Sender, EventArgs^ E)
{
	this->Close();
}

void ArchiveBrowser::BottomToolbarTextboxFilter_TextChanged(Object^ Sender, EventArgs^ E)
{
	UpdateFilter(BottomToolbarTextboxFilter->Text);
}

void ArchiveBrowser::BottomToolbarTextboxFilter_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::Enter:
		CompleteSelection();
		return;
	case Keys::Back:
		return;
	}

	DirectoryTreeListView_KeyDown(DirectoryTreeListView, E);
}

void ArchiveBrowser::Dialog_Load(Object^ Sender, EventArgs^ E)
{
	TopToolbar->Focus();
	TopToolbarOpenArchive->Focus();

	TopToolbarOpenArchive->RaiseClick();
}

void ArchiveBrowser::InitializeComponent()
{
	this->components = (gcnew System::ComponentModel::Container());
	auto resources = (gcnew System::Resources::ResourceManager("BSAViewer.ArchiveBrowser", Assembly::GetExecutingAssembly()));
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

void ArchiveBrowser::FinalizeComponents()
{
	DirectoryTreeListViewColName->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ArchiveBrowser::DirectoryTreeAspectNameGetter);
	DirectoryTreeListViewColName->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&ArchiveBrowser::DirectoryTreeImageNamesGetter);

	DirectoryTreeListViewColSize->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ArchiveBrowser::DirectoryTreeAspectSizeGetter);
	DirectoryTreeListViewColSize->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&ArchiveBrowser::DirectoryTreeAspectToStringConverterSize);

	DirectoryTreeListViewColType->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ArchiveBrowser::DirectoryTreeAspectTypeGetter);
	DirectoryTreeListViewColType->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&ArchiveBrowser::DirectoryTreeAspectToStringConverterType);

	DirectoryTreeListView->CanExpandGetter = gcnew BrightIdeasSoftware::TreeListView::CanExpandGetterDelegate(&ArchiveBrowser::DirectoryTreeCanExpandGetter);
	DirectoryTreeListView->ChildrenGetter = gcnew BrightIdeasSoftware::TreeListView::ChildrenGetterDelegate(&ArchiveBrowser::DirectoryTreeChildrenGetter);

	DirectoryTreeListView->KeyDown += gcnew KeyEventHandler(this, &ArchiveBrowser::DirectoryTreeListView_KeyDown);
	DirectoryTreeListView->KeyPress += gcnew KeyPressEventHandler(this, &ArchiveBrowser::DirectoryTreeListView_KeyPress);
	DirectoryTreeListView->ItemActivate += gcnew EventHandler(this, &ArchiveBrowser::DirectoryTreeListView_ItemActivate);

	TopToolbarOpenArchive->Click += gcnew EventHandler(this, &ArchiveBrowser::TopToolbarOpenArchive_Click);
	BottomToolbarOk->Click += gcnew EventHandler(this, &ArchiveBrowser::BottomToolbarOk_Click);
	BottomToolbarCancel->Click += gcnew EventHandler(this, &ArchiveBrowser::BottomToolbarCancel_Click);
	BottomToolbarTextboxFilter->TextChanged += gcnew EventHandler(this, &ArchiveBrowser::BottomToolbarTextboxFilter_TextChanged);
	BottomToolbarTextboxFilter->KeyDown += gcnew KeyEventHandler(this, &ArchiveBrowser::BottomToolbarTextboxFilter_KeyDown);
	this->Load += gcnew System::EventHandler(this, &ArchiveBrowser::Dialog_Load);
}

void ArchiveBrowser::CompleteSelection()
{
	auto Selection = dynamic_cast<DirectoryTreeNode^>(DirectoryTreeListView->SelectedObject);
	if (Selection == nullptr || !Selection->IsFile)
	{
		MessageBox::Show("Please select a valid file to continue.",
						 this->Text, MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}

	SelectedFilePath = "Data\\" + Selection->FullPath;
	this->Close();
}

bool ArchiveBrowser::LoadArchive(String^ ArchiveFilePath)
{
	auto ParsedArchive = ArchiveReader::ParseArchive(ArchiveFilePath);
	bool IsUsableArchive = ParsedArchive->Valid;

	if (IsUsableArchive)
	{
		ActiveArchive = ParsedArchive;
		TopToolbarCurrentArchiveName->Text = Path::GetFileNameWithoutExtension(ActiveArchive->FilePath);

		PopulateDirectoryTree(BottomToolbarTextboxFilter->Text);
	}

	return IsUsableArchive;
}

void ArchiveBrowser::PopulateDirectoryTree(String^ Filter)
{
	if (ActiveArchive->Files->Count == 0)
	{
		DirectoryTreeListView->EmptyListMsg = "Archive contains no usable files";
		return;
	}

	DirectoryTreeListView->DeselectAll();
	DirectoryTreeListView->ClearObjects();

	DirectoryTreeData = ActiveArchive->GenerateDirectoryTree(Filter, FileTypeFilter);
	DirectoryTreeListView->SetObjects(DirectoryTreeData, true);
	DirectoryTreeListView->EmptyListMsg = "No files that match the current filter";

	UpdateFilter(BottomToolbarTextboxFilter->Text);
}


void ArchiveBrowser::UpdateFilter(String^ Filter)
{
	if (Filter->Length > 0)
	{
		DirectoryTreeListView->UseFiltering = true;
		DirectoryTreeListView->ShowFilterMenuOnRightClick = false;
		DirectoryTreeListView->ModelFilter = FilterPredicate;
		DirectoryTreeListView->ExpandAll();
	}
	else
	{
		DirectoryTreeListView->UseFiltering = false;
		DirectoryTreeListView->ModelFilter = nullptr;

		DirectoryTreeListView->CollapseAll();
		for each (auto Itr in DirectoryTreeData)
			DirectoryTreeListView->Expand(Itr);
	}
}

Object^ ArchiveBrowser::DirectoryTreeAspectNameGetter(Object^ RowObject)
{
	auto Model = safe_cast<DirectoryTreeNode^>(RowObject);
	return Model->Name;
}

Object^ ArchiveBrowser::DirectoryTreeAspectSizeGetter(Object^ RowObject)
{
	auto Model = safe_cast<DirectoryTreeNode^>(RowObject);
	return Model->IsFile ? Model->SourceFile->SizeInBytes : UInt32::MaxValue;
}

String^ ArchiveBrowser::DirectoryTreeAspectToStringConverterSize(Object^ Aspect)
{
	auto Size = safe_cast<UInt32>(Aspect);
	return Size != UInt32::MaxValue ? String::Format("{0:F2} KB", Size / 1024.f) : String::Empty;
}

Object^ ArchiveBrowser::DirectoryTreeAspectTypeGetter(Object^ RowObject)
{
	auto Model = safe_cast<DirectoryTreeNode^>(RowObject);
	if (Model->IsFile)
		return Model->SourceFile->FileType;
	else
		return nullptr;
}

String^ ArchiveBrowser::DirectoryTreeAspectToStringConverterType(Object^ Aspect)
{
	if (Aspect == nullptr)
		return "Folder";
	else
		return Aspect->ToString()->ToUpper();
}

Object^ ArchiveBrowser::DirectoryTreeImageNamesGetter(Object^ RowObject)
{
	auto Model = safe_cast<DirectoryTreeNode^>(RowObject);
	return Model->IsFile ? IconFile : IconFolder;
}

bool ArchiveBrowser::DirectoryTreeCanExpandGetter(Object^ RowObject)
{
	auto Model = safe_cast<DirectoryTreeNode^>(RowObject);
	return Model->Children->Count > 0;
}

System::Collections::IEnumerable^ ArchiveBrowser::DirectoryTreeChildrenGetter(Object^ RowObject)
{
	auto Model = safe_cast<DirectoryTreeNode^>(RowObject);
	return Model->Children->Values;
}

bool ArchiveBrowser::DirectoryTreeFilterPredicate(Object ^ Model)
{
	auto Node = safe_cast<DirectoryTreeNode^>(Model);
	return BottomToolbarTextboxFilter->Text->Length == 0 || Node->FullPath->IndexOf(BottomToolbarTextboxFilter->Text, StringComparison::CurrentCultureIgnoreCase) != -1;
}

ArchiveBrowser::ArchiveBrowser(String^ DefaultDirectory, ArchiveFile::eFileType FileTypeFilter)
{
	this->DefaultDirectory = DefaultDirectory;
	this->FileTypeFilter = FileTypeFilter;
	this->ActiveArchive = nullptr;
	this->SelectedFilePath = String::Empty;

	DirectoryTreeData = nullptr;
	FilterPredicate = gcnew BrightIdeasSoftware::ModelFilter(gcnew Predicate<Object^>(this, &ArchiveBrowser::DirectoryTreeFilterPredicate));

	InitializeComponent();
	FinalizeComponents();

	if (IconFolder == nullptr || IconFile == nullptr)
	{
		auto Resources = gcnew System::Resources::ResourceManager("BSAViewer.Images", Assembly::GetExecutingAssembly());
		IconFolder = safe_cast<System::Drawing::Image^>(Resources->GetObject(L"Folder"));
		IconFile = safe_cast<System::Drawing::Image^>(Resources->GetObject(L"File"));
	}

	this->ShowDialog();
}

ArchiveBrowser::~ArchiveBrowser()
{
	FilterPredicate = nullptr;

	if (components)
	{
		delete components;
	}
}


} // namespace bsaViewer


} // namespace cse
