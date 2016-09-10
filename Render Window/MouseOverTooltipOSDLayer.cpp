#include "MouseOverTooltipOSDLayer.h"
#include "Render Window\RenderWindowManager.h"

namespace cse
{
	namespace renderWindow
	{
		MouseOverTooltipOSDLayer	MouseOverTooltipOSDLayer::Instance;

		MouseOverTooltipOSDLayer::MouseOverTooltipOSDLayer() :
			IRenderWindowOSDLayer()
		{

		}

		MouseOverTooltipOSDLayer::~MouseOverTooltipOSDLayer()
		{
			;//
		}

		void MouseOverTooltipOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (OSD->NeedsInput())
				return;
			else if (*TESRenderWindow::PathGridEditFlag == 0 && _RENDERWIN_XSTATE.CurrentMouseRef == nullptr)
				return;
			else if (*TESRenderWindow::PathGridEditFlag && _RENDERWIN_XSTATE.CurrentMousePathGridPoint == nullptr)
				return;
			else if (GetCapture() == *TESRenderWindow::WindowHandle && _RENDERWIN_MGR.GetMouseInputManager()->IsPaintingSelection() == false)
				return;

			SME_ASSERT((void*)_RENDERWIN_XSTATE.CurrentMouseRef != (void*)_RENDERWIN_XSTATE.CurrentMousePathGridPoint);

			char Buffer[0x200] = { 0 }, BaseBuffer[0x100] = { 0 };
			if (_RENDERWIN_XSTATE.CurrentMouseRef)
			{
				TESObjectREFR* Ref = _RENDERWIN_XSTATE.CurrentMouseRef;
				TESForm* Base = Ref->baseForm;
				SME_ASSERT(Base);

				if (Base->GetEditorID())
					FORMAT_STR(BaseBuffer, "Base Form: %s", Base->GetEditorID());
				else
					FORMAT_STR(BaseBuffer, "Base Form: %08X", Base->formID);

				BSExtraData* xData = Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				char xBuffer[0x50] = { 0 };
				if (xData)
				{
					ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
					FORMAT_STR(xBuffer, "\n\nParent: %s%s%08X%s, Opposite: %d",
						((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")),
							   (xParent->parent->editorID.Size() ? "(" : ""),
							   xParent->parent->formID,
							   (xParent->parent->editorID.Size() ? ")" : ""),
							   xParent->oppositeState);
				}

				char RefGroupBuffer[0x50] = { 0 };
				const char* ParentGroup = _RENDERWIN_MGR.GetGroupManager()->GetParentGroupID(Ref);
				if (ParentGroup)
					FORMAT_STR(RefGroupBuffer, "\nGroup: %s", ParentGroup);

				FORMAT_STR(Buffer, "%s%s%08X%s%s %s%s%s",
						(Ref->GetEditorID() ? Ref->GetEditorID() : ""),
						   (Ref->GetEditorID() ? "(" : ""),
						   Ref->formID,
						   (Ref->GetEditorID() ? ")" : ""),
						   Ref->IsActive() ? "*" : "",
						   BaseBuffer,
						   xBuffer,
						   RefGroupBuffer);
			}
			else
			{
				TESObjectREFR* Ref = _RENDERWIN_XSTATE.CurrentMousePathGridPoint->linkedRef;
				if (Ref == nullptr)
					return;

				TESForm* Base = Ref->baseForm;
				SME_ASSERT(Base);

				if (Base->GetEditorID())
					FORMAT_STR(BaseBuffer, "Base Form: %s", Base->GetEditorID());
				else
					FORMAT_STR(BaseBuffer, "Base Form: %08X", Base->formID);

				FORMAT_STR(Buffer, "Linked Reference: %s%s%08X%s %s",
					(Ref->GetEditorID() ? Ref->GetEditorID() : ""),
						   (Ref->GetEditorID() ? "(" : ""),
						   Ref->formID,
						   (Ref->GetEditorID() ? ")" : ""),
						   BaseBuffer);
			}

			ImGui::SetTooltip("%s", Buffer);
		}

		bool MouseOverTooltipOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		bool MouseOverTooltipOSDLayer::IsEnabled() const
		{
			int Enabled = settings::renderWindowOSD::kShowMouseRef.GetData().i;
			int ControlModified = settings::renderWindowOSD::kMouseRefCtrlModified.GetData().i;

			return Enabled && (ControlModified == false || GetAsyncKeyState(VK_CONTROL));
		}
	}
}