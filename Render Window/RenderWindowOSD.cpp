﻿#include "RenderWindowOSD.h"
#include "RenderWindowManager.h"
#include "IMGUI\imgui_internal.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"

#include "InfoOverlayOSDLayer.h"
#include "MouseOverTooltipOSDLayer.h"
#include "ToolbarOSDLayer.h"
#include "ReferenceEditorOSDLayer.h"
#include "WorkspaceManager.h"
#include "RenderWindowFlyCamera.h"

//#define OSD_LOAD_ALL_FONTS

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

#ifdef IMGUI_USE_BGRA_PACKED_COLOR
#define IMGUI_COL_TO_DX9_ARGB(_COL)     (_COL)
#else
#define IMGUI_COL_TO_DX9_ARGB(_COL)     (((_COL) & 0xFF00FF00) | (((_COL) & 0xFF0000) >> 16) | (((_COL) & 0xFF) << 16))
#endif
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
			if (d3d9_state_block->Capture() < 0)
			{
				d3d9_state_block->Release();
				return;
			}

			// Backup the DX9 transform (DX9 documentation suggests that it is included in the StateBlock but it doesn't appear to)
			D3DMATRIX last_world, last_view, last_projection;
			Impl->D3DDevice->GetTransform(D3DTS_WORLD, &last_world);
			Impl->D3DDevice->GetTransform(D3DTS_VIEW, &last_view);
			Impl->D3DDevice->GetTransform(D3DTS_PROJECTION, &last_projection);

			// Allocate buffers
			CUSTOMVERTEX* vtx_dst;
			ImDrawIdx* idx_dst;
			if (Impl->VertexBuffer->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
			{
				d3d9_state_block->Release();
				return;
			}
			if (Impl->IndexBuffer->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
			{
				Impl->VertexBuffer->Unlock();
				d3d9_state_block->Release();
				return;
			}

			// Copy and convert all vertices into a single contiguous buffer, convert colors to DX9 default format.
			// FIXME-OPT: This is a minor waste of resource, the ideal is to use imconfig.h and
			//  1) to avoid repacking colors:   #define IMGUI_USE_BGRA_PACKED_COLOR
			//  2) to avoid repacking vertices: #define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct ImDrawVert { ImVec2 pos; float z; ImU32 col; ImVec2 uv; }
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
				for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
				{
					vtx_dst->pos[0] = vtx_src->pos.x;
					vtx_dst->pos[1] = vtx_src->pos.y;
					vtx_dst->pos[2] = 0.0f;
					vtx_dst->col = IMGUI_COL_TO_DX9_ARGB(vtx_src->col);
					vtx_dst->uv[0] = vtx_src->uv.x;
					vtx_dst->uv[1] = vtx_src->uv.y;
					vtx_dst++;
					vtx_src++;
				}
				memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				idx_dst += cmd_list->IdxBuffer.Size;
			}
			Impl->VertexBuffer->Unlock();
			Impl->IndexBuffer->Unlock();
			Impl->D3DDevice->SetStreamSource(0, Impl->VertexBuffer, 0, sizeof(CUSTOMVERTEX));
			Impl->D3DDevice->SetIndices(Impl->IndexBuffer);
			Impl->D3DDevice->SetFVF(D3DFVF_CUSTOMVERTEX);


			// Setup render state: fixed-pipeline
			ImGuiDX9::SetupRenderState(Impl->D3DDevice, draw_data);

			// Render command lists
			// (Because we merged all buffers into a single one, we maintain our own offset into them)
			int global_vtx_offset = 0;
			int global_idx_offset = 0;
			ImVec2 clip_off = draw_data->DisplayPos;
			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
					if (pcmd->UserCallback != nullptr)
					{
						// User callback, registered via ImDrawList::AddCallback()
						// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
						if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
							ImGuiDX9::SetupRenderState(Impl->D3DDevice, draw_data);
						else
							pcmd->UserCallback(cmd_list, pcmd);
					}
					else
					{
						// Project scissor/clipping rectangles into framebuffer space
						ImVec2 clip_min(pcmd->ClipRect.x - clip_off.x, pcmd->ClipRect.y - clip_off.y);
						ImVec2 clip_max(pcmd->ClipRect.z - clip_off.x, pcmd->ClipRect.w - clip_off.y);
						if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
							continue;

						// Apply Scissor/clipping rectangle, Bind texture, Draw
						const RECT r = { (LONG)clip_min.x, (LONG)clip_min.y, (LONG)clip_max.x, (LONG)clip_max.y };
						const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->GetTexID();
						Impl->D3DDevice->SetTexture(0, texture);
						Impl->D3DDevice->SetScissorRect(&r);
						Impl->D3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pcmd->VtxOffset + global_vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, pcmd->IdxOffset + global_idx_offset, pcmd->ElemCount / 3);
					}
				}
				global_idx_offset += cmd_list->IdxBuffer.Size;
				global_vtx_offset += cmd_list->VtxBuffer.Size;
			}

			// Restore the DX9 transform
			Impl->D3DDevice->SetTransform(D3DTS_WORLD, &last_world);
			Impl->D3DDevice->SetTransform(D3DTS_VIEW, &last_view);
			Impl->D3DDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

			// Restore the DX9 state
			d3d9_state_block->Apply();
			d3d9_state_block->Release();
		}

		void ImGuiDX9::SetupRenderState(IDirect3DDevice9* device, ImDrawData* draw_data)
		{
			// Setup viewport
			D3DVIEWPORT9 vp;
			vp.X = vp.Y = 0;
			vp.Width = (DWORD)draw_data->DisplaySize.x;
			vp.Height = (DWORD)draw_data->DisplaySize.y;
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			device->SetViewport(&vp);

			// Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient), bilinear sampling.
			device->SetPixelShader(nullptr);
			device->SetVertexShader(nullptr);
			device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
			device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
			device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			device->SetRenderState(D3DRS_ZENABLE, FALSE);
			device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
			device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			device->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
			device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
			device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
			device->SetRenderState(D3DRS_FOGENABLE, FALSE);
			device->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
			device->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
			device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
			device->SetRenderState(D3DRS_CLIPPING, TRUE);
			device->SetRenderState(D3DRS_LIGHTING, FALSE);
			device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
			device->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			device->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

			// Setup orthographic projection matrix
			// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
			// Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
			{
				float L = draw_data->DisplayPos.x + 0.5f;
				float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
				float T = draw_data->DisplayPos.y + 0.5f;
				float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
				D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
				D3DMATRIX mat_projection =
				{ { {
						2.0f/(R-L),   0.0f,         0.0f,  0.0f,
						0.0f,         2.0f/(T-B),   0.0f,  0.0f,
						0.0f,         0.0f,         0.5f,  0.0f,
						(L+R)/(L-R),  (T+B)/(B-T),  0.5f,  1.0f
					} } };
				device->SetTransform(D3DTS_WORLD, &mat_identity);
				device->SetTransform(D3DTS_VIEW, &mat_identity);
				device->SetTransform(D3DTS_PROJECTION, &mat_projection);
			}
		}

		void ImGuiDX9::AddKeyEvent(ImGuiKey key, bool down, int native_keycode, int native_scancode)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddKeyEvent(key, down);
			io.SetKeyEventNativeData(key, native_keycode, native_scancode); // To support legacy indexing (<1.87 user code)
			IM_UNUSED(native_scancode);
		}

		void ImGuiDX9::UpdateKeyModifiers()
		{
			ImGuiIO& io = ImGui::GetIO();
			io.AddKeyEvent(ImGuiMod_Ctrl, IsVkDown(VK_CONTROL));
			io.AddKeyEvent(ImGuiMod_Shift, IsVkDown(VK_SHIFT));
			io.AddKeyEvent(ImGuiMod_Alt, IsVkDown(VK_MENU));
			io.AddKeyEvent(ImGuiMod_Super, IsVkDown(VK_APPS));
		}

		bool ImGuiDX9::IsVkDown(int vk)
		{
			return (::GetKeyState(vk) & 0x8000) != 0;
		}


		bool ImGuiDX9::CreateFontsTexture()
		{
			// Build texture atlas
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->Clear();

			std::string FontPathRoot(BGSEEWORKSPACE->GetDefaultWorkspace());
			FontPathRoot.append("Data\\Fonts\\");

#ifdef OSD_LOAD_ALL_FONTS
			// load all fonts in the folder for prototyping
			// ### SLOW!
			std::string IconFontPath(FontPathRoot + std::string("MaterialIcons-Regular.ttf"));
			for (IDirectoryIterator Itr(FontPathRoot.c_str(), "*.ttf"); !Itr.Done(); Itr.Next())
			{
				std::string FileName(Itr.Get()->cFileName);
				FileName = FileName.substr(FileName.rfind("\\") + 1);

				std::string MainFontPath(FontPathRoot + "\\" + FileName);
				if (FileName.find("MaterialIcons-Regular") == -1)
					AddFontFromFile(MainFontPath.c_str(), IconFontPath.c_str(), ICON_MIN_MD, ICON_MAX_MD, 0.f, 4.5f);
			}
#else
			std::string MainFontPath(FontPathRoot + std::string(settings::renderWindowOSD::kFontFace().s));
			std::string IconFontPath(FontPathRoot + std::string("MaterialIcons-Regular.ttf"));

			AddFontFromFile(MainFontPath.c_str(), IconFontPath.c_str(), ICON_MIN_MD, ICON_MAX_MD, 0.f, 4.5f);
#endif
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
			io.Fonts->SetTexID((ImTextureID)FontTexture);

			return true;
		}

		void ImGuiDX9::AddFontFromFile(const char* FontPath, const char* IconFontPath, ImWchar IconRangeStart, ImWchar IconRangeEnd, float IconOffsetX, float IconOffsetY)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImFontConfig config;
			config.OversampleH = 8;
			config.OversampleV = 8;

			if (GetFileAttributes(FontPath) != INVALID_FILE_ATTRIBUTES)
				io.Fonts->AddFontFromFileTTF(FontPath, settings::renderWindowOSD::kFontSize().i, &config);
			else
				io.Fonts->AddFontDefault(&config);

			// merge icons
			static const ImWchar icons_ranges[] = { IconRangeStart, IconRangeEnd, 0 };
			ImFontConfig icons_config;
			icons_config.MergeMode = true;
			icons_config.OversampleH = 2;
			icons_config.OversampleV = 2;
			icons_config.PixelSnapH = true;
			icons_config.GlyphOffset.x += IconOffsetX;
			icons_config.GlyphOffset.y += IconOffsetY;
			if (GetFileAttributes(IconFontPath) != INVALID_FILE_ATTRIBUTES)
				io.Fonts->AddFontFromFileTTF(IconFontPath, settings::renderWindowOSD::kFontSize().i + 2, &icons_config, icons_ranges);
		}

		ImGuiDX9::ImGuiDX9()
		{
			VertexBuffer = nullptr;
			IndexBuffer = nullptr;
			VertexBufferSize = 5000;
			IndexBufferSize = 10000;
			FontTexture = nullptr;
			Time = 0;
			TicksPerSecond = 0;
			KeyboardCodePage = 0;
			RenderWindowHandle = nullptr;
			D3DDevice = nullptr;
			ConsumeNextMouseRButtonDown = false;
			CurrentMouseCoord.x = CurrentMouseCoord.y = 0;
			MouseDownCursorPos.x = MouseDownCursorPos.y = 0;
			Initialized = false;
		}

		ImGuiDX9::~ImGuiDX9()
		{
			Shutdown();
		}


		// https://github.com/ocornut/imgui/issues/707#issuecomment-430613104
		void CherryTheme()
		{
			// cherry colors, 3 intensities
#define HI(v)   ImVec4(0.502f, 0.075f, 0.256f, v)
#define MED(v)  ImVec4(0.455f, 0.198f, 0.301f, v)
#define LOW(v)  ImVec4(0.232f, 0.201f, 0.271f, v)
			// backgrounds (@todo: complete with BG_MED, BG_LOW)
#define BG(v)   ImVec4(0.200f, 0.220f, 0.270f, v)
			// text
#define TEXT_COL(v) ImVec4(0.860f, 0.930f, 0.890f, v)

			auto &style = ImGui::GetStyle();
			style.Colors[ImGuiCol_Text]                  = TEXT_COL(0.78f);
			style.Colors[ImGuiCol_TextDisabled]          = TEXT_COL(0.28f);
			style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.17f, 1.00f);
			style.Colors[ImGuiCol_ChildBg]				 = BG( 0.58f);
			style.Colors[ImGuiCol_PopupBg]               = BG( 0.9f);
			style.Colors[ImGuiCol_Border]                = ImVec4(0.31f, 0.31f, 1.00f, 0.00f);
			style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg]               = BG( 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered]        = MED( 0.78f);
			style.Colors[ImGuiCol_FrameBgActive]         = MED( 1.00f);
			style.Colors[ImGuiCol_TitleBg]               = LOW( 1.00f);
			style.Colors[ImGuiCol_TitleBgActive]         = HI( 1.00f);
			style.Colors[ImGuiCol_TitleBgCollapsed]      = BG( 0.75f);
			style.Colors[ImGuiCol_MenuBarBg]             = BG( 0.47f);
			style.Colors[ImGuiCol_ScrollbarBg]           = BG( 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered]  = MED( 0.78f);
			style.Colors[ImGuiCol_ScrollbarGrabActive]   = MED( 1.00f);
			style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
			style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
			style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.71f, 0.22f, 0.27f, 1.00f);
			style.Colors[ImGuiCol_Button]                = ImVec4(0.47f, 0.77f, 0.83f, 0.14f);
			style.Colors[ImGuiCol_ButtonHovered]         = MED( 0.86f);
			style.Colors[ImGuiCol_ButtonActive]          = MED( 1.00f);
			style.Colors[ImGuiCol_Header]                = MED( 0.76f);
			style.Colors[ImGuiCol_HeaderHovered]         = MED( 0.86f);
			style.Colors[ImGuiCol_HeaderActive]          = HI( 1.00f);
			style.Colors[ImGuiCol_Separator]             = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);
			style.Colors[ImGuiCol_SeparatorHovered]      = MED( 0.78f);
			style.Colors[ImGuiCol_SeparatorActive]       = MED( 1.00f);
			style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
			style.Colors[ImGuiCol_ResizeGripHovered]     = MED( 0.78f);
			style.Colors[ImGuiCol_ResizeGripActive]      = MED( 1.00f);
			style.Colors[ImGuiCol_PlotLines]             = TEXT_COL(0.63f);
			style.Colors[ImGuiCol_PlotLinesHovered]      = MED( 1.00f);
			style.Colors[ImGuiCol_PlotHistogram]         = TEXT_COL(0.63f);
			style.Colors[ImGuiCol_PlotHistogramHovered]  = MED( 1.00f);
			style.Colors[ImGuiCol_TextSelectedBg]        = MED( 0.43f);
			style.Colors[ImGuiCol_ModalWindowDimBg]		 = BG( 0.73f);
			style.Colors[ImGuiCol_Border] = ImVec4(0.539f, 0.479f, 0.255f, 0.162f);
		}

		// https://github.com/ocornut/imgui/issues/707#issuecomment-431702777
		void DarkTheme()
		{
			// Setup style
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.12f, 0.12f, 1.00f);
			colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.53f, 0.53f, 0.53f, 0.46f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.40f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 0.53f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.48f, 0.47f, 0.47f, 0.91f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.55f, 0.55f, 0.62f);
			colors[ImGuiCol_Button] = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.67f, 0.67f, 0.68f, 0.63f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.63f);
			colors[ImGuiCol_Header] = ImVec4(0.54f, 0.54f, 0.54f, 0.58f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.64f, 0.65f, 0.65f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
			colors[ImGuiCol_Separator] = ImVec4(0.58f, 0.58f, 0.58f, 0.50f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.87f, 0.87f, 0.87f, 0.53f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
			colors[ImGuiCol_Tab] = ImVec4(0.01f, 0.01f, 0.01f, 0.86f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
			colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 0.47f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		}

		// https://github.com/ocornut/imgui/issues/707#issuecomment-468798935
		void CorporateGreyTheme()
		{
			ImGuiStyle & style = ImGui::GetStyle();
			ImVec4 * colors = style.Colors;

			colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled]           = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
			colors[ImGuiCol_ChildBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
			colors[ImGuiCol_WindowBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
			colors[ImGuiCol_PopupBg]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
			colors[ImGuiCol_Border]                 = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
			colors[ImGuiCol_BorderShadow]           = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_FrameBg]                = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
			colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
			colors[ImGuiCol_FrameBgActive]          = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
			colors[ImGuiCol_TitleBg]                = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
			colors[ImGuiCol_TitleBgActive]          = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
			colors[ImGuiCol_MenuBarBg]              = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
			colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
			colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
			colors[ImGuiCol_CheckMark]              = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
			colors[ImGuiCol_SliderGrab]             = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
			colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
			colors[ImGuiCol_Button]                 = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
			colors[ImGuiCol_ButtonHovered]          = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
			colors[ImGuiCol_ButtonActive]           = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
			colors[ImGuiCol_Header]                 = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
			colors[ImGuiCol_HeaderHovered]          = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
			colors[ImGuiCol_HeaderActive]           = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
			colors[ImGuiCol_Separator]              = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
			colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
			colors[ImGuiCol_SeparatorActive]        = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
			colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
			colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
			colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
			colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
			colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		}

		// https://github.com/ocornut/imgui/issues/707#issuecomment-512669512
		void DarkBlueTheme()
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
			colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
			colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		}

		// https://github.com/ocornut/imgui/issues/707#issuecomment-678611331
		void AnotherDarkTheme()
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_Text]                  = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
			style.Colors[ImGuiCol_ChildBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
			style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
			style.Colors[ImGuiCol_Border]                = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
			style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
			style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
			style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
			style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
			style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
			style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
			style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
			style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
			style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);
			style.Colors[ImGuiCol_Button]                = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
			style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
			style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
			style.Colors[ImGuiCol_Header]                = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
			style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
			style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
			style.Colors[ImGuiCol_Separator]             = style.Colors[ImGuiCol_Border];
			style.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
			style.Colors[ImGuiCol_SeparatorActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
			style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
			style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
			style.Colors[ImGuiCol_Tab]                   = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
			style.Colors[ImGuiCol_TabHovered]            = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
			style.Colors[ImGuiCol_TabActive]             = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
			style.Colors[ImGuiCol_TabUnfocused]          = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
			style.Colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
			style.Colors[ImGuiCol_PlotLines]             = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
			style.Colors[ImGuiCol_DragDropTarget]        = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
			style.Colors[ImGuiCol_NavHighlight]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
			style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			style.Colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			style.Colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
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

			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO();
			io.MouseDoubleClickTime = 1.f;
			io.UserData = this;

			// set up window styles and colors
			ImGuiStyle& style = ImGui::GetStyle();

			style.WindowPadding = ImVec2(10, 10);
			style.FramePadding = ImVec2(10, 1);
			style.CellPadding = ImVec2(5, 1);
			style.ItemSpacing = ImVec2(5, 5);
			style.ItemInnerSpacing = ImVec2(10, 10);
			style.IndentSpacing = 10.0f;
			style.ScrollbarSize = 15.0f;
			style.GrabMinSize = 15.0f;

			style.WindowBorderSize = 1.0f;
			style.ChildBorderSize = 0.0f;
			style.PopupBorderSize = 1.0f;
			style.FrameBorderSize = 0.0f;
			style.TabBorderSize = 1.0f;

			style.WindowRounding = 2.0f;
			style.ChildRounding = 2.0f;
			style.PopupRounding = 1.0f;
			style.FrameRounding = 1.0f;
			style.ScrollbarRounding = 1.0f;
			style.GrabRounding = 2.0f;
			style.TabRounding = 2.0f;

			style.WindowTitleAlign = ImVec2(0.5, 0.5);
			style.WindowMenuButtonPosition = ImGuiDir_Right;
			style.ColorButtonPosition = ImGuiDir_Right;
			style.ButtonTextAlign = ImVec2(0.5, 0.5);

			// Retrieve keyboard code page, required for handling of non-Unicode Windows.
			HKL keyboard_layout = GetKeyboardLayout(0);
			LCID keyboard_lcid = MAKELCID(HIWORD(keyboard_layout), SORT_DEFAULT);
			if (GetLocaleInfoA(keyboard_lcid, (LOCALE_RETURN_NUMBER | LOCALE_IDEFAULTANSICODEPAGE), (LPSTR)&KeyboardCodePage, sizeof(KeyboardCodePage)) == 0)
				KeyboardCodePage = CP_ACP; // Fallback to default ANSI code page when fails.

			Initialized = true;
			return true;
		}

		void ImGuiDX9::Shutdown()
		{
			if (!Initialized)
				return;

			InvalidateDeviceObjects();
			ImGui::DestroyContext();
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
				OutNeedsMouse = false;
				break;
			case WM_CHAR:
				if (wParam > 0 && wParam < 0x10000)
					CharacterEvent = true;
			case WM_KEYDOWN:
			case WM_KEYUP:
				KeyboardEvent = true;
				OutNeedsKeyboard = false;
				break;
			}

			bool NeedsMouse = false, NeedsKeyboard = false, NeedsTextInput = false;
			NeedsInput(NeedsMouse, NeedsKeyboard, NeedsTextInput);


			if (MouseEvent)
			{
				OutNeedsMouse = NeedsMouse;
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

			// Start the frame
			ImGui::NewFrame();
		}

		void ImGuiDX9::Render()
		{
			ImGui::Render();
			RenderDrawLists(ImGui::GetDrawData());
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
			ImGui::ResetMouseKeys();
			ImGui::ResetKeyboardKeys();

			if (ConsumeNextRButtonDown)
				ConsumeNextMouseRButtonDown = true;
		}

		bool ImGuiDX9::UpdateInputState(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			ImGuiIO& io = ImGui::GetIO();

			switch (msg)
			{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);

				return true;
			case WM_LBUTTONUP:
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);

				return true;
			//case wm_lbuttondblclk:
			//	mousedoubleclicked[imguimousebutton_left] = true;

			//	return true;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONDBLCLK:
				// ### HACK kludge to workaround the out-of-order dispatching of the button 
				// down message when opening the context menu in the render window
				if (ConsumeNextMouseRButtonDown == false)
				{
					io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
					io.AddMouseButtonEvent(ImGuiMouseButton_Right, true);
				}
				else
					ConsumeNextMouseRButtonDown = false;

				return true;
			case WM_RBUTTONUP:
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(ImGuiMouseButton_Right, false);
				
				return true;
			//case WM_RBUTTONDBLCLK:
			//	MouseDoubleClicked[ImGuiMouseButton_Right] = true;

			//	return true;
			case WM_MBUTTONDOWN:
			case WM_MBUTTONDBLCLK:
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(ImGuiMouseButton_Middle, true);

				return true;
			case WM_MBUTTONUP:
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMouseButtonEvent(ImGuiMouseButton_Middle, false);

				return true;
			//case WM_MBUTTONDBLCLK:
			//	MouseDoubleClicked[ImGuiMouseButton_Middle] = true;

			//	return true;
			case WM_MOUSEWHEEL:
				io.AddMouseWheelEvent(0.0f, (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA);
				
				return true;
			case WM_MOUSEHWHEEL:
				io.AddMouseWheelEvent(-(float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA, 0.0f);

				return true;
			case WM_MOUSEMOVE:
			{
				POINT mouse_pos = { (LONG)GET_X_LPARAM(lParam), (LONG)GET_Y_LPARAM(lParam) };
				io.AddMouseSourceEvent(ImGuiMouseSource_Mouse);
				io.AddMousePosEvent((float)mouse_pos.x, (float)mouse_pos.y);
				
				return true;
			}
			case WM_KEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			{
				const bool is_key_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
				if (wParam < 256)
				{
					// Submit modifiers
					ImGuiDX9::UpdateKeyModifiers();

					// Obtain virtual key code
					// (keypad enter doesn't have its own... VK_RETURN with KF_EXTENDED flag means keypad enter, see IM_VK_KEYPAD_ENTER definition for details, it is mapped to ImGuiKey_KeyPadEnter.)
					int vk = (int)wParam;
					if ((wParam == VK_RETURN) && (HIWORD(lParam) & KF_EXTENDED))
						vk = IM_VK_KEYPAD_ENTER;
					const ImGuiKey key = ImGui::VirtualKeyToImGuiKey(vk);
					const int scancode = (int)LOBYTE(HIWORD(lParam));

					// Special behavior for VK_SNAPSHOT / ImGuiKey_PrintScreen as Windows doesn't emit the key down event.
					if (key == ImGuiKey_PrintScreen && !is_key_down)
						ImGuiDX9::AddKeyEvent(key, true, vk, scancode);

					// Submit key event
					if (key != ImGuiKey_None)
						ImGuiDX9::AddKeyEvent(key, is_key_down, vk, scancode);

					// Submit individual left/right modifier events
					if (vk == VK_SHIFT)
					{
						// Important: Shift keys tend to get stuck when pressed together, missing key-up events are corrected in ImGui_ImplWin32_ProcessKeyEventsWorkarounds()
						if (IsVkDown(VK_LSHIFT) == is_key_down) { ImGuiDX9::AddKeyEvent(ImGuiKey_LeftShift, is_key_down, VK_LSHIFT, scancode); }
						if (IsVkDown(VK_RSHIFT) == is_key_down) { ImGuiDX9::AddKeyEvent(ImGuiKey_RightShift, is_key_down, VK_RSHIFT, scancode); }
					}
					else if (vk == VK_CONTROL)
					{
						if (IsVkDown(VK_LCONTROL) == is_key_down) { ImGuiDX9::AddKeyEvent(ImGuiKey_LeftCtrl, is_key_down, VK_LCONTROL, scancode); }
						if (IsVkDown(VK_RCONTROL) == is_key_down) { ImGuiDX9::AddKeyEvent(ImGuiKey_RightCtrl, is_key_down, VK_RCONTROL, scancode); }
					}
					else if (vk == VK_MENU)
					{
						if (IsVkDown(VK_LMENU) == is_key_down) { ImGuiDX9::AddKeyEvent(ImGuiKey_LeftAlt, is_key_down, VK_LMENU, scancode); }
						if (IsVkDown(VK_RMENU) == is_key_down) { ImGuiDX9::AddKeyEvent(ImGuiKey_RightAlt, is_key_down, VK_RMENU, scancode); }
					}
				}
				return true;
			}
			case WM_CHAR:
				if (IsWindowUnicode(hWnd))
				{
					// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
					if (wParam > 0 && wParam < 0x10000)
						io.AddInputCharacterUTF16((unsigned short)wParam);
				}
				else
				{
					wchar_t wch = 0;
					MultiByteToWideChar(KeyboardCodePage, MB_PRECOMPOSED, (char*)&wParam, 1, &wch, 1);
					io.AddInputCharacter(wch);
				}

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


		bool ImGuiDX9::IsInitialized() const
		{
			return Initialized;
		}

		bool ImGuiDX9::IsDraggingWindow() const
		{
			if (Initialized == false)
				return false;

			ImGuiContext* RenderContext = ImGui::GetCurrentContext();
			if (RenderContext->MovingWindow)
				return true;
			else
				return false;
		}

		bool ImGuiDX9::IsPopupHovered() const
		{
			ImGuiContext& g = *GImGui;
			int popup_idx = g.BeginPopupStack.Size - 1;
			SME_ASSERT(popup_idx >= 0);
			if (popup_idx < 0 || popup_idx > g.OpenPopupStack.Size || g.BeginPopupStack[popup_idx].PopupId != g.OpenPopupStack[popup_idx].PopupId)
				return false;

			auto& Itr = g.OpenPopupStack[popup_idx];
			return g.HoveredWindow == Itr.Window;
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

		RenderWindowOSD::GUIState::GUIState()
		{
			RedrawSingleFrame = false;
			MouseInClientArea = false;
			ConsumeMouseInputEvents = ConsumeKeyboardInputEvents = false;
		}

		std::string RenderWindowOSD::GUIState::ToString() const
		{
			std::string Out;

			Out += "RedrawSingleFrame = " + std::to_string(RedrawSingleFrame) + "\n";
			Out += "MouseInClientArea = " + std::to_string(MouseInClientArea) + "\n";
			Out += "ConsumeMouseInputEvents = " + std::to_string(ConsumeMouseInputEvents) + "\n";
			Out += "ConsumeKeyboardInputEvents = " +  std::to_string(ConsumeKeyboardInputEvents);

			return Out;
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

		void RenderWindowOSD::LoadColorsFromINI() const
		{
			ImGuiStyle& Style = ImGui::GetStyle();
			for (int i = 0; i < ColorSettings.size(); ++i)
			{
				if (!StringToColor(ColorSettings[i]->GetData().s, Style.Colors[i]))
					BGSEECONSOLE_MESSAGE("Couldn't load OSD color parameter '%s' from INI", ColorSettings[i]->GetKey());
			}
		}

		void RenderWindowOSD::SaveColorsToINI() const
		{
			ImGuiStyle& Style = ImGui::GetStyle();
			for (int i = 0; i < ImGuiCol_COUNT; ++i)
			{
				auto& ColorValue = Style.Colors[i];
				ColorSettings[i]->SetString("%s", ColorToString(ColorValue).c_str());

			}
		}

		bool RenderWindowOSD::StringToColor(const char* ColorString, ImVec4& OutColor) const
		{
			ImVec4 Buffer;
			auto Result = sscanf_s(ColorString, "%f,%f,%f,%f", &Buffer.x, &Buffer.y, &Buffer.z, &Buffer.w);
			if (Result != 4)
			{
				BGSEECONSOLE_MESSAGE("Invalid OSD color parameter string '%s'", ColorString);
				return false;
			}

			OutColor = Buffer;
			return true;
		}

		std::string RenderWindowOSD::ColorToString(const ImVec4& Color) const
		{
			char Buffer[100];
			FORMAT_STR(Buffer, "%.2f,%.2f,%.2f,%.2f", Color.x, Color.y, Color.z, Color.w);
			return Buffer;
		}

		bool RenderWindowOSD::RenderModalEditColors(RenderWindowOSD*, ImGuiDX9*, void*)
		{
			static ImGuiTextFilter Filter;

			ImGui::ShowHelpPopup("In the color list:\n"
				"Left-click on colored square to open color picker,\n"
				"Right-click to open edit options menu.");
			ImGui::SameLine(0, 5);
			Filter.Draw("Filter", 350);

			bool Closed = false;
			ImGuiStyle& Style = ImGui::GetStyle();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 2));
			ImGui::BeginChild("##color_list_child_window", ImVec2(0, -25), true, ImGuiChildFlags_AlwaysUseWindowPadding);
			ImGui::PopStyleVar();
			{
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					auto ColorName = ImGui::GetStyleColorName(i);
					auto& ColorValue = Style.Colors[i];
					if (!Filter.PassFilter(ColorName))
						continue;

					ImGui::PushID(i);
					ImGui::ColorEdit4("##color", reinterpret_cast<float*>(&ColorValue), ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
					ImGui::SameLine(0.0f, Style.ItemInnerSpacing.x);
					ImGui::TextUnformatted(ColorName);
					ImGui::PopID();
				}
			}
			ImGui::EndChild();

			if (ImGui::Button("Close", ImVec2(75, 0)))
				Closed = true;

			if (Closed)
				Filter.Clear();

			return Closed;
		}

		RenderWindowOSD::RenderWindowOSD()
		{
			Pipeline = new ImGuiDX9();
			AttachedLayers.reserve(10);
			Initialized = false;
			RenderingLayers = false;
			PauseRendering = false;

			// must have the same order as ImGuiColor
			ColorSettings.push_back(&settings::renderWindowOSD::kColorText);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTextDisabled);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorWindowBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorChildBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorPopupBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorBorder);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorBorderShadow);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorFrameBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorFrameBgHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorFrameBgActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTitleBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTitleBgActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTitleBgCollapsed);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorMenuBarBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorScrollbarBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorScrollbarGrab);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorScrollbarGrabHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorScrollbarGrabActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorCheckMark);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorSliderGrab);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorSliderGrabActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorButton);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorButtonHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorButtonActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorHeader);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorHeaderHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorHeaderActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorSeparator);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorSeparatorHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorSeparatorActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorResizeGrip);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorResizeGripHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorResizeGripActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTab);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTabHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTabActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTabUnfocused);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTabUnfocusedActive);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorPlotLines);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorPlotLinesHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorPlotHistogram);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorPlotHistogramHovered);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTableHeaderBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTableBorderStrong);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTableBorderLight);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTableRowBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTableRowBgAlt);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorTextSelectedBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorDragDropTarget);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorNavHighlight);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorNavWindowingHighlight);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorNavWindowingDimBg);
			ColorSettings.push_back(&settings::renderWindowOSD::kColorModalWindowDimBg);
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

			AttachLayer(&ModalWindowProviderOSDLayer::Instance);
			AttachLayer(&InfoOverlayOSDLayer::Instance);
			AttachLayer(&MouseOverTooltipOSDLayer::Instance);
			AttachLayer(&NotificationOSDLayer::Instance);
			AttachLayer(&ToolbarOSDLayer::Instance);
			AttachLayer(&ReferenceEditorOSDLayer::Instance);
#ifndef NDEBUG
			AttachLayer(&DebugOSDLayer::Instance);
#endif

			LoadColorsFromINI();

			Initialized = true;

			return Initialized;
		}

		void RenderWindowOSD::Deinitialize()
		{
			SME_ASSERT(Initialized);

			DetachLayer(&ModalWindowProviderOSDLayer::Instance);
			DetachLayer(&InfoOverlayOSDLayer::Instance);
			DetachLayer(&MouseOverTooltipOSDLayer::Instance);
			DetachLayer(&NotificationOSDLayer::Instance);
			DetachLayer(&ToolbarOSDLayer::Instance);
			DetachLayer(&ReferenceEditorOSDLayer::Instance);
#ifndef NDEBUG
			DetachLayer(&DebugOSDLayer::Instance);
#endif

			SaveColorsToINI();

			Initialized = false;
		}

		bool RenderWindowOSD::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, RenderWindowManager* Manager)
		{
			if (!Initialized)
				return false;

			// do nothing if we're still rendering the previous frame or have paused rendering
			if (RenderingLayers || PauseRendering)
				return false;

			bool Handled = false;
			bool Update = false;

			// get input data and flag the viewport for update
			// don't update input if the mouse input manager has free movement
			if (Manager->GetMouseInputManager()->IsFreeMouseMovementActive() == false)
			{
				if (Pipeline->UpdateInputState(hWnd, uMsg, wParam, lParam))
				{
					if (GetCapture() != hWnd)
					{
						// consume all input if we have modal windows open
						if (ModalWindowProviderOSDLayer::Instance.HasOpenModals())
						{
							State.ConsumeMouseInputEvents = State.ConsumeKeyboardInputEvents = true;
							Handled = true;
						}
						else if (Pipeline->CanAllowInputEventPassthrough(uMsg, wParam, lParam,
																		 State.ConsumeMouseInputEvents,
																		 State.ConsumeKeyboardInputEvents) == false)
						{
							Handled = true;
						}
					}
				}
			}
			else
			{
				// since the mouse input manager processes its messages after the OSD, the latter will have
				// processed the mouse button down message before free movement was activated in the former
				// and since the deactivation of free movement once again happens inside the mouse input manager's handler,
				// the OSD will never receive the mouse button up message, leaving the input state indeterminate
				// we reset the input state to prevent this from happening
				Pipeline->ResetInputState(false);
			}

			switch (uMsg)
			{
			case WM_LBUTTONDBLCLK:
				if (NeedsInput(uMsg))
				{
					// preempt the vanilla handler
					Handled = true;
				}

				break;
			case WM_MOUSEMOVE:
			case WM_NCMOUSEMOVE:
				if (GetActiveWindow() == hWnd)
					State.MouseInClientArea = true;

				break;
			case WM_MOUSELEAVE:
			case WM_NCMOUSELEAVE:
				State.MouseInClientArea = false;

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
					State.MouseInClientArea = false;
				}

				break;
			case WM_TIMER:
				// main render loop
				if (wParam == TESRenderWindow::kTimer_ViewportUpdate && *TESRenderWindow::ActiveCell)
				{
					// refresh the viewport if any of the layers need a background update or if the mouse is in the client area
					if (State.MouseInClientArea || NeedsBackgroundUpdate())
					{
						Update = true;
						State.RedrawSingleFrame = true;
					}
					else if (State.RedrawSingleFrame)
					{
						Update = true;
						State.RedrawSingleFrame = false;
					}
				}

				break;
			}

			if (Handled == false)
				Handled = NeedsInput(uMsg);

			if (Handled)
				Update = true;

			if (Update)
			{
				if (Manager->GetMouseInputManager()->IsTransformingSelection() == false)
					TESRenderWindow::Redraw();
			}

			return Handled;
		}

		void RenderWindowOSD::Draw()
		{
			if (!Initialized)
				return;
			else if (RenderWindowFlyCamera::Instance.IsActive())
				return;
			else if (!TESRenderWindow::IsAnyCellLoaded())
				return;
			else if (PauseRendering)
				return;

			Pipeline->NewFrame();
			RenderLayers();
		}

		void RenderWindowOSD::Render()
		{
			if (!Initialized)
				return;
			else if (RenderWindowFlyCamera::Instance.IsActive())
				return;
			else if (!TESRenderWindow::IsAnyCellLoaded())
				return;
			else if (PauseRendering)
				return;
			else if (RenderingLayers)
				return;

			// defer the final render call until all layers are done drawing
			Pipeline->Render();
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
			return Initialized == false || State.ConsumeKeyboardInputEvents || State.ConsumeMouseInputEvents;
		}

		void RenderWindowOSD::ToggleRendering()
		{
			PauseRendering = PauseRendering == false;
		}

		void RenderWindowOSD::ShowColorThemeEditor()
		{
			ModalWindowProviderOSDLayer::Instance.ShowModal("Render Window OSD Color Theme Editor",
															&RenderWindowOSD::RenderModalEditColors,
															nullptr,
															0, ImVec2(450, 500), ImGuiCond_Once);
		}


		std::string IRenderWindowOSDLayer::Helpers::GetRefEditorID(TESObjectREFR* Ref)
		{
			SME_ASSERT(Ref && Ref->baseForm);

			const char* EditorID = Ref->GetEditorID();
			if (EditorID == nullptr)
				return GetFormEditorID(Ref->baseForm);

			return EditorID;
		}


		std::string IRenderWindowOSDLayer::Helpers::GetFormEditorID(TESForm* Form)
		{
			SME_ASSERT(Form);

			auto EditorID = Form->GetEditorID();
			if (EditorID == nullptr)
				EditorID = "<No-EditorID>";

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

			if (ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) == false)
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

			auto MouseDragging = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
			auto MouseHoveringWindow = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);

			if (GUI->IsDraggingWindow())
				return;
			else if (!MouseHoveringWindow && !Active)
				return;

			if (MouseDragging && ImGui::IsAnyItemActive())
			{
				if (Active == false)
				{
					Active = true;
					DragBegin = true;
				}
			}
			else if (!MouseDragging)
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
			// ### TODO this will break if the position of the data inside the vector changes
			// ### hash the name (it's supposed to be unique) and use it as the ID instead
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

				if (BeginHover)
					ImGui::OpenPopup(PopupStrID);

				if (ImGui::BeginPopup(PopupStrID, ImGuiWindowFlags_NoMove))
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

						bool NeedsKeyboard = false, Throwaway = false;
						GUI->NeedsInput(Throwaway, Throwaway, NeedsKeyboard);
						if (Hovering == false &&
							(GUI->IsPopupHovered() || GUI->HasRootWindow(GUI->GetHoveredWindow(), PopupID) || NeedsKeyboard))
						{
							// reset the timeout/prevent ticking every frame when the mouse is hovering over the popup or its children or has keyboard input
							PreventActivePopupTicking = true;
							ActivePopupTimeout = kTimeout;
						}
					}

					ImGui::EndPopup();
				}
				else
				{
					// popup was closed through some other event, cleanup
					CloseActivePopup = false;
					ActivePopupTimeout = 0.f;
					ActivePopup = kInvalidID;

					if (PopupData.PositionType != kPosition_Default)
						io.MousePos = MousPosBuffer;
				}
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

			ImGui::SetNextWindowPos(ImVec2(10, *TESRenderWindow::ScreenHeight - 150));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
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

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3, 3));
			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(2, 2));
			ImGui::Dummy(ImVec2(1, 5));
			ImGui::Text("  %s  ", Current.Message.c_str());
			ImGui::Dummy(ImVec2(10, 15));
			ImGui::PopStyleVar(2);
			ImGui::ProgressBar(RemainingTime, ImVec2(-1, 3));
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
			vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Format, Args);
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
			ImGui::ShowDemoWindow();
		}

		bool DebugOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}


		ModalWindowProviderOSDLayer			ModalWindowProviderOSDLayer::Instance;

		ModalWindowProviderOSDLayer::ModalData::ModalData(const char* Name, ModalRenderDelegateT&& Delegate,
														  void* UserData, ImGuiWindowFlags Flags, const ImVec2& Size, ImGuiCond SizeCond) :
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
			if (OpenModals.empty())
				return;

			// only renders one modal at a time (the topmost)
			ModalData& Top = OpenModals.top();
			if (Top.Open == false)
			{
				ImGui::OpenPopup(Top.WindowName.c_str());
				Top.Open = true;
			}

			if (Top.HasCustomSize)
				ImGui::SetNextWindowSize(Top.WindowSize, Top.SizeSetCondition);

			ImGui::SetNextWindowPosCenter(ImGuiCond_Appearing);
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

		bool ModalWindowProviderOSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		void ModalWindowProviderOSDLayer::ShowModal(const char* Name, ModalRenderDelegateT Delegate, void* UserData,
													ImGuiWindowFlags Flags, const ImVec2& Size, ImGuiCond SizeCond)
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

			OpenModals.emplace(Name, std::move(Delegate), UserData, Flags, Size, SizeCond);
		}

		bool ModalWindowProviderOSDLayer::HasOpenModals() const
		{
			return OpenModals.size() != 0;
		}


	}
}