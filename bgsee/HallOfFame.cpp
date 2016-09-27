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

		HallOfFameManager::HallOfFameManager(const ExtenderHOFEntryArrayT& Entries, const UInt32 StartingFormID) :
			BaseFormID(0x450),
			Inductees(),
			Initialized(false)
		{
			SME_ASSERT(Singleton == nullptr);
			Singleton = this;

			Inductees.reserve(0x20);

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

		HallOfFameManager* HallOfFameManager::Get( void )
		{
			return Singleton;
		}

		bool HallOfFameManager::Initialize( const ExtenderHOFEntryArrayT& Entries, const UInt32 StartingFormID )
		{
			if (Singleton)
				return false;

			HallOfFameManager* Buffer = new HallOfFameManager(Entries, StartingFormID);
			return Buffer->Initialized;
		}

		void HallOfFameManager::Deinitialize()
		{
			SME_ASSERT(Singleton);
			delete Singleton;
		}

		UInt32 HallOfFameManager::GetBaseFormID(void) const
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