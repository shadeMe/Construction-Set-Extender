#pragma once
#include "Common.h"

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		enum
		{
			kStartupScript_OpenScriptWindow = 0,
			kStartupScript_ScriptEditorID,

			kStartupScript__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kStartupScriptINISettings[kStartupScript__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetStartupScript(void);
	}

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
		_DeclareMemHdlr(ScriptCompilerWriteByteCodeCheckSetExprParentheses, "fixes a bug that causes the scirpt compiler to skip warning the user about mismatching parentheses in set expressions");
	}
}
