#pragma once
#include "[Common]\Includes.h"

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

	void											FontButton_Click(Object^ Sender, EventArgs^ E);
	void											FCButton_Click(Object^ Sender, EventArgs^ E);
	void											BCButton_Click(Object^ Sender, EventArgs^ E);
	void											HCButton_Click(Object^ Sender, EventArgs^ E);
	void											BMCButton_Click(Object^ Sender, EventArgs^ E);

	void											OptionsBox_Cancel(Object^ Sender, CancelEventArgs^ E);

	Dictionary<INISetting^, BoundControl^>^			INIMap;

	Form^											OptionsBox;
	GroupBox^										GroupPreP;
	CheckBox^										AllowRedefinitions;
	CheckBox^										CreateMissingFromSegment;

	GroupBox^  										GroupIS;
	Label^  										ISThreshold;
	NumericUpDown^  								ThresholdLength;

	GroupBox^  										GroupGen;
	CheckBox^										AutoIndent;
	Button^  										FCButton;
	Button^  										FontButton;
	CheckBox^  										SuppressRefCountForQuestScripts;
	CheckBox^  										ColorEditorBox;

	FontDialog^  									FontSelection;
	Button^  										BCButton;
	Button^  										HCButton;
	
	Button^  										BMCButton;
	CheckBox^										SaveLastKnownPos;
	Label^  										TabStopSize;
	NumericUpDown^  								TabSize;
	CheckBox^										RecompileVarIdx;
	CheckBox^										UseCSParent;
	CheckBox^										DestroyOnLastTabClose;


public:	
	OptionsDialog();

	ColorDialog^  									FCDialog;
	ColorDialog^  									BCDialog;
	ColorDialog^  									HCDialog;
	ColorDialog^  									BMCDialog;

	void											LoadINI();
	void											SaveINI();
	void											PopulateINIMap();

	BoundControl^									FetchSetting(String^ Key);
	int												FetchSettingAsInt(String^ Key);
	String^											FetchSettingAsString(String^ Key);

	void											Show() { OptionsBox->ShowDialog(); }

	static OptionsDialog^%							GetSingleton();
};

#define OPTIONS										OptionsDialog::GetSingleton()