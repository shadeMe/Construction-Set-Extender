#include "TagBrowser.h"

using namespace ComponentDLLInterface;
using namespace ConstructionSetExtender;

extern ComponentDLLInterface::TagBrowserInterface g_InteropInterface;

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

void ShowTagBrowserDialog(HWND Parent)
{
	TAGBRWR->Show(IntPtr(Parent));
}

void HideTagBrowserDialog(void)
{
	TAGBRWR->Hide();
}

bool AddFormToActiveTag(ComponentDLLInterface::FormData* Data)
{
	return TAGBRWR->AddItemToActiveTag(Data);
}

HWND GetFormDropWindowHandle()
{
	return (HWND)TAGBRWR->GetFormListHandle();
}

HWND GetFormDropParentHandle()
{
	return (HWND)TAGBRWR->GetWindowHandle();
}

bool GetDragOpInProgress()
{
	return TAGBRWR->GetDragInProgress();
}

ComponentDLLInterface::TagBrowserInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	InitializeComponents,
	ShowTagBrowserDialog,
	HideTagBrowserDialog,
	AddFormToActiveTag,
	GetFormDropWindowHandle,
	GetFormDropParentHandle,
	GetDragOpInProgress
};