#include "CSEGlobalClipboard.h"
#include "CSEWorkspaceManager.h"
#include "Hooks\Hooks-Plugins.h"

namespace ConstructionSetExtender
{
	namespace GlobalClipboard
	{
		CSEGlobalClipboardOperator::CSEGlobalClipboardOperator() :
			BGSEditorExtender::BGSEEGlobalClipboardOperator(),
			LoadedFormBuffer()
		{
			;//
		}

		CSEGlobalClipboardOperator::~CSEGlobalClipboardOperator()
		{
			FreeBuffer();
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

		void CSEGlobalClipboardOperator::LoadForm( BGSEditorExtender::BGSEEPluginFileWrapper* File )
		{
			CSEPluginFileWrapper* Wrapper = dynamic_cast<CSEPluginFileWrapper*>(File);

			SME_ASSERT(Wrapper && Wrapper->GetWrappedPlugin());

			switch (Wrapper->GetRecordType())
			{
			case TESForm::kFormType_None:
			case TESForm::kFormType_TES4:
			case TESForm::kFormType_Group:
			case TESForm::kFormType_TOFT:
				break;
			default:
				{
					TESForm* TempForm = NULL;

					switch (Wrapper->GetRecordType())
					{
					case TESForm::kFormType_REFR:
					case TESForm::kFormType_ACHR:
					case TESForm::kFormType_ACRE:
						TempForm = TESForm::CreateInstance(TESForm::kFormType_REFR);
						break;
					default:
						TempForm = TESForm::CreateInstance(Wrapper->GetRecordType());
						break;
					}

					SME_ASSERT(TempForm);

					TempForm->MarkAsTemporary();
					if (TempForm->LoadForm(Wrapper->GetWrappedPlugin()) == false)
					{
						BGSEECONSOLE_MESSAGE("Couldn't load form %08X!", Wrapper->GetWrappedPlugin()->currentRecord.recordID);
						TempForm->DeleteInstance();
					}
					else
					{
#ifdef _DEBUG
						BGSEECONSOLE_MESSAGE("Read form %s %08X from buffer", TempForm->GetEditorID(), TempForm->formID);
#endif
						// save it later, for ze Precious...
						LoadedFormBuffer.push_back(TempForm);
					}
				}

				break;
			}
		}

		void CSEGlobalClipboardOperator::SaveForm( BGSEditorExtender::BGSEEPluginFileWrapper* File, BGSEditorExtender::BGSEEFormWrapper* Form )
		{
			CSEFormWrapper* FormW = dynamic_cast<CSEFormWrapper*>(Form);
			CSEPluginFileWrapper* FileW = dynamic_cast<CSEPluginFileWrapper*>(File);

			SME_ASSERT(FormW && FormW->GetWrappedForm() && FileW && FileW->GetWrappedPlugin());

			FormW->GetWrappedForm()->SaveFormRecord(FileW->GetWrappedPlugin());

#ifdef _DEBUG
			BGSEECONSOLE_MESSAGE("Wrote form %s %08X to buffer", Form->GetEditorID(), Form->GetFormID());
#endif
		}

		void CSEGlobalClipboardOperator::DisplayClipboardContents( BGSEditorExtender::BGSEEPluginFileWrapper* File )
		{
			CSEPluginFileWrapper* Wrapper = dynamic_cast<CSEPluginFileWrapper*>(File);

			SME_ASSERT(Wrapper && Wrapper->GetWrappedPlugin());

			TESFileFormListWindow::Show(NULL, Wrapper->GetWrappedPlugin());
		}

		void CSEGlobalClipboardOperator::PreSaveCallback( BGSEditorExtender::BGSEEFormListT& SaveForms, BGSEditorExtender::BGSEEPluginFileWrapper* File )
		{
			// get rid of the "Failed to CreateGroupData..." warning
			Hooks::_MemHdlr(TESFileUpdateOpenGroups).WriteJump();
		}

		void CSEGlobalClipboardOperator::PostSaveCallback( void )
		{
			// restore the warning
			Hooks::_MemHdlr(TESFileUpdateOpenGroups).WriteBuffer();
		}

		void CSEGlobalClipboardOperator::PreLoadCallback( void )
		{
			;//
		}

		void CSEGlobalClipboardOperator::PostLoadCallback( void )
		{
			bool ReplaceAll = false;
			bool CopyingRefs = false;
			UInt32 CopiedForms = 0;

			for (FormListT::iterator Itr = LoadedFormBuffer.begin(); Itr != LoadedFormBuffer.end(); Itr++)
			{
				TESForm* TempForm = *Itr;
				bool FormExists = false;

				TempForm->LinkForm();

				if (TempForm->IsReference())
				{
					CopyingRefs = true;
					if ((*TESRenderWindow::CurrentlyLoadedExteriorCell == NULL && _TES->currentInteriorCell == NULL) ||
						*TESRenderWindow::PathGridEditFlag ||
						*TESRenderWindow::LandscapeEditFlag)
					{
						BGSEECONSOLE_MESSAGE("Cannot copy references! Possible reasons: No cell loaded in the render window, pathgrid/landscape edit mode enabled");
						break;
					}
					else if ((CS_CAST(TempForm, TESForm, TESObjectREFR))->baseForm == NULL)
					{
						BGSEECONSOLE_MESSAGE("Couldn't copy reference %08X '%s' - Unresolved base form", TempForm->formID, TempForm->GetEditorID());
						continue;
					}
				}


				TESForm* CurrentForm = TESForm::LookupByEditorID(TempForm->GetEditorID());
				if (CurrentForm == NULL)
				{
					switch (TempForm->formType)
					{
					case TESForm::kFormType_REFR:
					case TESForm::kFormType_ACHR:
					case TESForm::kFormType_ACRE:
						CurrentForm = TESForm::CreateInstance(TESForm::kFormType_REFR);
						break;
					default:
						CurrentForm = TESForm::CreateInstance(TempForm->formType);
						break;
					}
				}
				else
				{
					FormExists = true;

					if (TempForm->formType != CurrentForm->formType)
					{
						BGSEECONSOLE_MESSAGE("Couldn't copy form %s %08X from clipboard - A form with the same editorID but different type exists!", TempForm->GetEditorID(), TempForm->formID);
						continue;
					}
					else if (ReplaceAll == false)
					{
						bool Skip = false;
						switch (BGSEEUI->MsgBoxI(NULL,
							MB_TASKMODAL|MB_SETFOREGROUND|MB_YESNOCANCEL,
							"Form %s already exists. Do you wish to replace it?\n\n\"Cancel\" will replace all existing forms.", CurrentForm->GetEditorID()))
						{
						case IDNO:
							Skip = true;
							break;
						case IDCANCEL:
							ReplaceAll = true;
							break;
						}

						if (Skip)
							continue;
					}
				}
				
				CurrentForm->CopyFrom(TempForm);
				CurrentForm->SetFromActiveFile(true);

				if (FormExists == false)
				{
					if(TempForm->GetEditorID())
						CurrentForm->SetEditorID(TempForm->GetEditorID());

					_DATAHANDLER->AddForm(CurrentForm);
				}

				if (CurrentForm->IsReference())
				{
					TESObjectREFR* Ref = CS_CAST(CurrentForm, TESForm, TESObjectREFR);
					TESObject* Base = CS_CAST(Ref->baseForm, TESForm, TESObject);

					SME_ASSERT(Ref && Base);

					Vector3 Position(Ref->position);
					Vector3 Rotation(Ref->rotation);

					TESObjectCELL* CurrentCell = _TES->currentInteriorCell;
					if (CurrentCell == NULL)
					{
						CurrentCell = *TESRenderWindow::CurrentlyLoadedExteriorCell;
						
						Position.x = CurrentCell->cellData.coords->x * 4096.0f;
						Position.y = CurrentCell->cellData.coords->y * 4096.0f;
					}									

					// move references to the currently loaded cell
					_DATAHANDLER->PlaceObjectRef(Base,
												&Position,
												&Rotation,
												CurrentCell,
												CurrentCell->GetParentWorldSpace(),
												Ref);
				}

				CopiedForms++;
			}

			if (CopyingRefs && CopiedForms)
			{
				TESRenderWindow::Redraw();
				TESCellViewWindow::SetCellSelection(*TESCellViewWindow::CurrentCellSelection);
			}

			if (CopiedForms)
				BGSEECONSOLE_MESSAGE("Pasted %d forms", CopiedForms);

			FreeBuffer();
		}

		void CSEGlobalClipboardOperator::FreeBuffer( void )
		{
			for (FormListT::iterator Itr = LoadedFormBuffer.begin(); Itr != LoadedFormBuffer.end(); Itr++)
				(*Itr)->DeleteInstance();

			LoadedFormBuffer.clear();
		}

		CSEFormListBuilder::CSEFormListBuilder() :
			FormList()
		{
			;//
		}

		CSEFormListBuilder::~CSEFormListBuilder()
		{
			for (BGSEditorExtender::BGSEEFormListT::iterator Itr = FormList.begin(); Itr != FormList.end(); Itr++)
				delete *Itr;

			FormList.clear();
		}

		void CSEFormListBuilder::Add( TESForm* Form )
		{
			SME_ASSERT(Form);

			FormList.push_back(new CSEFormWrapper(Form));
		}

		bool CSEFormListBuilder::Copy( void )
		{
			bool Result = false;
			
			if (FormList.size())
			{
				Result = BGSEECLIPBOARD->Copy(FormList);
			}

			return Result;
		}

		void Initialize( void )
		{
			bool ComponentInitialized = BGSEECLIPBOARD->Initialize(new CSEGlobalClipboardOperator(), new CSEPluginFileWrapper());

			SME_ASSERT(ComponentInitialized);
		}
	}
}