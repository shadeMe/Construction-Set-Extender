#include "CSERenderWindowPainter.h"

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kRenderWindowPainterINISettings[kRenderWindowPainter__MAX] =
		{
			{ "ShowSelectionStats",		"1",		"Displays selection details" },
			{ "ShowRAMUsage",			"1",		"Displays Editor RAM usage" }
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetRenderWindowPainter( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("RenderWindowPainter");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kRenderWindowPainter__MAX; i++)
					kFactory.Settings.push_back(&kRenderWindowPainterINISettings[i]);
			}

			return &kFactory;
		}

		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kRendererINISettings[kRenderer__MAX] =
		{
			{ "UpdatePeriod",					"8",		"Duration, in milliseconds, between render window updates" },
			{ "UpdateViewPortAsync",			"0",		"Allow the render window to be updated in the background" },
			{ "AltRefMovementSpeed",			"0.8",		"Alternate render window movement settings" },
			{ "AltRefSnapGrid",					"2",		"Alternate render window movement settings" },
			{ "AltRefRotationSpeed",			"1.0",		"Alternate render window movement settings" },
			{ "AltRefSnapAngle",				"45",		"Alternate render window movement settings" },
			{ "AltCamRotationSpeed",			"1.0",		"Alternate render window movement settings" },
			{ "AltCamZoomSpeed",				"0.5",		"Alternate render window movement settings" },
			{ "AltCamPanSpeed",					"5.0",		"Alternate render window movement settings" },
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetRenderer( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("Renderer");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kRenderer__MAX; i++)
					kFactory.Settings.push_back(&kRendererINISettings[i]);
			}

			return &kFactory;
		}
	}

#define PI					3.151592653589793

	namespace RenderWindowPainter
	{
		BGSEditorExtender::BGSEEStaticRenderChannel*	RenderChannelSelectionStats = NULL;
		BGSEditorExtender::BGSEEStaticRenderChannel*	RenderChannelRAMUsage = NULL;
		BGSEditorExtender::BGSEEDynamicRenderChannel*	RenderChannelNotifications = NULL;

		bool RenderChannelSelectionStats_Callback( std::string& RenderedText )
		{
			int Enabled = atoi(INISettings::GetRenderWindowPainter()->Get(INISettings::kRenderWindowPainter_ShowSelectionStats, BGSEEMAIN->INIGetter()));
			if (Enabled == 0 || _RENDERSEL->selectionCount == 0)
			{
				return false;
			}

			char Buffer[0x500] = {0};

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
						xParent->parent->formID, (UInt8)xParent->oppositeState);
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
					((Selection->formFlags & kTESObjectREFRSpecialFlags_3DInvisible)?("I"):("-")),
					((Selection->formFlags & kTESObjectREFRSpecialFlags_Children3DInvisible)?("CI"):("-")),
					((Selection->formFlags & kTESObjectREFRSpecialFlags_Frozen)?("F"):("-")),
					xBuffer);
			}

			RenderedText = Buffer;
			return true;
		}

		CSERAMUsageRenderChannel*		CSERAMUsageRenderChannel::Singleton = NULL;

		bool CSERAMUsageRenderChannel::RenderChannelCallback(std::string& RenderedText)
		{
			int Enabled = atoi(INISettings::GetRenderWindowPainter()->Get(INISettings::kRenderWindowPainter_ShowRAMUsage, BGSEEMAIN->INIGetter()));
			if (Enabled == 0)
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
					TESDialog::RedrawRenderWindow();
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
														Color, DrawArea, DrawFormat, DrawAreaFlags, RenderChannelCallback),
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
				DrawRect.left = -173;
				DrawRect.top = 3;
				DrawRect.right = 173;
				DrawRect.bottom = 100;

				Singleton = new CSERAMUsageRenderChannel(20, 0, FW_MEDIUM, "Consolas",
														D3DCOLOR_ARGB(230, 230, 230, 0),
														&DrawRect,
														DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
														BGSEditorExtender::BGSEERenderChannelBase::kDrawAreaFlags_RightAligned);
			}

			return Singleton;
		}

		void RenderWindowPainter::Initialize( void )
		{
			RECT DrawRect;
			bool ComponentInitialized = BGSEERWPAINTER->Initialize(*g_HWND_RenderWindow, _RENDERER->device);

			SME_ASSERT(ComponentInitialized);

			RenderChannelRAMUsage = CSERAMUsageRenderChannel::GetSingleton();

			DrawRect.left = 3;
			DrawRect.top = 3;
			DrawRect.right = 800;
			DrawRect.bottom = 600;
			RenderChannelSelectionStats = new BGSEditorExtender::BGSEEStaticRenderChannel(20, 0, FW_MEDIUM, "Consolas",
																						D3DCOLOR_ARGB(220, 189, 237, 99),
																						&DrawRect,
																						DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
																						0,
																						&RenderChannelSelectionStats_Callback);
			DrawRect.left = 3;
			DrawRect.top = -150;
			DrawRect.right = 800;
			DrawRect.bottom = 200;
			RenderChannelNotifications = new BGSEditorExtender::BGSEEDynamicRenderChannel(20, 0, FW_MEDIUM, "Consolas",
																						D3DCOLOR_ARGB(230, 230, 230, 0),
																						&DrawRect,
																						DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP,
																						BGSEditorExtender::BGSEERenderChannelBase::kDrawAreaFlags_BottomAligned);

			BGSEERWPAINTER->RegisterRenderChannel(RenderChannelSelectionStats);
			BGSEERWPAINTER->RegisterRenderChannel(RenderChannelRAMUsage);
			BGSEERWPAINTER->RegisterRenderChannel(RenderChannelNotifications);
		}
}
};

