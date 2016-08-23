#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class DefaultOverlayOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			DefaultOverlayOSDLayer();
			virtual ~DefaultOverlayOSDLayer();

			virtual void						Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool						NeedsBackgroundUpdate();

			static DefaultOverlayOSDLayer		Instance;
		};
	}
}