#pragma once

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowGroupManager
		{
			
/*
			class CosaveHandler : public serialization::PluginCosaveManager::IEventHandler
			{
				RenderWindowGroupManager*		Parent;
			public:
				CosaveHandler(RenderWindowGroupManager* Parent);

				virtual void					HandleLoad(const char* PluginName, const char* CosaveDirectory);
				virtual void					HandleSave(const char* PluginName, const char* CosaveDirectory);
				virtual void					HandleShutdown(const char* PluginName, const char* CosaveDirectory);
			};

			friend class CosaveHandler;*/

			typedef UInt32											ReferenceHandleT;

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

			typedef boost::shared_ptr<GroupData>					GroupDataHandleT;
			typedef std::map<ReferenceHandleT, GroupDataHandleT>	GroupDataStoreT;

			static bool									GetReferenceExists(ReferenceHandleT Ref);





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