#pragma once

#include "..\CSECoda.h"

using namespace bgsee::BGSEEScript;
using namespace bgsee::BGSEEScript::Commands;

namespace ConstructionSetExtender
{
	namespace BGSEEScript
	{
		namespace Commands
		{
			namespace Reference
			{
				CodaScriptCommandRegistrarDecl;

				CodaScriptCommandPrototype(CreateRef,
					0,
					"Creates an object reference in the passed cell. The worldspace parameter is ignored for interior cells.",
					0,
					9,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptCommandPrototype(GetRefPosition,
					0,
					"Returns the position of the reference in the given axis.",
					0,
					2,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptCommandPrototype(GetRefRotation,
					0,
					"Returns the rotation of the reference in the given axis.",
					0,
					2,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetRefScale,
					0,
					"Returns the scale of the reference.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetRefPersistent,
					0,
					"Returns the persistence of the reference.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetRefDisabled,
					0,
					"Returns the disabled state of the reference.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetRefVWD,
					0,
					"Returns the visible when distant state of the reference.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(GetRefBaseForm,
					0,
					"Returns the base form of the reference.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptParametricCommandPrototype(GetRefCell,
					0,
					"Returns the parent cell of the reference.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptCommandPrototype(SetRefPosition,
					0,
					"Sets the position of the reference in the given axis.",
					0,
					3,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(SetRefRotation,
					0,
					"Sets the rotation of the reference in the given axis.",
					0,
					3,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetRefScale,
					0,
					"Sets the scale of the reference.",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetRefPersistent,
					0,
					"Sets the persistence of the reference.",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetRefDisabled,
					0,
					"Sets the disabled state of the reference.",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(SetRefVWD,
					0,
					"Sets the visible when distant state of the reference.",
					0,
					2,
					FormNumber,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptCommandPrototype(GetCellObjects,
					0,
					"Returns an array of references that present in the given cell.",
					0,
					1,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptCommandPrototype(GetCellWorldspace,
					0,
					"Returns the worldspace of the given cell. Will return zero if the passed cell is an interior.",
					0,
					1,
					ICodaScriptDataStore::kDataType_Reference);

				CodaScriptSimpleCommandPrototype(GetCurrentRenderWindowSelection,
					0,
					"Returns an array of references that are currently selected in the render window.",
					0,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptSimpleCommandPrototype(GetLoadedRenderWindowCells,
					0,
					"Returns either an array of cells in the render window's active grid (when in an exterior location) or the current interior cell.",
					0,
					ICodaScriptDataStore::kDataType_Array);

				CodaScriptParametricCommandPrototype(AddRefToRenderWindowSelection,
					"addRefToSel",
					"Adds the reference to the active render window selection.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(RemoveRefFromRenderWindowSelection,
					"removeRefFromSel",
					"Removes the reference from the active render window selection.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(CreateRenderWindowSelectionGroup,
					"createSelGroup",
					"Creates a render window selection group from an array of references. Returns true if successful, false otherwise.",
					0,
					1,
					OneArray,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(DissolveRenderWindowSelectionGroup,
					"dissolveSelGroup",
					"Removes an existing render window selection group that consists an array of references. Returns true if successful, false otherwise.",
					0,
					1,
					OneArray,
					ICodaScriptDataStore::kDataType_Numeric);

				CodaScriptParametricCommandPrototype(FloorRef,
					0,
					"Moves the reference to the nearest collision object in the negative Z axis. The parameter reference needs to be loaded in the render window.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(LoadRefIntoRenderWindow,
					0,
					"Loads the reference's parent cell into the render window and places the camera at the reference's position.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Invalid);

				CodaScriptParametricCommandPrototype(DeleteRef,
					0,
					"Permanently removes the reference from its parent cell.",
					0,
					1,
					OneForm,
					ICodaScriptDataStore::kDataType_Invalid);
			}
		}
	}
}