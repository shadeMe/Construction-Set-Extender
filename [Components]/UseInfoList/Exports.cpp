#include "[Common]\ComponentDLLInterface.h"
#include "UseInfoList.h"

using namespace ComponentDLLInterface;
using namespace ConstructionSetExtender;

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
	USELST->Open(FilterString);
}

ComponentDLLInterface::UseInfoListInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	ShowUseInfoListDialog
};