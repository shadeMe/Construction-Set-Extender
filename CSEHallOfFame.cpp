#include "CSEHallOfFame.h"

namespace ConstructionSetExtender
{
	namespace HallOfFame
	{
		CSEHallOfFameEntry::CSEHallOfFameEntry( const char* Name, const char* Title, UInt8 Type ) :
			BGSEditorExtender::Extras::BGSEEHallOfFameEntry(Name, Title),
			Instance(NULL),
			FormType(Type)
		{
			;//
		}

		CSEHallOfFameEntry::~CSEHallOfFameEntry()
		{
			Instance = NULL;
		}

		void CSEHallOfFameEntry::Initialize( UInt32 FormID )
		{
			Instance = TESForm::CreateInstance(FormType);
			Instance->SetFormID(FormID);
			Instance->SetEditorID(Name);

			TESFullName* Name = CS_CAST(Instance, TESForm, TESFullName);
			if (Name)
			{
				Name->name.Set(Title);
			}

			TESObject* Object = CS_CAST(Instance, TESForm, TESObject);
			SME_ASSERT(Object);

			_DATAHANDLER->AddTESObject(Object);
			Instance->SetFromActiveFile(false);
		}

		void CSEHallOfFameEntry::Deinitialize( void )
		{
			;// nothing to do here, as the datahandler frees the form on its own
		}

		UInt32 CSEHallOfFameEntry::GetFormID( void ) const
		{
			if (Instance)
				return Instance->formID;
			else
				return 0;
		}

		CSEHallOfFameStaff::CSEHallOfFameStaff( const char* Name, const char* Title ) :
			CSEHallOfFameEntry(Name, Title, TESForm::kFormType_Weapon)
		{
			;//
		}

		CSEHallOfFameStaff::~CSEHallOfFameStaff()
		{
			;//
		}

		void CSEHallOfFameStaff::Initialize( UInt32 FormID )
		{
			CSEHallOfFameEntry::Initialize(FormID);

			TESObjectWEAP* Weapon = CS_CAST(Instance, TESForm, TESObjectWEAP);
			SME_ASSERT(Weapon);

			Weapon->weaponType = TESObjectWEAP::kWeaponType_Staff;
		}

		CSEHallOfFameFemaleNPC::CSEHallOfFameFemaleNPC( const char* Name, const char* Title ) :
			CSEHallOfFameEntry(Name, Title, TESForm::kFormType_NPC)
		{
			;//
		}

		CSEHallOfFameFemaleNPC::~CSEHallOfFameFemaleNPC()
		{
			;//
		}

		void CSEHallOfFameFemaleNPC::Initialize( UInt32 FormID )
		{
			CSEHallOfFameEntry::Initialize(FormID);

			TESNPC* NPC = CS_CAST(Instance, TESForm, TESNPC);
			SME_ASSERT(NPC);

			SME::MiscGunk::ToggleFlag(&NPC->actorFlags, TESActorBaseData::kNPCFlag_Female, true);
		}

		CSEHallOfFameSpellItem::CSEHallOfFameSpellItem( const char* Name, const char* Title ) :
			CSEHallOfFameEntry(Name, Title, TESForm::kFormType_Spell)
		{
			;//
		}

		CSEHallOfFameSpellItem::~CSEHallOfFameSpellItem()
		{
			;//
		}

		void CSEHallOfFameSpellItem::Initialize( UInt32 FormID )
		{
			void* Throwaway = NULL;
			SpellItem* Spell = SpellItem::InitializeDefaultPlayerSpell(Throwaway);
			Spell->name.Set(Title);
			Spell->SetFormID(FormID);
			Spell->SetEditorID(Name);
			Spell->SetFromActiveFile(false);

			_DATAHANDLER->spellItems.AddAt(Spell, eListEnd);
		}

		CSEHallOfFameMGEF::CSEHallOfFameMGEF( UInt32 Name, const char* Title ) :
			CSEHallOfFameEntry((const char*)Name, Title, TESForm::kFormType_EffectSetting)
		{
			;//
		}

		CSEHallOfFameMGEF::~CSEHallOfFameMGEF()
		{
			;//
		}

		void CSEHallOfFameMGEF::Initialize( UInt32 FormID )
		{
			// temporarily "killing" the dataHandler to prevent the TESForm ctor from assigning formIDs
			void* DataHandlerInstance = _DATAHANDLER;
			_DATAHANDLER = NULL;
			EffectSetting::CreateInstance((UInt32)Name, Title, 4, 0.0, 0, 0x170, -1, 1, 'LPSD');
			_DATAHANDLER = (TESDataHandler*)DataHandlerInstance;
		}

		CSEHallOfFameShadeMe::CSEHallOfFameShadeMe() :
			CSEHallOfFameEntry("shadeMe", "Likes Andrea Corr", TESForm::kFormType_NPC),
			TheGreatEye(NULL)
		{
			;//
		}

		CSEHallOfFameShadeMe::~CSEHallOfFameShadeMe()
		{
			;//
		}

		void CSEHallOfFameShadeMe::Initialize( UInt32 FormID )
		{
			CSEHallOfFameEntry::Initialize(FormID);

			TESObjectREFR* shadeMeRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
			TheGreatEye = shadeMeRef;

			shadeMeRef->SetBaseForm(TESForm::LookupByEditorID(Name));
			shadeMeRef->SetFormID(0x99);
			shadeMeRef->SetEditorID("TheShadeMeRef");
			shadeMeRef->SetFromActiveFile(false);
			shadeMeRef->SetPersistent(true);
		}

		void CSEHallOfFameShadeMe::Deinitialize()
		{
			_DATAHANDLER->clearingData = 1;			// set the flag to allow default objects to be released
			TheGreatEye->DeleteInstance();
			TheGreatEye = NULL;
			_DATAHANDLER->clearingData = 0;
		}

		void Initialize( void )
		{
			if (TESForm::LookupByEditorID("TheShadeMeRef"))
				return;

			BGSEditorExtender::Extras::ExtenderHOFEntryListT Inductees;

			Inductees.push_back(new CSEHallOfFameEntry("ianpat",						"The Overlord",					TESForm::kFormType_NPC));
			Inductees.push_back(new CSEHallOfFameEntry("ScruggsywuggsyTheFerret",		"Cthulu-like Being",			TESForm::kFormType_Creature));
			Inductees.push_back(new CSEHallOfFameEntry("behippo",						"Despises Peppermint",			TESForm::kFormType_Creature));
			Inductees.push_back(new CSEHallOfFameEntry("Cipscis",						"ACRONYM",						TESForm::kFormType_NPC));
			Inductees.push_back(new CSEHallOfFameEntry("haama",							"Think Tank. Not Literally",	TESForm::kFormType_Furniture));
			Inductees.push_back(new CSEHallOfFameEntry("PacificMorrowind",				"The Jack",						TESForm::kFormType_NPC));
			Inductees.push_back(new CSEHallOfFameEntry("tejon",							"Goblin Tinkerer",				TESForm::kFormType_Creature));
			Inductees.push_back(new CSEHallOfFameEntry("Vacuity",						"                ",				TESForm::kFormType_Container));
			Inductees.push_back(new CSEHallOfFameEntry("daemondarque",					"His Majestry The CTD",			TESForm::kFormType_NPC));
			Inductees.push_back(new CSEHallOfFameEntry("Shikishima",					"Slash-And-Burn",				TESForm::kFormType_NPC));
			Inductees.push_back(new CSEHallOfFameEntry("Waruddar",						"He-Whose-Name-I-Could-Never-Spell-Right",
																														TESForm::kFormType_NPC));
			Inductees.push_back(new CSEHallOfFameEntry("Corepc",						"Has Finally Changed His Avatar",
																														TESForm::kFormType_Misc));
			Inductees.push_back(new CSEHallOfFameEntry("AndalayBay",					"Utters The Name Of The Scottish Play Without Flinching",
																														TESForm::kFormType_Door));
			Inductees.push_back(new CSEHallOfFameFemaleNPC("SenChan",					"Is A Year Older Now"));
			Inductees.push_back(new CSEHallOfFameFemaleNPC("lilith",					"Likes Tea. Black. Like Me"));
			Inductees.push_back(new CSEHallOfFameFemaleNPC("greenwarden",				"The Wise Old Woman of Putney"));
			Inductees.push_back(new CSEHallOfFameSpellItem("kyoma",						"Deadliest Smiley Alive"));
			Inductees.push_back(new CSEHallOfFameStaff("DragoonWraith",					"The Mighty ARSE"));
			Inductees.push_back(new CSEHallOfFameMGEF('HSRJ',							"The Constant Physicist"));

			Inductees.push_back(new CSEHallOfFameShadeMe());		// oh yeah!

			bool ComponentInitialized = BGSEEHALLOFFAME->Initialize(Inductees, 0x450);

			SME_ASSERT(ComponentInitialized);
		}
	}
}

