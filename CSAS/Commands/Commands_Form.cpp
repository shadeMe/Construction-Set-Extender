#include "Commands_Form.h"
#include "..\ScriptCommands.h"
#include "..\ScriptRunner.h"

namespace CSAutomationScript
{
	void RegisterFormCommands()
	{
		REGISTER_CSASCOMMAND(GetBFCFullName, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCDescription, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCTexturePath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCModelBoundRadius, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCScript, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCUses, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCValue, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCHealth, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCWeight, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCQuality, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCAttackDamage, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCAttributes, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCSpellListEntries, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCLeveledListEntries, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCLeveledListChanceNone, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCContainerEntries, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataFactionList, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataMagicka, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataFatigue, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataBarterGold, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataLevel, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCAIFormAIStats, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCAIFormAITrainingSkill, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCAIFormAITrainingLevel, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCAIFormAIPackageList, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCReactionFormReactionEntries, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCSoundFilePath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCBipedModelSlotMask, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCBipedModelMaleBipedModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCBipedModelFemaleBipedModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCBipedModelMaleGroundModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCBipedModelFemaleGroundModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCBipedModelMaleIconPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCBipedModelFemaleIconPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCEnchantableFormEnchantment, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCEnchantableFormType, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCProduceFormIngredient, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCProduceFormChance, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(GetBFCModelListModels, "Base Form Component Functions");

		REGISTER_CSASCOMMAND(SetBFCFullName, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCDescription, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCTexturePath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCModelBoundRadius, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCScript, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCUses, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCValue, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCHealth, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCWeight, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCQuality, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCAttackDamage, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCAttribute, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCLeveledListChanceNone, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataMagicka, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataFatigue, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataBarterGold, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataLevel, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCAIFormAIStats, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCAIFormAITrainingSkill, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCAIFormAITrainingLevel, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCSoundFilePath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCBipedModelSlotMask, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCBipedModelMaleBipedModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCBipedModelFemaleBipedModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCBipedModelMaleGroundModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCBipedModelFemaleGroundModelPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCBipedModelMaleIconPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCBipedModelFemaleIconPath, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCEnchantableFormEnchantment, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCProduceFormIngredient, "Base Form Component Functions");
		REGISTER_CSASCOMMAND(SetBFCProduceFormChance, "Base Form Component Functions");
	}

	BEGIN_CSASCOMMAND_HANDLER(GetBFCFullName)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESFullName* Component = CS_CAST(Form, TESForm, TESFullName);
		if (Component)
			Result->SetString(Component->name.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCFullName, "Returns the full name component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCDescription)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESDescription* Component = CS_CAST(Form, TESForm, TESDescription);
		if (Component)
			Result->SetString(Component->description.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCDescription, "Returns the description component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCTexturePath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESTexture* Component = CS_CAST(Form, TESForm, TESTexture);
		if (Component)
			Result->SetString(Component->texturePath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCTexturePath, "Returns the texture path component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCModelPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESModel* Component = CS_CAST(Form, TESForm, TESModel);
		if (Component)
			Result->SetString(Component->modelPath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCModelPath, "Returns the model path component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCModelBoundRadius)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESModel* Component = CS_CAST(Form, TESForm, TESModel);
		if (Component)
			Result->SetNumber(Component->modelBound);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCModelBoundRadius, "Returns the model bound radius component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCScript)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESScriptableForm* Component = CS_CAST(Form, TESForm, TESScriptableForm);
		if (Component)
			Result->SetForm(Component->script);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCScript, "Returns the script component of the passed form", CSASDataElement::kParamType_Reference, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCUses)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESUsesForm* Component = CS_CAST(Form, TESForm, TESUsesForm);
		if (Component)
			Result->SetNumber(Component->uses);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCUses, "Returns the uses component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCValue)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESValueForm* Component = CS_CAST(Form, TESForm, TESValueForm);
		if (Component)
			Result->SetNumber(Component->goldValue);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCValue, "Returns the gold value component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCHealth)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESHealthForm* Component = CS_CAST(Form, TESForm, TESHealthForm);
		if (Component)
			Result->SetNumber(Component->health);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCHealth, "Returns the health component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCWeight)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESWeightForm* Component = CS_CAST(Form, TESForm, TESWeightForm);
		if (Component)
			Result->SetNumber(Component->weight);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCWeight, "Returns the weight component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCQuality)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESQualityForm* Component = CS_CAST(Form, TESForm, TESQualityForm);
		if (Component)
			Result->SetNumber(Component->quality);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCQuality, "Returns the quality component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAttackDamage)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESAttackDamageForm* Component = CS_CAST(Form, TESForm, TESAttackDamageForm);
		if (Component)
			Result->SetNumber(Component->damage);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCAttackDamage, "Returns the attack damage component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAttributes)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESAttributes* Component = CS_CAST(Form, TESForm, TESAttributes);
		if (Component)
		{
			CSASDataElement ArrayData[8];
			for (int i = 0; i < 8; i++)
				ArrayData[i].SetNumber(Component->attributes[i]);

			Result->SetArray(ArrayData, sizeof(ArrayData));
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCAttributes, "Returns the attributes component of the passed form", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCSpellListEntries)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESSpellList* Component = CS_CAST(Form, TESForm, TESSpellList);
		if (Component)
		{
			CSASDataElement* ArrayData = new CSASDataElement[Component->spells.Count()];
			for (int i = 0; i < Component->spells.Count(); i++)
				ArrayData[i].SetForm(Component->spells.GetNthItem(i));

			Result->SetArray(ArrayData, sizeof(ArrayData));
			delete [] ArrayData;
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCSpellListEntries, "Returns the spell list component of the passed form", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCLeveledListEntries)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESLeveledList* Component = CS_CAST(Form, TESForm, TESLeveledList);
		if (Component)
		{
			CSASDataElement* ArrayData = new CSASDataElement[Component->levList.Count()];

			for (int i = 0; i < Component->levList.Count(); i++)
			{
				CSASDataElement SubArrayData1[3];
				SubArrayData1[0].SetForm(Component->levList.GetNthItem(i)->form);
				SubArrayData1[1].SetNumber(Component->levList.GetNthItem(i)->level);
				SubArrayData1[2].SetNumber(Component->levList.GetNthItem(i)->count);

				ArrayData[i].SetArray(SubArrayData1, sizeof(SubArrayData1));
			}

			Result->SetArray(ArrayData, sizeof(ArrayData));
			delete [] ArrayData;
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCLeveledListEntries, "Returns the leveled list component of the passed form as an array of arrays. Subitems are of the following structure: Leveled List Item, Level, Count", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCLeveledListChanceNone)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESLeveledList* Component = CS_CAST(Form, TESForm, TESLeveledList);
		if (Component)
			Result->SetNumber(Component->chanceNone);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCLeveledListChanceNone, "Returns the 'chance none' attribute of leveled list component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCContainerEntries)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESContainer* Component = CS_CAST(Form, TESForm, TESContainer);
		if (Component)
		{
			CSASDataElement* ArrayData = new CSASDataElement[Component->contents.Count()];
			for (int i = 0; i < Component->contents.Count(); i++)
			{
				CSASDataElement SubArrayData1[2];
				SubArrayData1[0].SetForm(Component->contents.GetNthItem(i)->form);
				SubArrayData1[1].SetNumber(Component->contents.GetNthItem(i)->count);

				ArrayData[i].SetArray(SubArrayData1, sizeof(SubArrayData1));
			}

			Result->SetArray(ArrayData, sizeof(ArrayData));
			delete [] ArrayData;
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCContainerEntries, "Returns the container component of the passed form as an array of arrays. Subitems are of the following structure: Inventory Item, Count", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCActorBaseDataFactionList)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
		if (Component)
		{
			CSASDataElement* ArrayData = new CSASDataElement[Component->factionList.Count()];
			for (int i = 0; i < Component->factionList.Count(); i++)
			{
				CSASDataElement SubArrayData1[2];
				SubArrayData1[0].SetForm(Component->factionList.GetNthItem(i)->faction);
				SubArrayData1[1].SetNumber(Component->factionList.GetNthItem(i)->rank);

				ArrayData[i].SetArray(SubArrayData1, sizeof(SubArrayData1));
			}

			Result->SetArray(ArrayData, sizeof(ArrayData));
			delete [] ArrayData;
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCActorBaseDataFactionList, "Returns the 'faction list' attribute of actor basedata component the passed form as an array of arrays. Subitems are of the following structure: Faction, Rank", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCActorBaseDataMagicka)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
		if (Component)
			Result->SetNumber(Component->magicka);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCActorBaseDataMagicka, "Returns the 'magicka' attribute of actor basedata component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCActorBaseDataFatigue)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
		if (Component)
			Result->SetNumber(Component->fatigue);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCActorBaseDataFatigue, "Returns the 'fatigue' attribute of actor basedata component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCActorBaseDataBarterGold)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
		if (Component)
			Result->SetNumber(Component->barterGold);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCActorBaseDataBarterGold, "Returns the 'barter gold' attribute of actor basedata component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCActorBaseDataLevel)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESActorBaseData* Component = CS_CAST(Form, TESForm, TESActorBaseData);
		if (Component)
		{
			CSASDataElement ArrayData[3];
			ArrayData[0].SetNumber(Component->level);
			ArrayData[1].SetNumber(Component->minLevel);
			ArrayData[2].SetNumber(Component->maxLevel);

			Result->SetArray(ArrayData, sizeof(ArrayData));
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCActorBaseDataLevel, "Returns the 'level' attributes of actor basedata component of the passed form as an array with the following structure: Level, MinLevel, MaxLevel", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAIFormAIStats)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
		if (Component)
		{
			CSASDataElement ArrayData[4];
			for (int i = 0; i < TESAIForm::kAIStat__MAX; i++)
				ArrayData[i].SetNumber(Component->aiStats[i]);

			Result->SetArray(ArrayData, sizeof(ArrayData));
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCAIFormAIStats, "Returns the 'aistats' attributes of AI component of the passed form as an array with the following structure: Agression, Confidence, Energy, Responsibility", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAIFormAITrainingSkill)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
		if (Component)
			Result->SetNumber(Component->trainingSkill);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCAIFormAITrainingSkill, "Returns the 'training skill' attribute of AI component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAIFormAITrainingLevel)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
		if (Component)
			Result->SetNumber(Component->trainingLevel);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCAIFormAITrainingLevel, "Returns the 'training level' attribute of AI component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAIFormAIPackageList)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESAIForm* Component = CS_CAST(Form, TESForm, TESAIForm);
		if (Component)
		{
			CSASDataElement* ArrayData = new CSASDataElement[Component->packages.Count()];
			for (int i = 0; i < Component->packages.Count(); i++)
				ArrayData[i].SetForm(Component->packages.GetNthItem(i));

			Result->SetArray(ArrayData, sizeof(ArrayData));
			delete [] ArrayData;
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCAIFormAIPackageList, "Returns the 'package list' attribute of AI component of the passed form", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCReactionFormReactionEntries)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESReactionForm* Component = CS_CAST(Form, TESForm, TESReactionForm);
		if (Component)
		{
			CSASDataElement* ArrayData = new CSASDataElement[Component->reactionList.Count()];
			for (int i = 0; i < Component->reactionList.Count(); i++)
			{
				CSASDataElement SubArrayData1[2];
				SubArrayData1[0].SetForm(Component->reactionList.GetNthItem(i)->target);
				SubArrayData1[1].SetNumber(Component->reactionList.GetNthItem(i)->reaction);

				ArrayData[i].SetArray(SubArrayData1, sizeof(SubArrayData1));
			}

			Result->SetArray(ArrayData, sizeof(ArrayData));
			delete [] ArrayData;
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCReactionFormReactionEntries, "Returns the faction reaction component the passed form as an array of arrays. Subitems are of the following structure: Faction, Reaction", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCSoundFilePath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESSoundFile* Component = CS_CAST(Form, TESForm, TESSoundFile);
		if (Component)
			Result->SetString(Component->soundFilePath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCSoundFilePath, "Returns the sound file path component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCBipedModelSlotMask)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
		if (Component)
			Result->SetNumber(Component->slotMask);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCBipedModelSlotMask, "Returns the 'slot mask' attribute of biped model component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCBipedModelMaleBipedModelPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
		if (Component)
			Result->SetString(Component->maleBipedModel.modelPath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCBipedModelMaleBipedModelPath, "Returns the 'male biped model path' attribute of biped model component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCBipedModelFemaleBipedModelPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
		if (Component)
			Result->SetString(Component->femaleBipedModel.modelPath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCBipedModelFemaleBipedModelPath, "Returns the 'female biped model path' attribute of biped model component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCBipedModelMaleGroundModelPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
		if (Component)
			Result->SetString(Component->maleGroundModel.modelPath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCBipedModelMaleGroundModelPath, "Returns the 'male ground model path' attribute of biped model component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCBipedModelFemaleGroundModelPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
		if (Component)
			Result->SetString(Component->femaleGroundModel.modelPath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCBipedModelFemaleGroundModelPath, "Returns the 'female ground model path' attribute of biped model component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCBipedModelMaleIconPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
		if (Component)
			Result->SetString(Component->maleIcon.texturePath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCBipedModelMaleIconPath, "Returns the 'male icon path' attribute of biped model component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCBipedModelFemaleIconPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESBipedModelForm* Component = CS_CAST(Form, TESForm, TESBipedModelForm);
		if (Component)
			Result->SetString(Component->femaleIcon.texturePath.c_str());
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCBipedModelFemaleIconPath, "Returns the 'female icon path' attribute of biped model component of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCEnchantableFormEnchantment)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESEnchantableForm* Component = CS_CAST(Form, TESForm, TESEnchantableForm);
		if (Component)
			Result->SetForm(Component->enchantment);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCEnchantableFormEnchantment, "Returns the 'enchantment' attribute of enchantment component of the passed form", CSASDataElement::kParamType_Reference, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCEnchantableFormType)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESEnchantableForm* Component = CS_CAST(Form, TESForm, TESEnchantableForm);
		if (Component)
			Result->SetNumber(Component->enchantmentType);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCEnchantableFormType, "Returns the 'enchantment type' attribute of enchantment component of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCProduceFormIngredient)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESProduceForm* Component = CS_CAST(Form, TESForm, TESProduceForm);
		if (Component)
			Result->SetForm(Component->ingredient);
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCProduceFormIngredient, "Returns the 'ingredient' attribute of produce component of the passed form", CSASDataElement::kParamType_Reference, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCProduceFormChance)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESProduceForm* Component = CS_CAST(Form, TESForm, TESProduceForm);
		if (Component)
		{
			CSASDataElement ArrayData[4];
			ArrayData[0].SetNumber(Component->springHarvestChance);
			ArrayData[1].SetNumber(Component->summerHarvestChance);
			ArrayData[2].SetNumber(Component->fallHarvestChance);
			ArrayData[3].SetNumber(Component->winterHarvestChance);

			Result->SetArray(ArrayData, sizeof(ArrayData));
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCProduceFormChance, "Returns the 'harvest chance' attributes of produce component of the passed form as an array with the following structure: Spring, Summer, Fall, Winter", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCModelListModels)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESModelList* Component = CS_CAST(Form, TESForm, TESModelList);
		if (Component)
		{
			CSASDataElement* ArrayData = new CSASDataElement[Component->modelList.Count()];
			for (int i = 0; i < Component->modelList.Count(); i++)
				ArrayData[i].SetString(Component->modelList.GetNthItem(i));

			Result->SetArray(ArrayData, sizeof(ArrayData));
			delete [] ArrayData;
		}
		else
			return false;

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetBFCModelListModels, "Returns the model list component of the passed form", CSASDataElement::kParamType_Array, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCFullName)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCFullName, "Sets the passed form's full name component's value", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCDescription)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCDescription, "Sets the passed form's description component's value", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCTexturePath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCTexturePath, "Sets the passed form's texture path component's value", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCModelPath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCModelPath, "Sets the passed form's model component's path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCModelBoundRadius)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCModelBoundRadius, "Sets the passed form's model component's bound radius", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCScript)
	{
		TESForm* Form = NULL;
		TESForm* Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCScript, "Sets the passed form's script component's value", CSASDataElement::kParamType_Invalid, kParams_TwoForms, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCUses)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCUses, "Sets the passed form's uses component's value", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCValue)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCValue, "Sets the passed form's gold value component's value", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCHealth)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCHealth, "Sets the passed form's health component's value", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCWeight)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCWeight, "Sets the passed form's weight component's value", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCQuality)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCQuality, "Sets the passed form's quality component's value", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCAttackDamage)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCAttackDamage, "Sets the passed form's attack damage component's value", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_PARAMINFO(SetBFCAttribute, 3)
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "Attribute ID", CSASDataElement::kParamType_Numeric },
		{ "Value", CSASDataElement::kParamType_Numeric }
	};

	BEGIN_CSASCOMMAND_HANDLER(SetBFCAttribute)
	{
		TESForm* Form = NULL;
		double Buffer = 0;
		double Attribute = 0;

		if (!EXTRACT_CSASARGS(&Form, &Attribute, &Buffer))
			return false;
		else if (!Form)
			return false;
		else if (Attribute >= TESAttributes::kAttribute__MAX)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCAttribute, "Sets the passed form's attribute component's value(s)", CSASDataElement::kParamType_Invalid, kParams_SetBFCAttribute, 3);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCLeveledListChanceNone)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCLeveledListChanceNone, "Sets the passed form's leveled list component's 'chance none' attribute", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCActorBaseDataMagicka)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCActorBaseDataMagicka, "Sets the passed form's actor basedata component's 'magicka' attribute", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCActorBaseDataFatigue)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCActorBaseDataFatigue, "Sets the passed form's actor basedata component's 'fatigue' attribute", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCActorBaseDataBarterGold)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCActorBaseDataBarterGold, "Sets the passed form's actor basedata component's 'barter gold' attribute", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCActorBaseDataLevel)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCActorBaseDataLevel, "Sets the passed form's actor basedata component's 'level' attribute", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_PARAMINFO(SetBFCAIFormAIStats, 3)
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "AI Stat ID", CSASDataElement::kParamType_Numeric },
		{ "Value", CSASDataElement::kParamType_Numeric }
	};

	BEGIN_CSASCOMMAND_HANDLER(SetBFCAIFormAIStats)
	{
		TESForm* Form = NULL;
		double Buffer = 0;
		double AIStat = 0;

		if (!EXTRACT_CSASARGS(&Form, &AIStat, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCAIFormAIStats, "Sets the passed form's AI component's 'stats' attribute", CSASDataElement::kParamType_Invalid, kParams_SetBFCAIFormAIStats, 3);

	BEGIN_CSASCOMMAND_PARAMINFO(SetBFCAIFormAITrainingSkill, 3)
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "Skill ID", CSASDataElement::kParamType_Numeric }
	};

	BEGIN_CSASCOMMAND_HANDLER(SetBFCAIFormAITrainingSkill)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCAIFormAITrainingSkill, "Sets the passed form's AI component's 'training skill' attribute", CSASDataElement::kParamType_Invalid, kParams_SetBFCAIFormAITrainingSkill, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCAIFormAITrainingLevel)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCAIFormAITrainingLevel, "Sets the passed form's AI component's 'training level' attribute", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCSoundFilePath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCSoundFilePath, "Sets the passed form's sound file component's path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCBipedModelSlotMask)
	{
		TESForm* Form = NULL;
		double Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCBipedModelSlotMask, "Sets the passed form's biped model component's 'slot mask' attribute", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCBipedModelMaleBipedModelPath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCBipedModelMaleBipedModelPath, "Sets the passed form's biped model component's male biped model path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCBipedModelFemaleBipedModelPath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCBipedModelFemaleBipedModelPath, "Sets the passed form's biped model component's female biped model path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCBipedModelMaleGroundModelPath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCBipedModelMaleGroundModelPath, "Sets the passed form's biped model component's male ground model path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCBipedModelFemaleGroundModelPath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCBipedModelFemaleGroundModelPath, "Sets the passed form's biped model component's female ground model path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCBipedModelMaleIconPath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCBipedModelMaleIconPath, "Sets the passed form's biped model component's male icon path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCBipedModelFemaleIconPath)
	{
		TESForm* Form = NULL;
		char Buffer[0x4000] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCBipedModelFemaleIconPath, "Sets the passed form's biped model component's female icon path", CSASDataElement::kParamType_Invalid, kParams_FormString, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCEnchantableFormEnchantment)
	{
		TESForm* Form = NULL;
		TESForm* Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCEnchantableFormEnchantment, "Sets the passed form's enchantment", CSASDataElement::kParamType_Invalid, kParams_TwoForms, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetBFCProduceFormIngredient)
	{
		TESForm* Form = NULL;
		TESForm* Buffer = 0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCProduceFormIngredient, "Sets the passed form's produce component's ingredient", CSASDataElement::kParamType_Invalid, kParams_TwoForms, 2);

	BEGIN_CSASCOMMAND_PARAMINFO(SetBFCProduceFormChance, 3)
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "Harvest Season ID", CSASDataElement::kParamType_Numeric },
		{ "Value", CSASDataElement::kParamType_Numeric }
	};

	BEGIN_CSASCOMMAND_HANDLER(SetBFCProduceFormChance)
	{
		TESForm* Form = NULL;
		double Buffer = 0;
		double Season = 0;

		if (!EXTRACT_CSASARGS(&Form, &Season, &Buffer))
			return false;
		else if (!Form)
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
	DEFINE_CSASCOMMAND_PARAM(SetBFCProduceFormChance, "Sets the passed form's produce component's 'harvest chance' attribute", CSASDataElement::kParamType_Invalid, kParams_SetBFCProduceFormChance, 3);
}