#pragma once

#include "..\ScriptCommandInfo.h"

namespace CSAutomationScript
{
	void		RegisterGeneralCommands();

	DECLARE_CSASCOMMAND(PrintToConsole);
	DECLARE_CSASCOMMAND(FormatNumber);
	DECLARE_CSASCOMMAND(TypeOf);
}