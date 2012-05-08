#pragma once

namespace BGSEditorExtender
{
	namespace Extras
	{
		class BGSEEAchievementManager;

		class BGSEEAchievement
		{
			friend class BGSEEAchievementManager;
		protected:
			enum
			{
				kState_Locked	= 0,
				kState_Unlocked,
				kState_Triggered		// can be used to initiate deferred unlocking
			};

			UUID						BaseID;
			std::string					BaseIDString;
			const char*					Name;
			const char*					Description;
			UInt8						State;
			UInt32						IconID;

										// called by the manager before unlocking the achievement, return false to remain locked
			virtual bool				UnlockCallback(BGSEEAchievementManager* Parameter);
		public:
			BGSEEAchievement(const char* Name, const char* Desc, UInt32 IconID, const char* GUID);
			virtual ~BGSEEAchievement();

			bool						GetUnlocked(void) const;
		};

		typedef std::list<BGSEEAchievement*>			ExtenderAchievementListT;

		class BGSEEAchievementManager
		{
			friend class BGSEEAchivement;

			static INT_PTR CALLBACK						NotificationDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			struct NotificationUserData
			{
				BGSEEAchievementManager*				Instance;
				BGSEEAchievement*						Achievement;
				HBRUSH									BGBrush;
				HANDLE									Icon;
			};

			static BGSEEAchievementManager*				Singleton;

			std::string									RegistryKey;
			ExtenderAchievementListT					AchievementDepot;
			HINSTANCE									ResourceInstance;
			bool										Initialized;

			BGSEEAchievementManager();

			void										SaveAchievementState(BGSEEAchievement* Achievement);
			void										LoadAchievementState(BGSEEAchievement* Achievement);

			bool										GetRegValue(const char* Name, UInt32* OutValue);
			bool										SetRegValue(const char* Name, UInt32 Value);
		public:
			~BGSEEAchievementManager();

			static BGSEEAchievementManager*				GetSingleton(void);

														// takes ownership of achievement instances
			bool										Initialize(const char* ExtenderLongName, HINSTANCE ResourceInstance, ExtenderAchievementListT& Achievements);

			void										Unlock(BGSEEAchievement* Achievement, bool ForceUnlock = false, bool TriggerOnly = false);
			UInt32										GetTotalAchievements(void) const;
			UInt32										GetUnlockedAchievements(void) const;
		};

#define BGSEEACHIEVEMENTS								BGSEditorExtender::Extras::BGSEEAchievementManager::GetSingleton()
	}
}