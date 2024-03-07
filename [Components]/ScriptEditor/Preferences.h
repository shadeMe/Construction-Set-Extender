#pragma once
#include "ScriptSync.h"
#include "IntelliSenseBackend.h"

namespace cse
{


namespace scriptEditor
{


namespace preferences
{


using namespace System::ComponentModel;

ref class CustomColorConverter : public System::Drawing::ColorConverter
{
public:
	virtual bool GetStandardValuesSupported(System::ComponentModel::ITypeDescriptorContext ^ context) override { return false; }
};

ref class CustomColorEditor : public System::Drawing::Design::UITypeEditor
{
public:
	virtual System::Drawing::Design::UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext^ context) override;
	virtual Object^ EditValue(ITypeDescriptorContext^ context, IServiceProvider^ provider, Object^ value) override;

	virtual bool GetPaintValueSupported(ITypeDescriptorContext^ context) override;
	virtual void PaintValue(System::Drawing::Design::PaintValueEventArgs^ e) override;
};

ref class CustomFontEditor : public System::Drawing::Design::UITypeEditor
{
public:
	virtual System::Drawing::Design::UITypeEditorEditStyle GetEditStyle(ITypeDescriptorContext^ context) override;
	virtual Object^ EditValue(ITypeDescriptorContext^ context, IServiceProvider^ provider, Object^ value) override;
};

ref class SettingsGroup abstract
{
	static String^ SectionPrefix = "ScriptEditor::";

	bool IsPropertySerializable(System::Reflection::PropertyInfo^ Property);
public:
	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) abstract;
	void Save();
	bool Load();
	SettingsGroup^ Clone();
	String^ GetCategoryName();
	String^ GetIconName();
};

ref struct GeneralSettings : public SettingsGroup
{
	static String^ CategoryName = "General";
	static String^ IconName = "AppSettings";

	[Category("Editor")]
	[Description("Automatically indent script lines")]
	property bool AutoIndent;

	[Category("Editor")]
	[Description("Cut/copy entire line when no text is selected")]
	property bool CutCopyEntireLine;

	[Category("Editor")]
	[Description("Save/restore caret position on save/load")]
	property bool SaveRestoreCaret;

	[Category("Tools")]
	[Description("'Previous/Next Script' will skip scripts that are not from the active plugin file")]
	property bool OnlySwitchToScriptsFromActivePlugin;

	[Category("Tools")]
	[Description("'Load Script(s)' updates existing scripts if editorIDs match")]
	property bool LoadScriptUpdatesExistingScripts;

	[Category("Tools")]
	[Description("Recompile dependencies after variable index modification")]
	property bool RecompileDependsOnVarIdxMod;

	[Category("Script Picker")]
	[Description("Sort scripts according to their flags (modified, deleted, etc) by default")]
	property bool SortScriptsByFlags;

	[Category("Export")]
	[Description("File extension used scripts that are saved to disk")]
	property String^ ExportedScriptFileExtension;

	GeneralSettings()
	{
		AutoIndent = true;
		SaveRestoreCaret = false;
		OnlySwitchToScriptsFromActivePlugin = false;
		LoadScriptUpdatesExistingScripts = true;
		CutCopyEntireLine = true;
		RecompileDependsOnVarIdxMod = true;
		SortScriptsByFlags = true;
		ExportedScriptFileExtension = ".obscript";
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
};

ref struct IntelliSenseSettings : public SettingsGroup
{
	static String^ CategoryName = "IntelliSense";
	static String^ IconName = "IntelliSenseBackEnd";

	[Category("Popup Window")]
	[Description("Width of the popup window")]
	property UInt32 WindowWidth;

	[Category("Suggestions")]
	[Description("Automatically display suggestions on typing")]
	property bool ShowSuggestions;

	[Category("Suggestions")]
	[Description("Automatic suggestions will be shown after the current token at caret location is at least this many characters long")]
	property UInt32 SuggestionCharThreshold;

	[Category("Suggestions")]
	[Description("Insert suggestions with the ENTER key")]
	property bool InsertSuggestionsWithEnterKey;

	[Category("Suggestions")]
	[Description("Maximum number of items visible at once in the popup window")]
	property UInt32 MaxVisiblePopupItems;

	[Category("Suggestions")]
	[Description("Filter mode for suggestions")]
	property intellisense::eFilterMode SuggestionsFilter;

	[Category("Suggestions")]
	[Description("Maximum cost for fuzzy filtering")]
	property UInt32 FuzzyFilterMaxCost;


	[Category("Insight Info")]
	[Description("Show info tooltip on mouse hover")]
	property bool ShowInsightToolTip;

	[Category("Insight Info")]
	[Description("Duration in Earth seconds for which the info tooltip is displayed")]
	property UInt32 InsightToolTipDisplayDuration;

	[Category("Insight Info")]
	[Description("Show error messages in info tooltip")]
	property bool ShowErrorsInInsightToolTip;


	[Category("Database")]
	[Description("IntelliSense database update interval in Earth minutes")]
	property UInt32 DatabaseUpdateInterval;


	[Category("Background Analysis")]
	[Description("Background semantic analysis interval in Earth seconds. This settings affects the latency/accuracy of multiple script editor features")]
	property UInt32 BackgroundAnalysisInterval;

	IntelliSenseSettings()
	{
		WindowWidth = 250;
		ShowSuggestions = true;
		SuggestionCharThreshold = 5;
		InsertSuggestionsWithEnterKey = true;
		MaxVisiblePopupItems = 10;
		SuggestionsFilter = intellisense::eFilterMode::Prefix;
		FuzzyFilterMaxCost = 2;

		ShowInsightToolTip = true;
		InsightToolTipDisplayDuration = 8;
		ShowErrorsInInsightToolTip = true;
		DatabaseUpdateInterval = 5;
		BackgroundAnalysisInterval = 5;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
};

ref struct PreprocessorSettings : public SettingsGroup
{
	static String^ CategoryName = "Preprocessor";
	static String^ IconName = "Preprocessor";

	[Category("Preprocessor")]
	[Description("Allow macro redefinitions")]
	property bool AllowMacroRedefs;

	[Category("Preprocessor")]
	[Description("Number of preprocessing passes to apply")]
	property UInt32 NumPasses;

	PreprocessorSettings()
	{
		AllowMacroRedefs = true;
		NumPasses = 1;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
};

ref struct AppearanceSettings : public SettingsGroup
{
	static String^ CategoryName = "Appearance";
	static String^ IconName = "Appearance";

	[Category("General")]
	[Description("Location of the tab strip")]
	property DevComponents::DotNetBar::eTabStripAlignment TabStripLocation;

	[Category("General")]
	[Description("Width of the vertical tab strip")]
	property UInt32 TabStripVerticalWidth;

	[Category("General")]
	[Description("Display size of tab characters")]
	property UInt32 TabSize;

	[Category("General")]
	[Description("Default text font")]
	[Editor(CustomFontEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	property Drawing::Font^ TextFont;

	[Category("General")]
	[Description("Word wrap text")]
	property bool WordWrap;

	[Category("General")]
	[Description("Display tab characters")]
	property bool ShowTabs;

	[Category("General")]
	[Description("Display space characters")]
	property bool ShowSpaces;

	[Category("General")]
	[Description("Use a dark theme for the script editor interface")]
	property bool DarkMode;

	[Category("General")]
	[Description("Accent color for light mode")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color AccentColor_LM;

	[Category("General")]
	[Description("Accent color for dark mode")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color AccentColor_DM;


	[Category("Adornments")]
	[Description("Show navigation breadcrumb toolbar")]
	property bool ShowScopeBar;

	[Category("Adornments")]
	[Description("Show code folding margin")]
	property bool ShowCodeFolding;

	[Category("Adornments")]
	[Description("Show block visualizer")]
	property bool ShowBlockVisualizer;

	[Category("Adornments")]
	[Description("Show icon margin")]
	property bool ShowIconMargin;

	[Category("Tooltips")]
	[Description("Tooltip background color for error messages")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color TooltipBackColorError;

	[Category("Tooltips")]
	[Description("Tooltip foreground color for error messages")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color TooltipForeColorError;

	[Category("Tooltips")]
	[Description("Tooltip background color for warning messages")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color TooltipBackColorWarning;

	[Category("Tooltips")]
	[Description("Tooltip foreground color for warning messages")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color TooltipForeColorWarning;

	[Category("Tooltips")]
	[Description("Tooltip background color for bookmarks")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color TooltipBackColorBookmark;

	[Category("Tooltips")]
	[Description("Tooltip foreground color for bookmarks")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color TooltipForeColorBookmark;


	[Category("Highlighting (Light Mode)")]
	[Description("Text foreground color")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColor_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Text background color")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColor_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Syntax highlighting foreground color for script keywords")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorKeywords_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Syntax highlighting foreground color for digits")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorDigits_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Syntax highlighting color preprocessor directives")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorPreprocessor_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Syntax highlighting color script block names")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorScriptBlocks_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Syntax highlighting color string literals")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorStringLiterals_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Syntax highlighting foreground color for comments")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorComments_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Syntax highlighting foreground color for local variables")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorLocalVariables_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Highlighting background color for selected text")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorSelection_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Highlighting background color for current line")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorCurrentLine_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Highlighting color for error squigglies")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color UnderlineColorError_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Highlighting color for find results")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorFindResults_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Highlighting background color for task keywords such as TODO, FIXME, etc in comments")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorTaskComment_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Use bold-face font style for all highlighted text")]
	property bool BoldFaceHighlightedText_LM;

	[Category("Highlighting (Light Mode)")]
	[Description("Use italics font style for all keywords and script blocks")]
	property bool KeywordsAndBlocksInItalic_LM;


	[Category("Highlighting (Dark Mode)")]
	[Description("Text foreground color")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColor_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Text background color")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColor_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Syntax highlighting foreground color for script keywords")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorKeywords_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Syntax highlighting foreground color for digits")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorDigits_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Syntax highlighting color preprocessor directives")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorPreprocessor_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Syntax highlighting color script block names")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorScriptBlocks_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Syntax highlighting color string literals")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorStringLiterals_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Syntax highlighting foreground color for comments")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorComments_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Syntax highlighting foreground color for local variables")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color ForeColorLocalVariables_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Highlighting background color for selected text")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorSelection_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Highlighting background color for current line")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorCurrentLine_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Highlighting color for error squigglies")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color UnderlineColorError_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Highlighting color for find results")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorFindResults_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Highlighting background color for task keywords such as TODO, FIXME, etc in comments")]
	[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
	[TypeConverter(CustomColorConverter::typeid)]
	property Color BackColorTaskComment_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Use bold-face font style for all highlighted text")]
	property bool BoldFaceHighlightedText_DM;

	[Category("Highlighting (Dark Mode)")]
	[Description("Use italics font style for all keywords and script blocks")]
	property bool KeywordsInBlocksItalic_DM;


	// convenience accessors
	[Browsable(false)]
	property Color AccentColor
	{
		Color get() { return !DarkMode ? AccentColor_LM : AccentColor_DM; }
	}

	[Browsable(false)]
	property Color ForeColor
	{
		Color get() { return !DarkMode ? ForeColor_LM : ForeColor_DM; }
	}

	[Browsable(false)]
	property Color BackColor
	{
		Color get() { return !DarkMode ? BackColor_LM : BackColor_DM; }
	}

	[Browsable(false)]
	property Color ForeColorKeywords
	{
		Color get() { return !DarkMode ? ForeColorKeywords_LM : ForeColorKeywords_DM; }
	}

	[Browsable(false)]
	property Color ForeColorDigits
	{
		Color get() { return !DarkMode ? ForeColorDigits_LM : ForeColorDigits_DM; }
	}

	[Browsable(false)]
	property Color ForeColorPreprocessor
	{
		Color get() { return !DarkMode ? ForeColorPreprocessor_LM : ForeColorPreprocessor_DM; }
	}

	[Browsable(false)]
	property Color ForeColorScriptBlocks
	{
		Color get() { return !DarkMode ? ForeColorScriptBlocks_LM : ForeColorScriptBlocks_DM; }
	}

	[Browsable(false)]
	property Color ForeColorStringLiterals
	{
		Color get() { return !DarkMode ? ForeColorStringLiterals_LM : ForeColorStringLiterals_DM; }
	}

	[Browsable(false)]
	property Color ForeColorComments
	{
		Color get() { return !DarkMode ? ForeColorComments_LM : ForeColorComments_DM; }
	}

	[Browsable(false)]
	property Color ForeColorLocalVariables
	{
		Color get() { return !DarkMode ? ForeColorLocalVariables_LM : ForeColorLocalVariables_DM; }
	}

	[Browsable(false)]
	property Color BackColorSelection
	{
		Color get() { return !DarkMode ? BackColorSelection_LM : BackColorSelection_DM; }
	}

	[Browsable(false)]
	property Color BackColorCurrentLine
	{
		Color get() { return !DarkMode ? BackColorCurrentLine_LM : BackColorCurrentLine_DM; }
	}

	[Browsable(false)]
	property Color UnderlineColorError
	{
		Color get() { return !DarkMode ? UnderlineColorError_LM : UnderlineColorError_DM; }
	}

	[Browsable(false)]
	property Color BackColorFindResults
	{
		Color get() { return !DarkMode ? BackColorFindResults_LM : BackColorFindResults_DM; }
	}

	[Browsable(false)]
	property Color BackColorTaskComment
	{
		Color get() { return !DarkMode ? BackColorTaskComment_LM : BackColorTaskComment_DM; }
	}

	[Browsable(false)]
	property bool BoldFaceHighlightedText
	{
		bool get() { return !DarkMode ? BoldFaceHighlightedText_LM : BoldFaceHighlightedText_DM; }
	}

	[Browsable(false)]
	property bool KeywordsAndBlocksInItalic
	{
		bool get() { return !DarkMode ? KeywordsAndBlocksInItalic_LM : KeywordsInBlocksItalic_DM; }
	}

	AppearanceSettings()
	{
		TabStripLocation = DevComponents::DotNetBar::eTabStripAlignment::Top;
		TabStripVerticalWidth = 150;
		TabSize = 4;
		TextFont = gcnew Drawing::Font("Consolas", 9.75);
		WordWrap = true;
		ShowTabs = false;
		ShowSpaces = false;
		DarkMode = false;
		AccentColor_LM = Color::FromArgb(0, 122, 204);
		AccentColor_DM = Color::FromArgb(166, 123, 45);

		ShowScopeBar = true;
		ShowCodeFolding = true;
		ShowBlockVisualizer = true;
		ShowIconMargin = true;

		TooltipBackColorError = Color::FromArgb(164, 13, 20);
		TooltipForeColorError = Color::White;
		TooltipBackColorWarning = Color::FromArgb(227, 194, 4);
		TooltipForeColorWarning = Color::Black;
		TooltipBackColorBookmark = Color::FromArgb(80, 80, 160);
		TooltipForeColorBookmark = Color::White;

		ForeColor_LM = Color::FromArgb(0, 0, 0);
		BackColor_LM = Color::FromArgb(239, 241, 241);
		ForeColorKeywords_LM = Color::FromArgb(4, 124, 244);
		ForeColorDigits_LM = Color::FromArgb(185, 120, 0);
		ForeColorPreprocessor_LM = Color::FromArgb(165, 42, 42);
		ForeColorScriptBlocks_LM = Color::FromArgb(250, 30, 5);
		ForeColorStringLiterals_LM = Color::FromArgb(36, 166, 30);
		ForeColorComments_LM = Color::FromArgb(147, 129, 104);
		ForeColorLocalVariables_LM = Color::FromArgb(67, 135, 135);
		BackColorSelection_LM = Color::FromArgb(143, 70, 115);
		BackColorCurrentLine_LM = Color::FromArgb(210, 206, 202);
		UnderlineColorError_LM = Color::FromArgb(255, 0, 0);
		BackColorFindResults_LM = Color::FromArgb(164, 164, 0);
		BackColorTaskComment_LM = Color::FromArgb(150, 0, 0, 0);
		BoldFaceHighlightedText_LM = false;
		KeywordsAndBlocksInItalic_LM = true;

		ForeColor_DM = Color::FromArgb(253, 244, 193);
		BackColor_DM = Color::FromArgb(45, 45, 48);
		ForeColorKeywords_DM = Color::FromArgb(252, 128, 114);
		ForeColorDigits_DM = Color::FromArgb(255, 165, 0);
		ForeColorPreprocessor_DM = Color::FromArgb(165, 42, 42);
		ForeColorScriptBlocks_DM = Color::FromArgb(250, 30, 5);
		ForeColorStringLiterals_DM = Color::FromArgb(149, 192, 133);
		ForeColorComments_DM = Color::FromArgb(168, 153, 132);
		ForeColorLocalVariables_DM = Color::FromArgb(20, 153, 182);
		BackColorSelection_DM = Color::FromArgb(143, 70, 115);
		BackColorCurrentLine_DM = Color::FromArgb(102, 92, 84);
		UnderlineColorError_DM = Color::FromArgb(255, 0, 0);
		BackColorFindResults_DM = Color::FromArgb(255, 255, 224);
		BackColorTaskComment_LM = Color::FromArgb(150, 0, 0, 0);
		BoldFaceHighlightedText_DM = false;
		KeywordsInBlocksItalic_DM = true;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
};

ref struct SanitizerSettings : public SettingsGroup
{
	static String^ CategoryName = "Sanitizer";
	static String^ IconName = "Sanitise";

	[Category("General")]
	[Description("Normalize identifier casing")]
	property bool NormalizeIdentifiers;

	[Category("General")]
	[Description("Indent script lines")]
	property bool IndentLines;

	[Category("General")]
	[Description("Prefix 'Eval' to 'If' & 'ElseIf' statements")]
	property bool PrefixIfElseIfWithEval;

	[Category("General")]
	[Description("Apply compiler override to script blocks")]
	property bool ApplyCompilerOverride;

	SanitizerSettings()
	{
		NormalizeIdentifiers = true;
		IndentLines = true;
		PrefixIfElseIfWithEval = false;
		ApplyCompilerOverride = false;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override { return true; }
};

ref struct BackupSettings : public SettingsGroup
{
	static String^ CategoryName = "Backup";
	static String^ IconName = "Backup";

	[Category("Auto-Recovery")]
	[Description("Use Auto-Recovery")]
	property bool UseAutoRecovery;

	[Category("Auto-Recovery")]
	[Description("Auto-recovery save interval in Earth minutes")]
	property UInt32 AutoRecoveryInterval;

	BackupSettings()
	{
		UseAutoRecovery = true;
		AutoRecoveryInterval = 5;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
};

ref struct ValidatorSettings : public SettingsGroup
{
	static String^ CategoryName = "Validator";
	static String^ IconName = "Validator";

	[Category("Collision Checking")]
	[Description("Check for variable-form name collisions (recommended)")]
	property bool CheckVarFormNameCollisions;

	[Category("Collision Checking")]
	[Description("Check for variable-command name collisions (recommended)")]
	property bool CheckVarCommandNameCollisions;


	[Category("General")]
	[Description("Count local variable references")]
	property bool CountVariableRefs;

	[Category("General")]
	[Description("Suppress local variable references for Quest scripts")]
	property bool NoQuestVariableRefCounting;


	ValidatorSettings()
	{
		CheckVarFormNameCollisions = false;
		CheckVarCommandNameCollisions = false;
		CountVariableRefs = true;
		NoQuestVariableRefCounting = true;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override { return true; }
};

ref struct FindReplaceSettings : public SettingsGroup
{
	static String^ CategoryName = "Find-Replace";
	static String^ IconName = "FindReplace";

	[Category("General")]
	[Description("Case insensitive search")]
	property bool CaseInsensitive;

	[Category("General")]
	[Description("Match whole word only")]
	property bool MatchWholeWord;

	[Category("General")]
	[Description("Use regular expressions")]
	property bool UseRegEx;

	[Category("General")]
	[Description("Ignore hits inside comments")]
	property bool IgnoreComments;

	FindReplaceSettings()
	{
		CaseInsensitive = false;
		MatchWholeWord = false;
		UseRegEx = true;
		IgnoreComments = true;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override { return true; }
};

ref struct ScriptSyncSettings : public SettingsGroup
{
	static String^ CategoryName = "ScriptSync";
	static String^ IconName = "SyncFile";

	[Category("General")]
	[Description("Automatically sync changes")]
	property bool AutoSyncChanges;

	[Category("General")]
	[Description("Automatic sync interval in Earth seconds")]
	property UInt32 AutoSyncInterval;

	[Category("General")]
	[Description("Handling of existing files in the working directory at sync start")]
	property scriptSync::SyncStartEventArgs::eExistingFileHandlingOperation ExistingFileHandlingOp;

	[Category("General")]
	[Description("Automatically delete log files at sync end")]
	property bool AutoDeleteLogs;

	ScriptSyncSettings()
	{
		AutoSyncChanges = true;
		AutoSyncInterval = 5;
		ExistingFileHandlingOp = scriptSync::SyncStartEventArgs::eExistingFileHandlingOperation::Prompt;
		AutoDeleteLogs = false;
	}

	virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
};


ref class SettingsHolder
{
	static SettingsHolder^	Singleton = nullptr;

	SettingsHolder();
public:
	GeneralSettings^ General;
	IntelliSenseSettings^ IntelliSense;
	PreprocessorSettings^ Preprocessor;
	AppearanceSettings^ Appearance;
	SanitizerSettings^ Sanitizer;
	BackupSettings^ Backup;
	ValidatorSettings^ Validator;
	FindReplaceSettings^ FindReplace;
	ScriptSyncSettings^ ScriptSync;

	property List<SettingsGroup^>^ AllGroups;
	event EventHandler^ PreferencesChanged;

	void SaveToDisk();
	void LoadFromDisk();
	void RaisePreferencesChangedEvent();

	static SettingsHolder^ Get();
};


ref class PreferencesDialog : public DevComponents::DotNetBar::Metro::MetroForm
{
	System::ComponentModel::Container^ components;

	DevComponents::DotNetBar::AdvPropertyGrid^ PropertyGrid;
	DevComponents::DotNetBar::Bar^ TopBar;
	DevComponents::DotNetBar::LabelItem^ LabelCategories;
	DevComponents::DotNetBar::LabelItem^ LabelCurrentCategory;
	DevComponents::DotNetBar::Bar^ SidebarSettingsCategories;
	DevComponents::DotNetBar::PanelEx^ ContainerPropertyGrid;

	void SidebarSettingsCategoryButton_Click(Object^ Sender, EventArgs^ E);
	void Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);

	void InitializeComponent();
	bool PopulateCategories();
	bool SwitchCategory(SettingsGroup^ Group);

	Dictionary<SettingsGroup^, DevComponents::DotNetBar::ButtonItem^>^ RegisteredCategories;
	SettingsGroup^ CurrentSelection;
	SettingsGroup^ CurrentSelectionSnapshot;
public:
	PreferencesDialog();
	~PreferencesDialog();
};


} // namespace preferences


} // namespace scriptEditor


} // namespace cse