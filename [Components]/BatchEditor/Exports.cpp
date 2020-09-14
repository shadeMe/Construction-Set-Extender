#include "BatchEditor.h"

using namespace componentDLLInterface;
using namespace cse;

extern componentDLLInterface::BatchEditorInterface g_InteropInterface;

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

bool ShowBatchRefEditorDialog(BatchRefData* Data)
{
	return REFBE->InitializeBatchEditor(Data);
}

componentDLLInterface::BatchEditorInterface g_InteropInterface =
{
	InitializeComponents,
	ShowBatchRefEditorDialog
};