#include "BSStringT.h"
#include "Core.h"

bool BSStringT::Set(const char* string, SInt16 size)   // size determines allocated storage? 0 to allocate automatically
{
	return thisCall<bool>(0x004051E0, this, string, size);
}

void BSStringT::Clear()
{
	thisCall<UInt32>(0x004053D0, this);
}

SInt16 BSStringT::Size() const
{
	return thisCall<SInt16>(0x0040BAD0, this);
}
SInt16 BSStringT::Compare(const char* string, bool ignoreCase)
{
	if (ignoreCase)
		return _stricmp(m_data, string);
	else
		return strcmp(m_data, string);
}

BSStringT* BSStringT::CreateInstance(const char* String)
{
	BSStringT* NewInstance = (BSStringT*)FormHeap_Allocate(sizeof(BSStringT));
	thisCall<UInt32>(0x00497900, NewInstance, String);
	return NewInstance;
}

void BSStringT::DeleteInstance(bool ReleaseMemory)
{
	Clear();
	if (ReleaseMemory)
		FormHeap_Free(this);
}