#include "MiscUtilities.h"
#include "NativeWrapper.h"

namespace ConstructionSetExtender
{
	namespace NativeWrapper
	{
		ComponentDLLInterface::CSEInterfaceTable*		g_CSEInterfaceTable = (ComponentDLLInterface::CSEInterfaceTable*)NativeWrapper::QueryInterface();

		void NativeWrapper::ShowNonActivatingWindow(Control^ Window, IntPtr ParentHandle)
		{
			const int SW_SHOWNOACTIVATE = 4;
			const int HWND_TOPMOST = -1;
			const UInt32 SWP_NOACTIVATE = 0x0010;

			ShowWindow(Window->Handle, SW_SHOWNOACTIVATE);
			if (ParentHandle != IntPtr::Zero)
				SetWindowPos(Window->Handle, ParentHandle.ToInt32(), Window->Left, Window->Top, Window->Width, Window->Height, SWP_NOACTIVATE);
			else
				SetWindowPos(Window->Handle, 0, Window->Left, Window->Top, Window->Width, Window->Height, SWP_NOACTIVATE);
		}

		void NativeWrapper::WriteToMainWindowStatusBar(int PanelIndex, String^ Message)
		{
			CString CStr(Message);
			g_CSEInterfaceTable->EditorAPI.WriteToStatusBar(PanelIndex, CStr.c_str());
		}
	}
}