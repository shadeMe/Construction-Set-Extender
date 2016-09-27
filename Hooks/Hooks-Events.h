#pragma once
#include "Hooks-Common.h"

namespace cse
{
	namespace hooks
	{
		// hooks that add support for various event sources
		void PatchEventHooks(void);

		namespace events
		{
			namespace plugin
			{
				_DeclareMemHdlr(PreSave, "");
				_DeclareMemHdlr(PostSave, "");
				_DeclareMemHdlr(PrePostLoad, "");
				_DeclareMemHdlr(ClearData, "");
			}

			namespace form
			{
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
			}

			namespace renderer
			{
				_DeclareMemHdlr(NiDX9RendererRecreateA, "");
				_DeclareMemHdlr(NiDX9RendererRecreateB, "");
				_DeclareMemHdlr(NiDX9RendererRecreateC, "");
				_DeclareMemHdlr(PreSceneGraphRender, "");
				_DeclareMemHdlr(PostSceneGraphRender, "");
			}

			namespace dialog
			{
				_DeclareMemHdlr(CloseAll, "");

				namespace cellView
				{
					_DeclareMemHdlr(SelectCell, "");
				}

				namespace renderWindow
				{
					_DeclareMemHdlr(PlaceRef, "");
				}
			}
		}

	}
}
