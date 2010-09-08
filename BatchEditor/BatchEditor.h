#pragma once

#include "Common\Includes.h"
#include "Common\NativeWrapper.h"

public ref class BatchEditor
{
protected:
	Form^								BatchEditBox;
		Button^								ApplyButton;
		Button^								CancelButton;

	virtual void 						ApplyButton_Click(Object^ Sender, EventArgs^ E);
	virtual void 						CancelButton_Click(Object^ Sender, EventArgs^ E);

	virtual void						Cleanup() = 0;
	virtual bool						ScrubData() = 0;
	virtual void						Close(DialogResult Result);
};

public ref class RefBatchEditor : public BatchEditor
{
protected:
	static RefBatchEditor^				Singleton = nullptr;

	RefBatchEditor();

	ListView^						ObjectList;
		ColumnHeader^					ObjectList_CHEditorID;
		ColumnHeader^					ObjectList_CHFormID;
		ColumnHeader^					ObjectList_CHType;

	TabControl^						DataBox;
		TabPage^						DataBox_TB3DData;
			GroupBox^						PositionBox;
				Label^							LabelZ;
				Label^							LabelY;
				Label^							LabelX;
				TextBox^						PosZ;
				TextBox^						PosY;
				TextBox^						PosX;

				CheckBox^						UsePosX;
				CheckBox^						UsePosZ;
				CheckBox^						UsePosY;
			GroupBox^						RotationBox;
				Label^							LabelZEx;
				TextBox^						RotX;
				Label^							LabelYEx;
				TextBox^						RotY;
				Label^							LabelXEx;
				TextBox^						RotZ;

				CheckBox^						UseRotX;
				CheckBox^						UseRotZ;
				CheckBox^						UseRotY;

			Label^							LabelScale;
			TextBox^						Scale;
			CheckBox^						UseScale;
		TabPage^						DataBox_TBFlags;
			CheckBox^						FlagVWD;
			CheckBox^						FlagDisabled;
			CheckBox^						FlagPersistent;

			CheckBox^						UsePersistent;
			CheckBox^						UseDisabled;
			CheckBox^						UseVWD;
		TabPage^						DataBox_TBEnableParent;
			CheckBox^						UseEnableParent;

			Button^							SetParent;
			CheckBox^						OppositeState;
		TabPage^						DataBox_TBOwnership;
			CheckBox^						UseOwnership;

			Label^							LabelFaction;
			Label^							LabelNPC;
			ComboBox^						NPCList;
			Label^							LabelGlobal;
			ComboBox^						FactionList;
			Label^							LabelRank;
			ComboBox^						GlobalList;
			TextBox^						Rank;
			RadioButton^					UseNPCOwner;
			RadioButton^					UseFactionOwner;
		TabPage^						DataBox_TBExtra;
			Label^							LabelCount;
			TextBox^						Count;
			Label^							LabelHealth;
			TextBox^						Health;
			Label^							LabelCharge;
			TextBox^						Charge;
			Label^							LabelTimeLeft;
			TextBox^						TimeLeft;
			ComboBox^						SoulLevel;
			Label^							LabelSoul;

			CheckBox^						UseCount;
			CheckBox^						UseHealth;
			CheckBox^						UseTimeLeft;
			CheckBox^						UseSoulLevel;
			CheckBox^						UseCharge;


	BatchRefData*						BatchData;

	void								ObjectList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
	void								SetParent_Click(Object^ Sender, EventArgs^ E);

	virtual void						Cleanup() override;
	void								PopulateObjectList(BatchRefData* Data);
	void								PopulateFormLists();
	virtual bool						ScrubData() override;
	
	float								GetFloatFromString(String^ Text);
	int									GetIntFromString(String^ Text);
	UInt32								GetUIntFromString(String^ Text);
	void								SanitizeControls(Control^ Container);
public:
	static RefBatchEditor^%				GetSingleton();

	bool								InitializeBatchEditor(BatchRefData* Data);
	ComboBox^%							GetFormList(UInt8 ListID);
	void								AddToFormList(FormData* Data, UInt8 ListID);
};

#define REFBE							RefBatchEditor::GetSingleton()
