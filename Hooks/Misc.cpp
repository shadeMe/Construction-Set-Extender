#include "Misc.h"
#include "Dialog.h"
#include "TESFile.h"
#include "Renderer.h"
#include "..\RenderSelectionGroupManager.h"
#include "..\AuxiliaryViewport.h"
#include "..\ChangeLogManager.h"
#include "..\CSEAchievements.h"
#include "..\CSEHallOfFame.h"
#include "..\CSEWorkspaceManager.h"

#include <BGSEEToolBox.h>
#include <BGSEEToolBox.h>

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kGeneralINISettings[kGeneral__MAX] =
		{
			{ "ShowNumericEditorIDWarning",			"1",		"Display a warning when editorIDs start with an integer" }
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetGeneral( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("General");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kGeneral__MAX; i++)
					kFactory.Settings.push_back(&kGeneralINISettings[i]);
			}

			return &kFactory;
		}
	}

	namespace Hooks
	{
		_DefineHookHdlrWithBuffer(CSInit, 0x00419260, 5, 0xE8, 0xEB, 0xC5, 0x2C, 0x0);
		_DefineHookHdlr(CSExit, 0x0041936E);
		_DefineNopHdlr(MissingTextureWarning, 0x0044F3AF, 14);
		_DefineHookHdlr(AssertOverride, 0x004B5670);
		_DefinePatchHdlr(TextureMipMapCheck, 0x0044F49B);
		_DefinePatchHdlr(UnnecessaryDialogEdits, 0x004EDFF7);
		_DefinePatchHdlr(UnnecessaryCellEdits, 0x005349A5);
		_DefineHookHdlr(DataHandlerClearData, 0x0047AE76);
		_DefineJumpHdlr(TopicInfoCopyProlog, 0x004F0738, 0x004F07C4);
		_DefineHookHdlr(TopicInfoCopyEpilog, 0x004F1280);
		_DefineHookHdlr(NumericEditorID, 0x00497670);
		_DefineHookHdlr(DataHandlerConstructSpecialForms, 0x00481049);
		_DefineHookHdlr(ResultScriptSaveForm, 0x004FD258);
		_DefineHookHdlr(TESObjectREFRDoCopyFrom, 0x0054763D);
		_DefineHookHdlr(TESFormAddReference, 0x0049644B);
		_DefineHookHdlr(TESFormRemoveReference, 0x00496494);
		_DefineHookHdlr(TESFormClearReferenceList, 0x0049641E);
		_DefineHookHdlr(TESFormPopulateUseInfoList, 0x004964F2);
		_DefineHookHdlr(TESFormDelete, 0x00498712);
		_DefinePatchHdlr(TextureSizeCheck, 0x0044F444);
		_DefineHookHdlr(DataHandlerPlaceTESObjectLIGH, 0x005116C7);
		_DefineHookHdlr(TESWorldSpaceDestroyCellMapA, 0x00560753);
		_DefineHookHdlr(TESWorldSpaceDestroyCellMapB, 0x0079CA33);
		_DefineJumpHdlr(CSRegistryEntries, 0x00406820, 0x00406AF4);
		_DefineHookHdlr(AchievementAddTopic, 0x004F2ED4);
		_DefineHookHdlr(AchievementDeleteShadeMe, 0x004986B4);
		_DefineHookHdlr(AchievementModifyShadeMe, 0x00497BEA);
		_DefineHookHdlr(AchievementCloneHallOfFameForms, 0x00413E4F);
		_DefineHookHdlr(AchievementPluginDescription, 0x0040CD20);
		_DefineHookHdlr(AchievementBuildRoads, 0x00563CFF);
		_DefineHookHdlr(AchievementDialogResponseCreation, 0x004F2CC3);
		_DefineHookHdlr(ExtraTeleportInitItem, 0x00462702);
		_DefinePatchHdlr(AllowMultipleEditors, 0x0041C7E1);
		_DefineNopHdlr(SEHOverride, 0x0041C55F, 0x0041C56C - 0x0041C55F);
		_DefineHookHdlr(VersionControlOverride, 0x0041C895);

		void PatchMiscHooks(void)
		{
			_MemHdlr(CSInit).WriteJump();
			_MemHdlr(CSExit).WriteJump();
			_MemHdlr(TextureMipMapCheck).WriteUInt8(0xEB);
			_MemHdlr(UnnecessaryCellEdits).WriteUInt8(0xEB);
			_MemHdlr(UnnecessaryDialogEdits).WriteUInt8(0xEB);
			_MemHdlr(AssertOverride).WriteJump();
			_MemHdlr(DataHandlerClearData).WriteJump();
			_MemHdlr(TopicInfoCopyProlog).WriteJump();
			_MemHdlr(TopicInfoCopyEpilog).WriteJump();
			_MemHdlr(NumericEditorID).WriteJump();
			_MemHdlr(DataHandlerConstructSpecialForms).WriteJump();
			_MemHdlr(ResultScriptSaveForm).WriteJump();
			_MemHdlr(TESObjectREFRDoCopyFrom).WriteJump();
			_MemHdlr(TESFormAddReference).WriteJump();
			_MemHdlr(TESFormRemoveReference).WriteJump();
			_MemHdlr(TESFormClearReferenceList).WriteJump();
			_MemHdlr(TESFormPopulateUseInfoList).WriteJump();
			_MemHdlr(TESFormDelete).WriteJump();
			_MemHdlr(TextureSizeCheck).WriteUInt8(0xEB);
			_MemHdlr(DataHandlerPlaceTESObjectLIGH).WriteJump();
			_MemHdlr(TESWorldSpaceDestroyCellMapA).WriteJump();
			_MemHdlr(TESWorldSpaceDestroyCellMapB).WriteJump();
			_MemHdlr(AchievementAddTopic).WriteJump();
			_MemHdlr(AchievementDeleteShadeMe).WriteJump();
			_MemHdlr(AchievementModifyShadeMe).WriteJump();
			_MemHdlr(AchievementCloneHallOfFameForms).WriteJump();
			_MemHdlr(AchievementPluginDescription).WriteJump();
			_MemHdlr(AchievementBuildRoads).WriteJump();
			_MemHdlr(AchievementDialogResponseCreation).WriteJump();
			_MemHdlr(ExtraTeleportInitItem).WriteJump();
		}

		void PatchEntryPointHooks(void)
		{
			_MemHdlr(CSRegistryEntries).WriteJump();
			_MemHdlr(AllowMultipleEditors).WriteUInt8(0xEB);
			_MemHdlr(SEHOverride).WriteNop();
			_MemHdlr(VersionControlOverride).WriteJump();
		}

		void __stdcall MessageHandlerOverride(const char* Message)
		{
			BGSEECONSOLE->LogWarning("CS", "%s", Message);
		}

		void PatchMessageHanders(void)
		{
			// patch spammed output function calls to improve performance
			_DefinePatchHdlr(FileFinderLogMessage, 0x00468597);
			_DefineNopHdlr(DataHandlerAutoSave, 0x0043083B, 5);
			_DefineNopHdlr(AnimGroupNote, 0x004CA21D, 5);
			_DefineNopHdlr(TangentSpaceCreation, 0x0076989C, 5);
			_DefineNopHdlr(HeightMapGenA, 0x005E0D9D, 5);
			_DefineNopHdlr(HeightMapGenB, 0x005E0DB6, 5);
			_DefineNopHdlr(ModelLoadError, 0x0046C215, 5);
			_DefineNopHdlr(LoadTerrainLODQuad, 0x005583F1, 5);
			_DefineNopHdlr(FaceGenControlFreeformA, 0x0044B2EA, 5);
			_DefineNopHdlr(FaceGenControlFreeFormB, 0x0044B348, 5);
			_DefineNopHdlr(FaceGenControlStoringUndoA, 0x004DD652, 5);
			_DefineNopHdlr(FaceGenControlStoringUndoB, 0x004E8EC8, 5);
			_DefineNopHdlr(DataHandlerConstructObjectA, 0x004838D2, 5);
			_DefineNopHdlr(DataHandlerConstructObjectB, 0x00483C89, 5);
			_DefineNopHdlr(DataHandlerConstructObjectC, 0x00483D53, 5);
			_DefineNopHdlr(DataHandlerConstructObjectD, 0x0048403C, 5);
			_DefineNopHdlr(DataHandlerConstructObjectE, 0x00484137, 5);
			_DefineNopHdlr(DataHandlerLoadPluginsA, 0x0048557F, 5);
			_DefineNopHdlr(DataHandlerLoadPluginsB, 0x00484BDE, 5);
			_DefineNopHdlr(SpeedTreeMultiBound, 0x00596617, 5);
			_DefineNopHdlr(BackgroundLoaderRefs, 0x0046D3B6, 5);
			_DefineNopHdlr(TESLoadCellCrapA, 0x0052B75A, 5);
			_DefineNopHdlr(TESLoadCellCrapB, 0x00477647, 5);
			_DefineNopHdlr(TESPathGridGenerateNiNode, 0x0054ED59, 5);
			_DefineNopHdlr(TESWorldspaceBuildRoadsA, 0x00563C09, 5);
			_DefineNopHdlr(TESWorldspaceBuildRoadsB, 0x00563C71, 5);

			_MemHdlr(FileFinderLogMessage).WriteUInt8(0xEB);
			_MemHdlr(DataHandlerAutoSave).WriteNop();
			_MemHdlr(AnimGroupNote).WriteNop();
			_MemHdlr(TangentSpaceCreation).WriteNop();
			_MemHdlr(HeightMapGenA).WriteNop();
			_MemHdlr(HeightMapGenB).WriteNop();
			_MemHdlr(ModelLoadError).WriteNop();
			_MemHdlr(LoadTerrainLODQuad).WriteNop();
			_MemHdlr(FaceGenControlFreeformA).WriteNop();
			_MemHdlr(FaceGenControlFreeFormB).WriteNop();
			_MemHdlr(FaceGenControlStoringUndoA).WriteNop();
			_MemHdlr(FaceGenControlStoringUndoB).WriteNop();
			_MemHdlr(DataHandlerConstructObjectA).WriteNop();
			_MemHdlr(DataHandlerConstructObjectB).WriteNop();
			_MemHdlr(DataHandlerConstructObjectC).WriteNop();
			_MemHdlr(DataHandlerConstructObjectD).WriteNop();
			_MemHdlr(DataHandlerConstructObjectE).WriteNop();
			_MemHdlr(DataHandlerLoadPluginsA).WriteNop();
			_MemHdlr(DataHandlerLoadPluginsB).WriteNop();
			_MemHdlr(SpeedTreeMultiBound).WriteNop();
			_MemHdlr(BackgroundLoaderRefs).WriteNop();
			_MemHdlr(TESLoadCellCrapA).WriteNop();
			_MemHdlr(TESLoadCellCrapB).WriteNop();
			_MemHdlr(TESPathGridGenerateNiNode).WriteNop();
			_MemHdlr(TESWorldspaceBuildRoadsA).WriteNop();
			_MemHdlr(TESWorldspaceBuildRoadsB).WriteNop();

			if (BGSEECONSOLE->GetLogsWarnings() == false)
				return;

			const UInt32 kMessageHandlerVTBL = 0x00940760;

			SafeWrite32(kMessageHandlerVTBL + 0, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x4, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x8, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x10, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x14, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x18, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x1C, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x20, (UInt32)&MessageHandlerOverride);
			SafeWrite32(kMessageHandlerVTBL + 0x24, (UInt32)&MessageHandlerOverride);
		}

		void __stdcall DoCSInitHook()
		{
			if (!*g_HWND_CSParent || !*g_HWND_ObjectWindow || !*g_HWND_CellView || !*g_HWND_RenderWindow)
				return;								// prevents the hook from being called before the full init

			_MemHdlr(CSInit).WriteBuffer();

			BGSEEMAIN->Daemon()->ExecuteInitCallbacks(BGSEditorExtender::BGSEEDaemon::kInitCallback_Epilog);
		}

		#define _hhName	CSInit
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00419265);
			_hhSetVar(Call, 0x006E5850);
			__asm
			{
				call	[_hhGetVar(Call)]
				call	DoCSInitHook
				jmp		[_hhGetVar(Retn)]
			}
		}

		void __stdcall DoCSExitHook(HWND MainWindow)
		{
			delete BGSEEMAIN;
		}

		#define _hhName	CSExit
		_hhBegin()
		{
			__asm
			{
				push    ebx
				call    DoCSExitHook
			}
		}

		void __stdcall DoAssertOverrideHook(UInt32 EIP)
		{
			BGSEEACHIEVEMENTS->Unlock(Achievements::kWTF);

			BGSEECONSOLE->Indent();
			BGSEECONSOLE->LogAssertion("CS", "ASSERTION FAILED: 0x%08X", EIP);
			BGSEECONSOLE->Exdent();

			MessageBeep(MB_ICONHAND);
		}

		#define _hhName	AssertOverride
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004B575E);
			__asm
			{
				mov		eax, [esp]
				sub		eax, 5
				pushad
				push	eax
				call	DoAssertOverrideHook
				popad
				xor		eax, eax

				jmp		[_hhGetVar(Retn)]
			}
		}

		void __stdcall DoDataHandlerClearDataHook(void)
		{
			delete BGSEEHALLOFFAME;

			RenderSelectionGroupManager::Instance.Clear();
			GameSettingCollection::Instance->ResetCollection();
		}

		#define _hhName	DataHandlerClearData
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047AE7B);
			__asm
			{
				lea     edi, [ebx + 0x44]
				mov     ecx, edi
				pushad
				call	DoDataHandlerClearDataHook
				popad

				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	TopicInfoCopyEpilog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004F1286);
			__asm
			{
				pushad
				mov		eax, [esi]
				mov		eax, [eax + 0x94]		// SetFromActiveFile
				push	1
				mov		ecx, esi
				call	eax
				popad

				mov     [esi + 0x30], bx
				mov     eax, [edi]
				jmp		[_hhGetVar(Retn)]
			}
		}

		void __stdcall DoNumericEditorIDHook(TESForm* Form, const char* EditorID)
		{
			bool ShowWarning = atoi(INISettings::GetGeneral()->Get(INISettings::kGeneral_ShowNumericEditorIDWarning, BGSEEMAIN->INIGetter()));

			if (g_LoadingSavingPlugins == false && ShowWarning && BGSEEMAIN->Daemon()->GetFullInitComplete() && strlen(EditorID) > 0 &&
				isdigit((int)*EditorID) && (Form->formFlags & TESForm::kFormFlags_Temporary) == 0)
			{
				BGSEEUI->MsgBoxW("The editorID '%s' begins with an integer.\n\nWhile this is generally accepted by the engine, scripts referring this form might fail to run or compile as the script compiler can attempt to parse it as an integer.\n\nConsider starting the editorID with an alphabet.", EditorID);
			}
		}

		#define _hhName	NumericEditorID
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00497676);
			__asm
			{
				mov		eax, [esp + 0x4]
				pushad
				push	eax
				push	ecx
				call	DoNumericEditorIDHook
				popad

				xor		eax, eax
				push	ebp
				mov		ebp, esp
				sub		esp, 0x10
				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	DataHandlerConstructSpecialForms
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0048104E);
			_hhSetVar(Call, 0x00505070);
			__asm
			{
				pushad
				call	HallOfFame::Initialize
				popad

				call	[_hhGetVar(Call)]
				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	ResultScriptSaveForm
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004FD260);
			__asm
			{
				mov		eax, [ecx]
				mov		edx, [eax + 0x8]
				test	edx, edx
				jz		FAIL

				jmp		[_hhGetVar(Retn)]
			FAIL:
				mov		eax, 0x004FD271
				jmp		eax
			}
		}

		#define _hhName	TESObjectREFRDoCopyFrom
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00547668);
			_hhSetVar(Call1, 0x00460380);
			_hhSetVar(Call2, 0x0053F7A0);
			__asm
			{
				pushad
				push	ebx
				mov		ecx, ebp
				call	[_hhGetVar(Call1)]		// ExtraDataList::CopyList
				popad

				pushad
				mov		ecx, edi
				call	[_hhGetVar(Call2)]		// TESObjectREFR::RemoveExtraTeleport
				popad

				jmp		[_hhGetVar(Retn)]
			}
		}

		void __stdcall DoTESFormAddReferenceHook(FormCrossReferenceListT* ReferenceList, TESForm* Form)
		{
			FormCrossReferenceData* Data = FormCrossReferenceData::LookupFormInCrossReferenceList(ReferenceList, Form);
			if (Data)
			{
				Data->IncrementRefCount();
				}
			else
			{
				FormCrossReferenceData* NewNode = FormCrossReferenceData::CreateInstance(Form);
				ReferenceList->AddAt(NewNode, eListEnd);
			}
		}

		#define _hhName	TESFormAddReference
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00496461);
			__asm
			{
				push	edi
				push	eax
				call	DoTESFormAddReferenceHook
				jmp		[_hhGetVar(Retn)]
			}
		}

		void __stdcall DoTESFormRemoveReferenceHook(TESForm* Parent, tList<FormCrossReferenceData>* ReferenceList, TESForm* Form)
		{
			FormCrossReferenceData* Data = FormCrossReferenceData::LookupFormInCrossReferenceList(ReferenceList, Form);
			if (Data)
			{
				if (Data->DecrementRefCount() == 0)
				{
					HACK("Possible bug in tList::Remove, corrupts the state somehow. investigate")
					// ReferenceList->Remove(Data);

					thisCall<UInt32>(0x00452AE0, ReferenceList, Data);
					Data->DeleteInstance();
				}

 				if (ReferenceList->IsEmpty())
 					Parent->CleanupCrossReferenceList();
			}
		}

		#define _hhName	TESFormRemoveReference
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004964AE);
			__asm
			{
				push	ebx
				push	esi
				push	edi
				call	DoTESFormRemoveReferenceHook
				jmp		[_hhGetVar(Retn)]
			}
		}

		void __stdcall DoTESFormClearReferenceListHook(FormCrossReferenceListT* ReferenceList)
		{
			for (FormCrossReferenceListT::Iterator Itr = ReferenceList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
			{
				FormCrossReferenceData* Data = Itr.Get();
				Data->DeleteInstance();
			}

			ReferenceList->RemoveAll();
		}

		#define _hhName	TESFormClearReferenceList
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00496423);
			__asm
			{
				pushad
				push	ecx
				call	DoTESFormClearReferenceListHook
				popad

				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	TESFormPopulateUseInfoList
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004964FB);
			_hhSetVar(Jump, 0x00496509);
			_hhSetVar(Call, 0x004FC950);
			__asm
			{
				call	[_hhGetVar(Call)]
				test	eax, eax
				jz		FAIL

				mov		eax, [eax]
				test	eax, eax
				jz		FAIL

				jmp		[_hhGetVar(Retn)]
			FAIL:
				jmp		[_hhGetVar(Jump)]
			}
		}

		#define _hhName	TESFormDelete
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00498717);
			_hhSetVar(Jump, 0x0049872D);
			_hhSetVar(Call, 0x004FC950);
			__asm
			{
				call	[_hhGetVar(Call)]
				cmp		eax, ebx
				jz		FAIL

				mov		eax, [eax]
				jmp		[_hhGetVar(Retn)]
			FAIL:
				jmp		[_hhGetVar(Jump)]
			}
		}

		#define _hhName	DataHandlerPlaceTESObjectLIGH
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005116CF);
			_hhSetVar(Jump, 0x00511749);
			__asm
			{
				test	esi, esi
				jz		FIX
				mov     eax, [esi]
				mov     edx, [eax + 0x1A0]
				jmp		[_hhGetVar(Retn)]
			FIX:
				jmp		[_hhGetVar(Jump)]
			}
		}

		#define _hhName	TESWorldSpaceDestroyCellMapA
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00560703);
			__asm
			{
				mov		eax, [esi + eax * 4]
				cmp		eax, 0x10000
				jle		FIX
				jmp		[_hhGetVar(Retn)]
			FIX:
				xor		eax, eax
				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	TESWorldSpaceDestroyCellMapB
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0079CA38);
			_hhSetVar(Jump, 0x0079CA5A);
			__asm
			{
				mov     edi, [ecx + ebx * 4]
				cmp		edi, 0x10000
				jle		FIX

				mov		edx, [edi]
				jmp		[_hhGetVar(Retn)]
			FIX:
				xor		edi, edi
				jmp		[_hhGetVar(Jump)]
			}
		}

		#define _hhName	AchievementAddTopic
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004F2ED9);
			_hhSetVar(Call, 0x004F5D20);

			__asm	pushad
			BGSEEACHIEVEMENTS->Unlock(Achievements::kCardinalSin);
			__asm	popad

			__asm
			{
				call	[_hhGetVar(Call)]
				jmp		[_hhGetVar(Retn)]
			}
		}

		bool __stdcall CheckIsFormShadeMe(TESForm* Form)
		{
			TESForm* shadeMe = TESForm::LookupByEditorID("shadeMe");

			if (Form == shadeMe)
				return true;
			else
				return false;
		}

		#define _hhName	AchievementDeleteShadeMe
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004986B9);
			_hhSetVar(Call, 0x00401EA0);
			__asm
			{
				pushad
				push	esi
				call	CheckIsFormShadeMe
				test	al, al
				jz		EXIT
			}

			BGSEEACHIEVEMENTS->Unlock(Achievements::kAntiChrist);

			__asm
			{
			EXIT:
				popad

				call	[_hhGetVar(Call)]
				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	AchievementModifyShadeMe
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00497BEF);
			__asm
			{
				pushad
				push	esi
				call	CheckIsFormShadeMe
				test	al, al
				jz		EXIT
			}

			BGSEEACHIEVEMENTS->Unlock(Achievements::kHeretic);

			__asm
			{
			EXIT:
				popad

				mov     eax, [esi + 0x8]
				mov     ecx, eax
				jmp		[_hhGetVar(Retn)]
			}
		}

		bool __stdcall DoAchievementCloneHallOfFameForms(TESForm* Form)
		{
			if (BGSEEHALLOFFAME->GetIsInductee(Form->formID))
				return true;
			else
				return false;
		}

		#define _hhName	AchievementCloneHallOfFameForms
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00413E54);
			_hhSetVar(Call, 0x004793F0);
			__asm
			{
				pushad
				push	edi
				call	DoAchievementCloneHallOfFameForms
				test	al, al
				jz		EXIT
			}

			BGSEEACHIEVEMENTS->Unlock(Achievements::kMadScientist);

			__asm
			{
			EXIT:
				popad

				call	[_hhGetVar(Call)]
				jmp		[_hhGetVar(Retn)]
			}
		}

		void __stdcall DoAchievementPluginDescriptionHook(TESFile* File)
		{
			if (File->description.c_str() == NULL)
				return;

			std::stringstream DescriptionStream(File->description.c_str(), std::ios::in);
			char Buffer[0x200] = {0};

			while (DescriptionStream.eof() == false)
			{
				ZeroMemory(Buffer, sizeof(Buffer));
				DescriptionStream.getline(Buffer, sizeof(Buffer));

				std::string Line(Buffer);
				SME::StringHelpers::MakeLower(Line);

				if (Line.find("version:") != -1 ||
					Line.find("version :") != -1 ||
					Line.find("version-") != -1 ||
					Line.find("version -") != -1 ||
					Line.find("version=") != -1 ||
					Line.find("version =") != -1)
				{
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPedantic);
					break;
				}
			}
		}

		#define _hhName	AchievementPluginDescription
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0040CD25);
			_hhSetVar(Call, 0x004894B0);
			__asm
			{
				pushad
				push	ecx
				call	DoAchievementPluginDescriptionHook
				popad

				call	[_hhGetVar(Call)]
				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	AchievementBuildRoads
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00563D04);
			__asm	pushad
			BGSEEACHIEVEMENTS->Unlock(Achievements::kBobTheBuilder);
			__asm	popad

			__asm
			{
				call	TESDialog::WriteToStatusBar
				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	AchievementDialogResponseCreation
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004F2CC8);
			_hhSetVar(Call, 0x004EA510);
			__asm	pushad
			BGSEEACHIEVEMENTS->Unlock(Achievements::kLoquacious);
			__asm	popad

			__asm
			{
				call	[_hhGetVar(Call)]
				jmp		[_hhGetVar(Retn)]
			}
		}

		#define _hhName	ExtraTeleportInitItem
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0046270E);
			_hhSetVar(Jump, 0x004626F2);
			__asm
			{
				mov     ecx, [esi]
				mov     edx, [ecx]
				mov     eax, [edx + 0x19C]
				call    eax

				test	eax, eax
				jz		SKIP

				jmp		[_hhGetVar(Retn)]
			SKIP:
				jmp		[_hhGetVar(Jump)]
			}
		}

		#define _hhName	VersionControlOverride
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041C901);
			__asm
			{
				mov		eax, 0x009EA608
				mov     byte ptr [eax], 0
				jmp		[_hhGetVar(Retn)]
			}
		}
	}
}

#pragma warning(pop)
#pragma optimize("", on)