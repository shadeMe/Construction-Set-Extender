#pragma once

#include "Common\Includes.h"
#include "Common\NativeWrapper.h"

public ref class RefBatchEditor
{
	static RefBatchEditor^				Singleton = nullptr;

	RefBatchEditor();
	Form^								BatchEditBox;
		ListView^							ObjectList;
			ColumnHeader^						ObjectList_CHEditorID;
			ColumnHeader^						ObjectList_CHFormID;
			ColumnHeader^						ObjectList_CHType;
		Button^								ApplyButton;
		Button^								CancelButton;
		TabControl^							DataBox;
			TabPage^						DataBox_TB3DData;
				CheckBox^						Use3DData;
				GroupBox^						PositionBox;
					Label^							LabelZ;
					Label^							LabelY;
					Label^							LabelX;
					TextBox^						PosZ;
					TextBox^						PosY;
					TextBox^						PosX;
				GroupBox^						RotationBox;
					Label^							LabelZEx;
					Label^							LabelXEx;
					Label^							LabelYEx;
					TextBox^						RotX;
					TextBox^						RotY;
					TextBox^						RotZ;			
				Label^							LabelScale;
				TextBox^						Scale;	
			TabPage^						DataBox_TBFlags;
				CheckBox^						UseFlags;
				CheckBox^						FlagVWD;
				CheckBox^						FlagDisabled;
				CheckBox^						FlagPersistent;
			TabPage^						DataBox_TBEnableParent;
			TabPage^						DataBox_TBOwnership;

	BatchRefData*						BatchData;

	void 								ApplyButton_Click(Object^ Sender, EventArgs^ E);
	void 								CancelButton_Click(Object^ Sender, EventArgs^ E);
	void								ObjectList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);


	void								Close(DialogResult Result);
	void								Cleanup();
	void								PopulateObjectList(BatchRefData* Data);
	bool								ScrubData(BatchRefData* Data);
public:
	static RefBatchEditor^%				GetSingleton();

	bool								InitializeBatchEditor(BatchRefData* Data);
};

#define REFBE							RefBatchEditor::GetSingleton()
