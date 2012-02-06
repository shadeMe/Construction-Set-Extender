#include "BSString.h"
#include "Core.h"

bool BSString::Set(const char* string, SInt16 size)   // size determines allocated storage? 0 to allocate automatically
{
	return thisCall<bool>(0x004051E0, this, string, size);
}

void BSString::Clear()
{
	thisCall<UInt32>(0x004053D0, this);
}

SInt16 BSString::Size() const
{
	return thisCall<SInt16>(0x0040BAD0, this);
}
SInt16 BSString::Compare(const char* string, bool ignoreCase)
{
	if (ignoreCase)
		return _stricmp(m_data, string);
	else
		return strcmp(m_data, string);
}

BSString* BSString::CreateInstance(const char* String)
{
	BSString* NewInstance = (BSString*)FormHeap_Allocate(sizeof(BSString));
	thisCall<UInt32>(0x00497900, NewInstance, String);
	return NewInstance;
}

void BSString::DeleteInstance(bool ReleaseMemory)
{
	Clear();
	if (ReleaseMemory)
		FormHeap_Free(this);
}