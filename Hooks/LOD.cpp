#include "LOD.h"

namespace Hooks
{
	_DefineNopHdlr(LODLandTextureMipMapLevelA, 0x00411008, 2);
	_DefineHookHdlr(LODLandTextureMipMapLevelB, 0x005E0306);
	_DefineHookHdlr(LODLandTextureAllocation, 0x00410D08);
	_DefineHookHdlr(LODLandTextureDestruction, 0x00410A5A);
	_DefineHookHdlr(LODLandD3DTextureSelection, 0x00410A90);
	_DefineHookHdlr(LODLandBSTextureSelection, 0x00410AE0);
	_DefineHookHdlr(GenerateLODPartialTexture, 0x00412AB1);
	_DefineHookHdlr(GenerateLODFullTexture, 0x0041298B);
	_DefinePatchHdlr(GenerateLODFullTextureFileName, 0x00412241 + 1);

	#define LOD_DIFFUSE_MAP_RESOLUTION 256

	static const char*				s_LODFullTexturePath = ".\\Data\\Textures\\LandscapeLOD\\Generated\\%i.%02i.%02i.%i.dds";

	void PatchLODHooks(void)
	{
		_MemoryHandler(LODLandTextureMipMapLevelA).WriteNop();
		_MemoryHandler(LODLandTextureMipMapLevelB).WriteJump();
		_MemoryHandler(LODLandTextureAllocation).WriteJump();
		_MemoryHandler(LODLandTextureDestruction).WriteJump();
		_MemoryHandler(LODLandD3DTextureSelection).WriteJump();
		_MemoryHandler(LODLandBSTextureSelection).WriteJump();
		_MemoryHandler(GenerateLODPartialTexture).WriteJump();
		_MemoryHandler(GenerateLODFullTexture).WriteJump();
		_MemoryHandler(GenerateLODFullTextureFileName).WriteUInt32((UInt32)s_LODFullTexturePath);
	}

	#define _hhName		LODLandTextureMipMapLevelB
	_hhBegin()
	{
		_hhSetVar(Retn, 0x005E030F);
		__asm
		{
			push    2
			push    0x14
			push    0
			mov		eax, 8
			push	eax
			push    ebx

			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoLODLandTextureAllocationHook(void)
	{
		LPDIRECT3DDEVICE9 D3DDevice = (*g_CSRenderer)->device;

		D3DXCreateTexture(D3DDevice, 32, 32, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture32x);
		D3DXCreateTexture(D3DDevice, 128, 128, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture128x);
		D3DXCreateTexture(D3DDevice, 512, 512, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture512x);
		D3DXCreateTexture(D3DDevice, 2048, 2048, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture2048x);

		D3DXCreateTexture(D3DDevice, 64, 64, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture64x);
		D3DXCreateTexture(D3DDevice, 1024, 1024, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture1024x);

		*g_LODBSTexture32x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 32, 21, 0, 0);
		*g_LODBSTexture128x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 128, 21, 0, 0);
		*g_LODBSTexture512x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 512, 21, 0, 0);
		*g_LODBSTexture2048x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 2048, 21, 0, 0);

		*g_LODBSTexture64x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 64, 21, 0, 0);
		*g_LODBSTexture1024x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 1024, 21, 0, 0);

		D3DXCreateTexture(D3DDevice, 256, 256, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &g_LODD3DTexture256x);
		D3DXCreateTexture(D3DDevice, 4096, 4096, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &g_LODD3DTexture4096x);
		D3DXCreateTexture(D3DDevice, 8192, 8192, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &g_LODD3DTexture8192x);

		g_LODBSTexture256x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 256, 21, 0, 0);
		g_LODBSTexture4096x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 4096, 21, 0, 0);
		g_LODBSTexture8192x = (BSRenderedTexture*)thisCall(kBSTextureManager_CreateBSRenderedTexture, *g_TextureManager, *g_CSRenderer, 8192, 21, 0, 0);
	}

	#define _hhName		LODLandTextureAllocation
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00410EB4);
		__asm
		{
			call	DoLODLandTextureAllocationHook
			jmp		[_hhGetVar(Retn)]
		}
	}

	void __stdcall DoLODLandTextureDestructionHook(void)
	{
		*g_LODBSTexture128x = NULL;

		g_LODD3DTexture256x->Release();
		g_LODD3DTexture4096x->Release();
		g_LODD3DTexture8192x->Release();

		g_LODBSTexture256x = NULL;
		g_LODBSTexture4096x = NULL;
		g_LODBSTexture8192x = NULL;
	}

	#define _hhName		LODLandTextureDestruction
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00410A60);
		__asm
		{
			call	DoLODLandTextureDestructionHook
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		LODLandD3DTextureSelection
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00410A99);
		__asm
		{
			mov     eax, [esp + 0x4]
			cmp		eax, 256
			jz		FETCH256
			cmp		eax, 4096
			jz		FETCH4096
			cmp		eax, 8192
			jz		FETCH8192

			cmp     eax, 0x200
			jmp		[_hhGetVar(Retn)]
		FETCH256:
			mov		eax, g_LODD3DTexture256x
			retn
		FETCH4096:
			mov		eax, g_LODD3DTexture4096x
			retn
		FETCH8192:
			mov		eax, g_LODD3DTexture8192x
			retn
		}
	}

	#define _hhName		LODLandBSTextureSelection
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00410AE9);
		__asm
		{
			mov     eax, [esp + 0x4]
			cmp		eax, 256
			jz		FETCH256
			cmp		eax, 4096
			jz		FETCH4096
			cmp		eax, 8192
			jz		FETCH8192

			cmp     eax, 0x200
			jmp		[_hhGetVar(Retn)]
		FETCH256:
			mov		eax, g_LODBSTexture256x
			retn
		FETCH4096:
			mov		eax, g_LODBSTexture4096x
			retn
		FETCH8192:
			mov		eax, g_LODBSTexture8192x
			retn
		}
	}

	#define _hhName		GenerateLODPartialTexture
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00412ABC);
		_hhSetVar(Byte, 0x00A8E696);
		__asm
		{
			push	LOD_DIFFUSE_MAP_RESOLUTION		// new partial resolution
			push	ecx
			push	edx
			mov		eax, [_hhGetVar(Byte)]
			mov		[eax], 1
			jmp		[_hhGetVar(Retn)]
		}
	}

	#define _hhName		GenerateLODFullTexture
	_hhBegin()
	{
		_hhSetVar(Retn, 0x00412991);
		__asm
		{
			push    LOD_DIFFUSE_MAP_RESOLUTION
			push    0x20
			push    ecx
			push    eax
			jmp		[_hhGetVar(Retn)]
		}
	}
}
