#include "MiscUtilities.h"
#include "NativeWrapper.h"

CStringWrapper::CStringWrapper(System::String^% Source)
{
	P = Marshal::StringToHGlobalAnsi(Source);
	CString = static_cast<char*>(P.ToPointer());
}

void DebugDump(UInt8 Source, String^% Message)
{
	CStringWrapper^ CStr = gcnew CStringWrapper(Message);
	NativeWrapper::_D_PRINT(Source, CStr->String());
}

void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
{
	if (State)	*Flag |= Mask;
	else		*Flag &= ~Mask;
}

ImageResourceManager::ImageResourceManager(String^ BaseName)
{
	Manager = gcnew ResourceManager(BaseName, Assembly::GetExecutingAssembly());
}

Image^ ImageResourceManager::CreateImageFromResource(String^ ResourceIdentifier)
{
	try
	{
		return dynamic_cast<Image^>(Manager->GetObject(ResourceIdentifier));
	}
	catch (...)
	{
		return nullptr;
	}
}

void ImageResourceManager::SetupImageForToolStripButton(ToolStripButton^ Control)
{
	Control->Image = CreateImageFromResource(Control->Name);
}

namespace Log
{
	namespace ScriptEditor
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung) {
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_SE, Message);
		}
	}
	namespace UseInfoList
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung) {
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_UL, Message);
		}
	}
	namespace BatchEditor
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung) {
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_BE, Message);
		}
	}
	namespace BSAViewer
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung) {
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_BSA, Message);
		}
	}
	namespace TagBrowser
	{
		void DebugPrint(String^ Message, bool Achtung)
		{
			if (Achtung) {
				Media::SystemSounds::Hand->Play();
			}
			DebugDump(e_TAG, Message);
		}
	}
}