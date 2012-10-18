#pragma once
#include "../[Common]/CLIWrapper.h"
#include "../[Common]/HandshakeStructs.h"
#include "../CSEUIManager.h"
#include "../Construction Set Extender_Resource.h"
#include "../Exports.h"

class TESForm;
class TESObjectCELL;
class TESWorldSpace;
class INISetting;

using namespace	MemoryHandler;

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		extern FARPROC		g_TempIATProcBuffer;

		void __stdcall IATCacheGetWindowTextAddress(void);
		void __stdcall IATCacheCreateDialogParamAddress(void);
		void __stdcall IATCacheEndDialogAddress(void);
		void __stdcall IATCacheEnableWindowAddress(void);
		void __stdcall IATCacheGetWindowLongAddress(void);
		void __stdcall IATCacheCreateWindowExAddress(void);
		void __stdcall IATCacheSetWindowTextAddress(void);
		void __stdcall IATCacheSendDlgItemMessageAddress(void);
		void __stdcall IATCacheSendMessageAddress(void);
		void __stdcall IATCacheTrackPopupMenuAddress(void);
		void __stdcall IATCacheShowWindowAddress(void);
		void __stdcall IATCacheMessageBoxAddress(void);
		void __stdcall IATCacheGetClientRectAddress(void);
		void __stdcall IATCacheInterlockedDecrementAddress(void);

		UInt32 __stdcall IsControlKeyDown(void);

	#define MOV_LARGEFS0_EAX		__asm _emit 0x64 \
									__asm _emit 0x0A3 \
									__asm _emit 0 \
									__asm _emit 0 \
									__asm _emit 0 \
									__asm _emit 0

	#define MOV_LARGEFS0_ECX		__asm _emit 0x64 \
									__asm _emit 0x89 \
									__asm _emit 0x0D \
									__asm _emit 0 \
									__asm _emit 0 \
									__asm _emit 0 \
									__asm _emit 0
	}
}