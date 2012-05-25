#include "TESIdleForm.h"

void**								g_IdleFormTree = (void**)0x00A107F8;

void TESIdleForm::InitializeIdleFormTreeRootNodes()
{
	thisCall<UInt32>(0x004D4490, *g_IdleFormTree);
}

void TESIdleForm::ResetIdleFormTree()
{
	thisCall<void>(0x004D4540, *g_IdleFormTree);

	// some idle forms aren't being freed for some reason (caused by the plugin, prolly)
	// can't be bothered to figure out the cause, so we removed the orphans manually

	for (CSE_GlobalClasses::NiTMapIterator Itr = g_TESFormFormIDMap->GetFirstPos(); Itr;)
	{
		UInt32 FormID = NULL;
		TESForm* Form = NULL;

		g_TESFormFormIDMap->GetNext(Itr, FormID, Form);
		if (FormID && Form)
		{
			if (Form->formType == TESForm::kFormType_Idle)
				Form->DeleteInstance();
		}
	}
}
