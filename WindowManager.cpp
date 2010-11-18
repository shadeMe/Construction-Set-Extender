#include "ExtenderInternals.h"
#include "WindowManager.h"
#include "MiscHooks.h"
#include "Common\HandShakeStructs.h"
#include "Common\CLIWrapper.h"
#include "resource.h"

WNDPROC						g_FindTextOrgWindowProc = NULL;
WNDPROC						g_DataDlgOrgWindowProc = NULL;
WNDPROC						g_CSMainWndOrgWindowProc = NULL;
WNDPROC						g_RenderWndOrgWindowProc = NULL;
WNDPROC						g_ConsoleWndOrgWindowProc = NULL;
WNDPROC						g_ConsoleEditControlOrgWindowProc = NULL;

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
			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 0, g_Buffer, sizeof(g_Buffer));
			std::string EditorID, FormTypeStr(g_Buffer);

			ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 1, g_Buffer, sizeof(g_Buffer));
			EditorID = g_Buffer;
			UInt32 PadStart = EditorID.find("'") + 1, PadEnd  = EditorID.find("'", PadStart + 1);
			if (PadStart != std::string::npos && PadEnd != std::string::npos) {
				EditorID = EditorID.substr(PadStart, PadEnd - PadStart);
				LoadFormIntoView(EditorID.c_str(), FormTypeStr.c_str());
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
	switch (uMsg)
	{
	case WM_COMMAND:
		break;
	case WM_DESTROY: 
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_DataDlgOrgWindowProc);
		break; 
	}
 
	return CallWindowProc(g_DataDlgOrgWindowProc, hWnd, uMsg, wParam, lParam); 
} 
LRESULT CALLBACK CSMainWndSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
	switch (uMsg)
	{
	case 0x40C:			// save handler
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
		case 9901:		// save as menu item
			if (!(*g_dataHandler)->unk8B8.activeFile)		break;

			*g_WorkingFileFlag = 0;
			g_SaveAsRoutine = true;
			char FileName[0x104];
			if (SelectTESFileCommonDialog(hWnd, g_LocalMasterPath->Data, 0, FileName, 0x104)) {
				g_SaveAsBuffer = (*g_dataHandler)->unk8B8.activeFile;

				g_SaveAsBuffer->flags &= ~(1 << 3);			// clear active flag
				(*g_dataHandler)->unk8B8.activeFile = NULL;

				if (SendMessage(*g_HWND_CSParent, 0x40C, NULL, (LPARAM)FileName)) {
					sub_4306F0(false);
					g_SaveAsBuffer->flags &= ~(1 << 2);		// clear loaded flag
				} else {
					(*g_dataHandler)->unk8B8.activeFile = g_SaveAsBuffer;
					g_SaveAsBuffer->flags |= 1 << 3;
				}
			}
			*g_WorkingFileFlag = 1;
			g_SaveAsRoutine = false;
			break;
		case 9902:				// batch edit menu item
			{
			TESObjectCELL* ThisCell = (*g_TES)->currentInteriorCell;
			if (!ThisCell)	ThisCell = (*g_TES)->currentExteriorCell;

			if (ThisCell) {
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
				while (ThisNode) {
					ThisRef = ThisNode->refr;
					if (!ThisRef)		break;
					ThisRefData = &RefData[i];

					ThisRefData->EditorID = (!ThisRef->editorData.editorID.m_data)?ThisRef->baseForm->editorData.editorID.m_data:ThisRef->editorData.editorID.m_data;
					ThisRefData->FormID = ThisRef->refID;
					ThisRefData->TypeID = ThisRef->baseForm->typeID;
					ThisRefData->Flags = ThisRef->flags;
					ThisRefData->Selected = false;
					for (TESRenderWindowBuffer::SelectedObjectsEntry* j = (*g_TESRenderWindowBuffer)->RenderSelection; j != 0; j = j->Next) {
						if (j->Data && j->Data == ThisRef) {
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

				if (CLIWrapper::BatchEditor::InitializeRefBatchEditor(BatchData)) {
					for (UInt32 k = 0; k < RefCount; k++) {
						ThisRef = (TESObjectREFR*)RefData[k].ParentForm;
						ThisRefData = &RefData[k];
						bool Modified = false;
															// Pass the Actor VTBl for actors
						if (ThisRefData->Selected) {		// TODO: filter out ref types that don't have ownership extradata and count
							if (BatchData->World3DData.UsePosX())	ThisRef->posX = BatchData->World3DData.PosX, Modified = true;
							if (BatchData->World3DData.UsePosY())	ThisRef->posY = BatchData->World3DData.PosY, Modified = true;
							if (BatchData->World3DData.UsePosZ())	ThisRef->posZ = BatchData->World3DData.PosZ, Modified = true;

							if (BatchData->World3DData.UseRotX())	ThisRef->rotX = BatchData->World3DData.RotX * PI / 180, Modified = true;
							if (BatchData->World3DData.UseRotY())	ThisRef->rotY = BatchData->World3DData.RotY * PI / 180, Modified = true;
							if (BatchData->World3DData.UseRotZ())	ThisRef->rotZ = BatchData->World3DData.RotZ * PI / 180, Modified = true;

							if (BatchData->World3DData.UseScale())	ThisRef->scale = BatchData->World3DData.Scale, Modified = true;

							if (BatchData->Flags.UsePersistent() && 
								ThisRef->baseForm->typeID != kFormType_NPC && 
								ThisRef->baseForm->typeID != kFormType_Creature)	ToggleFlag(&ThisRef->flags, TESObjectREFR::kFlags_Persistent, BatchData->Flags.Persistent), Modified = true;
							if (BatchData->Flags.UseDisabled())		ToggleFlag(&ThisRef->flags, TESObjectREFR::kFlags_Disabled, BatchData->Flags.Disabled), Modified = true;
							if (BatchData->Flags.UseVWD())			ToggleFlag(&ThisRef->flags, TESForm::kFormFlags_VisibleWhenDistant, BatchData->Flags.VWD), Modified = true;

							if (BatchData->EnableParent.UseEnableParent()) {
								TESObjectREFR* Parent = (TESObjectREFR*)BatchData->EnableParent.Parent;
								if (Parent != ThisRef) {
									thisCall(kBaseExtraList_ModExtraEnableStateParent, &ThisRef->baseExtraList, Parent);
									thisCall(kTESObjectREFR_SetExtraEnableStateParent_OppositeState, ThisRef, BatchData->EnableParent.OppositeState);
									Modified = true;
								}
							}

							if (BatchData->Ownership.UseOwnership() &&
								ThisRef->baseForm->typeID != kFormType_NPC && 
								ThisRef->baseForm->typeID != kFormType_Creature) {
							//	ThisRef->baseExtraList.ModExtraOwnership(NULL);
							//	ThisRef->baseExtraList.ModExtraGlobal(NULL);
							//	ThisRef->baseExtraList.ModExtraRank(-1);

								TESForm* Owner = (TESForm*)BatchData->Ownership.Owner;
								if (BatchData->Ownership.UseNPCOwner()) {
						//			thisCall(kBaseExtraList_ModExtraGlobal, &ThisRef->baseExtraList, (TESGlobal*)BatchData->Ownership.Global);
								} else {
						//			thisCall(kBaseExtraList_ModExtraRank, &ThisRef->baseExtraList, BatchData->Ownership.Rank);								
								}
								Modified = true;
							}

							if (BatchData->Extra.UseCharge())		thisCall(kTESObjectREFR_ModExtraCharge, ThisRef, BatchData->Extra.Charge), Modified = true;
							if (BatchData->Extra.UseHealth())		thisCall(kTESObjectREFR_ModExtraHealth, ThisRef, BatchData->Extra.Health), Modified = true;
							if (BatchData->Extra.UseTimeLeft())		thisCall(kTESObjectREFR_ModExtraTimeLeft, ThisRef, BatchData->Extra.TimeLeft), Modified = true;
							if (BatchData->Extra.UseSoulLevel())	thisCall(kTESObjectREFR_ModExtraSoul, ThisRef, BatchData->Extra.SoulLevel), Modified = true;
							if (BatchData->Extra.UseCount()) {
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

						if (Modified) {
					//		if (!thisVirtualCall(kVTBL_TESObjectREFR, 0xBC, ThisRef, (TESForm*)ThisRef))
					//			DebugPrint("REF: %08X | virt BC returned false!", ThisRef->refID);
					//		thisVirtualCall(kVTBL_TESObjectREFR, 0x104, ThisRef);
					//		thisVirtualCall(kVTBL_TESObjectREFR, 0x94, ThisRef, 1);	// SetFromActiveFile(bool fromActiveFile);
					//		thisVirtualCall(kVTBL_TESObjectREFR, 0xB8, ThisRef, (TESForm*)ThisRef);
					//		thisVirtualCall(kVTBL_TESObjectREFR, 0x17C, ThisRef, thisCall(0x00542950, ThisRef));
						}
					}			
				}

				delete [] RefData;
				delete BatchData;
			}
			break;
			}
		case 9903:				// console window menu item
			if (CONSOLE->IsConsoleInitalized()) {
				HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
				if (CONSOLE->ToggleDisplayState())
					CheckMenuItem(ViewMenu, 9903, MF_CHECKED);
				else
					CheckMenuItem(ViewMenu, 9903, MF_UNCHECKED);
			}
			break;
		case 9904:				// hide unmodified forms item
			{
			HMENU MainMenu = GetMenu(*g_HWND_CSParent), ViewMenu = GetSubMenu(MainMenu, 2);
			if (AreUnModifiedFormsHidden())
			{
				// show all forms
				ToggleHideUnModifiedForms(false);
				CheckMenuItem(ViewMenu, 9904, MF_UNCHECKED);
			}
			else
			{
				// only show active forms
				ToggleHideUnModifiedForms(true);
				CheckMenuItem(ViewMenu, 9904, MF_CHECKED);		
			}
			break;
			}
		case 9905:				// CSE preferences item
			g_INIEditGUI->InitializeGUI(g_DLLInstance, *g_HWND_CSParent, g_INIManager);
			break;
		}
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
		case 9902:	
			SendMessage(*g_HWND_CSParent, WM_COMMAND, 9902, 0);
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
			GetDlgItemText(hWnd, EDIT_TEXTLINE, g_Buffer, sizeof(g_Buffer));
			EndDialog(hWnd, (INT_PTR)g_Buffer);
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

LRESULT CALLBACK ConsoleDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
		{
		tagRECT WindowRect;
		GetWindowRect(hWnd, &WindowRect);
		MoveWindow(GetDlgItem(hWnd, EDIT_CONSOLE), 0, 0, WindowRect.right - WindowRect.left - 9, WindowRect.bottom  - WindowRect.top - 27, TRUE);
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
				if (AlwaysOnTopFlag) {
					SetWindowPos(CONSOLE->GetWindowHandle(), HWND_NOTOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
					AlwaysOnTopFlag = false;
				}
				else {
					SetWindowPos(CONSOLE->GetWindowHandle(), HWND_TOPMOST, 0, 1, 1, 1, SWP_NOMOVE|SWP_NOSIZE|SWP_SHOWWINDOW);
					AlwaysOnTopFlag = true;
				}
				break;
			case CONSOLEMENU_HIDECONSOLE:
				CONSOLE->ToggleDisplayState();
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