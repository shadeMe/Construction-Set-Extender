#include "ScriptEditorManager.h"
#include "Globals.h"
#include "[Common]\HandShakeStructs.h"
#include "[Common]\NativeWrapper.h"
#include "ScriptListDialog.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ScriptEditorManager::ScriptEditorManager()
		{
			AllocatedWorkspaces = gcnew LinkedList<ScriptEditor::Workspace^>();
			AllocatedWorkspaceContainers = gcnew LinkedList<ScriptEditor::WorkspaceContainer^>();
			NextWorkspaceHandleIndex = 1;
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
				case OperationType::e_AllocateWorkspaceContainer:
					AllocateNewWorkspaceContainer((ComponentDLLInterface::ScriptData*)((UInt32)Parameters->ParameterList[0]),
						(UInt32)Parameters->ParameterList[1],
						(UInt32)Parameters->ParameterList[2],
						(UInt32)Parameters->ParameterList[3],
						(UInt32)Parameters->ParameterList[4]);
					break;
				case OperationType::e_AllocateWorkspace:
					Result = AllocateNewWorkspace(dynamic_cast<ScriptEditor::WorkspaceContainer^>(Parameters->ParameterList[0]),
						(ComponentDLLInterface::ScriptData*)((UInt32)Parameters->ParameterList[1]));
					break;
				case OperationType::e_ReleaseWorkspaceContainer:
					ReleaseWorkspaceContainer(dynamic_cast<ScriptEditor::WorkspaceContainer^>(Parameters->ParameterList[0]));
					break;
				case OperationType::e_ReleaseWorkspace:
					ReleaseWorkspace(dynamic_cast<ScriptEditor::Workspace^>(Parameters->ParameterList[0]));
					break;
				case OperationType::e_WorkspaceTearing:
					WorkspaceTearingHandler((WorkspaceTearOpType)Parameters->ParameterList[0],
						dynamic_cast<ScriptEditor::Workspace^>(Parameters->ParameterList[1]),
						dynamic_cast<ScriptEditor::WorkspaceContainer^>(Parameters->ParameterList[2]),
						(Point)(Parameters->ParameterList[3]));
					break;
				case OperationType::e_CloseAllOpenEditors:
					CloseAllOpenEditors();
					break;
				}
			}
			catch (Exception^ E)
			{
				DebugPrint("ScriptEditorManager couldn't complete operation '" + TypeIdentifier[(int)Op] + "'", true);
				DebugPrint("\tException: " + E->Message);
				DebugPrint("\tStack Trace:\n" + E->StackTrace);

				if (Marshal::GetHRForException(E) == 0x80131515)
				{
					DebugPrint("This error is caused by a blocked .NET assembly. Ensure that the downloaded CSE archive and its contents were unblocked as specified in the readMe file");
				}
			}

			return Result;
		}

		ScriptEditor::Workspace^ ScriptEditorManager::GetAllocatedWorkspace(UInt32 AllocatedIndex)
		{
			ScriptEditor::Workspace^ Result = nullptr;

			for each (ScriptEditor::Workspace^ Itr in AllocatedWorkspaces)
			{
				if (Itr->GetHandleIndex() == AllocatedIndex)
				{
					Result = Itr;
					break;
				}
			}
			return Result;
		}

		void ScriptEditorManager::AllocateNewWorkspaceContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height)
		{
			AllocatedWorkspaceContainers->AddLast(gcnew ScriptEditor::WorkspaceContainer(InitScript, PosX, PosY, Width, Height));
			ScriptEditor::WorkspaceContainer::LastUsedBounds.X = PosX;
			ScriptEditor::WorkspaceContainer::LastUsedBounds.Y = PosY;
			ScriptEditor::WorkspaceContainer::LastUsedBounds.Width = Width;
			ScriptEditor::WorkspaceContainer::LastUsedBounds.Height = Height;
		}

		ScriptEditor::Workspace^ ScriptEditorManager::AllocateNewWorkspace(ScriptEditor::WorkspaceContainer^ Parent, ComponentDLLInterface::ScriptData* InitScript)
		{
			AllocatedWorkspaces->AddLast(gcnew ScriptEditor::Workspace(NextWorkspaceHandleIndex++, Parent, InitScript));
			return GetAllocatedWorkspace(NextWorkspaceHandleIndex - 1);
		}

		void ScriptEditorManager::ReleaseWorkspaceContainer(ScriptEditor::WorkspaceContainer^ Container)
		{
			AllocatedWorkspaceContainers->Remove(Container);
			delete Container;
		}

		void ScriptEditorManager::ReleaseWorkspace( ScriptEditor::Workspace^ Workspace)
		{
			AllocatedWorkspaces->Remove(Workspace);
			delete Workspace;
		}

		void ScriptEditorManager::WorkspaceTearingHandler(WorkspaceTearOpType Operation, ScriptEditor::Workspace^ Workspace, ScriptEditor::WorkspaceContainer^ Container, Point MousePos)
		{
			switch (Operation)
			{
			case WorkspaceTearOpType::e_NewContainer:
				{
					AllocateNewWorkspaceContainer(0, MousePos.X, MousePos.Y, ScriptEditor::WorkspaceContainer::LastUsedBounds.Width, ScriptEditor::WorkspaceContainer::LastUsedBounds.Height);
					Container = AllocatedWorkspaceContainers->Last->Value;
					WorkspaceTearingHandler(WorkspaceTearOpType::e_RelocateToContainer, Workspace, Container, MousePos);
					Container->LookupWorkspaceByTabIndex(0)->CloseScript();
					break;
				}
			case WorkspaceTearOpType::e_RelocateToContainer:
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
			for each (ScriptEditor::Workspace^ Itr in AllocatedWorkspaces)
				delete Itr;
			for each (ScriptEditor::WorkspaceContainer^ Itr in AllocatedWorkspaceContainers)
				delete Itr;

			AllocatedWorkspaces->Clear();
			AllocatedWorkspaceContainers->Clear();
		}

		UInt32 ScriptEditorManager::GetOpenEditorCount( void )
		{
			return AllocatedWorkspaces->Count;
		}
	}
}