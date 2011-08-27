#include "Commands_Reference.h"
#include "..\ScriptCommands.h"
#include "..\ScriptRunner.h"

#define PI	3.151592653589793

namespace CSAutomationScript
{
	void RegisterReferenceCommands()
	{
		REGISTER_CSASCOMMAND(CreateRef, "Reference Functions");

		REGISTER_CSASCOMMAND(GetRefPosition, "Reference Functions");
		REGISTER_CSASCOMMAND(GetRefRotation, "Reference Functions");
		REGISTER_CSASCOMMAND(GetRefScale, "Reference Functions");
		REGISTER_CSASCOMMAND(GetRefPersistent, "Reference Functions");
		REGISTER_CSASCOMMAND(GetRefDisabled, "Reference Functions");
		REGISTER_CSASCOMMAND(GetRefVWD, "Reference Functions");

		REGISTER_CSASCOMMAND(SetRefRotation, "Reference Functions");
		REGISTER_CSASCOMMAND(SetRefPosition, "Reference Functions");
		REGISTER_CSASCOMMAND(SetRefScale, "Reference Functions");
		REGISTER_CSASCOMMAND(SetRefPersistent, "Reference Functions");
		REGISTER_CSASCOMMAND(SetRefDisabled, "Reference Functions");
		REGISTER_CSASCOMMAND(SetRefVWD, "Reference Functions");

		REGISTER_CSASCOMMAND(GetCellObjects, "Reference Functions");
		REGISTER_CSASCOMMAND(GetCurrentRenderWindowSelection, "Reference Functions");
	}

	BEGIN_CSASCOMMAND_PARAMINFO(CreateRef, 9)
	{
		{ "Base Form", CSASDataElement::kParamType_Reference },
		{ "Pos X", CSASDataElement::kParamType_Numeric },
		{ "Pos Y", CSASDataElement::kParamType_Numeric },
		{ "Pos X", CSASDataElement::kParamType_Numeric },
		{ "Rot X", CSASDataElement::kParamType_Numeric },
		{ "Rot Y", CSASDataElement::kParamType_Numeric },
		{ "Rot Z", CSASDataElement::kParamType_Numeric },
		{ "Cell", CSASDataElement::kParamType_Reference },
		{ "Worldspace", CSASDataElement::kParamType_Reference }
	};
	BEGIN_CSASCOMMAND_HANDLER(CreateRef)
	{
		TESForm* BaseForm = NULL;
		TESForm* Cell = NULL;
		TESForm* WorldSpace = NULL;
		Vector3 Position, Rotation;

		if (!EXTRACT_CSASARGS(&BaseForm, &Position.x, &Position.y, &Position.z, &Rotation.x, &Rotation.y, &Rotation.z, &Cell, &WorldSpace))
			return false;
		else if (!BaseForm || !Cell)
			return false;

		TESObject* Base = CS_CAST(BaseForm, TESForm, TESObject);
		TESObjectCELL* ParentCell = CS_CAST(Cell, TESForm, TESObjectCELL);
		TESWorldSpace* ParentWorldspace = CS_CAST(WorldSpace, TESForm, TESWorldSpace);

		if (!Base || !ParentCell || (ParentCell->GetIsInterior() == false && !ParentWorldspace))
			return false;

		TESObjectREFR* NewRef = _DATAHANDLER->PlaceObjectRef(Base, &Position, &Rotation, ParentCell, ParentWorldspace, NULL);
		Result->SetForm(NewRef);
		return true;
	}
	DEFINE_CSASCOMMAND(CreateRef, "Creates an object reference in the passed cell. The worldspace parameter is ignored for interior cells.", CSASDataElement::kParamType_Reference, 9);

	BEGIN_CSASCOMMAND_PARAMINFO(GetRefPosition, 2)
	{
		{ "Reference", CSASDataElement::kParamType_Reference },
		{ "Axis", CSASDataElement::kParamType_String }
	};
	BEGIN_CSASCOMMAND_HANDLER(GetRefPosition)
	{
		TESForm* Form = NULL;
		char Buffer[0x10] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		if (!_stricmp(Buffer, "x"))
			Result->SetNumber(Reference->position.x);
		else if (!_stricmp(Buffer, "y"))
			Result->SetNumber(Reference->position.y);
		else
			Result->SetNumber(Reference->position.z);

		return true;
	}
	DEFINE_CSASCOMMAND(GetRefPosition, "Returns the position of the reference in the given axis.", CSASDataElement::kParamType_Numeric, 2);

	BEGIN_CSASCOMMAND_PARAMINFO(GetRefRotation, 2)
	{
		{ "Reference", CSASDataElement::kParamType_Reference },
		{ "Axis", CSASDataElement::kParamType_String }
	};
	BEGIN_CSASCOMMAND_HANDLER(GetRefRotation)
	{
		TESForm* Form = NULL;
		char Buffer[0x10] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		if (!_stricmp(Buffer, "x"))
			Result->SetNumber(Reference->rotation.x * 180 / PI);
		else if (!_stricmp(Buffer, "y"))
			Result->SetNumber(Reference->rotation.y * 180 / PI);
		else
			Result->SetNumber(Reference->rotation.z * 180 / PI);

		return true;
	}
	DEFINE_CSASCOMMAND(GetRefRotation, "Returns the rotation of the reference in the given axis.", CSASDataElement::kParamType_Numeric, 2);

	BEGIN_CSASCOMMAND_HANDLER(GetRefScale)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		Result->SetNumber(Reference->scale);
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetRefScale, "Returns the scale of the reference.", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetRefPersistent)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		Result->SetNumber((Reference->formFlags & TESForm::kFormFlags_QuestItem));
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetRefPersistent, "Returns the persistence of the reference.", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetRefDisabled)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		Result->SetNumber((Reference->formFlags & TESForm::kFormFlags_Disabled));
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetRefDisabled, "Returns the disabled state of the reference.", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetRefVWD)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		Result->SetNumber((Reference->formFlags & TESForm::kFormFlags_VisibleWhenDistant));
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetRefVWD, "Returns the visible when distant state of the reference.", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_PARAMINFO(SetRefPosition, 3)
	{
		{ "Reference", CSASDataElement::kParamType_Reference },
		{ "Axis", CSASDataElement::kParamType_String },
		{ "Value", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(SetRefPosition)
	{
		TESForm* Form = NULL;
		char Buffer[0x10] = {0};
		double Value = 0.0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer, &Value))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		if (!_stricmp(Buffer, "x"))
			Reference->position.x = Value;
		else if (!_stricmp(Buffer, "y"))
			Reference->position.y = Value;
		else
			Reference->position.z = Value;

		Reference->Update3D();
		return true;
	}
	DEFINE_CSASCOMMAND(SetRefPosition, "Sets the position of the reference in the given axis.", CSASDataElement::kParamType_Invalid, 3);

	BEGIN_CSASCOMMAND_PARAMINFO(SetRefRotation, 3)
	{
		{ "Reference", CSASDataElement::kParamType_Reference },
		{ "Axis", CSASDataElement::kParamType_String },
		{ "Value", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(SetRefRotation)
	{
		TESForm* Form = NULL;
		char Buffer[0x10] = {0};
		double Value = 0.0;

		if (!EXTRACT_CSASARGS(&Form, &Buffer, &Value))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		if (!_stricmp(Buffer, "x"))
			Reference->position.x = Value * PI / 180;
		else if (!_stricmp(Buffer, "y"))
			Reference->position.y = Value * PI / 180;
		else
			Reference->position.z = Value * PI / 180;

		Reference->Update3D();
		return true;
	}
	DEFINE_CSASCOMMAND(SetRefRotation, "Sets the rotation of the reference in the given axis.", CSASDataElement::kParamType_Invalid, 3);

	BEGIN_CSASCOMMAND_HANDLER(SetRefScale)
	{
		TESForm* Form = NULL;
		double Value = 0.0;

		if (!EXTRACT_CSASARGS(&Form, &Value))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		Reference->scale = Value;

		Reference->Update3D();
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(SetRefScale, "Sets the scale of the reference.", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetRefPersistent)
	{
		TESForm* Form = NULL;
		double Value = 0.0;

		if (!EXTRACT_CSASARGS(&Form, &Value))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		ToggleFlag(&Reference->formFlags, TESForm::kFormFlags_QuestItem, (bool)Value);
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(SetRefPersistent, "Sets the persistence of the reference.", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetRefDisabled)
	{
		TESForm* Form = NULL;
		double Value = 0.0;

		if (!EXTRACT_CSASARGS(&Form, &Value))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		ToggleFlag(&Reference->formFlags, TESForm::kFormFlags_Disabled, (bool)Value);
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(SetRefDisabled, "Sets the disabled state of the reference.", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_HANDLER(SetRefVWD)
	{
		TESForm* Form = NULL;
		double Value = 0.0;

		if (!EXTRACT_CSASARGS(&Form, &Value))
			return false;
		else if (!Form)
			return false;

		TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
		if (!Reference)
			return false;

		ToggleFlag(&Reference->formFlags, TESForm::kFormFlags_VisibleWhenDistant, (bool)Value);
		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(SetRefVWD, "Sets the visible when distant state of the reference.", CSASDataElement::kParamType_Invalid, kParams_FormNumber, 2);

	BEGIN_CSASCOMMAND_PARAMINFO(GetCellObjects, 1)
	{
		{ "Cell", CSASDataElement::kParamType_Reference }
	};

	BEGIN_CSASCOMMAND_HANDLER(GetCellObjects)
	{
		TESForm* Form = NULL;

		if (!EXTRACT_CSASARGS(&Form))
			return false;
		else if (!Form)
			return false;

		TESObjectCELL* Cell = CS_CAST(Form, TESForm, TESObjectCELL);
		if (!Cell)
			return false;

		std::vector<CSASDataElement> ArrayData;
		for (TESObjectCELL::ObjectREFRList::Iterator Itr = Cell->objectList.Begin(); !Itr.End() && Itr.Get(); ++Itr)
			ArrayData.push_back(Itr.Get());

		Result->SetArray(&ArrayData[0], ArrayData.size());
		ArrayData.clear();

		return true;
	}
	DEFINE_CSASCOMMAND(GetCellObjects, "Returns an array of references that present in the given cell.", CSASDataElement::kParamType_Array, 1);

	BEGIN_CSASCOMMAND_HANDLER(GetCurrentRenderWindowSelection)
	{
		std::vector<CSASDataElement> ArrayData;
		for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->selectionList; Itr && Itr->Data; Itr = Itr->Next)
		{
			TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
			if (Ref)
				ArrayData.push_back(Ref);
		}

		Result->SetArray(&ArrayData[0], ArrayData.size());
		ArrayData.clear();

		return true;
	}
	DEFINE_CSASCOMMAND_PARAM(GetCurrentRenderWindowSelection, "Returns an array of references that are currently selected in the render window.", CSASDataElement::kParamType_Array, NULL, 0);
}