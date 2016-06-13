#include "RenderWindowPainter.h"
#include "Achievements.h"

namespace cse
{
	namespace renderWindow
	{
		bgsee::DynamicRenderChannel*	RenderChannelNotifications = NULL;

		bool SelectionInfoRenderChannel::DrawText(std::string& OutText)
		{
			if (settings::renderWindowPainter::kShowSelectionStats.GetData().i == 0 || _RENDERSEL->selectionCount == 0)
				return false;

			char Buffer[0x200] = { 0 };

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
				if (Selection == NULL)
					return false;			// in the off-chance that the selection contains a regular form

				BSExtraData* xData = Selection->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				char xBuffer[0x50] = { 0 };
				if (xData)
				{
					ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
					FORMAT_STR(xBuffer, "\nParent: %s [%08X]  Opposite State: %d",
							   ((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")),
							   xParent->parent->formID, xParent->oppositeState);
				}

				char cBuffer[0x50] = { 0 };
				if (Selection->parentCell->GetIsInterior() == false)
				{
					FORMAT_STR(cBuffer, "\nParent Cell: %s(%08X) %d,%d", Selection->parentCell->GetEditorID(),
							   Selection->parentCell->formID,
							   Selection->parentCell->cellData.coords->x, Selection->parentCell->cellData.coords->y);
				}

				FORMAT_STR(Buffer, "%s(%08X) BASE[%s(%08X)]\nP[%.04f, %.04f, %.04f]\nR[%.04f, %.04f, %.04f]\nS[%.04f]\nFlags: %s %s %s %s %s %s%s%s",
						   ((Selection->editorID.Size()) ? (Selection->editorID.c_str()) : ("")), Selection->formID,
						   ((Selection->baseForm->editorID.Size()) ? (Selection->baseForm->editorID.c_str()) : ("")), Selection->baseForm->formID,
						   Selection->position.x, Selection->position.y, Selection->position.z,
						   Selection->rotation.x * 57.2957763671875,
						   Selection->rotation.y * 57.2957763671875,
						   Selection->rotation.z * 57.2957763671875,
						   Selection->scale,
						   ((Selection->formFlags & TESForm::kFormFlags_QuestItem) ? ("P") : ("-")),
						   ((Selection->formFlags & TESForm::kFormFlags_Disabled) ? ("D") : ("-")),
						   ((Selection->formFlags & TESForm::kFormFlags_VisibleWhenDistant) ? ("V") : ("-")),
						   (Selection->GetInvisible() ? ("I") : ("-")),
						   (Selection->GetChildrenInvisible() ? ("CI") : ("-")),
						   (Selection->GetFrozen() ? ("F") : ("-")),
						   xBuffer,
						   cBuffer);
			}

			OutText = Buffer;
			return true;
		}

		SelectionInfoRenderChannel::SelectionInfoRenderChannel(const char* FontFace, UInt32 FontSize, RECT* DrawRect) :
			bgsee::StaticRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
														SME::StringHelpers::GetRGBD3D(settings::renderWindowPainter::kColorSelectionStats().s, 255),
														DrawRect,
														DT_WORDBREAK|DT_LEFT|DT_TOP|DT_NOCLIP)
		{
			;//
		}

		SelectionInfoRenderChannel::~SelectionInfoRenderChannel()
		{
			;//
		}

		SelectionInfoRenderChannel* SelectionInfoRenderChannel::GetInstance(const char* FontFace, UInt32 FontSize)
		{
			RECT DrawRect;
			DrawRect.left = 3;
			DrawRect.top = 3;
			DrawRect.right = 800;
			DrawRect.bottom = 600;

			static SelectionInfoRenderChannel kInstance(FontFace, FontSize, &DrawRect);

			return &kInstance;
		}

		bool RAMUsageRenderChannel::DrawText(std::string& OutText)
		{
			if (settings::renderWindowPainter::kShowRAMUsage.GetData().i == 0)
				return false;

			char Buffer[0x50] = { 0 };
			FORMAT_STR(Buffer, "RAM Usage: %d MB", GetInstance()->RAMCounter);
			OutText = Buffer;
			return true;
		}

		VOID CALLBACK RAMUsageRenderChannel::TimerCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
		{
			PROCESS_MEMORY_COUNTERS_EX MemCounter = {0};
			if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter)))
			{
				UInt32 CurrentRAMCounter = MemCounter.WorkingSetSize / (1024 * 1024);		// in megabytes
				if (CurrentRAMCounter != GetInstance()->RAMCounter)
				{
					GetInstance()->RAMCounter = CurrentRAMCounter;

					if (settings::renderWindowPainter::kShowRAMUsage.GetData().i)
						TESRenderWindow::Redraw();

					if (CurrentRAMCounter > 3000)
						BGSEEACHIEVEMENTS->Unlock(achievements::kOver3000);
				}
			}
		}

		RAMUsageRenderChannel::RAMUsageRenderChannel(const char* FontFace, UInt32 FontSize, RECT* DrawRect) :
			bgsee::StaticRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
														SME::StringHelpers::GetRGBD3D(settings::renderWindowPainter::kColorRAMUsage().s, 255),
														DrawRect,
														DT_WORDBREAK|DT_RIGHT|DT_TOP|DT_NOCLIP,
														bgsee::RenderChannelBase::kDrawAreaFlags_RightAligned),
			TimerID(0),
			RAMCounter(0)
		{
			TimerID = SetTimer(NULL, NULL, 1000, TimerCallback);
			SME_ASSERT(TimerID);
		}

		RAMUsageRenderChannel::~RAMUsageRenderChannel()
		{
			KillTimer(NULL, TimerID);
		}

		RAMUsageRenderChannel* RAMUsageRenderChannel::GetInstance(const char* FontFace /*= NULL*/, UInt32 FontSize /*= NULL*/)
		{
			RECT DrawRect;
			DrawRect.left = -185;
			DrawRect.top = 3;
			DrawRect.right = 185;
			DrawRect.bottom = 100;

			static RAMUsageRenderChannel kInstance(FontFace, FontSize, &DrawRect);

			return &kInstance;
		}

		MouseRefRenderChannel::MouseRefRenderChannel(const char* FontFace, UInt32 FontSize) :
			bgsee::StaticRenderChannel(FontSize, 0, FW_MEDIUM, FontFace,
														SME::StringHelpers::GetRGBD3D(settings::renderWindowPainter::kColorMouseRef().s, 255),
														NULL,
														DT_WORDBREAK | DT_LEFT | DT_TOP | DT_NOCLIP,
														bgsee::RenderChannelBase::kDrawAreaFlags_Default)
		{
			;//
		}

		MouseRefRenderChannel::~MouseRefRenderChannel()
		{
			;//
		}

		bool MouseRefRenderChannel::DrawText(std::string& OutText)
		{
			if (*TESRenderWindow::PathGridEditFlag == 0 && TESRenderWindow::CurrentMouseRef == NULL)
				return false;
			else if (*TESRenderWindow::PathGridEditFlag && TESRenderWindow::CurrentMousePathGridPoint == NULL)
				return false;

			SME_ASSERT((TESRenderWindow::CurrentMouseRef && TESRenderWindow::CurrentMousePathGridPoint == NULL) ||
					   (TESRenderWindow::CurrentMouseRef == NULL && TESRenderWindow::CurrentMousePathGridPoint));

			char Buffer[0x200] = { 0 }, BaseBuffer[0x100] = { 0 };
			if (TESRenderWindow::CurrentMouseRef)
			{
				TESObjectREFR* Ref = TESRenderWindow::CurrentMouseRef;
				TESForm* Base = Ref->baseForm;
				SME_ASSERT(Base);

				if (Base->GetEditorID())
					FORMAT_STR(BaseBuffer, "BASE(%s)", Base->GetEditorID());
				else
					FORMAT_STR(BaseBuffer, "BASE(%08X)", Base->formID);

				BSExtraData* xData = Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				char xBuffer[0x50] = { 0 };
				if (xData)
				{
					ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
					FORMAT_STR(xBuffer, "\nPARENT[%s%s%08X%s] OPPOSITE(%d)",
							   ((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")),
							   (xParent->parent->editorID.Size() ? "(" : ""),
							   xParent->parent->formID,
							   (xParent->parent->editorID.Size() ? ")" : ""),
							   xParent->oppositeState);
				}

				FORMAT_STR(Buffer, "%s%s%08X) %s%s",
						   (Ref->GetEditorID() ? Ref->GetEditorID() : ""),
						   (Ref->GetEditorID() ? "(" : "REF("),
						   Ref->formID,
						   BaseBuffer,
						   xBuffer);
			}
			else
			{
				TESObjectREFR* Ref = TESRenderWindow::CurrentMousePathGridPoint->linkedRef;
				if (Ref == NULL)
					return false;

				TESForm* Base = Ref->baseForm;
				SME_ASSERT(Base);

				if (Base->GetEditorID())
					FORMAT_STR(BaseBuffer, "BASE(%s)", Base->GetEditorID());
				else
					FORMAT_STR(BaseBuffer, "BASE(%08X)", Base->formID);

				FORMAT_STR(Buffer, "LINKED REF[%s%s%08X%s %s]",
						   (Ref->GetEditorID() ? Ref->GetEditorID() : ""),
						   (Ref->GetEditorID() ? "(" : ""),
						   Ref->formID,
						   (Ref->GetEditorID() ? ")" : ""),
						   BaseBuffer);
			}

			OutText = Buffer;
			RenderArea.left = TESRenderWindow::CurrentMouseCoord.x + 25;
			RenderArea.top = TESRenderWindow::CurrentMouseCoord.y + 10;
			RenderArea.right = RenderArea.left + 300;
			RenderArea.bottom = RenderArea.top + 100;

			return true;
		}

		MouseRefRenderChannel* MouseRefRenderChannel::GetInstance(const char* FontFace /*= NULL*/, UInt32 FontSize /*= NULL*/)
		{
			static MouseRefRenderChannel kInstance(FontFace, FontSize);

			return &kInstance;
		}

		RenderWindowPainterOperator::~RenderWindowPainterOperator()
		{
			;//
		}

		LPDIRECT3DDEVICE9 RenderWindowPainterOperator::GetD3DDevice(void)
		{
			return _NIRENDERER->device;
		}

		HWND RenderWindowPainterOperator::GetD3DWindow(void)
		{
			return *TESRenderWindow::WindowHandle;
		}

		void RenderWindowPainterOperator::RedrawRenderWindow(void)
		{
			TESRenderWindow::Redraw();
		}
	}
};