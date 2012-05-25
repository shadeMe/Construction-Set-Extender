#include "CodaScriptCommands-Form.h"

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			namespace Form
			{
				CodaScriptCommandRegistrarDef("Form")

				CodaScriptCommandPrototypeDef(GetFormByEditorID);
				CodaScriptCommandPrototypeDef(GetFormByFormID);
				CodaScriptCommandPrototypeDef(GetEditorID);
				CodaScriptCommandPrototypeDef(GetFormType);
				CodaScriptCommandPrototypeDef(SetEditorID);
				CodaScriptCommandPrototypeDef(SetFormID);
				CodaScriptCommandPrototypeDef(MarkAsModified);
				CodaScriptCommandPrototypeDef(GetDataHandlerFormList);

				CodaScriptCommandParamData(SetEditorID, 2)
				{
					{ "Form",							ICodaScriptDataStore::kDataType_Reference },
					{ "EditorID",						ICodaScriptDataStore::kDataType_String }
				};

				CodaScriptCommandParamData(SetFormID, 2)
				{
					{ "Form",							ICodaScriptDataStore::kDataType_Reference },
					{ "FormID",							ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(MarkAsModified, 2)
				{
					{ "Form",							ICodaScriptDataStore::kDataType_Reference },
					{ "Modified Flag",					ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(GetDataHandlerFormList, 1)
				{
					{ "Form Type",						ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandHandler(GetFormByEditorID)
				{
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Buffer);

					TESForm* Form = TESForm::LookupByEditorID(Buffer);
					if (Form)
						Result->SetFormID(Form->formID);
					else
						Result->SetFormID(0);

					return true;
				}

				CodaScriptCommandHandler(GetFormByFormID)
				{
					CodaScriptNumericDataTypeT FormID = 0;

					CodaScriptCommandExtractArgs(&FormID);

					TESForm* Form = TESForm::LookupByFormID((UInt32)FormID);
					if (Form)
						Result->SetFormID(Form->formID);
					else
						Result->SetFormID(0);

					return true;
				}

				CodaScriptCommandHandler(GetEditorID)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					Result->SetString((Form->editorID.c_str())?Form->editorID.c_str():"");
					return true;
				}

				CodaScriptCommandHandler(GetFormType)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					Result->SetNumber(Form->formType);
					return true;
				}

				CodaScriptCommandHandler(SetEditorID)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					if (Buffer)
						Result->SetNumber(Form->SetEditorID(Buffer));

					return true;
				}

				CodaScriptCommandHandler(SetFormID)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT FormID = 0;

					CodaScriptCommandExtractArgs(&Form, &FormID);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					if (FormID && Form->formID != FormID)
						Form->SetFormID(FormID);

					return true;
				}

				CodaScriptCommandHandler(MarkAsModified)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT State = 0;

					CodaScriptCommandExtractArgs(&Form, &State);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					Form->SetFromActiveFile(State);

					return true;
				}

				template <typename T>
				void AddFormToCodaArray(tList<T>* List, ICodaScriptDataStore* Array, ICodaScriptCommandHandlerHelper* Utilities)
				{
					for (tList<T>::Iterator Itr = List->Begin(); !Itr.End() && Itr.Get(); ++Itr)
						Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Itr->formID);
				}

				CodaScriptCommandHandler(GetDataHandlerFormList)
				{
					CodaScriptNumericDataTypeT FormType = 0;

					CodaScriptCommandExtractArgs(&FormType);

					if (FormType < TESForm::kFormType_GMST || FormType > TESForm::kFormType_TOFT)
						return false;

					ICodaScriptDataStore* Array = Utilities->ArrayAllocate(200);
					SME_ASSERT(Array);

					switch ((UInt32)FormType)
					{
					case TESForm::kFormType_Global:
						AddFormToCodaArray(&_DATAHANDLER->globals, Array, Utilities);
						break;
					case TESForm::kFormType_Class:
						AddFormToCodaArray(&_DATAHANDLER->classes, Array, Utilities);
						break;
					case TESForm::kFormType_Faction:
						AddFormToCodaArray(&_DATAHANDLER->factions, Array, Utilities);
						break;
					case TESForm::kFormType_Hair:
						AddFormToCodaArray(&_DATAHANDLER->hairs, Array, Utilities);
						break;
					case TESForm::kFormType_Eyes:
						AddFormToCodaArray(&_DATAHANDLER->eyes, Array, Utilities);
						break;
					case TESForm::kFormType_Race:
						AddFormToCodaArray(&_DATAHANDLER->races, Array, Utilities);
						break;
					case TESForm::kFormType_Script:
						AddFormToCodaArray(&_DATAHANDLER->scripts, Array, Utilities);
						break;
					case TESForm::kFormType_LandTexture:
						AddFormToCodaArray(&_DATAHANDLER->landTextures, Array, Utilities);
						break;
					case TESForm::kFormType_Enchantment:
						AddFormToCodaArray(&_DATAHANDLER->enchantmentItems, Array, Utilities);
						break;
					case TESForm::kFormType_Spell:
						AddFormToCodaArray(&_DATAHANDLER->spellItems, Array, Utilities);
						break;
					case TESForm::kFormType_BirthSign:
						AddFormToCodaArray(&_DATAHANDLER->birthsigns, Array, Utilities);
						break;
					case TESForm::kFormType_Weather:
						AddFormToCodaArray(&_DATAHANDLER->weathers, Array, Utilities);
						break;
					case TESForm::kFormType_Climate:
						AddFormToCodaArray(&_DATAHANDLER->climates, Array, Utilities);
						break;
					case TESForm::kFormType_WorldSpace:
						AddFormToCodaArray(&_DATAHANDLER->worldSpaces, Array, Utilities);
						break;
					case TESForm::kFormType_Topic:
						AddFormToCodaArray(&_DATAHANDLER->topics, Array, Utilities);
						break;
					case TESForm::kFormType_Quest:
						AddFormToCodaArray(&_DATAHANDLER->quests, Array, Utilities);
						break;
					case TESForm::kFormType_AnimObject:
						AddFormToCodaArray(&_DATAHANDLER->objectAnios, Array, Utilities);
						break;
					case TESForm::kFormType_CombatStyle:
						AddFormToCodaArray(&_DATAHANDLER->combatStyles, Array, Utilities);
						break;
					case TESForm::kFormType_LoadScreen:
						AddFormToCodaArray(&_DATAHANDLER->loadScreens, Array, Utilities);
						break;
					case TESForm::kFormType_WaterForm:
						AddFormToCodaArray(&_DATAHANDLER->waterForms, Array, Utilities);
						break;
					case TESForm::kFormType_EffectShader:
						AddFormToCodaArray(&_DATAHANDLER->effectShaders, Array, Utilities);
						break;
					case TESForm::kFormType_Package:
						AddFormToCodaArray(&_DATAHANDLER->packages, Array, Utilities);
						break;
					case TESForm::kFormType_GMST:
						break;
					case TESForm::kFormType_Skill:
						for (int i = 0; i < 0x15; i++)
						{
							TESSkill* Skill = &_DATAHANDLER->skills[i];
							Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Skill->formID);
						}

						break;
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
					case TESForm::kFormType_LeveledSpell:
						for (TESObject* Itr = _DATAHANDLER->objects->first; Itr; Itr = Itr->next)
						{
							if (Itr->formType == FormType)
								Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Itr->formID);
						}

						break;
					case TESForm::kFormType_Cell:
						break;
					case TESForm::kFormType_Idle:
						break;
					}

					Result->SetArray(Array);

					return true;
				}
			}
		}
	}
}