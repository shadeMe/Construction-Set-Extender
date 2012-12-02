#include "CSEAchievements.h"
#include "Construction Set Extender_Resource.h"

namespace ConstructionSetExtender
{
	namespace Achievements
	{
		BGSEditorExtender::Extras::BGSEEAchievement*		CSEAchievementBase::AllClearAchievement = NULL;

		bool CSEAchievementBase::UnlockCallback( BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter )
		{
			if (Parameter->GetTotalAchievements() - 1 == Parameter->GetUnlockedAchievements())
			{
				Parameter->Unlock(AllClearAchievement, false, true);
			}

			return true;
		}

		CSEAchievementBase::CSEAchievementBase( const char* Name, const char* Desc, UInt32 IconID, const char* GUID ) :
			BGSEditorExtender::Extras::BGSEEAchievement(Name, Desc, IconID, GUID)
		{
			;//
		}

		CSEAchievementBase::~CSEAchievementBase()
		{
			;//
		}

		const UInt32	CSEAchievementTimeLapsed::kIdleTimeOut = 30 * 1000;

		void CSEAchievementTimeLapsed::ResetTimer( void )
		{
			if (TimerID)
			{
				KillTimer(NULL, TimerID);
				TimerID = 0;
			}
		}

		CSEAchievementTimeLapsed::CSEAchievementTimeLapsed( const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdHours ) :
			CSEAchievementBase(Name, Desc, IconID, GUID),
			TimerID(0),
			InitTickCount(GetTickCount()),
			ElapsedTicks(0),
			HoursRequired(ReqdHours)
		{
			;//
		}

		CSEAchievementTimeLapsed::~CSEAchievementTimeLapsed()
		{
			ResetTimer();
		}

		bool CSEAchievementTimeLapsed::GetIsIdling( void )
		{
			LASTINPUTINFO InputInfo = {0};
			InputInfo.cbSize = sizeof(LASTINPUTINFO);

			GetLastInputInfo(&InputInfo);
			UInt32 IdleTime = GetTickCount() - InputInfo.dwTime;

			if (IdleTime >= kIdleTimeOut || GetActiveWindow() == NULL)
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		CSEAchievementCheat*	CSEAchievementCheat::Singleton = NULL;

		VOID CALLBACK CSEAchievementCheat::TimerCallback( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
		{
			if (CSEAchievementCheat::GetSingleton()->GetIsIdling())
			{
				CSEAchievementCheat::GetSingleton()->InitTickCount = dwTime;
				return;
			}

			CSEAchievementCheat::GetSingleton()->ElapsedTicks += dwTime - CSEAchievementCheat::GetSingleton()->InitTickCount;
			CSEAchievementCheat::GetSingleton()->InitTickCount = dwTime;

			if (CSEAchievementCheat::GetSingleton()->ElapsedTicks / (3600 * 1000) >= CSEAchievementCheat::GetSingleton()->HoursRequired)
			{
				BGSEEACHIEVEMENTS->Unlock(CSEAchievementCheat::GetSingleton());
			}
		}

		CSEAchievementCheat::CSEAchievementCheat() :
			CSEAchievementTimeLapsed("Cheat",
									"Managed to complete a 6 hour long CS session without any CTDs",
									IDB_ACHIEVEMENT_CHEAT,
									"0F7F9D05-9679-4E75-9AE3-0B419E6C813A",
									6)
		{
			TimerID = SetTimer(NULL, NULL, 60 * 1000, TimerCallback);
			SME_ASSERT(TimerID);
		}

		CSEAchievementCheat::~CSEAchievementCheat()
		{
			Singleton = NULL;
		}

		CSEAchievementCheat* CSEAchievementCheat::GetSingleton()
		{
			if (Singleton == NULL)
				Singleton = new CSEAchievementCheat();

			return Singleton;
		}

		CSEAchievementLost*		CSEAchievementLost::Singleton = NULL;

		VOID CALLBACK CSEAchievementLost::TimerCallback( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
		{
			if (CSEAchievementLost::GetSingleton()->GetIsIdling())
			{
				CSEAchievementLost::GetSingleton()->InitTickCount = dwTime;
				return;
			}

			CSEAchievementLost::GetSingleton()->ElapsedTicks += dwTime - CSEAchievementLost::GetSingleton()->InitTickCount;
			CSEAchievementLost::GetSingleton()->InitTickCount = dwTime;

			if ((CSEAchievementLost::GetSingleton()->ExtraData + CSEAchievementLost::GetSingleton()->ElapsedTicks) / (3600 * 1000) >=
				CSEAchievementLost::GetSingleton()->HoursRequired)
			{
				BGSEEACHIEVEMENTS->Unlock(CSEAchievementLost::GetSingleton());
			}
		}

		CSEAchievementLost::CSEAchievementLost( UInt32 ReqdHours ) :
			CSEAchievementTimeLapsed("Lost",
									"You spend far too much time with the editor, man! Go do something else; watch telly, take a dump, walk the dog - Anything!",
									IDB_ACHIEVEMENT_LOST,
									"40AB6814-1B55-4906-A564-74E9E37BC2EC",
									ReqdHours)
		{
			TimerID = SetTimer(NULL, NULL, 60 * 1000, TimerCallback);
			SME_ASSERT(TimerID);
		}

		CSEAchievementLost::~CSEAchievementLost()
		{
			Singleton = NULL;
		}

		CSEAchievementLost* CSEAchievementLost::GetSingleton()
		{
			if (Singleton == NULL)
				Singleton = new CSEAchievementLost(666);

			return Singleton;
		}

		float CSEAchievementLost::GetLoggedHours( void ) const
		{
			return ExtraData / (3600.0 * 1000.0);
		}

		bool CSEAchievementLost::SaveCallback( BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter )
		{
			ExtraData += ElapsedTicks;

			return true;
		}

		bool CSEAchievementIncremented::UnlockCallback( BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter )
		{
			if (++CurrentCount >= CountRequired)
				return CSEAchievementBase::UnlockCallback(Parameter);
			else
				return false;
		}

		CSEAchievementIncremented::CSEAchievementIncremented( const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdCount ) :
			CSEAchievementBase(Name, Desc, IconID, GUID),
			CountRequired(ReqdCount),
			CurrentCount(0)
		{
			;//
		}

		CSEAchievementIncremented::~CSEAchievementIncremented()
		{
			;//
		}

		CSEAchievementBase*			kTheWiseOne				= NULL;
		CSEAchievementBase*			kFearless				= NULL;
		CSEAchievementBase*			kAutomaton				= NULL;
		CSEAchievementBase*			kHeretic				= NULL;
		CSEAchievementIncremented*  kPowerUser				= NULL;
		CSEAchievementBase*  		kAntiChrist				= NULL;
		CSEAchievementBase*  		kLazyBum				= NULL;
		CSEAchievementBase*  		kMadScientist			= NULL;
		CSEAchievementBase*  		kSoprano				= NULL;
		CSEAchievementBase* 		kWTF					= NULL;
		CSEAchievementBase*  		kFlyingBlind			= NULL;
		CSEAchievementBase*  		kPedantic				= NULL;
		CSEAchievementBase*  		kCardinalSin			= NULL;
		CSEAchievementBase*  		kMagister				= NULL;
		CSEAchievementIncremented*  kCommandant				= NULL;
		CSEAchievementBase*  		kCompartmentalizer		= NULL;
		CSEAchievementBase* 		kEruditeModder			= NULL;
		CSEAchievementBase*  		kBobTheBuilder			= NULL;
		CSEAchievementIncremented*  kLoquacious				= NULL;
		CSEAchievementBase*  		kSaboteur				= NULL;
		CSEAchievementBase*  		kOldestTrickInTheBook	= NULL;
		CSEAchievementBase*  		kOver3000				= NULL;
		CSEAchievementBase*  		kFunnyGuy				= NULL;

		void Initialize()
		{
			CSEAchievementBase::AllClearAchievement = new BGSEditorExtender::Extras::BGSEEAchievement("Totally Jobless", "Collected all achievements",
																									IDB_ACHIEVEMENT_TOTALLYJOBLESS, "B43425BE-323A-42C5-90ED-DE9CE014D842");

			kTheWiseOne				= new CSEAchievementBase("The Wise One", "Installed the Construction Set Extender",
															IDB_ACHIEVEMENT_THEWISEONE, "F6B59632-FD01-4762-866C-25A91AB5157D");

			kFearless				= new CSEAchievementBase("Fearless", "Set Oblivion.esm as the active file and hit the 'OK' button",
															IDB_ACHIEVEMENT_FEARLESS, "66DC462C-46E6-438D-8F3C-0DBFB1FD8889");

			kAutomaton				= new CSEAchievementBase("Automaton", "Executed a Coda Script",
															IDB_ACHIEVEMENT_AUTOMATON, "2E76F5E4-EC16-43C9-8DD6-5EAA62A5AEEC");

			kHeretic				= new CSEAchievementBase("Heretic", "Defiled the awesomeness of our Lord shadeMe",
															IDB_ACHIEVEMENT_HERECTIC, "7C70C6BB-5872-43DF-BD2C-1362081C2379");

			kPowerUser				= new CSEAchievementIncremented("Power User", "Sampled much of the new tools CSE adds",
															IDB_ACHIEVEMENT_POWERUSER, "1F87BA7E-317F-4252-9D03-67A51F6A4E24", 400);

			kAntiChrist				= new CSEAchievementBase("Anti-Christ", "Attempted to remove our Lord shadeMe from this plane of existence",
															IDB_ACHIEVEMENT_ANTICHRIST, "5ECDC14F-FE88-49AD-8F3D-BF761E95D527");

			kLazyBum				= new CSEAchievementBase("Lazy Bum", "Launched the game from the main menu",
															IDB_ACHIEVEMENT_LAZYBUM, "49C3EE3D-B073-4473-8FAB-2C249CE77D2F");

			kMadScientist			= new CSEAchievementBase("Mad Scientist", "Cloned one of the New Age Divines",
															IDB_ACHIEVEMENT_MADSCIENTIST, "71B4F945-3185-4D8F-96B8-7D0C0BF8C052");

			kSoprano				= new CSEAchievementBase("Soprano", "Successfully generated a lip synch file",
															IDB_ACHIEVEMENT_SOPRANO, "61115B4A-0866-4420-886C-4BEE59DBEE70");

			kWTF					= new CSEAchievementBase("WTF!", "Managed to trigger an assertion in the editor code",
															IDB_ACHIEVEMENT_WTF, "992F82BE-4DDD-40CA-AD96-153353FDDA35");

			kFlyingBlind			= new CSEAchievementBase("Flying Blind", "Disabled editor warning message logging",
															IDB_ACHIEVEMENT_FLYINGBLIND, "51721B2C-DB4F-42C0-B4F4-38154F32DF92");

			kPedantic				= new CSEAchievementBase("Pedantic", "Saved version information to a plugin's description",
															IDB_ACHIEVEMENT_PEDANTIC, "DB3F67DE-60F8-4CA9-928A-044AB83119B2");

			kCardinalSin			= new CSEAchievementBase("Cardinal Sin", "Used the Added Topics list with the GREETING topic",
															IDB_ACHIEVEMENT_CARDINALSIN, "0D76A162-B749-4F20-9570-853AECEA003C");

			kMagister				= new CSEAchievementBase("Magister", "Modified a magic effect",
															IDB_ACHIEVEMENT_MAGISTER, "B2E9FB5D-54FD-4759-B981-DA129B859CA2");

			kCommandant				= new CSEAchievementIncremented("Commandant", "Registered a metric [censored] tonne of script commands",
															IDB_ACHIEVEMENT_COMMANDANT, "13C90CC7-D73A-4F2C-B6AF-982FDE29EC84", 2400);

			kCompartmentalizer		= new CSEAchievementBase("CompartmentalizeR", "Used an editor workspace other than the default",
															IDB_ACHIEVEMENT_COMPARTMENTALIZER, "5BD1463C-EC15-4C3C-A357-26F0F51E3E38");

			kEruditeModder			= new CSEAchievementBase("Erudite Modder", "Remembered to read the ReadMe at least once",
															IDB_ACHIEVEMENT_ERUDITEMODDER, "56385249-C3D8-4D1C-A70E-EC9B05A4338F");

			kBobTheBuilder			= new CSEAchievementBase("Bob The Builder", "Laid roads for a worldspace",
															IDB_ACHIEVEMENT_BOBTHEBUILDER, "10A0F510-062B-4E21-A517-B04CC28D68A7");

			kLoquacious				= new CSEAchievementIncremented("Loquacious", "Created a LOT of dialog responses in a single CS session",
															IDB_ACHIEVEMENT_LOQUACIOUS, "0E0C034F-0E4E-4709-A07D-D79059846841", 100);

			kSaboteur				= new CSEAchievementBase("Saboteur", "Managed to crash the extended CS!",
															IDB_ACHIEVEMENT_SABOTEUR, "F2E645A0-2503-4FFC-87A0-E6462325EBB8");

			kOldestTrickInTheBook	= new CSEAchievementBase("Oldest Trick In The Book", "\"Never Gonna Give You Up, Never Gonna Let You Down!\"",
															IDB_ACHIEVEMENT_OLDESTTRICK, "51602A18-037E-4FDC-88A8-B5F472D2026F");

			kOver3000				= new CSEAchievementBase("Over 3000!", "OMFGodrays! RAM usage of 3 gigs?! It's gonna blow!!",
															IDB_ACHIEVEMENT_OVER3000, "17C0EA7A-DB0C-46BA-A19F-232A73630AD5");

			kFunnyGuy				= new CSEAchievementBase("Funny Guy Bob", "How lovely! You've got a sense of humour",
															IDB_ACHIEVEMENT_FUNNYGUY, "13275AC0-DA3B-4802-813A-4DB3E7C4168D");

			BGSEditorExtender::Extras::ExtenderAchievementListT AchievementDepot;

			AchievementDepot.push_back(CSEAchievementBase::AllClearAchievement);

			AchievementDepot.push_back(kTheWiseOne);
			AchievementDepot.push_back(kFearless);
			AchievementDepot.push_back(kAutomaton);
			AchievementDepot.push_back(kHeretic);
			AchievementDepot.push_back(kPowerUser);
			AchievementDepot.push_back(kAntiChrist);
			AchievementDepot.push_back(kLazyBum);
			AchievementDepot.push_back(kMadScientist);
			AchievementDepot.push_back(kSoprano);
			AchievementDepot.push_back(kWTF);
			AchievementDepot.push_back(kFlyingBlind);
			AchievementDepot.push_back(kPedantic);
			AchievementDepot.push_back(kCardinalSin);
			AchievementDepot.push_back(kMagister);
			AchievementDepot.push_back(kCommandant);
			AchievementDepot.push_back(kCompartmentalizer);
			AchievementDepot.push_back(kEruditeModder);
			AchievementDepot.push_back(kBobTheBuilder);
			AchievementDepot.push_back(kLoquacious);
			AchievementDepot.push_back(kSaboteur);
			AchievementDepot.push_back(CSEAchievementCheat::GetSingleton());
			AchievementDepot.push_back(CSEAchievementLost::GetSingleton());
			AchievementDepot.push_back(kOldestTrickInTheBook);
			AchievementDepot.push_back(kOver3000);
			AchievementDepot.push_back(kFunnyGuy);

			bool ComponentInitialized = BGSEEACHIEVEMENTS->Initialize(BGSEEMAIN->ExtenderGetLongName(), BGSEEMAIN->GetExtenderHandle(), AchievementDepot);

			SME_ASSERT(ComponentInitialized);

			BGSEECONSOLE->Pad(1);
			BGSEECONSOLE_MESSAGE("Precious time wasted on the CS: %0.1f hours", CSEAchievementLost::GetSingleton()->GetLoggedHours());
		}
	}
}