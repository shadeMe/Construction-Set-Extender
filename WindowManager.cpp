#include "ExtenderInternals.h"
#include "WindowManager.h"
#include "[Common]\HandShakeStructs.h"
#include "[Common]\CLIWrapper.h"
#include "Console.h"
#include "resource.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "CSInterop.h"
#include "Exports.h"
#include "Hooks\Misc.h"
#include "Hooks\Dialog.h"
#include "Hooks\AssetSelector.h"
#include "ScriptEditorAllocator.h"
#include "CSDialogs.h"
#include "RenderSelectionGroupManager.h"
#include "RenderTimeManager.h"
#include "ToolManager.h"
#include "WorkspaceManager.h"
#include "RenderWindowTextPainter.h"

WNDPROC						g_FindTextOrgWindowProc = NULL;
WNDPROC						g_DataDlgOrgWindowProc = NULL;
WNDPROC						g_CSMainWndOrgWindowProc = NULL;
WNDPROC						g_RenderWndOrgWindowProc = NULL;
WNDPROC						g_ConsoleWndOrgWindowProc = NULL;
WNDPROC						g_ConsoleEditControlOrgWindowProc = NULL;
WNDPROC						g_ConsoleCmdBoxOrgWindowProc = NULL;
WNDPROC						g_ObjectWndOrgWindowProc = NULL;
WNDPROC						g_CellViewWndOrgWindowProc = NULL;
WNDPROC						g_ResponseWndOrgWindowProc = NULL;

WNDPROC						g_TagBrowserOrgWindowProc = NULL;

HFONT						g_CSDefaultFont = NULL;

#define PI					3.151592653589793

LRESULT CALLBACK FindTextDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMACTIVATE:				// ID = 1018
			NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 0, g_TextBuffer, sizeof(g_TextBuffer));
			std::string FormID, FormTypeStr(g_TextBuffer);

			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 1, g_TextBuffer, sizeof(g_TextBuffer));
			FormID = g_TextBuffer;
			UInt32 PadStart = FormID.find("(") + 1, PadEnd  = FormID.find(")", PadStart + 1);
			if (PadStart != std::string::npos && PadEnd != std::string::npos)
			{
				FormID = FormID.substr(PadStart, PadEnd - PadStart);
				UInt32 FormIDInt = 0;
				sscanf_s(FormID.c_str(), "%08X", &FormIDInt);
				if (TESForm_LookupByFormID(FormIDInt))
					ShowFormEditDialog(FormIDInt, FormTypeStr.c_str());
			}
			break;
		}
		break;
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_FindTextOrgWindowProc);
		break;
	}

	return CallWindowProc(g_FindTextOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK DataDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static ModEntry::Data** s_ActiveTESFile = (ModEntry::Data**)0x00A0AA7C;

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case DATA_QUICKLOAD:
			g_QuickLoadToggle = (IsDlgButtonChecked(hWnd, DATA_QUICKLOAD) == BST_CHECKED);
			break;
		case DATA_SETSTARTUPPLUGIN:
			{
			HWND PluginList = GetDlgItem(hWnd, 1056);
			int SelectedItem = ListView_GetNextItem(PluginList, -1, LVNI_SELECTED);

			if (SelectedItem != -1)
			{
				LVITEM SelectedPluginItem;

				SelectedPluginItem.iItem = SelectedItem;
				SelectedPluginItem.iSubItem = 0;
				SelectedPluginItem.mask = LVIF_TEXT;
				SelectedPluginItem.pszText = g_TextBuffer;
				SelectedPluginItem.cchTextMax = sizeof(g_TextBuffer);

				if (ListView_GetItem(PluginList, &SelectedPluginItem) == TRUE)
				{
					g_INIManager->FetchSetting("StartupPluginName")->SetValue(g_TextBuffer);

					char Buffer[0x200];
					sprintf_s(Buffer, 0x200, "Startup plugin set to '%s'.", g_TextBuffer);

					MessageBox(hWnd, Buffer, "CSE", MB_OK|MB_ICONEXCLAMATION);
					DebugPrint(Buffer);
				}
			}
			break;
			}
		case 1:		// OK btn
			if (EDAL->GetTrackedEditorCount())
			{
				if (MessageBox(hWnd, "There are open script windows. Are you sure you'd like to proceed?", "CSE", MB_YESNO|MB_ICONWARNING) == IDNO)
					return FALSE;
			}
			else if (*s_ActiveTESFile == NULL && g_QuickLoadToggle)
			{
				MessageBox(hWnd, "An active plugin must be set when quick-loading.", "CSE", MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else if ((*s_ActiveTESFile) != NULL && !_stricmp((*s_ActiveTESFile)->name, "oblivion.esm"))
			{
				if (MessageBox(hWnd, "You have set Oblvion.esm as an active file. Are you absolutely sure this is the end of the world ?", "CSE", MB_YESNO|MB_ICONWARNING) == IDNO)
					return FALSE;
			}

			break;
		}
		break;
	case WM_DESTROY:
		TESFile* ActiveFile = (*g_dataHandler)->unk8B8.activeFile;					// required for correct esm handling
		if (ActiveFile)
			ToggleFlag(&ActiveFile->flags, ModEntry::Data::kFlag_IsMaster, 0);

		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_DataDlgOrgWindowProc);
		break;
	}

	return CallWindowProc(g_DataDlgOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CSMainWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case 0x40C:				// save handler
		if (g_QuickLoadToggle) {
			if (MessageBox(*g_HWND_CSParent,
					"Are you sure you want to save the quick-loaded active plugin? There will be a loss of data if it contains master-dependent records.",
					"Save Warning",
					MB_ICONWARNING|MB_YESNO) == IDNO)
			{
				return FALSE;
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case MAIN_DATA_SAVEAS:
			if (!(*g_dataHandler)->unk8B8.activeFile)
			{
				MessageBox(*g_HWND_CSParent, "An active plugin must be set before using this tool.", "CSE", MB_OK|MB_ICONEXCLAMATION);
				break;
			}

			*g_WorkingFileFlag = 0;

			char FileName[0x104];
			if (SelectTESFileCommonDialog(hWnd, g_LocalMasterPath->sData, 0, FileName, 0x104))
			{
				ModEntry::Data* SaveAsBuffer = (*g_dataHandler)->unk8B8.activeFile;

				ToggleFlag(&SaveAsBuffer->flags, ModEntry::Data::kFlag_Active, false);
				ToggleFlag(&SaveAsBuffer->flags, ModEntry::Data::kFlag_Loaded, false);

				(*g_dataHandler)->unk8B8.activeFile = NULL;

				if (SendMessage(*g_HWND_CSParent, 0x40C, NULL, (LPARAM)FileName))
				{
					TESDialog_SetCSWindowTitleModifiedFlag(false);
				}
				else
				{
					(*g_dataHandler)->unk8B8.activeFile = SaveAsBuffer;
					ToggleFlag(&SaveAsBuffer->flags, ModEntry::Data::kFlag_Active, true);
					ToggleFlag(&SaveAsBuffer->flags, ModEntry::Data::kFlag_Loaded, true);
				}
			}

			*g_WorkingFileFlag = 1;
			break;
		case MAIN_WORLD_BATCHEDIT:
			{
			TESObjectCELL* ThisCell = (*g_TES)->currentInteriorCell;
			if (!ThisCell)	ThisCell = (*g_TES)->currentExteriorCell;

			if (ThisCell) 
			{
				UInt32 RefCount = 0, i = 0;
				TESObjectCELL::ObjectListEntry* ThisNode = &ThisCell->objectList;
				TESObjectREFR* ThisRef = NULL;

				while (ThisNode) {
					ThisRef = ThisNode->refr;
					if (!ThisRef)		break;

					RefCount++;
					ThisNode = ThisNode->Next();
				}

				if (RefCount < 2)	break;

				CellObjectData* RefData = new CellObjectData[RefCount], *ThisRefData = NULL;
				BatchRefData* BatchData = new BatchRefData();

				ThisNode = &ThisCell->objectList;
				while (ThisNode)
				{
					ThisRef = ThisNode->refr;
					if (!ThisRef)		break;
					ThisRefData = &RefData[i];

					ThisRefData->EditorID = (!ThisRef->editorData.editorID.m_data)?ThisRef->baseForm->editorData.editorID.m_data:ThisRef->editorData.editorID.m_data;
					ThisRefData->FormID = ThisRef->refID;
					ThisRefData->TypeID = ThisRef->baseForm->typeID;
					ThisRefData->Flags = ThisRef->flags;
					ThisRefData->Selected = false;
					for (TESRenderSelection::SelectedObjectsEntry* j = (*g_TESRenderSelectionPrimary)->RenderSelection; j != 0; j = j->Next)
					{
						if (j->Data && j->Data == ThisRef)
						{
							ThisRefData->Selected = true;
							break;
						}
					}
					ThisRefData->ParentForm = ThisRef;

					i++;
					ThisNode = ThisNode->Next();
				}

				BatchData->CellObjectListHead = RefData;
				BatchData->ObjectCount = RefCount;

				if (CLIWrapper::BatchEditor::InitializeRefBatchEditor(BatchData))
				{
					EnterCriticalSection(g_ExtraListCS);

					for (UInt32 k = 0; k < RefCount; k++)
					{
						ThisRef = (TESObjectREFR*)RefData[k].ParentForm;
						ThisRefData = &RefData[k];
						bool Modified = false;

						if (ThisRefData->Selected)
						{
							if (BatchData->World3DData.UsePosX())	ThisRef->posX = BatchData->World3DData.PosX, Modified = true;
							if (BatchData->World3DData.UsePosY())	ThisRef->posY = BatchData->World3DData.PosY, Modified = true;
							if (BatchData->World3DData.UsePosZ())	ThisRef->posZ = BatchData->World3DData.PosZ, Modified = true;

							if (BatchData->World3DData.UseRotX())	ThisRef->rotX = BatchData->World3DData.RotX * PI / 180, Modified = true;
							if (BatchData->World3DData.UseRotY())	ThisRef->rotY = BatchData->World3DData.RotY * PI / 180, Modified = true;
							if (BatchData->World3DData.UseRotZ())	ThisRef->rotZ = BatchData->World3DData.RotZ * PI / 180, Modified = true;

							if (BatchData->World3DData.UseScale())	ThisRef->scale = BatchData->World3DData.Scale, Modified = true;

							if (BatchData->Flags.UsePersistent() &&
								ThisRef->baseForm->typeID != kFormType_NPC &&
								ThisRef->baseForm->typeID != kFormType_Creature)
								ToggleFlag(&ThisRef->flags, TESObjectREFR::kFlags_Persistent, BatchData->Flags.Persistent), Modified = true;
							if (BatchData->Flags.UseDisabled())		ToggleFlag(&ThisRef->flags, TESObjectREFR::kFlags_Disabled, BatchData->Flags.Disabled), Modified = true;
							if (BatchData->Flags.UseVWD())			ToggleFlag(&ThisRef->flags, TESForm::kFormFlags_VisibleWhenDistant, BatchData->Flags.VWD), Modified = true;

							if (BatchData->EnableParent.UseEnableParent())
							{
								TESObjectREFR* Parent = (TESObjectREFR*)BatchData->EnableParent.Parent;
								if (Parent != ThisRef)
								{
									thisCall(kBaseExtraList_ModExtraEnableStateParent, &ThisRef->baseExtraList, Parent);
									thisCall(kTESObjectREFR_SetExtraEnableStateParent_OppositeState, ThisRef, BatchData->EnableParent.OppositeState);
									Modified = true;
								}
							}

							if (BatchData->Ownership.UseOwnership() &&
								ThisRef->baseForm->typeID != kFormType_NPC &&
								ThisRef->baseForm->typeID != kFormType_Creature)
							{
								thisCall(kBaseExtraList_ModExtraGlobal, &ThisRef->baseExtraList, 0);
								thisCall(kBaseExtraList_ModExtraRank, &ThisRef->baseExtraList, -1);
								thisCall(kBaseExtraList_ModExtraOwnership, &ThisRef->baseExtraList, 0);

								TESForm* Owner = (TESForm*)BatchData->Ownership.Owner;
								thisCall(kBaseExtraList_ModExtraOwnership, &ThisRef->baseExtraList, Owner);
								if (BatchData->Ownership.UseNPCOwner())
								{
									thisCall(kBaseExtraList_ModExtraGlobal, &ThisRef->baseExtraList, (TESGlobal*)BatchData->Ownership.Global);
								}
								else
								{
									thisCall(kBaseExtraList_ModExtraRank, &ThisRef->baseExtraList, BatchData->Ownership.Rank);
								}
								Modified = true;
							}

							if (BatchData->Extra.UseCharge())		thisCall(kTESObjectREFR_ModExtraCharge, ThisRef, BatchData->Extra.Charge), Modified = true;
							if (BatchData->Extra.UseHealth())		thisCall(kTESObjectREFR_ModExtraHealth, ThisRef, BatchData->Extra.Health), Modified = true;
							if (BatchData->Extra.UseTimeLeft())		thisCall(kTESObjectREFR_ModExtraTimeLeft, ThisRef, BatchData->Extra.TimeLeft), Modified = true;
							if (BatchData->Extra.UseSoulLevel())	thisCall(kTESObjectREFR_ModExtraSoul, ThisRef, BatchData->Extra.SoulLevel), Modified = true;
							if (BatchData->Extra.UseCount())
							{
								switch (ThisRef->baseForm->typeID)
								{
									case kFormType_Apparatus:
									case kFormType_Armor:
									case kFormType_Book:
									case kFormType_Clothing:
									case kFormType_Ingredient:
									case kFormType_Misc:
									case kFormType_Weapon:
									case kFormType_Ammo:
									case kFormType_SoulGem:
									case kFormType_Key:
									case kFormType_AlchemyItem:
									case kFormType_SigilStone:
										thisCall(kBaseExtraList_ModExtraCount, &ThisRef->baseExtraList, BatchData->Extra.Count), Modified = true;
									case kFormType_Light:
										TESObjectLIGH* Light = CS_CAST(ThisRef->baseForm, TESForm, TESObjectLIGH);
										if (Light)
											if (Light->IsCarriable())
												thisCall(kBaseExtraList_ModExtraCount, &ThisRef->baseExtraList, BatchData->Extra.Count), Modified = true;
								}
							}
						}

						if (Modified)
						{
							thisVirtualCall(*((UInt32*)ThisRef), 0x104, ThisRef);	// UpdateUsageInfo
							thisVirtualCall(*((UInt32*)ThisRef), 0x94, ThisRef, 1);	// SetFromActiveFile
							thisVirtualCall(*((UInt32*)ThisRef), 0xB8, ThisRef, ThisRef); // CopyFrom, to update NiNode data
							thisVirtualCall(*((UInt32*)ThisRef), 0x17C, ThisRef, thisCall(kTESObjectREFR_GetExtraRef3DData, ThisRef));
						}
					}

					LeaveCriticalSection(g_ExtraListCS);
				}

				delete [] RefData;
				delete BatchData;
			}
			break;
			}
		case MAIN_VIEW_CONSOLEWINDOW:
			if (CONSOLE->IsConsoleInitalized()) {
				HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
				if (CONSOLE->ToggleDisplayState())
					CheckMenuItem(ViewMenu, MAIN_VIEW_CONSOLEWINDOW, MF_CHECKED);
				else
					CheckMenuItem(ViewMenu, MAIN_VIEW_CONSOLEWINDOW, MF_UNCHECKED);
			}
			break;
		case MAIN_VIEW_MODIFIEDRECORDS:
			FormEnumerationWrapper::ToggleUnmodifiedFormVisibility();
			break;
		case MAIN_DATA_CSEPREFERENCES:
			g_INIEditGUI->InitializeGUI(g_DLLInstance, *g_HWND_CSParent, g_INIManager);
			break;
		case MAIN_VIEW_DELETEDRECORDS:
			FormEnumerationWrapper::ToggleDeletedFormVisibility();
			break;
		case MAIN_WORLD_UNLOADCELL:
			ResetRenderWindow();
			break;
		case MAIN_GAMEPLAY_GLOBALSCRIPT:
			CreateDialog(g_DLLInstance, MAKEINTRESOURCE(DLG_GLOBALSCRIPT), hWnd, (DLGPROC)GlobalScriptDlgProc);
			break;
		case MAIN_LAUNCHGAME:
			ShellExecute(NULL, "open", (LPSTR)(std::string(g_AppPath + "obse_loader.exe")).c_str(), NULL, NULL, SW_SHOW);
			break;
		case MAIN_VIEW_TAGBROWSER:
			CLIWrapper::TagBrowser::Show(*g_HWND_CSParent);
			break;
		case MAIN_DATA_SETWORKSPACE:
			g_WorkspaceManager.SelectWorkspace(NULL);
			break;
		case ID_TOOLS_MANAGE:
			DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_MANAGETOOLS), *g_HWND_CSParent, ManageToolsDlgProc);
			break;
		case MAIN_DATA_SAVEESPMASTERS:
			{
				HMENU FileMenu = GetMenu(*g_HWND_CSParent); FileMenu = GetSubMenu(FileMenu, 0);
				bool Checked = (GetMenuState(FileMenu, MAIN_DATA_SAVEESPMASTERS, MF_BYCOMMAND)) & MF_CHECKED;
				if (!Checked)
				{
					g_INIManager->FetchSetting("SaveLoadedESPsAsMasters")->SetValue("1");
					CheckMenuItem(FileMenu, MAIN_DATA_SAVEESPMASTERS, MF_CHECKED);
				}
				else
				{
					g_INIManager->FetchSetting("SaveLoadedESPsAsMasters")->SetValue("0");
					CheckMenuItem(FileMenu, MAIN_DATA_SAVEESPMASTERS, MF_UNCHECKED);
				}
			}

			break;
		}

		if (LOWORD(wParam) > ToolMenuIdentifierBase)
			g_ToolManager.RunTool(LOWORD(wParam));

		break;
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_CSMainWndOrgWindowProc);
		break;
	}

	return CallWindowProc(g_CSMainWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK RenderWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case RENDER_BATCHEDIT:
			SendMessage(*g_HWND_CSParent, WM_COMMAND, MAIN_WORLD_BATCHEDIT, 0);
			break;
		case ID_SELECTIONVISIBILITY_SHOWALL:
			{
				TESObjectCELL* CurrentCell = (*g_TES)->currentInteriorCell;
				if (CurrentCell == NULL)
					CurrentCell = (*g_TES)->currentExteriorCell;
				if (CurrentCell)
				{
					for (TESObjectCELL::ObjectListEntry* Itr = &CurrentCell->objectList; Itr && Itr->refr; Itr = Itr->next)
					{
						TESObjectREFR* Ref = Itr->refr;
						ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_3DInvisible, false);
						ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_Children3DInvisible, false);
					}

					PrintToRender("Reset visibility flags on the active cell's references", 3);
				}
			}
			break;
		case ID_SELECTIONVISIBILITY_TOGGLEVISIBILITY:
		case ID_SELECTIONVISIBILITY_TOGGLECHILDRENVISIBILITY:
		case ID_SELECTIONFREEZING_FREEZE:
		case ID_SELECTIONFREEZING_UNFREEZE:
			for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
				UInt32 FlagMask = 0;

				switch (LOWORD(wParam))
				{
				case ID_SELECTIONVISIBILITY_TOGGLEVISIBILITY:
					ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_3DInvisible, !(Ref->flags & kTESObjectREFRSpecialFlags_3DInvisible));
					PrintToBuffer("Selection '%08X's visibility toggled", Ref->refID);
					break;
				case ID_SELECTIONVISIBILITY_TOGGLECHILDRENVISIBILITY:
					ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_Children3DInvisible, !(Ref->flags & kTESObjectREFRSpecialFlags_Children3DInvisible));
					PrintToBuffer("Selection '%08X's children visibility toggled", Ref->refID);
					break;
				case ID_SELECTIONFREEZING_FREEZE:
					ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_Frozen, true);
					PrintToBuffer("Selection '%08X' frozen", Ref->refID);
					break;
				case ID_SELECTIONFREEZING_UNFREEZE:
					ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_Frozen, false);
					PrintToBuffer("Selection '%08X' thawed", Ref->refID);
					break;
				}
				PrintToRender(g_TextBuffer, 2);
			}
			break;
		case ID_SELECTIONGROUPING_GROUP:
		case ID_SELECTIONGROUPING_UNGROUP:
			if ((*g_TESRenderSelectionPrimary)->SelectionCount > 1)
			{
				TESObjectCELL* CurrentCell = (*g_TES)->currentInteriorCell;
				if (CurrentCell == NULL)
					CurrentCell = (*g_TES)->currentExteriorCell;

				if (CurrentCell == NULL)
					break;

				switch (LOWORD(wParam))
				{
				case ID_SELECTIONGROUPING_GROUP:
					if (!g_RenderSelectionGroupManager.AddGroup(CurrentCell, *g_TESRenderSelectionPrimary))
					{
						MessageBox(hWnd, "Couldn't add current selection to a new group.\n\nMake sure none of the selected objects belong to a pre-existing group",
									"CSE", MB_OK|MB_ICONEXCLAMATION);
					}
					else
						RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_2, "Created new selection group for current cell", 2);
					break;
				case ID_SELECTIONGROUPING_UNGROUP:
					if (!g_RenderSelectionGroupManager.RemoveGroup(CurrentCell, *g_TESRenderSelectionPrimary))
					{
						MessageBox(hWnd, "Couldn't remove current selection group.\n\nMake sure the selected objects belong to a pre-existing group",
									"CSE", MB_OK|MB_ICONEXCLAMATION);
					}
					else
						RENDERTEXT->QueueDrawTask(RenderWindowTextPainter::kRenderChannel_2, "Removed selection group from current cell", 2);
					break;
				}
			}
			break;
		case ID_ALIGNSELECTION_BYXAXIS:
		case ID_ALIGNSELECTION_BYYAXIS:
		case ID_ALIGNSELECTION_BYZAXIS:
			if ((*g_TESRenderSelectionPrimary)->SelectionCount > 1)
			{
		//		thisCall(kTESRenderUndoStack_RecordReference, *g_TESRenderUndoStack, TESRenderUndoStack::kUndoOperation_unk03, (*g_TESRenderSelectionPrimary)->RenderSelection);

				TESObjectREFR* AlignRef = CS_CAST((*g_TESRenderSelectionPrimary)->RenderSelection->Data, TESForm, TESObjectREFR);

				for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->RenderSelection->Next; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* ThisRef = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

					switch (LOWORD(wParam))
					{
					case ID_ALIGNSELECTION_BYXAXIS:
						ThisRef->posX = AlignRef->posX;
						break;
					case ID_ALIGNSELECTION_BYYAXIS:
						ThisRef->posY = AlignRef->posY;
						break;
					case ID_ALIGNSELECTION_BYZAXIS:
						ThisRef->posZ = AlignRef->posZ;
						break;
					}

					thisVirtualCall(*((UInt32*)ThisRef), 0x104, ThisRef);	// UpdateUsageInfo
					thisVirtualCall(*((UInt32*)ThisRef), 0x94, ThisRef, 1);	// SetFromActiveFile
					thisVirtualCall(*((UInt32*)ThisRef), 0xB8, ThisRef, ThisRef);
					thisVirtualCall(*((UInt32*)ThisRef), 0x17C, ThisRef, thisCall(kTESObjectREFR_GetExtraRef3DData, ThisRef));
				}

				PrintToBuffer("Selection aligned to %08X", AlignRef->refID);
				PrintToRender(g_TextBuffer, 2);
			}
			break;
		}
		break;
	}

	return CallWindowProc(g_RenderWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK AssetSelectorDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_FILEB:
			EndDialog(hWnd, e_FileBrowser);
			return TRUE;
		case BTN_BSAB:
			EndDialog(hWnd, e_BSABrowser);
			return TRUE;
		case BTN_EDITPATH:
			EndDialog(hWnd, e_EditPath);
			return TRUE;
		case BTN_COPYPATH:
			EndDialog(hWnd, e_CopyPath);
			return TRUE;
		case BTN_CLEARPATH:
			EndDialog(hWnd, e_ClearPath);
			return TRUE;
		case BTN_CANCEL:
			EndDialog(hWnd, e_Close);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
BOOL CALLBACK TextEditDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_OK:
			GetDlgItemText(hWnd, EDIT_TEXTLINE, g_TextBuffer, sizeof(g_TextBuffer));
			EndDialog(hWnd, (INT_PTR)g_TextBuffer);
			return TRUE;
		case BTN_CANCEL:
			EndDialog(hWnd, NULL);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		SetDlgItemText(hWnd, EDIT_TEXTLINE, (LPSTR)lParam);
		break;
	}
	return FALSE;
}
BOOL CALLBACK TESFileDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_ESP:
			EndDialog(hWnd, 0);
			return TRUE;
		case BTN_ESM:
			EndDialog(hWnd, 1);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL CALLBACK TESComboBoxDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND ComboBox = GetDlgItem(hWnd, COMBO_FORMLIST);

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_OK:
		{
			TESForm* SelectedForm = (TESForm*)TESDialog_GetSelectedItemData(ComboBox);
			EndDialog(hWnd, (INT_PTR)SelectedForm);
			return TRUE;
		}
		case BTN_CANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		TESDialog_ComboBoxPopulateWithForms(ComboBox, lParam, true, false);
		break;
	}
	return FALSE;
}

#define			MOUSEHOOK_CANCEL		0x9990
#define			MOUSEHOOK_COMPLETE		0x9991
HWND			g_MouseHookParentWindow = NULL;
HHOOK			g_MouseHookHandle = NULL;

LRESULT CALLBACK CopyPathMouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)  // do not process the message
		return CallNextHookEx(g_MouseHookHandle, nCode, wParam, lParam);

	switch (wParam)
	{
	case WM_RBUTTONUP:
		SendMessage(g_MouseHookParentWindow, MOUSEHOOK_COMPLETE, NULL, lParam);
		break;
	case WM_LBUTTONUP:
		SendMessage(g_MouseHookParentWindow, MOUSEHOOK_CANCEL, NULL, NULL);
		break;
	}

	return CallNextHookEx(g_MouseHookHandle, nCode, wParam, lParam);
}

// ### replace mouse hook with setcapture()
BOOL CALLBACK CopyPathDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case MOUSEHOOK_COMPLETE:
	case MOUSEHOOK_CANCEL:
		if (!UnhookWindowsHookEx(g_MouseHookHandle))
		{
			MessageBox(hWnd, "Couldn't deinitialize mouse hook. Error logged to the console.", "CSE", MB_OK|MB_ICONERROR);
			LogWinAPIErrorMessage(GetLastError());
		}
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		SetDlgItemText(hWnd, BTN_STARTMOUSEHOOK, "Click To Begin");
		g_MouseHookParentWindow = NULL;

		if (uMsg == MOUSEHOOK_COMPLETE)
		{
			MOUSEHOOKSTRUCT* Data = (MOUSEHOOKSTRUCT*)lParam;
			HWND Window = WindowFromPoint(Data->pt);
			if (Window)
			{
				GetWindowText(Window, g_TextBuffer, sizeof(g_TextBuffer));
				Edit_SetText(GetDlgItem(hWnd, EDIT_PATH), g_TextBuffer);
			}
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_STARTMOUSEHOOK:
		{
			if (g_MouseHookParentWindow == NULL)
			{
				g_MouseHookHandle = SetWindowsHookEx(WH_MOUSE, &CopyPathMouseHookProc, g_DLLInstance, NULL);
				if (g_MouseHookHandle == NULL)
				{
					MessageBox(hWnd, "Couldn't initialize mouse hook. Error logged to the console.", "CSE", MB_OK|MB_ICONERROR);
					LogWinAPIErrorMessage(GetLastError());
				}
				else
				{
					SetDlgItemText(hWnd, BTN_STARTMOUSEHOOK, "Right Click On The Button Displaying The New Path");
					g_MouseHookParentWindow = hWnd;
					SetCursor(LoadCursor(NULL, IDC_HAND));
				}
			}
			else
			{
				MessageBox(hWnd, "A previous copy operation was in progress. Try again.", "CSE", MB_OK);
			}
			break;
		}
		case BTN_OK:
		{
			char Buffer[0x200] = {0};

			GetDlgItemText(hWnd, EDIT_PATH, Buffer, sizeof(Buffer));
			switch ((int)GetWindowLong(hWnd, GWL_USERDATA))
			{
			case e_SPT:
				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "\\%s", Buffer);
				break;
			case e_KF:
				if (strlen(g_TextBuffer) > 27)
				{
					std::string Temp(Buffer); Temp = Temp.substr(26);
					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%s", Temp.c_str());
				}
				break;
			default:
				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%s", Buffer);
			}
			EndDialog(hWnd, (INT_PTR)g_TextBuffer);
			return TRUE;
		}
		case BTN_CANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);
		break;
	}
	return FALSE;
}

LRESULT CALLBACK ConsoleDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		tagRECT WindowRect, EditRect;
		GetWindowRect(hWnd, &WindowRect);
		MoveWindow(GetDlgItem(hWnd, EDIT_CONSOLE), 0, 0, WindowRect.right - WindowRect.left - 9, WindowRect.bottom - WindowRect.top - 50, TRUE);
		GetWindowRect(GetDlgItem(hWnd, EDIT_CONSOLE), &EditRect);
		SetWindowPos(GetDlgItem(hWnd, EDIT_CMDBOX), HWND_NOTOPMOST, 0, EditRect.bottom - EditRect.top, WindowRect.right - WindowRect.left - 10, 45, SWP_NOZORDER);
		break;
	}
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ConsoleWndOrgWindowProc);
		break;
	}

	return CallWindowProc(g_ConsoleWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ConsoleEditControlSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_TIMER:
		switch (wParam)
		{
		case CONSOLE_UPDATETIMER:
			if (CONSOLE->GetShouldUpdate())
			{
				SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);
				Edit_SetText(hWnd, (LPCSTR)CONSOLE->GetMessageBuffer());
				SendMessage(hWnd, WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);
				SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
			}
			SetTimer(hWnd, CONSOLE_UPDATETIMER, CONSOLE_UPDATEPERIOD, NULL);
			break;
		}
		return TRUE;
	case WM_RBUTTONUP:
	{
		static bool AlwaysOnTopFlag = false;

		RECT Rect;
		POINT Point;

		GetClientRect(hWnd, &Rect);
		Point.x = GET_X_LPARAM(lParam);
        Point.y = GET_Y_LPARAM(lParam);

		if (PtInRect((LPRECT) &Rect, Point)) {
			HMENU Popup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU1);
			Popup = GetSubMenu(Popup, 0);
			if (AlwaysOnTopFlag)	CheckMenuItem(Popup, 1, MF_CHECKED|MF_BYPOSITION);
			else					CheckMenuItem(Popup, 1, MF_UNCHECKED|MF_BYPOSITION);

			ClientToScreen(hWnd, (LPPOINT) &Point);

			switch (TrackPopupMenu(Popup, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, Point.x, Point.y, 0, hWnd, NULL))
			{
			case CONSOLEMENU_CLEARCONSOLE:
				CONSOLE->Clear();
				break;
			case CONSOLEMENU_ALWAYSONTOP:
				if (AlwaysOnTopFlag)
				{
					SetWindowPos(CONSOLE->GetWindowHandle(), HWND_NOTOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
					AlwaysOnTopFlag = false;
				}
				else
				{
					SetWindowPos(CONSOLE->GetWindowHandle(), HWND_TOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
					AlwaysOnTopFlag = true;
				}
				break;
			case CONSOLEMENU_HIDECONSOLE:
				CONSOLE->ToggleDisplayState();
				break;
			case CONSOLEMENU_OPENDEBUGLOG:
				ShellExecute(NULL, "open", (LPSTR)CONSOLE->GetDebugLogPath(), NULL, NULL, SW_SHOW);
				break;
			}
			DestroyMenu(Popup);
		}
		return FALSE;
	}
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ConsoleEditControlOrgWindowProc);
		break;
	}

	return CallWindowProc(g_ConsoleEditControlOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ConsoleCmdBoxSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static std::stack<std::string> CommandStack, AlternateCommandStack;

	switch (uMsg)
	{
	case CONSOLECMDBOX_RESETCOMMANDSTACK:
		while (AlternateCommandStack.empty() == false)
		{
			CommandStack.push(AlternateCommandStack.top());
			AlternateCommandStack.pop();
		}
		return TRUE;
	case CONSOLECMDBOX_CLEARCOMMANDSTACK:
		while (AlternateCommandStack.empty() == false)
			AlternateCommandStack.pop();
		while (CommandStack.empty() == false)
			CommandStack.pop();
		return TRUE;
	case WM_CHAR:
		if (wParam == VK_RETURN)
			return TRUE;
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_RETURN:
			{
				char Buffer[0x200];
				Edit_GetText(hWnd, Buffer, sizeof(Buffer));
				if (strlen(Buffer) > 2)
				{
					CONSOLE->LogMessage("CMD", Buffer);
					SendMessage(hWnd, CONSOLECMDBOX_RESETCOMMANDSTACK, NULL, NULL);
					CommandStack.push(Buffer);
				}
				Edit_SetText(hWnd, NULL);
				return TRUE;
			}
		case VK_UP:
			{
				if (CommandStack.empty() == false)
				{
					std::string Command(CommandStack.top());
					Edit_SetText(hWnd, Command.c_str());
					CommandStack.pop();
					AlternateCommandStack.push(Command);
				}
				else
					Edit_SetText(hWnd, NULL);

				return TRUE;
			}
		case VK_DOWN:
			{
				if (AlternateCommandStack.empty() == false)
				{
					std::string Command(AlternateCommandStack.top());
					Edit_SetText(hWnd, Command.c_str());
					AlternateCommandStack.pop();
					CommandStack.push(Command);
				}
				else
					Edit_SetText(hWnd, NULL);

				return TRUE;
			}
		}
		break;
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ConsoleCmdBoxOrgWindowProc);
		break;
	case WM_INITDIALOG:
		return TRUE;
	}

	return CallWindowProc(g_ConsoleCmdBoxOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

void EvaluatePopupMenuItems(HWND hWnd, int Identifier, TESForm* Form)
{
	switch (Identifier)
	{
		case POPUP_SETFORMID:
		{
			if (Form->refID < 0x800)	break;

			sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%08X", Form->refID);
			LPSTR FormIDString = (LPSTR)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TEXTEDIT), hWnd, (DLGPROC)TextEditDlgProc, (LPARAM)g_TextBuffer);
			if (FormIDString)
			{
				UInt32 FormID = 0;
				sscanf_s(FormIDString, "%08X", &FormID);
				if (errno == ERANGE || errno == EINVAL)
				{
					MessageBox(hWnd, "Bad FormID string - FormIDs should be unsigned 32-bit hex integers (e.g: 00503AB8)", "CSE", MB_OK);
					break;
				}
				else if ((FormID & 0x00FFFFFF) < 0x800)
				{
					MessageBox(hWnd, "Invalid FormID - Base should be at least 0x800", "CSE", MB_OK);
					break;
				}

				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Change FormID from %08X to %08X ?\n\nMod index bits will be automatically corrected by the CS when saving.\nCheck the console for formID bashing on confirmation.", Form->refID, FormID);
				if (MessageBox(hWnd, g_TextBuffer, "CSE", MB_YESNO) == IDYES)
				{
					thisCall(kTESForm_SetFormID, Form, (UInt32)FormID, true);
					thisVirtualCall(*((UInt32*)Form), 0x94, Form, 1);		// SetFromActiveFile
				}
			}
			break;
		}
		case POPUP_MARKUNMODIFIED:
			sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Are you sure you want to mark form '%s' (%08X) as unmodified ?\n\nThis will not revert any changes made to it.", Form->editorData.editorID.m_data, Form->refID);
			if (MessageBox(hWnd, g_TextBuffer, "CSE", MB_YESNO) == IDYES)
			{
				thisVirtualCall(*((UInt32*)Form), 0x94, Form, 0);
			}
			break;
		case POPUP_JUMPTOUSEINFOLIST:
		{
			const char* EditorID = Form->editorData.editorID.m_data;

			if (EditorID)
				CLIWrapper::UseInfoList::OpenUseInfoBox(EditorID);
			else
			{
				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%08X", Form->refID);
				CLIWrapper::UseInfoList::OpenUseInfoBox(g_TextBuffer);
			}
			break;
		}
		case POPUP_UNDELETE:
		{
			PrintToBuffer("Are you sure you want to undelete form '%s' (%08X) ?\n\nOld references to it will not be restored.", Form->editorData.editorID.m_data, Form->refID);
			if (MessageBox(hWnd, g_TextBuffer, "CSE", MB_YESNO) == IDYES)
			{
				thisVirtualCall(*((UInt32*)Form), 0x90, Form, 0);		// SetDeleted
			}
			break;
		}
		case POPUP_EDITBASEFORM:
		{
			TESForm* BaseForm = (CS_CAST(Form, TESForm, TESObjectREFR))->baseForm;
			if (BaseForm && BaseForm->editorData.editorID.m_data)
			{
				ShowFormEditDialog(BaseForm->editorData.editorID.m_data, BaseForm->typeID);
			}
			break;
		}
		case POPUP_ADDTOTAG:
		{
			g_FormData->FillFormData(Form);
			CLIWrapper::TagBrowser::AddFormToActiveTag(g_FormData);
			break;
		}
		case POPUP_TOGGLEVISIBILITY:
		{
			TESObjectREFR* Ref = CS_CAST(Form, TESForm, TESObjectREFR);
			ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_3DInvisible, !(Ref->flags & kTESObjectREFRSpecialFlags_3DInvisible));
			break;
		}
		case POPUP_TOGGLECHILDRENVISIBILITY:
		{
			TESObjectREFR* Ref = CS_CAST(Form, TESForm, TESObjectREFR);
			ToggleFlag(&Ref->flags, kTESObjectREFRSpecialFlags_Children3DInvisible, !(Ref->flags & kTESObjectREFRSpecialFlags_Children3DInvisible));
			break;
		}
		case POPUP_SHOWOVERRIDES:
		{
			std::string Buffer = PrintToBuffer("Override list for form '%08X':\n\n", Form->refID);
			for (TESForm::ModReferenceList* Itr = &Form->modRefList; Itr && Itr->data; Itr = Itr->next)
			{
				TESFile* File = (TESFile*)Itr->data;
				Buffer += PrintToBuffer("\t%s\n", File->name);
			}
			MessageBox(*g_HWND_CSParent, Buffer.c_str(), "CSE", MB_OK|MB_ICONINFORMATION);
			break;
		}
	}
	UpdateWindow(hWnd);
}

LRESULT CALLBACK ObjectWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		break;
	}

	return CallWindowProc(g_ObjectWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CellViewWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND* s_CellViewListViewUnderCursor = (HWND*)0x00A0A9E4;

	switch (uMsg)
	{
	case WM_COMMAND:
		break;
	}

	return CallWindowProc(g_CellViewWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK ResponseWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND VoiceList = GetDlgItem(hWnd, 2168);

	switch (uMsg)
	{
	case WM_COMMAND:
	{
		int SelectedItem = ListView_GetNextItem(VoiceList, -1, LVNI_SELECTED);

		char VoicePath[0x200] = {0};
		LVITEM SelectedVoiceItem;

		SelectedVoiceItem.iItem = SelectedItem;
		SelectedVoiceItem.iSubItem = 6;
		SelectedVoiceItem.mask = LVIF_TEXT;
		SelectedVoiceItem.pszText = VoicePath;
		SelectedVoiceItem.cchTextMax = sizeof(VoicePath);

		switch (LOWORD(wParam))
		{
		case 2223:					// Copy external file
		{
			if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
				return FALSE;

			char FilePath[0x200] = {0};
			OPENFILENAME SelectFile;
			SelectFile.lStructSize = sizeof(OPENFILENAME);
			SelectFile.hwndOwner = hWnd;
			SelectFile.lpstrFilter = "MP3 Files\0*.mp3\0\0";
			SelectFile.lpstrCustomFilter = NULL;
			SelectFile.nFilterIndex = 0;
			SelectFile.lpstrFile = FilePath;
			SelectFile.nMaxFile = sizeof(FilePath);
			SelectFile.lpstrFileTitle = NULL;
			SelectFile.lpstrInitialDir = NULL;
			SelectFile.lpstrTitle = "Select an audio file to use as the current response's voice";
			SelectFile.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR;

			if (GetOpenFileName(&SelectFile))
			{
				std::string Destination(g_AppPath + "\\" + std::string(VoicePath));
				if (!CopyFile(FilePath, Destination.c_str(), TRUE))
				{
					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Couldn't copy external file '%s' to '%s'!\n\nCheck the console for more information.", FilePath, Destination.c_str());
					LogWinAPIErrorMessage(GetLastError());
				}
				else
					DebugPrint("Copied external audio file '%s' to '%s'", FilePath, Destination.c_str());
			}
			return FALSE;
		}
		case 1016:					// Generate LIP
			if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
				return FALSE;

			std::string Path(VoicePath);
			Path = Path.substr(0, Path.find_last_of("."));

			if (!CSIOM->DoGenerateLIPOperation(Path.c_str(), (*g_ResponseEditorData)->responseLocalCopy->responseText.m_data))
			{
				MessageBox(hWnd, "Couldn't generate LIP file for the selected voice.\n\nCheck the console for more information.", "CSE", MB_OK);
			}
			else
				DebugPrint("Successfully generated LIP file for the selected voice");

			return FALSE;
		}
		break;
	}
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ResponseWndOrgWindowProc);
		break;
	}

	return CallWindowProc(g_ResponseWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GlobalScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_OK:
			{
				char QuestID[0x200] = {0};
				char ScriptID[0x200] = {0};
				char Delay[8] = {0};

				GetDlgItemText(hWnd, EDIT_QUESTID, QuestID, sizeof(QuestID));
				GetDlgItemText(hWnd, EDIT_SCRIPTID, ScriptID, sizeof(ScriptID));
				GetDlgItemText(hWnd, EDIT_DELAY, Delay, sizeof(Delay));

				TESForm* Form = NULL;
				TESQuest* Quest = NULL;
				Script* QuestScript = NULL;

				Form = GetFormByID(QuestID);
				if (Form)
				{
					if (Form->typeID == kFormType_Quest)
					{
						sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Quest '%s' already exists. Do you want to replace its script with a newly created one ?", QuestID);
						if (MessageBox(hWnd, g_TextBuffer, "CSE", MB_YESNO) != IDYES)
							return TRUE;
					}
					else
					{
						sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "EditorID '%s' is already in use.", QuestID);
						MessageBox(hWnd, g_TextBuffer, "CSE", MB_OK|MB_ICONEXCLAMATION);
						return TRUE;
					}

					Quest = CS_CAST(Form, TESForm, TESQuest);
				}
				else
				{
					Quest = (TESQuest*)FormHeap_Allocate(0x74);
					thisCall(kTESQuest_Ctor, Quest);
					thisVirtualCall(kVTBL_TESQuest, 0x94, Quest);
					thisCall(kLinkedListNode_NewNode, &(*g_dataHandler)->quests, Quest);
					thisCall(kTESForm_SetEditorID, Quest, QuestID);
				}

				if (strlen(ScriptID) < 1)
					sprintf_s(ScriptID, sizeof(ScriptID), "%sScript", QuestID);

				Form = GetFormByID(ScriptID);
				if (Form)
				{
					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "EditorID '%s' is already in use.", ScriptID);
					MessageBox(hWnd, g_TextBuffer, "CSE", MB_OK|MB_ICONEXCLAMATION);
					return TRUE;
				}
				else
				{
					QuestScript = (Script*)FormHeap_Allocate(0x54);
					thisCall(kScript_Ctor, QuestScript);
					thisVirtualCall(kVTBL_Script, 0x94, QuestScript);
					thisCall(kLinkedListNode_NewNode, &(*g_dataHandler)->scripts, QuestScript);
					thisCall(kDataHandler_SortScripts, (*g_dataHandler));
					QuestScript->info.type = Script::eType_Quest;

					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "scn %s\n\nfloat fQuestDelayTime\n\nBegin GameMode\n\tlet fQuestDelayTime := %s\n\nend", ScriptID, Delay);
					thisCall(kScript_SetText, QuestScript, g_TextBuffer);
					thisCall(kTESForm_SetEditorID, QuestScript, ScriptID);
				}

				Quest->scriptable.script = QuestScript;

				thisVirtualCall(kVTBL_TESQuest, 0x70, Quest);
				thisVirtualCall(kVTBL_Script, 0x70, QuestScript);

				thisVirtualCall(kVTBL_TESQuest, 0x104, Quest);
				thisVirtualCall(kVTBL_Script, 0x104, QuestScript);
				thisCall(kTESForm_AddReference, QuestScript, Quest);

				MessageBox(hWnd, "Global script created successfully.\n\nIt will now be opened for editing ...", "CSE", MB_OK|MB_ICONINFORMATION);
				InstantitateCustomScriptEditor(ScriptID);

				DestroyWindow(hWnd);
				return TRUE;
			}
		case BTN_CANCEL:
			DestroyWindow(hWnd);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL CALLBACK BindScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND EditorIDBox = GetDlgItem(hWnd, EDIT_EDITORID);
	HWND RefIDBox = GetDlgItem(hWnd, EDIT_REFEDITORID);
	HWND ExistFormList = GetDlgItem(hWnd, COMBO_FORMLIST);
	HWND SelParentCellBtn = GetDlgItem(hWnd, BTN_PARENTCELL);

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_PARENTCELL:
		{
			TESForm* Selection = (TESForm*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TESCOMBOBOX), hWnd, (DLGPROC)TESComboBoxDlgProc, (LPARAM)kFormType_Cell);
			if (Selection)
			{
				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%s (%08X)", Selection->editorData.editorID.m_data, Selection->refID);
				SetWindowText(SelParentCellBtn, (LPCSTR)g_TextBuffer);
				SetWindowLong(SelParentCellBtn, GWL_USERDATA, (LONG)Selection);
			}
			break;
		}
		case BTN_OK:
		{
			if (IsDlgButtonChecked(hWnd, RADIO_EXISTFORM))
			{
				TESForm* SelectedForm = (TESForm*)TESDialog_GetSelectedItemData(ExistFormList);
				if (SelectedForm)
				{
					EndDialog(hWnd, (INT_PTR)SelectedForm);
					return TRUE;
				}
				else
					MessageBox(hWnd, "Invalid existing form selected", "CSE", MB_OK|MB_ICONERROR);
			}
			else
			{
				char BaseEditorID[0x200] = {0};
				char RefEditorID[0x200] = {0};

				Edit_GetText(EditorIDBox, BaseEditorID, 0x200);
				if (GetFormByID(BaseEditorID))
				{
					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "EditorID '%s' is already in use.", BaseEditorID);
					MessageBox(hWnd, g_TextBuffer, "CSE", MB_OK|MB_ICONERROR);
				}
				else
				{
					if (IsDlgButtonChecked(hWnd, RADIO_QUEST))
					{
						bool StartGameEnabledFlag = IsDlgButtonChecked(hWnd, CHECK_QUEST_STARTGAMEENABLED);
						bool RepeatedStagesFlag = IsDlgButtonChecked(hWnd, CHECK_QUEST_REPEATEDSTAGES);

						TESQuest* Quest = (TESQuest*)FormHeap_Allocate(0x74);
						thisCall(kTESQuest_Ctor, Quest);
						thisVirtualCall(kVTBL_TESQuest, 0x94, Quest);
						thisCall(kLinkedListNode_NewNode, &(*g_dataHandler)->quests, Quest);
						thisCall(kTESForm_SetEditorID, Quest, BaseEditorID);

						thisCall(kTESQuest_SetStartEnabled, Quest, StartGameEnabledFlag);
						thisCall(kTESQuest_SetAllowedRepeatedStages, Quest, RepeatedStagesFlag);

						FormEnumerationWrapper::ReinitializeFormLists();
						EndDialog(hWnd, (INT_PTR)Quest);
						return TRUE;
					}
					else
					{
						if (IsDlgButtonChecked(hWnd, RADIO_OBJECTTOKEN))
						{
							bool QuestItem = IsDlgButtonChecked(hWnd, CHECK_OBJECTTOKEN_QUESTITEM);

							TESObjectCLOT* Token = (TESObjectCLOT*)FormHeap_Allocate(0x158);
							thisCall(kTESObjectCLOT_Ctor, Token);
							thisVirtualCall(kVTBL_TESObjectCLOT, 0x94, Token);
							thisCall(kDataHandler_AddBoundObject, (*g_dataHandler)->boundObjects, Token);
							thisCall(kTESForm_SetEditorID, Token, BaseEditorID);

							ToggleFlag((UInt16*)((UInt32)Token + 0x96), 1 << TESBipedModelForm::kFlags_NotPlayable, true);
							ToggleFlag(&Token->flags, TESForm::kFormFlags_Essential, QuestItem);

							FormEnumerationWrapper::ReinitializeFormLists();
							EndDialog(hWnd, (INT_PTR)Token);
							return TRUE;
						}
						else
						{
							Edit_GetText(RefIDBox, RefEditorID, 0x200);
							if (GetFormByID(RefEditorID))
							{
								sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "EditorID '%s' is already in use.", RefEditorID);
								MessageBox(hWnd, g_TextBuffer, "CSE", MB_OK|MB_ICONERROR);
							}
							else
							{
								bool InitiallyDisabled = IsDlgButtonChecked(hWnd, CHECK_OBJECTREFERENCE_DISABLED);
								TESForm* ParentCell = (TESForm*)GetWindowLong(SelParentCellBtn, GWL_USERDATA);

								if (!ParentCell || thisCall(kTESObjectCELL_GetIsInterior, ParentCell) == 0)
								{
									MessageBox(hWnd, "Invalid/exterior cell selected as parent.", "CSE", MB_OK|MB_ICONERROR);
								}
								else
								{
									TESObjectACTI* Activator = (TESObjectACTI*)FormHeap_Allocate(0x98);
									thisCall(kTESObjectACTI_Ctor, Activator);
									thisVirtualCall(kVTBL_TESObjectACTI, 0x94, Activator);
									thisCall(kDataHandler_AddBoundObject, (*g_dataHandler)->boundObjects, Activator);
									thisCall(kTESForm_SetEditorID, Activator, BaseEditorID);

									static NiVector3 ZeroVector = { 0.0, 0.0, 0.0 };

									TESObjectCELL* CurrentInteriorCell = TES::GetSingleton()->currentInteriorCell;	// kludgy, but should do the trick
									TES::GetSingleton()->currentInteriorCell = CS_CAST(ParentCell, TESForm, TESObjectCELL);
									TESObjectREFR* Ref = DataHandler_PlaceTESBoundObjectReference(Activator, &ZeroVector, &ZeroVector, 1);
									TES::GetSingleton()->currentInteriorCell = CurrentInteriorCell;

									ToggleFlag(&Ref->flags, TESForm::kFormFlags_InitiallyDisabled, InitiallyDisabled);
									ToggleFlag(&Ref->flags, TESForm::kFormFlags_QuestItem, true);

									thisCall(kTESForm_SetEditorID, Ref, RefEditorID);

									FormEnumerationWrapper::ReinitializeFormLists();
									EndDialog(hWnd, (INT_PTR)Activator);
									return TRUE;
								}
							}
						}
					}
				}
			}

			return FALSE;
		}
		case BTN_CANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		Edit_SetText(EditorIDBox, "Base Form EditorID");
		Edit_SetText(RefIDBox, "Ref EditorID");

		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Activator, true, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Apparatus, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Armor, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Book, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Clothing, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Container, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Door, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Ingredient, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Light, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Misc, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Furniture, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Weapon, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Ammo, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_NPC, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Creature, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_SoulGem, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Key, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_AlchemyItem, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_SigilStone, false, false);
		TESDialog_ComboBoxPopulateWithForms(ExistFormList, kFormType_Quest, false, false);

		CheckDlgButton(hWnd, RADIO_EXISTFORM, BST_CHECKED);
		CheckDlgButton(hWnd, RADIO_QUEST, BST_CHECKED);
		CheckDlgButton(hWnd, RADIO_OBJECTTOKEN, BST_CHECKED);

		SetWindowLong(SelParentCellBtn, GWL_USERDATA, (LONG)0);

		break;
	}
	return FALSE;
}

LRESULT CALLBACK TagBrowserSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case 0x407:			// Form Drag-Drop Notification
		for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->RenderSelection; Itr && Itr->Data; Itr = Itr->Next)
		{
			TESForm* Form = Itr->Data;
			if (GetFormDialogTemplate(Form->typeID) == 1)
			{
				g_FormData->FillFormData(Form);
				CLIWrapper::TagBrowser::AddFormToActiveTag(g_FormData);
			}
		}

		thisCall(kTESRenderSelection_ClearSelection, *g_TESRenderSelectionPrimary, 0);
		return TRUE;
	}

	return CallWindowProc(g_TagBrowserOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

#define MANAGETOOLS_CLEAREDITFIELD		99999

BOOL CALLBACK ManageToolsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND ToolList = GetDlgItem(hWnd, LISTBOX_TOOLLIST);
	HWND TitleBox =  GetDlgItem(hWnd, EDIT_TITLE);
	HWND CmdLineBox =  GetDlgItem(hWnd, EDIT_CMDLINE);
	HWND InitDirBox =  GetDlgItem(hWnd, EDIT_INITDIR);

	static ToolManager::Tool* s_ToolBuffer = NULL;

	switch (uMsg)
	{
	case MANAGETOOLS_CLEAREDITFIELD:
		SetWindowText(TitleBox, NULL);
		SetWindowText(CmdLineBox, NULL);
		SetWindowText(InitDirBox, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_CLOSE:
			SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(LISTBOX_TOOLLIST, LBN_SELCHANGE), (LPARAM)ToolList);
			g_ToolManager.ClearToolList(false);
			for (int i = 0, j = SendMessage(ToolList, LB_GETCOUNT, NULL, NULL); i < j; i++)
			{
				ToolManager::Tool* Tool = (ToolManager::Tool*)SendMessage(ToolList, LB_GETITEMDATA, i, NULL);
				g_ToolManager.AddTool(Tool);
			}

			g_ToolManager.ReloadToolsMenu();
			DebugPrint("Updated Tool List");

			EndDialog(hWnd, 0);
			return TRUE;
		case BTN_ADDTOOL:
			{
				char TitleBuffer[0x200] = {0}, CmdLineBuffer[MAX_PATH] = {0}, InitDirBuffer[MAX_PATH] = {0};
				GetWindowText(TitleBox, (LPSTR)TitleBuffer, 0x200);
				GetWindowText(CmdLineBox, (LPSTR)CmdLineBuffer, MAX_PATH);
				GetWindowText(InitDirBox, (LPSTR)InitDirBuffer, MAX_PATH);

				if ((strlen(TitleBuffer) < 1 || strlen(CmdLineBuffer) < 1 || strlen(InitDirBuffer) < 1) ||
					(strstr(TitleBuffer, "=") || strstr(CmdLineBuffer, "=") || strstr(InitDirBuffer, "=")) ||
					(strstr(TitleBuffer, "|") || strstr(CmdLineBuffer, "|") || strstr(InitDirBuffer, "|")))
				{
					MessageBox(hWnd, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.", "CSE", MB_OK|MB_ICONEXCLAMATION);
					break;
				}

				ToolManager::Tool* NewTool = g_ToolManager.AddTool(TitleBuffer, CmdLineBuffer, InitDirBuffer);
				if (NewTool == NULL)
				{
					MessageBox(hWnd, "Enter a unique tool title", "CSE", MB_OK|MB_ICONEXCLAMATION);
					break;
				}

				int Index = SendMessage(ToolList, LB_INSERTSTRING, -1, (LPARAM)NewTool->GetTitle());
				SendMessage(ToolList, LB_SETITEMDATA, Index, (LPARAM)NewTool);
			}
			break;
		case BTN_REMOVETOOL:
			{
				int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
				if (Index != LB_ERR)
				{
					if (SendMessage(ToolList, LB_GETTEXT, Index, (LPARAM)g_TextBuffer) != LB_ERR)
					{
						g_ToolManager.RemoveTool(g_TextBuffer, true);
						SendMessage(ToolList, LB_DELETESTRING, Index, NULL);
						SendMessage(hWnd, MANAGETOOLS_CLEAREDITFIELD, NULL, NULL);
						SendMessage(ToolList, LB_SETSEL, FALSE, -1);
					}
				}
			}
			break;
		case BTN_MOVEDOWN:
		case BTN_MOVEUP:
			{
				int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
				if (Index != LB_ERR)
				{
					SendMessage(ToolList, LB_GETTEXT, Index, (LPARAM)g_TextBuffer);
					ToolManager::Tool* Tool = (ToolManager::Tool*)SendMessage(ToolList, LB_GETITEMDATA, Index, NULL);

					int NewIndex = 0;
					if (LOWORD(wParam) == BTN_MOVEDOWN)
						NewIndex = Index + 1;
					else
						NewIndex = Index - 1;

					if (NewIndex < 0)
						NewIndex = 0;
					else if (NewIndex == SendMessage(ToolList, LB_GETCOUNT, NULL, NULL))
						NewIndex--;

					SendMessage(ToolList, LB_DELETESTRING, Index, NULL);
					SendMessage(ToolList, LB_INSERTSTRING, NewIndex, (LPARAM)g_TextBuffer);
					SendMessage(ToolList, LB_SETITEMDATA, NewIndex, (LPARAM)Tool);
					SendMessage(ToolList, LB_SETSEL, TRUE, NewIndex);
					SetFocus(ToolList);
				}
			}
			break;
		case BTN_SELECTCMDLINE:
			{
				char FilePath[MAX_PATH] = {0};

				OPENFILENAME SelectFile;
				SelectFile.lStructSize = sizeof(OPENFILENAME);
				SelectFile.hwndOwner = hWnd;
				SelectFile.lpstrFilter = "All Files\0*.*\0\0";
				SelectFile.lpstrCustomFilter = NULL;
				SelectFile.nFilterIndex = 0;
				SelectFile.lpstrFile = FilePath;
				SelectFile.nMaxFile = sizeof(FilePath);
				SelectFile.lpstrFileTitle = NULL;
				SelectFile.lpstrInitialDir = NULL;
				SelectFile.lpstrTitle = "Select a file";
				SelectFile.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_NOCHANGEDIR;

				if (GetOpenFileName(&SelectFile))
				{
					SetWindowText(CmdLineBox, (LPSTR)FilePath);
				}
			}
			break;
		case BTN_SELECTINITDIR:
			{
				char FolderPath[MAX_PATH] = {0};

				BROWSEINFO FolderInfo;
				FolderInfo.hwndOwner = hWnd;
				FolderInfo.iImage = NULL;
				FolderInfo.pszDisplayName = FolderPath;
				FolderInfo.lpszTitle = "Select an initial directory for the tool";
				FolderInfo.ulFlags = BIF_NEWDIALOGSTYLE|BIF_RETURNONLYFSDIRS;
				FolderInfo.pidlRoot = NULL;
				FolderInfo.lpfn = NULL;
				FolderInfo.lParam = NULL;

				PIDLIST_ABSOLUTE ReturnPath = SHBrowseForFolder(&FolderInfo);
				if (ReturnPath)
				{
					if (SHGetPathFromIDList(ReturnPath, FolderPath))
					{
						SetWindowText(InitDirBox, (LPSTR)FolderPath);
					}
				}
			}
			break;
		case LISTBOX_TOOLLIST:
			{
				switch (HIWORD(wParam))
				{
				case LBN_SELCHANGE:
					if (s_ToolBuffer)
					{
						char CmdLineBuffer[MAX_PATH] = {0}, InitDirBuffer[MAX_PATH] = {0};
						GetWindowText(CmdLineBox, (LPSTR)CmdLineBuffer, MAX_PATH);
						GetWindowText(InitDirBox, (LPSTR)InitDirBuffer, MAX_PATH);

						if (strlen(CmdLineBuffer) > 1 && strlen(InitDirBuffer) > 1)
						{
							if ((strstr(CmdLineBuffer, "=") || strstr(InitDirBuffer, "=")) ||
								(strstr(CmdLineBuffer, "|") || strstr(InitDirBuffer, "|")))
							{
								MessageBox(hWnd, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.", "CSE", MB_OK|MB_ICONEXCLAMATION);
								break;
							}

							s_ToolBuffer->SetCommandLine(CmdLineBuffer);
							s_ToolBuffer->SetInitialDir(InitDirBuffer);
						}
					}

					int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
					if (Index != LB_ERR)
					{
						ToolManager::Tool* Tool = (ToolManager::Tool*)SendMessage(ToolList, LB_GETITEMDATA, Index, NULL);
						SetWindowText(TitleBox, (LPSTR)Tool->GetTitle());
						SetWindowText(CmdLineBox, (LPSTR)Tool->GetCommandLine());
						SetWindowText(InitDirBox, (LPSTR)Tool->GetInitialDir());
						s_ToolBuffer = Tool;
					}
					else
						SendMessage(hWnd, MANAGETOOLS_CLEAREDITFIELD, NULL, NULL);
					break;
				}
			}
			break;
		}
		break;
	case WM_INITDIALOG:
		g_ToolManager.PopulateListBoxWithTools(ToolList);
		break;
	}
	return FALSE;
}

void InitializeWindowManager(void)
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent),
		  GameplayMenu = GetSubMenu(MainMenu, 5),
		  ViewMenu = GetSubMenu(MainMenu, 2),
		  FileMenu = GetSubMenu(MainMenu, 0),
		  WorldMenu = GetSubMenu(MainMenu, 3);

	MENUITEMINFO ItemGameplayUseInfo,
				ItemViewRenderWindow,
				ItemDataSaveAs,
				ItemWorldBatchEdit,
				ItemViewConsole,
				ItemViewModifiedRecords,
				ItemFileCSEPreferences,
				ItemViewDeletedRecords,
				ItemWorldUnloadCell,
				ItemGameplayGlobalScript,
				ItemLaunchGame,
				ItemViewTagBrowser,
				ItemDataSetWorkspace,
				ItemFileSaveESPMasters;
	ItemGameplayUseInfo.cbSize = sizeof(MENUITEMINFO);
	ItemGameplayUseInfo.fMask = MIIM_STRING;
	ItemGameplayUseInfo.dwTypeData = "Use Info Listings";
	ItemGameplayUseInfo.cch = 15;
	SetMenuItemInfo(GameplayMenu, 245, FALSE, &ItemGameplayUseInfo);

	ItemViewRenderWindow.cbSize = sizeof(MENUITEMINFO);		// the tool coder seems to have mixed up the controlID for the button
	ItemViewRenderWindow.fMask = MIIM_ID|MIIM_STATE;		// as the code to handle hiding/showing is already present in the wndproc
	ItemViewRenderWindow.wID = 40423;						// therefore we simply change it to the one that's expected by the proc
	ItemViewRenderWindow.fState = MFS_CHECKED;
	SetMenuItemInfo(ViewMenu, 40198, FALSE, &ItemViewRenderWindow);

	ItemDataSaveAs.cbSize = sizeof(MENUITEMINFO);
	ItemDataSaveAs.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemDataSaveAs.wID = MAIN_DATA_SAVEAS;
	ItemDataSaveAs.fState = MFS_ENABLED;
	ItemDataSaveAs.dwTypeData = "Save As";
	ItemDataSaveAs.cch = 7;
	InsertMenuItem(FileMenu, 40127, FALSE, &ItemDataSaveAs);

	ItemWorldBatchEdit.cbSize = sizeof(MENUITEMINFO);
	ItemWorldBatchEdit.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemWorldBatchEdit.wID = MAIN_WORLD_BATCHEDIT;
	ItemWorldBatchEdit.fState = MFS_ENABLED;
	ItemWorldBatchEdit.dwTypeData = "Batch Edit References";
	ItemWorldBatchEdit.cch = 0;
	InsertMenuItem(WorldMenu, 40194, FALSE, &ItemWorldBatchEdit);
	InsertMenuItem(*g_RenderWindowPopup, 293, FALSE, &ItemWorldBatchEdit);

	ItemViewConsole.cbSize = sizeof(MENUITEMINFO);
	ItemViewConsole.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemViewConsole.wID = MAIN_VIEW_CONSOLEWINDOW;
	ItemViewConsole.fState = MFS_ENABLED|MFS_CHECKED;
	ItemViewConsole.dwTypeData = "Console Window";
	ItemViewConsole.cch = 0;
	InsertMenuItem(ViewMenu, 40455, FALSE, &ItemViewConsole);

	ItemViewModifiedRecords.cbSize = sizeof(MENUITEMINFO);
	ItemViewModifiedRecords.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemViewModifiedRecords.wID = MAIN_VIEW_MODIFIEDRECORDS;
	ItemViewModifiedRecords.fState = MFS_ENABLED|MFS_UNCHECKED;
	ItemViewModifiedRecords.dwTypeData = "Hide Unmodified Forms";
	ItemViewModifiedRecords.cch = 0;
	InsertMenuItem(ViewMenu, 40030, FALSE, &ItemViewModifiedRecords);

	ItemFileCSEPreferences.cbSize = sizeof(MENUITEMINFO);
	ItemFileCSEPreferences.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemFileCSEPreferences.wID = MAIN_DATA_CSEPREFERENCES;
	ItemFileCSEPreferences.fState = MFS_ENABLED;
	ItemFileCSEPreferences.dwTypeData = "CSE Preferences";
	ItemFileCSEPreferences.cch = 0;
	InsertMenuItem(FileMenu, 40003, FALSE, &ItemFileCSEPreferences);

	ItemViewDeletedRecords.cbSize = sizeof(MENUITEMINFO);
	ItemViewDeletedRecords.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemViewDeletedRecords.wID = MAIN_VIEW_DELETEDRECORDS;
	ItemViewDeletedRecords.fState = MFS_ENABLED|MFS_UNCHECKED;
	ItemViewDeletedRecords.dwTypeData = "Hide Deleted Forms";
	ItemViewDeletedRecords.cch = 0;
	InsertMenuItem(ViewMenu, 40030, FALSE, &ItemViewDeletedRecords);

	ItemWorldUnloadCell.cbSize = sizeof(MENUITEMINFO);
	ItemWorldUnloadCell.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemWorldUnloadCell.wID = MAIN_WORLD_UNLOADCELL;
	ItemWorldUnloadCell.fState = MFS_ENABLED;
	ItemWorldUnloadCell.dwTypeData = "Unload Current Cell";
	ItemWorldUnloadCell.cch = 0;
	InsertMenuItem(WorldMenu, 40426, FALSE, &ItemWorldUnloadCell);

	ItemGameplayGlobalScript.cbSize = sizeof(MENUITEMINFO);
	ItemGameplayGlobalScript.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemGameplayGlobalScript.wID = MAIN_GAMEPLAY_GLOBALSCRIPT;
	ItemGameplayGlobalScript.fState = MFS_ENABLED;
	ItemGameplayGlobalScript.dwTypeData = "Create Global Script";
	ItemGameplayGlobalScript.cch = 0;
	InsertMenuItem(GameplayMenu, 40167, FALSE, &ItemGameplayGlobalScript);

	ItemLaunchGame.cbSize = sizeof(MENUITEMINFO);
	ItemLaunchGame.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemLaunchGame.wID = MAIN_LAUNCHGAME;
	ItemLaunchGame.fState = MFS_ENABLED;
	ItemLaunchGame.dwTypeData = "Launch Game";
	ItemLaunchGame.cch = 0;
	InsertMenuItem(MainMenu, -1, FALSE, &ItemLaunchGame);

	ItemViewTagBrowser.cbSize = sizeof(MENUITEMINFO);
	ItemViewTagBrowser.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemViewTagBrowser.wID = MAIN_VIEW_TAGBROWSER;
	ItemViewTagBrowser.fState = MFS_ENABLED;
	ItemViewTagBrowser.dwTypeData = "Tag Browser";
	ItemViewTagBrowser.cch = 0;
	InsertMenuItem(ViewMenu, 40455, FALSE, &ItemViewTagBrowser);

	ItemDataSetWorkspace.cbSize = sizeof(MENUITEMINFO);
	ItemDataSetWorkspace.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemDataSetWorkspace.wID = MAIN_DATA_SETWORKSPACE;
	ItemDataSetWorkspace.fState = MFS_ENABLED;
	ItemDataSetWorkspace.dwTypeData = "Set Workspace";
	ItemDataSetWorkspace.cch = 0;
	InsertMenuItem(FileMenu, 40003, FALSE, &ItemDataSetWorkspace);

	ItemFileSaveESPMasters.cbSize = sizeof(MENUITEMINFO);
	ItemFileSaveESPMasters.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemFileSaveESPMasters.wID = MAIN_DATA_SAVEESPMASTERS;
	if (g_INIManager->GetINIInt("SaveLoadedESPsAsMasters"))
		ItemFileSaveESPMasters.fState = MFS_ENABLED|MFS_CHECKED;
	else
		ItemFileSaveESPMasters.fState = MFS_ENABLED|MFS_UNCHECKED;
	ItemFileSaveESPMasters.dwTypeData = "Save ESPs as Master Files";
	ItemFileSaveESPMasters.cch = 0;
	InsertMenuItem(FileMenu, MAIN_DATA_SAVEAS, FALSE, &ItemFileSaveESPMasters);

	HMENU AlignMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU2); AlignMenuPopup = GetSubMenu(AlignMenuPopup, 0);
	HMENU GroupMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU3); GroupMenuPopup = GetSubMenu(GroupMenuPopup, 0);
	HMENU FreezeMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU4); FreezeMenuPopup = GetSubMenu(FreezeMenuPopup, 0);
	HMENU VisibilityMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU5); VisibilityMenuPopup = GetSubMenu(VisibilityMenuPopup, 0);

	InsertMenu(*g_RenderWindowPopup, -1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)AlignMenuPopup, "Selection Alignment");
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)GroupMenuPopup, "Selection Grouping");
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)FreezeMenuPopup, "Selection Freezing");
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)VisibilityMenuPopup, "Selection Visibility");

	DrawMenuBar(*g_HWND_CSParent);

	g_RenderWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_RenderWindow, GWL_WNDPROC, (LONG)RenderWndSubClassProc);
	g_CSMainWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_CSParent, GWL_WNDPROC, (LONG)CSMainWndSubClassProc);

	g_CSDefaultFont = CreateFont(12, 5.5, 0, 0,
                             FW_THIN, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             FF_DONTCARE, "MS Shell Dlg");

	SetTimer(*g_HWND_RenderWindow, 1, g_INIManager->GetINIInt("UpdatePeriod"), NULL);

	g_TagBrowserOrgWindowProc = (WNDPROC)SetWindowLong(CLIWrapper::TagBrowser::GetFormDropParentHandle(), GWL_WNDPROC, (LONG)TagBrowserSubClassProc);

	g_DragDropSupportDialogs.AddHandle(CLIWrapper::TagBrowser::GetFormDropWindowHandle());
}