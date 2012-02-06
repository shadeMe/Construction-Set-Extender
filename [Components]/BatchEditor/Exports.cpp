#include "[Common]\ComponentDLLInterface.h"
#include "BatchEditor.h"

using namespace ComponentDLLInterface;
using namespace ConstructionSetExtender;

extern ComponentDLLInterface::BatchEditorInterface g_InteropInterface;

extern "C"
{
	__declspec(dllexport) void* QueryInterface(void)
	{
		return &g_InteropInterface;
	}
}

bool ShowBatchRefEditorDialog(BatchRefData* Data)
{
	return REFBE->InitializeBatchEditor(Data);
}

ComponentDLLInterface::BatchEditorInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	ShowBatchRefEditorDialog
};