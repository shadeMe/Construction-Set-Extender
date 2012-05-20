#include "TESFile.h"

TESFile* TESFile::CreateInstance(const char* WorkingDirectory, const char* FileName,  UInt8 OpenMode)
{
	TESFile* NewInstance = (TESFile*)FormHeap_Allocate(sizeof(TESFile));
	thisCall<UInt32>(0x00489590, NewInstance, WorkingDirectory, FileName, OpenMode);
	return NewInstance;
}

void TESFile::DeleteInstance(bool ReleaseMemory)
{
	thisCall<UInt32>(0x00487E60, this);
	if (ReleaseMemory)
		FormHeap_Free(this);
}

bool TESFile::IsActive( void )
{
	return thisCall<bool>(0x00485BA0, this);
}

bool TESFile::SetLoaded( bool State )
{
	return thisCall<bool>(0x00485B70, this, State);
}

bool TESFile::SetActive( bool State )
{
	return thisCall<bool>(0x00485BB0, this, State);
}

bool TESFile::SetMaster( bool State )
{
	return thisCall<bool>(0x00485B10, this, State);
}