#include "TESQuest.h"

HWND*	TESQuest::WindowHandle = (HWND*)0x00A0B034;
HWND*	TESQuest::FilteredDialogWindowHandle = (HWND*)0x00A0B038;

void TESQuest::SetStartGameEnabledFlag(bool State)
{
	thisCall<UInt32>(0x004DD7E0, this, State);
}

void TESQuest::SetAllowedRepeatedStagesFlag(bool State)
{
	thisCall<UInt32>(0x004DD7C0, this, State);
}


void TESQuest::StageData::RemoveStageItem( QuestStageItem* Item )
{
	thisCall<void>(0x004E23F0, this, Item);
}