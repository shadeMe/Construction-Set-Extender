#pragma once
#include "../[Common]/CLIWrapper.h"
#include "../[Common]/HandshakeStructs.h"
#include "../WindowManager.h"
#include "../resource.h"
#include "../HallofFame.h"
#include "../CSInterop.h"
#include "../WindowManager.h"
#include "../Exports.h"

class TESForm;
class TESObjectCELL;
class TESWorldSpace;
class INISetting;

#pragma warning(disable:4005)

namespace Hooks
{
	extern FARPROC		g_TempIATProcBuffer;

	using namespace	MemoryHandler;

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
}