#include "CodaScriptCommands-BFC.h"

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			namespace BaseFormComponent
			{
				CodaScriptCommandRegistrarDef("Base Form Component");

				CodaScriptCommandPrototypeDef(GetBFCFullName);
				CodaScriptCommandPrototypeDef(GetBFCDescription);
				CodaScriptCommandPrototypeDef(GetBFCTexturePath);
				CodaScriptCommandPrototypeDef(GetBFCModelPath);
				CodaScriptCommandPrototypeDef(GetBFCModelBoundRadius);
				CodaScriptCommandPrototypeDef(GetBFCScript);
				CodaScriptCommandPrototypeDef(GetBFCUses);
				CodaScriptCommandPrototypeDef(GetBFCValue);
				CodaScriptCommandPrototypeDef(GetBFCHealth);
				CodaScriptCommandPrototypeDef(GetBFCWeight);
				CodaScriptCommandPrototypeDef(GetBFCQuality);
				CodaScriptCommandPrototypeDef(GetBFCAttackDamage);
				CodaScriptCommandPrototypeDef(GetBFCAttributes);
				CodaScriptCommandPrototypeDef(GetBFCSpellListEntries);
				CodaScriptCommandPrototypeDef(GetBFCLeveledListEntries);
				CodaScriptCommandPrototypeDef(GetBFCLeveledListChanceNone);
				CodaScriptCommandPrototypeDef(GetBFCContainerEntries);
				CodaScriptCommandPrototypeDef(GetBFCActorBaseDataFactionList);
				CodaScriptCommandPrototypeDef(GetBFCActorBaseDataMagicka);
				CodaScriptCommandPrototypeDef(GetBFCActorBaseDataFatigue);
				CodaScriptCommandPrototypeDef(GetBFCActorBaseDataBarterGold);
				CodaScriptCommandPrototypeDef(GetBFCActorBaseDataLevel);
				CodaScriptCommandPrototypeDef(GetBFCAIFormAIStats);
				CodaScriptCommandPrototypeDef(GetBFCAIFormAITrainingSkill);
				CodaScriptCommandPrototypeDef(GetBFCAIFormAITrainingLevel);
				CodaScriptCommandPrototypeDef(GetBFCAIFormAIPackageList);
				CodaScriptCommandPrototypeDef(GetBFCReactionFormReactionEntries);
				CodaScriptCommandPrototypeDef(GetBFCSoundFilePath);
				CodaScriptCommandPrototypeDef(GetBFCBipedModelSlotMask);
				CodaScriptCommandPrototypeDef(GetBFCBipedModelMaleBipedModelPath);
				CodaScriptCommandPrototypeDef(GetBFCBipedModelFemaleBipedModelPath);
				CodaScriptCommandPrototypeDef(GetBFCBipedModelMaleGroundModelPath);
				CodaScriptCommandPrototypeDef(GetBFCBipedModelFemaleGroundModelPath);
				CodaScriptCommandPrototypeDef(GetBFCBipedModelMaleIconPath);
				CodaScriptCommandPrototypeDef(GetBFCBipedModelFemaleIconPath);
				CodaScriptCommandPrototypeDef(GetBFCEnchantableFormEnchantment);
				CodaScriptCommandPrototypeDef(GetBFCEnchantableFormType);
				CodaScriptCommandPrototypeDef(GetBFCProduceFormIngredient);
				CodaScriptCommandPrototypeDef(GetBFCProduceFormChance);
				CodaScriptCommandPrototypeDef(GetBFCModelListModels);

				CodaScriptCommandPrototypeDef(SetBFCFullName);
				CodaScriptCommandPrototypeDef(SetBFCDescription);
				CodaScriptCommandPrototypeDef(SetBFCTexturePath);
				CodaScriptCommandPrototypeDef(SetBFCModelPath);
				CodaScriptCommandPrototypeDef(SetBFCModelBoundRadius);
				CodaScriptCommandPrototypeDef(SetBFCScript);
				CodaScriptCommandPrototypeDef(SetBFCUses);
				CodaScriptCommandPrototypeDef(SetBFCValue);
				CodaScriptCommandPrototypeDef(SetBFCHealth);
				CodaScriptCommandPrototypeDef(SetBFCWeight);
				CodaScriptCommandPrototypeDef(SetBFCQuality);
				CodaScriptCommandPrototypeDef(SetBFCAttackDamage);
				CodaScriptCommandPrototypeDef(SetBFCAttribute);
				CodaScriptCommandPrototypeDef(SetBFCLeveledListChanceNone);
				CodaScriptCommandPrototypeDef(SetBFCActorBaseDataMagicka);
				CodaScriptCommandPrototypeDef(SetBFCActorBaseDataFatigue);
				CodaScriptCommandPrototypeDef(SetBFCActorBaseDataBarterGold);
				CodaScriptCommandPrototypeDef(SetBFCActorBaseDataLevel);
				CodaScriptCommandPrototypeDef(SetBFCAIFormAIStats);
				CodaScriptCommandPrototypeDef(SetBFCAIFormAITrainingSkill);
				CodaScriptCommandPrototypeDef(SetBFCAIFormAITrainingLevel);
				CodaScriptCommandPrototypeDef(SetBFCSoundFilePath);
				CodaScriptCommandPrototypeDef(SetBFCBipedModelSlotMask);
				CodaScriptCommandPrototypeDef(SetBFCBipedModelMaleBipedModelPath);
				CodaScriptCommandPrototypeDef(SetBFCBipedModelFemaleBipedModelPath);
				CodaScriptCommandPrototypeDef(SetBFCBipedModelMaleGroundModelPath);
				CodaScriptCommandPrototypeDef(SetBFCBipedModelFemaleGroundModelPath);
				CodaScriptCommandPrototypeDef(SetBFCBipedModelMaleIconPath);
				CodaScriptCommandPrototypeDef(SetBFCBipedModelFemaleIconPath);
				CodaScriptCommandPrototypeDef(SetBFCEnchantableFormEnchantment);
				CodaScriptCommandPrototypeDef(SetBFCProduceFormIngredient);
				CodaScriptCommandPrototypeDef(SetBFCProduceFormChance);

				CodaScriptCommandParamData(SetBFCAttribute, 3)
				{
					{ "Form",					ICodaScriptDataStore::kDataType_Reference },
					{ "Attribute ID",			ICodaScriptDataStore::kDataType_Numeric },
					{ "Value",					ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(SetBFCAIFormAIStats, 3)
				{
					{ "Form",					ICodaScriptDataStore::kDataType_Reference },
					{ "AI Stat ID",				ICodaScriptDataStore::kDataType_Numeric },
					{ "Value",					ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(SetBFCAIFormAITrainingSkill, 2)
				{
					{ "Form",					ICodaScriptDataStore::kDataType_Reference },
					{ "Skill ID",				ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandParamData(SetBFCProduceFormChance, 3)
				{
					{ "Form",					ICodaScriptDataStore::kDataType_Reference },
					{ "Harvest Season ID",		ICodaScriptDataStore::kDataType_Numeric },
					{ "Value",					ICodaScriptDataStore::kDataType_Numeric }
				};

				CodaScriptCommandHandler(GetBFCFullName)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESFullName* Component = CS_CAST(Form, TESForm, TESFullName);
					if (Component)
						Result->SetString(Component->name.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCDescription)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESDescription* Component = CS_CAST(Form, TESForm, TESDescription);
					if (Component)
						Result->SetString(Component->description.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCTexturePath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESTexture* Component = CS_CAST(Form, TESForm, TESTexture);
					if (Component)
						Result->SetString(Component->texturePath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCModelPath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESModel* Component = CS_CAST(Form, TESForm, TESModel);
					if (Component)
						Result->SetString(Component->modelPath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCModelBoundRadius)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESModel* Component = CS_CAST(Form, TESForm, TESModel);
					if (Component)
						Result->SetNumber(Component->modelBound);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCScript)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESScriptableForm* Component = CS_CAST(Form, TESForm, TESScriptableForm);
					if (Component)
					{
						if (Component->script)
							Result->SetFormID(Component->script->formID);
						else
							Result->SetFormID(0);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCUses)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESUsesForm* Component = CS_CAST(Form, TESForm, TESUsesForm);
					if (Component)
						Result->SetNumber(Component->uses);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCValue)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESValueForm* Component = CS_CAST(Form, TESForm, TESValueForm);
					if (Component)
						Result->SetNumber(Component->goldValue);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCHealth)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESHealthForm* Component = CS_CAST(Form, TESForm, TESHealthForm);
					if (Component)
						Result->SetNumber(Component->health);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCWeight)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESWeightForm* Component = CS_CAST(Form, TESForm, TESWeightForm);
					if (Component)
						Result->SetNumber(Component->weight);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCQuality)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESQualityForm* Component = CS_CAST(Form, TESForm, TESQualityForm);
					if (Component)
						Result->SetNumber(Component->quality);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCAttackDamage)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAttackDamageForm* Component = CS_CAST(Form, TESForm, TESAttackDamageForm);
					if (Component)
						Result->SetNumber(Component->damage);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCAttributes)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAttributes* Component = CS_CAST(Form, TESForm, TESAttributes);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(8);
						SME_ASSERT(Array);

						for (int i = 0; i < 8; i++)
							Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->attributes[i]);

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCSpellListEntries)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESSpellList* Component = CS_CAST(Form, TESForm, TESSpellList);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(Component->spells.Count());
						SME_ASSERT(Array);

						for (int i = 0; i < Component->spells.Count(); i++)
							Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Component->spells.GetNthItem(i)->formID);

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCLeveledListEntries)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESLeveledList* Component = CS_CAST(Form, TESForm, TESLeveledList);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(Component->levList.Count());
						SME_ASSERT(Array);

						for (int i = 0; i < Component->levList.Count(); i++)
						{
							ICodaScriptDataStore* SubArray = Utilities->ArrayAllocate(3);
							SME_ASSERT(SubArray);

							Utilities->ArrayPushback(SubArray, (CodaScriptReferenceDataTypeT)Component->levList.GetNthItem(i)->form->formID);
							Utilities->ArrayPushback(SubArray, (CodaScriptNumericDataTypeT)Component->levList.GetNthItem(i)->level);
							Utilities->ArrayPushback(SubArray, (CodaScriptNumericDataTypeT)Component->levList.GetNthItem(i)->count);

							Utilities->ArrayPushback(Array, SubArray);
						}

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCLeveledListChanceNone)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESLeveledList* Component = CS_CAST(Form, TESForm, TESLeveledList);
					if (Component)
						Result->SetNumber(Component->chanceNone);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCContainerEntries)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESContainer* Component = CS_CAST(Form, TESForm, TESContainer);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(Component->contents.Count());
						SME_ASSERT(Array);

						for (int i = 0; i < Component->contents.Count(); i++)
						{
							ICodaScriptDataStore* SubArray = Utilities->ArrayAllocate(2);
							SME_ASSERT(SubArray);

							Utilities->ArrayPushback(SubArray, (CodaScriptReferenceDataTypeT)Component->contents.GetNthItem(i)->form->formID);
							Utilities->ArrayPushback(SubArray, (CodaScriptNumericDataTypeT)Component->contents.GetNthItem(i)->count);

							Utilities->ArrayPushback(Array, SubArray);
						}

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCActorBaseDataFactionList)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(Component->factionList.Count());
						SME_ASSERT(Array);

						for (int i = 0; i < Component->factionList.Count(); i++)
						{
							ICodaScriptDataStore* SubArray = Utilities->ArrayAllocate(2);
							SME_ASSERT(SubArray);

							Utilities->ArrayPushback(SubArray, (CodaScriptReferenceDataTypeT)Component->factionList.GetNthItem(i)->faction->formID);
							Utilities->ArrayPushback(SubArray, (CodaScriptNumericDataTypeT)Component->factionList.GetNthItem(i)->rank);

							Utilities->ArrayPushback(Array, SubArray);
						}

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCActorBaseDataMagicka)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
						Result->SetNumber(Component->magicka);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCActorBaseDataFatigue)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
						Result->SetNumber(Component->fatigue);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCActorBaseDataBarterGold)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
						Result->SetNumber(Component->barterGold);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCActorBaseDataLevel)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(3);
						SME_ASSERT(Array);

						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->level);
						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->minLevel);
						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->maxLevel);

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCAIFormAIStats)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(4);
						SME_ASSERT(Array);

						for (int i = 0; i < TESAIForm::kAIStat__MAX; i++)
							Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->aiStats[i]);

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCAIFormAITrainingSkill)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
					if (Component)
						Result->SetNumber(Component->trainingSkill);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCAIFormAITrainingLevel)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
					if (Component)
						Result->SetNumber(Component->trainingLevel);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCAIFormAIPackageList)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(Component->packages.Count());
						SME_ASSERT(Array);

						for (int i = 0; i < Component->packages.Count(); i++)
							Utilities->ArrayPushback(Array, (CodaScriptReferenceDataTypeT)Component->packages.GetNthItem(i)->formID);

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCReactionFormReactionEntries)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESReactionForm* Component = CS_CAST(Form, TESForm, TESReactionForm);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(Component->reactionList.Count());
						SME_ASSERT(Array);

						for (int i = 0; i < Component->reactionList.Count(); i++)
						{
							ICodaScriptDataStore* SubArray = Utilities->ArrayAllocate(2);
							SME_ASSERT(SubArray);

							Utilities->ArrayPushback(SubArray, (CodaScriptReferenceDataTypeT)Component->reactionList.GetNthItem(i)->target->formID);
							Utilities->ArrayPushback(SubArray, (CodaScriptNumericDataTypeT)Component->reactionList.GetNthItem(i)->reaction);

							Utilities->ArrayPushback(Array, SubArray);
						}

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCSoundFilePath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESSoundFile* Component = CS_CAST(Form, TESForm, TESSoundFile);
					if (Component)
						Result->SetString(Component->soundFilePath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCBipedModelSlotMask)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
						Result->SetNumber(Component->slotMask);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCBipedModelMaleBipedModelPath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
						Result->SetString(Component->maleBipedModel.modelPath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCBipedModelFemaleBipedModelPath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
						Result->SetString(Component->femaleBipedModel.modelPath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCBipedModelMaleGroundModelPath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
						Result->SetString(Component->maleGroundModel.modelPath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCBipedModelFemaleGroundModelPath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
						Result->SetString(Component->femaleGroundModel.modelPath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCBipedModelMaleIconPath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
						Result->SetString(Component->maleIcon.texturePath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCBipedModelFemaleIconPath)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
						Result->SetString(Component->femaleIcon.texturePath.c_str());
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCEnchantableFormEnchantment)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESEnchantableForm* Component = CS_CAST(Form, TESForm, TESEnchantableForm);
					if (Component)
					{
						if (Component->enchantment)
							Result->SetFormID(Component->enchantment->formID);
						else
							Result->SetFormID(0);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCEnchantableFormType)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESEnchantableForm* Component = CS_CAST(Form, TESForm, TESEnchantableForm);
					if (Component)
						Result->SetNumber(Component->enchantmentType);
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCProduceFormIngredient)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESProduceForm* Component = CS_CAST(Form, TESForm, TESProduceForm);
					if (Component)
					{
						if (Component->ingredient)
							Result->SetFormID(Component->ingredient->formID);
						else
							Result->SetFormID(0);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCProduceFormChance)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESProduceForm* Component = CS_CAST(Form, TESForm, TESProduceForm);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(4);
						SME_ASSERT(Array);

						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->springHarvestChance);
						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->summerHarvestChance);
						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->fallHarvestChance);
						Utilities->ArrayPushback(Array, (CodaScriptNumericDataTypeT)Component->winterHarvestChance);

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(GetBFCModelListModels)
				{
					TESForm* Form = NULL;

					CodaScriptCommandExtractArgs(&Form);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESModelList* Component = CS_CAST(Form, TESForm, TESModelList);
					if (Component)
					{
						ICodaScriptDataStore* Array = Utilities->ArrayAllocate(Component->modelList.Count());
						SME_ASSERT(Array);

						for (int i = 0; i < Component->modelList.Count(); i++)
							Utilities->ArrayPushback(Array, Component->modelList.GetNthItem(i));

						Result->SetArray(Array);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCFullName)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESFullName* Component = CS_CAST(Form, TESForm, TESFullName);
					if (Component)
					{
						Component->name.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCDescription)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESDescription* Component = CS_CAST(Form, TESForm, TESDescription);
					if (Component)
					{
						Component->description.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCTexturePath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESTexture* Component = CS_CAST(Form, TESForm, TESTexture);
					if (Component)
					{
						Component->texturePath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCModelPath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESModel* Component = CS_CAST(Form, TESForm, TESModel);
					if (Component)
					{
						Component->modelPath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCModelBoundRadius)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESModel* Component = CS_CAST(Form, TESForm, TESModel);
					if (Component)
					{
						Component->modelBound = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCScript)
				{
					TESForm* Form = NULL;
					TESForm* Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(2, &Form, &Buffer);

					if (Form == NULL)
						return false;

					TESScriptableForm* Component = CS_CAST(Form, TESForm, TESScriptableForm);
					if (Component)
					{
						Script* NewScript = CS_CAST(Buffer, TESForm, Script);
						if (!NewScript)
							return false;

						Component->script = NewScript;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCUses)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESUsesForm* Component = CS_CAST(Form, TESForm, TESUsesForm);
					if (Component)
					{
						Component->uses = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCValue)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESValueForm* Component = CS_CAST(Form, TESForm, TESValueForm);
					if (Component)
					{
						Component->goldValue = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCHealth)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESHealthForm* Component = CS_CAST(Form, TESForm, TESHealthForm);
					if (Component)
					{
						Component->health = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCWeight)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESWeightForm* Component = CS_CAST(Form, TESForm, TESWeightForm);
					if (Component)
					{
						Component->weight = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCQuality)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESQualityForm* Component = CS_CAST(Form, TESForm, TESQualityForm);
					if (Component)
					{
						if (Buffer > 255)	Buffer = 255;
						Component->quality = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCAttackDamage)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAttackDamageForm* Component = CS_CAST(Form, TESForm, TESAttackDamageForm);
					if (Component)
					{
						Component->damage = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCAttribute)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = 0;
					CodaScriptNumericDataTypeT Attribute = 0;

					CodaScriptCommandExtractArgs(&Form, &Attribute, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					if (Attribute >= TESAttributes::kAttribute__MAX)
						return false;

					TESAttributes* Component = CS_CAST(Form, TESForm, TESAttributes);
					if (Component)
					{
						if (Buffer > 255)		Buffer = 255;
						Component->attributes[(int)Attribute] = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCLeveledListChanceNone)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESLeveledList* Component = CS_CAST(Form, TESForm, TESLeveledList);
					if (Component)
					{
						if (Buffer > 100)		Buffer = 100;
						Component->chanceNone = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCActorBaseDataMagicka)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
					{
						Component->magicka = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCActorBaseDataFatigue)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
					{
						Component->fatigue = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCActorBaseDataBarterGold)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
					{
						Component->barterGold = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCActorBaseDataLevel)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
					if (Component)
					{
						Component->level = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCAIFormAIStats)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = 0;
					CodaScriptNumericDataTypeT AIStat = 0;

					CodaScriptCommandExtractArgs(&Form, &AIStat, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;
					else if (AIStat >= TESAIForm::kAIStat__MAX)
						return false;

					TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
					if (Component)
					{
						if (Buffer > 255)		Buffer = 255;
						Component->aiStats[(int)AIStat] = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCAIFormAITrainingSkill)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;
					else if (Buffer >= ActorValues::kActorVal__MAX_Skill || Buffer < ActorValues::kActorVal_Armorer)
						return false;

					TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
					if (Component)
					{
						Component->trainingSkill = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCAIFormAITrainingLevel)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
					if (Component)
					{
						if (Buffer > 255)		Buffer = 255;
						Component->trainingLevel = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCSoundFilePath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESSoundFile* Component = CS_CAST(Form, TESForm, TESSoundFile);
					if (Component)
					{
						Component->soundFilePath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCBipedModelSlotMask)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
					{
						Component->slotMask = Buffer;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCBipedModelMaleBipedModelPath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
					{
						Component->maleBipedModel.modelPath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCBipedModelFemaleBipedModelPath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
					{
						Component->femaleBipedModel.modelPath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCBipedModelMaleGroundModelPath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
					{
						Component->maleGroundModel.modelPath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCBipedModelFemaleGroundModelPath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
					{
						Component->femaleGroundModel.modelPath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCBipedModelMaleIconPath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
					{
						Component->maleIcon.texturePath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCBipedModelFemaleIconPath)
				{
					TESForm* Form = NULL;
					CodaScriptStringParameterTypeT Buffer = NULL;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;

					TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
					if (Component)
					{
						Component->femaleIcon.texturePath.Set(Buffer);
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCEnchantableFormEnchantment)
				{
					TESForm* Form = NULL;
					TESForm* Buffer = 0;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(2, &Form, &Buffer);

					if (Form == NULL)
						return false;

					TESEnchantableForm* Component = CS_CAST(Form, TESForm, TESEnchantableForm);
					if (Component)
					{
						EnchantmentItem* NewEnchantment = CS_CAST(Buffer, TESForm, EnchantmentItem);
						if (!NewEnchantment)
							return false;
						else if (NewEnchantment->enchType != Component->enchantmentType)
							return false;

						Component->enchantment = NewEnchantment;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCProduceFormIngredient)
				{
					TESForm* Form = NULL;
					TESForm* Buffer = 0;

					CodaScriptCommandExtractArgs(&Form, &Buffer);
					ExtractFormArguments(2, &Form, &Buffer);

					if (Form == NULL)
						return false;

					TESProduceForm* Component = CS_CAST(Form, TESForm, TESProduceForm);
					if (Component)
					{
						IngredientItem* NewIngredient = CS_CAST(Buffer, TESForm, IngredientItem);
						if (!NewIngredient)
							return false;

						Component->ingredient = NewIngredient;
					}
					else
						return false;

					return true;
				}

				CodaScriptCommandHandler(SetBFCProduceFormChance)
				{
					TESForm* Form = NULL;
					CodaScriptNumericDataTypeT Buffer = 0;
					CodaScriptNumericDataTypeT Season = 0;

					CodaScriptCommandExtractArgs(&Form, &Season, &Buffer);
					ExtractFormArguments(1, &Form);

					if (Form == NULL)
						return false;
					else if (Season > 3)
						return false;

					TESProduceForm* Component = CS_CAST(Form, TESForm, TESProduceForm);
					if (Component)
					{
						if (Buffer > 100)		Buffer = 100;
						switch ((int)Season)
						{
						case 0:
							Component->springHarvestChance = Buffer;
							break;
						case 1:
							Component->summerHarvestChance = Buffer;
							break;
						case 2:
							Component->fallHarvestChance = Buffer;
							break;
						case 3:
							Component->winterHarvestChance = Buffer;
							break;
						}
					}
					else
						return false;

					return true;
				}
			}
		}
	}
}