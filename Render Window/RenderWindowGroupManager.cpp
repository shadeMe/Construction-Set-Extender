#include "RenderWindowGroupManager.h"
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		void RenderWindowGroupManager::GroupData::ValidateMembers(MemberRosterT& OutDelinquents, TESObjectREFRArrayT* OutValidMembers)
		{
			if (OutValidMembers)
				OutValidMembers->clear();
			OutDelinquents.clear();

			for (MemberRosterT::iterator Itr = Members.begin(); Itr != Members.end();)
			{
				TESObjectREFR* ValidRef = nullptr;
				if (RenderWindowGroupManager::GetReferenceExists(*Itr, ValidRef) == false)
				{
					OutDelinquents.push_back(*Itr);
					Itr = Members.erase(Itr);
					continue;
				}
				else if (OutValidMembers)
					OutValidMembers->push_back(ValidRef);

				Itr++;
			}
		}

		void RenderWindowGroupManager::GroupData::AddMember(ReferenceHandleT Ref)
		{
			MemberRosterT::iterator Match = Members.end();
			if (GetIsMember(Ref, Match) == false)
				Members.push_back(Ref);
		}

		bool RenderWindowGroupManager::GroupData::GetIsMember(ReferenceHandleT Ref, MemberRosterT::iterator& Match)
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

		RenderWindowGroupManager::GroupData::GroupData(GroupIDT ID, TESRenderSelection* Selection) :
			Members(),
			ID(ID)
		{
			SME_ASSERT(Selection);

			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				UInt8 FormType = Itr->Data->formType;

				SME_ASSERT(FormType == TESForm::kFormType_ACHR || FormType == TESForm::kFormType_ACRE || FormType == TESForm::kFormType_REFR);
				Members.push_back(Itr->Data->formID);
			}
		}

		void RenderWindowGroupManager::GroupData::RemoveMember(ReferenceHandleT Ref)
		{
			MemberRosterT::iterator Match = Members.end();

			if (GetIsMember(Ref, Match))
				Members.erase(Match);
		}

		void RenderWindowGroupManager::GroupData::ConvertToSelection(TESRenderSelection* Selection, bool ClearSelection)
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

		bool RenderWindowGroupManager::GetReferenceExists(ReferenceHandleT Ref, TESObjectREFR*& OutResolvedRef)
		{
			TESForm* Form = TESForm::LookupByFormID(Ref);

			if (Form)
			{
				SME_ASSERT(Form->formType == TESForm::kFormType_ACHR || Form->formType == TESForm::kFormType_ACRE || Form->formType == TESForm::kFormType_REFR);

				OutResolvedRef = CS_CAST(Form, TESForm, TESObjectREFR);
				return (Form->IsDeleted() == false);
			}
			else
				return false;
		}

		bool RenderWindowGroupManager::GetGroupExists(GroupIDT ID) const
		{
			GroupDataHandleT Existing = LookupGroup(ID);
			if (Existing)
				return true;
			else
				return false;
		}

		bool RenderWindowGroupManager::GetGroupExists(const char* ID) const
		{
			return GetGroupExists(GroupIDT(ID));
		}

		bool RenderWindowGroupManager::GetGroupData(const char* ID, TESObjectREFRArrayT& OutMembers)
		{
			GroupDataHandleT Exisiting = LookupGroup(ID);
			if (Exisiting)
			{
				if (ValidateGroup(Exisiting, &OutMembers))
					return true;
			}

			return false;
		}

		RenderWindowGroupManager::GroupDataHandleT RenderWindowGroupManager::LookupGroup(GroupIDT ID) const
		{
			for (auto Itr : RegisteredGroups)
			{
				if (!_stricmp(ID.c_str(), Itr->GetID().c_str()))
					return Itr;
			}

			return nullptr;
		}

		RenderWindowGroupManager::GroupDataHandleT RenderWindowGroupManager::GetParentGroup(ReferenceHandleT Ref) const
		{
			if (ReferenceTable.count(Ref))
				return GroupDataHandleT(ReferenceTable.at(Ref));
			else
				return GroupDataHandleT(nullptr);
		}

		bool RenderWindowGroupManager::ValidateGroup(GroupDataHandleT Group, TESObjectREFRArrayT* OutValidMembers)
		{
			MemberRosterT Delinquents;
			Group->ValidateMembers(Delinquents, OutValidMembers);

			for (auto Itr : Delinquents)
				ReferenceTable.erase(Itr);

			if (Group->GetSize() <= 1)
			{
				StandardOutput("Empty group '%s' dissolved", Group->GetID().c_str());
				DeregisterGroup(Group, true);
				return false;
			}
			else
				return true;
		}


		void RenderWindowGroupManager::RegisterGroup(GroupDataHandleT Group, bool RegisterRefs)
		{
			RegisteredGroups.push_back(Group);
			if (RegisterRefs)
			{
				for (auto Itr : Group->Members)
					ReferenceTable[Itr] = Group;
			}
		}

		void RenderWindowGroupManager::DeregisterGroup(GroupDataHandleT Group, bool DeregisterRefs)
		{
			GroupDataArrayT::iterator Match = std::find(RegisteredGroups.begin(), RegisteredGroups.end(), Group);
			SME_ASSERT(Match != RegisteredGroups.end());
			RegisteredGroups.erase(Match);

			if (DeregisterRefs)
			{
				for (auto Itr : Group->Members)
					ReferenceTable.erase(Itr);
			}
		}

		void RenderWindowGroupManager::StandardOutput(const char* Fmt, ...) const
		{
			if (Fmt == nullptr)
				return;

			char Buffer[0x1000] = { 0 };
			va_list Args;
			va_start(Args, Fmt);
			vsprintf_s(Buffer, sizeof(Buffer), Fmt, Args);
			va_end(Args);

			if (strlen(Buffer))
			{
				NotificationOSDLayer::Instance.ShowNotification(Buffer);
				BGSEECONSOLE->PrintToMessageLogContext(ConsoleContext, false, "%s", Buffer);
			}
		}

		RenderWindowGroupManager::RenderWindowGroupManager() :
			ReferenceTable(),
			RegisteredGroups(),
			ConsoleContext(nullptr)
		{
			;//
		}

		RenderWindowGroupManager::~RenderWindowGroupManager()
		{
			Clear();
		}

		void RenderWindowGroupManager::Initialize()
		{
			ConsoleContext = BGSEECONSOLE->RegisterMessageLogContext("Reference Group Manager");
			SME_ASSERT(ConsoleContext);
		}

		void RenderWindowGroupManager::Deinitialize()
		{
			BGSEECONSOLE->UnregisterMessageLogContext(ConsoleContext);
			ConsoleContext = nullptr;
		}

		bool RenderWindowGroupManager::AddGroup(GroupIDT ID, TESRenderSelection* Selection)
		{
			SME_ASSERT(Selection);

			bool Result = true;
			if (GetGroupExists(ID))
			{
				StandardOutput("Group with ID '%s' already exists", ID.c_str());
				Result = false;
			}
			else if (IsSelectionGroupable(Selection) == false)
				Result = false;
			else
			{
				GroupDataHandleT Grouping(new GroupData(ID, Selection));
				RegisterGroup(Grouping, true);
			}

			return Result;
		}

		void RenderWindowGroupManager::RemoveSelectionGroups(TESRenderSelection* Selection)
		{
			SME_ASSERT(Selection);

			int Count = 0;
			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				ReferenceHandleT FormID = Itr->Data->formID;
				GroupDataHandleT Exisiting(GetParentGroup(FormID));

				if (Exisiting)
				{
					StandardOutput("Group '%s' dissolved", Exisiting->GetID().c_str());
					DeregisterGroup(Exisiting, true);
					Count++;
				}
			}

			if (Count == 0)
				StandardOutput("No groups in selection");
		}

		bool RenderWindowGroupManager::RemoveParentGroup(TESObjectREFR* Ref)
		{
			SME_ASSERT(Ref);

			ReferenceHandleT FormID(Ref->formID);
			GroupDataHandleT Group(GetParentGroup(FormID));
			if (Group)
			{
				DeregisterGroup(Group, true);
				return true;
			}
			else
			{
				StandardOutput("Reference %08X is not a member of any group", Ref);
				return false;
			}
		}

		bool RenderWindowGroupManager::Orphanize(TESObjectREFR* Ref)
		{
			SME_ASSERT(Ref);

			ReferenceHandleT FormID(Ref->formID);
			GroupDataHandleT Group(GetParentGroup(FormID));
			if (Group)
			{
				Group->RemoveMember(FormID);
				ReferenceTable.erase(FormID);

				if (Group->GetSize() <= 1)
				{
					StandardOutput("Empty group '%s' dissolved", Group->GetID().c_str());
					DeregisterGroup(Group, true);
				}

				return true;
			}
			else
			{
				StandardOutput("Reference %08X is not a member of any group", Ref);
				return false;
			}
		}

		const char* RenderWindowGroupManager::GetParentGroupID(TESObjectREFR* Ref) const
		{
			SME_ASSERT(Ref);

			ReferenceHandleT FormID(Ref->formID);
			GroupDataHandleT Group(GetParentGroup(FormID));
			if (Group)
			{
				// ### using the accessor method mangles up the this pointer at this location for some reason, in debug mode atleast
				return Group->ID.c_str();
			}
			else
				return nullptr;
		}

		bool RenderWindowGroupManager::SelectAffiliatedGroup(TESObjectREFR* Ref, TESRenderSelection* Selection, bool ClearSelection)
		{
			SME_ASSERT(Ref && Selection);

			bool Result = false;
			ReferenceHandleT FormID(Ref->formID);
			GroupDataHandleT Group(GetParentGroup(FormID));
			if (Group)
			{
				// validate the members first to account for deleted refs
				if (ValidateGroup(Group))
				{
					Group->ConvertToSelection(Selection, ClearSelection);
					Result = true;
				}
			}

			return Result;
		}

		bool RenderWindowGroupManager::IsSelectionGroupable(TESRenderSelection* Selection) const
		{
			SME_ASSERT(Selection);

			bool Result = true;
			if (Selection->selectionCount < 2)
			{
				StandardOutput("Current selection is too small");
				Result = false;
			}
			else
			{
				for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					ReferenceHandleT FormID = Itr->Data->formID;
					GroupDataHandleT Exisiting(GetParentGroup(FormID));

					if (Exisiting != nullptr)
					{
						StandardOutput("Reference %08X is already a member of group '%s'", FormID, Exisiting->GetID().c_str());
						Result = false;
					}
				}
			}

			return Result;
		}

		int RenderWindowGroupManager::GetSelectionGroups(TESRenderSelection* Selection, std::vector<std::string>& OutGroupIDs) const
		{
			OutGroupIDs.clear();

			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				ReferenceHandleT FormID = Itr->Data->formID;
				GroupDataHandleT Exisiting(GetParentGroup(FormID));

				if (Exisiting)
				{
					std::string GroupID(Exisiting->GetID());
					if (std::find(OutGroupIDs.begin(), OutGroupIDs.end(), GroupID) == OutGroupIDs.end())
						OutGroupIDs.push_back(GroupID);
				}
			}

			return OutGroupIDs.size();
		}

		void RenderWindowGroupManager::Clear()
		{
			ReferenceTable.clear();
			RegisteredGroups.clear();
		}
	}
}
