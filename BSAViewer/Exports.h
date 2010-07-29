#pragma once

#include "Common\Includes.h"
#include "Common\HandshakeStructs.h"

extern "C"{

__declspec(dllexport) const char* InitializeViewer(const char* AppPath, const char* Filter);

}