#include "AuxiliaryViewport.h"
#include "Resource.h"
#include "WindowManager.h"
#include "RenderWindowTextPainter.h"
#include "Hooks\Renderer.h"

AuxiliaryViewport*		AuxiliaryViewport::Singleton = NULL;

AuxiliaryViewport* AuxiliaryViewport::GetSingleton()
{
	if (Singleton == NULL)
		Singleton = new AuxiliaryViewport();

	return Singleton;
}

INT_PTR CALLBACK ViewportWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static float		s_AspectRatio = 1.0000000000f;
	static bool			s_LockAspectRatio = false;

	switch (uMsg)
	{
	case WM_SIZING:
		{
			if (s_LockAspectRatio)
			{
				RECT* NewBounds = (RECT*)lParam;
				tagRECT CurrentBounds;
				GetClientRect(hWnd, &CurrentBounds);

				switch (wParam)
				{
				case WMSZ_LEFT:
				case WMSZ_RIGHT:
					{
						//Adjust height and vertical position
						int TargetHeight = (int)((CurrentBounds.right - CurrentBounds.left) / s_AspectRatio);
						int OriginalHeight = NewBounds->bottom - NewBounds->top;
						int DiffHeight = OriginalHeight - TargetHeight;

						NewBounds->top += (DiffHeight / 2);
						NewBounds->bottom = NewBounds->top + TargetHeight;
						break;
					}
				case WMSZ_TOP:
				case WMSZ_BOTTOM:
					{
						//Adjust width and horizontal position
						int TargetWidth = (int)((CurrentBounds.bottom - CurrentBounds.top) * s_AspectRatio);
						int OriginalWidth = NewBounds->right - NewBounds->left;
						int DiffWidth = OriginalWidth - TargetWidth;

						NewBounds->left += (DiffWidth / 2);
						NewBounds->right = NewBounds->left + TargetWidth;
						break;
					}
				case WMSZ_RIGHT + WMSZ_BOTTOM:
					//Lower-right corner
					NewBounds->bottom = NewBounds->top + (int)((CurrentBounds.right - CurrentBounds.left)  / s_AspectRatio);
					break;
				case WMSZ_LEFT + WMSZ_BOTTOM:
					//Lower-left corner
					NewBounds->bottom = NewBounds->top + (int)((CurrentBounds.right - CurrentBounds.left)  / s_AspectRatio);
					break;
				case WMSZ_LEFT + WMSZ_TOP:
					//Upper-left corner
					NewBounds->left = NewBounds->right - (int)((CurrentBounds.bottom - CurrentBounds.top) * s_AspectRatio);
					break;
				case WMSZ_RIGHT + WMSZ_TOP:
					//Upper-right corner
					NewBounds->right = NewBounds->left + (int)((CurrentBounds.bottom - CurrentBounds.top) * s_AspectRatio);
					break;
				}
			}

			AUXVIEWPORT->Redraw();
			break;
		}
    case WM_MOVING:
		AUXVIEWPORT->Redraw();
        return g_WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
    case WM_ENTERSIZEMOVE:
        return g_WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		break;
	case WM_INITDIALOG:
		break;
	}

	return FALSE;
}

AuxiliaryViewport::AuxiliaryViewport()
{
	Valid = true;
	DisplayState = false;
	WindowHandle = CreateDialogParam(g_DLLInstance, MAKEINTRESOURCE(DLG_AUXVIEWPORT), *g_HWND_CSParent, ViewportWindowProc, NULL);
	ViewportCamera = (NiCamera*)FormHeap_Allocate(0x124);
	thisCall<void>(0x006FF430, ViewportCamera);		// ctor
	ViewportCamera->m_uiRefCount++;
	Frozen = false;

	if (!g_INIManager->GetINIInt("Visible", "Extender::AuxViewport"))
		DisplayState = true;

	HMENU ViewMenu = GetMenu(*g_HWND_CSParent); ViewMenu = GetSubMenu(ViewMenu, 2);
	MENUITEMINFO ItemViewAuxViewport;
	ItemViewAuxViewport.cbSize = sizeof(MENUITEMINFO);
	ItemViewAuxViewport.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemViewAuxViewport.wID = MAIN_VIEW_AUXVIEWPORT;
	ItemViewAuxViewport.fState = MFS_ENABLED|MFS_CHECKED;
	ItemViewAuxViewport.dwTypeData = "Auxiliary Viewport Window";
	ItemViewAuxViewport.cch = 0;
	InsertMenuItem(ViewMenu, 40199, FALSE, &ItemViewAuxViewport);
}

void AuxiliaryViewport::LoadINISettings()
{
	if (!Valid)
		return;

	int Top = g_INIManager->FetchSetting("Top", "Extender::AuxViewport")->GetValueAsInteger(),
		Left = g_INIManager->FetchSetting("Left", "Extender::AuxViewport")->GetValueAsInteger(),
		Right = g_INIManager->FetchSetting("Right", "Extender::AuxViewport")->GetValueAsInteger(),
		Bottom = g_INIManager->FetchSetting("Bottom", "Extender::AuxViewport")->GetValueAsInteger();

	SetWindowPos(WindowHandle, HWND_NOTOPMOST, Left, Top, Right, Bottom, NULL);
}

void AuxiliaryViewport::SaveINISettings()
{
	if (!Valid)
		return;

	tagRECT WindowRect;
	GetWindowRect(WindowHandle, &WindowRect);

	char Buffer[0x200] = {0};
	_itoa_s(WindowRect.top, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Top", "Extender::AuxViewport")->SetValue(Buffer);

	_itoa_s(WindowRect.left, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Left", "Extender::AuxViewport")->SetValue(Buffer);

	_itoa_s(WindowRect.right - WindowRect.left, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Right", "Extender::AuxViewport")->SetValue(Buffer);

	_itoa_s(WindowRect.bottom - WindowRect.top, Buffer, sizeof(Buffer), 10);
	g_INIManager->FetchSetting("Bottom", "Extender::AuxViewport")->SetValue(Buffer);

	g_INIManager->FetchSetting("Visible", "Extender::AuxViewport")->SetValue((DisplayState == true)?"1":"0");
}

bool AuxiliaryViewport::ToggleDisplayState()
{
	if (!Valid)
		return false;

	HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);

	if (IsHidden())
	{
		ShowWindow(WindowHandle, SW_SHOWNA);
		CheckMenuItem(ViewMenu, MAIN_VIEW_AUXVIEWPORT, MF_CHECKED);
		DisplayState = true;
	}
	else
	{
		ShowWindow(WindowHandle, SW_HIDE);
		CheckMenuItem(ViewMenu, MAIN_VIEW_AUXVIEWPORT, MF_UNCHECKED);
		DisplayState = false;
	}

	Redraw();
	return DisplayState;
}

void AuxiliaryViewport::Initialize()
{
	ToggleDisplayState();
	LoadINISettings();

	g_CustomMainWindowChildrenDialogs.AddHandle(WindowHandle);

	ClearScreen();
}

void AuxiliaryViewport::Deinitialize()
{
	SaveINISettings();

	g_CustomMainWindowChildrenDialogs.RemoveHandle(WindowHandle);
	DestroyWindow(WindowHandle);
}

void AuxiliaryViewport::ClearScreen()
{
	InvalidateRect(WindowHandle, NULL, TRUE);
}

void AuxiliaryViewport::Redraw()
{
	TESDialog::RedrawRenderWindow();
}

void AuxiliaryViewport::SyncViewportCamera( NiCamera* Camera )
{
	UInt32 RefCountBuffer = ViewportCamera->m_uiRefCount;

	memcpy(ViewportCamera, Camera, sizeof(NiCamera));
	ViewportCamera->m_uiRefCount = RefCountBuffer;
	ViewportCamera->m_parent = NULL;
}

bool AuxiliaryViewport::ToggleFrozenState()
{
	if (IsFrozen())
		Frozen = false;
	else
		Frozen = true;

	return Frozen;
}

void AuxiliaryViewport::Draw( NiNode* NodeToRender, NiCamera* Camera )
{
	Hooks::_MemHdlr(NiDX9RendererPresent).WriteUInt16(0x9090);
	if (Camera == NULL)
		Camera = ViewportCamera;

	RENDERTEXT->SetEnabled(false);
	_RENDERCMPT->RenderNode(Camera, NodeToRender);
	RENDERTEXT->SetEnabled(true);
	Hooks::_MemHdlr(NiDX9RendererPresent).WriteBuffer();

	DrawBackBuffer();
}

void AuxiliaryViewport::DrawBackBuffer( void )
{
	_RENDERER->device->Present(NULL, NULL, WindowHandle, NULL);
}
