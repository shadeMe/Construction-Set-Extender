#pragma once

namespace cse
{
	// adapted from Timeslip's BSA reading code

	public ref class BSAFileInfo
	{
	public:
		String^										FileName;
		String^										FolderName;
		UInt32										Size;
		UInt32										Offset;
		bool										Compressed;

		BSAFileInfo(String^% FolderName, UInt32 Size, UInt32 Offset, bool Compressed) :
		FolderName(FolderName), Size(Size), Offset(Offset), Compressed(Compressed) {}

		String^										GetRelativePath() { return FolderName + "\\" + FileName; }
	};

	public ref class BSAReader
	{
		static BSAReader^							Singleton = nullptr;

		BSAReader();

		bool										InUse;
		bool										Compressed;
		int											FolderCount;
		int											FileCount;
		List<BSAFileInfo^>^							Files;

		String^										ReadString(BinaryReader^% Stream);
		String^										ReadString(BinaryReader^% Stream, UInt32 Length);
	public:
		static BSAReader^%							GetSingleton();

		virtual bool								OpenArchive(String^% Path);
		virtual void								CloseArchive();
		List<BSAFileInfo^>^%						GetFiles() { return Files; }
	};

#define BSAR									BSAReader::GetSingleton()
}