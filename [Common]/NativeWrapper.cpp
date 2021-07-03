#include "MiscUtilities.h"
#include "NativeWrapper.h"

namespace cse
{
	namespace nativeWrapper
	{
		componentDLLInterface::CSEInterfaceTable* g_CSEInterfaceTable = nullptr;

		void Initialize()
		{
			g_CSEInterfaceTable = (componentDLLInterface::CSEInterfaceTable*)nativeWrapper::QueryInterface();
			Debug::Assert(g_CSEInterfaceTable != nullptr);
		}

		void nativeWrapper::ShowNonActivatingWindow(Control^ Window, IntPtr ParentHandle)
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

		void nativeWrapper::WriteToMainWindowStatusBar(int PanelIndex, String^ Message)
		{
			CString CStr(Message);
			g_CSEInterfaceTable->EditorAPI.WriteToStatusBar(PanelIndex, CStr.c_str());
		}

		void PrintToConsole(UInt8 Source, String^% Message)
		{
			CString CStr(Message);
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.DebugPrint(Source, CStr.c_str());
		}

		void SetControlRedraw(Control^ Window, bool Enabled)
		{
			const int WM_SETREDRAW = 11;

			SendMessageA(Window->Handle, WM_SETREDRAW, static_cast<IntPtr>(Enabled), static_cast<IntPtr>(0));
		}

	}

	namespace log
	{
		namespace scriptEditor
		{
			void DebugPrint(String^ Message, bool Achtung)
			{
				if (Achtung)
					Media::SystemSounds::Hand->Play();

				nativeWrapper::PrintToConsole(e_SE, Message);
			}
		}
		namespace useInfoList
		{
			void DebugPrint(String^ Message, bool Achtung)
			{
				if (Achtung)
					Media::SystemSounds::Hand->Play();

				nativeWrapper::PrintToConsole(e_UL, Message);
			}
		}
		namespace bsaViewer
		{
			void DebugPrint(String^ Message, bool Achtung)
			{
				if (Achtung)
					Media::SystemSounds::Hand->Play();

				nativeWrapper::PrintToConsole(e_BSA, Message);
			}
		}
		namespace tagBrowser
		{
			void DebugPrint(String^ Message, bool Achtung)
			{
				if (Achtung)
					Media::SystemSounds::Hand->Play();

				nativeWrapper::PrintToConsole(e_TAG, Message);
			}
		}
	}
}