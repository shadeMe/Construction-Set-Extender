#include "CLIWrapper.h"
#include "Exports.h"

CLIWrapper::_SE_AllocateNewEditor				CLIWrapper::SE_AllocateNewEditor = NULL;
CLIWrapper::_SE_InitializeScript				CLIWrapper::SE_InitializeScript = NULL;
CLIWrapper::_SE_SendMessagePingback				CLIWrapper::SE_SendMessagePingback = NULL;


CLIWrapper::_SE_InitializeComponents			CLIWrapper::SE_InitializeComponents = NULL;
CLIWrapper::_SE_AddToURLMap						CLIWrapper::SE_AddToURLMap = NULL;
CLIWrapper::_SE_SetScriptListItemData			CLIWrapper::SE_SetScriptListItemData = NULL;

CLIWrapper::_SE_SetVariableListItemData			CLIWrapper::SE_SetVariableListItemData = NULL;

CLIWrapper::_UIL_OpenUseInfoBox					CLIWrapper::UIL_OpenUseInfoBox = NULL;
CLIWrapper::_UIL_SetFormListItemData			CLIWrapper::UIL_SetFormListItemData = NULL;
CLIWrapper::_UIL_SetUseListObjectItemData		CLIWrapper::UIL_SetUseListObjectItemData = NULL;
CLIWrapper::_UIL_SetUseListCellItemData			CLIWrapper::UIL_SetUseListCellItemData = NULL;

CLIWrapper::_BSAV_InitializeViewer				CLIWrapper::BSAV_InitializeViewer = NULL;
CLIWrapper::_BE_InitializeRefBatchEditor		CLIWrapper::BE_InitializeRefBatchEditor = NULL;
CLIWrapper::_BE_AddFormListItem					CLIWrapper::BE_AddFormListItem = NULL;


bool CLIWrapper::Import(const OBSEInterface * obse)
{
	SetErrorMode(0);
	HMODULE hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\ScriptEditor.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load ScriptEditor.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::SE_AllocateNewEditor = (CLIWrapper::_SE_AllocateNewEditor)GetProcAddress(hMod, "AllocateNewEditor");
	CLIWrapper::SE_InitializeScript = (CLIWrapper::_SE_InitializeScript)GetProcAddress(hMod, "InitializeScript");
	CLIWrapper::SE_SendMessagePingback = (CLIWrapper::_SE_SendMessagePingback)GetProcAddress(hMod, "SendMessagePingback");

	CLIWrapper::SE_InitializeComponents = (CLIWrapper::_SE_InitializeComponents)GetProcAddress(hMod, "InitializeComponents");
	CLIWrapper::SE_AddToURLMap = (CLIWrapper::_SE_AddToURLMap)GetProcAddress(hMod, "AddToURLMap");

	CLIWrapper::SE_SetScriptListItemData = (CLIWrapper::_SE_SetScriptListItemData)GetProcAddress(hMod, "SetScriptListItemData");
	CLIWrapper::SE_SetVariableListItemData = (CLIWrapper::_SE_SetVariableListItemData)GetProcAddress(hMod, "SetVariableListItemData");


	if (!SE_AddToURLMap || 
		!SE_AllocateNewEditor || 
		!SE_InitializeComponents ||
		!SE_InitializeScript ||
		!SE_SendMessagePingback ||
		!SE_SetScriptListItemData ||
		!SE_SetVariableListItemData)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\UseInfoList.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load UseInfoList.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::UIL_OpenUseInfoBox = (CLIWrapper::_UIL_OpenUseInfoBox)GetProcAddress(hMod, "OpenUseInfoBox");
	CLIWrapper::UIL_SetFormListItemData = (CLIWrapper::_UIL_SetFormListItemData)GetProcAddress(hMod, "SetFormListItemData");
	CLIWrapper::UIL_SetUseListObjectItemData = (CLIWrapper::_UIL_SetUseListObjectItemData)GetProcAddress(hMod, "SetUseListObjectItemData");
	CLIWrapper::UIL_SetUseListCellItemData = (CLIWrapper::_UIL_SetUseListCellItemData)GetProcAddress(hMod, "SetUseListCellItemData");

	if (!UIL_OpenUseInfoBox || 
		!UIL_SetFormListItemData ||
		!UIL_SetUseListObjectItemData || 
		!UIL_SetUseListCellItemData)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\BSAViewer.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load BSAViewer.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::BSAV_InitializeViewer = (CLIWrapper::_BSAV_InitializeViewer)GetProcAddress(hMod, "InitializeViewer");

	if (!BSAV_InitializeViewer)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\ComponentDLLs\\CSE\\BatchEditor.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load BatchEditor.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::BE_InitializeRefBatchEditor = (CLIWrapper::_BE_InitializeRefBatchEditor)GetProcAddress(hMod, "InitializeRefBatchEditor");
	CLIWrapper::BE_AddFormListItem = (CLIWrapper::_BE_AddFormListItem)GetProcAddress(hMod, "AddFormListItem");

	if (!BE_InitializeRefBatchEditor ||
		!BE_AddFormListItem)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}


	return true;
}