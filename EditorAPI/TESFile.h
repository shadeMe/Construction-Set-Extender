#pragma once
#include "TESForm.h"
#include "NiClasses.h"

//	EditorAPI: TESFile and related classes.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

namespace ConstructionSetExtender_OverriddenClasses
{
	template <class TKEY, class TVAL> class NiTPointerMap;
}

class	BSFile;

/*
	A "chunk" is the basic unit of the TES4 mod file. A chunk is headed by a type code and size field, followed by binary data
	This struct is public (not a subclass of TESFile) because it is also used by RecordInfo
*/

// 08 (NOTE: 06 for non-record chunks on disk, see below)
class ChunkInfo
{
public:
	/*00*/ UInt32    chunkType;		// e.g. 'GRUP', 'GLOB', etc.
	/*04*/ UInt32    chunkLength;	// size from end of chunk header (NOTE: chunkLength field is a UInt16 for non-record chunks on disk)
};

/*
	Records are chunks whose data consists entirely of subchunks.  They have additional identifying information in the header
	During serialization, each form corresponds to a single record, but there are also a few records (e.g. GRUP,TES4) that do not correspond to any form
	This struct is public (not a subclass of TESFile) because it is also used by TESForm.h
*/

// 14
class RecordInfo : public ChunkInfo
{
public:
	enum RecordFlags
	{
		// not sure what flags are specific to forms, references, or records, and what is shared
		kRecordFlags__FormFlags                = 0x000A8EE0, // flag bits copied from forms for form records
		kRecordFlags__FileFlags                = 0xFF000091, // flag bits copied from files for TES4 records
		kRecordFlags_Ignored            = /*0C*/ 0x00001000, // record is ignored during loading
		kRecordFlags_Compressed         = /*12*/ 0x00040000, // record data is compressed using ZLib.  Note that group records cannot be compressed
	};

	//     /*00*/ ChunkInfo
	/*08*/ UInt32        recordFlags; //
	// form records: see 'record flags' above
	// TES4 records: fileFlags
	// group records: contained form chunk type / parent formid / block number / grid coords
	/*0C*/ UInt32        recordID; //
	// form records: formID
	// TES4 records: 0
	// group records: group class (0-10) indicating what kind of group record
	/*10*/ TrackingData  trackingData; // used for internal revision control
};

// 420
class TESFile
{
public:
	static const UInt32 kMAX_PATH = 260;			// i.e. windows constant MAX_PATH

	enum FileFlags
	{
		kFileFlag__SavedInRecord       = 0xFF000091, // flag bits copied from TES4 record
		kFileFlag_Master        = /*00*/ 0x00000001, // set for master files (as indicated by bit in TES4 record flags)
		kFileFlag_Open          = /*01*/ 0x00000002, // set in OpenBSFile()
		kFileFlag_Loaded        = /*02*/ 0x00000004, // flags file for loading, set before file is actually loaded
		kFileFlag_Active        = /*03*/ 0x00000008, // set for the currently active file
	};

	enum FileErrorStates
	{
		kFileState_None         = 0x0,
		kFileState_Unk2         = 0x2, // set in OpenBSFile
		kFileState_Unk9         = 0x9, // set in OpenBSFile
		kFileState_WriteError   = 0xA,
		kFileState_UnkC         = 0xC, // set in OpenBSFile
	};

	struct FileHeaderInfo
	{
		/*00*/ float         fileVersion;
		/*04*/ UInt32        numRecords; // number of record blocks in file
		/*08*/ UInt32        nextFormID; // including file index in highest byte
	};

	// Data for Master files - compared against size in findData of masters to check if they have changed since last edit
	// 08
	struct MasterFileData
	{
		/*00*/ DWORD nFileSizeHigh;
		/*04*/ DWORD nFileSizeLow;
	};
	typedef tList<MasterFileData> MasterDataList;
	typedef tList<const char> MasterNameList;

	// Group record struct
	// 18
	class GroupInfo : public RecordInfo
	{
		//     /*00*/ RecordInfo		// for group records, the size includes the 14 bytes of the header
		/*14*/ UInt32        recordOffset;   // used internally to track header offsets of all open groups
	};
	typedef tList<GroupInfo> GroupList;
	typedef ConstructionSetExtender_OverriddenClasses::NiTPointerMap< UInt32, TESFile* > ChildThreadFileMapT;

	// members
	/*000*/ UInt32               errorState;
	/*004*/ TESFile*             ghostFileParent; // for ghost files, the parent TESFile* from the main thread
	/*008*/ ChildThreadFileMapT* childThreadGhostFiles; // read-only duplicate files mapped by threadID for child threads
	/*00C*/ BSFile*              unkFile00C; // temp file for backups?
	/*010*/ BSFile*              bsFile; // used for actual read from / write to disk operations
	/*014*/ UInt32               unkFile014;
	/*018*/ UInt32               unkFile018;
	/*01C*/ char                 fileName[kMAX_PATH];
	/*120*/ char                 filePath[kMAX_PATH]; // relative to "Oblivion\"
	/*224*/ void*                unkFile224; // simple object, no destructor
	/*228*/ UInt32               bufferSize; // buffer size used when opening BSFile.  init to 0x2800
	/*22C*/ UInt32               unkFile22C;
	/*230*/ UInt32               unkFile230;
	/*234*/ UInt32               unkFile234;
	/*238*/ UInt32               unkFile238;
	/*23C*/ RecordInfo           currentRecord;
	/*250*/ ChunkInfo            currentChunk;
	/*258*/ UInt32               fileSize; // same as FileSizeLow in find data
	/*25C*/ UInt32               currentRecordOffset; // offset of current record in file
	/*260*/ UInt32               currentChunkOffset; // offset of current chunk in record
	/*264*/ UInt32               fetchedChunkDataSize; // number of bytes read in last GetChunkData() call
	/*268*/ GroupInfo            unkFile268; // used when saving empty form records, e.g. for deleted forms
	/*280*/ UInt32               unkFile280; // used when saving empty form records, e.g. for deleted forms
	/*284*/ GroupList            openGroups; // stack of open group records, from lowest level to highest
	/*28C*/ bool                 headerRead; // set after header has been successfully parsed
	/*28D*/ UInt8                padFile28D[3];
	/*290*/ WIN32_FIND_DATA      findData;
	/*3D0*/ FileHeaderInfo       fileHeader;
	/*3DC*/ UInt32               fileFlags;
	/*3E0*/ MasterNameList       masterNames;
	/*3E8*/ MasterDataList       masterData;
	/*3F0*/ UInt32               masterCount;
	/*3F4*/ TESFile**            masterFiles; // pointer to TESFile*[parentCount] of currently loaded masters
	/*3F8*/ UInt32               unkFile3F8;
	/*3FC*/ UInt32               unkFile3FC;
	/*400*/ UInt8                fileIndex; // index of this file in load order (or 0xFF if not loaded)
	/*401*/ UInt8                padFile401[3];
	/*404*/ BSString			 authorName;
	/*40C*/ BSString             description;
	/*414*/ void*                currentRecordDCBuffer; // buffer for decompressed record data
	/*418*/ UInt32               currentRecordDCLength; // length of decompressed record data
	/*41C*/ TESFile*             unkFile41C; // file this object was cloned from. used for local copies of network files?

	// methods
	bool						IsActive(void) const;
	bool						IsMaster(void) const;
	bool						IsLoaded(void) const;

	bool						SetLoaded(bool State);
	bool						SetActive(bool State);
	bool						SetMaster(bool State);

	static TESFile*				CreateInstance(const char* WorkingDirectory, const char* FileName, UInt8 OpenMode = NiFile::kFileMode_ReadOnly);
	void						DeleteInstance(bool ReleaseMemory = true);
};
STATIC_ASSERT(sizeof(TESFile) == 0x420);