#pragma once

#include "UtilityBox.h"

using namespace MemoryHandler;

extern UInt32	g_CompileResultBuffer;
extern UInt32	g_CompileCallerAddr;
extern bool		g_RecompilingScripts;

void PatchCompilerErrorDetours();

void RerouteScriptErrorsHook(void);
void CompilerPrologResetHook(void);
void CompilerEpilogCheckHook(void);
void ParseScriptLineOverride(void);

#define GetErrorMemHdlr(hookaddr)								CompilerErrorOverrideHandler##hookaddr
#define DefineCompilerErrorOverrideHook(hookaddr, jmpaddr, stackoffset)		\
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