#include "TESObjectREFR.h"
#include "..\Hooks\Renderer.h"

void TESObjectREFR::Update3D()
{
	Hooks::kTESObjectREFRUpdate3D.WriteJump();

	Hooks::g_TESObjectREFRUpdate3DBuffer = this;
	thisVirtualCall(kVTBL_TESObjectREFR, 0x11C, this, NULL);		// TESObjectREFR::DeinitializeDialog
	Hooks::g_TESObjectREFRUpdate3DBuffer = this;

	Hooks::kTESObjectREFRUpdate3D.WriteBuffer();
}