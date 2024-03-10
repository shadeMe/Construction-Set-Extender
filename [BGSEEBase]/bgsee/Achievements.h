#pragma once

namespace bgsee
{
	namespace extras
	{
		class AchievementManager;

		class Achievement
		{
			friend class AchievementManager;
		protected:
			enum
			{
				kState_Locked	= 0,
				kState_Unlocked,
				kState_Triggered		// can be used to initiate deferred unlocking
			};

			UUID						BaseID;
			std::string					BaseIDString;
			std::string					Name;
			std::string					Description;
			UInt8						State;
			UInt64						ExtraData;
			UInt32						IconID;

										// called by the manager before unlocking the achievement, return false to remain locked
			virtual bool				UnlockCallback(AchievementManager* Parameter);
										// called before the achievement's state is serialized, return false to cancel the save op
			virtual bool				SaveCallback(AchievementManager* Parameter);

			virtual void				GetName(std::string& OutBuffer) const;
			virtual void				GetDescription(std::string& OutBuffer) const;

			virtual bool				GetUnlockable(void) const;		// only unlockable achievements count towards the total/unlocked achievement count
		public:
			Achievement(const char* Name, const char* Desc, UInt32 IconID, const char* GUID);
			virtual ~Achievement();

			bool						GetUnlocked(void) const;
			bool						GetTriggered(void) const;
		};

		typedef std::vector<Achievement*>				ExtenderAchievementArrayT;

		class AchievementManager
		{
			static INT_PTR CALLBACK						NotificationDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

			struct NotificationUserData
			{
				AchievementManager*						Instance;
				Achievement*							Achievement;
				HBRUSH									BGBrush;
				HANDLE									Icon;
			};

			static AchievementManager*					Singleton;

			std::string									RegistryKeyRoot;
			std::string									RegistryKeyExtraData;
			ExtenderAchievementArrayT					AchievementDepot;
			HINSTANCE									ResourceInstance;
			bool										Initialized;

			AchievementManager(const char* ExtenderLongName, HINSTANCE ResourceInstance, ExtenderAchievementArrayT& Achievements);
			~AchievementManager();

			void										SaveAchievementState(Achievement* Achievement, bool StateOnly = false);
			void										LoadAchievementState(Achievement* Achievement, bool StateOnly = false);

			template <typename T>
			bool										GetRegValue(const char* Name, T* OutValue, const char* Key);
			template <typename T>
			bool										SetRegValue(const char* Name, T Value, const char* Key);
		public:

			static AchievementManager*					Get(void);

			static bool									Initialize(const char* ExtenderLongName, HINSTANCE ResourceInstance, ExtenderAchievementArrayT& Achievements);	// takes ownership of achievement instances
			static void									Deinitialize();

			void										Unlock(Achievement* Achievement, bool ForceUnlock = false, bool TriggerOnly = false, bool PreserveUnlockState = false);
			UInt32										GetTotalAchievements(void) const;
			UInt32										GetUnlockedAchievements(void) const;
		};

#define BGSEEACHIEVEMENTS								bgsee::extras::AchievementManager::Get()
	}
}