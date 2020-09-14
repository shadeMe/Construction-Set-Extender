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

	return nullptr;
}

void GameSettingCollection::CreateDefaultCopy()
{
	for (cseOverride::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
	{
		const char* Name = nullptr;
		Setting* Data = nullptr;

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
		const char* Name = nullptr;
		Setting* Data = nullptr;

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

UInt32 GameSettingCollection::GetCount(bool OnlyActive)
{
	if (!OnlyActive)
		return settingMap.GetCount();

	UInt32 Count = 0;
	for (cseOverride::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
	{
		const char* Name = nullptr;
		Setting* Data = nullptr;

		settingMap.GetNext(Itr, Name, Data);
		if (Name)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			if (SettingForm->IsActive())
				++Count;
		}
	}

	return Count;
}

void GameSettingCollection::MarshalAll(componentDLLInterface::GMSTData** OutData, UInt32* OutCount, bool OnlyActive)
{
	*OutCount = GameSettingCollection::Instance->GetCount(OnlyActive);
	if (*OutCount == 0)
		return;

	*OutData = new componentDLLInterface::GMSTData[*OutCount];

	UInt32 Index = 0;
	for (cseOverride::NiTMapIterator Itr = settingMap.GetFirstPos(); Itr;)
	{
		const char* Name = nullptr;
		Setting* Data = nullptr;

		settingMap.GetNext(Itr, Name, Data);
		if (Name)
		{
			GameSetting* SettingForm = (GameSetting*)((UInt32)Data - 0x24);
			if (OnlyActive && SettingForm->IsActive() == false)
				continue;

			(*OutData)[Index].FillFormData(SettingForm);
			(*OutData)[Index].FillVariableData(SettingForm);
			++Index;
		}
	}
}


