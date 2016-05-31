#pragma once
#include <bgsee\Achievements.h>

namespace cse
{
	namespace achievements
	{
		class AchievementBase : public bgsee::extras::Achievement
		{
		protected:
			virtual bool				UnlockCallback(bgsee::extras::AchievementManager* Parameter);
		public:
			AchievementBase(const char* Name, const char* Desc, UInt32 IconID, const char* GUID);
			virtual ~AchievementBase();

			static Achievement*	AllClearAchievement;
		};

		class AchievementTimeLapsed : public AchievementBase
		{
		protected:
			static const UInt32				kIdleTimeOut;

			DWORD							TimerID;
			DWORD							InitTickCount;
			DWORD							ElapsedTicks;
			UInt32							HoursRequired;

			void							ResetTimer(void);
			bool							GetIsIdling(void);
		public:
			AchievementTimeLapsed(const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdHours);
			virtual ~AchievementTimeLapsed() = 0;
		};

		class AchievementCheat : public AchievementTimeLapsed
		{
		protected:
			static AchievementCheat*		Singleton;

			static VOID CALLBACK			TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			AchievementCheat();
		public:
			virtual ~AchievementCheat();

			static AchievementCheat*		GetSingleton();
		};

		class AchievementLost : public AchievementTimeLapsed
		{
		protected:
			static AchievementLost*		Singleton;

			static VOID CALLBACK			TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			virtual bool					SaveCallback(bgsee::extras::AchievementManager* Parameter);

			AchievementLost(UInt32 ReqdHours);
		public:
			virtual ~AchievementLost();

			static AchievementLost*		GetSingleton();

			float							GetLoggedHours(void) const;
		};

		class AchievementIncremented : public AchievementBase
		{
		protected:
			UInt32							CountRequired;
			UInt32							CurrentCount;

			virtual bool					UnlockCallback(bgsee::extras::AchievementManager* Parameter);
		public:
			AchievementIncremented(const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdCount);
			virtual ~AchievementIncremented();
		};

		class AchievementTimeTriggered : public AchievementBase
		{
		protected:
			UInt8							Day;
			UInt8							Month;
			UInt16							Year;

			virtual bool					UnlockCallback(bgsee::extras::AchievementManager* Parameter);
			virtual bool					SaveCallback(bgsee::extras::AchievementManager* Parameter);

			virtual bool					GetUnlockable(void) const;
		public:
			AchievementTimeTriggered(const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt8 EventDay, UInt8 EventMonth, UInt16 EventYear = 0);		// pass 0 as eventyear to repeat every year
			virtual ~AchievementTimeTriggered();
		};

		class AchievementHappypotamus : public AchievementTimeTriggered
		{
		protected:
			UInt16							GeborenJahr;

			virtual void					GetName(std::string& OutBuffer) const;
			virtual bool					GetUnlockable(void) const;
		public:
			AchievementHappypotamus(const char* Name, const char* Desc, const char* GUID, UInt8 EventDay, UInt8 EventMonth, UInt16 EventYear);
			virtual ~AchievementHappypotamus();
		};

		class AchievementPowerUser : public AchievementBase
		{
		protected:
			UInt8							ThresholdCount;

			UInt8							GetUnlockedToolCount() const;
		public:
			AchievementPowerUser(UInt8 Threshold);
			virtual ~AchievementPowerUser();

			enum
			{
				kTool__BEING = -1,

				kTool_SaveAs = 0,
				kTool_SaveAsESM,
				kTool_StartupLoad,
				kTool_Workspace,
				kTool_AssetSelection,
				kTool_FormContextMenu,
				kTool_LiveChangeLog,
				kTool_QuickLookup,
				kTool_GlobalScript,
				kTool_GenerateLIP,
				kTool_FormListFilter,
				kTool_AuxViewPort,
				kTool_AlternateRenderWindowMovement,
				kTool_PathGridAdditions,
				kTool_FlyCamera,
				kTool_RefGrouping,
				kTool_RefFreezing,
				kTool_RefVisibility,
				kTool_RefAlignment,
				kTool_BatchEditor,
				kTool_GlobalClipboard,
				kTool_GlobalUndo,
				kTool_Toolbox,
				kTool_UseInfoListing,
				kTool_TagBrowser,
				kTool_ScriptEditor,
				kTool_MultipleObjectWindows,
				kTool_ObjectPalette,
				kTool_ObjectPrefabs,

				kTool__MAX
			};

			void							UnlockTool(UInt8 Tool);
		};

		extern AchievementBase*			kTheWiseOne;
		extern AchievementBase*			kFearless;
		extern AchievementBase*			kAutomaton;
		extern AchievementBase*			kHeretic;
		extern AchievementPowerUser*  	kPowerUser;
		extern AchievementBase*  		kAntiChrist;
		extern AchievementBase*  		kLazyBum;
		extern AchievementBase*  		kMadScientist;
		extern AchievementBase*  		kSoprano;
		extern AchievementBase* 		kWTF;
		extern AchievementBase*  		kFlyingBlind;
		extern AchievementBase*  		kPedantic;
		extern AchievementBase*  		kCardinalSin;
		extern AchievementBase*  		kMagister;
		extern AchievementIncremented*  kCommandant;
		extern AchievementBase*  		kCompartmentalizer;
		extern AchievementBase* 		kEruditeModder;
		extern AchievementBase*  		kBobTheBuilder;
		extern AchievementIncremented*  kLoquacious;
		extern AchievementBase*  		kSaboteur;
		extern AchievementBase*  		kOldestTrickInTheBook;
		extern AchievementBase*  		kOver3000;
		extern AchievementBase*  		kFunnyGuy;
		extern AchievementHappypotamus* kHappyBDayMoi;
		extern AchievementBase*  		kChicken;

		void								Initialize(void);
	}
}
