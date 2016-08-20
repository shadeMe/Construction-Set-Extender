#include "HallOfFame.h"

namespace bgsee
{
	namespace extras
	{
		HallOfFameEntry::HallOfFameEntry( const char* Name, const char* Title ) :
			Name(Name),
			Title(Title)
		{
			;//
		}

		HallOfFameEntry::~HallOfFameEntry()
		{
			;//
		}

		HallOfFameManager*			HallOfFameManager::Singleton = nullptr;

		HallOfFameManager::HallOfFameManager() :
			BaseFormID(0x450),
			Inductees(),
			Initialized(false)
		{
			Inductees.reserve(0x20);
		}

		HallOfFameManager::~HallOfFameManager()
		{
			for (auto Itr: Inductees)
			{
				Itr->Deinitialize();
				delete Itr;
			}

			Inductees.clear();
			Initialized = false;

			Singleton = nullptr;
		}

		HallOfFameManager* HallOfFameManager::GetSingleton( void )
		{
			if (Singleton == nullptr)
				Singleton = new HallOfFameManager();

			return Singleton;
		}

		bool HallOfFameManager::Initialize( const ExtenderHOFEntryArrayT& Entries, const UInt32 StartingFormID )
		{
			if (Initialized)
				return false;

			SME_ASSERT(StartingFormID < 0x800 && StartingFormID > 0x400);

			Initialized = true;
			BaseFormID = StartingFormID;
			Inductees.assign(Entries.begin(), Entries.end());

			int FormIDCounter = StartingFormID;
			for (auto Itr : Inductees)
			{
				Itr->Initialize(FormIDCounter);
				FormIDCounter += 1;
			}

			return Initialized;
		}

		UInt32 HallOfFameManager::GetBaseFormID( void ) const
		{
			return BaseFormID;
		}

		bool HallOfFameManager::GetIsInductee( UInt32 FormID ) const
		{
			for (auto Itr : Inductees)
			{
				if (Itr->GetFormID() == FormID)
					return true;
			}

			return false;
		}
	}
}