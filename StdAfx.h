// Precompiled header
//	>> Include headers with stable code
//	!! Recompile header after modifications
//
#pragma once
// 4018 - signed/unsigned mismatch
// 4244 - loss of data by assignment
// 4267 - possible loss of data (truncation)
// 4305 - truncation by assignment
// 4288 - disable warning for crap microsoft extension screwing up the scope of variables defined in for loops
// 4311 - pointer truncation
// 4312 - pointer extension
#pragma warning(disable: 4018 4244 4267 4305 4288 4312 4311 4800 4005)

// WIN32
#define _WIN32_WINNT	0x0500
#include <winsock2.h>
#include <windows.h>
#include <windowsx.h>
#include <atltypes.h>
#include <commctrl.h>
#include <richedit.h>
#include <shlobj.h>
#include <Rpc.h>
#include <Dbghelp.h>
#include <uxtheme.h>

// CRT
#include <time.h>
#include <intrin.h>
#include <errno.h>
#include <assert.h>

// STL
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <cstdarg>
#include <cmath>
#include <cwchar>
#include <algorithm>
#include <list>
#include <map>
#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <string>
#include <fstream>
#include <stdexcept>
#include <exception>
#include <sstream>
#include <memory>
#include <iostream>
#include <complex>
#include <iomanip>
#include <numeric>
#include <functional>

// DIRECTX
#include <D3dx9tex.h>

// OBSE
#include "obse_common/obse_version.h"

#include "common/ITypes.h"
#include "common/IErrors.h"
#include "common/IDynamicCreate.h"
#include "common/IDebugLog.h"
#include "common/ISingleton.h"
#include "common/IDirectoryIterator.h"
#include "common/IFileStream.h"

#include "obse\PluginAPI.h"
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

// SME
#include "[Libraries]\MemoryHandler\MemoryHandler.h"
#include "[Libraries]\INI Manager\INIManager.h"
#include "[Libraries]\INI Manager\INIEditGUI.h"

using namespace SME;
using namespace MemoryHandler;

// CSE
#include "Console.h"
#include "[Common]\CLIWrapper.h"

extern std::string									g_APPPath;
extern std::string									g_INIPath;
extern std::string									g_DLLPath;
extern char											g_TextBuffer[0x800];
extern bool											g_PluginPostLoad;

extern OBSEMessagingInterface*						g_msgIntfc;
extern PluginHandle									g_pluginHandle;
extern HINSTANCE									g_DLLInstance;
extern SME::INI::INIManager*						g_INIManager;
extern SME::INI::INIEditGUI*						g_INIEditGUI;
extern OBSECommandTableInterface*					g_commandTableIntfc;
extern ComponentDLLInterface::CommandTableData		g_CommandTableData;

class CSEINIManager : public INI::INIManager
{
public:
	void							Initialize();
};

void				WaitUntilDebuggerAttached();
const char*			PrintToBuffer(const char* fmt, ...);

void				ToggleFlag(UInt8* Flag, UInt32 Mask, bool State);
void				ToggleFlag(UInt16* Flag, UInt32 Mask, bool State);
void				ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);