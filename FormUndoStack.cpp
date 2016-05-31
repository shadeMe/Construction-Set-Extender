#include "FormUndoStack.h"

namespace cse
{
	namespace formUndoStack
	{
		FormUndoProxy::FormUndoProxy( TESForm* ParentForm, TESForm* CopySource ) :
			bgsee::FormUndoProxy(),
			Parent(ParentForm),
			Buffer(NULL)
		{
			SME_ASSERT(ParentForm && ParentForm != CopySource);

			if (CopySource == NULL)
				CopySource = ParentForm;
			
			if (CopySource->GetEditorID())
				EditorID = CopySource->GetEditorID();
			else
				EditorID = "";

			Modified = CopySource->formFlags & TESForm::kFormFlags_FromActiveFile;

			if (CopySource == ParentForm && (CopySource->formFlags & TESForm::kFormFlags_Temporary))
			{
				BGSEEUNDOSTACK->Print("Creating a undo proxy for a temporary form %s %08X - WTF!", EditorID.c_str(), CopySource->formID);
			}

			Buffer = TESForm::CreateTemporaryCopy(CopySource, false);
		}

		FormUndoProxy::~FormUndoProxy()
		{
			SME_ASSERT(Buffer);

			Buffer->DeleteInstance();
		}

		UInt32 FormUndoProxy::GetFormID( void ) const
		{
			return Parent->formID;
		}

		UInt8 FormUndoProxy::GetType( void ) const
		{
			return Parent->formType;
		}

		const char* FormUndoProxy::GetTypeString( void ) const
		{
			return Parent->GetTypeIDString();
		}

		void FormUndoProxy::Undo( void )
		{
			Parent->CopyFrom(Buffer);

			if (EditorID.length())
			{
				if (_stricmp(EditorID.c_str(), Parent->GetEditorID()))
				{
				
					if (TESForm::LookupByEditorID(EditorID.c_str()) == NULL)
					{
						// copy editorID if it's been changed and no other form's using it
						if (Parent->SetEditorID(EditorID.c_str()) == false)
						{
							BGSEEUNDOSTACK->Print("Couldn't copy editorID from undo proxy to form %08X", GetFormID());
						}
					}
					else
					{
						BGSEEUNDOSTACK->Print("Couldn't undo editorID for proxy %08X - Old editorID is in use", GetFormID());
					}
				}
			}

			Parent->SetFromActiveFile(Modified);
		}


		FormUndoStackOperator::~FormUndoStackOperator()
		{
			;//
		}

		bool FormUndoStackOperator::GetIsFormTypeUndoable( UInt8 Type )
		{
			switch (Type)
			{
			// FormEdit forms
			case TESForm::kFormType_LandTexture:
			case TESForm::kFormType_Enchantment:
			case TESForm::kFormType_Spell:
			case TESForm::kFormType_Sound:
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
			case TESForm::kFormType_Package:
			case TESForm::kFormType_CombatStyle:
			case TESForm::kFormType_LoadScreen:
			case TESForm::kFormType_LeveledSpell:
			case TESForm::kFormType_AnimObject:
			case TESForm::kFormType_WaterForm:
			case TESForm::kFormType_EffectShader:
			// ListView forms
			case TESForm::kFormType_Class:
			case TESForm::kFormType_Faction:
			case TESForm::kFormType_Hair:
			case TESForm::kFormType_Eyes:
			case TESForm::kFormType_Race:
			case TESForm::kFormType_Skill:
			case TESForm::kFormType_EffectSetting:
			case TESForm::kFormType_BirthSign:
			case TESForm::kFormType_Weather:
			case TESForm::kFormType_Climate:
			case TESForm::kFormType_WorldSpace:
			case TESForm::kFormType_GMST:
			case TESForm::kFormType_Global:			//	### weird bugger, this - has a stubbed out TESForm::GetFromDialog
													//		counter-intuitively, the data sync is performed directly inside the dlg message callback
													//		remnant of the Morrowind CS code base, methinks. perhaps even ToddCode™
			// Misc forms
//			case TESForm::kFormType_TopicInfo:			### can be copied but might be troublesome
//			case TESForm::kFormType_Idle:				### pass, given its linked list'y nature
//			case TESForm::kFormType_Script:				### use the super awesome script editor
//			case TESForm::kFormType_Region:				### doesn't allow copying
//			case TESForm::kFormType_REFR:				### use the built-in undo tool
//			case TESForm::kFormType_ACHR:
//			case TESForm::kFormType_ACRE:
//			case TESForm::kFormType_PathGrid:			### implemented elsewhere
//			case TESForm::kFormType_Land:				### use the built-in undo tool
//			case TESForm::kFormType_Road:				### what this is, I don't even...
//			case TESForm::kFormType_Topic:				### doesn't allow copying
//			case TESForm::kFormType_Cell:				### yet another special-case (Morrowind code again?), not gonna bother supporting this one either
//			case TESForm::kFormType_Quest:				### up is down with this form's UI code! too screwed up to fix/enhance
				return true;
			default:
				return false;
			}
		}

		bool FormUndoStackOperator::PreUndoRedoCallback( bgsee::FormUndoProxy* Proxy, bgsee::FormUndoProxy** OutAltProxy )
		{
			SME_ASSERT(Proxy && OutAltProxy);

			bool Result = false;

			FormUndoProxy* Current = dynamic_cast<FormUndoProxy*>(Proxy);
			SME_ASSERT(Current);

			*OutAltProxy = new FormUndoProxy(Current->Parent);
			Result = true;

			return Result;
		}

		void FormUndoStackOperator::PostUndoRedoCallback( void )
		{
			TESObjectWindow::RefreshFormList();
		}

		void Initialize( void )
		{
			bool ComponentInitialized = BGSEEUNDOSTACK->Initialize(new FormUndoStackOperator());

			SME_ASSERT(ComponentInitialized);
		}
	}
}