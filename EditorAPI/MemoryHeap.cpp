#include "MemoryHeap.h"

MemoryHeap*				MemoryHeap::FormHeap = (MemoryHeap*)0x00A09E90;

void* FormHeap_Allocate( UInt32 Size )
{
	return cdeclCall<void*>(0x00401E80, Size);
}

void FormHeap_Free( void* Ptr )
{
	cdeclCall<void>(0x00401EA0, Ptr);
}