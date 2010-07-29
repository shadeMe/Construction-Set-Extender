#pragma once
#include "Common\Includes.h"
#include "IntelliSense.h"
#include "Common\HandShakeStructs.h"

extern "C"{

__declspec(dllexport) void InitializeComponents(CommandTableData* Data);
__declspec(dllexport) void AddToURLMap(const char* CmdName, const char* URL);

__declspec(dllexport) void AllocateNewEditor(UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);
__declspec(dllexport) void InitializeScript(UInt32 VanillaHandleIndex, ScriptData* Data);
__declspec(dllexport) void SendMessagePingback(UInt32 VanillaHandleIndex, UInt16 Message);

__declspec(dllexport) bool IsActivePluginScriptRecord(const char* EditorID);
__declspec(dllexport) void SetScriptListItemData(UInt32 VanillaHandleIndex, ScriptData* Data);

__declspec(dllexport) void SetVariableListItemData(UInt32 VanillaHandleIndex, ScriptVarIndexData::ScriptVarInfo* Data);

}
