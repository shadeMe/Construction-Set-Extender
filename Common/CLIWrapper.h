#pragma once
#include "ExtenderInternals.h"
#include "SEHooks.h"
#include "Common\HandShakeStructs.h"

class CLIWrapper
{
	typedef void							(*_SE_AllocateNewEditor)(UInt32, UInt32, UInt32, UInt32);
	typedef void							(*_SE_InitializeScript)(UInt32, ScriptData*);
	typedef void							(*_SE_SendMessagePingback)(UInt32, UInt16);

	typedef void							(*_SE_InitializeComponents)(CommandTableData*);
	typedef void							(*_SE_AddToURLMap)(const char*, const char*);
	typedef void							(*_SE_SetScriptListItemData)(UInt32, ScriptData*);

	typedef void							(*_SE_SetVariableListItemData)(UInt32, ScriptVarIndexData::ScriptVarInfo*);

	typedef void							(*_UIL_OpenUseInfoBox)(void);
	typedef void							(*_UIL_SetFormListItemData)(FormData*);
	typedef void							(*_UIL_SetUseListObjectItemData)(FormData*);
	typedef void							(*_UIL_SetUseListCellItemData)(UseListCellItemData*);

	typedef const char*						(*_BSAV_InitializeViewer)(const char*, const char*);
	typedef bool							(*_BE_InitializeRefBatchEditor)(BatchRefData*);
	typedef void							(*_BE_AddFormListItem)(FormData*, UInt8);
public:
	static _SE_AllocateNewEditor			SE_AllocateNewEditor;
	static _SE_InitializeScript				SE_InitializeScript;
	static _SE_SendMessagePingback			SE_SendMessagePingback;


	static _SE_InitializeComponents			SE_InitializeComponents;
	static _SE_AddToURLMap					SE_AddToURLMap;
	static _SE_SetScriptListItemData		SE_SetScriptListItemData;

	static _SE_SetVariableListItemData		SE_SetVariableListItemData;

	static _UIL_OpenUseInfoBox				UIL_OpenUseInfoBox;
	static _UIL_SetFormListItemData			UIL_SetFormListItemData;
	static _UIL_SetUseListObjectItemData	UIL_SetUseListObjectItemData;
	static _UIL_SetUseListCellItemData		UIL_SetUseListCellItemData;
	
	static _BSAV_InitializeViewer			BSAV_InitializeViewer;
	static _BE_InitializeRefBatchEditor		BE_InitializeRefBatchEditor;
	static _BE_AddFormListItem				BE_AddFormListItem;

	static bool								Import(const OBSEInterface * obse);
};