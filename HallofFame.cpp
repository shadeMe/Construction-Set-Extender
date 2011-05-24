#include "HallofFame.h"

namespace HallOfFame
{

	void __stdcall Initialize(bool ResetCSWindows)
	{
		UInt32 FormID = kBaseFormID;
		for (int i = 0; i < ListSize; i++)
		{
			Entry Itr = Entries[i];

			Itr.Form = FormHeap_Allocate(Itr.Size);
			thisCall(Itr.Ctor, Itr.Form);
			thisCall(kTESForm_SetFormID, Itr.Form, FormID, 1);
			thisCall(kTESForm_SetEditorID, Itr.Form, Itr.EditorID);

			if (!_stricmp("lilith", Itr.EditorID) ||
				!_stricmp("greenwarden", Itr.EditorID))
			{
				TESNPC* NPC = (TESNPC*)Itr.Form;
				ToggleFlag(&NPC->actorBaseData.flags, TESActorBaseData::kFlag_IsFemale, true);
			}
			else if (!_stricmp("DragoonWraith", Itr.EditorID))
			{
				TESObjectWEAP* Weapon = (TESObjectWEAP*)Itr.Form;
				UInt32 Type = (UInt32)Weapon + 0xDC;
				*(UInt32*)Type = TESObjectWEAP::kType_Staff;
			}

			TESFullName* Name = CS_CAST(Itr.Form, TESForm, TESFullName);
			if (Name)
//				Name->name.Set(Itr.Name);		### FIX

			thisCall(kDataHandler_AddBoundObject, (*g_dataHandler)->boundObjects, Itr.Form);
			thisVirtualCall(Itr.VTBL, 0x94, Itr.Form, 0);

			FormID++;
		}

		void* Throwaway = NULL;
		SpellItem* Spell = InitializeDefaultPlayerSpell(Throwaway);
	//	BSStringT* FullNamePtr = (BSStringT*)((UInt32)Spell + 0x28);		### FIX
	//	FullNamePtr->Set("Deadliest Smiley Alive");
		thisCall(kTESForm_SetFormID, Spell, FormID, 1);
		thisCall(kTESForm_SetEditorID, Spell, "kyoma");
		thisVirtualCall(kVTBL_SpellItem, 0x94, Spell, 0);
		thisCall(kLinkedListNode_NewNode, &(*g_dataHandler)->spellitems, Spell);


		// temporarily "killing" the dataHandler to prevent the TESForm ctor from assigning formIDs
		void* DataHandlerInstance = *g_dataHandler;
		*g_dataHandler = NULL;
		ConstructEffectSetting('HSRJ', "The Constant Physicist", 4, 0.0, 0, 0x170, -1, 1, 'LPSD');
		*g_dataHandler = (DataHandler*)DataHandlerInstance;

		TESObjectREFR* shadeMeRef = (TESObjectREFR*)FormHeap_Allocate(0x60);
		thisCall(kTESObjectREFR_Ctor, shadeMeRef);
		thisCall(kTESObjectREFR_SetBaseForm, shadeMeRef, TESForm_LookupByEditorID("shadeMe"));
		thisCall(kTESForm_SetFormID, shadeMeRef, 0x99, 1);
		thisCall(kTESForm_SetEditorID, shadeMeRef, "TheShadeMeRef");
		thisVirtualCall(kVTBL_TESObjectREFR, 0x94, shadeMeRef, 0);
		thisCall(kTESObjectREFR_SetFlagPersistent, shadeMeRef, 1);

		if (ResetCSWindows)
		{
			TESDialog_DeinitializeCSWindows();
			TESDialog_InitializeCSWindows();
		}
	}

}