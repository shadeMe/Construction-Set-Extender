#pragma once

#include "Includes.h"

public ref class CStringWrapper
{
	IntPtr												P;
	const char*											CString;

	void												Free() { Marshal::FreeHGlobal(P); }
public:
	CStringWrapper(String^% Source);
	~CStringWrapper()									{ this->!CStringWrapper(); }
	!CStringWrapper()									{ this->Free(); }

	const char*											String() { return CString; }
};

void													DumpToLog(String^% Message);
void													ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]

public ref class CSEGeneralException : Exception
{
public:
	CSEGeneralException(String^ Message) : Exception(Message) {};
};