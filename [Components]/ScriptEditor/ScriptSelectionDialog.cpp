#include "ScriptSelectionDialog.h"
#include "[Common]\NativeWrapper.h"
#include "ScriptSync.h"
#include "IScriptEditorView.h"
#include "Preferences.h"

namespace cse
{


namespace scriptEditor
{


namespace selectScript
{


NativeScriptDataWrapper::NativeScriptDataWrapper(componentDLLInterface::ScriptData* Data)
{
	ScriptData = Data;
	EditorID = gcnew String(Data->EditorID);
	ParentPluginName = gcnew String(Data->ParentPluginName);

	if (Data->UDF)
		Type = "User-Defined Function";
	else switch (Data->Type)
	{
	case componentDLLInterface::ScriptData::kScriptType_Object:
		Type = "Object";
		break;
	case componentDLLInterface::ScriptData::kScriptType_Quest:
		Type = "Quest";
		break;
	case componentDLLInterface::ScriptData::kScriptType_Magic:
		Type = "Magic Effect";
		break;
	}
}

int ScriptCollectionSorter::CompareFieldAscending(NativeScriptDataWrapper^ X, NativeScriptDataWrapper^ Y, eSortField Field)
{
	switch (Field)
	{
	case eSortField::Flags:
		return safe_cast<int>(ScriptSelectionDialog::GetFlagType(X->ScriptData)).CompareTo(safe_cast<int>(ScriptSelectionDialog::GetFlagType(Y->ScriptData)));
	case eSortField::EditorID:
		return String::Compare(X->EditorID, Y->EditorID, System::StringComparison::CurrentCultureIgnoreCase);
	case eSortField::FormID:
		return X->ScriptData->FormID.CompareTo(Y->ScriptData->FormID);
	case eSortField::Type:
		return String::Compare(X->Type, Y->Type, System::StringComparison::CurrentCultureIgnoreCase);
	case eSortField::ParentPlugin:
		return String::Compare(X->ParentPluginName, Y->ParentPluginName, System::StringComparison::CurrentCultureIgnoreCase);
	default:
		return 0;
	}
}

ScriptCollectionSorter::ScriptCollectionSorter(eSortField Field, SortOrder Order)
	: Field(Field), Order(Order)
{
}

int ScriptCollectionSorter::Compare(NativeScriptDataWrapper^ X, NativeScriptDataWrapper^ Y)
{
	int Result = CompareFieldAscending(X, Y, Field);

	// Sort by editorID if equal
	if (Result == 0)
		Result = CompareFieldAscending(X, Y, eSortField::EditorID);

	if (Order == SortOrder::Descending)
		Result = -Result;

	return Result;
}

int ScriptCollectionSorter::Compare(Object^ X, Object^ Y)
{
	return Compare(safe_cast<NativeScriptDataWrapper^>(X), safe_cast<NativeScriptDataWrapper^>(Y));
}

FastScriptListViewDataSource::FastScriptListViewDataSource(BrightIdeasSoftware::FastObjectListView^ Parent)
	: FastObjectListDataSource(Parent)
{
}

void FastScriptListViewDataSource::Sort(BrightIdeasSoftware::OLVColumn^ column, System::Windows::Forms::SortOrder order)
{
	if (order == SortOrder::None)
		return;

	ScriptCollectionSorter::eSortField SortField = safe_cast<ScriptCollectionSorter::eSortField>(column->Tag);
	auto Comparator = gcnew ScriptCollectionSorter(SortField, order);
	ObjectList->Sort(Comparator);
	FilteredObjectList->Sort(Comparator);

	RebuildIndexMap();
}

ScriptSelectionDialog::Params::Params()
{
	this->SelectedScriptEditorID = "";
	this->FilterString = "";
	this->ShowDeletedScripts = true;
	this->PreventSyncedScriptSelection = false;
	this->ParentWindowHandle = IntPtr::Zero;
}

ScriptSelectionDialog::SelectionResult::SelectionResult()
{
	this->SelectedScriptEditorIDs = gcnew List<String^>;
}

Image^ ScriptSelectionDialog::GetFlagIcon(eFlagType FlagType)
{
	if (FlagIcons->Count == 0)
	{
		auto ImageResources = view::components::CommonIcons::Get()->ResourceManager;
		FlagIcons->Add(eFlagType::Default, ImageResources->CreateImage("Transparent"));
		FlagIcons->Add(eFlagType::Active, ImageResources->CreateImage("Asterisk"));
		FlagIcons->Add(eFlagType::Deleted, ImageResources->CreateImage("Trash"));
		FlagIcons->Add(eFlagType::Syncing, ImageResources->CreateImage("Syncing"));
		FlagIcons->Add(eFlagType::Uncompiled, view::components::CommonIcons::Get()->Warning);
	}

	return FlagIcons[FlagType];
}

void ScriptSelectionDialog::ScriptList_SelectionChanged(Object^ Sender, EventArgs^ E)
{
	DeferredSelectionUpdateTimer->Start();
}

void ScriptSelectionDialog::ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::F1:
		ShowUseReport();
		break;
	case Keys::Escape:
		this->Close();
		E->Handled = true;
		E->SuppressKeyPress = true;
		break;
	case Keys::Back:
		if (BottomToolbarTextboxFilter->Text->Length >= 1)
		{
			BottomToolbarTextboxFilter->Text = BottomToolbarTextboxFilter->Text->Remove(BottomToolbarTextboxFilter->Text->Length - 1);
			ScriptList->Focus();
		}

		E->Handled = true;
		break;
	}
}

void ScriptSelectionDialog::ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
{
	if ((E->KeyChar > 0x29 && E->KeyChar < 0x3A) ||
		(E->KeyChar > 0x60 && E->KeyChar < 0x7B))
	{
		BottomToolbarTextboxFilter->Text += E->KeyChar.ToString();
		ScriptList->Focus();
		E->Handled = true;
	}
}

void ScriptSelectionDialog::ScriptList_ItemActivate(Object^ Sender, EventArgs^ E)
{
	CompleteSelection();
}

void ScriptSelectionDialog::BottomToolbarCompleteSelection_Click(Object^ Sender, EventArgs^ E)
{
	CompleteSelection();
}

void ScriptSelectionDialog::MenuSelectAllActive_Click(Object^ Sender, EventArgs^ E)
{
	ScriptList->DeselectAll();

	List<NativeScriptDataWrapper^>^ NewSelection = gcnew List<NativeScriptDataWrapper ^>;
	for each (NativeScriptDataWrapper ^ Itr in ListDataSource)
	{
		if (Itr->ScriptData->IsActive())
			NewSelection->Add(Itr);
	}

	ScriptList->SelectObjects(NewSelection);
}

void ScriptSelectionDialog::MenuSelectAllUncompiled_Click(Object^ Sender, EventArgs^ E)
{
	ScriptList->DeselectAll();

	List<NativeScriptDataWrapper^>^ NewSelection = gcnew List<NativeScriptDataWrapper^>;
	for each (NativeScriptDataWrapper ^ Itr in ListDataSource)
	{
		if (Itr->ScriptData->Compiled == false)
			NewSelection->Add(Itr);
	}

	ScriptList->SelectObjects(NewSelection);
}

void ScriptSelectionDialog::MenuSelectAllDeleted_Click(Object^ Sender, EventArgs^ E)
{
	ScriptList->DeselectAll();

	List<NativeScriptDataWrapper^>^ NewSelection = gcnew List<NativeScriptDataWrapper^>;
	for each (NativeScriptDataWrapper ^ Itr in ListDataSource)
	{
		if (Itr->ScriptData->IsDeleted())
			NewSelection->Add(Itr);
	}

	ScriptList->SelectObjects(NewSelection);
}

void ScriptSelectionDialog::BottomToolbarTextboxFilter_TextChanged(Object^ Sender, EventArgs^ E)
{
	PopulateLoadedScripts(BottomToolbarTextboxFilter->Text, false, false);
}

void ScriptSelectionDialog::BottomToolbarTextboxFilter_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	switch (E->KeyCode)
	{
	case Keys::Enter:
		CompleteSelection();
		return;
	case Keys::Back:
		return;
	}

	ScriptList_KeyDown(ScriptList, E);
}

void ScriptSelectionDialog::DeferredSelectionUpdateTimer_Tick(Object^ Sender, EventArgs^ E)
{
	BottomToolbarSelectionCount->Text = ScriptList->SelectedObjects->Count + " selected";

	if (ScriptList->SelectedObjects->Count == 1)
	{
		NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)ScriptList->SelectedObject;
		String^ ScriptText = gcnew String(Data->ScriptData->Text);

		ScriptTextPreview->Text = ScriptText->Replace("\n", "\r\n");
	}
	else
		ScriptTextPreview->Text = "";

	if (ScriptList->SelectedObjects->Count > 0)
		BottomToolbarCompleteSelection->Enabled = true;
	else
		BottomToolbarCompleteSelection->Enabled = false;

	DeferredSelectionUpdateTimer->Stop();
}

void ScriptSelectionDialog::Dialog_Cancel(Object^ Sender, CancelEventArgs^ E)
{
}

void ScriptSelectionDialog::Dialog_Load(Object^ Sender, EventArgs^ E)
{
	BottomToolbarTextboxFilter->Focus();
}

void ScriptSelectionDialog::InitializeComponent()
{
	this->components = (gcnew System::ComponentModel::Container());
	auto resources = (gcnew System::Resources::ResourceManager("ScriptEditor.ScriptSelectionDialog", Assembly::GetExecutingAssembly()));
	this->BottomToolbarCompleteSelection = (gcnew DevComponents::DotNetBar::ButtonX());
	this->BottomToolbarContainer = (gcnew DevComponents::DotNetBar::PanelEx());
	this->BottomToolbar = (gcnew DevComponents::DotNetBar::Bar());
	this->BottomToolbarTextboxFilter = (gcnew DevComponents::DotNetBar::TextBoxItem());
	this->BottomToolbarSelectAll = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->MenuSelectAllActive = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->MenuSelectAllDeleted = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->MenuSelectAllUncompiled = (gcnew DevComponents::DotNetBar::ButtonItem());
	this->BottomToolbarSelectionCount = (gcnew DevComponents::DotNetBar::LabelItem());
	this->ScriptTextPreview = (gcnew DevComponents::DotNetBar::Controls::TextBoxX());
	this->ScriptList = (gcnew BrightIdeasSoftware::FastObjectListView());
	this->ScriptListCFlags = (gcnew BrightIdeasSoftware::OLVColumn());
	this->ScriptListCScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
	this->ScriptListCFormID = (gcnew BrightIdeasSoftware::OLVColumn());
	this->ScriptListCType = (gcnew BrightIdeasSoftware::OLVColumn());
	this->ScriptListCParentPlugin = (gcnew BrightIdeasSoftware::OLVColumn());
	this->DeferredSelectionUpdateTimer = (gcnew System::Windows::Forms::Timer(this->components));
	this->ColorManager = (gcnew DevComponents::DotNetBar::StyleManagerAmbient(this->components));
	this->BottomToolbarContainer->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomToolbar))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->BeginInit();
	this->SuspendLayout();
	//
	// BottomToolbarCompleteSelection
	//
	this->BottomToolbarCompleteSelection->AccessibleRole = System::Windows::Forms::AccessibleRole::PushButton;
	this->BottomToolbarCompleteSelection->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
	this->BottomToolbarCompleteSelection->ColorTable = DevComponents::DotNetBar::eButtonColor::OrangeWithBackground;
	this->BottomToolbarCompleteSelection->Location = System::Drawing::Point(404, 539);
	this->BottomToolbarCompleteSelection->Name = L"BottomToolbarCompleteSelection";
	this->BottomToolbarCompleteSelection->Size = System::Drawing::Size(81, 29);
	this->BottomToolbarCompleteSelection->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->BottomToolbarCompleteSelection->TabIndex = 2;
	this->BottomToolbarCompleteSelection->Text = L"OK";
	//
	// BottomToolbarContainer
	//
	this->BottomToolbarContainer->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
	this->BottomToolbarContainer->CanvasColor = System::Drawing::SystemColors::Control;
	this->BottomToolbarContainer->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->BottomToolbarContainer->Controls->Add(this->BottomToolbar);
	this->BottomToolbarContainer->DisabledBackColor = System::Drawing::Color::Empty;
	this->BottomToolbarContainer->Location = System::Drawing::Point(3, 539);
	this->BottomToolbarContainer->Name = L"BottomToolbarContainer";
	this->BottomToolbarContainer->Size = System::Drawing::Size(395, 29);
	this->BottomToolbarContainer->Style->Alignment = System::Drawing::StringAlignment::Center;
	this->BottomToolbarContainer->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->BottomToolbarContainer->Style->Border = DevComponents::DotNetBar::eBorderType::SingleLine;
	this->BottomToolbarContainer->Style->BorderColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->BottomToolbarContainer->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->BottomToolbarContainer->Style->GradientAngle = 90;
	//
	// BottomToolbar
	//
	this->BottomToolbar->AntiAlias = true;
	this->BottomToolbar->Dock = System::Windows::Forms::DockStyle::Fill;
	this->BottomToolbar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
	this->BottomToolbar->IsMaximized = false;
	this->BottomToolbar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(3) {
		this->BottomToolbarTextboxFilter,
			this->BottomToolbarSelectAll, this->BottomToolbarSelectionCount
	});
	this->BottomToolbar->ItemSpacing = 7;
	this->BottomToolbar->Location = System::Drawing::Point(0, 0);
	this->BottomToolbar->Name = L"BottomToolbar";
	this->BottomToolbar->Size = System::Drawing::Size(395, 29);
	this->BottomToolbar->Stretch = true;
	this->BottomToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->BottomToolbar->TabIndex = 0;
	this->BottomToolbar->TabStop = false;
	this->BottomToolbar->Text = L"bar1";
	//
	// BottomToolbarTextboxFilter
	//
	this->BottomToolbarTextboxFilter->Name = L"BottomToolbarTextboxFilter";
	this->BottomToolbarTextboxFilter->TextBoxWidth = 165;
	this->BottomToolbarTextboxFilter->WatermarkColor = System::Drawing::SystemColors::GrayText;
	this->BottomToolbarTextboxFilter->WatermarkText = L"Filter";
	//
	// BottomToolbarSelectAll
	//
	this->BottomToolbarSelectAll->AutoExpandOnClick = true;
	this->BottomToolbarSelectAll->BeginGroup = true;
	this->BottomToolbarSelectAll->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::ImageAndText;
	this->BottomToolbarSelectAll->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"BottomToolbarSelectAll.Image")));
	this->BottomToolbarSelectAll->ImagePaddingHorizontal = 15;
	this->BottomToolbarSelectAll->ImagePaddingVertical = 10;
	this->BottomToolbarSelectAll->Name = L"BottomToolbarSelectAll";
	this->BottomToolbarSelectAll->SubItems->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(3) {
		this->MenuSelectAllActive,
			this->MenuSelectAllDeleted, this->MenuSelectAllUncompiled
	});
	this->BottomToolbarSelectAll->Text = L"Select All...";
	//
	// MenuSelectAllActive
	//
	this->MenuSelectAllActive->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"MenuSelectAllActive.Image")));
	this->MenuSelectAllActive->Name = L"MenuSelectAllActive";
	this->MenuSelectAllActive->Text = L"Active Scripts";
	//
	// MenuSelectAllDeleted
	//
	this->MenuSelectAllDeleted->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"MenuSelectAllDeleted.Image")));
	this->MenuSelectAllDeleted->Name = L"MenuSelectAllDeleted";
	this->MenuSelectAllDeleted->Text = L"Deleted Scripts";
	//
	// MenuSelectAllUncompiled
	//
	this->MenuSelectAllUncompiled->Image = (cli::safe_cast<System::Drawing::Image^>(resources->GetObject(L"MenuSelectAllUncompiled.Image")));
	this->MenuSelectAllUncompiled->Name = L"MenuSelectAllUncompiled";
	this->MenuSelectAllUncompiled->Text = L"Uncompiled Scripts";
	//
	// BottomToolbarSelectionCount
	//
	this->BottomToolbarSelectionCount->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
	this->BottomToolbarSelectionCount->Name = L"BottomToolbarSelectionCount";
	this->BottomToolbarSelectionCount->Text = L"0 selected";
	//
	// ScriptTextPreview
	//
	this->ScriptTextPreview->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left)
		| System::Windows::Forms::AnchorStyles::Right));
	this->ScriptTextPreview->BackColor = System::Drawing::Color::Black;
	//
	//
	//
	this->ScriptTextPreview->Border->Class = L"TextBoxBorder";
	this->ScriptTextPreview->Border->CornerType = DevComponents::DotNetBar::eCornerType::Square;
	this->ScriptTextPreview->Font = (gcnew System::Drawing::Font(L"Consolas", 9));
	this->ScriptTextPreview->Location = System::Drawing::Point(491, 0);
	this->ScriptTextPreview->MaxLength = 65535;
	this->ScriptTextPreview->Multiline = true;
	this->ScriptTextPreview->Name = L"ScriptTextPreview";
	this->ScriptTextPreview->PreventEnterBeep = true;
	this->ScriptTextPreview->ReadOnly = true;
	this->ScriptTextPreview->ScrollBars = System::Windows::Forms::ScrollBars::Both;
	this->ScriptTextPreview->Size = System::Drawing::Size(406, 568);
	this->ScriptTextPreview->TabStop = false;
	this->ScriptTextPreview->WordWrap = false;
	//
	// ScriptList
	//
	this->ScriptList->AllColumns->Add(this->ScriptListCFlags);
	this->ScriptList->AllColumns->Add(this->ScriptListCScriptName);
	this->ScriptList->AllColumns->Add(this->ScriptListCFormID);
	this->ScriptList->AllColumns->Add(this->ScriptListCType);
	this->ScriptList->AllColumns->Add(this->ScriptListCParentPlugin);
	this->ScriptList->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
		| System::Windows::Forms::AnchorStyles::Left));
	this->ScriptList->CellEditUseWholeCell = false;
	this->ScriptList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(5) {
		this->ScriptListCFlags,
			this->ScriptListCScriptName, this->ScriptListCFormID, this->ScriptListCType, this->ScriptListCParentPlugin
	});
	this->ScriptList->Cursor = System::Windows::Forms::Cursors::Default;
	this->ScriptList->EmptyListMsg = L"Doesn\'t look like anything to me...";
	this->ScriptList->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
		static_cast<System::Byte>(0)));
	this->ColorManager->SetEnableAmbientSettings(this->ScriptList, DevComponents::DotNetBar::eAmbientSettings::All);
	this->ScriptList->FullRowSelect = true;
	this->ScriptList->HideSelection = false;
	this->ScriptList->Location = System::Drawing::Point(3, 0);
	this->ScriptList->Name = L"ScriptList";
	this->ScriptList->ShowGroups = false;
	this->ScriptList->Size = System::Drawing::Size(482, 533);
	this->ScriptList->TabIndex = 1;
	this->ScriptList->UseCompatibleStateImageBehavior = false;
	this->ScriptList->View = System::Windows::Forms::View::Details;
	this->ScriptList->VirtualMode = true;
	//
	// ScriptListCFlags
	//
	this->ScriptListCFlags->MaximumWidth = 20;
	this->ScriptListCFlags->MinimumWidth = 20;
	this->ScriptListCFlags->Text = L"";
	this->ScriptListCFlags->Width = 20;
	//
	// ScriptListCScriptName
	//
	this->ScriptListCScriptName->Text = L"Name";
	this->ScriptListCScriptName->Width = 180;
	//
	// ScriptListCFormID
	//
	this->ScriptListCFormID->Text = L"Form ID";
	this->ScriptListCFormID->Width = 75;
	//
	// ScriptListCType
	//
	this->ScriptListCType->Text = L"Type";
	this->ScriptListCType->Width = 54;
	//
	// ScriptListCParentPlugin
	//
	this->ScriptListCParentPlugin->Text = L"Parent Plugin";
	this->ScriptListCParentPlugin->Width = 114;
	//
	// ScriptEditorSelectScripts
	//
	this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
	this->ClientSize = System::Drawing::Size(900, 574);
	this->StartPosition = FormStartPosition::CenterParent;
	this->Controls->Add(this->BottomToolbarCompleteSelection);
	this->Controls->Add(this->BottomToolbarContainer);
	this->Controls->Add(this->ScriptTextPreview);
	this->Controls->Add(this->ScriptList);
	this->DoubleBuffered = true;
	this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
	this->Name = L"ScriptEditorSelectScripts";
	this->ShowIcon = false;
	this->Text = L"Select Scripts";
	this->BottomToolbarContainer->ResumeLayout(false);
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->BottomToolbar))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->EndInit();
	this->ResumeLayout(false);
}

void ScriptSelectionDialog::FinalizeComponents()
{
	this->ScriptListCFlags->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ScriptSelectionDialog::ScriptListAspectFlagsGetter);
	this->ScriptListCFlags->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&ScriptSelectionDialog::ScriptListImageFlagsGetter);
	this->ScriptListCFlags->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&ScriptSelectionDialog::ScriptListAspectToStringConverterFlags);
	this->ScriptListCScriptName->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ScriptSelectionDialog::ScriptListAspectScriptNameGetter);
	this->ScriptListCFormID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ScriptSelectionDialog::ScriptListAspectFormIDGetter);
	this->ScriptListCFormID->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&ScriptSelectionDialog::ScriptListAspectToStringConverterFormID);
	this->ScriptListCType->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ScriptSelectionDialog::ScriptListAspectTypeGetter);
	this->ScriptListCParentPlugin->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ScriptSelectionDialog::ScriptListAspectParentPluginGetter);

	// ### The invocation of the SelectionChanged event apparently is tied to the parent thread's idle message loop
	// ### This breaks event handling for this event when the idle loop isn't active (which are usually active when thread-modal dialog is being shown)
	// ### We need to use SelectedIndexChanged instead.
	ScriptList->SelectedIndexChanged += gcnew EventHandler(this, &ScriptSelectionDialog::ScriptList_SelectionChanged);
	// ### The SelectedIndexChanged event does not update the SelectedObjects collection when the Shift-key selection triggers the event
	// ### We need to workaround this by deferring the original handler by using a timer
	DeferredSelectionUpdateTimer->Tick += gcnew EventHandler(this, &ScriptSelectionDialog::DeferredSelectionUpdateTimer_Tick);
	ScriptList->KeyDown += gcnew KeyEventHandler(this, &ScriptSelectionDialog::ScriptList_KeyDown);
	ScriptList->KeyPress += gcnew KeyPressEventHandler(this, &ScriptSelectionDialog::ScriptList_KeyPress);
	ScriptList->ItemActivate += gcnew EventHandler(this, &ScriptSelectionDialog::ScriptList_ItemActivate);
	BottomToolbarCompleteSelection->Click += gcnew EventHandler(this, &ScriptSelectionDialog::BottomToolbarCompleteSelection_Click);
	MenuSelectAllActive->Click += gcnew EventHandler(this, &ScriptSelectionDialog::MenuSelectAllActive_Click);
	MenuSelectAllDeleted->Click += gcnew EventHandler(this, &ScriptSelectionDialog::MenuSelectAllDeleted_Click);
	MenuSelectAllUncompiled->Click += gcnew EventHandler(this, &ScriptSelectionDialog::MenuSelectAllUncompiled_Click);
	BottomToolbarTextboxFilter->TextChanged += gcnew EventHandler(this, &ScriptSelectionDialog::BottomToolbarTextboxFilter_TextChanged);
	BottomToolbarTextboxFilter->KeyDown += gcnew KeyEventHandler(this, &ScriptSelectionDialog::BottomToolbarTextboxFilter_KeyDown);
	this->Closing += gcnew CancelEventHandler(this, &ScriptSelectionDialog::Dialog_Cancel);
	this->Load += gcnew System::EventHandler(this, &ScriptSelectionDialog::Dialog_Load);

	// Set the sort fields in the columns' tag fields
	ScriptListCFlags->Tag = ScriptCollectionSorter::eSortField::Flags;
	ScriptListCScriptName->Tag = ScriptCollectionSorter::eSortField::EditorID;
	ScriptListCFormID->Tag = ScriptCollectionSorter::eSortField::FormID;
	ScriptListCType->Tag = ScriptCollectionSorter::eSortField::Type;
	ScriptListCParentPlugin->Tag = ScriptCollectionSorter::eSortField::ParentPlugin;

	ScriptList->VirtualListDataSource = gcnew FastScriptListViewDataSource(ScriptList);

	auto EmptyMsgOverlay = safe_cast<BrightIdeasSoftware::TextOverlay^>(ScriptList->EmptyListMsgOverlay);
	EmptyMsgOverlay->TextColor = Color::White;
	EmptyMsgOverlay->BackColor = Color::FromArgb(75, 29, 32, 33);
	EmptyMsgOverlay->BorderWidth = 0.f;

	/*bool DarkMode = preferences::SettingsHolder::Get()->Appearance->DarkMode;
	auto BackColor = DarkMode ? DevComponents::DotNetBar::StyleManager::MetroColorGeneratorParameters.VisualStudio2012Dark.CanvasColor : DevComponents::DotNetBar::StyleManager::MetroColorGeneratorParameters.VisualStudio2012Light.CanvasColor;
	auto AccentColor = preferences::SettingsHolder::Get()->Appearance->AccentColor;
	auto ForeColor = DarkMode ? Color::White : Color::Black;

	ScriptList->ForeColor = ForeColor;
	ScriptList->BackColor = BackColor;*/
}


void ScriptSelectionDialog::ShowUseReport()
{
	if (ScriptList->SelectedObject != nullptr)
	{
		NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)ScriptList->SelectedObject;
		nativeWrapper::g_CSEInterfaceTable->EditorAPI.ShowUseReportDialog(Data->ScriptData->EditorID);
	}
}

void ScriptSelectionDialog::CompleteSelection()
{
	if (ScriptList->SelectedObjects->Count == 0)
	{
		MessageBox::Show("Please select one or more scripts to continue.",
						 view::IScriptEditorView::MainWindowDefaultTitle,
						 MessageBoxButtons::OK,
						 MessageBoxIcon::Information);
		return;
	}

	if (HasResult)
		return;

	HasResult = true;
	for each (Object ^ Itr in ScriptList->SelectedObjects)
	{
		NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)Itr;
		ResultData->SelectedScriptEditorIDs->Add(Data->EditorID);
	}

	this->Close();
}

void ScriptSelectionDialog::PopulateLoadedScripts(String^ FilterString, bool DefaultSelection, bool SortByFlags)
{
	ScriptList->DeselectAll();
	ScriptList->ClearObjects();
	ListDataSource->Clear();
	DisabledScripts->Clear();
	ScriptTextPreview->Text = "";

	NativeScriptDataWrapper^ DefaultSelectedItem = nullptr;
	if (LoadedScripts)
	{
		for (int i = 0; i < LoadedScripts->ScriptCount; i++)
		{
			componentDLLInterface::ScriptData* ThisScript = &LoadedScripts->ScriptListHead[i];
			if (ThisScript->IsValid() == false)
				continue;
			else if (Parameters->ShowDeletedScripts == false && ThisScript->IsDeleted())
				continue;

			String^ ScriptEID = gcnew String(ThisScript->EditorID);
			if (FilterString->Length > 0)
			{
				String^ ScriptFormID = ThisScript->FormID.ToString("X8");
				String^ ScriptParent = gcnew String(ThisScript->ParentPluginName);

				if (ScriptEID->IndexOf(FilterString, StringComparison::CurrentCultureIgnoreCase) == -1
					&& ScriptFormID->IndexOf(FilterString, StringComparison::CurrentCultureIgnoreCase) == -1
					&& ScriptParent->IndexOf(FilterString, StringComparison::CurrentCultureIgnoreCase) == -1)
					continue;
			}

			NativeScriptDataWrapper^ NewItem = gcnew NativeScriptDataWrapper(ThisScript);
			ListDataSource->Add(NewItem);
			if (scriptSync::DiskSync::Get()->IsScriptBeingSynced(ScriptEID) && Parameters->PreventSyncedScriptSelection)
				DisabledScripts->Add(NewItem);

			if (DefaultSelection && ScriptEID->Equals(Parameters->SelectedScriptEditorID))
				DefaultSelectedItem = NewItem;
		}
	}

	ScriptList->SetObjects(ListDataSource);
	ScriptList->DisableObjects(DisabledScripts);

	if (SortByFlags)
		ScriptList->Sort(ScriptListCFlags, SortOrder::Ascending);

	if (ScriptList->GetItemCount())
	{
		if (DefaultSelectedItem != nullptr)
		{
			ScriptList->SelectedObject = DefaultSelectedItem;
			ScriptList->EnsureModelVisible(DefaultSelectedItem);
		}
		else
			ScriptList->SelectedIndex = 0;
	}
}

ScriptSelectionDialog::eFlagType ScriptSelectionDialog::GetFlagType(componentDLLInterface::ScriptData* Script)
{
	String^ ScriptEID = gcnew String(Script->EditorID);
	if (scriptSync::DiskSync::Get()->IsScriptBeingSynced(ScriptEID))
		return eFlagType::Syncing;
	else if (Script->Compiled == false)
		return eFlagType::Uncompiled;
	else if (Script->IsActive())
		return eFlagType::Active;
	else if (Script->IsDeleted())
		return eFlagType::Deleted;
	else
		return eFlagType::Default;
}

System::Object^ ScriptSelectionDialog::ScriptListAspectScriptNameGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
	return Data->EditorID;
}

System::Object^ ScriptSelectionDialog::ScriptListAspectFormIDGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
	return Data->ScriptData->FormID;
}

System::String^ ScriptSelectionDialog::ScriptListAspectToStringConverterFormID(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	return ((UInt32)RowObject).ToString("X8");
}

System::Object^ ScriptSelectionDialog::ScriptListAspectTypeGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
	return Data->Type;
}

System::Object^ ScriptSelectionDialog::ScriptListAspectParentPluginGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
	return Data->ParentPluginName;
}

System::Object^ ScriptSelectionDialog::ScriptListAspectFlagsGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
	return GetFlagType(Data->ScriptData);
}

System::String^ ScriptSelectionDialog::ScriptListAspectToStringConverterFlags(Object^ RowObject)
{
	return String::Empty;
}

System::Object^ ScriptSelectionDialog::ScriptListImageFlagsGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
	return GetFlagIcon(GetFlagType(Data->ScriptData));
}

ScriptSelectionDialog::ScriptSelectionDialog(Params^ Parameters)
{
	InitializeComponent();
	FinalizeComponents();

	HasResult = false;
	ResultData = gcnew SelectionResult;

	this->Parameters = Parameters;
	LoadedScripts = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetScriptList();
	ListDataSource = gcnew List<NativeScriptDataWrapper ^>;
	DisabledScripts = gcnew List<NativeScriptDataWrapper ^>;

	PopulateLoadedScripts(Parameters->FilterString, true, preferences::SettingsHolder::Get()->General->SortScriptsByFlags);
	BottomToolbarCompleteSelection->Enabled = false;

	if (Parameters->ParentWindowHandle != IntPtr::Zero)
		this->ShowDialog(gcnew WindowHandleWrapper(Parameters->ParentWindowHandle));
	else
		this->ShowDialog();
}

ScriptSelectionDialog::~ScriptSelectionDialog()
{
	if (LoadedScripts)
		nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(LoadedScripts);

	if (components)
	{
		delete components;
	}
}


} // namespace selectScript


} // namespace scriptEditor


} // namespace cse
