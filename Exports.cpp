#include "[Common]\CLIWrapper.h"

#include "Main.h"
#include "Hooks\Hooks-CompilerErrorDetours.h"
#include "Hooks\Hooks-Misc.h"
#include "Hooks\Hooks-ScriptEditor.h"
#include "Hooks\Hooks-Plugins.h"
#include "Hooks\Hooks-Dialog.h"
#include "CustomDialogProcs.h"

#include <bgsee\WorkspaceManager.h>

using namespace componentDLLInterface;
using namespace cse;
using namespace cse::hooks;

extern componentDLLInterface::CSEInterfaceTable g_InteropInterface;

extern "C"
{
	QUERYINTERFACE_EXPORT
	{
		return &g_InteropInterface;
	}
}

void DeleteInterOpData(IDisposableData* Pointer, bool IsArray)
{
	if (IsArray)
		delete[] Pointer;
	else
		delete Pointer;
}

void DeleteData(void* Pointer, bool IsArray)
{
	if (IsArray)
		delete[] Pointer;
	else
		delete Pointer;
}

/**** BEGIN EDITORAPI SUBINTERFACE ****/
#pragma region EditorAPI
void ComponentDLLDebugPrint(UInt8 Source, const char* Message)
{
	const char* Prefix = "";

	enum MessageSource
	{
		e_BE = 2,
		e_UL,
		e_SE,
		e_BSA,
		e_TAG
	};

	bool InvalidPrefix = false;
	switch (Source)
	{
	case e_BE:
		Prefix = "BE";
		break;
	case e_UL:
		Prefix = "UL";
		break;
	case e_SE:
		Prefix = "SE";
		break;
	case e_BSA:
		Prefix = "BSA";
		break;
	case e_TAG:
		Prefix = "TAG";
		break;
	default:
		SME_ASSERT(InvalidPrefix);
		break;
	}

	BGSEECONSOLE->LogMsg(Prefix, Message);
}

const char* GetAppPath(void)
{
	return BGSEEMAIN->GetAPPPath();
}

void WriteToStatusBar(int PanelIndex, const char* Message)
{
	if (PanelIndex < 0 || PanelIndex > 3)
		PanelIndex = 3;

	TESCSMain::WriteToStatusBar(MAKEWPARAM(PanelIndex, 0), Message);
}

HWND GetCSMainWindowHandle(void)
{
	return *TESCSMain::WindowHandle;
}

HWND GetRenderWindowHandle(void)
{
	return *TESRenderWindow::WindowHandle;
}

FormData* LookupFormByEditorID(const char* EditorID)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	FormData* Result = nullptr;

	if (Form)
	{
		Result = new FormData();
		Result->FillFormData(Form);
	}

	return Result;
}

ScriptData* LookupScriptableByEditorID(const char* EditorID)
{
	ScriptData* Result = nullptr;
	TESForm* Form = TESForm::LookupByEditorID(EditorID);

	if (Form)
	{
		if (Form->IsReference())
		{
			TESObjectREFR* Ref =  CS_CAST(Form, TESForm, TESObjectREFR);
			if (Ref)
				Form = Ref->baseForm;
		}

		if (Form->formType == TESForm::kFormType_Script)
		{
			Result = new ScriptData();
			Result->FillScriptData(CS_CAST(Form, TESForm, Script));
			Result->ParentID = nullptr;
		}
		else
		{
			TESScriptableForm* ScriptableForm = CS_CAST(Form, TESForm, TESScriptableForm);
			if (ScriptableForm)
			{
				Script* FormScript = ScriptableForm->script;
				if (FormScript)
				{
					Result = new ScriptData();
					Result->FillScriptData(FormScript);
					Result->ParentID = Form->editorID.c_str();				// EditorID of the script's parent form
				}
			}
		}
	}

	return Result;
}

bool GetIsFormReference(const char* EditorID)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	bool Result = false;

	if (Form)
		Result = Form->IsReference();

	return Result;
}

const char* GetFormTypeIDLongName(UInt8 TypeID)
{
	return TESForm::GetFormTypeIDLongName(TypeID);
}

void LoadFormForEditByEditorID(const char* EditorID)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	if (Form)
	{
		switch (Form->formType)
		{
		case TESForm::kFormType_Script:
			TESDialog::ShowScriptEditorDialog(Form);
			break;
		case TESForm::kFormType_REFR:
			_TES->LoadCellIntoViewPort((CS_CAST(Form, TESForm, TESObjectREFR))->GetPosition(), CS_CAST(Form, TESForm, TESObjectREFR));
			break;
		default:
			TESDialog::ShowFormEditDialog(Form);
			break;
		}
	}
}

void LoadFormForEditByFormID(UInt32 FormID)
{
	TESForm* Form = TESForm::LookupByFormID(FormID);
	if (Form)
	{
		switch (Form->formType)
		{
		case TESForm::kFormType_Script:
			TESDialog::ShowScriptEditorDialog(Form);
			break;
		case TESForm::kFormType_REFR:
			_TES->LoadCellIntoViewPort((CS_CAST(Form, TESForm, TESObjectREFR))->GetPosition(), CS_CAST(Form, TESForm, TESObjectREFR));
			break;
		default:
			TESDialog::ShowFormEditDialog(Form);
			break;
		}
	}
}

FormData* ShowPickReferenceDialog(HWND Parent)
{
	TESObjectREFR* Ref = RefSelectControl::ShowSelectReferenceDialog(Parent, nullptr, true);

	if (!Ref)
		return nullptr;
	else
		return new FormData(Ref);
}

void ShowUseReportDialog(const char* EditorID)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);

	if (Form)
		TESDialog::ShowUseReportDialog(Form);
}

void SaveActivePlugin(void)
{
	SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kToolbar_Save, NULL);
}

void ReadFromINI(const char* Setting, const char* Section, const char* Default, char* OutBuffer, UInt32 Size)
{
	BGSEEMAIN->INIGetter()(Setting, Section, Default, OutBuffer, Size);
}

void WriteToINI(const char* Setting, const char* Section, const char* Value)
{
	BGSEEMAIN->INISetter()(Setting, Section, Value);
}

UInt32 GetFormListActiveItemForegroundColor(void)
{
	 return SME::StringHelpers::GetRGB(settings::dialogs::kActiveFormForeColor.GetData().s);
}

UInt32 GetFormListActiveItemBackgroundColor(void)
{
	return SME::StringHelpers::GetRGB(settings::dialogs::kActiveFormBackColor.GetData().s);
}

bool GetShouldColorizeActiveForms(void)
{
	return settings::dialogs::kColorizeActiveForms.GetData().i;
}

bool GetShouldSortActiveFormsFirst(void)
{
	return settings::dialogs::kSortFormListsByActiveForm.GetData().i;
}
#pragma endregion
/**** END EDITORAPI SUBINTERFACE ****/

/**** BEGIN SCRIPTEDITOR SUBINTERFACE ****/
#pragma region ScriptEditor
ScriptData* CreateNewScript(void)
{
	Script* NewInstance = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Script), TESForm, Script);
	ScriptData* Data = new ScriptData(NewInstance);
	NewInstance->SetFromActiveFile(true);
	_DATAHANDLER->scripts.AddAt(NewInstance, eListEnd);
	_DATAHANDLER->SortScripts();

	return Data;
}

bool CompileScript(ScriptCompileData* Data)
{
	Script* ScriptForm = CS_CAST(Data->Script.ParentForm, TESForm, Script);
	char Buffer[0x200] = {0};

	if ((ScriptForm->formFlags & TESForm::kFormFlags_Deleted))
	{
		BGSEEUI->MsgBoxI(nullptr,
						MB_TASKMODAL|MB_TOPMOST|MB_SETFOREGROUND|MB_OK,
						"Script %s {%08X} has been deleted, ergo it cannot be compiled.", ScriptForm->editorID.c_str(), ScriptForm->formID);

		Data->CompileResult = false;
	}
	else
	{
		BSString* OldText = BSString::CreateInstance(ScriptForm->text);

		ScriptForm->info.type = Data->Script.Type;
		ScriptForm->UpdateUsageInfo();
		ScriptForm->SetText(Data->Script.Text);
		ScriptForm->SetFromActiveFile(true);
		ScriptForm->compileResult = Data->CompileResult = ScriptForm->Compile();
		if (ScriptForm->compileResult)
		{
			_DATAHANDLER->SortScripts();
			Data->Script.FillScriptData(ScriptForm);
		}
		else
		{
			Data->CompileErrorData.Count = TESScriptCompiler::AuxiliaryErrorDepot.size();
			if (TESScriptCompiler::AuxiliaryErrorDepot.size())
			{
				Data->CompileErrorData.ErrorListHead = new ScriptErrorListData::ErrorData[Data->CompileErrorData.Count];

				for (int i = 0; i < Data->CompileErrorData.Count; i++)
				{
					TESScriptCompiler::CompilerErrorData* Error = &TESScriptCompiler::AuxiliaryErrorDepot[i];
					Data->CompileErrorData.ErrorListHead[i].Line = Error->Line;
					Data->CompileErrorData.ErrorListHead[i].Message = Error->Message.c_str();
				}
			}
			else
				Data->CompileErrorData.ErrorListHead = nullptr;

			ScriptForm->SetText(OldText->c_str());
		}

		OldText->DeleteInstance();
	}

	return Data->CompileResult;
}

void RecompileScripts(void)
{
	TESScriptCompiler::PreventErrorDetours = true;

	UInt32 ScriptCount = 0, Current = 0;

	for (tList<Script>::Iterator Itr = _DATAHANDLER->scripts.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		Script* ScriptForm = Itr.Get();
		if ((ScriptForm->formFlags & TESForm::kFormFlags_Deleted) == 0 &&
			(ScriptForm->formFlags & TESForm::kFormFlags_FromActiveFile))
		{
			ScriptCount++;
		}
	}

	HWND NotificationDialog = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), BGSEEUI->GetMainWindow(), nullptr, NULL);
	Static_SetText(GetDlgItem(NotificationDialog, -1), "Please Wait");
	char Buffer[0x200] = {0};

	BGSEECONSOLE_MESSAGE("Recompiling active scripts...");
	BGSEECONSOLE->Indent();

	static const UInt32 kPreprocessorBufferSize = 2 * 1024 * 1024;
	char* PreprocessedTextBuffer = new char[kPreprocessorBufferSize];
	for (tList<Script>::Iterator Itr = _DATAHANDLER->scripts.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		Script* ScriptForm = Itr.Get();
		if ((ScriptForm->formFlags & TESForm::kFormFlags_Deleted) == 0 &&
			(ScriptForm->formFlags & TESForm::kFormFlags_FromActiveFile))
		{
			BGSEECONSOLE_MESSAGE("Script '%s' {%08X}:", ScriptForm->editorID.c_str(), ScriptForm->formID);
			BGSEECONSOLE->Indent();

			Current++;
			FORMAT_STR(Buffer, "Please Wait\nCompiling Script %d/%d", Current, ScriptCount);
			Static_SetText(GetDlgItem(NotificationDialog, -1), Buffer);

			ZeroMemory(PreprocessedTextBuffer, kPreprocessorBufferSize);
			bool PreprocessResult = cliWrapper::interfaces::SE->PreprocessScript(ScriptForm->text, PreprocessedTextBuffer, kPreprocessorBufferSize);
			if (PreprocessResult)
			{
				BSString* OldText = BSString::CreateInstance(ScriptForm->text);

				ScriptForm->SetText(PreprocessedTextBuffer);
				ScriptForm->Compile();
				ScriptForm->SetText(OldText->c_str());

				OldText->DeleteInstance();
			}
			else
			{
				BGSEECONSOLE_MESSAGE("Preprocessing failed!");
			}

			BGSEECONSOLE->Outdent();
		}
	}

	delete [] PreprocessedTextBuffer;
	BGSEECONSOLE->Outdent();

	DestroyWindow(NotificationDialog);
	BGSEECONSOLE_MESSAGE("Recompile active scripts operation completed!");

	TESScriptCompiler::PreventErrorDetours = false;
}

void ToggleScriptCompilation(bool State)
{
	TESScriptCompiler::ToggleScriptCompilation(State);
}

void DeleteScript(const char* EditorID)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	if (Form)
	{
		Script* ScriptForm = CS_CAST(Form, TESForm, Script);
		if (ScriptForm)
		{
			ScriptForm->SetDeleted(true);
			ScriptForm->UpdateUsageInfo();
		}
	}
}

enum
{
	kDirection_Forward = 0,
	kDirection_Backward,
};

Script* GetScriptNeighbour(Script* Current, UInt8 Direction)
{
	SME_ASSERT(_DATAHANDLER->scripts.Count());

	int Index = _DATAHANDLER->scripts.IndexOf(Current);
	Script* Result = nullptr;

	switch (Direction)
	{
	case kDirection_Forward:
		if (Index + 1 < _DATAHANDLER->scripts.Count())
			Result = _DATAHANDLER->scripts.GetNthItem(Index + 1);
		else
			Result = _DATAHANDLER->scripts.GetNthItem(0);

		break;
	case kDirection_Backward:
		if (Index - 1 > -1)
			Result = _DATAHANDLER->scripts.GetNthItem(Index - 1);
		else
			Result = _DATAHANDLER->scripts.GetLastItem();

		break;
	}

	if (Result->GetEditorID() == nullptr)
		Result = GetScriptNeighbour(Result, Direction);

	return Result;
}

ScriptData* GetPreviousScriptInList(void* CurrentScript)
{
	Script* ScriptForm = CS_CAST(CurrentScript, TESForm, Script);
	ScriptData* Result = nullptr;
	Script* Switch = nullptr;

	if (_DATAHANDLER->scripts.Count())
	{
		if (ScriptForm == nullptr)
		{
			Switch = _DATAHANDLER->scripts.GetLastItem();
			if (Switch->GetEditorID() == nullptr)
				Switch = GetScriptNeighbour(ScriptForm, kDirection_Backward);
		}
		else
			Switch = GetScriptNeighbour(ScriptForm, kDirection_Backward);
	}

	if (Switch && Switch->GetEditorID())
	{
		Result = new ScriptData();
		Result->FillScriptData(Switch);
	}

	return Result;
}

ScriptData* GetNextScriptInList(void* CurrentScript)
{
	Script* ScriptForm = CS_CAST(CurrentScript, TESForm, Script);
	ScriptData* Result = nullptr;
	Script* Switch = nullptr;

	if (_DATAHANDLER->scripts.Count())
	{
		if (ScriptForm == nullptr)
		{
			Switch = _DATAHANDLER->scripts.GetNthItem(0);
			if (Switch->GetEditorID() == nullptr)
				Switch = GetScriptNeighbour(ScriptForm, kDirection_Forward);
		}
		else
			Switch = GetScriptNeighbour(ScriptForm, kDirection_Forward);
	}

	if (Switch && Switch->GetEditorID())
	{
		Result = new ScriptData();
		Result->FillScriptData(Switch);
	}

	return Result;
}

void RemoveScriptBytecode(void* CurrentScript)
{
	Script* ScriptForm = CS_CAST(CurrentScript, TESForm, Script);
	ScriptForm->RemoveCompiledData();
}

void DestroyScriptInstance(void* CurrentScript)
{
	Script* ScriptForm = CS_CAST(CurrentScript, TESForm, Script);
	thisCall<void>(0x00452AE0, &_DATAHANDLER->scripts, ScriptForm);
	ScriptForm->DeleteInstance();
}

void SaveEditorBoundsToINI(UInt32 Top, UInt32 Left, UInt32 Width, UInt32 Height)
{
	char Buffer[0x200] = {0};

	FORMAT_STR(Buffer, "%d", Left);
	WritePrivateProfileString("General", "Script Edit X", Buffer, TESCSMain::INIFilePath);
	FORMAT_STR(Buffer, "%d", Top);
	WritePrivateProfileString("General", "Script Edit Y", Buffer, TESCSMain::INIFilePath);
	FORMAT_STR(Buffer, "%d", Width);
	WritePrivateProfileString("General", "Script Edit W", Buffer, TESCSMain::INIFilePath);
	FORMAT_STR(Buffer, "%d", Height);
	WritePrivateProfileString("General", "Script Edit H", Buffer, TESCSMain::INIFilePath);
}

ScriptListData* GetScriptList(void)
{
	ScriptListData* Result = new ScriptListData();
	if (_DATAHANDLER->scripts.Count())
	{
		Result->ScriptCount = _DATAHANDLER->scripts.Count();
		Result->ScriptListHead = new ScriptData[Result->ScriptCount];
		int i = 0;
		for (tList<Script>::Iterator Itr = _DATAHANDLER->scripts.Begin(); !Itr.End() && Itr.Get(); ++Itr)
		{
			Script* ScriptForm = Itr.Get();
			Result->ScriptListHead[i].FillScriptData(ScriptForm);
			i++;
		}
	}

	return Result;
}

ScriptVarListData* GetScriptVarList(const char* EditorID)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	ScriptVarListData* Result = new ScriptVarListData();

	if (Form)
	{
		Script* ScriptForm = CS_CAST(Form, TESForm, Script);
		if (ScriptForm && ScriptForm->varList.Count())
		{
			Result->ScriptVarListCount = ScriptForm->varList.Count();
			Result->ScriptVarListHead = new ScriptVarListData::ScriptVarInfo[Result->ScriptVarListCount];

			int i = 0;
			for (Script::VariableListT::Iterator Itr = ScriptForm->varList.Begin(); !Itr.End() && Itr.Get(); ++Itr)
			{
				Script::VariableInfo* Variable = Itr.Get();

				Result->ScriptVarListHead[i].Name = Variable->name.c_str();
				Result->ScriptVarListHead[i].Type = Variable->type;
				Result->ScriptVarListHead[i].Index = Variable->index;

				if (Result->ScriptVarListHead[i].Type == Script::kVariableTypes_Float)
				{
					for (Script::RefVariableListT::Iterator ItrEx = ScriptForm->refList.Begin(); !ItrEx.End() && ItrEx.Get(); ++ItrEx)
					{
						if (ItrEx.Get()->variableIndex == Variable->index)
						{
							Result->ScriptVarListHead[i].Type = 2;		// ref var
							break;
						}
					}
				}

				i++;
			}
		}
	}

	return Result;
}

bool UpdateScriptVarIndices(const char* EditorID, ScriptVarListData* Data)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	bool Result = false;

	if (Form)
	{
		Script* ScriptForm = CS_CAST(Form, TESForm, Script);

		if (ScriptForm)
		{
			for (int i = 0; i < Data->ScriptVarListCount; i++)
			{
				ScriptVarListData::ScriptVarInfo* VarInfo = &Data->ScriptVarListHead[i];
				Script::VariableInfo* ScriptVar = ScriptForm->LookupVariableInfoByName(VarInfo->Name);

				if (ScriptVar)
				{
					if (VarInfo->Type == 2)
					{
						Script::RefVariable* RefVar = ScriptForm->LookupRefVariableByIndex(ScriptVar->index);
						if (RefVar)
							RefVar->variableIndex = VarInfo->Index;
					}

					ScriptVar->index = VarInfo->Index;
				}
			}

			Result = true;
			ScriptForm->SetFromActiveFile(true);
		}
	}

	return Result;
}

void CompileCrossReferencedForms(TESForm* Form)
{
	BGSEECONSOLE_MESSAGE("Parsing object use list of %08X...", Form->formID);
	BGSEECONSOLE->Indent();

	std::list<Script*> ScriptDepends;		// updating usage info inside an use list loop invalidates the list.
	std::list<TESTopicInfo*> InfoDepends;	// so store the objects ptrs and parse them later
	std::list<TESQuest*> QuestDepends;

	for (FormCrossReferenceListT::Iterator Itr = Form->GetCrossReferenceList()->Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TESForm* Depends = Itr.Get()->GetForm();
		switch (Depends->formType)
		{
		case TESForm::kFormType_TopicInfo:
		{
			InfoDepends.push_back(CS_CAST(Depends, TESForm, TESTopicInfo));
			break;
		}
		case TESForm::kFormType_Quest:
		{
			QuestDepends.push_back(CS_CAST(Depends, TESForm, TESQuest));
			break;
		}
		case TESForm::kFormType_Script:
		{
			ScriptDepends.push_back(CS_CAST(Depends, TESForm, Script));
			break;
		}
		default:	// ### any other type that needs handling ?
			break;
		}
	}

	// scripts
	for (std::list<Script*>::const_iterator Itr = ScriptDepends.begin(); Itr != ScriptDepends.end(); Itr++)
	{
		BGSEECONSOLE_MESSAGE("Script %s {%08X}:", (*Itr)->editorID.c_str(), (*Itr)->formID);
		BGSEECONSOLE->Indent();

		if ((*Itr)->info.dataLength > 0)
		{
			if (!(*Itr)->Compile())
			{
				BGSEECONSOLE_MESSAGE("Script failed to compile due to errors!");
			}
		}

		BGSEECONSOLE->Outdent();
	}

	// quests
	for (std::list<TESQuest*>::const_iterator Itr = QuestDepends.begin(); Itr != QuestDepends.end(); Itr++)
	{
		BGSEECONSOLE_MESSAGE("Quest %s {%08X}:", (*Itr)->editorID.c_str(), (*Itr)->formID);
		BGSEECONSOLE->Indent();

		for (TESQuest::StageListT::Iterator i = (*Itr)->stageList.Begin(); !i.End(); ++i)
		{
			TESQuest::StageData* Stage = i.Get();
			if (!Stage)
				break;

			int Count = 0;
			for (TESQuest::StageData::StageItemListT::Iterator j = Stage->stageItemList.Begin(); !j.End(); ++j, ++Count)
			{
				TESQuest::StageData::QuestStageItem* StageItem = j.Get();
				if (!StageItem)
					break;

				if (StageItem->resultScript.info.dataLength > 0)
				{
					if (!StageItem->resultScript.Compile(true))
					{
						BGSEECONSOLE_MESSAGE("Result script in stage item %d-%d failed to compile due to errors!", Stage->index, Count);
					}
				}

				BGSEECONSOLE_MESSAGE("Found %d conditions in stage item %d-%d that referenced source script",
					TESConditionItem::GetScriptableFormConditionCount(&StageItem->conditions, Form), Stage->index, Count);
			}
		}

		for (TESQuest::TargetListT::Iterator j = (*Itr)->targetList.Begin(); !j.End(); ++j)
		{
			TESQuest::TargetData* Target = j.Get();
			if (!Target)
				break;

			BGSEECONSOLE_MESSAGE("Found %d conditions in target entry {%08X} that referenced source script",
						TESConditionItem::GetScriptableFormConditionCount(&Target->conditionList, Form), Target->target->formID);
		}

		(*Itr)->UpdateUsageInfo();
		BGSEECONSOLE->Outdent();
	}

	// topic infos
	for (std::list<TESTopicInfo*>::const_iterator Itr = InfoDepends.begin(); Itr != InfoDepends.end(); Itr++)
	{
		BGSEECONSOLE_MESSAGE("Topic info %08X:", (*Itr)->formID);
		BGSEECONSOLE->Indent();

		if ((*Itr)->resultScript.info.dataLength > 0)
		{
			if (!(*Itr)->resultScript.Compile(true))
			{
				BGSEECONSOLE_MESSAGE("Result script failed to compile due to errors!");
			}
		}

		BGSEECONSOLE_MESSAGE("Found %d conditions that referenced source script",
					TESConditionItem::GetScriptableFormConditionCount(&(*Itr)->conditions, Form));

		(*Itr)->UpdateUsageInfo();
		BGSEECONSOLE->Outdent();
	}

	BGSEECONSOLE->Outdent();
}

void CompileDependencies(const char* EditorID)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	if (Form == nullptr)
		return;

	Script* ScriptForm = CS_CAST(Form, TESForm, Script);
	if (ScriptForm == nullptr)
		return;

	BGSEECONSOLE_MESSAGE("Recompiling dependencies of script %s {%08X}...", ScriptForm->editorID.c_str(), ScriptForm->formID);
	BGSEECONSOLE->Indent();

	BGSEECONSOLE_MESSAGE("Resolving script parent...");
	BGSEECONSOLE->Indent();
	switch (ScriptForm->info.type)
	{
	case Script::kScriptType_Object:
	{
		BGSEECONSOLE_MESSAGE("Script type = Object");
		for (FormCrossReferenceListT::Iterator Itr = Form->GetCrossReferenceList()->Begin(); !Itr.End() && Itr.Get(); ++Itr)
		{
			TESForm* Parent = Itr.Get()->GetForm();
			TESScriptableForm* ValidParent = CS_CAST(Parent, TESForm, TESScriptableForm);

			if (ValidParent)
			{
				BGSEECONSOLE_MESSAGE("Scriptable Form %s ; Type = %d:", Parent->editorID.c_str(), Parent->formType);
				BGSEECONSOLE_MESSAGE("Parsing cell use list...");
				BGSEECONSOLE->Indent();

				TESCellUseList* UseList = CS_CAST(Form, TESForm, TESCellUseList);
				for (TESCellUseList::CellUseInfoListT::Iterator Itr = UseList->cellUses.Begin(); !Itr.End(); ++Itr)
				{
					TESCellUseList::CellUseInfo* Data = Itr.Get();
					if (!Data)
						break;

					for (TESObjectCELL::ObjectREFRList::Iterator Itr = Data->cell->objectList.Begin(); !Itr.End(); ++Itr)
					{
						TESObjectREFR* ThisReference = Itr.Get();
						if (!ThisReference)
							break;

						if (ThisReference->baseForm == Parent)
							CompileCrossReferencedForms((TESForm*)ThisReference);
					}
				}

				BGSEECONSOLE->Outdent();
			}
		}
		break;
	}
	case Script::kScriptType_Quest:
	{
		BGSEECONSOLE_MESSAGE("Script type = Quest");
		for (FormCrossReferenceListT::Iterator Itr = Form->GetCrossReferenceList()->Begin(); !Itr.End() && Itr.Get(); ++Itr)
		{
			TESForm* Parent = Itr.Get()->GetForm();
			if (Parent->formType == TESForm::kFormType_Quest)
			{
				BGSEECONSOLE_MESSAGE("Quest %s:", Parent->editorID.c_str());
				CompileCrossReferencedForms(Parent);
			}
		}
		break;
	}
	}
	BGSEECONSOLE->Outdent();

	BGSEECONSOLE_MESSAGE("Parsing direct dependencies...");
	CompileCrossReferencedForms(Form);

	BGSEECONSOLE->Outdent();
	BGSEECONSOLE_MESSAGE("Recompile dependencies operation completed!");
}

IntelliSenseUpdateData* GetIntelliSenseUpdateData(void)
{
	IntelliSenseUpdateData* Data = new IntelliSenseUpdateData();

	UInt32 QuestCount = _DATAHANDLER->quests.Count(),
			ScriptCount = 0,
			GlobalCount = _DATAHANDLER->globals.Count(),
			EditorIDFormCount = 0;

	ScriptData TestData;
	std::vector<UInt32> UDFFormIDs;
	for (tList<Script>::Iterator Itr = _DATAHANDLER->scripts.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TestData.FillScriptData(Itr.Get());
		if (TestData.UDF)
		{
			ScriptCount++;
			UDFFormIDs.push_back(TestData.FormID);
		}
	}

	for (cseOverride::NiTMapIterator Itr = TESForm::EditorIDMap->GetFirstPos(); Itr;)
	{
		const char*	 EditorID = nullptr;
		TESForm* Form = nullptr;

		TESForm::EditorIDMap->GetNext(Itr, EditorID, Form);
		if (EditorID)
		{
			if (Form->formType != TESForm::kFormType_GMST &&
				Form->formType != TESForm::kFormType_Global &&
				Form->formType != TESForm::kFormType_Quest &&
				Form->formType != TESForm::kFormType_LandTexture &&
				Form->formType != TESForm::kFormType_Tree &&
				Form->formType != TESForm::kFormType_Grass &&
				Form->formType != TESForm::kFormType_Region &&
				Form->formType != TESForm::kFormType_LoadScreen &&
				Form->formType != TESForm::kFormType_AnimObject)
			{
				if (std::find(UDFFormIDs.begin(), UDFFormIDs.end(), Form->formID) == UDFFormIDs.end())
					EditorIDFormCount++;
			}
		}
	}

	Data->QuestListHead = new QuestData[QuestCount];
	Data->QuestCount = QuestCount;
	Data->ScriptListHead = new ScriptData[ScriptCount];
	Data->ScriptCount = ScriptCount;
	Data->GlobalListHead = new GlobalData[GlobalCount];
	Data->GlobalCount = GlobalCount;
	Data->EditorIDListHead = new FormData[EditorIDFormCount];
	Data->EditorIDCount = EditorIDFormCount;

	QuestCount = 0, ScriptCount = 0, GlobalCount = 0, EditorIDFormCount = 0;
	for (tList<TESQuest>::Iterator Itr = _DATAHANDLER->quests.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		Data->QuestListHead[QuestCount].FillFormData(Itr.Get());
		Data->QuestListHead[QuestCount].FullName = Itr.Get()->name.c_str();
		Data->QuestListHead[QuestCount].ScriptName = nullptr;
		if (Itr.Get()->script)
			Data->QuestListHead[QuestCount].ScriptName = Itr.Get()->script->editorID.c_str();

		QuestCount++;
	}

	for (tList<Script>::Iterator Itr = _DATAHANDLER->scripts.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		TestData.FillScriptData(Itr.Get());
		if (TestData.UDF)
		{
			Data->ScriptListHead[ScriptCount].FillScriptData(Itr.Get());
			ScriptCount++;
		}
	}

	for (tList<TESGlobal>::Iterator Itr = _DATAHANDLER->globals.Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		Data->GlobalListHead[GlobalCount].FillFormData(Itr.Get());
		Data->GlobalListHead[GlobalCount].FillVariableData(Itr.Get());
		GlobalCount++;
	}

	for (cseOverride::NiTMapIterator Itr = TESForm::EditorIDMap->GetFirstPos(); Itr;)
	{
		const char*	 EditorID = nullptr;
		TESForm* Form = nullptr;

		TESForm::EditorIDMap->GetNext(Itr, EditorID, Form);
		if (EditorID)
		{
			if (Form->formType != TESForm::kFormType_GMST &&
				Form->formType != TESForm::kFormType_Global &&
				Form->formType != TESForm::kFormType_Quest &&
				Form->formType != TESForm::kFormType_LandTexture &&
				Form->formType != TESForm::kFormType_Tree &&
				Form->formType != TESForm::kFormType_Grass &&
				Form->formType != TESForm::kFormType_Region &&
				Form->formType != TESForm::kFormType_LoadScreen &&
				Form->formType != TESForm::kFormType_AnimObject)
			{
				if (std::find(UDFFormIDs.begin(), UDFFormIDs.end(), Form->formID) == UDFFormIDs.end())
				{
					Data->EditorIDListHead[EditorIDFormCount].FillFormData(Form);
					EditorIDFormCount++;
				}
			}
		}
	}

	return Data;
}

void BindScript(const char* EditorID, HWND Parent)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);
	if (Form == nullptr)
		return;

	Script* ScriptForm = CS_CAST(Form, TESForm, Script);
	if (ScriptForm == nullptr)
		return;

	Form = (TESForm*)DialogBox(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_BINDSCRIPT), Parent, (DLGPROC)uiManager::BindScriptDlgProc);

	if (Form)
	{
		TESQuest* Quest = CS_CAST(Form, TESForm, TESQuest);
		TESBoundObject* BoundObj = CS_CAST(Form, TESForm, TESBoundObject);
		TESScriptableForm* ScriptableForm = CS_CAST(Form, TESForm, TESScriptableForm);

		if ((Quest && ScriptForm->info.type != Script::kScriptType_Quest) ||
			(BoundObj && ScriptForm->info.type != Script::kScriptType_Object))
		{
			BGSEEUI->MsgBoxW(Parent, 0, "Script type doesn't correspond to binding form.");
		}
		else if (ScriptableForm == nullptr)
			BGSEEUI->MsgBoxW(Parent, 0, "Binding form isn't scriptable.");
		else
		{
			ScriptableForm->script = ScriptForm;
			ScriptForm->AddCrossReference(Form);
			Form->SetFromActiveFile(true);

			BGSEEUI->MsgBoxW(Parent, 0, "Script '%s' bound to form '%s'", ScriptForm->editorID.c_str(), Form->editorID.c_str());
		}
	}
}

void SetScriptText(void* CurrentScript, const char* ScriptText)
{
	Script* ScriptForm = CS_CAST(CurrentScript, TESForm, Script);
	ScriptForm->SetText(ScriptText);
}

void UpdateScriptVarNames(const char* EditorID, componentDLLInterface::ScriptVarRenameData* Data)
{
	TESForm* Form = TESForm::LookupByEditorID(EditorID);

	if (Form)
	{
		Script* ScriptForm = CS_CAST(Form, TESForm, Script);

		if (ScriptForm)
		{
			BGSEECONSOLE_MESSAGE("Updating script '%s' variable names...", ScriptForm->editorID.c_str());
			BGSEECONSOLE->Indent();
			for (int i = 0; i < Data->ScriptVarListCount; i++)
			{
				ScriptVarRenameData::ScriptVarInfo* VarInfo = &Data->ScriptVarListHead[i];
				Script::VariableInfo* ScriptVar = ScriptForm->LookupVariableInfoByName(VarInfo->OldName);

				if (ScriptVar)
				{
					ScriptVar->name.Set(VarInfo->NewName);
					BGSEECONSOLE_MESSAGE("Variable '%s' renamed to '%s'", VarInfo->OldName, VarInfo->NewName);

					Script::RefVariable* RefVar = ScriptForm->LookupRefVariableByIndex(ScriptVar->index);
					if (RefVar && !RefVar->form)
						RefVar->name.Set(VarInfo->NewName);
				}
			}

			BGSEECONSOLE->Outdent();
			ScriptForm->SetFromActiveFile(true);
		}
	}
}

bool CanUpdateIntelliSenseDatabase(void)
{
	return TESDataHandler::PluginLoadSaveInProgress == false;
}

const char* GetDefaultCachePath(void)
{
	static const std::string kBuffer = bgsee::ResourceLocation(CSE_SEDEPOT)();
	return kBuffer.c_str();
}

const char* GetAutoRecoveryCachePath(void)
{
	static const std::string kBuffer = bgsee::ResourceLocation(CSE_SEAUTORECDEPOT)();
	return kBuffer.c_str();
}

const char* GetPreprocessorBasePath(void)
{
	static const std::string kBuffer = bgsee::ResourceLocation(CSE_SEPREPROCDEPOT)();
	return kBuffer.c_str();
}

const char* GetPreprocessorStandardPath(void)
{
	static const std::string kBuffer = bgsee::ResourceLocation(CSE_SEPREPROCSTDDEPOT)();
	return kBuffer.c_str();
}

const char* GetSnippetCachePath(void)
{
	static const std::string kBuffer = bgsee::ResourceLocation(CSE_SESNIPPETDEPOT)();
	return kBuffer.c_str();
}

ScriptVarRenameData* AllocateVarRenameData(UInt32 VarCount)
{
	ScriptVarRenameData* Data = new ScriptVarRenameData();
	Data->ScriptVarListHead = new ScriptVarRenameData::ScriptVarInfo[VarCount];
	Data->ScriptVarListCount = VarCount;

	return Data;
}

ScriptCompileData* AllocateCompileData(void)
{
	ScriptCompileData* Data = new ScriptCompileData();
	return Data;
}

#pragma endregion
/**** END SCRIPTEDITOR SUBINTERFACE ****/

/**** BEGIN USEINFOLIST SUBINTERFACE ****/
#pragma region UseInfoList
template <typename tData>
void AddLinkedListContentsToFormList(tList<tData>* List, FormListData* FormList, UInt32& CurrentIndex)
{
	for (tList<tData>::Iterator Itr = List->Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		FormData* ThisForm = &FormList->FormListHead[CurrentIndex];
		ThisForm->FillFormData(Itr.Get());
		CurrentIndex++;
	}
}

UseInfoListFormData* GetLoadedForms(void)
{
	UseInfoListFormData* Result = new UseInfoListFormData();

	TESCSMain::WriteToStatusBar(2, "Initializing Use Info List...");

	UInt32 TotalFormCount = _DATAHANDLER->objects->objectCount;
	TotalFormCount += _DATAHANDLER->packages.Count();
	TotalFormCount += _DATAHANDLER->worldSpaces.Count();
	TotalFormCount += _DATAHANDLER->climates.Count();
	TotalFormCount += _DATAHANDLER->weathers.Count();
	TotalFormCount += _DATAHANDLER->enchantmentItems.Count();
	TotalFormCount += _DATAHANDLER->spellItems.Count();
	TotalFormCount += _DATAHANDLER->hairs.Count();
	TotalFormCount += _DATAHANDLER->eyes.Count();
	TotalFormCount += _DATAHANDLER->races.Count();
	TotalFormCount += _DATAHANDLER->landTextures.Count();
	TotalFormCount += _DATAHANDLER->classes.Count();
	TotalFormCount += _DATAHANDLER->factions.Count();
	TotalFormCount += _DATAHANDLER->scripts.Count();
	TotalFormCount += _DATAHANDLER->sounds.Count();
	TotalFormCount += _DATAHANDLER->globals.Count();
	TotalFormCount += _DATAHANDLER->topics.Count();
	TotalFormCount += _DATAHANDLER->quests.Count();
	TotalFormCount += _DATAHANDLER->birthsigns.Count();
	TotalFormCount += _DATAHANDLER->combatStyles.Count();
	TotalFormCount += _DATAHANDLER->loadScreens.Count();
	TotalFormCount += _DATAHANDLER->waterForms.Count();
	TotalFormCount += _DATAHANDLER->effectShaders.Count();
	TotalFormCount += _DATAHANDLER->objectAnios.Count();

	Result->FormCount = TotalFormCount;
	Result->FormListHead = new FormData[Result->FormCount];

	UInt32 Index = 0;
	for (TESObject* Itr = _DATAHANDLER->objects->first; Itr; Itr = Itr->next)
	{
		FormData* ThisForm = &Result->FormListHead[Index];
		ThisForm->FillFormData(Itr);
		Index++;
	}

	AddLinkedListContentsToFormList(&_DATAHANDLER->packages, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->worldSpaces, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->climates, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->weathers, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->enchantmentItems, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->spellItems, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->hairs, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->eyes, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->races, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->landTextures, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->classes, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->factions, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->scripts, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->sounds, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->globals, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->topics, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->quests, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->birthsigns, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->combatStyles, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->loadScreens, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->waterForms, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->effectShaders, (FormListData*)Result, Index);
	AddLinkedListContentsToFormList(&_DATAHANDLER->objectAnios, (FormListData*)Result, Index);

	TESCSMain::WriteToStatusBar(2, "Use Info List Initialized.");
	return Result;
}

UseInfoListCrossRefData* GetCrossRefDataForForm(const char* EditorID)
{
	UseInfoListCrossRefData* Result = new UseInfoListCrossRefData();
	TESForm* Form = TESForm::LookupByEditorID(EditorID);

	if (Form)
	{
		FormCrossReferenceListT* CrossRefList = Form->GetCrossReferenceList();
		ScriptMagicItemCrossRefArrayT EffectRefs;
		MagicItemScriptCrossRefArrayT ScriptRefs;

		UInt32 Count = CrossRefList->Count();
		MagicItem* Item = nullptr;

		if (Form->formType == TESForm::kFormType_Script)
		{
			Script* ThisScript = CS_CAST(Form, TESForm, Script);
			Count += ThisScript->GetEffectItemReferences(EffectRefs);
		}
		else if ((Item = CS_CAST(Form, TESForm, MagicItem)))
		{
			Count += Item->GetScriptReferences(ScriptRefs);
		}

		if (Count)
		{
			Result->FormCount = Count;
			Result->FormListHead = new FormData[Result->FormCount];

			int i = 0;
			for (FormCrossReferenceListT::Iterator Itr = CrossRefList->Begin(); !Itr.End() && Itr.Get(); ++Itr, ++i)
				Result->FormListHead[i].FillFormData(Itr.Get()->GetForm());

			for (ScriptMagicItemCrossRefArrayT::iterator Itr = EffectRefs.begin(); Itr != EffectRefs.end(); ++Itr, ++i)
				Result->FormListHead[i].FillFormData(*Itr);

			for (MagicItemScriptCrossRefArrayT::iterator Itr = ScriptRefs.begin(); Itr != ScriptRefs.end(); ++Itr, ++i)
				Result->FormListHead[i].FillFormData(*Itr);
		}
	}

	return Result;
}

UseInfoListCellItemListData* GetCellRefDataForForm(const char* EditorID)
{
	UseInfoListCellItemListData* Result = new UseInfoListCellItemListData();
	TESForm* Form = TESForm::LookupByEditorID(EditorID);

	if (Form)
	{
		TESCellUseList* UseList = CS_CAST(Form, TESForm, TESCellUseList);
		if (UseList && UseList->cellUses.Count())
		{
			TESCellUseList::CellUseInfoListT* CellUseList = &UseList->cellUses;
			if (CellUseList->Count())
			{
				Result->UseInfoListCellItemListCount = CellUseList->Count();
				Result->UseInfoListCellItemListHead = new UseInfoListCellItemData[Result->UseInfoListCellItemListCount];
				int i = 0;

				for (TESCellUseList::CellUseInfoListT::Iterator Itr = CellUseList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
				{
					TESCellUseList::CellUseInfo* Data = Itr.Get();
					TESObjectREFR* FirstRef = Data->cell->FindFirstRef(Form, false);
					TESWorldSpace* WorldSpace = Data->cell->GetParentWorldSpace();

					Result->UseInfoListCellItemListHead[i].FillFormData(Data->cell);
					Result->UseInfoListCellItemListHead[i].WorldEditorID = ((!WorldSpace)?"Interior":WorldSpace->editorID.c_str());
					Result->UseInfoListCellItemListHead[i].RefEditorID = ((!FirstRef || !FirstRef->editorID.c_str())?"<Unnamed>":FirstRef->editorID.c_str());
					Result->UseInfoListCellItemListHead[i].RefFormID = (FirstRef == nullptr ? 0 : FirstRef->formID);
					Result->UseInfoListCellItemListHead[i].ParentCellInterior = Data->cell->cellFlags & TESObjectCELL::kCellFlags_Interior;
					Result->UseInfoListCellItemListHead[i].XCoord = Data->cell->cellData.coords->x;
					Result->UseInfoListCellItemListHead[i].YCoord = Data->cell->cellData.coords->y;
					Result->UseInfoListCellItemListHead[i].UseCount = Data->count;

					i++;
				}
			}
		}
	}

	return Result;
}
/**** END USEINFOLIST SUBINTERFACE ****/
#pragma endregion

/**** BEGIN BATCHREFEDITOR SUBINTERFACE ****/
#pragma region BatchRefEditor
bool OwnershipDataSortComparator(TESForm* First, TESForm* Second)
{
	if (First->GetEditorID() == nullptr || Second->GetEditorID() == nullptr)
		return false;

	return _stricmp(First->GetEditorID(), Second->GetEditorID()) < 0;
}

BatchRefOwnerFormData* GetOwnershipData(void)
{
	BatchRefOwnerFormData* Result = new BatchRefOwnerFormData();
	std::list<TESForm*> SortedNPCs, SortedFactions, SortedGlobals;

	UInt32 TotalFormCount = 0;
	TotalFormCount += _DATAHANDLER->factions.Count();
	TotalFormCount += _DATAHANDLER->globals.Count();

	for (tList<TESFaction>::Iterator Itr = _DATAHANDLER->factions.Begin(); !Itr.End() && Itr.Get(); ++Itr)
		SortedFactions.push_back(Itr.Get());

	for (tList<TESGlobal>::Iterator Itr = _DATAHANDLER->globals.Begin(); !Itr.End() && Itr.Get(); ++Itr)
		SortedGlobals.push_back(Itr.Get());

	for (TESObject* Itr = _DATAHANDLER->objects->first; Itr; Itr = Itr->next)
	{
		if (Itr->formType == TESForm::kFormType_NPC)
		{
			SortedNPCs.push_back(Itr);
			TotalFormCount++;
		}
	}

	SortedNPCs.sort(OwnershipDataSortComparator);
	SortedFactions.sort(OwnershipDataSortComparator);
	SortedGlobals.sort(OwnershipDataSortComparator);

	Result->FormCount = TotalFormCount;
	Result->FormListHead = new FormData[Result->FormCount];

	UInt32 Index = 0;
	for (auto Itr : SortedNPCs)
	{
		FormData* ThisForm = &Result->FormListHead[Index];
		ThisForm->FillFormData(Itr);
		Index++;
	}

	for (auto Itr : SortedFactions)
	{
		FormData* ThisForm = &Result->FormListHead[Index];
		ThisForm->FillFormData(Itr);
		Index++;
	}

	for (auto Itr : SortedGlobals)
	{
		FormData* ThisForm = &Result->FormListHead[Index];
		ThisForm->FillFormData(Itr);
		Index++;
	}

	return Result;
}
#pragma endregion
/**** END USEINFOLIST SUBINTERFACE ****/

/**** BEGIN TAGBROWSER SUBINTERFACE ****/
#pragma region TagBrowser
void InstantiateObjects(TagBrowserInstantiationData* Data)
{
	HWND Window = WindowFromPoint(Data->InsertionPoint);
	if (Window)
	{
		bool ValidRecipient = false;
		for (int i = 0; i < Data->FormCount; i++)
		{
			FormData* ThisData = &Data->FormListHead[i];
			UInt32 FormID = ThisData->FormID;

			TESForm* Form = TESForm::LookupByFormID(FormID);
			if (Form && TESDialog::GetIsWindowDragDropRecipient(Form->formType, Window))
			{
				ValidRecipient = true;
				break;
			}
		}

		if (ValidRecipient)
		{
			_RENDERSEL->ClearSelection(true);

			for (int i = 0; i < Data->FormCount; i++)
			{
				FormData* ThisData = &Data->FormListHead[i];
				UInt32 FormID = ThisData->FormID;

				TESForm* Form = TESForm::LookupByFormID(FormID);
				if (!Form)
				{
					BGSEECONSOLE_MESSAGE("Couldn't find form '%08X'!", FormID);
					continue;
				}
				_RENDERSEL->AddToSelection(Form);
			}

			HWND Parent = GetParent(Window);
			if (!Parent || Parent == *TESCSMain::WindowHandle)
				SendMessage(Window, TESDialog::kWindowMessage_HandleDragDrop, NULL, (LPARAM)&Data->InsertionPoint);
			else
				SendMessage(Parent, TESDialog::kWindowMessage_HandleDragDrop, NULL, (LPARAM)&Data->InsertionPoint);
		}
	}
}

void InitiateDragonDrop(void)
{
	for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
	{
		TESForm* Form = Itr->Data;
		componentDLLInterface::FormData Data(Form);
		if (!cliWrapper::interfaces::TAG->AddFormToActiveTag(&Data))
			break;
	}

	_RENDERSEL->ClearSelection();
}

TagBrowserInstantiationData* AllocateInstantionData(UInt32 FormCount)
{
	componentDLLInterface::TagBrowserInstantiationData* Data = new componentDLLInterface::TagBrowserInstantiationData();
	Data->FormCount = FormCount;
	Data->FormListHead = new FormData[FormCount];

	return Data;
}
#pragma endregion
/**** END TAGBROWSER SUBINTERFACE ****/

componentDLLInterface::CSEInterfaceTable g_InteropInterface =
{
	DeleteInterOpData,
	DeleteData,
	{
		ComponentDLLDebugPrint,
		WriteToStatusBar,
		GetAppPath,
		GetCSMainWindowHandle,
		GetRenderWindowHandle,
		LookupFormByEditorID,
		LookupScriptableByEditorID,
		GetIsFormReference,
		GetFormTypeIDLongName,
		LoadFormForEditByEditorID,
		LoadFormForEditByFormID,
		ShowPickReferenceDialog,
		ShowUseReportDialog,
		SaveActivePlugin,
		ReadFromINI,
		WriteToINI,
		GetFormListActiveItemForegroundColor,
		GetFormListActiveItemBackgroundColor,
		GetShouldColorizeActiveForms,
		GetShouldSortActiveFormsFirst,
	},
	{
		CreateNewScript,
		DestroyScriptInstance,
		CompileScript,
		RecompileScripts,
		ToggleScriptCompilation,
		DeleteScript,
		GetPreviousScriptInList,
		GetNextScriptInList,
		RemoveScriptBytecode,
		SaveEditorBoundsToINI,
		GetScriptList,
		GetScriptVarList,
		UpdateScriptVarIndices,
		CompileDependencies,
		GetIntelliSenseUpdateData,
		BindScript,
		SetScriptText,
		UpdateScriptVarNames,
		CanUpdateIntelliSenseDatabase,
		GetDefaultCachePath,
		GetAutoRecoveryCachePath,
		GetPreprocessorBasePath,
		GetPreprocessorStandardPath,
		GetSnippetCachePath,
		AllocateVarRenameData,
		AllocateCompileData,
	},
	{
		GetLoadedForms,
		GetCrossRefDataForForm,
		GetCellRefDataForForm,
	},
	{
		GetOwnershipData,
	},
	{
		InstantiateObjects,
		InitiateDragonDrop,
		AllocateInstantionData,
	}
};