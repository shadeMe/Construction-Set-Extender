#pragma once

#include "UtilityBox.h"

using namespace MemoryHandler;

extern UInt32	g_CompileResultBuffer;
extern UInt32	g_CompileCallerAddr;

void PatchCompilerErrorDetours();

void RerouteScriptErrorsHook(void);
void CompilerPrologResetHook(void);
void CompilerEpilogCheckHook(void);
void ParseScriptLineOverride(void);

#define CURRENT_ADDR		0x0

#define GET_ERROR_MEMHDLR(hookaddr)								CompilerErrorOverrideHandler##hookaddr
#define DEFINE_ERROR_MEMHDLR(hookaddr)							MemHdlr GET_ERROR_MEMHDLR(hookaddr)(##hookaddr##, GET_ERROR_MEMHDLR(hookaddr)##Hook, 0, 0)
#define BEGIN_ERROR_HOOK(hookaddr)								void __declspec(naked) GET_ERROR_MEMHDLR(hookaddr)##Hook(void)
#define SET_ERROR_CUSTOMADDR(hookaddr, suffix, customaddr)		static UInt32 GET_ERROR_MEMHDLR(hookaddr)##suffix##Addr = ##customaddr
#define GET_ERROR_CUSTOMADDR(hookaddr, suffix)					GET_ERROR_MEMHDLR(hookaddr)##suffix##Addr

#define SET_COMPILE_BUFFER_VALUE(value)							mov		g_CompileResultBuffer, ##value


#define DEFINE_SHOWCOMPILERERROR_HOOK(hookaddr, jmpaddr, stackoffset)		\
void CompilerErrorOverrideHandler##hookaddr##Hook(void);					\
MemHdlr CompilerErrorOverrideHandler##hookaddr##(##hookaddr##, CompilerErrorOverrideHandler##hookaddr##Hook, 0, 0);		\
void __declspec(naked) CompilerErrorOverrideHandler##hookaddr##Hook(void)	\
{																			\
	static UInt32 CompilerErrorOverrideHandler##hookaddr##RetnAddr = jmpaddr##;		\
	{																		\
		__asm	call	ShowCompilerErrorEx									\
		__asm	mov		g_CompileResultBuffer, 0							\
		__asm	add		esp, stackoffset									\
		__asm	jmp		CompilerErrorOverrideHandler##hookaddr##RetnAddr	\
	}																		\
}