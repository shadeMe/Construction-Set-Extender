#pragma once
#include "UtilityBox.h"

struct CommandTableData;
struct FormData;
struct ScriptData;
struct OBSEInterface;
class Script;
class TESForm;
struct ScriptVarIndexData;

using namespace	MemoryHandler;

bool PatchSEHooks();
void FillScriptDataPackage(Script* ScriptForm);

extern Script*				g_EditorInitScript;
extern ScriptData*			g_ScriptDataPackage;
extern Script*				g_SetEditorTextCache;
extern Script*				g_ScriptListResult;
extern Script*				g_EditorAuxScript;
extern const char*			g_DefaultWaterTextureStr;


extern MemHdlr				kMainWindowEntryPoint;
extern MemHdlr				kScriptableFormEntryPoint;
extern MemHdlr				kScriptEffectItemEntryPoint;
extern MemHdlr				kLoadRelease;
extern MemHdlr				kEditorWindowProc;
extern MemHdlr				kEditorWindowWParam;
extern MemHdlr				kRecompileScripts;
extern MemHdlr				kEditorInitScript;
extern MemHdlr				kEditorInitGetAuxScript;
extern MemHdlr				kEditorInitWindowPos;
extern MemHdlr				kMessagingCallbackNewScript;
extern MemHdlr				kMessagingCallbackOpenNextScript;
extern MemHdlr				kMessagingCallbackPreviousScript;
extern MemHdlr				kMessagingCallbackClose;
extern MemHdlr				kMessagingCallbackSave;
extern MemHdlr				kScriptListOpen;
extern MemHdlr				kScriptListDelete;
extern MemHdlr				kSaveDialogBox;
extern MemHdlr				kLogRecompileResults;
extern MemHdlr				kRecompileScriptsMessageBoxString;
extern MemHdlr				kSaveDialogBoxType;
extern MemHdlr				kToggleScriptCompilingOriginalData;
extern MemHdlr				kToggleScriptCompilingNewData;
extern MemHdlr				kMaxScriptSizeOverrideScriptBufferCtor;
extern MemHdlr				kMaxScriptSizeOverridesParseScriptLine;

void __stdcall SendPingBack(UInt16 Message);

void MainWindowEntryPointHook(void);// patches the various entry points to spawn our editor
void ScriptableFormEntryPointHook(void);
void ScriptEffectItemEntryPointHook(void);

void LoadReleaseHook(void);// releases all allocated editors
void EditorWindowProcHook(void);// caches the handle to the script editor processed by the current WndProc callback
void EditorWindowWParamHook(void);// caches the WPARAM processed by the current WndProc callback
void RecompileScriptsHook(void);// patches the recompile script routine to skip scripts outside the active plugin
void EditorInitScriptHook(void);// patches the init routines of the script editor's WndProc to allow the initialization of custom instances and hide the vanilla editor
void EditorInitGetAuxScriptHook(void);
void EditorInitWindowPosHook(void);

void MessagingCallbackNewScriptHook(void);// patches various locations to provide callbacks to CSE's editor. Only at calls to f_Script::SetEditorText (with the exception of e_Close)
void MessagingCallbackOpenNextScriptHook(void);	// same location for all 3 callbacks (open, next and delete)
void MessagingCallbackPreviousScriptHook(void);
void MessagingCallbackCloseHook(void);
void MessagingCallbackSaveHook(void);

void ScriptListOpenHook(void);// passes the script selected in our editor's open dialog instead of the vanilla handler's
void ScriptListDeleteHook(void);

void SaveDialogBoxHook(void);// adds a cancel option to the message box
void LogRecompileResultsHook(void);// logs failed recompile script calls to the console
void ToggleScriptCompiling(bool Enable);// toggles script compiling during a save callback

void MaxScriptSizeOverrideScriptBufferCtorHook(void);
void MaxScriptSizeOverrideParseScriptLineHook(void);