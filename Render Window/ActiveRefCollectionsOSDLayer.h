#pragma once
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class ActiveRefCollectionsOSDLayer : public IRenderWindowOSDLayer
		{
			enum
			{
				kTab_Invisible = 0,
				kTab_Frozen,
				kTab_Groups,
				kTab_Layers,

				kTab__MAX
			};

			int								CurrentTab;
			ImGuiTextFilter					FilterHelper;

			void							RenderRefTableContents(int Tab);
			void							RenderTabContents(int Tab);

			struct EditGroupMembersData
			{
				std::string					GroupName;
				TESObjectREFRArrayT			MemberList;
			};

			static bool						RenderModalEditGroupMembers(RenderWindowOSD* OSD, ImGuiDX9* GUI, void* UserData);
		public:
			ActiveRefCollectionsOSDLayer();
			virtual ~ActiveRefCollectionsOSDLayer();

			virtual void								Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI);
			virtual bool								NeedsBackgroundUpdate();

			static ActiveRefCollectionsOSDLayer			Instance;
		};
	}
}