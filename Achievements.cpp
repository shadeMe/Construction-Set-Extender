#include "Achievements.h"
#include "WindowManager.h"
#include "Resource.h"

namespace Achievements
{
	const UInt32		kMaxCSSessionLength = 6;	// in hours
	const UInt32		kMaxNewToolsSampleCount = 50;
	const UInt32		kMaxScriptCommandCount = 2400;
	const UInt32		kMaxTopicInfosSaved = 100;
	const UInt32		kHalfWayUnlockCount = 14;

	UInt32				g_NewToolsSampleCount = 0;
	UInt32				g_TopicInfosSaved = 0;
	DWORD				g_TickCount = GetTickCount();

	const char*			kAchivementRegKeyStr = "Software\\Imitation Camel\\Construction Set Extender\\Achievements\\";
	const char*			kUnlockedAchievementMaskStr = "UnlockedAchievementMask";

	#define TableSize	22

	Achievement		Entries[TableSize] =
	{
		{ "The Wise One",			"Installed the Construction Set Extender",														kAchievement_TheWiseOne,			false },
		{ "Cheat",					"Managed to complete a 6 hour long CS session without any CTDs",								kAchievement_Cheat,					false },
		{ "Fearless",				"Set Oblivion.esm as the active file and hit the 'OK' button",									kAchievement_Fearless,				false },
		{ "Automaton",				"Executed a CS Automation Script",																kAchievement_Automaton,				false },
		{ "Heretic",				"Defiled the awesomeness of our lord shadeMe",													kAchievement_Heretic,				false },
		{ "Power User",				"Sampled much of the new tools CSE adds",														kAchievement_PowerUser,				false },
		{ "Anti-Christ",			"Attempted to remove our lord shadeMe from this plane of existence",							kAchievement_AntiChrist,			false },
		{ "Lazy Bum",				"Launched the game from the main menu",															kAchievement_LazyBum,				false },
		{ "Mad Scientist",			"Cloned one of the seventeen Divines",															kAchievement_MadScientist,			false },
		{ "Soprano",				"Successfully generated a lip synch file",														kAchievement_Soprano,				false },
		{ "WTF!",					"Managed to trigger an assertion in the editor code",											kAchievement_WTF,					false },
		{ "Flying Blind",			"Disabled editor warning logging",																kAchievement_FlyingBlind,			false },
		{ "Pedantic",				"Saved version information to a plugin's description",											kAchievement_Pedantic,				false },
		{ "Cardinal Sin",			"Used the Added Topics list when adding new dialog topics to responses",						kAchievement_CardinalSin,			false },
		{ "Magister",				"Modified a magic effect",																		kAchievement_Magister,				false },
		{ "Commadant",				"Over 2400 script commands registered",															kAchievement_Commandant,			false },
		{ "CompartmentalizeR",		"Used an editor workspace other than the default",												kAchievement_Compartmentalizer,		false },
		{ "Erudite Modder",			"Remembered to read CSE's documentation at least once",											kAchievement_EruditeModder,			false },
		{ "Road Builder",			"Layed roads for a worldspace",																	kAchievement_RoadBuilder,			false },
		{ "Loquacious",				"Over 100 dialog responses created in a single CS session",										kAchievement_Loquacious,			false },
		{ "Saboteur",				"Managed to crash the extended CS!",															kAchievement_Saboteur,				false },

		{ "Totally Jobless",		"Collected all achievements",																	kAchievement_TotallyJobless,		false }
	};

	bool Achievement::Initialize()
	{
		UInt32 Out = 0;
		if (GetUnlockedAchievementMask(&Out) && (Out & (1 << ID)))
			Unlocked = true;

		return Unlocked;
	}

	bool GetUnlockedAchievementMask(UInt32* Out)
	{
		*Out = 0;
		HKEY AchievementKey = NULL;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, kAchivementRegKeyStr, NULL, KEY_ALL_ACCESS, &AchievementKey) == ERROR_SUCCESS)
		{
			UInt32 Type = 0, Size = 4;
			if (RegQueryValueEx(AchievementKey, kUnlockedAchievementMaskStr, NULL, &Type, (LPBYTE)Out, &Size) == ERROR_SUCCESS)
				return true;
		}

		return false;
	}

	bool SetUnlockedAchievementMask(UInt32 Mask)
	{
		HKEY AchievementKey = NULL;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, kAchivementRegKeyStr, NULL, KEY_ALL_ACCESS, &AchievementKey) == ERROR_SUCCESS)
		{
			if (RegSetValueEx(AchievementKey, kUnlockedAchievementMaskStr, NULL, REG_DWORD, (const BYTE*)&Mask, REG_DWORD) == ERROR_SUCCESS)
				return true;
		}

		return false;
	}

	void Initialize()
	{
		HKEY AchievementKey = NULL;
		if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, kAchivementRegKeyStr, NULL, NULL, NULL, KEY_ALL_ACCESS , NULL, &AchievementKey, NULL) != ERROR_SUCCESS)
		{
			DebugPrint("Couldn't create achievement reg key!");
			LogWinAPIErrorMessage(GetLastError());
		}

		UInt32 Mask = 0;
		if (!GetUnlockedAchievementMask(&Mask))		// create key if not found
		{
			if (!SetUnlockedAchievementMask(Mask))
			{
				DebugPrint("Couldn't create achievement mask value!");
				LogWinAPIErrorMessage(GetLastError());
			}
		}

		UInt32 UnlockedCount = 0;
		for (int i = 0; i < TableSize; i++)
		{
			Achievement& Itr = Entries[i];

			if (Itr.Initialize())
				UnlockedCount++;
		}

		DebugPrint("Achievements Unlocked: %d/%d", UnlockedCount, TableSize);
	}

	void __stdcall UnlockAchievement(UInt32 ID)
	{
		Achievement* UnlockableAchievement = &Entries[ID];
		if (UnlockableAchievement->Unlocked)
			return;

		switch (ID)
		{
		case kAchievement_PowerUser:
			g_NewToolsSampleCount++;
			if (g_NewToolsSampleCount < kMaxNewToolsSampleCount)
				return;
			break;
		case kAchievement_Loquacious:
			g_TopicInfosSaved++;
			if (g_TopicInfosSaved < kMaxTopicInfosSaved)
				return;
			break;
		}

		UnlockableAchievement->Unlocked = true;
		DebugPrint("New Achievement Unlocked: %s", UnlockableAchievement->Name);
		DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_ACHIEVEMENTUNLOCKED), *g_HWND_CSParent, AchievementUnlockedDlgProc, (LPARAM)ID);
		UInt32 AchievementMask = 0;
		if (!GetUnlockedAchievementMask(&AchievementMask))
		{
			DebugPrint("Couldn't get achievement mask value!");
			LogWinAPIErrorMessage(GetLastError());
			return;
		}
		AchievementMask |= 1 << ID;

		if (GetUnlockedAchievementCount() == TableSize)
			UnlockAchievement(kAchievement_TotallyJobless);
		else if (GetUnlockedAchievementCount() >= kHalfWayUnlockCount)
		{
			if (!(AchievementMask & (1 << kAchievement_HalfWayUnlocked)))
			{
				AchievementMask |= 1 << kAchievement_HalfWayUnlocked;
				MessageBox(*g_HWND_CSParent, "Congratulations!\n\nYou've unlocked more than half of editor's achievements! You may now ask shadeMe to induct you into the CSE Hall of Fame!", "CSE", MB_OK|MB_ICONEXCLAMATION);
			}
		}

		if (!SetUnlockedAchievementMask(AchievementMask))
		{
			DebugPrint("Couldn't set achievement mask value!");
			LogWinAPIErrorMessage(GetLastError());
		}
	}

	UInt32 GetUnlockedAchievementCount()
	{
		UInt32 UnlockedCount = 0;
		for (int i = 0; i < TableSize; i++)
		{
			Achievement& Itr = Entries[i];

			if (Itr.Unlocked)
				UnlockedCount++;
		}

		return UnlockedCount;
	}

	bool GetAchievementUnlocked(UInt8 ID)
	{
		return Entries[ID].Unlocked;
	}
}