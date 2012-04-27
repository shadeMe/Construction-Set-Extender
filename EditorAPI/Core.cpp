#include "Core.h"
#include "Hooks\Renderer.h"
#include "AuxiliaryViewport.h"

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
LPDIRECT3DTEXTURE9					g_LODD3DTexture384x = NULL;
BSRenderedTexture*					g_LODBSTexture384x = NULL;
LPDIRECT3DTEXTURE9					g_LODD3DTexture4096x = NULL;
BSRenderedTexture*					g_LODBSTexture4096x = NULL;
LPDIRECT3DTEXTURE9					g_LODD3DTexture6144x = NULL;
BSRenderedTexture*					g_LODBSTexture6144x = NULL;

Setting*							g_INILocalMasterPath = (Setting*)0x009ED710;



void* Oblivion_DynamicCast( void * SrcObj, UInt32 Arg1, const void * FromType, const void * ToType, UInt32 Arg4 )
{
	return cdeclCall<void*>(0x0088DC0C, SrcObj, Arg1, FromType, ToType, Arg4);
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

LPDIRECT3DTEXTURE9 BSRenderedTexture::ConvertToD3DTexture(UInt32 Width, UInt32 Height)
{
	LPDIRECT3DTEXTURE9 D3DTexture = NULL, Result = NULL;
	if (Width == 0)
		Width = this->renderedTexture->unk030->width;
	if (Height == 0)
		Height = this->renderedTexture->unk030->height;

	D3DXCreateTexture(_RENDERER->device, Width, Height, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &D3DTexture);
	Hooks::_MemHdlr(ConvertNiRenderedTexToD3DBaseTex).WriteJump();
	Result = cdeclCall<LPDIRECT3DTEXTURE9>(0x004113E0, this->renderedTexture, 0, 0, Width, D3DTexture, 0, 1, NULL);
	Hooks::_MemHdlr(ConvertNiRenderedTexToD3DBaseTex).WriteBuffer();
	D3DTexture->Release();
	D3DTexture = NULL;

	return Result;
}

void BSRenderedTexture::DeleteInstance( bool ReleaseMemory /*= 0*/ )
{
	thisVirtualCall<void>(0x0, this, ReleaseMemory);
}