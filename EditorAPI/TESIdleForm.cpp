#include "TESIdleForm.h"

void**							TESIdleForm::IdleFormTree = (void**)0x00A107F8;

void TESIdleForm::InitializeIdleFormTreeRootNodes()
{
	thisCall<UInt32>(0x004D4490, *IdleFormTree);
}

void TESIdleForm::ResetIdleFormTree()
{
	thisCall<void>(0x004D4540, *IdleFormTree);
}

