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

		typedef std::vector<HallOfFameEntry*>		ExtenderHOFEntryArrayT;

		class HallOfFameManager
		{
			static HallOfFameManager*			Singleton;

			ExtenderHOFEntryArrayT				Inductees;
			UInt32								BaseFormID;
			bool								Initialized;

			HallOfFameManager(const ExtenderHOFEntryArrayT& Entries, const UInt32 StartingFormID);
			~HallOfFameManager();
		public:

			static HallOfFameManager*			Get(void);
			static bool							Initialize(const ExtenderHOFEntryArrayT& Entries, const UInt32 StartingFormID = 0x450); // takes ownership of the entries
			static void							Deinitialize();

			UInt32								GetBaseFormID(void) const;
			bool								GetIsInductee(UInt32 FormID) const;
		};

#define BGSEEHALLOFFAME							bgsee::extras::HallOfFameManager::Get()
	}
}