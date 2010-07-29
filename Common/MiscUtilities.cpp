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
	NativeWrapper::_D_PRINT(CStr->String());
}