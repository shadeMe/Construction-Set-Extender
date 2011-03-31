#include "OptionsDialog.h"
#include "[Common]\NativeWrapper.h"
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
			case ValueType::e_Font_Style:
				Result = ((int)Control->Font->Style).ToString();
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
			case ValueType::e_Font_Style:
				Control->Font = gcnew Font(Control->Font->FontFamily->Name, Control->Font->Size, (FontStyle)(int)Numeric);
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
	// Appearance
	INIMap->Add(gcnew INISetting("Font", "ScriptEditor::Appearance", "Lucida Console"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_FontFamily_Name));
	INIMap->Add(gcnew INISetting("FontSize", "ScriptEditor::Appearance", "10"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_Size));
	INIMap->Add(gcnew INISetting("FontStyle", "ScriptEditor::Appearance", "0"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_Style));

	INIMap->Add(gcnew INISetting("ForeColorR", "ScriptEditor::Appearance", "0"), gcnew BoundControl(FCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting("ForeColorG", "ScriptEditor::Appearance", "0"), gcnew BoundControl(FCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting("ForeColorB", "ScriptEditor::Appearance", "0"), gcnew BoundControl(FCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	INIMap->Add(gcnew INISetting("BackColorR", "ScriptEditor::Appearance", "255"), gcnew BoundControl(BCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting("BackColorG", "ScriptEditor::Appearance", "255"), gcnew BoundControl(BCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting("BackColorB", "ScriptEditor::Appearance", "255"), gcnew BoundControl(BCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	INIMap->Add(gcnew INISetting("HiliteColorR", "ScriptEditor::Appearance", "158"), gcnew BoundControl(HCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting("HiliteColorG", "ScriptEditor::Appearance", "195"), gcnew BoundControl(HCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting("HiliteColorB", "ScriptEditor::Appearance", "200"), gcnew BoundControl(HCDialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));
	
	INIMap->Add(gcnew INISetting("TabSize", "ScriptEditor::Appearance", "0"), gcnew BoundControl(TabSize, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	INIMap->Add(gcnew INISetting("LinesToScroll", "ScriptEditor::Appearance", "6"), gcnew BoundControl(LinesToScroll, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));

	// General
	INIMap->Add(gcnew INISetting("SuppressRefCountForQuestScripts", "ScriptEditor::General", "1"), gcnew BoundControl(SuppressRefCountForQuestScripts, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("AutoIndent", "ScriptEditor::General", "1"), gcnew BoundControl(AutoIndent, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("SaveLastKnownPos", "ScriptEditor::General", "1"), gcnew BoundControl(SaveLastKnownPos, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("RecompileVarIdx", "ScriptEditor::General", "1"), gcnew BoundControl(RecompileVarIdx, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("UseCSParent", "ScriptEditor::General", "0"), gcnew BoundControl(UseCSParent, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("DestroyOnLastTabClose", "ScriptEditor::General", "1"), gcnew BoundControl(DestroyOnLastTabClose, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("LoadScriptUpdateExistingScripts", "ScriptEditor::General", "0"), gcnew BoundControl(LoadScriptUpdateExistingScripts, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

	// IntelliSense
	INIMap->Add(gcnew INISetting("ThresholdLength", "ScriptEditor::IntelliSense", "4"), gcnew BoundControl(ThresholdLength, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	INIMap->Add(gcnew INISetting("DatabaseUpdateInterval", "ScriptEditor::IntelliSense", "10"), gcnew BoundControl(DatabaseUpdateInterval, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	INIMap->Add(gcnew INISetting("UseQuickView", "ScriptEditor::IntelliSense", "1"), gcnew BoundControl(UseQuickView, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

	// Preprocessor
	INIMap->Add(gcnew INISetting("AllowRedefinitions", "ScriptEditor::Preprocessor", "0"), gcnew BoundControl(AllowRedefinitions, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	
	// Sanitize
	INIMap->Add(gcnew INISetting("AnnealCasing", "ScriptEditor::Sanitize", "1"), gcnew BoundControl(AnnealCasing, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	INIMap->Add(gcnew INISetting("IndentLines", "ScriptEditor::Sanitize", "1"), gcnew BoundControl(IndentLines, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

//	DebugPrint("Populated INI Map");
}

void OptionsDialog::LoadINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in INIMap) {
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		Control->SetValue(INIWrapper::GetINIValue(INI->Section, INI->Key, INI->DefaultValue));
	}
//	DebugPrint("Loaded settings from INI");
}

void OptionsDialog::SaveINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in INIMap) {
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		INIWrapper::SetINIValue(INI->Section, INI->Key, Control->GetValue());
	}
//	DebugPrint("Saved settings to INI");
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
		int Result = 1;
		try {
			Result = float::Parse(Value);
		} catch (Exception^ E)
		{
			DebugPrint("Couldn't fetch INI setting '" + Key + "' value.\n\tException: " + E->Message);
		}
		return Result;
	}
	else
	{
		DebugPrint("Couldn't fetch INI setting '" + Key + "' value.\n\tException: Key doesn't exist.");
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
	CmDlgHighlightColor = (gcnew Button());
	CmDlgBackColor = (gcnew Button());
	CmDlgForeColor = (gcnew Button());
	CmDlgFont = (gcnew Button());
	FontSelection = gcnew FontDialog();
	FCDialog = gcnew ColorDialog();
	BCDialog = gcnew ColorDialog();
	HCDialog = gcnew ColorDialog();
	
	CmDlgForeColor->Location = System::Drawing::Point(95, 19);
	CmDlgForeColor->Name = L"FCButton";
	CmDlgForeColor->Size = System::Drawing::Size(83, 29);
	CmDlgForeColor->TabIndex = 1;
	CmDlgForeColor->Text = L"Fore Color";
	CmDlgForeColor->UseVisualStyleBackColor = true;
	CmDlgForeColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgForeColor_Click);

	CmDlgFont->Location = System::Drawing::Point(6, 19);
	CmDlgFont->Name = L"FontButton";
	CmDlgFont->Size = System::Drawing::Size(83, 29);
	CmDlgFont->TabIndex = 0;
	CmDlgFont->Text = L"Font";
	CmDlgFont->UseVisualStyleBackColor = true;
	CmDlgFont->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgFont_Click);

	FontSelection->AllowScriptChange = false;
	FontSelection->AllowVerticalFonts = false;
	FontSelection->ShowEffects = false;
	FCDialog->AnyColor = true;
	BCDialog->AnyColor = true;
	HCDialog->AnyColor = true;

	CmDlgHighlightColor->Location = System::Drawing::Point(379, 19);
	CmDlgHighlightColor->Name = L"HCButton";
	CmDlgHighlightColor->Size = System::Drawing::Size(103, 29);
	CmDlgHighlightColor->TabIndex = 4;
	CmDlgHighlightColor->Text = L"Highlight Color";
	CmDlgHighlightColor->UseVisualStyleBackColor = true;
	CmDlgHighlightColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgHighlightColor_Click);

	CmDlgBackColor->Location = System::Drawing::Point(184, 19);
	CmDlgBackColor->Name = L"BCButton";
	CmDlgBackColor->Size = System::Drawing::Size(83, 29);
	CmDlgBackColor->TabIndex = 5;
	CmDlgBackColor->Text = L"Back Color";
	CmDlgBackColor->UseVisualStyleBackColor = true;
	CmDlgBackColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgBackColor_Click);


	AllowRedefinitions = (gcnew CheckBox());
	LabelISThreshold = (gcnew Label());
	ThresholdLength = (gcnew NumericUpDown());
	DestroyOnLastTabClose = (gcnew CheckBox());
	UseCSParent = (gcnew CheckBox());
	RecompileVarIdx = (gcnew CheckBox());
	LabelTabSize = (gcnew Label());
	TabSize = (gcnew NumericUpDown());
	SaveLastKnownPos = (gcnew CheckBox());
	AutoIndent = (gcnew CheckBox());
	TabContainer = (gcnew TabControl());
	TabGeneral = (gcnew TabPage());
	TabIntelliSense = (gcnew TabPage());
	TabPreprocessor = (gcnew TabPage());
	TabAppearance = (gcnew TabPage());
	SuppressRefCountForQuestScripts = (gcnew CheckBox());
	LoadScriptUpdateExistingScripts = (gcnew CheckBox());
	LabelISDBUpdatePeriod = (gcnew Label());
	DatabaseUpdateInterval = (gcnew NumericUpDown());
	LabelLinesToScroll = (gcnew Label());
	LinesToScroll = (gcnew NumericUpDown());
	UseQuickView = (gcnew CheckBox());
	TabSanitize = (gcnew TabPage());
	AnnealCasing = (gcnew CheckBox());
	IndentLines = (gcnew CheckBox());
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(ThresholdLength))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TabSize))->BeginInit();
	TabContainer->SuspendLayout();
	TabGeneral->SuspendLayout();
	TabIntelliSense->SuspendLayout();
	TabPreprocessor->SuspendLayout();
	TabAppearance->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(DatabaseUpdateInterval))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(LinesToScroll))->BeginInit();
	TabSanitize->SuspendLayout();
	// 
	// AllowRedefinitions
	// 
	AllowRedefinitions->Location = System::Drawing::Point(147, 120);
	AllowRedefinitions->Name = L"AllowRedefinitions";
	AllowRedefinitions->Size = System::Drawing::Size(168, 51);
	AllowRedefinitions->TabIndex = 9;
	AllowRedefinitions->Text = L"Allow Macro Redefinitions";
	AllowRedefinitions->UseVisualStyleBackColor = true;
	// 
	// LabelISThreshold
	// 
	LabelISThreshold->AutoSize = true;
	LabelISThreshold->Location = System::Drawing::Point(30, 69);
	LabelISThreshold->Name = L"LabelISThreshold";
	LabelISThreshold->Size = System::Drawing::Size(148, 13);
	LabelISThreshold->TabIndex = 1;
	LabelISThreshold->Text = L"IntelliSense Pop-up Threshold";
	// 
	// ThresholdLength
	// 
	ThresholdLength->Location = System::Drawing::Point(30, 104);
	ThresholdLength->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {10, 0, 0, 0});
	ThresholdLength->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
	ThresholdLength->Name = L"ThresholdLength";
	ThresholdLength->Size = System::Drawing::Size(148, 20);
	ThresholdLength->TabIndex = 0;
	ThresholdLength->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
	// 
	// DestroyOnLastTabClose
	// 
	DestroyOnLastTabClose->AutoSize = true;
	DestroyOnLastTabClose->Location = System::Drawing::Point(22, 92);
	DestroyOnLastTabClose->Name = L"DestroyOnLastTabClose";
	DestroyOnLastTabClose->Size = System::Drawing::Size(152, 17);
	DestroyOnLastTabClose->TabIndex = 11;
	DestroyOnLastTabClose->Text = L"Close Editor With Last Tab";
	DestroyOnLastTabClose->UseVisualStyleBackColor = true;
	// 
	// UseCSParent
	// 
	UseCSParent->AutoSize = true;
	UseCSParent->Location = System::Drawing::Point(22, 46);
	UseCSParent->Name = L"UseCSParent";
	UseCSParent->Size = System::Drawing::Size(166, 17);
	UseCSParent->TabIndex = 10;
	UseCSParent->Text = L"Show Editor As Child Window";
	UseCSParent->UseVisualStyleBackColor = true;
	// 
	// RecompileVarIdx
	// 
	RecompileVarIdx->AutoSize = true;
	RecompileVarIdx->Location = System::Drawing::Point(22, 250);
	RecompileVarIdx->Name = L"RecompileVarIdx";
	RecompileVarIdx->Size = System::Drawing::Size(303, 17);
	RecompileVarIdx->TabIndex = 9;
	RecompileVarIdx->Text = L"Recompile Dependencies After Variable Index Modification";
	RecompileVarIdx->UseVisualStyleBackColor = true;
	// 
	// LabelTabSize
	// 
	LabelTabSize->AutoSize = true;
	LabelTabSize->Location = System::Drawing::Point(160, 55);
	LabelTabSize->Name = L"LabelTabSize";
	LabelTabSize->Size = System::Drawing::Size(103, 18);
	LabelTabSize->TabIndex = 3;
	LabelTabSize->Text = L"Tab Size\n( In Pixels )";
	// 
	// TabSize
	// 
	TabSize->Location = System::Drawing::Point(269, 53);
	TabSize->Name = L"TabSize";
	TabSize->Size = System::Drawing::Size(103, 20);
	TabSize->TabIndex = 2;
	// 
	// SaveLastKnownPos
	// 
	SaveLastKnownPos->AutoSize = true;
	SaveLastKnownPos->Location = System::Drawing::Point(22, 69);
	SaveLastKnownPos->Name = L"SaveLastKnownPos";
	SaveLastKnownPos->Size = System::Drawing::Size(174, 17);
	SaveLastKnownPos->TabIndex = 8;
	SaveLastKnownPos->Text = L"Save Caret Position With Script";
	SaveLastKnownPos->UseVisualStyleBackColor = true;
	// 
	// AutoIndent
	// 
	AutoIndent->AutoSize = true;
	AutoIndent->Location = System::Drawing::Point(22, 23);
	AutoIndent->Name = L"AutoIndent";
	AutoIndent->Size = System::Drawing::Size(139, 17);
	AutoIndent->TabIndex = 5;
	AutoIndent->Text = L"Auto-Indent Script Lines";
	AutoIndent->UseVisualStyleBackColor = true;
	// 
	// CmDlgHighlightColor
	// 
	CmDlgHighlightColor->Location = System::Drawing::Point(21, 153);
	CmDlgHighlightColor->Name = L"CmDlgHighlightColor";
	CmDlgHighlightColor->Size = System::Drawing::Size(111, 34);
	CmDlgHighlightColor->TabIndex = 3;
	CmDlgHighlightColor->Text = L"Line Highlight Color";
	CmDlgHighlightColor->UseVisualStyleBackColor = true;
	// 
	// CmDlgBackColor
	// 
	CmDlgBackColor->Location = System::Drawing::Point(21, 103);
	CmDlgBackColor->Name = L"CmDlgBackColor";
	CmDlgBackColor->Size = System::Drawing::Size(111, 34);
	CmDlgBackColor->TabIndex = 2;
	CmDlgBackColor->Text = L"Background Color";
	CmDlgBackColor->UseVisualStyleBackColor = true;
	// 
	// CmDlgForeColor
	// 
	CmDlgForeColor->Location = System::Drawing::Point(21, 203);
	CmDlgForeColor->Name = L"CmDlgForeColor";
	CmDlgForeColor->Size = System::Drawing::Size(111, 34);
	CmDlgForeColor->TabIndex = 1;
	CmDlgForeColor->Text = L"Foreground Color";
	CmDlgForeColor->UseVisualStyleBackColor = true;
	// 
	// CmDlgFont
	// 
	CmDlgFont->Location = System::Drawing::Point(21, 53);
	CmDlgFont->Name = L"CmDlgFont";
	CmDlgFont->Size = System::Drawing::Size(111, 34);
	CmDlgFont->TabIndex = 0;
	CmDlgFont->Text = L"Editor Font";
	CmDlgFont->UseVisualStyleBackColor = true;
	// 
	// TabContainer
	// 
	TabContainer->Controls->Add(TabGeneral);
	TabContainer->Controls->Add(TabIntelliSense);
	TabContainer->Controls->Add(TabPreprocessor);
	TabContainer->Controls->Add(TabAppearance);
	TabContainer->Controls->Add(TabSanitize);
	TabContainer->HotTrack = true;
	TabContainer->Location = System::Drawing::Point(12, 12);
	TabContainer->Multiline = true;
	TabContainer->Name = L"TabContainer";
	TabContainer->SelectedIndex = 0;
	TabContainer->Size = System::Drawing::Size(431, 316);
	TabContainer->TabIndex = 0;
	// 
	// TabGeneral
	// 
	TabGeneral->Controls->Add(LoadScriptUpdateExistingScripts);
	TabGeneral->Controls->Add(DestroyOnLastTabClose);
	TabGeneral->Controls->Add(SuppressRefCountForQuestScripts);
	TabGeneral->Controls->Add(SaveLastKnownPos);
	TabGeneral->Controls->Add(RecompileVarIdx);
	TabGeneral->Controls->Add(AutoIndent);
	TabGeneral->Controls->Add(UseCSParent);
	TabGeneral->Location = System::Drawing::Point(4, 22);
	TabGeneral->Name = L"TabGeneral";
	TabGeneral->Padding = Padding(3);
	TabGeneral->Size = System::Drawing::Size(423, 290);
	TabGeneral->TabIndex = 0;
	TabGeneral->Text = L"General";
	TabGeneral->UseVisualStyleBackColor = true;
	// 
	// TabIntelliSense
	// 
	TabIntelliSense->Controls->Add(UseQuickView);
	TabIntelliSense->Controls->Add(LabelISDBUpdatePeriod);
	TabIntelliSense->Controls->Add(DatabaseUpdateInterval);
	TabIntelliSense->Controls->Add(LabelISThreshold);
	TabIntelliSense->Controls->Add(ThresholdLength);
	TabIntelliSense->Location = System::Drawing::Point(4, 22);
	TabIntelliSense->Name = L"TabIntelliSense";
	TabIntelliSense->Padding = Padding(3);
	TabIntelliSense->Size = System::Drawing::Size(423, 290);
	TabIntelliSense->TabIndex = 1;
	TabIntelliSense->Text = L"IntelliSense";
	TabIntelliSense->UseVisualStyleBackColor = true;
	// 
	// TabPreprocessor
	// 
	TabPreprocessor->Controls->Add(AllowRedefinitions);
	TabPreprocessor->Location = System::Drawing::Point(4, 22);
	TabPreprocessor->Name = L"TabPreprocessor";
	TabPreprocessor->Padding = Padding(3);
	TabPreprocessor->Size = System::Drawing::Size(423, 290);
	TabPreprocessor->TabIndex = 2;
	TabPreprocessor->Text = L"Preprocessor";
	TabPreprocessor->UseVisualStyleBackColor = true;
	// 
	// TabAppearance
	// 
	TabAppearance->Controls->Add(LabelLinesToScroll);
	TabAppearance->Controls->Add(LinesToScroll);
	TabAppearance->Controls->Add(CmDlgFont);
	TabAppearance->Controls->Add(CmDlgBackColor);
	TabAppearance->Controls->Add(CmDlgHighlightColor);
	TabAppearance->Controls->Add(CmDlgForeColor);
	TabAppearance->Controls->Add(LabelTabSize);
	TabAppearance->Controls->Add(TabSize);
	TabAppearance->Location = System::Drawing::Point(4, 22);
	TabAppearance->Name = L"TabAppearance";
	TabAppearance->Padding = Padding(3);
	TabAppearance->Size = System::Drawing::Size(423, 290);
	TabAppearance->TabIndex = 3;
	TabAppearance->Text = L"Appearance";
	TabAppearance->UseVisualStyleBackColor = true;
	// 
	// SuppressRefCountForQuestScripts
	// 
	SuppressRefCountForQuestScripts->AutoSize = true;
	SuppressRefCountForQuestScripts->Location = System::Drawing::Point(22, 227);
	SuppressRefCountForQuestScripts->Name = L"SuppressRefCountForQuestScripts";
	SuppressRefCountForQuestScripts->Size = System::Drawing::Size(293, 17);
	SuppressRefCountForQuestScripts->TabIndex = 12;
	SuppressRefCountForQuestScripts->Text = L"Suppress Variable Reference Counting For Quest Scripts";
	SuppressRefCountForQuestScripts->UseVisualStyleBackColor = true;
	// 
	// LoadScriptUpdateExistingScripts
	// 
	LoadScriptUpdateExistingScripts->AutoSize = true;
	LoadScriptUpdateExistingScripts->Location = System::Drawing::Point(22, 115);
	LoadScriptUpdateExistingScripts->Name = L"LoadScriptUpdateExistingScripts";
	LoadScriptUpdateExistingScripts->Size = System::Drawing::Size(236, 17);
	LoadScriptUpdateExistingScripts->TabIndex = 13;
	LoadScriptUpdateExistingScripts->Text = L"\'Load Multiple Scripts\' Tool Updates Existing Scripts";
	LoadScriptUpdateExistingScripts->UseVisualStyleBackColor = true;
	// 
	// LabelISDBUpdatePeriod
	// 
	LabelISDBUpdatePeriod->AutoSize = true;
	LabelISDBUpdatePeriod->Location = System::Drawing::Point(30, 152);
	LabelISDBUpdatePeriod->Name = L"LabelISDBUpdatePeriod";
	LabelISDBUpdatePeriod->Size = System::Drawing::Size(181, 26);
	LabelISDBUpdatePeriod->TabIndex = 3;
	LabelISDBUpdatePeriod->Text = L"IntelliSense Database Update Period\r\n(In Earth Minutes)";
	// 
	// DatabaseUpdateInterval
	// 
	DatabaseUpdateInterval->Location = System::Drawing::Point(30, 201);
	DatabaseUpdateInterval->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {15, 0, 0, 0});
	DatabaseUpdateInterval->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
	DatabaseUpdateInterval->Name = L"DatabaseUpdateInterval";
	DatabaseUpdateInterval->Size = System::Drawing::Size(148, 20);
	DatabaseUpdateInterval->TabIndex = 2;
	DatabaseUpdateInterval->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
	// 
	// LabelLinesToScroll
	// 
	LabelLinesToScroll->AutoSize = true;
	LabelLinesToScroll->Location = System::Drawing::Point(160, 94);
	LabelLinesToScroll->Name = L"LabelLinesToScroll";
	LabelLinesToScroll->Size = System::Drawing::Size(98, 26);
	LabelLinesToScroll->TabIndex = 5;
	LabelLinesToScroll->Text = L"Lines To Scroll\r\nWith Mouse Wheel";
	// 
	// LinesToScroll
	// 
	LinesToScroll->Location = System::Drawing::Point(269, 100);
	LinesToScroll->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {15, 0, 0, 0});
	LinesToScroll->Minimum = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
	LinesToScroll->Name = L"LinesToScroll";
	LinesToScroll->Size = System::Drawing::Size(103, 20);
	LinesToScroll->TabIndex = 4;
	LinesToScroll->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {5, 0, 0, 0});
	// 
	// UseQuickView
	// 
	UseQuickView->AutoSize = true;
	UseQuickView->Location = System::Drawing::Point(231, 69);
	UseQuickView->Name = L"UseQuickView";
	UseQuickView->Size = System::Drawing::Size(89, 17);
	UseQuickView->TabIndex = 6;
	UseQuickView->Text = L"Quick-View™";
	UseQuickView->UseVisualStyleBackColor = true;
	// 
	// TabSanitize
	// 
	TabSanitize->Controls->Add(IndentLines);
	TabSanitize->Controls->Add(AnnealCasing);
	TabSanitize->Location = System::Drawing::Point(4, 22);
	TabSanitize->Name = L"TabSanitize";
	TabSanitize->Padding = Padding(3);
	TabSanitize->Size = System::Drawing::Size(423, 290);
	TabSanitize->TabIndex = 4;
	TabSanitize->Text = L"Sanitize";
	TabSanitize->UseVisualStyleBackColor = true;
	// 
	// AnnealCasing
	// 
	AnnealCasing->AutoSize = true;
	AnnealCasing->Location = System::Drawing::Point(128, 109);
	AnnealCasing->Name = L"AnnealCasing";
	AnnealCasing->Size = System::Drawing::Size(205, 17);
	AnnealCasing->TabIndex = 11;
	AnnealCasing->Text = L"Anneal Script Command Name Casing";
	AnnealCasing->UseVisualStyleBackColor = true;
	// 
	// IndentLines
	// 
	IndentLines->AutoSize = true;
	IndentLines->Location = System::Drawing::Point(128, 164);
	IndentLines->Name = L"IndentLines";
	IndentLines->Size = System::Drawing::Size(114, 17);
	IndentLines->TabIndex = 12;
	IndentLines->Text = L"Indent Script Lines";
	IndentLines->UseVisualStyleBackColor = true;
	// 
	// OptionsDialog
	// 
	OptionsBox = gcnew Form();
	OptionsBox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	OptionsBox->AutoScaleMode = AutoScaleMode::Font;
	OptionsBox->ClientSize = System::Drawing::Size(456, 337);
	OptionsBox->Controls->Add(TabContainer);
	OptionsBox->FormBorderStyle = FormBorderStyle::FixedDialog;
	OptionsBox->MaximizeBox = false;
	OptionsBox->MinimizeBox = false;
	OptionsBox->StartPosition = FormStartPosition::CenterScreen;
	OptionsBox->Text = L"Preferences";
	OptionsBox->Closing += gcnew CancelEventHandler(this, &OptionsDialog::OptionsBox_Cancel);

	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(ThresholdLength))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TabSize))->EndInit();
	TabContainer->ResumeLayout(false);
	TabGeneral->ResumeLayout(false);
	TabGeneral->PerformLayout();
	TabIntelliSense->ResumeLayout(false);
	TabIntelliSense->PerformLayout();
	TabPreprocessor->ResumeLayout(false);
	TabAppearance->ResumeLayout(false);
	TabAppearance->PerformLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(DatabaseUpdateInterval))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(LinesToScroll))->EndInit();
	TabSanitize->ResumeLayout(false);
	TabSanitize->PerformLayout();

	INIMap = gcnew Dictionary<INISetting^, BoundControl^>();

	OptionsBox->Hide();
	PopulateINIMap();
	LoadINI();
}


void OptionsDialog::CmDlgFont_Click(Object^ Sender, EventArgs^ E)
{
	FontSelection->ShowDialog();
}

void OptionsDialog::CmDlgForeColor_Click(Object^ Sender, EventArgs^ E)
{
	FCDialog->ShowDialog();
}

void OptionsDialog::CmDlgBackColor_Click(Object^ Sender, EventArgs^ E)
{
	BCDialog->ShowDialog();
}


void OptionsDialog::CmDlgHighlightColor_Click(Object^ Sender, EventArgs^ E)
{
	HCDialog->ShowDialog();
}

void OptionsDialog::OptionsBox_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	SaveINI();
}


Color OptionsDialog::GetColor(OptionsDialog::ColorType Type)
{
	switch (Type)
	{
	case OptionsDialog::ColorType::e_Foreground:
		return FCDialog->Color;
	case OptionsDialog::ColorType::e_Background:
		return BCDialog->Color;
	case OptionsDialog::ColorType::e_Highlight:
		return HCDialog->Color;
	default:
		return Color::Black;
	}
}