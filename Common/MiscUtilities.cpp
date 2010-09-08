#include "MiscUtilities.h"
#include "NativeWrapper.h"

CStringWrapper::CStringWrapper(System::String^% Source)
{
	P = Marshal::StringToHGlobalAnsi(Source);
	CString = static_cast<char*>(P.ToPointer());
}

void DumpToLog(String^% Message)
{
	CStringWrapper^ CStr = gcnew CStringWrapper(Message);
	NativeWrapper::_D_PRINT_EXP(CStr->String());
}

void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
{
	if (State)	*Flag |= Mask;
	else		*Flag &= ~Mask;
}