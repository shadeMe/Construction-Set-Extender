#include "TESObjectLAND.h"

HWND*		TESObjectLAND::WindowHandle = (HWND*)0x00A0AF54;

const TESObjectLAND::HeightLimitData* TESObjectLAND::GetHeightLimits( HeightLimitData* Out )
{
	SME_ASSERT(Out);

	return thisCall<const HeightLimitData*>(0x00525F00, this, Out);
}
