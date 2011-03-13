#pragma once

public ref class INISetting 
{
public:
	String^											Key;
	String^											Section;
	String^											DefaultValue;

	INISetting(String^ Key, String^ Section, String^ DefaultValue) : Key(Key), Section(Section), DefaultValue(DefaultValue) {}
};

public ref class BoundControl 
{
public:
	static enum class								ControlType
														{
															e_Checkbox = 0,
															e_NumericUpDown,
															e_FontDialog,
															e_ColorDialog
														};
	static enum class								ValueType
														{
															e_Checked = 0,
															e_Value,
															e_Font_FontFamily_Name,
															e_Font_Size,
															e_Font_Style,
															e_Color_R,
															e_Color_G,
															e_Color_B
														};
private:
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


public ref class OptionsDialog
{
	static OptionsDialog^							Singleton = nullptr;

	void											CmDlgFont_Click(Object^ Sender, EventArgs^ E);
	void											CmDlgForeColor_Click(Object^ Sender, EventArgs^ E);
	void											CmDlgBackColor_Click(Object^ Sender, EventArgs^ E);
	void											CmDlgHighlightColor_Click(Object^ Sender, EventArgs^ E);

	void											OptionsBox_Cancel(Object^ Sender, CancelEventArgs^ E);

	Dictionary<INISetting^, BoundControl^>^			INIMap;

	Form^										OptionsBox;
		TabControl^									TabContainer;
			TabPage^										TabGeneral;
				CheckBox^										AutoIndent;
				CheckBox^										SaveLastKnownPos;
				CheckBox^										RecompileVarIdx;
				CheckBox^										UseCSParent;
				CheckBox^										DestroyOnLastTabClose;
				CheckBox^										SuppressRefCountForQuestScripts;
				CheckBox^										LoadScriptUpdateExistingScripts;
			TabPage^									TabIntelliSense;
				Label^										LabelISThreshold;
				NumericUpDown^								ThresholdLength;
				Label^										LabelISDBUpdatePeriod;
				NumericUpDown^								DatabaseUpdateInterval;
				CheckBox^									UseQuickView;
			TabPage^									TabPreprocessor;
				CheckBox^									AllowRedefinitions;
			TabPage^									TabAppearance;
				Button^										CmDlgHighlightColor;
				Button^										CmDlgBackColor;
				Button^										CmDlgForeColor;
				Button^										CmDlgFont;
				Label^										LabelTabSize;
				NumericUpDown^								TabSize;
				Label^										LabelLinesToScroll;
				NumericUpDown^								LinesToScroll;
			TabPage^									TabSanitize;
				CheckBox^									IndentLines;
				CheckBox^									AnnealCasing;

		ColorDialog^  								FCDialog;
		ColorDialog^  								BCDialog;
		ColorDialog^  								HCDialog;
		FontDialog^									FontSelection;
public:	
	OptionsDialog();

	void											LoadINI();
	void											SaveINI();
	void											PopulateINIMap();

	BoundControl^									FetchSetting(String^ Key);
	int												FetchSettingAsInt(String^ Key);
	String^											FetchSettingAsString(String^ Key);

	static enum class								ColorType
													{
														e_Foreground = 0,
														e_Background,
														e_Highlight
													};

	Color											GetColor(ColorType Type);

	void											Show() { OptionsBox->ShowDialog(); }

	static OptionsDialog^%							GetSingleton();
};

#define OPTIONS										OptionsDialog::GetSingleton()