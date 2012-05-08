#pragma once
#include <BGSEEAchievements.h>

namespace ConstructionSetExtender
{
	namespace Achievements
	{
		class CSEAchievementBase : public BGSEditorExtender::Extras::BGSEEAchievement
		{
		protected:
			virtual bool				UnlockCallback(BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter);
		public:
			CSEAchievementBase(const char* Name, const char* Desc, UInt32 IconID, const char* GUID);
			virtual ~CSEAchievementBase();

			static BGSEEAchievement*	AllClearAchievement;
		};

		class CSEAchievementCheat : public CSEAchievementBase
		{
		protected:
			static CSEAchievementCheat*		Singleton;

			static VOID CALLBACK			TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			DWORD							TimerID;
			DWORD							TickCount;
			UInt32							HoursRequired;

			void							ResetTimer(void);

			CSEAchievementCheat(UInt32 ReqdHours);
		public:
			virtual ~CSEAchievementCheat();

			static CSEAchievementCheat*		GetSingleton();
		};

		class CSEAchievementIncremented : public CSEAchievementBase
		{
		protected:
			UInt32							CountRequired;
			UInt32							CurrentCount;

			virtual bool					UnlockCallback(BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter);
		public:
			CSEAchievementIncremented(const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdCount);
			virtual ~CSEAchievementIncremented();
		};

		extern CSEAchievementBase*			kTheWiseOne;
		extern CSEAchievementBase*			kFearless;
		extern CSEAchievementBase*			kAutomaton;
		extern CSEAchievementBase*			kHeretic;
		extern CSEAchievementIncremented*  	kPowerUser;
		extern CSEAchievementBase*  		kAntiChrist;
		extern CSEAchievementBase*  		kLazyBum;
		extern CSEAchievementBase*  		kMadScientist;
		extern CSEAchievementBase*  		kSoprano;
		extern CSEAchievementBase* 			kWTF;
		extern CSEAchievementBase*  		kFlyingBlind;
		extern CSEAchievementBase*  		kPedantic;
		extern CSEAchievementBase*  		kCardinalSin;
		extern CSEAchievementBase*  		kMagister;
		extern CSEAchievementIncremented*  	kCommandant;
		extern CSEAchievementBase*  		kCompartmentalizer;
		extern CSEAchievementBase* 			kEruditeModder;
		extern CSEAchievementBase*  		kBobTheBuilder;
		extern CSEAchievementIncremented*  	kLoquacious;
		extern CSEAchievementBase*  		kSaboteur;

		void								Initialize(void);
	}
}
