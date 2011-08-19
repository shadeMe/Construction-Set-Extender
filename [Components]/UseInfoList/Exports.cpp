#include "[Common]\ComponentDLLInterface.h"
#include "UseInfoList.h"

using namespace ComponentDLLInterface;

extern ComponentDLLInterface::UseInfoListInterface g_InteropInterface;

extern "C"
{
	__declspec(dllexport) void* QueryInterface(void)
	{
		return &g_InteropInterface;
	}
}

void ShowUseInfoListDialog(const char* FilterString)
{
	try
	{
		System::Threading::Thread::CurrentThread->SetApartmentState(System::Threading::ApartmentState::STA);
	}
	catch (Exception^ E)
	{
		DebugPrint("Couldn't set thread apartment state to STA\n\tException: " + E->Message);
	}

	USELST->Open(FilterString);
}

ComponentDLLInterface::UseInfoListInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	ShowUseInfoListDialog
};