#include "RenderWindowOSD.h"
#include <bgsee\RenderWindowFlyCamera.h>

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
			io.MouseDoubleClickTime = 1.f;			// need to use a large value here as we aren't updating the GUI every tick
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
			{
				bool ObjectsCreated = CreateDeviceObjects();
				SME_ASSERT(ObjectsCreated);
			}

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

			// Start the frame
			ImGui::NewFrame();
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

			// don't update input state if the mouse is captured elsewhere
			if (GetCapture() != hWnd)
			{
				// get input data and flag the viewport for update
				if (Pipeline->UpdateInputState(hWnd, uMsg, wParam, lParam))
				{
					TESRenderWindow::Redraw();

					// check if the GUI needs input, skip the org wndproc if true
					// the check is performed on the previous frame's state but it works for our purposes
					if (Pipeline->NeedsInput())
						Return = true;
				}
			}


			switch (uMsg)
			{
			case WM_DESTROY:
				ExtraData->Remove(DialogExtraData::kTypeID);
				delete xData;

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
					Parent->GUILogic();
				}

				break;
			}

			return DlgProcResult;
		}


		void RenderWindowOSD::RenderSelectionControls()
		{
			if (_RENDERSEL->selectionCount == 0)
				return;

			ImGui::SetNextWindowPos(ImVec2(10, 300), ImGuiSetCond_FirstUseEver);
			if (!ImGui::Begin("Selection Controls", NULL, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				return;
			}

			ImGui::PushItemWidth(-10);
			ImGui::TextDisabled("(?)");
			ImGui::PopItemWidth();
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Help text placeholder");

			ImGui::Separator();
			ImGui::Button("Turn OFF Vorians");

			if (ImGui::CollapsingHeader("3D Data + Flags"))
			{
				ImGui::BeginChild("PosData", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 100), true);
				ImGui::TextDisabled("Position");
				ImGui::PushItemWidth(250);
				ImGui::Button("X");
				if (ImGui::IsItemActive())
				{
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					draw_list->PushClipRectFullScreen();
					draw_list->AddLine(ImGui::CalcItemRectClosestPoint(ImGui::GetIO().MousePos, true, -2.0f), ImGui::GetIO().MousePos, ImColor(ImGui::GetStyle().Colors[ImGuiCol_Button]), 4.0f);
					draw_list->PopClipRect();
					ImVec2 Delta = ImGui::GetIO().MouseDelta;
					_PRIMARYRENDERER->MoveReferenceSelection(Delta.x + Delta.y, Delta.y, true, false, false);
				}
				ImGui::SameLine();
				ImGui::Button("Y");
				if (ImGui::IsItemActive())
				{
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					draw_list->PushClipRectFullScreen();
					draw_list->AddLine(ImGui::CalcItemRectClosestPoint(ImGui::GetIO().MousePos, true, -2.0f), ImGui::GetIO().MousePos, ImColor(ImGui::GetStyle().Colors[ImGuiCol_Button]), 4.0f);
					draw_list->PopClipRect();
					ImVec2 Delta = ImGui::GetIO().MouseDelta;
					_PRIMARYRENDERER->MoveReferenceSelection(Delta.x + Delta.y, Delta.x + Delta.y, false, true, false);
				}
				ImGui::SameLine();
				ImGui::Button("Z");
				if (ImGui::IsItemActive())
				{
					ImDrawList* draw_list = ImGui::GetWindowDrawList();
					draw_list->PushClipRectFullScreen();
					draw_list->AddLine(ImGui::CalcItemRectClosestPoint(ImGui::GetIO().MousePos, true, -2.0f), ImGui::GetIO().MousePos, ImColor(ImGui::GetStyle().Colors[ImGuiCol_Button]), 4.0f);
					draw_list->PopClipRect();
					ImVec2 Delta = ImGui::GetIO().MouseDelta;
					_PRIMARYRENDERER->MoveReferenceSelection(Delta.x + Delta.y, Delta.y, false, false, true);
				}
				ImGui::PopItemWidth();
				ImGui::EndChild();
			}

			ImGui::End();
		}

		void RenderWindowOSD::GUILogic()
		{
			for each (auto Itr in AttachedLayers)
				Itr->Draw(this, Pipeline);

			RenderSelectionControls();

			ImGui::ShowTestWindow();
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

			Initialized = true;

			return Initialized;
		}

		void RenderWindowOSD::Deinitialize()
		{
			SME_ASSERT(Initialized);

			DetachLayer(&DefaultOverlayOSDLayer::Instance);
			DetachLayer(&MouseOverTooltipOSDLayer::Instance);
			DetachLayer(&NotificationOSDLayer::Instance);

			Initialized = false;
		}

		void RenderWindowOSD::Render()
		{
			if (Initialized)
			{
				ImGui::Render();
			}
		}

		void RenderWindowOSD::AttachLayer(IRenderWindowOSDLayer* Layer)
		{
			SME_ASSERT(Layer);

			if (std::find(AttachedLayers.begin(), AttachedLayers.end(), Layer) != AttachedLayers.end())
				BGSEECONSOLE_MESSAGE("Attempting to re-add the same OSD layer");
			else
				AttachedLayers.push_back(Layer);
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
				return Pipeline->NeedsInput();
		}

		DefaultOverlayOSDLayer		DefaultOverlayOSDLayer::Instance;

		DefaultOverlayOSDLayer::~DefaultOverlayOSDLayer()
		{
			;//
		}

		void DefaultOverlayOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			ImGui::SetNextWindowPos(ImVec2(10, 10));
			if (!ImGui::Begin("Default Info Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus))
			{
				ImGui::End();
				return;
			}

			TESObjectCELL* CurrentCell = _TES->currentInteriorCell;
			if (CurrentCell == NULL)
				CurrentCell = *TESRenderWindow::ActiveCell;

			if (CurrentCell)
			{
				char Buffer[0x50] = { 0 };
				if (CurrentCell->GetIsInterior())
					FORMAT_STR(Buffer, "%s (%08X)", CurrentCell->GetEditorID(), CurrentCell->formID);
				else
				{
					FORMAT_STR(Buffer, "%s %d,%d (%08X)", CurrentCell->GetEditorID(), CurrentCell->cellData.coords->x,
							   CurrentCell->cellData.coords->y, CurrentCell->formID);
				}

				ImGui::Text("Current Cell: %s", Buffer);
				ImGui::Separator();
			}

			if (settings::renderWindowOSD::kShowSelectionStats.GetData().i && _RENDERSEL->selectionCount)
			{
				int R, G, B;
				SME::StringHelpers::GetRGB(settings::renderWindowOSD::kColorSelectionStats().s, R, G, B);
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
					if (Selection)	// in the off-chance that the selection contains a regular form
					{
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

						FORMAT_STR(Buffer, "%s(%08X) Base Form: %s(%08X)\nPosition: %.04f, %.04f, %.04f\nRotation: %.04f, %.04f, %.04f\nScale: %.04f\nFlags: %s %s %s %s %s %s%s%s",
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

				}

				ImGui::TextColored(ImColor(R, G, B), "%s", Buffer);
				ImGui::Separator();
			}

			if (settings::renderWindowOSD::kShowRAMUsage.GetData().i)
			{
				int R, G, B;
				SME::StringHelpers::GetRGB(settings::renderWindowOSD::kColorRAMUsage().s, R, G, B);

				PROCESS_MEMORY_COUNTERS_EX MemCounter = { 0 };
				if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter)))
				{
					UInt32 CurrentRAMCounter = MemCounter.WorkingSetSize / (1024 * 1024);		// in megabytes
					ImGui::TextColored(ImColor(R, G, B), "RAM Usage: %d MB", CurrentRAMCounter);
				}
			}

			ImGui::Text("Avg. %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		bool DefaultOverlayOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		MouseOverTooltipOSDLayer	MouseOverTooltipOSDLayer::Instance;

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

			ImGui::SetNextWindowPos(ImVec2(10, *TESRenderWindow::ScreeHeight - 60));
			if (!ImGui::Begin("Notification Overlay", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
							  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				return;
			}

			int R, G, B;
			SME::StringHelpers::GetRGB(settings::renderWindowOSD::kColorNotifications().s, R, G, B);
			ImGui::TextColored(ImColor(R, G, B), "%s", GetNextNotification().Message.c_str());
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



	}
}