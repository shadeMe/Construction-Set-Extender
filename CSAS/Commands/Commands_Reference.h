#pragma once

#include "..\ScriptCommandInfo.h"

namespace CSAutomationScript
{
	void		RegisterReferenceCommands();

	DECLARE_CSASCOMMAND(CreateRef);

	DECLARE_CSASCOMMAND(GetRefPosition);
	DECLARE_CSASCOMMAND(GetRefRotation);
	DECLARE_CSASCOMMAND(GetRefScale);
	DECLARE_CSASCOMMAND(GetRefPersistent);
	DECLARE_CSASCOMMAND(GetRefDisabled);
	DECLARE_CSASCOMMAND(GetRefVWD);

	DECLARE_CSASCOMMAND(SetRefRotation);
	DECLARE_CSASCOMMAND(SetRefPosition);
	DECLARE_CSASCOMMAND(SetRefScale);
	DECLARE_CSASCOMMAND(SetRefPersistent);
	DECLARE_CSASCOMMAND(SetRefDisabled);
	DECLARE_CSASCOMMAND(SetRefVWD);

	DECLARE_CSASCOMMAND(GetCellObjects);
	DECLARE_CSASCOMMAND(GetCurrentRenderWindowSelection);
}