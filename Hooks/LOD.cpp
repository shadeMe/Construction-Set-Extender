#include "LOD.h"
#include "..\AuxiliaryViewport.h"

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kLODINISettings[kLOD__MAX] =
		{
			{ "PartialTextureResolution",			"384",		"Resolution of the generated partial LOD diffuse maps. Size of the full map = 16 * Res. Must be a power of 2 and b'ween 32 and 384" },
			{ "DeletePartialsAfterGeneration",		"1",		"Delete the partial LOD textures after the full map is generated" }
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetLOD( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("LOD");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kLOD__MAX; i++)
					kFactory.Settings.push_back(&kLODINISettings[i]);
			}

			return &kFactory;
		}
	}

	namespace Hooks
	{
		#define SAFERELEASE_BSR(X)		if (X)	{ X->DeleteInstance(); X = NULL; }

		static const char*				kLODFullTexturePath = ".\\Data\\Textures\\LandscapeLOD\\Generated\\%i.%02i.%02i.%i.dds";
		static HWND						s_NotificationDialog = NULL;
		static UInt32					s_NotificationMapCounter = 0;
		static UInt32					s_LODDiffuseMapPartialResolution = 384;		// values higher than 384 will cause buffer overruns
		UInt8							g_LODDiffuseMapGeneratorState = kLODDiffuseMapGeneratorState_NotInUse;		// maintained by CSE

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
			_MemHdlr(GenerateLODFullTextureFileName).WriteUInt32((UInt32)kLODFullTexturePath);
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
			LPDIRECT3DDEVICE9 D3DDevice = (*g_CSRenderer)->device;

			D3DXCreateTexture(D3DDevice, 32, 32, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture32x);
			D3DXCreateTexture(D3DDevice, 128, 128, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture128x);
			D3DXCreateTexture(D3DDevice, 512, 512, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture512x);
			D3DXCreateTexture(D3DDevice, 2048, 2048, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture2048x);

			D3DXCreateTexture(D3DDevice, 64, 64, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture64x);
			D3DXCreateTexture(D3DDevice, 1024, 1024, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, g_LODD3DTexture1024x);

			*g_LODBSTexture32x = _TEXMGR->CreateTexture(*g_CSRenderer, 32, 21, 0, 0);
			*g_LODBSTexture128x = _TEXMGR->CreateTexture(*g_CSRenderer, 128, 21, 0, 0);
			*g_LODBSTexture512x = _TEXMGR->CreateTexture(*g_CSRenderer, 512, 21, 0, 0);
			*g_LODBSTexture2048x = _TEXMGR->CreateTexture(*g_CSRenderer, 2048, 21, 0, 0);

			*g_LODBSTexture64x = _TEXMGR->CreateTexture(*g_CSRenderer, 64, 21, 0, 0);
			*g_LODBSTexture1024x = _TEXMGR->CreateTexture(*g_CSRenderer, 1024, 21, 0, 0);

			D3DXCreateTexture(D3DDevice, 256, 256, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &g_LODD3DTexture256x);
			D3DXCreateTexture(D3DDevice, 384, 384, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &g_LODD3DTexture384x);
			D3DXCreateTexture(D3DDevice, 4096, 4096, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &g_LODD3DTexture4096x);
			D3DXCreateTexture(D3DDevice, 6144, 6144, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &g_LODD3DTexture6144x);

			g_LODBSTexture256x = _TEXMGR->CreateTexture(*g_CSRenderer, 256, 21, 0, 0);
			g_LODBSTexture384x = _TEXMGR->CreateTexture(*g_CSRenderer, 384, 21, 0, 0);
			g_LODBSTexture4096x = _TEXMGR->CreateTexture(*g_CSRenderer, 4096, 21, 0, 0);
			g_LODBSTexture6144x = _TEXMGR->CreateTexture(*g_CSRenderer, 6144, 21, 0, 0);
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
			SAFERELEASE_D3D(g_LODD3DTexture256x)
			SAFERELEASE_D3D(g_LODD3DTexture384x)
			SAFERELEASE_D3D(g_LODD3DTexture4096x)
			SAFERELEASE_D3D(g_LODD3DTexture6144x)

			SAFERELEASE_BSR((*g_LODBSTexture32x))
			SAFERELEASE_BSR((*g_LODBSTexture128x))
			SAFERELEASE_BSR((*g_LODBSTexture512x))
			SAFERELEASE_BSR((*g_LODBSTexture2048x))
			SAFERELEASE_BSR((*g_LODBSTexture64x))
			SAFERELEASE_BSR((*g_LODBSTexture1024x))

			SAFERELEASE_BSR(g_LODBSTexture256x)
			SAFERELEASE_BSR(g_LODBSTexture384x)
			SAFERELEASE_BSR(g_LODBSTexture4096x)
			SAFERELEASE_BSR(g_LODBSTexture6144x)
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
				mov		eax, g_LODD3DTexture256x
				retn
			FETCH384:
				mov		eax, g_LODD3DTexture384x
				retn
			FETCH4096:
				mov		eax, g_LODD3DTexture4096x
				retn
			FETCH6144:
				mov		eax, g_LODD3DTexture6144x
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
				mov		eax, g_LODBSTexture256x
				retn
			FETCH384:
				mov		eax, g_LODBSTexture384x
				retn
			FETCH4096:
				mov		eax, g_LODBSTexture4096x
				retn
			FETCH6144:
				mov		eax, g_LODBSTexture6144x
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
				sprintf_s(Buffer, sizeof(Buffer), "Please Wait\nAssembling Partials");
				Static_SetText(GetDlgItem(s_NotificationDialog, -1), Buffer);
			}

			g_LODDiffuseMapGeneratorState = kLODDiffuseMapGeneratorState_FullMap;
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
			if (g_LODDiffuseMapGeneratorState == kLODDiffuseMapGeneratorState_NotInUse ||
				*((TESWorldSpace**)0x00A0AB14) == NULL ||		// current worldspace
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
			return g_LODDiffuseMapGeneratorState == kLODDiffuseMapGeneratorState_FullMap;
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
			if (State && g_LODDiffuseMapGeneratorState == kLODDiffuseMapGeneratorState_NotInUse)
			{
				s_NotificationDialog = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), BGSEEUI->GetMainWindow(), NULL, NULL);
				Static_SetText(GetDlgItem(s_NotificationDialog, -1), "Please Wait\nDiffuse Map 0/256");
				g_LODDiffuseMapGeneratorState = kLODDiffuseMapGeneratorState_Partials;

				// reduce time spent updating the main windows during cell switch
				UIManager::CSEWindowInvalidationManager::Instance.Push(*g_HWND_CellView);
				UIManager::CSEWindowInvalidationManager::Instance.Push(*g_HWND_CSParent);
				UIManager::CSEWindowInvalidationManager::Instance.Push(*g_HWND_RenderWindow);

				s_LODDiffuseMapPartialResolution = atoi(INISettings::GetLOD()->Get(INISettings::kLOD_PartialTextureResolution, BGSEEMAIN->INIGetter()));

				if (s_LODDiffuseMapPartialResolution % 2 || s_LODDiffuseMapPartialResolution < 32)
					s_LODDiffuseMapPartialResolution = 32;
				else if (s_LODDiffuseMapPartialResolution > 384)
					s_LODDiffuseMapPartialResolution = 384;

				Setting* Current = NULL;

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

				*g_TESCSAllowAutoSaveFlag = 0;
			}
			else if (g_LODDiffuseMapGeneratorState == kLODDiffuseMapGeneratorState_FullMap)
			{
				BGSEECONSOLE_MESSAGE("Generated %d partial diffuse map(s) in total", s_NotificationMapCounter);
				DestroyWindow(s_NotificationDialog);

				s_NotificationDialog = NULL;
				s_NotificationMapCounter = 0;
				g_LODDiffuseMapGeneratorState = kLODDiffuseMapGeneratorState_NotInUse;

				UIManager::CSEWindowInvalidationManager::Instance.Pop(*g_HWND_CellView);
				UIManager::CSEWindowInvalidationManager::Instance.Pop(*g_HWND_CSParent);
				UIManager::CSEWindowInvalidationManager::Instance.Pop(*g_HWND_RenderWindow);

				if (atoi(INISettings::GetLOD()->Get(INISettings::kLOD_DeletePartialsAfterGeneration, BGSEEMAIN->INIGetter())))
				{
					char Buffer[MAX_PATH + 1] = {0};
					SHFILEOPSTRUCT DeleteFolderData = {0};

					sprintf_s(Buffer, sizeof(Buffer), "%s\\Data\\Textures\\LandscapeLOD\\Generated\\Partial\\*.dds\0", BGSEEMAIN->GetAPPPath());
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
				*g_TESCSAllowAutoSaveFlag = 1;
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
				jnz		GENFINISHED										// already generating the full map, so piss off
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
				sprintf_s(Buffer, sizeof(Buffer), "Please Wait\nDiffuse Map %d/256", ++s_NotificationMapCounter);
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
				thisCall<void>(0x006F25E0, Camera, 0.0, 1);			// NiNode::Update
			}

			// render the map's node as it gets updated with each partial's trishape
			if (AUXVIEWPORT->GetVisible() && AUXVIEWPORT->GetFrozen() == false)
			{
				thisCall<void>(0x006F28A0, RenderNode);
				thisCall<void>(0x006F2C10, RenderNode);
				thisCall<void>(0x006F25E0, RenderNode, 0.0, 1);
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
			HACK("Doesn't work")

			Camera->m_kViewFrustum.n = -10.0f;
			Camera->m_kViewFrustum.f = 10000000.0f;
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
	}
}

#pragma warning(pop)
#pragma optimize("", on)