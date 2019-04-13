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

// Header Version - Win7+
#define WINVER			0x0601
#define _WIN32_WINNT	0x0601
#define DIRECTINPUT_VERSION 0x0800
#define DPSAPI_VERSION	1
#define NOMINMAX 1

// WIN32
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
#include <filesystem>
#include <stdexcept>
#include <exception>
#include <sstream>
#include <memory>
#include <iostream>
#include <complex>
#include <iomanip>
#include <numeric>
#include <functional>
#include <regex>
#include <unordered_map>
#include <unordered_set>

// RPC
#include <Rpc.h>

// DIRECTX
#include <d3d9.h>
#include <d3d9types.h>
#include <d3dx9.h>
#include <d3dx9tex.h>
#include <dinput.h>

// BOOST
//#include <boost\scoped_ptr.hpp>
//#include <boost\scoped_array.hpp>
//#include <boost\shared_ptr.hpp>
//#include <boost\shared_array.hpp>
//#include <boost\intrusive_ptr.hpp>
//#include <boost\weak_ptr.hpp>
//#include <boost\filesystem.hpp>
//#include <boost\algorithm\string.hpp>
//
//namespace bfs = std::filesystem;

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

using namespace SME;
using namespace SME::INI;
using namespace SME::Functors;
using namespace SME::MemoryHandler;

// OBSE
#include "obse_common\obse_version.h"
#include "obse\PluginAPI.h"
#include "obse\GameTypes.h"
#include "obse\Utilities.h"

// BGSEEBASE
#include <bgsee\Console.h>
#include <bgsee\UIManager.h>


// CSE
#include "Main.h"
#include "EditorAPI\TESEditorAPI.h"
#include "Settings.h"
#include "EventSources.h"
#include "EventSinks.h"
#include "Serialization.h"

#define PI						3.151592653589793

#undef SME_ASSERT
#define SME_ASSERT(_Expression) (void)( (!!(_Expression)) || (BGSEECONSOLE->LogAssertion("CSE", "ASSERTION FAILED (%s, %d): %s", __FILE__, __LINE__, #_Expression), _wassert(_CRT_WIDE(#_Expression), _CRT_WIDE(__FILE__), __LINE__), 0) )


// required for assertions in d'tors (for static instances) as we don't want it to trigger the crash handler recursively
#ifdef NDEBUG
#define DEBUG_ASSERT(_Expression)		(void)( (!!(_Expression)) || (BGSEECONSOLE->LogAssertion("CSE", "ASSERTION FAILED (%s, %d): %s", __FILE__, __LINE__, #_Expression), 0) )
#else
#define DEBUG_ASSERT(expr)		SME_ASSERT(expr)
#endif