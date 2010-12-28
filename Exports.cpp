#include "Exports.h"
#include "ExtenderInternals.h"
#include "SEHooks.h"
#include "[Common]\CLIWrapper.h"
#include "MiscHooks.h"
#include "resource.h"




extern "C"{

__declspec(dllexport) void _D_PRINT(UInt8 Source, const char* Message)
{
	DebugPrint(Source, Message);
}

__declspec(dllexport) const char* GetINIString(const char* Section, const char* Key, const char* Default)
{
	GetPrivateProfileString(Section, Key, Default, g_Buffer, sizeof(g_Buffer), g_INIPath.c_str());
	return (const char*)g_Buffer;
}

__declspec(dllexport) const char* GetAppPath(void){
	return g_AppPath.c_str();
}

__declspec(dllexport) void WriteStatusBarText(int PanelIndex, const char* Message)
{
	if (PanelIndex < 0 || PanelIndex > 2)	PanelIndex = 2;
	WriteToStatusBar(MAKEWPARAM(PanelIndex, 0), (LPARAM)Message);
}

__declspec(dllexport) HWND GetCSMainWindowHandle(void)
{
	return *g_HWND_CSParent;
}




__declspec(dllexport) void ScriptEditor_MessagingInterface(UInt32 TrackedEditorIndex, UInt16 Message)
{
	static UInt16 ReturnType[8] = 
	{
		0x9CDB,		// New
		0x9CD9,		// Open
		0x9D38,		// Previous
		0x9D39,		// Next
		0x9CDA,		// Save
		0x9D12,		// Recompile
		0x9D13,		// Delete
		0x9D14		// Close
	};

	HWND ScriptEditor = EDAL->GetTrackedDialog(TrackedEditorIndex);
	switch (Message)
	{
	case 5:
		EDAL->SetLastContactedEditor(0);
		break;
	default:
		EDAL->SetLastContactedEditor(TrackedEditorIndex);
		break;
	}
	
	SendMessage(ScriptEditor, WM_COMMAND, ReturnType[Message], NULL);
}


__declspec(dllexport) void ScriptEditor_SetScriptData(UInt32 TrackedEditorIndex, ScriptData* Data)
{
	HWND ScriptEditor = EDAL->GetTrackedDialog(TrackedEditorIndex);
	SetWindowText(EDAL->GetTrackedREC(ScriptEditor), (LPSTR)Data->Text);
	SendMessage(EDAL->GetTrackedLBC(ScriptEditor), CB_SETCURSEL, Data->Type, NULL);
	SendMessage(EDAL->GetTrackedREC(ScriptEditor), EM_SETMODIFY, Data->ModifiedFlag, NULL);
}

__declspec(dllexport) void ScriptEditor_SetWindowParameters(UInt32 TrackedEditorIndex, UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	HWND ScriptEditor = EDAL->GetTrackedDialog(TrackedEditorIndex);
	SetWindowPos(ScriptEditor, HWND_NOTOPMOST, Left, Top, Width, Height, SWP_NOZORDER);
}


__declspec(dllexport) UInt32 ScriptEditor_InstantiateCustomEditor(const char* ScriptID)
{
	if (ScriptID) {
		TESForm* Form = GetFormByID(ScriptID);
		if (!Form)									return 0;
		Script* ScriptForm = CS_CAST(Form, TESForm, Script);
		if (!ScriptForm)							return 0;

		g_EditorInitScript = ScriptForm;
	} else
		g_EditorInitScript = NULL;

	HWND CustomEditor = CreateDialogParamA(*g_TESCS_Instance, (LPCSTR)0xBC, NULL, g_ScriptEditor_DlgProc, NULL);
	g_EditorInitScript = NULL;
	UInt32 AllocatedIndex = EDAL->TrackNewEditor(CustomEditor);

	return AllocatedIndex;
}

__declspec(dllexport) void ScriptEditor_PostProcessEditorInit(UInt32 AllocatedIndex)
{
	HWND ScriptEditorHandle = EDAL->GetTrackedDialog(AllocatedIndex);
	ShowWindow(ScriptEditorHandle, SW_HIDE);

	if (IsWindowEnabled(EDAL->GetTrackedREC(ScriptEditorHandle))) {
		FillScriptDataPackage(g_SetEditorTextCache);
		CLIWrapper::ScriptEditor::InitializeScript(AllocatedIndex, g_ScriptDataPackage);
	}

}

__declspec(dllexport) ScriptData* ScriptEditor_GetScriptData()			// retrieves the cached script data
{
	if (!g_SetEditorTextCache) {
		g_ScriptDataPackage->Type = 99;									// trouble!
		return g_ScriptDataPackage;
	}

	FillScriptDataPackage(g_SetEditorTextCache);

	return g_ScriptDataPackage;
}

__declspec(dllexport) const char* ScriptEditor_GetAuxScriptName()
{
	if (g_EditorAuxScript)
		return g_EditorAuxScript->editorData.editorID.m_data;
	else
		return NULL;
}




__declspec(dllexport) ScriptData* FetchScriptFromForm(const char* EditorID)
{
	g_ScriptDataPackage->ParentID = NULL, g_ScriptDataPackage->EditorID = NULL;

	TESForm* Form = GetFormByID(EditorID);

	if (!Form)									return g_ScriptDataPackage;

	if (Form->IsReference()) {
		TESObjectREFR* Ref =  CS_CAST(Form, TESForm, TESObjectREFR);
		if (Ref)	Form = Ref->baseForm;
	}

	g_ScriptDataPackage->ParentID = Form->editorData.editorID.m_data;				// EditorID of the script's parent form

	TESScriptableForm* ScriptableForm = CS_CAST(Form, TESForm, TESScriptableForm);
	if (!ScriptableForm)						return g_ScriptDataPackage;
	Script* FormScript = ScriptableForm->script;
	if (!FormScript)							return g_ScriptDataPackage;

	FillScriptDataPackage(FormScript);		
	return g_ScriptDataPackage;
}



__declspec(dllexport) bool IsFormAnObjRefr(const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)						return false;
	else							return Form->IsReference();
}

__declspec(dllexport) void ScriptEditor_GetScriptListData(UInt32 TrackedEditorIndex)
{
	Script* ThisScript = NULL;
	DataHandler::Node<Script>* ThisNode = &(*g_dataHandler)->scripts;
	while (ThisNode) {
		ThisScript = ThisNode->data;
		if (!ThisScript)		break;
		else if (!ThisScript->editorData.editorID.m_data) {
			ThisNode = ThisNode->next;
			continue;
		}

		FillScriptDataPackage(ThisScript);

		CLIWrapper::ScriptEditor::SetScriptListItemData(TrackedEditorIndex, g_ScriptDataPackage);
		ThisNode = ThisNode->next;
	}
}

__declspec(dllexport) const char* ScriptEditor_GetScriptListItemText(const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)									return NULL;
	Script* ScriptForm = CS_CAST(Form, TESForm, Script);
	if (!ScriptForm)							return NULL;
	return ScriptForm->text;
}

__declspec(dllexport) void ScriptEditor_SetScriptListResult(const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)									return;
	Script* ScriptForm = CS_CAST(Form, TESForm, Script);
	if (!ScriptForm)							return;

	g_ScriptListResult = ScriptForm;
}

__declspec(dllexport) void ScriptEditor_GetUseReportForForm(const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)						return;

	CreateDialogParam(*g_TESCS_Instance, (LPCSTR)0xDC, NULL, g_UseReport_DlgProc, (LPARAM)Form);
}

__declspec(dllexport) void ScriptEditor_GetScriptVariableIndices(UInt32 TrackedEditorIndex, const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)						return;
	Script* ScriptForm = CS_CAST(Form, TESForm, Script);
	if (!ScriptForm)				return;

	Script::VarInfoEntry* ThisNode = &ScriptForm->varList;
	Script::VariableInfo* ThisVariable = NULL;

	ScriptVarIndexData::ScriptVarInfo Data;

	while (ThisNode) {
		ThisVariable = ThisNode->data;
		if (!ThisVariable)		break;

		Data.Name = ThisVariable->name.m_data;
		Data.Type = ThisVariable->type;
		if (Data.Type == 0) {
			for (Script::RefListEntry* entry = &ScriptForm->refList; entry; entry = entry->next) {
				if (entry->var && entry->var->name.m_data && !_stricmp(entry->var->name.m_data, Data.Name)) {
					Data.Type = 2;
				}
			}
		}
		Data.Index = ThisVariable->idx;
		CLIWrapper::ScriptEditor::SetVariableListItemData(TrackedEditorIndex, &Data);

		ThisNode = ThisNode->next;
	}
}

__declspec(dllexport) bool ScriptEditor_SetScriptVariableIndex(const char* EditorID, ScriptVarIndexData::ScriptVarInfo* Data)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)						return false;
	Script* ScriptForm = CS_CAST(Form, TESForm, Script);
	if (!ScriptForm)				return false;

	Script::VariableInfo* ScriptVar = ScriptForm->GetVariableByName(Data->Name);
	if (ScriptVar) {
		if (Data->Type == 2) {
			Script::RefVariable* RefVar = ScriptForm->GetVariable(ScriptVar->idx);
			if (RefVar) {
				RefVar->varIdx = Data->Index;
			} else return false;
		}
		ScriptVar->idx = Data->Index;
	} else return false;

	return true;
}

__declspec(dllexport) void ScriptEditor_CompileDependencies(const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)						return;
	Script* ScriptForm = CS_CAST(Form, TESForm, Script);
	if (!ScriptForm)				return;	

	DebugPrint("Recompiling dependencies of script %s {%08X}...", ScriptForm->editorData.editorID.m_data, ScriptForm->refID);
	CONSOLE->Indent();

	DebugPrint("Resolving script parent...");
	CONSOLE->Indent();
	switch (ScriptForm->info.type)
	{
	case Script::eType_Object:
	{
		DebugPrint("Source script type = Object Script");
		for (GenericNode<TESForm>* i = (GenericNode<TESForm>*)thisCall(kTESForm_GetObjectUseList, Form, 0); i != 0; i = i->next) {
			if (!i || !i->data) {
				DebugPrint("No Parents found!");
				break;
			}

			TESForm* Parent = i->data;
			TESScriptableForm* ValidParent = CS_CAST(Parent, TESForm, TESScriptableForm);

			if (ValidParent)
			{
				DebugPrint("Scriptable Form EDID = %s ; TYPE = %d", Parent->editorData.editorID.m_data, Parent->typeID);
				DebugPrint("Parsing cell use list...");
				CONSOLE->Indent();
				TESCellUseList* UseList = CS_CAST(Parent, TESForm, TESCellUseList);
				for (GenericNode<TESCellUseData>* CellList = (GenericNode<TESCellUseData>*)thisCall(kTESCellUseList_GetUseListRefHead, UseList); CellList; CellList = CellList->next) {
					TESCellUseData* Data = CellList->data;
					if (!Data)		break;
					
					for (TESObjectCELL::ObjectListEntry* RefList = &Data->Cell->objectList; RefList; RefList = RefList->Next()) {
						TESObjectREFR* ThisReference = RefList->Info();
						if (!ThisReference)		break;

						if (ThisReference->baseForm == Parent)
						{
							ScriptEditor_CompileDependencies_ParseObjectUseList((TESForm*)ThisReference);
						}
					}
				}
				CONSOLE->Exdent();
			}
		}
		break;
	}
	case Script::eType_Quest:
	{
		DebugPrint("Source script type = Quest Script");
		for (GenericNode<TESForm>* i = (GenericNode<TESForm>*)thisCall(kTESForm_GetObjectUseList, Form, 0); i != 0; i = i->next) {
			if (!i || !i->data) {
				DebugPrint("No Parents found!");
				break;
			}

			TESForm* Parent = i->data;
			if (Parent->typeID == kFormType_Quest)
			{
				DebugPrint("Quest EDID = %s", Parent->editorData.editorID.m_data);
				ScriptEditor_CompileDependencies_ParseObjectUseList(Parent);
			}
		}
		break;
	}
	}
	CONSOLE->Exdent();

	DebugPrint("Parsing direct dependencies...");
	ScriptEditor_CompileDependencies_ParseObjectUseList(Form);

	CONSOLE->Exdent();
	DebugPrint("Recompile operation completed!");
}

__declspec(dllexport) IntelliSenseUpdateData* ScriptEditor_BeginIntelliSenseDatabaseUpdate()
{
	IntelliSenseUpdateData* Data = new IntelliSenseUpdateData();

	UInt32 QuestCount = 0, ScriptCount = 0;
	for (DataHandler::Node<TESQuest>* Itr = &(*g_dataHandler)->quests; Itr; Itr = Itr->next)
	{
		if (Itr->data)
			QuestCount++;
	}

	ScriptData TestData;
	for (DataHandler::Node<Script>* Itr = &(*g_dataHandler)->scripts; Itr; Itr = Itr->next)
	{
		if (Itr->data)
		{
			TestData.FillScriptData(Itr->data);
			if (TestData.UDF)	ScriptCount++;
		}	
	}

	Data->QuestListHead = new QuestData[QuestCount];
	Data->QuestCount = QuestCount;
	Data->ScriptListHead = new ScriptData[ScriptCount];
	Data->ScriptCount = ScriptCount;

	QuestCount = 0, ScriptCount = 0;
	for (DataHandler::Node<TESQuest>* Itr = &(*g_dataHandler)->quests; Itr; Itr = Itr->next)
	{
		if (Itr->data)
		{
			Data->QuestListHead[QuestCount].FillFormData(Itr->data);
			Data->QuestListHead[QuestCount].FullName = Itr->data->fullName.name.m_data;
			Data->QuestListHead[QuestCount].ScriptName = NULL;
			if (Itr->data->scriptable.script)
			{
				Data->QuestListHead[QuestCount].ScriptName = Itr->data->scriptable.script->editorData.editorID.m_data;
			}
		}
		QuestCount++;
	}
		
	for (DataHandler::Node<Script>* Itr = &(*g_dataHandler)->scripts; Itr; Itr = Itr->next)
	{
		if (Itr->data)
		{
			TestData.FillScriptData(Itr->data);
			if (TestData.UDF)
			{
				Data->ScriptListHead[ScriptCount].FillFormData(Itr->data);
				Data->ScriptListHead[ScriptCount].FillScriptData(Itr->data);
				ScriptCount++;
			}
		}
	}

	return Data;
}

__declspec(dllexport) void ScriptEditor_EndIntelliSenseDatabaseUpdate(IntelliSenseUpdateData* Data)
{
	delete [] Data->ScriptListHead;
	delete [] Data->QuestListHead;
	delete Data;
}

__declspec(dllexport) void ScriptEditor_ToggleScriptCompiling(bool Enable)
{
	ToggleScriptCompiling(Enable);
}

__declspec(dllexport) void ScriptEditor_SaveActivePlugin()
{
	SendMessage(*g_HWND_CSParent, WM_COMMAND, 0x9CD2, NULL);
}



__declspec(dllexport) void UseInfoList_SetFormListItemText()
{
	// bound objects
	UInt32 Count = 0, Total = (*g_dataHandler)->boundObjects->boundObjectCount;
	WriteStatusBarText(0, "FormList += BoundObjects...");

	for (TESBoundObject* Itr = (*g_dataHandler)->boundObjects->last; Itr; Itr = Itr->next) {
		UIL_FormData->EditorID = Itr->editorData.editorID.m_data;
		UIL_FormData->FormID = Itr->refID;
		UIL_FormData->TypeID = Itr->typeID;
		CLIWrapper::UseInfoList::SetFormListItemData(UIL_FormData);
		Count++;

		sprintf_s(g_Buffer, sizeof(g_Buffer), "[%d/%d]", Count, Total);
		WriteStatusBarText(1, g_Buffer);
		WriteStatusBarText(2, UIL_FormData->EditorID);
	}

	// everything else.
	WriteStatusBarText(0, "FormList += Packages...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->packages);
	WriteStatusBarText(0, "FormList += WorldSpaces...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->worldSpaces);
	WriteStatusBarText(0, "FormList += Climates...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->climates);
	WriteStatusBarText(0, "FormList += Weathers...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->weathers);
	WriteStatusBarText(0, "FormList += Enchantments...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->enchantmentItems);
	WriteStatusBarText(0, "FormList += Spells...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->spellitems);
	WriteStatusBarText(0, "FormList += Hair...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->hairs);
	WriteStatusBarText(0, "FormList += Eyes...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->eyes);
	WriteStatusBarText(0, "FormList += Races...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->races);
	WriteStatusBarText(0, "FormList += LandTextures...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->landTextures);
	WriteStatusBarText(0, "FormList += Classes...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->classes);
	WriteStatusBarText(0, "FormList += Factions...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->factions);
	WriteStatusBarText(0, "FormList += Scripts...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->scripts);
	WriteStatusBarText(0, "FormList += Sounds...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->sounds);
	WriteStatusBarText(0, "FormList += Globals...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->globals);
	WriteStatusBarText(0, "FormList += Topics...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->topics);
	WriteStatusBarText(0, "FormList += Quests...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->quests);
	WriteStatusBarText(0, "FormList += Birthsigns...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->birthsigns);
	WriteStatusBarText(0, "FormList += CombatStyles...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->combatStyles);
	WriteStatusBarText(0, "FormList += LoadScreens...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->loadScreens);
	WriteStatusBarText(0, "FormList += WaterForms...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->waterForms);
	WriteStatusBarText(0, "FormList += EffectShaders...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->effectShaders);
	WriteStatusBarText(0, "FormList += AnimObjects...");
	UseInfoList_SetFormListItemText_ParseFormNode(&(*g_dataHandler)->objectAnios);


	WriteStatusBarText(0, ""), WriteStatusBarText(1, "");
	WriteStatusBarText(2, "UseInfoList Populated");
}

__declspec(dllexport) void UseInfoList_SetObjectListItemText(const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)									return;

	for (DataHandler::Node<TESForm>* UseList = (DataHandler::Node<TESForm>*)thisCall(kTESForm_GetObjectUseList, Form, 0); UseList; UseList = UseList->next) {
		TESForm* Reference = UseList->data;

		UIL_FormData->EditorID = Reference->editorData.editorID.m_data;
		UIL_FormData->FormID = Reference->refID;
		UIL_FormData->TypeID = Reference->typeID;

		CLIWrapper::UseInfoList::SetUseListObjectItemData(UIL_FormData);
	}
}

__declspec(dllexport) void UseInfoList_SetCellListItemText(const char* EditorID)
{
	TESForm* Form = GetFormByID(EditorID);
	if (!Form)									return;

	TESCellUseList* UseList = CS_CAST(Form, TESForm, TESCellUseList);
	for (GenericNode<TESCellUseData>* CellList = (GenericNode<TESCellUseData>*)thisCall(kTESCellUseList_GetUseListRefHead, UseList); CellList; CellList = CellList->next) {
		TESCellUseData* Data = CellList->data;
		if (!Data)		break;
		
		TESObjectREFR* FirstRef = TESForm_LoadIntoView_GetReference(Data->Cell, Form);
		TESWorldSpace* WorldSpace = (TESWorldSpace*)thisCall(kTESObjectCELL_GetParentWorldSpace, Data->Cell);

		UIL_CellData->EditorID = Data->Cell->editorData.editorID.m_data;
		UIL_CellData->FormID = Data->Cell->refID;
		UIL_CellData->Flags = Data->Cell->flags0 & TESObjectCELL::kFlags0_Interior;
		UIL_CellData->WorldEditorID = ((!WorldSpace)?"Interior":WorldSpace->editorData.editorID.m_data);
		UIL_CellData->RefEditorID = ((!FirstRef || !FirstRef->editorData.editorID.m_data)?"<Unnamed>":FirstRef->editorData.editorID.m_data);
		UIL_CellData->XCoord = Data->Cell->coords->x;
		UIL_CellData->YCoord = Data->Cell->coords->y;
		UIL_CellData->UseCount = Data->Count;

		CLIWrapper::UseInfoList::SetUseListCellItemData(UIL_CellData);
	}
}

__declspec(dllexport) void TESForm_LoadIntoView(const char* EditorID, const char* FormType)
{
	LoadFormIntoView(EditorID, FormType);
}

__declspec(dllexport) void BatchRefEditor_SetFormListItem(UInt8 ListID)
{
	switch(ListID)
	{
	case BatchRefData::kListID_NPC:
		{
		FormData Data;
		Data.EditorID = "NONE";
		Data.ParentForm = NULL;
		CLIWrapper::BatchEditor::AddFormListItem(&Data, ListID);

		for (TESBoundObject* Itr = (*g_dataHandler)->boundObjects->last; Itr; Itr = Itr->next) {
			TESNPC* NPC = CS_CAST(Itr, TESBoundObject, TESNPC);

			if (NPC) {
				Data.EditorID = Itr->editorData.editorID.m_data;
				Data.FormID = Itr->refID;
				Data.TypeID = Itr->typeID;
				Data.ParentForm = Itr;
				CLIWrapper::BatchEditor::AddFormListItem(&Data, ListID);
			}
		}
		break;
		}
	case BatchRefData::kListID_Global:
		BatchRefEditor_ParseFormNode(&(*g_dataHandler)->globals, ListID);
		break;
	case BatchRefData::kListID_Faction:
		BatchRefEditor_ParseFormNode(&(*g_dataHandler)->factions, ListID);
		break;
	}		
}

__declspec(dllexport) const char* BatchRefEditor_ChooseParentReference(BatchRefData* Data, HWND Parent)
{
	TESObjectREFR* Ref = NULL;
	while (true) {
		Ref = ChooseReferenceDlg(Parent);
		if (!Ref)	break;
		if (!Ref->IsPersistent()) {		// ### not required as the dialog doesn't enumerate non-persistent refs
			MessageBox(Parent, "The parent needs to be a presistent reference", "Choose Reference", MB_OK|MB_ICONERROR);
			continue;
		}
		break;
	}

	Data->EnableParent.Parent = Ref;
	if (Ref)
		sprintf_s(g_Buffer, sizeof(g_Buffer), "%08X", Ref->refID);
	else
		sprintf_s(g_Buffer, sizeof(g_Buffer), "NONE");
	return (!Ref || !Ref->editorData.editorID.m_data)?g_Buffer:Ref->editorData.editorID.m_data;
}

}

template <typename tData>
void UseInfoList_SetFormListItemText_ParseFormNode(DataHandler::Node<tData>* ThisNode)
{
	UInt32 Count = 0;
	while (ThisNode) {
		tData* ThisObject = ThisNode->data;
		if (!ThisObject)		break;

		UIL_FormData->EditorID = ThisObject->editorData.editorID.m_data;
		UIL_FormData->FormID = ThisObject->refID;
		UIL_FormData->TypeID = ThisObject->typeID;

		CLIWrapper::UseInfoList::SetFormListItemData(UIL_FormData);
		ThisNode = ThisNode->next;
		Count++;

		sprintf_s(g_Buffer, sizeof(g_Buffer), "[%d]", Count);
		WriteStatusBarText(1, g_Buffer);
	}
}

TESObjectREFR* TESForm_LoadIntoView_GetReference(TESObjectCELL* Cell, TESForm* Parent)
{
	TESObjectREFR* ThisReference = NULL;

	for (TESObjectCELL::ObjectListEntry* RefList = &Cell->objectList; RefList; RefList = RefList->Next()) {
		ThisReference = RefList->Info();
		if (!ThisReference)		break;
		if (ThisReference->baseForm == Parent && ThisReference->editorData.editorID.m_data) {
			return ThisReference;
		}
	}
	return NULL;
}

template <typename tData>
void BatchRefEditor_ParseFormNode(DataHandler::Node<tData>* ThisNode, UInt8 ListID)
{
	FormData Data;
	Data.EditorID = "NONE";
	Data.ParentForm = NULL;
	CLIWrapper::BatchEditor::AddFormListItem(&Data, ListID);

	while (ThisNode) {
		tData* ThisObject = ThisNode->data;
		if (!ThisObject)		break;

		Data.EditorID = ThisObject->editorData.editorID.m_data;
		Data.FormID = ThisObject->refID;
		Data.TypeID = ThisObject->typeID;
		Data.ParentForm = ThisObject;

		CLIWrapper::BatchEditor::AddFormListItem(&Data, ListID);
		ThisNode = ThisNode->next;
	}
}

UInt32 ScriptEditor_CompileDependencies_CheckConditions(ConditionEntry* Entry, TESForm* ToCompare)
{
	UInt32 ScriptableConditions = 0;

	for (ConditionEntry* j = Entry; j != 0; j = j->next) {
		if (!j || !j->data)		break;	

		ConditionEntry::Data* Data = j->data;
		if (Data->functionIndex == (53 & 0x0FFF) || j->data->functionIndex == (79 & 0x0FFF))			// GetScriptVariable || GetQuestVariable
		{
			if (Data->param1.form && Data->param1.form == ToCompare)
			{
				ScriptableConditions++;
			}
		}

	}
	return ScriptableConditions;
}

void ScriptEditor_CompileDependencies_ParseObjectUseList(TESForm* Form)
{
	DebugPrint("Parsing object use list of %08X...", Form->refID);
	CONSOLE->Indent();

	std::vector<Script*> ScriptDepends;		// updating usage info inside an use list loop invalidates the list.
	std::vector<TESTopicInfo*> InfoDepends; // so store the objects ptrs and parse them later
	std::vector<TESQuest*> QuestDepends;

	for (GenericNode<TESForm>* i = (GenericNode<TESForm>*)thisCall(kTESForm_GetObjectUseList, Form, 0); i != 0; i = i->next) {
		if (!i || !i->data) {
			DebugPrint("No dependencies found!");
			break;
		}

		TESForm* Depends = i->data;
		switch (Depends->typeID)
		{
		case kFormType_DialogInfo:
		{
			InfoDepends.push_back(CS_CAST(Depends, TESForm, TESTopicInfo));
			break;
		}
		case kFormType_Quest:
		{
			QuestDepends.push_back(CS_CAST(Depends, TESForm, TESQuest));
			break;
		}
		case kFormType_Script:
		{
			ScriptDepends.push_back(CS_CAST(Depends, TESForm, Script));
			break;
		}
		default:	// ### any other type that needs handling ?
	//		DebugPrint("Skipping form type %d", Depends->typeID);
			break;
		}
	}

	// scripts
	for (std::vector<Script*>::const_iterator Itr = ScriptDepends.begin(); Itr != ScriptDepends.end(); Itr++)
	{
		DebugPrint("Script %s {%08X}:", (*Itr)->editorData.editorID.m_data, (*Itr)->refID);
		CONSOLE->Indent();
		
		if ((*Itr)->info.dataLength > 0) {
			if (!thisCall(kScript_SaveScript, g_ScriptCompilerUnkObj, (*Itr), 0)) {
				DebugPrint("Script failed to compile due to errors!");
			}
		}

		CONSOLE->Exdent();		
	}
	// quests
	for (std::vector<TESQuest*>::const_iterator Itr = QuestDepends.begin(); Itr != QuestDepends.end(); Itr++)
	{
		DebugPrint("Quest %s {%08X}:", (*Itr)->editorData.editorID.m_data, (*Itr)->refID);
		CONSOLE->Indent();

		for (TESQuest::StageEntry* j = &(*Itr)->stageList; j != 0; j = j->next) 
		{
			if (!j->data)		break;

			for (TESQuest::StageItemList* l = &j->data->itemList; l != 0; l = l->next) 
			{
				if (!l->item)		break;

				QuestStageItem* QuestStage = l->item;
				if (QuestStage->resultScript.info.dataLength > 0) {
					if (!thisCall(kScript_SaveResultScript, g_ScriptCompilerUnkObj, &QuestStage->resultScript, 0, 0)) {
						DebugPrint("Result script in stage %d-%d failed to compile due to errors!", j->data->index, QuestStage->index);
					}
				}

				DebugPrint("Found %d conditions in stage %d-%d that referenced source script", ScriptEditor_CompileDependencies_CheckConditions(&QuestStage->conditionList, Form), j->data->index, QuestStage->index);
			}
		}

		for (TESQuest::TargetEntry* j = &(*Itr)->targetList; j != 0; j = j->next) 
		{
			if (!j->data)		break;

			DebugPrint("Found %d conditions in target entry {%08X} that referenced source script", ScriptEditor_CompileDependencies_CheckConditions(&j->data->conditionList, Form), j->data->target->refID);
		}

		thisVirtualCall(kVTBL_TESQuest, 0x104, (*Itr));	// UpdateUsageInfo.
		CONSOLE->Exdent();	
	}
	// topic infos
	for (std::vector<TESTopicInfo*>::const_iterator Itr = InfoDepends.begin(); Itr != InfoDepends.end(); Itr++)
	{
		DebugPrint("Topic info %08X:", (*Itr)->refID);
		CONSOLE->Indent();
		
		if ((*Itr)->resultScript.info.dataLength > 0) {
			if (!thisCall(kScript_SaveResultScript, g_ScriptCompilerUnkObj, &(*Itr)->resultScript, 0, 0)) {
				DebugPrint("Result script failed to compile due to errors!");
			}
		}

		DebugPrint("Found %d conditions that referenced source script", ScriptEditor_CompileDependencies_CheckConditions(&(*Itr)->conditions, Form));
		thisVirtualCall(kVTBL_TESTopicInfo, 0x104, (*Itr));	// UpdateUsageInfo
		CONSOLE->Exdent();	
	}

	CONSOLE->Exdent();
	DebugPrint("Operation complete!");
}