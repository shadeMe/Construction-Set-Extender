#pragma once

#include "..\ScriptCommandInfo.h"

namespace CSAutomationScript
{
	void		RegisterStringCommands();

	DECLARE_CSASCOMMAND(StringLength);
	DECLARE_CSASCOMMAND(StringCompare);
	DECLARE_CSASCOMMAND(StringErase);
	DECLARE_CSASCOMMAND(StringFind);
	DECLARE_CSASCOMMAND(StringInsert);
	DECLARE_CSASCOMMAND(StringSubStr);
	DECLARE_CSASCOMMAND(StringIsNumber);
}