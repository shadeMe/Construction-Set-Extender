#pragma once
#include "Common\Includes.h"


public ref class ScriptListDialog
{
	int													LastSortColumn;
public:
	static enum class									Operation
															{
																e_Open = 0,
																e_Delete
															};

	static enum class									Icon
															{
																e_Deleted = 0,
																e_Active
															};
private:
	void												ScriptList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
	void												ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void												ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
	void												ScriptList_ItemActivate(Object^ Sender, EventArgs^ E);
	void												ScriptList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);

	void												ScriptBox_Cancel(Object^ Sender, CancelEventArgs^ E);
	void												SelectBox_Click(Object^ Sender, EventArgs^ E);

	void												SearchBox_TextChanged(Object^ Sender, EventArgs^ E);
	void												SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

	void												GetUseReport();
	void												SelectScript();
	void												Close();


	UInt32												ParentIndex;
	Operation											CurrentOp;

	static ImageList^									FlagIcons = gcnew ImageList();


	Form^												ScriptBox;								
	TextBox^											PreviewBox;
	ListView^											ScriptList;
		ColumnHeader^										ScriptListCFlags;
		ColumnHeader^										ScriptListCScriptName;
		ColumnHeader^										ScriptListCFormID;
		ColumnHeader^										ScriptListCScriptType;
	TextBox^											SearchBox;
	Button^												SelectBox;
public:
	ScriptListDialog(UInt32 AllocatedIndex);

	void												Show(Operation Op);
	void												AddScript(String^% ScriptName, String^% FormID, String^% ScriptType, UInt32 Flags);
	void												Destroy() { ScriptBox->Close(); }
};
