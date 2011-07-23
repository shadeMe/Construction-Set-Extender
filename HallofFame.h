#pragma once

class TESForm;

// vanity would an understatement
namespace HallOfFame
{
	const UInt32		kBaseFormID = 0x450;

	struct HallOfFameEntry
	{
		const char*		EditorID;
		UInt8			FormType;
		TESForm*		Form;
		const char*		Name;
	};

	extern UInt32 TableSize;
	extern HallOfFameEntry	Entries[];

	void __stdcall				Initialize(bool ResetCSWindows);
}