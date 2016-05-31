#pragma once
#include "Hooks-Common.h"

namespace cse
{
	namespace hooks
	{
		// hooks that modify how the CS handles plugin files
		void PatchTESFileHooks(void);

		_DeclareMemHdlr(SavePluginMasterEnum, "allows esps to be enumerated while filling the file header");
		_DeclareNopHdlr(CheckIsActivePluginAnESM, "allows master files to be set as active plugins");
		_DeclareNopHdlr(TESFormGetUnUsedFormID, "");
		_DeclareMemHdlr(LoadPluginsProlog, "");
		_DeclareMemHdlr(LoadPluginsEpilog, "");
		_DeclareMemHdlr(PostPluginSave, "");
		_DeclareMemHdlr(PostPluginLoad, "");
		_DeclareMemHdlr(DataDialogPluginDescription, "allows the Author and Description fields of an ESM file to be viewed and modified correctly");
		_DeclareMemHdlr(DataDialogPluginAuthor, "");
		_DeclareMemHdlr(SavePluginCommonDialog, "allows the creation of ESM files in the CS");
		_DeclareMemHdlr(DataHandlerPostError, "fixes a crash when the CS attempts to load an unknown record/group");
		_DeclareMemHdlr(DataHandlerSaveFormToFile, "allows records in esp masters to be overridden with deleted records");
		_DeclareMemHdlr(TESFileUpdateHeader, "prevents TESFile::UpdateHeader from continuing for locked files");
		_DeclareMemHdlr(DataHandlerSavePluginEpilog, "prevents the esm flag bit from being reset");
		_DeclareMemHdlr(TESFileUpdateHeaderFlagBit, "");
		_DeclareMemHdlr(TESObjectCELLSaveReferencesProlog, "prevents malformed records of deleted refs from being written");
		_DeclareMemHdlr(TESObjectCELLSaveReferencesEpilog, "");
		_DeclareMemHdlr(MissingMasterOverride, "allows the loading of plugins with missing masters");
		_DeclareMemHdlr(AutoLoadActivePluginOnStartup, "temporary hook that allows the automatic loading of plugins on startup");
		_DeclareMemHdlr(DataHandlerSavePluginResetA, "patches various locations in DataHandler::SavePlugin to prevent a premature exit from disabling the save tool");
		_DeclareMemHdlr(DataHandlerSavePluginResetB, "");
		_DeclareMemHdlr(DataHandlerSavePluginResetC, "");
		_DeclareNopHdlr(DataHandlerSavePluginOverwriteESM, "allows the overwriting of ESM files");
		_DeclareMemHdlr(DataHandlerSavePluginRetainTimeStamps, "allows the retention of plugin timestamps during save operations");
		_DeclareMemHdlr(DataDialogUpdateCurrentTESFileHeaderA, "");
		_DeclareMemHdlr(DataDialogUpdateCurrentTESFileHeaderB, "");
		_DeclareMemHdlr(DataHandlerAutoSaveA, "prevents the active file from being added as a master to the autosave backup");
		_DeclareMemHdlr(DataHandlerAutoSaveB, "");
		_DeclareMemHdlr(DataDlgCancelled, "fixes a bug that causes the editor to reset the 'dirty' flag on closing the data dialog");
	}
}