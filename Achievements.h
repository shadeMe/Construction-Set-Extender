#pragma once

// I'm tired...
namespace Achievements
{
	extern const UInt32		kMaxCSSessionLength;
	extern const UInt32		kMaxNewToolsSampleCount;
	extern const UInt32		kMaxScriptCommandCount;
	extern const UInt32		kMaxTopicInfosSaved;
	extern const UInt32		kHalfWayUnlockCount;

	extern UInt32			g_NewToolsSampleCount;
	extern UInt32			g_TopicInfosSaved;
	extern DWORD			g_TickCount;

	struct Achievement
	{
		const char*			Name;
		const char*			Description;
		UInt32				ID;
		bool				Unlocked;

		bool				Initialize();
	};

	enum
	{
		kAchievement_TheWiseOne				=	0,
		kAchievement_Cheat,
		kAchievement_Fearless,
		kAchievement_Automaton,
		kAchievement_Heretic,
		kAchievement_PowerUser,
		kAchievement_AntiChrist,
		kAchievement_LazyBum,
		kAchievement_MadScientist,
		kAchievement_Soprano,
		kAchievement_WTF,
		kAchievement_FlyingBlind,
		kAchievement_Pedantic,
		kAchievement_CardinalSin,
		kAchievement_Magister,
		kAchievement_Commandant,
		kAchievement_Compartmentalizer,
		kAchievement_EruditeModder,
		kAchievement_BobTheBuilder,
		kAchievement_Loquacious,
		kAchievement_Saboteur,

		kAchievement_TotallyJobless,
		kAchievement_HalfWayUnlocked		=	31
	};

	extern Achievement		Entries[];

	bool		GetUnlockedAchievementMask(UInt32* Out);
	bool		SetUnlockedAchievementMask(UInt32 Mask);
	void		Initialize();
	void __stdcall UnlockAchievement(UInt32 ID);
	UInt32		GetUnlockedAchievementCount();
	bool		GetAchievementUnlocked(UInt8 ID);
}