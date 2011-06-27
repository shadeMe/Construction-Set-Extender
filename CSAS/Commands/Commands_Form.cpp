#include "Commands_Form.h"
#include "..\..\ExtenderInternals.h"
#include "..\ScriptCommands.h"
#include "..\ScriptRunner.h"

namespace CSAutomationScript
{
	void RegisterFormCommands()
	{
/*		REGISTER_CSASCOMMAND(GetFormByEditorID);
		REGISTER_CSASCOMMAND(GetFormByFormID);

		REGISTER_CSASCOMMAND(GetEditorID);
		REGISTER_CSASCOMMAND(GetFormType);

		REGISTER_CSASCOMMAND(GetBFCFullName);
		REGISTER_CSASCOMMAND(GetBFCDescription);
		REGISTER_CSASCOMMAND(GetBFCTexturePath);
		REGISTER_CSASCOMMAND(GetBFCModelPath);
		REGISTER_CSASCOMMAND(GetBFCModelBoundRadius);
		REGISTER_CSASCOMMAND(GetBFCScript);
		REGISTER_CSASCOMMAND(GetBFCUses);
		REGISTER_CSASCOMMAND(GetBFCValue);
		REGISTER_CSASCOMMAND(GetBFCHealth);
		REGISTER_CSASCOMMAND(GetBFCWeight);
		REGISTER_CSASCOMMAND(GetBFCQuality);
		REGISTER_CSASCOMMAND(GetBFCAttackDamage);
		REGISTER_CSASCOMMAND(GetBFCAttributes);
		REGISTER_CSASCOMMAND(GetBFCSpellListEntries);
		REGISTER_CSASCOMMAND(GetBFCLeveledListEntries);
		REGISTER_CSASCOMMAND(GetBFCLeveledListChanceNone);
		REGISTER_CSASCOMMAND(GetBFCContainerEntries);
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataNPCFlags);
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataCreatureFlags);
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataFactionList);
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataMagicka);
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataFatigue);
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataBarterGold);
		REGISTER_CSASCOMMAND(GetBFCActorBaseDataLevel);
		REGISTER_CSASCOMMAND(GetBFCAIFormAIStats);
		REGISTER_CSASCOMMAND(GetBFCAIFormAIServiceFlags);
		REGISTER_CSASCOMMAND(GetBFCAIFormAITrainingSkill);
		REGISTER_CSASCOMMAND(GetBFCAIFormAITrainingLevel);
		REGISTER_CSASCOMMAND(GetBFCAIFormAIPackageList);
		REGISTER_CSASCOMMAND(GetBFCReactionFormReactionEntries);
		REGISTER_CSASCOMMAND(GetBFCSoundFilePath);
		REGISTER_CSASCOMMAND(GetBFCBipedModelSlotMask);
		REGISTER_CSASCOMMAND(GetBFCBipedModelFlags);
		REGISTER_CSASCOMMAND(GetBFCBipedModelMaleBipedModelPath);
		REGISTER_CSASCOMMAND(GetBFCBipedModelFemaleBipedModelPath);
		REGISTER_CSASCOMMAND(GetBFCBipedModelMaleGroundModelPath);
		REGISTER_CSASCOMMAND(GetBFCBipedModelFemaleGroundModelPath);
		REGISTER_CSASCOMMAND(GetBFCBipedModelMaleIconPath);
		REGISTER_CSASCOMMAND(GetBFCBipedModelFemaleIconPath);
		REGISTER_CSASCOMMAND(GetBFCBipedModelMaleBipedModelPath);
		REGISTER_CSASCOMMAND(GetBFCEnchantableFormEnchantment);
		REGISTER_CSASCOMMAND(GetBFCEnchantableFormType);
		REGISTER_CSASCOMMAND(GetBFCProduceFormIngredient);
		REGISTER_CSASCOMMAND(GetBFCProduceFormChance);
		REGISTER_CSASCOMMAND(GetBFCModelListModels);

		REGISTER_CSASCOMMAND(SetBFCFullName);
		REGISTER_CSASCOMMAND(SetBFCDescription);
		REGISTER_CSASCOMMAND(SetBFCTexturePath);
		REGISTER_CSASCOMMAND(SetBFCModelPath);
		REGISTER_CSASCOMMAND(SetBFCModelBoundRadius);
		REGISTER_CSASCOMMAND(SetBFCScript);
		REGISTER_CSASCOMMAND(SetBFCUses);
		REGISTER_CSASCOMMAND(SetBFCValue);
		REGISTER_CSASCOMMAND(SetBFCHealth);
		REGISTER_CSASCOMMAND(SetBFCWeight);
		REGISTER_CSASCOMMAND(SetBFCQuality);
		REGISTER_CSASCOMMAND(SetBFCAttackDamage);
		REGISTER_CSASCOMMAND(SetBFCAttribute);
		REGISTER_CSASCOMMAND(SetBFCLeveledListChanceNone);
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataNPCFlags);
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataCreatureFlags);
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataMagicka);
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataFatigue);
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataBarterGold);
		REGISTER_CSASCOMMAND(SetBFCActorBaseDataLevel);
		REGISTER_CSASCOMMAND(SetBFCAIFormAIStats);
		REGISTER_CSASCOMMAND(SetBFCAIFormAIServiceFlags);
		REGISTER_CSASCOMMAND(SetBFCAIFormAITrainingSkill);
		REGISTER_CSASCOMMAND(SetBFCAIFormAITrainingLevel);
		REGISTER_CSASCOMMAND(SetBFCSoundFilePath);
		REGISTER_CSASCOMMAND(SetBFCBipedModelSlotMask);
		REGISTER_CSASCOMMAND(SetBFCBipedModelFlags);
		REGISTER_CSASCOMMAND(SetBFCBipedModelMaleBipedModelPath);
		REGISTER_CSASCOMMAND(SetBFCBipedModelFemaleBipedModelPath);
		REGISTER_CSASCOMMAND(SetBFCBipedModelMaleGroundModelPath);
		REGISTER_CSASCOMMAND(SetBFCBipedModelFemaleGroundModelPath);
		REGISTER_CSASCOMMAND(SetBFCBipedModelMaleIconPath);
		REGISTER_CSASCOMMAND(SetBFCBipedModelFemaleIconPath);
		REGISTER_CSASCOMMAND(SetBFCBipedModelMaleBipedModelPath);
		REGISTER_CSASCOMMAND(SetBFCEnchantableFormEnchantment);
		REGISTER_CSASCOMMAND(SetBFCEnchantableFormType);
		REGISTER_CSASCOMMAND(SetBFCProduceFormIngredient);
		REGISTER_CSASCOMMAND(SetBFCProduceFormChance);
*/
	}

/*
	BEGIN_CSASCOMMAND_PARAMINFO(GetFormByEditorID, 1)
	{
		{ "EditorID", CSASDataElement::kParamType_String }
	};
	BEGIN_CSASCOMMAND_HANDLER(GetFormByEditorID)
	{
		char Buffer[0x400] = {0};

		if (!EXTRACT_CSASARGS(&Buffer))
			return false;

		TESForm* Form = TESForm_LookupByEditorID(Buffer);
		
		Result->SetForm(Form);
		return true;
	}
	DEFINE_CSASCOMMAND_ALIAS(GetFormByEditorID, "refEID", "Fetches the form with the passed editorID", CSASDataElement::kParamType_Reference);


	BEGIN_CSASCOMMAND_PARAMINFO(GetFormByFormID, 1)
	{
		{ "FormID", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(GetFormByFormID)
	{
		double FormID = 0;

		if (!EXTRACT_CSASARGS(&FormID))
			return false;

		TESForm* Form = TESForm_LookupByFormID((UInt32)FormID);
		
		Result->SetForm(Form);
		return true;
	}
	DEFINE_CSASCOMMAND_ALIAS(GetFormByFormID, "refFID", "Fetches the form with the passed formID", CSASDataElement::kParamType_Reference);


	BEGIN_CSASCOMMAND_HANDLER(GetEditorID)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		Result->SetString((Form->editorID.c_str())?Form->editorID.c_str():"");
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetEditorID, "Returns the editorID of the passed form", CSASDataElement::kParamType_String, kParams_OneForm);


	BEGIN_CSASCOMMAND_HANDLER(GetFormType)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		Result->SetNumber(Form->formType);
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetFormType, "Returns the typeID of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCFullName)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);

		TESFullName* FullName = CS_CAST(Form, TESForm, TESFullName);
		if (FullName)
			Result->SetString(FullName->name.m_data);

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCDescription)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);

		TESDescription* Description = CS_CAST(Form, TESForm, TESDescription);
		if (Description)
			Result->SetString(Description->description.c_str());
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCTexturePath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);

		TESTexture* Texture = CS_CAST(Form, TESForm, TESTexture);
		if (Texture)
			Result->SetString(Texture->ddsPath.m_data);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCModelPath)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);

		TESModel* Model = CS_CAST(Form, TESForm, TESModel);
		if (Model)
			Result->SetString(Model->nifPath)
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCModelBoundRadius)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCScript)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCUses)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCValue)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCHealth)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCWeight)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCQuality)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAttackDamage)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

	BEGIN_CSASCOMMAND_HANDLER(GetBFCAttributes)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;
		Result->SetNumber(0);
		
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(, "", CSASDataElement::kParamType_, kParams_OneForm);

*/
}