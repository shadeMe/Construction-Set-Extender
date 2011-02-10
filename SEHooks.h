#pragma once
#include "Hooks_Common.h"

struct CommandTableData;
struct FormData;
struct ScriptData;
struct OBSEInterface;
class Script;
class TESForm;
struct ScriptVarIndexData;

bool PatchSEHooks();
void FillScriptDataPackage(Script* ScriptForm);

extern Script*				g_EditorInitScript;
extern ScriptData*			g_ScriptDataPackage;
extern Script*				g_SetEditorTextCache;
extern Script*				g_ScriptListResult;
extern Script*				g_EditorAuxScript;


extern MemHdlr				kMainWindowEntryPoint;// patches the various entry points to spawn our editor
extern MemHdlr				kScriptableFormEntryPoint;
extern MemHdlr				kScriptEffectItemEntryPoint;
extern MemHdlr				kLoadRelease;// releases all allocated editors
extern MemHdlr				kEditorWindowProc;// caches the handle to the script editor processed by the current WndProc callback
extern MemHdlr				kEditorWindowWParam;// caches the WPARAM processed by the current WndProc callback
extern MemHdlr				kRecompileScripts;// patches the recompile script routine to skip scripts outside the active plugin
extern MemHdlr				kEditorInitScript;// patches the init routines of the script editor's WndProc to allow the initialization of custom instances and hide the vanilla editor
extern MemHdlr				kEditorInitGetAuxScript;
extern MemHdlr				kEditorInitWindowPos;
extern MemHdlr				kMessagingCallbackNewScript;// patches various locations to provide callbacks to CSE's editor. Only at calls to f_Script::SetEditorText (with the exception of e_Close)
extern MemHdlr				kMessagingCallbackOpenNextScript;// same location for all 3 callbacks (open, next and delete)
extern MemHdlr				kMessagingCallbackPreviousScript;
extern MemHdlr				kMessagingCallbackClose;
extern MemHdlr				kMessagingCallbackSave;
extern MemHdlr				kScriptListOpen;// passes the script selected in our editor's open dialog instead of the vanilla handler's
extern MemHdlr				kScriptListDelete;
extern MemHdlr				kSaveDialogBox;// adds a cancel option to the message box
extern MemHdlr				kSaveDialogBoxType;
extern MemHdlr				kLogRecompileResults;// logs failed recompile script calls to the console
extern MemHdlr				kRecompileScriptsMessageBoxString;
extern MemHdlr				kToggleScriptCompilingOriginalData;// toggles script compiling during a save callback
extern MemHdlr				kToggleScriptCompilingNewData;
extern MemHdlr				kMaxScriptSizeOverrideScriptBufferCtor;// sounds obvious enough
extern MemHdlr				kMaxScriptSizeOverridesParseScriptLine;

void __stdcall SendPingBack(UInt16 Message);

void MainWindowEntryPointHook(void);
void ScriptableFormEntryPointHook(void);
void ScriptEffectItemEntryPointHook(void);

void LoadReleaseHook(void);
void EditorWindowProcHook(void);
void EditorWindowWParamHook(void);
void RecompileScriptsHook(void);
void EditorInitScriptHook(void);
void EditorInitGetAuxScriptHook(void);
void EditorInitWindowPosHook(void);

void MessagingCallbackNewScriptHook(void);
void MessagingCallbackOpenNextScriptHook(void);	
void MessagingCallbackPreviousScriptHook(void);
void MessagingCallbackCloseHook(void);
void MessagingCallbackSaveHook(void);

void ScriptListOpenHook(void);
void ScriptListDeleteHook(void);

void SaveDialogBoxHook(void);
void LogRecompileResultsHook(void);
void ToggleScriptCompiling(bool Enable);

void MaxScriptSizeOverrideScriptBufferCtorHook(void);
void MaxScriptSizeOverrideParseScriptLineHook(void);