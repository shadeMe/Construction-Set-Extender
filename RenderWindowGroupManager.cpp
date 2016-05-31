#include "RenderWindowGroupManager.h"

namespace cse
{
	RenderWindowGroupManager			RenderWindowGroupManager::Instance;
	
	bool RenderWindowGroupManager::GetReferenceExists( ReferenceHandleT Ref )
	{
		TESForm* Form = TESForm::LookupByFormID(Ref);

		if (Form)
		{
			SME_ASSERT(Form->formType == TESForm::kFormType_ACHR || Form->formType == TESForm::kFormType_ACRE || Form->formType == TESForm::kFormType_REFR);

			return (Form->IsDeleted() == false);
		}
		else
			return false;
	}

	UInt32 RenderWindowGroupManager::GroupData::GetNextID( void )
	{
		static UInt32 kBaseID = 0x1;

		SME_ASSERT(kBaseID < 0xFFFFFFFF);			// sanity check, who the heck needs so many groups anyway!
		return kBaseID++;
	}

	UInt32 RenderWindowGroupManager::GroupData::ValidateMembers( void )
	{
		for (MemberRosterT::iterator Itr = Members.begin(); Itr != Members.end();)
		{
			if (RenderWindowGroupManager::GetReferenceExists(*Itr) == false)
			{
				Itr = Members.erase(Itr);
				continue;
			}

			Itr++;
		}

		return GetSize();
	}

	bool RenderWindowGroupManager::GroupData::GetIsMember( ReferenceHandleT Ref, MemberRosterT::iterator& Match )
	{
		for (MemberRosterT::iterator Itr = Members.begin(); Itr != Members.end(); Itr++)
		{
			if (*Itr == Ref)
			{
				Match = Itr;
				return true;
			}
		}

		return false;
	}

	RenderWindowGroupManager::GroupData::GroupData( TESRenderSelection* Selection ) :
		Members(),
		ID(GetID())
	{
		SME_ASSERT(Selection);

		for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
		{
			UInt8 FormType = Itr->Data->formType;

			SME_ASSERT(FormType == TESForm::kFormType_ACHR || FormType == TESForm::kFormType_ACRE || FormType == TESForm::kFormType_REFR);
			Members.push_back(Itr->Data->formID);
		}
	}

	void RenderWindowGroupManager::GroupData::RemoveMember( ReferenceHandleT Ref )
	{
		MemberRosterT::iterator Match = Members.end();

		if (GetIsMember(Ref, Match))
		{
			Members.erase(Match);
		}
	}

	void RenderWindowGroupManager::GroupData::ConvertToSelection( TESRenderSelection* Selection, bool ClearSelection )
	{
		SME_ASSERT(Selection);

		if (ClearSelection)
			Selection->ClearSelection(true);

		for (MemberRosterT::iterator Itr = Members.begin(); Itr != Members.end(); Itr++)
		{
			TESObjectREFR* Ref = CS_CAST(TESForm::LookupByFormID(*Itr), TESForm, TESObjectREFR);
			SME_ASSERT(Ref);

			if (Selection->HasObject(Ref) == false)
				Selection->AddToSelection(Ref, true);
		}
	}

	RenderWindowGroupManager::RenderWindowGroupManager() :
		DataStore()
	{
		;//
	}

	bool RenderWindowGroupManager::AddGroup( TESRenderSelection* Selection )
	{
		SME_ASSERT(Selection);

		bool Result = true;

		if (Selection->selectionCount < 2)
			Result = false;
		else
		{
			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				ReferenceHandleT FormID = Itr->Data->formID;

				if (DataStore.count(FormID))
				{
					BGSEECONSOLE_MESSAGE("Reference %08X is already a member of an existing group", FormID);
					Result = false;
				}
			}

			if (Result)
			{
				GroupDataHandleT Grouping(new GroupData(Selection));

				for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					DataStore[Itr->Data->formID] = GroupDataHandleT(Grouping);
				}
			}
		}

		return Result;
	}

	bool RenderWindowGroupManager::RemoveGroup( TESRenderSelection* Selection )
	{
		SME_ASSERT(Selection);

		bool Result = true;

		UInt32 GroupID = 0;
		UInt32 GroupSize = 0;
		for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
		{
			ReferenceHandleT FormID = Itr->Data->formID;

			if (DataStore.count(FormID) == 0)
			{
				BGSEECONSOLE_MESSAGE("Reference %08X is not a member of an existing group", FormID);
				Result = false;
			}
			else
			{
				UInt32 CurrentID = DataStore[FormID]->GetID();

				if (GroupID == 0)
				{
					GroupID = CurrentID;
					GroupSize = DataStore[FormID]->GetSize();
				}

				if (GroupID != CurrentID)
				{
					BGSEECONSOLE_MESSAGE("Group ID mismatch - Reference %08X must be a member of group %d, not %d", GroupID, CurrentID);
					Result = false;
				}
			}
		}

		if (Result)
		{
			if (GroupSize != Selection->selectionCount)
			{
				BGSEECONSOLE_MESSAGE("Group size mismatch - Expected %d, selection contained %d", GroupSize, Selection->selectionCount);
				Result = false;
			}
			else
			{
				for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					GroupDataStoreT::iterator Match = DataStore.find(Itr->Data->formID);

					SME_ASSERT(Match != DataStore.end());
					DataStore.erase(Match);
				}
			}
		}

		return Result;
	}

	void RenderWindowGroupManager::Orphanize( ReferenceHandleT Ref )
	{
		SME_ASSERT(Ref);

		GroupDataStoreT::iterator Match = DataStore.find(Ref);

		if (Match != DataStore.end())
		{
			DataStore[Ref]->RemoveMember(Ref);
			DataStore.erase(Match);
		}
	}

	bool RenderWindowGroupManager::SelectAffiliatedGroup( TESObjectREFR* Ref, TESRenderSelection* Selection, bool ClearSelection )
	{
		SME_ASSERT(Ref && Selection);

		bool Result = false;

		if (DataStore.count(Ref->formID))
		{
			GroupDataHandleT Group = DataStore[Ref->formID];

			if (Group->ValidateMembers() == 1)
			{
				// only one member (the current ref) in the group, so dissolve it
				DataStore.erase(DataStore.find(Ref->formID));
			}
			else
			{
				Group->ConvertToSelection(Selection, ClearSelection);
				Result = true;
			}
		}

		return Result;
	}

	void RenderWindowGroupManager::Clear()
	{
		DataStore.clear();
	}
}
