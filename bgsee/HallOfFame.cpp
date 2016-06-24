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

		HallOfFameManager*			HallOfFameManager::Singleton = NULL;

		HallOfFameManager::HallOfFameManager() :
			BaseFormID(0x450),
			Inductees(),
			Initialized(false)
		{
			;//
		}

		HallOfFameManager::~HallOfFameManager()
		{
			for (ExtenderHOFEntryArrayT::iterator Itr = Inductees.begin(); Itr != Inductees.end(); Itr++)
			{
				(*Itr)->Deinitialize();
				delete *Itr;
			}

			Inductees.clear();
			Initialized = false;

			Singleton = NULL;
		}

		HallOfFameManager* HallOfFameManager::GetSingleton( void )
		{
			if (Singleton == NULL)
				Singleton = new HallOfFameManager();

			return Singleton;
		}

		bool HallOfFameManager::Initialize( ExtenderHOFEntryArrayT& Entries, UInt32 StartingFormID  )
		{
			if (Initialized)
				return false;

			SME_ASSERT(StartingFormID < 0x800 && StartingFormID > 0x400);

			Initialized = true;
			BaseFormID = StartingFormID;
			Inductees = Entries;

			for (ExtenderHOFEntryArrayT::iterator Itr = Inductees.begin(); Itr != Inductees.end(); Itr++)
			{
				(*Itr)->Initialize(StartingFormID);
				StartingFormID++;
			}

			return Initialized;
		}

		UInt32 HallOfFameManager::GetBaseFormID( void ) const
		{
			return BaseFormID;
		}

		bool HallOfFameManager::GetIsInductee( UInt32 FormID ) const
		{
			for (ExtenderHOFEntryArrayT::const_iterator Itr = Inductees.begin(); Itr != Inductees.end(); Itr++)
			{
				if ((*Itr)->GetFormID() == FormID)
					return true;
			}

			return false;
		}
	}
}