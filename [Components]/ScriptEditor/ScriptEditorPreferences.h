#pragma once
#include "[Common]\AuxiliaryWindowsForm.h"

namespace ConstructionSetExtender
{
	namespace Preferences
	{
		ref class INISetting
		{
		public:
			String^											Key;
			String^											Section;
			String^											DefaultValue;

			INISetting(String^ Key, String^ Section, String^ DefaultValue) : Key(Key), Section("ScriptEditor::" + Section), DefaultValue(DefaultValue) {}
		};

		ref class BoundControl
		{
		public:
			static enum class								ControlType
			{
				Checkbox = 0,
				NumericUpDown,
				FontDialog,
				ColorDialog
			};
			static enum class								ValueType
			{
				Checked = 0,
				Value,
				Font_FontFamily_Name,
				Font_Size,
				Font_Style,
				Color_R,
				Color_G,
				Color_B
			};
		protected:
			Control^										INIControl;
			CommonDialog^									INIDialog;
			ControlType										BoundType;
			ValueType										Property;
		public:
			BoundControl(Control^ Ctrl, ControlType CtrlType, ValueType ValType) : INIControl(Ctrl), INIDialog(nullptr), BoundType(CtrlType), Property(ValType) {}			// for controls derived from Windows::Forms::Control
			BoundControl(CommonDialog^ Ctrl, ControlType CtrlType, ValueType ValType) : INIControl(nullptr), INIDialog(Ctrl), BoundType(CtrlType), Property(ValType) {}		// for controls derived from Windows::Forms::CommonDialog

			String^											GetValue();
			void											SetValue(String^ Value);
		};

		ref class ScriptEditorPreferences
		{
		protected:
			static ScriptEditorPreferences^					Singleton = nullptr;

			void											CmDlgFont_Click(Object^ Sender, EventArgs^ E);
			void											CmDlgColor_Click(Object^ Sender, EventArgs^ E);
			void											ForceDatabaseUpdate_Click(Object^ Sender, EventArgs^ E);

			void											OptionsBox_Cancel(Object^ Sender, CancelEventArgs^ E);

			Dictionary<INISetting^, BoundControl^>^		SettingCollection;

			AnimatedForm^								OptionsBox;
			TabControl^									TabBox;
			TabPage^									TabGeneral;
			CheckBox^									AutoIndent;
			CheckBox^									SaveLastKnownPos;
			CheckBox^									RecompileVarIdx;
			CheckBox^									UseCSParent;
			CheckBox^									DestroyOnLastTabClose;
			CheckBox^									LoadScriptUpdateExistingScripts;
			CheckBox^									CutCopyEntireLine;
			TabPage^									TabIntelliSense;
			Label^										LabelISThreshold;
			NumericUpDown^								ThresholdLength;
			Label^										LabelISDBUpdatePeriod;
			NumericUpDown^								DatabaseUpdateInterval;
			CheckBox^									UseQuickView;
			Label^										LabelMaxVisibleItems;
			NumericUpDown^								MaxVisibleItems;
			CheckBox^									NoFocusUI;
			Button^										ForceDatabaseUpdate;
			CheckBox^									SubstringSearch;
			TabPage^									TabPreprocessor;
			CheckBox^									AllowRedefinitions;
			Label^										LabelNoOfPasses;
			NumericUpDown^								NoOfPasses;
			TabPage^									TabAppearance;
			Button^										CmDlgFont;
			Label^										LabelTabSize;
			NumericUpDown^								TabSize;
			GroupBox^									GroupBoxSyntaxHighlighting;
			Button^										CmDlgSyntaxCommentsColor;
			Button^										CmDlgSyntaxDigitsColor;
			Button^										CmDlgSyntaxPreprocessorColor;
			Button^										CmDlgSyntaxScriptBlocksColor;
			Button^										CmDlgSyntaxDelimitersColor;
			Button^										CmDlgSyntaxStringsColor;
			Button^										CmDlgSyntaxKeywordsColor;
			Label^										LabelPreprocessor;
			Label^										LabelScriptBlocks;
			Label^										LabelStrings;
			Label^										LabelComments;
			Label^										LabelDigits;
			Label^										LabelDelimiters;
			Label^										LabelKeywords;
			Button^										CmDlgCurrentLineHighlightColor;
			Label^										LabelCurrentLineHighlight;
			Button^										CmDlgCharLimitHighlightColor;
			Label^										LabelCharLimitHighlight;
			Button^										CmDlgErrorHighlightColor;
			Label^										LabelErrorHighlight;
			Button^										CmDlgSelectionHighlightColor;
			Label^										LabelSelectionHighlight;
			Button^										CmDlgFindResultsHighlightColor;
			Label^										LabelFindResultsHighlight;
			Button^										CmDlgBackgroundColor;
			Label^										LabelBackgroundColor;
			Button^										CmDlgForegroundColor;
			Label^										LabelForegroundColor;
			CheckBox^									WordWrap;
			CheckBox^									ShowTabs;
			CheckBox^									ShowSpaces;
			CheckBox^									CodeFolding;
			CheckBox^									TabsOnTop;
			CheckBox^									BoldFacedHighlighting;
			TabPage^									TabSanitize;
			CheckBox^									IndentLines;
			CheckBox^									AnnealCasing;
			CheckBox^									EvalifyIfs;
			CheckBox^									CompilerOverrideBlocks;
			TabPage^									TabBackup;
			Label^										LabelAutoRecoveryInterval;
			CheckBox^									UseAutoRecovery;
			NumericUpDown^								AutoRecoverySavePeriod;
			TabPage^									TabValidator;
			CheckBox^									VarFormNameCollisions;
			CheckBox^									VarCmdNameCollisions;
			CheckBox^									SuppressRefCountForQuestScripts;
			CheckBox^									CountVarRefs;
			Button^										CmDlgSyntaxLocalVarsColor;
			Label^										LabelLocalVars;
			// hidden controls, for settings that aren't displayed in the preferences window

			// Find/Replace
			CheckBox^									CaseInsensitive;
			CheckBox^									MatchWholeWord;
			CheckBox^									UseRegEx;

			FontDialog^									FontSelection;

			Dictionary<String^, ColorDialog^>^			ColorDatabase;
			bool										Closing;

			void										RegisterColorSetting(String^ Key, Color Default, Control^ Parent);
			virtual void								OnPreferencesSaved(EventArgs^ E);
			void										InitializeSettings();
		public:
			ScriptEditorPreferences();

			event EventHandler^							PreferencesSaved;

			void										LoadINI();
			void										SaveINI();

			BoundControl^								FetchSetting(String^ Key, String^ Section);
			int											FetchSettingAsInt(String^ Key, String^ Section);
			String^										FetchSettingAsString(String^ Key, String^ Section);

			Color										LookupColorByKey(String^ Key);

			void										Show();

			static ScriptEditorPreferences^%			GetSingleton();
		};

#define PREFERENCES									Preferences::ScriptEditorPreferences::GetSingleton()
	}
}