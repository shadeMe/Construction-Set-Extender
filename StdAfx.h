// Precompiled header
//	>> Include headers with stable code
//	!! Recompile header after modifications
//
#pragma once
// 4018 - signed/unsigned mismatch
// 4244 - loss of data by assignment
// 4267 - possible loss of data (truncation)
// 4305 - truncation by assignment
// 4288 - disable warning for crap Microsoft extension screwing up the scope of variables defined in for loops
// 4311 - pointer truncation
// 4312 - pointer extension
#pragma warning(disable: 4018 4244 4267 4305 4288 4312 4311 4800)

#define _WIN32_WINNT	0x0501
#define DPSAPI_VERSION	1

// WIN32
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
#include <Objbase.h>
#include <Psapi.h>
#include <Shellapi.h>

// .NET INTEROP
#include <MSCorEE.h>

// CRT
#include <time.h>
#include <intrin.h>
#include <errno.h>
#include <crtdefs.h>

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

// RPC
#include <Rpc.h>

// DIRECTX
#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#include <dinput.h>

// BOOST
#include <boost\scoped_ptr.hpp>
#include <boost\scoped_array.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\shared_array.hpp>
#include <boost\intrusive_ptr.hpp>
#include <boost\weak_ptr.hpp>

// xSE Common
#include <ITypes.h>
#include <IErrors.h>
#include <IDynamicCreate.h>
#include <ISingleton.h>
#include <IDirectoryIterator.h>
#include <IFileStream.h>

// SME
#include <MemoryHandler.h>
#include <INIManager.h>
#include <INIEditGUI.h>
#include <Functors.h>
#include <StringHelpers.h>
#include <UIHelpers.h>
#include <MiscGunk.h>
#include <MersenneTwister.h>

// OBSE
#include "obse_common/obse_version.h"
#include "obse\PluginAPI.h"
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

using namespace SME;
using namespace SME::INI;
using namespace SME::MemoryHandler;
using namespace SME::Functors;

// BGSEEBASE
#include <BGSEEConsole.h>
#include <BGSEEUIManager.h>

// CSE
#include "CSESettings.h"
#include "CSEUIManager.h"

#define PI					3.151592653589793
