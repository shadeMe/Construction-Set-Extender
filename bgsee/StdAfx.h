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

// Headers for Windows 7+
#define WINVER			0x0601
#define _WIN32_WINNT	0x0601
#define DIRECTINPUT_VERSION 0x0800
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
#include <uxtheme.h>
#include <Objbase.h>
#include <Psapi.h>
#include <ShellAPI.h>

// .NET INTEROP
#include <MSCorEE.h>
#include <MetaHost.h>

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
#include <unordered_map>
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
#include <boost\range\adaptor\reversed.hpp>

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

using namespace SME::Functors;
using namespace SME::INI;

#define SHOW_LEAKAGE_MESSAGE(title)					\
		MessageBox(nullptr, "\"Security\" is not a dirty word; perhaps \"Crevasse\" is. But \"Leak\" is a positively disgusting word.\n\nDetails in the log.", \
		title, MB_OK|MB_ICONWARNING|MB_SETFOREGROUND|MB_TASKMODAL)