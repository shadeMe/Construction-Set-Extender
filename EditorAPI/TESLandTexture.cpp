#include "TESLandTexture.h"

bool TESLandTexture::LoadSourceTexture()
{
	return thisCall<bool>(0x005311C0, this);
}

void TESLandTexture::ReleaseSourceTexture()
{
	thisCall<void>(0x00531370, this);
}
