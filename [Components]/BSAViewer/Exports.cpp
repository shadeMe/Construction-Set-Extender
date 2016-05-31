#include "Exports.h"
#include "BSAViewer.h"

extern componentDLLInterface::BSAViewerInterface g_InteropInterface;

using namespace cse;

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

componentDLLInterface::BSAViewerInterface g_InteropInterface =
{
	InitializeComponents,
	ShowBSAViewerDialog
};