#include "Core.h"

TES**								g_TES = (TES**)0x00A0ABB0;
TESDataHandler**					g_TESDataHandler = (TESDataHandler **)0x00A0E064;
FileFinder**						g_FileFinder = (FileFinder**)0x00A0DE8C;
BSTextureManager**					g_BSTextureManager = (BSTextureManager**)0x00A8E760;
NiDX9Renderer**						g_CSRenderer = (NiDX9Renderer**)0x00A0F87C;

LPDIRECT3DTEXTURE9*					g_LODD3DTexture32x = (LPDIRECT3DTEXTURE9*)0x00A0AAC4;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture64x = (LPDIRECT3DTEXTURE9*)0x00A0AAC0;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture128x = (LPDIRECT3DTEXTURE9*)0x00A0AABC;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture512x = (LPDIRECT3DTEXTURE9*)0x00A0AAC8;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture1024x = (LPDIRECT3DTEXTURE9*)0x00A0AAD0;
LPDIRECT3DTEXTURE9*					g_LODD3DTexture2048x = (LPDIRECT3DTEXTURE9*)0x00A0AACC;

BSRenderedTexture**					g_LODBSTexture32x = (BSRenderedTexture**)0x00A0AADC;
BSRenderedTexture**					g_LODBSTexture64x = (BSRenderedTexture**)0x00A0AAD8;
BSRenderedTexture**					g_LODBSTexture128x = (BSRenderedTexture**)0x00A0AAD4;
BSRenderedTexture**					g_LODBSTexture512x = (BSRenderedTexture**)0x00A0AAE0;
BSRenderedTexture**					g_LODBSTexture1024x = (BSRenderedTexture**)0x00A0AAE8;
BSRenderedTexture**					g_LODBSTexture2048x = (BSRenderedTexture**)0x00A0AAE4;

LPDIRECT3DTEXTURE9					g_LODD3DTexture256x = NULL;
BSRenderedTexture*					g_LODBSTexture256x = NULL;
LPDIRECT3DTEXTURE9					g_LODD3DTexture4096x = NULL;
BSRenderedTexture*					g_LODBSTexture4096x = NULL;
LPDIRECT3DTEXTURE9					g_LODD3DTexture8192x = NULL;
BSRenderedTexture*					g_LODBSTexture8192x = NULL;

Setting*							g_INILocalMasterPath = (Setting*)0x009ED710;

void* FormHeap_Allocate( UInt32 Size )
{
	return cdeclCall<void*>(0x00401E80, Size);
}

void FormHeap_Free( void* Ptr )
{
	cdeclCall<UInt32>(0x00401EA0, Ptr);
}

void* Oblivion_DynamicCast( void * SrcObj, UInt32 Arg1, const void * FromType, const void * ToType, UInt32 Arg4 )
{
	return cdeclCall<void*>(0x0088DC0C, SrcObj, Arg1, FromType, ToType, Arg4);
}

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

bool TESFile::IsActive( void )
{
	return thisCall<bool>(0x00485BA0, this);
}

TESFile* TESDataHandler::LookupPluginByName(const char* PluginName)
{
	for (tList<TESFile>::Iterator Itr = fileList.Begin(); !Itr.End(); ++Itr)
	{
		TESFile* Plugin = Itr.Get();
		if (!Plugin)
			break;

		if (!_stricmp(Plugin->fileName, PluginName))
			return Plugin;
	}

	return NULL;
}

TESFile* TESDataHandler::LookupPluginByIndex(UInt32 Index)
{
	return thisCall<TESFile*>(0x0047BEC0, this, Index);
}

void TESDataHandler::SortScripts()
{
	thisCall<UInt32>(0x0047BA30, this);
}

TESForm* TESDataHandler::CreateForm(UInt8 TypeID)
{
	return thisCall<TESForm*>(0x004793F0, this, TypeID);
}

void TESDataHandler::PopulatePluginArray(const char* WorkingDirectory)
{
	thisCall<UInt32>(0x0047E4C0, this, WorkingDirectory);
}

void TESDataHandler::AddTESObject(TESObject* Object)
{
	this->objects->AddObject(Object);
}

TESObjectREFR* TESDataHandler::PlaceObjectRef( TESObject* BaseObject, Vector3* Position, Vector3* Rotation, TESObjectCELL* Cell, TESWorldSpace* WorldSpace, TESObjectREFR* ExistingRef )
{
	return thisCall<TESObjectREFR*>(0x0047A060, this, BaseObject, Position, Rotation, Cell, WorldSpace, ExistingRef);
}

void TESDataHandler::ClearPluginArray()
{
	for (tList<TESFile>::Iterator Itr = fileList.Begin(); !Itr.End(); ++Itr)
	{
		if (!Itr.Get())
			break;

		Itr.Get()->DeleteInstance();
	}
	fileList.RemoveAll();
}

void TES::LoadCellIntoViewPort(Vector3* CameraCoordData, TESObjectREFR* Reference)
{
	cdeclCall<UInt32>(0x00430F40, CameraCoordData, Reference);
}

UInt8 FileFinder::FindFile(const char* Path, UInt32 Unk02, UInt32 Unk03, int Unk04)
{
	return thisVirtualCall<UInt8>(0x4, this, Path, Unk02, Unk03, Unk04);
}

void FileFinder::AddSearchPath(const char* Path)
{
	thisVirtualCall<UInt32>(0x8, this, Path);
}

NiBinaryStream* FileFinder::GetFileStream(const char* Path, bool WriteAccess, UInt32 BufferSize)
{
	return cdeclCall<NiBinaryStream*>(0x00468400, Path, WriteAccess, BufferSize);
}

BSRenderedTexture* BSTextureManager::CreateTexture( NiRenderer* Renderer, UInt32 Size, UInt32 Flags, UInt32 Unk04 /*= 0*/, UInt32 Unk05 /*= 0*/ )
{
	return thisCall<BSRenderedTexture*>(0x00773080, this, Renderer, Size, Flags, Unk04, Unk05);
}