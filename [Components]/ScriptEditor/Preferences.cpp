#include "Preferences.h"
#include "[Common]\NativeWrapper.h"
#include "Globals.h"
#include "IntelliSenseDatabase.h"

namespace cse
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

		void SettingsGroup::Save()
		{
			Type^ GroupType = this->GetType();
			try
			{
				String^ CategoryName = safe_cast<String^>(GroupType->GetField("CategoryName")->GetValue(this));
				for each (System::Reflection::PropertyInfo ^ Property in GroupType->GetProperties())
				{
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

			const UInt32 kMinSuggestionCharThreshold = 1, kMaxSuggestionCharThreshold = 10;
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

			if (ScriptFileExtension != Old->ScriptFileExtension && scriptEditor::scriptSync::DiskSync::Get()->InProgress)
			{
				Success = false;
				Errors = "ScriptFileExtension cannot be modified when a syncing operation is in progress";
				Errors += "\n";
				ScriptFileExtension = Old->ScriptFileExtension;
			}
			else if (ScriptFileExtension->Length < 2 || ScriptFileExtension[0] != '.' || ScriptFileExtension == scriptEditor::scriptSync::SyncedScriptData::LogFileExtension)
			{
				Success = false;
				Errors = "ScriptFileExtension must start with '.' and cannot be '" + scriptEditor::scriptSync::SyncedScriptData::LogFileExtension + "'";
				Errors += "\n";
				ScriptFileExtension = Old->ScriptFileExtension;
			}

			OutMessage = Errors;
			return Success;
		}

		void SettingsHolder::SaveToDisk()
		{
			for each (SettingsGroup^ Group in AllGroups)
				Group->Save();

			SavedToDisk(this, gcnew EventArgs);
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

		SettingsHolder^ SettingsHolder::Get()
		{
			if (Singleton == nullptr)
				Singleton = gcnew SettingsHolder;

			return Singleton;
		}


		void PreferencesDialog::ToolStripCategoryButton_Click(Object^ Sender, EventArgs^ E)
		{
			Debug::Assert(CurrentSelection != nullptr);

			auto Button = safe_cast<ToolStripButton^>(Sender);
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
														SCRIPTEDITOR_TITLE, MessageBoxButtons::YesNo, MessageBoxIcon::Stop);
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
			this->PropertyGrid = (gcnew System::Windows::Forms::PropertyGrid());
			this->ToolStripSettingCategories = (gcnew System::Windows::Forms::ToolStrip());
			this->ToolStripLabelCategories = (gcnew System::Windows::Forms::ToolStripLabel());
			this->LabelCurrentCategory = (gcnew System::Windows::Forms::Label());
			this->ToolStripSettingCategories->SuspendLayout();
			this->SuspendLayout();
			//
			// PropertyGrid
			//
			this->PropertyGrid->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->PropertyGrid->Location = System::Drawing::Point(67, 12);
			this->PropertyGrid->Name = L"PropertyGrid";
			this->PropertyGrid->Size = System::Drawing::Size(355, 437);
			this->PropertyGrid->TabIndex = 1;
			//
			// ToolStripSettingCategories
			//
			this->ToolStripSettingCategories->AutoSize = false;
			this->ToolStripSettingCategories->Dock = System::Windows::Forms::DockStyle::Left;
			this->ToolStripSettingCategories->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->ToolStripSettingCategories->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->ToolStripLabelCategories });
			this->ToolStripSettingCategories->Location = System::Drawing::Point(0, 0);
			this->ToolStripSettingCategories->Name = L"ToolStripSettingCategories";
			this->ToolStripSettingCategories->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
			this->ToolStripSettingCategories->Size = System::Drawing::Size(64, 461);
			this->ToolStripSettingCategories->TabIndex = 2;
			//
			// ToolStripLabelCategories
			//
			this->ToolStripLabelCategories->Name = L"ToolStripLabelCategories";
			this->ToolStripLabelCategories->Padding = System::Windows::Forms::Padding(5);
			this->ToolStripLabelCategories->Size = System::Drawing::Size(62, 25);
			this->ToolStripLabelCategories->Text = L"Categories";
			//
			// LabelCurrentCategory
			//
			this->LabelCurrentCategory->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Right));
			this->LabelCurrentCategory->AutoSize = true;
			this->LabelCurrentCategory->Font = (gcnew System::Drawing::Font(L"Segoe UI", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->LabelCurrentCategory->Location = System::Drawing::Point(292, 9);
			this->LabelCurrentCategory->Name = L"LabelCurrentCategory";
			this->LabelCurrentCategory->Size = System::Drawing::Size(130, 21);
			this->LabelCurrentCategory->TabIndex = 3;
			this->LabelCurrentCategory->Text = L"Current Category";
			this->LabelCurrentCategory->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			//
			// OptionsDialog
			//
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(434, 461);
			this->Controls->Add(this->LabelCurrentCategory);
			this->Controls->Add(this->ToolStripSettingCategories);
			this->Controls->Add(this->PropertyGrid);
			this->MinimizeBox = false;
			this->MinimumSize = System::Drawing::Size(450, 500);
			this->Name = L"OptionsDialog";
			this->ShowIcon = false;
			this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
			this->Text = L"Script Editor Preferences";
			this->ToolStripSettingCategories->ResumeLayout(false);
			this->ToolStripSettingCategories->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

			this->ToolStripSettingCategories->Renderer = gcnew CustomToolStripSystemRenderer;
			this->Closing += gcnew CancelEventHandler(this, &PreferencesDialog::Dialog_Cancel);
		}

		bool PreferencesDialog::PopulateCategories()
		{
			try
			{
				for each (SettingsGroup^ Group in SettingsHolder::Get()->AllGroups)
				{
					String^ CategoryName = Group->GetCategoryName();

					ToolStripButton^ NewButton = gcnew ToolStripButton;
					NewButton->Text = CategoryName;
					NewButton->Image = Globals::ImageResources()->CreateImage("PreferencesDialogCategory" + CategoryName);
					NewButton->Click += gcnew System::EventHandler(this, &PreferencesDialog::ToolStripCategoryButton_Click);
					NewButton->Tag = Group;
					NewButton->CheckState = CheckState::Unchecked;

					if (NewButton->Image)
						NewButton->DisplayStyle = ToolStripItemDisplayStyle::Image;
					else
						NewButton->DisplayStyle = ToolStripItemDisplayStyle::Text;

					ToolStripSettingCategories->Items->Add(NewButton);
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
						SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Stop);
					return false;
				}
			}

			SettingsGroup^ OldSelection = CurrentSelection;
			CurrentSelection = Group;
			CurrentSelectionSnapshot = CurrentSelection->Clone();

			ToolStripButton^ NewButton = RegisteredCategories[CurrentSelection];
			NewButton->CheckState = CheckState::Checked;

			if (OldSelection != nullptr)
			{
				ToolStripButton^ OldButton = RegisteredCategories[OldSelection];
				OldButton->CheckState = CheckState::Unchecked;
			}

			PropertyGrid->SelectedObject = CurrentSelection;
			LabelCurrentCategory->Text = CurrentSelection->GetCategoryName();
			return true;
		}

		PreferencesDialog::PreferencesDialog()
		{
			RegisteredCategories = gcnew Dictionary<SettingsGroup^, ToolStripButton^>;
			CurrentSelection = nullptr;
			CurrentSelectionSnapshot = nullptr;

			InitializeComponent();
			if (PopulateCategories() == false)
			{
				MessageBox::Show("The preferences dialog couldn't be initialized. Check the console for more information.",
					SCRIPTEDITOR_TITLE,
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

			this->Hide();
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



	}
}