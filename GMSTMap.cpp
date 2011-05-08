#include "GMSTMap.h"
#include "[Common]\HandShakeStructs.h"

_DefaultGMSTMap						g_DefaultGMSTMap;

void InitializeDefaultGMSTMap()
{
	void* Unk01 = (void*)thisCall(0x0051F920, (void*)g_GMSTMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		SettingData* Data;

		thisCall(0x005E0F90, (void*)g_GMSTMap, &Unk01, &Name, &Data);
		if (Name)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);

			GameSetting* TempSetting = (GameSetting*)FormHeap_Allocate(0x2C);
			thisCall(kGameSetting_Ctor, TempSetting);
			thisCall(kTESForm_SetTemporary, TempSetting);
			TempSetting->SetSettingID(Name);
			thisVirtualCall(*(UInt32*)TempSetting, 0xB8, TempSetting, SettingForm);

			g_DefaultGMSTMap.insert(std::make_pair<const char*, GameSetting*>(Name, TempSetting));
		}
	}
}

UInt32 CountGMSTForms()
{
	UInt32 Count = 0;

	void* Unk01 = (void*)thisCall(0x0051F920, (void*)g_GMSTMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		SettingData* Data;

		thisCall(0x005E0F90, (void*)g_GMSTMap, &Unk01, &Name, &Data);
		if (Name)
			Count++;
	}

	return Count;
}

void InitializeHandShakeGMSTData(GMSTData* HandShakeData)
{
	UInt32 Index = 0;
	void* Unk01 = (void*)thisCall(0x0051F920, (void*)g_GMSTMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		SettingData* Data;

		thisCall(0x005E0F90, (void*)g_GMSTMap, &Unk01, &Name, &Data);
		if (Name)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			HandShakeData[Index].FillFormData((TESForm*)SettingForm);
			HandShakeData[Index].FillVariableData(Name);
			Index++;
		}
	}

}