#pragma once

#include "..\Coda.h"

using namespace bgsee::script;
using namespace bgsee::script::commands;

namespace cse
{
	namespace script
	{
		namespace commands
		{
			namespace form
			{
				CodaScriptCommandRegistrarDecl;

				CodaScriptParametricCommandPrototype(GetFormByEditorID,
					"RefEID",
					"Fetches the form with the passed editorID.",
					0,
					1,
					OneString,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptParametricCommandPrototype(GetFormByFormID,
					"RefFID",
					"Fetches the form with the passed formID.",
					0,
					1,
					OneNumber,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptParametricCommandPrototype(GetEditorID,
					0,
					"Returns the editorID of the passed form.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_String);

				CodaScriptParametricCommandPrototype(GetFormType,
					0,
					"Returns the typeID of the passed form.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptCommandPrototype(SetEditorID,
					0,
					"Sets the form's editorID and returns true if successful.",
					0,
					2,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptCommandPrototype(SetFormID,
					0,
					"Sets the form's formID.",
					0,
					2,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(MarkAsModified,
					0,
					"Sets the 'Modified' flag on a form.",
					0,
					2,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(GetDataHandlerFormList,
					0,
					"Returns the loaded list of forms of the passed type.",
					"Form Type IDs<ul><li>&nbsp; 0: None</li><li>&nbsp; 1: TES4</li><li>&nbsp; 2: Group</li><li>&nbsp; 3: GMST</li><li>&nbsp; 4: Global</li><li>&nbsp; 5: Class</li><li>&nbsp; 6: Faction</li><li>&nbsp; 7: Hair</li><li>&nbsp; 8: Eyes</li><li>&nbsp; 9: Race</li><li>10: Sound</li><li>11: Skill</li><li>12: Effect</li><li>13: Script</li><li>14: LandTexture</li><li>15: Enchantment</li><li>16: Spell</li><li>17: BirthSign</li><li>18: Activator</li><li>19: Apparatus</li><li>20: Armor</li><li>21: Book</li><li>22: Clothing</li><li>23: Container</li><li>24: Door</li><li>25: Ingredient</li><li>26: Light</li><li>27: Misc</li><li>28: Stat</li><li>29: Grass</li><li>30: Tree</li><li>31: Flora</li><li>32: Furniture</li><li>33: Weapon</li><li>34: Ammo</li><li>35: NPC</li><li>36: Creature</li><li>37: LeveledCreature</li><li>38: SoulGem</li><li>39: Key</li><li>40: AlchemyItem</li><li>41: SubSpace</li><li>42: SigilStone</li><li>43: LeveledItem</li><li>44: SNDG</li><li>45: Weather</li><li>46: Climate</li><li>47: Region</li><li>48: Cell</li><li>49: REFR</li><li>50: ACHR</li><li>51: ACRE</li><li>52: PathGrid</li><li>53: WorldSpace</li><li>54: Land</li><li>55: TLOD</li><li>56: Road</li><li>57: Dialog</li><li>58: DialogInfo</li><li>59: Quest</li><li>60: Idle</li><li>61: Package</li><li>62: CombatStyle</li><li>63: LoadScreen</li><li>64: LeveledSpell</li><li>65: ANIO</li><li>66: WaterForm</li><li>67: EffectShader</li><li>68: TOFT</li></ul>",
					1,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(GetCellUseList,
					0,
					"Returns the list of cells containing references of the passed base form.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Array);
			}
		}
	}
}