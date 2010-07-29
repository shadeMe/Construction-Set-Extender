#pragma once
#include "Common\Includes.h"


// TODO: ++++++++++++++++++



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
	void												ScriptList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);
	void												ScriptList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
	void												ScriptList_MouseUp(Object^ Sender, MouseEventArgs^ E);

	void												ScriptBox_Cancel(Object^ Sender, CancelEventArgs^ E);

	void												SearchBox_TextChanged(Object^ Sender, EventArgs^ E);
	void												SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E);


	void												GetUseReport();
	void												OpenScript();
	void												Close();

	UInt32												ParentIndex;
	Operation											CurrentOp;
public:
	static ImageList^									FlagIcons = gcnew ImageList();


	Form^												ScriptBox;								
	TextBox^											PreviewBox;
	ListView^											ScriptList;
		ColumnHeader^										ScriptListCFlags;
		ColumnHeader^										ScriptListCScriptName;
		ColumnHeader^										ScriptListCFormID;
		ColumnHeader^										ScriptListCScriptType;
	TextBox^											SearchBox;
	Button^												FilterBox;

	ScriptListDialog(UInt32 AllocatedIndex);

	void												Show(Operation Op);
	void												AddScript(String^% ScriptName, String^% FormID, String^% ScriptType, UInt32 Flags);
};
