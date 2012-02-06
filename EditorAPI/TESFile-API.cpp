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