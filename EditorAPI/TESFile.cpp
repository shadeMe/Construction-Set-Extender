#include "TESFile.h"

TESFile* TESFile::CreateInstance(const char* WorkingDirectory, const char* FileName,  UInt8 OpenMode)
{
	TESFile* NewInstance = (TESFile*)FormHeap_Allocate(sizeof(TESFile));
	thisCall<UInt32>(0x00489590, NewInstance, WorkingDirectory, FileName, OpenMode);
	return NewInstance;
}

void TESFile::DeleteInstance(bool ReleaseMemory)
{
	thisCall<UInt32>(0x00487E60, this);
	if (ReleaseMemory)
		FormHeap_Free(this);
}

bool TESFile::IsActive( void ) const
{
	return fileFlags & kFileFlag_Active;
}

bool TESFile::IsMaster( void ) const
{
	return fileFlags & kFileFlag_Master;
}

bool TESFile::IsLoaded( void ) const
{
	return fileFlags & kFileFlag_Loaded;
}

bool TESFile::SetLoaded( bool State )
{
	return thisCall<bool>(0x00485B70, this, State);
}

bool TESFile::SetActive( bool State )
{
	return thisCall<bool>(0x00485BB0, this, State);
}

bool TESFile::SetMaster( bool State )
{
	return thisCall<bool>(0x00485B10, this, State);
}

UInt32 TESFile::Open()
{
	return thisCall<UInt32>(0x004891F0, this);
}

UInt32 TESFile::SaveHeader()
{
	return thisCall<UInt32>(0x00489070, this);
}

UInt32 TESFile::CorrectHeader()
{
	return thisCall<UInt32>(0x00487980, this);
}

bool TESFile::Close()
{
	return thisCall<bool>(0x004877A0, this);
}

UInt8 TESFile::GetRecordType()
{
	return thisCall<UInt8>(0x00488130, this);
}

bool TESFile::GetNextRecord( bool SkipIgnoredRecords )
{
	return thisCall<bool>(0x004890A0, this);
}

bool TESFile::InitializeBSFile( UInt32 Arg1 /*= 0*/, bool Arg2 /*= false*/ )
{
	return thisCall<bool>(0x004889A0, this, Arg1, Arg2);
}

bool TESFile::CreateTempFile( UInt32 Arg1 /*= 1*/ )
{
	return thisCall<bool>(0x00488D60, this, Arg1);
}

void TESFile::OpenGroupRecord( RecordInfo* GroupRecord )
{
	thisCall<void>(0x00487500, this, GroupRecord);
}

void TESFile::CloseGroupRecord()
{
	thisCall<void>(0x00487580, this);
}

void TESFile::SetFileIndex( UInt8 Index )
{
	thisCall<void>(0x00485CA0, this, Index);
}
