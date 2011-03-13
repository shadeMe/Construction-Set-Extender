#include "Exports.h"
#include "TagBrowser.h"

extern "C"
{

__declspec(dllexport) void Show(HWND Handle)
{
	TAGBRWR->Show(IntPtr(Handle));
}
__declspec(dllexport) void Hide(void)
{
	TAGBRWR->Hide();
}
__declspec(dllexport) void AddFormToActiveTag(FormData* Data)
{
	TAGBRWR->AddItemToActiveTag(Data);
}

}