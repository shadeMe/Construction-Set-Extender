#include "SettingCollection.h"

GameSettingCollection*				g_GMSTCollection = (GameSettingCollection*)0x00A10198;

struct GMSTMap_Key_Comparer
{
	bool operator()(const char* Key1, const char* Key2) const {
		return _stricmp(Key1, Key2) < 0;
	}
};

typedef std::map<const char*, GameSetting*, GMSTMap_Key_Comparer>		_DefaultGMSTMap;
_DefaultGMSTMap						g_DefaultGMSTMap;

void GameSettingCollection::CreateDefaultCopy()
{
	void* Unk01 = thisCall<void*>(0x0051F920, &settingMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		Setting* Data;

		thisCall<UInt32>(0x005E0F90, &settingMap, &Unk01, &Name, &Data);
		if (Name)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);

			GameSetting* TempSetting = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_GMST), TESForm, GameSetting);
			TempSetting->name = Name;
			TempSetting->MarkAsTemporary();
			TempSetting->CopyFrom(SettingForm);

			g_DefaultGMSTMap.insert(std::make_pair<const char*, GameSetting*>(Name, TempSetting));
		}
	}
}

void GameSettingCollection::ResetCollection()
{
	void* Unk01 = thisCall<void*>(0x0051F920, &settingMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		Setting* Data = NULL;

		thisCall<UInt32>(0x005E0F90, &settingMap, &Unk01, &Name, &Data);
		if (Data)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			SettingForm->SetFromActiveFile(false);

			_DefaultGMSTMap::iterator Match = g_DefaultGMSTMap.find(Name);

			if (Match != g_DefaultGMSTMap.end())
			{
				GameSetting* DefaultGMST = Match->second;
				SettingForm->CopyFrom(DefaultGMST);
			}
		}
	}
}

UInt32 GameSettingCollection::GetGMSTCount()
{
	UInt32 Count = 0;

	void* Unk01 = thisCall<void*>(0x0051F920, &settingMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		Setting* Data;

		thisCall<UInt32>(0x005E0F90, &settingMap, &Unk01, &Name, &Data);
		if (Name)
			Count++;
	}

	return Count;
}

void GameSettingCollection::SerializeGMSTDataForHandShake(GMSTData* HandShakeData)
{
	UInt32 Index = 0;
	void* Unk01 = thisCall<void*>(0x0051F920, &settingMap);
	while (Unk01)
	{
		const char*	 Name = NULL;
		Setting* Data;

		thisCall<UInt32>(0x005E0F90, &settingMap, &Unk01, &Name, &Data);
		if (Name)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			HandShakeData[Index].FillFormData((TESForm*)SettingForm);
			HandShakeData[Index].FillVariableData(Name);
			Index++;
		}
	}
}