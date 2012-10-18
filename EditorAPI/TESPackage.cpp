#include "TESPackage.h"

void TESPackage::InitializeListViewColumns( HWND ListView )
{
	cdeclCall<void>(0x004529D0, ListView);
}
