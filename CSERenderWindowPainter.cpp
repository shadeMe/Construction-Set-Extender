#include "CSERenderWindowPainter.h"
#include "CSEAchievements.h"

namespace ConstructionSetExtender
{
#define PI					3.151592653589793

	namespace RenderWindowPainter
	{
		BGSEditorExtender::BGSEEStaticRenderChannel*	RenderChannelSelectionStats = NULL;
		BGSEditorExtender::BGSEEDynamicRenderChannel*	RenderChannelNotifications = NULL;

		bool RenderChannelSelectionStats_Callback( std::string& RenderedText )
		{
			if (Settings::RenderWindowPainter::kShowSelectionStats.GetData().i == 0 || _RENDERSEL->selectionCount == 0)
				return false;

			char Buffer[0x200] = {0};

			if (_RENDERSEL->selectionCount > 1)
			{
				FORMAT_STR(Buffer, "%d Objects Selected\nNominal Center: %.04f, %.04f, %.04f",
					_RENDERSEL->selectionCount,
					_RENDERSEL->selectionPositionVectorSum.x,
					_RENDERSEL->selectionPositionVectorSum.y,
					_RENDERSEL->selectionPositionVectorSum.z);
			}
			else
			{
				TESObjectREFR* Selection = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
				BSExtraData* xData = Selection->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				char xBuffer[0x50] = {0};
				if (xData)
				{
					ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
					FORMAT_STR(xBuffer, "Parent: %s [%08X]  Opposite State: %d",
						((xParent->parent->editorID.Size())?(xParent->parent->editorID.c_str()):("")),
						xParent->parent->formID, xParent->oppositeState);
				}

				FORMAT_STR(Buffer, "%s (%08X) BASE[%s (%08X)]\nP[%.04f, %.04f, %.04f]\nR[%.04f, %.04f, %.04f]\nS[%.04f]\nFlags: %s %s %s %s %s %s\n%s",
					((Selection->editorID.Size())?(Selection->editorID.c_str()):("")), Selection->formID,
					((Selection->baseForm->editorID.Size())?(Selection->baseForm->editorID.c_str()):("")), Selection->baseForm->formID,
					Selection->position.x, Selection->position.y, Selection->position.z,
					Selection->rotation.x * 180.0 / PI,
					Selection->rotation.y * 180.0 / PI,
					Selection->rotation.z * 180.0 / PI,
					Selection->scale,
					((Selection->formFlags & TESForm::kFormFlags_QuestItem)?("P"):("-")),
					((Selection->formFlags & TESForm::kFormFlags_Disabled)?("D"):("-")),
					((Selection->formFlags & TESForm::kFormFlags_VisibleWhenDistant)?("V"):("-")),
					(Selection->GetInvisible()?("I"):("-")),
					(Selection->GetChildrenInvisible()?("CI"):("-")),
					(Selection->GetFrozen()?("F"):("-")),
					xBuffer);
			}

			RenderedText = Buffer;
			return true;
		}

		CSERAMUsageRenderChannel*		CSERAMUsageRenderChannel::Singleton = NULL;

		bool CSERAMUsageRenderChannel::RenderChannelCallback(std::string& RenderedText)
		{
			if (Settings::RenderWindowPainter::kShowRAMUsage.GetData().i == 0)
				return false;

			char Buffer[0x50] = {0};
			FORMAT_STR(Buffer, "RAM Usage: %d MB", GetSingleton()->RAMCounter);
			RenderedText = Buffer;
			return true;
		}

		VOID CALLBACK CSERAMUsageRenderChannel::TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
		{
			PROCESS_MEMORY_COUNTERS_EX MemCounter = {0};
			if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter)))
			{
				UInt32 CurrentRAMCounter = MemCounter.WorkingSetSize / (1024 * 1024);		// in megabytes
				if (CurrentRAMCounter != GetSingleton()->RAMCounter)
				{
					GetSingleton()->RAMCounter = CurrentRAMCounter;
					TESRenderWindow::Redraw();

					if (CurrentRAMCounter > 3000)
						BGSEEACHIEVEMENTS->Unlock(Achievements::kOver3000);
				}
			}
		}

		CSERAMUsageRenderChannel::CSERAMUsageRenderChannel( INT FontHeight,
															INT FontWidth,
															UINT FontWeight,
															const char* FontFace,
															D3DCOLOR Color,
															RECT* DrawArea,
															DWORD DrawFormat,
															UInt32 DrawAreaFlags ) :
			BGSEditorExtender::BGSEEStaticRenderChannel(FontHeight, FontWidth, FontWeight, FontFace,
														Color, DrawArea, DrawFormat,
														DrawAreaFlags,
														RenderChannelCallback),
			TimerID(0),
			RAMCounter(0)
		{
			TimerID = SetTimer(NULL, NULL, 1000, TimerCallback);
			SME_ASSERT(TimerID);
		}

		CSERAMUsageRenderChannel::~CSERAMUsageRenderChannel()
		{
			KillTimer(NULL, TimerID);

			Singleton = NULL;
		}

		CSERAMUsageRenderChannel* CSERAMUsageRenderChannel::GetSingleton()
		{
			if (Singleton == NULL)
			{
				RECT DrawRect;
				DrawRect.left = -185;
				DrawRect.top = 3;
				DrawRect.right = 185;
				DrawRect.bottom = 100;

				int FontSize = Settings::RenderWindowPainter::kFontSize.GetData().i;
				const char* FontFace = Settings::RenderWindowPainter::kFontFace.GetData().s;

				Singleton = new CSERAMUsageRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
														D3DCOLOR_ARGB(230, 230, 230, 0),
														&DrawRect,
														DT_WORDBREAK|DT_RIGHT|DT_TOP|DT_NOCLIP,
														BGSEditorExtender::BGSEERenderChannelBase::kDrawAreaFlags_RightAligned);
			}

			return Singleton;
		}

		CSEMouseRefRenderChannel*			CSEMouseRefRenderChannel::Singleton = NULL;


		bool CSEMouseRefRenderChannel::RenderChannelCallback( std::string& RenderedText )
		{
			if (TESRenderWindow::CurrentMouseRef == NULL)
				return false;

			char Buffer[0x200] = {0}, BaseBuffer[0x100] = {0};
			TESObjectREFR* Ref = TESRenderWindow::CurrentMouseRef;
			TESForm* Base = Ref->baseForm;
			SME_ASSERT(Base);

			if (Base->GetEditorID())
				FORMAT_STR(BaseBuffer, "BASE(%s)", Base->GetEditorID());
			else
				FORMAT_STR(BaseBuffer, "BASE(%08X)", Base->formID);

			FORMAT_STR(Buffer, "%s%s%08X) %s",
				(Ref->GetEditorID() ? Ref->GetEditorID() : ""),
				(Ref->GetEditorID() ? "(" : "REF("),
				Ref->formID,
				BaseBuffer);

			RenderedText = Buffer;
			return true;
		}

		void CSEMouseRefRenderChannel::Render( void* Parameter, LPD3DXSPRITE RenderToSprite )
		{
			RenderArea.left = TESRenderWindow::CurrentMouseCoord.x + 25;
			RenderArea.top = TESRenderWindow::CurrentMouseCoord.y + 10;
			RenderArea.right = RenderArea.left + 300;
			RenderArea.bottom = RenderArea.top + 100;

			BGSEEStaticRenderChannel::Render(Parameter, RenderToSprite);
		}

		CSEMouseRefRenderChannel::CSEMouseRefRenderChannel( INT FontHeight,
															INT FontWidth,
															UINT FontWeight,
															const char* FontFace,
															D3DCOLOR Color,
															DWORD DrawFormat ) :
			BGSEditorExtender::BGSEEStaticRenderChannel(FontHeight, FontWidth, FontWeight, FontFace,
														Color, NULL, DrawFormat,
														BGSEditorExtender::BGSEERenderChannelBase::kDrawAreaFlags_Default,
														RenderChannelCallback)
		{
			;//
		}

		CSEMouseRefRenderChannel::~CSEMouseRefRenderChannel()
		{
			Singleton = NULL;
		}

		CSEMouseRefRenderChannel* CSEMouseRefRenderChannel::GetSingleton()
		{
			if (Singleton == NULL)
			{				
				int FontSize = Settings::RenderWindowPainter::kFontSize.GetData().i;
				const char* FontFace = Settings::RenderWindowPainter::kFontFace.GetData().s;

				Singleton = new CSEMouseRefRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
														D3DCOLOR_ARGB(255, 255, 128, 0),
														DT_SINGLELINE|DT_LEFT|DT_TOP|DT_NOCLIP);
			}

			return Singleton;
		}

		void RenderWindowPainter::Initialize( void )
		{
			RECT DrawRect;
			bool ComponentInitialized = BGSEERWPAINTER->Initialize(*TESRenderWindow::WindowHandle, _NIRENDERER->device);

			SME_ASSERT(ComponentInitialized);

			int FontSize = Settings::RenderWindowPainter::kFontSize.GetData().i;
			const char* FontFace = Settings::RenderWindowPainter::kFontFace.GetData().s;

			DrawRect.left = 3;
			DrawRect.top = 3;
			DrawRect.right = 800;
			DrawRect.bottom = 600;
			RenderChannelSelectionStats = new BGSEditorExtender::BGSEEStaticRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
																						D3DCOLOR_ARGB(220, 189, 237, 99),
																						&DrawRect,
																						DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
																						0,
																						&RenderChannelSelectionStats_Callback);

			BGSEditorExtender::RenderChannelFlyCamStatus = new BGSEditorExtender::BGSEEStaticRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
																						D3DCOLOR_ARGB(220, 189, 237, 99),
																						&DrawRect,
																						DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
																						0,
																						&BGSEditorExtender::RenderChannelFlyCamStatus_Callback);
			DrawRect.left = 3;
			DrawRect.top = -150;
			DrawRect.right = 800;
			DrawRect.bottom = 200;
			RenderChannelNotifications = new BGSEditorExtender::BGSEEDynamicRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
																						D3DCOLOR_ARGB(230, 230, 230, 0),
																						&DrawRect,
																						DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
																						BGSEditorExtender::BGSEERenderChannelBase::kDrawAreaFlags_BottomAligned);

			BGSEERWPAINTER->RegisterRenderChannel(RenderChannelSelectionStats);
			BGSEERWPAINTER->RegisterRenderChannel(CSERAMUsageRenderChannel::GetSingleton());
			BGSEERWPAINTER->RegisterRenderChannel(RenderChannelNotifications);
			BGSEERWPAINTER->RegisterRenderChannel(BGSEditorExtender::RenderChannelFlyCamStatus);
			BGSEERWPAINTER->RegisterRenderChannel(CSEMouseRefRenderChannel::GetSingleton());
		}
	}
};