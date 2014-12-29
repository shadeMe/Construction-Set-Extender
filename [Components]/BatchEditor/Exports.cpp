#include "BatchEditor.h"

using namespace ComponentDLLInterface;
using namespace ConstructionSetExtender;

extern ComponentDLLInterface::BatchEditorInterface g_InteropInterface;

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

bool ShowBatchRefEditorDialog(BatchRefData* Data)
{
	return REFBE->InitializeBatchEditor(Data);
}

ComponentDLLInterface::BatchEditorInterface g_InteropInterface =
{
	InitializeComponents,
	ShowBatchRefEditorDialog
};