#include "TESPackage.h"

HWND*		TESPackage::WindowHandle = (HWND*)0x00A0AFD8;

void TESPackage::ListViewGetDispInfoCallback(NMLVDISPINFO* Data)
{
	thisCall<void>(0x004519D0, this, Data);
}

void TESPackage::InitializeListViewColumns( HWND ListView )
{
	cdeclCall<void>(0x004529D0, ListView);
}

