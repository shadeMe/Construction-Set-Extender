#include "[Common]\ComponentDLLInterface.h"
#include "BSAViewer.h"

extern ComponentDLLInterface::BSAViewerInterface g_InteropInterface;

using namespace ConstructionSetExtender;

extern "C"
{
	__declspec(dllexport) void* QueryInterface(void)
	{
		return &g_InteropInterface;
	}
}

void ShowBSAViewerDialog(const char* WorkingDir, const char* ExtensionFilter, char* ReturnPathOut, UInt32 BufferSize)
{
	CopyStringToCharBuffer(BSAV->InitializeViewer(gcnew String(WorkingDir), gcnew String(ExtensionFilter)), ReturnPathOut, BufferSize);
}

ComponentDLLInterface::BSAViewerInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	ShowBSAViewerDialog
};