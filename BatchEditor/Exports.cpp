#include "Exports.h"
#include "BatchEditor.h"

extern "C"{

__declspec(dllexport) bool InitializeRefBatchEditor(BatchRefData* Data)
{
	return REFBE->InitializeBatchEditor(Data);
}

}