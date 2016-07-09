#include "TESObjectLAND.h"

HWND*		TESObjectLAND::WindowHandle = (HWND*)0x00A0AF54;

const TESObjectLAND::LandHeightLimit* TESObjectLAND::GetHeightLimits( LandHeightLimit* Out )
{
	SME_ASSERT(Out);

	return thisCall<const LandHeightLimit*>(0x00525F00, this, Out);
}

bool TESObjectLAND::ReleaseLandData()
{
	return thisCall<bool>(0x0052B780, this);
}

void TESObjectLAND::Refresh3D(bool ReloadTextures)
{
	thisCall<void>(0x005296B0, this, ReloadTextures, (UInt32)0);
}

NiNode* TESObjectLAND::GetQuadLandNode(int Quad)
{
	return thisCall<NiNode*>(0x00523720, this, Quad);
}
