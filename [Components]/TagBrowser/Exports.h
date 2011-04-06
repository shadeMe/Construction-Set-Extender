#pragma once

#include "[Common]\HandshakeStructs.h"

typedef void* HWND;

extern "C"
{

__declspec(dllexport) void Show(HWND Handle);
__declspec(dllexport) void Hide(void);
__declspec(dllexport) void AddFormToActiveTag(FormData* Data);
__declspec(dllexport) HWND GetFormDropWindowHandle(void);
__declspec(dllexport) HWND GetFormDropParentHandle(void);

}