#include "AuxiliaryViewport.h"
#include "Resource.h"
#include "WindowManager.h"

/*
#define SafeRelease(x)	if (x) { x->Release(); x=NULL; }
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_TEX1)
struct D3DVERTEX
{
	float X, Y, Z;
	float U, V;		// tex coords
};

void AuxiliaryViewport::RenderTextureOnDisk( const char* Path )
{
	D3DDevice->BeginScene();

	LPDIRECT3DTEXTURE9 TextureFile;
	if (SUCCEEDED(D3DXCreateTextureFromFile(D3DDevice, (LPCSTR)Path, &TextureFile)))
	{
		D3DDevice->SetTexture(0, TextureFile);
		D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
		SafeRelease(TextureFile);
	}

	D3DDevice->EndScene();
	D3DDevice->Present(NULL, NULL, NULL, NULL);
}

bool AuxiliaryViewport::Recreate()
{
	Release();

	tagRECT WindowRect;
	GetClientRect(WindowHandle, &WindowRect);
	float Width = WindowRect.right - WindowRect.left, Height = WindowRect.bottom - WindowRect.top;
	if (Width <= 0.0 || Height <= 0.0)
		return false;

	D3DObject = Direct3DCreate9(D3D_SDK_VERSION);
	if (D3DObject == NULL)
	{
		DebugPrint("Couldn't initialize DirectX for auxiliary viewport");
		return false;
	}

	D3DPRESENT_PARAMETERS DXParams;
	ZeroMemory(&DXParams, sizeof(D3DPRESENT_PARAMETERS));
	DXParams.Windowed = true;
	DXParams.BackBufferWidth = 0;
	DXParams.BackBufferHeight = 0;
	DXParams.BackBufferFormat = D3DFMT_UNKNOWN;
	DXParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	DXParams.hDeviceWindow = WindowHandle;

	if (FAILED(D3DObject->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, WindowHandle, D3DCREATE_HARDWARE_VERTEXPROCESSING, &DXParams, &D3DDevice)))
	{
		DebugPrint("Couldn't create D3D device for auxiliary viewport");
		return false;
	}

	for (int i = 0; i < 8; i++)
	{
		D3DDevice->SetSamplerState(i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		D3DDevice->SetSamplerState(i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		D3DDevice->SetSamplerState(i, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
		D3DDevice->SetSamplerState(i, D3DSAMP_MAXANISOTROPY, 16);
	}

	D3DDevice->SetRenderState(D3DRS_AMBIENT, RGB(255,255,255));
	D3DDevice->SetRenderState(D3DRS_LIGHTING, false);
	D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	D3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	D3DVERTEX WindowQuad[] =
	{
		{ -1,	+1,		1,		0 + (1.0f / Width) * 0.5,		0 + (1.0f / Height) * 0.5 },
		{ -1,	-1,		1,		0 + (1.0f / Width) * 0.5,		1 + (1.0f / Height) * 0.5 },
		{ +1,	+1,		1,		1 + (1.0f / Width) * 0.5,		0 + (1.0f / Height) * 0.5 },
		{ +1,	-1,		1,		1 + (1.0f / Width) * 0.5,		1 + (1.0f /  Height) * 0.5 }
	};

	if (FAILED(D3DDevice->CreateVertexBuffer(4 * sizeof(D3DVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &VertexBuffer, 0)))
	{
		DebugPrint("Couldn't create vertex buffers for auxiliary viewport");
		return false;
	}

	void* VertexData = NULL;
	VertexBuffer->Lock(0, sizeof(VertexData), &VertexData, 0);
	CopyMemory(VertexData, WindowQuad, sizeof(WindowQuad));
	VertexBuffer->Unlock();

	if (FAILED(D3DDevice->SetStreamSource(0, VertexBuffer, 0, sizeof(D3DVERTEX))))
	{
		DebugPrint("Couldn't set device stream source for auxiliary viewport");
		return false;
	}

	return true;
}

void AuxiliaryViewport::Release()
{
	SafeRelease(VertexBuffer);
	SafeRelease(D3DDevice)
	SafeRelease(D3DObject);
}

void AuxiliaryViewport::RenderTextureInMemory( LPDIRECT3DTEXTURE9 Texture, bool TakeOwnership, bool ReleaseAfterUse )
{
	LPDIRECT3DTEXTURE9 TextureToRender = NULL;
	if (TakeOwnership)
	{
		LPD3DXBUFFER TextureBuffer = NULL;
		if (FAILED(D3DXCreateBuffer(0x500000, &TextureBuffer)) ||
			FAILED(D3DXSaveTextureToFileInMemory(&TextureBuffer, D3DXIFF_DDS, Texture, NULL)) ||
			FAILED(D3DXCreateTextureFromFileInMemory(AUXVIEWPORT->GetDevice(), TextureBuffer->GetBufferPointer(), TextureBuffer->GetBufferSize(), &TextureToRender)))
		{
			DebugPrint("Couldn't create temporary buffer for rendered texture copy");
		}

		SafeRelease(TextureBuffer);

		if (ReleaseAfterUse)
			SafeRelease(Texture);
	}
	else
		TextureToRender = Texture;

	if (TextureToRender == NULL)
		return;

	D3DDevice->BeginScene();

	D3DDevice->SetTexture(0, TextureToRender);
	D3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	if (ReleaseAfterUse)
		SafeRelease(TextureToRender);

	D3DDevice->EndScene();
	D3DDevice->Present(NULL, NULL, NULL, NULL);
}*/

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

// 	D3DObject = NULL;
// 	D3DDevice = NULL;
// 	VertexBuffer = NULL;

	if (g_INIManager->GetINIInt("HideOnStartup", "Extender::AuxViewport"))
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

	_itoa_s(WindowRect.top, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Top", "Extender::AuxViewport")->SetValue(g_TextBuffer);

	_itoa_s(WindowRect.left, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Left", "Extender::AuxViewport")->SetValue(g_TextBuffer);

	_itoa_s(WindowRect.right - WindowRect.left, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Right", "Extender::AuxViewport")->SetValue(g_TextBuffer);

	_itoa_s(WindowRect.bottom - WindowRect.top, g_TextBuffer, sizeof(g_TextBuffer), 10);
	g_INIManager->FetchSetting("Bottom", "Extender::AuxViewport")->SetValue(g_TextBuffer);
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

// 	if (Recreate() == false)
// 	{
// 		DebugPrint("Failed to initialize auxiliary viewport correctly");
// 		Valid = false;
// 	}

	ClearScreen();
}

void AuxiliaryViewport::Deinitialize()
{
//	Release();
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