#pragma once

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowGroupManager
		{
			typedef UInt32								ReferenceHandleT;

			static bool									GetReferenceExists(ReferenceHandleT Ref);

			class GroupData
			{
				static UInt32							GetNextID(void);

				typedef std::vector<ReferenceHandleT>	MemberRosterT;

				MemberRosterT							Members;
				UInt32									ID;

				bool									GetIsMember(ReferenceHandleT Ref, MemberRosterT::iterator& Match);
			public:
				GroupData(TESRenderSelection* Selection);

				UInt32									ValidateMembers(void);
				void									RemoveMember(ReferenceHandleT Ref);
				void									ConvertToSelection(TESRenderSelection* Selection, bool ClearSelection);

				const UInt32							GetID(void) const { return ID; }
				const UInt32							GetSize(void) const { return Members.size(); }
			};

			typedef boost::shared_ptr<GroupData>		GroupDataHandleT;

			typedef std::map<ReferenceHandleT, GroupDataHandleT>	GroupDataStoreT;

			GroupDataStoreT								DataStore;
		public:
			RenderWindowGroupManager();

			bool										AddGroup(TESRenderSelection* Selection);		// returns false if any of the refs in the selection were already a part of some group
			bool										RemoveGroup(TESRenderSelection* Selection);
			void										Orphanize(ReferenceHandleT Ref);				// removes the ref from its parent group, if any

																										// if the ref is a part of a group, replaces the selection with the group and returns true. returns false otherwise
			bool										SelectAffiliatedGroup(TESObjectREFR* Ref, TESRenderSelection* Selection, bool ClearSelection);

			void										Clear();
		};
	}
}