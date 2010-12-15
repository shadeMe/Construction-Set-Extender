#pragma once
#include "ExtenderInternals.h"

// vanity would an understatement.

namespace HallOfFame
{
	const UInt32		kBaseFormID = 0x450;

	struct Entry
	{
		const char*		EditorID;
		UInt32			Ctor;
		UInt32			Size;
		UInt32			VTBL;
		void*			Form;
		const char*		Name;
	};

	const UInt32			ListSize = 15;
	const Entry				Entries[ListSize] =
	{	
		{ "shadeMe",					kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"Likes Andrea Corr" },
		{ "ScruggsywuggsyTheFerret",	kTESCreature_Ctor,		0x18C,	kVTBL_TESCreature, NULL,	"Cthulu-like Being" },
		{ "ianpat",						kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"The Overlord" },
		{ "behippo",					kTESCreature_Ctor,		0x18C,	kVTBL_TESCreature, NULL,	"Despises Peppermint" },
		{ "Cipscis",					kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"ACRONYM" },
		{ "haama",						kTESFurniture_Ctor,		0x9C,	kVTBL_TESFurniture, NULL,	"Think Tank. Not Literally" },
		{ "Waruddar",					kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"He-Whose-Name-I-Could-Never-Spell-Right" },
		{ "Corepc",						kTESObjectMISC_Ctor,	0xBC,	kVTBL_TESObjectMISC, NULL,	"Has Finally Changed His Avatar." },
		{ "DragoonWraith",				kTESObjectWEAP_Ctor,	0xEC,	kVTBL_TESObjectWEAP, NULL,	"The Mighty ARSE" },
		{ "PacificMorrowind",			kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"The Jack" },
		{ "tejon",						kTESCreature_Ctor,		0x18C,	kVTBL_TESCreature, NULL,	"Goblin Tinkerer" },
		{ "Vacuity",					kTESObjectCONT_Ctor,	0xBC,	kVTBL_TESObjectCONT, NULL,	"                " },
		{ "SenChan",					kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"Is A Year Older Now" },
		{ "lilith",						kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"Likes Tea. Black. Like Me" },
		{ "daemondarque",				kTESNPC_Ctor,			0x230,	kVTBL_TESNPC, NULL,			"His Majestry The CTD" }
	};

	void __stdcall				Initialize(bool ResetCSWindows);
}
