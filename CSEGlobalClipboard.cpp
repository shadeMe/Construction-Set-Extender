#include "CSEGlobalClipboard.h"
#include "CSEWorkspaceManager.h"

namespace ConstructionSetExtender
{
	namespace GlobalClipboard
	{
		CSEGlobalCopyBuffer::CSEGlobalCopyBuffer() :
			FormList()
		{
			;//
		}

		CSEGlobalCopyBuffer::~CSEGlobalCopyBuffer()
		{
			for (bgsee::BGSEEFormListT::iterator Itr = FormList.begin(); Itr != FormList.end(); Itr++)
				delete *Itr;

			FormList.clear();
		}

		void CSEGlobalCopyBuffer::Add(TESForm* Form)
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

			FormList.push_back(new CSEFormWrapper(Form));
		}

		bool CSEGlobalCopyBuffer::Copy(void)
		{
			bool Result = false;

			if (FormList.size())
				Result = BGSEECLIPBOARD->Copy(FormList);

			return Result;
		}

		CSEGlobalClipboardOperator::CSEGlobalClipboardOperator() :
			bgsee::BGSEEGlobalClipboardOperator()
		{
			DefaultFormSerializer = new CSEDefaultFormCollectionSerializer;
			ObjectRefSerializer = new CSEObjectRefCollectionSerializer(true);
		}

		CSEGlobalClipboardOperator::~CSEGlobalClipboardOperator()
		{
			SAFEDELETE(DefaultFormSerializer);
			SAFEDELETE(ObjectRefSerializer);
		}

		bool CSEGlobalClipboardOperator::GetIsFormTypeReplicable( UInt8 Type )
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

		void CSEGlobalClipboardOperator::DisplayClipboardContents(bgsee::BGSEEPluginFileWrapper* File)
		{
			CSEPluginFileWrapper* Wrapper = dynamic_cast<CSEPluginFileWrapper*>(File);

			SME_ASSERT(Wrapper && Wrapper->GetWrappedPlugin());

			TESFileFormListWindow::Show(NULL, Wrapper->GetWrappedPlugin());
		}

		bgsee::BGSEEFormCollectionSerializer* CSEGlobalClipboardOperator::GetSerializer(bgsee::BGSEEFormListT& Forms)
		{
			bool ExpectedRefs = false;
			SME_ASSERT(Forms.size());

			TESForm* Wrapped = dynamic_cast<CSEFormWrapper*>(Forms.at(0))->GetWrappedForm();
			if (Wrapped->IsReference())
				ExpectedRefs = true;

			bgsee::BGSEEFormCollectionSerializer* Out = DefaultFormSerializer;
			if (ExpectedRefs)
				Out = ObjectRefSerializer;

			for each (auto Itr in Forms)
			{
				TESForm* Wrapped = dynamic_cast<CSEFormWrapper*>(Itr)->GetWrappedForm();
				if ((Wrapped->IsReference() && ExpectedRefs == false) ||
					(Wrapped->IsReference() == false && ExpectedRefs))
				{
					BGSEECONSOLE_MESSAGE("Selection type mismatch! Selection can be either base forms of the same type or object references");
					Out = NULL;
				}
			}

			return Out;
		}

		bgsee::BGSEEFormCollectionSerializer* CSEGlobalClipboardOperator::GetDeserializer(bgsee::BGSEEPluginFileWrapper* File)
		{
			switch (ICSEFormCollectionSerializer::GetFileSerializerType(File))
			{
			case ICSEFormCollectionSerializer::kSerializer_DefaultForm:
				return DefaultFormSerializer;
			case ICSEFormCollectionSerializer::kSerializer_ObjectRef:
				return ObjectRefSerializer;
			default:
				return NULL;
			}
		}

		void CSEGlobalClipboardOperator::PreCopyCallback(bgsee::BGSEEFormListT& CopyForms, bgsee::BGSEEPluginFileWrapper* File)
		{
			;//
		}

		void CSEGlobalClipboardOperator::PostCopyCallback(bool Successful)
		{
			;//
		}

		void CSEGlobalClipboardOperator::PrePasteCallback(bgsee::BGSEEPluginFileWrapper* File)
		{
			;//
		}

		void CSEGlobalClipboardOperator::PostPasteCallback(bool Successful, bgsee::BGSEEFormCollectionSerializer* Deserializer)
		{
			if (Successful)
			{
				ICSEFormCollectionSerializer* CSESerializer = dynamic_cast<ICSEFormCollectionSerializer*>(Deserializer);
				SME_ASSERT(CSESerializer);

				CSEDefaultFormCollectionInstantiator DefaultInit;
				CSEObjectRefCollectionInstantiator ObjRefInit;

				switch (CSESerializer->GetType())
				{
				case ICSEFormCollectionSerializer::kSerializer_DefaultForm:
					DefaultInit.Instantiate(CSESerializer);
					break;
				case ICSEFormCollectionSerializer::kSerializer_ObjectRef:
					ObjRefInit.Instantiate(CSESerializer);
					break;
				}
			}
		}

		void Initialize( void )
		{
			bool ComponentInitialized = BGSEECLIPBOARD->Initialize(new CSEGlobalClipboardOperator(), new CSEPluginFileWrapper());

			SME_ASSERT(ComponentInitialized);
		}
	}
}