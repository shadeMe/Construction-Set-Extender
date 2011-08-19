#include "WindowManager.h"
#include "resource.h"
#include "CSInterop.h"
#include "Exports.h"
#include "Hooks\Misc.h"
#include "Hooks\Dialog.h"
#include "Hooks\AssetSelector.h"
#include "Hooks\Renderer.h"
#include "CSDialogs.h"
#include "RenderSelectionGroupManager.h"
#include "ElapsedTimeCounter.h"
#include "ToolManager.h"
#include "WorkspaceManager.h"
#include "RenderWindowTextPainter.h"
#include "ChangeLogManager.h"
#include "WindowEdgeSnapper.h"
#include "Achievements.h"
#include "CSAS\ScriptRunner.h"

using namespace Hooks;

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
WNDPROC						g_LandscapeTextureUseOrgWindowProc = NULL;

HFONT						g_CSDefaultFont = NULL;
const POINT					g_ObjectWindowTreePosOffset = { 0, 24 };
const POINT					g_CellViewWindowObjListPosOffset = { 0, 12 };
std::string					g_ObjectWindowFilterStr = "", g_CellViewWindowFilterStr = "";

CSnapWindow					g_WindowEdgeSnapper;

#define PI					3.151592653589793

LRESULT CALLBACK FindTextDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMACTIVATE:
			NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
			char Buffer[0x200] = {0};

			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 0, Buffer, sizeof(Buffer));
			std::string FormID, FormTypeStr(Buffer);

			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 1, Buffer, sizeof(Buffer));
			FormID = Buffer;
			UInt32 PadStart = FormID.find("(") + 1, PadEnd  = FormID.find(")", PadStart + 1);
			if (PadStart != std::string::npos && PadEnd != std::string::npos)
			{
				FormID = FormID.substr(PadStart, PadEnd - PadStart);
				UInt32 FormIDInt = 0;
				sscanf_s(FormID.c_str(), "%08X", &FormIDInt);
				if (TESForm::LookupByFormID(FormIDInt))
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

static TESFile**	s_DataDlgActiveTESFile = (TESFile**)0x00A0AA7C;
static bool			s_DataDlgSortOrderAscending = true;
static UInt32		s_DataDlgSortColumn = 0;

int CALLBACK DataDlgTESFileListViewStringCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TESFile* File1 = _DATAHANDLER->LookupPluginByIndex(lParam1);
	TESFile* File2 = _DATAHANDLER->LookupPluginByIndex(lParam2);
	int Result = 0;

	if (!File1 || !File2)
		return 0;

	if (lParamSort == 0)
		Result = _stricmp(File1->fileName, File2->fileName);
	else
	{
		Result = File1->fileFlags - File2->fileFlags;
	}

	if (!s_DataDlgSortOrderAscending)
		Result *= -1;

	return Result;
}

LRESULT CALLBACK DataDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND PluginList = GetDlgItem(hWnd, 1056);

	switch (uMsg)
	{
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_COLUMNCLICK:
			break;				// can't use this unless the dlg proc's code is patched to not refer to the file list's itesm by their index,
								// which is far too much work for meh

			NMLISTVIEW* ListView = (NMLISTVIEW*)lParam;
			if (PluginList == ListView->hdr.hwndFrom)
			{
				if (ListView->iSubItem != s_DataDlgSortColumn)
				{
					s_DataDlgSortOrderAscending = true;
					s_DataDlgSortColumn = ListView->iSubItem;
				}
				else
					s_DataDlgSortOrderAscending = (s_DataDlgSortOrderAscending == false);

				ListView_SortItems(ListView->hdr.hwndFrom, DataDlgTESFileListViewStringCompareFunc, ListView->iSubItem);
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case DATA_QUICKLOAD:
			g_QuickLoadToggle = (IsDlgButtonChecked(hWnd, DATA_QUICKLOAD) == BST_CHECKED);
			break;
		case DATA_SETSTARTUPPLUGIN:
			{
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

					Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
				}
			}
			break;
			}
		case 1:		// OK btn
			if (CLIWrapper::Interfaces::SE->GetOpenEditorCount())
			{
				if (MessageBox(hWnd, "There are open script windows. Are you sure you'd like to proceed?", "CSE", MB_YESNO|MB_ICONWARNING) == IDNO)
					return FALSE;
			}
			else if (*s_DataDlgActiveTESFile == NULL && g_QuickLoadToggle)
			{
				MessageBox(hWnd, "An active plugin must be set when quick-loading.", "CSE", MB_OK|MB_ICONEXCLAMATION);
				return FALSE;
			}
			else if ((*s_DataDlgActiveTESFile) != NULL && !_stricmp((*s_DataDlgActiveTESFile)->fileName, "oblivion.esm"))
			{
				if (MessageBox(hWnd, "You have set Oblvion.esm as an active file. Are you absolutely sure this is the end of the world ?", "CSE", MB_YESNO|MB_ICONWARNING) == IDNO)
					return FALSE;

				Achievements::UnlockAchievement(Achievements::kAchievement_Fearless);
			}

			break;
		}
		break;
	case WM_DESTROY:
		TESFile* ActiveFile = _DATAHANDLER->activeFile;					// required for correct esm handling
		if (ActiveFile)
			ToggleFlag(&ActiveFile->fileFlags, TESFile::kFileFlag_Master, 0);

		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_DataDlgOrgWindowProc);
		break;
	}

	return CallWindowProc(g_DataDlgOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CSMainWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
 	case WM_TIMER:
		{
			switch (wParam)		// need to handle this right here as the main window proc doesn't check the timer ID when processing the WM_TIMER message
			{
			case 2:				// autosave timer
				if (*g_TESCSAllowAutoSaveFlag && *g_TESCSExittingCSFlag == 0)
					TESDialog::AutoSave();
				break;
			case GLOBALSCRIPT_EXECUTION_TIMER:
				GLOBALSCRIPTMANAGER->ExecuteScripts();
				break;
			}

			if (!Achievements::GetAchievementUnlocked(Achievements::kAchievement_Cheat))
			{
				UInt32 TimeElapsed = GetTickCount() - Achievements::g_TickCount;
				if (TimeElapsed / (3600.0 * 1000.0) >= Achievements::kMaxCSSessionLength)
					Achievements::UnlockAchievement(Achievements::kAchievement_Cheat);
			}

			return TRUE;
		}
    case WM_MOVING:
        return g_WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
    case WM_ENTERSIZEMOVE:
        return g_WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
	case 0x40C:				// save handler
		if (g_QuickLoadToggle)
		{
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
			if (!_DATAHANDLER->activeFile)
			{
				MessageBox(*g_HWND_CSParent, "An active plugin must be set before using this tool.", "CSE", MB_OK|MB_ICONEXCLAMATION);
				break;
			}

			*g_TESCSAllowAutoSaveFlag = 0;

			char FileName[0x104];
			if (TESDialog::SelectTESFileCommonDialog(hWnd, g_INILocalMasterPath->value.s, 0, FileName, 0x104))
			{
				TESFile* SaveAsBuffer = _DATAHANDLER->activeFile;

				ToggleFlag(&SaveAsBuffer->fileFlags, TESFile::kFileFlag_Active, false);
				ToggleFlag(&SaveAsBuffer->fileFlags, TESFile::kFileFlag_Loaded, false);

				_DATAHANDLER->activeFile = NULL;

				if (SendMessage(*g_HWND_CSParent, 0x40C, NULL, (LPARAM)FileName))
				{
					TESDialog::SetCSWindowTitleModifiedFlag(false);
				}
				else
				{
					_DATAHANDLER->activeFile = SaveAsBuffer;
					ToggleFlag(&SaveAsBuffer->fileFlags, TESFile::kFileFlag_Active, true);
					ToggleFlag(&SaveAsBuffer->fileFlags, TESFile::kFileFlag_Loaded, true);
				}

				Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			}

			*g_TESCSAllowAutoSaveFlag = 1;
			break;
		case MAIN_WORLD_BATCHEDIT:
			{
			TESObjectCELL*	ThisCell = (*g_TES)->currentInteriorCell;
			if (!ThisCell)	ThisCell = (*g_TES)->currentExteriorCell;

			if (ThisCell)
			{
				UInt32 RefCount = ThisCell->objectList.Count(), i = 0;

				if (RefCount < 2)	break;

				ComponentDLLInterface::CellObjectData* RefData = new ComponentDLLInterface::CellObjectData[RefCount];
				ComponentDLLInterface::BatchRefData* BatchData = new ComponentDLLInterface::BatchRefData();

				for (TESObjectCELL::ObjectREFRList::Iterator Itr = ThisCell->objectList.Begin(); !Itr.End(); ++Itr, ++i)
				{
					TESObjectREFR* ThisRef = Itr.Get();
					ComponentDLLInterface::CellObjectData* ThisRefData = &RefData[i];

					ThisRefData->EditorID = (!ThisRef->editorID.c_str())?ThisRef->baseForm->editorID.c_str():ThisRef->editorID.c_str();
					ThisRefData->FormID = ThisRef->formID;
					ThisRefData->TypeID = ThisRef->baseForm->formType;
					ThisRefData->Flags = ThisRef->formFlags;
					ThisRefData->Selected = false;

					for (TESRenderSelection::SelectedObjectsEntry* j = (*g_TESRenderSelectionPrimary)->selectionList; j != 0; j = j->Next)
					{
						if (j->Data && j->Data == ThisRef)
						{
							ThisRefData->Selected = true;
							break;
						}
					}
					ThisRefData->ParentForm = ThisRef;
				}

				BatchData->CellObjectListHead = RefData;
				BatchData->ObjectCount = RefCount;

				if (CLIWrapper::Interfaces::BE->ShowBatchRefEditorDialog(BatchData))
				{
					EnterCriticalSection(g_ExtraListCS);

					for (UInt32 k = 0; k < RefCount; k++)
					{
						TESObjectREFR* ThisRef = (TESObjectREFR*)RefData[k].ParentForm;
						ComponentDLLInterface::CellObjectData* ThisRefData = &RefData[k];
						bool Modified = false;

						if (ThisRefData->Selected)
						{
							if (BatchData->World3DData.UsePosX())	ThisRef->position.x = BatchData->World3DData.PosX, Modified = true;
							if (BatchData->World3DData.UsePosY())	ThisRef->position.y = BatchData->World3DData.PosY, Modified = true;
							if (BatchData->World3DData.UsePosZ())	ThisRef->position.z = BatchData->World3DData.PosZ, Modified = true;

							if (BatchData->World3DData.UseRotX())	ThisRef->rotation.x = BatchData->World3DData.RotX * PI / 180, Modified = true;
							if (BatchData->World3DData.UseRotY())	ThisRef->rotation.y = BatchData->World3DData.RotY * PI / 180, Modified = true;
							if (BatchData->World3DData.UseRotZ())	ThisRef->rotation.z = BatchData->World3DData.RotZ * PI / 180, Modified = true;

							if (BatchData->World3DData.UseScale())	ThisRef->scale = BatchData->World3DData.Scale, Modified = true;

							if (BatchData->Flags.UsePersistent() &&
								ThisRef->baseForm->formType != TESForm::kFormType_NPC &&
								ThisRef->baseForm->formType != TESForm::kFormType_Creature)
							{
								ToggleFlag(&ThisRef->formFlags, TESForm::kFormFlags_QuestItem, BatchData->Flags.Persistent), Modified = true;
							}

							if (BatchData->Flags.UseDisabled())
								ToggleFlag(&ThisRef->formFlags, TESForm::kFormFlags_Disabled, BatchData->Flags.Disabled), Modified = true;
							if (BatchData->Flags.UseVWD())
								ToggleFlag(&ThisRef->formFlags, TESForm::kFormFlags_VisibleWhenDistant, BatchData->Flags.VWD), Modified = true;

							if (BatchData->EnableParent.UseEnableParent())
							{
								TESObjectREFR* Parent = (TESObjectREFR*)BatchData->EnableParent.Parent;
								if (Parent != ThisRef)
								{
									ThisRef->extraData.ModExtraEnableStateParent(Parent);
									ThisRef->SetExtraEnableStateParentOppositeState(BatchData->EnableParent.OppositeState);
									Modified = true;
								}
							}

							if (BatchData->Ownership.UseOwnership() &&
								ThisRef->baseForm->formType != TESForm::kFormType_NPC &&
								ThisRef->baseForm->formType != TESForm::kFormType_Creature)
							{
								ThisRef->extraData.ModExtraGlobal(NULL);
								ThisRef->extraData.ModExtraRank(-1);
								ThisRef->extraData.ModExtraOwnership(NULL);

								TESForm* Owner = (TESForm*)BatchData->Ownership.Owner;
								ThisRef->extraData.ModExtraOwnership(Owner);
								if (BatchData->Ownership.UseNPCOwner())
									ThisRef->extraData.ModExtraGlobal((TESGlobal*)BatchData->Ownership.Global);
								else
									ThisRef->extraData.ModExtraRank(BatchData->Ownership.Rank);

								Modified = true;
							}

							if (BatchData->Extra.UseCharge())		ThisRef->ModExtraCharge((float)BatchData->Extra.Charge), Modified = true;
							if (BatchData->Extra.UseHealth())		ThisRef->ModExtraHealth((float)BatchData->Extra.Health), Modified = true;
							if (BatchData->Extra.UseTimeLeft())		ThisRef->ModExtraTimeLeft((float)BatchData->Extra.TimeLeft), Modified = true;
							if (BatchData->Extra.UseSoulLevel())	ThisRef->ModExtraSoul(BatchData->Extra.SoulLevel), Modified = true;
							if (BatchData->Extra.UseCount())
							{
								switch (ThisRef->baseForm->formType)
								{
									case TESForm::kFormType_Apparatus:
									case TESForm::kFormType_Armor:
									case TESForm::kFormType_Book:
									case TESForm::kFormType_Clothing:
									case TESForm::kFormType_Ingredient:
									case TESForm::kFormType_Misc:
									case TESForm::kFormType_Weapon:
									case TESForm::kFormType_Ammo:
									case TESForm::kFormType_SoulGem:
									case TESForm::kFormType_Key:
									case TESForm::kFormType_AlchemyItem:
									case TESForm::kFormType_SigilStone:
										ThisRef->extraData.ModExtraCount(BatchData->Extra.Count), Modified = true;
									case TESForm::kFormType_Light:
										TESObjectLIGH* Light = CS_CAST(ThisRef->baseForm, TESForm, TESObjectLIGH);
										if (Light)
										{
											if (Light->IsCarriable())
												ThisRef->extraData.ModExtraCount(BatchData->Extra.Count), Modified = true;
										}
								}
							}
						}

						if (Modified)
						{
							ThisRef->UpdateUsageInfo();
							ThisRef->SetFromActiveFile(true);
							ThisRef->Update3D();
						}
					}

					LeaveCriticalSection(g_ExtraListCS);

					Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
				}

				delete BatchData;
			}
			break;
			}
		case MAIN_VIEW_CONSOLEWINDOW:
			if (CONSOLE->IsConsoleInitalized())
			{
				HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
				if (CONSOLE->ToggleDisplayState())
					CheckMenuItem(ViewMenu, MAIN_VIEW_CONSOLEWINDOW, MF_CHECKED);
				else
					CheckMenuItem(ViewMenu, MAIN_VIEW_CONSOLEWINDOW, MF_UNCHECKED);
			}
			break;
		case MAIN_VIEW_MODIFIEDRECORDS:
			FormEnumerationWrapper::ToggleUnmodifiedFormVisibility();
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case MAIN_DATA_CSEPREFERENCES:
			g_INIEditGUI->InitializeGUI(g_DLLInstance, *g_HWND_CSParent, g_INIManager);
			break;
		case MAIN_VIEW_DELETEDRECORDS:
			FormEnumerationWrapper::ToggleDeletedFormVisibility();
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case MAIN_WORLD_UNLOADCELL:
			ResetRenderWindow();
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case MAIN_GAMEPLAY_GLOBALSCRIPT:
			CreateDialog(g_DLLInstance, MAKEINTRESOURCE(DLG_GLOBALSCRIPT), hWnd, (DLGPROC)GlobalScriptDlgProc);
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case MAIN_LAUNCHGAME:
			ShellExecute(NULL, "open", (LPSTR)(std::string(g_APPPath + "obse_loader.exe")).c_str(), NULL, NULL, SW_SHOW);
			Achievements::UnlockAchievement(Achievements::kAchievement_LazyBum);
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case MAIN_VIEW_TAGBROWSER:
			CLIWrapper::Interfaces::TAG->ShowTagBrowserDialog(*g_HWND_CSParent);
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case MAIN_DATA_SETWORKSPACE:
			g_WorkspaceManager.SelectWorkspace(NULL);
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case ID_TOOLS_MANAGE:
			DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_MANAGETOOLS), *g_HWND_CSParent, ManageToolsDlgProc);
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case ID_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS:
			{
				HMENU SaveOptionsMenu = GetMenu(*g_HWND_CSParent); SaveOptionsMenu = GetSubMenu(SaveOptionsMenu, 0); SaveOptionsMenu = GetSubMenu(SaveOptionsMenu, 1);
				bool Checked = (GetMenuState(SaveOptionsMenu, ID_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS, MF_BYCOMMAND)) & MF_CHECKED;
				if (!Checked)
				{
					g_INIManager->FetchSetting("PreventTimeStampChanges")->SetValue("1");
					CheckMenuItem(SaveOptionsMenu, ID_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS, MF_CHECKED);
				}
				else
				{
					g_INIManager->FetchSetting("PreventTimeStampChanges")->SetValue("0");
					CheckMenuItem(SaveOptionsMenu, ID_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS, MF_UNCHECKED);
				}
			}

			return FALSE;
		case ID_SAVEOPTIONS_SAVELOADEDESP:
			{
				HMENU SaveOptionsMenu = GetMenu(*g_HWND_CSParent); SaveOptionsMenu = GetSubMenu(SaveOptionsMenu, 0); SaveOptionsMenu = GetSubMenu(SaveOptionsMenu, 1);
				bool Checked = (GetMenuState(SaveOptionsMenu, ID_SAVEOPTIONS_SAVELOADEDESP, MF_BYCOMMAND)) & MF_CHECKED;
				if (!Checked)
				{
					g_INIManager->FetchSetting("SaveLoadedESPsAsMasters")->SetValue("1");
					CheckMenuItem(SaveOptionsMenu, ID_SAVEOPTIONS_SAVELOADEDESP, MF_CHECKED);
				}
				else
				{
					g_INIManager->FetchSetting("SaveLoadedESPsAsMasters")->SetValue("0");
					CheckMenuItem(SaveOptionsMenu, ID_SAVEOPTIONS_SAVELOADEDESP, MF_UNCHECKED);
				}
			}

			return FALSE;
		case ID_CSAS_GLOBALDATA:
			GLOBALSCRIPTMANAGER->ShowGlobalVariableDialog();
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case ID_CSAS_RELOADGLOBALSCRIPTS:
			GLOBALSCRIPTMANAGER->ReloadScripts();
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
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x52:		// R
			if (*g_RenderWindowPathGridEditModeFlag)
			{
				if (GetAsyncKeyState(VK_CONTROL))
				{
					for (tList<TESPathGridPoint>::Iterator Itr = g_RenderWindowSelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
					{
						TESPathGridPoint* Point = Itr.Get();		// ### fix this once the class has been fully exposed

						TESObjectREFR* LinkedRef = *(TESObjectREFR**)((UInt32)Point + 0x20);
						TESPathGrid* ParentGrid = *(TESPathGrid**)((UInt32)Point + 0x24);
						NiTMapBase<TESObjectREFR*, tList<TESPathGridPoint>*>* LinkedRefMap = (NiTMapBase<TESObjectREFR*, tList<TESPathGridPoint>*>*)((UInt32)ParentGrid + 0x40);

						if (LinkedRef)
						{
							tList<TESPathGridPoint>* LinkedNodes = NULL;
							if (thisCall<UInt32>(0x004ADB90, LinkedRefMap, LinkedRef, &LinkedNodes))	// NiTPointerMap_LookupByKey
							{
								LinkedNodes->Remove(Point);
								if (LinkedNodes->Count() == 0)
								{
									thisCall<UInt32>(0x004BCBD0, LinkedRefMap, LinkedRef);				// NiTPointerMap_Remove
									FormHeap_Free(LinkedNodes);
								}

								*((UInt32*)((UInt32)Point + 0x20)) = NULL;
							}
						}
					}

					SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);		// reinitialize render window
					SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);

					Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
					return TRUE;
				}
				else
				{
					for (tList<TESPathGridPoint>::Iterator Itr = g_RenderWindowSelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
					{
						TESObjectREFR* LinkedRef = *(TESObjectREFR**)((UInt32)Itr.Get() + 0x20);
						if (LinkedRef)
						{
							MessageBox(*g_HWND_RenderWindow, "One or more of the selected path grid points is already linked to a reference.\n\nThey cannot not be linked to a different reference until they are unlinked first.", "CSE", MB_OK|MB_ICONEXCLAMATION);

							SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);
							SendMessage(*g_HWND_CSParent, WM_COMMAND, 40195, NULL);
							return TRUE;
						}
					}
				}
			}
			break;
		case 0x51:		// Q
			if (GetAsyncKeyState(VK_CONTROL))
			{
				if (g_RenderWindowAltMovementSettings)
					PrintToRender("Using vanilla movement settings", 3);
				else
					PrintToRender("Using alternate movement settings", 3);

				g_RenderWindowAltMovementSettings = (g_RenderWindowAltMovementSettings == false);
				Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
				return TRUE;
			}
		}
		break;
    case WM_MOVING:
        return g_WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
    case WM_ENTERSIZEMOVE:
        return g_WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
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
					for (TESObjectCELL::ObjectREFRList::Iterator Itr = CurrentCell->objectList.Begin(); !Itr.End(); ++Itr)
					{
						TESObjectREFR* Ref = Itr.Get();
						if (!Ref)
							break;

						ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_3DInvisible, false);
						ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Children3DInvisible, false);
					}

					PrintToRender("Reset visibility flags on the active cell's references", 3);
				}
				Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			}
			break;
		case ID_SELECTIONVISIBILITY_TOGGLEVISIBILITY:
		case ID_SELECTIONVISIBILITY_TOGGLECHILDRENVISIBILITY:
		case ID_SELECTIONFREEZING_FREEZE:
		case ID_SELECTIONFREEZING_UNFREEZE:
			for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
				UInt32 FlagMask = 0;

				switch (LOWORD(wParam))
				{
				case ID_SELECTIONVISIBILITY_TOGGLEVISIBILITY:
					ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_3DInvisible, !(Ref->formFlags & kTESObjectREFRSpecialFlags_3DInvisible));
					PrintToBuffer("Selection '%08X's visibility toggled", Ref->formID);
					break;
				case ID_SELECTIONVISIBILITY_TOGGLECHILDRENVISIBILITY:
					ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Children3DInvisible, !(Ref->formFlags & kTESObjectREFRSpecialFlags_Children3DInvisible));
					PrintToBuffer("Selection '%08X's children visibility toggled", Ref->formID);
					break;
				case ID_SELECTIONFREEZING_FREEZE:
					ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Frozen, true);
					PrintToBuffer("Selection '%08X' frozen", Ref->formID);
					break;
				case ID_SELECTIONFREEZING_UNFREEZE:
					ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Frozen, false);
					PrintToBuffer("Selection '%08X' thawed", Ref->formID);
					break;
				}
				PrintToRender(g_TextBuffer, 2);
			}
			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			break;
		case ID_SELECTIONGROUPING_GROUP:
		case ID_SELECTIONGROUPING_UNGROUP:
			if ((*g_TESRenderSelectionPrimary)->selectionCount > 1)
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
				Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			}
			break;
		case ID_ALIGNSELECTION_BYXAXIS:
		case ID_ALIGNSELECTION_BYYAXIS:
		case ID_ALIGNSELECTION_BYZAXIS:
			if ((*g_TESRenderSelectionPrimary)->selectionCount > 1)
			{
//				(*g_TESRenderUndoStack)->RecordReference(TESRenderUndoStack::kUndoOperation_Unk03, (*g_TESRenderSelectionPrimary));

				TESObjectREFR* AlignRef = CS_CAST((*g_TESRenderSelectionPrimary)->selectionList->Data, TESForm, TESObjectREFR);

				for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->selectionList->Next; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* ThisRef = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

					switch (LOWORD(wParam))
					{
					case ID_ALIGNSELECTION_BYXAXIS:
						ThisRef->position.x = AlignRef->position.x;
						break;
					case ID_ALIGNSELECTION_BYYAXIS:
						ThisRef->position.y = AlignRef->position.y;
						break;
					case ID_ALIGNSELECTION_BYZAXIS:
						ThisRef->position.z = AlignRef->position.z;
						break;
					}

					ThisRef->UpdateUsageInfo();
					ThisRef->SetFromActiveFile(true);
					ThisRef->Update3D();
				}

				PrintToBuffer("Selection aligned to %08X", AlignRef->formID);
				PrintToRender(g_TextBuffer, 2);
				Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
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
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* SizeInfo = (MINMAXINFO*)lParam;
			SizeInfo->ptMaxTrackSize.x = SizeInfo->ptMinTrackSize.x = 194;
			SizeInfo->ptMaxTrackSize.y = SizeInfo->ptMinTrackSize.y = 213;
			break;
		}
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
		case BTN_EXTRACTFILE:
			EndDialog(hWnd, e_ExtractPath);
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
			TESForm* SelectedForm = (TESForm*)TESComboBox::GetSelectedItemData(ComboBox);
			EndDialog(hWnd, (INT_PTR)SelectedForm);
			return TRUE;
		}
		case BTN_CANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		TESComboBox::PopulateWithForms(ComboBox, lParam, true, false);
		break;
	}
	return FALSE;
}

BOOL CALLBACK CopyPathDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool s_DraggingMouse = false;

	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
		if (!s_DraggingMouse)
		{
			s_DraggingMouse = true;
			Edit_SetText(GetDlgItem(hWnd, EDIT_PATH), "Dragging...");
			SetCapture(hWnd);
		}
		break;
	case WM_LBUTTONUP:
		if (s_DraggingMouse)
		{
			POINT Location = {0};
			Location.x = GET_X_LPARAM(lParam);
			Location.y = GET_Y_LPARAM(lParam);
			ClientToScreen(hWnd, &Location);

			HWND Window = WindowFromPoint(Location);
			if (Window)
			{
				GetWindowText(Window, g_TextBuffer, sizeof(g_TextBuffer));
				Edit_SetText(GetDlgItem(hWnd, EDIT_PATH), g_TextBuffer);
			}
			else
				Edit_SetText(GetDlgItem(hWnd, EDIT_PATH), NULL);

			ReleaseCapture();
			s_DraggingMouse = false;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
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
					{
						std::string Buffer(g_TextBuffer);
						int Offset = Buffer.find_first_of("IdleAnims\\");
						if (Offset != -1)
							Buffer = Buffer.substr(Offset + 9);
						PrintToBuffer("%s", Buffer.c_str());
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
    case WM_MOVING:
        return g_WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
    case WM_ENTERSIZEMOVE:
        return g_WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
	{
		tagRECT WindowRect, EditRect;
		GetWindowRect(hWnd, &WindowRect);
		MoveWindow(GetDlgItem(hWnd, EDIT_CONSOLE), 0, 0, WindowRect.right - WindowRect.left - 15, WindowRect.bottom - WindowRect.top - 65, TRUE);
		GetWindowRect(GetDlgItem(hWnd, EDIT_CONSOLE), &EditRect);
		SetWindowPos(GetDlgItem(hWnd, EDIT_CMDBOX), HWND_NOTOPMOST, 0, EditRect.bottom - EditRect.top, WindowRect.right - WindowRect.left - 10, 45, SWP_NOZORDER);
		break;
	}
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_ConsoleWndOrgWindowProc);
		break;
	case WM_INITDIALOG:
		HFONT EditFont = CreateFont(20, 0, 0, 0,
                             FW_BOLD, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, 5,		// CLEARTYPE_QUALITY
                             FF_DONTCARE, "Consolas");
		SendMessage(GetDlgItem(hWnd, EDIT_CMDBOX), WM_SETFONT, (WPARAM)EditFont, (LPARAM)TRUE);
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

		if (PtInRect((LPRECT) &Rect, Point))
		{
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
			case CONSOLEMENU_OPENCHANGELOG:
				VersionControl::CHANGELOG->OpenSessionLog();
				Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
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
				Edit_SetText(hWnd, NULL);
				if (CommandStack.empty() == false)
				{
					std::string Command(CommandStack.top());
					Edit_SetText(hWnd, Command.c_str());
					CommandStack.pop();
					AlternateCommandStack.push(Command);
				}
				return TRUE;
			}
		case VK_DOWN:
			{
				Edit_SetText(hWnd, NULL);
				if (AlternateCommandStack.empty() == false)
				{
					std::string Command(AlternateCommandStack.top());
					Edit_SetText(hWnd, Command.c_str());
					AlternateCommandStack.pop();
					CommandStack.push(Command);
				}
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
			if (Form->formID < 0x800)	break;

			sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%08X", Form->formID);
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

				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Change FormID from %08X to %08X ?\n\nMod index bits will be automatically corrected by the CS when saving.\nCheck the console for formID bashing on confirmation.", Form->formID, FormID);
				if (MessageBox(hWnd, g_TextBuffer, "CSE", MB_YESNO) == IDYES)
				{
					Form->SetFormID(FormID);
					Form->SetFromActiveFile(true);
				}
			}
			break;
		}
		case POPUP_MARKUNMODIFIED:
			sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Are you sure you want to mark form '%s' (%08X) as unmodified ?\n\nThis will not revert any changes made to it.", Form->editorID.c_str(), Form->formID);
			if (MessageBox(hWnd, g_TextBuffer, "CSE", MB_YESNO) == IDYES)
			{
				Form->SetFromActiveFile(false);
			}
			break;
		case POPUP_JUMPTOUSEINFOLIST:
		{
			const char* EditorID = Form->editorID.c_str();

			if (EditorID)
				CLIWrapper::Interfaces::USE->ShowUseInfoListDialog(EditorID);
			else
			{
				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%08X", Form->formID);
				CLIWrapper::Interfaces::USE->ShowUseInfoListDialog(g_TextBuffer);
			}
			break;
		}
		case POPUP_UNDELETE:
		{
			PrintToBuffer("Are you sure you want to undelete form '%s' (%08X) ?\n\nOld references to it will not be restored.", Form->editorID.c_str(), Form->formID);
			if (MessageBox(hWnd, g_TextBuffer, "CSE", MB_YESNO) == IDYES)
			{
				Form->SetDeleted(false);
			}
			break;
		}
		case POPUP_EDITBASEFORM:
		{
			TESForm* BaseForm = (CS_CAST(Form, TESForm, TESObjectREFR))->baseForm;
			if (BaseForm && BaseForm->editorID.c_str())
			{
				ShowFormEditDialog(BaseForm->editorID.c_str(), BaseForm->formType);
			}
			break;
		}
		case POPUP_ADDTOTAG:
		{
			ComponentDLLInterface::FormData Data(Form);
			CLIWrapper::Interfaces::TAG->AddFormToActiveTag(&Data);
			break;
		}
		case POPUP_TOGGLEVISIBILITY:
		{
			TESObjectREFR* Ref = CS_CAST(Form, TESForm, TESObjectREFR);
			ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_3DInvisible, !(Ref->formFlags & kTESObjectREFRSpecialFlags_3DInvisible));
			break;
		}
		case POPUP_TOGGLECHILDRENVISIBILITY:
		{
			TESObjectREFR* Ref = CS_CAST(Form, TESForm, TESObjectREFR);
			ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Children3DInvisible, !(Ref->formFlags & kTESObjectREFRSpecialFlags_Children3DInvisible));
			break;
		}
		case POPUP_SHOWOVERRIDES:
		{
			std::string Buffer = PrintToBuffer("Override list for form '%08X':\n\n", Form->formID);
			for (TESForm::OverrideFileListT::Iterator Itr = Form->fileList.Begin(); !Itr.End(); ++Itr)
			{
				TESFile* File = Itr.Get();
				if (!File)
					break;

				Buffer += PrintToBuffer("\t%s\n", File->fileName);
			}
			MessageBox(hWnd, Buffer.c_str(), "CSE", MB_OK|MB_ICONINFORMATION);
			break;
		}
	}
	UpdateWindow(hWnd);
	Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
}

#define OBJECTCELLWINDOW_FILTERINPUTTIMERID				0x99
#define OBJECTCELLWINDOW_FILTERINPUTTIMERPERIOD			500

LRESULT CALLBACK ObjectWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int s_FilterTimeCounter = -1;

	HWND FilterEditBox = GetDlgItem(hWnd, OBJCELL_FILTEREDIT);
	HWND FilterLabel = GetDlgItem(hWnd, OBJCELL_FILTERLBL);

	switch (uMsg)
	{
	case 0x417:		// destroy window
		KillTimer(hWnd, OBJECTCELLWINDOW_FILTERINPUTTIMERID);
		g_ObjectWindowFilterStr = "";
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case OBJCELL_FILTEREDIT:
			if (HIWORD(wParam) == EN_CHANGE)
				s_FilterTimeCounter = 0;
			break;
		}
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case OBJECTCELLWINDOW_FILTERINPUTTIMERID:
			if (s_FilterTimeCounter >= 750)
			{
				char Buffer[0x200] = {0};
				GetWindowText(FilterEditBox, (LPSTR)Buffer, sizeof(Buffer));
				if (strlen(Buffer))
				{
					g_ObjectWindowFilterStr = Buffer;
					MakeLower(g_ObjectWindowFilterStr);
				}
				else
				{
					g_ObjectWindowFilterStr = "";
				}

				s_FilterTimeCounter = -1;

				HTREEITEM Selection = TreeView_GetSelection(*g_HWND_ObjectWindow_Tree);
				TreeView_SelectItem(*g_HWND_ObjectWindow_Tree, NULL);
				TreeView_SelectItem(*g_HWND_ObjectWindow_Tree, Selection);
			}
			else if (s_FilterTimeCounter != -1)
				s_FilterTimeCounter += OBJECTCELLWINDOW_FILTERINPUTTIMERPERIOD;

			break;
		}
		break;
    case WM_MOVING:
        return g_WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
    case WM_ENTERSIZEMOVE:
        return g_WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
	case WM_INITDIALOG:
		SetTimer(hWnd, OBJECTCELLWINDOW_FILTERINPUTTIMERID, OBJECTCELLWINDOW_FILTERINPUTTIMERPERIOD, NULL);

		if (!FilterEditBox || !FilterLabel)		// create controls for the first time
		{
			FilterLabel = CreateWindowEx(0,
										"STATIC",
										"Filter",
										WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE,
										5, 2, 30, 20,
										hWnd,
										(HMENU)OBJCELL_FILTERLBL,
										GetModuleHandle(NULL),
										NULL);
			FilterEditBox = CreateWindowEx(WS_EX_CLIENTEDGE,
										"EDIT",
										"",
										ES_LEFT|ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
										35, 2, 120, 20,
										hWnd,
										(HMENU)OBJCELL_FILTEREDIT,
										GetModuleHandle(NULL),
										NULL);

			SendMessage(FilterLabel, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(FilterEditBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
		}

		s_FilterTimeCounter = -1;
		break;
	}

	return CallWindowProc(g_ObjectWndOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CellViewWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND*	s_CellViewListViewUnderCursor = (HWND*)0x00A0A9E4;
	static int		s_FilterTimeCounter = -1;

	HWND FilterEditBox = GetDlgItem(hWnd, OBJCELL_FILTEREDIT);
	HWND FilterLabel = GetDlgItem(hWnd, OBJCELL_FILTERLBL);
	HWND XCoordEditBox = GetDlgItem(hWnd, CELLVIEW_XEDIT);
	HWND YCoordEditBox = GetDlgItem(hWnd, CELLVIEW_YEDIT);

	switch (uMsg)
	{
	case 0x417:		// destroy window
		KillTimer(hWnd, OBJECTCELLWINDOW_FILTERINPUTTIMERID);
		g_CellViewWindowFilterStr = "";
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case CELLVIEW_GOBTN:
			{
				char XCoord[4] = {0}, YCoord[4] = {0};
				GetWindowText(XCoordEditBox, (LPSTR)XCoord, sizeof(XCoord));
				GetWindowText(YCoordEditBox, (LPSTR)YCoord, sizeof(YCoord));

				if (strlen(XCoord) && strlen(YCoord))
				{
					Vector3 Coords((atoi(XCoord) << 12) + 2048, (atoi(YCoord) << 12) + 2048, 0);
					_TES->LoadCellIntoViewPort(&Coords, NULL);
				}
				break;
			}
		case OBJCELL_FILTEREDIT:
			if (HIWORD(wParam) == EN_CHANGE)
				s_FilterTimeCounter = 0;
			break;
		}
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case OBJECTCELLWINDOW_FILTERINPUTTIMERID:
			if (s_FilterTimeCounter >= 750)
			{
				char Buffer[0x200] = {0};
				GetWindowText(FilterEditBox, (LPSTR)Buffer, sizeof(Buffer));
				if (strlen(Buffer))
				{
					g_CellViewWindowFilterStr = Buffer;
					MakeLower(g_CellViewWindowFilterStr);
				}
				else
				{
					g_CellViewWindowFilterStr = "";
				}

				s_FilterTimeCounter = -1;

				SendMessage(hWnd, 0x40F, NULL, NULL);	// reinit object list
			}
			else if (s_FilterTimeCounter != -1)
				s_FilterTimeCounter += OBJECTCELLWINDOW_FILTERINPUTTIMERPERIOD;

			break;
		}
		break;
    case WM_MOVING:
        return g_WindowEdgeSnapper.OnSnapMoving(hWnd, uMsg, wParam, lParam);
    case WM_ENTERSIZEMOVE:
        return g_WindowEdgeSnapper.OnSnapEnterSizeMove(hWnd, uMsg, wParam, lParam);
	case WM_INITDIALOG:
		SetTimer(hWnd, OBJECTCELLWINDOW_FILTERINPUTTIMERID, OBJECTCELLWINDOW_FILTERINPUTTIMERPERIOD, NULL);

		if (!FilterEditBox || !FilterLabel)		// create controls for the first time
		{
			FilterLabel = CreateWindowEx(0,
										"STATIC",
										"Filter",
										WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE,
										0, 0, 25, 20,
										hWnd,
										(HMENU)OBJCELL_FILTERLBL,
										GetModuleHandle(NULL),
										NULL);
			FilterEditBox = CreateWindowEx(WS_EX_CLIENTEDGE,
										"EDIT",
										"",
										ES_LEFT|ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
										0, 0, 150, 20,
										hWnd,
										(HMENU)OBJCELL_FILTEREDIT,
										GetModuleHandle(NULL),
										NULL);
			HWND XCoordLbl = CreateWindowEx(0,
										"STATIC",
										"X",
										WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE,
										0, 0, 5, 20,
										hWnd,
										(HMENU)CELLVIEW_XLBL,
										GetModuleHandle(NULL),
										NULL);
			HWND YCoordLbl = CreateWindowEx(0,
										"STATIC",
										"Y",
										WS_CHILD|WS_VISIBLE|SS_CENTERIMAGE,
										0, 0, 5, 20,
										hWnd,
										(HMENU)CELLVIEW_YLBL,
										GetModuleHandle(NULL),
										NULL);
			XCoordEditBox = CreateWindowEx(WS_EX_CLIENTEDGE,
										"EDIT",
										"",
										ES_LEFT|ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
										0, 0, 8, 20,
										hWnd,
										(HMENU)CELLVIEW_XEDIT,
										GetModuleHandle(NULL),
										NULL);
			YCoordEditBox = CreateWindowEx(WS_EX_CLIENTEDGE,
										"EDIT",
										"",
										ES_LEFT|ES_AUTOHSCROLL|WS_CHILD|WS_VISIBLE|WS_TABSTOP,
										0, 0, 8, 20,
										hWnd,
										(HMENU)CELLVIEW_YEDIT,
										GetModuleHandle(NULL),
										NULL);
			HWND GoButton = CreateWindowEx(0,
										"BUTTON",
										"Go",
										WS_CHILD|WS_VISIBLE|WS_TABSTOP,
										0, 0, 10, 20,
										hWnd,
										(HMENU)CELLVIEW_GOBTN,
										GetModuleHandle(NULL),
										NULL);

			SendMessage(FilterLabel, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(FilterEditBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(XCoordLbl, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(YCoordLbl, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(XCoordEditBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(YCoordEditBox, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
			SendMessage(GoButton, WM_SETFONT, (WPARAM)g_CSDefaultFont, TRUE);
		}

		s_FilterTimeCounter = -1;
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
			OPENFILENAME SelectFile = {0};
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
				std::string Destination(g_APPPath + "\\" + std::string(VoicePath)), DirPath(Destination);
				DirPath = DirPath.substr(0, DirPath.find_last_of("\\") + 1);

				if (SHCreateDirectoryEx(NULL, DirPath.c_str(), NULL) && GetLastError() != ERROR_FILE_EXISTS && GetLastError() != ERROR_ALREADY_EXISTS)
				{
					DebugPrint("Couldn't create intermediate path for the new voice file!");
					LogWinAPIErrorMessage(GetLastError());
				}

				if (!CopyFile(FilePath, Destination.c_str(), FALSE))
				{
					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "Couldn't copy external file '%s' to '%s'!\n\nCheck the console for more information.", FilePath, Destination.c_str());
					MessageBox(hWnd, g_TextBuffer, "CSE", MB_OK|MB_ICONEXCLAMATION);
					LogWinAPIErrorMessage(GetLastError());
				}
				else
					DebugPrint("Copied external audio file '%s' to '%s'", FilePath, Destination.c_str());

				Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
			}
			return FALSE;
		}
		case 1016:					// Generate LIP
			if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
				return FALSE;

			std::string Path(VoicePath);
			Path = Path.substr(0, Path.find_last_of("."));

			if (!CSIOM->DoGenerateLIPOperation(Path.c_str(), (*g_ResponseEditorData)->responseLocalCopy->responseText.c_str()))
			{
				MessageBox(hWnd, "Couldn't generate LIP file for the selected voice.\n\nCheck the console for more information.", "CSE", MB_OK|MB_ICONEXCLAMATION);
			}
			else
			{
				DebugPrint("Successfully generated LIP file for the selected voice");
				Achievements::UnlockAchievement(Achievements::kAchievement_Soprano);
			}

			Achievements::UnlockAchievement(Achievements::kAchievement_PowerUser);
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

				Form = TESForm::LookupByEditorID(QuestID);
				if (Form)
				{
					if (Form->formType == TESForm::kFormType_Quest)
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
					Quest = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Quest), TESForm, TESQuest);
					Quest->SetFromActiveFile(true);
					_DATAHANDLER->quests.AddAt(Quest, eListEnd);
					Quest->SetEditorID(QuestID);
				}

				if (strlen(ScriptID) < 1)
					sprintf_s(ScriptID, sizeof(ScriptID), "%sScript", QuestID);

				Form = TESForm::LookupByEditorID(ScriptID);
				if (Form)
				{
					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "EditorID '%s' is already in use.", ScriptID);
					MessageBox(hWnd, g_TextBuffer, "CSE", MB_OK|MB_ICONEXCLAMATION);
					return TRUE;
				}
				else
				{
					QuestScript = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Script), TESForm, Script);
					QuestScript->SetFromActiveFile(true);
					_DATAHANDLER->scripts.AddAt(QuestScript, eListEnd);
					_DATAHANDLER->SortScripts();
					QuestScript->info.type = Script::kScriptType_Quest;

					sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "scn %s\n\nfloat fQuestDelayTime\n\nBegin GameMode\n\tlet fQuestDelayTime := %s\n\nend", ScriptID, Delay);
					QuestScript->SetText(g_TextBuffer);
					QuestScript->SetEditorID(ScriptID);
				}

				Quest->script = QuestScript;

				Quest->Link();
				QuestScript->Link();

				Quest->UpdateUsageInfo();
				QuestScript->UpdateUsageInfo();
				QuestScript->AddCrossReference(Quest);

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
			TESForm* Selection = (TESForm*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TESCOMBOBOX), hWnd, (DLGPROC)TESComboBoxDlgProc, (LPARAM)TESForm::kFormType_Cell);
			if (Selection)
			{
				sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "%s (%08X)", Selection->editorID.c_str(), Selection->formID);
				SetWindowText(SelParentCellBtn, (LPCSTR)g_TextBuffer);
				SetWindowLong(SelParentCellBtn, GWL_USERDATA, (LONG)Selection);
			}
			break;
		}
		case BTN_OK:
		{
			if (IsDlgButtonChecked(hWnd, RADIO_EXISTFORM))
			{
				TESForm* SelectedForm = (TESForm*)TESComboBox::GetSelectedItemData(ExistFormList);
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
				if (TESForm::LookupByEditorID(BaseEditorID))
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

						TESQuest* Quest = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Quest), TESForm, TESQuest);
						Quest->SetFromActiveFile(true);
						_DATAHANDLER->quests.AddAt(Quest, eListEnd);
						Quest->SetEditorID(BaseEditorID);
						Quest->SetStartGameEnabledFlag(StartGameEnabledFlag);
						Quest->SetAllowedRepeatedStagesFlag(RepeatedStagesFlag);

						FormEnumerationWrapper::ReinitializeFormLists();
						EndDialog(hWnd, (INT_PTR)Quest);
						return TRUE;
					}
					else
					{
						if (IsDlgButtonChecked(hWnd, RADIO_OBJECTTOKEN))
						{
							bool QuestItem = IsDlgButtonChecked(hWnd, CHECK_OBJECTTOKEN_QUESTITEM);

							TESObjectCLOT* Token = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Clothing), TESForm, TESObjectCLOT);
							Token->SetFromActiveFile(true);
							_DATAHANDLER->AddTESObject(Token);
							Token->SetEditorID(BaseEditorID);

							ToggleFlag(&Token->bipedModelFlags, TESBipedModelForm::kBipedModelFlags_NotPlayable, true);
							ToggleFlag(&Token->formFlags, TESForm::kFormFlags_QuestItem, QuestItem);

							FormEnumerationWrapper::ReinitializeFormLists();
							EndDialog(hWnd, (INT_PTR)Token);
							return TRUE;
						}
						else
						{
							Edit_GetText(RefIDBox, RefEditorID, 0x200);
							if (TESForm::LookupByEditorID(RefEditorID))
							{
								sprintf_s(g_TextBuffer, sizeof(g_TextBuffer), "EditorID '%s' is already in use.", RefEditorID);
								MessageBox(hWnd, g_TextBuffer, "CSE", MB_OK|MB_ICONERROR);
							}
							else
							{
								bool InitiallyDisabled = IsDlgButtonChecked(hWnd, CHECK_OBJECTREFERENCE_DISABLED);
								TESObjectCELL* ParentCell = CS_CAST(GetWindowLong(SelParentCellBtn, GWL_USERDATA), TESForm, TESObjectCELL);

								if (!ParentCell || ParentCell->GetIsInterior() == 0)
								{
									MessageBox(hWnd, "Invalid/exterior cell selected as parent.", "CSE", MB_OK|MB_ICONERROR);
								}
								else
								{
									TESObjectACTI* Activator =  CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Activator), TESForm, TESObjectACTI);
									Activator->SetFromActiveFile(true);
									_DATAHANDLER->AddTESObject(Activator);
									Activator->SetEditorID(BaseEditorID);

									static Vector3 ZeroVector(0.0, 0.0, 0.0);

									TESObjectREFR* Ref = _DATAHANDLER->PlaceObjectRef(Activator, &ZeroVector, &ZeroVector, CS_CAST(ParentCell, TESForm, TESObjectCELL), NULL, NULL);

									ToggleFlag(&Ref->formFlags, TESForm::kFormFlags_Disabled, InitiallyDisabled);
									ToggleFlag(&Ref->formFlags, TESForm::kFormFlags_QuestItem, true);

									Ref->SetEditorID(RefEditorID);

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

		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Activator, true, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Apparatus, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Armor, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Book, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Clothing, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Container, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Door, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Ingredient, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Light, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Misc, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Furniture, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Weapon, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Ammo, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_NPC, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Creature, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_SoulGem, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Key, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_AlchemyItem, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_SigilStone, false, false);
		TESComboBox::PopulateWithForms(ExistFormList, TESForm::kFormType_Quest, false, false);

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
		for (TESRenderSelection::SelectedObjectsEntry* Itr = (*g_TESRenderSelectionPrimary)->selectionList; Itr && Itr->Data; Itr = Itr->Next)
		{
			TESForm* Form = Itr->Data;
			ComponentDLLInterface::FormData Data(Form);
			if (!CLIWrapper::Interfaces::TAG->AddFormToActiveTag(&Data))
				break;
		}

		(*g_TESRenderSelectionPrimary)->ClearSelection();
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

				OPENFILENAME SelectFile = {0};
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

				BROWSEINFO FolderInfo = {0};
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
		case BTN_APPLY:
			{
				int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
				if (Index != LB_ERR)
				{
					ToolManager::Tool* Tool = (ToolManager::Tool*)SendMessage(ToolList, LB_GETITEMDATA, Index, NULL);
					char CmdLineBuffer[MAX_PATH] = {0}, InitDirBuffer[MAX_PATH] = {0};
					GetWindowText(CmdLineBox, (LPSTR)CmdLineBuffer, MAX_PATH);
					GetWindowText(InitDirBox, (LPSTR)InitDirBuffer, MAX_PATH);

					if (strlen(CmdLineBuffer) > 0 && strlen(InitDirBuffer) > 0)
					{
						if ((strstr(CmdLineBuffer, "=") || strstr(InitDirBuffer, "=")) ||
							(strstr(CmdLineBuffer, "|") || strstr(InitDirBuffer, "|")))
						{
							MessageBox(hWnd, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.", "CSE", MB_OK|MB_ICONEXCLAMATION);
							break;
						}

						Tool->SetCommandLine(CmdLineBuffer);
						Tool->SetInitialDir(InitDirBuffer);
					}
				}
			}
			break;
		case LISTBOX_TOOLLIST:
			{
				switch (HIWORD(wParam))
				{
				case LBN_SELCHANGE:
					int Index = SendMessage(ToolList, LB_GETCURSEL, NULL, NULL);
					if (Index != LB_ERR)
					{
						ToolManager::Tool* Tool = (ToolManager::Tool*)SendMessage(ToolList, LB_GETITEMDATA, Index, NULL);
						SetWindowText(TitleBox, (LPSTR)Tool->GetTitle());
						SetWindowText(CmdLineBox, (LPSTR)Tool->GetCommandLine());
						SetWindowText(InitDirBox, (LPSTR)Tool->GetInitialDir());
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

#define CSASGLOBALS_CLEAREDITFIELD		99999

BOOL CALLBACK CSASGlobalsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND GlobaList = GetDlgItem(hWnd, LISTBOX_GLOBALLIST);
	HWND NameBox =  GetDlgItem(hWnd, EDIT_NAME);
	HWND ValueBox =  GetDlgItem(hWnd, EDIT_VALUE);

	switch (uMsg)
	{
	case CSASGLOBALS_CLEAREDITFIELD:
		SetWindowText(NameBox, NULL);
		SetWindowText(ValueBox, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_CLOSE:
			EndDialog(hWnd, 0);
			return TRUE;
		case BTN_ADDGLOBALSTR:
		case BTN_ADDGLOBALNUM:
			{
				char NameBuffer[0x200] = {0}, ValueBuffer[0x200] = {0};
				GetWindowText(NameBox, (LPSTR)NameBuffer, 0x200);
				GetWindowText(ValueBox, (LPSTR)ValueBuffer, 0x200);

				if ((strlen(NameBuffer) < 1 || strlen(ValueBuffer) < 1) ||
					(strstr(NameBuffer, "=") || strstr(ValueBuffer, "=")) ||
					(strstr(NameBuffer, "|") || strstr(ValueBuffer, "|")))
				{
					MessageBox(hWnd, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.", "CSE", MB_OK|MB_ICONEXCLAMATION);
					break;
				}

				CSAutomationScript::ScriptVariable* NewGlobal = NULL;
				switch (LOWORD(wParam))
				{
				case BTN_ADDGLOBALSTR:
					NewGlobal = GLOBALSCRIPTMANAGER->AddGlobalVariable(NameBuffer, ValueBuffer);
					break;
				case BTN_ADDGLOBALNUM:
					NewGlobal = GLOBALSCRIPTMANAGER->AddGlobalVariable(NameBuffer, atof(ValueBuffer));
					break;
				}

				if (NewGlobal == NULL)
				{
					MessageBox(hWnd, "Enter a unique variable name", "CSE", MB_OK|MB_ICONEXCLAMATION);
					break;
				}

				sprintf_s(NameBuffer, sizeof(NameBuffer), "%s [%s]", NewGlobal->GetName(),
					((NewGlobal->GetDataType() <  CSAutomationScript::CSASDataElement::kParamType_String)?"i":"s"));
				int Index = SendMessage(GlobaList, LB_INSERTSTRING, -1, (LPARAM)NameBuffer);
				SendMessage(GlobaList, LB_SETITEMDATA, Index, (LPARAM)NewGlobal);
			}
			break;
		case BTN_REMOVE:
			{
				int Index = SendMessage(GlobaList, LB_GETCURSEL, NULL, NULL);
				if (Index != LB_ERR)
				{
					CSAutomationScript::ScriptVariable* Global = (CSAutomationScript::ScriptVariable*)SendMessage(GlobaList, LB_GETITEMDATA, Index, NULL);
					GLOBALSCRIPTMANAGER->RemoveGlobalVariable(Global->GetName(), true);
					SendMessage(GlobaList, LB_DELETESTRING, Index, NULL);
					SendMessage(hWnd, CSASGLOBALS_CLEAREDITFIELD, NULL, NULL);
					SendMessage(GlobaList, LB_SETSEL, FALSE, -1);
				}
			}
			break;
		case BTN_APPLYCHANGES:
			{
				int Index = SendMessage(GlobaList, LB_GETCURSEL, NULL, NULL);
				if (Index != LB_ERR)
				{
					CSAutomationScript::ScriptVariable* Global = (CSAutomationScript::ScriptVariable*)SendMessage(GlobaList, LB_GETITEMDATA, Index, NULL);
					char ValueBuffer[0x200] = {0};
					GetWindowText(ValueBox, (LPSTR)ValueBuffer, 0x200);

					if (strlen(ValueBuffer) > 0)
					{
						if (strstr(ValueBuffer, "=") || strstr(ValueBuffer, "|"))
						{
							MessageBox(hWnd, "Invalid input. Make sure the strings are non-null and don't contain a '=' or a '|'.", "CSE", MB_OK|MB_ICONEXCLAMATION);
							break;
						}

						switch (Global->GetDataType())
						{
						case CSAutomationScript::CSASDataElement::kParamType_Numeric:
						case CSAutomationScript::CSASDataElement::kParamType_Reference:
							Global->SetValue(mup::Value(atof(ValueBuffer)));
							break;
						case CSAutomationScript::CSASDataElement::kParamType_String:
							Global->SetValue(mup::Value(ValueBuffer));
							break;
						default:
							assert(0);
							break;
						}
					}
				}
			}
			break;
		case LISTBOX_GLOBALLIST:
			{
				switch (HIWORD(wParam))
				{
				case LBN_SELCHANGE:
					int Index = SendMessage(GlobaList, LB_GETCURSEL, NULL, NULL);
					if (Index != LB_ERR)
					{
						CSAutomationScript::ScriptVariable* Global = (CSAutomationScript::ScriptVariable*)SendMessage(GlobaList, LB_GETITEMDATA, Index, NULL);
						SetWindowText(NameBox, (LPSTR)Global->GetName());

						switch (Global->GetDataType())
						{
						case CSAutomationScript::CSASDataElement::kParamType_Numeric:
						case CSAutomationScript::CSASDataElement::kParamType_Reference:
							SetWindowText(ValueBox, (LPSTR)PrintToBuffer("%0.6f", Global->GetValue().GetFloat() * 1.0));
							break;
						case CSAutomationScript::CSASDataElement::kParamType_String:
							SetWindowText(ValueBox, (LPSTR)Global->GetValue().GetString().c_str());
							break;
						default:
							assert(0);
							break;
						}
					}
					else
						SendMessage(hWnd, CSASGLOBALS_CLEAREDITFIELD, NULL, NULL);
					break;
				}
			}
			break;
		}
		break;
	case WM_INITDIALOG:
		GLOBALSCRIPTMANAGER->PopulateListBoxWithGlobalVariables(GlobaList);
		break;
	}
	return FALSE;
}

BOOL CALLBACK AchievementUnlockedDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		return TRUE;
	case WM_INITDIALOG:
		SendMessage(hWnd, WM_SETREDRAW, FALSE, 0);

		Achievements::Achievement* UnlockedAchievement = &Achievements::Entries[(int)lParam];
		SetDlgItemText(hWnd, LBL_TITLE, UnlockedAchievement->Name);
		Edit_SetText(GetDlgItem(hWnd, EDIT_DESC), UnlockedAchievement->Description);
		SendMessage(GetDlgItem(hWnd, EDIT_DESC), WM_VSCROLL, SB_BOTTOM, (LPARAM)NULL);
		Edit_SetSel(GetDlgItem(hWnd, EDIT_DESC), -1, -1);

		SendMessage(hWnd, WM_SETREDRAW, TRUE, 0);
		MessageBeep(MB_ICONASTERISK);
		break;
	}

	return FALSE;
}

LRESULT CALLBACK LandscapeTextureUseSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case LVN_ITEMACTIVATE:
			NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
			char Buffer[0x200] = {0};
			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 0, Buffer, sizeof(Buffer));

			TESForm* Texture = TESForm::LookupByEditorID(Buffer);
			if (Texture)
			{
				*g_ActiveLandscapeEditTexture = CS_CAST(Texture, TESForm, TESLandTexture);
				SendMessage(*g_HWND_LandscapeEdit, 0x41A, NULL, NULL);			// select the new texture in the landscape edit dialog
				SetForegroundWindow(*g_HWND_RenderWindow);
				PrintToRender("Active landscape texture changed", 3);
			}
			break;
		}
		break;
	case WM_DESTROY:
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_LandscapeTextureUseOrgWindowProc);
		break;
	}

	return CallWindowProc(g_LandscapeTextureUseOrgWindowProc, hWnd, uMsg, wParam, lParam);
}

BOOL CALLBACK EditResultScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BTN_CANCEL:
			EndDialog(hWnd, NULL);
			return TRUE;
		case BTN_COMPILE:
		case BTN_SAVE:
			char Buffer[0x1000] = {0};
			HWND Parent = (HWND)GetWindowLong(hWnd, GWL_USERDATA);

			GetDlgItemText(hWnd, EDIT_SCRIPTTEXT, Buffer, sizeof(Buffer));
			SetDlgItemText(Parent, 1444, (LPSTR)Buffer);

			if (LOWORD(wParam) == BTN_COMPILE)
				EndDialog(hWnd, (INT_PTR)1);
			else
				EndDialog(hWnd, NULL);

			return TRUE;
		}
		break;
	case WM_INITDIALOG:
		PARAFORMAT FormattingData = {0};

		FormattingData.cbSize = sizeof(PARAFORMAT);
		FormattingData.dwMask = PFM_TABSTOPS;
		FormattingData.cTabCount = 32;

		for (int i = 0, j = 300; i < sizeof(FormattingData.rgxTabs); i++, j += 300)
			FormattingData.rgxTabs[i] = j;

		SendDlgItemMessage(hWnd, EDIT_SCRIPTTEXT, EM_SETPARAFORMAT, NULL, (LPARAM)&FormattingData);
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)lParam);

		HWND ResultScriptEditBox = GetDlgItem((HWND)lParam, 1444);
		char Buffer[0x1000] = {0};

		GetWindowText(ResultScriptEditBox, Buffer, sizeof(Buffer));
		SetDlgItemText(hWnd, EDIT_SCRIPTTEXT, (LPSTR)Buffer);
		break;
	}
	return FALSE;
}

void InitializeWindowManager(void)
{
	HMENU MainMenu = GetMenu(*g_HWND_CSParent),
		  EditMenu = GetSubMenu(MainMenu, 1),
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
				ItemDataSetWorkspace;
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

	HMENU SaveOptionsMenu = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU8); SaveOptionsMenu = GetSubMenu(SaveOptionsMenu, 0);
	InsertMenu(FileMenu, 40127, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)SaveOptionsMenu, "Save Options");
	if (g_INIManager->GetINIInt("SaveLoadedESPsAsMasters"))
		CheckMenuItem(SaveOptionsMenu, ID_SAVEOPTIONS_SAVELOADEDESP, MF_CHECKED);
	if (g_INIManager->GetINIInt("PreventTimeStampChanges"))
		CheckMenuItem(SaveOptionsMenu, ID_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS, MF_CHECKED);

	ItemDataSaveAs.cbSize = sizeof(MENUITEMINFO);
	ItemDataSaveAs.fMask = MIIM_ID|MIIM_STATE|MIIM_STRING;
	ItemDataSaveAs.wID = MAIN_DATA_SAVEAS;
	ItemDataSaveAs.fState = MFS_ENABLED;
	ItemDataSaveAs.dwTypeData = "Save As";
	ItemDataSaveAs.cch = 7;
	InsertMenuItem(FileMenu, 3, TRUE, &ItemDataSaveAs);

	HMENU AlignMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU2); AlignMenuPopup = GetSubMenu(AlignMenuPopup, 0);
	HMENU GroupMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU3); GroupMenuPopup = GetSubMenu(GroupMenuPopup, 0);
	HMENU FreezeMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU4); FreezeMenuPopup = GetSubMenu(FreezeMenuPopup, 0);
	HMENU VisibilityMenuPopup = LoadMenu(g_DLLInstance, (LPSTR)IDR_MENU5); VisibilityMenuPopup = GetSubMenu(VisibilityMenuPopup, 0);

	InsertMenu(*g_RenderWindowPopup, -1, MF_BYPOSITION|MF_SEPARATOR, NULL, NULL);
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)AlignMenuPopup, "Selection Alignment");
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)GroupMenuPopup, "Selection Grouping");
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)FreezeMenuPopup, "Selection Freezing");
	InsertMenu(*g_RenderWindowPopup, -1, MF_BYCOMMAND|MF_POPUP|MF_STRING, (UINT_PTR)VisibilityMenuPopup, "Selection Visibility");

	// remove redundant menu items
	DeleteMenu(EditMenu, 40101, MF_BYCOMMAND);
	DeleteMenu(EditMenu, 40192, MF_BYCOMMAND);
	DeleteMenu(EditMenu, 40193, MF_BYCOMMAND);
	DeleteMenu(ViewMenu, 40028, MF_BYCOMMAND);

	DrawMenuBar(*g_HWND_CSParent);

	g_RenderWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_RenderWindow, GWL_WNDPROC, (LONG)RenderWndSubClassProc);
	g_CSMainWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_CSParent, GWL_WNDPROC, (LONG)CSMainWndSubClassProc);
	g_CellViewWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_CellView, GWL_WNDPROC, (LONG)CellViewWndSubClassProc);
	g_ObjectWndOrgWindowProc = (WNDPROC)SetWindowLong(*g_HWND_ObjectWindow, GWL_WNDPROC, (LONG)ObjectWndSubClassProc);

	g_CSDefaultFont = CreateFont(12, 5.5, 0, 0,
                             FW_THIN, FALSE, FALSE, FALSE,
                             ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, 5,	// CLEARTYPE_QUALITY
                             FF_DONTCARE, "MS Shell Dlg");

	SetTimer(*g_HWND_RenderWindow, 1, g_INIManager->GetINIInt("UpdatePeriod"), NULL);
	g_TagBrowserOrgWindowProc = (WNDPROC)SetWindowLong(CLIWrapper::Interfaces::TAG->GetFormDropParentHandle(), GWL_WNDPROC, (LONG)TagBrowserSubClassProc);
	g_DragDropSupportDialogs.AddHandle(CLIWrapper::Interfaces::TAG->GetFormDropWindowHandle());

	// make sure new controls, if any, are added to the main windows
	TESDialog::DeinitializeCSWindows();
	TESDialog::InitializeCSWindows();

	// resize the cell view window a bit to fix the layout
	RECT CellViewBounds = {0};
	GetClientRect(*g_HWND_CellView, &CellViewBounds);
	SendMessage(*g_HWND_CellView, WM_SIZE, (WPARAM)SIZE_MAXSHOW, MAKELPARAM(CellViewBounds.right - CellViewBounds.left + 1, CellViewBounds.bottom - CellViewBounds.top - 1));
	SendMessage(*g_HWND_CellView, WM_SIZE, (WPARAM)SIZE_MAXSHOW, MAKELPARAM(CellViewBounds.right - CellViewBounds.left - 1, CellViewBounds.bottom - CellViewBounds.top + 1));
	RedrawWindow(*g_HWND_CellView, NULL, NULL, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE|RDW_ALLCHILDREN);
}