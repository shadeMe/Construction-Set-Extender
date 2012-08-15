#include "TagBrowser.h"
#include "[Common]\ListViewUtilities.h"
#include "[Common]\NativeWrapper.h"

#using "Microsoft.VisualBasic.dll"

namespace ConstructionSetExtender
{
	void DragonDropForm::WndProc( Message% m )
	{
		switch(m.Msg)
		{
		case 0x407:
			NativeWrapper::g_CSEInterfaceTable->TagBrowser.InitiateDragonDrop();
			return;
		}

		Form::WndProc(m);
	}

	TagDatabase::TagDatabase(AdvTree::AdvTree^ Tree)
	{
		Database = gcnew Dictionary<AdvTree::Node^, List<String^>^>();
		ParentTree = Tree;
	}
	void TagDatabase::AddTag(AdvTree::Node^ ParentTag, String^% Name)
	{
		AdvTree::Node^ NewNode = gcnew AdvTree::Node();
		NewNode->Text = Name;

		if (ParentTag != nullptr)
			ParentTag->Nodes->Add(NewNode);
		else
			ParentTree->Nodes->Add(NewNode);

		Database->Add(NewNode, gcnew List<String^>());
	}
	void TagDatabase::RemoveTag(AdvTree::Node^ Tag)
	{
		for (int i = 0; i < Tag->Nodes->Count; i++)
			RemoveTag(Tag->Nodes[i]);

		ParentTree->BeginUpdate();

		List<String^>^ TagItems = nullptr;
		if (GetItemsForTag(Tag, TagItems))
		{
			TagItems->Clear();
			Database->Remove(Tag);

			AdvTree::Node^ Parent = Tag->Parent;
			if (Parent != nullptr)
				Parent->Nodes->Remove(Tag);
			else
				ParentTree->Nodes->Remove(Tag);
		}

		ParentTree->DeselectNode(nullptr, AdvTree::eTreeAction::Code);
		ParentTree->EndUpdate();
	}
	bool TagDatabase::TagItem(AdvTree::Node^ Tag, ComponentDLLInterface::FormData* Data)
	{
		if (GetItemExistsInTag(Tag,  gcnew String(Data->EditorID)) == false)
		{
			List<String^>^ TagItems = nullptr;
			if (GetItemsForTag(Tag, TagItems))
				TagItems->Add(gcnew String(Data->EditorID));

			return true;
		}
		else
		{
			MessageBox::Show("Item '"+ gcnew String(Data->EditorID) + "' has already been tagged with '" + Tag->Text + "'.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
			return false;
		}
	}
	bool TagDatabase::TagItem(String^% TagName, ComponentDLLInterface::FormData* Data)
	{
		AdvTree::Node^ Tag = ParentTree->FindNodeByText(TagName);
		return TagItem(Tag, Data);
	}
	void TagDatabase::UntagItem(AdvTree::Node^ Tag, String^% Name)
	{
		List<String^>^ TagItems = nullptr;
		if (GetItemsForTag(Tag, TagItems))
		{
			UInt32 Index = -1, Count = 0;
			for each (String^ Itr in TagItems)
			{
				if (!String::Compare(Itr, Name, true))
				{
					Index = Count;
					break;
				}
				Count++;
			}

			if (Index != -1)
				TagItems->RemoveAt(Index);
		}
	}
	bool TagDatabase::GetItemExistsInTag(AdvTree::Node^ Tag, String^% Name)
	{
		List<String^>^ TagItems = nullptr;
		if (GetItemsForTag(Tag, TagItems))
		{
			for each (String^ Itr in TagItems)
			{
				if (String::Compare(Itr, Name, true) == 0)
					return true;
			}
		}

		return false;
	}
	bool TagDatabase::GetTagExists(String^% Name)
	{
		for each (KeyValuePair<AdvTree::Node^, List<String^>^>% Itr in Database)
		{
			if (!String::Compare(Itr.Key->Text, Name, true))
				return true;
		}
		return false;
	}
	void TagDatabase::Clear(void)
	{
		for each (KeyValuePair<AdvTree::Node^, List<String^>^>% Itr in Database)
			Itr.Value->Clear();

		Database->Clear();
	}
	String^ TagDatabase::SerializeDatabase(void)
	{
		StringWriter^ TextParser = gcnew StringWriter();
		ParentTree->Save(TextParser);
		String^ SerializedData = TextParser->ToString();

		SerializedData += "\n\n" + SerializedDataMidWayMarker + "\n\n";

		for each (KeyValuePair<AdvTree::Node^, List<String^>^>% Itr in Database)
		{
			SerializedData += Itr.Key->Text;
			for each (String^ ItrItem in Itr.Value)
				SerializedData += ";" + ItrItem;

			if (SerializedData[SerializedData->Length - 1] != ';')
				SerializedData += ";";

			SerializedData += "\n";
		}

		return SerializedData;
	}
	bool TagDatabase::DeserializeDatabase(String^ SerializedData)
	{
		try
		{
			int MarkerIndex = SerializedData->IndexOf(SerializedDataMidWayMarker);
			if (MarkerIndex == -1)
				throw gcnew CSEGeneralException("Serialization marker not found.");

			Clear();

			StringReader^ TextParser = gcnew StringReader(SerializedData->Substring(0, MarkerIndex));
			ParentTree->Load(TextParser);

			for each (AdvTree::Node^ Itr in ParentTree->Nodes)
				AddTagNodeToDatabase(Itr);

			StringReader^ DatabaseParser = gcnew StringReader(SerializedData->Substring(MarkerIndex));
			String^ ReadLine = DatabaseParser->ReadLine();
			bool BadItems = false;
			for (ReadLine = DatabaseParser->ReadLine(); ReadLine != nullptr; ReadLine = DatabaseParser->ReadLine())
			{
				if (ReadLine == "")
					continue;

				array<String^>^ Tokens = ReadLine->Split((gcnew String(";"))->ToCharArray());
				String^ Tag = Tokens[0];

				for (int i = 1; i < Tokens->Length; i++)
				{
					String^ Token = Tokens[i];
					if (Token != "")
					{
						ComponentDLLInterface::FormData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupFormByEditorID((CString(Token)).c_str());
						if (Data)
							TagItem(Tag, Data);
						else
						{
							BadItems = true;
							DebugPrint("Couldn't find form '" + Token + "'");
						}
						NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);
					}
				}
			}

			if (BadItems)
				MessageBox::Show("Some forms were found missing while populating the tag list.\n\nDetails logged to the console.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);

			return true;
		}
		catch (Exception^ E)
		{
			DebugPrint("Tag database parser Error - " + E->Message);
			return false;
		}
	}
	void TagDatabase::AddTagNodeToDatabase(AdvTree::Node^ Tag)
	{
		Database->Add(Tag, gcnew List<String^>());

		for each(AdvTree::Node^ Itr in Tag->Nodes)
			AddTagNodeToDatabase(Itr);
	}

	TagBrowser^% TagBrowser::GetSingleton()
	{
		if (Singleton == nullptr)
			Singleton = gcnew TagBrowser();
		return Singleton;
	}

	TagBrowser::TagBrowser()
	{
		Application::EnableVisualStyles();

		TagBrowserBox = gcnew DragonDropForm();
		SearchBox = (gcnew TextBox());
		FormList = (gcnew ListView());
		FormListHEditorID = (gcnew ColumnHeader());
		FormListHRefID = (gcnew ColumnHeader());
		FormListHType = (gcnew ColumnHeader());
		FormListContextMenu = (gcnew ContextMenuStrip());
		FormListContextMenuRemove = (gcnew ToolStripMenuItem());

		TagTree = (gcnew DevComponents::AdvTree::AdvTree());
		TagTreeContextMenu = (gcnew ContextMenuStrip());
		TagTreeContextMenuAdd = (gcnew ToolStripMenuItem());
		TagTreeContextMenuRemove = (gcnew ToolStripMenuItem());
		TagTreeNodeConnector = (gcnew DevComponents::AdvTree::NodeConnector());
		TagTreeElementStyle2 = (gcnew DevComponents::DotNetBar::ElementStyle());
		TagTreeElementStyle1 = (gcnew DevComponents::DotNetBar::ElementStyle());
		SaveTags = (gcnew Button());
		LoadTags = (gcnew Button());
		FormListContextMenu->SuspendLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TagTree))->BeginInit();
		TagTreeContextMenu->SuspendLayout();
		TagBrowserBox->SuspendLayout();
		//
		// SearchBox
		//
		SearchBox->Location = System::Drawing::Point(310, 624);
		//	SearchBox->Multiline = true;
		SearchBox->Name = L"SearchBox";
		SearchBox->Size = System::Drawing::Size(554, 33);
		SearchBox->TabIndex = 1;
		SearchBox->TextChanged += gcnew EventHandler(this, &TagBrowser::SearchBox_TextChanged);
		SearchBox->Font = gcnew Font("Consolas", 14.25F, FontStyle::Regular);
		//
		// FormList
		//
		FormList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(3) {FormListHEditorID,
			FormListHRefID, FormListHType});
		FormList->ContextMenuStrip = FormListContextMenu;
		FormList->FullRowSelect = true;
		FormList->Location = System::Drawing::Point(310, 12);
		FormList->Name = L"FormList";
		FormList->Size = System::Drawing::Size(646, 606);
		FormList->TabIndex = 2;
		FormList->UseCompatibleStateImageBehavior = false;
		FormList->View = View::Details;
		FormList->Tag = (int)1;
		FormList->HideSelection = false;
		FormList->ItemActivate += gcnew EventHandler(this, &TagBrowser::FormSelectionList_ItemActivate);
		FormList->ColumnClick += gcnew ColumnClickEventHandler(this, &TagBrowser::FormSelectionList_ColumnClick);
		FormList->MouseDown += gcnew MouseEventHandler(this, &TagBrowser::FormList_MouseDown);
		//	FormList->MouseUp += gcnew MouseEventHandler(this, &TagBrowser::FormList_MouseUp);
		//
		// FormListHEditorID
		//
		FormListHEditorID->Text = L"EditorID";
		FormListHEditorID->Width = 482;
		//
		// FormListHRefID
		//
		FormListHRefID->Text = L"FormID";
		FormListHRefID->Width = 64;
		//
		// FormListHType
		//
		FormListHType->Text = L"Type";
		FormListHType->Width = 77;
		//
		// FormListContextMenu
		//
		FormListContextMenu->Items->AddRange(gcnew cli::array< ToolStripItem^  >(1) {FormListContextMenuRemove});
		FormListContextMenu->Name = L"FormListContextMenu";
		FormListContextMenu->Size = Size(165, 26);
		FormListContextMenu->ShowImageMargin = false;

		FormListContextMenuRemove->Name = L"FormListContextMenuRemove";
		FormListContextMenuRemove->Size = Size(164, 22);
		FormListContextMenuRemove->Text = L"Remove From Tag";
		FormListContextMenuRemove->Click += gcnew EventHandler(this, &TagBrowser::FormListContextMenuRemove_Click);
		//
		// TagTree
		//
		TagTree->AccessibleRole = AccessibleRole::Outline;
		TagTree->AllowDrop = true;
		TagTree->BackColor = SystemColors::Window;
		TagTree->BackgroundStyle->Class = L"TreeBorderKey";
		TagTree->CellEdit = true;
		TagTree->ColorSchemeStyle = DevComponents::AdvTree::eColorSchemeStyle::VS2005;
		TagTree->ColumnsVisible = false;
		TagTree->ContextMenuStrip = TagTreeContextMenu;
		TagTree->Cursor = Cursors::Arrow;
		TagTree->GridRowLines = true;
		TagTree->HideSelection = true;
		TagTree->HotTracking = true;
		TagTree->Location = System::Drawing::Point(9, 12);
		TagTree->Name = L"TagTree";
		TagTree->NodesConnector = TagTreeNodeConnector;
		TagTree->NodeSpacing = 4;
		TagTree->NodeStyle = TagTreeElementStyle2;
		TagTree->PathSeparator = L";";
		TagTree->SelectionBoxStyle = DevComponents::AdvTree::eSelectionStyle::NodeMarker;
		TagTree->SelectionPerCell = true;
		TagTree->Size = System::Drawing::Size(295, 645);
		TagTree->Styles->Add(TagTreeElementStyle1);
		TagTree->Styles->Add(TagTreeElementStyle2);
		TagTree->TabIndex = 4;
		TagTree->Text = L"Tags";
		TagTree->NodeClick += gcnew AdvTree::TreeNodeMouseEventHandler(this, &TagBrowser::TagTree_NodeClick);
		TagTree->AfterCellEdit += gcnew AdvTree::CellEditEventHandler(this, &TagBrowser::TagTree_AfterCellEdit);

		//
		// TagTreeContextMenu
		//
		TagTreeContextMenu->Items->AddRange(gcnew cli::array< ToolStripItem^  >(2) {TagTreeContextMenuAdd,
			TagTreeContextMenuRemove});
		TagTreeContextMenu->Name = L"TagTreeContextMenu";
		TagTreeContextMenu->Size = Size(141, 48);
		TagTreeContextMenu->ShowImageMargin = false;
		//
		// TagTreeContextMenuAdd
		//
		TagTreeContextMenuAdd->Name = L"TagTreeContextMenuAdd";
		TagTreeContextMenuAdd->Size = Size(140, 22);
		TagTreeContextMenuAdd->Text = L"Add Tag";
		TagTreeContextMenuAdd->Click += gcnew EventHandler(this, &TagBrowser::TagTreeContextMenuAdd_Click);
		//
		// TagTreeContextMenuRemove
		//
		TagTreeContextMenuRemove->Name = L"TagTreeContextMenuRemove";
		TagTreeContextMenuRemove->Size = Size(140, 22);
		TagTreeContextMenuRemove->Text = L"Remove Tag";
		TagTreeContextMenuRemove->Click += gcnew EventHandler(this, &TagBrowser::TagTreeContextMenuRemove_Click);
		//
		// TagTreeNodeConnector
		//
		TagTreeNodeConnector->DashStyle = Drawing2D::DashStyle::Dot;
		TagTreeNodeConnector->LineColor = SystemColors::ControlText;
		//
		// TagTreeElementStyle2
		//
		TagTreeElementStyle2->BackColor = Color::White;
		TagTreeElementStyle2->BackColor2 = Color::FromArgb(static_cast<System::Int32>(static_cast<System::Byte>(228)),
			static_cast<System::Int32>(static_cast<System::Byte>(228)), static_cast<System::Int32>(static_cast<System::Byte>(240)));
		TagTreeElementStyle2->BackColorGradientAngle = 90;
		TagTreeElementStyle2->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::Solid;
		TagTreeElementStyle2->BorderBottomWidth = 1;
		TagTreeElementStyle2->BorderColor = Color::DarkGray;
		TagTreeElementStyle2->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::Solid;
		TagTreeElementStyle2->BorderLeftWidth = 1;
		TagTreeElementStyle2->BorderRight = DevComponents::DotNetBar::eStyleBorderType::Solid;
		TagTreeElementStyle2->BorderRightWidth = 1;
		TagTreeElementStyle2->BorderTop = DevComponents::DotNetBar::eStyleBorderType::Solid;
		TagTreeElementStyle2->BorderTopWidth = 1;
		TagTreeElementStyle2->CornerDiameter = 4;
		TagTreeElementStyle2->Description = L"Gray";
		TagTreeElementStyle2->Name = L"TagTreeElementStyle2";
		TagTreeElementStyle2->PaddingBottom = 1;
		TagTreeElementStyle2->PaddingLeft = 1;
		TagTreeElementStyle2->PaddingRight = 1;
		TagTreeElementStyle2->PaddingTop = 1;
		TagTreeElementStyle2->TextColor = Color::Black;
		//
		// TagTreeElementStyle1
		//
		TagTreeElementStyle1->Name = L"TagTreeElementStyle1";
		TagTreeElementStyle1->TextColor = SystemColors::ControlText;
		//
		// SaveTags
		//
		SaveTags->BackColor = System::Drawing::SystemColors::Control;
		SaveTags->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0)));
		SaveTags->ForeColor = System::Drawing::Color::Black;
		SaveTags->Location = System::Drawing::Point(870, 624);
		SaveTags->Name = L"SaveTags";
		SaveTags->Size = System::Drawing::Size(40, 33);
		SaveTags->TabIndex = 19;
		SaveTags->Text = L"Save";
		SaveTags->UseVisualStyleBackColor = false;
		SaveTags->Click += gcnew EventHandler(this, &TagBrowser::SaveTags_Click);
		//
		// LoadTags
		//
		LoadTags->BackColor = System::Drawing::SystemColors::Control;
		LoadTags->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(0)));
		LoadTags->ForeColor = System::Drawing::Color::Black;
		LoadTags->Location = System::Drawing::Point(916, 624);
		LoadTags->Name = L"LoadTags";
		LoadTags->Size = System::Drawing::Size(40, 33);
		LoadTags->TabIndex = 20;
		LoadTags->Text = L"Load";
		LoadTags->UseVisualStyleBackColor = false;
		LoadTags->Click += gcnew EventHandler(this, &TagBrowser::LoadTags_Click);
		//
		// TagBrowser
		//
		TagBrowserBox->AutoScaleDimensions = SizeF(6, 13);
		TagBrowserBox->AutoScaleMode = AutoScaleMode::Font;
		TagBrowserBox->ClientSize = Size(968, 669);
		TagBrowserBox->Controls->Add(LoadTags);
		TagBrowserBox->Controls->Add(SaveTags);
		TagBrowserBox->Controls->Add(TagTree);
		TagBrowserBox->Controls->Add(FormList);
		TagBrowserBox->Controls->Add(SearchBox);
		TagBrowserBox->FormBorderStyle = FormBorderStyle::FixedSingle;
		TagBrowserBox->MaximizeBox = false;
		TagBrowserBox->Name = L"TagBrowser";
		TagBrowserBox->StartPosition = FormStartPosition::CenterScreen;
		TagBrowserBox->Text = L"Tag Browser";
		TagBrowserBox->KeyPreview = true;
		TagBrowserBox->Closing += gcnew CancelEventHandler(this, &TagBrowser::TagBrowserBox_Cancel);
		FormListContextMenu->ResumeLayout(false);
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TagTree))->EndInit();
		TagTreeContextMenu->ResumeLayout(false);
		TagBrowserBox->ResumeLayout(false);
		TagBrowserBox->PerformLayout();

		TagBrowserBox->Hide();

		Database = gcnew TagDatabase(TagTree);
		MouseDragInProgress = false;
		GlobalMouseHook_MouseUpHandler = gcnew MouseEventHandler(this, &TagBrowser::GlobalInputMonitor_MouseUp);
	}

	void TagBrowser::GlobalInputMonitor_MouseUp(Object^ Sender, MouseEventArgs^ E)
	{
		HookManager::MouseUp -= GlobalMouseHook_MouseUpHandler;

		switch (E->Button)
		{
		case MouseButtons::Right:
			if (MouseDragInProgress)
			{
				DebugPrint("PlaceObject Operation interrupted by right mouse button");
			}
			break;
		case MouseButtons::Left:
			HookManager::MouseUp -= GlobalMouseHook_MouseUpHandler;
			if (MouseDragInProgress)
			{
				IntPtr Window = NativeWrapper::WindowFromPoint(E->Location);
				if (Window != GetFormListHandle() && Window != GetWindowHandle())
				{
					ComponentDLLInterface::TagBrowserInstantiationData InteropData;
					ComponentDLLInterface::FormData* Data = InteropData.FormListHead = new ComponentDLLInterface::FormData[FormList->SelectedItems->Count];
					InteropData.FormCount = FormList->SelectedItems->Count;
					InteropData.InsertionPoint = E->Location;

					UInt32 Index = 0;
					for each (ListViewItem^ Itr in FormList->SelectedItems)
					{
						Data[Index].FormID = UInt32::Parse(Itr->SubItems[1]->Text, Globalization::NumberStyles::HexNumber);
						Index++;
					}

					NativeWrapper::g_CSEInterfaceTable->TagBrowser.InstantiateObjects(&InteropData);
				}
			}
			else
				DebugPrint("Global PlaceObject hook called out of turn! Expecting an unresolved operation.");

			break;
		}

		MouseDragInProgress = false;
	}

	void TagBrowser::TagBrowserBox_Cancel(Object^ Sender, CancelEventArgs^ E)
	{
		Hide();
		E->Cancel = true;
	}

	void TagBrowser::FormList_MouseDown(Object^ Sender, MouseEventArgs^ E)
	{
		if (FormList->SelectedItems->Count > 0)
		{
			MouseDragInProgress = true;
			HookManager::MouseUp += GlobalMouseHook_MouseUpHandler;
		}
	}
	void TagBrowser::FormSelectionList_ItemActivate(Object^ Sender, EventArgs^ E)
	{
		ListView^ List = dynamic_cast<ListView^>(Sender);

		ListViewItem^ Selected = GetListViewSelectedItem(List);
		if (Selected != nullptr)
		{
			NativeWrapper::g_CSEInterfaceTable->EditorAPI.LoadFormForEdit((CString(Selected->Text)).c_str());
		}
	}
	void TagBrowser::FormSelectionList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
	{
		ListView^ List = dynamic_cast<ListView^>(Sender);

		if (E->Column != (int)List->Tag)
		{
			List->Tag = E->Column;
			List->Sorting = SortOrder::Ascending;
		}
		else
		{
			if (List->Sorting == SortOrder::Ascending)
				List->Sorting = SortOrder::Descending;
			else
				List->Sorting = SortOrder::Ascending;
		}

		List->Sort();
		System::Collections::IComparer^ Sorter;
		switch (E->Column)
		{
		case 1:							// FormID
			Sorter = gcnew ListViewIntSorter(E->Column, List->Sorting, true);
			break;
		default:
			Sorter = gcnew ListViewStringSorter(E->Column, List->Sorting);
			break;
		}
		List->ListViewItemSorter = Sorter;
	}
	void TagBrowser::TagTree_NodeClick(Object^ Sender, AdvTree::TreeNodeMouseEventArgs^ E)
	{
		AdvTree::Node^ SelectedNode = TagTree->SelectedNode;
		if (SelectedNode != nullptr)
		{
			UpdateFormListForTag(SelectedNode);
		}
	}
	void TagBrowser::TagTree_AfterCellEdit(Object^ Sender, AdvTree::CellEditEventArgs ^ E)
	{
		if (E->Cell->Text != E->NewText)
		{
			if (Database->GetTagExists(E->NewText))
			{
				MessageBox::Show("Tag '" + E->NewText + "' already exists. Please enter an unique name.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				E->Cancel = true;
			}
			else if (E->NewText->IndexOf(";") != -1)
			{
				MessageBox::Show("Tag '" + E->NewText + "' contains invalid characters.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
				E->Cancel = true;
			}
		}
	}
	void TagBrowser::SaveTags_Click(Object^ Sender, EventArgs^ E)
	{
		SaveFileDialog^ SaveManager = gcnew SaveFileDialog();

		SaveManager->DefaultExt = "*.csetags";
		SaveManager->Filter = "CSE Tag Database|*.csetags";
		SaveManager->RestoreDirectory = true;

		if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->FileName->Length > 0)
		{
			String^ SerializedData = Database->SerializeDatabase();
			try
			{
				StreamWriter^ TextParser = gcnew StreamWriter(SaveManager->FileName);
				TextParser->Write(SerializedData);
				TextParser->Close();
			}
			catch (Exception^ E)
			{
				MessageBox::Show("Errors were encountered while saving tag database '" + SaveManager->FileName + "'.\n\nError logged to the console.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Error);
				DebugPrint("Couldn't read from tag database " + SaveManager->FileName + "!\n\tException: " + E->Message);
			}

			MessageBox::Show("Database saved to '" + SaveManager->FileName + "' successfully.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Information);
		}
	}
	void TagBrowser::LoadTags_Click(Object^ Sender, EventArgs^ E)
	{
		OpenFileDialog^ LoadManager = gcnew OpenFileDialog();

		LoadManager->DefaultExt = "*.csetags";
		LoadManager->Filter = "CSE Tag Database|*.csetags";
		LoadManager->RestoreDirectory = true;

		if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileName->Length > 0)
		{
			try
			{
				StreamReader^ TextParser = gcnew StreamReader(LoadManager->FileName);
				String^ FileContents = TextParser->ReadToEnd();
				TextParser->Close();

				if (!Database->DeserializeDatabase(FileContents))
					throw gcnew CSEGeneralException("Database suffers inconsistencies.");
			}
			catch (Exception^ E)
			{
				MessageBox::Show("Errors were encountered while parsing tag database '" + LoadManager->FileName + "'.\n\nError logged to the console.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Error);
				DebugPrint("Couldn't read from tag database " + LoadManager->FileName + "!\n\tException: " + E->Message);
			}
		}
	}

	void TagBrowser::SearchBox_TextChanged(Object^ Sender, EventArgs^ E)
	{
		if (SearchBox->Text != "" && FormList->Items->Count > 1)
		{
			for each (ListViewItem^ Itr in FormList->SelectedItems)
				Itr->Selected = false;

			ListViewItem^ Result = FormList->FindItemWithText(SearchBox->Text, true, 0, true);

			if (Result != nullptr)
			{
				Result->Selected = true;
				FormList->TopItem = Result;
			}
			else
			{
				Result = GetListViewSelectedItem(FormList);
				if (Result != nullptr)
					Result->Selected = false;
			}
		}
	}
	void TagBrowser::TagTreeContextMenuAdd_Click(Object^ Sender, EventArgs^ E)
	{
		String^ Name = "Name";

		Name = Microsoft::VisualBasic::Interaction::InputBox("Enter a name used to identify the new tag",
			"Tag Browser",
			"",
			SystemInformation::PrimaryMonitorSize.Width / 2,
			SystemInformation::PrimaryMonitorSize.Height / 2);

		if (Name->Length == 0)
			return;
		else if (Database->GetTagExists(Name))
		{
			MessageBox::Show("Tag '" + Name + "' already exists. Please enter an unique name.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
			return;
		}

		AdvTree::Node^ SelectedNode = TagTree->SelectedNode;
		Database->AddTag(SelectedNode, Name);
	}

	void TagBrowser::TagTreeContextMenuRemove_Click(Object^ Sender, EventArgs^ E)
	{
		AdvTree::Node^ SelectedNode = TagTree->SelectedNode;
		if (SelectedNode != nullptr)
		{
			if (SelectedNode->Nodes->Count > 0)
			{
				if (MessageBox::Show("Are you sure ? This operation will remove all child tags.", "Tag Browser",
					MessageBoxButtons::YesNo, MessageBoxIcon::Exclamation) != DialogResult::Yes)
				{
					return;
				}
			}

			Database->RemoveTag(SelectedNode);
			FormList->Items->Clear();
		}
	}
	void TagBrowser::FormListContextMenuRemove_Click(Object^ Sender, EventArgs^ E)
	{
		AdvTree::Node^ SelectedNode = TagTree->SelectedNode;

		for each (ListViewItem^ Itr in FormList->SelectedItems)
			Database->UntagItem(SelectedNode, Itr->Text);

		FormList->BeginUpdate();
		for (ListViewItem^ Item = GetListViewSelectedItem(FormList); Item != nullptr; Item = GetListViewSelectedItem(FormList))
			FormList->Items->Remove(Item);
		FormList->EndUpdate();
	}

	void TagBrowser::UpdateFormListForTag(AdvTree::Node^ Tag)
	{
		List<String^>^ TagItems = nullptr;
		if (Database->GetItemsForTag(Tag, TagItems))
		{
			FormList->BeginUpdate();
			FormList->Items->Clear();

			bool BadItems = false;
			for each (String^ Itr in TagItems)
			{
				ComponentDLLInterface::FormData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupFormByEditorID((CString(Itr)).c_str());
				if (Data)
				{
					AddItemToFormList(Data);
				}
				else
				{
					BadItems = true;
					DebugPrint("Couldn't find form '" + Itr + "'");
				}
				NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);
			}

			FormList->EndUpdate();

			if (BadItems)
				MessageBox::Show("Some forms were found missing while populating the tag list.\n\nDetails logged to the console.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
		}
	}
	void TagBrowser::AddItemToFormList(ComponentDLLInterface::FormData* Data)
	{
		ListViewItem^ Item = gcnew ListViewItem(gcnew String(Data->EditorID));
		Item->SubItems->Add(Data->FormID.ToString("X8"));
		if (Data->TypeID < 0x45)
			Item->SubItems->Add(TypeIdentifier[(int)Data->TypeID]);
		else
			Item->SubItems->Add("<Unknown>");

		UInt32 ActiveForeColor = NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormListActiveItemForegroundColor();
		UInt32 ActiveBackColor = NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetFormListActiveItemBackgroundColor();
		bool ColorizeActiveForms = NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetShouldColorizeActiveForms();
		if (Data->IsActive() && ColorizeActiveForms)
		{
			Item->ForeColor = System::Drawing::Color::FromArgb(ActiveForeColor & 0xFF,
				(ActiveForeColor >> 8) & 0xFF,
				(ActiveForeColor >> 16) & 0xFF);

			Item->BackColor = System::Drawing::Color::FromArgb(ActiveBackColor & 0xFF,
				(ActiveBackColor >> 8) & 0xFF,
				(ActiveBackColor >> 16) & 0xFF);
		}

		FormList->Items->Add(Item);
	}

	bool TagBrowser::AddItemToActiveTag(ComponentDLLInterface::FormData* Data)
	{
		AdvTree::Node^ SelectedNode = TagTree->SelectedNode;
		if (SelectedNode != nullptr)
		{
			if (Database->TagItem(SelectedNode, Data))
			{
				AddItemToFormList(Data);
				return true;
			}
		}
		else
			MessageBox::Show("No tag selected.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);

		return false;
	}

	void TagBrowser::Show( IntPtr Handle )
	{
		if (TagBrowserBox->Visible)
			TagBrowserBox->BringToFront();
		else
			TagBrowserBox->Show(gcnew WindowHandleWrapper(Handle));
	}

	void TagBrowser::Hide()
	{
		TagBrowserBox->Hide();
	}

	System::IntPtr TagBrowser::GetFormListHandle()
	{
		return FormList->Handle;
	}

	System::IntPtr TagBrowser::GetWindowHandle()
	{
		return TagBrowserBox->Handle;
	}
}