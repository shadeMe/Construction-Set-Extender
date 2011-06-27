#include "Commands_General.h"
#include "..\..\ExtenderInternals.h"
#include "..\ScriptCommands.h"
#include "..\ScriptRunner.h"

namespace CSAutomationScript
{
	void RegisterGeneralCommands()
	{
		REGISTER_CSASCOMMAND(PrintToConsole);
		REGISTER_CSASCOMMAND(FormatNumber);
		REGISTER_CSASCOMMAND(TypeOf);
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
	DEFINE_CSASCOMMAND_ALIAS(PrintToConsole, "printC", "Prints a message to the CSE console window", CSASDataElement::kParamType_Invalid);


	BEGIN_CSASCOMMAND_PARAMINFO(FormatNumber, 2)
	{
		{ "Format String", CSASDataElement::kParamType_String },
		{ "Number", CSASDataElement::kParamType_Numeric }
	};
	BEGIN_CSASCOMMAND_HANDLER(FormatNumber)
	{
		char Buffer[0x200] = {0},
			 OutBuffer[0x32] = {0};
		double Number = 0.0;

		if (!EXTRACT_CSASARGS(&Buffer, &Number))
			return false;

		sprintf_s(OutBuffer, sizeof(OutBuffer), Buffer, Number);

		Result->SetString(OutBuffer);
		return true;
	}
	DEFINE_CSASCOMMAND_ALIAS(FormatNumber, "fmtNum", "Formats a numeric value as a string", CSASDataElement::kParamType_String);


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
		assert(CallingContext);

		ScriptVariable* Variable = CallingContext->LookupVariableByName(Buffer);
		if (!Variable)
			throw std::exception("Invalid variable ID");

		Result->SetNumber(Variable->GetDataType());
		return true;
	}
	DEFINE_CSASCOMMAND(TypeOf, "Returns the type of value stored in a variable", CSASDataElement::kParamType_Numeric);



}