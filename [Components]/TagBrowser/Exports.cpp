#include "Exports.h"
#include "TagBrowser.h"

extern "C"
{
	__declspec(dllexport) void Show(HWND Handle)
	{
		try
		{
			System::Threading::Thread::CurrentThread->SetApartmentState(System::Threading::ApartmentState::STA);
		}
		catch (Exception^ E)
		{
			DebugPrint("Couldn't set thread apartment state to STA\n\tException: " + E->Message);
		}

		TAGBRWR->Show(IntPtr(Handle));
	}
	__declspec(dllexport) void Hide(void)
	{
		TAGBRWR->Hide();
	}
	__declspec(dllexport) bool AddFormToActiveTag(FormData* Data)
	{
		return TAGBRWR->AddItemToActiveTag(Data);
	}

	__declspec(dllexport) HWND GetFormDropWindowHandle()
	{
		return (HWND)TAGBRWR->GetFormListHandle();
	}

	__declspec(dllexport) HWND GetFormDropParentHandle()
	{
		return (HWND)TAGBRWR->GetWindowHandle();
	}
}