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

		class CSEAchievementTimeLapsed : public CSEAchievementBase
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
			CSEAchievementTimeLapsed(const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt32 ReqdHours);
			virtual ~CSEAchievementTimeLapsed() = 0;
		};

		class CSEAchievementCheat : public CSEAchievementTimeLapsed
		{
		protected:
			static CSEAchievementCheat*		Singleton;

			static VOID CALLBACK			TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			CSEAchievementCheat();
		public:
			virtual ~CSEAchievementCheat();

			static CSEAchievementCheat*		GetSingleton();
		};

		class CSEAchievementLost : public CSEAchievementTimeLapsed
		{
		protected:
			static CSEAchievementLost*		Singleton;

			static VOID CALLBACK			TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

			virtual bool					SaveCallback(BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter);

			CSEAchievementLost(UInt32 ReqdHours);
		public:
			virtual ~CSEAchievementLost();

			static CSEAchievementLost*		GetSingleton();

			float							GetLoggedHours(void) const;
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

		class CSEAchievementTimeTriggered : public CSEAchievementBase
		{
		protected:
			UInt8							Day;
			UInt8							Month;
			UInt16							Year;

			virtual bool					UnlockCallback(BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter);
			virtual bool					SaveCallback(BGSEditorExtender::Extras::BGSEEAchievementManager* Parameter);

			virtual bool					GetUnlockable(void) const;
		public:
			CSEAchievementTimeTriggered(const char* Name, const char* Desc, UInt32 IconID, const char* GUID, UInt8 EventDay, UInt8 EventMonth, UInt16 EventYear = 0);		// pass 0 as eventyear to repeat every year
			virtual ~CSEAchievementTimeTriggered();
		};

		class CSEAchievementHappypotamus : public CSEAchievementTimeTriggered
		{
		protected:
			UInt16							GeborenJahr;

			virtual void					GetName(std::string& OutBuffer) const;
			virtual bool					GetUnlockable(void) const;
		public:
			CSEAchievementHappypotamus(const char* Name, const char* Desc, const char* GUID, UInt8 EventDay, UInt8 EventMonth, UInt16 EventYear);
			virtual ~CSEAchievementHappypotamus();
		};

		class CSEAchievementPowerUser : public CSEAchievementBase
		{
		protected:
			UInt8							ThresholdCount;

			UInt8							GetUnlockedToolCount() const;
		public:
			CSEAchievementPowerUser(UInt8 Threshold);
			virtual ~CSEAchievementPowerUser();

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

		extern CSEAchievementBase*			kTheWiseOne;
		extern CSEAchievementBase*			kFearless;
		extern CSEAchievementBase*			kAutomaton;
		extern CSEAchievementBase*			kHeretic;
		extern CSEAchievementPowerUser*  	kPowerUser;
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
		extern CSEAchievementBase*  		kOldestTrickInTheBook;
		extern CSEAchievementBase*  		kOver3000;
		extern CSEAchievementBase*  		kFunnyGuy;
		extern CSEAchievementHappypotamus*  kHappyBDayMoi;
		extern CSEAchievementBase*  		kChicken;

		void								Initialize(void);
	}
}
