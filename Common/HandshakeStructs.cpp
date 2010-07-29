#ifdef CSE_INTERFACE
#error user-only code used by interface
#endif

#include "ScriptEditor\Globals.h"
#include "HandshakeStructs.h"

void CommandTableData::DumpData()
{
	DebugPrint(String::Format("\tCommandTable Data: Start= 0x{0:X8} ; End = 0x{1:X8}; GetCommandReturnType = 0x{2:X8}", (UInt32)CommandTableStart, (UInt32)CommandTableEnd, (UInt32)GetCommandReturnType));
}