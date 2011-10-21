// Precompiled Header Compilation Unit
/*#include "StdAfx.h"		>> Force Included */

#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "d3d9.lib")

std::string										g_INIPath;
std::string										g_APPPath;
std::string										g_DLLPath;
bool											g_PluginPostLoad = false;
INI::INIManager*								g_INIManager = new CSEINIManager();
INI::INIEditGUI*								g_INIEditGUI = new INI::INIEditGUI();
char											g_TextBuffer[0x800] = {0};
HINSTANCE										g_DLLInstance = NULL;
PluginHandle									g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*							g_msgIntfc = NULL;
OBSECommandTableInterface*						g_commandTableIntfc = NULL;
ComponentDLLInterface::CommandTableData			g_CommandTableData;

void CSEINIManager::Initialize()
{
	DebugPrint("INI Path: %s", INIFile.c_str());
	std::fstream INIStream(INIFile.c_str(), std::fstream::in);
	bool CreateINI = false;

	if (INIStream.fail())
	{
		DebugPrint("INI File not found; Creating one...");
		CreateINI = true;
	}

	INIStream.close();
	INIStream.clear();		// only initializing non script editor keys as those are taken care of by its code

	RegisterSetting(new SME::INI::INISetting(this, "Top", "Extender::Console", "150", "Client Rect Top"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Left", "Extender::Console", "150", "Client Rect Left"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Right", "Extender::Console", "500", "Client Rect Right"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Bottom", "Extender::Console", "350", "Client Rect Bottom"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "LogCSWarnings", "Extender::Console", "1", "Log CS Warnings to the Console"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "LogAssertions", "Extender::Console", "1", "Log CS Assertions to the Console"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "HideOnStartup", "Extender::Console", "0", "Hide the console on CS startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "UpdatePeriod", "Extender::Console", "2000", "Duration, in milliseconds, between console window updates"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "LoadPluginOnStartup", "Extender::General", "0", "Load a plugin on CS startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "StartupPluginName", "Extender::General", "", "Name of the plugin, with extension, that is to be loaded on startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "OpenScriptWindowOnStartup", "Extender::General", "0", "Open an empty script editor window on startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "StartupScriptEditorID", "Extender::General", "", "EditorID of the script to be loaded on startup, should a script editor also be opened. An empty string results in a blank workspace"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "ShowNumericEditorIDWarning", "Extender::General", "1", "Display a warning when editorIDs start with an integer"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "SetWorkspaceOnStartup", "Extender::General", "0", "Set the working directory to a custom path on startup"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "DefaultWorkspacePath", "Extender::General", "", "Path of the custom workspace directory"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "SaveLoadedESPsAsMasters", "Extender::General", "1", "Save loaded plugin files as the active plugin's master"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "PreventTimeStampChanges", "Extender::General", "0", "Prevents the modifications to the timestamps of plugins being saved"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "UpdatePeriod", "Extender::Renderer", "8", "Duration, in milliseconds, between render window updates"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "DisplaySelectionStats", "Extender::Renderer", "1", "Display info on the render window selection"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "UpdateViewPortAsync", "Extender::Renderer", "0", "Allow the render window to be updated in the background"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "AltRefMovementSpeed", "Extender::Renderer", "0.8", "Alternate render window movement settings"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "AltRefSnapGrid", "Extender::Renderer", "2", "Alternate render window movement settings"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "AltRefRotationSpeed", "Extender::Renderer", "1.0", "Alternate render window movement settings"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "AltRefSnapAngle", "Extender::Renderer", "45", "Alternate render window movement settings"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "AltCamRotationSpeed", "Extender::Renderer", "1.0", "Alternate render window movement settings"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "AltCamZoomSpeed", "Extender::Renderer", "0.5", "Alternate render window movement settings"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "AltCamPanSpeed", "Extender::Renderer", "5.0", "Alternate render window movement settings"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "BackupOnSave", "Extender::VersionControl", "0", "Create a backup copy of the active plugin to the 'Backup' directory in the active workspace before commencing a save operation"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "GlobalScriptExecutionPeriod", "Extender::CSAS", "10", "Duration, in milliseconds, between global script execution operations"), (CreateINI == false));

	RegisterSetting(new SME::INI::INISetting(this, "Top", "Extender::AuxViewport", "150", "Client Rect Top"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Left", "Extender::AuxViewport", "150", "Client Rect Left"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Right", "Extender::AuxViewport", "500", "Client Rect Right"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "Bottom", "Extender::AuxViewport", "350", "Client Rect Bottom"), (CreateINI == false));
	RegisterSetting(new SME::INI::INISetting(this, "HideOnStartup", "Extender::AuxViewport", "0", "Hide the viewport on CS startup"), (CreateINI == false));

	if (CreateINI)		SaveSettingsToINI();
	else				ReadSettingsFromINI();
}

void WaitUntilDebuggerAttached()
{
	CONSOLE->Indent();
	DebugPrint("Waiting For Debugger...");
	while (!IsDebuggerPresent())
		Sleep(5000);
	DebugPrint("Debugger Attached!");
	CONSOLE->Exdent();
}
const char* PrintToBuffer(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf_s(g_TextBuffer, sizeof(g_TextBuffer), fmt, args);
	va_end(args);

	return g_TextBuffer;
}
void ToggleFlag(UInt8* Flag, UInt32 Mask, bool State)
{
	if (State)		*Flag |= Mask;
	else			*Flag &= ~Mask;
}
void ToggleFlag(UInt16* Flag, UInt32 Mask, bool State)
{
	if (State)		*Flag |= Mask;
	else			*Flag &= ~Mask;
}
void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
{
	if (State)		*Flag |= Mask;
	else			*Flag &= ~Mask;
}