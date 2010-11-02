#pragma once

struct CommandTableData;
struct FormData;
struct ScriptData;
struct OBSEInterface;
class Script;
class TESForm;
struct ScriptVarIndexData;

extern Script*				g_EditorInitScript;
extern ScriptData*			g_ScriptDataPackage;
extern Script*				g_SetEditorTextCache;
extern Script*				g_ScriptListResult;
extern std::string			g_ActivePluginName;

bool PatchSEHooks();
void FillScriptDataPackage(Script* ScriptForm);

const UInt32				kScriptSetEditorTextCallAddr = 0x004FC1F0;
const UInt32				kRecompileScriptsMessageBoxStringPatchAddr = 0x004FEF3F;

extern Script*				g_EditorAuxScript;
extern const char*			g_DefaultWaterTextureStr;




// patches the various entry points to spawn our editor
const UInt32			kMainWindowEntryPointHookAddr =	0x0041A5F6;	
const UInt32			kMainWindowEntryPointRetnAddr =	0x0041A610;	
const UInt32			kScriptableFormEntryPointHookAddr =	0x004A16AD;	
const UInt32			kScriptableFormEntryPointRetnAddr =	0x004A16C5;	
const UInt32			kScriptEffectItemEntryPointHookAddr =	0x00566387;	
const UInt32			kScriptEffectItemEntryPointRetnAddr =	0x0056639F;	

void MainWindowEntryPointHook(void);
void ScriptableFormEntryPointHook(void);
void ScriptEffectItemEntryPointHook(void);
// releases all allocated editors
const UInt32			kLoadReleaseHookAddr =	0x0040D090;	
const UInt32			kLoadReleaseRetnAddr =	0x0040D096;	

void LoadReleaseHook(void);
// caches the handle to the script editor processed by the current WndProc callback
const UInt32			kEditorWindowProcHookAddr =	0x004FE7AC;	
const UInt32			kEditorWindowProcRetnAddr =	0x004FE7B2;	

void EditorWindowProcHook(void);
// caches the WPARAM processed by the current WndProc callback
const UInt32			kEditorWindowWParamHookAddr =	0x004FEC46;	
const UInt32			kEditorWindowWParamRetnAddr =	0x004FED08;	

void EditorWindowWParamHook(void);
// calls CLIWrapper::UpdateIntelliSense and fixes the default water texture
const UInt32			kGetPluginNameSaveHookAddr	=	0x0041BBCD;
const UInt32			kGetPluginNameSaveRetnAddr	=	0x0041BBD3;

void GetPluginNameSaveHook(void);

const UInt32			kGetPluginNameLoadHookAddr	=	0x0041BEF4;
const UInt32			kGetPluginNameLoadRetnAddr	=	0x0041BEFA;

void GetPluginNameLoadHook(void);
// patches the recompile script routine to skip scripts outside the active plugin
const UInt32 kRecompileScriptsHookAddr = 0x004FEFEA;
const UInt32 kRecompileScriptsRetnAddr = 0x004FEFF1;
const UInt32 kRecompileScriptsCallAddr = 0x0047A4E0;				// sub_47A4E0
const UInt32 kRecompileScriptsSkipAddr = 0x004FF102;

void RecompileScriptsHook(void);
// patches the init routines of the script editor's WndProc to allow the initialization of custom instances and hide the vanilla editor
const UInt32			kEditorInitScriptHookAddr =	0x004FEB1F;	
const UInt32			kEditorInitScriptAuxRetnAddr =	0x004FEB28;	
const UInt32			kEditorInitScriptCSERetnAddr =	0x004FEB51;	

void EditorInitScriptHook(void);

const UInt32			kEditorInitGetAuxScriptHookAddr =	0x004FEB48;	
const UInt32			kEditorInitGetAuxScriptRetnAddr =	0x004FEB4E;	

void EditorInitGetAuxScriptHook(void);

const UInt32			kEditorInitWindowPosHookAddr =	0x004FEB9A;
const UInt32			kEditorInitWindowPosRetnAddr =	0x004FEBB0;

void EditorInitWindowPosHook(void);
// patches various locations to provide callbacks to CSE's editor. Only at calls to f_Script::SetEditorText (with the exception of e_Close)
void __stdcall SendPingBack(UInt16 Message);

const UInt32			kMessagingCallbackNewScriptHookAddr =	0x004FEDFD;	
const UInt32			kMessagingCallbackNewScriptRetnAddr =	0x004FEE0E;	

void MessagingCallbackNewScriptHook(void);

const UInt32			kMessagingCallbackOpenNextScriptHookAddr =	0x004FEE6F;			// same location for all 3 callbacks (open, next and delete)
const UInt32			kMessagingCallbackOpenNextScriptRetnAddr =	0x004FEE75;	

void MessagingCallbackOpenNextScriptHook(void);

const UInt32			kMessagingCallbackPreviousScriptHookAddr =	0x004FEEDF;	
const UInt32			kMessagingCallbackPreviousScriptRetnAddr =	0x004FF2D2;	

void MessagingCallbackPreviousScriptHook(void);

const UInt32			kMessagingCallbackCloseHookAddr =	0x004FED69;
const UInt32			kMessagingCallbackCloseRetnAddr =	0x004FED6F;	

void MessagingCallbackCloseHook(void);

const UInt32			kMessagingCallbackSaveHookAddr	=	0x004FE63D;	
const UInt32			kMessagingCallbackSaveRetnAddr	=	0x004FE642;	
const UInt32			kMessagingCallbackSaveCallAddr	=	0x00503450;					// f_PrecompileSub()

void MessagingCallbackSaveHook(void);
// passes the script selected in our editor's open dialog instead of the vanilla handler's
const UInt32			kScriptListOpenHookAddr	=	0x004FEE1D;	
const UInt32			kScriptListOpenRetnAddr	=	0x004FEE33;	

const UInt32			kScriptListDeleteHookAddr	=	0x004FF133;	
const UInt32			kScriptListDeleteRetnAddr	=	0x004FF14B;	

void ScriptListOpenHook(void);
void ScriptListDeleteHook(void);
// takes care of the vanilla expression parser's 73-bytes/line CTD
// ## moved into OBSE core
const UInt32			kExpressionParserSwapBufferAHookAddr	=	0x004F9712;	
const UInt32			kExpressionParserSwapBufferARetnAddr	=	0x004F9717;	

void ExpressionParserSwapBufferAHook(void);

const UInt32			kExpressionParserSwapBufferBHookAddr	=	0x004F9863;	
const UInt32			kExpressionParserSwapBufferBRetnAddr	=	0x004F986A;	

void ExpressionParserSwapBufferBHook(void);
// adds a cancel option to the message box
const UInt32			kSaveDialogBoxHookAddr	=	0x004FE56D;	
const UInt32			kSaveDialogBoxYesRetnAddr	=	0x004FE59D;	
const UInt32			kSaveDialogBoxNoRetnAddr	=	0x004FE572;	
const UInt32			kSaveDialogBoxCancelRetnAddr	=	0x004FE731;	
const UInt32			kSaveDialogBoxTypePatchAddr	=	0x004FE558;	

void SaveDialogBoxHook(void);
// logs failed recompile script calls to the console
const UInt32			kLogRecompileResultsHookAddr = 0x004FF07E;
const UInt32			kLogRecompileResultsRetnAddr = 0x004FF083;
const UInt32			kLogRecompileResultsCallAddr = 0x00503450;

void LogRecompileResultsHook(void);