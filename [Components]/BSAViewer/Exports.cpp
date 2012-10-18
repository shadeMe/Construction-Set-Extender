#include "Exports.h"
#include "BSAViewer.h"

extern ComponentDLLInterface::BSAViewerInterface g_InteropInterface;

using namespace ConstructionSetExtender;

extern "C"
{
	QUERYINTERFACE_EXPORT
	{
		return &g_InteropInterface;
	}
}

void InitializeComponents(void)
{
	return;
}

void ShowBSAViewerDialog(const char* WorkingDir, const char* ExtensionFilter, char* ReturnPathOut, UInt32 BufferSize)
{
	CopyStringToCharBuffer(BSAV->InitializeViewer(gcnew String(WorkingDir), gcnew String(ExtensionFilter)), ReturnPathOut, BufferSize);
}

ComponentDLLInterface::BSAViewerInterface g_InteropInterface =
{
	DeleteManagedHeapPointer,
	InitializeComponents,
	ShowBSAViewerDialog
};