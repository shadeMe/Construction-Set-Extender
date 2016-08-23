#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class SelectionControlsOSDLayer : public IRenderWindowOSDLayer
		{

			int						LocalTransformation;
			bool					AlignmentAxisX;
			bool					AlignmentAxisY;
			bool					AlignmentAxisZ;

			void					EditReference(TESObjectREFR* Ref);
			void					EditBaseForm(TESObjectREFR* Ref);
			void					DrawDragTrail();

			void					MoveSelection(bool X, bool Y, bool Z);
			void					RotateSelection(bool Local, bool X, bool Y, bool Z);
			void					ScaleSelection(bool Local);
			void					AlignSelection(bool Position, bool Rotation);
		public:
			SelectionControlsOSDLayer();
			virtual ~SelectionControlsOSDLayer();

			virtual void			Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool			NeedsBackgroundUpdate();

			static SelectionControlsOSDLayer			Instance;
		};
	}
}