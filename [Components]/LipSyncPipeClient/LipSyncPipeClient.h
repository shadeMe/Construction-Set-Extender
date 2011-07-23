#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void LipSyncPipeClient_Initialize();

#ifdef __cplusplus
}
#endif

struct CSECSInteropData;
typedef unsigned long DWORD;

void MainWindowMessageLoopHook(void);
void LogOC3AnimFactoryMessagesHook(void);

void __stdcall HandleDebugText(const char* Message);

void DebugPrint(const char* fmt, ...);
void LogWinAPIErrorMessage(DWORD ErrorID);