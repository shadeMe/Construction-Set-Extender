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
			switch (Property)
			{
			case ValueType::e_Color_R:
				Control->Color = Color::FromArgb(255, Numeric, Control->Color.G, Control->Color.B);
				break;
			case ValueType::e_Color_G:
				Control->Color = Color::FromArgb(255, Control->Color.R, Numeric, Control->Color.B);
				break;
			case ValueType::e_Color_B:
				Control->Color = Color::FromArgb(255, Control->Color.R, Control->Color.G, Numeric);
				break;
			}

			Button^ Parent = dynamic_cast<Button^>(Control->Tag);
			if (Parent != nullptr)
				Parent->BackColor = Control->Color;
			break;
		}
	}	
}

void OptionsDialog::RegisterColorSetting(String^ Key, Color Default, Control^ Parent)
{
	ColorDialog^ Dialog = gcnew ColorDialog();
	Dialog->AnyColor = true;
	Dialog->Color = Default;
	Dialog->Tag = Parent;

	Parent->Tag = Dialog;

	INIMap->Add(gcnew INISetting(Key + "R", "ScriptEditor::Appearance", Default.R.ToString()), gcnew BoundControl(Dialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	INIMap->Add(gcnew INISetting(Key + "G", "ScriptEditor::Appearance", Default.R.ToString()), gcnew BoundControl(Dialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	INIMap->Add(gcnew INISetting(Key + "B", "ScriptEditor::Appearance", Default.R.ToString()), gcnew BoundControl(Dialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	ColorDictionary->Add(Key, Dialog);
}

void OptionsDialog::PopulateINIMap()
{
	// Appearance
	INIMap->Add(gcnew INISetting("Font", "ScriptEditor::Appearance", "Lucida Console"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_FontFamily_Name));
	INIMap->Add(gcnew INISetting("FontSize", "ScriptEditor::Appearance", "10"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_Size));
	INIMap->Add(gcnew INISetting("FontStyle", "ScriptEditor::Appearance", "0"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_Style));

	RegisterColorSetting("SyntaxCommentsColor", Color::DarkBlue, CmDlgSyntaxCommentsColor);
	RegisterColorSetting("SyntaxDigitsColor", Color::DarkGoldenrod, CmDlgSyntaxDigitsColor);
	RegisterColorSetting("SyntaxPreprocessorColor", Color::DarkBlue, CmDlgSyntaxPreprocessorColor);
	RegisterColorSetting("SyntaxScriptBlocksColor", Color::Red, CmDlgSyntaxScriptBlocksColor);
	RegisterColorSetting("SyntaxDelimitersColor", Color::DarkMagenta, CmDlgSyntaxDelimitersColor);
	RegisterColorSetting("SyntaxStringsColor", Color::Firebrick, CmDlgSyntaxStringsColor);
	RegisterColorSetting("SyntaxKeywordsColor", Color::RoyalBlue, CmDlgSyntaxKeywordsColor);

	RegisterColorSetting("CurrentLineHighlightColor", Color::DodgerBlue, CmDlgCurrentLineHighlightColor);
	RegisterColorSetting("CharLimitHighlightColor", Color::IndianRed, CmDlgCharLimitHighlightColor);
	RegisterColorSetting("ErrorHighlightColor", Color::Red, CmDlgErrorHighlightColor);
	RegisterColorSetting("SelectionHighlightColor", Color::Gold, CmDlgSelectionHighlightColor);
	RegisterColorSetting("FindResultsHighlightColor", Color::Gold, CmDlgFindResultsHighlightColor);

	INIMap->Add(gcnew INISetting("TabSize", "ScriptEditor::Appearance", "0"), gcnew BoundControl(TabSize, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	INIMap->Add(gcnew INISetting("WordWrap", "ScriptEditor::Appearance", "0"), gcnew BoundControl(Wordwrap, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

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

}

void OptionsDialog::LoadINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in INIMap)
	{
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		Control->SetValue(INIWrapper::GetINIValue(INI->Section, INI->Key, INI->DefaultValue));
	}
}

void OptionsDialog::SaveINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in INIMap)
	{
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		INIWrapper::SetINIValue(INI->Section, INI->Key, Control->GetValue());
	}
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
		try
		{
			Result = float::Parse(Value);
		} 
		catch (Exception^ E)
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
	if (Singleton == nullptr)
	{
		Singleton = gcnew OptionsDialog();
	}
	return Singleton;
}

OptionsDialog::OptionsDialog()
{
	FontSelection = gcnew FontDialog();
	FontSelection->AllowScriptChange = false;
	FontSelection->AllowVerticalFonts = false;
	FontSelection->ShowEffects = false;

	OptionsBox = gcnew Form();
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
	CmDlgFont = (gcnew Button());
	TabContainer = (gcnew TabControl());
	TabGeneral = (gcnew TabPage());
	LoadScriptUpdateExistingScripts = (gcnew CheckBox());
	SuppressRefCountForQuestScripts = (gcnew CheckBox());
	TabIntelliSense = (gcnew TabPage());
	UseQuickView = (gcnew CheckBox());
	LabelISDBUpdatePeriod = (gcnew Label());
	DatabaseUpdateInterval = (gcnew NumericUpDown());
	TabPreprocessor = (gcnew TabPage());
	TabAppearance = (gcnew TabPage());
	TabSanitize = (gcnew TabPage());
	IndentLines = (gcnew CheckBox());
	AnnealCasing = (gcnew CheckBox());
	GroupBoxSyntaxHighlighting = (gcnew GroupBox());
	LabelKeywords = (gcnew Label());
	LabelDelimiters = (gcnew Label());
	LabelDigits = (gcnew Label());
	LabelComments = (gcnew Label());
	LabelStrings = (gcnew Label());
	LabelScriptBlocks = (gcnew Label());
	LabelPreprocessor = (gcnew Label());
	CmDlgSyntaxCommentsColor = gcnew Button();
	CmDlgSyntaxKeywordsColor = (gcnew Button());
	CmDlgSyntaxStringsColor = (gcnew Button());
	CmDlgSyntaxDelimitersColor = (gcnew Button());
	CmDlgSyntaxScriptBlocksColor = (gcnew Button());
	CmDlgSyntaxPreprocessorColor = (gcnew Button());
	CmDlgSyntaxDigitsColor = (gcnew Button());
	Wordwrap = (gcnew CheckBox());
	CmDlgSelectionHighlightColor = (gcnew Button());
	LabelSelectionHighlight = (gcnew Label());
	CmDlgErrorHighlightColor = (gcnew Button());
	LabelErrorHighlight = (gcnew Label());
	CmDlgCharLimitHighlightColor = (gcnew Button());
	LabelCharLimitHighlight = (gcnew Label());
	CmDlgCurrentLineHighlightColor = (gcnew Button());
	LabelCurrentLineHighlight = (gcnew Label());
	CmDlgFindResultsHighlightColor = (gcnew Button());
	LabelFindResultsHighlight = (gcnew Label());
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(ThresholdLength))->BeginInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TabSize))->BeginInit();
	TabContainer->SuspendLayout();
	TabGeneral->SuspendLayout();
	TabIntelliSense->SuspendLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(DatabaseUpdateInterval))->BeginInit();
	TabPreprocessor->SuspendLayout();
	TabAppearance->SuspendLayout();
	TabSanitize->SuspendLayout();
	GroupBoxSyntaxHighlighting->SuspendLayout();
	OptionsBox->SuspendLayout();
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
	LabelTabSize->Location = System::Drawing::Point(286, 194);
	LabelTabSize->Name = L"LabelTabSize";
	LabelTabSize->Size = System::Drawing::Size(131, 25);
	LabelTabSize->TabIndex = 3;
	LabelTabSize->Text = L"Tab Size (Characters)";
	// 
	// TabSize
	// 
	TabSize->Location = System::Drawing::Point(289, 217);
	TabSize->Name = L"TabSize";
	TabSize->Size = System::Drawing::Size(128, 20);
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
	// CmDlgFont
	// 
	CmDlgFont->Location = System::Drawing::Point(342, 249);
	CmDlgFont->Name = L"CmDlgFont";
	CmDlgFont->Size = System::Drawing::Size(75, 24);
	CmDlgFont->TabIndex = 0;
	CmDlgFont->Text = L"Editor Font";
	CmDlgFont->UseVisualStyleBackColor = true;
	CmDlgFont->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgFont_Click);
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
	// LoadScriptUpdateExistingScripts
	// 
	LoadScriptUpdateExistingScripts->AutoSize = true;
	LoadScriptUpdateExistingScripts->Location = System::Drawing::Point(22, 115);
	LoadScriptUpdateExistingScripts->Name = L"LoadScriptUpdateExistingScripts";
	LoadScriptUpdateExistingScripts->Size = System::Drawing::Size(236, 17);
	LoadScriptUpdateExistingScripts->TabIndex = 13;
	LoadScriptUpdateExistingScripts->Text = L"\'Load Script(s)\' Tool Updates Existing Scripts";
	LoadScriptUpdateExistingScripts->UseVisualStyleBackColor = true;
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
	TabAppearance->Controls->Add(CmDlgFindResultsHighlightColor);
	TabAppearance->Controls->Add(LabelFindResultsHighlight);
	TabAppearance->Controls->Add(CmDlgCurrentLineHighlightColor);
	TabAppearance->Controls->Add(LabelCurrentLineHighlight);
	TabAppearance->Controls->Add(CmDlgCharLimitHighlightColor);
	TabAppearance->Controls->Add(LabelCharLimitHighlight);
	TabAppearance->Controls->Add(CmDlgErrorHighlightColor);
	TabAppearance->Controls->Add(LabelErrorHighlight);
	TabAppearance->Controls->Add(CmDlgSelectionHighlightColor);
	TabAppearance->Controls->Add(Wordwrap);
	TabAppearance->Controls->Add(LabelSelectionHighlight);
	TabAppearance->Controls->Add(GroupBoxSyntaxHighlighting);
	TabAppearance->Controls->Add(CmDlgFont);
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
	// GroupBoxSyntaxHighlighting
	// 
	GroupBoxSyntaxHighlighting->Controls->Add(CmDlgSyntaxCommentsColor);
	GroupBoxSyntaxHighlighting->Controls->Add(CmDlgSyntaxDigitsColor);
	GroupBoxSyntaxHighlighting->Controls->Add(CmDlgSyntaxPreprocessorColor);
	GroupBoxSyntaxHighlighting->Controls->Add(CmDlgSyntaxScriptBlocksColor);
	GroupBoxSyntaxHighlighting->Controls->Add(CmDlgSyntaxDelimitersColor);
	GroupBoxSyntaxHighlighting->Controls->Add(CmDlgSyntaxStringsColor);
	GroupBoxSyntaxHighlighting->Controls->Add(CmDlgSyntaxKeywordsColor);
	GroupBoxSyntaxHighlighting->Controls->Add(LabelPreprocessor);
	GroupBoxSyntaxHighlighting->Controls->Add(LabelScriptBlocks);
	GroupBoxSyntaxHighlighting->Controls->Add(LabelStrings);
	GroupBoxSyntaxHighlighting->Controls->Add(LabelComments);
	GroupBoxSyntaxHighlighting->Controls->Add(LabelDigits);
	GroupBoxSyntaxHighlighting->Controls->Add(LabelDelimiters);
	GroupBoxSyntaxHighlighting->Controls->Add(LabelKeywords);
	GroupBoxSyntaxHighlighting->Location = System::Drawing::Point(6, 3);
	GroupBoxSyntaxHighlighting->Name = L"GroupBoxSyntaxHighlighting";
	GroupBoxSyntaxHighlighting->Size = System::Drawing::Size(203, 281);
	GroupBoxSyntaxHighlighting->TabIndex = 4;
	GroupBoxSyntaxHighlighting->TabStop = false;
	GroupBoxSyntaxHighlighting->Text = L"Syntax Highlighting";
	// 
	// LabelKeywords
	// 
	LabelKeywords->Location = System::Drawing::Point(6, 22);
	LabelKeywords->Name = L"LabelKeywords";
	LabelKeywords->Size = System::Drawing::Size(78, 20);
	LabelKeywords->TabIndex = 5;
	LabelKeywords->Text = L"Keywords";
	// 
	// LabelDelimiters
	// 
	LabelDelimiters->Location = System::Drawing::Point(6, 150);
	LabelDelimiters->Name = L"LabelDelimiters";
	LabelDelimiters->Size = System::Drawing::Size(78, 20);
	LabelDelimiters->TabIndex = 6;
	LabelDelimiters->Text = L"Delimiters";
	// 
	// LabelDigits
	// 
	LabelDigits->Location = System::Drawing::Point(6, 54);
	LabelDigits->Name = L"LabelDigits";
	LabelDigits->Size = System::Drawing::Size(78, 20);
	LabelDigits->TabIndex = 7;
	LabelDigits->Text = L"Digits";
	// 
	// LabelComments
	// 
	LabelComments->Location = System::Drawing::Point(6, 215);
	LabelComments->Name = L"LabelComments";
	LabelComments->Size = System::Drawing::Size(78, 20);
	LabelComments->TabIndex = 8;
	LabelComments->Text = L"Comments";
	// 
	// LabelStrings
	// 
	LabelStrings->Location = System::Drawing::Point(6, 182);
	LabelStrings->Name = L"LabelStrings";
	LabelStrings->Size = System::Drawing::Size(78, 20);
	LabelStrings->TabIndex = 9;
	LabelStrings->Text = L"String Literals";
	// 
	// LabelScriptBlocks
	// 
	LabelScriptBlocks->Location = System::Drawing::Point(6, 118);
	LabelScriptBlocks->Name = L"LabelScriptBlocks";
	LabelScriptBlocks->Size = System::Drawing::Size(78, 20);
	LabelScriptBlocks->TabIndex = 10;
	LabelScriptBlocks->Text = L"Script Blocks";
	// 
	// LabelPreprocessor
	// 
	LabelPreprocessor->Location = System::Drawing::Point(6, 86);
	LabelPreprocessor->Name = L"LabelPreprocessor";
	LabelPreprocessor->Size = System::Drawing::Size(78, 20);
	LabelPreprocessor->TabIndex = 11;
	LabelPreprocessor->Text = L"Preprocessor";
	// 
	// CmDlgSyntaxCommentsColor
	// 
	CmDlgSyntaxCommentsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	CmDlgSyntaxCommentsColor->Location = System::Drawing::Point(117, 211);
	CmDlgSyntaxCommentsColor->Name = L"CmDlgSyntaxCommentsColor";
	CmDlgSyntaxCommentsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxCommentsColor->TabIndex = 18;
	CmDlgSyntaxCommentsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxCommentsColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// CmDlgSyntaxKeywordsColor
	// 
	CmDlgSyntaxKeywordsColor->FlatStyle = FlatStyle::Flat;
	CmDlgSyntaxKeywordsColor->Location = System::Drawing::Point(117, 18);
	CmDlgSyntaxKeywordsColor->Name = L"CmDlgSyntaxKeywordsColor";
	CmDlgSyntaxKeywordsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxKeywordsColor->TabIndex = 12;
	CmDlgSyntaxKeywordsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxKeywordsColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// CmDlgSyntaxStringsColor
	// 
	CmDlgSyntaxStringsColor->FlatStyle = FlatStyle::Flat;
	CmDlgSyntaxStringsColor->Location = System::Drawing::Point(117, 178);
	CmDlgSyntaxStringsColor->Name = L"CmDlgSyntaxStringsColor";
	CmDlgSyntaxStringsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxStringsColor->TabIndex = 13;
	CmDlgSyntaxStringsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxStringsColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// CmDlgSyntaxDelimitersColor
	// 
	CmDlgSyntaxDelimitersColor->FlatStyle = FlatStyle::Flat;
	CmDlgSyntaxDelimitersColor->Location = System::Drawing::Point(117, 146);
	CmDlgSyntaxDelimitersColor->Name = L"CmDlgSyntaxDelimitersColor";
	CmDlgSyntaxDelimitersColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxDelimitersColor->TabIndex = 14;
	CmDlgSyntaxDelimitersColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxDelimitersColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// CmDlgSyntaxScriptBlocksColor
	// 
	CmDlgSyntaxScriptBlocksColor->FlatStyle = FlatStyle::Flat;
	CmDlgSyntaxScriptBlocksColor->Location = System::Drawing::Point(117, 112);
	CmDlgSyntaxScriptBlocksColor->Name = L"CmDlgSyntaxScriptBlocksColor";
	CmDlgSyntaxScriptBlocksColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxScriptBlocksColor->TabIndex = 15;
	CmDlgSyntaxScriptBlocksColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxScriptBlocksColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// CmDlgSyntaxPreprocessorColor
	// 
	CmDlgSyntaxPreprocessorColor->FlatStyle = FlatStyle::Flat;
	CmDlgSyntaxPreprocessorColor->Location = System::Drawing::Point(117, 82);
	CmDlgSyntaxPreprocessorColor->Name = L"CmDlgSyntaxPreprocessorColor";
	CmDlgSyntaxPreprocessorColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxPreprocessorColor->TabIndex = 16;
	CmDlgSyntaxPreprocessorColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxPreprocessorColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// CmDlgSyntaxDigitsColor
	// 
	CmDlgSyntaxDigitsColor->FlatStyle = FlatStyle::Flat;
	CmDlgSyntaxDigitsColor->Location = System::Drawing::Point(117, 50);
	CmDlgSyntaxDigitsColor->Name = L"CmDlgSyntaxDigitsColor";
	CmDlgSyntaxDigitsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxDigitsColor->TabIndex = 17;
	CmDlgSyntaxDigitsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxDigitsColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// Wordwrap
	// 
	Wordwrap->AutoSize = true;
	Wordwrap->Location = System::Drawing::Point(255, 254);
	Wordwrap->Name = L"Wordwrap";
	Wordwrap->Size = System::Drawing::Size(81, 17);
	Wordwrap->TabIndex = 5;
	Wordwrap->Text = L"Word-Wrap";
	Wordwrap->UseVisualStyleBackColor = true;
	// 
	// CmDlgSelectionHighlightColor
	// 
	CmDlgSelectionHighlightColor->FlatStyle = FlatStyle::Flat;
	CmDlgSelectionHighlightColor->Location = System::Drawing::Point(369, 21);
	CmDlgSelectionHighlightColor->Name = L"CmDlgSelectionHighlightColor";
	CmDlgSelectionHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgSelectionHighlightColor->TabIndex = 19;
	CmDlgSelectionHighlightColor->UseVisualStyleBackColor = true;
	CmDlgSelectionHighlightColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// LabelSelectionHighlight
	// 
	LabelSelectionHighlight->Location = System::Drawing::Point(228, 25);
	LabelSelectionHighlight->Name = L"LabelSelectionHighlight";
	LabelSelectionHighlight->Size = System::Drawing::Size(108, 20);
	LabelSelectionHighlight->TabIndex = 18;
	LabelSelectionHighlight->Text = L"Selection Highlight";
	// 
	// CmDlgErrorHighlightColor
	// 
	CmDlgErrorHighlightColor->FlatStyle = FlatStyle::Flat;
	CmDlgErrorHighlightColor->Location = System::Drawing::Point(369, 114);
	CmDlgErrorHighlightColor->Name = L"CmDlgErrorHighlightColor";
	CmDlgErrorHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgErrorHighlightColor->TabIndex = 21;
	CmDlgErrorHighlightColor->UseVisualStyleBackColor = true;
	CmDlgErrorHighlightColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// LabelErrorHighlight
	// 
	LabelErrorHighlight->Location = System::Drawing::Point(228, 122);
	LabelErrorHighlight->Name = L"LabelErrorHighlight";
	LabelErrorHighlight->Size = System::Drawing::Size(108, 20);
	LabelErrorHighlight->TabIndex = 20;
	LabelErrorHighlight->Text = L"Error Highlight";
	// 
	// CmDlgCharLimitColor
	// 
	CmDlgCharLimitHighlightColor->FlatStyle = FlatStyle::Flat;
	CmDlgCharLimitHighlightColor->Location = System::Drawing::Point(369, 83);
	CmDlgCharLimitHighlightColor->Name = L"CmDlgCharLimitColor";
	CmDlgCharLimitHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgCharLimitHighlightColor->TabIndex = 23;
	CmDlgCharLimitHighlightColor->UseVisualStyleBackColor = true;
	CmDlgCharLimitHighlightColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// LabelCharLimitHighlight
	// 
	LabelCharLimitHighlight->Location = System::Drawing::Point(228, 83);
	LabelCharLimitHighlight->Name = L"LabelCharLimitHighlight";
	LabelCharLimitHighlight->Size = System::Drawing::Size(108, 30);
	LabelCharLimitHighlight->TabIndex = 22;
	LabelCharLimitHighlight->Text = L"Character Limit Highlight";
	// 
	// CmDlgCurrentLineHighlightColor
	// 
	CmDlgCurrentLineHighlightColor->FlatStyle = FlatStyle::Flat;
	CmDlgCurrentLineHighlightColor->Location = System::Drawing::Point(369, 52);
	CmDlgCurrentLineHighlightColor->Name = L"CmDlgCurrentLineHighlightColor";
	CmDlgCurrentLineHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgCurrentLineHighlightColor->TabIndex = 25;
	CmDlgCurrentLineHighlightColor->UseVisualStyleBackColor = true;
	CmDlgCurrentLineHighlightColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// LabelCurrentLineHighlight
	// 
	LabelCurrentLineHighlight->Location = System::Drawing::Point(228, 54);
	LabelCurrentLineHighlight->Name = L"LabelCurrentLineHighlight";
	LabelCurrentLineHighlight->Size = System::Drawing::Size(108, 30);
	LabelCurrentLineHighlight->TabIndex = 24;
	LabelCurrentLineHighlight->Text = L"Current Line Highlight";
	// 
	// CmDlgFindResultsHighlightColor
	// 
	CmDlgFindResultsHighlightColor->FlatStyle = FlatStyle::Flat;
	CmDlgFindResultsHighlightColor->Location = System::Drawing::Point(369, 145);
	CmDlgFindResultsHighlightColor->Name = L"CmDlgFindResultsHighlightColor";
	CmDlgFindResultsHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgFindResultsHighlightColor->TabIndex = 27;
	CmDlgFindResultsHighlightColor->UseVisualStyleBackColor = true;
	CmDlgFindResultsHighlightColor->Click += gcnew System::EventHandler(this, &OptionsDialog::CmDlgColor_Click);
	// 
	// LabelFindResultsHighlight
	// 
	LabelFindResultsHighlight->Location = System::Drawing::Point(228, 151);
	LabelFindResultsHighlight->Name = L"LabelFindResultsHighlight";
	LabelFindResultsHighlight->Size = System::Drawing::Size(108, 33);
	LabelFindResultsHighlight->TabIndex = 26;
	LabelFindResultsHighlight->Text = L"Find Results Highlight";
	// 
	// OptionsDialog
	// 
	OptionsBox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	OptionsBox->AutoScaleMode = AutoScaleMode::Font;
	OptionsBox->ClientSize = System::Drawing::Size(456, 337);
	OptionsBox->Controls->Add(TabContainer);
	OptionsBox->FormBorderStyle = FormBorderStyle::FixedDialog;
	OptionsBox->Name = L"OptionsDialog";
	OptionsBox->StartPosition = FormStartPosition::CenterScreen;
	OptionsBox->Text = L"Preferences";
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(ThresholdLength))->EndInit();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(TabSize))->EndInit();
	TabContainer->ResumeLayout(false);
	TabGeneral->ResumeLayout(false);
	TabGeneral->PerformLayout();
	TabIntelliSense->ResumeLayout(false);
	TabIntelliSense->PerformLayout();
	(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(DatabaseUpdateInterval))->EndInit();
	TabPreprocessor->ResumeLayout(false);
	TabAppearance->ResumeLayout(false);
	TabAppearance->PerformLayout();
	TabSanitize->ResumeLayout(false);
	TabSanitize->PerformLayout();
	GroupBoxSyntaxHighlighting->ResumeLayout(false);
	OptionsBox->ResumeLayout(false);
	OptionsBox->Closing += gcnew CancelEventHandler(this, &OptionsDialog::OptionsBox_Cancel);


	INIMap = gcnew Dictionary<INISetting^, BoundControl^>();
	ColorDictionary = gcnew Dictionary<String^, ColorDialog^>();

	OptionsBox->Hide();
	PopulateINIMap();
	LoadINI();
}


void OptionsDialog::CmDlgFont_Click(Object^ Sender, EventArgs^ E)
{
	FontSelection->ShowDialog();
}

void OptionsDialog::CmDlgColor_Click(Object^ Sender, EventArgs^ E)
{
	Button^ ThisButton = dynamic_cast<Button^>(Sender);

	if (ThisButton != nullptr)
	{
		ColorDialog^ Dialog = dynamic_cast<ColorDialog^>(ThisButton->Tag);
		if (Dialog != nullptr)
		{
			Dialog->ShowDialog();
			ThisButton->BackColor = Dialog->Color;
		}
	}
}


void OptionsDialog::OptionsBox_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	SaveINI();
}


Color OptionsDialog::GetColor(String^ Key)
{
	for each (KeyValuePair<String^, ColorDialog^>% Itr in ColorDictionary)
	{
		if (!String::Compare(Itr.Key, Key, true))
		{
			return Itr.Value->Color;
		}
	}
	return Color::GhostWhite;
}