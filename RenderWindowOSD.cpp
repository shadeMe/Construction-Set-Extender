#include "RenderWindowOSD.h"
#include <bgsee\RenderWindowFlyCamera.h>
#include "IMGUI\imgui_internal.h"
#include "RenderWindowManager.h"
#include "Achievements.h"
#include "Construction Set Extender_Resource.h"

namespace cse
{
	namespace renderWindow
	{
		struct CUSTOMVERTEX
		{
			float    pos[3];
			D3DCOLOR col;
			float    uv[2];
		};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

		void ImGuiDX9::RenderDrawLists(ImDrawData* draw_data)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImGuiDX9* Impl = (ImGuiDX9*)io.UserData;
			SME_ASSERT(Impl->IsInitialized());

			// Avoid rendering when minimized
			if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
				return;

			// Create and grow buffers if needed
			if (!Impl->VertexBuffer || Impl->VertexBufferSize < draw_data->TotalVtxCount)
			{
				if (Impl->VertexBuffer) { Impl->VertexBuffer->Release(); Impl->VertexBuffer = NULL; }
				Impl->VertexBufferSize = draw_data->TotalVtxCount + 5000;
				if (Impl->D3DDevice->CreateVertexBuffer(Impl->VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &Impl->VertexBuffer, NULL) < 0)
					return;
			}
			if (!Impl->IndexBuffer || Impl->IndexBufferSize < draw_data->TotalIdxCount)
			{
				if (Impl->IndexBuffer) { Impl->IndexBuffer->Release(); Impl->IndexBuffer = NULL; }
				Impl->IndexBufferSize = draw_data->TotalIdxCount + 10000;
				if (Impl->D3DDevice->CreateIndexBuffer(Impl->IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &Impl->IndexBuffer, NULL) < 0)
					return;
			}

			// Backup the DX9 state
			IDirect3DStateBlock9* d3d9_state_block = NULL;
			if (Impl->D3DDevice->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
				return;

			// Copy and convert all vertices into a single contiguous buffer
			CUSTOMVERTEX* vtx_dst;
			ImDrawIdx* idx_dst;
			if (Impl->VertexBuffer->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
				return;
			if (Impl->IndexBuffer->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
				return;
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				const ImDrawVert* vtx_src = &cmd_list->VtxBuffer[0];
				for (int i = 0; i < cmd_list->VtxBuffer.size(); i++)
				{
					vtx_dst->pos[0] = vtx_src->pos.x;
					vtx_dst->pos[1] = vtx_src->pos.y;
					vtx_dst->pos[2] = 0.0f;
					vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
					vtx_dst->uv[0] = vtx_src->uv.x;
					vtx_dst->uv[1] = vtx_src->uv.y;
					vtx_dst++;
					vtx_src++;
				}
				memcpy(idx_dst, &cmd_list->IdxBuffer[0], cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx));
				idx_dst += cmd_list->IdxBuffer.size();
			}
			Impl->VertexBuffer->Unlock();
			Impl->IndexBuffer->Unlock();
			Impl->D3DDevice->SetStreamSource(0, Impl->VertexBuffer, 0, sizeof(CUSTOMVERTEX));
			Impl->D3DDevice->SetIndices(Impl->IndexBuffer);
			Impl->D3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

			// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing
			Impl->D3DDevice->SetPixelShader(NULL);
			Impl->D3DDevice->SetVertexShader(NULL);
			Impl->D3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			Impl->D3DDevice->SetRenderState(D3DRS_LIGHTING, false);
			Impl->D3DDevice->SetRenderState(D3DRS_ZENABLE, false);
			Impl->D3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			Impl->D3DDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
			Impl->D3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			Impl->D3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			Impl->D3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			Impl->D3DDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
			Impl->D3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			Impl->D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			Impl->D3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			Impl->D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			Impl->D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			Impl->D3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			Impl->D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			Impl->D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

			// Setup orthographic projection matrix
			// Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
			{
				const float L = 0.5f, R = io.DisplaySize.x + 0.5f, T = 0.5f, B = io.DisplaySize.y + 0.5f;
				D3DMATRIX mat_identity = { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } };
				D3DMATRIX mat_projection =
				{
					2.0f / (R - L),   0.0f,         0.0f,  0.0f,
					0.0f,         2.0f / (T - B),   0.0f,  0.0f,
					0.0f,         0.0f,         0.5f,  0.0f,
					(L + R) / (L - R),  (T + B) / (B - T),  0.5f,  1.0f,
				};
				Impl->D3DDevice->SetTransform(D3DTS_WORLD, &mat_identity);
				Impl->D3DDevice->SetTransform(D3DTS_VIEW, &mat_identity);
				Impl->D3DDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
			}

			// Render command lists
			int vtx_offset = 0;
			int idx_offset = 0;
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
					if (pcmd->UserCallback)
					{
						pcmd->UserCallback(cmd_list, pcmd);
					}
					else
					{
						const RECT r = { (LONG)pcmd->ClipRect.x, (LONG)pcmd->ClipRect.y, (LONG)pcmd->ClipRect.z, (LONG)pcmd->ClipRect.w };
						Impl->D3DDevice->SetTexture(0, (LPDIRECT3DTEXTURE9)pcmd->TextureId);
						Impl->D3DDevice->SetScissorRect(&r);
						Impl->D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.size(), idx_offset, pcmd->ElemCount / 3);
					}
					idx_offset += pcmd->ElemCount;
				}
				vtx_offset += cmd_list->VtxBuffer.size();
			}

			// Restore the DX9 state
			d3d9_state_block->Apply();
			d3d9_state_block->Release();
		}

		bool ImGuiDX9::CreateFontsTexture()
		{
			// Build texture atlas
			ImGuiIO& io = ImGui::GetIO();
			ImFontConfig config;
			config.OversampleH = 8;
			config.OversampleV = 8;

			std::string FontPath(BGSEEWORKSPACE->GetDefaultWorkspace());
			FontPath += "Data\\Fonts\\DroidSans.ttf";
			if (_FILEFINDER->FindFile("Fonts\\DroidSans.ttf") == FileFinder::kFileStatus_Unpacked)
				io.Fonts->AddFontFromFileTTF(FontPath.c_str(), 15, &config);

			unsigned char* pixels;
			int width, height, bytes_per_pixel;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

			// Upload texture to graphics system
			SAFERELEASE_D3D(FontTexture);
			if (D3DDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &FontTexture, NULL) < 0)
				return false;
			D3DLOCKED_RECT tex_locked_rect;
			if (FontTexture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
				return false;
			for (int y = 0; y < height; y++)
				memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
			FontTexture->UnlockRect(0);

			// Store our identifier
			io.Fonts->TexID = (void *)FontTexture;

			return true;
		}

		ImGuiDX9::ImGuiDX9()
		{
			VertexBuffer = NULL;
			IndexBuffer = NULL;
			VertexBufferSize = 5000;
			IndexBufferSize = 10000;
			FontTexture = NULL;
			Time = 0;
			TicksPerSecond = 0;
			RenderWindowHandle = NULL;
			D3DDevice = NULL;
			MouseDoubleClicked[0] = MouseDoubleClicked[1] = false;
			Initialized = false;
		}

		ImGuiDX9::~ImGuiDX9()
		{
			Shutdown();
		}

		bool ImGuiDX9::Initialize(HWND RenderWindow, IDirect3DDevice9* Device)
		{
			RenderWindowHandle = RenderWindow;
			D3DDevice = Device;
			SME_ASSERT(RenderWindowHandle && D3DDevice);

			if (!QueryPerformanceFrequency((LARGE_INTEGER *)&TicksPerSecond))
				return false;
			if (!QueryPerformanceCounter((LARGE_INTEGER *)&Time))
				return false;

			ImGuiIO& io = ImGui::GetIO();
			io.KeyMap[ImGuiKey_Tab] = VK_TAB;
			io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
			io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
			io.KeyMap[ImGuiKey_Home] = VK_HOME;
			io.KeyMap[ImGuiKey_End] = VK_END;
			io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			io.KeyMap[ImGuiKey_A] = 'A';
			io.KeyMap[ImGuiKey_C] = 'C';
			io.KeyMap[ImGuiKey_V] = 'V';
			io.KeyMap[ImGuiKey_X] = 'X';
			io.KeyMap[ImGuiKey_Y] = 'Y';
			io.KeyMap[ImGuiKey_Z] = 'Z';
			io.MouseDoubleClickTime = 1.f;
			io.RenderDrawListsFn = RenderDrawLists;
			io.UserData = this;

			Initialized = true;
			return true;
		}

		void ImGuiDX9::Shutdown()
		{
			InvalidateDeviceObjects();
			ImGui::Shutdown();
		}

		void ImGuiDX9::NewFrame()
		{
			SME_ASSERT(Initialized);

			if (!FontTexture)
				CreateDeviceObjects();

			ImGuiIO& io = ImGui::GetIO();

			// Setup display size (every frame to accommodate for window resizing)
			RECT rect;
			GetClientRect(RenderWindowHandle, &rect);
			io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

			// Setup time step
			INT64 current_time;
			QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
			io.DeltaTime = (float)(current_time - Time) / TicksPerSecond;
			Time = current_time;

			// Read keyboard modifiers inputs
			io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
			io.KeySuper = false;

			// set up colors
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, settings::renderWindowOSD::kWindowBGAlpha().f);
			style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.61f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.49f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.31f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.78f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.80f, 0.53f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.35f, 0.55f, 0.61f, 0.51f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.69f, 0.42f, 0.39f, 0.00f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.69f, 0.42f, 0.44f, 0.44f);

			// Start the frame
			ImGui::NewFrame();

			// manually update the double click state as ImGui's default polling doesn't consistently catch the events given our conditional rendering
			io.MouseDoubleClicked[0] = MouseDoubleClicked[0];
			io.MouseDoubleClicked[1] = MouseDoubleClicked[1];
		}

		void ImGuiDX9::Render()
		{
			ImGui::Render();

			// reset mouse double click state for the next frame
			MouseDoubleClicked[0] = MouseDoubleClicked[1] = false;
		}

		void ImGuiDX9::InvalidateDeviceObjects()
		{
			if (!D3DDevice)
				return;

			SAFERELEASE_D3D(VertexBuffer);
			SAFERELEASE_D3D(IndexBuffer);

			if (LPDIRECT3DTEXTURE9 tex = (LPDIRECT3DTEXTURE9)ImGui::GetIO().Fonts->TexID)
			{
				tex->Release();
				ImGui::GetIO().Fonts->TexID = 0;
			}

			FontTexture = NULL;
		}

		bool ImGuiDX9::CreateDeviceObjects()
		{
			if (!D3DDevice)
				return false;

			if (!CreateFontsTexture())
				return false;

			return true;
		}

		bool ImGuiDX9::UpdateInputState(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			ImGuiIO& io = ImGui::GetIO();

			switch (msg)
			{
			case WM_LBUTTONDOWN:
				io.MouseDown[0] = true;
				return true;
			case WM_LBUTTONUP:
				io.MouseDown[0] = false;
				return true;
			case WM_RBUTTONDOWN:
				io.MouseDown[1] = true;
				return true;
			case WM_RBUTTONUP:
				io.MouseDown[1] = false;
				return true;
			case WM_MBUTTONDOWN:
				io.MouseDown[2] = true;
				return true;
			case WM_MBUTTONUP:
				io.MouseDown[2] = false;
				return true;
			case WM_MOUSEWHEEL:
				io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
				return true;
			case WM_MOUSEMOVE:
				io.MousePos.x = (signed short)(lParam);
				io.MousePos.y = (signed short)(lParam >> 16);
				return true;
			case WM_KEYDOWN:
				if (wParam < 256)
					io.KeysDown[wParam] = 1;
				return true;
			case WM_KEYUP:
				if (wParam < 256)
					io.KeysDown[wParam] = 0;
				return true;
			case WM_CHAR:
				// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
				if (wParam > 0 && wParam < 0x10000)
					io.AddInputCharacter((unsigned short)wParam);
				return true;
			case WM_LBUTTONDBLCLK:
				MouseDoubleClicked[0] = true;
				return true;
			case WM_RBUTTONDBLCLK:
				MouseDoubleClicked[1] = true;
				return true;
			}

			return false;
		}

		bool ImGuiDX9::NeedsInput() const
		{
			ImGuiIO& io = ImGui::GetIO();
			return io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;
		}

		bool ImGuiDX9::IsInitialized() const
		{
			return Initialized;
		}

		bool ImGuiDX9::IsDraggingWindow() const
		{
			if (Initialized == false)
				return false;

			ImGuiContext* RenderContext = ImGui::GetCurrentContext();
			if (ImGui::IsMouseDragging() && RenderContext->ActiveId == RenderContext->MovedWindowMoveId)
				return true;
			else
				return false;
		}

		IRenderWindowOSDLayer::IRenderWindowOSDLayer(INISetting& Toggle, UInt32 Priority) :
			Toggle(&Toggle),
			Priority(Priority)
		{
			;//
		}

		IRenderWindowOSDLayer::IRenderWindowOSDLayer(UInt32 Priority) :
			Toggle(NULL),
			Priority(Priority)
		{
			;//
		}

		UInt32 IRenderWindowOSDLayer::GetPriority() const
		{
			return Priority;
		}


		bool IRenderWindowOSDLayer::IsEnabled() const
		{
			return Toggle == NULL || Toggle->GetData().i == 1;
		}

		RenderWindowOSD::DialogExtraData::DialogExtraData(RenderWindowOSD* OSD) :
			bgsee::WindowExtraData(kTypeID),
			Parent(OSD)
		{
			SME_ASSERT(OSD);
		}

		RenderWindowOSD::DialogExtraData::~DialogExtraData()
		{
			Parent = NULL;
		}

		RenderWindowOSD::GUIState::GUIState()
		{
			MouseInClientArea = false;
		}



		// lParam = DialogExtraData*
#define WM_RENDERWINDOWOSD_INITXDATA			(WM_USER + 2015)

		LRESULT CALLBACK RenderWindowOSD::OSDSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, bgsee::WindowExtraDataCollection* ExtraData)
		{
			LRESULT DlgProcResult = TRUE;
			Return = false;

			if (uMsg == WM_RENDERWINDOWOSD_INITXDATA)
			{
				bool Added = ExtraData->Add((DialogExtraData*)lParam);
				SME_ASSERT(Added);
			}

			DialogExtraData* xData = BGSEE_GETWINDOWXDATA_QUICK(DialogExtraData, ExtraData);
			if (xData == NULL)
				return DlgProcResult;

			RenderWindowOSD* Parent = xData->Parent;
			ImGuiDX9* Pipeline = Parent->Pipeline;

			if (bgsee::RenderWindowFlyCamera::IsActive() && uMsg != WM_DESTROY)
			{
				// do nothing if the fly camera is active
				return DlgProcResult;
			}
			else if (Parent->RenderingLayers)
			{
				// do nothing if we're still rendering the previous frame
				return DlgProcResult;
			}

			// get input data and flag the viewport for update
			if (Pipeline->UpdateInputState(hWnd, uMsg, wParam, lParam))
			{
				if (GetCapture() != hWnd && GetActiveWindow() == hWnd)
				{
					TESRenderWindow::Redraw();

					// check if the GUI needs input, skip the org wndproc if true
					// the check is performed on the previous frame's state but it works for our purposes
					if (Pipeline->NeedsInput())
					{
						if (uMsg != WM_KEYDOWN && uMsg != WM_KEYUP)
							Return = true;
						else
						{
							// special-case shortcuts
							ImGuiIO& io = ImGui::GetIO();
							if (io.WantTextInput)
								Return = true;
						}
					}
				}
			}


			switch (uMsg)
			{
			case WM_DESTROY:
				ExtraData->Remove(DialogExtraData::kTypeID);
				delete xData;

				break;
			case WM_LBUTTONDBLCLK:
				{
					if (Parent->NeedsInput())
					{
						// preempt the vanilla handler
						Return = true;
					}
				}

				break;
			case WM_MOUSEMOVE:
			case WM_NCMOUSEMOVE:
				if (GetActiveWindow() == hWnd)
					Parent->State.MouseInClientArea = true;

				break;
			case WM_MOUSELEAVE:
			case WM_NCMOUSELEAVE:
				Parent->State.MouseInClientArea = false;

				break;
			case WM_ACTIVATE:
				if (LOWORD(wParam) == WA_INACTIVE)
					Parent->State.MouseInClientArea = false;

				break;
			case WM_TIMER:
				// our main render loop
				if (wParam == TESRenderWindow::kTimer_ViewportUpdate)
				{
					// refresh the viewport if the mouse is in the client area or there are pending notifications
					if (Parent->State.MouseInClientArea || Parent->NeedsBackgroundUpdate())
						TESRenderWindow::Redraw();

					Pipeline->NewFrame();
					Parent->RenderLayers();
				}

				break;
			}

			return DlgProcResult;
		}

		void RenderWindowOSD::RenderLayers()
		{
			SME::MiscGunk::ScopedSetter<bool> Sentry(RenderingLayers, true);
			for each (auto Itr in AttachedLayers)
			{
				if (Itr->IsEnabled())
					Itr->Draw(this, Pipeline);
			}
		}

		bool RenderWindowOSD::NeedsBackgroundUpdate() const
		{
			for each (auto Itr in AttachedLayers)
			{
				if (Itr->NeedsBackgroundUpdate())
					return true;
			}

			return false;
		}

		RenderWindowOSD::RenderWindowOSD() :
			State(),
			AttachedLayers()
		{
			Pipeline = new ImGuiDX9();
			AttachedLayers.reserve(10);
			Initialized = false;
			RenderingLayers = false;
		}

		RenderWindowOSD::~RenderWindowOSD()
		{
			DEBUG_ASSERT(Initialized == false);
			DEBUG_ASSERT(AttachedLayers.size() == 0);

			SAFEDELETE(Pipeline);
			AttachedLayers.clear();
		}

		bool RenderWindowOSD::Initialize()
		{
			SME_ASSERT(Initialized == false);
			SME_ASSERT(_NIRENDERER);

			Pipeline->Initialize(*TESRenderWindow::WindowHandle, _NIRENDERER->device);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_RenderWindow, OSDSubclassProc);
			SendMessage(*TESRenderWindow::WindowHandle, WM_RENDERWINDOWOSD_INITXDATA, NULL, (LPARAM)new DialogExtraData(this));

			AttachLayer(&DefaultOverlayOSDLayer::Instance);
			AttachLayer(&MouseOverTooltipOSDLayer::Instance);
			AttachLayer(&NotificationOSDLayer::Instance);
			AttachLayer(&ToolbarOSDLayer::Instance);
			AttachLayer(&SelectionControlsOSDLayer::Instance);
#ifndef NDEBUG
			AttachLayer(&DebugOSDLayer::Instance);
#endif
			Initialized = true;

			return Initialized;
		}

		void RenderWindowOSD::Deinitialize()
		{
			SME_ASSERT(Initialized);

			DetachLayer(&DefaultOverlayOSDLayer::Instance);
			DetachLayer(&MouseOverTooltipOSDLayer::Instance);
			DetachLayer(&NotificationOSDLayer::Instance);
			DetachLayer(&ToolbarOSDLayer::Instance);
			DetachLayer(&SelectionControlsOSDLayer::Instance);
#ifndef NDEBUG
			DetachLayer(&DebugOSDLayer::Instance);
#endif
			Initialized = false;
		}

		void RenderWindowOSD::Render()
		{
			if (Initialized && bgsee::RenderWindowFlyCamera::IsActive() == false)
			{
				if (RenderingLayers == false)
				{
					// defer the final render call until all layers are done drawing
					Pipeline->Render();
				}
			}
		}

		void RenderWindowOSD::AttachLayer(IRenderWindowOSDLayer* Layer)
		{
			SME_ASSERT(Layer);

			if (std::find(AttachedLayers.begin(), AttachedLayers.end(), Layer) != AttachedLayers.end())
				BGSEECONSOLE_MESSAGE("Attempting to re-add the same OSD layer");
			else
			{
				AttachedLayers.push_back(Layer);
				std::sort(AttachedLayers.begin(), AttachedLayers.end(),
						  [](const IRenderWindowOSDLayer* LHS, const IRenderWindowOSDLayer* RHS) { return LHS->GetPriority() > RHS->GetPriority(); });
			}
		}

		void RenderWindowOSD::DetachLayer(IRenderWindowOSDLayer* Layer)
		{
			SME_ASSERT(Layer);

			LayerArrayT::iterator Match = std::find(AttachedLayers.begin(), AttachedLayers.end(), Layer);
			if (Match != AttachedLayers.end())
				AttachedLayers.erase(Match);
		}

		void RenderWindowOSD::HandleD3DRelease()
		{
			if (Initialized)
				Pipeline->InvalidateDeviceObjects();
		}

		void RenderWindowOSD::HandleD3DRenew()
		{
			;// nothing to do here as the device objects get renewed on demand
		}

		bool RenderWindowOSD::NeedsInput() const
		{
			if (Initialized == false)
				return false;
			else
				return Pipeline->NeedsInput() || ImGui::IsRootWindowOrAnyChildHovered();
		}

		DefaultOverlayOSDLayer		DefaultOverlayOSDLayer::Instance;

		DefaultOverlayOSDLayer::DefaultOverlayOSDLayer() :
			IRenderWindowOSDLayer(settings::renderWindowOSD::kShowInfoOverlay, IRenderWindowOSDLayer::kPriority_DefaultOverlay)
		{
			;//
		}

		DefaultOverlayOSDLayer::~DefaultOverlayOSDLayer()
		{
			;//
		}

		void DefaultOverlayOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			static const float FirstCoulmnWidth = 150;

			ImGui::SetNextWindowPos(ImVec2(10, 10));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.85f);
			if (!ImGui::Begin("Default Info Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				ImGui::PopStyleVar(2);
				return;
			}

			TESObjectCELL* CurrentCell = _TES->currentInteriorCell;
			if (CurrentCell == NULL)
				CurrentCell = *TESRenderWindow::ActiveCell;

			int R = 0, G = 0, B = 0;
			const NiVector3* CameraCoords = _PRIMARYRENDERER->GetCameraWorldTranslate();

			ImGui::Columns(2, "Cell and Cam Data", false);
			{
				if (CurrentCell)
				{
					char Buffer[0x50] = { 0 };
					if (CurrentCell->GetIsInterior())
						FORMAT_STR(Buffer, "%s (%08X)   ", CurrentCell->GetEditorID(), CurrentCell->formID);
					else
					{
						FORMAT_STR(Buffer, "%s %d,%d (%08X)   ", CurrentCell->GetEditorID(), CurrentCell->cellData.coords->x,
								   CurrentCell->cellData.coords->y, CurrentCell->formID);
					}

					ImGui::Text("Current Cell:"); ImGui::SameLine(); ImGui::TextDisabled("  (?)"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					if (ImGui::IsItemHovered())
					{
						std::string Geom(TESRenderWindow::GetCellGeomDescription(CurrentCell).c_str());
						SME::StringHelpers::Tokenizer Extractor(Geom.c_str(), ",");
						std::string CurrentArg, Extract;
						int Count = 0;
						while (Extractor.NextToken(CurrentArg) != -1)
						{
							switch (Count)
							{
							case 1:
							case 2:
							case 3:
								Extract += "  " + CurrentArg.substr(1) + "\n";
								break;
							}

							Count++;
						}
						ImGui::SetTooltip("Geometry:\n%s", Extract.c_str());
					}
					ImGui::Text("%s  ", Buffer); ImGui::NextColumn();

					ImGui::Text("Camera:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					ImGui::Text("%.0f, %0.f, %0.f  ", CameraCoords->x, CameraCoords->y, CameraCoords->z); ImGui::NextColumn();
					ImGui::Separator();
				}
			}
			ImGui::Columns(1);
			SME::StringHelpers::GetRGB(settings::renderWindowOSD::kColorSelectionStats().s, R, G, B);

			ImGui::Columns(2, "Selection Data", false);
			{
				if (_RENDERSEL->selectionCount)
				{
					char Buffer[0x200] = { 0 };

					if (_RENDERSEL->selectionCount > 1)
					{
						ImGui::TextColored(ImColor(R, G, B), "%d Objects Selected", _RENDERSEL->selectionCount); ImGui::NextColumn(); ImGui::NextColumn();
						ImGui::TextColored(ImColor(R, G, B), "Nominal Center:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
						ImGui::TextColored(ImColor(R, G, B), "%.3f, %.3f, %.3f   ", _RENDERSEL->selectionPositionVectorSum.x,
										   _RENDERSEL->selectionPositionVectorSum.y, _RENDERSEL->selectionPositionVectorSum.z); ImGui::NextColumn();
					}
					else
					{
						TESObjectREFR* Selection = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
						if (Selection)	// in the off-chance that the selection contains a regular form
						{
							ImGui::TextColored(ImColor(R, G, B), "%s(%08X)", ((Selection->editorID.Size()) ? (Selection->editorID.c_str()) : ("")), Selection->formID); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%s", TESForm::GetFormTypeIDLongName(Selection->baseForm->formType)); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Base Form:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%s(%08X)",
								((Selection->baseForm->editorID.Size()) ? (Selection->baseForm->editorID.c_str()) : ("")), Selection->baseForm->formID); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Position:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%.03f, %.03f, %.03f   ", Selection->position.x, Selection->position.y, Selection->position.z); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Rotation:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%.03f, %.03f, %.03f   ",
											   Selection->rotation.x * 57.2957763671875,
											   Selection->rotation.y * 57.2957763671875,
											   Selection->rotation.z * 57.2957763671875); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Scale:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							ImGui::TextColored(ImColor(R, G, B), "%0.3f", Selection->scale); ImGui::NextColumn();

							ImGui::TextColored(ImColor(R, G, B), "Flags:");  ImGui::SameLine(); ImGui::TextDisabled("  (?)"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("P  - Persistent\nD  - Initially Disabled\nV  - Visible When Distant\nI  - Invisible (Editor only)\nCI - Children Invisible (Editor only)\nF  - Frozen (Editor only)");
							ImGui::TextColored(ImColor(R, G, B), "%s %s %s %s %s %s   ",
								((Selection->formFlags & TESForm::kFormFlags_QuestItem) ? ("P") : ("-")),
											   ((Selection->formFlags & TESForm::kFormFlags_Disabled) ? ("D") : ("-")),
											   ((Selection->formFlags & TESForm::kFormFlags_VisibleWhenDistant) ? ("V") : ("-")),
											   (Selection->GetInvisible() ? ("I") : ("-")),
											   (Selection->GetChildrenInvisible() ? ("CI") : ("-")),
											   (Selection->GetFrozen() ? ("F") : ("-"))); ImGui::NextColumn();

							BSExtraData* xData = Selection->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
							char xBuffer[0x50] = { 0 };
							if (xData)
							{
								ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
								ImGui::TextColored(ImColor(R, G, B), "Parent:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
								ImGui::TextColored(ImColor(R, G, B), "%s(%08X) %s   ",
									((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")),
												   xParent->parent->formID,
												   (xParent->oppositeState ? "[X]" : "")); ImGui::NextColumn();
							}

							char cBuffer[0x50] = { 0 };
							if (Selection->parentCell->GetIsInterior() == false)
							{
								ImGui::TextColored(ImColor(R, G, B), "Parent Cell:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
								ImGui::TextColored(ImColor(R, G, B), "%s(%08X) %d,%d   ", Selection->parentCell->GetEditorID(), Selection->parentCell->formID,
												   Selection->parentCell->cellData.coords->x, Selection->parentCell->cellData.coords->y); ImGui::NextColumn();
							}
						}

					}

					ImGui::Separator();
				}

				if (TESRenderWindow::SelectedPathGridPoints->Count() == 1)
				{
					TESPathGridPoint* Point = TESRenderWindow::SelectedPathGridPoints->Head()->Item();
					ImGui::TextColored(ImColor(R, G, B), "Position:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					ImGui::TextColored(ImColor(R, G, B), "%.03f, %.03f, %.03f   ", Point->position.x, Point->position.y, Point->position.z); ImGui::NextColumn();
					if (Point->linkedRef)
					{
						ImGui::TextColored(ImColor(R, G, B), "Linked Reference:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
						ImGui::TextColored(ImColor(R, G, B), "%s(%08X)", ((Point->linkedRef->editorID.Size()) ? (Point->linkedRef->editorID.c_str()) : ("")), Point->linkedRef->formID); ImGui::NextColumn();
					}

					ImGui::Separator();
				}
			}
			ImGui::Columns(1);

			ImGui::Columns(2, "RAM Data", false);
			{
				PROCESS_MEMORY_COUNTERS_EX MemCounter = { 0 };
				SME::StringHelpers::GetRGB(settings::renderWindowOSD::kColorRAMUsage().s, R, G, B);
				if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter)))
				{
					UInt32 CurrentRAMCounter = MemCounter.WorkingSetSize / (1024 * 1024);		// in megabytes
					ImGui::TextColored(ImColor(R, G, B), "RAM Usage:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
					ImGui::TextColored(ImColor(R, G, B), "%d MB  ", CurrentRAMCounter); ImGui::NextColumn();
				}

				ImGui::Text("FPS:"); ImGui::NextColumn(); ImGui::SetColumnOffset(-1, FirstCoulmnWidth);
				ImGui::Text("%.1f  ", ImGui::GetIO().Framerate); ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		bool DefaultOverlayOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		MouseOverTooltipOSDLayer	MouseOverTooltipOSDLayer::Instance;

		MouseOverTooltipOSDLayer::MouseOverTooltipOSDLayer() :
			IRenderWindowOSDLayer(IRenderWindowOSDLayer::kPriority_MouseTooltip)
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
			else if (*TESRenderWindow::PathGridEditFlag == 0 && TESRenderWindow::CurrentMouseRef == NULL)
				return;
			else if (*TESRenderWindow::PathGridEditFlag && TESRenderWindow::CurrentMousePathGridPoint == NULL)
				return;

			SME_ASSERT((TESRenderWindow::CurrentMouseRef && TESRenderWindow::CurrentMousePathGridPoint == NULL) ||
				(TESRenderWindow::CurrentMouseRef == NULL && TESRenderWindow::CurrentMousePathGridPoint));

			char Buffer[0x200] = { 0 }, BaseBuffer[0x100] = { 0 };
			if (TESRenderWindow::CurrentMouseRef)
			{
				TESObjectREFR* Ref = TESRenderWindow::CurrentMouseRef;
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
					FORMAT_STR(xBuffer, "\nEnable-state Parent: %s%s%08X%s, Opposite State: %d",
							((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")),
							(xParent->parent->editorID.Size() ? "(" : ""),
							xParent->parent->formID,
							(xParent->parent->editorID.Size() ? ")" : ""),
							xParent->oppositeState);
				}

				FORMAT_STR(Buffer, "%s%s%08X%s %s%s",
					(Ref->GetEditorID() ? Ref->GetEditorID() : ""),
						   (Ref->GetEditorID() ? "(" : ""),
						   Ref->formID,
						   (Ref->GetEditorID() ? ")" : ""),
						   BaseBuffer,
						   xBuffer);
			}
			else
			{
				TESObjectREFR* Ref = TESRenderWindow::CurrentMousePathGridPoint->linkedRef;
				if (Ref == NULL)
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


		NotificationOSDLayer		NotificationOSDLayer::Instance;

		NotificationOSDLayer::Notification::Notification(std::string Message) :
			Message(Message),
			StartTickCount(0)
		{
			ZeroMemory(&StartTickCount, sizeof(StartTickCount));
		}

		bool NotificationOSDLayer::Notification::HasElapsed()
		{
			if (StartTickCount == 0)
			{
				StartTickCount = GetTickCount64();
				return false;
			}

			if (GetTickCount64() - StartTickCount > kNotificationDisplayTime)
				return true;
			else
				return false;
		}


		NotificationOSDLayer::NotificationOSDLayer() :
			IRenderWindowOSDLayer(settings::renderWindowOSD::kShowNotifications, IRenderWindowOSDLayer::kPriority_Notifications),
			Notifications()
		{
			;//
		}


		NotificationOSDLayer::~NotificationOSDLayer()
		{
			while (Notifications.size())
				Notifications.pop();
		}

		void NotificationOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (HasNotifications() == false)
				return;

			ImGui::SetNextWindowPos(ImVec2(10, *TESRenderWindow::ScreeHeight - 75));
			if (!ImGui::Begin("Notification Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
							  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs))
			{
				ImGui::End();
				return;
			}

			ImGui::Text("%s", GetNextNotification().Message.c_str());
			ImGui::End();
		}

		bool NotificationOSDLayer::NeedsBackgroundUpdate()
		{
			return HasNotifications();
		}

		bool NotificationOSDLayer::HasNotifications()
		{
			while (Notifications.size())
			{
				Notification& Next = Notifications.front();
				if (Next.HasElapsed())
					Notifications.pop();
				else
					return true;
			}

			return false;
		}

		const NotificationOSDLayer::Notification& NotificationOSDLayer::GetNextNotification() const
		{
			SME_ASSERT(Notifications.size() > 0);

			return Notifications.front();
		}

		void NotificationOSDLayer::ShowNotification(const char* Format, ...)
		{
			if (Format == NULL)
				return;

			char Buffer[0x1000] = { 0 };
			va_list Args;
			va_start(Args, Format);
			vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
			va_end(Args);

			if (strlen(Buffer))
				Notifications.push(Notification(Buffer));
		}

		DebugOSDLayer			DebugOSDLayer::Instance;

		DebugOSDLayer::DebugOSDLayer() :
			IRenderWindowOSDLayer(IRenderWindowOSDLayer::kPriority_Debug)
		{

		}

		DebugOSDLayer::~DebugOSDLayer()
		{
			;//
		}

		void DebugOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			ImGui::ShowTestWindow();
		}

		bool DebugOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		ToolbarOSDLayer				ToolbarOSDLayer::Instance;

		void ToolbarOSDLayer::RenderBottomToolbar()
		{
			int XSize = *TESRenderWindow::ScreeWidth;
			int YPos = *TESRenderWindow::ScreeHeight - (BottomExpanded ? 65 : 35);
			int YSize = BottomExpanded ? 65 : 35;

			ImGui::SetNextWindowPos(ImVec2(0, YPos));
			ImGui::SetNextWindowSize(ImVec2(XSize, YSize));

			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
			if (!ImGui::Begin("Bottom Toolbar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus |
							  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				ImGui::PopStyleVar(2);
				return;
			}

			float CamPan = *TESRenderWindow::CameraPanSpeed;
			float CamZoom = *TESRenderWindow::CameraZoomSpeed;
			float CamRot = *TESRenderWindow::CameraRotationSpeed;
			float RefMov = *TESRenderWindow::RefMovementSpeed;
			float RefRot = *TESRenderWindow::RefRotationSpeed;
			float TOD = _TES->GetSkyTOD();
			UInt32 Flags = *TESRenderWindow::StateFlags;
			bool SnapGrid = Flags & TESRenderWindow::kRenderWindowState_SnapToGrid;
			bool SnapAngle = Flags & TESRenderWindow::kRenderWindowState_SnapToAngle;
			int GridVal = *(UInt32*)TESRenderWindow::SnapGridDistance;
			int AngleVal = *(UInt32*)TESRenderWindow::SnapAngle;
			TESObjectREFR* SnapRef = *TESRenderWindow::SnapReference;
			float FOV = settings::renderer::kCameraFOV().f;

			if (ImGui::Checkbox("Snap Grid", &SnapGrid))
			{
				if (SnapGrid)
					Flags |= TESRenderWindow::kRenderWindowState_SnapToGrid;
				else
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToGrid;
			} ImGui::SameLine();
			ImGui::PushItemWidth(50); ImGui::DragInt("##gridDist", &GridVal, 1, 0, 5000); ImGui::PopItemWidth(); ImGui::SameLine(0, 50);

			if (ImGui::Checkbox("Snap Angle", &SnapAngle))
			{
				if (SnapAngle)
					Flags |= TESRenderWindow::kRenderWindowState_SnapToAngle;
				else
					Flags &= ~TESRenderWindow::kRenderWindowState_SnapToAngle;
			} ImGui::SameLine();
			ImGui::PushItemWidth(50); ImGui::DragInt("##anglVal", &AngleVal, 1, 0, 500); ImGui::PopItemWidth(); ImGui::SameLine(0, 50);

			char Buffer[0x100] = {0};
			if (SnapRef)
			{
				const char* SnapRefID = SnapRef->GetEditorID();
				FORMAT_STR(Buffer, "%s%s%08X%s", (SnapRefID ? SnapRefID : ""), (SnapRefID ? " (" : ""), SnapRef->formID, (SnapRefID ? ")" : ""));
			}
			else
				FORMAT_STR(Buffer, "Snap Reference");

			ImGui::PushItemWidth(100);
			if (ImGui::Button(Buffer))
				SnapRef = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, SnapRef, true);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(Buffer);
			ImGui::PopItemWidth(); ImGui::SameLine(0, 50);


			ImGui::PushItemWidth(100);
			ImGui::DragFloat("##TOD", &TOD, 0.25f, 0.f, 24.f, "TOD: %.2f");
			if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
				ImGui::SetTooltip("Time of Day");

			ImGui::SameLine(0, 50);
			ImGui::DragFloat("##FOV", &FOV, 1.f, 50.f, 120.f, "FOV: %.0f");
			if (ImGui::IsItemHovered() && ImGui::IsItemActive() == false)
				ImGui::SetTooltip("Camera FOV");
			ImGui::PopItemWidth();


			ImGui::SameLine(XSize - 35);
			ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0, 0.7f, 0.7f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0, 0.8f, 0.8f));
			if (ImGui::Button((BottomExpanded ? " V " : " ^ ")))
				BottomExpanded = BottomExpanded == false;

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip(BottomExpanded ? "Close" : "Expand");

			ImGui::PopStyleColor(3);

			ImGui::SameLine(XSize - 60);
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Click and drag the values to change them. CTRL + click to directly edit them.");

			if (BottomExpanded)
			{
				ImGui::Text("Camera:"); ImGui::SameLine(0, 10);
				ImGui::PushItemWidth(125);
				ImGui::DragFloat("##Cam. Pan", &CamPan, 0.05f, 0.01, 10, "Pan: %.3f"); ImGui::SameLine();
				ImGui::DragFloat("##Cam. Zoom", &CamZoom, 0.05f, 0.01, 10, "Zoom: %.3f"); ImGui::SameLine();
				ImGui::DragFloat("##Cam. Rotation", &CamRot, 0.05f, 0.01, 10, "Rotation: %.3f"); ImGui::SameLine(0, 20);
				ImGui::PopItemWidth();

				ImGui::Text("Reference:"); ImGui::SameLine(0, 10);
				ImGui::PushItemWidth(125);
				ImGui::DragFloat("##Ref. Move", &RefMov, 0.05f, 0.01, 10, "Movement: %.3f"); ImGui::SameLine();
				ImGui::DragFloat("##Ref. Rotation", &RefRot, 0.05f, 0.01, 10, "Rotation: %.3f"); ImGui::SameLine(0, 20);
				ImGui::PopItemWidth();
			}


			*TESRenderWindow::CameraPanSpeed = CamPan;
			*TESRenderWindow::CameraZoomSpeed = CamZoom;
			*TESRenderWindow::CameraRotationSpeed = CamRot;
			*TESRenderWindow::RefMovementSpeed = RefMov;
			*TESRenderWindow::RefRotationSpeed = RefRot;
			*(UInt32*)TESRenderWindow::SnapGridDistance = GridVal;
			*(UInt32*)TESRenderWindow::SnapAngle = AngleVal;
			*TESRenderWindow::StateFlags = Flags;
			*TESRenderWindow::SnapReference = SnapRef;

			if (TOD < 0 || TOD > 24)
				TOD = 10;

			if (TOD != _TES->GetSkyTOD())
				_TES->SetSkyTOD(TOD);

			if (FOV < 50 || FOV > 120)
				FOV = 75;

			if (settings::renderer::kCameraFOV().f != FOV)
			{
				settings::renderer::kCameraFOV.SetFloat(FOV);
				_RENDERWIN_MGR.RefreshFOV();
			}

			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		ToolbarOSDLayer::ToolbarOSDLayer():
			IRenderWindowOSDLayer(settings::renderWindowOSD::kShowToolbar, IRenderWindowOSDLayer::kPriority_Toolbar)
		{
			BottomExpanded = false;
		}

		ToolbarOSDLayer::~ToolbarOSDLayer()
		{
			;//
		}

		void ToolbarOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			RenderBottomToolbar();
		}

		bool ToolbarOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		SelectionControlsOSDLayer		SelectionControlsOSDLayer::Instance;

		void SelectionControlsOSDLayer::EditBaseForm(TESObjectREFR* Ref)
		{
			TESDialog::ShowFormEditDialog(Ref->baseForm);
		}

		void SelectionControlsOSDLayer::CheckDragChange(ImGuiDX9* GUI, bool& OutDragBegin, bool& OutDragEnd)
		{
			if (GUI->IsDraggingWindow())
				return;
			else if (ImGui::IsMouseHoveringWindow() == false)
				return;

			if (ImGui::IsMouseDragging() && ImGui::IsAnyItemActive())
			{
				if (DragActive == false)
				{
					DragActive = true;
					OutDragBegin = true;
				}
			}
			else if (ImGui::IsMouseDragging() == false)
			{
				if (DragActive)
				{
					DragActive = false;
					OutDragEnd = true;
				}
			}
		}

		void SelectionControlsOSDLayer::CheckTextInputChange(ImGuiDX9* GUI, bool& OutGotFocus, bool& OutLostFocus)
		{
			if (ImGui::IsMouseHoveringWindow() == false)
				return;

			ImGuiIO& io = ImGui::GetIO();
			if (io.WantTextInput)
			{
				if (TextInputActive == false)
				{
					TextInputActive = true;
					OutGotFocus = true;
				}
			}
			else if (TextInputActive)
			{
				TextInputActive = false;
				OutLostFocus = true;
			}
		}

		void SelectionControlsOSDLayer::DrawDragTrail()
		{
			if (ImGui::IsItemActive())
			{
				ImDrawList* DrawList = ImGui::GetWindowDrawList();
				DrawList->PushClipRectFullScreen();
				DrawList->AddLine(ImGui::CalcItemRectClosestPoint(ImGui::GetIO().MousePos, true, -2.0f),
									ImGui::GetIO().MousePos,
									ImColor(ImGui::GetStyle().Colors[ImGuiCol_Button]),
								  4.0f);
				DrawList->PopClipRect();
			}
		}

		void SelectionControlsOSDLayer::MoveSelection(bool X, bool Y, bool Z)
		{
			if (ImGui::IsItemActive() == false)
				return;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;
			_PRIMARYRENDERER->MoveReferenceSelection(Delta.x, Delta.x, X, Y, Z);
		}

		void SelectionControlsOSDLayer::RotateSelection(bool Local, bool X, bool Y, bool Z)
		{
			if (ImGui::IsItemActive() == false)
				return;

			UInt32 FlagsState = *TESRenderWindow::StateFlags;
			UInt32 FlagsBuffer = FlagsState & ~TESRenderWindow::kRenderWindowState_UseWorld;
			if (Local == false)
				FlagsBuffer |= TESRenderWindow::kRenderWindowState_UseWorld;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;

			*TESRenderWindow::StateFlags = FlagsBuffer;
			_PRIMARYRENDERER->RotateReferenceSelection(Delta.x, X, Y, Z);
			*TESRenderWindow::StateFlags = FlagsState;
		}

		void SelectionControlsOSDLayer::ScaleSelection(bool Local)
		{
			if (ImGui::IsItemActive() == false)
				return;

			ImVec2 Delta = ImGui::GetIO().MouseDelta;
			_PRIMARYRENDERER->ScaleReferenceSelection(Delta.x, Local == false);
		}

		void SelectionControlsOSDLayer::AlignSelection(bool Position, bool Rotation)
		{
			TESObjectREFR* AlignRef = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, NULL, false);
			if (AlignRef)
			{
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					if (Ref == AlignRef)
						continue;

					if (Position)
						Ref->SetPosition(*AlignRef->GetPosition());

					if (Rotation)
						Ref->SetRotation(*AlignRef->GetRotation(), true);

					Ref->SetFromActiveFile(true);
				}

				NotificationOSDLayer::Instance.ShowNotification("Selection %s aligned to %08X", Position ? "position" : "roatation", AlignRef->formID);
				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_RefAlignment);
			}
		}

		SelectionControlsOSDLayer::SelectionControlsOSDLayer() :
			IRenderWindowOSDLayer(settings::renderWindowOSD::kShowSelectionControls, IRenderWindowOSDLayer::kPriority_SelectionControls)
		{
			TextInputActive = false;
			DragActive = false;
			LocalTransformation = 1;
		}

		SelectionControlsOSDLayer::~SelectionControlsOSDLayer()
		{
			;//
		}

		void SelectionControlsOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (_RENDERSEL->selectionCount == 0)
				return;

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, (_RENDERSEL->selectionCount == 0 ? 0.1f : 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
			ImGui::SetNextWindowPosCenter(ImGuiSetCond_FirstUseEver);
			if (!ImGui::Begin("Quick Controls", NULL, ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				ImGui::PopStyleVar(2);
				return;
			}

			bool SingleSel = _RENDERSEL->selectionCount == 1;
			bool GotInputFocus = false, LostInputFocus = false;
			bool DragBegin = false, DragEnd = false;

			CheckTextInputChange(GUI, GotInputFocus, LostInputFocus);
			CheckDragChange(GUI, DragBegin, DragEnd);

			if (GotInputFocus || DragBegin)
			{
				if (_RENDERSEL->selectionCount)
				{
					// recording just once crashes the editor. also, this code only seems to support undos
					// ### investigate
					_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_RefChange3D, _RENDERSEL->selectionList);
					_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_RefChange3D, _RENDERSEL->selectionList);
				}
			}

			if (SingleSel)
			{
				TESObjectREFR* ThisRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
				SME_ASSERT(ThisRef);

				Vector3 Position(ThisRef->position);
				Vector3 Rotation(ThisRef->rotation);
				float Scale = ThisRef->scale;
				Rotation.Scale(57.2957763671875);

				ImGui::PushStyleColor(ImGuiCol_Button, ImColor::HSV(0, 0.6f, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImColor::HSV(0, 0.7f, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImColor::HSV(0, 0.8f, 0.8f));
				{
					if (ImGui::Button("Edit Base Form"))
						EditBaseForm(ThisRef);
				}
				ImGui::PopStyleColor(3);

				ImGui::SameLine(ImGui::GetWindowWidth() - 40); ImGui::TextDisabled("(?)");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Click and drag the buttons to transform the selection.\nCTRL + click to directly edit the values.");

				ImGui::DragFloat3("Position##single_pos", &Position.x, 1, 0, 0, "%.2f");
				ImGui::DragFloat3("Rotation##single_rot", &Rotation.x, 1, 0, 0, "%.2f");
				ImGui::SliderFloat("Scale##single_scale", &Scale, 0.01f, 10.f);
				ImGui::NewLine();

				if (DragActive || TextInputActive || LostInputFocus || DragEnd)
				{
					ThisRef->SetPosition(Position);
					ThisRef->SetRotation(Rotation);
					ThisRef->SetScale(Scale);
				}

				if (LostInputFocus || DragEnd)
					ThisRef->SetFromActiveFile(true);
			}
			else if (_RENDERSEL->selectionCount)
			{
				ImGui::Text("Transformation:"); ImGui::SameLine(0, 15);
				ImGui::RadioButton("Local", &LocalTransformation, 1); ImGui::SameLine(); ImGui::RadioButton("Global", &LocalTransformation, 0);
				ImGui::SameLine(ImGui::GetWindowWidth() - 40); ImGui::TextDisabled("(?)");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Click and drag the buttons to transform the selection");

				float Width = 0;
				ImGui::Columns(4, "Position + Rotation", false);
				{
					ImGui::Button("X##multi_pos_x", ImVec2(ImGui::GetColumnWidth(), 20)); MoveSelection(true, false, false); DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Y##multi_pos_y", ImVec2(ImGui::GetColumnWidth(), 20)); MoveSelection(false, true, false); DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Z##multi_pos_z", ImVec2(ImGui::GetColumnWidth(), 20)); MoveSelection(false, false, true); DrawDragTrail(); ImGui::NextColumn();

					ImGui::Text("Position"); ImGui::NextColumn();

					ImGui::Button("X##multi_rot_x", ImVec2(ImGui::GetColumnWidth(), 20)); RotateSelection(LocalTransformation, true, false, false);
					DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Y##multi_rot_y", ImVec2(ImGui::GetColumnWidth(), 20)); RotateSelection(LocalTransformation, false, true, false);
					DrawDragTrail(); ImGui::NextColumn();
					ImGui::Button("Z##multi_rot_z", ImVec2(ImGui::GetColumnWidth(), 20)); RotateSelection(LocalTransformation, false, false, true);
					DrawDragTrail(); ImGui::NextColumn();

					Width = ImGui::GetColumnOffset();
					ImGui::Text("Rotation"); ImGui::NextColumn();
				}
				ImGui::Columns(1);

				ImGui::Button("Scale##multi_scale", ImVec2(Width - 10, 20)); ScaleSelection(LocalTransformation);
				ImGui::NewLine();

				ImGui::Columns(3, "Grouping", false);
				{
					ImGui::Text("Grouping:"); ImGui::NextColumn();
					if (ImGui::Button("Group##group_sel", ImVec2(ImGui::GetColumnWidth(), 20)))
						_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_GROUP);
					ImGui::NextColumn();
					if (ImGui::Button("Ungroup##ungroup_sel", ImVec2(ImGui::GetColumnWidth(), 20)))
						_RENDERWIN_MGR.InvokeContextMenuTool(IDC_RENDERWINDOWCONTEXT_UNGROUP);
					ImGui::NextColumn();
				}
				ImGui::Columns(1);
			}

			ImGui::Columns(3, "Alignment", false);
			{
				ImGui::Text("Alignment: "); ImGui::NextColumn();
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Aligns the selection to the position/rotation of another reference");
				if (ImGui::Button("Postion##align_pos", ImVec2(ImGui::GetColumnWidth(), 20)))
					AlignSelection(true, false);
				ImGui::NextColumn();
				if (ImGui::Button("Rotation##align_rot", ImVec2(ImGui::GetColumnWidth(), 20)))
					AlignSelection(false, true);
				ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::Separator();

			ImGui::Columns(4, "Flags", false);
			{
				ImGui::Text("Flags: "); ImGui::NextColumn();
				if (SingleSel)
				{
					TESObjectREFR* ThisRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
					SME_ASSERT(ThisRef);

					bool Persistent = ThisRef->IsQuestItem();
					bool VWD = ThisRef->IsVWD();
					bool Disabled = ThisRef->IsInitiallyDisabled();
					bool Changed = false;

					if (ImGui::Checkbox("Persistent##flag_persistent", &Persistent))
						ThisRef->SetPersistent(Persistent), Changed = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("VWD##flag_vwd", &VWD))
						ThisRef->SetVWD(VWD), Changed = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("Disabled##flag_disabled", &Disabled))
						ThisRef->SetInitiallyDisabled(Disabled), Changed = true;
					ImGui::NextColumn();

					if (Changed)
						ThisRef->SetFromActiveFile(true);
				}
				else
				{
					bool Persistent = true, VWD = true, Disabled = true;
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						if (Ref->IsQuestItem() == false)
							Persistent = false;
						if (Ref->IsVWD() == false)
							VWD = false;
						if (Ref->IsInitiallyDisabled() == false)
							Disabled = false;
					}

					bool SetPersistent = false, SetVWD = false, SetDisabled = false;
					if (ImGui::Checkbox("Persistent##flag_persistent", &Persistent))
						SetPersistent = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("VWD##flag_vwd", &VWD))
						SetVWD = true;
					ImGui::NextColumn();
					if (ImGui::Checkbox("Disabled##flag_disabled", &Disabled))
						SetDisabled = true;
					ImGui::NextColumn();

					if (SetPersistent || SetDisabled || SetVWD)
					{
						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* ThisRef = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
							if (SetPersistent)
								ThisRef->SetPersistent(Persistent);
							else if (SetVWD)
								ThisRef->SetVWD(VWD);
							else if (SetDisabled)
								ThisRef->SetInitiallyDisabled(Disabled);

							ThisRef->SetFromActiveFile(true);
						}
					}
				}
			}
			ImGui::Columns(1);

			if (ImGui::CollapsingHeader("Enable State Parent##linkedref_header"))
			{
				bool Modified = false;
				if (SingleSel)
				{
					TESObjectREFR* ThisRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
					SME_ASSERT(ThisRef);

					BSExtraData* xData = ThisRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
					ExtraEnableStateParent* xParent = NULL;
					if (xData)
						xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);

					ImGui::Columns(3, "Enable State Parent##linkedref_cols_single", false);
					{
						ImGui::Text("Linked Ref:"); ImGui::NextColumn();
						if (xParent)
						{
							ImGui::Text("%s(%08X)",
								((xParent->parent->editorID.Size()) ? (xParent->parent->editorID.c_str()) : ("")), xParent->parent->formID);
						}
						else
							ImGui::Text("None");
						ImGui::NextColumn();

						if (xParent)
							ImGui::Text("Opposite: %s   ", (xParent->oppositeState ? "Yes" : "No"));
						else if (ImGui::Button("Set", ImVec2(ImGui::GetColumnWidth(), 20)))
						{
							TESObjectREFR* NewParent = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, xParent ? xParent->parent : NULL, true);
							if (NewParent)
								ThisRef->extraData.ModExtraEnableStateParent(NewParent), Modified = true;
						}
						ImGui::NextColumn();

						if (xParent && ImGui::Button("Change", ImVec2(ImGui::GetColumnWidth(), 20)))
						{
							TESObjectREFR* NewParent = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, xParent ? xParent->parent : NULL, true);
							if (NewParent)
								ThisRef->extraData.ModExtraEnableStateParent(NewParent), Modified = true;
						}
						ImGui::NextColumn();

						if (xParent)
						{
							if (ImGui::Button("Clear", ImVec2(ImGui::GetColumnWidth(), 20)))
								ThisRef->extraData.ModExtraEnableStateParent(NULL), Modified = true;
						}
						ImGui::NextColumn();

						if (xParent)
						{
							if (ImGui::Button("Toggle Opposite", ImVec2(ImGui::GetColumnWidth(), 20)))
								ThisRef->SetExtraEnableStateParentOppositeState(xParent->oppositeState == 0), Modified = true;
						}
						ImGui::NextColumn();
					}
					ImGui::Columns(1);

					if (Modified)
						ThisRef->SetFromActiveFile(true);
				}
				else
				{
					ImGui::Columns(4, "Enable State Parent##linkedref_cols_multi", false);
					{
						bool SameParent = true;
						TESObjectREFR* ParentRefMark = NULL;
						bool OppositeStateMark = false;
						bool MarksSet = false;

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* ParentComp = NULL;
							bool OppositeComp = false;

							TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
							BSExtraData* xData = Ref->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
							if (xData)
							{
								ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
								ParentComp = xParent->parent;
								OppositeComp = xParent->oppositeState == 1;
							}

							if (MarksSet == false)
							{
								ParentRefMark = ParentComp;
								OppositeStateMark = OppositeComp;
								MarksSet = true;
							}

							if (ParentRefMark != ParentComp || OppositeStateMark != OppositeComp)
							{
								SameParent = false;
								break;
							}
						}

						ImGui::Text("Linked Ref:"); ImGui::NextColumn();
						if (SameParent && ParentRefMark)
						{
							ImGui::Text("%s(%08X)",
								((ParentRefMark->editorID.Size()) ? (ParentRefMark->editorID.c_str()) : ("")), ParentRefMark->formID);
						}
						else if (SameParent && ParentRefMark == NULL)
							ImGui::Text("None");
						else if (SameParent == false)
							ImGui::Text("(multiple)");
						ImGui::NextColumn();
						if (SameParent)
							ImGui::Text("Opposite: %s   ", (OppositeStateMark ? "Yes" : "No"));
						ImGui::NextColumn();
						ImGui::NextColumn();

						char Buffer[0x100] = { 0 };
						FORMAT_STR(Buffer, "%s", (SameParent && ParentRefMark ? "Change" : "Set"));
						TESObjectREFR* NewParent = NULL;
						bool ClearParent = false;
						bool ToggleOpposite = false;
						bool NewOpposite = false;

						if (ImGui::Button(Buffer, ImVec2(ImGui::GetColumnWidth(), 20)))
							NewParent = RefSelectControl::ShowSelectReferenceDialog(*TESRenderWindow::WindowHandle, SameParent ? ParentRefMark : NULL, true);
						ImGui::NextColumn();

						if (ImGui::Button("Clear", ImVec2(ImGui::GetColumnWidth(), 20)))
							ClearParent = true;
						ImGui::NextColumn();

						if (ImGui::Button("Set Opposite", ImVec2(ImGui::GetColumnWidth(), 20)))
							ToggleOpposite = NewOpposite = true;
						ImGui::NextColumn();

						if (ImGui::Button("Clear Opposite", ImVec2(ImGui::GetColumnWidth(), 20)))
							ToggleOpposite = true;
						ImGui::NextColumn();

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
							if (NewParent)
								Ref->extraData.ModExtraEnableStateParent(NewParent);

							if (ClearParent)
								Ref->extraData.ModExtraEnableStateParent(NULL);

							if (ToggleOpposite)
								Ref->SetExtraEnableStateParentOppositeState(NewOpposite);

							if (NewParent || ClearParent || ToggleOpposite)
								Ref->SetFromActiveFile(true);
						}
					}
					ImGui::Columns(1);
				}
			}


			ImGui::End();
			ImGui::PopStyleVar(2);
		}

		bool SelectionControlsOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}
	}
}