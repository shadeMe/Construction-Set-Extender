#pragma once
#include "[Common]\HandShakeStructs.h"
#include "[Common]\AuxiliaryWindowsForm.h"

public ref class ScriptListDialog
{
	int													LastSortColumn;
public:
	static enum class									Operation
															{
																e_Open = 0,
																e_Delete
															};

	static enum class									FlagIcons
															{
																e_Deleted = 0,
																e_Active
															};
private:
	static Size											LastKnownSize = Size(0, 0);

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
	ComponentDLLInterface::ScriptListData*				ScriptListCache;
	ComponentDLLInterface::ScriptData*					FirstSelectionCache;

	static ImageList^									ScriptFlagIcons = gcnew ImageList();

	AnimatedForm^										ScriptBox;
	TextBox^											PreviewBox;
	ListView^											ScriptList;
		ColumnHeader^										ScriptListCFlags;
		ColumnHeader^										ScriptListCScriptName;
		ColumnHeader^										ScriptListCFormID;
		ColumnHeader^										ScriptListCScriptType;
	TextBox^											SearchBox;
	Button^												SelectBox;

	bool												Closing;

	void												Destroy();
public:
	ScriptListDialog(UInt32 ParentIndex);
	~ScriptListDialog()
	{
		Destroy();
	}

	ComponentDLLInterface::ScriptData*					Show(Operation Op, String^ FilterString);
};