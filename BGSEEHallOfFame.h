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

			virtual UInt32			GetFormID(void) const = 0;
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

			bool										Initialize(ExtenderHOFEntryListT& Entries, UInt32 StartingFormID = 0x450);
														// takes ownership of the entries

			UInt32										GetBaseFormID(void) const;
			bool										GetIsInductee(UInt32 FormID) const;
		};

#define BGSEEHALLOFFAME									BGSEditorExtender::Extras::BGSEEHallOfFameManager::GetSingleton()
	}
}