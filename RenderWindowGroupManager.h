#pragma once

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowOSD;

		class RenderWindowGroupManager
		{
			typedef UInt32								ReferenceHandleT;
			typedef std::string							GroupIDT;
			typedef std::vector<ReferenceHandleT>		MemberRosterT;

			class GroupData
			{
				friend class RenderWindowGroupManager;

				GroupIDT								ID;
				MemberRosterT							Members;

				bool									GetIsMember(ReferenceHandleT Ref, MemberRosterT::iterator& Match);
			public:
				GroupData(GroupIDT ID, TESRenderSelection* Selection);

				UInt32									ValidateMembers(MemberRosterT& OutDelinquents);		// returns the new size
				void									AddMember(ReferenceHandleT Ref);
				void									RemoveMember(ReferenceHandleT Ref);
				void									ConvertToSelection(TESRenderSelection* Selection, bool ClearSelection);

				const GroupIDT							GetID(void) const { return ID; }
				const UInt32							GetSize(void) const { return Members.size(); }
			};

			typedef boost::shared_ptr<GroupData>						GroupDataHandleT;
			typedef std::map<ReferenceHandleT, GroupDataHandleT>		GroupDataReferenceMapT;
			typedef std::vector<GroupDataHandleT>						GroupDataArrayT;

			static bool									GetReferenceExists(ReferenceHandleT Ref);
			bool										GetGroupExists(GroupIDT ID);
			GroupDataHandleT							LookupGroup(GroupIDT ID);
			GroupDataHandleT							GetParentGroup(ReferenceHandleT Ref);
			bool										ValidateGroup(GroupDataHandleT Group);		// returns false if the group has <= 1 member
			void										RegisterGroup(GroupDataHandleT Group, bool RegisterRefs);
			void										DeregisterGroup(GroupDataHandleT Group, bool DeregisterRefs);
			void										StandardOutput(const char* Fmt, ...);

			GroupDataReferenceMapT						ReferenceTable;
			GroupDataArrayT								RegisteredGroups;
			void*										ConsoleContext;
		public:
			RenderWindowGroupManager();
			~RenderWindowGroupManager();

			void										Initialize();
			void										Deinitialize();

			bool										AddGroup(GroupIDT ID, TESRenderSelection* Selection);		// returns false if any of the refs in the selection were already a part of some group
			bool										RemoveGroup(TESRenderSelection* Selection);
			void										Orphanize(TESObjectREFR* Ref);				// removes the ref from its parent group, if any
			const char*									GetParentGroupID(TESObjectREFR* Ref);		// returns NULL if not a member of any group
																									// if the ref is a part of a group, replaces the selection with the group and returns true. returns false otherwise
			bool										SelectAffiliatedGroup(TESObjectREFR* Ref, TESRenderSelection* Selection, bool ClearSelection);
			bool										IsSelectionGroupable(TESRenderSelection* Selection);
			void										Clear();
			bool										GetGroupExists(const char* ID);
		};
	}
}