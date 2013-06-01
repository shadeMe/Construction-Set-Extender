#pragma once
#include "Hooks-Common.h"

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		// hooks that put a spike up the vanilla script editor's bottom
		void PatchScriptEditorHooks(void);

		_DeclareMemHdlr(MainWindowEntryPoint, "patches the various entry points to spawn our editor");
		_DeclareMemHdlr(ScriptableFormEntryPoint, "");
		_DeclareMemHdlr(ScriptEffectItemEntryPoint, "");
		_DeclareMemHdlr(ToggleScriptCompilingOriginalData, "toggles script compiling during a save callback");
		_DeclareMemHdlr(ToggleScriptCompilingNewData, "");
		_DeclareMemHdlr(MaxScriptSizeOverrideScriptBufferCtor, "sounds obvious enough");
		_DeclareMemHdlr(MaxScriptSizeOverrideParseScriptLine, "");
		_DeclareMemHdlr(InitializeScriptLineBufferLFLineEnds, "patches the code to indentify LF line ends in addition to those with CRLF");
		_DeclareMemHdlr(ScriptCompileCheckSyntaxInvalidRef, "fixes a bug that causes a CTD when a ref var was used as a calling reference in a compound expression, e.g. message \"object agi: %d\" someRefvar.getAV Agility");
		_DeclareMemHdlr(ScriptCompilerWriteByteCodeCheckSetExprParentheses, "fixes a bug that causes the script compiler to skip warning the user about mismatching parentheses in set expressions");
	}
}
