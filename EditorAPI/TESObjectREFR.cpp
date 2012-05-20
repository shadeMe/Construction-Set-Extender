#include "TESObjectREFR.h"
#include "..\Hooks\Renderer.h"

using namespace ConstructionSetExtender;

void TESObjectREFR::UpdateNiNode()
{
	Hooks::_MemHdlr(TESObjectREFRUpdate3D).WriteJump();

	Hooks::g_TESObjectREFRUpdate3DBuffer = this;
	thisVirtualCall<UInt32>(0x11C, this, NULL);		// TESObjectREFR::DeinitializeDialog
	Hooks::g_TESObjectREFRUpdate3DBuffer = this;

	Hooks::_MemHdlr(TESObjectREFRUpdate3D).WriteBuffer();
}

bool TESObjectREFR::SetBaseForm(TESForm* BaseForm)
{
	return thisCall<bool>(0x005415A0, this, BaseForm);
}

void TESObjectREFR::SetPersistent(bool Persistent)
{
	thisCall<UInt32>(0x0053F0D0, this, Persistent);
}

void TESObjectREFR::ModExtraHealth( float Health )
{
	thisCall<UInt32>(0x0053F4E0, this, Health);
}

void TESObjectREFR::ModExtraCharge( float Charge )
{
	thisCall<UInt32>(0x0053F3C0, this, Charge);
}

void TESObjectREFR::ModExtraTimeLeft( float Time )
{
	thisCall<UInt32>(0x0053F620, this, Time);
}

void TESObjectREFR::ModExtraSoul( UInt8 SoulLevel )
{
	thisCall<UInt32>(0x0053F710, this, SoulLevel);
}

void TESObjectREFR::SetExtraEnableStateParentOppositeState( bool State )
{
	thisCall<UInt32>(0x0053FA80, this, State);
}

NiNode* TESObjectREFR::GetExtraRef3DData( void )
{
	return thisCall<NiNode*>(0x00542950, this);
}

void TESObjectREFR::RemoveExtraTeleport( void )
{
	thisCall<UInt32>(0x0053F7A0, this);
}

NiNode* TESObjectREFR::GenerateNiNode()
{
	return thisVirtualCall<NiNode*>(0x178, this);
}

void TESObjectREFR::SetScale( float Scale )
{
	thisCall<void>(0x00542420, this, Scale);
}
