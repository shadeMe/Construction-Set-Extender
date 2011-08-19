#pragma once

namespace Hooks
{
	extern UInt32	g_CompileResultBuffer;
	extern bool		g_PreventScriptCompileErrorRerouting;

	struct CompilerErrorData
	{
		UInt32				Line;
		std::string			Message;

		CompilerErrorData(UInt32 Line, const char* Message) : Line(Line), Message(Message) {}
	};

	typedef std::vector<CompilerErrorData>	CompilerErrorListT;
	extern CompilerErrorListT		g_CompilerErrorListBuffer;

	void PatchCompilerErrorDetours();

	_DeclareNopHdlr(RidScriptErrorMessageBox, "prevents the vanilla script error message box from being displayed");
	_DeclareNopHdlr(RidUnknownFunctionCodeMessage, "removes a redundant error message");
	_DeclareMemHdlr(RerouteScriptErrors, "reroutes script compilation error messages to the parent script editor");
	_DeclareMemHdlr(CompilerEpilogCheck, "adds support for the above");
	_DeclareMemHdlr(CompilerPrologReset, "");
	_DeclareMemHdlr(ParseScriptLineOverride, "");
	_DeclareMemHdlr(CheckLineLengthLineCount, "fixes a bug in the compiler that causes empty lines to be skipped when line numbers are counted");
	_DeclareMemHdlr(ResultScriptErrorNotification, "displays a notification when result scripts fail to compile");

	#define GetErrorMemHdlr(hookaddr)								CompilerErrorOverrideHandler##hookaddr
	#define DefineCompilerErrorOverrideHook(hookaddr, jmpaddr, stackoffset)		\
	void CompilerErrorOverrideHandler##hookaddr##Hook(void);					\
	MemHdlr CompilerErrorOverrideHandler##hookaddr##(##hookaddr##, CompilerErrorOverrideHandler##hookaddr##Hook, 0, 0);		\
	void __declspec(naked) CompilerErrorOverrideHandler##hookaddr##Hook(void)	\
	{																			\
	static UInt32 CompilerErrorOverrideHandler##hookaddr##RetnAddr = jmpaddr##;		\
	{																	\
	__asm	call	TESScriptCompiler::ShowMessage						\
	__asm	mov		g_CompileResultBuffer, 0							\
	__asm	add		esp, stackoffset									\
	__asm	jmp		CompilerErrorOverrideHandler##hookaddr##RetnAddr	\
	}																	\
	}
}