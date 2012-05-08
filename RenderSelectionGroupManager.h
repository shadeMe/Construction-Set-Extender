#pragma once

namespace ConstructionSetExtender
{
	class RenderSelectionGroupManager
	{
		typedef std::map<TESObjectCELL*,
			std::vector<TESRenderSelection*>>	SelectionGroupMapT;

		SelectionGroupMapT							SelectionGroupMap;

		TESObjectREFR*								GetRefAtSelectionIndex(TESRenderSelection* Selection, UInt32 Index);

		std::vector<TESRenderSelection*>*			GetCellExists(TESObjectCELL* Cell);
		TESRenderSelection*							AllocateNewSelection(TESRenderSelection* Selection);
		TESRenderSelection*							GetTrackedSelection(TESObjectCELL* Cell, TESRenderSelection* Selection);			// returns the tracked copy of the source selection
		void										UntrackSelection(TESObjectCELL* Cell, TESRenderSelection* TrackedSelection);		// pass GetTrackedSelection's result
	public:
		bool										AddGroup(TESObjectCELL* Cell, TESRenderSelection* Selection);
		bool										RemoveGroup(TESObjectCELL* Cell, TESRenderSelection* Selection);

		TESRenderSelection*							GetRefSelectionGroup(TESObjectREFR* Ref, TESObjectCELL* Cell);
		void										Clear();

		static RenderSelectionGroupManager			Instance;
	};
}