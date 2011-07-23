#include "TESIdleForm.h"

void**								g_IdleFormTree = (void**)0x00A107F8;

void TESIdleForm::InitializeIdleFormTreeRootNodes()
{
	thisCall<UInt32>(0x004D4490, *g_IdleFormTree);
}