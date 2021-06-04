#pragma once
#include "IncludesCLR.h"
#include "ComponentDLLInterface.h"

namespace cse
{
	namespace nativeWrapper
	{
		extern componentDLLInterface::CSEInterfaceTable* g_CSEInterfaceTable;

		[DllImport("Construction Set Extender.dll")]
		void*											QueryInterface(void);

		[DllImport("user32.dll")]
		bool											LockWindowUpdate(IntPtr hWndLock);
		[DllImport("user32.dll")]
		IntPtr											WindowFromPoint(Point Point);
		[DllImport("user32.dll")]
		IntPtr											GetParent(IntPtr Handle);
		[DllImport("user32.dll")]
		bool											SetWindowPos(IntPtr hWnd, int hWndInsertAfter, int X, int Y, int cx, int cy, UInt32 uFlags);
		[DllImport("user32.dll")]
		bool											ShowWindow(IntPtr hWnd, int nCmdShow);
		[DllImport("user32.dll")]
		IntPtr											SendMessageA(IntPtr hWnd, int Msg, IntPtr wParam, IntPtr lParam);
		[DllImport("user32.dll")]
		IntPtr											SetActiveWindow(IntPtr handle);
		[DllImport("user32.dll")]
		bool											BringWindowToTop(IntPtr handle);
		[DllImport("user32.dll")]
		IntPtr											GetActiveWindow();
		[DllImport("user32.dll")]
		IntPtr											GetForegroundWindow();
		[DllImport("user32.dll")]
		IntPtr											GetFocus();
		[DllImport("user32.dll")]
		IntPtr											SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);
		[DllImport("user32.dll")]
		int												GetWindowLong(IntPtr hWnd, int nIndex);
		[DllImport("gdi32.dll")]
		int												DeleteObject(IntPtr hObject);

		void											WriteToMainWindowStatusBar(int PanelIndex, String^ Message);
		void											ShowNonActivatingWindow(Control^ Window, IntPtr ParentHandle);
		void											PrintToConsole(UInt8 Source, String^% Message);

		void Initialize();
	};

	template <typename T>
	class DisposibleDataAutoPtr
	{
		T* Data;
	public:
		DisposibleDataAutoPtr(T* Data = nullptr) : Data(Data) {}
		~DisposibleDataAutoPtr() { reset(); }

		T& operator*() { return *Data; }
		T* operator->() { return Data; }
		T* get() { return Data; }
		operator bool() { return Data != nullptr; }
		void reset(T* NewData = nullptr)
		{
			if (Data)
				nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(Data, false);

			Data = NewData;
		}
		T* release()
		{
			auto Temp = Data;
			Data = nullptr;
			return Temp;
		}
	};

	namespace log
	{
		enum MessageSource
		{
			e_CSE = 0,
			e_CS,
			e_UL,
			e_SE,
			e_BSA,
			e_TAG
		};

		namespace scriptEditor
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
		namespace useInfoList
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
		namespace bsaViewer
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
		namespace tagBrowser
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
	}
}