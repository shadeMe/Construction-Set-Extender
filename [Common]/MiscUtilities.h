#pragma once
#include "IncludesCLR.h"

namespace ConstructionSetExtender
{
	ref class CString
	{
		IntPtr												P;

		void												Free() { Marshal::FreeHGlobal(P); }
	public:
		CString(String^% Source);

		~CString();

		const char*											c_str() { return static_cast<char*>(P.ToPointer()); }
	};

	void													DebugDump(UInt8 Source, String^% Message);
	void													ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]

	ref class CSEGeneralException : public System::InvalidOperationException
	{
	public:
		CSEGeneralException(String^ Message) : System::InvalidOperationException(Message) {};
	};

	ref class ImageResourceManager
	{
		ResourceManager^					Manager;
	public:
		ImageResourceManager(String^ BaseName);

		Image^								CreateImageFromResource(String^ ResourceIdentifier);
		void								SetupImageForToolStripButton(ToolStripButton^ Control);
	};

	ref class CSEControlDisposer
	{
	public:
		CSEControlDisposer(Control^ Source);
		CSEControlDisposer(Control::ControlCollection^ Source);
	};

	ref class WindowHandleWrapper : public IWin32Window
	{
		IntPtr					_hwnd;
	public:
		WindowHandleWrapper(IntPtr Handle) : _hwnd(Handle) {}

		property IntPtr Handle
		{
			virtual IntPtr get()
			{
				return _hwnd;
			}
		};
	};

	void									DeleteManagedHeapPointer(void* Pointer, bool IsArray);
	void									CopyStringToCharBuffer(String^% Source, char* Buffer, UInt32 Size);

	namespace Log
	{
		enum MessageSource
		{
			e_CSE = 0,
			e_CS,
			e_BE,
			e_UL,
			e_SE,
			e_BSA,
			e_TAG
		};

		namespace ScriptEditor
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
		namespace UseInfoList
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
		namespace BatchEditor
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
		namespace BSAViewer
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
		namespace TagBrowser
		{
			void DebugPrint(String^ Message, bool Achtung = false);
		}
	}
}