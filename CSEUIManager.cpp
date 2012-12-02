#include "CSEUIManager.h"
#include "Construction Set Extender_Resource.h"
#include "AuxiliaryViewport.h"
#include "[Common]\CLIWrapper.h"
#include "CSEWorkspaceManager.h"
#include "CSERenderWindowPainter.h"
#include "CSERenderWindowFlyCamera.h"
#include "CSEAchievements.h"
#include "Hooks\Hooks-AssetSelector.h"
#include "Hooks\Hooks-Dialog.h"
#include "Hooks\Hooks-Renderer.h"
#include "Hooks\Hooks-LOD.h"
#include "ChangeLogManager.h"
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
						TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);
						if (Reference)
							Form = Reference->baseForm;
					}

					if (UserData->FilterString.size() && Form)
					{
						std::string EditorID, FullName, Description;

						TESFullName* FullNameCmpt = CS_CAST(Form, TESForm, TESFullName);
						TESDescription* DescriptionCmpt = CS_CAST(Form, TESForm, TESDescription);

						if (FullNameCmpt && FullNameCmpt->name.c_str())
							FullName = FullNameCmpt->name.c_str();

						if (DescriptionCmpt && DescriptionCmpt->description.c_str())
							Description = DescriptionCmpt->description.c_str();

						if (Form->editorID.c_str())
							EditorID = Form->editorID.c_str();

						SME::StringHelpers::MakeLower(EditorID);
						SME::StringHelpers::MakeLower(FullName);
						SME::StringHelpers::MakeLower(Description);

						if (EditorID.find(UserData->FilterString) == std::string::npos &&
							FullName.find(UserData->FilterString) == std::string::npos &&
							Description.find(UserData->FilterString) == std::string::npos)
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
				ActiveWindows.insert(std::make_pair(Window, new FilterableWindowData(Window, FilterEdit, FormList, ObjRefList, TimePeriod)));
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

		CSECellViewExtraData::~CSECellViewExtraData()
		{
			;//
		}

		CSECellViewExtraData::CSECellViewExtraData() :
			BGSEditorExtender::BGSEEWindowExtraData(),
			FilterEditBox(),
			FilterLabel(),
			XLabel(),
			YLabel(),
			XEdit(),
			YEdit(),
			GoBtn()
		{
			;//
		}

		CSEDialogExtraFittingsData::CSEDialogExtraFittingsData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			LastCursorPos.x = LastCursorPos.y = 0;
			LastCursorPosWindow = NULL;
			QuickViewTriggered = false;

			AssetControlToolTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
												TTS_ALWAYSTIP|TTS_NOPREFIX,
												CW_USEDEFAULT, CW_USEDEFAULT,
												CW_USEDEFAULT, CW_USEDEFAULT,
												NULL, NULL, NULL, NULL);
			ZeroMemory(&AssetControlToolData, sizeof(AssetControlToolData));
			AssetControlToolData.cbSize = sizeof(AssetControlToolData);
			LastTrackedTool = NULL;
			TrackingToolTip = false;
		}

		CSEDialogExtraFittingsData::~CSEDialogExtraFittingsData()
		{
			if (AssetControlToolTip)
				DestroyWindow(AssetControlToolTip);
		}

		CSEMainWindowMiscData::CSEMainWindowMiscData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			ToolbarExtras = Subwindow::CreateInstance();
		}

		CSEMainWindowMiscData::~CSEMainWindowMiscData()
		{
			if (ToolbarExtras)
			{
				ToolbarExtras->TearDown();
				ToolbarExtras->DeleteInstance();
			}
		}

		CSEMainWindowToolbarData::CSEMainWindowToolbarData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			SettingTODSlider = false;
		}

		CSEMainWindowToolbarData::~CSEMainWindowToolbarData()
		{
			;//
		}

		CSERenderWindowMiscData::CSERenderWindowMiscData() :
			BGSEditorExtender::BGSEEWindowExtraData()
		{
			TunnellingKeyMessage = false;
		}

		CSERenderWindowMiscData::~CSERenderWindowMiscData()
		{
			;//
		}

		LRESULT CALLBACK FindTextDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_NOTIFY:
				switch (((LPNMHDR)lParam)->code)
				{
				case LVN_ITEMACTIVATE:
					NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
					TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hdr.hwndFrom, Data->iItem);

					if (Data->hdr.idFrom == 1018 && Form)
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

					break;
				}

				break;
			case WM_DESTROY:
				break;
			}

			return DlgProcResult;
		}

#define WM_DATADLG_RECURSEMASTERS				(WM_USER + 2002)
		// lParam = TESFile*

		LRESULT CALLBACK DataDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			TESFile* ActiveTESFile = *((TESFile**)0x00A0AA7C);
			HWND PluginList = GetDlgItem(hWnd, 1056);

			switch (uMsg)
			{
			case WM_DATADLG_RECURSEMASTERS:
				{
					Return = true;

					TESFile* PluginFile = (TESFile*)lParam;

					if (PluginFile)
					{
						for (int i = 0; i < PluginFile->masterCount; i++)
							SendMessage(hWnd, WM_DATADLG_RECURSEMASTERS, NULL, (LPARAM)PluginFile->masterFiles[i]);

						PluginFile->SetLoaded(true);
					}
				}

				break;
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

										if (CurrentFile->IsMaster())
										{
											DrawData->clrTextBk = RGB(242, 247, 243);
										}

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

					if (Return == false)
					{
						CSEFormEnumerationManager::Instance.ResetVisibility();
						CLIWrapper::Interfaces::SE->CloseAllOpenEditors();

						if (ActiveTESFile)
							SendMessage(hWnd, WM_DATADLG_RECURSEMASTERS, NULL, (LPARAM)ActiveTESFile);
						else
						{
							int Selection = -1;
							do
							{
								Selection = ListView_GetNextItem(PluginList, Selection, LVNI_SELECTED);
								if (Selection == -1)
									break;

								UInt32 PluginIndex = (UInt32)TESListView::GetItemData(PluginList, Selection);
								TESFile* CurrentFile = _DATAHANDLER->LookupPluginByIndex(PluginIndex);

								if (CurrentFile && CurrentFile->IsLoaded())
									SendMessage(hWnd, WM_DATADLG_RECURSEMASTERS, NULL, (LPARAM)CurrentFile);
							}
							while (true);
						}
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

		LRESULT CALLBACK MainWindowMenuInitSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
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
									if (*TESRenderWindow::PathGridEditFlag)
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
								case IDC_MAINMENU_SAVEOPTIONS_CREATEBACKUPBEFORESAVING:
									if (atoi(INISettings::GetVersionControl()->Get(INISettings::kVersionControl_BackupOnSave, BGSEEMAIN->INIGetter())))
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
								case IDC_MAINMENU_SORTACTIVEFORMSFIRST:
									if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_SortFormListsByActiveForm, BGSEEMAIN->INIGetter())))
										CheckItem = true;

									break;
								case IDC_MAINMENU_COLORIZEACTIVEFORMS:
									if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ColorizeActiveForms, BGSEEMAIN->INIGetter())))
										CheckItem = true;

									break;
								case IDC_MAINMENU_COLORIZEFORMOVERRIDES:
									if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ColorizeFormOverrides, BGSEEMAIN->INIGetter())))
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

		LRESULT CALLBACK MainWindowMenuSelectSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMMAND:
				Return = true;

				switch (LOWORD(wParam))
				{
				case 40157:		// Help > Contents
				case 40413:		// Character > Export Dialogue
					{
						if (Achievements::kOldestTrickInTheBook->GetUnlocked() == false)
						{
							ShellExecute(NULL, "open", "http://www.youtube.com/watch?v=oHg5SJYRHA0", NULL, NULL, SW_SHOWNORMAL);
							BGSEEACHIEVEMENTS->Unlock(Achievements::kOldestTrickInTheBook);
						}
						else
							Return = false;
					}

					break;
				case IDC_MAINMENU_SAVEAS:
					{
						if (_DATAHANDLER->activeFile == NULL)
						{
							BGSEEUI->MsgBoxE("An active plugin must be set before using this tool.");
							break;
						}

						*TESCSMain::AllowAutoSaveFlag = 0;

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
								TESCSMain::SetTitleModified(false);
							}
							else
							{
								_DATAHANDLER->activeFile = SaveAsBuffer;

								SaveAsBuffer->SetActive(true);
								SaveAsBuffer->SetLoaded(true);
							}

							BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						}

						*TESCSMain::AllowAutoSaveFlag = 1;
					}

					break;
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
				case IDC_MAINMENU_LAUNCHGAME:
					{
						std::string AppPath = BGSEEMAIN->GetAPPPath();
						AppPath += "\\";

						IFileStream SteamLoader;
						if (SteamLoader.Open((std::string(AppPath + "obse_steam_loader.dll")).c_str()) == false)
							AppPath += "obse_loader.exe";
						else
							AppPath += "Oblivion.exe";

						ShellExecute(NULL, "open", (LPCSTR)AppPath.c_str(), NULL, NULL, SW_SHOW);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kLazyBum);
					}

					break;
				case IDC_MAINMENU_CREATEGLOBALSCRIPT:
					BGSEEUI->ModelessDialog(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_GLOBALSCRIPT), hWnd, (DLGPROC)CreateGlobalScriptDlgProc);
					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

					break;
				case IDC_MAINMENU_TAGBROWSER:
					CLIWrapper::Interfaces::TAG->ShowTagBrowserDialog(NULL);
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
				case IDC_MAINMENU_SAVEOPTIONS_CREATEBACKUPBEFORESAVING:
					if (atoi(INISettings::GetVersionControl()->Get(INISettings::kVersionControl_BackupOnSave, BGSEEMAIN->INIGetter())))
						INISettings::GetVersionControl()->Set(INISettings::kVersionControl_BackupOnSave, BGSEEMAIN->INISetter(), "0");
					else
						INISettings::GetVersionControl()->Set(INISettings::kVersionControl_BackupOnSave, BGSEEMAIN->INISetter(), "1");

					break;
				case IDC_MAINMENU_SORTACTIVEFORMSFIRST:
					if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_SortFormListsByActiveForm, BGSEEMAIN->INIGetter())))
						INISettings::GetDialogs()->Set(INISettings::kDialogs_SortFormListsByActiveForm, BGSEEMAIN->INISetter(), "0");
					else
						INISettings::GetDialogs()->Set(INISettings::kDialogs_SortFormListsByActiveForm, BGSEEMAIN->INISetter(), "1");

					break;
				case IDC_MAINMENU_COLORIZEACTIVEFORMS:
					if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ColorizeActiveForms, BGSEEMAIN->INIGetter())))
						INISettings::GetDialogs()->Set(INISettings::kDialogs_ColorizeActiveForms, BGSEEMAIN->INISetter(), "0");
					else
						INISettings::GetDialogs()->Set(INISettings::kDialogs_ColorizeActiveForms, BGSEEMAIN->INISetter(), "1");

					break;
				case IDC_MAINMENU_COLORIZEFORMOVERRIDES:
					if (atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ColorizeFormOverrides, BGSEEMAIN->INIGetter())))
						INISettings::GetDialogs()->Set(INISettings::kDialogs_ColorizeFormOverrides, BGSEEMAIN->INISetter(), "0");
					else
						INISettings::GetDialogs()->Set(INISettings::kDialogs_ColorizeFormOverrides, BGSEEMAIN->INISetter(), "1");

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

		LRESULT CALLBACK MainWindowMiscSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
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

					CSEMainWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowMiscData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSEMainWindowMiscData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_MAINWINDOW_INITEXTRADATA:
				{
					CSEMainWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowMiscData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSEMainWindowMiscData();

						xData->ToolbarExtras->hInstance = BGSEEMAIN->GetExtenderHandle();
						xData->ToolbarExtras->hDialog = *TESCSMain::MainToolbarHandle;
						xData->ToolbarExtras->hContainer = *TESCSMain::MainToolbarHandle;
						xData->ToolbarExtras->position.x = 485;
						xData->ToolbarExtras->position.y = 0;

						if (xData->ToolbarExtras->Build(IDD_TOOLBAREXTRAS) == false)
						{
							BGSEECONSOLE_ERROR("Couldn't build main window toolbar subwindow!");
						}
						else
						{
							BGSEEUI->GetSubclasser()->RegisterRegularWindowSubclass(*TESCSMain::MainToolbarHandle, MainWindowToolbarSubClassProc);
							SendMessage(*TESCSMain::MainToolbarHandle, WM_INITDIALOG, NULL, NULL);

							HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
							HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

							TESDialog::ClampDlgEditField(TODEdit, 0.0, 24.0);

							SendMessage(TODSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 23));
							SendMessage(TODSlider, TBM_SETLINESIZE, NULL, 1);
							SendMessage(TODSlider, TBM_SETPAGESIZE, NULL, 4);

							SendMessage(*TESCSMain::MainToolbarHandle, WM_MAINTOOLBAR_SETTOD, _TES->GetSkyTOD() * 4.0, NULL);
						}

						ExtraData->Add(xData);
					}
				}

				break;
			case WM_TIMER:
				DlgProcResult = TRUE;
				Return = true;

				switch (wParam)
				{
				case 2:				// autosave timer, needs to be handled here as the org wndproc doesn't compare the timerID
					if ((*TESCSMain::AllowAutoSaveFlag) != 0 && (*TESCSMain::ExittingCSFlag) == 0)
					{
						TESCSMain::AutoSave();
					}

					break;
				case ID_PATHGRIDTOOLBARBUTTION_TIMERID:
					{
						TBBUTTONINFO PathGridData = {0};
						PathGridData.cbSize = sizeof(TBBUTTONINFO);
						PathGridData.dwMask = TBIF_STATE;

						SendMessage(*TESCSMain::MainToolbarHandle, TB_GETBUTTONINFO, 40195, (LPARAM)&PathGridData);
						if ((PathGridData.fsState & TBSTATE_CHECKED) == false && *TESRenderWindow::PathGridEditFlag)
						{
							PathGridData.fsState |= TBSTATE_CHECKED;
							SendMessage(*TESCSMain::MainToolbarHandle, TB_SETBUTTONINFO, 40195, (LPARAM)&PathGridData);
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

		LRESULT CALLBACK MainWindowToolbarSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
			HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSEMainWindowToolbarData();
						ExtraData->Add(xData);
					}
				}

				break;
			case WM_DESTROY:
				{
					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSEMainWindowToolbarData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_COMMAND:
				{
					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					SME_ASSERT(xData);

					if (HIWORD(wParam) == EN_CHANGE &&
						LOWORD(wParam) == IDC_TOOLBAR_TODCURRENT &&
						xData->SettingTODSlider == false)
					{
						xData->SettingTODSlider = true;
						float TOD = TESDialog::GetDlgItemFloat(hWnd, IDC_TOOLBAR_TODCURRENT);
						SendMessage(hWnd, WM_MAINTOOLBAR_SETTOD, TOD * 4.0, NULL);
						xData->SettingTODSlider = false;
					}
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

					CSEMainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(CSEMainWindowToolbarData, ExtraData);
					SME_ASSERT(xData);

					if (xData->SettingTODSlider == false)
						TESDialog::SetDlgItemFloat(hWnd, IDC_TOOLBAR_TODCURRENT, TOD, 2);

					TESPreviewControl::UpdatePreviewWindows();
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK RenderWindowMenuInitSelectSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
																bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = TRUE;
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
									if (TESRenderWindow::FreezeInactiveRefs)
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
								case IDC_RENDERWINDOWCONTEXT_STATICCAMERAPIVOT:
									if (atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_FixedCameraPivot, BGSEEMAIN->INIGetter())))
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
				case IDC_RENDERWINDOWCONTEXT_STATICCAMERAPIVOT:
					{
						bool Enabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_FixedCameraPivot, BGSEEMAIN->INIGetter()));
						Enabled = (Enabled == false);

						INISettings::GetRenderer()->Set(INISettings::kRenderer_FixedCameraPivot, BGSEEMAIN->INISetter(), "%d", Enabled);

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_UNLOADCURRENTCELLS:
					TESRenderWindow::Reset();

					break;
				case IDC_RENDERWINDOWCONTEXT_SWITCHCNY:
					{
						bool Enabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						Enabled = (Enabled == false);

						INISettings::GetRenderer()->Set(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INISetter(), "%d", Enabled);

						Return = true;
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_FREEZEINACTIVE:
					TESRenderWindow::FreezeInactiveRefs = (TESRenderWindow::FreezeInactiveRefs == false);

					if (TESRenderWindow::FreezeInactiveRefs)
						RenderWindowPainter::RenderChannelNotifications->Queue(4, "Inactive references frozen");
					else
						RenderWindowPainter::RenderChannelNotifications->Queue(4, "Inactive references thawed");

					break;
				case IDC_RENDERWINDOWCONTEXT_INVERTSELECTION:
					if (*TESRenderWindow::PathGridEditFlag == 0)
					{
						TESObjectCELL* CurrentCell = _TES->currentInteriorCell;

						if (CurrentCell == NULL)
							CurrentCell = *TESRenderWindow::CurrentlyLoadedCell;

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
							TESRenderWindow::Redraw();
						}
					}

					break;
				case IDC_RENDERWINDOWCONTEXT_BATCHREFERENCEEDITOR:
					{
						TESObjectCELL* ThisCell = _TES->currentInteriorCell;
						if (!ThisCell)
							ThisCell = *TESRenderWindow::CurrentlyLoadedCell;

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
											ThisRef->SetQuestItem(BatchData->Flags.Persistent);
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
				case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
				case IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL:
					{
						TESObjectCELL* CurrentCell = _TES->currentInteriorCell;

						if (CurrentCell == NULL)
							CurrentCell = *TESRenderWindow::CurrentlyLoadedCell;

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
									if (Ref->GetInvisible())
										Ref->ToggleInvisiblity();

									if (Ref->GetChildrenInvisible())
										Ref->ToggleChildrenInvisibility();

									break;
								case IDC_RENDERWINDOWCONTEXT_THAWALLINCELL:
									Ref->SetFrozenState(false);

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

						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						TESRenderWindow::Redraw();
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

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY:
							Ref->ToggleInvisiblity();

							break;
						case IDC_RENDERWINDOWCONTEXT_TOGGLECHILDRENVISIBILITY:
							Ref->ToggleChildrenInvisibility();

							break;
						case IDC_RENDERWINDOWCONTEXT_FREEZE:
							Ref->SetFrozenState(true);

							break;
						case IDC_RENDERWINDOWCONTEXT_THAW:
							Ref->SetFrozenState(false);

							break;
						}
					}

					BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
					TESRenderWindow::Redraw();
					Return = true;

					break;
				case IDC_RENDERWINDOWCONTEXT_GROUP:
				case IDC_RENDERWINDOWCONTEXT_UNGROUP:
					if (_RENDERSEL->selectionCount > 1)
					{
						TESObjectCELL* CurrentCell = _TES->currentInteriorCell;

						if (CurrentCell == NULL)
							CurrentCell = *TESRenderWindow::CurrentlyLoadedCell;

						if (CurrentCell == NULL)
							break;

						switch (LOWORD(wParam))
						{
						case IDC_RENDERWINDOWCONTEXT_GROUP:
							if (!RenderSelectionGroupManager::Instance.AddGroup(CurrentCell, _RENDERSEL))
							{
								BGSEEUI->MsgBoxW(hWnd, 0,
												"Couldn't add current selection to a new group.\n\nMake sure none of the selected objects belong to a preexisting group.");
							}
							else
								RenderWindowPainter::RenderChannelNotifications->Queue(2, "Created new selection group for current cell");

							break;
						case IDC_RENDERWINDOWCONTEXT_UNGROUP:
							if (!RenderSelectionGroupManager::Instance.RemoveGroup(CurrentCell, _RENDERSEL))
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
						_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);
						_RENDERUNDO->RecordReference(TESRenderWindow::UndoStack::kUndoOperation_Unk03, _RENDERSEL->selectionList);

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

							ThisRef->SetFromActiveFile(true);
							ThisRef->UpdateNiNode();
						}

						RenderWindowPainter::RenderChannelNotifications->Queue(2, "Selection aligned to %08X", AlignRef->formID);
						BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);
						TESRenderWindow::Redraw();

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

		LRESULT CALLBACK RenderWindowMiscSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = TRUE;
			Return = false;

			UInt8* YKeyState = (UInt8*)0x00A0BC1E;
			float* UnkRotFactor = (float*)0x00A0BAC4;

			static Vector3 kCameraStaticPivot;

			switch (uMsg)
			{
			case WM_RENDERWINDOW_UPDATEFOV:
				{
					Return = true;

					float CameraFOV = atof(INISettings::GetRenderer()->Get(INISettings::kRenderer_CameraFOV, BGSEEMAIN->INIGetter()));
					if (CameraFOV > 120.0f)
						CameraFOV = 120.0f;
					else if (CameraFOV < 50.0f)
						CameraFOV = 50.0f;

					TESRender::SetCameraFOV(_PRIMARYRENDERER->primaryCamera, CameraFOV);
				}

				break;
			case WM_INITDIALOG:
				{
					CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSERenderWindowMiscData();
						ExtraData->Add(xData);
					}
				}

				break;
			case WM_DESTROY:
				{
					CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSERenderWindowMiscData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_RENDERWINDOW_GETCAMERASTATICPIVOT:
				{
					SetWindowLongPtr(hWnd, DWLP_MSGRESULT, (LONG_PTR)&kCameraStaticPivot);
					DlgProcResult = TRUE;
					Return = true;
				}

				break;
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, 40423, NULL);
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

					if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kLODDiffuseMapGeneratorState_NotInUse)
					{
						// prevent the OS from triggering the screen-saver/switching to standby mode
						SetThreadExecutionState(ES_CONTINUOUS|ES_DISPLAY_REQUIRED|ES_SYSTEM_REQUIRED);
					}

					break;
				}

				break;
			case WM_SIZING:
				{
					if (TESPreviewControl::ActivePreviewControls->Count())
					{
						BGSEEUI->MsgBoxW(hWnd, 0, "Please close any dialogs with preview controls before attempting to resize the render window.");

						Return = true;
					}
				}

				break;
			case WM_LBUTTONDOWN:
				TESRenderWindow::CurrentMouseCoordDelta.x = GET_X_LPARAM(lParam);
				TESRenderWindow::CurrentMouseCoordDelta.y = GET_Y_LPARAM(lParam);

				break;
			case WM_LBUTTONUP:
				TESRenderWindow::CurrentMouseCoordDelta.x -= GET_X_LPARAM(lParam);
				TESRenderWindow::CurrentMouseCoordDelta.y -= GET_Y_LPARAM(lParam);

				TESRenderWindow::CurrentMouseCoordDelta.x = abs(TESRenderWindow::CurrentMouseCoordDelta.x);
				TESRenderWindow::CurrentMouseCoordDelta.y = abs(TESRenderWindow::CurrentMouseCoordDelta.y);

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
				case VK_SHIFT:
					{
						_PRIMARYRENDERER->GetCameraPivot(&kCameraStaticPivot, 0.18);
					}

					break;
				case VK_F1:		// F1
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						TESDialog::ShowUseReportDialog(Ref);
					}

					Return = true;

					break;
				case 0x5A:		// Z
					if (*TESRenderWindow::PathGridEditFlag && GetAsyncKeyState(VK_CONTROL))
					{
						PathGridUndoManager::Instance.PerformUndo();
						Return = true;
					}

					break;
				case 0x59:		// Y
					if (*TESRenderWindow::PathGridEditFlag && GetAsyncKeyState(VK_CONTROL))
					{
						PathGridUndoManager::Instance.PerformRedo();
						Return = true;
					}
					else
					{
						int SwitchEnabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
						SME_ASSERT(xData);

						if (SwitchEnabled)
						{
							xData->TunnellingKeyMessage = true;
							SendMessage(hWnd, WM_KEYDOWN, 0x43, lParam);
							xData->TunnellingKeyMessage = false;

							Return = true;
						}
					}

					break;
				case 0x43:		// C
					{
						int SwitchEnabled = atoi(INISettings::GetRenderer()->Get(INISettings::kRenderer_SwitchCAndY, BGSEEMAIN->INIGetter()));
						CSERenderWindowMiscData* xData = BGSEE_GETWINDOWXDATA(CSERenderWindowMiscData, ExtraData);
						SME_ASSERT(xData);

						if (SwitchEnabled && xData->TunnellingKeyMessage == false)
						{
							if (*YKeyState == 0)
								*UnkRotFactor = 0.0;

							*YKeyState = 1;

							Return = true;
						}
					}

					break;
				case 0x52:		// R
					if (*TESRenderWindow::PathGridEditFlag)
					{
						if (GetAsyncKeyState(VK_CONTROL))
						{
							PathGridUndoManager::Instance.ResetRedoStack();

							if (TESRenderWindow::SelectedPathGridPoints->Count())
								PathGridUndoManager::Instance.RecordOperation(PathGridUndoManager::kOperation_DataChange, TESRenderWindow::SelectedPathGridPoints);

							for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								TESPathGridPoint* Point = Itr.Get();
								Point->UnlinkFromReference();
								Point->HideSelectionRing();
							}

							TESRenderWindow::Redraw(true);
							BGSEEACHIEVEMENTS->Unlock(Achievements::kPowerUser);

							Return = true;
						}
						else
						{
							std::list<TESPathGridPoint*> Delinquents;

							for (tList<TESPathGridPoint>::Iterator Itr = TESRenderWindow::SelectedPathGridPoints->Begin(); !Itr.End() && Itr.Get(); ++Itr)
							{
								if (Itr.Get()->linkedRef)
									Delinquents.push_back(Itr.Get());

								Itr.Get()->HideSelectionRing();
							}

							for (std::list<TESPathGridPoint*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); Itr++)
								thisCall<void>(0x00452AE0, TESRenderWindow::SelectedPathGridPoints, *Itr);
						}
					}

					break;
				case 0x51:		// Q
					if (GetAsyncKeyState(VK_CONTROL))
					{
						if (TESRenderWindow::UseAlternateMovementSettings)
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Using vanilla movement settings");
						else
							RenderWindowPainter::RenderChannelNotifications->Queue(3, "Using alternate movement settings");

						TESRenderWindow::UseAlternateMovementSettings = (TESRenderWindow::UseAlternateMovementSettings == false);
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
					if (GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(VK_MENU))
					{
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_INVERTSELECTION, NULL);

						Return = true;
					}

					break;
				case 0x32:		// 2
					SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_TOGGLEVISIBILITY, NULL);
					TESRenderWindow::Redraw();

					Return = true;
					break;
				case 0x47:		// G
					SendMessage(BGSEEUI->GetMainWindow(), WM_COMMAND, 40195, NULL);

					Return = true;
					break;
				case VK_F5:
					{
						SendMessage(hWnd, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);
						SendMessage(hWnd, WM_COMMAND, IDC_RENDERWINDOWCONTEXT_REVEALALLINCELL, NULL);
					}

					break;
				case VK_OEM_3:	// ~
					{
						if (TESLODTextureGenerator::GeneratorState != TESLODTextureGenerator::kLODDiffuseMapGeneratorState_NotInUse)
							break;
						else if (GetCapture())
							break;

						BGSEditorExtender::BGSEERenderWindowFlyCamera* xFreeCamData = BGSEE_GETWINDOWXDATA(BGSEditorExtender::BGSEERenderWindowFlyCamera, ExtraData);
						SME_ASSERT(xFreeCamData == NULL);

						xFreeCamData = new BGSEditorExtender::BGSEERenderWindowFlyCamera(new CSERenderWindowFlyCameraOperator(hWnd,
																															TESDialog::kDialogTemplate_RenderWindow));
						ExtraData->Add(xFreeCamData);

						Return = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK ObjectWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
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
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, 40199, NULL);
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
					MoveWindow(FormList, Point.x, Point.y, (unsigned __int16)lParam - Point.x - 6, (unsigned int)(lParam >> 16) - Point.y - 6, 1);

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

		LRESULT CALLBACK CellViewWindowSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
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
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, 40200, NULL);
				Return = true;

				break;
			case WM_DESTROY:
				{
					CSECellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CSECellViewExtraData, ExtraData);

					if (xData)
					{
						ExtraData->Remove(CSECellViewExtraData::kTypeID);
						delete xData;
					}
				}
			case 0x417:		// destroy window
				CSEFilterableFormListManager::Instance.Unregister(hWnd);

				break;
			case WM_INITDIALOG:
				{
					CSECellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CSECellViewExtraData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSECellViewExtraData();
						ExtraData->Add(xData);

						RECT Bounds = {0};

						SME::UIHelpers::GetClientRectInitBounds(FilterEditBox, hWnd, &xData->FilterEditBox);
						SME::UIHelpers::GetClientRectInitBounds(FilterLabel, hWnd, &xData->FilterLabel);
						SME::UIHelpers::GetClientRectInitBounds(XLabel, hWnd, &xData->XLabel);
						SME::UIHelpers::GetClientRectInitBounds(YLabel, hWnd, &xData->YLabel);
						SME::UIHelpers::GetClientRectInitBounds(XEdit, hWnd, &xData->XEdit);
						SME::UIHelpers::GetClientRectInitBounds(YEdit, hWnd, &xData->YEdit);
						SME::UIHelpers::GetClientRectInitBounds(GoBtn, hWnd, &xData->GoBtn);

						TESDialog::GetPositionFromINI("Cell View", &Bounds);
						SetWindowPos(hWnd, NULL, Bounds.left, Bounds.top, Bounds.right, Bounds.bottom, SWP_NOZORDER);
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

					SetRect(&CurrentRect, 0, 0, LOWORD(lParam), HIWORD(lParam));
					int DeltaDlgWidth = (CurrentRect.right - BaseDlgRect->right) >> 1;
					HDWP DeferPosData = BeginDeferWindowPos(3);

					DeferWindowPos(DeferPosData, CellList, 0,
								0, 0,
								DeltaDlgWidth + BaseCellListRect->right, CurrentRect.bottom + BaseCellListRect->bottom - BaseDlgRect->bottom,
								SWP_NOMOVE);

					DeferWindowPos(DeferPosData, WorldspaceCombo, 0,
								0, 0,
								BaseCellListRect->right + DeltaDlgWidth - BaseWorldspaceLabelRect->right, BaseWorldspaceComboRect->bottom,
								SWP_NOMOVE);

					DeferWindowPos(DeferPosData, RefList, 0,
								DeltaDlgWidth + BaseRefListRect->left, BaseRefListRect->top,
								DeltaDlgWidth + BaseRefListRect->right, CurrentRect.bottom + BaseRefListRect->bottom - BaseDlgRect->bottom,
								NULL);

					DeferWindowPos(DeferPosData, CellLabel, 0,
								BaseCellNameLabelRect->left + DeltaDlgWidth, BaseCellNameLabelRect->top,
								DeltaDlgWidth + BaseCellNameLabelRect->right, BaseCellNameLabelRect->bottom,
								NULL);

					InvalidateRect(CellLabel, NULL, TRUE);
					InvalidateRect(WorldspaceLabel, NULL, TRUE);

					EndDeferWindowPos(DeferPosData);

					CSECellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CSECellViewExtraData, ExtraData);

					if (xData)
					{
						DeferPosData = BeginDeferWindowPos(7);

						DeferWindowPos(DeferPosData, XLabel, 0,
							DeltaDlgWidth + xData->XLabel.left, xData->XLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, XEdit, 0,
							DeltaDlgWidth + xData->XEdit.left, xData->XEdit.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, YLabel, 0,
							DeltaDlgWidth + xData->YLabel.left, xData->YLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, YEdit, 0,
							DeltaDlgWidth + xData->YEdit.left, xData->YEdit.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, GoBtn, 0,
							DeltaDlgWidth + xData->GoBtn.left, xData->GoBtn.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, FilterLabel, 0,
							DeltaDlgWidth + xData->FilterLabel.left, xData->FilterLabel.top,
							0, 0,
							SWP_NOSIZE);

						DeferWindowPos(DeferPosData, FilterEditBox, 0,
							DeltaDlgWidth + xData->FilterEditBox.left, xData->FilterEditBox.top,
							DeltaDlgWidth + xData->FilterEditBox.right, xData->FilterEditBox.bottom + 2,
							NULL);

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

		LRESULT CALLBACK ResponseDlgSubclassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
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

						if (CSIOM->GenerateLIPSyncFile(Path.c_str(), (*ResponseEditorData::EditorCache)->responseLocalCopy->responseText.c_str()) == false)
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

		LRESULT CALLBACK LandscapeTextureUseDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
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
						*TESRenderWindow::ActiveLandscapeTexture = CS_CAST(Texture, TESForm, TESLandTexture);

						SendMessage(*TESObjectLAND::WindowHandle, 0x41A, NULL, NULL);			// select the new texture in the landscape edit dialog
						SetForegroundWindow(*TESRenderWindow::WindowHandle);

						RenderWindowPainter::RenderChannelNotifications->Queue(3, "Active landscape texture changed");
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK FilteredDialogQuestDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData =
					(BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);

					if (UserData->TemplateID == TESDialog::kDialogTemplate_Quest)
					{
						LVCOLUMN ColumnData = {0};
						ColumnData.mask = LVCF_WIDTH;
						HWND QuestList = GetDlgItem(hWnd, 2064);

						ColumnData.cx = 200;
						ListView_SetColumn(QuestList, 0, &ColumnData);

						ColumnData.cx = 42;
						ListView_SetColumn(QuestList, 2, &ColumnData);
					}
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_QUEST_EDITRESULTSCRIPT:
					if (IsWindowEnabled(GetDlgItem(hWnd, 1444)))
					{
						if (BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
												MAKEINTRESOURCE(IDD_EDITRESULTSCRIPT),
												hWnd,
												UIManager::EditResultScriptDlgProc,
												(LPARAM)hWnd))
						{
							SendMessage(hWnd, WM_COMMAND, 1591, NULL);		// compile result script
						}
					}

					Return = true;
					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK AboutDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					HINSTANCE SplashImageResource = BGSEEUI->GetDialogHotSwapper()->GetAlternateResourceInstance(TESDialog::kDialogTemplate_SplashScreen);
					if (SplashImageResource)
					{
						HANDLE Image = LoadImage(SplashImageResource,
												MAKEINTRESOURCE(100),
												IMAGE_BITMAP,
												0,
												0,
												LR_DEFAULTSIZE);

						SME_ASSERT(Image);

						HWND PictureControl = GetDlgItem(hWnd, 1963);
						SendMessage(PictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Image);
					}

					DWORD FileVersionHandle = 0;
					DWORD FileVersionSize = GetFileVersionInfoSize(BGSEEMAIN->GetDLLPath(), &FileVersionHandle);

					if (FileVersionSize)
					{
						char* Buffer = new char[FileVersionSize];
						char VersionString[0x100] = {0};
						void* VersionStringPtr = NULL;

						GetFileVersionInfo(BGSEEMAIN->GetDLLPath(), FileVersionHandle, FileVersionSize, Buffer);
						VerQueryValue(Buffer, "\\StringFileInfo\\040904b0\\ProductVersion", &VersionStringPtr, (PUINT)FileVersionHandle);
						FORMAT_STR(VersionString, "%s v%s\r\n\"%s\"", BGSEEMAIN->ExtenderGetLongName(),
																	VersionStringPtr,
																	BGSEEMAIN->ExtenderGetReleaseName());

						std::string ReplacedString(VersionString);
						std::replace(ReplacedString.begin(), ReplacedString.end(), ',', '.');

						SetDlgItemText(hWnd, 1580, (LPCSTR)ReplacedString.c_str());

						delete [] Buffer;
					}
				}

				break;
			case WM_DESTROY:
				{
					HANDLE Image = (HANDLE)SendDlgItemMessage(hWnd, 1963, STM_GETIMAGE, IMAGE_BITMAP, NULL);

					if (Image)
						DeleteBitmap(Image);
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK RaceDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_RACE_COPYEYES:
				case IDC_CSE_RACE_COPYHAIR:
					{
						TESRace* WorkingRace = CS_CAST(TESDialog::GetDialogExtraLocalCopy(hWnd), TESForm, TESRace);
						TESForm* Selection = (TESForm*)BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
																			MAKEINTRESOURCE(IDD_TESCOMBOBOX),
																			hWnd,
																			(DLGPROC)UIManager::TESComboBoxDlgProc,
																			(LPARAM)TESForm::kFormType_Race);

						if (Selection)
						{
							TESRace* SelectedRace = CS_CAST(Selection, TESForm, TESRace);

							if (WorkingRace && WorkingRace != SelectedRace)
							{
								int Count = 0;
								switch (LOWORD(wParam))
								{
								case IDC_CSE_RACE_COPYEYES:
									{
										tList<TESEyes>* Source = &SelectedRace->eyeList;
										tList<TESEyes>* Destination = &WorkingRace->eyeList;

										for (tList<TESEyes>::Iterator Itr = Source->Begin(); !Itr.End() && Itr.Get(); ++Itr)
										{
											if (Destination->IndexOf(Itr.Get()) == -1)
											{
												Destination->AddAt(Itr.Get(), eListEnd);
												Count++;
											}
										}

										BGSEEUI->MsgBoxI(hWnd, 0, "Copied %d eye forms from race '%s'.", Count, SelectedRace->editorID.c_str());
									}

									break;
								case IDC_CSE_RACE_COPYHAIR:
									{
										tList<TESHair>* Source = &SelectedRace->hairList;
										tList<TESHair>* Destination = &WorkingRace->hairList;

										for (tList<TESHair>::Iterator Itr = Source->Begin(); !Itr.End() && Itr.Get(); ++Itr)
										{
											if (Destination->IndexOf(Itr.Get()) == -1)
											{
												Destination->AddAt(Itr.Get(), eListEnd);
												Count++;
											}
										}

										BGSEEUI->MsgBoxI(hWnd, 0, "Copied %d hair forms from race '%s'.", Count, SelectedRace->editorID.c_str());
									}

									break;
								}
							}
						}
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

#define ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID						0x108
#define ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID					0x109

		enum
		{
			kFormList_ObjectWindowObjects						= 1041,
			kFormList_TESPackage								= 1977,
			kFormList_CellViewCells								= 1155,
			kFormList_CellViewRefs								= 1156,
			kFormList_TESFormIDListView							= 2064,
			kFormList_DialogEditorTopics						= 1448,
			kFormList_DialogEditorTopicInfos					= 1449,
			kFormList_DialogEditorAddedTopics					= 1453,
			kFormList_DialogEditorLinkedToTopics				= 1456,
			kFormList_DialogEditorLinkedFromTopics				= 1455,
			kFormList_Generic									= 1018,
			kFormList_TESContainer								= 2035,
			kFormList_TESSpellList								= 1485,
			kFormList_ActorFactions								= 1088,
			kFormList_TESLeveledList							= 2036,
			kFormList_WeatherSounds								= 2286,
			kFormList_ClimateWeatherRaceHairFindTextTopics		= 1019,
			kFormList_RaceEyes									= 2163,
			kFormList_TESReactionForm							= 1591,
			kFormList_FindTextTopicInfos						= 1952,
			kFormList_LandTextures								= 1492,
		};

		enum
		{
			kAssetFileButton_Model								= 1043,		// includes idle animations and trees
			kAssetFileButton_Texture							= 1044,
			kAssetFileButton_Sound								= 1451,
			kAssetFileButton_Script								= 1226,		// not really an asset but meh
			kAssetFileButton_BipedModel_Male					= 1045,
			kAssetFileButton_BipedModel_Female					= 1046,
			kAssetFileButton_WorldModel_Male					= 2088,
			kAssetFileButton_WorldModel_Female					= 2091,
			kAssetFileButton_BipedIcon_Male						= 2089,
			kAssetFileButton_BipedIcon_Female					= 2092,
		};

		LRESULT CALLBACK CommonDialogExtraFittingsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;
			CSEDialogExtraFittingsData* xData = BGSEE_GETWINDOWXDATA_QUICK(CSEDialogExtraFittingsData, ExtraData);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					xData = BGSEE_GETWINDOWXDATA(CSEDialogExtraFittingsData, ExtraData);
					if (xData == NULL)
					{
						xData = new CSEDialogExtraFittingsData();
						ExtraData->Add(xData);
					}

					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID, 100, NULL);
					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID, 650, NULL);
				}

				break;
			case WM_DESTROY:
				{
					xData = BGSEE_GETWINDOWXDATA(CSEDialogExtraFittingsData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(CSEDialogExtraFittingsData::kTypeID);
						delete xData;

						xData = NULL;
					}

					KillTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID);
					KillTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID);
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID:
					{
						if (xData && xData->LastCursorPosWindow && xData->TrackingToolTip == false)
						{
							HWND WindowAtPoint = xData->LastCursorPosWindow;
							int CtrlID = GetDlgCtrlID(WindowAtPoint);

							if (CtrlID == kAssetFileButton_Model ||
								CtrlID == kAssetFileButton_Script ||
								CtrlID == kAssetFileButton_Sound ||
								CtrlID == kAssetFileButton_Texture ||
								CtrlID == kAssetFileButton_BipedModel_Male ||
								CtrlID == kAssetFileButton_BipedModel_Female ||
								CtrlID == kAssetFileButton_WorldModel_Male ||
								CtrlID == kAssetFileButton_WorldModel_Female ||
								CtrlID == kAssetFileButton_BipedIcon_Male ||
								CtrlID == kAssetFileButton_BipedIcon_Female)
							{
								// valid control, show tooltip
								char Buffer[MAX_PATH + 1] = {0};
								GetWindowText(WindowAtPoint, (LPSTR)Buffer, MAX_PATH);

								if (strlen(Buffer) > 1)
								{
									ZeroMemory(&xData->AssetControlToolData, sizeof(TOOLINFO));
									xData->AssetControlToolData.cbSize = sizeof(TOOLINFO);
									xData->AssetControlToolData.uFlags = TTF_TRACK;
									xData->AssetControlToolData.lpszText = Buffer;

									SendMessage(xData->AssetControlToolTip, TTM_ADDTOOL, NULL, (LPARAM)&xData->AssetControlToolData);
									SendMessage(xData->AssetControlToolTip,
												TTM_TRACKPOSITION,
												NULL,
												(LPARAM)MAKELONG(xData->LastCursorPos.x + 18, xData->LastCursorPos.y));
									SendMessage(xData->AssetControlToolTip, TTM_SETMAXTIPWIDTH, NULL, (LPARAM)355);
									SendMessage(xData->AssetControlToolTip, TTM_SETWINDOWTHEME, NULL, (LPARAM)L"Explorer");
									SendMessage(xData->AssetControlToolTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&xData->AssetControlToolData);

									xData->TrackingToolTip = true;
									xData->LastTrackedTool = WindowAtPoint;
								}
							}
						}

						else if (xData && xData->TrackingToolTip && xData->LastCursorPosWindow != xData->LastTrackedTool)
						{
							SendMessage(xData->AssetControlToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&xData->AssetControlToolData);
							SendMessage(xData->AssetControlToolTip, TTM_DELTOOL, NULL, (LPARAM)&xData->AssetControlToolData);

							xData->TrackingToolTip = false;
						}
					}

					break;
				case ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID:
					{
						// we need to defer the looked-up window's creation a bit to keep the source window from hogging focus
						if (xData && xData->LastCursorPosWindow && xData->QuickViewTriggered)
						{
							xData->QuickViewTriggered = false;

							HWND WindowAtPoint = xData->LastCursorPosWindow;
							TESForm* Form = NULL;

							char Buffer[0x200] = {0};
							GetClassName(WindowAtPoint, Buffer, sizeof(Buffer));

							if (!_stricmp("SysListView32", Buffer))
							{
								POINT Coords = {0};
								Coords.x = xData->LastCursorPos.x;
								Coords.y = xData->LastCursorPos.y;

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

							xData->LastCursorPosWindow = NULL;

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
												// valid listviews
						if (wParam == kFormList_ObjectWindowObjects ||
							wParam == kFormList_TESPackage ||
							wParam == kFormList_CellViewCells ||
							wParam == kFormList_CellViewRefs ||
							wParam == kFormList_TESFormIDListView ||
							wParam == kFormList_DialogEditorTopics ||
							wParam == kFormList_DialogEditorTopicInfos ||
							wParam == kFormList_DialogEditorAddedTopics ||
							wParam == kFormList_DialogEditorLinkedToTopics ||
							wParam == kFormList_DialogEditorLinkedFromTopics ||
							wParam == kFormList_Generic ||
							wParam == kFormList_TESContainer ||
							wParam == kFormList_TESSpellList ||
							wParam == kFormList_ActorFactions ||
							wParam == kFormList_TESLeveledList ||
							wParam == kFormList_WeatherSounds ||
							wParam == kFormList_ClimateWeatherRaceHairFindTextTopics ||
							wParam == kFormList_RaceEyes ||
							wParam == kFormList_TESReactionForm ||
							wParam == kFormList_FindTextTopicInfos ||
							wParam == kFormList_LandTextures)
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
									TESForm* Form = NULL;

									switch (wParam)
									{
									case kFormList_ActorFactions:
										{
											TESActorBase::FactionInfo* Data = (TESActorBase::FactionInfo*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Data);
											Form = Data->faction;
										}

										break;
									case kFormList_TESContainer:
										{
											TESContainer::ContentEntry* Entry = (TESContainer::ContentEntry*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Entry);
											Form = Entry->form;
										}

										break;
									case kFormList_TESLeveledList:
										{
											TESLeveledList::ListEntry* Entry = (TESLeveledList::ListEntry*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Entry);
											Form = Entry->form;
										}

										break;
									case kFormList_WeatherSounds:
										{
											UInt32* FormID = (UInt32*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(FormID);
											Form = TESForm::LookupByFormID(*FormID);
										}

										break;
									case kFormList_ClimateWeatherRaceHairFindTextTopics:
										{
											BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData =
											(BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);

											switch (UserData->TemplateID)
											{
											case TESDialog::kDialogTemplate_Climate:
												{
													TESClimate::WeatherEntry* Entry = (TESClimate::WeatherEntry*)DrawData->nmcd.lItemlParam;
													SME_ASSERT(Entry);
													Form = Entry->weather;
												}

												break;
											case TESDialog::kDialogTemplate_Race:
												{
													Form = (TESForm*)DrawData->nmcd.lItemlParam;
												}

												break;
											case TESDialog::kDialogTemplate_FindText:
												{
													FindTextWindowData::TopicSearchResult* Data = (FindTextWindowData::TopicSearchResult*)DrawData->nmcd.lItemlParam;
													SME_ASSERT(Data);
													Form = Data->topic;
												}

												break;
											}
										}

										break;
									case kFormList_TESReactionForm:
										{
											TESReactionForm::ReactionInfo* Info = (TESReactionForm::ReactionInfo*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Info);
											Form = Info->target;
										}

										break;
									default:
										Form = (TESForm*)DrawData->nmcd.lItemlParam;

										break;
									}

									bool ColorizeActiveFormsEnabled = atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ColorizeActiveForms,
																	BGSEEMAIN->INIGetter())) &&
																	CSEFormEnumerationManager::Instance.GetVisibleUnmodifiedForms();
									bool ColorizeFormOverridesEnabled =  atoi(INISettings::GetDialogs()->Get(INISettings::kDialogs_ColorizeFormOverrides,
																		BGSEEMAIN->INIGetter()));

									if (Form &&
										(ColorizeActiveFormsEnabled ||
										ColorizeFormOverridesEnabled))
									{
										COLORREF ForeColor, BackColor;
										bool ColorOverridden = false;

										if (ColorizeFormOverridesEnabled)
										{
											ColorOverridden = true;

											switch (Form->fileList.Count())
											{
											case 0:
												ForeColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel0ForeColor,
																					BGSEEMAIN->INIGetter()));
												BackColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel0BackColor,
																					BGSEEMAIN->INIGetter()));
												break;
											case 1:
												ForeColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel1ForeColor,
																					BGSEEMAIN->INIGetter()));
												BackColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel1BackColor,
																					BGSEEMAIN->INIGetter()));
												break;
											case 2:
												ForeColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel2ForeColor,
																					BGSEEMAIN->INIGetter()));
												BackColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel2BackColor,
																					BGSEEMAIN->INIGetter()));
												break;
											default:
												ForeColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel3ForeColor,
																					BGSEEMAIN->INIGetter()));
												BackColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																					INISettings::kDialogs_FormOverrideLevel3BackColor,
																					BGSEEMAIN->INIGetter()));
												break;
											}
										}

										if (Form->IsActive() && ColorizeActiveFormsEnabled)
										{
											ColorOverridden = true;

											ForeColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																				INISettings::kDialogs_ActiveFormForeColor,
																				BGSEEMAIN->INIGetter()));
											BackColor = SME::StringHelpers::GetRGB(INISettings::GetDialogs()->Get(
																				INISettings::kDialogs_ActiveFormBackColor,
																				BGSEEMAIN->INIGetter()));
										}

										TESObjectREFR* Reference = CS_CAST(Form, TESForm, TESObjectREFR);

										if (Reference &&
											Reference->GetNiNode() &&
											(Reference->GetNiNode()->m_flags & NiAVObject::kFlag_AppCulled))
										{
											ColorOverridden = true;

											ForeColor = RGB(255, 0, 255);
											BackColor = RGB(255, 255, 255);
										}

										if (ColorOverridden)
										{
											DrawData->clrText = ForeColor;
											DrawData->clrTextBk = BackColor;

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
			case WM_MOUSEACTIVATE:
			case WM_SETCURSOR:		// the mouse move message isn't dispatched if the pointer is over a dialog control, but this one is
			case WM_MBUTTONUP:
				{
					if (uMsg == WM_MOUSEACTIVATE)
					{
						if (HIWORD(lParam) != WM_MBUTTONDOWN &&
							HIWORD(lParam) != WM_MBUTTONUP)
						{
							break;
						}
					}
					else if (uMsg == WM_SETCURSOR)
					{
						if (HIWORD(lParam) != WM_MOUSEMOVE)
						{
							break;
						}
					}

					POINT Coords = {0};

					if (uMsg == WM_MOUSEACTIVATE ||
						uMsg == WM_SETCURSOR)
					{
						GetCursorPos(&Coords);
					}
					else
					{
						Coords.x = GET_X_LPARAM(lParam);
						Coords.y = GET_Y_LPARAM(lParam);
						ClientToScreen(hWnd, &Coords);
					}

					if (xData)
					{
						xData->LastCursorPos.x = Coords.x;
						xData->LastCursorPos.y = Coords.y;
						xData->LastCursorPosWindow = WindowFromPoint(Coords);

						if (uMsg == WM_MBUTTONUP ||
							uMsg == WM_MOUSEACTIVATE)
						{
							xData->QuickViewTriggered = true;
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

		LRESULT CALLBACK SelectTopicsQuestsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
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

					ColumnData.cx = 360;
					ListView_SetColumn(FormList, 0, &ColumnData);

					SetWindowLongPtr(FormList, GWL_USERDATA, NULL);
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
							int SortOrder = GetWindowLongPtr(FormList, GWL_USERDATA);

							SortOrder = (SortOrder == 0);
							SendMessage(FormList, LVM_SORTITEMS, SortOrder, (LPARAM)SelectTopicsQuestsFormListComparator);

							SetWindowLongPtr(FormList, GWL_USERDATA, (LONG)SortOrder);

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

#define WM_TESFORMIDLISTVIEW_HASCHANGES							(WM_USER + 2006)
		// return TRUE if there are changes
#define WM_TESFORMIDLISTVIEW_SAVECHANGES						(WM_USER + 2007)

		LRESULT CALLBACK TESFormIDListViewDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData* UserData =
			(BGSEditorExtender::BGSEEWindowSubclasser::DialogSubclassUserData*)GetWindowLongPtr(hWnd, DWL_USER);

			Return = false;

			switch (uMsg)
			{
			case WM_TESFORMIDLISTVIEW_HASCHANGES:
				{
					Return = true;

					if (IsWindowEnabled(GetDlgItem(hWnd, 1)))
					{
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

						if (WorkingCopy)
						{
							LocalCopy->GetDataFromDialog(hWnd);

							if (WorkingCopy->CompareTo(LocalCopy))
							{
								DlgProcResult = TRUE;
							}
						}
					}

					SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
				}

				break;
			case WM_TESFORMIDLISTVIEW_SAVECHANGES:
				{
					if (IsWindowEnabled(GetDlgItem(hWnd, 1)))
					{
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

						if (WorkingCopy)
						{
							LocalCopy->GetDataFromDialog(hWnd);

							if (WorkingCopy->CompareTo(LocalCopy))
							{
								if (WorkingCopy->UpdateUsageInfo())
								{
									WorkingCopy->SetFromActiveFile(true);
									WorkingCopy->CopyFrom(LocalCopy);

									if (WorkingCopy->formType == TESForm::kFormType_EffectSetting)
										BGSEEACHIEVEMENTS->Unlock(Achievements::kMagister);
								}
							}
						}
					}

					Return = true;
				}

				break;
			case WM_INITDIALOG:
				{
					if (UserData->TemplateID != TESDialog::kDialogTemplate_Quest)
					{
						SetWindowText(GetDlgItem(hWnd, 1), "Apply");
						SetWindowText(GetDlgItem(hWnd, 2), "Close");
					}
				}

				break;
			case WM_DESTROY:
				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case 1:			// OK button
					{
						if (UserData->TemplateID != TESDialog::kDialogTemplate_Quest)
						{
							Return = true;
							SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, NULL);
						}
					}

					break;
				}

				break;
			case WM_NOTIFY:
				{
					if (UserData->TemplateID == TESDialog::kDialogTemplate_Quest)
						break;

					NMHDR* NotificationData = (NMHDR*)lParam;

					if (NotificationData->idFrom != kFormList_TESFormIDListView)
						break;

					switch (NotificationData->code)
					{
					case LVN_ITEMCHANGED:
						{
							NMLISTVIEW* ChangeData = (NMLISTVIEW*)lParam;

							if ((ChangeData->uChanged & 8) && (ChangeData->uOldState & LVIS_FOCUSED) && (ChangeData->uNewState & LVIS_FOCUSED) == false)
							{
								if (SendMessage(hWnd, WM_TESFORMIDLISTVIEW_HASCHANGES, NULL, NULL) == TRUE)
								{
									int MsgResult = BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "Save changes made to the active form?");

									switch (MsgResult)
									{
									case IDYES:
										SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, NULL);

										break;
									}

									Return = true;
									SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
								}
							}
						}

						break;
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK LandscapeEditDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					if (*TESRenderWindow::CurrentlyLoadedCell == NULL)		// immediately close the dialog if you haven't got any cell loaded
						SendMessage(hWnd, WM_COMMAND, 2, NULL);			// otherwise, the editor will crash as soon as the render window acquires input focus
					else
						SendDlgItemMessage(hWnd, 1492, LVM_SORTITEMS, 0, (LPARAM)0x0041E7D0);		// TESDialog::SortComparatorLandTextureList
				}

				break;
			case 0x41A:		// update active landscape texture
				{
					Return = true;

					HWND TexList = GetDlgItem(hWnd, 1492);
					int Selection = TESListView::GetItemByData(TexList, *TESRenderWindow::ActiveLandscapeTexture);
					if (Selection != -1)
					{
						TESListView::SetSelectedItem(TexList, Selection);
						ListView_EnsureVisible(TexList, Selection, FALSE);
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK AIPackagesDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					HWND PackageListView = GetDlgItem(hWnd, 1977);
					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 175;
					ListView_SetColumn(PackageListView, 0, &ColumnData);
				}

				break;
			case WM_COMMAND:
				if (LOWORD(wParam) == 2)		// prevents the dialog from closing itself on renaming an AI package
				{
					if (HIWORD(wParam) == EN_KILLFOCUS || HIWORD(wParam) == EN_SETFOCUS)
					{
						Return = true;
						DlgProcResult = TRUE;
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK AIFormDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, BGSEditorExtender::BGSEEWindowExtraDataCollection* ExtraData )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					HWND PackageListView = GetDlgItem(hWnd, 1977);

					TESPackage::InitializeListViewColumns(PackageListView);	// do this just once, not every time the listview is refreshed

					LVCOLUMN ColumnData = {0};
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 150;
					ListView_SetColumn(PackageListView, 0, &ColumnData);
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
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Faction, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Class, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Skill, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_EffectSetting, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_GameSetting, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Globals, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Birthsign, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Climate, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Worldspace, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Hair, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Quest, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Eyes, TESFormIDListViewDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weather, TESFormIDListViewDlgSubClassProc);

			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FindText, FindTextDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Data, DataDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_ResponseEditor, ResponseDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_TextureUse, LandscapeTextureUseDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Quest, FilteredDialogQuestDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_FilteredDialog, FilteredDialogQuestDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_About, AboutDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Race, RaceDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectTopic, SelectTopicsQuestsSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectQuests, SelectTopicsQuestsSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_LandscapeEdit, LandscapeEditDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIPackages, AIPackagesDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_AIForm, AIFormDlgSubClassProc);

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
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_Weather, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectTopic, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterDialogSubclass(TESDialog::kDialogTemplate_SelectQuests, CommonDialogExtraFittingsSubClassProc);
			}

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
				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_Weather, RegularAppWindow);
			}

			SendMessage(*TESCSMain::WindowHandle, WM_MAINWINDOW_INITEXTRADATA, NULL, NULL);
			SendMessage(*TESRenderWindow::WindowHandle, WM_RENDERWINDOW_UPDATEFOV, NULL, NULL);
		}
	}
}