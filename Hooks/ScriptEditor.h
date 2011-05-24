#pragma once
#include "Common.h"

namespace Hooks
{
	// hooks that put a spike up the vanilla script editor's bottom
	void PatchScriptEditorHooks(void);
	void FillScriptDataPackage(Script* ScriptForm);

	extern Script* g_EditorInitScript;
	extern ScriptData* g_ScriptDataPackage;
	extern Script* g_SetEditorTextCache;
	extern Script* g_ScriptListResult;
	extern Script* g_EditorAuxScript;


	_DeclareMemHdlr(MainWindowEntryPoint, "patches the various entry points to spawn our editor");
	_DeclareMemHdlr(ScriptableFormEntryPoint, "");
	_DeclareMemHdlr(ScriptEffectItemEntryPoint, "");
	_DeclareMemHdlr(LoadRelease, "releases all allocated editors");
	_DeclareMemHdlr(EditorWindowProc, "caches the handle to the script editor processed by the current WndProc callback");
	_DeclareMemHdlr(EditorWindowWParam, "caches the WPARAM processed by the current WndProc callback");
	_DeclareMemHdlr(RecompileScripts, "patches the recompile script routine to skip scripts outside the active plugin");
	_DeclareMemHdlr(EditorInitScript, "patches the init routines of the script editor's WndProc to allow the initialization of custom instances and hide the vanilla editor");
	_DeclareMemHdlr(EditorInitGetAuxScript, "");
	_DeclareMemHdlr(EditorInitWindowPos, "");
	_DeclareMemHdlr(MessagingCallbackNewScript, "patches various locations to provide callbacks to CSE's editor. Only at calls to f_Script::SetEditorText (with the exception of e_Close)");
	_DeclareMemHdlr(MessagingCallbackOpenNextScript, "same location for all 3 callbacks (open, next and delete)");
	_DeclareMemHdlr(MessagingCallbackPreviousScript, "");
	_DeclareMemHdlr(MessagingCallbackClose, "");
	_DeclareMemHdlr(MessagingCallbackSave, "");
	_DeclareMemHdlr(ScriptListOpen, "passes the script selected in our editor's open dialog instead of the vanilla handler's");
	_DeclareMemHdlr(ScriptListDelete, "");
	_DeclareMemHdlr(SaveDialogBox, "adds a cancel option to the message box");
	_DeclareMemHdlr(SaveDialogBoxType, "");
	_DeclareMemHdlr(LogRecompileResults, "logs failed recompile script calls to the console");
	_DeclareMemHdlr(RecompileScriptsMessageBoxString, "");
	_DeclareMemHdlr(ToggleScriptCompilingOriginalData, "toggles script compiling during a save callback");
	_DeclareMemHdlr(ToggleScriptCompilingNewData, "");
	_DeclareMemHdlr(MaxScriptSizeOverrideScriptBufferCtor, "sounds obvious enough");
	_DeclareMemHdlr(MaxScriptSizeOverrideParseScriptLine, "");

	void __stdcall SendPingBack(UInt16 Message);
	void ToggleScriptCompiling(bool Enable);
}
