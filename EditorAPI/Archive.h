#pragma once

#include "BSFile.h"

//	EditorAPI: Archive class and co.

/*
	...
*/

class	Archive;

// 15C
// represents a file in an archive
class ArchiveFile : public BSFile
{
public:
	// members
	//     /*000*/ BSFile
	/*154*/ Archive*				parentArchive;
	/*158*/ UInt32					archiveOffset;
};
STATIC_ASSERT(sizeof(ArchiveFile) == 0x15C);

// 174
class CompressedArchiveFile : public ArchiveFile
{
public:
	// 48
	class ZLibInflateStream
	{
	public:
		/*00*/ UInt32				unk00[(0x20) >> 2];
		/*20*/ void*				allocateBufferFn;
		/*24*/ void*				releaseBufferFn;
		/*28*/ UInt32				unk028[(0x48 - 0x28) >> 2];
	};

	// members
	//     /*000*/ ArchiveFile
	/*15C*/ ZLibInflateStream*		zlibStream;
	/*160*/ void*					fileBuffer;
	/*164*/ UInt32					unk164;		// buffer size
	/*168*/ UInt32					unk168;		// init to unk164
	/*16C*/ UInt32					unk16C;		// init to 0
	/*170*/ UInt32					unk170;		// init to 0
};
STATIC_ASSERT(sizeof(CompressedArchiveFile) == 0x174);

// a.k.a BSHash, in the database
// 8 (not confirmed but unlikely to be larger)
class BSFilePathHash
{
public:
	// members
	/*00*/ UInt32					unk00;
	/*04*/ UInt32					unk04;
};
STATIC_ASSERT(sizeof(BSFilePathHash) == 0x08);

// 24
class BSArchiveHeader
{
public:
	enum
	{
		kHeaderFlags_HasDirNames	=	/*00*/ 0x001,	// always set
		kHeaderFlags_HasFileNames	=	/*01*/ 0x002,	// always set
		kHeaderFlags_Compressed		=	/*02*/ 0x004,
		kHeaderFlags_Unk03			=	/*03*/ 0x008,	// retain directory string table?
		kHeaderFlags_Unk04			=	/*04*/ 0x010,	// retain filename string table?
		kHeaderFlags_Unk05			=	/*05*/ 0x020,	// retain filename offset table?
		kHeaderFlags_Unk07			=	/*07*/ 0x080
	};

	enum
	{
		kFileFlags_HasNIF			=	/*00*/ 0x001,	// includes KF
		kFileFlags_HasDDS			=	/*01*/ 0x002,
		kFileFlags_HasXML			=	/*02*/ 0x004,
		kFileFlags_HasWAV			=	/*03*/ 0x008,
		kFileFlags_HasMP3			=	/*04*/ 0x010,
		kFileFlags_HasTXT			=	/*05*/ 0x020,	// includes HTML, BAT, SCC, PSO, VSO, VSH, PSH, (H)LSL
		kFileFlags_HasSPT			=	/*06*/ 0x040,
		kFileFlags_HasFNT			=	/*07*/ 0x080,	// includes TEX
		kFileFlags_HasCTL			=	/*08*/ 0x100	
	};

	// members
	/*00*/ UInt32					id;					// init to 'BSA'
	/*04*/ UInt32					archiveVersion;		// init to 0x67
	/*08*/ UInt32					headerSize;			// init to 0x24
	/*0C*/ UInt32					headerFlags;
	/*10*/ UInt32					dirCount;
	/*14*/ UInt32					fileCount;
	/*18*/ UInt32					dirNamesLength;
	/*1C*/ UInt32					fileNamesLength;
	/*20*/ UInt32					fileFlags;
};
STATIC_ASSERT(sizeof(BSArchiveHeader) == 0x24);

// 24
class BSArchive : public BSArchiveHeader
{
public:
	// members
	//     /*00*/ BSArchiveHeader

	// no additional members
};
STATIC_ASSERT(sizeof(BSArchive) == 0x24);

// 280
class Archive : public BSFile, public BSArchive
{
public:
	enum
	{
		kArchiveFlags_Unidentified		=	/*00*/ 0x001,		// set when the header ID != 'BSA' || archiveVersion != 0x67
		kArchiveFlags_Unk02				=	/*01*/ 0x002,		// if set, the archive is released after its last open file is closed
		kArchiveFlags_Unk03				=	/*03*/ 0x008,		// don't load contents, set when ctor arg4 != 0 and BSFile's open
		kArchiveFlags_HasDirectoryTable	=	/*04*/ 0x010,
		kArchiveFlags_HasFileNameTable	=	/*05*/ 0x020
	};

	// used to represent both directories and files
	// the sign bit is used to signify something in both the class members(regarding invalidation, for files?)
	// 10
	class ArchiveContentRawData : public BSFilePathHash
	{
	public:
		// members
		//     /*00*/ BSFilePathHash
		/*08*/ UInt32						entries;	// no of files for directory instances
		/*0C*/ ArchiveContentRawData**		buffer;		// probably a union { void*, ArchiveContentRawData** }, the former for file instances and the latter for directories
	};

	// members
	//     /*000*/ BSFile
	//     /*154*/ BSArchiveHeader
	/*178*/ ArchiveContentRawData**	rawData;					// contains as many objects as the archive's dirCount
	/*17C*/ UInt32					unk17C;
	/*180*/ __time64_t				lastModifiedTimeStamp;
	/*188*/ SInt32					fileNameTableOffset;
	/*18C*/ SInt32					unk18C;						// last accessed directory index?
	/*190*/ SInt32					lastAccessedFileIndex;
	/*194*/ UInt8					archiveFlags;
	/*195*/ UInt8					pad195[3];
	/*198*/ char**					directoryNameTable;			// array of directory names, terminated by a ' ' entry
	/*19C*/ UInt32*					directoryNameLengthTable;	// array of string lengths
	/*1A0*/ char**					fileNameTable;
	/*1A4*/ UInt32*					fileOffsetTable;			// corresponding to the entries in fileNameTable, each stores the offset into the raw data
	/*1A8*/ volatile LONG			openFiles;					// number of active child ArchiveFile/CompressedArchiveFile instances
	/*1AC*/ UInt8					releaseFlag;				// set by Archive::CheckDelete, call d'tor if set
	/*1AD*/ UInt8					pad1AD[3];
	/*1B0*/ UInt8					unk1B0[0x50];
	/*200*/ CRITICAL_SECTION		criticalSection;
	/*218*/ UInt8					unk218[0x68];
};
STATIC_ASSERT(sizeof(Archive) == 0x280);

typedef tList<Archive>		ArchiveListT;

class ArchiveManager
{
	static Archive*				LoadArchive(const char* ArchiveName, UInt16 Unk02 = 0, UInt8 Unk03 = 0);
public:
	// methods
	static void					LoadSkippedArchives(const char* ArchiveDirectory);
	static bool					ExtractArchiveFile(const char* InPath, const char* OutPath, bool AppendFilePath = false);

	static ArchiveListT**		LoadedArchives;
};