#include "Hooks-Events.h"

#pragma warning(push)
/*#pragma optimize("", off)*/
#pragma warning(disable: 4005 4748)

namespace cse
{
	namespace hooks
	{
		void PatchEventHooks(void)
		{
			events::plugin::_MemHdlr(PreSave).WriteJump();
			events::plugin::_MemHdlr(PostSave).WriteJump();
			events::plugin::_MemHdlr(PrePostLoad).WriteJump();
			events::plugin::_MemHdlr(ClearData).WriteJump();

			events::form::_MemHdlr(TESFormSetFromActivePlugin).WriteJump();
			events::form::_MemHdlr(TESFormSetDeleted).WriteJump();
			events::form::_MemHdlr(TESFormSetFormID).WriteJump();
			events::form::_MemHdlr(TESFormSetEditorID).WriteJump();
			events::form::_MemHdlr(TESDialogFormEditNewForm).WriteJump();
			events::form::_MemHdlr(ObjectWindowDragDropNewForm).WriteJump();
			events::form::_MemHdlr(TESDialogFormIDListViewNewForm).WriteJump();
			events::form::_MemHdlr(TESDialogCreateNewForm).WriteJump();
			events::form::_MemHdlr(TESFormClone).WriteJump();
			events::form::_MemHdlr(TESObjectCELLDuplicate).WriteJump();
			events::form::_MemHdlr(TESObjectREFRDuplicate).WriteJump();
			events::form::_MemHdlr(TESTopicCreate).WriteJump();
			events::form::_MemHdlr(TESTopicInfoCreate).WriteJump();
			events::form::_MemHdlr(TESIdleFormCreateSibling).WriteJump();
			events::form::_MemHdlr(TESIdleFormCreateChild).WriteJump();

			events::renderer::_MemHdlr(NiDX9RendererRecreateA).WriteJump();
			events::renderer::_MemHdlr(NiDX9RendererRecreateB).WriteJump();
			events::renderer::_MemHdlr(NiDX9RendererRecreateC).WriteJump();
			events::renderer::_MemHdlr(PreSceneGraphRender).WriteCall();
			events::renderer::_MemHdlr(PostSceneGraphRender).WriteJump();

			events::dialog::_MemHdlr(CloseAll).WriteJump();
			events::dialog::cellView::_MemHdlr(SelectCell).WriteJump();
			events::dialog::renderWindow::_MemHdlr(PlaceRef).WriteJump();
		}

		namespace events
		{
			namespace plugin
			{
				_DefineHookHdlr(PreSave, 0x0047EC8D);
				_DefineHookHdlr(PostSave, 0x0047F12A);
				_DefineHookHdlr(PrePostLoad, 0x0041BD98);
				_DefineHookHdlr(ClearData, 0x0047AE76);

				void __stdcall DoPrePostLoad(bool State)
				{
					if (State)
					{
						cse::events::plugin::kPreLoad.RaiseEvent();
						TESDataHandler::PluginLoadSaveInProgress = true;
					}
					else
					{
						TESDataHandler::PluginLoadSaveInProgress = false;
						cse::events::plugin::kPostLoad.RaiseEvent();
					}
				}

				#define _hhName		PrePostLoad
				_hhBegin()
				{
					_hhSetVar(Retn, 0x0041BD9D);
					_hhSetVar(Call, 0x00484D00);
					__asm
					{
						pushad
						push	1
						call	DoPrePostLoad
						popad

						call	_hhGetVar(Call)

						pushad
						push	0
						call	DoPrePostLoad
						popad

						jmp		_hhGetVar(Retn)
					}
				}

				void __stdcall DoPreSave(TESFile* SaveFile)
				{
					cse::events::plugin::kPreSave.RaiseEvent(SaveFile);
					TESDataHandler::PluginLoadSaveInProgress = true;
				}

				#define _hhName		PreSave
				_hhBegin()
				{
					_hhSetVar(Retn, 0x0047EC92);
					__asm
					{
						pushad
						push	ebx
						call	DoPreSave
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				void __stdcall DoPostSave()
				{
					TESDataHandler::PluginLoadSaveInProgress = false;
					cse::events::plugin::kPostSave.RaiseEvent();
				}

				#define _hhName		PostSave
				_hhBegin()
				{
					_hhSetVar(Retn, 0x0047F12F);
					_hhSetVar(Call, 0x00416E50);
					__asm
					{
						pushad
						call	DoPostSave
						popad

						call	_hhGetVar(Call)
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName	ClearData
				_hhBegin()
				{
					_hhSetVar(Retn, 0x0047AE7B);
					__asm
					{
						lea     edi, [ebx + 0x44]
						mov     ecx, edi
						pushad
						lea		ecx, cse::events::plugin::kClearData
						call	cse::events::BasicEventSource::RaiseEvent
						popad

						jmp		_hhGetVar(Retn)
					}
				}
			}

			namespace form
			{
				_DefineHookHdlr(TESFormSetFromActivePlugin, 0x00497BE0);
				_DefineHookHdlr(TESFormSetDeleted, 0x00498896);
				_DefineHookHdlr(TESFormSetFormID, 0x00497F63);
				_DefineHookHdlr(TESFormSetEditorID, 0x004977E0);
				_DefineHookHdlr(TESDialogFormEditNewForm, 0x00447B7A);
				_DefineHookHdlr(ObjectWindowDragDropNewForm, 0x00421CF9);
				_DefineHookHdlr(TESDialogFormIDListViewNewForm, 0x00449417);
				_DefineHookHdlr(TESDialogCreateNewForm, 0x00413E43);
				_DefineHookHdlr(TESFormClone, 0x0049803D);
				_DefineHookHdlr(TESObjectCELLDuplicate, 0x0047B3F0);
				_DefineHookHdlr(TESObjectREFRDuplicate, 0x005474C5);
				_DefineHookHdlr(TESTopicCreate, 0x004F1805);
				_DefineHookHdlr(TESTopicInfoCreate, 0x004F07E9);
				_DefineHookHdlr(TESIdleFormCreateSibling, 0x004D4093);
				_DefineHookHdlr(TESIdleFormCreateChild, 0x004D426B);


				#define _hhName		TESFormSetFromActivePlugin
				_hhBegin()
				{
					_hhSetVar(Retn, 0x00497BE6);
					__asm
					{
						mov		eax, [esp]
						cmp		eax, 0x00497460			// don't log if called from inside TESForm::UpdateUsageInfo
						jz		EXIT

						movzx	eax, byte ptr [esp + 0x4]
						pushad
						push	eax
						push	ecx
						lea		ecx, cse::events::form::kSetActive
						call	cse::events::BasicTESFormEventSource::HandleSetActive
						popad
					EXIT:
						cmp     byte ptr [esp + 0x4], 0
						push    esi

						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESFormSetDeleted
				_hhBegin()
				{
					_hhSetVar(Retn, 0x0049889C);
					_hhSetVar(Jump, 0x004988B1);
					__asm
					{
						movzx	eax, bl
						pushad
						push	eax
						push	esi
						lea		ecx, cse::events::form::kSetDeleted
						call	cse::events::BasicTESFormEventSource::HandleSetDeleted
						popad

						test	bl, bl
						jz		REMOVE

						mov     edx, [esi]
						jmp		_hhGetVar(Retn)
					REMOVE:
						jmp		_hhGetVar(Jump)
					}
				}

				#define _hhName		TESFormSetFormID
				_hhBegin()
				{
					_hhSetVar(Retn, 0x00497F68);
					_hhSetVar(Call, 0x00590360);
					__asm
					{
						pushad
						push	edi
						push	esi
						lea		ecx, cse::events::form::kSetFormID
						call	cse::events::BasicTESFormEventSource::HandleSetFormID
						popad

						call	_hhGetVar(Call)
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESFormSetEditorID
				_hhBegin()
				{
					_hhSetVar(Retn, 0x004977E5);
					_hhSetVar(Call, 0x004051E0);
					__asm
					{
						pushad
						push	esi
						push	edi
						lea		ecx, cse::events::form::kSetFormID
						call	cse::events::BasicTESFormEventSource::HandleSetEditorID
						popad

						call	_hhGetVar(Call)
						jmp		_hhGetVar(Retn)
					}
				}

				void __stdcall DoFormInstantiation(TESForm* Form)
				{
					cse::events::form::kInstantiation.HandleInstantiation(Form);
				}

				#define _hhName		TESDialogFormEditNewForm
				_hhBegin()
				{
					_hhSetVar(Retn, 0x00447B7F);
					_hhSetVar(Call, 0x004793F0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		ObjectWindowDragDropNewForm
				_hhBegin()
				{
					_hhSetVar(Retn, 0x00421CFE);
					_hhSetVar(Call, 0x004793F0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESDialogFormIDListViewNewForm
				_hhBegin()
				{
					_hhSetVar(Retn, 0x0044941C);
					_hhSetVar(Call, 0x004793F0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESDialogCreateNewForm
				_hhBegin()
				{
					_hhSetVar(Retn, 0x00413E48);
					_hhSetVar(Call, 0x004793F0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESFormClone
				_hhBegin()
				{
					_hhSetVar(Retn, 0x00498042);
					_hhSetVar(Call, 0x004793F0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESObjectCELLDuplicate
				_hhBegin()
				{
					_hhSetVar(Retn, 0x0047B3F5);
					_hhSetVar(Call, 0x0053E630);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESObjectREFRDuplicate
				_hhBegin()
				{
					_hhSetVar(Retn, 0x005474CA);
					_hhSetVar(Call, 0x00541870);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESTopicCreate
				_hhBegin()
				{
					_hhSetVar(Retn, 0x004F180A);
					_hhSetVar(Call, 0x004F12A0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESTopicInfoCreate
				_hhBegin()
				{
					_hhSetVar(Retn, 0x004F07EE);
					_hhSetVar(Call, 0x004F6300);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESIdleFormCreateSibling
				_hhBegin()
				{
					_hhSetVar(Retn, 0x004D4098);
					_hhSetVar(Call, 0x004D25D0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		TESIdleFormCreateChild
				_hhBegin()
				{
					_hhSetVar(Retn, 0x004D4270);
					_hhSetVar(Call, 0x004D25D0);
					__asm
					{
						call	_hhGetVar(Call)
						pushad
						push	eax
						call	DoFormInstantiation
						popad
						jmp		_hhGetVar(Retn)
					}
				}
			}

			namespace renderer
			{
				void __cdecl OverrideSceneGraphRendering(NiCamera* Camera, NiNode* SceneGraph, NiCullingProcess* CullingProc, BSRenderedTexture* RenderTarget)
				{
					cse::events::renderer::kPreSceneGraphRender.RaiseEvent(Camera, SceneGraph, CullingProc, RenderTarget);
				}

				_DefineCallHdlr(PreSceneGraphRender, 0x0040643A, OverrideSceneGraphRendering);
				_DefineHookHdlr(PostSceneGraphRender, 0x00406442);
				_DefineHookHdlr(NiDX9RendererRecreateA, 0x006D79E8);
				_DefineHookHdlr(NiDX9RendererRecreateB, 0x006D7A0D);
				_DefineHookHdlr(NiDX9RendererRecreateC, 0x006D7CFA);

				void __stdcall DoPostSceneGraphRender(void)
				{
					cse::events::renderer::kPostSceneGraphRender.RaiseEvent();
				}

				#define _hhName		PostSceneGraphRender
				_hhBegin()
				{
					_hhSetVar(Call, 0x0076A3B0);
					_hhSetVar(Retn, 0x00406447);
					__asm
					{
						call	_hhGetVar(Call)

						pushad
						call	DoPostSceneGraphRender
						popad

						jmp		_hhGetVar(Retn)
					}
				}

				void __stdcall DoNiDX9RendererRecreateHook(bool State)
				{
					if (State == false)
						cse::events::renderer::kRelease.RaiseEvent();
					else
						cse::events::renderer::kRenew.RaiseEvent();
				}

				#define _hhName		NiDX9RendererRecreateA
				_hhBegin()
				{
					_hhSetVar(Retn, 0x006D79ED);
					_hhSetVar(Call, 0x006D7260);
					__asm
					{
						pushad
						push	0
						call	DoNiDX9RendererRecreateHook
						popad

						call	_hhGetVar(Call)

						pushad
						push	1
						call	DoNiDX9RendererRecreateHook
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		NiDX9RendererRecreateB
				_hhBegin()
				{
					_hhSetVar(Retn, 0x006D7A12);
					_hhSetVar(Call, 0x006D7260);
					__asm
					{
						pushad
						push	0
						call	DoNiDX9RendererRecreateHook
						popad

						call	_hhGetVar(Call)

						pushad
						push	1
						call	DoNiDX9RendererRecreateHook
						popad
						jmp		_hhGetVar(Retn)
					}
				}

				#define _hhName		NiDX9RendererRecreateC
				_hhBegin()
				{
					_hhSetVar(Retn, 0x006D7CFF);
					_hhSetVar(Call, 0x006D7260);
					__asm
					{
						pushad
						push	0
						call	DoNiDX9RendererRecreateHook
						popad

						call	_hhGetVar(Call)

						pushad
						push	1
						call	DoNiDX9RendererRecreateHook
						popad
						jmp		_hhGetVar(Retn)
					}
				}
			}

			namespace dialog
			{
				_DefineHookHdlr(CloseAll, 0x00431182);

				#define _hhName		CloseAll
				_hhBegin()
				{
					_hhSetVar(Retn, 0x00431187);
					_hhSetVar(Call, 0x005A7C40);
					__asm
					{
						pushad
						lea		ecx, cse::events::dialog::kCloseAll
						call	cse::events::BasicEventSource::RaiseEvent
						popad

						call	_hhGetVar(Call)
						jmp		_hhGetVar(Retn)
					}
				}

				namespace cellView
				{
					_DefineHookHdlr(SelectCell, 0x0040A8E7);

					void __stdcall DoSelectCell()
					{
						TESObjectCELL* CurrenCell = _TES->currentInteriorCell;
						if (CurrenCell == nullptr)
							CurrenCell = _TES->currentExteriorCell;

						cse::events::dialog::cellView::kSelectCell.HandleSelectCell(CurrenCell);
					}

					#define _hhName		SelectCell
					_hhBegin()
					{
						_hhSetVar(Retn, 0x0040AFC3);
						__asm
						{
							pushad
							call	DoSelectCell
							popad

							jmp		_hhGetVar(Retn)
						}
					}
				}

				namespace renderWindow
				{
					_DefineHookHdlr(PlaceRef, 0x0042DE45);


					#define _hhName		PlaceRef
					_hhBegin()
					{
						_hhSetVar(Retn, 0x0042DE4A);
						_hhSetVar(Call, 0x0047C610);
						__asm
						{
							call	_hhGetVar(Call)

							pushad
							lea		ecx, cse::events::dialog::renderWindow::kPlaceRef
							push	eax
							call	cse::events::dialog::renderWindow::RenderWindowDialogEventSource::HandlePlaceRef
							popad

							jmp		_hhGetVar(Retn)
						}
					}
				}
			}
		}
	}
}

#pragma warning(pop)
/*#pragma optimize("", on)*/