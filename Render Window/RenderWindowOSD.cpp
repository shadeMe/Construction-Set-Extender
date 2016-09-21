#include "RenderWindowOSD.h"
#include "Render Window\RenderWindowManager.h"
#include "IMGUI\imgui_internal.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

#include "DefaultOverlayOSDLayer.h"
#include "MouseOverTooltipOSDLayer.h"
#include "ToolbarOSDLayer.h"
#include "SelectionControlsOSDLayer.h"

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
				if (Impl->VertexBuffer) { Impl->VertexBuffer->Release(); Impl->VertexBuffer = nullptr; }
				Impl->VertexBufferSize = draw_data->TotalVtxCount + 5000;
				if (Impl->D3DDevice->CreateVertexBuffer(Impl->VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &Impl->VertexBuffer, nullptr) < 0)
					return;
			}
			if (!Impl->IndexBuffer || Impl->IndexBufferSize < draw_data->TotalIdxCount)
			{
				if (Impl->IndexBuffer) { Impl->IndexBuffer->Release(); Impl->IndexBuffer = nullptr; }
				Impl->IndexBufferSize = draw_data->TotalIdxCount + 10000;
				if (Impl->D3DDevice->CreateIndexBuffer(Impl->IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &Impl->IndexBuffer, nullptr) < 0)
					return;
			}

			// Backup the DX9 state
			IDirect3DStateBlock9* d3d9_state_block = nullptr;
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
			Impl->D3DDevice->SetPixelShader(nullptr);
			Impl->D3DDevice->SetVertexShader(nullptr);
			Impl->D3DDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
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

			io.Fonts->Clear();

			std::string FontPathRoot(BGSEEWORKSPACE->GetDefaultWorkspace());
			FontPathRoot.append("Data\\Fonts\\");
			std::string MainFontPath(FontPathRoot + std::string(settings::renderWindowOSD::kFontFace().s));
			std::string IconFontPath(FontPathRoot + std::string("MaterialIcons-Regular.ttf"));

			if (GetFileAttributes(MainFontPath.c_str()) != INVALID_FILE_ATTRIBUTES)
				io.Fonts->AddFontFromFileTTF(MainFontPath.c_str(), settings::renderWindowOSD::kFontSize().i, &config);
			else
				io.Fonts->AddFontDefault(&config);

			// merge icons from MD
			static const ImWchar icons_ranges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.OversampleH = 2;
			icons_config.OversampleV = 2;
			icons_config.PixelSnapH = true;
			icons_config.MergeGlyphCenterV = true;
			if (GetFileAttributes(IconFontPath.c_str()) != INVALID_FILE_ATTRIBUTES)
				io.Fonts->AddFontFromFileTTF(IconFontPath.c_str(), settings::renderWindowOSD::kFontSize().i + 2, &icons_config, icons_ranges);

			unsigned char* pixels;
			int width, height, bytes_per_pixel;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

			// Upload texture to graphics system
			SAFERELEASE_D3D(FontTexture);
			if (D3DDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &FontTexture, nullptr) < 0)
				return false;
			D3DLOCKED_RECT tex_locked_rect;
			if (FontTexture->LockRect(0, &tex_locked_rect, nullptr, 0) != D3D_OK)
				return false;
			for (int y = 0; y < height; y++)
				memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
			FontTexture->UnlockRect(0);

			// Store our identifier
			io.Fonts->TexID = (void *)FontTexture;

			return true;
		}

		ImGuiDX9::ImGuiDX9() :
			PassthroughWhitelistMouseEvents()
		{
			VertexBuffer = nullptr;
			IndexBuffer = nullptr;
			VertexBufferSize = 5000;
			IndexBufferSize = 10000;
			FontTexture = nullptr;
			Time = 0;
			TicksPerSecond = 0;
			RenderWindowHandle = nullptr;
			D3DDevice = nullptr;
			PassthroughWhitelistMouseEvents.reserve(20);
			MouseDoubleClicked[0] = MouseDoubleClicked[1] = false;
			ConsumeNextMouseRButtonDown = false;
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

			// set up window styles and colors
			ImGuiStyle& style = ImGui::GetStyle();

			style.WindowPadding = ImVec2(10, 10);
			style.WindowRounding = 5.0f;
			style.ChildWindowRounding = 5.0f;
			style.FramePadding = ImVec2(5, 3);
			style.FrameRounding = 4.0f;
			style.ItemSpacing = ImVec2(12, 8);
			style.ItemInnerSpacing = ImVec2(8, 6);
			style.IndentSpacing = 25.0f;
			style.ScrollbarSize = 15.0f;
			style.ScrollbarRounding = 9.0f;
			style.GrabRounding = 3.0f;

			style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, settings::renderWindowOSD::kWindowBGAlpha().f);
			style.Colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
			style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.f);
			style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.90f, 0.80f, 0.80f, 0.49f);
			style.Colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.31f);
			style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
			style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.78f);
			style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.40f, 0.80f, 0.53f);
			style.Colors[ImGuiCol_Button] = ImVec4(0.35f, 0.55f, 0.61f, 0.51f);
			style.Colors[ImGuiCol_Header] = ImVec4(0.69f, 0.42f, 0.39f, 0.00f);
			style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.69f, 0.42f, 0.44f, 0.44f);

			Initialized = true;
			return true;
		}

		void ImGuiDX9::Shutdown()
		{
			InvalidateDeviceObjects();
			ImGui::Shutdown();
		}

		bool ImGuiDX9::IsActiveItemInWhitelist(const ImGuiWidgetIDArrayT& Whitelist) const
		{
			ImGuiID Active = ImGui::GetCurrentContext()->ActiveId;
			if (Active == NULL || std::find(PassthroughWhitelistMouseEvents.begin(), PassthroughWhitelistMouseEvents.end(), Active) == PassthroughWhitelistMouseEvents.end())
				return false;
			else
				return true;
		}

		bool ImGuiDX9::CanAllowInputEventPassthrough(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& OutNeedsMouse, bool& OutNeedsKeyboard) const
		{
			// check if the GUI needs input, skip the org wndproc if true
			// the check is performed on the previous frame's state but it works for our purposes
			bool MouseEvent = false, KeyboardEvent = false, CharacterEvent = false;
			switch (uMsg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MOUSEWHEEL:
			case WM_MOUSEMOVE:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
				MouseEvent = true;
				break;
			case WM_CHAR:
				if (wParam > 0 && wParam < 0x10000)
					CharacterEvent = true;
				break;
			case WM_KEYDOWN:
			case WM_KEYUP:
				KeyboardEvent = true;
				break;
			}

			bool NeedsMouse = false, NeedsKeyboard = false, NeedsTextInput = false;
			NeedsInput(NeedsMouse, NeedsKeyboard, NeedsTextInput);

			bool MouseWhitelisted = IsActiveItemInWhitelist(PassthroughWhitelistMouseEvents);
			OutNeedsMouse = OutNeedsKeyboard = false;

			if (MouseEvent)
			{
				OutNeedsMouse = NeedsMouse && MouseWhitelisted == false;
				return OutNeedsMouse == false;
			}
			else if (KeyboardEvent)
			{
				if (HasActiveItem() && (NeedsKeyboard || NeedsTextInput))
					OutNeedsKeyboard = true;

				return OutNeedsKeyboard == false;
			}
			else
				return true;
		}

		bool ImGuiDX9::HasActiveItem() const
		{
			ImGuiID Active = ImGui::GetCurrentContext()->ActiveId;
			return Active != NULL;
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

			// clear the input event whitelists every frame
			PassthroughWhitelistMouseEvents.clear();

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

			FontTexture = nullptr;
		}

		bool ImGuiDX9::CreateDeviceObjects()
		{
			if (!D3DDevice)
				return false;

			if (!CreateFontsTexture())
				return false;

			return true;
		}

		void ImGuiDX9::ResetInputState(bool ConsumeNextRButtonDown)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeyCtrl = false;
			io.KeyShift = false;
			io.KeyAlt = false;
			io.KeySuper = false;
			io.MouseDown[0] = io.MouseDown[1] = false;

			for (auto& Key : io.KeysDown)
				Key = false;

			if (ConsumeNextRButtonDown)
				ConsumeNextMouseRButtonDown = true;
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
				// ### kludge to workaround the out-of-order dispatching of the button down message when opening the context menu in the render window
				if (ConsumeNextMouseRButtonDown == false)
					io.MouseDown[1] = true;
				else
					ConsumeNextMouseRButtonDown = false;

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
			case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
				if (wParam < 256)
					io.KeysDown[wParam] = 1;
				return true;
			case WM_SYSKEYUP:
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

		void ImGuiDX9::NeedsInput(bool& OutNeedsMouse, bool& OutNeedsKeyboard, bool& OutNeedsTextInput) const
		{
			ImGuiIO& io = ImGui::GetIO();
			OutNeedsMouse = io.WantCaptureMouse;
			OutNeedsKeyboard = io.WantCaptureKeyboard;
			OutNeedsTextInput = io.WantTextInput;
		}

		void ImGuiDX9::WhitelistItemForMouseEvents()
		{
			ImGuiID LastItem = ImGui::GetCurrentWindow()->DC.LastItemId;
			if (std::find(PassthroughWhitelistMouseEvents.begin(), PassthroughWhitelistMouseEvents.end(), LastItem) == PassthroughWhitelistMouseEvents.end())
				PassthroughWhitelistMouseEvents.push_back(LastItem);
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

		bool ImGuiDX9::IsPopupHovered() const
		{
			ImGuiContext& g = *GImGui;
			int popup_idx = g.CurrentPopupStack.Size - 1;
			SME_ASSERT(popup_idx >= 0);
			if (popup_idx < 0 || popup_idx > g.OpenPopupStack.Size || g.CurrentPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
				return false;

			ImGuiPopupRef& Itr = g.OpenPopupStack[popup_idx];
			return g.HoveredWindow == Itr.Window;
		}

		bool ImGuiDX9::IsHoveringWindow() const
		{
			return ImGui::GetCurrentWindowRead() && ImGui::IsRootWindowOrAnyChildHovered();
		}

		bool ImGuiDX9::IsChildWindowHovering(void* RootWindow) const
		{
			ImGuiContext* RenderContext = ImGui::GetCurrentContext();
			ImGuiWindow* HoverWindow = RenderContext->HoveredWindow;

			while (HoverWindow && HoverWindow->RootWindow != HoverWindow)
			{
				if (HoverWindow->RootWindow == RootWindow)
					return true;
				else
					HoverWindow = HoverWindow->RootWindow;
			}

			return false;
		}

		bool ImGuiDX9::HasRootWindow(void* ChildWindow, void* RootWindow) const
		{
			ImGuiWindow* Child = (ImGuiWindow*)ChildWindow;
			while (Child && Child->ParentWindow)
			{
				if (Child->ParentWindow == RootWindow)
					return true;
				else
					Child = Child->ParentWindow;
			}

			return false;
		}

		void* ImGuiDX9::GetLastItemID() const
		{
			return (void*)ImGui::GetCurrentWindow()->DC.LastItemId;
		}

		void* ImGuiDX9::GetMouseHoverItemID() const
		{
			return (void*)ImGui::GetCurrentContext()->HoveredId;
		}

		void* ImGuiDX9::GetCurrentWindow() const
		{
			return ImGui::GetCurrentWindow();
		}

		void* ImGuiDX9::GetHoveredWindow() const
		{
			return ImGui::GetCurrentContext()->HoveredWindow;
		}

		void* ImGuiDX9::GetCurrentPopup() const
		{
			ImGuiContext& g = *GImGui;
			if (g.OpenPopupStack.empty())
				return nullptr;

			return g.OpenPopupStack.back().Window;
		}

		RenderWindowOSD::DialogExtraData::DialogExtraData(RenderWindowOSD* OSD) :
			bgsee::WindowExtraData(kTypeID),
			Parent(OSD)
		{
			SME_ASSERT(OSD);
		}

		RenderWindowOSD::DialogExtraData::~DialogExtraData()
		{
			Parent = nullptr;
		}

		RenderWindowOSD::GUIState::GUIState()
		{
			RedrawSingleFrame = false;
			MouseInClientArea = false;
			ConsumeMouseInputEvents = ConsumeKeyboardInputEvents = false;
			MouseHoveringOSD = false;
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
			if (xData == nullptr)
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
				Parent->State.ConsumeMouseInputEvents = Parent->State.ConsumeKeyboardInputEvents = false;
				Parent->State.MouseHoveringOSD = Pipeline->IsHoveringWindow();
				if (GetCapture() != hWnd && GetActiveWindow() == hWnd)
				{
					TESRenderWindow::Redraw();

					// consume all input if we have modal windows open
					if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
					{
						Parent->State.ConsumeMouseInputEvents = Parent->State.ConsumeKeyboardInputEvents = true;
						Return = true;
					}
					else if (Pipeline->CanAllowInputEventPassthrough(uMsg, wParam, lParam,
																Parent->State.ConsumeMouseInputEvents,
																Parent->State.ConsumeKeyboardInputEvents) == false)
					{
						Return = true;
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
				if (Parent->NeedsInput(uMsg))
				{
					// preempt the vanilla handler
					Return = true;
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
			case WM_UNINITMENUPOPUP:
				Pipeline->ResetInputState(true);

				break;
			case WM_KILLFOCUS:
				Pipeline->ResetInputState(false);

				break;
			case WM_NCACTIVATE:
				if (wParam == FALSE)
					Pipeline->ResetInputState(false);

				break;
			case WM_ACTIVATE:
				if (LOWORD(wParam) == WA_INACTIVE)
				{
					Pipeline->ResetInputState(false);
					Parent->State.MouseInClientArea = false;
				}

				break;
			case WM_TIMER:
				// main render loop
				if (wParam == TESRenderWindow::kTimer_ViewportUpdate && *TESRenderWindow::ActiveCell)
				{
					// refresh the viewport if the mouse is in the client area or if any of the layers need a background update
					if (Parent->State.MouseInClientArea || Parent->NeedsBackgroundUpdate())
					{
						TESRenderWindow::Redraw();
						Parent->State.RedrawSingleFrame = true;
					}
					else if (Parent->State.RedrawSingleFrame)
					{
						TESRenderWindow::Redraw();
						Parent->State.RedrawSingleFrame = false;
					}
				}

				break;
			}

			return DlgProcResult;
		}

		void RenderWindowOSD::RenderLayers()
		{
			SME::MiscGunk::ScopedSetter<bool> Sentry(RenderingLayers, true);
			for (auto Itr : AttachedLayers)
			{
				if (Itr->IsEnabled())
					Itr->Draw(this, Pipeline);
			}
		}

		bool RenderWindowOSD::NeedsBackgroundUpdate() const
		{
			for (auto Itr : AttachedLayers)
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

			AttachLayer(&ModalWindowProviderOSDLayer::Instance);
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

			DetachLayer(&ModalWindowProviderOSDLayer::Instance);
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

		void RenderWindowOSD::Draw()
		{
			if (Initialized &&
				bgsee::RenderWindowFlyCamera::IsActive() == false)
			{
				Pipeline->NewFrame();
				RenderLayers();
			}
		}

		void RenderWindowOSD::Render()
		{
			if (Initialized &&
				bgsee::RenderWindowFlyCamera::IsActive() == false )
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

		bool RenderWindowOSD::NeedsInput(UINT uMsg) const
		{
			if (Initialized == false)
				return false;
			else if (State.MouseHoveringOSD)
				return true;
			else switch (uMsg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
			case WM_MOUSEWHEEL:
			case WM_MOUSEMOVE:
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
				return State.ConsumeMouseInputEvents;
			case WM_CHAR:
			case WM_KEYDOWN:
			case WM_KEYUP:
				return State.ConsumeKeyboardInputEvents;
			default:
				return false;
			}
		}


		bool RenderWindowOSD::NeedsInput() const
		{
			return Initialized == false || State.MouseHoveringOSD || State.ConsumeKeyboardInputEvents || State.ConsumeMouseInputEvents;
		}

		std::string IRenderWindowOSDLayer::Helpers::GetRefEditorID(TESObjectREFR* Ref)
		{
			SME_ASSERT(Ref && Ref->baseForm);

			const char* EditorID = Ref->GetEditorID();
			if (EditorID == nullptr)
			{
				EditorID = Ref->baseForm->GetEditorID();

				if (EditorID == nullptr)
					return "<No-EditorID>";
				else
					return std::string(EditorID).append("*");
			}
			else
				return EditorID;
		}


		bool IRenderWindowOSDLayer::Helpers::ResolveReference(UInt32 FormID, TESObjectREFR*& OutRef)
		{
			TESForm* Form = TESForm::LookupByFormID(FormID);

			if (Form)
			{
				SME_ASSERT(Form->formType == TESForm::kFormType_ACHR || Form->formType == TESForm::kFormType_ACRE || Form->formType == TESForm::kFormType_REFR);

				OutRef = CS_CAST(Form, TESForm, TESObjectREFR);
				return (Form->IsDeleted() == false);
			}
			else
				return false;
		}

		IRenderWindowOSDLayer::IRenderWindowOSDLayer(const INISetting* Toggle) :
			Toggle(Toggle)
		{
			;//
		}

		bool IRenderWindowOSDLayer::IsEnabled() const
		{
			return Toggle == nullptr || Toggle->GetData().i == 1;
		}



		void OSDLayerStateData::TextInputData::Update(ImGuiDX9* GUI)
		{
			GotFocus = LostFocus = false;

			if (ImGui::IsMouseHoveringWindow() == false)
				return;

			ImGuiIO& io = ImGui::GetIO();
			if (io.WantTextInput)
			{
				if (Active == false)
				{
					Active = true;
					GotFocus = true;
				}
			}
			else if (Active)
			{
				Active = false;
				LostFocus = true;
			}
		}

		void OSDLayerStateData::DragInputData::Update(ImGuiDX9* GUI)
		{
			DragBegin = DragEnd = false;

			if (GUI->IsDraggingWindow())
				return;
			else if (ImGui::IsMouseHoveringWindow() == false && Active == false)
				return;

			if (ImGui::IsMouseDragging() && ImGui::IsAnyItemActive())
			{
				if (Active == false)
				{
					Active = true;
					DragBegin = true;
				}
			}
			else if (ImGui::IsMouseDragging() == false)
			{
				if (Active)
				{
					Active = false;
					DragEnd = true;
				}
			}
		}

		OSDLayerStateData::OSDLayerStateData()
		{
			TextInput.Active = TextInput.GotFocus = TextInput.LostFocus = false;
			DragInput.Active = DragInput.DragBegin = DragInput.DragEnd = false;
		}

		void OSDLayerStateData::Update(ImGuiDX9* GUI)
		{
			TextInput.Update(GUI);
			DragInput.Update(GUI);
		}


		MouseOverPopupProvider::PopupData::PopupData(const char* Name,
													 RenderDelegateT DrawButton, RenderDelegateT DrawPopup,
													 UInt8 PositionType, ImVec2& Pos) :
			PopupName(Name),
			ButtonHoverState(false),
			PopupState(),
			DrawButton(DrawButton),
			DrawPopup(DrawPopup),
			Position(Pos),
			PositionType(PositionType)
		{
			SME_ASSERT(Name && DrawButton && DrawPopup);

			if (PositionType == kPosition_Absolute)
				SME_ASSERT(Pos.x > -1 && Pos.y > -1);
		}


		void MouseOverPopupProvider::PopupData::CheckButtonHoverChange(ImGuiDX9* GUI, void* ParentWindow, bool& OutHovering, bool& OutBeginHover, bool& OutEndHover)
		{
			// assuming the last item was the popup's button
			if (ImGui::IsItemHoveredRect() && GUI->GetHoveredWindow() == ParentWindow)
			{
				OutHovering = true;
				if (ButtonHoverState == false)
				{
					ButtonHoverState = true;
					OutBeginHover = true;
				}
			}
			else
			{
				if (ButtonHoverState)
				{
					ButtonHoverState = false;
					OutEndHover = true;
				}
			}
		}

		MouseOverPopupProvider::MouseOverPopupProvider() :
			RegisteredPopups(),
			ActivePopup(kInvalidID),
			ActivePopupTimeout(0),
			CloseActivePopup(false),
			PreventActivePopupTicking(false)
		{
			;//
		}

		MouseOverPopupProvider::PopupIDT MouseOverPopupProvider::RegisterPopup(const char* Name,
																			   RenderDelegateT DrawButton, RenderDelegateT DrawPopup,
																			   UInt8 PositionType, ImVec2& Pos)
		{
			RegisteredPopups.push_back(PopupData(Name, DrawButton, DrawPopup, PositionType, Pos));
			return RegisteredPopups.size() - 1;
		}

		void MouseOverPopupProvider::Draw(PopupIDT ID, ImGuiDX9* GUI, void* ParentWindow)
		{
			SME_ASSERT(ID != kInvalidID);
			SME_ASSERT(GUI && ParentWindow);
			SME_ASSERT(ID < RegisteredPopups.size());

			PopupData& PopupData = RegisteredPopups.at(ID);
			bool Hovering = false, BeginHover = false, EndHover = false;
			const char* PopupStrID = PopupData.PopupName.c_str();

			// draw the button
			PopupData.DrawButton();

			// just render the button if there are open modals
			if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
				return;

			OSDLayerStateData& CurrentState = PopupData.PopupState;
			bool ActiveStateDragging = false;
			if (ActivePopup != kInvalidID)
			{
				OSDLayerStateData& ActiveState = RegisteredPopups.at(ActivePopup).PopupState;
				if (ActiveState.DragInput.Active)
					ActiveStateDragging = true;
			}

			PopupData.CheckButtonHoverChange(GUI, ParentWindow, Hovering, BeginHover, EndHover);
			if (BeginHover && ActiveStateDragging == false)
			{
				if (ActivePopup != kInvalidID && ActivePopup != ID)
				{
					// another popup is active, close it first
					CloseActivePopup = true;
					PopupData.ButtonHoverState = false;
					return;
				}

				ActivePopup = ID;
				ActivePopupTimeout = 0.f;
			}
			else if (EndHover && ActiveStateDragging == false)
			{
				ActivePopupTimeout = kTimeout;
			}

			if (ActivePopup == ID)
			{
				if (BeginHover)
					ImGui::OpenPopup(PopupStrID);

				// ### HACK HACK
				// manually update the mouse position to modify the popup's start pos
				// reset it after the window's created
				ImGuiIO& io = ImGui::GetIO();
				ImVec2 MousPosBuffer(io.MousePos);

				switch (PopupData.PositionType)
				{
				case kPosition_Absolute:
					io.MousePos = PopupData.Position;
					break;
				case kPosition_Relative:
					io.MousePos.x += PopupData.Position.x;
					io.MousePos.y += PopupData.Position.y;
					break;
				}

				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10);
				if (ImGui::BeginPopupWithStyling(PopupStrID, ImGuiWindowFlags_AlwaysAutoResize, ImGui::GetStyle().WindowRounding))
				{
					if (PopupData.PositionType != kPosition_Default)
						io.MousePos = MousPosBuffer;

					void* PopupID = GUI->GetCurrentWindow();
					CurrentState.Update(GUI);

					if (CloseActivePopup)
					{
						CloseActivePopup = false;
						ActivePopupTimeout = 0.f;
						ActivePopup = kInvalidID;
						ImGui::CloseCurrentPopup();
					}
					else
					{
						// render the contents of the current popup
						PopupData.DrawPopup();

						if (Hovering == false &&
							(GUI->IsPopupHovered() || GUI->HasRootWindow(GUI->GetHoveredWindow(), PopupID)))
						{
							// reset the timeout/prevent ticking every frame when the mouse is hovering over the popup or its children
							PreventActivePopupTicking = true;
							ActivePopupTimeout = kTimeout;
						}
					}

					ImGui::EndPopup();
				}
				ImGui::PopStyleVar();
			}
		}

		void MouseOverPopupProvider::Update()
		{
			if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
				return;
			else if (PreventActivePopupTicking)
			{
				PreventActivePopupTicking = false;
				return;
			}

			if (ActivePopup != kInvalidID && ActivePopupTimeout != 0.f && CloseActivePopup == false)
			{
				SME_ASSERT(ActivePopup < RegisteredPopups.size());

				OSDLayerStateData& ActiveState = RegisteredPopups.at(ActivePopup).PopupState;
				if (ActiveState.DragInput.Active == false)
				{
					ActivePopupTimeout -= ImGui::GetIO().DeltaTime;
					if (ActivePopupTimeout <= 0.f)
					{
						CloseActivePopup = true;
						ActivePopupTimeout = 0.f;
					}
				}
			}
		}

		NotificationOSDLayer		NotificationOSDLayer::Instance;

		NotificationOSDLayer::Notification::Notification(std::string Message, int Duration) :
			Message(Message),
			StartTickCount(0),
			Duration(Duration)
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

			if (GetTickCount64() - StartTickCount > Duration)
				return true;
			else
				return false;
		}

		ULONGLONG NotificationOSDLayer::Notification::GetRemainingTicks() const
		{
			return Duration - (GetTickCount64() - StartTickCount);
		}

		NotificationOSDLayer::NotificationOSDLayer() :
			IRenderWindowOSDLayer(&settings::renderWindowOSD::kShowNotifications),
			Queue()
		{
			;//
		}

		NotificationOSDLayer::~NotificationOSDLayer()
		{
			while (Queue.size())
				Queue.pop();
		}

		void NotificationOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (Tick() == false)
				return;

			ImGui::SetNextWindowPos(ImVec2(10, *TESRenderWindow::ScreeHeight - 150));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			if (!ImGui::Begin("Notification Overlay", nullptr,
							  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
							  ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
							  ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs))
			{
				ImGui::End();
				ImGui::PopStyleVar(4);
				return;
			}

			const Notification& Current = GetNextNotification();
			float RemainingTime = Current.GetRemainingTicks() / (float)Current.Duration;

			ImGui::Text("  %s  ", Current.Message.c_str());
			ImGui::Dummy(ImVec2(10, 15));
			ImGui::ProgressBar(RemainingTime, ImVec2(-1, 1));
			ImGui::End();
			ImGui::PopStyleVar(4);
		}

		bool NotificationOSDLayer::NeedsBackgroundUpdate()
		{
			return Tick();
		}

		bool NotificationOSDLayer::Tick()
		{
			while (Queue.size())
			{
				Notification& Next = Queue.front();
				if (Next.HasElapsed())
					Queue.pop();
				else
					return true;
			}

			return false;
		}

		const NotificationOSDLayer::Notification& NotificationOSDLayer::GetNextNotification() const
		{
			SME_ASSERT(Queue.size() > 0);

			return Queue.front();
		}

		void NotificationOSDLayer::ShowNotification(const char* Format, ...)
		{
			if (Format == nullptr)
				return;

			char Buffer[0x1000] = { 0 };
			va_list Args;
			va_start(Args, Format);
			vsprintf_s(Buffer, sizeof(Buffer), Format, Args);
			va_end(Args);

			if (strlen(Buffer))
				Queue.push(Notification(Buffer));
		}


		void NotificationOSDLayer::ClearNotificationQueue()
		{
			if (Queue.size() <= 1)
				return;

			Notification Current(Queue.front());

			while (Queue.size())
				Queue.pop();

			Queue.push(Current);
		}

		DebugOSDLayer			DebugOSDLayer::Instance;

		DebugOSDLayer::DebugOSDLayer() :
			IRenderWindowOSDLayer()
		{

		}

		DebugOSDLayer::~DebugOSDLayer()
		{
			;//
		}

		void DebugOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			ImGui::ShowTestWindow();
			ImGui::ShowMetricsWindow();
		}

		bool DebugOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}


		ModalWindowProviderOSDLayer			ModalWindowProviderOSDLayer::Instance;

		ModalWindowProviderOSDLayer::ModalData::ModalData(const char* Name, ModalRenderDelegateT Delegate,
														  void* UserData, ImGuiWindowFlags Flags, const ImVec2& Size, ImGuiSetCond SizeCond) :
			WindowName(Name),
			Delegate(Delegate),
			UserData(UserData),
			Flags(Flags),
			Open(false),
			WindowSize(Size),
			SizeSetCondition(SizeCond)
		{
			char Buffer[0x100] = {0};
			SME::MersenneTwister::init_genrand(GetTickCount());
			FORMAT_STR(Buffer, "##%d_%s", SME::MersenneTwister::genrand_int32(), Name);

			WindowName.append(Buffer);

			if (WindowSize.x != 0.f && WindowSize.y != 0.f)
				HasCustomSize = true;
			else
				HasCustomSize = false;
		}

		ModalWindowProviderOSDLayer::ModalWindowProviderOSDLayer() :
			IRenderWindowOSDLayer(),
			OpenModals()
		{
			;//
		}

		ModalWindowProviderOSDLayer::~ModalWindowProviderOSDLayer()
		{
			while (OpenModals.size())
			{
				OpenModals.pop();
				ImGui::CloseCurrentPopup();
			}
		}

		void ModalWindowProviderOSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (OpenModals.size())
			{
				// only renders one modal at a time (the topmost)
				ModalData& Top = OpenModals.top();
				if (Top.Open == false)
				{
					ImGui::OpenPopup(Top.WindowName.c_str());
					Top.Open = true;
				}

				if (Top.HasCustomSize)
					ImGui::SetNextWindowSize(Top.WindowSize, Top.SizeSetCondition);

				ImGui::SetNextWindowPosCenter(ImGuiSetCond_Once);
				if (ImGui::BeginPopupModal(Top.WindowName.c_str(), nullptr, Top.Flags | ImGuiWindowFlags_NoSavedSettings))
				{
					if (Top.Delegate(OSD, GUI, Top.UserData))
					{
						ImGui::CloseCurrentPopup();
						OpenModals.pop();
					}

					ImGui::EndPopup();
				}
			}
		}

		bool ModalWindowProviderOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		void ModalWindowProviderOSDLayer::ShowModal(const char* Name, ModalRenderDelegateT Delegate, void* UserData,
													ImGuiWindowFlags Flags, const ImVec2& Size, ImGuiSetCond SizeCond)
		{
			SME_ASSERT(Name && Delegate);
			SME_ASSERT(*TESRenderWindow::ActiveCell);		// ### modal doesn't show up when called when the scenegraph is empty

			// flag the current open modal, if any, as closed
			if (OpenModals.size())
			{
				ModalData& Top = OpenModals.top();
				if (Top.Open)
					Top.Open = false;
			}

			ModalData NewModal(Name, Delegate, UserData, Flags, Size, SizeCond);
			OpenModals.push(NewModal);
		}

		bool ModalWindowProviderOSDLayer::HasOpenModals() const
		{
			return OpenModals.size() != 0;
		}


	}
}