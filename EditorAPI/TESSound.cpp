#include "TESSound.h"

void TESSound::PlaySoundFile(const char* Path)
{
	cdeclCall<UInt32>(0x005047B0, Path);
}

void TESSound::DeinitializeSoundSampler()
{
	cdeclCall<UInt32>(0x00503F90, 1);
}