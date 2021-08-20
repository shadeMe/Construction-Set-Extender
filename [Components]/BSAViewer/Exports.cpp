#include "Exports.h"
#include "BSAViewer.h"
#include <stdio.h>

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
	nativeWrapper::Initialize();
}

void ShowBSAViewerDialog(const char* WorkingDir, const char* ExtensionFilter, char* ReturnPathOut, UInt32 BufferSize)
{
	auto WorkingDirectory = gcnew String(WorkingDir);
	auto ExtensionString = gcnew String(ExtensionFilter);

	bsaViewer::ArchiveBrowser Browser(WorkingDirectory, bsaViewer::ArchiveFile::GetFileTypeFromExtension(ExtensionString));

	auto CString = (const char*)(void*)Marshal::StringToHGlobalAnsi(Browser.SelectedFilePath);
	_snprintf_s(ReturnPathOut, BufferSize, _TRUNCATE, "%s", CString);
	Marshal::FreeHGlobal((IntPtr)(void*)CString);
}

componentDLLInterface::BSAViewerInterface g_InteropInterface =
{
	InitializeComponents,
	ShowBSAViewerDialog
};