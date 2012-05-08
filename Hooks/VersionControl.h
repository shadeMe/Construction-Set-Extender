#pragma once
#include "Common.h"

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		// hooks that add support for various version control (change log) related routines
		void PatchVersionControlHooks(void);

		_DeclareMemHdlr(DataHandlerSavePluginProlog, "records plugin saves and creates backups");
		_DeclareMemHdlr(DataHandlerLoadPluginsWrapper, "records plugin loads");
		_DeclareMemHdlr(TESFormSetFromActivePlugin, "records modifictions to a form's modified flag");
		_DeclareMemHdlr(TESFormSetDeleted, "records modifications to a form's deleted flag");
		_DeclareMemHdlr(TESFormSetFormID, "records changes to a form's formID");
		_DeclareMemHdlr(TESFormSetEditorID, "records changes to a form's editorID");
		_DeclareMemHdlr(TESDialogFormEditNewForm, "records the creation of new forms from various sources");
		_DeclareMemHdlr(ObjectWindowDragDropNewForm, "");
		_DeclareMemHdlr(TESDialogFormIDListViewNewForm, "");
		_DeclareMemHdlr(TESDialogCreateNewForm, "");
		_DeclareMemHdlr(TESFormClone, "");
		_DeclareMemHdlr(TESObjectCELLDuplicate, "");
		_DeclareMemHdlr(TESObjectREFRDuplicate, "");
		_DeclareMemHdlr(TESTopicCreate, "");
		_DeclareMemHdlr(TESTopicInfoCreate, "");
		_DeclareMemHdlr(TESIdleFormCreateSibling, "");
		_DeclareMemHdlr(TESIdleFormCreateChild, "");
		_DeclareMemHdlr(TESDialogFormEditCopyForm, "records changes made to a form on closing a form edit dialog");
	}
}
