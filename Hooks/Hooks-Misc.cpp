#include "Hooks-Misc.h"
#include "Hooks-Dialog.h"
#include "Hooks-Plugins.h"
#include "Hooks-Renderer.h"
#include "..\RenderWindowGroupManager.h"
#include "..\AuxiliaryViewport.h"
#include "..\Achievements.h"
#include "..\HallOfFame.h"
#include "..\WorkspaceManager.h"
#include "..\FormUndoStack.h"
#include "..\Console.h"

#pragma warning(push)
#pragma optimize("", off)
#pragma warning(disable: 4005 4748)

namespace cse
{
	namespace hooks
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
		_DefinePatchHdlr(TextureSizeCheckA, 0x0044F444);
		_DefinePatchHdlr(TextureSizeCheckB, 0x0044F3F2);
		_DefineHookHdlr(DataHandlerPlaceTESObjectLIGH, 0x005116C7);
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
		_DefineHookHdlr(TESObjectCELLGetDataFromDialog, 0x0053849E);
		_DefineHookHdlr(InteriorCellDuplicate, 0x0053CC3E);
		_DefineHookHdlr(InteriorCellLightingDataInit, 0x00532410);
		_DefineHookHdlr(ConvertDDSToRGBA32, 0x004AE4B4);
		_DefineJumpHdlr(ExportNPCFaceTextures, 0x004D9617, 0x004D9626);
		_DefineNopHdlr(TESTestAllCells, 0x00478C75, 6);
		_DefineHookHdlr(DataHandlerGetInteriorAtIndex, 0x0047BB19);
		_DefineHookHdlr(MessageHandlerShowWarning, 0x00403490);
		_DefineHookHdlr(NiControllerSequenceShowWarning, 0x00867EB0);
		_DefineHookHdlr(ExtraDataListInitForCell, 0x0053C573);
		_DefineHookHdlr(ExtraDataListInitForRef, 0x005488D8);
		_DefineHookHdlr(GameSettingCopyFrom, 0x004FA388);
		_DefineHookHdlr(DisableSplashScreen, 0x0041CDBA);
		_DefineHookHdlr(ScriptEnumerateEffectCrossRefs, 0x00433AF1);

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
			_MemHdlr(TextureSizeCheckA).WriteUInt8(0xEB);
			_MemHdlr(TextureSizeCheckB).WriteUInt8(0xEB);
			_MemHdlr(DataHandlerPlaceTESObjectLIGH).WriteJump();
			_MemHdlr(AchievementAddTopic).WriteJump();
			_MemHdlr(AchievementDeleteShadeMe).WriteJump();
			_MemHdlr(AchievementModifyShadeMe).WriteJump();
			_MemHdlr(AchievementCloneHallOfFameForms).WriteJump();
			_MemHdlr(AchievementPluginDescription).WriteJump();
			_MemHdlr(AchievementBuildRoads).WriteJump();
			_MemHdlr(AchievementDialogResponseCreation).WriteJump();
			_MemHdlr(ExtraTeleportInitItem).WriteJump();
			_MemHdlr(TESObjectCELLGetDataFromDialog).WriteJump();
			_MemHdlr(InteriorCellDuplicate).WriteJump();
			_MemHdlr(InteriorCellLightingDataInit).WriteJump();
			_MemHdlr(ConvertDDSToRGBA32).WriteJump();
			_MemHdlr(ExportNPCFaceTextures).WriteJump();
			_MemHdlr(TESTestAllCells).WriteNop();
			_MemHdlr(DataHandlerGetInteriorAtIndex).WriteJump();
			_MemHdlr(MessageHandlerShowWarning).WriteJump();
			_MemHdlr(NiControllerSequenceShowWarning).WriteJump();
			_MemHdlr(ExtraDataListInitForCell).WriteJump();
			_MemHdlr(ExtraDataListInitForRef).WriteJump();
			_MemHdlr(GameSettingCopyFrom).WriteJump();
			_MemHdlr(ScriptEnumerateEffectCrossRefs).WriteJump();
		}

		void PatchEntryPointHooks(void)
		{
			_MemHdlr(CSRegistryEntries).WriteJump();
			_MemHdlr(AllowMultipleEditors).WriteUInt8(0xEB);
			_MemHdlr(SEHOverride).WriteNop();
			_MemHdlr(VersionControlOverride).WriteJump();
			_MemHdlr(DisableSplashScreen).WriteJump();
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
			_DefineNopHdlr(AnimControllerMorphError, 0x004A67B5, 5);
			_DefineNopHdlr(ZBufferVertexColorExportA, 0x0046C533, 5);
			_DefineNopHdlr(ZBufferVertexColorExportB, 0x0046C547, 5);
			_DefinePatchHdlr(DataHandlerNonParentForm, 0x00483477);
			_DefineNopHdlr(DataHandlerExportTopicToFile, 0x004EDCFA, 5);

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
			_MemHdlr(AnimControllerMorphError).WriteNop();
			_MemHdlr(ZBufferVertexColorExportA).WriteNop();
			_MemHdlr(ZBufferVertexColorExportB).WriteNop();
			_MemHdlr(DataHandlerNonParentForm).WriteUInt8(0xEB);
			_MemHdlr(DataHandlerExportTopicToFile).WriteNop();

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
			if (*TESCSMain::WindowHandle == NULL ||
				*TESObjectWindow::Initialized == 0 ||
				*TESCellViewWindow::WindowHandle == NULL ||
				*TESRenderWindow::WindowHandle == NULL)
			{
				return;								// prevents the hook from being called before the full init
			}

			_MemHdlr(CSInit).WriteBuffer();

			BGSEEMAIN->GetDaemon()->ExecuteInitCallbacks(bgsee::Daemon::kInitCallback_Epilog);
		}

		#define _hhName	CSInit
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00419265);
			_hhSetVar(Call, 0x006E5850);
			__asm
			{
				call	_hhGetVar(Call)
				call	DoCSInitHook
				jmp		_hhGetVar(Retn)
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
			BGSEEACHIEVEMENTS->Unlock(achievements::kWTF);

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

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoDataHandlerClearDataHook(void)
		{
			delete BGSEEHALLOFFAME;
			RenderWindowGroupManager::Instance.Clear();
			GameSettingCollection::Instance->ResetCollection();
			BGSEEUNDOSTACK->Reset();
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

				jmp		_hhGetVar(Retn)
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
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoNumericEditorIDHook(TESForm* Form, const char* EditorID)
		{
			bool ShowWarning = settings::general::kShowNumericEditorIDWarning.GetData().i;

			if (TESDataHandler::PluginLoadSaveInProgress == false &&
				ShowWarning &&
				BGSEEMAIN->GetDaemon()->GetFullInitComplete() &&
				EditorID &&
				strlen(EditorID) > 0 &&
				isdigit((int)*EditorID) &&
				(Form->formFlags & TESForm::kFormFlags_Temporary) == 0)
			{
				BGSEEUI->MsgBoxW(NULL,
								MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND|MB_OK,
								"The editorID '%s' begins with an integer.\n\nWhile this is generally accepted by the engine, scripts referring this form might fail to run or compile as the script compiler might attempt to parse it as an integer.\nConsider beginning the editorID with an alphabet.\n\nThis warning can be disabled in the 'CSE Preferences' dialog (File menu).",
								EditorID);
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
				jmp		_hhGetVar(Retn)
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
				call	hallOfFame::Initialize
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
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

				jmp		_hhGetVar(Retn)
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
				call	_hhGetVar(Call1)		// ExtraDataList::CopyList
				popad

				pushad
				mov		ecx, edi
				call	_hhGetVar(Call2)		// TESObjectREFR::RemoveExtraTeleport
				popad

				jmp		_hhGetVar(Retn)
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
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESFormRemoveReferenceHook(TESForm* Parent, tList<FormCrossReferenceData>* ReferenceList, TESForm* Form)
		{
			FormCrossReferenceData* Data = FormCrossReferenceData::LookupFormInCrossReferenceList(ReferenceList, Form);
			if (Data)
			{
				if (Data->DecrementRefCount() == 0)
				{
					thisCall<void>(0x00452AE0, ReferenceList, Data);
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
				jmp		_hhGetVar(Retn)
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

				jmp		_hhGetVar(Retn)
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
				call	_hhGetVar(Call)
				test	eax, eax
				jz		FAIL

				mov		eax, [eax]
				test	eax, eax
				jz		FAIL

				jmp		_hhGetVar(Retn)
			FAIL:
				jmp		_hhGetVar(Jump)
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
				call	_hhGetVar(Call)
				cmp		eax, ebx
				jz		FAIL

				mov		eax, [eax]
				jmp		_hhGetVar(Retn)
			FAIL:
				jmp		_hhGetVar(Jump)
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
				jmp		_hhGetVar(Retn)
			FIX:
				jmp		_hhGetVar(Jump)
			}
		}

		void __stdcall DoAchievementAddTopicHook(TESTopic* Parent)
		{
			if (Parent)
			{
				if (Parent->editorID.c_str() && !_stricmp(Parent->editorID.c_str(), "GREETING"))
				{
					BGSEEACHIEVEMENTS->Unlock(achievements::kCardinalSin);
				}
			}
		}

		#define _hhName	AchievementAddTopic
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004F2ED9);
			_hhSetVar(Call, 0x004F5D20);
			__asm
			{
				pushad
				mov		eax, [ebx + 0xC]
				push	eax
				call	DoAchievementAddTopicHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
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

			BGSEEACHIEVEMENTS->Unlock(achievements::kAntiChrist);

			__asm
			{
			EXIT:
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
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

			BGSEEACHIEVEMENTS->Unlock(achievements::kHeretic);

			__asm
			{
			EXIT:
				popad

				mov     eax, [esi + 0x8]
				mov     ecx, eax
				jmp		_hhGetVar(Retn)
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

			BGSEEACHIEVEMENTS->Unlock(achievements::kMadScientist);

			__asm
			{
			EXIT:
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
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
					BGSEEACHIEVEMENTS->Unlock(achievements::kPedantic);
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

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName	AchievementBuildRoads
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00563D04);
			__asm	pushad
			BGSEEACHIEVEMENTS->Unlock(achievements::kBobTheBuilder);
			__asm	popad

			__asm
			{
				call	TESCSMain::WriteToStatusBar
				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName	AchievementDialogResponseCreation
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004F2CC8);
			_hhSetVar(Call, 0x004EA510);
			__asm	pushad
			BGSEEACHIEVEMENTS->Unlock(achievements::kLoquacious);
			__asm	popad

			__asm
			{
				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
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

				jmp		_hhGetVar(Retn)
			SKIP:
				jmp		_hhGetVar(Jump)
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
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoTESObjectCELLGetDataFromDialogHook(TESObjectCELL* Cell, HWND Dialog)
		{
			if (IsDlgButtonChecked(Dialog, TESObjectCELL::kHasWaterCheckBox))		// has water
			{
				float WaterHeight = TESDialog::GetDlgItemFloat(Dialog, TESObjectCELL::kWaterHeightTextBox);
				Cell->ModExtraCellWaterHeight(WaterHeight);

				TESWaterForm* WaterType = (TESWaterForm*)TESComboBox::GetSelectedItemData(GetDlgItem(Dialog, TESObjectCELL::kWaterTypeComboBox));
				Cell->ModExtraCellWaterType(WaterType);
			}
			else
			{
				Cell->ModExtraCellWaterHeight(0.0);
				Cell->ModExtraCellWaterType(NULL);
			}
		}

		#define _hhName		TESObjectCELLGetDataFromDialog
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005384D0);
			__asm
			{
				pushad
				push	esi
				push	edi
				call	DoTESObjectCELLGetDataFromDialogHook
				popad
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoInteriorCellDuplicateHook(TESObjectCELL* Source, TESObjectCELL* Copy)
		{
			if (Source->GetIsInterior() && Copy->GetIsInterior() &&
				Source->cellData.lighting && Copy->cellData.lighting)
			{
				Copy->cellData.lighting->ambient = Source->cellData.lighting->ambient;
				Copy->cellData.lighting->directional = Source->cellData.lighting->directional;
				Copy->cellData.lighting->directionalFade = Source->cellData.lighting->directionalFade;
				Copy->cellData.lighting->fog = Source->cellData.lighting->fog;
				Copy->cellData.lighting->fogClipDistance = Source->cellData.lighting->fogClipDistance;
				Copy->cellData.lighting->fogFar = Source->cellData.lighting->fogFar;
				Copy->cellData.lighting->fogNear = Source->cellData.lighting->fogNear;
				Copy->cellData.lighting->rotXY = Source->cellData.lighting->rotXY;
				Copy->cellData.lighting->rotZ = Source->cellData.lighting->rotZ;
			}
		}

		#define _hhName		InteriorCellDuplicate
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0053CC43);
			_hhSetVar(Call, 0x005321A0);
			__asm
			{
				pushad
				push	ebp
				push	edi
				call	DoInteriorCellDuplicateHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoInteriorCellLightingDataInitHook(TESObjectCELL::LightingData* Data)
		{
			Data->fogNear = 0.0001;
		}

		#define _hhName		InteriorCellLightingDataInit
		_hhBegin()
		{
			__asm
			{
				pushad
				push	eax
				call	DoInteriorCellLightingDataInitHook
				popad

				mov		[edi + 0x48], eax
				pop		edi
				pop		esi
				retn
			}
		}

		#define _hhName		ConvertDDSToRGBA32
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004AE4BB);
			_hhSetVar(Jump, 0x004AE6BA);
			__asm
			{
				mov		edx, [ecx]
				mov		eax, [edx + 0x14]
				call	eax
				test	eax, eax
				jz		SKIP

				jmp		_hhGetVar(Retn)
			SKIP:
				jmp		_hhGetVar(Jump)
			}
		}

		#define _hhName		DataHandlerGetInteriorAtIndex
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0047BB1E);
			_hhSetVar(Call, 0x0059AD60);
			__asm
			{
				call	_hhGetVar(Call)

				lea		eax, [esi + 0xC0]
				mov		eax, [eax + 0x10]
				mov		ecx, [esp + 0x8]
				cmp		ecx, eax			// check index with array size
				jge		SKIP

				jmp		_hhGetVar(Retn)
			SKIP:
				mov		eax, 0
				pop		esi
				retn	4
			}
		}

		bool __stdcall CheckWithWarningManager(UInt32 CallSite)
		{
			if (console::Initialized == false)
				return true;
			else
				return BGSEECONSOLE->GetWarningManager()->GetWarningEnabled(CallSite);
		}

		static bool kExtraDataList_InitErrorState =	false;
		static bool	kExtraDataList_InitCallState = false;

		void __stdcall CheckExtraDataListInitCallState(void)
		{
			if (kExtraDataList_InitCallState)
				kExtraDataList_InitErrorState = true;
		}

		#define _hhName		MessageHandlerShowWarning
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00403498);
			__asm
			{
				call	CheckExtraDataListInitCallState

				mov		eax, [esp]
				sub		eax, 5
				push	eax
				call	CheckWithWarningManager
				test	al, al
				jz		SKIP

				mov		ecx, [esp + 0x4]
				lea		eax, [esp + 0x8]
				jmp		_hhGetVar(Retn)
			SKIP:
				retn
			}
		}

		#define _hhName		NiControllerSequenceShowWarning
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00867EB6);
			__asm
			{
				mov		eax, [esp]
				sub		eax, 5
				push	eax
				call	CheckWithWarningManager
				test	al, al
				jz		SKIP

				mov		eax, [esp + 0x8]
				test	eax, eax
				jmp		_hhGetVar(Retn)
			SKIP:
				retn
			}
		}

		void __stdcall DoExtraDataListInitHook(TESForm* LinkedForm, bool State)
		{
			SME_ASSERT(LinkedForm);

			if (State)
			{
				kExtraDataList_InitCallState = true;
				kExtraDataList_InitErrorState = false;
			}
			else
			{
				if (kExtraDataList_InitErrorState == true)
				{
					BGSEECONSOLE->LogWarning("CS", "ExtraData initialization encountered errors for form %08X {%s}...",
											 LinkedForm->formID,
											 LinkedForm->GetEditorID());
				}

				kExtraDataList_InitErrorState = false;
				kExtraDataList_InitCallState = false;
			}
		}

		#define _hhName		ExtraDataListInitForCell
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0053C578);
			_hhSetVar(Call, 0x0045D740);
			__asm
			{
				pushad
				push	1
				push	esi
				call	DoExtraDataListInitHook
				popad

				call	_hhGetVar(Call)

				pushad
				push	0
				push	esi
				call	DoExtraDataListInitHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		#define _hhName		ExtraDataListInitForRef
		_hhBegin()
		{
			_hhSetVar(Retn, 0x005488DD);
			_hhSetVar(Call, 0x0045D740);
			__asm
			{
				pushad
				push	1
				push	esi
				call	DoExtraDataListInitHook
				popad

				call	_hhGetVar(Call)

				pushad
				push	0
				push	esi
				call	DoExtraDataListInitHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoGameSettingCopyFromHook(GameSetting* New, GameSetting* Source)
		{
			if (New->IsTemporary())
			{
				if (New->name == NULL)
				{
					switch (Source->GetValueType())
					{
					case Setting::kSetting_String:
						New->name = "sDefault";
						break;
					case Setting::kSetting_Float:
						New->name = "fDefault";
						break;
					default:
						New->name = "iDefault";
						break;
					}
				}
			}
		}

		#define _hhName		GameSettingCopyFrom
		_hhBegin()
		{
			_hhSetVar(Retn, 0x004FA38D);
			_hhSetVar(Call, 0x004C66D0);
			__asm
			{
				pushad
				push	edi
				push	esi
				call	DoGameSettingCopyFromHook
				popad

				call	_hhGetVar(Call)
				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoDisableSplashScreenHook(HWND SplashScreen)
		{
			if (settings::general::kDisableSplashScreen().i)
				ShowWindow(SplashScreen, SW_HIDE);
			else
			{
				ShowWindow(SplashScreen, SW_SHOW);
				UpdateWindow(SplashScreen);
			}
		}

		#define _hhName		DisableSplashScreen
		_hhBegin()
		{
			_hhSetVar(Retn, 0x0041CDC8);
			__asm
			{
				mov		[ebp - 0x14], eax
				pushad
				push	eax
				call	DoDisableSplashScreenHook
				popad

				jmp		_hhGetVar(Retn)
			}
		}

		void __stdcall DoScriptEnumerateEffectCrossRefsHook(TESForm* Form, HWND ListView)
		{
			Form->PopulateCrossReferenceListView(ListView);

			MagicItem* Item = NULL;
			if (Form->formType == TESForm::kFormType_Script)
			{
				Script* ThisScript = CS_CAST(Form, TESForm, Script);
				ScriptMagicItemCrossRefListT EffectRefs;

				ThisScript->GetEffectItemReferences(EffectRefs);
				for (ScriptMagicItemCrossRefListT::iterator Itr = EffectRefs.begin(); Itr != EffectRefs.end(); ++Itr)
					TESListView::InsertItem(ListView, *Itr);
			}
			else if ((Item = CS_CAST(Form, TESForm, MagicItem)))
			{
				MagicItemScriptCrossRefListT ScriptRefs;

				Item->GetScriptReferences(ScriptRefs);
				for (MagicItemScriptCrossRefListT::iterator Itr = ScriptRefs.begin(); Itr != ScriptRefs.end(); ++Itr)
					TESListView::InsertItem(ListView, *Itr);
			}
		}

		#define _hhName		ScriptEnumerateEffectCrossRefs
		_hhBegin()
		{
			_hhSetVar(Retn, 0x00433AF6);
			_hhSetVar(Call, 0x004964C0);
			__asm
			{
				push	ecx
				call	DoScriptEnumerateEffectCrossRefsHook

				jmp		_hhGetVar(Retn)
			}
		}
	}
}

#pragma warning(pop)
#pragma optimize("", on)