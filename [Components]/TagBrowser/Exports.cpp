#include "TagBrowser.h"

using namespace componentDLLInterface;
using namespace cse;

extern componentDLLInterface::TagBrowserInterface g_InteropInterface;

extern "C"
{
	QUERYINTERFACE_EXPORT
	{
		return &g_InteropInterface;
	}
}

void InitializeComponents(void)
{
	nativeWrapper::Initialize();
}

void ShowTagBrowserDialog(HWND Parent)
{
	TAGBRWR->Show(IntPtr(Parent));
}

void HideTagBrowserDialog(void)
{
	TAGBRWR->Hide();
}

bool AddFormToActiveTag(componentDLLInterface::FormData* Data)
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

componentDLLInterface::TagBrowserInterface g_InteropInterface =
{
	InitializeComponents,
	ShowTagBrowserDialog,
	HideTagBrowserDialog,
	AddFormToActiveTag,
	GetFormDropWindowHandle,
	GetFormDropParentHandle,
	GetDragOpInProgress
};