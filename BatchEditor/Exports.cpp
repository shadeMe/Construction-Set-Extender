#include "Exports.h"
#include "BatchEditor.h"

extern "C"{

__declspec(dllexport) bool InitializeRefBatchEditor(BatchRefData* Data)
{
	return REFBE->InitializeBatchEditor(Data);
}

__declspec(dllexport) void AddFormListItem(FormData* Data, UInt8 ListID)
{
	REFBE->AddToFormList(Data, ListID);
}

}