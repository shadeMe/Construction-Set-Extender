#include "CSERenderSelectionGroupManager.h"

namespace ConstructionSetExtender
{
	CSERenderSelectionGroupManager			CSERenderSelectionGroupManager::Instance;

	std::vector<TESRenderSelection*>* CSERenderSelectionGroupManager::GetCellExists(TESObjectCELL* Cell)
	{
		SelectionGroupMapT::iterator Match = SelectionGroupMap.find(Cell);
		if (Match != SelectionGroupMap.end())
			return &Match->second;
		else
			return NULL;
	}

	TESRenderSelection* CSERenderSelectionGroupManager::GetRefSelectionGroup(TESObjectREFR* Ref, TESObjectCELL* Cell)
	{
		TESRenderSelection* Result = NULL;

		std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
		if (SelectionList)
		{
			for (std::vector<TESRenderSelection*>::iterator Itr = SelectionList->begin(); Itr != SelectionList->end(); Itr++)
			{
				for (TESRenderSelection::SelectedObjectsEntry* ItrEx = (*Itr)->selectionList; ItrEx && ItrEx->Data; ItrEx = ItrEx->Next)
				{
					if (ItrEx->Data == Ref)
					{
						Result = *Itr;
						break;
					}
				}
			}
		}

		return Result;
	}

	void CSERenderSelectionGroupManager::Clear()
	{
		for (SelectionGroupMapT::iterator Itr = SelectionGroupMap.begin(); Itr != SelectionGroupMap.end(); Itr++)
		{
			for (std::vector<TESRenderSelection*>::iterator ItrEx = Itr->second.begin(); ItrEx != Itr->second.end(); ItrEx++)
				(*ItrEx)->DeleteInstance();

			Itr->second.clear();
		}
		SelectionGroupMap.clear();
	}

	TESRenderSelection* CSERenderSelectionGroupManager::AllocateNewSelection(TESRenderSelection* Selection)
	{
		TESRenderSelection* Group = TESRenderSelection::CreateInstance();

		for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			Group->AddToSelection(Itr->Data, false);

		return Group;
	}

	TESObjectREFR* CSERenderSelectionGroupManager::GetRefAtSelectionIndex(TESRenderSelection* Selection, UInt32 Index)
	{
		UInt32 Count = 0;
		for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next, Count++)
		{
			if (Count == Index)
				return CS_CAST(Itr->Data, TESForm, TESObjectREFR);
		}
		return NULL;
	}

	TESRenderSelection* CSERenderSelectionGroupManager::GetTrackedSelection(TESObjectCELL* Cell, TESRenderSelection* Selection)
	{
		TESRenderSelection* Result = NULL;

		std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
		if (SelectionList)
		{
			for (std::vector<TESRenderSelection*>::iterator Itr = SelectionList->begin(); Itr != SelectionList->end(); Itr++)
			{
				TESRenderSelection* Base = *Itr;
				if (Base->selectionCount == Selection->selectionCount)
				{
					bool Mismatch = false;
					for (int i = 0; i < Selection->selectionCount; i++)
					{
						if (GetRefAtSelectionIndex(Selection, i) != GetRefAtSelectionIndex(Base, i))
						{
							Mismatch = true;
							break;
						}
					}
					if (Mismatch)
						break;

					Result = Base;
				}
			}
		}

		return Result;
	}

	void CSERenderSelectionGroupManager::UntrackSelection(TESObjectCELL* Cell, TESRenderSelection* TrackedSelection)
	{
		std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
		if (SelectionList)
		{
			std::vector<TESRenderSelection*>::const_iterator EraseItr = SelectionList->end();

			for (std::vector<TESRenderSelection*>::iterator Itr = SelectionList->begin(); Itr != SelectionList->end(); Itr++)
			{
				if (*Itr == TrackedSelection)
				{
					(*Itr)->DeleteInstance();
					EraseItr = Itr;
					break;
				}
			}

			if (EraseItr != SelectionList->end())
				SelectionList->erase(EraseItr);
		}
	}

	bool CSERenderSelectionGroupManager::AddGroup(TESObjectCELL *Cell, TESRenderSelection *Selection)
	{
		bool Result = false;

		std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
		if (SelectionList)
		{
			bool ExistingGroup = false;
			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				if (GetRefSelectionGroup(CS_CAST(Itr->Data, TESForm, TESObjectREFR), Cell))
				{
					ExistingGroup = true;
					break;
				}
			}

			if (!ExistingGroup)
			{
				TESRenderSelection* Group = AllocateNewSelection(Selection);
				SelectionList->push_back(Group);
				Result = true;
			}
		}
		else
		{
			SelectionGroupMap[Cell] = std::vector<TESRenderSelection*>();

			TESRenderSelection* Group = AllocateNewSelection(Selection);
			SelectionGroupMap[Cell].push_back(Group);
			Result = true;
		}

		return Result;
	}

	bool CSERenderSelectionGroupManager::RemoveGroup(TESObjectCELL *Cell, TESRenderSelection *Selection)
	{
		bool Result = false;

		std::vector<TESRenderSelection*>* SelectionList = GetCellExists(Cell);
		if (SelectionList)
		{
			TESRenderSelection* TrackedSelection = GetTrackedSelection(Cell, Selection);
			if (TrackedSelection)
			{
				UntrackSelection(Cell, TrackedSelection);
				Result = true;
			}
		}

		return Result;
	}
}