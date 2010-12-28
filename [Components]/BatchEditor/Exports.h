#pragma once

#include "[Common]\Includes.h"
#include "[Common]\HandshakeStructs.h"

extern "C"{

__declspec(dllexport) bool InitializeRefBatchEditor(BatchRefData* Data);
__declspec(dllexport) void AddFormListItem(FormData* Data, UInt8 ListID);

}