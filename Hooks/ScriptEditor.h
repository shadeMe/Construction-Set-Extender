#pragma once
#include "Common.h"

namespace Hooks
{
	// hooks that put a spike up the vanilla script editor's bottom
	void PatchScriptEditorHooks(void);

	_DeclareMemHdlr(MainWindowEntryPoint, "patches the various entry points to spawn our editor");
	_DeclareMemHdlr(ScriptableFormEntryPoint, "");
	_DeclareMemHdlr(ScriptEffectItemEntryPoint, "");
	_DeclareMemHdlr(LoadRelease, "releases all allocated editors");
	_DeclareMemHdlr(ToggleScriptCompilingOriginalData, "toggles script compiling during a save callback");
	_DeclareMemHdlr(ToggleScriptCompilingNewData, "");
	_DeclareMemHdlr(MaxScriptSizeOverrideScriptBufferCtor, "sounds obvious enough");
	_DeclareMemHdlr(MaxScriptSizeOverrideParseScriptLine, "");
}