#include "Exports.h"
#include "UseInfoList.h"

extern "C"{

__declspec(dllexport) void OpenUseInfoBox(const char* InitForm)
{
	USELST->Open(InitForm);
}

__declspec(dllexport) void SetFormListItemData(FormData* Data)
{
	if (!Data->IsValid())		return;
	USELST->AddFormListItem(gcnew String(Data->EditorID), Data->FormID.ToString("X8"), Data->TypeID);
}

__declspec(dllexport) void SetUseListObjectItemData(FormData* Data)
{
	if (!Data->IsValid())		return;
	USELST->AddObjectListItem(gcnew String(Data->EditorID), Data->FormID.ToString("X8"), Data->TypeID);
}

__declspec(dllexport) void SetUseListCellItemData(UseListCellItemData* Data)
{
	if (!Data->IsValid())		return;
	USELST->AddCellListItem(gcnew String(Data->RefEditorID), gcnew String(Data->WorldEditorID), Data->FormID.ToString("X8"), gcnew String(Data->EditorID), ((!Data->Flags)?String::Format("{0}, {1}", Data->XCoord, Data->YCoord):"Interior"), Data->UseCount);
}

}