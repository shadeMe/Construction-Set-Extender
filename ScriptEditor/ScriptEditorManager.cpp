#include "ScriptEditorManager.h"
#include "Common\NativeWrapper.h"
#include "Globals.h"
#include "Common\HandShakeStructs.h"
#include "ScriptListDialog.h"
#include "IntelliSense.h"

ScriptEditorManager::ScriptEditorManager()
{
	WorkspaceAllocationMap = gcnew LinkedList<ScriptEditor::Workspace^>();
	TabContainerAllocationMap = gcnew LinkedList<ScriptEditor::TabContainer^>();
}

ScriptEditorManager^% ScriptEditorManager::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew ScriptEditorManager();
		DebugPrint("Initialized ScriptEditorManager");
	}
	return Singleton;
}

void ScriptEditorManager::PerformOperation(ScriptEditorManager::OperationType Op, ScriptEditorManager::OperationParams^ Parameters)
{
	try {
		UInt16 NoOfParams = Parameters->ParameterList->Count, ParamsRequired = ParamCount[(int)Op];

		if (NoOfParams < ParamsRequired)
			throw gcnew Exception("Incorrect number of parameters passed (" + NoOfParams + ")");
		if (GetAllocatedWorkspace(Parameters->VanillaHandleIndex) == ScriptEditor::Workspace::NullSE && Parameters->VanillaHandleIndex)
			throw gcnew Exception("Invalid editor index '" + Parameters->VanillaHandleIndex + "' passed");

		switch (Op)
		{
		case OperationType::e_AllocateTabContainer:
			AllocateNewTabContainer((UInt32)Parameters->ParameterList[0],
								(UInt32)Parameters->ParameterList[1],
								(UInt32)Parameters->ParameterList[2],
								(UInt32)Parameters->ParameterList[3]);
			break;
		case OperationType::e_InitializeScript:
			InitializeScript(Parameters->VanillaHandleIndex,
						dynamic_cast<String^>(Parameters->ParameterList[0]),
						(UInt16)Parameters->ParameterList[1],
						dynamic_cast<String^>(Parameters->ParameterList[2]),
						(UInt32)Parameters->ParameterList[3],
						(UInt32)Parameters->ParameterList[4],
						(UInt32)Parameters->ParameterList[5]);
			break;

		case OperationType::e_SendMessage:
			switch ((SendReceiveMessageType)Parameters->ParameterList[0])
			{
				case SendReceiveMessageType::e_New:
					MessageHandler_SendNew(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Open:
					MessageHandler_SendOpen(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Previous:
					MessageHandler_SendPrevious(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Next:
					MessageHandler_SendNext(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Save:
					MessageHandler_SendSave(Parameters->VanillaHandleIndex, (SaveWorkspaceOpType)Parameters->ParameterList[1]);
					break;
				case SendReceiveMessageType::e_Recompile:
					MessageHandler_SendRecompile(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Delete:
					MessageHandler_SendDelete(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Close:
					MessageHandler_SendClose(Parameters->VanillaHandleIndex);
					break;
			}
			break;
		case OperationType::e_ReceiveMessage:
			switch ((SendReceiveMessageType)((UInt16)Parameters->ParameterList[0]))
			{
				case SendReceiveMessageType::e_New:
					MessageHandler_ReceiveNew(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Open:
					MessageHandler_ReceiveOpen(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Previous:
					MessageHandler_ReceivePrevious(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Next:
					MessageHandler_ReceiveNext(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Save:
					MessageHandler_ReceiveSave(Parameters->VanillaHandleIndex);
					break;
				case SendReceiveMessageType::e_Close:
					MessageHandler_ReceiveClose(Parameters->VanillaHandleIndex);
					break;

				case SendReceiveMessageType::e_LoadRelease:
					MessageHandler_ReceiveLoadRelease();
					break;
				case SendReceiveMessageType::e_PostPluginLoad:
					MessageHandler_ReceivePostPluginLoad();
					break;
				case SendReceiveMessageType::e_PostPluginSave:
					MessageHandler_ReceivePostPluginSave();
					break;
			}
			break;
		case OperationType::e_SetOpenDialogItemData:
			SetOpenDialogItemData(Parameters->VanillaHandleIndex,
						dynamic_cast<String^>(Parameters->ParameterList[0]),
						(UInt32)Parameters->ParameterList[1],
						(UInt16)Parameters->ParameterList[2],
						(UInt32)Parameters->ParameterList[3]);
			break;
		case OperationType::e_SetVariableListItemData:
			SetVariableListItemData(Parameters->VanillaHandleIndex,
						dynamic_cast<String^>(Parameters->ParameterList[0]),
						(UInt8)Parameters->ParameterList[1],
						(UInt32)Parameters->ParameterList[2]);
			break;
		case OperationType::e_AllocateWorkspace:
			AllocateNewWorkspace((UInt32)Parameters->ParameterList[0],
						dynamic_cast<ScriptEditor::TabContainer^>(Parameters->ParameterList[1]));
			break;
		case OperationType::e_DestroyTabContainer:
			DestroyTabContainer(dynamic_cast<ScriptEditor::TabContainer^>(Parameters->ParameterList[0]));
			break;
		case OperationType::e_TabTearOp:
			TabTearOpHandler((TabTearOpType)Parameters->ParameterList[0],
						dynamic_cast<ScriptEditor::Workspace^>(Parameters->ParameterList[1]),
						dynamic_cast<ScriptEditor::TabContainer^>(Parameters->ParameterList[2]),
						(Point)(Parameters->ParameterList[3]));
			break;
		}
	}
	catch (CSEGeneralException^ E) {
		DebugPrint("ScriptEditorManager couldn't complete operation '" + TypeIdentifier[(int)Op] + "'\n\tException: " + E->Message, true);
	}
}


ScriptEditor::Workspace^ ScriptEditorManager::GetAllocatedWorkspace(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^ Result = ScriptEditor::Workspace::NullSE;
	for each (ScriptEditor::Workspace^% Itr in WorkspaceAllocationMap) {
		if (Itr->AllocatedIndex == AllocatedIndex) {
			Result = Itr;
			break;
		}
	}
	return Result;
}

void ScriptEditorManager::MoveScriptDataToVanillaEditor(ScriptEditor::Workspace^% CSEEditor, SaveWorkspaceOpType SaveOperation)
{
	CSEEditor->PreProcessScriptText(PreProcessor::PreProcessOp::e_Expand, nullptr, (SaveOperation == SaveWorkspaceOpType::e_SaveButDontCompile));
	CStringWrapper^ CScriptText = gcnew CStringWrapper(CSEEditor->PreProcessedText);
	g_ScriptDataPackage->Text = CScriptText->String();
	g_ScriptDataPackage->Type = CSEEditor->GetScriptType();
	g_ScriptDataPackage->ModifiedFlag = CSEEditor->EditorTab->ImageIndex;
	NativeWrapper::ScriptEditor_SetScriptData(CSEEditor->AllocatedIndex, g_ScriptDataPackage);
}

void ScriptEditorManager::MoveScriptDataFromVanillaEditor(ScriptEditor::Workspace^% CSEEditor)
{
	g_ScriptDataPackage = NativeWrapper::ScriptEditor_GetScriptData();

	switch (g_ScriptDataPackage->Type)
	{
	case 9:									// Function script
		g_ScriptDataPackage->Type = 0;
		break;
	case 99:
		DebugPrint("Couldn't fetch script data from the vanilla editor!", true);
		return;
	}

	InitializeScript(CSEEditor->AllocatedIndex, 
						gcnew String((const char*)g_ScriptDataPackage->Text), 
						g_ScriptDataPackage->Type, 
						gcnew String((const char*)g_ScriptDataPackage->EditorID), 
						(UInt32)g_ScriptDataPackage->ByteCode, 
						g_ScriptDataPackage->Length,
						g_ScriptDataPackage->FormID);
}

void ScriptEditorManager::AllocateNewTabContainer(UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height)
{
#ifdef _DEBUG
	Debugger::Launch();
#endif
	DebugPrint(String::Format("Allocated a new tab container"));
	TabContainerAllocationMap->AddLast(gcnew ScriptEditor::TabContainer(PosX, PosY, Width, Height));
	ScriptEditor::TabContainer::LastUsedBounds.X = PosX; 
	ScriptEditor::TabContainer::LastUsedBounds.Y = PosY; 
	ScriptEditor::TabContainer::LastUsedBounds.Width = Width; 
	ScriptEditor::TabContainer::LastUsedBounds.Height = Height; 
}

void ScriptEditorManager::InitializeScript(UInt32 AllocatedIndex, String^ ScriptText, UInt16 ScriptType, String^ ScriptName, UInt32 Data, UInt32 DataLength, UInt32 FormID)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	Itr->TextSet = true;
	Itr->PreProcessScriptText(PreProcessor::PreProcessOp::e_Collapse, ScriptText, false);
	Itr->ScriptEditorID = gcnew String(ScriptName);
	Itr->EditorTab->Text = ScriptName + " [" + FormID.ToString("X8") + "]";
	Itr->ParentStrip->EditorForm->Text = Itr->EditorTab->Text + " - CSE Editor";
	Itr->SetScriptType(ScriptType);

	Itr->EnableControls();
	Itr->EditorTab->ImageIndex = 0;
	Itr->ErrorBox->Items->Clear();
	Itr->VariableBox->Items->Clear();
	Itr->ToolBarByteCodeSize->Value = DataLength;
	Itr->ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(DataLength / (float)1024));

	Itr->CalculateLineOffsets(Data, DataLength, ScriptText);
	Itr->ISBox->UpdateLocalVars();

	Itr->GetVariableData = false;
	Itr->ToolBarUpdateVarIndices->Enabled = false;
	Itr->ClearFindImagePointers();
}



void ScriptEditorManager::MessageHandler_SendNew(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);
	
	MoveScriptDataToVanillaEditor(Itr, SaveWorkspaceOpType::e_SaveAndCompile);
	NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_New);

}
void ScriptEditorManager::MessageHandler_SendOpen(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);
		
	MoveScriptDataToVanillaEditor(Itr, SaveWorkspaceOpType::e_SaveAndCompile);
	NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Open);
}
void ScriptEditorManager::MessageHandler_SendPrevious(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);
		
	MoveScriptDataToVanillaEditor(Itr, SaveWorkspaceOpType::e_SaveAndCompile);
	NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Previous);
}
void ScriptEditorManager::MessageHandler_SendNext(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);
		
	MoveScriptDataToVanillaEditor(Itr, SaveWorkspaceOpType::e_SaveAndCompile);
	NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Next);
}
void ScriptEditorManager::MessageHandler_SendSave(UInt32 AllocatedIndex, SaveWorkspaceOpType Operation)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);
	
	MoveScriptDataToVanillaEditor(Itr, Operation);

	switch (Operation)
	{
	case SaveWorkspaceOpType::e_SaveAndCompile:
		NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Save);
		break;
	case SaveWorkspaceOpType::e_SaveButDontCompile:
		NativeWrapper::ScriptEditor_ToggleScriptCompiling(false);
		NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Save);
		NativeWrapper::ScriptEditor_ToggleScriptCompiling(true);
		break;
	case SaveWorkspaceOpType::e_SaveActivePluginToo:
		NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Save);
		NativeWrapper::ScriptEditor_SaveActivePlugin();
		break;
	}
}
void ScriptEditorManager::MessageHandler_SendRecompile(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);
		
	MoveScriptDataToVanillaEditor(Itr, SaveWorkspaceOpType::e_SaveAndCompile);
	NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Recompile);
}
void ScriptEditorManager::MessageHandler_SendDelete(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);
		
	MoveScriptDataToVanillaEditor(Itr, SaveWorkspaceOpType::e_SaveAndCompile);
	NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Delete);
}
void ScriptEditorManager::MessageHandler_SendClose(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	MoveScriptDataToVanillaEditor(Itr, SaveWorkspaceOpType::e_SaveAndCompile);
	NativeWrapper::ScriptEditor_SetWindowParameters(AllocatedIndex, 
													Itr->ParentStrip->EditorForm->Top, 
													Itr->ParentStrip->EditorForm->Left, 
													Itr->ParentStrip->EditorForm->Width, 
													Itr->ParentStrip->EditorForm->Height);
	NativeWrapper::ScriptEditor_MessagingInterface(AllocatedIndex, (UInt16)SendReceiveMessageType::e_Close);
}



void ScriptEditorManager::MessageHandler_ReceiveNew(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	InitializeScript(AllocatedIndex, "", (UInt16)ScriptParser::ScriptType::e_Object, "New Script", 0, 0, 0);
	Itr->EditorTab->ImageIndex = 1;
}
void ScriptEditorManager::MessageHandler_ReceiveOpen(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	MoveScriptDataFromVanillaEditor(Itr);
}
void ScriptEditorManager::MessageHandler_ReceivePrevious(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	MoveScriptDataFromVanillaEditor(Itr);
}
void ScriptEditorManager::MessageHandler_ReceiveNext(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	MoveScriptDataFromVanillaEditor(Itr);
}

void ScriptEditorManager::MessageHandler_ReceiveSave(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	Itr->EditorTab->ImageIndex = 0;
	g_ScriptDataPackage = NativeWrapper::ScriptEditor_GetScriptData();
	switch (g_ScriptDataPackage->Type)
	{
	case 9:									// Function script
		g_ScriptDataPackage->Type = 0;
		break;
	case 99:
		DebugPrint("Couldn't fetch script data from the vanilla editor!", true);
		return;
	}
	Itr->ScriptEditorID = gcnew String(g_ScriptDataPackage->EditorID);
	Itr->EditorTab->Text = Itr->ScriptEditorID + " [" + g_ScriptDataPackage->FormID.ToString("X8") + "]";
	Itr->ParentStrip->EditorForm->Text = Itr->EditorTab->Text + " - CSE Editor";
	Itr->CalculateLineOffsets((UInt32)g_ScriptDataPackage->ByteCode, g_ScriptDataPackage->Length, gcnew String(g_ScriptDataPackage->Text));
	Itr->ToolBarByteCodeSize->Value = g_ScriptDataPackage->Length;
	Itr->ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(g_ScriptDataPackage->Length / (float)1024));
	Itr->ValidateScript(Itr->GetScriptType());
	Itr->ToolBarUpdateVarIndices->Enabled = false;
	Itr->GetVariableIndices(false);
}
void ScriptEditorManager::MessageHandler_ReceiveClose(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	DebugPrint(String::Format("Released allocated workspace at index {0}", AllocatedIndex));
	Itr->Destroy();
	WorkspaceAllocationMap->Remove(Itr);
}

void ScriptEditorManager::MessageHandler_ReceiveLoadRelease()
{
	for each (ScriptEditor::TabContainer^% Itr in TabContainerAllocationMap) {
		Itr->Destroying = true;
	}
	for each (ScriptEditor::Workspace^% Itr in WorkspaceAllocationMap) {
		Itr->Destroy();
	}

	WorkspaceAllocationMap->Clear();
	TabContainerAllocationMap->Clear();

	DebugPrint("Released all allocated editors");
}
void ScriptEditorManager::MessageHandler_ReceivePostPluginLoad()
{
	ISDB->ForceUpdateDatabase();
}
void ScriptEditorManager::MessageHandler_ReceivePostPluginSave()
{
	ISDB->ForceUpdateDatabase();
}



void ScriptEditorManager::SetOpenDialogItemData(UInt32 AllocatedIndex, String^% ScriptName, UInt32 FormID, UInt16 Type, UInt32 Flags)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	String^ ScriptType;
	switch (Type)
	{
	case 0:
		ScriptType = "Object";
		break;
	case 1:
		ScriptType = "Quest";
		break;
	case 2:
		ScriptType = "Magic Effect";
		break;
	case 9:
		ScriptType = "Function";
		break;
	}
	Itr->ScriptListBox->AddScript(ScriptName, FormID.ToString("X8"), ScriptType, Flags);
}


void ScriptEditorManager::SetVariableListItemData(UInt32 AllocatedIndex, String^% Name, UInt32 Type, UInt32 Index)
{
	ScriptEditor::Workspace^% Itr = GetAllocatedWorkspace(AllocatedIndex);

	String^ VarType;
	switch (Type)
	{
	case 0:
		VarType = "Float";
		break;
	case 1:
		VarType = "Integer";
		break;
	case 2:
		VarType = "Reference";
		break;
	}

	ListViewItem^ Item = gcnew ListViewItem(Name);
	Item->SubItems->Add(VarType);
	Item->SubItems->Add(Index.ToString());
	Itr->VariableBox->Items->Add(Item);	
}

void ScriptEditorManager::AllocateNewWorkspace(UInt32 AllocatedIndex, ScriptEditor::TabContainer^% Parent)
{
	WorkspaceAllocationMap->AddLast(gcnew ScriptEditor::Workspace(AllocatedIndex, Parent));
	DebugPrint(String::Format("Allocated a new workspace at index {0}", AllocatedIndex));
}

void ScriptEditorManager::DestroyTabContainer(ScriptEditor::TabContainer^ Container)
{
	DebugPrint("Released an allocated container");
	Container->Destroy();
	TabContainerAllocationMap->Remove(Container);
}

void ScriptEditorManager::TabTearOpHandler(TabTearOpType Operation, ScriptEditor::Workspace^ Workspace, ScriptEditor::TabContainer^ Container, Point MousePos)
{
	switch (Operation)
	{
	case TabTearOpType::e_NewContainer:
		{
			AllocateNewTabContainer(MousePos.X, MousePos.Y, ScriptEditor::TabContainer::LastUsedBounds.Width, ScriptEditor::TabContainer::LastUsedBounds.Height);
			Container = TabContainerAllocationMap->Last->Value;
			TabTearOpHandler(TabTearOpType::e_RelocateToContainer, Workspace, Container, MousePos);
			MessageHandler_ReceiveClose((dynamic_cast<ScriptEditor::Workspace^>(Container->ScriptStrip->Tabs[1]->Tag))->AllocatedIndex);	
			DebugPrint("Moved workspace " + Workspace->AllocatedIndex.ToString() + " to a new tab container");
			break;
		}
	case TabTearOpType::e_RelocateToContainer:
		{
			ScriptEditor::TabContainer^ Parent = Workspace->ParentStrip;
			Workspace->ParentStrip = Container;
			Parent->Destroying = true;
			Parent->ScriptStrip->Tabs->Remove(Workspace->EditorTab);
			Parent->Destroying = false;
				
			Container->ScriptStrip->Tabs->Add(Workspace->EditorTab);
			Container->ScriptStrip->Controls->Add(Workspace->EditorControlBox);
			DebugPrint("Moved workspace " + Workspace->AllocatedIndex.ToString() + " to another tab container");
			break;
		}
	}
	Container->EditorForm->Invalidate(true);
	Container->ScriptStrip->SelectedTab = Workspace->EditorTab;
	Container->ScriptStrip->TabStrip->EnsureVisible(Container->ScriptStrip->SelectedTab);
}