#pragma once

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include "windows.h"
#include "windowsx.h"
#include "atltypes.h"
#include "commctrl.h"
#include "richedit.h"
#include "shlobj.h"
#include "resource.h"
#include "common/IDirectoryIterator.h"
#include "[Libraries]\MemoryHandler\MemoryHandler.h"

using namespace SME;
struct BaseExtraList;

#define CS_CAST(obj, from, to)								(to *)Oblivion_DynamicCast((void*)(obj), 0, RTTI_ ## from, RTTI_ ## to, 0)

void DebugPrint(const char* fmt, ...);
void DebugPrint(UInt8 source, const char* fmt, ...);
void CSEDumpClass(void * theClassPtr, UInt32 nIntsToDump = 512);
void WaitUntilDebuggerAttached();
void __stdcall ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]
void LogWinAPIErrorMessage(DWORD ErrorID);
void DumpExtraDataList(BaseExtraList* List);
