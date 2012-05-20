#pragma once
#include <CodaIntrinsics.inl>

/********** CSE Interface API ****************************************************
*	Interface object passed through the OBSE messaging system. A pointer to the
*	object will be dispatched to plugins that pass an arbitrary message of type
*	'CSEI' post post-plugin load (reply will be of the same type).
*
*	All other interfaces need to be initialized by calling InitializeInterface().
*	All RegisterXXX type calls must be made inside the OBSE Post-Post-Load message handler
*********************************************************************************/

struct CSEInterface
{
	enum
	{
		kCSEInterface_Console = 0,
		kCSEInterface_IntelliSense,
		kCSEInterface_Renderer,
		kCSEInterface_Script
	};

	// Used to initialize CSE's interface objects. Similar to OBSE's QueryInterface.
	const void*		(* InitializeInterface)(UInt8 InterfaceType);
	// Returns the implementation version of the extender interface
	UInt8			(* GetInterfaceVersion)(void);
};

struct CSEIntelliSenseInterface
{
	// Registers an arbitrary URL to a script command. Registered URLs will be displayed in the
	// script editor's context menu when the corresponding command is selected.
	void			(* RegisterCommandURL)(const char* CommandName, const char* URL);
};

struct CSEConsoleInterface
{
	typedef void	(* ConsolePrintCallback)(const char* Message, const char* Prefix);

	// Prints a message to the console in the following format: [<Prefix>]\t\t<Message>
	// Mustn't be called inside a print callback.

	// Reserved Prefixes: CMD, CSE, SE, CS, BSAV, USE, BE, TAG
	void			(* PrintToConsole)(const char*	Prefix, const char* FormatString, ...);
	// Registers a handler that gets called whenever a message is printed to the console.
	void			(* RegisterCallback)(ConsolePrintCallback Handler);
};

struct CSERendererInterface
{
	// Queues a text message for display in the render window
	void			(* PrintToRenderWindow)(const char* Message, float DisplayDuration);
};

struct CSEScriptInterface
{
	// Registers a Coda command to the VM
	// The caller retains ownership of the script command pointer
	void			(* RegisterCommand)(BGSEditorExtender::BGSEEScript::ICodaScriptCommand* Command);
};

