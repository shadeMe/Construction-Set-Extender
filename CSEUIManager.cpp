#include "CSEUIManager.h"
#include "Construction Set Extender_Resource.h"
#include "AuxiliaryViewport.h"
#include "[Common]\CLIWrapper.h"
#include "CSEWorkspaceManager.h"
#include "CSERenderWindowPainter.h"
#include "CSEAchievements.h"
#include "Hooks\AssetSelector.h"
#include "Hooks\Dialog.h"
#include "Hooks\Renderer.h"
#include "Hooks\LOD.h"
#include "RenderSelectionGroupManager.h"
#include "PathGridUndoManager.h"
#include "CSInterop.h"

#include <BGSEEToolBox.h>
#include <BGSEEScript\CodaVM.h>

#define PI					3.151592653589793

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kStartupPluginINISettings[kStartupPlugin__MAX] =
		{
			{ "LoadPlugin",							"0",		"Load a plugin on CS startup" },
			{ "PluginName",							"",			"Name of the plugin, with extension, that is to be loaded on startup" }
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetStartupPlugin( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("Startup");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kStartupPlugin__MAX; i++)
					kFactory.Settings.push_back(&kStartupPluginINISettings[i]);
			}

			return &kFactory;
		}

		const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kPluginsINISettings[kPlugins__MAX] =
		{
			{ "PreventTimeStampChanges",			"0",		"Prevents modifications to the timestamps of plugins being saved" },
			{ "SaveLoadedESPsAsMasters",			"1",		"Allows ESP files to be saved as the active plugin's master" }
		};

		BGSEditorExtender::BGSEEINIManagerSettingFactory* GetPlugins( void )
		{
			static BGSEditorExtender::BGSEEINIManagerSettingFactory	kFactory("Plugins");
			if (kFactory.Settings.size() == 0)
			{
				for (int i = 0; i < kPlugins__MAX; i++)
					kFactory.Settings.push_back(&kPluginsINISettings[i]);
			}

			return &kFactory;
		}
	}

	namespace UIManager
	{
#define ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID				0x99

		CSEFilterableFormListManager		CSEFilterableFormListManager::Instance;

		CSEFilterableFormListManager::FilterableWindowData::FilterableWindowData( HWND Parent, HWND EditBox, HWND FormList, bool RefList, int TimerPeriod ) :
			ParentWindow(Parent),
			FilterEditBox(EditBox),
			FormListView(FormList),
			FormListWndProc(NULL),
			FilterString(""),
			TimerPeriod(TimerPeriod),
			TimeCounter(-1),
			ObjRefList(RefList)
		{
			SME_ASSERT(ParentWindow && FilterEditBox && FormListView);

			FormListWndProc = (WNDPROC)SetWindowLongPtr(FormListView, GWL_WNDPROC, (LONG)FormListSubclassProc);
			SetWindowLongPtr(FormListView, GWL_USERDATA, (LONG)this);

			SetTimer(ParentWindow, ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID, TimerPeriod, NULL);
		}

		CSEFilterableFormListManager::FilterableWindowData::~FilterableWindowData()
		{
			KillTimer(ParentWindow, ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID);
			SetWindowLongPtr(FormListView, GWL_WNDPROC, (LONG)FormListWndProc);
		}

		LRESULT CALLBACK CSEFilterableFormListManager::FilterableWindowData::FormListSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			FilterableWindowData* UserData = (FilterableWindowData*)GetWindowLongPtr(hWnd, GWL_USERDATA);

			switch (uMsg)
			{
			case LVM_INSERTITEMA:
			case LVM_INSERTITEMW:
				LVITEM* Item = (LVITEM*)lParam;

				if (Item->lParam)
				{
					TESForm* Form = (TESForm*)Item->lParam;
					if (Form->editorID.c_str() == NULL && UserData->ObjRefList)
					{
						TESObjectREFR* Ref = CS_CAST(Form, TESForm, TESObjectREFR);
						if (Ref)
							Form = Ref->baseForm;
					}

					if (UserData->FilterString.size() && Form && Form->editorID.c_str())
					{
						std::string Buffer(Form->editorID.c_str());
						SME::StringHelpers::MakeLower(Buffer);

						if (Buffer.find(UserData->FilterString) == std::string::npos)
						{
							return -1;		// couldn't find the filter string, so skip insertion
						}
					}
				}

				break;
			}

			return CallWindowProc(UserData->FormListWndProc, hWnd, uMsg, wParam, lParam);
		}

		bool CSEFilterableFormListManager::FilterableWindowData::HandleMessages( HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == EN_CHANGE &&
					LOWORD(wParam) == IDC_CSEFILTERABLEFORMLIST_FILTEREDIT)
				{
					TimeCounter = 0;
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_CSEFILTERABLEFORMLIST_FILTERINPUTTIMERID:
					if (TimeCounter && TimeCounter != -1 && TimeCounter >= TimerPeriod * 1.5)
					{
						char Buffer[0x200] = {0};
						GetWindowText(FilterEditBox, (LPSTR)Buffer, sizeof(Buffer));

						if (strlen(Buffer))
						{
							FilterString = Buffer;
							SME::StringHelpers::MakeLower(FilterString);
						}
						else
						{
							FilterString = "";
						}

						TimeCounter = -1;
						return true;
					}
					else if (TimeCounter != -1)
					{
						TimeCounter += TimerPeriod;
					}

					break;
				}

				break;
			}

			return false;
		}

		CSEFilterableFormListManager::CSEFilterableFormListManager() :
			ActiveWindows()
		{
			;//
		}

		CSEFilterableFormListManager::~CSEFilterableFormListManager()
		{
			for (FilterDataMapT::iterator Itr = ActiveWindows.begin(); Itr != ActiveWindows.end(); Itr++)
				delete Itr->second;

			ActiveWindows.clear();
		}

		bool CSEFilterableFormListManager::Register( HWND Window, HWND FilterEdit, HWND FormList, bool ObjRefList /*= false*/, int TimePeriod /*= 500*/ )
		{
			SME_ASSERT(Window);

			if (ActiveWindows.count(Window) == 0)
			{
				ActiveWindows.insert(std::make_pair<HWND, FilterableWindowData*>(Window,
																			new FilterableWindowData(Window, FilterEdit, FormList, ObjRefList, TimePeriod)));
				return true;
			}

			return false;
		}

		bool CSEFilterableFormListManager::Unregister( HWND Window )
		{
			SME_ASSERT(Window);

			FilterDataMapT::iterator Match = ActiveWindows.find(Window);
			if (Match != ActiveWindows.end())
			{
				delete Match->second;
				ActiveWindows.erase(Match);
				return true;
			}

			return false;
		}

		bool CSEFilterableFormListManager::HandleMessages( HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			SME_ASSERT(Window);

			if (ActiveWindows.count(Window))
			{
				return ActiveWindows[Window]->HandleMessages(Window, uMsg, wParam, lParam);
			}

			return false;
		}

		CSEFormEnumerationManager		CSEFormEnumerationManager::Instance;

		CSEFormEnumerationManager::CSEFormEnumerationManager() :
			VisibilityDeletedForms(true),
			VisibilityUnmodifiedForms(true)
		{
			;//
		}

		CSEFormEnumerationManager::~CSEFormEnumerationManager()
		{
			;//
		}

		inline bool CSEFormEnumerationManager::GetVisibleDeletedForms( void ) const
		{
			return VisibilityDeletedForms == true;
		}

		inline bool CSEFormEnumerationManager::GetVisibleUnmodifiedForms( void ) const
		{
			return VisibilityUnmodifiedForms == true;
		}

		bool CSEFormEnumerationManager::ToggleVisibilityUnmodifiedForms( void )
		{
			VisibilityUnmodifiedForms = (VisibilityUnmodifiedForms == false);
			TESDialog::ResetFormListControls();

			return VisibilityUnmodifiedForms;
		}

		bool CSEFormEnumerationManager::ToggleVisibilityDeletedForms( void )
		{
			VisibilityDeletedForms = (VisibilityDeletedForms == false);
			TESDialog::ResetFormListControls();

			return VisibilityDeletedForms;
		}

		bool CSEFormEnumerationManager::GetShouldEnumerate( TESForm* Form )
		{
			if (Form && (Form->formFlags & TESForm::kFormFlags_FromActiveFile) == 0 && GetVisibleUnmodifiedForms() == false)
				return false;
			else if (Form && (Form->formFlags & TESForm::kFormFlags_Deleted) && GetVisibleDeletedForms() == false)
				return false;
			else
				return true;
		}

		void CSEFormEnumerationManager::ResetVisibility( void )
		{
			VisibilityDeletedForms = true;
			VisibilityUnmodifiedForms = true;
		}

		int CSEFormEnumerationManager::CompareActiveForms( TESForm* FormA, TESForm* FormB, int OriginalResult )
		{
			int Result = OriginalResult;
			bool Enabled = atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_SortFormListsByActiveForm, BGSEEMAIN->INIGetter()));

			if (FormA && FormB)
			{
				bool ActiveFormA = (FormA->formFlags & TESForm::kFormFlags_FromActiveFile);
				bool ActiveFormB = (FormB->formFlags & TESForm::kFormFlags_FromActiveFile);

				if (Enabled)
				{
					if (ActiveFormA == true && ActiveFormB == false)
						Result = -1;
					else if (ActiveFormA == false && ActiveFormB == true)
						Result = 1;
				}
			}

			return Result;
		}

		CSEWindowInvalidationManager		CSEWindowInvalidationManager::Instance;

		CSEWindowInvalidationManager::CSEWindowInvalidationManager() :
			ActiveInvalidatedWindows()
		{
			;//
		}

		CSEWindowInvalidationManager::~CSEWindowInvalidationManager()
		{
			for (InvalidationMapT::iterator Itr = ActiveInvalidatedWindows.begin(); Itr != ActiveInvalidatedWindows.end(); Itr++)
				Invalidate(Itr->first, false);

			ActiveInvalidatedWindows.clear();
		}

		void CSEWindowInvalidationManager::Push( HWND Window )
		{
			SME_ASSERT(Window);

			if (ActiveInvalidatedWindows.count(Window) == 0)
			{
				ActiveInvalidatedWindows.insert(std::make_pair<HWND, UInt32>(Window, 1));
				Invalidate(Window, true);
			}
			else
				ActiveInvalidatedWindows[Window] += 1;
		}

		void CSEWindowInvalidationManager::Pop( HWND Window )
		{
			SME_ASSERT(Window);
			SME_ASSERT(ActiveInvalidatedWindows.count(Window));

			UInt32 RefCount = ActiveInvalidatedWindows[Window];
			SME_ASSERT(RefCount);

			if (RefCount == 1)
			{
				Invalidate(Window, false);
				ActiveInvalidatedWindows.erase(Window);
			}
			else
				ActiveInvalidatedWindows[Window] -= 1;
		}

		void CSEWindowInvalidationManager::Invalidate( HWND Window, bool State )
		{
			SME_ASSERT(Window);

			if (State)
			{
				SendMessage(Window, WM_SETREDRAW, FALSE, NULL);
			}
			else
			{
				SendMessage(Window, WM_SETREDRAW, TRUE, NULL);
				RedrawWindow(Window, NULL, NULL, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE|RDW_ALLCHILDREN);
			}
		}

		CSEDialogExtraFittingsData::CSEDialogExtraFittingsData()
		{
			QuickViewCursorPos.x = QuickViewCursorPos.y = 0;
			QuickViewWindowUnderCursor = NULL;

			ActiveFormListFont = NULL;

			ActiveFormListFont = CreateFont(14,
											0,
											0,
											0,
											FW_MEDIUM,
											FALSE,
											FALSE,
											FALSE,
											DEFAULT_CHARSET,
											OUT_DEFAULT_PRECIS,
											CLIP_DEFAULT_PRECIS,
											ANTIALIASED_QUALITY,
											FF_DONTCARE,
											"MS Shell Dlg");
		}

		CSEDialogExtraFittingsData::~CSEDialogExtraFittingsData()
		{
			if (ActiveFormListFont)
				DeleteFont(ActiveFormListFont);
		}

		CSEMainWindowMiscExtraData::CSEMainWindowMiscExtraData()
		{
			ToolbarExtras = Subwindow::CreateInstance();
		}

		CSEMainWindowMiscExtraData::~CSEMainWindowMiscExtraData()
		{
			if (ToolbarExtras)
			{
				ToolbarExtras->TearDown();
				ToolbarExtras->DeleteInstance();
			}
		}

		LRESULT CALLBACK FindTextDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

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
					int PadStart = FormID.find("(") + 1, PadEnd = FormID.find(")", PadStart + 1);

					if (PadStart != std::string::npos && PadEnd != std::string::npos)
					{
						FormID = FormID.substr(PadStart, PadEnd - PadStart);
						UInt32 FormIDInt = 0;
						sscanf_s(FormID.c_str(), "%08X", &FormIDInt);
						TESForm* Form = TESForm::LookupByFormID(FormIDInt);

						if (Form)
						{
							switch (Form->formType)
							{
							case TESForm::kFormType_Script:
								TESDialog::ShowScriptEditorDialog(Form);
								break;
							case TESForm::kFormType_REFR:
								_TES->LoadCellIntoViewPort((CS_CAST(Form, TESForm, TESObjectREFR))->GetPosition(), CS_CAST(Form, TESForm, TESObjectREFR));
								break;
							default:
								TESDialog::ShowFormEditDialog(Form);
								break;
							}
						}
					}

					break;
				}

				break;
			case WM_DESTROY:
				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK DataDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

			TESFile* ActiveTESFile = *((TESFile**)0x00A0AA7C);
			HWND PluginList = GetDlgItem(hWnd, 1056);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 305;
					ListView_SetColumn(PluginList, 0, &ColumnData);

					ColumnData.cx = 65;
					ListView_SetColumn(PluginList, 1, &ColumnData);

					bool LoadStartupPlugin = atoi(INISettings::GetStartupPlugin()->Get(INISettings::kStartupPlugin_LoadPlugin, BGSEEMAIN->INIGetter()));
					if (LoadStartupPlugin)
						CheckDlgButton(hWnd, IDC_CSE_DATA_LOADSTARTUPPLUGIN, BST_CHECKED);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case NM_CUSTOMDRAW:
						if (wParam == 1056)
						{
							NMLVCUSTOMDRAW* DrawData = (NMLVCUSTOMDRAW*)lParam;

							switch (DrawData->nmcd.dwDrawStage)
							{
							case CDDS_PREPAINT:
								{
									SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
									DlgProcResult = TRUE;
									Return = true;
								}

								break;
							case CDDS_ITEMPREPAINT:
								{
									UInt32 PluginIndex = (UInt32)DrawData->nmcd.lItemlParam;
									TESFile* CurrentFile = _DATAHANDLER->LookupPluginByIndex(PluginIndex);
									if (CurrentFile)
									{
										bool Update = true;

										if (CurrentFile == ActiveTESFile)
										{
											DrawData->clrTextBk = RGB(227, 183, 251);
										}
										else if (!_stricmp(INISettings::GetStartupPlugin()->Get(INISettings::kStartupPlugin_PluginName,
												BGSEEMAIN->INIGetter()),
												CurrentFile->fileName))
										{
											DrawData->clrTextBk = RGB(248, 227, 186);
										}
										else if (CurrentFile->authorName.c_str() &&
												!_stricmp(CurrentFile->authorName.c_str(), "shadeMe"))
										{
											DrawData->clrTextBk = RGB(249, 255, 255);
										}
										else
										{
											Update = false;
										}

										if (Update)
										{
											SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NEWFONT);
											DlgProcResult = TRUE;
											Return = true;
										}
									}
								}

								break;
							}
						}

						break;
					}
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_DATA_LOADSTARTUPPLUGIN:
					{
						if (IsDlgButtonChecked(hWnd, IDC_CSE_DATA_LOADSTARTUPPLUGIN) == BST_CHECKED)
							INISettings::GetStartupPlugin()->Set(INISettings::kStartupPlugin_LoadPlugin, BGSEEMAIN->INISetter(), "1");
						else
							INISettings::GetStartupPlugin()->Set(INISettings::kStartupPlugin_LoadPlugin, BGSEEMAIN->INISetter(), "0");
					}

					break;
				case IDC_CSE_DATA_SETSTARTUPPLUGIN:
					{
						int SelectedItem = ListView_GetNextItem(PluginList, -1, LVNI_SELECTED);

						if (SelectedItem != -1)
						{
							LVITEM SelectedPluginItem;
							char PluginNameBuffer[0x200] = {0};

							SelectedPluginItem.iItem = SelectedItem;
							SelectedPluginItem.iSubItem = 0;
							SelectedPluginItem.mask = LVIF_TEXT;
							SelectedPluginItem.pszText = PluginNameBuffer;
							SelectedPluginItem.cchTextMax = sizeof(PluginNameBuffer);

							if (ListView_GetItem(PluginList, &SelectedPluginItem) == TRUE)
							{
								INISettings::GetStartupPlugin()->Set(INISettings::kStartupPlugin_PluginName, BGSEEMAIN->INISetter(), PluginNameBuffer);

								BGSEEUI->MsgBoxI(hWnd, 0, "Startup plugin set to '%s'.", PluginNameBuffer);
								BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
							}
						}
					}

					break;
				case 1:		// OK
					if (CLIWrapper::Interfaces::SE->GetOpenEditorCount())
					{
						if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "There are open script windows. Are you sure you'd like to proceed?") == IDNO)
							Return = true;
					}
					else if (ActiveTESFile != NULL && !_stricmp(ActiveTESFile->fileName, "oblivion.esm"))
					{
						if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "You have set Oblvion.esm as an active file. Are you absolutely sure this is the end of the world?") == IDNO)
							Return = true;
						else
							BGSEEACHIEVEMENTS->Unlock(Achievements::kFearless);
					}
					else if (Return == false)
					{
						CSEFormEnumerationManager::Instance.ResetVisibility();
					}

					break;
				}

				break;
			case WM_DESTROY:
				TESFile* ActiveFile = _DATAHANDLER->activeFile;					// required for correct ESM handling

				if (ActiveFile)
					ActiveFile->SetMaster(false);

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowMenuInitSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITMENUPOPUP:
				{
					if (HIWORD(lParam) == FALSE)
					{
						HMENU Popup = (HMENU)wParam;

						for (int i = 0, j = GetMenuItemCount(Popup); i < j; i++ )
						{
							MENUITEMINFO CurrentItem = {0};
							CurrentItem.cbSize = sizeof(MENUITEMINFO);
							CurrentItem.fMask = MIIM_ID|MIIM_STATE;

							if (GetMenuItemInfo(Popup, i, TRUE, &CurrentItem) == TRUE)
							{
								bool UpdateItem = true;
								bool CheckItem = false;

								switch (CurrentItem.wID)
								{
								case 40194:		// Edit Cell Path Grid
									if (*g_RenderWindowPathGridEditModeFlag)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_SAVEESPMASTERS:
									if (atoi(INISettings::GetPlugins()->Get(INISettings::kPlugins_SaveLoadedESPsAsMasters, BGSEEMAIN->INIGetter())))
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS:
									if (atoi(INISettings::GetPlugins()->Get(INISettings::kPlugins_PreventTimeStampChanges, BGSEEMAIN->INIGetter())))
										CheckItem = true;

									break;
								case IDC_MAINMENU_CONSOLE:
									if (BGSEECONSOLE->GetVisible())
										CheckItem = true;

									break;
								case IDC_MAINMENU_AUXVIEWPORT:
									if (AUXVIEWPORT->GetVisible())
										CheckItem = true;

									break;
								case IDC_MAINMENU_HIDEUNMODIFIEDFORMS:
									if (CSEFormEnumerationManager::Instance.GetVisibleUnmodifiedForms() == false)
										CheckItem = true;

									break;
								case IDC_MAINMENU_HIDEDELETEDFORMS:
									if (CSEFormEnumerationManager::Instance.GetVisibleDeletedForms() == false)
										CheckItem = true;

									break;
								case IDC_MAINMENU_CODABACKGROUNDER:
									if (CODAVM->GetBackgrounderState())
										CheckItem = true;

									break;
								default:
									UpdateItem = false;
									break;
								}

								if (UpdateItem)
								{
									if (CheckItem)
									{
										CurrentItem.fState &= ~MFS_UNCHECKED;
										CurrentItem.fState |= MFS_CHECKED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_CHECKED;
										CurrentItem.fState |= MFS_UNCHECKED;
									}

									CurrentItem.fMask = MIIM_STATE;
									SetMenuItemInfo(Popup, i, TRUE, &CurrentItem);
								}
							}
						}
					}

					Return = true;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowMenuSelectSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMMAND:
				Return = true;

				switch (LOWORD(wParam))
				{
				case IDC_MAINMENU_SAVEAS:
					{
						if (_DATAHANDLER->activeFile == NULL)
						{
							BGSEEUI->MsgBoxE("An active plugin must be set before using this tool.");
							break;
						}

						*g_TESCSAllowAutoSaveFlag = 0;

						char FileName[0x104] = {0};

						if (TESDialog::SelectTESFileCommonDialog(hWnd,
																INISettingCollection::Instance->LookupByName("sLocalMasterPath:General")->value.s,
																0,
																FileName,
																0x104))
						{
							TESFile* SaveAsBuffer = _DATAHANDLER->activeFile;

							SaveAsBuffer->SetActive(false);
							SaveAsBuffer->SetLoaded(false);

							_DATAHANDLER->activeFile = NULL;

							if (SendMessage(hWnd, 0x40C, NULL, (LPARAM)FileName))
							{
								TESDialog::SetMainWindowTitleModified(false);
							}
							else
							{
								_DATAHANDLER->activeFile = SaveAsBuffer;

								SaveAsBuffer->SetActive(true);
								SaveAsBuffer->SetLoaded(true);
							}

							BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						}

						*g_TESCSAllowAutoSaveFlag = 1;
					}

					break;
				case IDC_MAINMENU_BATCHREFERENCEEDITOR:
					{
						TESObjectCELL* ThisCell = (*g_TES)->currentInteriorCell;
						if (!ThisCell)
							ThisCell = *g_RenderWindowCurrentlyLoadedCell;

						if (ThisCell)
						{
							UInt32 RefCount = ThisCell->objectList.Count(), i = 0;

							if (RefCount < 2)
								break;

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

								for (TESRenderSelection::SelectedObjectsEntry* j = _RENDERSEL->selectionList; j != 0; j = j->Next)
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
											SME::MiscGunk::ToggleFlag(&ThisRef->formFlags,
																	TESForm::kFormFlags_QuestItem,
																	BatchData->Flags.Persistent);
											Modified = true;
										}

										if (BatchData->Flags.UseDisabled())
											SME::MiscGunk::ToggleFlag(&ThisRef->formFlags,
																	TESForm::kFormFlags_Disabled,
																	BatchData->Flags.Disabled), Modified = true;

										if (BatchData->Flags.UseVWD())
											SME::MiscGunk::ToggleFlag(&ThisRef->formFlags,
																	TESForm::kFormFlags_VisibleWhenDistant,
																	BatchData->Flags.VWD), Modified = true;

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

												break;
											case TESForm::kFormType_Light:
												TESObjectLIGH* Light = CS_CAST(ThisRef->baseForm, TESForm, TESObjectLIGH);
												if (Light)
												{
													if (Light->IsCarriable())
														ThisRef->extraData.ModExtraCount(BatchData->Extra.Count), Modified = true;
												}

												break;
											}
										}
									}

									if (Modified)
									{
										ThisRef->UpdateUsageInfo();
										ThisRef->SetFromActiveFile(true);
										ThisRef->UpdateNiNode();
									}
								}

								BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
							}

							delete BatchData;
						}

						break;
					}
				case IDC_MAINMENU_CONSOLE:
					BGSEECONSOLE->ToggleVisibility();

					break;
				case IDC_MAINMENU_HIDEDELETEDFORMS:
					CSEFormEnumerationManager::Instance.ToggleVisibilityDeletedForms();
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_HIDEUNMODIFIEDFORMS:
					CSEFormEnumerationManager::Instance.ToggleVisibilityUnmodifiedForms();
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_CSEPREFERENCES:
					BGSEEMAIN->ShowPreferencesGUI();

					break;
				case IDC_MAINMENU_UNLOADCURRENTCELL:
					TESDialog::ResetRenderWindow();

					break;
				case IDC_MAINMENU_LAUNCHGAME:
					ShellExecute(NULL,
								"open",
								(LPCSTR)(std::string(std::string(BGSEEMAIN->GetAPPPath()) + "\\" + std::string(BGSEEMAIN->ExtenderGetSEName()) + "_loader.exe")).c_str(),
								NULL,
								NULL,
								SW_SHOW);

					BGSEEACHIEVEMENTS->Unlock(Achievements::kLazyBum);

					break;
				case IDC_MAINMENU_CREATEGLOBALSCRIPT:
					BGSEEUI->ModelessDialog(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_GLOBALSCRIPT), hWnd, (DLGPROC)CreateGlobalScriptDlgProc);
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_TAGBROWSER:
					CLIWrapper::Interfaces::TAG->ShowTagBrowserDialog(hWnd);
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_SETWORKSPACE:
					BGSEEWORKSPACE->SelectCurrentWorkspace();
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_TOOLS:
					BGSEETOOLBOX->ShowToolListMenu(BGSEEMAIN->GetExtenderHandle(), hWnd);
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_CODAMANAGEGLOBALDATASTORE:
					CODAVM->ShowGlobalStoreEditDialog(BGSEEMAIN->GetExtenderHandle(), hWnd);

					break;
				case IDC_MAINMENU_CODABACKGROUNDER:
					CODAVM->ToggleBackgrounderState();

					break;
				case IDC_MAINMENU_SAVEOPTIONS_SAVEESPMASTERS:
					if (atoi(INISettings::GetPlugins()->Get(INISettings::kPlugins_SaveLoadedESPsAsMasters, BGSEEMAIN->INIGetter())))
						INISettings::GetPlugins()->Set(INISettings::kPlugins_SaveLoadedESPsAsMasters, BGSEEMAIN->INISetter(), "0");
					else
						INISettings::GetPlugins()->Set(INISettings::kPlugins_SaveLoadedESPsAsMasters, BGSEEMAIN->INISetter(), "1");

					break;
				case IDC_MAINMENU_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS:
					if (atoi(INISettings::GetPlugins()->Get(INISettings::kPlugins_PreventTimeStampChanges, BGSEEMAIN->INIGetter())))
						INISettings::GetPlugins()->Set(INISettings::kPlugins_PreventTimeStampChanges, BGSEEMAIN->INISetter(), "0");
					else
						INISettings::GetPlugins()->Set(INISettings::kPlugins_PreventTimeStampChanges, BGSEEMAIN->INISetter(), "1");

					break;
				case IDC_MAINMENU_AUXVIEWPORT:
					AUXVIEWPORT->ToggleVisibility();

					break;
				case IDC_MAINMENU_USEINFOLISTING:
					CLIWrapper::Interfaces::USE->ShowUseInfoListDialog(NULL);

					break;
				case IDC_MAINMENU_BATCHLIPGENERATOR:
					{
						if (CSIOM->GetInitialized() == false)
						{
							BGSEEUI->MsgBoxE("The CSInteropManager is not initialized!");
							break;
						}

						bool SkipInactiveTopicInfos = false;
						bool OverwriteExisting = false;

						if (BGSEEUI->MsgBoxI(hWnd,
											MB_YESNO,
											"Only process active topic infos?") == IDYES)
						{
							SkipInactiveTopicInfos = true;
						}

						if (BGSEEUI->MsgBoxI(hWnd,
											MB_YESNO,
											"Overwrite existing LIP files?") == IDYES)
						{
							OverwriteExisting = true;
						}

						HWND IdleWindow = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), hWnd, NULL, NULL);
						IFileStream ExistingFile;
						int BatchGenCounter = 0, FailedCounter = 0;
						bool HasError = false;

						for (tList<TESTopic>::Iterator ItrTopic = _DATAHANDLER->topics.Begin(); ItrTopic.End() == false && ItrTopic.Get(); ++ItrTopic)
						{
							TESTopic* Topic = ItrTopic.Get();
							SME_ASSERT(Topic);

							for (TESTopic::TopicDataListT::Iterator ItrTopicData = Topic->topicData.Begin();
																	ItrTopicData.End() == false && ItrTopicData.Get();
																	++ItrTopicData)
							{
								TESQuest* Quest = ItrTopicData->parentQuest;
								SME_ASSERT(Quest);

								for (int i = 0; i < ItrTopicData->questInfos.numObjs; i++)
								{
									TESTopicInfo* Info = ItrTopicData->questInfos.data[i];
									SME_ASSERT(Info);

									TESFile* OverrideFile = Info->GetOverrideFile(-1);

									if (OverrideFile)
									{
										if (SkipInactiveTopicInfos == false || (Info->formFlags & TESForm::kFormFlags_FromActiveFile))
										{
											for (tList<TESRace>::Iterator ItrRace = _DATAHANDLER->races.Begin();
																		ItrRace.End() == false && ItrRace.Get();
																		++ItrRace)
											{
												TESRace* Race = ItrRace.Get();
												SME_ASSERT(Race);

												int ResponseCounter = 1;
												for (TESTopicInfo::ResponseListT::Iterator ItrResponse = Info->responseList.Begin();
																						ItrResponse.End() == false && ItrResponse.Get();
																						++ItrResponse)
												{
													TESTopicInfo::ResponseData* Response = ItrResponse.Get();
													SME_ASSERT(Response);

													char VoiceFilePath[MAX_PATH] = {0};

													for (int j = 0; j < 2; j++)
													{
														const char* Sex = "M";
														if (j)
															Sex = "F";

														FORMAT_STR(VoiceFilePath, "Data\\Sound\\Voice\\%s\\%s\\%s\\%s_%s_%08X_%u",
																				OverrideFile->fileName,
																				Race->name.c_str(),
																				Sex,
																				Quest->editorID.c_str(),
																				Topic->editorID.c_str(),
																				(Info->formID & 0xFFFFFF),
																				ResponseCounter);

														std::string MP3Path(VoiceFilePath); MP3Path += ".mp3";
														std::string WAVPath(VoiceFilePath); WAVPath += ".wav";
														std::string LIPPath(VoiceFilePath); LIPPath += ".lip";

														if (ExistingFile.Open(MP3Path.c_str()) ||
															ExistingFile.Open(WAVPath.c_str()))
														{
															if (OverwriteExisting || ExistingFile.Open(LIPPath.c_str()) == false)
															{
																if (CSIOM->GenerateLIPSyncFile(VoiceFilePath, Response->responseText.c_str()))
																	BatchGenCounter++;
																else
																{
																	HasError = true;
																	FailedCounter++;
																}
															}
														}
													}

													ResponseCounter++;
												}
											}
										}
									}
								}
							}
						}

						DestroyWindow(IdleWindow);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

						if (HasError)
							BGSEEUI->MsgBoxW("Batch generation completed with some errors!\n\nGenerated: %d files\nFailed: %d Files", BatchGenCounter, FailedCounter);
						else
							BGSEEUI->MsgBoxI("Batch generation completed successfully!\n\nGenerated: %d files.", BatchGenCounter);
					}

					break;
				default:
					Return = false;

					break;
				}

				break;
			}

			return DlgProcResult;
		}

#define ID_PATHGRIDTOOLBARBUTTION_TIMERID		0x99
#define WM_MAINWINDOW_INITEXTRADATA				(WM_USER + 2003)
#define WM_MAINTOOLBAR_SETTOD					(WM_USER + 2004)
		// wParam = position

		LRESULT CALLBACK MainWindowMiscSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					SetTimer(hWnd, ID_PATHGRIDTOOLBARBUTTION_TIMERID, 500, NULL);
					Return = true;
				}

				break;
			case WM_DESTROY:
				{
					KillTimer(hWnd, ID_PATHGRIDTOOLBARBUTTION_TIMERID);
					if (InstanceUserData)
					{
						CSEMainWindowMiscExtraData* IUD = (CSEMainWindowMiscExtraData*)InstanceUserData;
						delete IUD;
					}
				}

				break;
			case WM_MAINWINDOW_INITEXTRADATA:
				{
					if (InstanceUserData == NULL)
					{
						CSEMainWindowMiscExtraData* IUD = new CSEMainWindowMiscExtraData();
						InstanceUserData = (LPARAM)IUD;

						IUD->ToolbarExtras->hInstance = BGSEEMAIN->GetExtenderHandle();
						IUD->ToolbarExtras->hDialog = *g_HWND_MainToolbar;
						IUD->ToolbarExtras->hContainer = *g_HWND_MainToolbar;
						IUD->ToolbarExtras->position.x = 515;
						IUD->ToolbarExtras->position.y = 0;

						if (IUD->ToolbarExtras->Build(IDD_TOOLBAREXTRAS) == false)
						{
							BGSEECONSOLE_ERROR("Couldn't build main window toolbar subwindow!");
						}
						else
						{
							BGSEEUI->GetSubclasser()->RegisterRegularWindowSubclass(*g_HWND_MainToolbar, MainWindowToolbarSubClassProc);

							HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
							HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

							TESDialog::ClampDlgEditField(TODEdit, 0.0, 24.0);

							SendMessage(TODSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 23));
							SendMessage(TODSlider, TBM_SETLINESIZE, NULL, 1);
							SendMessage(TODSlider, TBM_SETPAGESIZE, NULL, 4);

							SendMessage(*g_HWND_MainToolbar, WM_MAINTOOLBAR_SETTOD, _TES->GetSkyTOD() * 4.0, NULL);
						}
					}
				}

				break;
			case WM_TIMER:
				DlgProcResult = TRUE;
				Return = true;

				switch (wParam)
				{
				case 2:				// autosave timer, needs to be handled here as the org wndproc doesn't compare the timerID
					if ((*g_TESCSAllowAutoSaveFlag) != 0 && (*g_TESCSExittingCSFlag) == 0)
					{
						TESDialog::AutoSave();
					}

					break;
				case ID_PATHGRIDTOOLBARBUTTION_TIMERID:
					{
						TBBUTTONINFO PathGridData = {0};
						PathGridData.cbSize = sizeof(TBBUTTONINFO);
						PathGridData.dwMask = TBIF_STATE;

						SendMessage(*g_HWND_MainToolbar, TB_GETBUTTONINFO, 40195, (LPARAM)&PathGridData);
						if ((PathGridData.fsState & TBSTATE_CHECKED) == false && *g_RenderWindowPathGridEditModeFlag)
						{
							PathGridData.fsState |= TBSTATE_CHECKED;
							SendMessage(*g_HWND_MainToolbar, TB_SETBUTTONINFO, 40195, (LPARAM)&PathGridData);
						}
					}

					break;
				}

				break;
			case 0x40C:				// save handler
				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowToolbarSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
			HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

			switch (uMsg)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == EN_CHANGE &&
					LOWORD(wParam) == IDC_TOOLBAR_TODCURRENT &&
					InstanceUserData == 0)
				{
					InstanceUserData = 1;
					float TOD = TESDialog::GetDlgItemFloat(hWnd, IDC_TOOLBAR_TODCURRENT);
					SendMessage(hWnd, WM_MAINTOOLBAR_SETTOD, TOD * 4.0, NULL);
					InstanceUserData = 0;
				}

				break;
			case WM_HSCROLL:
				{
					bool BreakOut = true;

					switch (LOWORD(wParam))
					{
					case TB_BOTTOM:
					case TB_ENDTRACK:
					case TB_LINEDOWN:
					case TB_LINEUP:
					case TB_PAGEDOWN:
					case TB_PAGEUP:
					case TB_THUMBPOSITION:
					case TB_THUMBTRACK:
					case TB_TOP:
						if ((HWND)lParam == TODSlider)
						{
							BreakOut = false;
						}

						break;
					}

					if (BreakOut)
						break;
				}
			case WM_MAINTOOLBAR_SETTOD:
				{
					if (uMsg != WM_HSCROLL)
						SendDlgItemMessage(hWnd, IDC_TOOLBAR_TODSLIDER, TBM_SETPOS, TRUE, (LPARAM)wParam);

					int Position = SendMessage(TODSlider, TBM_GETPOS, NULL, NULL);
					float TOD = Position / 4.0;

					if (TOD > 24.0f)
						TOD = 24.0f;

					_TES->SetSkyTOD(TOD);

					if (InstanceUserData == 0)
						TESDialog::SetDlgItemFloat(hWnd, IDC_TOOLBAR_TODCURRENT, TOD, 2);

					TESDialog::UpdatePreviewWindows();
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK RenderWindowMenuInitSelectSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = TRUE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

			switch (uMsg)
			{
			case WM_INITMENUPOPUP:
				{
					if (HIWORD(lParam) == FALSE)
					{
						HMENU Popup = (HMENU)wParam;

						for (int i = 0, j = GetMenuItemCount(Popup); i < j; i++ )
						{
							MENUITEMINFO CurrentItem = {0};
							CurrentItem.cbSize = sizeof(MENUITEMINFO);
							CurrentItem.fMask = MIIM_ID|MIIM_STATE;

							if (GetMenuItemInfo(Popup, i, TRUE, &CurrentItem) == TRUE)
							{
								bool UpdateItem = true;
								bool CheckItem = false;

								switch (CurrentItem.wID)
								{
								case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
									if (Hooks::g_FreezeInactiveRefs)
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
									if (atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_CoplanarRefDrops, BGSEEMAIN->INIGetter())))
										CheckItem = true;

									break;
								case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
									if (atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter())))
										CheckItem = true;

									break;
								default:
									UpdateItem = false;
									break;
								}

								if (UpdateItem)
								{
									if (CheckItem)
									{
										CurrentItem.fState &= ~MFS_UNCHECKED;
										CurrentItem.fState |= MFS_CHECKED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_CHECKED;
										CurrentItem.fState |= MFS_UNCHECKED;
									}

									CurrentItem.fMask = MIIM_STATE;
									SetMenuItemInfo(Popup, i, TRUE, &CurrentItem);
								}
							}
						}
					}

					Return = true;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
					{
						bool SwitchCY = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						SwitchCY = (SwitchCY == false);

						INISettings::GetRenderer()->Set(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INISetter(), "%d", SwitchCY);

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
					Hooks::g_FreezeInactiveRefs = (Hooks::g_FreezeInactiveRefs == false);

					if (Hooks::g_FreezeInactiveRefs)
						RenderWindowPainter::RenderChannelNotifications->Queue(4, "Inactive references frozen");
					else
						RenderWindowPainter::RenderChannelNotifications->Queue(4, "Inactive references thawed");

					break;
				case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
					if (*g_RenderWindowPathGridEditModeFlag == 0)
					{
						TESObjectCELL* CurrentCell = (*g_TES)->currentInteriorCell;

						if (CurrentCell == NULL)
							CurrentCell = *g_RenderWindowCurrentlyLoadedCell;

						if (CurrentCell)
						{
							TESRenderSelection* Buffer = TESRenderSelection::CreateInstance(_RENDERSEL);

							_RENDERSEL->ClearSelection(true);

							for (TESObjectCELL::ObjectREFRList::Iterator Itr = CurrentCell->objectList.Begin(); !Itr.End(); ++Itr)
							{
								TESObjectREFR* Ref = Itr.Get();

								if (Ref == NULL)
									break;

								if (Buffer->HasObject(Ref) == false)
									_RENDERSEL->AddToSelection(Ref, true);
							}

							Buffer->DeleteInstance();
							TESDialog::RedrawRenderWindow();
						}
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_BATCHREFERENCEEDITOR:
					SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, IDC_MAINMENU_BATCHREFERENCEEDITOR, 0);

					break;
				case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
				case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
					{
						TESObjectCELL* CurrentCell = (*g_TES)->currentInteriorCell;

						if (CurrentCell == NULL)
							CurrentCell = *g_RenderWindowCurrentlyLoadedCell;

						if (CurrentCell)
						{
							for (TESObjectCELL::ObjectREFRList::Iterator Itr = CurrentCell->objectList.Begin(); !Itr.End(); ++Itr)
							{
								TESObjectREFR* Ref = Itr.Get();

								if (Ref == NULL)
									break;

								switch (LOWORD(wParam))
								{
								case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
									SME::MiscGunk::ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_3DInvisible, false);
									SME::MiscGunk::ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Children3DInvisible, false);

									break;
								case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
									SME::MiscGunk::ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Frozen, false);

									break;
								}
							}

							switch (LOWORD(wParam))
							{
							case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
								RenderWindowPainter::RenderChannelNotifications->Queue(3, "Reset visibility flags on the active cell's references");

								break;
							case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
								RenderWindowPainter::RenderChannelNotifications->Queue(3, "Thawed all of the active cell's references");

								break;
							}
						}

						TESDialog::RedrawRenderWindow();
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
				case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
				case IDC_RENDERWINDOWCONTEXT_FREEZE:
				case IDC_RENDERWINDOWCONTEXT_THAW:
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						char Buffer[0x100] = {0};

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
							SME::MiscGunk::ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_3DInvisible,
													!(Ref->formFlags & kTESObjectREFRSpecialFlags_3DInvisible));
		//					FORMAT_STR(Buffer, "Selection '%08X's visibility toggled", Ref->formID);

							break;
						case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
							SME::MiscGunk::ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Children3DInvisible,
													!(Ref->formFlags & kTESObjectREFRSpecialFlags_Children3DInvisible));
		//					FORMAT_STR(Buffer, "Selection '%08X's children visibility toggled", Ref->formID);

							break;
						case IDC_RENDERWINDOWCONTEXT_FREEZE:
							SME::MiscGunk::ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Frozen, true);
		//					FORMAT_STR(Buffer, "Selection '%08X' frozen", Ref->formID);

							break;
						case IDC_RENDERWINDOWCONTEXT_THAW:
							SME::MiscGunk::ToggleFlag(&Ref->formFlags, kTESObjectREFRSpecialFlags_Frozen, false);
		//					FORMAT_STR(Buffer, "Selection '%08X' thawed", Ref->formID);

							break;
						}

						RenderWindowPainter::RenderChannelNotifications->Queue(3, Buffer);
					}

					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
					Return = true;

					break;
				case IDC_RENDERWINDOWCONTEXT_GROUP:
				case IDC_RENDERWINDOWCONTEXT_UNGROUP:
					if (_RENDERSEL->selectionCount > 1)
					{
						TESObjectCELL* CurrentCell = (*g_TES)->currentInteriorCell;

						if (CurrentCell == NULL)
							CurrentCell = *g_RenderWindowCurrentlyLoadedCell;

						if (CurrentCell == NULL)
							break;

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_GROUP:
							if (!RenderSelectionGroupManager::Instance.AddGroup(CurrentCell, *g_TESRenderSelectionPrimary))
							{
								BGSEEUI->MsgBoxW(hWnd, 0,
												"Couldn't add current selection to a new group.\n\nMake sure none of the selected objects belong to a preexisting group.");
							}
							else
								RenderWindowPainter::RenderChannelNotifications->Queue(2, "Created new selection group for current cell");

							break;
						case IDC_RENDERWINDOWCONTEXT_UNGROUP:
							if (!RenderSelectionGroupManager::Instance.RemoveGroup(CurrentCell, *g_TESRenderSelectionPrimary))
							{
								BGSEEUI->MsgBoxW(hWnd, 0,
												"Couldn't remove current selection group.\n\nMake sure the selected objects belong to a preexisting group.");
							}
							else
								RenderWindowPainter::RenderChannelNotifications->Queue(2, "Removed selection group from current cell");

							break;
						}

						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOX:
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOY:
				case IDC_RENDERWINDOWCONTEXT_ALIGNTOZ:
					if (_RENDERSEL->selectionCount > 1)
					{
						// record the op twice, otherwise the thingy will crash on undo for some reason
						(*g_TESRenderUndoStack)->RecordReference(TESRenderUndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);
						(*g_TESRenderUndoStack)->RecordReference(TESRenderUndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);

						TESObjectREFR* AlignRef = CS_CAST(_RENDERSEL->selectionList->Data, TESForm, TESObjectREFR);

						for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList->Next; Itr && Itr->Data; Itr = Itr->Next)
						{
							TESObjectREFR* ThisRef = CS_CAST(Itr->Data, TESForm, TESObjectREFR);

							switch (LOWORD(wParam))
							{
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOX:
								ThisRef->position.x = AlignRef->position.x;
								break;
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOY:
								ThisRef->position.y = AlignRef->position.y;
								break;
							case IDC_RENDERWINDOWCONTEXT_ALIGNTOZ:
								ThisRef->position.z = AlignRef->position.z;
								break;
							}

							ThisRef->UpdateUsageInfo();
							ThisRef->SetFromActiveFile(true);
							ThisRef->UpdateNiNode();
						}

						RenderWindowPainter::RenderChannelNotifications->Queue(2, "Selection aligned to %08X", AlignRef->formID);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_COPLANARDROP:
					{
						bool CoplanarDrop = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_CoplanarRefDrops, BGSEEMAIN->INIGetter()));
						CoplanarDrop = (CoplanarDrop == false);

						INISettings::GetRenderer()->Set(INISettings::kRenderer_CoplanarRefDrops, BGSEEMAIN->INISetter(), "%d", CoplanarDrop);

						if (CoplanarDrop)
							RenderWindowPainter::RenderChannelNotifications->Queue(6, "Enabled co-planar dropping");
						else
							RenderWindowPainter::RenderChannelNotifications->Queue(6, "Disabled co-planar dropping");

						Return = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK RenderWindowMiscSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = TRUE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

			UInt8* YKeyState = (UInt8*)0x00A0BC1E;
			float* UnkRotFactor = (float*)0x00A0BAC4;

			switch (uMsg)
			{
			case WM_CLOSE:
				SendMessage(*g_HWND_CSParent, WM_COMMAND, 40423, NULL);
				Return = true;

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case 1:			// update timer
					static bool SetTimerPeriod = true;
					if (SetTimerPeriod)
					{
						SetTimerPeriod = false;
						UInt32 Period = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_UpdatePeriod, BGSEEMAIN->INIGetter()));
						if (Period == 0 || Period >= 100)
							Period = 50;

						SetTimer(hWnd, 1, Period, NULL);
					}

					if (Hooks::g_LODDiffuseMapGeneratorState != Hooks::kLODDiffuseMapGeneratorState_NotInUse)
					{
						// prevent the OS from triggering the screen-saver/switching to standby mode
						SetThreadExecutionState(ES_CONTINUOUS|ES_DISPLAY_REQUIRED|ES_SYSTEM_REQUIRED);
					}

					break;
				}

				break;
			case WM_LBUTTONDOWN:
				Hooks::g_MouseCaptureDelta.x = GET_X_LPARAM(lParam);
				Hooks::g_MouseCaptureDelta.y = GET_Y_LPARAM(lParam);

				break;
			case WM_LBUTTONUP:
				Hooks::g_MouseCaptureDelta.x -= GET_X_LPARAM(lParam);
				Hooks::g_MouseCaptureDelta.y -= GET_Y_LPARAM(lParam);

				Hooks::g_MouseCaptureDelta.x = abs(Hooks::g_MouseCaptureDelta.x);
				Hooks::g_MouseCaptureDelta.y = abs(Hooks::g_MouseCaptureDelta.y);

				break;
			case WM_KEYUP:
				switch (wParam)
				{
				case 0x43:		// C
					{
						int SwitchEnabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						if (SwitchEnabled)
						{
							if (*YKeyState)
								*UnkRotFactor = 0.0;

							*YKeyState = 0;

							Return = true;
						}
					}

					break;
				case 0x59:		// Y
					{
						int SwitchEnabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						if (SwitchEnabled)
						{
							Return = true;
						}
					}

					break;
				}

				break;
			case WM_KEYDOWN:
				switch (wParam)
				{
				case VK_F1:		// F1
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						TESDialog::ShowUseReportDialog(Ref);
					}

					Return = true;

					break;
				case 0x5A:		// Z
					if (*g_RenderWindowPathGridEditModeFlag && GetAsyncKeyState(VK_CONTROL))
					{
						PathGridUndoManager::Instance.PerformUndo();
						Return = true;
					}

					break;
				case 0x59:		// Y
					if (*g_RenderWindowPathGridEditModeFlag && GetAsyncKeyState(VK_CONTROL))
					{
						PathGridUndoManager::Instance.PerformRedo();
						Return = true;
					}
					else
					{
						int SwitchEnabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						if (SwitchEnabled)
						{
							InstanceUserData = 1;
							SendMessage(hWnd, WM_KEYDOWN, 0x43, lParam);
							InstanceUserData = 0;

							Return = true;
						}
					}

					break;
				case 0x43:		// C
					{
						int SwitchEnabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						if (SwitchEnabled && InstanceUserData == 0)
						{
							if (*YKeyState == 0)
								*UnkRotFactor = 0.0;

							*YKeyState = 1;

							Return = true;
						}
					}

					break;
				case 0x52:		// R
					if (*g_RenderWindowPathGridEditModeFlag)
					{
						if (GetAsyncKeyState(VK_CONTROL))
						{
							PathGridUndoManager::Instance.ResetRedoStack();

							if (g_RenderWindowSelectedPathGridPoints->Count())
								PathGridUndoManager::Instance.RecordOperation(PathGridUndoManager::kOperation_DataChange, g_RenderWindowSelectedPathGridPoints);

							for (tList<TESPathGridPoint>::Iterator Itr = g_RenderWindowSelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								TESPathGridPoint* Point = Itr.Get();
								Point->UnlinkFromReference();
								Point->HideSelectionRing();
							}

							TESDialog::RedrawRenderWindow();
							BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

							Return = true;
						}
						else
						{
							std::list<TESPathGridPoint*> Delinquents;

							for (tList<TESPathGridPoint>::Iterator Itr = g_RenderWindowSelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								if (Itr.Get()->linkedRef)
									Delinquents.push_back(Itr.Get());

								Itr.Get()->HideSelectionRing();
							}

							for (std::list<TESPathGridPoint*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); Itr++)
								thisCall<void>(0x00452AE0, g_RenderWindowSelectedPathGridPoints, *Itr);
						}
					}

					break;
				case 0x51:		// Q
					if (GetAsyncKeyState(VK_CONTROL))
					{
						if (Hooks::g_RenderWindowAltMovementSettings)
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Using vanilla movement settings");
						else
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Using alternate movement settings");

						Hooks::g_RenderWindowAltMovementSettings = (Hooks::g_RenderWindowAltMovementSettings == false);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

						Return = true;
					}
					else if (GetAsyncKeyState(VK_SHIFT) && AUXVIEWPORT->GetVisible())
					{
						if (AUXVIEWPORT->ToggleFrozenState())
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Froze auxiliary viewport camera");
						else
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Released auxiliary viewport camera");

						Return = true;
					}

					break;
				case 0x45:		// E
					if (GetAsyncKeyState(VK_SHIFT))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_COPLANARDROP, NULL);

						Return = true;
					}

					break;
				case 0x56:		// V
					if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_CONTROL) == FALSE)
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_INVERTSELECTION, NULL);

						Return = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK ObjectWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FilterLabel = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL);
			HWND FormList = GetDlgItem(hWnd, 1041);
			HWND TreeList = GetDlgItem(hWnd, 2093);
			HWND Splitter = GetDlgItem(hWnd, 2157);

			if (FilterEditBox == NULL)
				return DlgProcResult;

			switch (uMsg)
			{
			case WM_CLOSE:
				SendMessage(*g_HWND_CSParent, WM_COMMAND, 40199, NULL);
				Return = true;

				break;
			case 0x417:		// destroy window
			case WM_DESTROY:
				CSEFilterableFormListManager::Instance.Unregister(hWnd);

				break;
			case WM_INITDIALOG:
				CSEFilterableFormListManager::Instance.Register(hWnd, FilterEditBox, FormList);

				break;
			case WM_SIZE:
				if (wParam != SIZE_MINIMIZED)
				{
					RECT RectFormList = {0}, RectTreeView = {0}, RectSplitter = {0}, RectFilterLbl = {0}, RectFilterEdit = {0};
					POINT Point = {0};

					GetWindowRect(TreeList, &RectTreeView);
					GetWindowRect(Splitter, &RectSplitter);
					GetWindowRect(FormList, &RectFormList);
					GetWindowRect(FilterLabel, &RectFilterLbl);
					GetWindowRect(FilterEditBox, &RectFilterEdit);

					Point.x = RectFilterLbl.left;
					Point.y = RectFilterLbl.bottom + 3;
					ScreenToClient(hWnd, &Point);
					MoveWindow(TreeList, Point.x, Point.y, RectTreeView.right - RectTreeView.left, (unsigned int)(lParam >> 16) - Point.y - 6, 1);

					Point.x = RectFormList.left;
					Point.y = RectFilterEdit.top;
					ScreenToClient(hWnd, &Point);
					MoveWindow(FormList, Point.x, Point.y, (unsigned __int16)lParam - Point.x - 7, (unsigned int)(lParam >> 16) - Point.y - 6, 1);

					Point.x = RectSplitter.left;
					ScreenToClient(hWnd, &Point);
					MoveWindow(Splitter, Point.x, 0, RectSplitter.right - RectSplitter.left, (unsigned int)lParam >> 16, 1);

					Return = true;
				}

				break;
			default:
				if (CSEFilterableFormListManager::Instance.HandleMessages(hWnd, uMsg, wParam, lParam))
				{
					HTREEITEM Selection = TreeView_GetSelection(TreeList);

					TreeView_SelectItem(TreeList, NULL);
					TreeView_SelectItem(TreeList, Selection);
				}

				break;
			}

			return DlgProcResult;
		}

		enum
		{
			kCellViewRefListColumn_Persistent = 5,
			kCellViewRefListColumn_Disabled,
			kCellViewRefListColumn_VWD,
			kCellViewRefListColumn_EnableParent,
			kCellViewRefListColumn_Count
		};

		int CALLBACK CellViewFormListCustomComparator( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
		{
			int Result = 0;

			TESObjectREFR* FormA = (TESObjectREFR*)lParam1;
			TESObjectREFR* FormB = (TESObjectREFR*)lParam2;

			if (FormA && FormB)
			{
				switch ((UInt32)abs(lParamSort))
				{
				case kCellViewRefListColumn_Persistent:
					if ((bool)(FormA->formFlags & TESForm::kFormFlags_QuestItem) == true &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_QuestItem) == false)
					{
						Result = -1;
					}
					else if ((bool)(FormA->formFlags & TESForm::kFormFlags_QuestItem) == false &&
							(bool)(FormB->formFlags & TESForm::kFormFlags_QuestItem) == true)
					{
						Result = 1;
					}

					break;
				case kCellViewRefListColumn_Disabled:
					if ((bool)(FormA->formFlags & TESForm::kFormFlags_Disabled) == true &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_Disabled) == false)
					{
						Result = -1;
					}
					else if ((bool)(FormA->formFlags & TESForm::kFormFlags_Disabled) == false &&
							(bool)(FormB->formFlags & TESForm::kFormFlags_Disabled) == true)
					{
						Result = 1;
					}

					break;
				case kCellViewRefListColumn_VWD:
					if ((bool)(FormA->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == true &&
						(bool)(FormB->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == false)
					{
						Result = -1;
					}
					else if ((bool)(FormA->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == false &&
							(bool)(FormB->formFlags & TESForm::kFormFlags_VisibleWhenDistant) == true)
					{
						Result = 1;
					}

					break;
				case kCellViewRefListColumn_EnableParent:
					{
						BSExtraData* AxData = FormA->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
						BSExtraData* BxData = FormB->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);

						if (AxData && BxData == NULL)
							Result = -1;
						else if (AxData == NULL && BxData)
							Result = 1;
						else if (AxData && BxData)
						{
							ExtraEnableStateParent* AxParent = CS_CAST(AxData, BSExtraData, ExtraEnableStateParent);
							ExtraEnableStateParent* BxParent = CS_CAST(BxData, BSExtraData, ExtraEnableStateParent);
							SME_ASSERT(AxParent->parent && BxParent->parent);

							if (AxParent->parent->formID < BxParent->parent->formID)
								Result = -1;
							else if (AxParent->parent->formID > BxParent->parent->formID)
								Result = 1;
						}
					}

					break;
				case kCellViewRefListColumn_Count:
					{
						BSExtraData* AxData = FormA->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);
						BSExtraData* BxData = FormB->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);

						if (AxData && BxData == NULL)
							Result = -1;
						else if (AxData == NULL && BxData)
							Result = 1;
						else if (AxData && BxData)
						{
							ExtraCount* AxCount = CS_CAST(AxData, BSExtraData, ExtraCount);
							ExtraCount* BxCount = CS_CAST(BxData, BSExtraData, ExtraCount);

							if (AxCount->count < BxCount->count)
								Result = -1;
							else if (AxCount->count > BxCount->count)
								Result = 1;
						}
					}

					break;
				}

				if (lParamSort < 0)
					Result *= -1;

				Result = CSEFormEnumerationManager::Instance.CompareActiveForms(FormA, FormB, Result);
			}

			return Result;
		}

		LRESULT CALLBACK CellViewWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

			HWND WorldspaceLabel = GetDlgItem(hWnd, 1164);
			HWND WorldspaceCombo = GetDlgItem(hWnd, 2083);
			HWND CellLabel = GetDlgItem(hWnd, 1163);
			HWND CellList = GetDlgItem(hWnd, 1155);
			HWND RefList = GetDlgItem(hWnd, 1156);

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FilterLabel = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL);
			HWND XLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_XLBL);
			HWND YLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_YLBL);
			HWND XEdit = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_XEDIT);
			HWND YEdit = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_YEDIT);
			HWND GoBtn = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_GOBTN);

			int* RefListSortColumn = (int*)0x00A0A9D4;

			if (FilterEditBox == NULL)
				return DlgProcResult;

			switch (uMsg)
			{
			case WM_CLOSE:
				SendMessage(*g_HWND_CSParent, WM_COMMAND, 40200, NULL);
				Return = true;

				break;
			case WM_DESTROY:
				delete (CSECellViewExtraData*)UserData->ExtraData;
			case 0x417:		// destroy window
				CSEFilterableFormListManager::Instance.Unregister(hWnd);

				break;
			case WM_INITDIALOG:
				{
					if (UserData->ExtraData == NULL)
					{
						CSECellViewExtraData* ExtraData = new CSECellViewExtraData();
						UserData->ExtraData = (LPARAM)ExtraData;

						POINT Position = {0};
						RECT Bounds = {0};

						GetWindowRect(FilterEditBox, &ExtraData->FilterEditBox);
						Position.x = ExtraData->FilterEditBox.left;
						Position.y = ExtraData->FilterEditBox.top;
						ScreenToClient(hWnd, &Position);
						ExtraData->FilterEditBox.left = Position.x;
						ExtraData->FilterEditBox.top = Position.y;
						GetClientRect(FilterEditBox, &Bounds);
						ExtraData->FilterEditBox.right = Bounds.right;
						ExtraData->FilterEditBox.bottom = Bounds.bottom;

						GetWindowRect(FilterLabel, &ExtraData->FilterLabel);
						Position.x = ExtraData->FilterLabel.left;
						Position.y = ExtraData->FilterLabel.top;
						ScreenToClient(hWnd, &Position);
						ExtraData->FilterLabel.left = Position.x;
						ExtraData->FilterLabel.top = Position.y;
						GetClientRect(FilterLabel, &Bounds);
						ExtraData->FilterLabel.right = Bounds.right;
						ExtraData->FilterLabel.bottom = Bounds.bottom;

						GetWindowRect(XLabel, &ExtraData->XLabel);
						Position.x = ExtraData->XLabel.left;
						Position.y = ExtraData->XLabel.top;
						ScreenToClient(hWnd, &Position);
						ExtraData->XLabel.left = Position.x;
						ExtraData->XLabel.top = Position.y;
						GetClientRect(XLabel, &Bounds);
						ExtraData->XLabel.right = Bounds.right;
						ExtraData->XLabel.bottom = Bounds.bottom;

						GetWindowRect(YLabel, &ExtraData->YLabel);
						Position.x = ExtraData->YLabel.left;
						Position.y = ExtraData->YLabel.top;
						ScreenToClient(hWnd, &Position);
						ExtraData->YLabel.left = Position.x;
						ExtraData->YLabel.top = Position.y;
						GetClientRect(YLabel, &Bounds);
						ExtraData->YLabel.right = Bounds.right;
						ExtraData->YLabel.bottom = Bounds.bottom;

						GetWindowRect(XEdit, &ExtraData->XEdit);
						Position.x = ExtraData->XEdit.left;
						Position.y = ExtraData->XEdit.top;
						ScreenToClient(hWnd, &Position);
						ExtraData->XEdit.left = Position.x;
						ExtraData->XEdit.top = Position.y;
						GetClientRect(XEdit, &Bounds);
						ExtraData->XEdit.right = Bounds.right;
						ExtraData->XEdit.bottom = Bounds.bottom;

						GetWindowRect(YEdit, &ExtraData->YEdit);
						Position.x = ExtraData->YEdit.left;
						Position.y = ExtraData->YEdit.top;
						ScreenToClient(hWnd, &Position);
						ExtraData->YEdit.left = Position.x;
						ExtraData->YEdit.top = Position.y;
						GetClientRect(YEdit, &Bounds);
						ExtraData->YEdit.right = Bounds.right;
						ExtraData->YEdit.bottom = Bounds.bottom;

						GetWindowRect(GoBtn, &ExtraData->GoBtn);
						Position.x = ExtraData->GoBtn.left;
						Position.y = ExtraData->GoBtn.top;
						ScreenToClient(hWnd, &Position);
						ExtraData->GoBtn.left = Position.x;
						ExtraData->GoBtn.top = Position.y;
						GetClientRect(GoBtn, &Bounds);
						ExtraData->GoBtn.right = Bounds.right;
						ExtraData->GoBtn.bottom = Bounds.bottom;

						TESDialog::GetPositionFromINI("Cell View", &Bounds);
						SetWindowPos(hWnd, NULL, Bounds.left, Bounds.top, Bounds.right, Bounds.bottom, 4);
					}

					CSEFilterableFormListManager::Instance.Register(hWnd, FilterEditBox, RefList, true);

					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH|LVCF_TEXT|LVCF_SUBITEM|LVCF_FMT;
					ColumnData.fmt = LVCFMT_CENTER;

					ColumnData.cx = 45;
					ColumnData.pszText = "Persistent";
					ColumnData.iSubItem = kCellViewRefListColumn_Persistent;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 45;
					ColumnData.pszText = "Initially Disabled";
					ColumnData.iSubItem = kCellViewRefListColumn_Disabled;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 45;
					ColumnData.pszText = "VWD";
					ColumnData.iSubItem = kCellViewRefListColumn_VWD;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 100;
					ColumnData.pszText = "Enable Parent";
					ColumnData.iSubItem = kCellViewRefListColumn_EnableParent;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 40;
					ColumnData.pszText = "Count";
					ColumnData.iSubItem = kCellViewRefListColumn_Count;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case LVN_GETDISPINFO:
						if (NotificationData->hwndFrom == RefList)
						{
							NMLVDISPINFO* DisplayData = (NMLVDISPINFO*)lParam;

							if ((DisplayData->item.mask & LVIF_TEXT) && DisplayData->item.lParam)
							{
								DlgProcResult = TRUE;
								Return = true;

								TESObjectREFR* Current = (TESObjectREFR*)DisplayData->item.lParam;

								switch (DisplayData->item.iSubItem)
								{
								case kCellViewRefListColumn_Persistent:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
											((Current->formFlags & TESForm::kFormFlags_QuestItem) ? "Y" : ""));

									break;
								case kCellViewRefListColumn_Disabled:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
											((Current->formFlags & TESForm::kFormFlags_Disabled) ? "Y" : ""));

									break;
								case kCellViewRefListColumn_VWD:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
											((Current->formFlags & TESForm::kFormFlags_VisibleWhenDistant) ? "Y" : ""));

									break;
								case kCellViewRefListColumn_EnableParent:
									{
										BSExtraData* xData = Current->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
										if (xData)
										{
											ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
											SME_ASSERT(xParent->parent);

											if (xParent->parent->editorID.c_str() == NULL)
											{
												sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax,
													"%08X %s",
													xParent->parent->formID,
													(xParent->oppositeState ? " *" : ""));
											}
											else
											{
												sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax,
													"%s %s",
													xParent->parent->editorID.c_str(),
													(xParent->oppositeState ? " *" : ""));
											}
										}
										else
											sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "");
									}

									break;
								case kCellViewRefListColumn_Count:
									{
										BSExtraData* xData = Current->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);
										if (xData)
										{
											ExtraCount* xCount = CS_CAST(xData, BSExtraData, ExtraCount);
											sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%d", xCount->count);
										}
										else
											sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "");
									}

									break;
								default:
									Return = false;
								}
							}
						}

						break;
					case LVN_COLUMNCLICK:
						if (NotificationData->hwndFrom == RefList)
						{
							NMLISTVIEW* ListViewData = (NMLISTVIEW*)lParam;
							if (ListViewData->iSubItem >= kCellViewRefListColumn_Persistent)
							{
								if (*RefListSortColumn > 0)
								{
									*RefListSortColumn = ListViewData->iSubItem;
									*RefListSortColumn *= -1;
								}
								else
									*RefListSortColumn = ListViewData->iSubItem;

								SendMessage(RefList, LVM_SORTITEMS, *RefListSortColumn, (LPARAM)CellViewFormListCustomComparator);

								DlgProcResult = TRUE;
								Return = true;
							}
						}

						break;
					}
				}

				break;
			case WM_SIZE:
				if (wParam != SIZE_MINIMIZED)
				{
					RECT CurrentRect = {0};

					RECT* BaseDlgRect = (RECT*)0x00A0AA38;
					RECT* BaseCellListRect = (RECT*)0x00A0AA48;
					RECT* BaseRefListRect = (RECT*)0x00A0AA04;
					RECT* BaseWorldspaceComboRect = (RECT*)0x00A0AA24;
					RECT* BaseWorldspaceLabelRect = (RECT*)0x00A0AA14;
					RECT* BaseCellNameLabelRect = (RECT*)0x00A0A9F0;

					SetRect(&CurrentRect, 0, 0, (unsigned __int16)lParam, (unsigned int)lParam >> 16);
					int DeltaDlgWidth = (CurrentRect.right - BaseDlgRect->right) >> 1;
					HDWP DeferPosData = BeginDeferWindowPos(3);

					DeferWindowPos(DeferPosData, CellList, 0,
								0, 0,
								DeltaDlgWidth + BaseCellListRect->right, CurrentRect.bottom + BaseCellListRect->bottom - BaseDlgRect->bottom,
								2u);

					DeferWindowPos(DeferPosData, WorldspaceCombo, 0,
								0, 0,
								BaseCellListRect->right + DeltaDlgWidth - BaseWorldspaceLabelRect->right, BaseWorldspaceComboRect->bottom,
								2u);

					DeferWindowPos(DeferPosData, RefList, 0,
								DeltaDlgWidth + BaseRefListRect->left, BaseRefListRect->top,
								DeltaDlgWidth + BaseRefListRect->right, CurrentRect.bottom + BaseRefListRect->bottom - BaseDlgRect->bottom,
								0);

					DeferWindowPos(DeferPosData, CellLabel, 0,
								BaseCellNameLabelRect->left + DeltaDlgWidth, BaseCellNameLabelRect->top,
								DeltaDlgWidth + BaseCellNameLabelRect->right, BaseCellNameLabelRect->bottom,
								0);

					InvalidateRect(CellLabel, NULL, TRUE);
					InvalidateRect(WorldspaceLabel, NULL, TRUE);

					EndDeferWindowPos(DeferPosData);

					CSECellViewExtraData* ExtraData = (CSECellViewExtraData*)UserData->ExtraData;

					if (ExtraData)
					{
						DeferPosData = BeginDeferWindowPos(7);

						DeferWindowPos(DeferPosData, XLabel, 0,
							DeltaDlgWidth + ExtraData->XLabel.left, ExtraData->XLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, XEdit, 0,
							DeltaDlgWidth + ExtraData->XEdit.left, ExtraData->XEdit.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, YLabel, 0,
							DeltaDlgWidth + ExtraData->YLabel.left, ExtraData->YLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, YEdit, 0,
							DeltaDlgWidth + ExtraData->YEdit.left, ExtraData->YEdit.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, GoBtn, 0,
							DeltaDlgWidth + ExtraData->GoBtn.left, ExtraData->GoBtn.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, FilterLabel, 0,
							DeltaDlgWidth + ExtraData->FilterLabel.left, ExtraData->FilterLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, FilterEditBox, 0,
							DeltaDlgWidth + ExtraData->FilterEditBox.left, ExtraData->FilterEditBox.top,
							DeltaDlgWidth + ExtraData->FilterEditBox.right, ExtraData->FilterEditBox.bottom + 2,
							0);

						InvalidateRect(XLabel, NULL, TRUE);
						InvalidateRect(YLabel, NULL, TRUE);
						InvalidateRect(FilterLabel, NULL, TRUE);

						EndDeferWindowPos(DeferPosData);
					}

					Return = true;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_CELLVIEW_GOBTN:
					{
						char XCoord[4] = {0}, YCoord[4] = {0};
						GetWindowText(XEdit, (LPSTR)XCoord, sizeof(XCoord));
						GetWindowText(YEdit, (LPSTR)YCoord, sizeof(YCoord));

						if (strlen(XCoord) && strlen(YCoord))
						{
							Vector3 Coords((atoi(XCoord) << 12) + 2048.0, (atoi(YCoord) << 12) + 2048.0, 0);
							_TES->LoadCellIntoViewPort(&Coords, NULL);
						}
					}

					break;
				}
			default:
				if (CSEFilterableFormListManager::Instance.HandleMessages(hWnd, uMsg, wParam, lParam))
				{
					SendMessage(hWnd, 0x40F, NULL, NULL);		// reinit object list
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK ResponseDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

			HWND VoiceList = GetDlgItem(hWnd, 2168);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				EnableWindow(GetDlgItem(hWnd, 1016), TRUE);

				break;
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
							{
								Return = true;
								break;
							}

							char FilePath[MAX_PATH] = {0};
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
								std::string Destination(std::string(BGSEEMAIN->GetAPPPath()) + "\\" + std::string(VoicePath)), DirPath(Destination);
								DirPath = DirPath.substr(0, DirPath.rfind("\\") + 1);

								if (SHCreateDirectoryEx(NULL, DirPath.c_str(), NULL) &&
									GetLastError() != ERROR_FILE_EXISTS &&
									GetLastError() != ERROR_ALREADY_EXISTS)
								{
									BGSEECONSOLE_ERROR("Couldn't create intermediate path for the new voice file!");
								}

								if (CopyFile(FilePath, Destination.c_str(), FALSE) == FALSE)
								{
									BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't copy external file '%s' to '%s'!\n\nCheck the console for more information.",
													FilePath, Destination.c_str());

									BGSEECONSOLE_ERROR("UIManager::ResponseDlgSubclassProc - Couldn't copy external file!");
								}
								else
									BGSEECONSOLE_MESSAGE("Copied external audio file '%s' to '%s'", FilePath, Destination.c_str());

								BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
							}

							Return = true;
						}

						break;
					case 1016:					// Generate LIP
						if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
						{
							Return = true;
							break;
						}

						std::string Path(VoicePath);
						Path = Path.substr(0, Path.rfind("."));

						HWND IdleWindow = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), BGSEEUI->GetMainWindow(), NULL, NULL);

						if (CSIOM->GenerateLIPSyncFile(Path.c_str(), (*g_ResponseEditorData)->responseLocalCopy->responseText.c_str()) == false)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't generate LIP file for the selected voice.\n\nCheck the console for more information.");
						}
						else
						{
							BGSEECONSOLE_MESSAGE("Successfully generated LIP file for the selected voice");
							BGSEEACHIEVEMENTS->Unlock(Achievements::kSoprano);
						}

						DestroyWindow(IdleWindow);

						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						Return = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK LandscapeTextureUseDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData = (BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);
			Return = false;

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

						RenderWindowPainter::RenderChannelNotifications->Queue(3, "Active landscape texture changed");
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK QuestDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;
					HWND QuestList = GetDlgItem(hWnd, 2064);

					ColumnData.cx = 200;
					ListView_SetColumn(QuestList, 0, &ColumnData);

					ColumnData.cx = 42;
					ListView_SetColumn(QuestList, 2, &ColumnData);
				}

				break;
			}

			return DlgProcResult;
		}

#define ID_COMMONDLGQUICKVIEW_TIMERID							0x108
#define WM_COMDLGEXTRA_EXTRACTCOLOR								(WM_USER + 2005)
		// wParam = const char*, lParam = COLORREF*

		LRESULT CALLBACK CommonDialogExtraFittingsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMDLGEXTRA_EXTRACTCOLOR:
				{
					DlgProcResult = TRUE;
					Return = true;

					const char* ColorString = (const char*)wParam;
					COLORREF* ColorOut = (COLORREF*)lParam;

					if (ColorString && ColorOut)
					{
						SME::StringHelpers::Tokenizer ColorParser(ColorString, ", ");
						int R = 0, G = 0, B = 0;

						for (int i = 0; i < 3; i++)
						{
							std::string Buffer;
							if (ColorParser.NextToken(Buffer) != -1)
							{
								switch (i)
								{
								case 0:		// R
									R = atoi(Buffer.c_str());
									break;
								case 1:		// G
									G = atoi(Buffer.c_str());
									break;
								case 2:		// B
									B = atoi(Buffer.c_str());
									break;
								}
							}
						}

						*ColorOut = RGB(R, G, B);
					}
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_COMMONDLGQUICKVIEW_TIMERID:
					{
						CSEDialogExtraFittingsData* IUD = (CSEDialogExtraFittingsData*)InstanceUserData;

						if (IUD && IUD->QuickViewWindowUnderCursor)
						{
							HWND WindowAtPoint = IUD->QuickViewWindowUnderCursor;
							TESForm* Form = NULL;

							char Buffer[0x200] = {0};
							GetClassName(WindowAtPoint, Buffer, sizeof(Buffer));

							if (!_stricmp("SysListView32", Buffer))
							{
								POINT Coords = {0};
								Coords.x = IUD->QuickViewCursorPos.x;
								Coords.y = IUD->QuickViewCursorPos.y;

								ScreenToClient(WindowAtPoint, &Coords);

								LVHITTESTINFO HitTestData = {0};
								HitTestData.pt.x = Coords.x;
								HitTestData.pt.y = Coords.y;
								HitTestData.flags = LVHT_ONITEM;

								if (ListView_SubItemHitTest(WindowAtPoint, &HitTestData) != -1)
								{
									int Item = HitTestData.iItem;
									int SubItem = HitTestData.iSubItem;

									ZeroMemory(Buffer, sizeof(Buffer));
									ListView_GetItemText(WindowAtPoint, Item, SubItem, Buffer, sizeof(Buffer));
								}
							}
							else
							{
								GetWindowText(WindowAtPoint, Buffer, sizeof(Buffer));
							}

							if (strlen(Buffer) < 2)
								break;

							std::string PotentialEditorID(Buffer);
							int StatusIndicatorOffset = -1;

							if ((StatusIndicatorOffset = PotentialEditorID.find(" *")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);
							if ((StatusIndicatorOffset = PotentialEditorID.find(" D")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);

							Form = TESForm::LookupByEditorID(PotentialEditorID.c_str());

							IUD->QuickViewWindowUnderCursor = NULL;

							if (Form)
							{
								switch (Form->formType)
								{
								case TESForm::kFormType_Script:
									TESDialog::ShowScriptEditorDialog(Form);
									break;
								case TESForm::kFormType_REFR:
									_TES->LoadCellIntoViewPort((CS_CAST(Form, TESForm, TESObjectREFR))->GetPosition(), CS_CAST(Form, TESForm, TESObjectREFR));
									break;
								default:
									TESDialog::ShowFormEditDialog(Form);
									break;
								}

								DlgProcResult = TRUE;
								Return = true;
							}
						}
					}

					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case NM_CUSTOMDRAW:
													// valid listviews:
						if (wParam == 1041 ||		//		object window
							wParam == 1977 ||		//		AI packages
							wParam == 1155 ||		//		cell view (cell list)
							wParam == 1156 ||		//		cell view (ref list)
							wParam == 2064 ||		//		tesformIDlistview
							wParam == 1448 ||		//		dialog editor (topic list)
							wParam == 1449 ||		//		dialog editor (topicinfo list)
							wParam == 1018)			//		select topics/quest
						{
							bool Enabled = atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ColorizeActiveForms, BGSEEMAIN->INIGetter()));

							if (Enabled && CSEFormEnumerationManager::Instance.GetVisibleUnmodifiedForms())
							{
								NMLVCUSTOMDRAW* DrawData = (NMLVCUSTOMDRAW*)lParam;

								switch (DrawData->nmcd.dwDrawStage)
								{
								case CDDS_PREPAINT:
									{
										// modal dialogs have trouble receiving the item pre-paint notification
										// so we perform some monkey business to workaround it
										// that being said - For fuck sake! This needs to documented somewhere!
										// then again, it stands to reason that the return val must be passed as the dialog's result
										// there! now I've gone and contradicted myself...
										if (IsWindowEnabled(GetParent(hWnd)) == FALSE)
										{
											SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
											DlgProcResult = TRUE;
											Return = true;
										}
									}

									break;
								case CDDS_ITEMPREPAINT:
									{
										TESForm* Form = (TESForm*)DrawData->nmcd.lItemlParam;

										if (Form)
										{
											if (Form->IsActive())
											{
												COLORREF ForeColor = 0, BackColor = 0;

												SendMessage(hWnd,
															WM_COMDLGEXTRA_EXTRACTCOLOR,
															(WPARAM)INISettings::GetDialogs()->Get(INISettings::kDialogs_ActiveFormForeColor, BGSEEMAIN->INIGetter()),
															(LPARAM)&ForeColor);

												SendMessage(hWnd,
															WM_COMDLGEXTRA_EXTRACTCOLOR,
															(WPARAM)INISettings::GetDialogs()->Get(INISettings::kDialogs_ActiveFormBackColor, BGSEEMAIN->INIGetter()),
															(LPARAM)&BackColor);

												DrawData->clrText = ForeColor;
												DrawData->clrTextBk = BackColor;

												CSEDialogExtraFittingsData* IUD = (CSEDialogExtraFittingsData*)InstanceUserData;
												if (IUD && IUD->ActiveFormListFont)
													SelectObject(DrawData->nmcd.hdc, IUD->ActiveFormListFont);

												// same here - if modal dialog, donkey boner
												if (IsWindowEnabled(GetParent(hWnd)) == FALSE)
												{
													SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NEWFONT);

													DlgProcResult = TRUE;
													Return = true;
												}
												else
												{
													DlgProcResult = CDRF_NEWFONT;
													Return = true;
												}
											}
										}
									}

									break;
								}
							}
						}

						break;
					}
				}

				break;
			case WM_INITDIALOG:
				{
					CSEDialogExtraFittingsData* IUD = new CSEDialogExtraFittingsData();
					InstanceUserData = (LPARAM)IUD;

					// we need to defer the looked-up window's creation a bit to keep the source window from hogging focus
					SetTimer(hWnd, ID_COMMONDLGQUICKVIEW_TIMERID, 100, NULL);
				}

				break;

			case WM_DESTROY:
				{
					CSEDialogExtraFittingsData* IUD = (CSEDialogExtraFittingsData*)InstanceUserData;

					delete IUD;
					InstanceUserData = NULL;

					KillTimer(hWnd, ID_COMMONDLGQUICKVIEW_TIMERID);
				}

				break;
			case WM_MOUSEACTIVATE:
				if (HIWORD(lParam) != WM_MBUTTONDOWN &&
					HIWORD(lParam) != WM_MBUTTONUP)
				{
					break;
				}
			case WM_MBUTTONUP:
				{
					POINT Coords = {0};

					if (uMsg == WM_MOUSEACTIVATE)
					{
						GetCursorPos(&Coords);
					}
					else
					{
						Coords.x = GET_X_LPARAM(lParam);
						Coords.y = GET_Y_LPARAM(lParam);
						ClientToScreen(hWnd, &Coords);
					}

					HWND WindowAtPoint = WindowFromPoint(Coords);

					if (WindowAtPoint)
					{
						CSEDialogExtraFittingsData* IUD = (CSEDialogExtraFittingsData*)InstanceUserData;

						if (IUD)
						{
							IUD->QuickViewCursorPos.x = Coords.x;
							IUD->QuickViewCursorPos.y = Coords.y;
							IUD->QuickViewWindowUnderCursor = WindowAtPoint;
						}
					}
				}

				break;
			}

			return DlgProcResult;
		}

		int CALLBACK SelectTopicsQuestsFormListComparator( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
		{
			int Result = 0;

			TESForm* FormA = (TESForm*)lParam1;
			TESForm* FormB = (TESForm*)lParam2;

			if (FormA && FormB)
			{
				const char* EDIDA = FormA->editorID.c_str();
				const char* EDIDB = FormB->editorID.c_str();

				if (EDIDA && EDIDB)
				{
					Result = _stricmp(EDIDA, EDIDB);
					if (lParamSort)
						Result *= -1;

					Result = CSEFormEnumerationManager::Instance.CompareActiveForms(FormA, FormB, Result);
				}
			}

			return Result;
		}

		LRESULT CALLBACK SelectTopicsQuestsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool& Return, LPARAM& InstanceUserData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FormList = GetDlgItem(hWnd, 1018);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 340;
					ListView_SetColumn(FormList, 0, &ColumnData);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case LVN_COLUMNCLICK:
						{
							// only one column, so let's sort that
							InstanceUserData = (InstanceUserData == 0);
							SendMessage(FormList, LVM_SORTITEMS, InstanceUserData, (LPARAM)SelectTopicsQuestsFormListComparator);

							DlgProcResult = TRUE;
							Return = true;
						}

						break;
					}
				}

				break;
			}

			return DlgProcResult;
		}

		BOOL CALLBACK AssetSelectorDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_GETMINMAXINFO:
				{
					MINMAXINFO* SizeInfo = (MINMAXINFO*)lParam;
					SizeInfo->ptMaxTrackSize.x = SizeInfo->ptMinTrackSize.x = 189;
					SizeInfo->ptMaxTrackSize.y = SizeInfo->ptMinTrackSize.y = 223;

					break;
				}
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_ASSETSELECTOR_FILEBROWSER:
					EndDialog(hWnd, Hooks::e_FileBrowser);

					return TRUE;
				case IDC_ASSETSELECTOR_ARCHIVEBROWSER:
					EndDialog(hWnd, Hooks::e_BSABrowser);

					return TRUE;
				case IDC_ASSETSELECTOR_PATHEDITOR:
					EndDialog(hWnd, Hooks::e_EditPath);

					return TRUE;
				case IDC_ASSETSELECTOR_PATHCOPIER:
					EndDialog(hWnd, Hooks::e_CopyPath);

					return TRUE;
				case IDC_ASSETSELECTOR_CLEARPATH:
					EndDialog(hWnd, Hooks::e_ClearPath);

					return TRUE;
				case IDC_ASSETSELECTOR_ASSETEXTRACTOR:
					EndDialog(hWnd, Hooks::e_ExtractPath);

					return TRUE;
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, Hooks::e_Close);

					return TRUE;
				}

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK TextEditDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				{
					InitDialogMessageParamT<UInt32>* InitParam = (InitDialogMessageParamT<UInt32>*)GetWindowLongPtr(hWnd, GWL_USERDATA);

					switch (LOWORD(wParam))
					{
					case IDC_CSE_OK:
						GetDlgItemText(hWnd, IDC_TEXTEDIT_TEXT, InitParam->Buffer, sizeof(InitParam->Buffer));
						EndDialog(hWnd, 1);

						return TRUE;
					case IDC_CSE_CANCEL:
						EndDialog(hWnd, NULL);

						return TRUE;
					}
				}

				break;
			case WM_INITDIALOG:
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);
				SetDlgItemText(hWnd, IDC_TEXTEDIT_TEXT, ((InitDialogMessageParamT<UInt32>*)lParam)->Buffer);

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK TESFileSaveDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_TESFILESAVE_SAVEESP:
					EndDialog(hWnd, 0);

					return TRUE;
				case IDC_TESFILESAVE_SAVEESM:
					EndDialog(hWnd, 1);

					return TRUE;
				}

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK TESComboBoxDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			HWND ComboBox = GetDlgItem(hWnd, IDC_TESCOMBOBOX_FORMLIST);

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_OK:
					{
						TESForm* SelectedForm = (TESForm*)TESComboBox::GetSelectedItemData(ComboBox);
						EndDialog(hWnd, (INT_PTR)SelectedForm);

						return TRUE;
					}
				case IDC_CSE_CANCEL:
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
					Edit_SetText(GetDlgItem(hWnd, IDC_COPYPATH_ASSETPATH), "Dragging...");
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
						char Buffer[0x200] = {0};
						GetWindowText(Window, Buffer, sizeof(Buffer));
						Edit_SetText(GetDlgItem(hWnd, IDC_COPYPATH_ASSETPATH), Buffer);
					}
					else
						Edit_SetText(GetDlgItem(hWnd, IDC_COPYPATH_ASSETPATH), NULL);

					ReleaseCapture();
					s_DraggingMouse = false;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_OK:
					{
						char Buffer[0x200] = {0};
						GetDlgItemText(hWnd, IDC_COPYPATH_ASSETPATH, Buffer, sizeof(Buffer));
						InitDialogMessageParamT<UInt32>* InitParam = (InitDialogMessageParamT<UInt32>*)GetWindowLongPtr(hWnd, GWL_USERDATA);

						switch (InitParam->ExtraData)
						{
						case Hooks::e_SPT:
							FORMAT_STR(InitParam->Buffer, "\\%s", Buffer);

							break;
						case Hooks::e_KF:
							{
								std::string STLBuffer(Buffer);
								int Offset = STLBuffer.find("IdleAnims\\");

								if (Offset != -1)
									STLBuffer = STLBuffer.substr(Offset + 9);

								FORMAT_STR(InitParam->Buffer, "%s", STLBuffer.c_str());
							}

							break;
						default:
							FORMAT_STR(InitParam->Buffer, "%s", Buffer);

							break;
						}

						EndDialog(hWnd, 1);

						return TRUE;
					}
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, 0);

					return TRUE;
				}

				break;
			case WM_INITDIALOG:
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);

				break;
			}

			return FALSE;
		}

		LRESULT CALLBACK CreateGlobalScriptDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_OK:
					{
						char QuestID[0x200] = {0};
						char ScriptID[0x200] = {0};
						char Delay[8] = {0};
						char Buffer[0x200] = {0};

						GetDlgItemText(hWnd, IDC_GLOBALSCRIPT_QUESTID, QuestID, sizeof(QuestID));
						GetDlgItemText(hWnd, IDC_GLOBALSCRIPT_SCRIPTID, ScriptID, sizeof(ScriptID));
						GetDlgItemText(hWnd, IDC_GLOBALSCRIPT_DELAY, Delay, sizeof(Delay));

						TESForm* Form = NULL;
						TESQuest* Quest = NULL;
						Script* QuestScript = NULL;

						Form = TESForm::LookupByEditorID(QuestID);

						if (Form)
						{
							if (Form->formType == TESForm::kFormType_Quest)
							{
								if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO,
													"Quest '%s' already exists. Do you want to replace its script with a newly created one?", QuestID) != IDYES)
								{
									return TRUE;
								}
							}
							else
							{
								BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", QuestID);

								return TRUE;
							}

							Quest = CS_CAST(Form, TESForm, TESQuest);
						}
						else
						{
							Quest = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Quest), TESForm, TESQuest);
							Quest->SetFromActiveFile(true);
							Quest->SetEditorID(QuestID);

							_DATAHANDLER->quests.AddAt(Quest, eListEnd);
						}

						if (strlen(ScriptID) < 1)
						{
							FORMAT_STR(ScriptID, "%sScript", QuestID);
						}

						Form = TESForm::LookupByEditorID(ScriptID);

						if (Form)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", ScriptID);

							return TRUE;
						}
						else
						{
							QuestScript = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Script), TESForm, Script);
							QuestScript->info.type = Script::kScriptType_Quest;
							QuestScript->SetFromActiveFile(true);

							FORMAT_STR(Buffer, "scn %s\n\nfloat fQuestDelayTime\n\nBegin GameMode\n\tset fQuestDelayTime to %s\n\nend", ScriptID, Delay);
							QuestScript->SetText(Buffer);
							QuestScript->SetEditorID(ScriptID);

							_DATAHANDLER->scripts.AddAt(QuestScript, eListEnd);
							_DATAHANDLER->SortScripts();
						}

						Quest->script = QuestScript;

						Quest->Link();
						QuestScript->Link();

						Quest->UpdateUsageInfo();
						QuestScript->UpdateUsageInfo();
						QuestScript->AddCrossReference(Quest);

						TESDialog::ShowScriptEditorDialog(TESForm::LookupByEditorID(ScriptID));
						DestroyWindow(hWnd);

						return TRUE;
					}
				case IDC_CSE_CANCEL:
					DestroyWindow(hWnd);

					return TRUE;
				}

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK BindScriptDlgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
		{
			HWND EditorIDBox = GetDlgItem(hWnd, IDC_BINDSCRIPT_NEWFORMEDITORID);
			HWND RefIDBox = GetDlgItem(hWnd, IDC_BINDSCRIPT_NEWREFEDITORID);
			HWND ExistFormList = GetDlgItem(hWnd, IDC_BINDSCRIPT_EXISTINGFORMLIST);
			HWND SelParentCellBtn = GetDlgItem(hWnd, IDC_BINDSCRIPT_SELECTPARENTCELL);

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_BINDSCRIPT_SELECTPARENTCELL:
					{
						TESForm* Selection = (TESForm*)BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
																		MAKEINTRESOURCE(IDD_TESCOMBOBOX),
																		hWnd,
																		(DLGPROC)TESComboBoxDlgProc,
																		(LPARAM)TESForm::kFormType_Cell);

						if (Selection)
						{
							char Buffer[0x200] = {0};
							FORMAT_STR(Buffer, "%s (%08X)", Selection->editorID.c_str(), Selection->formID);

							SetWindowText(SelParentCellBtn, (LPCSTR)Buffer);
							SetWindowLongPtr(SelParentCellBtn, GWL_USERDATA, (LONG_PTR)Selection);
						}
						break;
					}
				case IDC_CSE_OK:
					{
						if (IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_BINDEXISTFORM))
						{
							TESForm* SelectedForm = (TESForm*)TESComboBox::GetSelectedItemData(ExistFormList);
							if (SelectedForm)
							{
								EndDialog(hWnd, (INT_PTR)SelectedForm);
								return TRUE;
							}
							else
								BGSEEUI->MsgBoxE(hWnd, 0, "Invalid existing form selected");
						}
						else
						{
							char BaseEditorID[0x200] = {0};
							char RefEditorID[0x200] = {0};
							char Buffer[0x200] = {0};

							Edit_GetText(EditorIDBox, BaseEditorID, 0x200);
							if (TESForm::LookupByEditorID(BaseEditorID))
							{
								BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", BaseEditorID);
							}
							else
							{
								if (IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_QUESTFORM))
								{
									bool StartGameEnabledFlag = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_QUESTSTARTGAMEENABLED);
									bool RepeatedStagesFlag = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_QUESTREPEATEDSTAGES);

									TESQuest* Quest = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Quest), TESForm, TESQuest);
									Quest->SetFromActiveFile(true);
									Quest->SetEditorID(BaseEditorID);
									Quest->SetStartGameEnabledFlag(StartGameEnabledFlag);
									Quest->SetAllowedRepeatedStagesFlag(RepeatedStagesFlag);
									_DATAHANDLER->quests.AddAt(Quest, eListEnd);

									TESDialog::ResetFormListControls();
									EndDialog(hWnd, (INT_PTR)Quest);

									return TRUE;
								}
								else
								{
									if (IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_OBJECTTOKEN))
									{
										bool QuestItem = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_OBJECTTOKENQUESTITEM);

										TESObjectCLOT* Token = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Clothing), TESForm, TESObjectCLOT);
										Token->SetFromActiveFile(true);
										Token->SetEditorID(BaseEditorID);
										_DATAHANDLER->AddTESObject(Token);

										SME::MiscGunk::ToggleFlag(&Token->bipedModelFlags, TESBipedModelForm::kBipedModelFlags_NotPlayable, true);
										SME::MiscGunk::ToggleFlag(&Token->formFlags, TESForm::kFormFlags_QuestItem, QuestItem);

										TESDialog::ResetFormListControls();
										EndDialog(hWnd, (INT_PTR)Token);

										return TRUE;
									}
									else
									{
										Edit_GetText(RefIDBox, RefEditorID, 0x200);
										if (TESForm::LookupByEditorID(RefEditorID))
										{
											BGSEEUI->MsgBoxE(hWnd, 0, "EditorID '%s' is already in use.", BaseEditorID);
										}
										else
										{
											bool InitiallyDisabled = IsDlgButtonChecked(hWnd, IDC_BINDSCRIPT_OBJECTREFERENCEDISABLED);
											TESObjectCELL* ParentCell = CS_CAST(GetWindowLongPtr(SelParentCellBtn, GWL_USERDATA), TESForm, TESObjectCELL);

											if (!ParentCell || ParentCell->GetIsInterior() == 0)
											{
												BGSEEUI->MsgBoxE(hWnd, 0, "Invalid/exterior cell selected as parent.");
											}
											else
											{
												TESObjectACTI* Activator =  CS_CAST(TESForm::CreateInstance(TESForm::kFormType_Activator), TESForm, TESObjectACTI);
												Activator->SetFromActiveFile(true);
												Activator->SetEditorID(BaseEditorID);
												_DATAHANDLER->AddTESObject(Activator);

												static Vector3 ZeroVector(0.0, 0.0, 0.0);

												TESObjectREFR* Ref = _DATAHANDLER->PlaceObjectRef(Activator,
																								&ZeroVector,
																								&ZeroVector,
																								CS_CAST(ParentCell, TESForm, TESObjectCELL),
																								NULL,
																								NULL);

												SME::MiscGunk::ToggleFlag(&Ref->formFlags, TESForm::kFormFlags_Disabled, InitiallyDisabled);
												SME::MiscGunk::ToggleFlag(&Ref->formFlags, TESForm::kFormFlags_QuestItem, true);

												Ref->SetEditorID(RefEditorID);

												TESDialog::ResetFormListControls();
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
				case IDC_CSE_CANCEL:
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

				CheckDlgButton(hWnd, IDC_BINDSCRIPT_BINDEXISTFORM, BST_CHECKED);
				CheckDlgButton(hWnd, IDC_BINDSCRIPT_QUESTFORM, BST_CHECKED);
				CheckDlgButton(hWnd, IDC_BINDSCRIPT_OBJECTTOKEN, BST_CHECKED);

				SetWindowLongPtr(SelParentCellBtn, GWL_USERDATA, (LONG_PTR)0);

				break;
			}

			return FALSE;
		}

		BOOL CALLBACK EditResultScriptDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_CANCEL:
					EndDialog(hWnd, NULL);

					return TRUE;
				case IDC_EDITRESULTSCRIPT_COMPILE:
				case IDC_EDITRESULTSCRIPT_SAVE:
					char Buffer[0x1000] = {0};
					HWND Parent = (HWND)GetWindowLongPtr(hWnd, GWL_USERDATA);

					GetDlgItemText(hWnd, IDC_EDITRESULTSCRIPT_SCRIPTTEXT, Buffer, sizeof(Buffer));
					SetDlgItemText(Parent, 1444, (LPSTR)Buffer);

					if (LOWORD(wParam) == IDC_EDITRESULTSCRIPT_COMPILE)
						EndDialog(hWnd, 1);
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

				SendDlgItemMessage(hWnd, IDC_EDITRESULTSCRIPT_SCRIPTTEXT, EM_SETPARAFORMAT, NULL, (LPARAM)&FormattingData);
				SetWindowLongPtr(hWnd, GWL_USERDATA, (LONG_PTR)lParam);

				HWND ResultScriptEditBox = GetDlgItem((HWND)lParam, 1444);
				char Buffer[0x1000] = {0};

				GetWindowText(ResultScriptEditBox, Buffer, sizeof(Buffer));
				SetDlgItemText(hWnd, IDC_EDITRESULTSCRIPT_SCRIPTTEXT, (LPSTR)Buffer);

				break;
			}

			return FALSE;
		}

		void Initialize( void )
		{
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FindText, FindTextDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Data, DataDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ResponseEditor, ResponseDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_TextureUse, LandscapeTextureUseDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Quest, QuestDlgSubClassProc);

			{
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CellEdit, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Data, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SearchReplace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandscapeEdit, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FindText, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_IdleAnimations, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIPackages, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FilteredDialog, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weapon, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Armor, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Clothing, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_MiscItem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Static, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Reference, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Apparatus, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Book, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Container, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Activator, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIForm, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Light, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Potion, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Enchantment, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledCreature, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Sound, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Door, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledItem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandTexture, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SoulGem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ammo, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Spell, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Flora, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Tree, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_CombatStyle, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Water, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_NPC, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Creature, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Grass, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Furniture, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LoadingScreen, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Ingredient, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LeveledSpell, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AnimObject, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Subspace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectShader, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SigilStone, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Faction, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Class, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Skill, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectSetting, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_GameSetting, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Globals, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Birthsign, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Climate, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Worldspace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Hair, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Quest, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Eyes, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Dialog, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectTopic, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectQuests, CommonDialogExtraFittingsSubClassProc);
			}

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectTopic, SelectTopicsQuestsSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectQuests, SelectTopicsQuestsSubClassProc);

			BGSEEUI->GetWindowHandleCollection(BGSEditorExtender::BGSEEUIManager::kHandleCollection_DragDropableWindows)->Add(
																								CLIWrapper::Interfaces::TAG->GetFormDropWindowHandle());

			if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ShowEditDialogsInTaskbar, BGSEEMAIN->INIGetter())))
			{
				BGSEditorExtender::BGSEEWindowStyler::StyleData RegularAppWindow = {0};
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = BGSEditorExtender::BGSEEWindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_CellEdit, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LandscapeEdit, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_IdleAnimations, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_AIPackages, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_FilteredDialog, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Weapon, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Armor, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Clothing, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_MiscItem, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Static, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Reference, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Apparatus, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Book, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Container, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Activator, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_AIForm, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Light, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Potion, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Enchantment, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LeveledCreature, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Sound, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Door, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LeveledItem, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LandTexture, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_SoulGem, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Ammo, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Spell, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Flora, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Tree, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_CombatStyle, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Water, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_NPC, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Creature, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Grass, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Furniture, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LoadingScreen, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Ingredient, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_LeveledSpell, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_AnimObject, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Subspace, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_EffectShader, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_SigilStone, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Faction, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Race, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Class, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Skill, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_EffectSetting, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_GameSetting, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Globals, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Birthsign, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Climate, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Worldspace, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Hair, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Quest, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Eyes, RegularAppWindow);
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Dialog, RegularAppWindow);
			}

			SendMessage(BGSEEUI->GetMainWindow(), WM_MAINWINDOW_INITEXTRADATA, NULL, NULL);
		}
	}
}