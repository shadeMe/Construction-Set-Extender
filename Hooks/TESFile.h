#pragma once
#include "Common.h"

// hooks that modify plugin file loading behaviour
void PatchTESFileHooks(void);

_DeclareMemHdlr(SavePluginMasterEnum, "allows esps to be enumerated while filling the file header and provides support for the save as tool");
_DeclareNopHdlr(CheckIsActivePluginAnESM, "allows master files to be set as active plugins");
_DeclareNopHdlr(TESFormGetUnUsedFormID, "");
_DeclareMemHdlr(LoadPluginsProlog, "");
_DeclareMemHdlr(LoadPluginsEpilog, "");
_DeclareMemHdlr(DataDialogPluginDescription, "allows the Author and Description fields of an ESM file to be viewed and modified correctly");
_DeclareMemHdlr(DataDialogPluginAuthor, "");
_DeclareMemHdlr(SavePluginCommonDialog, "allows the creation of ESM files in the CS");
_DeclareMemHdlr(DataHandlerPostError, "fixes a crash when the CS attempts to load an unknown record/group");
_DeclareMemHdlr(DataHandlerSaveFormToFile, "allows records in esp masters to be overridden with deleted records");
_DeclareMemHdlr(TESFileUpdateHeader, "prevents TESFile::UpdateHeader from continuing for locked files");
_DeclareMemHdlr(DataHandlerSavePluginEpilog, "prevents the esm flag bit from being reset");
_DeclareMemHdlr(TESFileUpdateHeaderFlagBit, "same as above");
_DeclareMemHdlr(TESObjectCELLSaveReferencesProlog, "prevents malformed records of deleted refs from being written");
_DeclareMemHdlr(TESObjectCELLSaveReferencesEpilog, "");
_DeclareMemHdlr(MissingMasterOverride, "allows the loading of plugins with missing masters");
_DeclareMemHdlr(QuickLoadPluginLoadHandler, "adds support for the quick loading of plugins (only loads the active plugin)");
_DeclareMemHdlr(AutoLoadActivePluginOnStartup, "temporary hook that allows the automatic loading of plugins on startup");