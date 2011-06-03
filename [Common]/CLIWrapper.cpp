#include "CLIWrapper.h"
#include "Exports.h"
#include "ExtenderInternals.h"

namespace CLIWrapper
{

namespace ScriptEditor
{
	_AllocateNewEditor						AllocateNewEditor;
	_InitializeScript						InitializeScript;
	_SendMessagePingback					SendMessagePingback;


	_InitializeComponents					InitializeComponents;
	_AddToURLMap							AddToURLMap;
	_SetScriptListItemData					SetScriptListItemData;

	_SetVariableListItemData				SetVariableListItemData;
	_InitializeDatabaseUpdateTimer			InitializeDatabaseUpdateTimer;
	_PassScriptError						PassScriptError;
}
namespace UseInfoList
{
	_OpenUseInfoBox							OpenUseInfoBox;
	_SetFormListItemData					SetFormListItemData;
	_SetUseListObjectItemData				SetUseListObjectItemData;
	_SetUseListCellItemData					SetUseListCellItemData;
}
namespace BSAViewer
{
	_InitializeViewer						InitializeViewer;
}
namespace BatchEditor
{
	_InitializeRefBatchEditor				InitializeRefBatchEditor;
	_AddFormListItem						AddFormListItem;
}
namespace TagBrowser
{
	_Show									Show;
	_Hide									Hide;
	_AddFormToActiveTag						AddFormToActiveTag;
	_GetFormDropWindowHandle				GetFormDropWindowHandle;
	_GetFormDropParentHandle				GetFormDropParentHandle;
}

bool CLIWrapper::Import(const OBSEInterface * obse)
{
	SetErrorMode(0);
	HMODULE hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\CSE\\ScriptEditor.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load ScriptEditor.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::ScriptEditor::AllocateNewEditor = (CLIWrapper::ScriptEditor::_AllocateNewEditor)GetProcAddress(hMod, "AllocateNewEditor");
	CLIWrapper::ScriptEditor::InitializeScript = (CLIWrapper::ScriptEditor::_InitializeScript)GetProcAddress(hMod, "InitializeScript");
	CLIWrapper::ScriptEditor::SendMessagePingback = (CLIWrapper::ScriptEditor::_SendMessagePingback)GetProcAddress(hMod, "SendMessagePingback");
	CLIWrapper::ScriptEditor::InitializeComponents = (CLIWrapper::ScriptEditor::_InitializeComponents)GetProcAddress(hMod, "InitializeComponents");
	CLIWrapper::ScriptEditor::AddToURLMap = (CLIWrapper::ScriptEditor::_AddToURLMap)GetProcAddress(hMod, "AddToURLMap");
	CLIWrapper::ScriptEditor::SetScriptListItemData = (CLIWrapper::ScriptEditor::_SetScriptListItemData)GetProcAddress(hMod, "SetScriptListItemData");
	CLIWrapper::ScriptEditor::SetVariableListItemData = (CLIWrapper::ScriptEditor::_SetVariableListItemData)GetProcAddress(hMod, "SetVariableListItemData");
	CLIWrapper::ScriptEditor::InitializeDatabaseUpdateTimer = (CLIWrapper::ScriptEditor::_InitializeDatabaseUpdateTimer)GetProcAddress(hMod, "InitializeDatabaseUpdateTimer");
	CLIWrapper::ScriptEditor::PassScriptError = (CLIWrapper::ScriptEditor::_PassScriptError)GetProcAddress(hMod, "PassScriptError");


	if (!CLIWrapper::ScriptEditor::AddToURLMap || 
		!CLIWrapper::ScriptEditor::AllocateNewEditor || 
		!CLIWrapper::ScriptEditor::InitializeComponents ||
		!CLIWrapper::ScriptEditor::InitializeScript ||
		!CLIWrapper::ScriptEditor::SendMessagePingback ||
		!CLIWrapper::ScriptEditor::SetScriptListItemData ||
		!CLIWrapper::ScriptEditor::SetVariableListItemData ||
		!CLIWrapper::ScriptEditor::InitializeDatabaseUpdateTimer ||
		!CLIWrapper::ScriptEditor::PassScriptError)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\CSE\\UseInfoList.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load UseInfoList.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::UseInfoList::OpenUseInfoBox = (CLIWrapper::UseInfoList::_OpenUseInfoBox)GetProcAddress(hMod, "OpenUseInfoBox");
	CLIWrapper::UseInfoList::SetFormListItemData = (CLIWrapper::UseInfoList::_SetFormListItemData)GetProcAddress(hMod, "SetFormListItemData");
	CLIWrapper::UseInfoList::SetUseListObjectItemData = (CLIWrapper::UseInfoList::_SetUseListObjectItemData)GetProcAddress(hMod, "SetUseListObjectItemData");
	CLIWrapper::UseInfoList::SetUseListCellItemData = (CLIWrapper::UseInfoList::_SetUseListCellItemData)GetProcAddress(hMod, "SetUseListCellItemData");

	if (!CLIWrapper::UseInfoList::OpenUseInfoBox || 
		!CLIWrapper::UseInfoList::SetFormListItemData ||
		!CLIWrapper::UseInfoList::SetUseListObjectItemData || 
		!CLIWrapper::UseInfoList::SetUseListCellItemData)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\CSE\\BSAViewer.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load BSAViewer.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::BSAViewer::InitializeViewer = (CLIWrapper::BSAViewer::_InitializeViewer)GetProcAddress(hMod, "InitializeViewer");

	if (!CLIWrapper::BSAViewer::InitializeViewer)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\CSE\\BatchEditor.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load BatchEditor.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::BatchEditor::InitializeRefBatchEditor = (CLIWrapper::BatchEditor::_InitializeRefBatchEditor)GetProcAddress(hMod, "InitializeRefBatchEditor");
	CLIWrapper::BatchEditor::AddFormListItem = (CLIWrapper::BatchEditor::_AddFormListItem)GetProcAddress(hMod, "AddFormListItem");

	if (!CLIWrapper::BatchEditor::InitializeRefBatchEditor ||
		!CLIWrapper::BatchEditor::AddFormListItem)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	hMod = LoadLibrary(std::string(std::string(obse->GetOblivionDirectory()) + "Data\\OBSE\\Plugins\\CSE\\TagBrowser.dll").c_str());
	if (hMod == NULL) {
		DebugPrint("Couldn't load TagBrowser.dll");
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}

	CLIWrapper::TagBrowser::Show = (CLIWrapper::TagBrowser::_Show)GetProcAddress(hMod, "Show");
	CLIWrapper::TagBrowser::Hide = (CLIWrapper::TagBrowser::_Hide)GetProcAddress(hMod, "Hide");
	CLIWrapper::TagBrowser::AddFormToActiveTag = (CLIWrapper::TagBrowser::_AddFormToActiveTag)GetProcAddress(hMod, "AddFormToActiveTag");
	CLIWrapper::TagBrowser::GetFormDropWindowHandle = (CLIWrapper::TagBrowser::_GetFormDropWindowHandle)GetProcAddress(hMod, "GetFormDropWindowHandle");
	CLIWrapper::TagBrowser::GetFormDropParentHandle = (CLIWrapper::TagBrowser::_GetFormDropParentHandle)GetProcAddress(hMod, "GetFormDropParentHandle");

	if (!CLIWrapper::TagBrowser::Show ||
		!CLIWrapper::TagBrowser::Hide || 
		!CLIWrapper::TagBrowser::AddFormToActiveTag ||
		!CLIWrapper::TagBrowser::GetFormDropWindowHandle ||
		!CLIWrapper::TagBrowser::GetFormDropParentHandle)
	{	
		LogWinAPIErrorMessage(GetLastError());
		return false;
	}




	return true;
}

}