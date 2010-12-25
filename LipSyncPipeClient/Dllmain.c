#include "LipSyncPipeClient.h"
#include <Windows.h>

BOOL WINAPI DllMain(
        HANDLE  hDllHandle,
        DWORD   dwReason,
        LPVOID  lpreserved
        )
{
	switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			LipSyncPipeClient_Initialize();
			break;
	};

	return TRUE;
}
