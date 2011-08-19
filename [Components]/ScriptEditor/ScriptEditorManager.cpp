#include "ScriptEditorManager.h"
#include "Globals.h"
#include "[Common]\HandShakeStructs.h"
#include "[Common]\NativeWrapper.h"
#include "ScriptListDialog.h"
#include "IntelliSense.h"

using namespace IntelliSense;

ScriptEditorManager::ScriptEditorManager()
{
	AllocatedWorkspaces = gcnew LinkedList<ScriptEditor::Workspace^>();
	AllocatedTabContainers = gcnew LinkedList<ScriptEditor::TabContainer^>();
	NextEditorHandleIndex = 1;
}

ScriptEditorManager^% ScriptEditorManager::GetSingleton()
{
	if (Singleton == nullptr)
		Singleton = gcnew ScriptEditorManager();

	return Singleton;
}

Object^ ScriptEditorManager::PerformOperation(ScriptEditorManager::OperationType Op, ScriptEditorManager::OperationParams^ Parameters)
{
	Object^ Result = nullptr;

	try
	{
		UInt16 NoOfParams = Parameters->ParameterList->Count, ParamsRequired = ParamCount[(int)Op];

		if (NoOfParams < ParamsRequired)
			throw gcnew CSEGeneralException("Incorrect number of parameters passed (" + NoOfParams + ")");
		if (GetAllocatedWorkspace(Parameters->EditorHandleIndex) == nullptr && Parameters->EditorHandleIndex)
			throw gcnew CSEGeneralException("Invalid editor index '" + Parameters->EditorHandleIndex + "' passed");

		switch (Op)
		{
		case OperationType::e_AllocateTabContainer:
			AllocateNewTabContainer((ComponentDLLInterface::ScriptData*)((UInt32)Parameters->ParameterList[0]),
								(UInt32)Parameters->ParameterList[1],
								(UInt32)Parameters->ParameterList[2],
								(UInt32)Parameters->ParameterList[3],
								(UInt32)Parameters->ParameterList[4]);
			break;
		case OperationType::e_AllocateWorkspace:
			Result = AllocateNewWorkspace(dynamic_cast<ScriptEditor::TabContainer^>(Parameters->ParameterList[0]),
										(ComponentDLLInterface::ScriptData*)((UInt32)Parameters->ParameterList[1]));
			break;
		case OperationType::e_ReleaseTabContainer:
			ReleaseTabContainer(dynamic_cast<ScriptEditor::TabContainer^>(Parameters->ParameterList[0]));
			break;
		case OperationType::e_ReleaseWorkspace:
			ReleaseWorkspace(dynamic_cast<ScriptEditor::Workspace^>(Parameters->ParameterList[0]));
			break;
		case OperationType::e_TabTearOp:
			TabTearOpHandler((TabTearOpType)Parameters->ParameterList[0],
						dynamic_cast<ScriptEditor::Workspace^>(Parameters->ParameterList[1]),
						dynamic_cast<ScriptEditor::TabContainer^>(Parameters->ParameterList[2]),
						(Point)(Parameters->ParameterList[3]));
			break;
		case OperationType::e_CloseAllOpenEditors:
			CloseAllOpenEditors();
			break;
		}
	}
	catch (Exception^ E)
	{
		DebugPrint("ScriptEditorManager couldn't complete operation '" + TypeIdentifier[(int)Op] + "'\n\tException: " + E->Message, true);
	}

	return Result;
}

ScriptEditor::Workspace^ ScriptEditorManager::GetAllocatedWorkspace(UInt32 AllocatedIndex)
{
	ScriptEditor::Workspace^ Result = nullptr;

	for each (ScriptEditor::Workspace^% Itr in AllocatedWorkspaces)
	{
		if (Itr->GetAllocatedIndex() == AllocatedIndex)
		{
			Result = Itr;
			break;
		}
	}
	return Result;
}

void ScriptEditorManager::AllocateNewTabContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height)
{
	AllocatedTabContainers->AddLast(gcnew ScriptEditor::TabContainer(InitScript, PosX, PosY, Width, Height));
	ScriptEditor::TabContainer::LastUsedBounds.X = PosX;
	ScriptEditor::TabContainer::LastUsedBounds.Y = PosY;
	ScriptEditor::TabContainer::LastUsedBounds.Width = Width;
	ScriptEditor::TabContainer::LastUsedBounds.Height = Height;
}

ScriptEditor::Workspace^ ScriptEditorManager::AllocateNewWorkspace(ScriptEditor::TabContainer^% Parent, ComponentDLLInterface::ScriptData* InitScript)
{
	AllocatedWorkspaces->AddLast(gcnew ScriptEditor::Workspace(NextEditorHandleIndex++, Parent, InitScript));
	return GetAllocatedWorkspace(NextEditorHandleIndex - 1);
}

void ScriptEditorManager::ReleaseTabContainer(ScriptEditor::TabContainer^ Container)
{
	AllocatedTabContainers->Remove(Container);
	delete Container;
}

void ScriptEditorManager::ReleaseWorkspace( ScriptEditor::Workspace^ Workspace)
{
	AllocatedWorkspaces->Remove(Workspace);
	delete Workspace;
}

void ScriptEditorManager::TabTearOpHandler(TabTearOpType Operation, ScriptEditor::Workspace^ Workspace, ScriptEditor::TabContainer^ Container, Point MousePos)
{
	switch (Operation)
	{
	case TabTearOpType::e_NewContainer:
		{
			AllocateNewTabContainer(0, MousePos.X, MousePos.Y, ScriptEditor::TabContainer::LastUsedBounds.Width, ScriptEditor::TabContainer::LastUsedBounds.Height);
			Container = AllocatedTabContainers->Last->Value;
			TabTearOpHandler(TabTearOpType::e_RelocateToContainer, Workspace, Container, MousePos);
			Container->LookupWorkspaceByTabIndex(0)->CloseScript();
			break;
		}
	case TabTearOpType::e_RelocateToContainer:
		{
			Workspace->Relocate(Container);
			break;
		}
	}
	Container->Redraw();
	Workspace->BringToFront();
}

void ScriptEditorManager::CloseAllOpenEditors()
{
	for each (ScriptEditor::Workspace^% Itr in AllocatedWorkspaces)
		delete Itr;
	for each (ScriptEditor::TabContainer^% Itr in AllocatedTabContainers)
		delete Itr;

	AllocatedWorkspaces->Clear();
	AllocatedTabContainers->Clear();
}