#include "BGSEEHallOfFame.h"

namespace BGSEditorExtender
{
	namespace Extras
	{
		BGSEEHallOfFameEntry::BGSEEHallOfFameEntry( const char* Name, const char* Title ) :
			Name(Name),
			Title(Title)
		{
			;//
		}

		BGSEEHallOfFameEntry::~BGSEEHallOfFameEntry()
		{
			;//
		}

		BGSEEHallOfFameManager*			BGSEEHallOfFameManager::Singleton = NULL;

		BGSEEHallOfFameManager::BGSEEHallOfFameManager() :
			BaseFormID(0x450),
			Inductees(),
			Initialized(false)
		{
			;//
		}

		BGSEEHallOfFameManager::~BGSEEHallOfFameManager()
		{
			for (ExtenderHOFEntryListT::iterator Itr = Inductees.begin(); Itr != Inductees.end(); Itr++)
			{
				(*Itr)->Deinitialize();
				delete *Itr;
			}

			Inductees.clear();
			Initialized = false;

			Singleton = NULL;
		}

		BGSEEHallOfFameManager* BGSEEHallOfFameManager::GetSingleton( void )
		{
			if (Singleton == NULL)
				Singleton = new BGSEEHallOfFameManager();

			return Singleton;
		}

		bool BGSEEHallOfFameManager::Initialize( ExtenderHOFEntryListT& Entries, UInt32 StartingFormID  )
		{
			if (Initialized)
				return false;

			SME_ASSERT(StartingFormID < 0x800 && StartingFormID > 0x400);

			Initialized = true;
			BaseFormID = StartingFormID;
			Inductees = Entries;

			for (ExtenderHOFEntryListT::iterator Itr = Inductees.begin(); Itr != Inductees.end(); Itr++)
			{
				(*Itr)->Initialize(StartingFormID);
				StartingFormID++;
			}

			return Initialized;
		}

		UInt32 BGSEEHallOfFameManager::GetBaseFormID( void ) const
		{
			return BaseFormID;
		}

		bool BGSEEHallOfFameManager::GetIsInductee( UInt32 FormID )
		{
			for (ExtenderHOFEntryListT::iterator Itr = Inductees.begin(); Itr != Inductees.end(); Itr++)
			{
				if ((*Itr)->GetFormID() == FormID)
					return true;
			}

			return false;
		}
	}
}