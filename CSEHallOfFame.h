#pragma once
#include <BGSEEHallOfFame.h>

namespace ConstructionSetExtender
{
	namespace HallOfFame
	{
		class CSEHallOfFameEntry : public BGSEditorExtender::Extras::BGSEEHallOfFameEntry
		{
		protected:
			TESForm*				Instance;
			UInt8					FormType;
		public:
			CSEHallOfFameEntry(const char* Name, const char* Title, UInt8 Type);
			virtual ~CSEHallOfFameEntry();

			virtual void			Initialize(UInt32 FormID);
			virtual void			Deinitialize(void);
			virtual UInt32			GetFormID(void) const;
		};

		class CSEHallOfFameStaff : public CSEHallOfFameEntry
		{
		public:
			CSEHallOfFameStaff(const char* Name, const char* Title);
			virtual ~CSEHallOfFameStaff();

			virtual void			Initialize(UInt32 FormID);
		};

		class CSEHallOfFameFemaleNPC : public CSEHallOfFameEntry
		{
		public:
			CSEHallOfFameFemaleNPC(const char* Name, const char* Title);
			virtual ~CSEHallOfFameFemaleNPC();

			virtual void			Initialize(UInt32 FormID);
		};

		class CSEHallOfFameSpellItem : public CSEHallOfFameEntry
		{
		public:
			CSEHallOfFameSpellItem(const char* Name, const char* Title);
			virtual ~CSEHallOfFameSpellItem();

			virtual void			Initialize(UInt32 FormID);
		};

		class CSEHallOfFameMGEF : public CSEHallOfFameEntry
		{
		public:
			CSEHallOfFameMGEF(UInt32 Name, const char* Title);
			virtual ~CSEHallOfFameMGEF();

			virtual void			Initialize(UInt32 FormID);
		};

		class CSEHallOfFameScript : public CSEHallOfFameEntry
		{
		public:
			CSEHallOfFameScript(const char* Name, const char* Code);
			virtual ~CSEHallOfFameScript();

			virtual void			Initialize(UInt32 FormID);
		};

		class CSEHallOfFameShadeMe : public CSEHallOfFameEntry
		{
		protected:
			TESObjectREFR*			TheGreatEye;
		public:
			CSEHallOfFameShadeMe();
			virtual ~CSEHallOfFameShadeMe();

			virtual void			Initialize(UInt32 FormID);
			virtual void			Deinitialize();
		};

		void						Initialize(void);
		void						GetRandomESMember(std::string& OutName, bool Possessive = true);
	}
}