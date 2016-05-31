#pragma once

namespace bgsee
{
	namespace extras
	{
		class HallOfFameManager;

		class HallOfFameEntry
		{
		protected:
			const char*				Name;
			const char*				Title;
		public:
			HallOfFameEntry(const char* Name, const char* Title);
			virtual ~HallOfFameEntry() = 0;

			virtual void			Initialize(UInt32 FormID) = 0;
			virtual void			Deinitialize(void) = 0;

			virtual UInt32			GetFormID(void) const = 0;
		};

		typedef std::list<HallOfFameEntry*>		ExtenderHOFEntryListT;

		class HallOfFameManager
		{
			static HallOfFameManager*				Singleton;

			ExtenderHOFEntryListT						Inductees;
			UInt32										BaseFormID;
			bool										Initialized;

			HallOfFameManager();
		public:
			~HallOfFameManager();

			static HallOfFameManager*					GetSingleton(void);

			bool										Initialize(ExtenderHOFEntryListT& Entries, UInt32 StartingFormID = 0x450);
														// takes ownership of the entries

			UInt32										GetBaseFormID(void) const;
			bool										GetIsInductee(UInt32 FormID) const;
		};

#define BGSEEHALLOFFAME									bgsee::extras::HallOfFameManager::GetSingleton()
	}
}