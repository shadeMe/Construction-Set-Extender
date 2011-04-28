#pragma once
#include "Common.h"

// hooks that do odd jobs such as fixing bugs and shoveling dung
void PatchMiscHooks(void);

extern char g_NumericIDWarningBuffer[0x10];

_DeclareMemHdlr(CSExit, "adds fast exit to the CS");
_DeclareMemHdlr(CSInit, "adds an one-time only hook to the CS main windows wndproc as an alternative to WinMain()");
_DeclareMemHdlr(AssertOverride, "fixes crashes from assertion calls in the code and log them to the console/log instead");
_DeclareMemHdlr(TextureMipMapCheck, "allows the preview of textures with mipmaps");
_DeclareMemHdlr(UnnecessaryDialogEdits, "prevents unnecessary dialog edits in active plugins should its master have a DIAL record");
_DeclareMemHdlr(UnnecessaryCellEdits, "prevents unnecessary cell/worldspace edits in active plugins should its master have a CELL/WRLD record ### Figure out what the function's doing");
_DeclareMemHdlr(PluginSave, "provides a callback post-plugin load/save");
_DeclareMemHdlr(PluginLoad, "");
_DeclareMemHdlr(DataHandlerClearData, "event hook to perform various cleanup operations");
_DeclareMemHdlr(TopicInfoCopyEpilog, "fixes the bug that causes the wrong topic info to be flagged as active when using the copy popup menu option");
_DeclareMemHdlr(TopicInfoCopyProlog, "");
_DeclareMemHdlr(NumericEditorID, "displays a warning when editorIDs start with an integer");
_DeclareMemHdlr(DataHandlerConstructSpecialForms, "initializes easter egg forms");
_DeclareMemHdlr(ResultScriptSaveForm, "prevents a crash that occurs when a result script has local variable declarations");
_DeclareMemHdlr(TESObjectREFRDoCopyFrom, "patches the TESObjectREFR::Copy handler to fully duplicate extradata from the source");
_DeclareMemHdlr(TESFormAddReference, "patches the CS form referencing code to take into account mutiple references of a form by another");
_DeclareMemHdlr(TESFormRemoveReference, "");
_DeclareMemHdlr(TESFormClearReferenceList, "");
_DeclareMemHdlr(TESFormPopulateUseInfoList, "");
_DeclareMemHdlr(TESFormDelete, "");
_DeclareMemHdlr(TextureSizeCheck, "allows the preview of textures of resolution > 512px");
_DeclareMemHdlr(AboutDialog, "add a mention of CSE");

void PatchMessageHandler(void);
void __stdcall DoCSInitHook();
void __stdcall DoCSExitHook(HWND MainWindow);
void __stdcall MessageHandlerOverride(const char* Message);