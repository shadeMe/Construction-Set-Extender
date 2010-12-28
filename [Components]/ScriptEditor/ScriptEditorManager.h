#pragma once
#include "[Common]\Includes.h"
#include "ScriptEditor.h"

struct ScriptData;
using namespace DevComponents;

public ref class ScriptEditorManager
{
public:
	static enum class									OperationType
															{
																e_AllocateTabContainer = 0,
																e_InitializeScript,

																e_SendMessage,				// to the vanilla editor
																e_ReceiveMessage,			// from the vanilla editor, on a successful operation
																e_SetScriptSelectItemData,
																e_SetVariableListItemData,
																e_AllocateWorkspace,
																e_DestroyTabContainer,
																e_TabTearOp,
																e_AddToCompileErrorPool
															};

	static enum class									SendReceiveMessageType
															{
																e_New = 0,
																e_Open,
																e_Previous,
																e_Next,
																e_Save,
																e_Recompile,
																e_Delete,
																e_Close,
																							// receiving-only
																e_LoadRelease,
																e_PostPluginLoad,
																e_PostPluginSave
															};
	static enum class									TabTearOpType
															{
																e_NewContainer = 0,
																e_RelocateToContainer
															};
	static enum class									SaveWorkspaceOpType
															{
																e_SaveAndCompile = 0,
																e_SaveButDontCompile,
																e_SaveActivePluginToo
															};

	static array<String^>^								TypeIdentifier =
															{
																"Allocate TabContainer",
																"Initialize Script",
																"Send Message",
																"Receive Message",
																"Set ScriptSelect Item Data",
																"Set VariableList Item Data",
																"Allocate Workspace",
																"Destroy TabContainer",
																"Tab Tear Operation",
																"Add Compile Error To Message Pool"
															};

	static array<UInt16>^								ParamCount = 
															{ 
																4,
																5, 
																1, 
																1,
																3,
																3,
																2,
																1,
																4,
																2
															};
private:
	static ScriptEditorManager^							Singleton = nullptr;
	LinkedList<ScriptEditor::Workspace^>^				WorkspaceAllocationMap;
	LinkedList<ScriptEditor::TabContainer^>^			TabContainerAllocationMap;

	ScriptEditorManager();

	void												MoveScriptDataToVanillaEditor(ScriptEditor::Workspace^% CSEEditor, SaveWorkspaceOpType SaveOperation);
	void												MoveScriptDataFromVanillaEditor(ScriptEditor::Workspace^% CSEEditor);

																										// exported wrappers
	void												AllocateNewTabContainer(UInt32 PosX, UInt32 PosY, UInt32 Width, UInt32 Height);
	void												InitializeScript(UInt32 AllocatedIndex, String^ ScriptText, UInt16 ScriptType, String^ ScriptName, UInt32 Data, UInt32 DataLength, UInt32 FormID);

																										// message handlers
	void												MessageHandler_SendNew(UInt32 AllocatedIndex);
	void												MessageHandler_SendOpen(UInt32 AllocatedIndex);
	void												MessageHandler_SendPrevious(UInt32 AllocatedIndex);
	void												MessageHandler_SendNext(UInt32 AllocatedIndex);
	void												MessageHandler_SendSave(UInt32 AllocatedIndex, SaveWorkspaceOpType Operation);
	void												MessageHandler_SendRecompile(UInt32 AllocatedIndex);
	void												MessageHandler_SendDelete(UInt32 AllocatedIndex);
	void												MessageHandler_SendClose(UInt32 AllocatedIndex);


	void												MessageHandler_ReceiveNew(UInt32 AllocatedIndex);
	void												MessageHandler_ReceiveOpen(UInt32 AllocatedIndex);
	void												MessageHandler_ReceivePrevious(UInt32 AllocatedIndex);
	void												MessageHandler_ReceiveNext(UInt32 AllocatedIndex);
	void												MessageHandler_ReceiveSave(UInt32 AllocatedIndex);
	void												MessageHandler_ReceiveClose(UInt32 AllocatedIndex);
	void												MessageHandler_ReceiveLoadRelease();
	void												MessageHandler_ReceivePostPluginLoad();
	void												MessageHandler_ReceivePostPluginSave();

	void												SetScriptSelectItemData(UInt32 AllocatedIndex, String^% ScriptName, UInt32 FormID, UInt16 Type, UInt32 Flags);
	void												SetVariableListItemData(UInt32 AllocatedIndex, String^% Name, UInt32 Type, UInt32 Index);
	void												AllocateNewWorkspace(UInt32 AllocatedIndex, ScriptEditor::TabContainer^% Parent);
	void												DestroyTabContainer(ScriptEditor::TabContainer^ Container);
	void												TabTearOpHandler(TabTearOpType Operation, ScriptEditor::Workspace^ Workspace, ScriptEditor::TabContainer^ Container, Point MousePos);
	void												AddToCompileErrorPool(UInt32 AllocatedIndex, UInt32 Line, String^% Message);
public:
	ref struct											OperationParams
	{
		UInt32											VanillaHandleIndex;
		List<Object^>^									ParameterList;

		OperationParams() : VanillaHandleIndex(0), ParameterList(gcnew List<Object^>()) {}
	};

	property ScriptEditor::Workspace^					TornWorkspace;

	static ScriptEditorManager^%						GetSingleton();
	void												PerformOperation(OperationType Op, OperationParams^ Parameters);
	ScriptEditor::Workspace^							GetAllocatedWorkspace(UInt32 AllocatedIndex);
};

#define SEMGR											ScriptEditorManager::GetSingleton()

extern ScriptData*										g_ScriptDataPackage;