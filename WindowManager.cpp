#include "ExtenderInternals.h"
#include "WindowManager.h"
#include "MiscHooks.h"
#include "Common\HandShakeStructs.h"
#include "Common\CLIWrapper.h"

WNDPROC						g_FindTextOrgWindowProc = NULL;
WNDPROC						g_DataDlgOrgWindowProc = NULL;
WNDPROC						g_CSMainWndOrgWindowProc = NULL;
WNDPROC						g_RenderWndOrgWindowProc = NULL;

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
		case 40128:		// save as menu item
			if (!(*g_dataHandler)->unk8B8.activeFile)		break;

			*g_WorkingFileFlag = 0;
			g_SaveAsRoutine = true;
			char FileName[0x104];
			if (SelectTESFileCommonDialog(hWnd, g_INI_LocalMasterPath->Data, 0, FileName, 0x104)) {
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
			TESObjectCELL* ThisCell = (*g_TES)->currentInteriorCell;
			if (!ThisCell)	ThisCell = (*g_TES)->currentExteriorCell;

			struct RendSel // 0x18 ; ctor = 0x511A20
			{
				void*		unk00;			// 00 - sel head?
				UInt32		unk04;			// 04 - sel count
				UInt32		unk08;			// 08 init to 0x00A8AF64
				UInt32		unk0C;			// 0C
				UInt32		unk10;			// 10 init to 0x00A8AF6C
				double		unk14;			// 14 init to 0.0
			} **TestX = (RendSel**)0x00A0AF60;

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
					ThisRefData->Selected = false;							// TODO: Figure out where the selected objects linked lists exists
					ThisRefData->ParentREFR = ThisRef;

					i++;
					ThisNode = ThisNode->Next();
				}
				
				BatchData->CellObjectListHead = RefData;
				BatchData->ObjectCount = RefCount;

				if (CLIWrapper::BE_InitializeRefBatchEditor(BatchData)) {
					for (UInt32 k = 0; k < RefCount; k++) {
						ThisRef = (TESObjectREFR*)RefData[k].ParentREFR;
						ThisRefData = &RefData[i];
	
						if (ThisRefData->Selected) {
							if (BatchData->Use3DData) {
								ThisRef->posX = BatchData->World3DData.PosX;
								ThisRef->posY = BatchData->World3DData.PosY;
								ThisRef->posZ = BatchData->World3DData.PosZ;

								ThisRef->rotX = BatchData->World3DData.RotX;
								ThisRef->rotY = BatchData->World3DData.RotY;
								ThisRef->rotZ = BatchData->World3DData.RotZ;

								ThisRef->scale = BatchData->World3DData.Scale;
							}

							if (BatchData->UseFlags) {
								ToggleFlag(&ThisRef->flags, TESObjectREFR::kFlags_Persistent, BatchData->Flags.Persistent);
								ToggleFlag(&ThisRef->flags, TESObjectREFR::kFlags_Disabled, BatchData->Flags.Disabled);
								ToggleFlag(&ThisRef->flags, TESObjectREFR::kFlags_VWD, BatchData->Flags.VWD);
							}
						}
					}

					*g_ActiveChangesFlag = 1;
				//	SendMessageA(*g_HWND_RenderWindow, 0x419u, 6u, 1);
				//	SendMessageA(*g_HWND_RenderWindow, 0x419u, 5u, 0);
				//	InvalidateRect(*g_HWND_RenderWindow, 0, 1);

					
					UInt8* X1 = (UInt8*)0x00A0BC4C, *Y0 = (UInt8*)0x009ED634, *Z0 = (UInt8*)0x00A0BBCC;
				//	*X1 = 1;
				//	*Y0 = 0;
				//	*Z0 = 0;
				//	SendMessage(*g_HWND_RenderWindow, 0x111, 0, 0);

				//	SendMessage(*g_HWND_RenderWindow, 0x417u, 0, 0);
				//	thisVirtualCall(0x00957E0C, 0x70, ThisCell);
				//	SendMessageA(*g_HWND_RenderWindow, 0x40Du, 0, (LPARAM)((void**)0x00A8AF64));
				//	SendMessageA(*g_HWND_RenderWindow, 0x419u, 6u, 0);
				//	SendMessage(*g_HWND_RenderWindow, 0x40Au, 0, 0);
				//	SendMessageA(*g_HWND_RenderWindow, 0x419u, 5u, 0);

				
				}

				delete [] RefData;
				delete BatchData;
			}
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
		break;
	case WM_DESTROY: 
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)g_RenderWndOrgWindowProc);
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