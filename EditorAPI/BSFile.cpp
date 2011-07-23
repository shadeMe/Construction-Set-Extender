#include "BSFile.h"
#include "Core.h"

void BSFile::Flush()
{
	thisCall<UInt32>(0x00467150, this);
}

UInt32 BSFile::GetFileSize()
{
	return thisVirtualCall<UInt32>(0x1C, this);
}

UInt32 BSFile::DirectRead(void* buffer, UInt32 size)
{
	return thisVirtualCall<UInt32>(0x38, this, buffer, size);
}

UInt32 BSFile::DirectWrite(const void* buffer, UInt32 size)
{
	return thisVirtualCall<UInt32>(0x3C, this, buffer, size);
}

BSFile* BSFile::CreateInstance(const char* fileName, UInt32 mode, UInt32 bufferSize, bool isTextFile)
{
	BSFile* NewInstance = (BSFile*)FormHeap_Allocate(sizeof(BSFile));
	thisCall<UInt32>(0x004676A0, NewInstance, fileName, mode, bufferSize, isTextFile);
	return NewInstance;
}

void BSFile::DeleteInstance(bool ReleaseMemory)
{
	thisVirtualCall<UInt32>(0x0, this, ReleaseMemory);
}