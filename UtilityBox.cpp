#include "ExtenderInternals.h"
#include "Console.h"


void __stdcall ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
{
	if (State)	*Flag |= Mask;
	else		*Flag &= ~Mask;
}

void LogWinAPIErrorMessage(DWORD ErrorID)
{
	LPVOID ErrorMsg;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		ErrorID,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &ErrorMsg,
		0, NULL );

	DebugPrint("\tError Message: %s", (LPSTR)ErrorMsg); 
	LocalFree(ErrorMsg);
}

void DebugPrint(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	CONSOLE->LogMessage(Console::e_CSE, fmt, args);
	va_end(args);
}
void DebugPrint(UInt8 source, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	CONSOLE->LogMessage(source, fmt, args);
	va_end(args);
}


// modified to use plugin debugging tools
void CSEDumpClass(void * theClassPtr, UInt32 nIntsToDump)
{
	DebugPrint("DumpClass:");
	UInt32* basePtr = (UInt32*)theClassPtr;

	if (!theClassPtr) return;
	for (UInt32 ix = 0; ix < nIntsToDump; ix++ ) {
		UInt32* curPtr = basePtr+ix;
		const char* curPtrName = NULL;
		UInt32 otherPtr = 0;
		float otherFloat = 0.0;
		const char* otherPtrName = NULL;
		if (curPtr) {
			curPtrName = GetObjectClassName((void*)curPtr);

			__try
			{
				otherPtr = *curPtr;
				otherFloat = *(float*)(curPtr);
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				//
			}

			if (otherPtr) {
				otherPtrName = GetObjectClassName((void*)otherPtr);
			}
		}

		DebugPrint("\t%3d +%03X ptr: 0x%08X: %32s *ptr: 0x%08x | %f: %32s", ix, ix*4, curPtr, curPtrName, otherPtr, otherFloat, otherPtrName);
	}
}

void WaitUntilDebuggerAttached()
{
	DebugPrint("----> Waiting For Debugger <----");
	while (IsDebuggerPresent() == FALSE){
		Sleep(5000);
	}
	DebugPrint("----> Debugger Attached <----");
}

void DumpExtraDataList(BaseExtraList* List)
{
	DebugPrint("BaseExtraList Dump:");
	CONSOLE->Indent();

	if (List->m_data)
	{
		for(BSExtraData * traverse = List->m_data; traverse; traverse = traverse->next)
			DebugPrint("%s", GetObjectClassName(traverse));
	}
	else
		DebugPrint("No data in list");

	CONSOLE->Exdent();
}