#pragma once
#include "[Common]\AuxiliaryWindowsForm.h"
#include "ScriptSync.h"

namespace cse
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
			virtual System::Drawing::Design::UITypeEditorEditStyle
							GetEditStyle(ITypeDescriptorContext^ context) override;
			virtual Object^ EditValue(ITypeDescriptorContext^ context, IServiceProvider^ provider, Object^ value) override;

			virtual bool	GetPaintValueSupported(ITypeDescriptorContext^ context) override;
			virtual void	PaintValue(System::Drawing::Design::PaintValueEventArgs^ e) override;
		};

		ref class CustomFontEditor : public System::Drawing::Design::UITypeEditor
		{
		public:
			virtual System::Drawing::Design::UITypeEditorEditStyle
							GetEditStyle(ITypeDescriptorContext^ context) override;
			virtual Object^ EditValue(ITypeDescriptorContext^ context, IServiceProvider^ provider, Object^ value) override;
		};

		ref class SettingsGroup abstract
		{
			static String^	SectionPrefix = "ScriptEditor::";
		public:
			virtual bool	Validate(SettingsGroup^ OldValue, String^% OutMessage) abstract;
			void			Save();
			bool			Load();
			SettingsGroup^	Clone();

			String^			GetCategoryName();
		};

		ref struct GeneralSettings : public SettingsGroup
		{
			static String^ CategoryName = "General";

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
			[Description("'Load Script(s)' updates existing scripts if editorIDs match")]
			property bool LoadScriptUpdatesExistingScripts;

			[Category("Tools")]
			[Description("Recompile dependencies after variable index modification")]
			property bool RecompileDependsOnVarIdxMod;

			[Category("Window")]
			[Description("Hide script editor window in Windows taskbar/task-switcher")]
			property bool HideInTaskbar;

			[Category("Script Picker")]
			[Description("Sort scripts according to their flags (modified, deleted, etc) by default")]
			property bool SortScriptsByFlags;

			[Category("Export")]
			[Description("File extension used scripts that are saved to disk")]
			property String^ ExportedScriptFileExtension;

			GeneralSettings()
			{
				AutoIndent = true;
				HideInTaskbar = false;
				SaveRestoreCaret = false;
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

			[Category("Popup Window")]
			[Description("Width of the popup window")]
			property UInt32 WindowWidth;

			[Category("Suggestions")]
			[Description("Automatically display suggestions")]
			property bool ShowSuggestions;

			[Category("Suggestions")]
			[Description("Character threshold for automatic suggestions")]
			property UInt32 SuggestionCharThreshold;

			[Category("Suggestions")]
			[Description("Insert suggestions with ENTER key")]
			property bool InsertSuggestionsWithEnterKey;

			[Category("Suggestions")]
			[Description("Filter suggestions using substring search")]
			property bool UseSubstringSearch;

			[Category("Suggestions")]
			[Description("Maximum number of suggestions to display")]
			property UInt32 MaxSuggestionsToDisplay;


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
			[Description("Background semantic analysis interval in Earth seconds. "
							"This settings affects the latency/accuracy of multiple script editor features")]
			property UInt32 BackgroundAnalysisInterval;

			IntelliSenseSettings()
			{
				WindowWidth = 250;
				ShowSuggestions = true;
				SuggestionCharThreshold = 5;
				InsertSuggestionsWithEnterKey = true;
				UseSubstringSearch = false;
				MaxSuggestionsToDisplay = 7;
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

			[Category("General")]
			[Description("Display size of tab characters")]
			property UInt32 TabSize;

			[Category("General")]
			[Description("Default text font")]
			//[Editor(CustomFontEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			property Font^ TextFont;

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

			[Category("Adornments")]
			[Description("Show bytecode offset margin")]
			property bool ShowBytecodeOffsetMargin;

			[Category("Highlighting (Light Mode)")]
			[Description("Text foreground color")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Text background color")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Syntax highlighting foreground color for script keywords")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorKeywordsLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Syntax highlighting foreground color for digits")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorDigitsLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Syntax highlighting color preprocessor directives")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorPreprocessorLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Syntax highlighting color script block names")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorScriptBlocksLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Syntax highlighting color string literals")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorStringLiteralsLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Syntax highlighting foreground color for comments")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorCommentsLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Syntax highlighting foreground color for local variables")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorLocalVariablesLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Highlighting background color for selected text")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorSelectionLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Highlighting background color for current line")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorCurrentLineLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Highlighting background color for lines exceeding the character limit (512)")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorCharLimitLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Highlighting color for error squigglies")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color UnderlineColorErrorLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Highlighting color for find results")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorFindResultsLightMode;

			[Category("Highlighting (Light Mode)")]
			[Description("Use bold-face font style for all highlighted text")]
			property bool BoldFaceHighlightedTextLightMode;


			[Category("Highlighting (Dark Mode)")]
			[Description("Text foreground color")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Text background color")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Syntax highlighting foreground color for script keywords")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorKeywordsDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Syntax highlighting foreground color for digits")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorDigitsDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Syntax highlighting color preprocessor directives")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorPreprocessorDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Syntax highlighting color script block names")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorScriptBlocksDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Syntax highlighting color string literals")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorStringLiteralsDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Syntax highlighting foreground color for comments")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorCommentsDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Syntax highlighting foreground color for local variables")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color ForeColorLocalVariablesDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Highlighting background color for selected text")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorSelectionDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Highlighting background color for current line")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorCurrentLineDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Highlighting background color for lines exceeding the character limit (512)")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorCharLimitDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Highlighting color for error squigglies")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color UnderlineColorErrorDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Highlighting color for find results")]
			[Editor(CustomColorEditor::typeid, System::Drawing::Design::UITypeEditor::typeid)]
			[TypeConverter(CustomColorConverter::typeid)]
			property Color BackColorFindResultsDarkMode;

			[Category("Highlighting (Dark Mode)")]
			[Description("Use bold-face font style for all highlighted text")]
			property bool BoldFaceHighlightedTextDarkMode;



			AppearanceSettings()
			{
				TabSize = 4;
				TextFont = gcnew Font("Segoe UI", 12);
				WordWrap = false;
				ShowTabs = false;
				ShowSpaces = false;
				DarkMode = false;

				ShowScopeBar = true;
				ShowCodeFolding = true;
				ShowBlockVisualizer = true;
				ShowIconMargin = true;
				ShowBytecodeOffsetMargin = false;

				ForeColorLightMode = Color::FromArgb(253, 244, 193);
				BackColorLightMode = Color::FromArgb(29, 32, 33);
				ForeColorKeywordsLightMode = Color::FromArgb(252, 128, 114);
				ForeColorDigitsLightMode = Color::FromArgb(255, 165, 0);
				ForeColorPreprocessorLightMode = Color::FromArgb(165, 42, 42);
				ForeColorScriptBlocksLightMode = Color::FromArgb(250, 30, 5);
				ForeColorStringLiteralsLightMode = Color::FromArgb(149, 192, 133);
				ForeColorCommentsLightMode = Color::FromArgb(168, 153, 132);
				ForeColorLocalVariablesLightMode = Color::FromArgb(20, 153, 182);
				BackColorSelectionLightMode = Color::FromArgb(143, 70, 115);
				BackColorCurrentLineLightMode = Color::FromArgb(102, 92, 84);
				BackColorCharLimitLightMode = Color::FromArgb(139, 0, 139);
				UnderlineColorErrorLightMode = Color::FromArgb(255, 0, 0);
				BackColorFindResultsLightMode = Color::FromArgb(255, 255, 224);
				BoldFaceHighlightedTextLightMode = false;

				ForeColorDarkMode = Color::FromArgb(253, 244, 193);
				BackColorDarkMode = Color::FromArgb(29, 32, 33);
				ForeColorKeywordsDarkMode = Color::FromArgb(252, 128, 114);
				ForeColorDigitsDarkMode = Color::FromArgb(255, 165, 0);
				ForeColorPreprocessorDarkMode = Color::FromArgb(165, 42, 42);
				ForeColorScriptBlocksDarkMode = Color::FromArgb(250, 30, 5);
				ForeColorStringLiteralsDarkMode = Color::FromArgb(149, 192, 133);
				ForeColorCommentsDarkMode = Color::FromArgb(168, 153, 132);
				ForeColorLocalVariablesDarkMode = Color::FromArgb(20, 153, 182);
				BackColorSelectionDarkMode = Color::FromArgb(143, 70, 115);
				BackColorCurrentLineDarkMode = Color::FromArgb(102, 92, 84);
				BackColorCharLimitDarkMode = Color::FromArgb(139, 0, 139);
				UnderlineColorErrorDarkMode = Color::FromArgb(255, 0, 0);
				BackColorFindResultsDarkMode = Color::FromArgb(255, 255, 224);
				BoldFaceHighlightedTextDarkMode = false;
			}

			virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
		};

		ref struct SanitizerSettings : public SettingsGroup
		{
			static String^ CategoryName = "Sanitizer";

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

			[Category("General")]
			[Description("'Ctrl+F' displays inline search panel")]
			property bool ShowInlineSearchPanel;

			FindReplaceSettings()
			{
				CaseInsensitive = false;
				MatchWholeWord = false;
				UseRegEx = true;
				IgnoreComments = true;
				ShowInlineSearchPanel = true;
			}

			virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override { return true; }
		};

		ref struct ScriptSyncSettings : public SettingsGroup
		{
			static String^ CategoryName = "ScriptSync";

			[Category("General")]
			[Description("Automatically sync changes")]
			property bool AutoSyncChanges;

			[Category("General")]
			[Description("Automatic sync interval in Earth seconds")]
			property UInt32 AutoSyncInterval;

			[Category("General")]
			[Description("Handling of existing files in the working directory at sync start")]
			property scriptEditor::scriptSync::SyncStartEventArgs::ExistingFileHandlingOperation ExistingFileHandlingOp;

			[Category("General")]
			[Description("Automatically delete log files at sync end")]
			property bool AutoDeleteLogs;

			ScriptSyncSettings()
			{
				AutoSyncChanges = true;
				AutoSyncInterval = 5;
				ExistingFileHandlingOp = scriptEditor::scriptSync::SyncStartEventArgs::ExistingFileHandlingOperation::Prompt;
				AutoDeleteLogs = false;
			}

			virtual bool Validate(SettingsGroup^ OldValue, String^% OutMessage) override;
		};


		ref class SettingsHolder
		{
			static SettingsHolder^	Singleton = nullptr;

			SettingsHolder()
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
		public:
			GeneralSettings^		General;
			IntelliSenseSettings^	IntelliSense;
			PreprocessorSettings^	Preprocessor;
			AppearanceSettings^		Appearance;
			SanitizerSettings^		Sanitizer;
			BackupSettings^			Backup;
			ValidatorSettings^		Validator;
			FindReplaceSettings^	FindReplace;
			ScriptSyncSettings^		ScriptSync;

			property List<SettingsGroup^>^ AllGroups;
			event EventHandler^ PreferencesChanged;

			void SaveToDisk();
			void LoadFromDisk();
			void RaisePreferencesChangedEvent();

			static SettingsHolder^ Get();
		};


		ref class PreferencesDialog : public System::Windows::Forms::Form
		{
			static PreferencesDialog^	ActiveDialog = nullptr;


			System::ComponentModel::Container^		components;

			PropertyGrid^			PropertyGrid;
			ToolStrip^				ToolStripSettingCategories;
			ToolStripLabel^			ToolStripLabelCategories;
			Label^					LabelCurrentCategory;

			void					ToolStripCategoryButton_Click(Object^ Sender, EventArgs^ E);
			void					Dialog_Cancel(Object^ Sender, CancelEventArgs^ E);

			void					InitializeComponent();
			bool					PopulateCategories();
			bool					SwitchCategory(SettingsGroup^ Group);

			Dictionary<SettingsGroup^, ToolStripButton^>^
									RegisteredCategories;
			SettingsGroup^			CurrentSelection;
			SettingsGroup^			CurrentSelectionSnapshot;
		public:
			PreferencesDialog();
			~PreferencesDialog();

			static PreferencesDialog^	GetActiveInstance() { return ActiveDialog; }
		};
	}
}