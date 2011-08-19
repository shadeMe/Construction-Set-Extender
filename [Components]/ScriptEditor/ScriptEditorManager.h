#pragma once
#include "ScriptEditor.h"

using namespace DevComponents;

public ref class ScriptEditorManager
{
public:
	static enum class									OperationType
															{
																e_AllocateTabContainer = 0,
																e_AllocateWorkspace,
																e_ReleaseTabContainer,
																e_ReleaseWorkspace,
																e_TabTearOp,
																e_CloseAllOpenEditors
															};

	static enum class									TabTearOpType
															{
																e_NewContainer = 0,
																e_RelocateToContainer
															};

	static array<String^>^								TypeIdentifier =
															{
																"Allocate TabContainer",
																"Allocate Workspace",
																"Release TabContainer",
																"Relese Workspace",
																"Tab Tear Operation",
																"Close All Open Editors"
															};

	static array<UInt16>^								ParamCount =
															{
																5,	// init script, {window bounds}
																2,	// init script, handle index
																1,	// tabcontainer
																1,	// workspace
																4,
																0
															};
private:
	static ScriptEditorManager^							Singleton = nullptr;

	LinkedList<ScriptEditor::Workspace^>^				AllocatedWorkspaces;
	LinkedList<ScriptEditor::TabContainer^>^			AllocatedTabContainers;
	UInt32												NextEditorHandleIndex;

	ScriptEditorManager();

	void												AllocateNewTabContainer(ComponentDLLInterface::ScriptData* InitScript, UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);
	ScriptEditor::Workspace^							AllocateNewWorkspace(ScriptEditor::TabContainer^% Parent, ComponentDLLInterface::ScriptData* InitScript);
	void												ReleaseTabContainer(ScriptEditor::TabContainer^ Container);
	void												ReleaseWorkspace(ScriptEditor::Workspace^ Workspace);
	void												TabTearOpHandler(TabTearOpType Operation, ScriptEditor::Workspace^ Workspace, ScriptEditor::TabContainer^ Container, Point MousePos);
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
	Object^												PerformOperation(OperationType Op, OperationParams^ Parameters);
	ScriptEditor::Workspace^							GetAllocatedWorkspace(UInt32 AllocatedIndex);
	UInt32												GetOpenEditorCount(void) { return AllocatedWorkspaces->Count; }
};

#define SEMGR											ScriptEditorManager::GetSingleton()