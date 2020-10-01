#include "RenderWindowCommon.h"

namespace cse
{
	namespace renderWindow
	{
		ExecutionContext::ExecutionContext(UInt8 Context) :
			Context(Context)
		{
			SME_ASSERT(Context);
		}

		bool ExecutionContext::HasLandscapeEdit() const
		{
			return Context & kMode_LandscapeEdit;
		}

		bool ExecutionContext::HasPathGridEdit() const
		{
			return Context & kMode_PathGridEdit;
		}

		bool ExecutionContext::HasReferenceEdit() const
		{
			return Context & kMode_ReferenceEdit;
		}

		bool ExecutionContext::IsExecutable() const
		{
			bool Ref = *TESRenderWindow::PathGridEditFlag == 0 && *TESRenderWindow::LandscapeEditFlag == 0;
			bool PathGrid = *TESRenderWindow::PathGridEditFlag;
			bool Landscape = *TESRenderWindow::LandscapeEditFlag;

			if (Ref && HasReferenceEdit())
				return true;
			else if (PathGrid && HasPathGridEdit())
				return true;
			else if (Landscape && HasLandscapeEdit())
				return true;
			else
				return false;
		}

		bool ExecutionContext::operator!=(const ExecutionContext& RHS)
		{
			return (*this == RHS) == false;
		}

		bool ExecutionContext::operator==(const ExecutionContext& RHS)
		{
			if (HasReferenceEdit() && RHS.HasReferenceEdit())
				return true;
			else if (HasPathGridEdit() && RHS.HasPathGridEdit())
				return true;
			else if (HasLandscapeEdit() && RHS.HasLandscapeEdit())
				return true;
			else
				return false;
		}

		IRenderWindowAction::IRenderWindowAction(std::string Name, std::string Desc, UInt8 Context) :
			Context(Context),
			Name(Name),
			Description(Desc)
		{
			;//
		}

		IRenderWindowAction::~IRenderWindowAction() = default;

		const char* IRenderWindowAction::GetName() const
		{
			return Name.c_str();
		}

		const char* IRenderWindowAction::GetDescription() const
		{
			return Description.c_str();
		}

		const ExecutionContext& IRenderWindowAction::GetExecutionContext() const
		{
			return Context;
		}


		namespace input
		{

			BuiltIn::ModifierOverride	BuiltIn::ModifierOverride::Instance;

			BuiltIn::ModifierOverride::ModifierOverride() :
				Data()
			{
				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					switch (i)
					{
					case kKey_CTRL:
						Current.BaseState = TESRenderWindow::KeyState_Control;
						break;
					case kKey_SHIFT:
						Current.BaseState = TESRenderWindow::KeyState_Shift;
						break;
					case kKey_ALT:
						Current.BaseState = nullptr;
						break;
					case kKey_SPACE:
						Current.BaseState = TESRenderWindow::KeyState_SpaceMMB;
						break;
					case kKey_Z:
						Current.BaseState = TESRenderWindow::KeyState_Z;
						break;
					}

					Current.Active = Current.NewState = false;
				}
			}

			BuiltIn::ModifierOverride::~ModifierOverride()
			{
				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					SME_ASSERT(Current.Active == false);
				}
			}

			bool BuiltIn::ModifierOverride::IsActive(UInt8 Modifier) const
			{
				SME_ASSERT(Modifier);

				if (Modifier & kModifier_Control)
					return Data[kKey_CTRL].Active;
				else if (Modifier & kModifier_Shift)
					return Data[kKey_SHIFT].Active;
				else if (Modifier & kModifier_Alt)
					return Data[kKey_ALT].Active;
				else if (Modifier & kModifier_Space)
					return Data[kKey_SPACE].Active;
				else if (Modifier & kModifier_Z)
					return Data[kKey_Z].Active;
				else
					return false;
			}

			bool BuiltIn::ModifierOverride::GetOverrideState(UInt8 Modifier) const
			{
				SME_ASSERT(Modifier);

				const State* Current = nullptr;
				if (Modifier & kModifier_Control)
					Current = &Data[kKey_CTRL];
				else if (Modifier & kModifier_Shift)
					Current = &Data[kKey_SHIFT];
				else if (Modifier & kModifier_Alt)
					Current = &Data[kKey_ALT];
				else if (Modifier & kModifier_Space)
					Current = &Data[kKey_SPACE];
				else if (Modifier & kModifier_Z)
					Current = &Data[kKey_Z];
				else
					SME_ASSERT(Current);

				SME_ASSERT(Current->Active);
				return Current->NewState;
			}

			void BuiltIn::ModifierOverride::Activate(UInt8 Modifier, bool NewState)
			{
				SME_ASSERT(Modifier);

				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					if (Modifier & (1 << i))
					{
						SME_ASSERT(Current.Active == false);

						Current.Active = true;
						Current.NewState = NewState;

						if (Current.BaseState)
						{
							Current.StateBuffer = *Current.BaseState;
							*Current.BaseState = NewState;
						}
					}
				}
			}

			void BuiltIn::ModifierOverride::Deactivate(UInt8 Modifier)
			{
				SME_ASSERT(Modifier);

				for (int i = kKey_CTRL; i < kKey__MAX; i++)
				{
					State& Current = Data[i];
					if (Modifier & (1 << i))
					{
						SME_ASSERT(Current.Active);

						Current.Active = false;

						if (Current.BaseState)
						{
							*Current.BaseState = Current.StateBuffer;
						}
					}
				}
			}

			BuiltIn::KeyBinding::KeyBinding(SHORT Key, UInt8 Modifiers) :
				KeyCode(Key),
				Modifiers(Modifiers)
			{
				SME_ASSERT(KeyCode);
			}

			void BuiltIn::KeyBinding::Trigger() const
			{
				bool Control = Modifiers & kModifier_Control;
				bool Shift = Modifiers & kModifier_Shift;
				bool Alt = Modifiers & kModifier_Alt;
				bool Space = Modifiers & kModifier_Space;
				bool Z = Modifiers & kModifier_Z;

				ModifierOverride::Instance.Activate(kModifier_Control, Control);
				ModifierOverride::Instance.Activate(kModifier_Shift, Shift);
				ModifierOverride::Instance.Activate(kModifier_Alt, Alt);
				ModifierOverride::Instance.Activate(kModifier_Space, Space);
				ModifierOverride::Instance.Activate(kModifier_Z, Z);

				BGSEEUI->GetSubclasser()->TunnelMessageToOrgWndProc(*TESRenderWindow::WindowHandle,
															  WM_KEYDOWN,
															  KeyCode,
															  NULL,
															  true);

				ModifierOverride::Instance.Deactivate(kModifier_Control);
				ModifierOverride::Instance.Deactivate(kModifier_Shift);
				ModifierOverride::Instance.Deactivate(kModifier_Alt);
				ModifierOverride::Instance.Deactivate(kModifier_Space);
				ModifierOverride::Instance.Deactivate(kModifier_Z);
			}


			bool BuiltIn::KeyBinding::IsActivated(SHORT Key) const
			{
				bool Control = ((Modifiers & kModifier_Control) && GetAsyncKeyState(kHoldable_Control)) || ((Modifiers & kModifier_Control) == false && GetAsyncKeyState(kHoldable_Control) == NULL);
				bool Shift = ((Modifiers & kModifier_Shift) && GetAsyncKeyState(kHoldable_Shift)) || ((Modifiers & kModifier_Shift) == false && GetAsyncKeyState(kHoldable_Shift) == NULL);
				bool Alt = ((Modifiers & kModifier_Alt) && GetAsyncKeyState(VK_MENU)) || ((Modifiers & kModifier_Alt) == false && GetAsyncKeyState(VK_MENU) == NULL);
				bool Space = ((Modifiers & kModifier_Space) && GetAsyncKeyState(kHoldable_Space)) || ((Modifiers & kModifier_Space) == false && GetAsyncKeyState(kHoldable_Space) == NULL);
				bool Z = ((Modifiers & kModifier_Z) && GetAsyncKeyState(kHoldable_Z)) || ((Modifiers & kModifier_Z) == false && GetAsyncKeyState(kHoldable_Z) == NULL);

				if (Key == KeyCode)
				{
					if (Control && Shift && Alt && Space && Z)
						return true;
				}

				return false;
			}


			SHORT BuiltIn::KeyBinding::GetKeyCode() const
			{
				return KeyCode;
			}

			UInt8 BuiltIn::KeyBinding::GetModifiers(bool StripSpecialModifiers) const
			{
				UInt8 Out = Modifiers;

				if (StripSpecialModifiers)
				{
					Out &= ~kModifier_Space;
					Out &= ~kModifier_Z;
				}

				return Out;
			}
		}

		bool NamedReferenceCollection::IsMember(TESObjectREFRSafeHandleT Ref, TESObjectREFRSafeArrayT::iterator& Match)
		{
			for (TESObjectREFRSafeArrayT::iterator Itr = Members.begin(); Itr != Members.end(); Itr++)
			{
				if (*Itr == Ref)
				{
					Match = Itr;
					return true;
				}
			}

			return false;
		}

		bool NamedReferenceCollection::GetValidRef(TESObjectREFRSafeHandleT Ref, TESObjectREFR*& OutResolvedRef)
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

		NamedReferenceCollection::NamedReferenceCollection(const char* Name) :
			Name(Name),
			Members()
		{
			SME_ASSERT(this->Name.find_first_of("][") == -1);
			Members.reserve(25);
		}

		NamedReferenceCollection::NamedReferenceCollection(const std::string& InSerialized) :
			Name(),
			Members()
		{
			Members.reserve(25);
			Deserialize(InSerialized);
		}

		void NamedReferenceCollection::ValidateMembers(TESObjectREFRArrayT* OutValidMembers, TESObjectREFRSafeArrayT* OutDelinquents)
		{
			if (OutValidMembers)
				OutValidMembers->clear();

			if (OutDelinquents)
				OutDelinquents->clear();

			for (TESObjectREFRSafeArrayT::iterator Itr = Members.begin(); Itr != Members.end();)
			{
				TESObjectREFR* ValidRef = nullptr;
				if (GetValidRef(*Itr, ValidRef) == false)
				{
					if (OutDelinquents)
						OutDelinquents->push_back(*Itr);

					Itr = Members.erase(Itr);
					continue;
				}
				else if (OutValidMembers)
					OutValidMembers->push_back(ValidRef);

				Itr++;
			}
		}

		void NamedReferenceCollection::AddMember(TESObjectREFR* Ref)
		{
			SME_ASSERT(Ref);
			AddMember(Ref->formID);
		}

		void NamedReferenceCollection::AddMember(TESObjectREFRSafeHandleT Ref)
		{
			TESObjectREFRSafeArrayT::iterator Match = Members.end();
			if (IsMember(Ref, Match) == false)
				Members.push_back(Ref);
		}

		void NamedReferenceCollection::RemoveMember(TESObjectREFR* Ref)
		{
			SME_ASSERT(Ref);
			RemoveMember(Ref->formID);
		}

		void NamedReferenceCollection::RemoveMember(TESObjectREFRSafeHandleT Ref)
		{
			TESObjectREFRSafeArrayT::iterator Match = Members.end();
			if (IsMember(Ref, Match))
				Members.erase(Match);
		}

		void NamedReferenceCollection::ClearMembers()
		{
			Members.clear();
		}

		void NamedReferenceCollection::ConvertToSelection(TESRenderSelection* Selection, bool ClearSelection /*= true*/, bool ShowSelectionBox /*= false*/)
		{
			SME_ASSERT(Selection);

			if (ClearSelection)
				Selection->ClearSelection(true);

			TESObjectREFRArrayT Valid;
			ValidateMembers(&Valid);

			for (auto Itr : Valid)
			{
				if (Selection->HasObject(Itr) == false)
					Selection->AddToSelection(Itr, ShowSelectionBox);
			}
		}

		const char* NamedReferenceCollection::GetName() const
		{
			return Name.c_str();
		}

		const UInt32 NamedReferenceCollection::GetSize() const
		{
			return Members.size();
		}

		const TESObjectREFRSafeArrayT& NamedReferenceCollection::GetMembers() const
		{
			return Members;
		}

		int NamedReferenceCollection::Serialize(std::string& OutSerialized)
		{
			serialization::TESForm2Text Serializer;

			OutSerialized.clear();
			OutSerialized.append("[" + Name + "]\n");

			TESObjectREFRArrayT ValidMembers;
			ValidateMembers(&ValidMembers);
			for (auto Itr : ValidMembers)
			{
				std::string Buffer;
				Serializer.Serialize(Itr, Buffer);
				OutSerialized.append(Buffer).append("\n");
			}

			return ValidMembers.size();
		}

		void NamedReferenceCollection::Deserialize(const std::string& InSerialized)
		{
			serialization::TESForm2Text Deserializer;
			SME::StringHelpers::Tokenizer Tokenizer(InSerialized.c_str(), "\n");

			std::string Line;
			int Count = 0;

			Members.clear();
			while (Tokenizer.NextToken(Line) != -1)
			{
				if (Count == 0)
				{
					if (Name.empty())
					{
						int Start = Line.find("[");
						int End = Line.find("]");
						SME_ASSERT(Start != -1 && End != -1);

						Start++;
						Name = Line.substr(Start, End - Start);
					}
					else
						SME_ASSERT(Line.find("[" + Name + "]") == 0);
				}
				else
				{
					TESForm* Ref = nullptr;
					if (Deserializer.Deserialize(Line, &Ref))
					{
						SME_ASSERT(Ref->IsReference());
						AddMember(Ref->formID);
					}
				}

				Count++;
			}
		}

		NamedReferenceCollectionManager::CosaveHandler::CosaveHandler(NamedReferenceCollectionManager* Parent) :
			Parent(Parent)
		{
			SME_ASSERT(Parent);
		}

		void NamedReferenceCollectionManager::CosaveHandler::HandleLoad(const char* PluginName, const char* CosaveDirectory)
		{
			Parent->Load(PluginName, CosaveDirectory);
		}

		void NamedReferenceCollectionManager::CosaveHandler::HandleSave(const char* PluginName, const char* CosaveDirectory)
		{
			Parent->Save(PluginName, CosaveDirectory);
		}

		void NamedReferenceCollectionManager::CosaveHandler::HandleShutdown(const char* PluginName, const char* CosaveDirectory)
		{
			;//
		}

		const char*			NamedReferenceCollectionManager::kSigilBeginCollection	= "{NamedReferenceCollection:Begin}";
		const char*			NamedReferenceCollectionManager::kSigilEndCollection	= "{NamedReferenceCollection:End}";

		bool NamedReferenceCollectionManager::GetCollectionExists(const char* Name) const
		{
			return LookupCollection(Name) != nullptr;
		}

		NamedReferenceCollection* NamedReferenceCollectionManager::LookupCollection(const char* Name) const
		{
			for (auto& Itr : RegisteredCollections)
			{
				if (!_stricmp(Itr->GetName(), Name))
					return Itr.get();
			}

			return nullptr;
		}

		NamedReferenceCollection* NamedReferenceCollectionManager::GetParentCollection(TESObjectREFRSafeHandleT Ref) const
		{
			if (ReferenceTable.count(Ref))
				return ReferenceTable.at(Ref);
			else
				return nullptr;
		}

		NamedReferenceCollection* NamedReferenceCollectionManager::GetParentCollection(TESObjectREFR* Ref) const
		{
			return GetParentCollection(Ref->formID);
		}

		bool NamedReferenceCollectionManager::CheckCollsions(NamedReferenceCollection* Collection, bool CheckMembers) const
		{
			if (LookupCollection(Collection->GetName()))
				return false;
			else if (CheckMembers)
			{
				for (auto Itr : Collection->GetMembers())
				{
					if (ReferenceTable.count(Itr))
						return false;
				}
			}

			return true;
		}

		bool NamedReferenceCollectionManager::ValidateCollection(NamedReferenceCollection* Collection, TESObjectREFRArrayT* OutValidMembers /*= nullptr*/)
		{
			TESObjectREFRSafeArrayT Delinquents;
			Collection->ValidateMembers(OutValidMembers, &Delinquents);

			for (auto Itr : Delinquents)
				ReferenceTable.erase(Itr);

			if (ValidationPolicy == kValidationPolicy_Default)
				return true;		// do nothing
			else if (ValidationPolicy == kValidationPolicy_DissolveWhenSize && Collection->GetSize() >= DissolveThreshold)
				return true;
			else if (ValidationPolicy == kValidationPolicy_DissolveWhenEmpty && Collection->GetSize())
				return true;

			StandardOutput("Collection '%s' dissolved", Collection->GetName());
			DeregisterCollection(Collection, true);
			return false;
		}

		void NamedReferenceCollectionManager::RegisterCollection(NamedReferenceCollection* Collection, bool RegisterRefs)
		{
			SME_ASSERT(CheckCollsions(Collection, RegisterRefs));

			CollectionHandleT Temp(Collection);
			RegisteredCollections.push_back(std::move(Temp));
			if (RegisterRefs)
			{
				for (auto Itr : Collection->GetMembers())
					ReferenceTable[Itr] = Collection;
			}
		}

		void NamedReferenceCollectionManager::DeregisterCollection(NamedReferenceCollection* Collection, bool DeregisterRefs)
		{
			CollectionArrayT::iterator Match = std::find_if(RegisteredCollections.begin(), RegisteredCollections.end(),
														 [&Collection](CollectionHandleT& p) { return p.get() == Collection; });
			SME_ASSERT(Match != RegisteredCollections.end());

			if (DeregisterRefs)
			{
				for (auto Itr : Collection->GetMembers())
					ReferenceTable.erase(Itr);
			}

			// release at the end as it destroys the pointer
			RegisteredCollections.erase(Match);
		}

		NamedReferenceCollection* NamedReferenceCollectionManager::DeserializeCollection(const std::string& In) const
		{
			return new NamedReferenceCollection(In);
		}

		void NamedReferenceCollectionManager::Save(const char* PluginName, const char* DirPath)
		{
			if (RegisteredCollections.size() == 0)
				return;

			std::string FilePath(DirPath); FilePath += "\\" + std::string(GetSaveFileName());
			try
			{
				if (std::filesystem::exists(FilePath))
					std::filesystem::remove(FilePath);

				std::fstream Stream(FilePath.c_str(), std::ios::out);
				for (auto& Itr : RegisteredCollections)
				{
					std::string Temp;
					int Count = Itr->Serialize(Temp);
					if (Count || ValidationPolicy == kValidationPolicy_Default)
						Stream << kSigilBeginCollection << "\n" << Temp << kSigilEndCollection << "\n\n";
				}
				Stream.close();
			}
			catch (std::exception& e)
			{
				BGSEECONSOLE_MESSAGE("Couldn't save named reference collection to '%s'. Error - %s", FilePath.c_str(), e.what());
			}
		}

		void NamedReferenceCollectionManager::Load(const char* PluginName, const char* DirPath)
		{
			std::string FilePath(DirPath); FilePath += "\\" + std::string(GetSaveFileName());
			try
			{
				ClearCollections();
				if (std::filesystem::exists(FilePath) == false)
					return;

				std::string Line;
				std::string CollectionBlock;
				bool ExtractingBlock = false;

				std::fstream Stream(FilePath.c_str(), std::ios::in);
				int Count = 0;
				while (std::getline(Stream, Line))
				{
					Count++;
					if (Line.length() < 2)
						continue;

					if (Line.find(kSigilBeginCollection) == 0)
					{
						if (ExtractingBlock)
							throw std::exception("Invalid sigil at line %d", Count);

						ExtractingBlock = true;
						CollectionBlock.clear();
						continue;
					}
					else if (Line.find(kSigilEndCollection) == 0)
					{
						if (ExtractingBlock == false)
							throw std::exception("Invalid sigil at line %d", Count);

						ExtractingBlock = false;

						NamedReferenceCollection* NewColl = DeserializeCollection(CollectionBlock);
						RegisterCollection(NewColl, true);
						ValidateCollection(NewColl);
						continue;
					}
					else
						CollectionBlock += Line + "\n";
				}

				Stream.close();
			}
			catch (std::exception& e)
			{
				BGSEECONSOLE_MESSAGE("Couldn't load named reference collection from '%s'. Error - %s", FilePath.c_str(), e.what());
			}
		}

		void NamedReferenceCollectionManager::ClearCollections()
		{
			ReferenceTable.clear();
			RegisteredCollections.clear();
		}

		bool NamedReferenceCollectionManager::AddReference(TESObjectREFR* Ref, NamedReferenceCollection* To)
		{
			return AddReference(Ref->formID, To);
		}

		bool NamedReferenceCollectionManager::AddReference(TESObjectREFRSafeHandleT Ref, NamedReferenceCollection* To)
		{
			SME_ASSERT(Ref && To);

			NamedReferenceCollection* Parent = GetParentCollection(Ref);
			if (Parent && Parent != To)
			{
				StandardOutput("Ref %08X is already a member of collection '%s'", Ref, Parent->GetName());
				return false;
			}
			else if (Parent == nullptr)
			{
				To->AddMember(Ref);
				ReferenceTable.insert(std::make_pair(Ref, To));
			}

			return true;
		}

		void NamedReferenceCollectionManager::RemoveReference(TESObjectREFR* Ref)
		{
			return RemoveReference(Ref->formID);
		}

		void NamedReferenceCollectionManager::RemoveReference(TESObjectREFRSafeHandleT Ref)
		{
			SME_ASSERT(Ref);

			NamedReferenceCollection* Parent = GetParentCollection(Ref);
			if (Parent)
			{
				Parent->RemoveMember(Ref);
				ReferenceTable.erase(Ref);
				ValidateCollection(Parent);
			}
		}

		NamedReferenceCollectionManager::NamedReferenceCollectionManager(UInt8 Policy, int DissolveThreshold) :
			RegisteredCollections(),
			ReferenceTable(),
			ValidationPolicy(Policy),
			DissolveThreshold(DissolveThreshold)
		{
			CosaveInterface = new CosaveHandler(this);
			Initialized = false;
		}

		NamedReferenceCollectionManager::~NamedReferenceCollectionManager()
		{
			DEBUG_ASSERT(Initialized == false);

			SAFEDELETE(CosaveInterface);
			ClearCollections();
		}

		void NamedReferenceCollectionManager::Initialize()
		{
			SME_ASSERT(Initialized == false);

			_COSAVE.Register(CosaveInterface);

			Initialized = true;
		}

		void NamedReferenceCollectionManager::Deinitialize()
		{
			SME_ASSERT(Initialized);

			_COSAVE.Unregister(CosaveInterface);

			Initialized = false;
		}
	}
}