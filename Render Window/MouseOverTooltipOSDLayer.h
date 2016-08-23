#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class MouseOverTooltipOSDLayer : public IRenderWindowOSDLayer
		{
		public:
			MouseOverTooltipOSDLayer();
			virtual ~MouseOverTooltipOSDLayer();

			virtual void						Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool						NeedsBackgroundUpdate();

			static MouseOverTooltipOSDLayer		Instance;
		};
	}
}