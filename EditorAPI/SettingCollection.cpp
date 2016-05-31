#include "SettingCollection.h"

GameSettingCollection*				GameSettingCollection::Instance = (GameSettingCollection*)0x00A10198;
INISettingCollection*				INISettingCollection::Instance = (INISettingCollection*)0x00A102B8;
DefaultGMSTMapT						GameSettingCollection::DefaultCopy;

Setting* SettingCollectionList::LookupByName( const char* Name )
{
	for (SettingListT::Iterator Itr = settingList.Begin(); Itr.End() == false && Itr.Get(); ++Itr)
	{
		Setting* Current = Itr.Get();
		SME_ASSERT(Current);

		if (!_stricmp(Name, Current->name))
			return Current;
	}

	return NULL;
}

void GameSettingCollection::CreateDefaultCopy()
{
	for (cseOverride::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
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

			DefaultCopy.insert(std::make_pair(Name, TempSetting));
		}
	}
}

void GameSettingCollection::ResetCollection()
{
	for (cseOverride::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
	{
		const char* Name = NULL;
		Setting* Data = NULL;

		settingMap.GetNext(Itr, Name, Data);
		if (Data)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			SettingForm->SetFromActiveFile(false);

			DefaultGMSTMapT::iterator Match = DefaultCopy.find(Name);

			if (Match != DefaultCopy.end())
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

void GameSettingCollection::SerializeGMSTDataForHandShake(componentDLLInterface::GMSTData* HandShakeData)
{
	UInt32 Index = 0;
	for (cseOverride::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
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

