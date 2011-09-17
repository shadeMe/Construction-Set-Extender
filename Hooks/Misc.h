#pragma once
#include "Common.h"

namespace Hooks
{
	// hooks that do odd jobs such as fixing bugs and shoveling dung
	void PatchMiscHooks(void);
	void PatchEntryPointHooks(void);

	_DeclareMemHdlr(CSExit, "adds fast exit to the CS");
	_DeclareMemHdlr(CSInit, "adds an one-time only hook to the CS main windows wndproc as an alternative to WinMain()");
	_DeclareMemHdlr(AssertOverride, "fixes crashes from assertion calls in the code and log them to the console/log instead");
	_DeclareMemHdlr(TextureMipMapCheck, "allows the preview of textures with mipmaps");
	_DeclareMemHdlr(UnnecessaryDialogEdits, "prevents unnecessary dialog edits in active plugins should its master have a DIAL record");
	_DeclareMemHdlr(UnnecessaryCellEdits, "prevents unnecessary cell/worldspace edits in active plugins should its master have a CELL/WRLD record");
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
	_DeclareMemHdlr(DataHandlerPlaceTESObjectLIGH, "fixes a CTD that occurs on the first reference placement of a light object");
	_DeclareMemHdlr(TESWorldSpaceDestroyCellMapA, "fixes a CTD that sometime occurs for unknown reasons");
	_DeclareMemHdlr(TESWorldSpaceDestroyCellMapB, "");
	_DeclareMemHdlr(CSRegistryEntries, "prevents the CS from messing around the ESM/ESP file registy entries at startup");
	_DeclareMemHdlr(AchievementAddTopic, "various hooks that unlock achievements");
	_DeclareMemHdlr(AchievementDeleteShadeMe, "");
	_DeclareMemHdlr(AchievementModifyShadeMe, "");
	_DeclareMemHdlr(AchievementCloneHallOfFameForms, "");
	_DeclareMemHdlr(AchievementPluginDescription, "");
	_DeclareMemHdlr(AchievementBuildRoads, "");
	_DeclareMemHdlr(AchievementDialogResponseCreation, "");
	_DeclareMemHdlr(TESDialogBuildSubwindowDiagnostics, "adds a diagnostic message to TESDialog::BuildSubwindow()");
	_DeclareMemHdlr(ExtraTeleportInitItem, "patches the function to take into account refs without base forms (happens when quick-loading plugins)");
	_DeclareMemHdlr(NewSplashImage, "replaces the default splash image with a new one");

	void PatchMessageHandler(void);
	void __stdcall DoCSInitHook();
	void __stdcall DoCSExitHook(HWND MainWindow);
	void __stdcall MessageHandlerOverride(const char* Message);
}