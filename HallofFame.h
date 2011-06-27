#pragma once
#include "ExtenderInternals.h"

// vanity would an understatement.
using namespace SME::MemoryHandler;

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

	const UInt32			ListSize = 16;
	const Entry				Entries[ListSize] =
	{	
		{ "shadeMe",					kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"Likes Andrea Corr" },
		{ "ScruggsywuggsyTheFerret",	kCtor_TESCreature,		0x18C,	kVTBL_TESCreature, NULL,	"Cthulu-like Being" },
		{ "ianpat",						kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"The Overlord" },
		{ "behippo",					kCtor_TESCreature,		0x18C,	kVTBL_TESCreature, NULL,	"Despises Peppermint" },
		{ "Cipscis",					kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"ACRONYM" },
		{ "haama",						kCtor_TESFurniture,		0x9C,	kVTBL_TESFurniture, NULL,	"Think Tank. Not Literally" },
		{ "Waruddar",					kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"He-Whose-Name-I-Could-Never-Spell-Right" },
		{ "Corepc",						kCtor_TESObjectMISC,	0xBC,	kVTBL_TESObjectMISC, NULL,	"Has Finally Changed His Avatar." },
		{ "DragoonWraith",				kCtor_TESObjectWEAP,	0xEC,	kVTBL_TESObjectWEAP, NULL,	"The Mighty ARSE" },
		{ "PacificMorrowind",			kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"The Jack" },
		{ "tejon",						kCtor_TESCreature,		0x18C,	kVTBL_TESCreature, NULL,	"Goblin Tinkerer" },
		{ "Vacuity",					kCtor_TESObjectCONT,	0xBC,	kVTBL_TESObjectCONT, NULL,	"                " },
		{ "SenChan",					kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"Is A Year Older Now" },
		{ "lilith",						kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"Likes Tea. Black. Like Me" },
		{ "daemondarque",				kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"His Majestry The CTD" },
		{ "greenwarden",				kCtor_TESNPC,			0x230,	kVTBL_TESNPC, NULL,			"The Wise Old Woman of Putney" }
	};

	void __stdcall				Initialize(bool ResetCSWindows);
}
