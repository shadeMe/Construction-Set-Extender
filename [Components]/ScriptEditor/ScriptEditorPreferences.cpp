#include "ScriptEditorPreferences.h"
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

	try
	{
		Numeric = float::Parse(Value);
	}
	catch (...)
	{
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
			switch (Property)
			{
			case ValueType::e_Font_FontFamily_Name:
				Control->Font = gcnew Font(Value, Control->Font->Size, Control->Font->Style);
				break;
			case ValueType::e_Font_Size:
				Control->Font = gcnew Font(Control->Font->FontFamily->Name, Numeric,  Control->Font->Style);
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

void ScriptEditorPreferences::RegisterColorSetting(String^ Key, Color Default, Control^ Parent)
{
	ColorDialog^ Dialog = gcnew ColorDialog();
	Dialog->AnyColor = true;
	Dialog->Color = Default;
	Dialog->Tag = Parent;

	Parent->Tag = Dialog;

	SettingCollection->Add(gcnew INISetting(Key + "R", "ScriptEditor::Appearance", Default.R.ToString()), gcnew BoundControl(Dialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_R));
	SettingCollection->Add(gcnew INISetting(Key + "G", "ScriptEditor::Appearance", Default.R.ToString()), gcnew BoundControl(Dialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_G));
	SettingCollection->Add(gcnew INISetting(Key + "B", "ScriptEditor::Appearance", Default.R.ToString()), gcnew BoundControl(Dialog, BoundControl::ControlType::e_ColorDialog, BoundControl::ValueType::e_Color_B));

	ColorDatabase->Add(Key, Dialog);
}

void ScriptEditorPreferences::InitializeSettings()
{
	// Appearance
	SettingCollection->Add(gcnew INISetting("Font", "ScriptEditor::Appearance", "Lucida Console"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_FontFamily_Name));
	SettingCollection->Add(gcnew INISetting("FontSize", "ScriptEditor::Appearance", "10"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_Size));
	SettingCollection->Add(gcnew INISetting("FontStyle", "ScriptEditor::Appearance", "0"), gcnew BoundControl(FontSelection, BoundControl::ControlType::e_FontDialog, BoundControl::ValueType::e_Font_Style));

	SettingCollection->Add(gcnew INISetting("TabSize", "ScriptEditor::Appearance", "0"), gcnew BoundControl(TabSize, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	SettingCollection->Add(gcnew INISetting("WordWrap", "ScriptEditor::Appearance", "0"), gcnew BoundControl(WordWrap, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("ShowTabs", "ScriptEditor::Appearance", "0"), gcnew BoundControl(ShowTabs, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("ShowSpaces", "ScriptEditor::Appearance", "0"), gcnew BoundControl(ShowSpaces, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("CodeFolding", "ScriptEditor::Appearance", "1"), gcnew BoundControl(CodeFolding, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("TabsOnTop", "ScriptEditor::Appearance", "1"), gcnew BoundControl(TabsOnTop, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

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

	// General
	SettingCollection->Add(gcnew INISetting("SuppressRefCountForQuestScripts", "ScriptEditor::General", "1"), gcnew BoundControl(SuppressRefCountForQuestScripts, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("AutoIndent", "ScriptEditor::General", "1"), gcnew BoundControl(AutoIndent, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("SaveLastKnownPos", "ScriptEditor::General", "1"), gcnew BoundControl(SaveLastKnownPos, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("RecompileVarIdx", "ScriptEditor::General", "1"), gcnew BoundControl(RecompileVarIdx, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("UseCSParent", "ScriptEditor::General", "0"), gcnew BoundControl(UseCSParent, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("DestroyOnLastTabClose", "ScriptEditor::General", "1"), gcnew BoundControl(DestroyOnLastTabClose, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("LoadScriptUpdateExistingScripts", "ScriptEditor::General", "0"), gcnew BoundControl(LoadScriptUpdateExistingScripts, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("CutCopyEntireLine", "ScriptEditor::General", "0"), gcnew BoundControl(CutCopyEntireLine, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

	// IntelliSense
	SettingCollection->Add(gcnew INISetting("ThresholdLength", "ScriptEditor::IntelliSense", "4"), gcnew BoundControl(ThresholdLength, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	SettingCollection->Add(gcnew INISetting("DatabaseUpdateInterval", "ScriptEditor::IntelliSense", "5"), gcnew BoundControl(DatabaseUpdateInterval, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));
	SettingCollection->Add(gcnew INISetting("UseQuickView", "ScriptEditor::IntelliSense", "1"), gcnew BoundControl(UseQuickView, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));

	// Preprocessor
	SettingCollection->Add(gcnew INISetting("AllowRedefinitions", "ScriptEditor::Preprocessor", "0"), gcnew BoundControl(AllowRedefinitions, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("NoOfPasses", "ScriptEditor::Preprocessor", "1"), gcnew BoundControl(NoOfPasses, BoundControl::ControlType::e_NumericUpDown, BoundControl::ValueType::e_Value));

	// Sanitize
	SettingCollection->Add(gcnew INISetting("AnnealCasing", "ScriptEditor::Sanitize", "1"), gcnew BoundControl(AnnealCasing, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
	SettingCollection->Add(gcnew INISetting("IndentLines", "ScriptEditor::Sanitize", "1"), gcnew BoundControl(IndentLines, BoundControl::ControlType::e_Checkbox, BoundControl::ValueType::e_Checked));
}

void ScriptEditorPreferences::LoadINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in SettingCollection)
	{
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		char Buffer[0x200] = {0};
		g_CSEInterface->CSEEditorAPI.ReadFromINI((CString(INI->Key)).c_str(),
												(CString(INI->Section)).c_str(),
												(CString(INI->DefaultValue)).c_str(), Buffer, sizeof(Buffer));

		Control->SetValue(gcnew String(Buffer));
	}
}

void ScriptEditorPreferences::SaveINI()
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in SettingCollection)
	{
		INISetting^ INI = Itr.Key;
		BoundControl^ Control = Itr.Value;

		g_CSEInterface->CSEEditorAPI.WriteToINI((CString(INI->Key)).c_str(),
												(CString(INI->Section)).c_str(),
												(CString(Control->GetValue())).c_str());
	}
}

BoundControl^ ScriptEditorPreferences::FetchSetting(String^ Key)
{
	for each (KeyValuePair<INISetting^, BoundControl^>% Itr in SettingCollection)
	{
		INISetting^ INI = Itr.Key;
		if (!String::Compare(INI->Key, Key, true))
			return Itr.Value;
	}
	return nullptr;
}

int ScriptEditorPreferences::FetchSettingAsInt(String^ Key)
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
			DebugPrint("Couldn't fetch INI setting '" + Key + "' value. Bad Format.\n\tException: " + E->Message);
		}
		return Result;
	}
	else
	{
		DebugPrint("Couldn't fetch INI setting '" + Key + "' value.\n\tException: Key doesn't exist.");
		return 0;
	}
}

String^ ScriptEditorPreferences::FetchSettingAsString(String^ Key)
{
	BoundControl^ Control = FetchSetting(Key);
	if (Control)
		return Control->GetValue();
	else
	{
		DebugPrint("Couldn't fetch INI setting '" + Key + "' value.\n\tException: Key doesn't exist.");
		return "";
	}
}

ScriptEditorPreferences^% ScriptEditorPreferences::GetSingleton()
{
	if (Singleton == nullptr)
	{
		Singleton = gcnew ScriptEditorPreferences();
	}
	return Singleton;
}

ScriptEditorPreferences::ScriptEditorPreferences()
{
	FontSelection = gcnew FontDialog();
	FontSelection->AllowScriptChange = false;
	FontSelection->AllowVerticalFonts = false;
	FontSelection->ShowEffects = false;

	OptionsBox = gcnew AnimatedForm(0.25);
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
	WordWrap = (gcnew CheckBox());
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
	ShowTabs = (gcnew CheckBox());
	ShowSpaces = (gcnew CheckBox());
	CutCopyEntireLine = (gcnew CheckBox());
	LabelNoOfPasses = gcnew Label();
	NoOfPasses = gcnew NumericUpDown();
	CodeFolding = gcnew CheckBox();
	TabsOnTop = gcnew CheckBox();

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
	AllowRedefinitions->Location = System::Drawing::Point(125, 79);
	AllowRedefinitions->Name = L"AllowRedefinitions";
	AllowRedefinitions->Size = System::Drawing::Size(154, 23);
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
	LabelTabSize->Location = System::Drawing::Point(12, 263);
	LabelTabSize->Name = L"LabelTabSize";
	LabelTabSize->Size = System::Drawing::Size(49, 13);
	LabelTabSize->TabIndex = 3;
	LabelTabSize->Text = L"Tab Size";
	//
	// TabSize
	//
	TabSize->Location = System::Drawing::Point(73, 261);
	TabSize->Name = L"TabSize";
	TabSize->Size = System::Drawing::Size(88, 20);
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
	this->CmDlgFont->Location = System::Drawing::Point(312, 210);
	this->CmDlgFont->Name = L"CmDlgFont";
	this->CmDlgFont->Size = System::Drawing::Size(88, 24);
	CmDlgFont->TabIndex = 0;
	CmDlgFont->Text = L"Editor Font";
	CmDlgFont->UseVisualStyleBackColor = true;
	CmDlgFont->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgFont_Click);
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
	TabGeneral->Controls->Add(CutCopyEntireLine);
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
	TabPreprocessor->Controls->Add(LabelNoOfPasses);
	TabPreprocessor->Controls->Add(NoOfPasses);
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
	TabAppearance->Controls->Add(ShowTabs);
	TabAppearance->Controls->Add(ShowSpaces);
	TabAppearance->Controls->Add(CmDlgFindResultsHighlightColor);
	TabAppearance->Controls->Add(LabelFindResultsHighlight);
	TabAppearance->Controls->Add(CmDlgCurrentLineHighlightColor);
	TabAppearance->Controls->Add(LabelCurrentLineHighlight);
	TabAppearance->Controls->Add(CmDlgCharLimitHighlightColor);
	TabAppearance->Controls->Add(LabelCharLimitHighlight);
	TabAppearance->Controls->Add(CmDlgErrorHighlightColor);
	TabAppearance->Controls->Add(LabelErrorHighlight);
	TabAppearance->Controls->Add(CmDlgSelectionHighlightColor);
	TabAppearance->Controls->Add(WordWrap);
	TabAppearance->Controls->Add(LabelSelectionHighlight);
	TabAppearance->Controls->Add(GroupBoxSyntaxHighlighting);
	TabAppearance->Controls->Add(CmDlgFont);
	TabAppearance->Controls->Add(LabelTabSize);
	TabAppearance->Controls->Add(TabSize);
	TabAppearance->Controls->Add(CodeFolding);
	TabAppearance->Controls->Add(TabsOnTop);
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
	this->GroupBoxSyntaxHighlighting->Location = System::Drawing::Point(6, 3);
	this->GroupBoxSyntaxHighlighting->Name = L"GroupBoxSyntaxHighlighting";
	this->GroupBoxSyntaxHighlighting->Size = System::Drawing::Size(155, 247);
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
	this->LabelDigits->Location = System::Drawing::Point(6, 54);
	this->LabelDigits->Name = L"LabelDigits";
	this->LabelDigits->Size = System::Drawing::Size(78, 20);
	this->LabelDigits->TabIndex = 7;
	this->LabelDigits->Text = L"Digits";
	//
	// LabelComments
	//
	this->LabelComments->Location = System::Drawing::Point(6, 215);
	this->LabelComments->Name = L"LabelComments";
	this->LabelComments->Size = System::Drawing::Size(78, 20);
	this->LabelComments->TabIndex = 8;
	this->LabelComments->Text = L"Comments";
	//
	// LabelStrings
	//
	this->LabelStrings->Location = System::Drawing::Point(6, 182);
	this->LabelStrings->Name = L"LabelStrings";
	this->LabelStrings->Size = System::Drawing::Size(78, 20);
	this->LabelStrings->TabIndex = 9;
	this->LabelStrings->Text = L"String Literals";
	//
	// LabelScriptBlocks
	//
	this->LabelScriptBlocks->Location = System::Drawing::Point(6, 119);
	this->LabelScriptBlocks->Name = L"LabelScriptBlocks";
	this->LabelScriptBlocks->Size = System::Drawing::Size(78, 20);
	this->LabelScriptBlocks->TabIndex = 10;
	this->LabelScriptBlocks->Text = L"Script Blocks";
	//
	// LabelPreprocessor
	//
	this->LabelPreprocessor->Location = System::Drawing::Point(6, 86);
	this->LabelPreprocessor->Name = L"LabelPreprocessor";
	this->LabelPreprocessor->Size = System::Drawing::Size(78, 20);
	this->LabelPreprocessor->TabIndex = 11;
	this->LabelPreprocessor->Text = L"Preprocessor";
	//
	// CmDlgSyntaxCommentsColor
	//
	this->CmDlgSyntaxCommentsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSyntaxCommentsColor->Location = System::Drawing::Point(103, 211);
	this->CmDlgSyntaxCommentsColor->Name = L"CmDlgSyntaxCommentsColor";
	this->CmDlgSyntaxCommentsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxCommentsColor->TabIndex = 18;
	CmDlgSyntaxCommentsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxCommentsColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// CmDlgSyntaxKeywordsColor
	//
	this->CmDlgSyntaxKeywordsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSyntaxKeywordsColor->Location = System::Drawing::Point(103, 18);
	this->CmDlgSyntaxKeywordsColor->Name = L"CmDlgSyntaxKeywordsColor";
	this->CmDlgSyntaxKeywordsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxKeywordsColor->TabIndex = 12;
	CmDlgSyntaxKeywordsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxKeywordsColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// CmDlgSyntaxStringsColor
	//
	this->CmDlgSyntaxStringsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSyntaxStringsColor->Location = System::Drawing::Point(103, 178);
	this->CmDlgSyntaxStringsColor->Name = L"CmDlgSyntaxStringsColor";
	this->CmDlgSyntaxStringsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxStringsColor->TabIndex = 13;
	CmDlgSyntaxStringsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxStringsColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// CmDlgSyntaxDelimitersColor
	//
	this->CmDlgSyntaxDelimitersColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSyntaxDelimitersColor->Location = System::Drawing::Point(103, 146);
	this->CmDlgSyntaxDelimitersColor->Name = L"CmDlgSyntaxDelimitersColor";
	this->CmDlgSyntaxDelimitersColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxDelimitersColor->TabIndex = 14;
	CmDlgSyntaxDelimitersColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxDelimitersColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// CmDlgSyntaxScriptBlocksColor
	//
	this->CmDlgSyntaxScriptBlocksColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSyntaxScriptBlocksColor->Location = System::Drawing::Point(103, 113);
	this->CmDlgSyntaxScriptBlocksColor->Name = L"CmDlgSyntaxScriptBlocksColor";
	this->CmDlgSyntaxScriptBlocksColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxScriptBlocksColor->TabIndex = 15;
	CmDlgSyntaxScriptBlocksColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxScriptBlocksColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// CmDlgSyntaxPreprocessorColor
	//
	this->CmDlgSyntaxPreprocessorColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSyntaxPreprocessorColor->Location = System::Drawing::Point(103, 82);
	this->CmDlgSyntaxPreprocessorColor->Name = L"CmDlgSyntaxPreprocessorColor";
	this->CmDlgSyntaxPreprocessorColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxPreprocessorColor->TabIndex = 16;
	CmDlgSyntaxPreprocessorColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxPreprocessorColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// CmDlgSyntaxDigitsColor
	//
	this->CmDlgSyntaxDigitsColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSyntaxDigitsColor->Location = System::Drawing::Point(103, 50);
	this->CmDlgSyntaxDigitsColor->Name = L"CmDlgSyntaxDigitsColor";
	this->CmDlgSyntaxDigitsColor->Size = System::Drawing::Size(34, 21);
	CmDlgSyntaxDigitsColor->TabIndex = 17;
	CmDlgSyntaxDigitsColor->UseVisualStyleBackColor = true;
	CmDlgSyntaxDigitsColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// Wordwrap
	//
	this->WordWrap->AutoSize = true;
	this->WordWrap->Location = System::Drawing::Point(176, 217);
	this->WordWrap->Name = L"Wordwrap";
	this->WordWrap->Size = System::Drawing::Size(81, 17);
	this->WordWrap->TabIndex = 5;
	this->WordWrap->Text = L"Word-Wrap";
	this->WordWrap->UseVisualStyleBackColor = true;
	//
	// CmDlgSelectionHighlightColor
	//
	this->CmDlgSelectionHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgSelectionHighlightColor->Location = System::Drawing::Point(287, 7);
	this->CmDlgSelectionHighlightColor->Name = L"CmDlgSelectionHighlightColor";
	this->CmDlgSelectionHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgSelectionHighlightColor->TabIndex = 19;
	CmDlgSelectionHighlightColor->UseVisualStyleBackColor = true;
	CmDlgSelectionHighlightColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// LabelSelectionHighlight
	//
	this->LabelSelectionHighlight->Location = System::Drawing::Point(176, 10);
	this->LabelSelectionHighlight->Name = L"LabelSelectionHighlight";
	this->LabelSelectionHighlight->Size = System::Drawing::Size(108, 20);
	this->LabelSelectionHighlight->TabIndex = 18;
	this->LabelSelectionHighlight->Text = L"Selection Highlight";
	//
	// CmDlgErrorHighlightColor
	//
	this->CmDlgErrorHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgErrorHighlightColor->Location = System::Drawing::Point(287, 104);
	this->CmDlgErrorHighlightColor->Name = L"CmDlgErrorHighlightColor";
	this->CmDlgErrorHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgErrorHighlightColor->TabIndex = 21;
	CmDlgErrorHighlightColor->UseVisualStyleBackColor = true;
	CmDlgErrorHighlightColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// LabelErrorHighlight
	//
	this->LabelErrorHighlight->Location = System::Drawing::Point(176, 107);
	this->LabelErrorHighlight->Name = L"LabelErrorHighlight";
	this->LabelErrorHighlight->Size = System::Drawing::Size(108, 20);
	this->LabelErrorHighlight->TabIndex = 20;
	this->LabelErrorHighlight->Text = L"Error Highlight";
	//
	// CmDlgCharLimitColor
	//
	this->CmDlgCharLimitHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgCharLimitHighlightColor->Location = System::Drawing::Point(287, 72);
	this->CmDlgCharLimitHighlightColor->Name = L"CmDlgCharLimitColor";
	this->CmDlgCharLimitHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgCharLimitHighlightColor->TabIndex = 23;
	CmDlgCharLimitHighlightColor->UseVisualStyleBackColor = true;
	CmDlgCharLimitHighlightColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// LabelCharLimitHighlight
	//
	this->LabelCharLimitHighlight->Location = System::Drawing::Point(176, 68);
	this->LabelCharLimitHighlight->Name = L"LabelCharLimitHighlight";
	this->LabelCharLimitHighlight->Size = System::Drawing::Size(108, 30);
	this->LabelCharLimitHighlight->TabIndex = 22;
	this->LabelCharLimitHighlight->Text = L"Character Limit Highlight";
	//
	// CmDlgCurrentLineHighlightColor
	//
	this->CmDlgCurrentLineHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgCurrentLineHighlightColor->Location = System::Drawing::Point(287, 39);
	this->CmDlgCurrentLineHighlightColor->Name = L"CmDlgCurrentLineHighlightColor";
	this->CmDlgCurrentLineHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgCurrentLineHighlightColor->TabIndex = 25;
	CmDlgCurrentLineHighlightColor->UseVisualStyleBackColor = true;
	CmDlgCurrentLineHighlightColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// LabelCurrentLineHighlight
	//
	this->LabelCurrentLineHighlight->Location = System::Drawing::Point(176, 35);
	this->LabelCurrentLineHighlight->Name = L"LabelCurrentLineHighlight";
	this->LabelCurrentLineHighlight->Size = System::Drawing::Size(108, 26);
	this->LabelCurrentLineHighlight->TabIndex = 24;
	this->LabelCurrentLineHighlight->Text = L"Current Line Highlight";
	//
	// CmDlgFindResultsHighlightColor
	//
	this->CmDlgFindResultsHighlightColor->FlatStyle = System::Windows::Forms::FlatStyle::Flat;
	this->CmDlgFindResultsHighlightColor->Location = System::Drawing::Point(287, 136);
	this->CmDlgFindResultsHighlightColor->Name = L"CmDlgFindResultsHighlightColor";
	this->CmDlgFindResultsHighlightColor->Size = System::Drawing::Size(34, 21);
	CmDlgFindResultsHighlightColor->TabIndex = 27;
	CmDlgFindResultsHighlightColor->UseVisualStyleBackColor = true;
	CmDlgFindResultsHighlightColor->Click += gcnew System::EventHandler(this, &ScriptEditorPreferences::CmDlgColor_Click);
	//
	// LabelFindResultsHighlight
	//
	this->LabelFindResultsHighlight->Location = System::Drawing::Point(176, 135);
	this->LabelFindResultsHighlight->Name = L"LabelFindResultsHighlight";
	this->LabelFindResultsHighlight->Size = System::Drawing::Size(108, 33);
	this->LabelFindResultsHighlight->TabIndex = 26;
	this->LabelFindResultsHighlight->Text = L"Find Results Highlight";
	//
	// ShowTabs
	//
	this->ShowTabs->AutoSize = true;
	this->ShowTabs->Location = System::Drawing::Point(176, 240);
	this->ShowTabs->Name = L"ShowTabs";
	this->ShowTabs->Size = System::Drawing::Size(80, 17);
	this->ShowTabs->TabIndex = 28;
	this->ShowTabs->Text = L"Show Tabs";
	this->ShowTabs->UseVisualStyleBackColor = true;
	//
	// ShowSpaces
	//
	this->ShowSpaces->AutoSize = true;
	this->ShowSpaces->Location = System::Drawing::Point(176, 263);
	this->ShowSpaces->Name = L"ShowSpaces";
	this->ShowSpaces->Size = System::Drawing::Size(92, 17);
	this->ShowSpaces->TabIndex = 29;
	this->ShowSpaces->Text = L"Show Spaces";
	this->ShowSpaces->UseVisualStyleBackColor = true;
	//
	// CutCopyEntireLine
	//
	CutCopyEntireLine->AutoSize = true;
	CutCopyEntireLine->Location = System::Drawing::Point(22, 138);
	CutCopyEntireLine->Name = L"CutCopyEntireLine";
	CutCopyEntireLine->Size = System::Drawing::Size(242, 17);
	CutCopyEntireLine->TabIndex = 29;
	CutCopyEntireLine->Text = L"Copy/Cut Entire Line When Selection\'s Empty";
	CutCopyEntireLine->UseVisualStyleBackColor = true;
	//
	// LabelNoOfPasses
	//
	LabelNoOfPasses->Location = System::Drawing::Point(125, 133);
	LabelNoOfPasses->Name = L"LabelNoOfPasses";
	LabelNoOfPasses->Size = System::Drawing::Size(157, 20);
	LabelNoOfPasses->TabIndex = 11;
	LabelNoOfPasses->Text = L"Number of Passes";
	//
	// NoOfPasses
	//
	NoOfPasses->Location = System::Drawing::Point(125, 156);
	NoOfPasses->Name = L"NoOfPasses";
	NoOfPasses->Size = System::Drawing::Size(154, 20);
	NoOfPasses->TabIndex = 10;
	//
	// CodeFolding
	//
	this->CodeFolding->AutoSize = true;
	this->CodeFolding->Location = System::Drawing::Point(312, 240);
	this->CodeFolding->Name = L"CodeFolding";
	this->CodeFolding->Size = System::Drawing::Size(88, 17);
	this->CodeFolding->TabIndex = 30;
	this->CodeFolding->Text = L"Code Folding";
	this->CodeFolding->UseVisualStyleBackColor = true;
	//
	// TabsOnTop
	//
	this->TabsOnTop->AutoSize = true;
	this->TabsOnTop->Location = System::Drawing::Point(312, 263);
	this->TabsOnTop->Name = L"TabsOnTop";
	this->TabsOnTop->Size = System::Drawing::Size(89, 17);
	this->TabsOnTop->TabIndex = 31;
	this->TabsOnTop->Text = L"Tabs On Top";
	this->TabsOnTop->UseVisualStyleBackColor = true;
	//
	// ScriptEditorPreferences
	//
	OptionsBox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	OptionsBox->AutoScaleMode = AutoScaleMode::Font;
	OptionsBox->ClientSize = System::Drawing::Size(456, 337);
	OptionsBox->Controls->Add(TabContainer);
	OptionsBox->FormBorderStyle = FormBorderStyle::FixedDialog;
	OptionsBox->Name = L"ScriptEditorPreferences";
	OptionsBox->StartPosition = FormStartPosition::CenterScreen;
	OptionsBox->Text = L"Preferences";
	OptionsBox->MaximizeBox = false;
	OptionsBox->MinimizeBox = false;
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
	OptionsBox->Closing += gcnew CancelEventHandler(this, &ScriptEditorPreferences::OptionsBox_Cancel);

	SettingCollection = gcnew Dictionary<INISetting^, BoundControl^>();
	ColorDatabase = gcnew Dictionary<String^, ColorDialog^>();

	OptionsBox->Hide();
	InitializeSettings();
	LoadINI();

	Closing = false;
}

void ScriptEditorPreferences::CmDlgFont_Click(Object^ Sender, EventArgs^ E)
{
	FontSelection->ShowDialog();
}

void ScriptEditorPreferences::CmDlgColor_Click(Object^ Sender, EventArgs^ E)
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

void ScriptEditorPreferences::OptionsBox_Cancel(Object^ Sender, CancelEventArgs^ E)
{
	if (Closing == false)
	{
		E->Cancel = true;
		Closing = true;
		SaveINI();
		OptionsBox->Close();
	}
}

Color ScriptEditorPreferences::LookupColorByKey(String^ Key)
{
	for each (KeyValuePair<String^, ColorDialog^>% Itr in ColorDatabase)
	{
		if (!String::Compare(Itr.Key, Key, true))
		{
			return Itr.Value->Color;
		}
	}
	return Color::GhostWhite;
}

void ScriptEditorPreferences::Show()
{
	Closing = false;
	OptionsBox->ShowDialog();
}