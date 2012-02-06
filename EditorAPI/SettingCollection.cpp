#include "SettingCollection.h"

GameSettingCollection*				g_GMSTCollection = (GameSettingCollection*)0x00A10198;
const char*							g_CSINIPath = (const char*)0x00A0ABB8;

struct DefaultGMSTMapKeyComparer
{
	bool operator()(const char* Key1, const char* Key2) const
	{
		return _stricmp(Key1, Key2) < 0;
	}
};

typedef std::map<const char*, GameSetting*, DefaultGMSTMapKeyComparer>		DefaultGMSTMapT;
DefaultGMSTMapT						g_DefaultGMSTMap;

void GameSettingCollection::CreateDefaultCopy()
{
	for (CSE_GlobalClasses::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
	{
		const char* Name = NULL;
		Setting* Data = NULL;

		settingMap.GetNext(Itr, Name, Data);
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
	for (CSE_GlobalClasses::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
	{
		const char* Name = NULL;
		Setting* Data = NULL;

		settingMap.GetNext(Itr, Name, Data);
		if (Data)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			SettingForm->SetFromActiveFile(false);

			DefaultGMSTMapT::iterator Match = g_DefaultGMSTMap.find(Name);

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
	return settingMap.GetCount();
}

void GameSettingCollection::SerializeGMSTDataForHandShake(ComponentDLLInterface::GMSTData* HandShakeData)
{
	UInt32 Index = 0;
	for (CSE_GlobalClasses::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
	{
		const char* Name = NULL;
		Setting* Data = NULL;

		settingMap.GetNext(Itr, Name, Data);
		if (Name)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			HandShakeData[Index].FillFormData((TESForm*)SettingForm);
			HandShakeData[Index].FillVariableData(Name);
			Index++;
		}
	}
}