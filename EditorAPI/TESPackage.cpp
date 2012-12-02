#include "TESPackage.h"

HWND*		TESPackage::WindowHandle = (HWND*)0x00A0AFD8;

void TESPackage::InitializeListViewColumns( HWND ListView )
{
	cdeclCall<void>(0x004529D0, ListView);
}

