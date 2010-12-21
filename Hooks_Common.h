#pragma once 

#include "UtilityBox.h"

extern FARPROC g_WindowHandleCallAddr;

using namespace	MemoryHandler;

void __stdcall GetWindowTextAddress(void);
void __stdcall CreateDialogParamAddress(void);
void __stdcall EndDialogAddress(void);
void __stdcall EnableWindowAddress(void);
void __stdcall GetWindowLongAddress(void);
void __stdcall CreateWindowExAddress(void);
void __stdcall SetWindowTextAddress(void);
void __stdcall SendDlgItemMessageAddress(void);
void __stdcall SendMessageAddress(void);
void __stdcall TrackPopupMenuAddress(void);
