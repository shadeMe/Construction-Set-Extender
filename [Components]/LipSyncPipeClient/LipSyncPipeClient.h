#pragma once
#include "[Libraries]\MemoryHandler\MemoryHandler.h"

using namespace SME::MemoryHandler;

_DeclareMemHdlr(Crt0EntryPointInitialization, "prevents the CS windows from showing");
_DeclareNopHdlr(InitializeWindows, "");
_DeclareMemHdlr(ShowSplashScreenWindow, "");
_DeclareMemHdlr(MainWindowMessageLoop, "patches the message pump to add the pipe client code");
_DeclareMemHdlr(MessageHandlerDebugPrint, "redirects all warning messages to the server");
_DeclareMemHdlr(LogOC3AnimFactoryMessagesA, "handles any output from the OC3 code");
_DeclareNopHdlr(LogOC3AnimFactoryMessagesB, "");
_DeclareMemHdlr(AllowMultipleEditors, "allows multiple editor instances to be created regardless of the INI setting");

void __stdcall HandleDebugText(const char* Message);
void DebugPrint(const char* fmt, ...);
void LogWinAPIErrorMessage(DWORD ErrorID);
bool GenerateLIPFile(char* FilePath, char* ResponseText);