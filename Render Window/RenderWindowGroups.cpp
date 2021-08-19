#include "RenderWindowGroups.h"
#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		void RenderWindowGroupManager::StandardOutput(const char* Fmt, ...) const
		{
			if (Fmt == nullptr)
				return;

			char Buffer[0x1000] = { 0 };
			va_list Args;
			va_start(Args, Fmt);
			vsnprintf_s(Buffer, sizeof(Buffer), _TRUNCATE, Fmt, Args);
			va_end(Args);

			if (strlen(Buffer))
			{
				NotificationOSDLayer::Instance.ShowNotification(Buffer);
				BGSEECONSOLE->PrintToMessageLogContext(ConsoleContext, false, "%s", Buffer);
			}
		}

		const char* RenderWindowGroupManager::GetSaveFileName() const
		{
			return "RenderWindowGroups";
		}

		NamedReferenceCollection* RenderWindowGroupManager::DeserializeCollection(const std::string& In) const
		{
			return NamedReferenceCollectionManager::DeserializeCollection(In);
		}

		void RenderWindowGroupManager::ConvertToSelection(const TESObjectREFRArrayT& Refs, TESRenderSelection* Selection, bool ClearSelection) const
		{
			SME_ASSERT(Selection);

			if (ClearSelection)
				Selection->ClearSelection(true);

			for (auto Itr : Refs)
			{
				if (Selection->HasObject(Itr) == false)
					Selection->AddToSelection(Itr, true);
			}
		}

		void RenderWindowGroupManager::AddToCollection(TESRenderSelection* Selection, NamedReferenceCollection* Collection)
		{
			Collection->ClearMembers();
			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFRSafeHandleT FormID = Itr->Data->formID;
				Collection->AddMember(FormID);
			}
		}

		RenderWindowGroupManager::RenderWindowGroupManager() :
			NamedReferenceCollectionManager(kValidationPolicy_DissolveWhenSize, 2),
			ConsoleContext(nullptr)
		{
			;//
		}

		RenderWindowGroupManager::~RenderWindowGroupManager()
		{
			ConsoleContext = nullptr;
		}

		void RenderWindowGroupManager::Initialize()
		{
			NamedReferenceCollectionManager::Initialize();

			ConsoleContext = BGSEECONSOLE->RegisterMessageLogContext("Reference Groups");
			SME_ASSERT(ConsoleContext);
		}

		void RenderWindowGroupManager::Deinitialize()
		{
			NamedReferenceCollectionManager::Deinitialize();

			BGSEECONSOLE->UnregisterMessageLogContext(ConsoleContext);
			ConsoleContext = nullptr;
		}

		void RenderWindowGroupManager::Clear()
		{
			ClearCollections();
		}

		bool RenderWindowGroupManager::AddGroup(const char* ID, TESRenderSelection* Selection)
		{
			SME_ASSERT(Selection);

			bool Result = true;
			if (LookupCollection(ID))
			{
				StandardOutput("Group with ID '%s' already exists", ID);
				Result = false;
			}
			else if (IsSelectionGroupable(Selection) == false)
				Result = false;
			else
			{
				NamedReferenceCollection* Grouping = new NamedReferenceCollection(ID);
				AddToCollection(Selection, Grouping);
				RegisterCollection(Grouping, true);
			}

			return Result;
		}

		void RenderWindowGroupManager::RemoveSelectionGroups(TESRenderSelection* Selection)
		{
			SME_ASSERT(Selection);

			int Count = 0;
			for (TESRenderSelection::SelectedObjectsEntry* Itr = Selection->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFRSafeHandleT FormID = Itr->Data->formID;
				NamedReferenceCollection* Exisiting(GetParentCollection(FormID));

				if (Exisiting)
				{
					StandardOutput("Group '%s' dissolved", Exisiting->GetName());
					DeregisterCollection(Exisiting, true);
					Count++;
				}
			}

			if (Count == 0)
				StandardOutput("No groups in selection");
		}

		bool RenderWindowGroupManager::RemoveParentGroup(TESObjectREFR* Ref)
		{
			SME_ASSERT(Ref);

			TESObjectREFRSafeHandleT FormID(Ref->formID);
			NamedReferenceCollection* Group(GetParentCollection(FormID));

			if (Group)
			{
				DeregisterCollection(Group, true);
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

			TESObjectREFRSafeHandleT FormID(Ref->formID);
			NamedReferenceCollection* Group(GetParentCollection(FormID));

			if (Group)
			{
				RemoveReference(FormID);
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

			TESObjectREFRSafeHandleT FormID(Ref->formID);
			NamedReferenceCollection* Group(GetParentCollection(FormID));

			if (Group)
				return Group->GetName();
			else
				return nullptr;
		}

		bool RenderWindowGroupManager::SelectAffiliatedGroup(TESObjectREFR* Ref, TESRenderSelection* Selection, bool ClearSelection)
		{
			SME_ASSERT(Ref && Selection);

			bool Result = false;
			TESObjectREFRSafeHandleT FormID(Ref->formID);
			NamedReferenceCollection* Group(GetParentCollection(FormID));

			if (Group)
			{
				// validate the members first to account for deleted refs
				TESObjectREFRArrayT ValidRefs;
				if (ValidateCollection(Group, &ValidRefs))
				{
					ConvertToSelection(ValidRefs, Selection, ClearSelection);
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
					TESObjectREFRSafeHandleT FormID(Itr->Data->formID);
					NamedReferenceCollection* Exisiting(GetParentCollection(FormID));

					if (Exisiting)
					{
						StandardOutput("Reference %08X is already a member of group '%s'", FormID, Exisiting->GetName());
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
				TESObjectREFRSafeHandleT FormID(Itr->Data->formID);
				NamedReferenceCollection* Exisiting(GetParentCollection(FormID));

				if (Exisiting)
				{
					std::string GroupID(Exisiting->GetName());
					if (std::find(OutGroupIDs.begin(), OutGroupIDs.end(), GroupID) == OutGroupIDs.end())
						OutGroupIDs.push_back(GroupID);
				}
			}

			return OutGroupIDs.size();
		}

		bool RenderWindowGroupManager::GetGroupExists(const char* ID) const
		{
			return LookupCollection(ID);
		}

		bool RenderWindowGroupManager::GetGroupData(const char* ID, TESObjectREFRArrayT& OutMembers)
		{
			NamedReferenceCollection* Exisiting = LookupCollection(ID);
			if (Exisiting)
			{
				if (ValidateCollection(Exisiting, &OutMembers))
					return true;
			}

			return false;
		}
	}
}
