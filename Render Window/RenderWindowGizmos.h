#pragma once

#include "RenderWindowCommon.h"
#include "RenderWindowOSD.h"
#include "IMGUI\ImGuizmo.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowGizmoManager
		{
			class OSDLayer : public IRenderWindowOSDLayer
			{
				RenderWindowGizmoManager*	Parent;
			public:
				OSDLayer(RenderWindowGizmoManager* Parent);
				inline virtual ~OSDLayer() = default;

				virtual void				Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI) override;
				virtual bool				NeedsBackgroundUpdate() override;
			};

			bool						Active;
			ImGuizmo::OPERATION			Operation;
			ImGuizmo::MODE				Mode;
			OSDLayer					RenderLayer;
		public:
			RenderWindowGizmoManager();

			void						Initialize(RenderWindowOSD* OSD);
			void						Deinitialize(RenderWindowOSD* OSD);

			bool						IsActive() const;
		};
	}
}