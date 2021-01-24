#include "GlobalClipboard.h"
#include "WorkspaceManager.h"

namespace cse
{
	namespace globalClipboard
	{
		GlobalCopyBuffer::GlobalCopyBuffer() :
			FormList()
		{
			;//
		}

		GlobalCopyBuffer::~GlobalCopyBuffer()
		{
			for (bgsee::FormListT::iterator Itr = FormList.begin(); Itr != FormList.end(); Itr++)
				delete *Itr;

			FormList.clear();
		}

		void GlobalCopyBuffer::Add(TESForm* Form)
		{
			SME_ASSERT(Form);

			if (Form->IsReference())
			{
				TESObjectREFR* Ref = CS_CAST(Form, TESForm, TESObjectREFR);
				if (Ref->baseForm)
				{
					if (Ref->baseForm->formType == TESForm::kFormType_Door ||
						Ref->baseForm == TESForm::LookupByEditorID("DoorMarker"))
					{
						return;
					}
				}
			}

			FormList.push_back(new TESFormWrapper(Form));
		}

		bool GlobalCopyBuffer::Copy(void)
		{
			bool Result = false;

			if (FormList.size())
				Result = BGSEECLIPBOARD->Copy(FormList);

			return Result;
		}

		GlobalClipboardOperator::GlobalClipboardOperator() :
			bgsee::GlobalClipboardOperator()
		{
			DefaultFormSerializer = new DefaultFormCollectionSerializer;
			ObjectRefSerializer = new ObjectRefCollectionSerializer(false);
		}

		GlobalClipboardOperator::~GlobalClipboardOperator()
		{
			SAFEDELETE(DefaultFormSerializer);
			SAFEDELETE(ObjectRefSerializer);
		}

		bool GlobalClipboardOperator::GetIsFormTypeReplicable( UInt8 Type )
		{
			switch (Type)
			{
			case TESForm::kFormType_GMST:
			case TESForm::kFormType_Global:
			case TESForm::kFormType_Class:
			case TESForm::kFormType_Faction:
			case TESForm::kFormType_Hair:
			case TESForm::kFormType_Eyes:
			case TESForm::kFormType_Race:
			case TESForm::kFormType_Sound:
//			case TESForm::kFormType_Skill:
//			case TESForm::kFormType_EffectSetting:
			case TESForm::kFormType_LandTexture:
//			case TESForm::kFormType_Script:
			case TESForm::kFormType_Enchantment:
			case TESForm::kFormType_Spell:
			case TESForm::kFormType_BirthSign:
			case TESForm::kFormType_Activator:
			case TESForm::kFormType_Apparatus:
			case TESForm::kFormType_Armor:
			case TESForm::kFormType_Book:
			case TESForm::kFormType_Clothing:
			case TESForm::kFormType_Container:
			case TESForm::kFormType_Door:
			case TESForm::kFormType_Ingredient:
			case TESForm::kFormType_Light:
			case TESForm::kFormType_Misc:
			case TESForm::kFormType_Static:
			case TESForm::kFormType_Grass:
			case TESForm::kFormType_Tree:
			case TESForm::kFormType_Flora:
			case TESForm::kFormType_Furniture:
			case TESForm::kFormType_Weapon:
			case TESForm::kFormType_Ammo:
			case TESForm::kFormType_NPC:
			case TESForm::kFormType_Creature:
			case TESForm::kFormType_LeveledCreature:
			case TESForm::kFormType_SoulGem:
			case TESForm::kFormType_Key:
			case TESForm::kFormType_AlchemyItem:
			case TESForm::kFormType_SubSpace:
			case TESForm::kFormType_SigilStone:
			case TESForm::kFormType_LeveledItem:
			case TESForm::kFormType_Weather:
			case TESForm::kFormType_Climate:
//			case TESForm::kFormType_Cell:
//			case TESForm::kFormType_Region:
			case TESForm::kFormType_REFR:
			case TESForm::kFormType_ACHR:
			case TESForm::kFormType_ACRE:
//			case TESForm::kFormType_PathGrid:
			case TESForm::kFormType_WorldSpace:
//			case TESForm::kFormType_Land:
//			case TESForm::kFormType_Road:
//			case TESForm::kFormType_Topic:
//			case TESForm::kFormType_TopicInfo:
			case TESForm::kFormType_Quest:
//			case TESForm::kFormType_Idle:
			case TESForm::kFormType_Package:
			case TESForm::kFormType_CombatStyle:
			case TESForm::kFormType_LoadScreen:
			case TESForm::kFormType_LeveledSpell:
			case TESForm::kFormType_AnimObject:
			case TESForm::kFormType_WaterForm:
			case TESForm::kFormType_EffectShader:
				return true;
			default:
				return false;
			}
		}

		void GlobalClipboardOperator::DisplayClipboardContents(bgsee::PluginFileWrapper* File)
		{
			TESFileWrapper* Wrapper = dynamic_cast<TESFileWrapper*>(File);

			SME_ASSERT(Wrapper && Wrapper->GetWrappedPlugin());

			TESFileFormListWindow::Show(nullptr, Wrapper->GetWrappedPlugin());
		}

		bgsee::FormCollectionSerializer* GlobalClipboardOperator::GetSerializer(bgsee::FormListT& Forms)
		{
			bool ExpectedRefs = false;
			SME_ASSERT(Forms.size());

			TESForm* Wrapped = dynamic_cast<TESFormWrapper*>(Forms.at(0))->GetWrappedForm();
			if (Wrapped->IsReference())
				ExpectedRefs = true;

			bgsee::FormCollectionSerializer* Out = DefaultFormSerializer;
			if (ExpectedRefs)
				Out = ObjectRefSerializer;

			for (auto Itr : Forms)
			{
				TESForm* Wrapped = dynamic_cast<TESFormWrapper*>(Itr)->GetWrappedForm();
				if ((Wrapped->IsReference() && ExpectedRefs == false) ||
					(Wrapped->IsReference() == false && ExpectedRefs))
				{
					BGSEECONSOLE_MESSAGE("Selection type mismatch! Selection can be either base forms of the same type or object references");
					Out = nullptr;
				}
			}

			return Out;
		}

		bgsee::FormCollectionSerializer* GlobalClipboardOperator::GetDeserializer(bgsee::PluginFileWrapper* File)
		{
			switch (IFormCollectionSerializer::GetFileSerializerType(File))
			{
			case IFormCollectionSerializer::kSerializer_DefaultForm:
				return DefaultFormSerializer;
			case IFormCollectionSerializer::kSerializer_ObjectRef:
				return ObjectRefSerializer;
			default:
				return nullptr;
			}
		}

		void GlobalClipboardOperator::PreCopyCallback(bgsee::FormListT& CopyForms, bgsee::PluginFileWrapper* File)
		{
			;//
		}

		bool GlobalClipboardOperator::PostCopyCallback(bool SerializationSuccessful)
		{
			return SerializationSuccessful;
		}

		void GlobalClipboardOperator::PrePasteCallback(bgsee::PluginFileWrapper* File)
		{
			;//
		}

		bool GlobalClipboardOperator::PostPasteCallback(bool Successful, bgsee::FormCollectionSerializer* Deserializer)
		{
			bool Result = false;

			if (Successful)
			{
				IFormCollectionSerializer* CSESerializer = dynamic_cast<IFormCollectionSerializer*>(Deserializer);
				SME_ASSERT(CSESerializer);

				DefaultFormCollectionInstantiator DefaultInit;
				ObjectRefCollectionInstantiator ObjRefInit;

				switch (CSESerializer->GetType())
				{
				case IFormCollectionSerializer::kSerializer_DefaultForm:
					Result = DefaultInit.Instantiate(CSESerializer);
					break;
				case IFormCollectionSerializer::kSerializer_ObjectRef:
					Result = ObjRefInit.Instantiate(CSESerializer);
					break;
				}
			}

			return Result;
		}

		void Initialize( void )
		{
			bool ComponentInitialized = bgsee::GlobalClipboard::Initialize(new GlobalClipboardOperator(), new TESFileWrapper());
			SME_ASSERT(ComponentInitialized);
		}

		void Deinitialize()
		{
			bgsee::GlobalClipboard::Deinitialize();
		}
	}
}