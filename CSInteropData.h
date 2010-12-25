#pragma once

// struct passed as messages through the interop pipe
// data shouldn't exceed size of the struct

struct CSECSInteropData
{
	enum
	{
		kMessageType_Wait = 0,
		kMessageType_Quit,
		kMessageType_OperationResult,
		kMessageType_GenerateLIP,
		kMessageType_DebugPrint
	};

	int				MessageType;
	int				OperationResult;
	char			StringBufferA[0x350];			// Lip Parameter 1 / Debug Message
	char			StringBufferB[0x350];			// Lip Parameter 2

	CSECSInteropData(int MessageType)
	{
		this->MessageType = MessageType;
		OperationResult = 0;
	}
};

enum
{
	kPipeOperation_Read = 0,
	kPipeOperation_Write
};

typedef unsigned long	DWORD;
typedef void*			HANDLE;

bool PerformPipeOperation(HANDLE PipeHandle, int Operation, CSECSInteropData* Buffer, DWORD* BytesReadWrittenBuf);