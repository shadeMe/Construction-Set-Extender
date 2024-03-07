#pragma warning(disable:4800)

#include "OldCSInteropData.h"
#include <Windows.h>

bool PerformPipeOperation(HANDLE PipeHandle, int Operation, OldCSInteropData* Buffer, DWORD* BytesReadWrittenBuf)
{
	bool PipeOperation = false;

	if (PipeHandle != INVALID_HANDLE_VALUE)
	{
		switch (Operation)
		{
		case kPipeOperation_Read:
			PipeOperation = ReadFile(PipeHandle,
									Buffer,
									sizeof(OldCSInteropData),
									BytesReadWrittenBuf,
									nullptr);
			break;
		case kPipeOperation_Write:
			PipeOperation = WriteFile(PipeHandle,
									Buffer,
									sizeof(OldCSInteropData),
									BytesReadWrittenBuf,
									nullptr);
			break;
		}
	}

	// We need to handle this beauty...
	if (GetLastError() == ERROR_SUCCESS)
		PipeOperation = true;

	return PipeOperation;
}