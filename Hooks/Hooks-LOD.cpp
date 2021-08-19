#include "Hooks-LOD.h"
#include "Render Window\AuxiliaryViewport.h"
#include "WorkspaceManager.h"

#pragma warning(push)
#pragma warning(disable: 4005 4748)

namespace cse
{
	namespace hooks
	{
		#define SAFERELEASE_BSR(X)		if (X)	{ X->DeleteInstance(); X = nullptr; }

		static HWND						s_NotificationDialog = nullptr;
		static UInt32					s_NotificationMapCounter = 0;
		static UInt32					s_LODDiffuseMapPartialResolution = 384;		// values higher than 384 will cause buffer overruns

		static int						s_iFadeNodeMinNearDistance = 0;
		static float					s_fLODFadeOutPercent = 0.0f;
		static int						s_iPostProcessMillisecondsEditor = 0;
		static float					s_fFadeDistance = 0.0f;

		static const int				kiFadeNodeMinNearDistance_LOD = 999999999;
		static const float				kfLODFadeOutPercent_LOD = 0.0f;
		static const int				kiPostProcessMillisecondsEditor_BackgroundLoad = 6000;
		static const float				kfFadeDistance_DistantLOD = 999999990.0f;

		_DefineNopHdlr(LODLandTextureMipMapLevelA, 0x00411008, 2);
		_DefineHookHdlr(LODLandTextureMipMapLevelB, 0x005E0306);
		_DefineHookHdlr(LODLandTextureAllocation, 0x00410D08);
		_DefineHookHdlr(LODLandTextureDestruction, 0x00410A5A);
		_DefineHookHdlr(LODLandD3DTextureSelection, 0x00410A90);
		_DefineHookHdlr(LODLandBSTextureSelection, 0x00410AE0);
		_DefineHookHdlr(GenerateLODPartialTexture, 0x00412AB1);
		_DefineHookHdlr(GenerateLODFullTexture, 0x0041298B);
		_DefinePatchHdlr(GenerateLODFullTextureFileName, 0x00412241 + 1);
		_DefineHookHdlr(GenerateLODDiffuseMapsReentryGuardA, 0x00412BB3);
		_DefineHookHdlr(GenerateLODDiffuseMapsReentryGuardB, 0x004129FF);
		_DefineHookHdlr(GenerateLODDiffuseMapsReentryGuardC, 0x00412BA6);
		_DefineHookHdlr(GenerateLODDiffuseMapsReentryGuardD, 0x00412AD2);
		_DefineHookHdlr(GenerateLODDiffuseMapsReentryGuardE, 0x00412CE3);
		_DefineHookHdlr(LODTextureGenNotificationPrologA, 0x00429E3D);
		_DefineHookHdlr(LODTextureGenNotificationPrologB, 0x00429D97);
		_DefineHookHdlr(LODTextureGenNotificationEpilog, 0x00412D4A);
		_DefineHookHdlr(LODTextureGenNotificationUpdate, 0x00412ABC);
		_DefineHookHdlr(LODTextureGenBlackTextureFix, 0x00412115);
		_DefineHookHdlr(LODTextureGenBlackTexturePartialFix, 0x00412789);
		_DefineHookHdlr(NiRenderedTextureCreateSourceTextureA, 0x004AD4B7);
		_DefineHookHdlr(NiRenderedTextureCreateSourceTextureB, 0x004AD515);
		_DefineHookHdlr(LODTextureGenSavePartialToDisk, 0x0041143E);
		_DefineHookHdlrWithBuffer(GeneratePartialLODFilePath, 0x004128B3, 6, 0x6A, 0x0, 0x6A, 0x0, 0x6A, 0x0);

		void PatchLODHooks(void)
		{
			_MemHdlr(LODLandTextureMipMapLevelA).WriteNop();
			_MemHdlr(LODLandTextureMipMapLevelB).WriteJump();
			_MemHdlr(LODLandTextureAllocation).WriteJump();
			_MemHdlr(LODLandTextureDestruction).WriteJump();
			_MemHdlr(LODLandD3DTextureSelection).WriteJump();
			_MemHdlr(LODLandBSTextureSelection).WriteJump();
			_MemHdlr(GenerateLODPartialTexture).WriteJump();
			_MemHdlr(GenerateLODFullTexture).WriteJump();
			_MemHdlr(GenerateLODFullTextureFileName).WriteUInt32((UInt32)TESLODTextureGenerator::LODFullTexturePath);
			_MemHdlr(GenerateLODDiffuseMapsReentryGuardA).WriteJump();
			_MemHdlr(GenerateLODDiffuseMapsReentryGuardB).WriteJump();
			_MemHdlr(GenerateLODDiffuseMapsReentryGuardC).WriteJump();
			_MemHdlr(GenerateLODDiffuseMapsReentryGuardD).WriteJump();
			_MemHdlr(GenerateLODDiffuseMapsReentryGuardE).WriteJump();
			_MemHdlr(LODTextureGenNotificationPrologA).WriteJump();
			_MemHdlr(LODTextureGenNotificationPrologB).WriteJump();
			_MemHdlr(LODTextureGenNotificationEpilog).WriteJump();
			_MemHdlr(LODTextureGenNotificationUpdate).WriteJump();
			_MemHdlr(LODTextureGenBlackTextureFix).WriteJump();
			_MemHdlr(LODTextureGenBlackTexturePartialFix).WriteJump();
			_MemHdlr(NiRenderedTextureCreateSourceTextureA).WriteJump();
			_MemHdlr(NiRenderedTextureCreateSourceTextureB).WriteJump();
			_MemHdlr(LODTextureGenSavePartialToDisk).WriteJump();
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

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoLODLandTextureAllocationHook(void)
		{
			LPDIRECT3DDEVICE9 D3DDevice = _NIRENDERER->device;

			D3DXCreateTexture(D3DDevice, 32, 32, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, TESLODTextureGenerator::D3DTexture32x);
			D3DXCreateTexture(D3DDevice, 128, 128, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, TESLODTextureGenerator::D3DTexture128x);
			D3DXCreateTexture(D3DDevice, 512, 512, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, TESLODTextureGenerator::D3DTexture512x);
			D3DXCreateTexture(D3DDevice, 2048, 2048, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, TESLODTextureGenerator::D3DTexture2048x);

			D3DXCreateTexture(D3DDevice, 64, 64, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, TESLODTextureGenerator::D3DTexture64x);
			D3DXCreateTexture(D3DDevice, 1024, 1024, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, TESLODTextureGenerator::D3DTexture1024x);

			*TESLODTextureGenerator::BSTexture32x = _TEXMGR->CreateTexture(_NIRENDERER, 32, 21, 0, 0);
			*TESLODTextureGenerator::BSTexture128x = _TEXMGR->CreateTexture(_NIRENDERER, 128, 21, 0, 0);
			*TESLODTextureGenerator::BSTexture512x = _TEXMGR->CreateTexture(_NIRENDERER, 512, 21, 0, 0);
			*TESLODTextureGenerator::BSTexture2048x = _TEXMGR->CreateTexture(_NIRENDERER, 2048, 21, 0, 0);

			*TESLODTextureGenerator::BSTexture64x = _TEXMGR->CreateTexture(_NIRENDERER, 64, 21, 0, 0);
			*TESLODTextureGenerator::BSTexture1024x = _TEXMGR->CreateTexture(_NIRENDERER, 1024, 21, 0, 0);

			D3DXCreateTexture(D3DDevice, 256, 256, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &TESLODTextureGenerator::D3DTexture256x);
			D3DXCreateTexture(D3DDevice, 384, 384, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &TESLODTextureGenerator::D3DTexture384x);
			D3DXCreateTexture(D3DDevice, 4096, 4096, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &TESLODTextureGenerator::D3DTexture4096x);
			D3DXCreateTexture(D3DDevice, 6144, 6144, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &TESLODTextureGenerator::D3DTexture6144x);

			TESLODTextureGenerator::BSTexture256x = _TEXMGR->CreateTexture(_NIRENDERER, 256, 21, 0, 0);
			TESLODTextureGenerator::BSTexture384x = _TEXMGR->CreateTexture(_NIRENDERER, 384, 21, 0, 0);
			TESLODTextureGenerator::BSTexture4096x = _TEXMGR->CreateTexture(_NIRENDERER, 4096, 21, 0, 0);
			TESLODTextureGenerator::BSTexture6144x = _TEXMGR->CreateTexture(_NIRENDERER, 6144, 21, 0, 0);
		}

		#define _hhName		LODLandTextureAllocation
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00410EB4);
			__asm
			{
				call	DoLODLandTextureAllocationHook
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoLODLandTextureDestructionHook(void)
		{
			SAFERELEASE_D3D(TESLODTextureGenerator::D3DTexture256x)
			SAFERELEASE_D3D(TESLODTextureGenerator::D3DTexture384x)
			SAFERELEASE_D3D(TESLODTextureGenerator::D3DTexture4096x)
			SAFERELEASE_D3D(TESLODTextureGenerator::D3DTexture6144x)

			SAFERELEASE_BSR((*TESLODTextureGenerator::BSTexture32x))
			SAFERELEASE_BSR((*TESLODTextureGenerator::BSTexture128x))
			SAFERELEASE_BSR((*TESLODTextureGenerator::BSTexture512x))
			SAFERELEASE_BSR((*TESLODTextureGenerator::BSTexture2048x))
			SAFERELEASE_BSR((*TESLODTextureGenerator::BSTexture64x))
			SAFERELEASE_BSR((*TESLODTextureGenerator::BSTexture1024x))

			SAFERELEASE_BSR(TESLODTextureGenerator::BSTexture256x)
			SAFERELEASE_BSR(TESLODTextureGenerator::BSTexture384x)
			SAFERELEASE_BSR(TESLODTextureGenerator::BSTexture4096x)
			SAFERELEASE_BSR(TESLODTextureGenerator::BSTexture6144x)
		}

		#define _hhName		LODLandTextureDestruction
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00410A7E);
			__asm
			{
				call	DoLODLandTextureDestructionHook
				xor		bl, bl
				jmp		_hhGetVar(Retn)
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
				cmp		eax, 384
				jz		FETCH384
				cmp		eax, 4096
				jz		FETCH4096
				cmp		eax, 6144
				jz		FETCH6144

				cmp     eax, 0x200
				jmp		_hhGetVar(Retn)
			FETCH256:
				mov		eax, TESLODTextureGenerator::D3DTexture256x
				retn
			FETCH384:
				mov		eax, TESLODTextureGenerator::D3DTexture384x
				retn
			FETCH4096:
				mov		eax, TESLODTextureGenerator::D3DTexture4096x
				retn
			FETCH6144:
				mov		eax, TESLODTextureGenerator::D3DTexture6144x
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
				cmp		eax, 384
				jz		FETCH384
				cmp		eax, 4096
				jz		FETCH4096
				cmp		eax, 6144
				jz		FETCH6144

				cmp     eax, 0x200
				jmp		_hhGetVar(Retn)
			FETCH256:
				mov		eax, TESLODTextureGenerator::BSTexture256x
				retn
			FETCH384:
				mov		eax, TESLODTextureGenerator::BSTexture384x
				retn
			FETCH4096:
				mov		eax, TESLODTextureGenerator::BSTexture4096x
				retn
			FETCH6144:
				mov		eax, TESLODTextureGenerator::BSTexture6144x
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
				mov		eax, s_LODDiffuseMapPartialResolution
				push	eax
				push	ecx
				push	edx
				mov		eax, _hhGetVar(Byte)
				mov		[eax], 1
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoGenerateLODFullTextureHook(void)
		{
			if (s_NotificationDialog)
			{
				char Buffer[0x200] = {0};
				_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "Please Wait\nAssembling Partials");
				Static_SetText(GetDlgItem(s_NotificationDialog, -1), Buffer);
			}

			TESLODTextureGenerator::GeneratorState = TESLODTextureGenerator::kState_FullMap;
		}
		#define _hhName		GenerateLODFullTexture
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412991);
			__asm
			{
				pushad
				call	DoGenerateLODFullTextureHook
				popad

				push    s_LODDiffuseMapPartialResolution
				push    0x20
				push    ecx
				push    eax
				jmp		_hhGetVar(Retn)
			}
		}

		bool __stdcall GetIsLODDiffuseMapGeneratorInUse(void)
		{
			if (TESLODTextureGenerator::GeneratorState == TESLODTextureGenerator::kState_NotInUse ||
				*((TESWorldSpace**)0x00A0AB14) == nullptr ||		// current worldspace
				*((UInt32*)0x00A0AAF0) == 0 ||					// LOD gen state
				*((UInt8*)0x00A0AB13) == 0)						// LOD texture state
			{
				return false;
			}
			else
				return true;
		}

		bool __stdcall GetIsLODDiffuseMapGeneratorCreatingFullMap(void)
		{
			return TESLODTextureGenerator::GeneratorState == TESLODTextureGenerator::kState_FullMap;
		}

		#define _hhName		GenerateLODDiffuseMapsReentryGuardA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412BB9);
			_hhSetVar(Jump, 0x00412B48);
			__asm
			{
				mov		edx, 0x00A0AB14
				mov		edx, [edx]

				pushad
				call	GetIsLODDiffuseMapGeneratorInUse
				test	al, al
				jz		GENFINISHED
				popad
				jmp		_hhGetVar(Retn)
			GENFINISHED:
				popad
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		GenerateLODDiffuseMapsReentryGuardB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412A06);
			_hhSetVar(Jump, 0x00412DA7);
			__asm
			{
				mov		eax, 0x00A0AAFC
				mov		eax, [eax]
				mov		ecx, eax

				pushad
				call	GetIsLODDiffuseMapGeneratorInUse
				test	al, al
				jz		GENFINISHED
				popad
				jmp		_hhGetVar(Retn)
			GENFINISHED:
				popad
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		GenerateLODDiffuseMapsReentryGuardC
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412BAB);
			_hhSetVar(Jump, 0x00412B48);
			_hhSetVar(Call, 0x00430F40);
			__asm
			{
				pushad
				call	GetIsLODDiffuseMapGeneratorInUse		// check before and after function entry, just to be paranoid
				test	al, al
				jz		GENFINISHED
				popad

				call	_hhGetVar(Call)

				pushad
				call	GetIsLODDiffuseMapGeneratorInUse
				test	al, al
				jz		GENFINISHED
				popad

				jmp		_hhGetVar(Retn)
			GENFINISHED:
				popad

				add		esp, 0x8
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		GenerateLODDiffuseMapsReentryGuardD
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412AD7);
			_hhSetVar(Jump, 0x00412B48);
			__asm
			{
				mov		eax, 0x00A0AAFC
				mov		eax, [eax]

				pushad
				call	GetIsLODDiffuseMapGeneratorInUse
				test	al, al
				jz		GENFINISHED
				popad
				jmp		_hhGetVar(Retn)
			GENFINISHED:
				popad
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		GenerateLODDiffuseMapsReentryGuardE
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412CE8);
			_hhSetVar(Jump, 0x00412D21);
			_hhSetVar(Call, 0x00430F40);
			__asm
			{
				pushad
				call	GetIsLODDiffuseMapGeneratorInUse		// check before and after function entry, just to be paranoid
				test	al, al
				jz		GENFINISHED
				popad

				call	_hhGetVar(Call)

				pushad
				call	GetIsLODDiffuseMapGeneratorInUse
				test	al, al
				jz		GENFINISHED
				popad

				jmp		_hhGetVar(Retn)
			GENFINISHED:
				popad

				jmp		_hhGetVar(Jump)
			}
		}

		void __stdcall ShowLODTextureGenNotification(bool State)
		{
			if (State && TESLODTextureGenerator::GeneratorState == TESLODTextureGenerator::kState_NotInUse)
			{
				s_NotificationDialog = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), BGSEEUI->GetMainWindow(), nullptr, NULL);
				Static_SetText(GetDlgItem(s_NotificationDialog, -1), "Please Wait\nDiffuse Map 0/256");
				TESLODTextureGenerator::GeneratorState = TESLODTextureGenerator::kState_Partials;

				// reduce time spent updating the main windows during cell switch
				BGSEEUI->GetInvalidationManager()->Push(*TESCellViewWindow::WindowHandle);
				BGSEEUI->GetInvalidationManager()->Push(*TESCSMain::WindowHandle);
				BGSEEUI->GetInvalidationManager()->Push(*TESRenderWindow::WindowHandle);

				s_LODDiffuseMapPartialResolution = settings::lod::kPartialTextureResolution.GetData().i;

				if (s_LODDiffuseMapPartialResolution % 2 || s_LODDiffuseMapPartialResolution < 32)
					s_LODDiffuseMapPartialResolution = 32;
				else if (s_LODDiffuseMapPartialResolution > 384)
					s_LODDiffuseMapPartialResolution = 384;

				Setting* Current = nullptr;

				Current = INISettingCollection::Instance->LookupByName("iFadeNodeMinNearDistance:LOD");
				SME_ASSERT(Current);
				s_iFadeNodeMinNearDistance = Current->value.i;
				Current->value.i = kiFadeNodeMinNearDistance_LOD;

				Current = INISettingCollection::Instance->LookupByName("fLODFadeOutPercent:LOD");
				SME_ASSERT(Current);
				s_fLODFadeOutPercent = Current->value.f;
				Current->value.f = kfLODFadeOutPercent_LOD;

				Current = INISettingCollection::Instance->LookupByName("iPostProcessMillisecondsEditor:BackgroundLoad");
				SME_ASSERT(Current);
				s_iPostProcessMillisecondsEditor = Current->value.i;
				Current->value.i = kiPostProcessMillisecondsEditor_BackgroundLoad;

				Current = INISettingCollection::Instance->LookupByName("fFadeDistance:DistantLOD");
				SME_ASSERT(Current);
				s_fFadeDistance = Current->value.f;
				Current->value.f = kfFadeDistance_DistantLOD;

				*TESCSMain::AllowAutoSaveFlag = 0;
			}
			else if (TESLODTextureGenerator::GeneratorState == TESLODTextureGenerator::kState_FullMap)
			{
				BGSEECONSOLE_MESSAGE("Generated %d partial diffuse map(s) in total", s_NotificationMapCounter);
				DestroyWindow(s_NotificationDialog);

				s_NotificationDialog = nullptr;
				s_NotificationMapCounter = 0;
				TESLODTextureGenerator::GeneratorState = TESLODTextureGenerator::kState_NotInUse;

				BGSEEUI->GetInvalidationManager()->Pop(*TESCellViewWindow::WindowHandle);
				BGSEEUI->GetInvalidationManager()->Pop(*TESCSMain::WindowHandle);
				BGSEEUI->GetInvalidationManager()->Pop(*TESRenderWindow::WindowHandle);

				if (settings::lod::kDeletePartialsAfterGeneration.GetData().i)
				{
					char Buffer[MAX_PATH + 1] = {0};
					SHFILEOPSTRUCT DeleteFolderData = {0};

					_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "%s\\Data\\Textures\\LandscapeLOD\\Generated\\Partial\\*.dds\0", BGSEEWORKSPACE->GetCurrentWorkspace());
					Buffer[strlen(Buffer) + 1] = 0;
					DeleteFolderData.wFunc = FO_DELETE;
					DeleteFolderData.pFrom = Buffer;
					DeleteFolderData.fFlags = FOF_NOCONFIRMATION|FOF_SILENT|FOF_NOERRORUI;

					int Result = SHFileOperation(&DeleteFolderData);
					if (Result)
						BGSEECONSOLE_MESSAGE("Couldn't delete partial LOD textures! Error: %d", Result);
				}

				INISettingCollection::Instance->LookupByName("iFadeNodeMinNearDistance:LOD")->value.i = s_iFadeNodeMinNearDistance;
				INISettingCollection::Instance->LookupByName("fLODFadeOutPercent:LOD")->value.f = s_fLODFadeOutPercent;
				INISettingCollection::Instance->LookupByName("iPostProcessMillisecondsEditor:BackgroundLoad")->value.i = s_iPostProcessMillisecondsEditor;
				INISettingCollection::Instance->LookupByName("fFadeDistance:DistantLOD")->value.f = s_fFadeDistance;

				SetThreadExecutionState(ES_CONTINUOUS);
				*TESCSMain::AllowAutoSaveFlag = 1;
			}
		}

		#define _hhName		LODTextureGenNotificationPrologA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00429E42);
			_hhSetVar(Call, 0x00410EC0);
			__asm
			{
				pushad
				push	1
				call	ShowLODTextureGenNotification
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		LODTextureGenNotificationPrologB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00429D9C);
			_hhSetVar(Call, 0x00410EC0);
			__asm
			{
				pushad
				push	1
				call	ShowLODTextureGenNotification
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		LODTextureGenNotificationEpilog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412D4F);
			_hhSetVar(Jump, 0x00412E25);
			_hhSetVar(Call, 0x00412980);
			__asm
			{
				pushad
				call	GetIsLODDiffuseMapGeneratorInUse				// more paranoia
				test	al, al
				jz		GENFINISHED
				call	GetIsLODDiffuseMapGeneratorCreatingFullMap		// this is necessary though
				test	al, al
				jnz		GENFINISHED										// already generating the full map, so sod off
				popad

				jmp		GENINPROGRESS
			GENFINISHED:
				popad
				jmp		_hhGetVar(Jump)
			GENINPROGRESS:
				call	_hhGetVar(Call)

				pushad
				push	0
				call	ShowLODTextureGenNotification
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoLODTextureGenNotificationUpdateHook(void)
		{
			if (s_NotificationDialog)
			{
				char Buffer[0x200] = {0};
				_snprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, "Please Wait\nDiffuse Map %d/256", ++s_NotificationMapCounter);
				Static_SetText(GetDlgItem(s_NotificationDialog, -1), Buffer);
			}
		}

		#define _hhName		LODTextureGenNotificationUpdate
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412AC1);
			_hhSetVar(Call, 0x00412480);
			__asm
			{
				call	_hhGetVar(Call)
				test	al, al
				jz		SKIP

				pushad
				call	DoLODTextureGenNotificationUpdateHook
				popad
			SKIP:
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoLODTextureGenBlackTextureFixHook(NiNode* RenderNode, NiCamera* Camera)
		{
			// fix for the bug - prevent the full map's node from being clipped by the camera's near plane
			if (Camera->m_kViewFrustum.n > 0.05f)
			{
				Camera->m_kViewFrustum.n = 0.05f;
				TESRender::UpdateAVObject(Camera);
			}

			// render the map's node as it gets updated with each partial's trishape
			if (AUXVIEWPORT->IsVisible())
			{
				thisCall<void>(0x006F28A0, RenderNode);
				thisCall<void>(0x006F2C10, RenderNode);
				TESRender::UpdateAVObject(RenderNode);
				AUXVIEWPORT->Draw(RenderNode, Camera);			// output will be inverted
			}
		}

		#define _hhName		LODTextureGenBlackTextureFix
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00412120);
			__asm
			{
				push    0
				push    eax
				mov     eax, [edx + 0x84]
				call    eax

				mov		eax, [esp + 0x38]
				mov		edx, [esp + 0x2C - 0x8]
				pushad
				push	edx
				push	eax
				call	DoLODTextureGenBlackTextureFixHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoLODTextureGenBlackTexturePartialFixHook(NiCamera* Camera, NiFrustum* CameraFrustum, NiNode* Container)
		{
			// does this actually work?
			Camera->m_kViewFrustum.n = -10.0f;
			Camera->m_kViewFrustum.f = 10000000.0f;

			if (TESLODTextureGenerator::ExteriorSnapshotSource)
			{
				// by default, the LOD partial generator camera is setup to render a 2x2 cell grid at a time
				// the starting cell is placed at the top-right corner
				// we relocate the camera and change its frustum to just render the starting cell when generating custom snapshots
				SInt32 XCoord = TESLODTextureGenerator::ExteriorSnapshotSource->cellData.coords->x;
				SInt32 YCoord = TESLODTextureGenerator::ExteriorSnapshotSource->cellData.coords->y;

				float CameraPosX = (XCoord << 12) + 2048.0f;
				float CameraPosY = (YCoord << 12) + 2048.0f;

				Container->m_localTranslate.x = CameraPosX;
				Container->m_localTranslate.y = CameraPosY;

				SME_ASSERT(TESLODTextureGenerator::ExteriorSnapshotSource->land);
				TESObjectLAND::LandHeightLimit Limits = {0};
				TESLODTextureGenerator::ExteriorSnapshotSource->land->GetHeightLimits(&Limits);

				Container->m_localTranslate.z = Limits.minHeight + 20000.0f;

				Camera->m_kViewFrustum.l = -2048.0f;
				Camera->m_kViewFrustum.r = 2048.0f;
				Camera->m_kViewFrustum.t = 2048.0f;
				Camera->m_kViewFrustum.b = -2048.0f;
			}
		}

		#define _hhName		LODTextureGenBlackTexturePartialFix
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041278E);
			_hhSetVar(Call, 0x006FE030);
			__asm
			{
				mov		eax, [esp]
				push	eax
				call	_hhGetVar(Call)
				pop		eax

				pushad
				push	ebp
				push	eax
				push	ecx
				call	DoLODTextureGenBlackTexturePartialFixHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall ReportLODD3DError(int ID)
		{
			switch (ID)
			{
			case 1:
				BGSEECONSOLE_MESSAGE("Bad NiTexture::RendererData for partial %d!", s_NotificationMapCounter);
				break;
			case 2:
				BGSEECONSOLE_MESSAGE("CreateOffscreenPlainSurface/GetRenderTargetData failed for partial %d!", s_NotificationMapCounter);
				break;
			case 3:
				BGSEECONSOLE_MESSAGE("Bad NiSourceTexture for partial %d!", s_NotificationMapCounter);
				break;
			default:
				break;
			}
		}

		#define _hhName		NiRenderedTextureCreateSourceTextureA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004AD4C9);
			_hhSetVar(Skip, 0x004AD642);
			__asm
			{
				mov     eax, [esi]
				mov     edx, [eax + 0x18]
				mov     ecx, esi
				call    edx
				test	eax, eax
				jz		SODOFF

				mov     edx, [eax]
				mov     ecx, eax
				mov     eax, [edx + 0x14]
				call    eax
				test	eax, eax
				jz		SODOFF

				jmp		_hhGetVar(Retn)
			SODOFF:
				pushad
				push	1
				call	ReportLODD3DError
				popad
				jmp		_hhGetVar(Skip)
			}
		}

		#define _hhName		NiRenderedTextureCreateSourceTextureB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004AD51E);
			_hhSetVar(Skip, 0x004AD642);
			__asm
			{
				test	eax, eax					// test destination surface
				jz		SODOFF

				test	ecx, ecx					// test render target surface
				jz		SODOFF

				push	eax
				push	ecx
				push	edi
				call	edx

				mov		eax, [esp + 0x14]			// retest destination surface for a successful copy
				test	eax, eax
				jz		SODOFF

				jmp		_hhGetVar(Retn)
			SODOFF:
				pushad
				push	2
				call	ReportLODD3DError
				popad
				jmp		_hhGetVar(Skip)
			}
		}

		#define _hhName		LODTextureGenSavePartialToDisk
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00411448);
			_hhSetVar(Call, 0x004AD410);
			_hhSetVar(Skip, 0x004116A3);
			__asm
			{
				call	_hhGetVar(Call)
				mov		esi, eax
				add		esp, 0xC
				test	eax, eax					// check if we've got a valid source texture
				jz		SODOFF

				jmp		_hhGetVar(Retn)
			SODOFF:
				pushad
				push	3
				call	ReportLODD3DError
				popad
				jmp		_hhGetVar(Skip)
			}
		}

		#define _hhName		GeneratePartialLODFilePath
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004128B9);
			__asm
			{
				push	TESLODTextureGenerator::ExteriorSnapshotPathBuffer
				push	0
				push	1							// also generate mipmaps
				jmp		_hhGetVar(Retn)
			}
		}
	}
}

#pragma warning(pop)