#include "BSAReader.h"

BSAReader::BSAReader()
{
	InUse = false;
	Compressed = false;
	Files = gcnew List<BSAFileInfo^>();
	FolderCount = 0;
	FileCount = 0;
}

BSAReader^% BSAReader::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew BSAReader();
	}
	return Singleton;
}

String^ BSAReader::ReadString(BinaryReader^% Stream)
{
	String^ Result;
	Char B;
	do {
		B = Stream->ReadChar();
		if (!B)		break;
		Result += B;
	} while (true);
	return Result;
}

String^ BSAReader::ReadString(BinaryReader^% Stream, UInt32 Length)
{
	String^ Result;
	for (UInt32 i = 0; i < Length; i++) {
		Result += (Char)Stream->ReadByte();
	}
	return Result;
}

void BSAReader::CloseArchive()
{
	Files->Clear();
	FolderCount = 0;
	FileCount = 0;
	InUse = false;
	Compressed = false;
}

bool BSAReader::OpenArchive(String^% Path)
{
	try {
		if (InUse) {
			MessageBox::Show("BSAReader already in use. Please close any open archives before continuing.");
			return false;
		} else
			InUse = true;

		FileInfo^ FI = gcnew FileInfo(Path);
		BinaryReader^ Stream = gcnew BinaryReader(FI->OpenRead());

		if (ReadString(Stream) != "BSA" || Stream->ReadUInt32() != 0x67)
			throw gcnew CSEGeneralException("File isn't a vaild TES4 BSA archive");

		Stream->ReadUInt32();
		UInt32 Flags = Stream->ReadUInt32();
		if (Flags & 0x00000004)		Compressed = true;
		else						Compressed = false;

		FolderCount = Stream->ReadInt32();
		FileCount = Stream->ReadInt32();
		Stream->BaseStream->Position += 12;
		array<int>^ NumberOfFiles = gcnew array<int>(FolderCount);
		for (int i = 0; i < FolderCount; i++) {
			Stream->BaseStream->Position += 8;
			NumberOfFiles[i] = Stream->ReadInt32();
			Stream->ReadUInt32();
		}

		UInt32 Count = 0, Size = 0;
		for (int i = 0; i < FolderCount; i++) {
			String^ Folder = ReadString(Stream, Stream->ReadByte() - 1);
			Stream->ReadByte();
			for (int j = 0; j < NumberOfFiles[i]; j++) {
				Stream->BaseStream->Position += 8;
				Size = Stream->ReadUInt32();
				 bool FileCompressed = Compressed;
				 if (Size & (1 << 30)) {
					FileCompressed = !FileCompressed;
					Size ^= 1 << 30;
				 }
				 Files->Add(gcnew BSAFileInfo(Folder, Size, Stream->ReadUInt32(), FileCompressed));
				 Count++;
			}
		}

		Char C;
		for (int i = 0; i < FileCount; i++) {
			String^ File = "";
			do {
				C = Stream->ReadChar();
				if (C == '\0')		break;
				File += C;
			} while (true);
			Files[i]->FileName = File;
		}

		Stream->Close();
		return true;
	} catch (CSEGeneralException^ E) {
		String^ Message = "Exception raised while processing BSA archive '" + Path + "'!\n\tException: " + E->Message;
		DebugPrint(Message);
		MessageBox::Show(Message, "CSE");
		return false;
	}
}