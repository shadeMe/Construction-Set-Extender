#include "Achievements.h"
#include "Construction Set Extender_Resource.h"

namespace cse
{
	namespace achievements
	{
		bgsee::extras::Achievement*		AchievementBase::AllClearAchievement = NULL;

		bool AchievementBase::UnlockCallback( bgsee::extras::AchievementManager* Parameter )
		{
			if (Parameter->GetTotalAchievements() - 1 == Parameter->GetUnlockedAchievements())
				Parameter->Unlock(AllClearAchievement, false, true);

			return true;
		}

		AchievementBase::AchievementBase( const char* Name, const char* Desc, UInt32 IconID, const char* GUID ) :
			bgsee::extras::Achievement(Name, Desc, IconID, GUID)
		{
			;//
		}

		AchievementBase::~AchievementBase()
		{
			;//
		}

		const UInt32	AchievementTimeLapsed::kIdleTimeOut = 30 * 1000;

		void AchievementTimeLapsed::ResetTimer( void )
		{
			if (TimerID)
			{
				KillTimer(NULL, TimerID);
				TimerID = 0;
			}
		}

		AchievementTimeLapsed::AchievementTimeLapsed( const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdHours ) :
			AchievementBase(Name, Desc, IconID, GUID),
			TimerID(0),
			InitTickCount(GetTickCount()),
			ElapsedTicks(0),
			HoursRequired(ReqdHours)
		{
			;//
		}

		AchievementTimeLapsed::~AchievementTimeLapsed()
		{
			ResetTimer();
		}

		bool AchievementTimeLapsed::GetIsIdling( void )
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

		AchievementCheat*	AchievementCheat::Singleton = NULL;

		VOID CALLBACK AchievementCheat::TimerCallback( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
		{
			if (AchievementCheat::GetSingleton()->GetIsIdling())
			{
				AchievementCheat::GetSingleton()->InitTickCount = dwTime;
				return;
			}

			AchievementCheat::GetSingleton()->ElapsedTicks += dwTime - AchievementCheat::GetSingleton()->InitTickCount;
			AchievementCheat::GetSingleton()->InitTickCount = dwTime;

			if (AchievementCheat::GetSingleton()->ElapsedTicks / (3600 * 1000) >= AchievementCheat::GetSingleton()->HoursRequired)
				BGSEEACHIEVEMENTS->Unlock(AchievementCheat::GetSingleton());
		}

		AchievementCheat::AchievementCheat() :
			AchievementTimeLapsed("Cheat",
									"Managed to complete a 6 hour long CS session without any CTDs",
									IDB_ACHIEVEMENT_CHEAT,
									"0F7F9D05-9679-4E75-9AE3-0B419E6C813A",
									6)
		{
			TimerID = SetTimer(NULL, NULL, 60 * 1000, TimerCallback);
			SME_ASSERT(TimerID);
		}

		AchievementCheat::~AchievementCheat()
		{
			Singleton = NULL;
		}

		AchievementCheat* AchievementCheat::GetSingleton()
		{
			if (Singleton == NULL)
				Singleton = new AchievementCheat();

			return Singleton;
		}

		AchievementLost*		AchievementLost::Singleton = NULL;

		VOID CALLBACK AchievementLost::TimerCallback( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime )
		{
			if (AchievementLost::GetSingleton()->GetIsIdling())
			{
				AchievementLost::GetSingleton()->InitTickCount = dwTime;
				return;
			}

			AchievementLost::GetSingleton()->ElapsedTicks += dwTime - AchievementLost::GetSingleton()->InitTickCount;
			AchievementLost::GetSingleton()->InitTickCount = dwTime;

			if ((AchievementLost::GetSingleton()->ExtraData + AchievementLost::GetSingleton()->ElapsedTicks) / (3600 * 1000) >=
				AchievementLost::GetSingleton()->HoursRequired)
			{
				BGSEEACHIEVEMENTS->Unlock(AchievementLost::GetSingleton());
			}
		}

		AchievementLost::AchievementLost( UInt32 ReqdHours ) :
			AchievementTimeLapsed("Lost",
									"You spend far too much time with the editor, man! Go do something else; watch telly, take a dump, walk the dog - Anything!",
									IDB_ACHIEVEMENT_LOST,
									"40AB6814-1B55-4906-A564-74E9E37BC2EC",
									ReqdHours)
		{
			TimerID = SetTimer(NULL, NULL, 60 * 1000, TimerCallback);
			SME_ASSERT(TimerID);
		}

		AchievementLost::~AchievementLost()
		{
			Singleton = NULL;
		}

		AchievementLost* AchievementLost::GetSingleton()
		{
			if (Singleton == NULL)
				Singleton = new AchievementLost(666);

			return Singleton;
		}

		float AchievementLost::GetLoggedHours( void ) const
		{
			return ExtraData / (3600.0 * 1000.0);
		}

		bool AchievementLost::SaveCallback( bgsee::extras::AchievementManager* Parameter )
		{
			ExtraData += ElapsedTicks;

			return true;
		}

		bool AchievementIncremented::UnlockCallback( bgsee::extras::AchievementManager* Parameter )
		{
			if (++CurrentCount >= CountRequired)
				return AchievementBase::UnlockCallback(Parameter);
			else
				return false;
		}

		AchievementIncremented::AchievementIncremented( const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdCount ) :
			AchievementBase(Name, Desc, IconID, GUID),
			CountRequired(ReqdCount),
			CurrentCount(0)
		{
			;//
		}

		AchievementIncremented::~AchievementIncremented()
		{
			;//
		}

		bool AchievementTimeTriggered::UnlockCallback( bgsee::extras::AchievementManager* Parameter )
		{
			time_t CurrentTime = time(NULL);
			tm Now = {0};

			if (localtime_s(&Now, &CurrentTime))
				return false;

			if (Day == Now.tm_mday && Month == Now.tm_mon + 1 && (Year == 0 || Year == Now.tm_year + 1900))
			{
				if (Year == 0 || Year == Now.tm_year)
				{
					if (ExtraData)
						return false;
					else
						ExtraData = 1;		// prevents multiple unlocks during the same day
				}

				return true;
			}
			else
			{
				ExtraData = 0;
				return false;
			}
		}

		AchievementTimeTriggered::AchievementTimeTriggered( const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt8 EventDay, UInt8 EventMonth, UInt16 EventYear /*= 0*/ ) :
			AchievementBase(Name, Desc, IconID, GUID),
			Day(EventDay),
			Month(EventMonth),
			Year(EventYear)
		{
			;//
		}

		AchievementTimeTriggered::~AchievementTimeTriggered()
		{
			;//
		}

		bool AchievementTimeTriggered::SaveCallback( bgsee::extras::AchievementManager* Parameter )
		{
			if (Year == 0)
			{
				// repeatable events can't be unlocked in the traditional sense
				State = kState_Locked;
			}

			return true;
		}

		bool AchievementTimeTriggered::GetUnlockable( void ) const
		{
			if (Year == 0)			// don't unlock if repeatable
				return false;
			else
				return true;
		}

		AchievementHappypotamus::AchievementHappypotamus( const char* Name, const char* Desc, const char* GUID, UInt8 EventDay, UInt8 EventMonth, UInt16 EventYear ) :
			AchievementTimeTriggered(Name, Desc, IDB_ACHIEVEMENT_CAKE, GUID, EventDay, EventMonth, 0),
			GeborenJahr(EventYear)
		{
			;//
		}

		AchievementHappypotamus::~AchievementHappypotamus()
		{
			;//
		}

		void AchievementHappypotamus::GetName( std::string& OutBuffer ) const
		{
			time_t CurrentTime = time(NULL);
			tm Now = {0};

			if (localtime_s(&Now, &CurrentTime))
				OutBuffer = "Happy B'day, " + Name + "!";
			else
			{
				char Buffer[0x200] = {0};
				FORMAT_STR(Buffer, "%u years on and still alive! Go %s!", Now.tm_year + 1900 - GeborenJahr, Name.c_str());
				OutBuffer = Buffer;
			}
		}

		bool AchievementHappypotamus::GetUnlockable(void) const
		{
			return false;
		}

		UInt8 AchievementPowerUser::GetUnlockedToolCount() const
		{
			UINT8 Count = 0;
			for (int i = kTool__BEING + 1; i < kTool__MAX; i++)
			{
				if ((ExtraData & (UInt64)(1 << i)))
					Count++;
			}

			return Count;
		}

		AchievementPowerUser::AchievementPowerUser(UInt8 Threshold) :
			AchievementBase("Power User",
			"Sampled most of the new tools the CSE adds",
			IDB_ACHIEVEMENT_POWERUSER,
			"1F87BA7E-317F-4252-9D03-67A51F6A4E24"),
			ThresholdCount(Threshold)
		{
			SME_ASSERT(ThresholdCount > 0 && ThresholdCount <= kTool__MAX);
			SME_ASSERT(kTool__MAX < 64);
		}

		AchievementPowerUser::~AchievementPowerUser()
		{
			;//
		}

		void AchievementPowerUser::UnlockTool(UInt8 Tool)
		{
			SME_ASSERT(Tool > kTool__BEING && Tool < kTool__MAX);

			ExtraData |= (UInt64)(1 << Tool);
			if (GetUnlockedToolCount() > ThresholdCount)
			{
				if (GetUnlocked() == false)
					BGSEEACHIEVEMENTS->Unlock(this);
			}
		}

		AchievementBase*			kTheWiseOne				= NULL;
		AchievementBase*			kFearless				= NULL;
		AchievementBase*			kAutomaton				= NULL;
		AchievementBase*			kHeretic				= NULL;
		AchievementPowerUser*	kPowerUser				= NULL;
		AchievementBase*  		kAntiChrist				= NULL;
		AchievementBase*  		kLazyBum				= NULL;
		AchievementBase*  		kMadScientist			= NULL;
		AchievementBase*  		kSoprano				= NULL;
		AchievementBase* 		kWTF					= NULL;
		AchievementBase*  		kFlyingBlind			= NULL;
		AchievementBase*  		kPedantic				= NULL;
		AchievementBase*  		kCardinalSin			= NULL;
		AchievementBase*  		kMagister				= NULL;
		AchievementIncremented*  kCommandant				= NULL;
		AchievementBase*  		kCompartmentalizer		= NULL;
		AchievementBase* 		kEruditeModder			= NULL;
		AchievementBase*  		kBobTheBuilder			= NULL;
		AchievementIncremented*  kLoquacious				= NULL;
		AchievementBase*  		kSaboteur				= NULL;
		AchievementBase*  		kOldestTrickInTheBook	= NULL;
		AchievementBase*  		kOver3000				= NULL;
		AchievementBase*  		kFunnyGuy				= NULL;
		AchievementHappypotamus* kHappyBDayMoi			= NULL;
		AchievementBase*  		kChicken				= NULL;

		void Initialize()
		{
			AchievementBase::AllClearAchievement = new bgsee::extras::Achievement("Totally Jobless", "Collected all achievements",
																					IDB_ACHIEVEMENT_TOTALLYJOBLESS, "B43425BE-323A-42C5-90ED-DE9CE014D842");

			kTheWiseOne				= new AchievementBase("The Wise One", "Installed the Construction Set Extender",
															IDB_ACHIEVEMENT_THEWISEONE, "F6B59632-FD01-4762-866C-25A91AB5157D");

			kFearless				= new AchievementBase("Fearless", "Set Oblivion.esm as the active file and hit the 'OK' button",
															IDB_ACHIEVEMENT_FEARLESS, "66DC462C-46E6-438D-8F3C-0DBFB1FD8889");

			kAutomaton				= new AchievementBase("Automaton", "Executed a Coda Script",
															IDB_ACHIEVEMENT_AUTOMATON, "2E76F5E4-EC16-43C9-8DD6-5EAA62A5AEEC");

			kHeretic				= new AchievementBase("Heretic", "Defiled the awesomeness of our Lord shadeMe",
															IDB_ACHIEVEMENT_HERECTIC, "7C70C6BB-5872-43DF-BD2C-1362081C2379");

			kPowerUser				= new AchievementPowerUser(AchievementPowerUser::kTool__MAX - 4);

			kAntiChrist				= new AchievementBase("Anti-Christ", "Attempted to remove our Lord shadeMe from this plane of existence",
															IDB_ACHIEVEMENT_ANTICHRIST, "5ECDC14F-FE88-49AD-8F3D-BF761E95D527");

			kLazyBum				= new AchievementBase("Lazy Bum", "Launched the game from the main menu",
															IDB_ACHIEVEMENT_LAZYBUM, "49C3EE3D-B073-4473-8FAB-2C249CE77D2F");

			kMadScientist			= new AchievementBase("Mad Scientist", "Cloned one of the New Age Divines",
															IDB_ACHIEVEMENT_MADSCIENTIST, "71B4F945-3185-4D8F-96B8-7D0C0BF8C052");

			kSoprano				= new AchievementBase("Soprano", "Successfully generated a lip synch file",
															IDB_ACHIEVEMENT_SOPRANO, "61115B4A-0866-4420-886C-4BEE59DBEE70");

			kWTF					= new AchievementBase("WTF!", "Managed to trigger an assertion in the editor code",
															IDB_ACHIEVEMENT_WTF, "992F82BE-4DDD-40CA-AD96-153353FDDA35");

			kFlyingBlind			= new AchievementBase("Flying Blind", "Disabled editor warning message logging",
															IDB_ACHIEVEMENT_FLYINGBLIND, "51721B2C-DB4F-42C0-B4F4-38154F32DF92");

			kPedantic				= new AchievementBase("Pedantic", "Saved version information to a plugin's description",
															IDB_ACHIEVEMENT_PEDANTIC, "DB3F67DE-60F8-4CA9-928A-044AB83119B2");

			kCardinalSin			= new AchievementBase("Cardinal Sin", "Used the Added Topics list with the GREETING topic",
															IDB_ACHIEVEMENT_CARDINALSIN, "0D76A162-B749-4F20-9570-853AECEA003C");

			kMagister				= new AchievementBase("Magister", "Modified a magic effect",
															IDB_ACHIEVEMENT_MAGISTER, "B2E9FB5D-54FD-4759-B981-DA129B859CA2");

			kCommandant				= new AchievementIncremented("Commandant", "Registered a metric [censored] tonne of script commands",
															IDB_ACHIEVEMENT_COMMANDANT, "13C90CC7-D73A-4F2C-B6AF-982FDE29EC84", 2400);

			kCompartmentalizer		= new AchievementBase("CompartmentalizeR", "Used an editor workspace other than the default",
															IDB_ACHIEVEMENT_COMPARTMENTALIZER, "5BD1463C-EC15-4C3C-A357-26F0F51E3E38");

			kEruditeModder			= new AchievementBase("Erudite Modder", "Remembered to read the ReadMe at least once",
															IDB_ACHIEVEMENT_ERUDITEMODDER, "56385249-C3D8-4D1C-A70E-EC9B05A4338F");

			kBobTheBuilder			= new AchievementBase("Bob The Builder", "Laid roads for a worldspace",
															IDB_ACHIEVEMENT_BOBTHEBUILDER, "10A0F510-062B-4E21-A517-B04CC28D68A7");

			kLoquacious				= new AchievementIncremented("Loquacious", "Created a LOT of dialog responses in a single CS session",
															IDB_ACHIEVEMENT_LOQUACIOUS, "0E0C034F-0E4E-4709-A07D-D79059846841", 100);

			kSaboteur				= new AchievementBase("Saboteur", "Mon Dieu! Managed to crash the extended CS!",
															IDB_ACHIEVEMENT_SABOTEUR, "F2E645A0-2503-4FFC-87A0-E6462325EBB8");

			kOldestTrickInTheBook	= new AchievementBase("Oldest Trick In The Book", "\"Never Gonna Give You Up, Never Gonna Let You Down!\"",
															IDB_ACHIEVEMENT_OLDESTTRICK, "51602A18-037E-4FDC-88A8-B5F472D2026F");

			kOver3000				= new AchievementBase("Over 3000!", "OMFGodrays! RAM usage of 3 gigs?! It's gonna blow!!",
															IDB_ACHIEVEMENT_OVER3000, "17C0EA7A-DB0C-46BA-A19F-232A73630AD5");

			kFunnyGuy				= new AchievementBase("Funny Guy Bob", "How lovely! You've got a sense of humour...",
															IDB_ACHIEVEMENT_FUNNYGUY, "13275AC0-DA3B-4802-813A-4DB3E7C4168D");

			kHappyBDayMoi			= new AchievementHappypotamus("CSE", "I mean, who would have thought!",
															"3AA57BC5-7B1C-4C5B-B7F4-AC0A19EF1A2E",
															2, 3, 2010);

			kChicken				= new AchievementBase("Chicken", "Well, at least the Daleks will be grateful for that...",
															IDB_ACHIEVEMENT_CHICKEN, "D1A90233-3F6A-4F28-9B45-598070913B6A");

			bgsee::extras::ExtenderAchievementArrayT AchievementDepot;

			AchievementDepot.push_back(AchievementBase::AllClearAchievement);

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
			AchievementDepot.push_back(AchievementCheat::GetSingleton());
			AchievementDepot.push_back(AchievementLost::GetSingleton());
			AchievementDepot.push_back(kOldestTrickInTheBook);
			AchievementDepot.push_back(kOver3000);
			AchievementDepot.push_back(kFunnyGuy);
			AchievementDepot.push_back(kHappyBDayMoi);
			AchievementDepot.push_back(kChicken);

			bool ComponentInitialized = BGSEEACHIEVEMENTS->Initialize(BGSEEMAIN->ExtenderGetLongName(), BGSEEMAIN->GetExtenderHandle(), AchievementDepot);

			SME_ASSERT(ComponentInitialized);

			BGSEECONSOLE->Pad(1);
			BGSEECONSOLE_MESSAGE("Precious time wasted on the CS: %0.1f hours", AchievementLost::GetSingleton()->GetLoggedHours());
		}
	}
}