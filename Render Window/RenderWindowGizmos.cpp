#include "RenderWindowGizmos.h"

namespace cse
{
	namespace renderWindow
	{
		RenderWindowGizmoManager::OSDLayer::OSDLayer(RenderWindowGizmoManager* Parent) :
			Parent(Parent)
		{
		}

		void Frustum(float left, float right, float bottom, float top, float znear, float zfar, float *m16)
		{
			float temp, temp2, temp3, temp4;
			temp = 2.0f * znear;
			temp2 = right - left;
			temp3 = top - bottom;
			temp4 = zfar - znear;
			m16[0] = temp / temp2;
			m16[1] = 0.0;
			m16[2] = 0.0;
			m16[3] = 0.0;
			m16[4] = 0.0;
			m16[5] = temp / temp3;
			m16[6] = 0.0;
			m16[7] = 0.0;
			m16[8] = (right + left) / temp2;
			m16[9] = (top + bottom) / temp3;
			m16[10] = (-zfar - znear) / temp4;
			m16[11] = -1.0f;
			m16[12] = 0.0;
			m16[13] = 0.0;
			m16[14] = (-temp * zfar) / temp4;
			m16[15] = 0.0;
		}

		void Perspective(float fovyInDegrees, float aspectRatio, float znear, float zfar, float *m16)
		{
			float ymax, xmax;
			ymax = znear * tanf(fovyInDegrees * 3.141592f / 180.0f);
			xmax = ymax * aspectRatio;
			Frustum(-xmax, xmax, -ymax, ymax, znear, zfar, m16);
		}

		void RenderWindowGizmoManager::OSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			if (*TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0)
			{
				NiCamera* CameraRoot = _PRIMARYRENDERER->primaryCamera;
				D3DXMATRIX* ViewMatrix = (D3DXMATRIX*)&_NIRENDERER->viewMatrix;
				D3DXMATRIX* ProjMatrix = (D3DXMATRIX*)&_NIRENDERER->projMatrix;
				D3DXMATRIX ObjectMatrix, NewObject, NewView, TempMtx;
				CameraRoot->m_worldTransform.GetD3D(ObjectMatrix);

				D3DXMatrixLookAtLH(&NewView,
					(D3DXVECTOR3*)&CameraRoot->m_worldTransform,
								   (D3DXVECTOR3*)&CameraRoot->GetWorldDirection(),
								   (D3DXVECTOR3*)&CameraRoot->GetWorldUpVector());
// 				D3DXMatrixPerspectiveLH(&TempMtx,
// 										CameraRoot->m_kViewFrustum.r - CameraRoot->m_kViewFrustum.l,
// 										CameraRoot->m_kViewFrustum.b - CameraRoot->m_kViewFrustum.t,
// 										CameraRoot->m_kViewFrustum.n, CameraRoot->m_kViewFrustum.f);
		//		D3DXMatrixTranspose(&TempMtx, &NewView);
		//		memcpy(&NewView, TempMtx, sizeof(D3DXMATRIX));

		//		memcpy(&NewView, ViewMatrix, sizeof(D3DXMATRIX));
				D3DXMatrixRotationX(&NewObject, -1.57080);
				D3DXMatrixMultiply(&TempMtx, &NewView, &NewObject);
				memcpy(&NewView, TempMtx, sizeof(D3DXMATRIX));

		//		ViewMatrix = &NewView;
		//		ProjMatrix = &TempMtx;
				ImGuiIO& io = ImGui::GetIO();
				D3DXMATRIX NewProj;
				memcpy(&NewProj, ProjMatrix, sizeof(D3DXMATRIX));
				Perspective(90, io.DisplaySize.x / io.DisplaySize.y, CameraRoot->m_kViewFrustum.n, CameraRoot->m_kViewFrustum.f, (float*)&NewProj._11);

				TESObjectREFR* Selection = nullptr;
				if (_RENDERSEL->selectionCount)
				{
					Selection = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);
					if (Selection)
					{
						NiNode* RefNode = Selection->GetNiNode();
						RefNode->m_worldTransform.GetD3D(ObjectMatrix);
					};
				}

				if (ImGui::RadioButton("Translate", Parent->Operation == ImGuizmo::TRANSLATE))
					Parent->Operation = ImGuizmo::TRANSLATE;
				ImGui::SameLine();
				if (ImGui::RadioButton("Rotate", Parent->Operation == ImGuizmo::ROTATE))
					Parent->Operation = ImGuizmo::ROTATE;
				ImGui::SameLine();
				if (ImGui::RadioButton("Scale", Parent->Operation == ImGuizmo::SCALE))
					Parent->Operation = ImGuizmo::SCALE;

				float matrixTranslation[3], matrixRotation[3], matrixScale[3];
				ImGuizmo::DecomposeMatrixToComponents(&ObjectMatrix._11, matrixTranslation, matrixRotation, matrixScale);
				ImGui::InputFloat3("Tr", matrixTranslation);
				ImGui::InputFloat3("Rt", matrixRotation);
				ImGui::InputFloat3("Sc", matrixScale);
		//		ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, &ObjectMatrix._11);

				if (Parent->Operation != ImGuizmo::SCALE)
				{
					if (ImGui::RadioButton("Local", Parent->Mode == ImGuizmo::LOCAL))
						Parent->Mode = ImGuizmo::LOCAL;
					ImGui::SameLine();
					if (ImGui::RadioButton("World", Parent->Mode == ImGuizmo::WORLD))
						Parent->Mode = ImGuizmo::WORLD;
				}

				//ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
				ImGuizmo::Manipulate(&ViewMatrix->_11, &NewProj._11, Parent->Operation, Parent->Mode, &ObjectMatrix._11);


				if (Selection)
				{
					ImGuizmo::DecomposeMatrixToComponents(&ObjectMatrix._11, matrixTranslation, matrixRotation, matrixScale);

// 					Selection->SetPosition(matrixTranslation[0], matrixTranslation[1], matrixTranslation[2]);
// 					Selection->SetRotation(matrixRotation[0], matrixRotation[1], matrixRotation[2]);
// 					Selection->SetScale(matrixScale[0]);
				}
			}
		}

		bool RenderWindowGizmoManager::OSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}


		RenderWindowGizmoManager::RenderWindowGizmoManager() :
			Active(false),
			Operation(ImGuizmo::OPERATION::TRANSLATE),
			Mode(ImGuizmo::MODE::WORLD),
			RenderLayer(this)
		{
			;//
		}

		void RenderWindowGizmoManager::Initialize(RenderWindowOSD* OSD)
		{
#ifndef NDEBUG
			//OSD->AttachLayer(&RenderLayer);
#endif
		}

		void RenderWindowGizmoManager::Deinitialize(RenderWindowOSD* OSD)
		{
#ifndef NDEBUG
			//OSD->DetachLayer(&RenderLayer);
#endif
		}

		bool RenderWindowGizmoManager::IsActive() const
		{
			return Active;
		}
	}
}