#include "Exports.h"
#include "UseInfoList.h"

using namespace ComponentDLLInterface;
using namespace ConstructionSetExtender;

extern ComponentDLLInterface::UseInfoListInterface g_InteropInterface;

extern "C"
{
	QUERYINTERFACE_EXPORT
	{
		return &g_InteropInterface;
	}
}

void InitializeComponents(void)
{
	return;
}

void ShowUseInfoListDialog(const char* FilterString)
{
	USELST->Open(FilterString);
}

ComponentDLLInterface::UseInfoListInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	InitializeComponents,
	ShowUseInfoListDialog
};