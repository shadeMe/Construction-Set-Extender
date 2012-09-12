#include "BSAViewer.h"
#include "BSAReader.h"
#include "[Common]\ListViewUtilities.h"

namespace ConstructionSetExtender
{
	String^ FileSystemObject::GetPath(bool WithRoot)
	{
		String^ Path;
		FolderObject^ ParentObject = Parent;

		while (ParentObject != FSONULL && ParentObject != BSAViewer::Root)
		{
			if (!WithRoot && ParentObject->Parent == BSAViewer::Root)	break;

			Path = "\\" + ParentObject->Name + Path;
			ParentObject = ParentObject->Parent;
		}

		if (Path == nullptr)		return "";
		else						return Path->Substring(1);
	}

	BSAViewer::BSAViewer()
	{
		Application::EnableVisualStyles();

		Root = gcnew FolderObject(gcnew String("__ROOT"), FSONULL);
		ContentList= gcnew ListView();
		BSABox = gcnew Form();
		LargeIcons = gcnew ImageList();
		SmallIcons = gcnew ImageList();
		StatusBar = gcnew StatusStrip();
		OpenDialog = gcnew OpenFileDialog();
		ToolBar = gcnew ToolStrip();
		SeparatorA = gcnew ToolStripSeparator();
		ToolBarOpenArchive = gcnew ToolStripButton();
		ToolBarUp = gcnew ToolStripButton();
		ToolBarView = gcnew ToolStripButton();
		ToolBarArchiveName = gcnew ToolStripLabel();
		CurrentLocation = gcnew ToolStripLabel();

		LargeIcons->ImageSize = Size(48, 48);
		LargeIcons->ColorDepth = ColorDepth::Depth32Bit;
		SmallIcons->ImageSize = Size(20, 20);
		SmallIcons->ColorDepth = ColorDepth::Depth32Bit;

		WorkingDirectory = FSONULL;
		ResourceManager^ ImageResources = gcnew ResourceManager("CSEBSAViewer.Images", Assembly::GetExecutingAssembly());

		OpenDialog->DefaultExt = "bsa";
		OpenDialog->Filter = "Bethesda Archive|*.bsa";
		OpenDialog->Title = "Select a TES 4 Oblivion BSA Archive";
		OpenDialog->RestoreDirectory = true;
		OpenDialog->ShowHelp = true;

		ToolBar->GripStyle = ToolStripGripStyle::Hidden;
		ToolBar->Location = Point(0, 0);
		ToolBar->Size = Size(0x278, 0x19);
		ToolBar->Dock = DockStyle::Top;
		ToolBar->Items->Add(ToolBarOpenArchive);
		ToolBar->Items->Add(SeparatorA);
		ToolBar->Items->Add(ToolBarUp);
		ToolBar->Items->Add(ToolBarView);
		ToolBar->Items->Add(ToolBarArchiveName);

		SeparatorA->Size = Size(6, 0x19);

		ContentList->Dock = DockStyle::Fill;
		ContentList->Location = System::Drawing::Point(0, 0x1a);
		ContentList->Size = System::Drawing::Size(0x278, 0x13f);
		ContentList->UseCompatibleStateImageBehavior = false;
		ContentList->View = View::LargeIcon;
		ContentList->MultiSelect = false;
		ContentList->CheckBoxes = false;
		ContentList->FullRowSelect = true;
		ContentList->LargeImageList = LargeIcons;
		ContentList->SmallImageList = SmallIcons;
		ContentList->HideSelection = false;
		ContentList->ItemActivate += gcnew EventHandler(this, &BSAViewer::ContentList_ItemActivate);

		StatusBar->Dock = DockStyle::Bottom;
		StatusBar->Items->Add(CurrentLocation);

		CurrentLocation->AutoSize = true;
		CurrentLocation->Image = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("StatusBarCurrentLocation")));
		CurrentLocation->Text = "";

		ToolBarOpenArchive->Image = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("TBOpen")));
		ToolBarOpenArchive->AutoSize = true;
		ToolBarOpenArchive->ToolTipText = "Open Archive";
		ToolBarOpenArchive->Text = "Open Archive";
		ToolBarOpenArchive->DisplayStyle = ToolStripItemDisplayStyle::ImageAndText;
		ToolBarOpenArchive->Click += gcnew EventHandler(this, &BSAViewer::ToolBarOpenArchive_Click);

		ToolBarUp->Image = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("TBUp")));
		ToolBarUp->AutoSize = true;
		ToolBarUp->ToolTipText = "Go Up One Level";
		ToolBarUp->Text = "Go Up One Level";
		ToolBarUp->DisplayStyle = ToolStripItemDisplayStyle::ImageAndText;
		ToolBarUp->Click += gcnew EventHandler(this, &BSAViewer::ToolBarUp_Click);
		ToolBarUp->Enabled = false;

		ToolBarView->Image = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("TBView")));
		ToolBarView->AutoSize = true;
		ToolBarView->ToolTipText = "Change View";
		ToolBarView->Text = "Change View";
		ToolBarView->DisplayStyle = ToolStripItemDisplayStyle::ImageAndText;
		ToolBarView->Click += gcnew EventHandler(this, &BSAViewer::ToolBarView_Click);

		ToolBarArchiveName->AutoSize = true;
		ToolBarArchiveName->Text = "";
		ToolBarArchiveName->Alignment = ToolStripItemAlignment::Right;

		BSABox = gcnew Form();
		BSABox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		BSABox->AutoScaleMode = AutoScaleMode::Font;
		BSABox->ClientSize = System::Drawing::Size(632, 371);
		BSABox->Controls->Add(ContentList);
		BSABox->Controls->Add(ToolBar);
		BSABox->Controls->Add(StatusBar);
		BSABox->FormBorderStyle = FormBorderStyle::Sizable;
		BSABox->MaximizeBox = false;
		BSABox->MinimizeBox = false;
		BSABox->StartPosition = FormStartPosition::CenterScreen;
		BSABox->Text = L"BSA Browser";
		BSABox->Closing += gcnew CancelEventHandler(this, &BSAViewer::BSABox_Cancel);
		BSABox->Hide();

		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("FOLDER"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("FILE"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("DDS"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("NIF"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SOUND"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SOUND"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("LIP"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("KF"))));
		LargeIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SPT"))));

		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("FOLDER"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("FILE"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("DDS"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("NIF"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SOUND"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SOUND"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("LIP"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("KF"))));
		SmallIcons->Images->Add(gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SPT"))));
	}

	BSAViewer^% BSAViewer::GetSingleton()
	{
		if (Singleton == nullptr)
		{
			Singleton = gcnew BSAViewer();
		}
		return Singleton;
	}

	FolderObject^ BSAViewer::GetFolderInDirectory(FolderObject^% WorkingDirectory, String^% FolderName)
	{
		UInt32 Count = 0;
		for each (FileSystemObject^ Itr in WorkingDirectory->SubItems)
		{
			if (Itr->ObjectType == FileSystemObject::_ObjectType::e_Folder && !String::Compare(Itr->Name, FolderName))
			{
				return dynamic_cast<FolderObject^>(WorkingDirectory->SubItems[Count]);
			}
			Count++;
		}
		return FSONULL;
	}

	void BSAViewer::ParseBSAFilePath(String^% Path)
	{
		array<String^>^ Splits = Path->Split((gcnew String("\\"))->ToCharArray());
		FolderObject^ WorkingDirectory = Root;

		UInt32 Count = 0, Tokens = Splits->Length - 1;
		if (String::Compare(Splits[Tokens]->Substring(Splits[Tokens]->IndexOf(".") + 1), Filter))		return;

		for each (String^ Itr in Splits)
		{
			if (Count == Tokens)
			{							// last token = filename
				WorkingDirectory->SubItems->Add(gcnew FileObject(Itr, WorkingDirectory));
			}
			else
			{
				if (GetFolderInDirectory(WorkingDirectory, Itr) == FSONULL)
				{
					WorkingDirectory->SubItems->Add(gcnew FolderObject(Itr, WorkingDirectory));
					WorkingDirectory = dynamic_cast<FolderObject^>(WorkingDirectory->SubItems[WorkingDirectory->SubItems->Count - 1]);
				}
				else
					WorkingDirectory = GetFolderInDirectory(WorkingDirectory, Itr);
			}
			Count++;
		}
	}

	void BSAViewer::PopulateContentList(FolderObject^% WorkingDirectory)
	{
		ContentList->BeginUpdate();
		ContentList->Clear();
		for each (FileSystemObject^ Itr in WorkingDirectory->SubItems)
		{
			UInt32 ImageIndex = 0;
			if (Itr->ObjectType != FileSystemObject::_ObjectType::e_Folder)
			{
				ImageIndex = (int)(dynamic_cast<FileObject^>(Itr))->FileType + 1;
			}

			ListViewItem^ Item = gcnew ListViewItem(Itr->Name, ImageIndex);
			Item->Tag = Itr;
			ContentList->Items->Add(Item);
		}
		ContentList->EndUpdate();

		ToolBarUp->Enabled = true;
		if (WorkingDirectory != Root)		CurrentLocation->Text = WorkingDirectory->GetPathWithName(true);
		else								CurrentLocation->Text = "root";

		this->WorkingDirectory = WorkingDirectory;

		ContentList->Sorting = SortOrder::Ascending;
		ContentList->ListViewItemSorter = gcnew ListViewStringSorter(0, ContentList->Sorting);
		ContentList->Sort();
	}

	void BSAViewer::Close()
	{
		Cleanup();
		BSABox->Hide();
	}

	void BSAViewer::Cleanup()
	{
		ContentList->Clear();
		BSAR->CloseArchive();
		Root->SubItems->Clear();
		CurrentLocation->Text = "";
		WorkingDirectory = FSONULL;
	}

	String^ BSAViewer::InitializeViewer(String^% DefaultDirectory, String^% Filter)
	{
		this->Filter = Filter;
		OpenDialog->InitialDirectory = DefaultDirectory + "Data";
		ToolBarArchiveName->Text = "";
		BSABox->ShowDialog();

		if (ReturnPath != "__NULL")
			return ReturnPath;
		else
			return "";
	}

	void BSAViewer::OpenArchive(String^% Path)
	{
		if (!BSAR->OpenArchive(Path))	return;

		for each (BSAFileInfo^% Itr in BSAR->GetFiles())
		{
			ParseBSAFilePath(Itr->GetRelativePath());
		}

		PopulateContentList(Root);
		ToolBarUp->Enabled = false;
	}

	void BSAViewer::SanitizeReturnPath()
	{
		if (ReturnPath != "__NULL")
		{
			if (Filter == "kf")
			{
				int Offset = ReturnPath->IndexOf("IdleAnims\\", StringComparison::CurrentCultureIgnoreCase);
				if (Offset != -1)
					ReturnPath = ReturnPath->Substring(Offset + 9);
			}
			else if (Filter == "dds" && ReturnPath->IndexOf("menus\\icons\\", StringComparison::CurrentCultureIgnoreCase) == 0)
			{
				ReturnPath = ReturnPath->Substring(12);
			}
			else if (Filter == "spt")
			{
				ReturnPath = "\\" + ReturnPath;
			}
		}
	}

	void BSAViewer::BSABox_Cancel(Object^ Sender, CancelEventArgs^ E)
	{
		if (BSABox->DialogResult == DialogResult::Cancel)
			ReturnPath = "__NULL";
		Close();
	}

	void BSAViewer::ContentList_ItemActivate(Object^ Sender, EventArgs^ E)
	{
		ListViewItem^ Item = GetListViewSelectedItem(ContentList);
		if (Item == nullptr)					return;

		FileSystemObject^ FSO = dynamic_cast<FileSystemObject^>(Item->Tag);
		if (FSO->ObjectType == FileSystemObject::_ObjectType::e_Folder)
		{
			PopulateContentList(dynamic_cast<FolderObject^>(FSO));
		}
		else
		{
			ReturnPath = (dynamic_cast<FileObject^>(FSO))->GetPathWithName(false);
			SanitizeReturnPath();
			Close();
		}
	}

	void BSAViewer::ToolBarView_Click(Object^ Sender, EventArgs^ E)
	{
		if (ContentList->View == View::LargeIcon)		ContentList->View = View::List;
		else											ContentList->View = View::LargeIcon;
	}

	void BSAViewer::ToolBarUp_Click(Object^ Sender, EventArgs^ E)
	{
		if (WorkingDirectory == FSONULL)	return;

		if (WorkingDirectory->Parent != FSONULL)
		{
			PopulateContentList(dynamic_cast<FolderObject^>(WorkingDirectory->Parent));
		}

		if (WorkingDirectory->Parent == FSONULL)	ToolBarUp->Enabled = false;
	}

	void BSAViewer::ToolBarOpenArchive_Click(Object^ Sender, EventArgs^ E)
	{
		if (OpenDialog->ShowDialog() == DialogResult::OK && OpenDialog->FileName->Length > 0)
		{
			Cleanup();
			OpenArchive(OpenDialog->FileName);
			FileInfo^ FI = gcnew FileInfo(OpenDialog->FileName);
			ToolBarArchiveName->Text = FI->Name;
		}
	}
}