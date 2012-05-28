#pragma once

#include "..\CSECoda.h"

using namespace BGSEditorExtender::BGSEEScript;
using namespace BGSEditorExtender::BGSEEScript::Commands;

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			namespace BaseFormComponent
			{
				CodaScriptCommandRegistrarDecl;

				CodaScriptParametricCommandPrototype(GetBFCFullName,
					0,
					"Returns the full name component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCDescription,
					0,
					"Returns the description component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCTexturePath,
					0,
					"Returns the texture path component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCModelPath,
					0,
					"Returns the model path component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCModelBoundRadius,
					0,
					"Returns the model bound radius component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCScript,
					0,
					"Returns the script component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptParametricCommandPrototype(GetBFCUses,
					0,
					"Returns the uses component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCValue,
					0,
					"Returns the gold value component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCHealth,
					0,
					"Returns the health component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCWeight,
					0,
					"Returns the weight component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCQuality,
					0,
					"Returns the quality component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCAttackDamage,
					0,
					"Returns the attack damage component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCAttributes,
					0,
					"Returns the attributes component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCSpellListEntries,
					0,
					"Returns the spell list component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCLeveledListEntries,
					0,
					"Returns the leveled list component of the passed form as an array of arrays. Subitems are of the following structure: Leveled List Item, Level, Count",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCLeveledListChanceNone,
					0,
					"Returns the 'chance none' attribute of leveled list component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCContainerEntries,
					0,
					"Returns the container component of the passed form as an array of arrays. Subitems are of the following structure: Inventory Item, Count",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCActorBaseDataFactionList,
					0,
					"Returns the 'faction list' attribute of actor basedata component the passed form as an array of arrays. Subitems are of the following structure: Faction, Rank",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCActorBaseDataMagicka,
					0,
					"Returns the 'magicka' attribute of actor basedata component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCActorBaseDataFatigue,
					0,
					"Returns the 'fatigue' attribute of actor basedata component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCActorBaseDataBarterGold,
					0,
					"Returns the 'barter gold' attribute of actor basedata component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCActorBaseDataLevel,
					0,
					"Returns the 'level' attributes of actor basedata component of the passed form as an array with the following structure: Level, MinLevel, MaxLevel",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCAIFormAIStats,
					0,
					"Returns the 'aistats' attributes of AI component of the passed form as an array with the following structure: Agression, Confidence, Energy, Responsibility",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCAIFormAITrainingSkill,
					0,
					"Returns the 'training skill' attribute of AI component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCAIFormAITrainingLevel,
					0,
					"Returns the 'training level' attribute of AI component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCAIFormAIPackageList,
					0,
					"Returns the 'package list' attribute of AI component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCReactionFormReactionEntries,
					0,
					"Returns the faction reaction component the passed form as an array of arrays. Subitems are of the following structure: Faction, Reaction",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCSoundFilePath,
					0,
					"Returns the sound file path component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCBipedModelSlotMask,
					0,
					"Returns the 'slot mask' attribute of biped model component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCBipedModelMaleBipedModelPath,
					0,
					"Returns the 'male biped model path' attribute of biped model component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCBipedModelFemaleBipedModelPath,
					0,
					"Returns the 'female biped model path' attribute of biped model component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCBipedModelMaleGroundModelPath,
					0,
					"Returns the 'male ground model path' attribute of biped model component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCBipedModelFemaleGroundModelPath,
					0,
					"Returns the 'female ground model path' attribute of biped model component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCBipedModelMaleIconPath,
					0,
					"Returns the 'male icon path' attribute of biped model component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCBipedModelFemaleIconPath,
					0,
					"Returns the 'female icon path' attribute of biped model component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetBFCEnchantableFormEnchantment,
					0,
					"Returns the 'enchantment' attribute of enchantment component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptParametricCommandPrototype(GetBFCEnchantableFormType,
					0,
					"Returns the 'enchantment type' attribute of enchantment component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetBFCProduceFormIngredient,
					0,
					"Returns the 'ingredient' attribute of produce component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptParametricCommandPrototype(GetBFCProduceFormChance,
					0,
					"Returns the 'harvest chance' attributes of produce component of the passed form as an array with the following structure: Spring, Summer, Fall, Winter",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetBFCModelListModels,
					0,
					"Returns the model list component of the passed form",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(SetBFCFullName,
					0,
					"Sets the passed form's full name component's value",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCDescription,
					0,
					"Sets the passed form's description component's value",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCTexturePath,
					0,
					"Sets the passed form's texture path component's value",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCModelPath,
					0,
					"Sets the passed form's model component's path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCModelBoundRadius,
					0,
					"Sets the passed form's model component's bound radius",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCScript,
					0,
					"Sets the passed form's script component's value",
					0,
					2,
					TwoForms,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCUses,
					0,
					"Sets the passed form's uses component's value",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCValue,
					0,
					"Sets the passed form's gold value component's value",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCHealth,
					0,
					"Sets the passed form's health component's value",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCWeight,
					0,
					"Sets the passed form's weight component's value",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCQuality,
					0,
					"Sets the passed form's quality component's value",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCAttackDamage,
					0,
					"Sets the passed form's attack damage component's value",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(SetBFCAttribute,
					0,
					"Sets the passed form's attribute component's value(s)",
					0,
					3,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCLeveledListChanceNone,
					0,
					"Sets the passed form's leveled list component's 'chance none' attribute",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCActorBaseDataMagicka,
					0,
					"Sets the passed form's actor basedata component's 'magicka' attribute",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCActorBaseDataFatigue,
					0,
					"Sets the passed form's actor basedata component's 'fatigue' attribute",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCActorBaseDataBarterGold,
					0,
					"Sets the passed form's actor basedata component's 'barter gold' attribute",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCActorBaseDataLevel,
					0,
					"Sets the passed form's actor basedata component's 'level' attribute",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(SetBFCAIFormAIStats,
					0,
					"Sets the passed form's AI component's 'stats' attribute",
					0,
					3,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(SetBFCAIFormAITrainingSkill,
					0,
					"Sets the passed form's AI component's 'training skill' attribute",
					0,
					2,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCAIFormAITrainingLevel,
					0,
					"Sets the passed form's AI component's 'training level' attribute",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCSoundFilePath,
					0,
					"Sets the passed form's sound file component's path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCBipedModelSlotMask,
					0,
					"Sets the passed form's biped model component's 'slot mask' attribute",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCBipedModelMaleBipedModelPath,
					0,
					"Sets the passed form's biped model component's male biped model path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCBipedModelFemaleBipedModelPath,
					0,
					"Sets the passed form's biped model component's female biped model path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCBipedModelMaleGroundModelPath,
					0,
					"Sets the passed form's biped model component's male ground model path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCBipedModelFemaleGroundModelPath,
					0,
					"Sets the passed form's biped model component's female ground model path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCBipedModelMaleIconPath,
					0,
					"Sets the passed form's biped model component's male icon path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCBipedModelFemaleIconPath,
					0,
					"Sets the passed form's biped model component's female icon path",
					0,
					2,
					FormString,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCEnchantableFormEnchantment,
					0,
					"Sets the passed form's enchantment",
					0,
					2,
					TwoForms,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetBFCProduceFormIngredient,
					0,
					"Sets the passed form's produce component's ingredient",
					0,
					2,
					TwoForms,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(SetBFCProduceFormChance,
					0,
					"Sets the passed form's produce component's 'harvest chance' attribute",
					0,
					3,
					ICodaScriptDataStore::kDataType_Invalid);
			}
		}
	}
}