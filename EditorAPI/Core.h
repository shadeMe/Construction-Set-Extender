#pragma once

#include "obse\NiNodes.h"
#include "obse\NiObjects.h"
#include "obse\NiProperties.h"
#include "obse\NiRenderer.h"

#include "TESForm.h"
#include "TESSkill.h"
#include "NiTypes.h"

//	EditorAPI: Core classes.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

class	BSFile;
struct	TrackingData;
class   TESObjectListHead;
class   TESFile;
class   TESForm;
class   TESObject;
class   TESPackage;
class   TESWorldSpace;
class   TESClimate;
class   TESWeather;
class   EnchantmentItem;
class   SpellItem;
class   TESHair;
class   TESEyes;
class   TESRace;
class   TESLandTexture;
class   TESClass;
class   TESFaction;
class   Script;
class   TESSound;
class   TESGlobal;
class   TESTopic;
class   TESQuest;
class   BirthSign;
class   TESCombatStyle;
class   TESLoadScreen;
class   TESWaterForm;
class   TESEffectShader;
class   TESObjectANIO;
class   TESRegionList;
class   TESObjectCELL;
class   TESRegionDataManager;
class   TESSoulGem;
class   TESObjectSTAT;
class   TESObjectCLOT;
class   TESObjectMISC;
class   TESObjectCONT;
class   TESObjectDOOR;
class   TESObjectREFR;
class   ContainerExtraData;
class	TESNPC;
class	GridDistantArray;
class	GridCellArray;
struct	WaterSurfaceManager;		// non-polymorphic; arbitrarily named
class	Sky;
class	BSTextureManager;
class	BSRenderedTexture;
class	NiDX9Renderer;
class	Setting;

void*	FormHeap_Allocate(UInt32 Size);
void	FormHeap_Free(void* Ptr);
void*	Oblivion_DynamicCast(void * SrcObj, UInt32 Arg1, const void * FromType, const void * ToType, UInt32 Arg4);

#define CS_CAST(obj, from, to)								(to *)Oblivion_DynamicCast((void*)(obj), 0, RTTI_ ## from, RTTI_ ## to, 0)

// 08
class ChunkInfo
{
public:
	/*00*/ UInt32    chunkType;		// e.g. 'GRUP', 'GLOB', etc.
	/*04*/ UInt32    chunkLength;	// size from end of chunk header (NOTE: chunkLength field is a UInt16 for non-record chunks on disk)
};

// 14
class RecordInfo : public ChunkInfo
{
public:
	enum RecordFlags
	{
		kRecordFlags__FormFlags                = 0x000A8EE0, // flag bits copied from forms for form records
		kRecordFlags__FileFlags                = 0xFF000091, // flag bits copied from files for TES4 records
		kRecordFlags_Ignored            = /*0C*/ 0x00001000, // record is ignored during loading
		kRecordFlags_Compressed         = /*12*/ 0x00040000, // record data is compressed using ZLib.  Note that group records cannot be compressed
	};

	//     /*00*/ ChunkInfo
	/*08*/ UInt32        recordFlags;
	/*0C*/ UInt32        recordID;
	/*10*/ TrackingData  trackingData;
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
		/*08*/ UInt32        nextFormID; // inluding file index in highest byte
	};

	// compared against size in findData of masters to check if they have changed since last edit
	struct MasterFileData
	{
		/*00*/ DWORD nFileSizeHigh;
		/*04*/ DWORD nFileSizeLow;
	};
	typedef tList<MasterFileData> MasterDataList;
	typedef tList<const char> MasterNameList;

	// 18
	class GroupInfo : public RecordInfo
	{
		//     /*00*/ RecordInfo		// for group records, the size includes the 14 bytes of the header
		/*14*/ UInt32        recordOffset;   // used internally to track header offsets of all open groups
	};
	typedef tList<GroupInfo> GroupList;

	// members
	/*000*/ UInt32               errorState;
	/*004*/ UInt32               unkFile004;
	/*008*/ UInt32               unkFile008;
	/*00C*/ BSFile*              unkFile00C; // temp file for backups?
	/*010*/ BSFile*              bsFile; // used for actual read from / write to disk operations
	/*014*/ UInt32               unkFile014;
	/*018*/ UInt32               unkFile018;
	/*01C*/ char                 fileName[kMAX_PATH];
	/*120*/ char                 filePath[kMAX_PATH]; // relative to "Oblivion\"
	/*224*/ void*                unkFile224; // simple object, no destructor
	/*228*/ UInt32               unkFile228; // init to 0x2800
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
	/*404*/ BSStringT			 authorName;
	/*40C*/ BSStringT			 description;
	/*414*/ void*                currentRecordDCBuffer; // buffer for decompressed record data
	/*418*/ UInt32               currentRecordDCLength; // length of decompressed record data
	/*41C*/ TESFile*             unkFile41C; // file this object was cloned from. used for local copies of network files?

	// methods
	bool						IsActive(void);

	static TESFile*				CreateInstance(const char* WorkingDirectory, const char* FileName, UInt8 OpenMode = NiFile::kFileMode_ReadOnly);
	void						DeleteInstance(bool ReleaseMemory = true);
};
STATIC_ASSERT(sizeof(TESFile) == 0x420);

// 1220
class TESDataHandler
{
public:
	// members
	/*0000*/ TESObjectListHead*						objects;
	/*0004*/ tList<TESPackage>						packages;
	/*000C*/ tList<TESWorldSpace>					worldSpaces;
	/*0014*/ tList<TESClimate>						climates;
	/*001C*/ tList<TESWeather>						weathers;
	/*0024*/ tList<EnchantmentItem>					enchantmentItems;
	/*002C*/ tList<SpellItem>						spellItems;
	/*0034*/ tList<TESHair>							hairs;
	/*003C*/ tList<TESEyes>							eyes;
	/*0044*/ tList<TESRace>							races;
	/*004C*/ tList<TESLandTexture>					landTextures;
	/*0054*/ tList<TESClass>						classes;
	/*005C*/ tList<TESFaction>						factions;
	/*0064*/ tList<Script>							scripts;
	/*006C*/ tList<TESSound>						sounds;
	/*0074*/ tList<TESGlobal>						globals;
	/*007C*/ tList<TESTopic>						topics;
	/*0084*/ tList<TESQuest>						quests;
	/*008C*/ tList<BirthSign>						birthsigns;
	/*0094*/ tList<TESCombatStyle>				    combatStyles;
	/*009C*/ tList<TESLoadScreen>					loadScreens;
	/*00A4*/ tList<TESWaterForm>					waterForms;
	/*00AC*/ tList<TESEffectShader>					effectShaders;
	/*00B4*/ tList<TESObjectANIO>					objectAnios;
	/*00BC*/ TESRegionList*							regionList;
	/*00C0*/ NiTLargeArray<TESObjectCELL*>			cellArray;
	/*00D8*/ TESSkill								skills[0x15];
	/*0DF8*/ tList<void*>							unk8B8;     // general garbage list for unsupported form types?
	/*0E00*/ UInt32									nextFormID; // next available formID?
	/*0E04*/ TESFile*								activeFile;
	/*0E08*/ tList<TESFile>							fileList;   // all files in Oblivion\Data\ directory
	/*0E10*/ UInt32									fileCount;  // loaded files
	/*0E14*/ TESFile*								filesByID[0xFF]; // loaded files
	/*1210*/ UInt8									unkCD0;     // if true, prevents check for changes to masters during load
	/*1211*/ UInt8									unkCD1;     // set if there is an active file??
	/*1212*/ UInt8									unkCD2;
	/*1213*/ UInt8									unkCD3;
	/*1214*/ UInt8									unkCD4;     // set when data handler cleared
	/*1215*/ UInt8									unkCD5;     // set after fileList is initialized?
	/*1216*/ UInt8									unkCD6;     // set after data handler is constructed
	/*1217*/ UInt8									unkCD7;     // set after files in LO have been opened, cleared after files have been loaded
	/*1218*/ TESRegionDataManager*					regionDataManager;
	/*121C*/ ContainerExtraData*					unkCDC;

	// methods
	TESFile*										LookupPluginByName(const char* PluginName);
	TESFile*										LookupPluginByIndex(UInt32 Index);
	void											SortScripts();
	TESForm*										CreateForm(UInt8 TypeID);
	void											PopulatePluginArray(const char* WorkingDirectory);
	void											ClearPluginArray();
	void											AddTESObject(TESObject* Object);
	TESObjectREFR*									PlaceObjectRef(TESObject* BaseObject, Vector3* Position, Vector3* Rotation, TESObjectCELL* Cell, TESWorldSpace* WorldSpace, TESObjectREFR* ExistingRef);
													// places an object ref at the specified position in the specified cell/worldspace, with the specified base form.
													// if existingRef is provided, it is used as the ref, otherwise a new ref is created.  returns null on failure
};
STATIC_ASSERT(sizeof(TESDataHandler) == 0x1220);

extern TESDataHandler**			g_TESDataHandler;
#define _DATAHANDLER			(*g_TESDataHandler)

// AC
class TES
{
public:
	// 08
	struct Unk8C
	{
		/*00*/ TESNPC*	npc;
		/*04*/ UInt32	unk4;	// size?
	};

	// members
	///*00*/ void**					vtbl;					// oddly, vtbl pointer is NULL in global TES object though c'tor initializes it...
	/*04*/ GridDistantArray*		gridDistantArray;
	/*08*/ GridCellArray*			gridCellArray;
	/*0C*/ NiNode*					sceneGraphObjectRoot;
	/*10*/ NiNode*					sceneGraphLandLOD;
	/*14*/ BSTempNodeManager*		tempNodeManager;
	/*18*/ NiDirectionalLight*		sunDirectionalLight;
	/*1C*/ BSFogProperty*			fogProperty;
	/*20*/ UInt32					extXCoord;				// in the current worldspace
	/*24*/ UInt32					extYCoord;
	/*28*/ UInt32					unk28;					// same as unk20?
	/*2C*/ UInt32					unk2C;					// same as unk24?
	/*30*/ TESObjectCELL*			currentExteriorCell;	// set to NULL when the player's in an interior
	/*34*/ TESObjectCELL*			currentInteriorCell;	// set to NULL when the player's in an exterior
	/*38*/ void*					unk38;
	/*3C*/ TESObjectCELL**			cellArray;
	/*40*/ UInt32					unk40;
	/*44*/ UInt32					unk44;
	/*48*/ UInt32					unk48;					// seen caching unk20 in editor
	/*4C*/ UInt32					unk4C;					// seen caching unk24 in editor
	/*50*/ UInt32					unk50;
	/*54*/ WaterSurfaceManager*		waterSurfaceManager;
	/*58*/ void*					unk58;
	/*5C*/ Sky*						sky;
	/*60*/ UInt32					unk60;
	/*64*/ UInt32					unk64;
	/*68*/ UInt32					unk68;
	/*6C*/ float					unk6C;
	/*70*/ float					unk70;
	/*74*/ TESWorldSpace*			currentWorldSpace;
	/*78*/ UInt32					unk78[5];
	/*7C*/ tList<Unk8C>				list8C;
	/*94*/ NiSourceTexture*			bloodDecals[3];			// blood.dds, lichblood.dds, whillothewispblood.dds
	/*A0*/ tList<void*>				listA0;					// data is some struct containing NiNode*
	/*A8*/ UInt32					unkA8;

	virtual void					VFn00();

	// methods
	void							LoadCellIntoViewPort(Vector3* CameraCoordData, TESObjectREFR* Reference);	// arg1 = Camera position if arg is valid, else arg1 = ext. cell coords
																												// coord format: (x << 12) + 2048, (y << 12) + 2048
};
STATIC_ASSERT(sizeof(TES) == 0xAC);

extern TES**					g_TES;
#define _TES					(*g_TES)

// 10
class FileFinder
{
public:
	enum
	{
		kFileStatus_NotFound = 0,
		kFileStatus_Unpacked,
		kFileStatus_Packed
	};

	// members
	/*00*/ NiTArray<const char*>	searchPaths;

	// methods
	UInt8				FindFile(const char* Path, UInt32 Unk02 = 0, UInt32 Unk03 = 0, int Unk04 = -1);
	NiBinaryStream*		GetFileStream(const char* Path, bool WriteAccess = false, UInt32 BufferSize = 0x8000);
	void				AddSearchPath(const char* Path);
};
STATIC_ASSERT(sizeof(FileFinder) == 0x10);

extern FileFinder**				g_FileFinder;
#define _FILEFINDER				(*g_FileFinder)

extern NiDX9Renderer**			g_CSRenderer;

// 48
class BSTextureManager
{
public:
	// ?
	class RenderedTextureData
	{
	public:
		/*00*/ UInt32										unk00;
	};

	// members
	/*00*/ NiTPointerList<RenderedTextureData*>				unk00;
	/*10*/ NiTPointerList<RenderedTextureData*>				unk10;
	/*20*/ NiTPointerList<NiPointer<BSRenderedTexture>>		unk20;
	/*30*/ NiTPointerList<NiPointer<BSRenderedTexture>>		unk30;
	/*40*/ UInt32											unk40;
	/*44*/ UInt32											unk44;

	// methods
	BSRenderedTexture*										CreateTexture(NiRenderer* Renderer, UInt32 Size, UInt32 Flags, UInt32 Unk04 = 0, UInt32 Unk05 = 0);
};
extern BSTextureManager**		g_BSTextureManager;
#define _TEXMGR					(*g_BSTextureManager)

extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture32x;
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture64x;
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture128x;
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture512x;
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture1024x;
extern LPDIRECT3DTEXTURE9*		g_LODD3DTexture2048x;

extern BSRenderedTexture**		g_LODBSTexture32x;
extern BSRenderedTexture**		g_LODBSTexture64x;
extern BSRenderedTexture**		g_LODBSTexture128x;
extern BSRenderedTexture**		g_LODBSTexture512x;
extern BSRenderedTexture**		g_LODBSTexture1024x;
extern BSRenderedTexture**		g_LODBSTexture2048x;

extern LPDIRECT3DTEXTURE9		g_LODD3DTexture256x;
extern BSRenderedTexture*		g_LODBSTexture256x;
extern LPDIRECT3DTEXTURE9		g_LODD3DTexture4096x;
extern BSRenderedTexture*		g_LODBSTexture4096x;
extern LPDIRECT3DTEXTURE9		g_LODD3DTexture8192x;
extern BSRenderedTexture*		g_LODBSTexture8192x;

extern Setting*					g_INILocalMasterPath;