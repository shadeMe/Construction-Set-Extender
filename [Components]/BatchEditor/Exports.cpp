#include "Exports.h"
#include "BatchEditor.h"

extern "C"
{
	__declspec(dllexport) bool InitializeRefBatchEditor(BatchRefData* Data)
	{
		try
		{
			System::Threading::Thread::CurrentThread->SetApartmentState(System::Threading::ApartmentState::STA);
		}
		catch (Exception^ E)
		{
			DebugPrint("Couldn't set thread apartment state to STA\n\tException: " + E->Message);
		}

		return REFBE->InitializeBatchEditor(Data);
	}

	__declspec(dllexport) void AddFormListItem(FormData* Data, UInt8 ListID)
	{
		REFBE->AddToFormList(Data, ListID);
	}
}