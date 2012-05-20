#pragma once
#include "ScriptEditor.h"

using namespace DevComponents;

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ref class ScriptEditorManager
		{
		public:
			static enum class									OperationType
			{
				e_AllocateWorkspaceContainer = 0,
				e_AllocateWorkspace,
				e_ReleaseWorkspaceContainer,
				e_ReleaseWorkspace,
				e_WorkspaceTearing,
				e_CloseAllOpenEditors
			};

			static enum class									WorkspaceTearOpType
			{
				e_NewContainer = 0,
				e_RelocateToContainer
			};

			static array<String^>^								TypeIdentifier =
			{
				"Allocate Workspace Container",
				"Allocate Workspace",
				"Release Workspace Container",
				"Relese Workspace",
				"Workspace/Tab Tearing",
				"Close All Open Editors"
			};

			static array<UInt16>^								ParamCount =
			{
				5,	// init script, {window bounds}
				2,	// init script, handle index
				1,	// workspacecontainer
				1,	// workspace
				4,	// op type, workspace, workspacecontainer, mouse pos
				0
			};
		private:
			static ScriptEditorManager^							Singleton = nullptr;

			LinkedList<ScriptEditor::Workspace^>^				AllocatedWorkspaces;
			LinkedList<ScriptEditor::WorkspaceContainer^>^		AllocatedWorkspaceContainers;
			UInt32												NextWorkspaceHandleIndex;

			ScriptEditorManager();

			void												AllocateNewWorkspaceContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);
			ScriptEditor::Workspace^							AllocateNewWorkspace(ScriptEditor::WorkspaceContainer^ Parent, ComponentDLLInterface::ScriptData* InitScript);
			void												ReleaseWorkspaceContainer(ScriptEditor::WorkspaceContainer^ Container);
			void												ReleaseWorkspace(ScriptEditor::Workspace^ Workspace);
			void												WorkspaceTearingHandler(WorkspaceTearOpType Operation, ScriptEditor::Workspace^ Workspace, ScriptEditor::WorkspaceContainer^ Container, Point MousePos);
			void												CloseAllOpenEditors(void);
		public:
			ref struct											OperationParams
			{
				UInt32											EditorHandleIndex;
				List<Object^>^									ParameterList;

				OperationParams() : EditorHandleIndex(0), ParameterList(gcnew List<Object^>()) {}
			};

			property ScriptEditor::Workspace^					TornWorkspace;

			static ScriptEditorManager^%						GetSingleton();

			virtual Object^										PerformOperation(OperationType Op, OperationParams^ Parameters);
			ScriptEditor::Workspace^							GetAllocatedWorkspace(UInt32 AllocatedIndex);
			UInt32												GetOpenEditorCount(void);
		};

#define SEMGR											ScriptEditorManager::GetSingleton()
	}
}