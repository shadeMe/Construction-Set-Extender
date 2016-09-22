#pragma once
#include "RenderWindowCommon.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowGroupManager : public NamedReferenceCollectionManager
		{

			virtual void				StandardOutput(const char* Fmt, ...) const override;
			virtual const char*			GetSaveFileName() const override;

			void						ConvertToSelection(const TESObjectREFRSafeArrayT& Refs, TESRenderSelection* Selection, bool ClearSelection) const;
			void						AddToCollection(TESRenderSelection* Selection, NamedReferenceCollection* Collection);

			void*						ConsoleContext;
		public:
			RenderWindowGroupManager();
			virtual ~RenderWindowGroupManager() override;

			virtual void				Initialize() override;
			virtual void				Deinitialize() override;

			void						Clear();
			bool						AddGroup(const char* ID, TESRenderSelection* Selection);	// returns false if any of the refs in the selection were already a part of some group
			void						RemoveSelectionGroups(TESRenderSelection* Selection);		// removes every group in the selection
			bool						RemoveParentGroup(TESObjectREFR* Ref);
			bool						Orphanize(TESObjectREFR* Ref);								// removes the ref from its parent group, returns true if successful
			const char*					GetParentGroupID(TESObjectREFR* Ref) const;					// returns NULL if not a member of any group

			bool						SelectAffiliatedGroup(TESObjectREFR* Ref, TESRenderSelection* Selection, bool ClearSelection);	// if the ref is a part of a group, replaces the selection with the group and returns true. returns false otherwise
			bool						IsSelectionGroupable(TESRenderSelection* Selection) const;
			int							GetSelectionGroups(TESRenderSelection* Selection, std::vector<std::string>& OutGroupIDs) const;		// returns the IDs of all the groups in the selection

			bool						GetGroupExists(const char* ID) const;
			bool						GetGroupData(const char* ID, TESObjectREFRArrayT& OutMembers);		// returns false if the group doesn't exist, true otherwise
		};
	}
}