#include "Commands_General.h"
#include "..\ScriptCommands.h"
#include "..\ScriptRunner.h"

namespace CSAutomationScript
{
	void RegisterGeneralCommands()
	{
		REGISTER_CSASCOMMAND(PrintToConsole, "General Functions");
		REGISTER_CSASCOMMAND(FormatNumber, "General Functions");
		REGISTER_CSASCOMMAND(TypeOf, "General Functions");

		REGISTER_CSASCOMMAND(GetFormByEditorID, "General Functions");
		REGISTER_CSASCOMMAND(GetFormByFormID, "General Functions");
		REGISTER_CSASCOMMAND(GetEditorID, "General Functions");
		REGISTER_CSASCOMMAND(GetFormType, "General Functions");

		REGISTER_CSASCOMMAND(SetEditorID, "General Functions");
		REGISTER_CSASCOMMAND(SetFormID, "General Functions");
		REGISTER_CSASCOMMAND(MarkAsModified, "General Functions");
	}

	BEGIN_CSASCOMMAND_PARAMINFO(PrintToConsole, 1)
	{
		{ "Message", CSASDataElement::kParamType_String }
	};
	BEGIN_CSASCOMMAND_HANDLER(PrintToConsole)
	{
		char Buffer[0x400] = {0};

		if (!EXTRACT_CSASARGS(&Buffer))
			return false;

		DebugPrint(Buffer);
		return true;
	}
	DEFINE_CSASCOMMAND_ALIAS(PrintToConsole, "printC", "Prints a message to the CSE console window", CSASDataElement::kParamType_Invalid, 1);

	BEGIN_CSASCOMMAND_PARAMINFO(FormatNumber, 3)
	{
		{ "Format String", CSASDataElement::kParamType_String },
		{ "Number", CSASDataElement::kParamType_Numeric },
		{ "Interpret As Unsigned Integer", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(FormatNumber)
	{
		char Buffer[0x200] = {0},
			 OutBuffer[0x32] = {0};
		double Number = 0.0, InterpretAsUInt32 = 0.0;

		if (!EXTRACT_CSASARGS(&Buffer, &Number, &InterpretAsUInt32))
			return false;

		if (InterpretAsUInt32 > 0.0 || InterpretAsUInt32 < 0.0)
			sprintf_s(OutBuffer, sizeof(OutBuffer), Buffer, (UInt32)Number);
		else
			sprintf_s(OutBuffer, sizeof(OutBuffer), Buffer, Number);

		Result->SetString(OutBuffer);
		return true;
	}
	DEFINE_CSASCOMMAND_ALIAS(FormatNumber, "fmtNum", "Formats a numeric value as a string", CSASDataElement::kParamType_String, 3);

	BEGIN_CSASCOMMAND_PARAMINFO(TypeOf, 1)
	{
		{ "Variable Name", CSASDataElement::kParamType_String }
	};
	BEGIN_CSASCOMMAND_HANDLER(TypeOf)
	{
		char Buffer[0x400] = {0};

		if (!EXTRACT_CSASARGS(&Buffer))
			return false;

		ScriptContext* CallingContext = SCRIPTRUNNER->GetExecutingContext();
		assertR(CallingContext);

		ScriptVariable* Variable = CallingContext->LookupVariableByName(Buffer);
		if (!Variable)
			throw std::exception("Invalid variable ID");

		Result->SetNumber(Variable->GetDataType());
		return true;
	}
	DEFINE_CSASCOMMAND(TypeOf, "Returns the type of value stored in a variable", CSASDataElement::kParamType_Numeric, 1);

	BEGIN_CSASCOMMAND_PARAMINFO(GetFormByEditorID, 1)
	{
		{ "EditorID", CSASDataElement::kParamType_String }
	};
	BEGIN_CSASCOMMAND_HANDLER(GetFormByEditorID)
	{
		char Buffer[0x400] = {0};

		if (!EXTRACT_CSASARGS(&Buffer))
			return false;

		TESForm* Form = TESForm::LookupByEditorID(Buffer);

		Result->SetForm(Form);
		return true;
	}
	DEFINE_CSASCOMMAND_ALIAS(GetFormByEditorID, "refEID", "Fetches the form with the passed editorID", CSASDataElement::kParamType_Reference, 1);

	BEGIN_CSASCOMMAND_PARAMINFO(GetFormByFormID, 1)
	{
		{ "FormID", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(GetFormByFormID)
	{
		double FormID = 0;

		if (!EXTRACT_CSASARGS(&FormID))
			return false;

		TESForm* Form = TESForm::LookupByFormID((UInt32)FormID);

		Result->SetForm(Form);
		return true;
	}
	DEFINE_CSASCOMMAND_ALIAS(GetFormByFormID, "refFID", "Fetches the form with the passed formID", CSASDataElement::kParamType_Reference, 1);

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
	DEFINE_CSASCOMMAND_PARAM(GetEditorID, "Returns the editorID of the passed form", CSASDataElement::kParamType_String, kParams_OneForm, 1);

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
	DEFINE_CSASCOMMAND_PARAM(GetFormType, "Returns the typeID of the passed form", CSASDataElement::kParamType_Numeric, kParams_OneForm, 1);

	BEGIN_CSASCOMMAND_PARAMINFO(SetEditorID, 2)
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "EditorID", CSASDataElement::kParamType_String }
	};
	BEGIN_CSASCOMMAND_HANDLER(SetEditorID)
	{
		TESForm* Form = NULL;
		char Buffer[0x400] = {0};

		if (!EXTRACT_CSASARGS(&Form, &Buffer))
			return false;
		else if (!Form)
			return false;

		if (Buffer)
			Result->SetNumber(Form->SetEditorID(Buffer));

		return true;
	}
	DEFINE_CSASCOMMAND(SetEditorID, "Sets the form's editorID and returns true if succesful", CSASDataElement::kParamType_Numeric, 2);

	BEGIN_CSASCOMMAND_PARAMINFO(SetFormID, 2)
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "FormID", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(SetFormID)
	{
		TESForm* Form = NULL;
		double FormID = 0;

		if (!EXTRACT_CSASARGS(&Form, &FormID))
			return false;
		else if (!Form)
			return false;

		if (FormID && Form->formID != FormID)
			Form->SetFormID(FormID);

		return true;
	}
	DEFINE_CSASCOMMAND(SetFormID, "Sets the form's formID", CSASDataElement::kParamType_Invalid, 2);

	BEGIN_CSASCOMMAND_PARAMINFO(MarkAsModified, 2)
	{
		{ "Form", CSASDataElement::kParamType_Reference },
		{ "Modified Flag", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(MarkAsModified)
	{
		TESForm* Form = NULL;
		double State = 0;

		if (!EXTRACT_CSASARGS(&Form, &State))
			return false;
		else if (!Form)
			return false;

		Form->SetFromActiveFile(State);

		return true;
	}
	DEFINE_CSASCOMMAND(MarkAsModified, "Sets the 'Modified' flag on a form", CSASDataElement::kParamType_Invalid, 2);
}