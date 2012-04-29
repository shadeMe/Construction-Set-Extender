#pragma once

namespace BGSEditorExtender
{
	namespace Extras
	{
		class BGSEEHallOfFameManager;

		class BGSEEHallOfFameEntry
		{
		protected:
			const char*				Name;
			const char*				Title;
		public:
			BGSEEHallOfFameEntry(const char* Name, const char* Title);
			virtual ~BGSEEHallOfFameEntry() = 0;

			virtual void			Initialize(UInt32 FormID) = 0;
			virtual void			Deinitialize(void) = 0;
		};

		typedef std::list<BGSEEHallOfFameEntry*>		ExtenderHOFEntryListT;

		class BGSEEHallOfFameManager
		{
			static BGSEEHallOfFameManager*				Singleton;

			ExtenderHOFEntryListT						Inductees;
			UInt32										BaseFormID;
			bool										Initialized;

			BGSEEHallOfFameManager();
		public:
			~BGSEEHallOfFameManager();

			static BGSEEHallOfFameManager*				GetSingleton(void);

			bool										Initialize(UInt32 StartingFormID, ExtenderHOFEntryListT& Entries);
		};

#define BGSEEHALLOFFAME									BGSEEHallOfFameManager::GetSingleton()
	}
}