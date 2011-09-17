#pragma once

#include "..\ScriptCommandInfo.h"

namespace CSAutomationScript
{
	void		RegisterGeneralCommands();

	DECLARE_CSASCOMMAND(PrintToConsole);
	DECLARE_CSASCOMMAND(FormatNumber);
	DECLARE_CSASCOMMAND(TypeOf);

	DECLARE_CSASCOMMAND(GetFormByEditorID);
	DECLARE_CSASCOMMAND(GetFormByFormID);
	DECLARE_CSASCOMMAND(GetEditorID);
	DECLARE_CSASCOMMAND(GetFormType);

	DECLARE_CSASCOMMAND(SetEditorID);
	DECLARE_CSASCOMMAND(SetFormID);
}