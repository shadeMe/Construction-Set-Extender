#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class InfoOverlayOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			InfoOverlayOSDLayer();
			virtual ~InfoOverlayOSDLayer();

			virtual void						Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool						NeedsBackgroundUpdate();

			static InfoOverlayOSDLayer		Instance;
		};
	}
}