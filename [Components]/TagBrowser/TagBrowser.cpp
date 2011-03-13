#include "TagBrowser.h"
#include "[Common]\ListViewUtilities.h"
#include "[Common]\NativeWrapper.h"

#using "Microsoft.VisualBasic.dll"

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

//	ParentTree->SelectNode(NewNode, AdvTree::eTreeAction::Code);
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
bool TagDatabase::TagItem(AdvTree::Node^ Tag, FormData* Data)
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
bool TagDatabase::TagItem(String^% TagName, FormData* Data)
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
		for each (String^% Itr in TagItems)
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
		for each (String^% Itr in TagItems)
		{
			if (String::Compare(Itr, Name, true) == 0)
				return true;
		}
	}
	
	return false;
}
bool TagDatabase::GetTagExists(String^% Name)
{
	return ParentTree->FindNodeByText(Name, true) != nullptr;
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
		for each (String^% ItrItem in Itr.Value)
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
					FormData* Data = NativeWrapper::LookupFormByEditorID((gcnew CStringWrapper(Token))->String());
					if (Data)
						TagItem(Tag, Data);
					else
					{
						BadItems = true;
						DebugPrint("Couldn't find form '" + Token + "'");
					}
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

	TagBrowserBox = gcnew Form();
	SearchBox = (gcnew TextBox());
	FormList = (gcnew ListView());
	FormListHEditorID = (gcnew ColumnHeader());
	FormListHRefID = (gcnew ColumnHeader());
	FormListHType = (gcnew ColumnHeader());
	FormListContextMenu = (gcnew ContextMenuStrip());
	FormListContextMenuAdd = (gcnew ToolStripMenuItem());
	FormListContextMenuRemove = (gcnew ToolStripMenuItem());
	SelectionList = (gcnew ListView());
	SelectionListHEditorId = (gcnew ColumnHeader());
	SelectionListHRefID = (gcnew ColumnHeader());
	SelectionListHType = (gcnew ColumnHeader());
	SelectionListContextMenu = (gcnew ContextMenuStrip());
	SelectionListContextMenuRemove = (gcnew ToolStripMenuItem());
	TagTree = (gcnew DevComponents::AdvTree::AdvTree());
	TagTreeContextMenu = (gcnew ContextMenuStrip());
	TagTreeContextMenuAdd = (gcnew ToolStripMenuItem());
	TagTreeContextMenuRemove = (gcnew ToolStripMenuItem());
	TagTreeNodeConnector = (gcnew DevComponents::AdvTree::NodeConnector());
	TagTreeElementStyle2 = (gcnew DevComponents::DotNetBar::ElementStyle());
	TagTreeElementStyle1 = (gcnew DevComponents::DotNetBar::ElementStyle());
	AddObjectSelection = (gcnew Button());
	PlaceSelection = (gcnew Button());
	LabelSelectionList = (gcnew Label());
	ClearSelectionOnInstantiation = (gcnew CheckBox());
	CustomPos = (gcnew CheckBox());
	PositionX = (gcnew TextBox());
	PositionZ = (gcnew TextBox());
	PositionY = (gcnew TextBox());
	CustomRot = (gcnew CheckBox());
	RotationY = (gcnew TextBox());
	RotationZ = (gcnew TextBox());
	RotationX = (gcnew TextBox());
	ShowSelectionControls = (gcnew Button());
	SaveTags = (gcnew Button());
	LoadTags = (gcnew Button());
	FormListContextMenu->SuspendLayout();
	SelectionListContextMenu->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TagTree))->BeginInit();
	TagTreeContextMenu->SuspendLayout();
	TagBrowserBox->SuspendLayout();
	// 
	// SearchBox
	// 
	SearchBox->Location = Point(245, 389);
//	SearchBox->Multiline = true;
	SearchBox->Name = L"SearchBox";
	SearchBox->Size = Size(219, 33);
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
	FormList->Location = Point(245, 12);
	FormList->Name = L"FormList";
	FormList->Size = Size(384, 371);
	FormList->TabIndex = 2;
	FormList->UseCompatibleStateImageBehavior = false;
	FormList->View = View::Details;
	FormList->Tag = (int)1;
	FormList->ItemActivate += gcnew EventHandler(this, &TagBrowser::FormSelectionList_ItemActivate);
	FormList->ColumnClick += gcnew ColumnClickEventHandler(this, &TagBrowser::FormSelectionList_ColumnClick);
	// 
	// FormListHEditorID
	// 
	FormListHEditorID->Text = L"EditorID";
	FormListHEditorID->Width = 198;
	// 
	// FormListHRefID
	// 
	FormListHRefID->Text = L"FormID";
	FormListHRefID->Width = 70;
	// 
	// FormListHType
	// 
	FormListHType->Text = L"Type";
	FormListHType->Width = 111;
	// 
	// FormListContextMenu
	// 
	FormListContextMenu->Items->AddRange(gcnew cli::array< ToolStripItem^  >(2) {FormListContextMenuAdd, FormListContextMenuRemove});
	FormListContextMenu->Name = L"FormListContextMenu";
	FormListContextMenu->Size = Size(165, 26);
	// 
	// FormListContextMenuAdd
	// 
	FormListContextMenuAdd->Name = L"FormListContextMenuAdd";
	FormListContextMenuAdd->Size = Size(164, 22);
	FormListContextMenuAdd->Text = L"Add To Selection";
	FormListContextMenuAdd->Click += gcnew EventHandler(this, &TagBrowser::FormListContextMenuAdd_Click);

	FormListContextMenuRemove->Name = L"FormListContextMenuRemove";
	FormListContextMenuRemove->Size = Size(164, 22);
	FormListContextMenuRemove->Text = L"Remove From Tag";
	FormListContextMenuRemove->Click += gcnew EventHandler(this, &TagBrowser::FormListContextMenuRemove_Click);
	// 
	// SelectionList
	// 
	SelectionList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(3) {SelectionListHEditorId, 
		SelectionListHRefID, SelectionListHType});
	SelectionList->ContextMenuStrip = SelectionListContextMenu;
	SelectionList->FullRowSelect = true;
	SelectionList->Location = Point(692, 42);
	SelectionList->Name = L"SelectionList";
	SelectionList->Size = Size(350, 236);
	SelectionList->TabIndex = 3;
	SelectionList->UseCompatibleStateImageBehavior = false;
	SelectionList->View = View::Details;
	SelectionList->Tag = (int)1;
	SelectionList->ItemActivate += gcnew EventHandler(this, &TagBrowser::FormSelectionList_ItemActivate);
	SelectionList->ColumnClick += gcnew ColumnClickEventHandler(this, &TagBrowser::FormSelectionList_ColumnClick);
	// 
	// SelectionListHEditorId
	// 
	SelectionListHEditorId->Text = L"EditorID";
	SelectionListHEditorId->Width = 128;
	// 
	// SelectionListHRefID
	// 
	SelectionListHRefID->Text = L"FormID";
	SelectionListHRefID->Width = 70;
	// 
	// SelectionListHType
	// 
	SelectionListHType->Text = L"Type";
	SelectionListHType->Width = 111;
	// 
	// SelectionListContextMenu
	// 
	SelectionListContextMenu->Items->AddRange(gcnew cli::array< ToolStripItem^  >(1) {SelectionListContextMenuRemove});
	SelectionListContextMenu->Name = L"SelectionListContextMenu";
	SelectionListContextMenu->Size = Size(200, 26);
	// 
	// SelectionListContextMenuRemove
	// 
	SelectionListContextMenuRemove->Name = L"SelectionListContextMenuRemove";
	SelectionListContextMenuRemove->Size = Size(199, 22);
	SelectionListContextMenuRemove->Text = L"Remove From Selection";
	SelectionListContextMenuRemove->Click += gcnew EventHandler(this, &TagBrowser::SelectionListContextMenuRemove_Click);
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
	TagTree->Location = Point(9, 12);
	TagTree->Name = L"TagTree";
	TagTree->NodesConnector = TagTreeNodeConnector;
	TagTree->NodeSpacing = 4;
	TagTree->NodeStyle = TagTreeElementStyle2;
	TagTree->PathSeparator = L";";
	TagTree->SelectionBoxStyle = DevComponents::AdvTree::eSelectionStyle::NodeMarker;
	TagTree->SelectionPerCell = true;
	TagTree->Size = Size(225, 409);
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
	// AddObjectSelection
	// 
	AddObjectSelection->Location = Point(471, 389);
	AddObjectSelection->Name = L"AddObjectSelection";
	AddObjectSelection->Size = Size(157, 32);
	AddObjectSelection->TabIndex = 5;
	AddObjectSelection->Text = L"Add Object Window Selection";
	AddObjectSelection->UseVisualStyleBackColor = true;
	AddObjectSelection->Click += gcnew EventHandler(this, &TagBrowser::AddObjectSelection_Click);
	// 
	// PlaceSelection
	// 
	PlaceSelection->Location = Point(692, 387);
	PlaceSelection->Name = L"PlaceSelection";
	PlaceSelection->Size = Size(350, 32);
	PlaceSelection->TabIndex = 6;
	PlaceSelection->Text = L"Place Selection In Active Cell";
	PlaceSelection->UseVisualStyleBackColor = true;
	PlaceSelection->Click += gcnew EventHandler(this, &TagBrowser::PlaceSelection_Click);
	// 
	// LabelSelectionList
	// 
	LabelSelectionList->Font = (gcnew Font(L"Lucida Grande", 14.25F, FontStyle::Regular, GraphicsUnit::Point, 
		static_cast<System::Byte>(0)));
	LabelSelectionList->Location = Point(692, 12);
	LabelSelectionList->Name = L"LabelSelectionList";
	LabelSelectionList->Size = Size(350, 27);
	LabelSelectionList->TabIndex = 7;
	LabelSelectionList->Text = L"Selection";
	LabelSelectionList->TextAlign = ContentAlignment::MiddleCenter;
	// 
	// ClearSelectionOnInstantiation
	// 
	ClearSelectionOnInstantiation->Location = Point(692, 354);
	ClearSelectionOnInstantiation->Name = L"ClearSelectionOnInstantiation";
	ClearSelectionOnInstantiation->Size = Size(350, 32);
	ClearSelectionOnInstantiation->TabIndex = 8;
	ClearSelectionOnInstantiation->Text = L"Clear Selection After Reference Instantiation";
	ClearSelectionOnInstantiation->TextImageRelation = TextImageRelation::ImageAboveText;
	ClearSelectionOnInstantiation->UseVisualStyleBackColor = true;
	// 
	// CustomPos
	// 
	CustomPos->Location = Point(692, 284);
	CustomPos->Name = L"CustomPos";
	CustomPos->Size = Size(131, 32);
	CustomPos->TabIndex = 9;
	CustomPos->Text = L"Use Custom Position";
	CustomPos->TextImageRelation = TextImageRelation::ImageAboveText;
	CustomPos->UseVisualStyleBackColor = true;
	// 
	// PositionX
	// 
	PositionX->Location = Point(692, 325);
	PositionX->Name = L"PositionX";
	PositionX->Size = Size(55, 20);
	PositionX->TabIndex = 10;
	// 
	// PositionZ
	// 
	PositionZ->Location = Point(804, 325);
	PositionZ->Name = L"PositionZ";
	PositionZ->Size = Size(55, 20);
	PositionZ->TabIndex = 11;
	// 
	// PositionY
	// 
	PositionY->Location = Point(748, 325);
	PositionY->Name = L"PositionY";
	PositionY->Size = Size(55, 20);
	PositionY->TabIndex = 12;
	// 
	// CustomRot
	// 
	CustomRot->Location = Point(911, 284);
	CustomRot->Name = L"CustomRot";
	CustomRot->Size = Size(131, 32);
	CustomRot->TabIndex = 13;
	CustomRot->Text = L"Use Custom Rotation";
	CustomRot->TextImageRelation = TextImageRelation::ImageAboveText;
	CustomRot->UseVisualStyleBackColor = true;
	// 
	// RotationY
	// 
	RotationY->Location = Point(931, 325);
	RotationY->Name = L"RotationY";
	RotationY->Size = Size(55, 20);
	RotationY->TabIndex = 16;
	// 
	// RotationZ
	// 
	RotationZ->Location = Point(987, 325);
	RotationZ->Name = L"RotationZ";
	RotationZ->Size = Size(55, 20);
	RotationZ->TabIndex = 15;
	// 
	// RotationX
	// 
	RotationX->Location = Point(875, 325);
	RotationX->Name = L"RotationX";
	RotationX->Size = Size(55, 20);
	RotationX->TabIndex = 14;
	// 
	// ShowSelectionControls
	// 
	ShowSelectionControls->BackColor = SystemColors::Control;
	ShowSelectionControls->Font = (gcnew Font(L"Microsoft Sans Serif", 14.25F, FontStyle::Bold, 
		GraphicsUnit::Point, static_cast<System::Byte>(0)));
	ShowSelectionControls->ForeColor = Color::Black;
	ShowSelectionControls->Location = Point(639, 186);
	ShowSelectionControls->Name = L"ShowSelectionControls";
	ShowSelectionControls->Size = Size(40, 63);
	ShowSelectionControls->TabIndex = 17;
	ShowSelectionControls->Text = L"« »";
	ShowSelectionControls->UseVisualStyleBackColor = false;
	ShowSelectionControls->Click += gcnew EventHandler(this, &TagBrowser::ShowSelectionControls_Click);
	// 
	// SaveTags
	// 
	SaveTags->BackColor = System::Drawing::SystemColors::Control;
	SaveTags->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
		static_cast<System::Byte>(0)));
	SaveTags->ForeColor = System::Drawing::Color::Black;
	SaveTags->Location = System::Drawing::Point(639, 363);
	SaveTags->Name = L"SaveTags";
	SaveTags->Size = System::Drawing::Size(40, 27);
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
	LoadTags->Location = System::Drawing::Point(639, 392);
	LoadTags->Name = L"LoadTags";
	LoadTags->Size = System::Drawing::Size(40, 27);
	LoadTags->TabIndex = 20;
	LoadTags->Text = L"Load";
	LoadTags->UseVisualStyleBackColor = false;
	LoadTags->Click += gcnew EventHandler(this, &TagBrowser::LoadTags_Click);
	// 
	// TagBrowser
	// 
	TagBrowserBox->AutoScaleDimensions = SizeF(6, 13);
	TagBrowserBox->AutoScaleMode = AutoScaleMode::Font;
	TagBrowserBox->ClientSize = Size(1054, 434);
	TagBrowserBox->Controls->Add(LoadTags);
	TagBrowserBox->Controls->Add(SaveTags);
	TagBrowserBox->Controls->Add(ShowSelectionControls);
	TagBrowserBox->Controls->Add(RotationY);
	TagBrowserBox->Controls->Add(RotationZ);
	TagBrowserBox->Controls->Add(RotationX);
	TagBrowserBox->Controls->Add(CustomRot);
	TagBrowserBox->Controls->Add(PositionY);
	TagBrowserBox->Controls->Add(PositionZ);
	TagBrowserBox->Controls->Add(PositionX);
	TagBrowserBox->Controls->Add(CustomPos);
	TagBrowserBox->Controls->Add(ClearSelectionOnInstantiation);
	TagBrowserBox->Controls->Add(LabelSelectionList);
	TagBrowserBox->Controls->Add(PlaceSelection);
	TagBrowserBox->Controls->Add(AddObjectSelection);
	TagBrowserBox->Controls->Add(TagTree);
	TagBrowserBox->Controls->Add(SelectionList);
	TagBrowserBox->Controls->Add(FormList);
	TagBrowserBox->Controls->Add(SearchBox);
	TagBrowserBox->FormBorderStyle = FormBorderStyle::FixedSingle;
	TagBrowserBox->MaximizeBox = false;
	TagBrowserBox->Name = L"TagBrowser";
	TagBrowserBox->StartPosition = FormStartPosition::CenterScreen;
	TagBrowserBox->Text = L"Tag Browser";
	TagBrowserBox->Closing += gcnew CancelEventHandler(this, &TagBrowser::TagBrowserBox_Cancel);
	FormListContextMenu->ResumeLayout(false);
	SelectionListContextMenu->ResumeLayout(false);
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TagTree))->EndInit();
	TagTreeContextMenu->ResumeLayout(false);
	TagBrowserBox->ResumeLayout(false);
	TagBrowserBox->PerformLayout();

	TagBrowserBox->Hide();

	Database = gcnew TagDatabase(TagTree); 
}

void TagBrowser::TagBrowserBox_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	Hide();
	E->Cancel = true;
}
void TagBrowser::FormSelectionList_ItemActivate(Object^ Sender, EventArgs^ E)
{
	ListView^ List = dynamic_cast<ListView^>(Sender);

	ListViewItem^ Selected = GetListViewSelectedItem(List);
	if (Selected != nullptr)
	{
		NativeWrapper::TESForm_LoadIntoView((gcnew CStringWrapper(Selected->Text))->String(), (gcnew CStringWrapper(Selected->SubItems[2]->Text))->String());
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
		Sorter = gcnew CSEListViewIntSorter(E->Column, List->Sorting, true);
		break;
	default:
		Sorter = gcnew CSEListViewStringSorter(E->Column, List->Sorting);
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
void TagBrowser::AddObjectSelection_Click(Object^ Sender, EventArgs^ E)
{
	NativeWrapper::TagBrowser_GetObjectWindowSelection();
}
void TagBrowser::PlaceSelection_Click(Object^ Sender, EventArgs^ E)
{
	if (SelectionList->Items->Count == 0)
	{
		MessageBox::Show("No forms selected.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Information);
		return;
	}

	TagBrowserInstantiationData InteropData = {0};
	try
	{
		if (CustomPos->Checked)
		{
			ToggleFlag(&InteropData.Flags, TagBrowserInstantiationData::kFlag_UsePosition, true);
			InteropData.Position.x = float::Parse(PositionX->Text);
			InteropData.Position.y = float::Parse(PositionY->Text);
			InteropData.Position.z = float::Parse(PositionZ->Text);
		}
		if (CustomRot->Checked)
		{
			ToggleFlag(&InteropData.Flags, TagBrowserInstantiationData::kFlag_UseRotation, true);
			InteropData.Rotation.x = float::Parse(RotationX->Text);
			InteropData.Rotation.y = float::Parse(RotationY->Text);
			InteropData.Rotation.z = float::Parse(RotationZ->Text);
		}
	}
	catch (...)
	{
		MessageBox::Show("Invalid 3D data entered.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return;		
	}

	FormData* Data = InteropData.FormListHead = new FormData[SelectionList->Items->Count];
	InteropData.FormCount = SelectionList->Items->Count;

	UInt32 Index = 0;
	for each (ListViewItem^ Itr in SelectionList->Items)
	{
		Data[Index].FormID = UInt32::Parse(Itr->SubItems[1]->Text, Globalization::NumberStyles::HexNumber);
		Index++;
	}

	NativeWrapper::TagBrowser_InstantiateObjects(&InteropData);
	delete [] Data;

	if (ClearSelectionOnInstantiation->Checked)
		SelectionList->Items->Clear();
}
void TagBrowser::ShowSelectionControls_Click(Object^ Sender, EventArgs^ E)
{
	if (TagBrowserBox->Width != 696)
		TagBrowserBox->Width = 696;
	else
		TagBrowserBox->Width = 1054;
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
		ListViewItem^ Result = FormList->FindItemWithText(SearchBox->Text, true, 0);

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
void TagBrowser::FormListContextMenuAdd_Click(Object^ Sender, EventArgs^ E)
{
	SelectionList->BeginUpdate();
	for each (ListViewItem^ Itr in FormList->SelectedItems)
	{
		ListViewItem^ Item = gcnew ListViewItem(Itr->Text);
		Item->SubItems->Add(Itr->SubItems[1]);
		Item->SubItems->Add(Itr->SubItems[2]);
		SelectionList->Items->Add(Item);
	}
	SelectionList->EndUpdate();
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
void TagBrowser::SelectionListContextMenuRemove_Click(Object^ Sender, EventArgs^ E)
{
	SelectionList->BeginUpdate();
	for (ListViewItem^ Item = GetListViewSelectedItem(SelectionList); Item != nullptr; Item = GetListViewSelectedItem(SelectionList))
		SelectionList->Items->Remove(Item);
	SelectionList->EndUpdate();	
}

void TagBrowser::UpdateFormListForTag(AdvTree::Node^ Tag)
{
	List<String^>^ TagItems = nullptr;
	if (Database->GetItemsForTag(Tag, TagItems))
	{
		FormList->BeginUpdate();
		FormList->Items->Clear();

		bool BadItems = false;
		for each (String^% Itr in TagItems)
		{
			FormData* Data = NativeWrapper::LookupFormByEditorID((gcnew CStringWrapper(Itr))->String());
			if (Data)
			{
				AddItemToFormList(Data);
			}
			else
			{
				BadItems = true;
				DebugPrint("Couldn't find form '" + Itr + "'");
			}
		}

		FormList->EndUpdate();

		if (BadItems)
			MessageBox::Show("Some forms were found missing while populating the tag list.\n\nDetails logged to the console.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
	}
}
void TagBrowser::AddItemToFormList(FormData* Data)
{
	ListViewItem^ Item = gcnew ListViewItem(gcnew String(Data->EditorID));
	Item->SubItems->Add(Data->FormID.ToString("X8"));
	Item->SubItems->Add(TypeIdentifier[(int)Data->TypeID]);

	FormList->Items->Add(Item);

}


void TagBrowser::AddItemToActiveTag(FormData* Data)
{
	AdvTree::Node^ SelectedNode = TagTree->SelectedNode;
	if (SelectedNode != nullptr)
	{
		if (Database->TagItem(SelectedNode, Data))
			AddItemToFormList(Data);
	}
	else
		MessageBox::Show("No tag selected.", "Tag Browser", MessageBoxButtons::OK, MessageBoxIcon::Exclamation);		
}


