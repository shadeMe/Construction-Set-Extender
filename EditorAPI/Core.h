#pragma once

#include "obse\NiNodes.h"
#include "obse\NiObjects.h"
#include "obse\NiProperties.h"
#include "obse\NiRenderer.h"

#include "TESForm.h"
#include "TESSkill.h"

//	EditorAPI: Core classes.

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
class	Sky;
class	BSTextureManager;
class	BSRenderedTexture;
class	NiDX9Renderer;
class	Setting;
class	NiBinaryStream;
class	NiFile;
class	NiRenderTargetGroup;
class	BSFileEntry;
class	BackgroundCloneThread;
class	Model;
class	KFModel;
class	QueuedReference;
class	QueuedAnimIdle;
class	QueuedHelmet;
class	AttachDistant3DTask;

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
	/*00C0*/ NiTLargeArray<TESObjectCELL*>			interiorCellArray;
	/*00D8*/ TESSkill								skills[0x15];
	/*0DF8*/ tList<void*>							unk8B8;     // general garbage list for unsupported form types?, new EffectSettings added here
	/*0E00*/ UInt32									nextFormID; // next available formID?
	/*0E04*/ TESFile*								activeFile;
	/*0E08*/ tList<TESFile>							fileList;   // all files in Oblivion\Data\ directory
	/*0E10*/ UInt32									fileCount;  // loaded files
	/*0E14*/ TESFile*								filesByID[0xFF]; // loaded files
	/*1210*/ UInt8									unkCD0;     // if true, prevents check for changes to masters during load
	/*1211*/ UInt8									unkCD1;     // set if there is an active file??
	/*1212*/ UInt8									unkCD2;		// set when autosaving?
	/*1213*/ UInt8									unkCD3;
	/*1214*/ UInt8									clearingData;     // set when data handler cleared
	/*1215*/ UInt8									unkCD5;     // set after fileList is initialized?
	/*1216*/ UInt8									unkCD6;     // set after data handler is constructed
	/*1217*/ UInt8									unkCD7;     // set after files in LO have been opened, cleared after files have been loaded
	/*1218*/ TESRegionDataManager*					regionDataManager;
	/*121C*/ ContainerExtraData*					unkCDC;

	// methods
	TESFile*										LookupPluginByName(const char* PluginName);
	TESFile*										LookupPluginByIndex(UInt32 Index);
	bool											IsPluginLoaded(TESFile* File);
	void											SortScripts();
	TESForm*										CreateForm(UInt8 TypeID);
	void											PopulatePluginArray(const char* WorkingDirectory);
	void											ClearPluginArray();
	void											AddTESObject(TESObject* Object);
	bool											MoveReference(TESObjectCELL* Cell, TESObjectREFR* Reference);
	TESObjectREFR*									PlaceObjectRef(TESObject* BaseObject,
																   const Vector3* Position, const Vector3* Rotation,
																   TESObjectCELL* Cell, TESWorldSpace* WorldSpace, TESObjectREFR* ExistingRef);
													// places an object ref at the specified position in the specified cell/worldspace, with the specified base form.
													// if existingRef is provided, it is used as the ref, otherwise a new ref is created.  returns null on failure
	void											AutoSave(void);
	bool											SavePlugin(const char* FileName = NULL, bool AsESM = false);
													// if AsESM is set to true, all loaded records will be dumped to the save file regardless of their modified status

	TESObjectCELL*									GetExteriorCell(float XCoord, float YCoord, float ZCoord, bool* Create = NULL, TESWorldSpace* Worldspace = NULL);
	TESObjectCELL*									GetExteriorCell(float XCoord, float YCoord, TESWorldSpace* Worldspace, bool Create = false);
													// if Create is set to true, an exterior cell will be created at the input coords and returned

	bool											AddForm(TESForm* Form);
	void											RemoveCellProcess(TESObjectCELL* Cell);

	void											RemoveInvalidScripts(void);			// destroys any empty scripts, i.e., scripts without script text/editorID
	void											CleanCellWaterExtraData(void);		// removes instances of ExtraCellWaterHeight/Type from all cell objects that don't need it
	void											FixInteriorCellFogPlane(void);		// applies the "nVidia fog" fix

	void											PerformPostLoadTasks(void);

	bool											PanicSave(bool Initialize = false);	// last chance save handler, used when the editor crashes

	static TESDataHandler**							Singleton;

	static bool										PluginLoadSaveInProgress;			// managed by various hooks
};
STATIC_ASSERT(sizeof(TESDataHandler) == 0x1220);

#define _DATAHANDLER			(*TESDataHandler::Singleton)

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

	// 30
	struct	WaterSurfaceManager
	{
		/*00*/ UInt32				unk00;
		/*04*/ UInt32				unk04;
		/*08*/ UInt32				unk08;
		/*0C*/ UInt32				unk0C;
		/*10*/ UInt32				unk10;
		/*14*/ UInt32				unk14;
		/*18*/ UInt32				unk18;
		/*1C*/ UInt32				unk1C;
		/*20*/ UInt32				unk20;
		/*24*/ UInt32				unk24;
		/*28*/ UInt8				unk28;
		/*29*/ UInt8				unk29;					// init to 1
		/*2A*/ UInt16				pad2A;
		/*2C*/ float				unk2C;					// init to 0.0
	};

	// 20
	struct WaterPlaneData
	{
		/*00*/ UInt8				culled;
		/*01*/ UInt8				pad01[3];
		/*04*/ NiNode*				waterNode;
		/*08*/ NiTriShape**			waterPlaneArray;
		/*0C*/ void*				unk0C;
		/*10*/ NiSourceTexture*		unk10;					// current water texture?
		/*14*/ TESObjectCELL*		parent;
		/*18*/ UInt32				unk18;
		/*1C*/ UInt8				flags;
		/*1D*/ UInt8				pad1D[3];
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
	/*20*/ SInt32					extXCoord;				// in the current worldspace
	/*24*/ SInt32					extYCoord;
	/*28*/ SInt32					unk28;					// same as extXCoord, probably gets updated to store some offset into the loaded cell grid
	/*2C*/ SInt32					unk2C;					// same as above but for Y
	/*30*/ TESObjectCELL*			currentExteriorCell;	// set to NULL when the player's in an interior
	/*34*/ TESObjectCELL*			currentInteriorCell;	// set to NULL when the player's in an exterior
	/*38*/ TESObjectCELL**			interiorCellBufferArray;
	/*3C*/ TESObjectCELL**			exteriorCellBufferArray;
	/*40*/ UInt32					unk40;
	/*44*/ UInt32					unk44;
	/*48*/ SInt32					unk48;					// seen caching extXCoord
	/*4C*/ SInt32					unk4C;					// seen caching extYCoord
	/*50*/ UInt8					unk50;					// set/reset when testing cells
	/*51*/ UInt8					unk51;
	/*52*/ UInt8					unk52;
	/*53*/ UInt8					unk53;					// passed to a TESObjectLAND method (recreates land bounds if set?)
	/*54*/ WaterSurfaceManager*		waterSurfaceManager;
	/*58*/ WaterPlaneData*			waterNodeData;			// for the current interior
	/*5C*/ Sky*						sky;
	/*60*/ UInt32					unk60;
	/*64*/ UInt32					unk64;
	/*68*/ UInt32					unk68;
	/*6C*/ float					unk6C;
	/*70*/ float					unk70;
	/*74*/ TESWorldSpace*			currentWorldSpace;
	/*78*/ tList<TESObjectCELL>		unk78;					// active cells in the cell grid
	/*80*/ tList<void>				unk80;
	/*88*/ UInt32					unk88;
	/*8C*/ tList<Unk8C>				list8C;
	/*94*/ NiSourceTexture*			bloodDecals[3];			// blood.dds, lichblood.dds, willothewispblood.dds
	/*A0*/ tList<void>				unkA0;					// data is some struct containing NiNode*
	/*A8*/ UInt32					unkA8;

	virtual bool					VFn00(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, TESWorldSpace* worldspace = NULL);		// calls worldspace->vtbl0x183, if worldspace == NULL, uses the currentWorldspace member

	// methods
	void							LoadCellIntoViewPort(const Vector3* CameraCoordData, TESObjectREFR* Reference);	// arg1 = Camera position if arg is valid, else arg1 = ext. cell coords
																													// coord format: (x << 12) + 2048, (y << 12) + 2048
	void							SetSkyTOD(float TOD);	// actually belongs to the Sky class
	float							GetSkyTOD(void);		// this one too
	void							SetCurrentWorldspace(TESWorldSpace* Worldspace);
	int								PurgeExteriorCellBufer(bool SkipCurrentGrid = false, TESWorldSpace* ParentWorldSpace = NULL);		// returns the no of cells purged

	void							PurgeLoadedResources();
	TESObjectCELL*					GetCurrentCell() const;
	void							ReloadLandscapeTextures();

	static TES**					Singleton;
};
STATIC_ASSERT(sizeof(TES) == 0xAC);

#define _TES					(*TES::Singleton)

// ### appears to be as large as 0x10, the corresponding members in GridCellArray are probably members of this class
// strangely, the c'tor only inits the vtbl ptr. however, vtbl+4 seems to do the same for the other members
// 04+?
class GridArray
{
public:
	// members
	/// *00* / void**					vtbl;

	virtual void						Dtor(bool ReleaseMemory = true) = 0;
};
STATIC_ASSERT(sizeof(GridArray) == 0x4);

// 20
class GridCellArray : public GridArray
{
public:
	// 8+?
	struct CellInfo
	{
		/*00*/ TES::WaterPlaneData*		waterData;
		/*04*/ NiNode*					niNode;		// ### cell's node? confirm
	};

	// 04
	struct GridEntry
	{
		/*00*/ TESObjectCELL*	cell;
		/*04*/ CellInfo*		info;
	};

	// members
	//     /*00*/ GridArray
	/*04*/ SInt32			worldX;				// worldspace x coordinate of cell at center of grid
	/*08*/ SInt32			worldY;				// worldspace y
	/*0C*/ UInt32			size;				// grid is size ^ 2, size = uGridsToLoad
	/*10*/ GridEntry*		grid;				// dynamically alloc'ed array of GridEntry[size ^ 2]
	/*14*/ Vector3			extents;			// x = 4096 * worldX (exterior cells are 4096 square units),  y = 4096 * worldY, z = seen 0.0
												// init'ed with coords and passed to a bhkWorldM method, which calculates the extents

	// methods
	GridEntry*				GetCellEntry(SInt32 X, SInt32 Y);
};
STATIC_ASSERT(sizeof(GridCellArray) == 0x20);

// 14
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
	///*00*/ void**					vtbl;
	/*04*/ NiTArray<const char*>	searchPaths;

	// methods
	virtual void				VFn00(void* Unk01) = 0;

	UInt8						FindFile(const char* Path, UInt32 Unk02 = 0, UInt32 Unk03 = 0, int Unk04 = -1);
	NiBinaryStream*				GetFileStream(const char* Path, bool WriteAccess = false, UInt32 BufferSize = 0x8000);
	void						AddSearchPath(const char* Path);

	static FileFinder**			Singleton;

	static const char*			GetRelativePath(const char* Path, const char* RelativeTo);
};
STATIC_ASSERT(sizeof(FileFinder) == 0x14);

#define _FILEFINDER				(*FileFinder::Singleton)

// 24
class BSRenderedTexture : public NiRefObject
{
public:
	// members
	///*00*/ NiRefObject
	/*08*/ NiRenderTargetGroup*		renderTargets;
	/*0C*/ UInt32					unk0C;
	/*10*/ UInt32					unk10;
	/*14*/ UInt32					unk14;
	/*18*/ UInt32					unk18;
	/*1C*/ UInt32					unk1C;
	/*20*/ NiRenderedTexture*		renderedTexture;

	// methods
	LPDIRECT3DTEXTURE9				ConvertToD3DTexture(UInt32 Width = 0, UInt32 Height = 0);
	void							DeleteInstance(bool ReleaseMemory = 0);
};

// 10
class BSTexturePalette : public NiRefObject
{
public:
	// members
	///*00*/ NiRefObject
	/*08*/ cseOverride::NiTPointerMap<BSFileEntry*, NiPointer<NiTexture>>*		archivedTextures;
	/*0C*/ cseOverride::NiTStringPointerMap<NiPointer<NiTexture>>*				looseTextures;

	// members
	void							ReleaseTextures();

	static BSTexturePalette**		Singleton;
};

#define _TEXTUREPALETTE				(*BSTexturePalette::Singleton)

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

	static BSTextureManager**								Singleton;
};
#define _TEXMGR					(*BSTextureManager::Singleton)

// container class, arbitrarily named
class TESLODTextureGenerator
{
	static void						CreateTextureBuffers(void);
	static void						ReleaseTextureBuffers(void);
public:
	enum
	{
		kLODDiffuseMapGeneratorState_NotInUse = 0,
		kLODDiffuseMapGeneratorState_Partials,
		kLODDiffuseMapGeneratorState_FullMap,
	};

	static UInt8					GeneratorState;

	static LPDIRECT3DTEXTURE9*		D3DTexture32x;
	static LPDIRECT3DTEXTURE9*		D3DTexture64x;
	static LPDIRECT3DTEXTURE9*		D3DTexture128x;
	static LPDIRECT3DTEXTURE9*		D3DTexture512x;
	static LPDIRECT3DTEXTURE9*		D3DTexture1024x;
	static LPDIRECT3DTEXTURE9*		D3DTexture2048x;

	static BSRenderedTexture**		BSTexture32x;
	static BSRenderedTexture**		BSTexture64x;
	static BSRenderedTexture**		BSTexture128x;
	static BSRenderedTexture**		BSTexture512x;
	static BSRenderedTexture**		BSTexture1024x;
	static BSRenderedTexture**		BSTexture2048x;

	// custom buffers
	static LPDIRECT3DTEXTURE9		D3DTexture256x;
	static BSRenderedTexture*		BSTexture256x;
	static LPDIRECT3DTEXTURE9		D3DTexture384x;
	static BSRenderedTexture*		BSTexture384x;
	static LPDIRECT3DTEXTURE9		D3DTexture4096x;
	static BSRenderedTexture*		BSTexture4096x;
	static LPDIRECT3DTEXTURE9		D3DTexture6144x;
	static BSRenderedTexture*		BSTexture6144x;

	static const char*				LODFullTexturePath;
	static const char*				ExteriorSnapshotPathBuffer;
	static TESObjectCELL*			ExteriorSnapshotSource;

	static void						SaveExteriorSnapshot(TESObjectCELL* Exterior, UInt32 Resolution, const char* SavePath);
};

// 1C
class ModelLoader
{
public:
	// members
	/*00*/ LockFreeMap<Model*>*									modelMap;		// LockFreeCaseInsensitiveStringMap<Model>
	/*04*/ LockFreeMap<KFModel*>*								kfMap;			// LockFreeCaseInsensitiveStringMap<KFModel>
	/*08*/ LockFreeMap< NiPointer<QueuedReference*> >*			refMap;			// key is TESObjectREFR*
	/*0C*/ LockFreeMap< NiPointer<QueuedAnimIdle*> >*			idleMap;		// key is AnimIdle*
	/*10*/ LockFreeMap< NiPointer<QueuedHelmet*> >*				helmetMap;		// key is TESObjectREFR*
	/*14*/ LockFreeQueue< NiPointer<AttachDistant3DTask*> >*	distant3DMap;
	/*18*/ BackgroundCloneThread*								bgCloneThread;

	static ModelLoader**										Singleton;
};
#define _MODELLOADER			(*ModelLoader::Singleton)

// 188+
struct ThreadLocalData
{
	// members
	/*000*/ UInt32				unk00[0x184 >> 2];
	/*184*/ UInt8				saveLoadInProgress;		// this flag must be set when saving/loading TESFiles and linking loaded forms
	/*185*/ UInt8				unk185[3];				// more flags or padding

	static ThreadLocalData*		Get();

	static DWORD*				TLSIndex;
};
STATIC_ASSERT(sizeof(ThreadLocalData) == 0x188);
