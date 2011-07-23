#include "Exports.h"
#include "BSAViewer.h"

extern "C"
{
	__declspec(dllexport) const char* InitializeViewer(const char* AppPath, const char* Filter)
	{
		try
		{
			System::Threading::Thread::CurrentThread->SetApartmentState(System::Threading::ApartmentState::STA);
		}
		catch (Exception^ E)
		{
			DebugPrint("Couldn't set thread apartment state to STA\n\tException: " + E->Message);
		}

		return BSAV->InitializeViewer(gcnew String(AppPath), gcnew String(Filter));
	}
}