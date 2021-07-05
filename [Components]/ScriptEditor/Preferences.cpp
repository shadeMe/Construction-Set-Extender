#include "Preferences.h"
#include "[Common]\NativeWrapper.h"
#include "IntelliSenseBackend.h"
#include "IScriptEditorView.h"

namespace cse
{


namespace scriptEditor
{


namespace preferences
{


System::Drawing::Design::UITypeEditorEditStyle CustomColorEditor::GetEditStyle(ITypeDescriptorContext^ context)
{
	return System::Drawing::Design::UITypeEditorEditStyle::Modal;
}

System::Object^ CustomColorEditor::EditValue(ITypeDescriptorContext^ context, IServiceProvider^ provider, Object^ value)
{
	if (value->GetType() != Color::typeid)
		return value;

	auto Picker = gcnew ColorDialog;
	Picker->Color = (Color)value;
	Picker->AllowFullOpen = true;
	Picker->FullOpen = true;
	Picker->AnyColor = true;

	auto ParentWindowHandle = PreferencesDialog::GetActiveInstance()->Handle;
	if (Picker->ShowDialog(gcnew WindowHandleWrapper(ParentWindowHandle)) == Windows::Forms::DialogResult::OK)
		return Picker->Color;

	return value;
}

bool CustomColorEditor::GetPaintValueSupported(ITypeDescriptorContext^ context)
{
	return true;
}

void CustomColorEditor::PaintValue(System::Drawing::Design::PaintValueEventArgs^ e)
{
	Color ColorToPaint = (Color)e->Value;

	auto Brush = gcnew SolidBrush(ColorToPaint);
	e->Graphics->FillRectangle(Brush, e->Bounds);
	e->Graphics->DrawRectangle(Pens::Black, e->Bounds);
}

System::Drawing::Design::UITypeEditorEditStyle CustomFontEditor::GetEditStyle(ITypeDescriptorContext^ context)
{
	return System::Drawing::Design::UITypeEditorEditStyle::Modal;
}

System::Object^ CustomFontEditor::EditValue(ITypeDescriptorContext^ context, IServiceProvider^ provider, Object^ value)
{
	auto FontDlg = gcnew FontDialog;
	FontDlg->MinSize = 10;
	FontDlg->MaxSize = 32;
	FontDlg->ShowApply = false;
	FontDlg->ShowColor = false;
	FontDlg->ShowHelp = false;
	FontDlg->FontMustExist = false;
	//FontDlg->ScriptsOnly = false;
	//FontDlg->AllowScriptChange = false;

	//auto Font = safe_cast<System::Drawing::Font^>(value);
	//if (Font != nullptr)
	//	FontDlg->Font = Font;

	auto ParentWindowHandle = PreferencesDialog::GetActiveInstance()->Handle;
	if (FontDlg->ShowDialog(gcnew WindowHandleWrapper(ParentWindowHandle)) == DialogResult::OK)
		return FontDlg->Font;

	return value;
}

System::String^ SettingsGroup::GetCategoryName()
{
	return safe_cast<String^>(this->GetType()->GetField("CategoryName")->GetValue(this));
}

System::String^ SettingsGroup::GetIconName()
{
	return safe_cast<String^>(this->GetType()->GetField("IconName")->GetValue(this));
}

bool SettingsGroup::IsPropertySerializable(System::Reflection::PropertyInfo^ Property)
{
	return !Attribute::IsDefined(Property, BrowsableAttribute::typeid);
}

void SettingsGroup::Save()
{
	Type^ GroupType = this->GetType();
	try
	{
		String^ CategoryName = safe_cast<String^>(GroupType->GetField("CategoryName")->GetValue(this));
		for each (System::Reflection::PropertyInfo ^ Property in GroupType->GetProperties())
		{
			if (!IsPropertySerializable(Property))
				continue;

			String^ Name = Property->Name;
			Type^ PropertyType = Property->PropertyType;
			Object^ Value = Property->GetValue(this, nullptr);
			String^ ValueStr = nullptr;

			if (PropertyType == Drawing::Font::typeid)
			{
				FontConverter^ Converter = gcnew FontConverter;
				ValueStr = Converter->ConvertToInvariantString(dynamic_cast<Drawing::Font^>(Value));
			}
			else if (PropertyType == Color::typeid)
				ValueStr = safe_cast<Color>(Value).ToArgb().ToString();
			else if (PropertyType == bool::typeid)
				ValueStr = safe_cast<Boolean>(Value) == true ? "1" : "0";
			else
				ValueStr = Value->ToString();

			nativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI(CString(Name).c_str(),
				CString(SectionPrefix + CategoryName).c_str(), CString(ValueStr).c_str());
		}
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't save SettingsGroup " + GroupType->Name + "! Exception: " + E->Message, true);
	}
}

bool SettingsGroup::Load()
{
	bool Success = false;
	Type^ GroupType = this->GetType();
	try
	{
		String^ CategoryName = safe_cast<String^>(GroupType->GetField("CategoryName")->GetValue(this));
		for each (System::Reflection::PropertyInfo ^ Property in GroupType->GetProperties())
		{
			if (!IsPropertySerializable(Property))
				continue;

			String^ Name = Property->Name;
			Object^ OldValue = Property->GetValue(this, nullptr);
			Type^ PropertyType = Property->PropertyType;

			char Buffer[0x200] = { 0 };
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI(CString(Name).c_str(),
				CString(SectionPrefix + CategoryName).c_str(), CString(gcnew String("")).c_str(),
				Buffer, sizeof(Buffer));

			String^ ValueStr = gcnew String(Buffer);
			if (ValueStr->Length == 0)
			{
				// DebugPrint("Unable to load setting " + CategoryName + ":" + Name + "! Using original value");
				continue;
			}

			Object^ NewValue = nullptr;
			if (PropertyType == Drawing::Font::typeid)
			{
				FontConverter^ Converter = gcnew FontConverter;
				NewValue = Converter->ConvertFromInvariantString(ValueStr);
			}
			else if (PropertyType == Color::typeid)
				NewValue = Color::FromArgb(Convert::ToInt32(ValueStr));
			else if (PropertyType == UInt32::typeid)
				NewValue = UInt32::Parse(ValueStr);
			else if (PropertyType == bool::typeid)
				NewValue = ValueStr == "1" ? true : false;
			else if (PropertyType->IsEnum)
				NewValue = Enum::Parse(PropertyType, ValueStr, true);
			else if (PropertyType == String::typeid)
				NewValue = ValueStr;
			else
				throw gcnew System::Runtime::Serialization::SerializationException
				("Unsupported property type " + PropertyType->ToString() + " for property " + Name);

			Property->SetValue(this, NewValue, nullptr);
		}

		Success = true;
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't load SettingsGroup " + GroupType->Name + "! Exception: " + E->Message, true);
	}

	return Success;
}

SettingsGroup^ SettingsGroup::Clone()
{
	// A shallow copy is good enough for our use case
	return safe_cast<SettingsGroup^>(this->MemberwiseClone());
}

bool GeneralSettings::Validate(SettingsGroup^ OldValue, String^% OutMessage)
{
	bool Success = true;
	String^ Errors = "";
	GeneralSettings^ Old = safe_cast<GeneralSettings^>(OldValue);

	if (ExportedScriptFileExtension != Old->ExportedScriptFileExtension && scriptEditor::scriptSync::DiskSync::Get()->InProgress)
	{
		Success = false;
		Errors = "ExportedScriptFileExtension cannot be modified when a syncing operation is in progress";
		Errors += "\n";
		ExportedScriptFileExtension = Old->ExportedScriptFileExtension;
	}
	else if (ExportedScriptFileExtension->Length < 2 || ExportedScriptFileExtension[0] != '.' || ExportedScriptFileExtension == scriptEditor::scriptSync::SyncedScriptData::LogFileExtension)
	{
		Success = false;
		Errors = "ExportedScriptFileExtension must start with '.' and cannot be '" + scriptEditor::scriptSync::SyncedScriptData::LogFileExtension + "'";
		Errors += "\n";
		ExportedScriptFileExtension = Old->ExportedScriptFileExtension;
	}

	OutMessage = Errors;
	return Success;
}

bool IntelliSenseSettings::Validate(SettingsGroup^ OldValue, String^% OutMessage)
{
	bool Success = true;
	String^ Errors = "";
	IntelliSenseSettings^ Old = safe_cast<IntelliSenseSettings^>(OldValue);

	const UInt32 kMinWindowWidth = 200, kMaxWindowWidth = 600;
	if (WindowWidth < kMinWindowWidth|| WindowWidth > kMaxWindowWidth)
	{
		Success = false;
		Errors += "WindowWidth must be between " + kMinWindowWidth + " and " + kMaxWindowWidth;
		Errors += "\n";
		WindowWidth = Old->WindowWidth;
	}

	const UInt32 kMinSuggestionCharThreshold = 1, kMaxSuggestionCharThreshold = 20;
	if (SuggestionCharThreshold < kMinSuggestionCharThreshold || SuggestionCharThreshold > kMaxSuggestionCharThreshold)
	{
		Success = false;
		Errors += "SuggestionCharThreshold must be between " + kMinSuggestionCharThreshold + " and " + kMaxSuggestionCharThreshold;
		Errors += "\n";
		SuggestionCharThreshold = Old->SuggestionCharThreshold;
	}

	const UInt32 kMinMaxSuggestionsToDisplay = 3, kMaxMaxSuggestionsToDisplay = 20;
	if (MaxSuggestionsToDisplay < kMinMaxSuggestionsToDisplay || MaxSuggestionsToDisplay > kMaxMaxSuggestionsToDisplay)
	{
		Success = false;
		Errors += "MaxSuggestionsToDisplay must be between " + kMinMaxSuggestionsToDisplay + " and " + kMaxMaxSuggestionsToDisplay;
		Errors += "\n";
		MaxSuggestionsToDisplay = Old->MaxSuggestionsToDisplay;
	}

	const UInt32 kMinInsightToolTipDisplayDuration = 1, kMaxInsightToolTipDisplayDuration = 60;
	if (InsightToolTipDisplayDuration < kMinInsightToolTipDisplayDuration || InsightToolTipDisplayDuration > kMaxInsightToolTipDisplayDuration)
	{
		Success = false;
		Errors += "InsightToolTipDisplayDuration must be between " + kMinInsightToolTipDisplayDuration + " and " + kMaxInsightToolTipDisplayDuration;
		Errors += "\n";
		InsightToolTipDisplayDuration = Old->InsightToolTipDisplayDuration;
	}

	const UInt32 kMinDatabaseUpdateInterval = 1, kMaxDatabaseUpdateInterval = 60;
	if (DatabaseUpdateInterval < kMinDatabaseUpdateInterval || DatabaseUpdateInterval > kMaxDatabaseUpdateInterval)
	{
		Success = false;
		Errors += "DatabaseUpdateInterval must be between " + kMinDatabaseUpdateInterval + " and " + kMaxDatabaseUpdateInterval;
		Errors += "\n";
		DatabaseUpdateInterval = Old->DatabaseUpdateInterval;
	}

	const UInt32 kMinBackgroundAnalysisInterval = 2, kMaxBackgroundAnalysisInterval = 60;
	if (BackgroundAnalysisInterval < kMinBackgroundAnalysisInterval || BackgroundAnalysisInterval > kMaxBackgroundAnalysisInterval)
	{
		Success = false;
		Errors += "BackgroundAnalysisInterval must be between " + kMinBackgroundAnalysisInterval + " and " + kMaxBackgroundAnalysisInterval;
		Errors += "\n";
		BackgroundAnalysisInterval = Old->BackgroundAnalysisInterval;
	}

	OutMessage = Errors;
	return Success;
}

bool PreprocessorSettings::Validate(SettingsGroup^ OldValue, String^% OutMessage)
{
	bool Success = true;
	String^ Errors = "";
	PreprocessorSettings^ Old = safe_cast<PreprocessorSettings^>(OldValue);

	const UInt32 kMinNumPasses = 1, kMaxNumPasses = 10;
	if (NumPasses < kMinNumPasses || NumPasses > kMaxNumPasses)
	{
		Success = false;
		Errors = "NumPasses must be between " + kMinNumPasses + " and " + kMaxNumPasses;
		Errors += "\n";
		NumPasses = Old->NumPasses;
	}

	OutMessage = Errors;
	return Success;
}

bool AppearanceSettings::Validate(SettingsGroup^ OldValue, String^% OutMessage)
{
	bool Success = true;
	String^ Errors = "";
	AppearanceSettings^ Old = safe_cast<AppearanceSettings^>(OldValue);

	const UInt32 kMinTabSize = 1, kMaxTabSize = 10;
	if (TabSize < kMinTabSize || TabSize > kMaxTabSize)
	{
		Success = false;
		Errors = "TabSize must be between " + kMinTabSize + " and " + kMaxTabSize;
		Errors += "\n";
		TabSize = Old->TabSize;
	}

	OutMessage = Errors;
	return Success;
}

bool BackupSettings::Validate(SettingsGroup^ OldValue, String^% OutMessage)
{
	bool Success = true;
	String^ Errors = "";
	BackupSettings^ Old = safe_cast<BackupSettings^>(OldValue);

	const UInt32 kMinAutoRecoveryInterval = 1, kMaxAutoRecoveryInterval = 30;
	if (AutoRecoveryInterval < kMinAutoRecoveryInterval || AutoRecoveryInterval > kMaxAutoRecoveryInterval)
	{
		Success = false;
		Errors = "AutoRecoveryInterval must be between " + kMinAutoRecoveryInterval + " and " + kMaxAutoRecoveryInterval;
		Errors += "\n";
		AutoRecoveryInterval = Old->AutoRecoveryInterval;
	}

	OutMessage = Errors;
	return Success;
}


bool ScriptSyncSettings::Validate(SettingsGroup^ OldValue, String^% OutMessage)
{
	bool Success = true;
	String^ Errors = "";
	ScriptSyncSettings^ Old = safe_cast<ScriptSyncSettings^>(OldValue);

	const UInt32 kMinAutoSyncInterval = 1, kMaxAutoSyncInterval = 120;
	if (AutoSyncInterval < kMinAutoSyncInterval || AutoSyncInterval > kMaxAutoSyncInterval)
	{
		Success = false;
		Errors = "AutoSyncInterval must be between " + kMinAutoSyncInterval + " and " + kMaxAutoSyncInterval;
		Errors += "\n";
		AutoSyncInterval = Old->AutoSyncInterval;
	}

	OutMessage = Errors;
	return Success;
}

SettingsHolder::SettingsHolder()
{
	General = gcnew GeneralSettings;
	IntelliSense = gcnew IntelliSenseSettings;
	Preprocessor = gcnew PreprocessorSettings;
	Appearance = gcnew AppearanceSettings;
	Sanitizer = gcnew SanitizerSettings;
	Backup = gcnew BackupSettings;
	Validator = gcnew ValidatorSettings;
	FindReplace = gcnew FindReplaceSettings;
	ScriptSync = gcnew ScriptSyncSettings;

	AllGroups = gcnew List<SettingsGroup^>;

	AllGroups->Add(General);
	AllGroups->Add(IntelliSense);
	AllGroups->Add(Preprocessor);
	AllGroups->Add(Appearance);
	AllGroups->Add(Sanitizer);
	AllGroups->Add(Backup);
	AllGroups->Add(Validator);
	AllGroups->Add(FindReplace);
	AllGroups->Add(ScriptSync);
}

void SettingsHolder::SaveToDisk()
{
	for each (SettingsGroup^ Group in AllGroups)
		Group->Save();

	PreferencesChanged(this, gcnew EventArgs);
}

void SettingsHolder::LoadFromDisk()
{
	for each (SettingsGroup^ Group in AllGroups)
	{
		SettingsGroup^ Snapshot = Group->Clone();
		if (Group->Load())
		{
			String^ ValidationMessage = "";
			Group->Validate(Snapshot, ValidationMessage);
		}
	}
}

void SettingsHolder::RaisePreferencesChangedEvent()
{
	PreferencesChanged(this, gcnew EventArgs);
}

SettingsHolder^ SettingsHolder::Get()
{
	if (Singleton == nullptr)
		Singleton = gcnew SettingsHolder;

	return Singleton;
}


void PreferencesDialog::SidebarSettingsCategoryButton_Click(Object^ Sender, EventArgs^ E)
{
	Debug::Assert(CurrentSelection != nullptr);

	auto Button = safe_cast<DevComponents::DotNetBar::ButtonItem^>(Sender);
	auto Group = safe_cast<SettingsGroup^>(Button->Tag);

	if (Button == RegisteredCategories[CurrentSelection])
		return;

	SwitchCategory(Group);
}

void PreferencesDialog::Dialog_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	if (CurrentSelection)
	{
		String^ ValidationMessage = "";
		if (CurrentSelection->Validate(CurrentSelectionSnapshot, ValidationMessage) == false)
		{
			ValidationMessage = ValidationMessage->TrimEnd();
			auto MsgBoxResult = MessageBox::Show("Some values are invalid!\n\n" + ValidationMessage
												 + "\n\nAre you sure you want to close and revert the above changes?",
												 view::IScriptEditorView::MainWindowDefaultTitle, MessageBoxButtons::YesNo, MessageBoxIcon::Stop);
			if (MsgBoxResult == Windows::Forms::DialogResult::No)
			{
				E->Cancel = true;
				return;
			}
		}
	}

	SettingsHolder::Get()->SaveToDisk();
}

void PreferencesDialog::InitializeComponent(void)
{
	this->PropertyGrid = (gcnew DevComponents::DotNetBar::AdvPropertyGrid());
	this->TopBar = (gcnew DevComponents::DotNetBar::Bar());
	this->LabelCategories = (gcnew DevComponents::DotNetBar::LabelItem());
	this->LabelCurrentCategory = (gcnew DevComponents::DotNetBar::LabelItem());
	this->SidebarSettingsCategories = (gcnew DevComponents::DotNetBar::Bar());
	this->ContainerPropertyGrid = (gcnew DevComponents::DotNetBar::PanelEx());
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->PropertyGrid))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->TopBar))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->SidebarSettingsCategories))->BeginInit();
	this->ContainerPropertyGrid->SuspendLayout();
	this->SuspendLayout();
	//
	// PropertyGrid
	//
	this->PropertyGrid->Dock = System::Windows::Forms::DockStyle::Fill;
	this->PropertyGrid->Location = System::Drawing::Point(0, 0);
	this->PropertyGrid->Name = L"PropertyGrid";
	this->PropertyGrid->Size = System::Drawing::Size(398, 490);
	this->PropertyGrid->HelpType = DevComponents::DotNetBar::ePropertyGridHelpType::Panel;
	this->PropertyGrid->TabIndex = 0;
	//
	// TopBar
	//
	this->TopBar->AntiAlias = true;
	this->TopBar->Dock = System::Windows::Forms::DockStyle::Top;
	this->TopBar->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
	this->TopBar->IsMaximized = false;
	this->TopBar->Items->AddRange(gcnew cli::array< DevComponents::DotNetBar::BaseItem^  >(1) { this->LabelCategories });
	this->TopBar->Location = System::Drawing::Point(0, 0);
	this->TopBar->Name = L"TopBar";
	this->TopBar->PaddingBottom = 8;
	this->TopBar->PaddingTop = 8;
	this->TopBar->Size = System::Drawing::Size(434, 41);
	this->TopBar->Stretch = true;
	this->TopBar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->TopBar->TabIndex = 1;
	this->TopBar->TabStop = false;
	this->TopBar->Text = L"bar1";
	//
	// LabelCategories
	//
	this->LabelCategories->Name = L"LabelCategories";
	this->LabelCategories->Symbol = L"";
	this->LabelCategories->Text = L"Settings Category";
	//
	// LabelCurrentCategory
	//
	this->LabelCurrentCategory->ItemAlignment = DevComponents::DotNetBar::eItemAlignment::Far;
	this->LabelCurrentCategory->Name = L"LabelCurrentCategory";
	this->LabelCurrentCategory->Text = L"Current Category";
	//
	// SidebarSettingsCategories
	//
	this->SidebarSettingsCategories->AntiAlias = true;
	this->SidebarSettingsCategories->Dock = System::Windows::Forms::DockStyle::Left;
	this->SidebarSettingsCategories->DockOrientation = DevComponents::DotNetBar::eOrientation::Vertical;
	this->SidebarSettingsCategories->Font = (gcnew System::Drawing::Font(L"Segoe UI", 9));
	this->SidebarSettingsCategories->IsMaximized = false;
	this->SidebarSettingsCategories->ItemSpacing = 5;
	this->SidebarSettingsCategories->Location = System::Drawing::Point(0, 41);
	this->SidebarSettingsCategories->Name = L"SidebarSettingsCategories";
	this->SidebarSettingsCategories->PaddingLeft = 15;
	this->SidebarSettingsCategories->PaddingRight = 15;
	this->SidebarSettingsCategories->Size = System::Drawing::Size(36, 490);
	this->SidebarSettingsCategories->Stretch = true;
	this->SidebarSettingsCategories->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->SidebarSettingsCategories->TabIndex = 2;
	this->SidebarSettingsCategories->TabStop = false;
	this->SidebarSettingsCategories->Text = L"bar2";
	//
	// ContainerPropertyGrid
	//
	this->ContainerPropertyGrid->CanvasColor = System::Drawing::SystemColors::Control;
	this->ContainerPropertyGrid->ColorSchemeStyle = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	this->ContainerPropertyGrid->Controls->Add(this->PropertyGrid);
	this->ContainerPropertyGrid->DisabledBackColor = System::Drawing::Color::Empty;
	this->ContainerPropertyGrid->Dock = System::Windows::Forms::DockStyle::Fill;
	this->ContainerPropertyGrid->Location = System::Drawing::Point(36, 41);
	this->ContainerPropertyGrid->Name = L"ContainerPropertyGrid";
	this->ContainerPropertyGrid->Size = System::Drawing::Size(398, 490);
	this->ContainerPropertyGrid->Style->Alignment = System::Drawing::StringAlignment::Center;
	this->ContainerPropertyGrid->Style->BackColor1->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBackground;
	this->ContainerPropertyGrid->Style->Border = DevComponents::DotNetBar::eBorderType::SingleLine;
	this->ContainerPropertyGrid->Style->BorderColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelBorder;
	this->ContainerPropertyGrid->Style->ForeColor->ColorSchemePart = DevComponents::DotNetBar::eColorSchemePart::PanelText;
	this->ContainerPropertyGrid->Style->GradientAngle = 90;
	this->ContainerPropertyGrid->TabIndex = 3;
	this->ContainerPropertyGrid->Text = L"panelEx1";
	//
	// ScriptEditorPreferences
	//
	this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
	this->ClientSize = System::Drawing::Size(434, 531);
	this->Controls->Add(this->ContainerPropertyGrid);
	this->Controls->Add(this->SidebarSettingsCategories);
	this->Controls->Add(this->TopBar);
	this->DoubleBuffered = true;
	this->MinimizeBox = false;
	this->MinimumSize = System::Drawing::Size(450, 550);
	this->Name = L"ScriptEditorPreferences";
	this->ShowIcon = false;
	this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
	this->Text = L"Preferences";
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->PropertyGrid))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->TopBar))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->SidebarSettingsCategories))->EndInit();
	this->ContainerPropertyGrid->ResumeLayout(false);
	this->ResumeLayout(false);

	this->PropertyGrid->Toolbar->Items->Add(LabelCurrentCategory);
	this->Closing += gcnew CancelEventHandler(this, &PreferencesDialog::Dialog_Cancel);
}

bool PreferencesDialog::PopulateCategories()
{
	try
	{
		for each (SettingsGroup^ Group in SettingsHolder::Get()->AllGroups)
		{
			auto NewButton = gcnew DevComponents::DotNetBar::ButtonItem;
			auto ImageResources = view::components::CommonIcons::Get()->ResourceManager;

			NewButton->Text = Group->GetCategoryName();
			NewButton->Tooltip = Group->GetCategoryName();
			NewButton->Image = ImageResources->CreateImage(Group->GetIconName());
			NewButton->Click += gcnew System::EventHandler(this, &PreferencesDialog::SidebarSettingsCategoryButton_Click);
			NewButton->Tag = Group;
			NewButton->Checked = false;

			if (NewButton->Image)
				NewButton->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::Default;
			else
				NewButton->ButtonStyle = DevComponents::DotNetBar::eButtonStyle::TextOnlyAlways;

			SidebarSettingsCategories->Items->Add(NewButton);
			RegisteredCategories->Add(Group, NewButton);
		}
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't populate setting categories! Exception: " + E->Message, true);
		return false;
	}

	return true;
}

bool PreferencesDialog::SwitchCategory(SettingsGroup^ Group)
{
	if (CurrentSelection)
	{
		String^ ValidationMessage = "";
		if (CurrentSelection->Validate(CurrentSelectionSnapshot, ValidationMessage) == false)
		{
			ValidationMessage = ValidationMessage->TrimEnd();
			MessageBox::Show("Some values are invalid!\n\n" + ValidationMessage,
								view::IScriptEditorView::MainWindowDefaultTitle, MessageBoxButtons::OK, MessageBoxIcon::Stop);
			return false;
		}
	}

	SettingsGroup^ OldSelection = CurrentSelection;
	CurrentSelection = Group;
	CurrentSelectionSnapshot = CurrentSelection->Clone();

	auto NewButton = RegisteredCategories[CurrentSelection];
	NewButton->Checked = true;

	if (OldSelection != nullptr)
	{
		auto OldButton = RegisteredCategories[OldSelection];
		OldButton->Checked = false;
	}

	PropertyGrid->SelectedObject = CurrentSelection;
	LabelCurrentCategory->Text = CurrentSelection->GetCategoryName();
	return true;
}

PreferencesDialog::PreferencesDialog()
{
	RegisteredCategories = gcnew Dictionary<SettingsGroup^, DevComponents::DotNetBar::ButtonItem^>;
	CurrentSelection = nullptr;
	CurrentSelectionSnapshot = nullptr;

	InitializeComponent();
	PropertyGrid->GridLinesColor = DevComponents::DotNetBar::StyleManager::MetroColorGeneratorParameters.CanvasColor;

	if (PopulateCategories() == false)
	{
		MessageBox::Show("The preferences dialog couldn't be initialized. Check the console for more information.",
						 view::IScriptEditorView::MainWindowDefaultTitle,
						 MessageBoxButtons::OK,
						 MessageBoxIcon::Error);
		this->Close();
		return;
	}

	auto Enumerator = RegisteredCategories->Keys->GetEnumerator();
	Enumerator.MoveNext();
	SwitchCategory(Enumerator.Current);

	// ### HACK! In order to correctly blacklist the font and color common dialogs from being
	// affected by the custom editor color theme, we'll need to ensure that they are instantiated
	// as child windows of a Winforms dialog (which they aren't by default)
	// So, we need the handle of the parent form in order to pass it to their respective
	// ShowDialog() calls. Since this can't be accessed directly through the custom service provider API,
	// we'll need to cache the handle elsewhere. This means we can only have one active preferences dialog,
	// which is fine as long as we show it as an application-wide modal dialog

	Debug::Assert(ActiveDialog == nullptr);
	ActiveDialog = this;

	this->ShowDialog();

	// We need to instantiate these classes explicitly as the linker
	// will not recognize their being referenced in the custom CLR attributes
	// and automatically remove them if optimizations are turned on
	auto ThrowawayA = gcnew CustomColorConverter();
	delete ThrowawayA;

	auto ThrowawayB = gcnew CustomColorEditor();
	delete ThrowawayB;

	auto ThrowAwayC = gcnew CustomFontEditor();
	delete ThrowAwayC;
}

PreferencesDialog::~PreferencesDialog()
{
	Debug::Assert(ActiveDialog == this);
	ActiveDialog = nullptr;

	if (components)
	{
		delete components;
	}
}


} // namespace preferences


} // namespace scriptEditor


} // namespace cse