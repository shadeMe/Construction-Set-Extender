#pragma once

#include "Includes.h"

ref class CStringWrapper
{
	IntPtr												P;
	const char*											CString;

	void												Free() { Marshal::FreeHGlobal(P); }
public:
	CStringWrapper(String^% Source);
	~CStringWrapper()									{ this->!CStringWrapper(); }
	!CStringWrapper()									{ this->Free(); }

	const char*											String() { return CString; }
};

void													DebugDump(UInt8 Source, String^% Message);
void													ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]

ref class CSEGeneralException : public Exception
{
public:
	CSEGeneralException(String^ Message) : Exception(Message) {};
};

ref class ImageResourceManager
{
	ResourceManager^					Manager;
public:
	ImageResourceManager(String^ BaseName);

	Image^								CreateImageFromResource(String^ ResourceIdentifier);
	void								SetupImageForToolStripButton(ToolStripButton^ Control);
};

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