#pragma once

#include "obse\NiNodes.h"
#include "obse\NiObjects.h"
#include "obse\NiProperties.h"
#include "obse\NiRenderer.h"

#include "TESForm.h"
#include "TESSkill.h"

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
class	Sky;
class	BSTextureManager;
class	BSRenderedTexture;
class	NiDX9Renderer;
class	Setting;
class	NiBinaryStream;
class	NiFile;

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
	void											SortScripts();
	TESForm*										CreateForm(UInt8 TypeID);
	void											PopulatePluginArray(const char* WorkingDirectory);
	void											ClearPluginArray();
	void											AddTESObject(TESObject* Object);
	bool											MoveReference(TESObjectCELL* Cell, TESObjectREFR* Reference);
	TESObjectREFR*									PlaceObjectRef(TESObject* BaseObject, Vector3* Position, Vector3* Rotation, TESObjectCELL* Cell, TESWorldSpace* WorldSpace, TESObjectREFR* ExistingRef);
													// places an object ref at the specified position in the specified cell/worldspace, with the specified base form.
													// if existingRef is provided, it is used as the ref, otherwise a new ref is created.  returns null on failure
	void											AutoSave(void);
	bool											SavePlugin(const char* FileName = NULL, bool AsESM = false);
													// if AsESM is set to true, all loaded records will be dumped to the save file regardless of their modified status

	TESObjectCELL*									GetExteriorCell(float XCoord, float YCoord, float ZCoord, bool* Create = NULL, TESWorldSpace* Worldspace = NULL);
	TESObjectCELL*									GetExteriorCell(float XCoord, float YCoord, TESWorldSpace* Worldspace, bool Create = false);
													// if Create is set to true, an exterior cell will be created at the input coords and returned

	void											CleanCellWaterExtraData(void);		// removes instances of ExtraCellWaterHeight/Type from all cell objects that don't need it
	bool											PanicSave(bool Initialize = false);	// last chance save handler, used when the editor crashes
};
STATIC_ASSERT(sizeof(TESDataHandler) == 0x1220);

extern TESDataHandler**			g_TESDataHandler;
#define _DATAHANDLER			(*g_TESDataHandler)

// 04
class GridArray
{
public:
	// members
	/// *00* / void**					vtbl;

	virtual void						Dtor(bool ReleaseMemory = true) = 0;
};
STATIC_ASSERT(sizeof(GridArray) == 0x4);

// 020
class GridCellArray : public GridArray
{
public:
	// 8+?
	struct CellInfo
	{
		/*00*/ UInt32		unk00;
		/*04*/ NiNode*		niNode;
	};

	// 04
	struct GridEntry
	{
		/*00*/ TESObjectCELL*	cell;
		/*04*/ CellInfo*		info;
	};

	// members
	//     /*00*/ GridArray
	/*04*/ UInt32			worldX;		// worldspace x coordinate of cell at center of grid
	/*08*/ UInt32			worldY;		// worldspace y
	/*0C*/ UInt32			size;		// grid is size ^ 2, size = uGridsToLoad
	/*10*/ GridEntry*		grid;		// dynamically alloc'ed array of GridEntry[size ^ 2]
	/*14*/ float			posX;		// 4096 * worldX (exterior cells are 4096 square units)
	/*18*/ float			posY;		// 4096 * worldY
	/*1C*/ float			unk1C;		// seen 0.0
};
STATIC_ASSERT(sizeof(GridCellArray) == 0x20);

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
		/*00*/ UInt8				unk00;					// seen 0
		/*01*/ UInt8				pad01[3];
		/*04*/ NiNode*				waterNode;
		/*08*/ NiTriShape**			waterPlaneArray;
		/*0C*/ void*				unk0C;
		/*10*/ NiSourceTexture*		unk10;					// current water texture?
		/*14*/ TESObjectCELL*		currentCell;
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
	/*20*/ UInt32					extXCoord;				// in the current worldspace
	/*24*/ UInt32					extYCoord;
	/*28*/ UInt32					unk28;					// same as unk20?
	/*2C*/ UInt32					unk2C;					// same as unk24?
	/*30*/ TESObjectCELL*			currentExteriorCell;	// set to NULL when the player's in an interior
	/*34*/ TESObjectCELL*			currentInteriorCell;	// set to NULL when the player's in an exterior
	/*38*/ TESObjectCELL**			interiorCellBufferArray;
	/*3C*/ TESObjectCELL**			exteriorCellBufferArray;
	/*40*/ UInt32					unk40;
	/*44*/ UInt32					unk44;
	/*48*/ UInt32					unk48;					// seen caching unk20 in editor
	/*4C*/ UInt32					unk4C;					// seen caching unk24 in editor
	/*50*/ UInt32					unk50;
	/*54*/ WaterSurfaceManager*		waterSurfaceManager;
	/*58*/ WaterPlaneData*			waterNodeData;
	/*5C*/ Sky*						sky;
	/*60*/ UInt32					unk60;
	/*64*/ UInt32					unk64;
	/*68*/ UInt32					unk68;
	/*6C*/ float					unk6C;
	/*70*/ float					unk70;
	/*74*/ TESWorldSpace*			currentWorldSpace;
	/*78*/ tList<void>				unk78;
	/*80*/ tList<void>				unk80;
	/*88*/ UInt32					unk88;
	/*8C*/ tList<Unk8C>				list8C;
	/*94*/ NiSourceTexture*			bloodDecals[3];			// blood.dds, lichblood.dds, willothewispblood.dds
	/*A0*/ tList<void*>				listA0;					// data is some struct containing NiNode*
	/*A8*/ UInt32					unkA8;

	virtual void					VFn00();

	// methods
	void							LoadCellIntoViewPort(const Vector3* CameraCoordData, TESObjectREFR* Reference);	// arg1 = Camera position if arg is valid, else arg1 = ext. cell coords
																												// coord format: (x << 12) + 2048, (y << 12) + 2048
	void							SetSkyTOD(float TOD);	// actually belongs to the Sky class
	float							GetSkyTOD(void);		// this one too
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
#define _RENDERER				(*g_CSRenderer)

class NiRenderTargetGroup;

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

class BSFileEntry;

// 10
class BSTexturePalette : public NiRefObject
{
public:
	// members
	///*00*/ NiRefObject
	/*08*/ CSE_GlobalClasses::NiTPointerMap<BSFileEntry*, NiPointer<NiTexture>>*		archivedTextures;
	/*0C*/ CSE_GlobalClasses::NiTStringPointerMap<NiPointer<NiTexture>>*				looseTextures;
};

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
extern LPDIRECT3DTEXTURE9		g_LODD3DTexture384x;
extern BSRenderedTexture*		g_LODBSTexture384x;
extern LPDIRECT3DTEXTURE9		g_LODD3DTexture4096x;
extern BSRenderedTexture*		g_LODBSTexture4096x;
extern LPDIRECT3DTEXTURE9		g_LODD3DTexture6144x;
extern BSRenderedTexture*		g_LODBSTexture6144x;