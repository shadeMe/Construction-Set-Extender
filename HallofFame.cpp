#include "HallofFame.h"

using namespace SME::MemoryHandler;

namespace HallOfFame
{
	#define		TableSize		17
	HallOfFameEntry	Entries[TableSize] =
	{
		{ "shadeMe",					TESForm::kFormType_NPC,			NULL,	"Likes Andrea Corr" },
		{ "ScruggsywuggsyTheFerret",	TESForm::kFormType_Creature,	NULL,	"Cthulu-like Being" },
		{ "ianpat",						TESForm::kFormType_NPC,			NULL,	"The Overlord" },
		{ "behippo",					TESForm::kFormType_Creature,	NULL,	"Despises Peppermint" },
		{ "Cipscis",					TESForm::kFormType_NPC,			NULL,	"ACRONYM" },
		{ "haama",						TESForm::kFormType_Furniture,	NULL,	"Think Tank. Not Literally" },
		{ "Waruddar",					TESForm::kFormType_NPC,			NULL,	"He-Whose-Name-I-Could-Never-Spell-Right" },
		{ "Corepc",						TESForm::kFormType_Misc,		NULL,	"Has Finally Changed His Avatar." },
		{ "DragoonWraith",				TESForm::kFormType_Weapon,		NULL,	"The Mighty ARSE" },
		{ "PacificMorrowind",			TESForm::kFormType_NPC,			NULL,	"The Jack" },
		{ "tejon",						TESForm::kFormType_Creature,	NULL,	"Goblin Tinkerer" },
		{ "Vacuity",					TESForm::kFormType_Container,	NULL,	"                " },
		{ "SenChan",					TESForm::kFormType_NPC,			NULL,	"Is A Year Older Now" },
		{ "lilith",						TESForm::kFormType_NPC,			NULL,	"Likes Tea. Black. Like Me" },
		{ "daemondarque",				TESForm::kFormType_NPC,			NULL,	"His Majestry The CTD" },
		{ "greenwarden",				TESForm::kFormType_NPC,			NULL,	"The Wise Old Woman of Putney" },
		{ "AndalayBay",					TESForm::kFormType_Door,		NULL,	"Utters The Name Of The Scottish Play Without Flinching" }
	};

	void __stdcall Initialize(bool ResetCSWindows)
	{
		UInt32 FormID = kBaseFormID;
		for (int i = 0; i < TableSize; i++)
		{
			HallOfFameEntry& Itr = Entries[i];

			Itr.Form = TESForm::CreateInstance(Itr.FormType);
			Itr.Form->SetFormID(FormID);
			Itr.Form->SetEditorID(Itr.EditorID);

			if (!_stricmp("lilith", Itr.EditorID) ||
				!_stricmp("greenwarden", Itr.EditorID))
			{
				TESNPC* NPC = CS_CAST(Itr.Form, TESForm, TESNPC);
				ToggleFlag(&NPC->actorFlags, TESActorBaseData::kNPCFlag_Female, true);
			}
			else if (!_stricmp("DragoonWraith", Itr.EditorID))
			{
				TESObjectWEAP* Weapon = CS_CAST(Itr.Form, TESForm, TESObjectWEAP);
				Weapon->weaponType = TESObjectWEAP::kWeaponType_Staff;
			}

			TESFullName* Name = CS_CAST(Itr.Form, TESForm, TESFullName);
			if (Name)			Name->name.Set(Itr.Name);

			_DATAHANDLER->AddTESObject(CS_CAST(Itr.Form, TESForm, TESObject));
			Itr.Form->SetFromActiveFile(false);

			FormID++;
		}

		void* Throwaway = NULL;
		SpellItem* Spell = SpellItem::InitializeDefaultPlayerSpell(Throwaway);
		Spell->name.Set("Deadliest Smiley Alive");
		Spell->SetFormID(FormID);
		Spell->SetEditorID("kyoma");
		Spell->SetFromActiveFile(false);
		_DATAHANDLER->spellItems.AddAt(Spell, eListEnd);

		// temporarily "killing" the dataHandler to prevent the TESForm ctor from assigning formIDs
		void* DataHandlerInstance = *g_TESDataHandler;
		_DATAHANDLER = NULL;
		EffectItem::CreateEffectSetting('HSRJ', "The Constant Physicist", 4, 0.0, 0, 0x170, -1, 1, 'LPSD');
		_DATAHANDLER = (TESDataHandler*)DataHandlerInstance;

		TESObjectREFR* shadeMeRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
		shadeMeRef->SetBaseForm(TESForm::LookupByEditorID("shadeMe"));
		shadeMeRef->SetFormID(0x99);
		shadeMeRef->SetEditorID("TheShadeMeRef");
		shadeMeRef->SetFromActiveFile(false);
		shadeMeRef->SetPersistent(true);

		if (ResetCSWindows)
		{
			TESDialog::DeinitializeCSWindows();
			TESDialog::InitializeCSWindows();
		}
	}
}