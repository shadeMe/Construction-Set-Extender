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

		CSEHallOfFameScript::CSEHallOfFameScript( const char* Name, const char* Code ) :
			CSEHallOfFameEntry(Name, Code, TESForm::kFormType_Script)
		{
			;//
		}

		CSEHallOfFameScript::~CSEHallOfFameScript()
		{
			;//
		}

		void CSEHallOfFameScript::Initialize( UInt32 FormID )
		{
			Script* Form = CS_CAST(TESForm::CreateInstance(FormType), TESForm, Script);

			Form->SetFormID(FormID);
			Form->SetEditorID(Name);
			Form->SetFromActiveFile(false);
			Form->SetText(Title);

			_DATAHANDLER->scripts.AddAt(Form, eListEnd);
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
			UInt8 Flag = _DATAHANDLER->clearingData;

			_DATAHANDLER->clearingData = 1;			// set the flag to allow default objects to be released
			TheGreatEye->DeleteInstance();
			TheGreatEye = NULL;
			_DATAHANDLER->clearingData = Flag;
		}

		void Initialize( void )
		{
			if (TESForm::LookupByEditorID("TheShadeMeRef"))
				return;

			BGSEditorExtender::Extras::ExtenderHOFEntryListT Inductees;

			Inductees.push_back(new CSEHallOfFameEntry("ianpat",						"The Overlord",					TESForm::kFormType_Armor));
			Inductees.push_back(new CSEHallOfFameEntry("ScruggsywuggsyTheFerret",		"Cthulu-like Being",			TESForm::kFormType_SigilStone));
			Inductees.push_back(new CSEHallOfFameEntry("behippo",						"Despises Peppermint",			TESForm::kFormType_Apparatus));
			Inductees.push_back(new CSEHallOfFameEntry("Cipscis",						"ACRONYM",						TESForm::kFormType_AlchemyItem));
			Inductees.push_back(new CSEHallOfFameEntry("haama",							"Tricksy Ol' Sod",				TESForm::kFormType_Furniture));
			Inductees.push_back(new CSEHallOfFameEntry("PacificMorrowind",				"The Jack",						TESForm::kFormType_Book));
			Inductees.push_back(new CSEHallOfFameEntry("tejon",							"Goblin Tinkerer",				TESForm::kFormType_Creature));
			Inductees.push_back(new CSEHallOfFameEntry("Lojack",						"Land Ahoy!",					TESForm::kFormType_Creature));
			Inductees.push_back(new CSEHallOfFameEntry("Vacuity",						"                ",				TESForm::kFormType_Container));
			Inductees.push_back(new CSEHallOfFameEntry("daemondarque",					"His Majestry, The CTD",		TESForm::kFormType_Ingredient));
			Inductees.push_back(new CSEHallOfFameEntry("Shikishima",					"Slash-And-Burn",				TESForm::kFormType_Clothing));
			Inductees.push_back(new CSEHallOfFameEntry("Waruddar",						"He-Whose-Name-I-Could-Never-Spell-Right",
																														TESForm::kFormType_Key));
			Inductees.push_back(new CSEHallOfFameEntry("Corepc",						"Has Finally Changed His Avatar",
																														TESForm::kFormType_Misc));
			Inductees.push_back(new CSEHallOfFameEntry("AndalayBay",					"Utters The Name Of The Scottish Play Without Flinching",
																														TESForm::kFormType_Door));
			Inductees.push_back(new CSEHallOfFameEntry("SenChan",						"Is A Year Older Now",			TESForm::kFormType_Ammo));
			Inductees.push_back(new CSEHallOfFameFemaleNPC("lilith",					"Likes Tea. Black. Like Me."));
			Inductees.push_back(new CSEHallOfFameFemaleNPC("greenwarden",				"The Wise Old Woman of Putney"));
			Inductees.push_back(new CSEHallOfFameSpellItem("kyoma",						"Deadliest Smiley Alive"));
			Inductees.push_back(new CSEHallOfFameStaff("DragoonWraith",					"The Mighty ARSE"));
			Inductees.push_back(new CSEHallOfFameMGEF('HSRJ',							"The Constant Physicist"));
			Inductees.push_back(new CSEHallOfFameEntry("Arthmoor",						"Bollocks-Breaker",				TESForm::kFormType_SoulGem));
			Inductees.push_back(new CSEHallOfFameEntry("Vorians",						"Mr. Aux Viewport",				TESForm::kFormType_Activator));
			Inductees.push_back(new CSEHallOfFameScript("QQuix",						"scn QQuix\n\nbegin function {}\n\tsetFunctionValue \"I was the one who brought up the idea for Coda, CSE's scripting language for the CS environment! Woohoo!!\"\nend"));

			Inductees.push_back(new CSEHallOfFameShadeMe());		// oh yeah!

			bool ComponentInitialized = BGSEEHALLOFFAME->Initialize(Inductees, 0x450);

			SME_ASSERT(ComponentInitialized);
		}

		void GetRandomESMember(std::string& OutName, bool Possessive)
		{
			static std::vector<std::string> kESMembers;
			if (kESMembers.size() == 0)
			{
				// was flitting through the official forums and the Nexus on a wave of nostalgia when the thought hit me
				// thanks for all the memories and fish(sticks)!
				kESMembers.push_back("The Elder Scrolls Modding Community");
				kESMembers.push_back("saebel");
				kESMembers.push_back("Arkngt X");
				kESMembers.push_back("gruftikus");
				kESMembers.push_back("migck");
				kESMembers.push_back("forli");
				kESMembers.push_back("dev_akm");
				kESMembers.push_back("hlp");
				kESMembers.push_back("zilav");
				kESMembers.push_back("Drake the Dragon");
				kESMembers.push_back("ElminsterEU");
				kESMembers.push_back("Ethatron");
				kESMembers.push_back("Galahaut");
				kESMembers.push_back("Reneer");
				kESMembers.push_back("HeX_0ff");
				kESMembers.push_back("Timeslip");
				kESMembers.push_back("IlmrynAkios");
				kESMembers.push_back("kuertee");
				kESMembers.push_back("LogicDragon");
				kESMembers.push_back("bg2408");
				kESMembers.push_back("Qazaaq");
				kESMembers.push_back("scanti");
				kESMembers.push_back("Skycaptain");
				kESMembers.push_back("SkyRanger-1");
				kESMembers.push_back("Tekuromoto");
				kESMembers.push_back("TheNiceOne");
				kESMembers.push_back("TheTalkieToaster");
				kESMembers.push_back("Wrye");
				kESMembers.push_back("AlienSlof");
				kESMembers.push_back("Saiden Storm");
				kESMembers.push_back("Fliggerty");
				kESMembers.push_back("Kivan");
				kESMembers.push_back("Quarn");
				kESMembers.push_back("Dark0ne");
				kESMembers.push_back("Trollf");
				kESMembers.push_back("Shezrie");
				kESMembers.push_back("VagabondAngel");
				kESMembers.push_back("wz_");
				kESMembers.push_back("PurpleLunchbox");
				kESMembers.push_back("snakster");
				kESMembers.push_back("Emma");
				kESMembers.push_back("WillieSea");
				kESMembers.push_back("WhoGuru");
				kESMembers.push_back("Buddah");
				kESMembers.push_back("Phitt");
				kESMembers.push_back("MentalElf");
				kESMembers.push_back("Nicoroshi");
				kESMembers.push_back("Mr. Siika");
				kESMembers.push_back("throttlekitty");
				kESMembers.push_back("Cipscis");
				kESMembers.push_back("DarN");
				kESMembers.push_back("xilver");
				kESMembers.push_back("Elys");
				kESMembers.push_back("nuska");
				kESMembers.push_back("mmmpld");
				kESMembers.push_back("Ismelda Lasombra");
				kESMembers.push_back("Jaga");
				kESMembers.push_back("Qarl");
				kESMembers.push_back("SomeWelshGuy");
				kESMembers.push_back("SirFrederik");
				kESMembers.push_back("Cliffworms");
				kESMembers.push_back("Strategy Master");
				kESMembers.push_back("Quazzy");
				kESMembers.push_back("LHammonds");
				kESMembers.push_back("reznod");
				kESMembers.push_back("Simyaz");
				kESMembers.push_back("Sen-chan");
				kESMembers.push_back("Speedo");
				kESMembers.push_back("Axil");
				kESMembers.push_back("Eloise Shadowborn");
				kESMembers.push_back("Martigen");
				kESMembers.push_back("StarX");
				kESMembers.push_back("Waalx");
				kESMembers.push_back("Showler");
				kESMembers.push_back("Sein_Schatten");
				kESMembers.push_back("Random007");
				kESMembers.push_back("Sotobrastos");
				kESMembers.push_back("Newcomer24");
				kESMembers.push_back("L@zarus");
				kESMembers.push_back("Mr. Dave");
				kESMembers.push_back("Loth DeBonneville");
				kESMembers.push_back("ScripterRon");
				kESMembers.push_back("KomodoDave");
				kESMembers.push_back("dDefinder");
				kESMembers.push_back("wolve");
				kESMembers.push_back("Steve Carrow");
				kESMembers.push_back("The10CentGamer");
				kESMembers.push_back("UK47");
				kESMembers.push_back("XMarksTheSpot");
				kESMembers.push_back("Khettienna");
				kESMembers.push_back("InsanitySorrow");
				kESMembers.push_back("Windom Earle");
				kESMembers.push_back("dewshine");
				kESMembers.push_back("HTF");
				kESMembers.push_back("vtastek");
				kESMembers.push_back("entim");
				kESMembers.push_back("qzilla");
				kESMembers.push_back("Tibixie");
				kESMembers.push_back("Exnem");
				kESMembers.push_back("Hel Borne");
				kESMembers.push_back("Deathless Aphrodite");
				kESMembers.push_back("flyfightflea");
				kESMembers.push_back("Jog");
				kESMembers.push_back("Darkrider");
				kESMembers.push_back("Texian");
				kESMembers.push_back("Khugan");
				kESMembers.push_back("Lady Nerevar");
				kESMembers.push_back("ghastley");
				kESMembers.push_back("Vality");
				kESMembers.push_back("Hrnchamd");
				kESMembers.push_back("Lanceor");
				kESMembers.push_back("The SureAI Team");
				kESMembers.push_back("Ed_Conn");
				kESMembers.push_back("David Brasher");
				kESMembers.push_back("vurt");
				kESMembers.push_back("HeyYou");
				kESMembers.push_back("jonwd7");
				kESMembers.push_back("Zaldiir");
				kESMembers.push_back("Vagrant0");
				kESMembers.push_back("Maegfaer");
				kESMembers.push_back("Sigurd Stormhand");
				kESMembers.push_back("Todd Howard");
			}

			srand(GetTickCount());
			int BadRand = (double)rand() / (RAND_MAX + 1) * kESMembers.size();
			SME_ASSERT(BadRand >= 0 && BadRand < kESMembers.size());

			OutName = kESMembers.at(BadRand);
			if (OutName[OutName.length() - 1] == ' ')
				OutName.erase(OutName.length() - 1);
			if (OutName[0] == ' ')
				OutName.erase(0, 1);

			if (Possessive)
			{
				if (OutName[OutName.length() - 1] == 's' || OutName[OutName.length() - 1] == 'S')
					OutName += "'";
				else
					OutName += "'s";
			}
		}
	}
}