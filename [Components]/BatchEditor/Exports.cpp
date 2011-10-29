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
	try
	{
		System::Threading::Thread::CurrentThread->SetApartmentState(System::Threading::ApartmentState::STA);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't set thread apartment state to STA\n\tException: " + E->Message);
	}

	return REFBE->InitializeBatchEditor(Data);
}

ComponentDLLInterface::BatchEditorInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	ShowBatchRefEditorDialog
};