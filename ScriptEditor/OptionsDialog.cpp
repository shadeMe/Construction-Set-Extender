#include "OptionsDialog.h"
#include "Common\NativeWrapper.h"
#include "Globals.h"



String^ BoundControl::GetValue()
{
	String^ Result = "0";

	switch (BoundType)				// value type implicitly assumed for all controls except Font & Color dialogs
	{
	case ControlType::e_Checkbox: 
		{
			CheckBox^ Control = dynamic_cast<CheckBox^>(INIControl);
			Result = ((int)Control->Checked).ToString();
			break;
		}
	case ControlType::e_NumericUpDown:
		{
			NumericUpDown^ Control = dynamic_cast<NumericUpDown^>(INIControl);
			Result = Control->Value.ToString();
			break;
		}
	case ControlType::e_FontDialog:
		{
			FontDialog^ Control = dynamic_cast<FontDialog^>(INIDialog);
			switch (Property)
			{
			case ValueType::e_Font_FontFamily_Name:
				Result = Control->Font->FontFamily->Name;
				break;
			case ValueType::e_Font_Size:
				Result = Control->Font->Size.ToString();
				break;
			}
			break;
		}
	case ControlType::e_ColorDialog:
		{
			ColorDialog^ Control = dynamic_cast<ColorDialog^>(INIDialog);
			switch (Property)
			{
			case ValueType::e_Color_R:
				Result = Control->Color.R.ToString();
				break;
			case ValueType::e_Color_G:
				Result = Control->Color.G.ToString();
				break;
			case ValueType::e_Color_B:
				Result = Control->Color.B.ToString();
				break;
			}
			break;
		}
	}

	return Result;
}

void BoundControl::SetValue(String^ Value)
{
	float Numeric = 0;

	try {	
		Numeric = float::Parse(Value);
	} catch (...) {	
		Numeric = 0;
		if (Property != ValueType::e_Font_FontFamily_Name)
			DebugPrint(String::Format("Couldn't parse INI value of type {0}", (int)Property), true);
	}

	switch (BoundType)
	{
	case ControlType::e_Checkbox: 
		{
			CheckBox^ Control = dynamic_cast<CheckBox^>(INIControl);
			Control->Checked = Numeric?true:false;
			break;
		}
	case ControlType::e_NumericUpDown:
		{
			NumericUpDown^ Control = dynamic_cast<NumericUpDown^>(INIControl);
			Control->Value = (int)Numeric;
			break;
		}
	case ControlType::e_FontDialog:
		{
			FontDialog^ Control = dynamic_cast<FontDialog^>(INIDialog);
			switch (Property)							// the font name INI key should be parsed before size for correct init.
			{
			case ValueType::e_Font_FontFamily_Name:	
				Control->Font = gcnew Font(Value, 10, FontStyle::Regular);
				break;
			case ValueType::e_Font_Size:
				Control->Font = gcnew Font(Control->Font->FontFamily->Name, Numeric, FontStyle::Regular);
				break;
			}
			break;
		}
	case ControlType::e_ColorDialog:
		{
			ColorDialog^ Control = dynamic_cast<ColorDialog^>(INIDialog);
			switch (Property)							// the component channels must be parsed in the same order as below for correct init.
			{
			case ValueType::e_Color_R:
				Control->Color = Color::FromArgb(255, Numeric, 0, 0);
				break;
			case ValueType::e_Color_G:
				Control->Color = Color::FromArgb(255, Control->Color.R, Numeric, 0);
				break;
			case ValueType::e_Color_B:
				Control->Color = Color::FromArgb(255, Control->Color.R, Control->Color.G, Numeric);
				break;
			}
			break;
		}
	}	
}

void OptionsDialog::PopulateINIMap()
{
	// General
	INIMap->Add(gcnew INISetting("Font", "ScriptEditor::General", "Lucida Console"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_FontFamily_Name));
	INIMap->Add(gcnew INISetting("FontSize", "ScriptEditor::General", "10"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_Size));

	INIMap->Add(gcnew INISetting("ForeColorR", "ScriptEditor::General", "0"), gcnew BoundControl(FCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting("ForeColorG", "ScriptEditor::General", "0"), gcnew BoundControl(FCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting("ForeColorB", "ScriptEditor::General", "0"), gcnew BoundControl(FCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	INIMap->Add(gcnew INISetting("BackColorR", "ScriptEditor::General", "255"), gcnew BoundControl(BCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting("BackColorG", "ScriptEditor::General", "255"), gcnew BoundControl(BCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting("BackColorB", "ScriptEditor::General", "255"), gcnew BoundControl(BCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	INIMap->Add(gcnew INISetting("HiliteColorR", "ScriptEditor::General", "158"), gcnew BoundControl(HCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting("HiliteColorG", "ScriptEditor::General", "195"), gcnew BoundControl(HCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting("HiliteColorB", "ScriptEditor::General", "200"), gcnew BoundControl(HCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	INIMap->Add(gcnew INISetting("BookmarkColorR", "ScriptEditor::General", "235"), gcnew BoundControl(BMCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting("BookmarkColorG", "ScriptEditor::General", "33"), gcnew BoundControl(BMCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting("BookmarkColorB", "ScriptEditor::General", "38"), gcnew BoundControl(BMCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	INIMap->Add(gcnew INISetting("UseRegEx", "ScriptEditor::General", "0"), gcnew BoundControl(UseRegEx, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("ColorEditorBox", "ScriptEditor::General", "0"), gcnew BoundControl(ColorEditorBox, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("AutoIndent", "ScriptEditor::General", "1"), gcnew BoundControl(AutoIndent, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("SaveLastKnownPos", "ScriptEditor::General", "1"), gcnew BoundControl(SaveLastKnownPos, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("TabSize", "ScriptEditor::General", "0"), gcnew BoundControl(TabSize, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	INIMap->Add(gcnew INISetting("RecompileVarIdx", "ScriptEditor::General", "1"), gcnew BoundControl(RecompileVarIdx, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("UseCSParent", "ScriptEditor::General", "0"), gcnew BoundControl(UseCSParent, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("DestroyOnLastTabClose", "ScriptEditor::General", "1"), gcnew BoundControl(DestroyOnLastTabClose, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

	// IntelliSense
	INIMap->Add(gcnew INISetting("ThresholdLength", "ScriptEditor::IntelliSense", "4"), gcnew BoundControl(ThresholdLength, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));


	// Preprocessor
	INIMap->Add(gcnew INISetting("CreateMissingFromSegment", "ScriptEditor::Preprocessor", "1"), gcnew BoundControl(CreateMissingFromSegment, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("AllowRedefinitions", "ScriptEditor::Preprocessor", "1"), gcnew BoundControl(AllowRedefinitions, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("PreprocessorWarnings", "ScriptEditor::Preprocessor", "1"), gcnew BoundControl(PreprocessorWarnings, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

	DebugPrint("Populated INI Map");
}

void OptionsDialog::LoadINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in INIMap) {
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		Control->SetValue(INIWrapper::GetINIValue(INI->Section, INI->Key, INI->DefaultValue));
	}
	DebugPrint("Loaded settings from INI");
}

void OptionsDialog::SaveINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in INIMap) {
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		INIWrapper::SetINIValue(INI->Section, INI->Key, Control->GetValue());
	}
	DebugPrint("Saved settings to INI");
}

BoundControl^ OptionsDialog::FetchSetting(String^ Key)
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in INIMap)
	{
		INISetting^ INI = Itr.Key;
		if (!String::Compare(INI->Key, Key, true))
			return Itr.Value;
	}
	return nullptr;
}

int OptionsDialog::FetchSettingAsInt(String^ Key)
{
	BoundControl^ Control = FetchSetting(Key);
	if (Control)
	{
		String^ Value = Control->GetValue();
		int Result = 0;
		try {
			Result = int::Parse(Value);
		} catch (Exception^ E)
		{
			DebugPrint("Couldn't fetch INI setting '" + Key + "' value.\n\tException: " + E->Message);
		}
		return Result;
	}
	else
	{
		DebugPrint("Couldn't fetch INI setting '" + Key + "' value.\n\tException: Key desn't exist.");
		return 0;
	}
}

String^ OptionsDialog::FetchSettingAsString(String^ Key)
{
	BoundControl^ Control = FetchSetting(Key);
	if (Control)
		return Control->GetValue();
	else
	{
		DebugPrint("Couldn't fetch INI setting '" + Key + "' value.\n\tException: Key desn't exist.");
		return "";
	}
}

OptionsDialog^% OptionsDialog::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew OptionsDialog();
	}
	return Singleton;
}

OptionsDialog::OptionsDialog()
{
	AutoIndent = gcnew CheckBox();
	GroupPreP = gcnew GroupBox();
	CreateMissingFromSegment = gcnew CheckBox();
	AllowRedefinitions = gcnew CheckBox();
	GroupIS = gcnew GroupBox();
	ISThreshold = gcnew Label();
	ThresholdLength = gcnew NumericUpDown();
	GroupGen = gcnew GroupBox();
	UseRegEx = gcnew CheckBox();
	ColorEditorBox = gcnew CheckBox();
	RecompileVarIdx = gcnew CheckBox();
	PreprocessorWarnings = gcnew CheckBox();
	UseCSParent = gcnew CheckBox();
	DestroyOnLastTabClose = gcnew CheckBox();

	FontButton = gcnew Button();
	FCButton = gcnew Button();
	HCButton = gcnew Button();
	BCButton = gcnew Button();
	BMCButton = gcnew Button();

	FontSelection = gcnew FontDialog();
	FCDialog = gcnew ColorDialog();
	BCDialog = gcnew ColorDialog();
	HCDialog = gcnew ColorDialog();
	BMCDialog = gcnew ColorDialog();

	SaveLastKnownPos = gcnew CheckBox();

	TabStopSize = gcnew Label();
	TabSize = gcnew NumericUpDown();
	
	AutoIndent->AutoSize = true;
	AutoIndent->Location = System::Drawing::Point(14, 68);
	AutoIndent->Name = L"AutoIndent";
	AutoIndent->Size = System::Drawing::Size(81, 17);
	AutoIndent->TabIndex = 0;
	AutoIndent->Text = L"Auto-Indentation";
	AutoIndent->UseVisualStyleBackColor = true;
	AutoIndent->AutoSize = true;

	GroupPreP->Controls->Add(CreateMissingFromSegment);
	GroupPreP->Controls->Add(AllowRedefinitions);
	GroupPreP->Controls->Add(PreprocessorWarnings);
	
	GroupPreP->Location = System::Drawing::Point(12, 16);
	GroupPreP->Name = L"GroupPreP";
	GroupPreP->Size = System::Drawing::Size(240, 162);
	GroupPreP->TabIndex = 2;
	GroupPreP->TabStop = false;
	GroupPreP->Text = L"Preprocessor";

	CreateMissingFromSegment->Location = System::Drawing::Point(14, 30);
	CreateMissingFromSegment->Name = L"CreateMissingFromSegment";
	CreateMissingFromSegment->Size = System::Drawing::Size(167, 43);
	CreateMissingFromSegment->TabIndex = 0;
	CreateMissingFromSegment->Text = L"Create missing import segments from code";
	CreateMissingFromSegment->UseVisualStyleBackColor = true;

	AllowRedefinitions->Location = System::Drawing::Point(14, 80);
	AllowRedefinitions->Name = L"AllowRedefinitions";
	AllowRedefinitions->Size = System::Drawing::Size(142, 17);
	AllowRedefinitions->TabIndex = 1;
	AllowRedefinitions->Text = L"Allow macro re-definitions";
	AllowRedefinitions->UseVisualStyleBackColor = true;
	AllowRedefinitions->AutoSize = true;

	PreprocessorWarnings->AutoSize = true;
	PreprocessorWarnings->Location = System::Drawing::Point(13, 111);
	PreprocessorWarnings->Name = L"PreprocessorWarnings";
	PreprocessorWarnings->Size = System::Drawing::Size(130, 17);
	PreprocessorWarnings->TabIndex = 10;
	PreprocessorWarnings->Text = L"Show Script Warnings";
	PreprocessorWarnings->UseVisualStyleBackColor = true;

	GroupIS->Controls->Add(ISThreshold);
	GroupIS->Controls->Add(ThresholdLength);
	GroupIS->Location = System::Drawing::Point(258, 16);
	GroupIS->Name = L"GroupIS";
	GroupIS->Size = System::Drawing::Size(243, 162);
	GroupIS->TabIndex = 3;
	GroupIS->TabStop = false;
	GroupIS->Text = L"IntelliSense";

	ISThreshold->AutoSize = true;
	ISThreshold->Location = System::Drawing::Point(18, 30);
	ISThreshold->Name = L"ISThreshold";
	ISThreshold->Size = System::Drawing::Size(148, 13);
	ISThreshold->TabIndex = 1;
	ISThreshold->Text = L"IntelliSense Pop-up Threshold";

	ThresholdLength->Location = System::Drawing::Point(21, 53);
	ThresholdLength->Name = L"ThresholdLength";
	ThresholdLength->Size = System::Drawing::Size(201, 20);
	ThresholdLength->TabIndex = 0;

	GroupGen->Controls->Add(BCButton);
	GroupGen->Controls->Add(HCButton);
	GroupGen->Controls->Add(FontButton);
	GroupGen->Controls->Add(UseRegEx);
	GroupGen->Controls->Add(ColorEditorBox);
	GroupGen->Controls->Add(AutoIndent);
	GroupGen->Controls->Add(FCButton);
	GroupGen->Controls->Add(SaveLastKnownPos);
	GroupGen->Controls->Add(BMCButton);
	GroupGen->Controls->Add(TabStopSize);
	GroupGen->Controls->Add(TabSize);
	GroupGen->Controls->Add(RecompileVarIdx);
	GroupGen->Controls->Add(UseCSParent);
	GroupGen->Controls->Add(DestroyOnLastTabClose);
	
	
	GroupGen->Location = System::Drawing::Point(12, 184);
	GroupGen->Name = L"GroupGen";
	GroupGen->Size = System::Drawing::Size(488, 263);
	GroupGen->TabIndex = 4;
	GroupGen->TabStop = false;
	GroupGen->Text = L"General";

	UseRegEx->Location = System::Drawing::Point(324, 91);
	UseRegEx->Name = L"UseRegEx";
	UseRegEx->Size = System::Drawing::Size(144, 17);
	UseRegEx->TabIndex = 3;
	UseRegEx->Text = L"Use Regular Expressions";
	UseRegEx->UseVisualStyleBackColor = true;
	UseRegEx->AutoSize = true;
	UseRegEx->Enabled = false;

	ColorEditorBox->AutoSize = true;
	ColorEditorBox->Location = System::Drawing::Point(324, 68);
	ColorEditorBox->Name = L"ColorEditorBox";
	ColorEditorBox->Size = System::Drawing::Size(122, 17);
	ColorEditorBox->TabIndex = 2;
	ColorEditorBox->Text = L"Paint Script Window";
	ColorEditorBox->UseVisualStyleBackColor = true;
	ColorEditorBox->AutoSize = true;

	SaveLastKnownPos->Location = System::Drawing::Point(14, 91);
	SaveLastKnownPos->Name = L"SaveLastKnownPos";
	SaveLastKnownPos->Size = System::Drawing::Size(167, 17);
	SaveLastKnownPos->TabIndex = 7;
	SaveLastKnownPos->Text = L"Save caret position with script";
	SaveLastKnownPos->UseVisualStyleBackColor = true;
	SaveLastKnownPos->AutoSize = true;

	BMCButton->Location = System::Drawing::Point(273, 19);
	BMCButton->Name = L"BMCButton";
	BMCButton->Size = System::Drawing::Size(103, 29);
	BMCButton->TabIndex = 6;
	BMCButton->Text = L"Bookmark Color";
	BMCButton->UseVisualStyleBackColor = true;
	BMCButton->Click += gcnew System::EventHandler(this, &OptionsDialog::BMCButton_Click);
	
	FCButton->Location = System::Drawing::Point(95, 19);
	FCButton->Name = L"FCButton";
	FCButton->Size = System::Drawing::Size(83, 29);
	FCButton->TabIndex = 1;
	FCButton->Text = L"Fore Color";
	FCButton->UseVisualStyleBackColor = true;
	FCButton->Click += gcnew System::EventHandler(this, &OptionsDialog::FCButton_Click);

	FontButton->Location = System::Drawing::Point(6, 19);
	FontButton->Name = L"FontButton";
	FontButton->Size = System::Drawing::Size(83, 29);
	FontButton->TabIndex = 0;
	FontButton->Text = L"Font";
	FontButton->UseVisualStyleBackColor = true;
	FontButton->Click += gcnew System::EventHandler(this, &OptionsDialog::FontButton_Click);

	FontSelection->AllowScriptChange = false;
	FontSelection->AllowVerticalFonts = false;
	FontSelection->ShowEffects = false;
	FCDialog->AnyColor = true;
	BCDialog->AnyColor = true;
	HCDialog->AnyColor = true;
	BMCDialog->AnyColor = true;

	HCButton->Location = System::Drawing::Point(379, 19);
	HCButton->Name = L"HCButton";
	HCButton->Size = System::Drawing::Size(103, 29);
	HCButton->TabIndex = 4;
	HCButton->Text = L"Highlight Color";
	HCButton->UseVisualStyleBackColor = true;
	HCButton->Click += gcnew System::EventHandler(this, &OptionsDialog::HCButton_Click);

	BCButton->Location = System::Drawing::Point(184, 19);
	BCButton->Name = L"BCButton";
	BCButton->Size = System::Drawing::Size(83, 29);
	BCButton->TabIndex = 5;
	BCButton->Text = L"Back Color";
	BCButton->UseVisualStyleBackColor = true;
	BCButton->Click += gcnew System::EventHandler(this, &OptionsDialog::BCButton_Click);

	TabStopSize->AutoSize = true;
	TabStopSize->Location = System::Drawing::Point(11, 202);
	TabStopSize->Size = System::Drawing::Size(101, 13);
	TabStopSize->TabIndex = 1;
	TabStopSize->Text = L"Tab Size ( in pixels )";

	TabSize->Location = System::Drawing::Point(14, 225);
	TabSize->Size = System::Drawing::Size(98, 20);
	TabSize->Minimum = 0;
	TabSize->Maximum = 100;

	RecompileVarIdx->AutoSize = true;
	RecompileVarIdx->Location = System::Drawing::Point(15, 114);
	RecompileVarIdx->Name = L"RecompileVarIdx";
	RecompileVarIdx->Size = System::Drawing::Size(272, 17);
	RecompileVarIdx->TabIndex = 9;
	RecompileVarIdx->Text = L"Recompile dependencies post variable index update";
	RecompileVarIdx->UseVisualStyleBackColor = true;

	UseCSParent->AutoSize = true;
	UseCSParent->Location = System::Drawing::Point(324, 114);
	UseCSParent->Name = L"UseCSParent";
	UseCSParent->Size = System::Drawing::Size(109, 17);
	UseCSParent->TabIndex = 10;
	UseCSParent->Text = L"Use CS as Parent";
	UseCSParent->UseVisualStyleBackColor = true;

	DestroyOnLastTabClose->AutoSize = true;
	DestroyOnLastTabClose->Location = System::Drawing::Point(15, 137);
	DestroyOnLastTabClose->Name = L"DestroyOnLastTabClose";
	DestroyOnLastTabClose->Size = System::Drawing::Size(224, 17);
	DestroyOnLastTabClose->TabIndex = 11;
	DestroyOnLastTabClose->Text = L"Close script window on closing the last tab";
	DestroyOnLastTabClose->UseVisualStyleBackColor = true;

	OptionsBox = gcnew Form();
	OptionsBox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	OptionsBox->AutoScaleMode = AutoScaleMode::Font;
	OptionsBox->ClientSize = System::Drawing::Size(514, 465);
	OptionsBox->Controls->Add(GroupGen);
	OptionsBox->Controls->Add(GroupIS);
	OptionsBox->Controls->Add(GroupPreP);
	OptionsBox->FormBorderStyle = FormBorderStyle::FixedDialog;
	OptionsBox->MaximizeBox = false;
	OptionsBox->MinimizeBox = false;
	OptionsBox->StartPosition = FormStartPosition::CenterScreen;
	OptionsBox->Name = L"OptionsDialog::GetSingleton()";
	OptionsBox->Text = L"Options";
	OptionsBox->Closing += gcnew CancelEventHandler(this, &OptionsDialog::OptionsBox_Cancel);

	INIMap = gcnew Dictionary<INISetting^, BoundControl^>();

	OptionsBox->Hide();	
	PopulateINIMap();
	LoadINI();
}

void OptionsDialog::FontButton_Click(Object^ Sender, EventArgs^ E)
{
	FontSelection->ShowDialog();
}

void OptionsDialog::FCButton_Click(Object^ Sender, EventArgs^ E)
{
	FCDialog->ShowDialog();
}

void OptionsDialog::BCButton_Click(Object^ Sender, EventArgs^ E)
{
	BCDialog->ShowDialog();
}


void OptionsDialog::HCButton_Click(Object^ Sender, EventArgs^ E)
{
	HCDialog->ShowDialog();
}

void OptionsDialog::BMCButton_Click(Object^ Sender, EventArgs^ E)
{
	BMCDialog->ShowDialog();
}

void OptionsDialog::OptionsBox_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	SaveINI();
}
