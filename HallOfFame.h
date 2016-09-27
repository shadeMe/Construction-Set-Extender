#pragma once
#include <bgsee\HallOfFame.h>

namespace cse
{
	namespace hallOfFame
	{
		class HallOfFameEntry : public bgsee::extras::HallOfFameEntry
		{
		protected:
			TESForm*				Instance;
			UInt8					FormType;
		public:
			HallOfFameEntry(const char* Name, const char* Title, UInt8 Type);
			virtual ~HallOfFameEntry();

			virtual void			Initialize(UInt32 FormID);
			virtual void			Deinitialize(void);
			virtual UInt32			GetFormID(void) const;
		};

		class HallOfFameStaff : public HallOfFameEntry
		{
		public:
			HallOfFameStaff(const char* Name, const char* Title);
			virtual ~HallOfFameStaff();

			virtual void			Initialize(UInt32 FormID);
		};

		class HallOfFameFemaleNPC : public HallOfFameEntry
		{
		public:
			HallOfFameFemaleNPC(const char* Name, const char* Title);
			virtual ~HallOfFameFemaleNPC();

			virtual void			Initialize(UInt32 FormID);
		};

		class HallOfFameSpellItem : public HallOfFameEntry
		{
		public:
			HallOfFameSpellItem(const char* Name, const char* Title);
			virtual ~HallOfFameSpellItem();

			virtual void			Initialize(UInt32 FormID);
		};

		class HallOfFameMGEF : public HallOfFameEntry
		{
		public:
			HallOfFameMGEF(UInt32 Name, const char* Title);
			virtual ~HallOfFameMGEF();

			virtual void			Initialize(UInt32 FormID);
		};

		class HallOfFameScript : public HallOfFameEntry
		{
		public:
			HallOfFameScript(const char* Name, const char* Code);
			virtual ~HallOfFameScript();

			virtual void			Initialize(UInt32 FormID);
		};

		class HallOfFameShadeMe : public HallOfFameEntry
		{
		protected:
			TESObjectREFR*			TheGreatEye;
		public:
			HallOfFameShadeMe();
			virtual ~HallOfFameShadeMe();

			virtual void			Initialize(UInt32 FormID);
			virtual void			Deinitialize();
		};

		void						Initialize(void);
		void						Deinitialize();

		void						GetRandomESMember(std::string& OutName, bool Possessive = true);
		enum
		{
			kDisplayESMember_None	= 0,
			kDisplayESMember_ObjectPreview,
			kDisplayESMember_ObjectPreviewEdit,
		};
	}
}