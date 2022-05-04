#include "MainWindowOverrides.h"
#include "Construction Set Extender_Resource.h"
#include "[Common]\CLIWrapper.h"
#include "WorkspaceManager.h"
#include "Achievements.h"
#include "HallOfFame.h"
#include "OldCSInteropManager.h"
#include "GlobalClipboard.h"
#include "FormUndoStack.h"
#include "DialogImposterManager.h"
#include "ObjectPaletteManager.h"
#include "ObjectPrefabManager.h"
#include "Render Window\AuxiliaryViewport.h"
#include "Render Window\RenderWindowManager.h"
#include "CustomDialogProcs.h"

#include <bgsee\ToolBox.h>
#include <bgsee\Script\CodaVM.h>

namespace cse
{
	namespace uiManager
	{

		MainWindowMiscData::MainWindowMiscData() :
			bgsee::WindowExtraData(kTypeID)
		{
			ToolbarExtras = Subwindow::CreateInstance();
		}

		MainWindowMiscData::~MainWindowMiscData()
		{
			if (ToolbarExtras)
			{
				ToolbarExtras->TearDown();
				ToolbarExtras->DeleteInstance();
			}
		}

		MainWindowToolbarData::MainWindowToolbarData() :
			bgsee::WindowExtraData(kTypeID)
		{
			SettingTODSlider = false;
		}

		MainWindowToolbarData::~MainWindowToolbarData()
		{
			;//
		}


		LRESULT CALLBACK MainWindowMenuInitSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = FALSE;

			switch (uMsg)
			{
			case WM_INITMENUPOPUP:
				{
					if (HIWORD(lParam) == FALSE)
					{
						HMENU Popup = (HMENU)wParam;

						for (int i = 0, j = GetMenuItemCount(Popup); i < j; i++)
						{
							MENUITEMINFO CurrentItem = { 0 };
							CurrentItem.cbSize = sizeof(MENUITEMINFO);
							CurrentItem.fMask = MIIM_ID | MIIM_STATE;

							if (GetMenuItemInfo(Popup, i, TRUE, &CurrentItem) == TRUE)
							{
								bool UpdateItem = true;
								bool CheckItem = false;
								bool DisableItem = false;

								switch (CurrentItem.wID)
								{
								case TESCSMain::kMainMenu_World_EditCellPathGrid:
									if (*TESRenderWindow::PathGridEditFlag)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_SAVEESPMASTERS:
									if (settings::plugins::kSaveLoadedESPsAsMasters.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS:
									if (settings::plugins::kPreventTimeStampChanges.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SAVEOPTIONS_CREATEBACKUPBEFORESAVING:
									if (settings::versionControl::kBackupOnSave.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_GLOBALUNDO:
									if (BGSEEUNDOSTACK->IsUndoStackEmpty())
										DisableItem = true;

									break;
								case IDC_MAINMENU_GLOBALREDO:
									if (BGSEEUNDOSTACK->IsRedoStackEmpty())
										DisableItem = true;

									break;
								case IDC_MAINMENU_CONSOLE:
									if (BGSEECONSOLE->IsVisible())
										CheckItem = true;

									break;
								case IDC_MAINMENU_AUXVIEWPORT:
									if (AUXVIEWPORT->IsVisible())
										CheckItem = true;

									break;
								case IDC_MAINMENU_HIDEUNMODIFIEDFORMS:
									if (FormEnumerationManager::Instance.GetVisibleUnmodifiedForms() == false)
										CheckItem = true;

									break;
								case IDC_MAINMENU_HIDEDELETEDFORMS:
									if (FormEnumerationManager::Instance.GetVisibleDeletedForms() == false)
										CheckItem = true;

									break;
								case IDC_MAINMENU_SORTACTIVEFORMSFIRST:
									if (settings::dialogs::kSortFormListsByActiveForm.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_COLORIZEACTIVEFORMS:
									if (settings::dialogs::kColorizeActiveForms.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_COLORIZEFORMOVERRIDES:
									if (settings::dialogs::kColorizeFormOverrides.GetData().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_CODABACKGROUNDER:
									if (CODAVM->GetBackgroundDaemon()->IsEnabled())
										CheckItem = true;

									break;
								case IDC_MAINMENU_INITIALLYDISABLEDREFERENCES:
									if (_RENDERWIN_XSTATE.ShowInitiallyDisabledRefs)
										CheckItem = true;

									break;
								case IDC_MAINMENU_CHILDREFERENCESOFTHEDISABLED:
									if (_RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren)
										CheckItem = true;

									break;
								case IDC_MAINMENU_MULTIPLEPREVIEWWINDOWS:
									if (PreviewWindowImposterManager::Instance.GetEnabled())
										CheckItem = true;

									break;
								case IDC_MAINMENU_PARENTCHILDINDICATORS:
									if (settings::renderer::kParentChildVisualIndicator().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORS:
									if (settings::renderer::kPathGridLinkedRefIndicator().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDEBOUNDINGBOX:
									if ((settings::renderer::kPathGridLinkedRefIndicatorFlags().u &
										 settings::renderer::kPathGridLinkedRefIndicatorFlag_HidePointBoundingBox))
									{
										CheckItem = true;
									}

									break;
								case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDELINKEDREFNODE:
									if ((settings::renderer::kPathGridLinkedRefIndicatorFlags().u &
										 settings::renderer::kPathGridLinkedRefIndicatorFlag_HideLinkedRefNode))
									{
										CheckItem = true;
									}

									break;
								case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDELINECONNECTOR:
									if ((settings::renderer::kPathGridLinkedRefIndicatorFlags().u &
										settings::renderer::kPathGridLinkedRefIndicatorFlag_HideLineConnector))
									{
										CheckItem = true;
									}

									break;
								case IDC_MAINMENU_CUSTOMCOLORTHEMEMODE:
									if (BGSEEUI->GetColorThemer()->IsEnabled())
										CheckItem = true;

									break;
								case IDC_MAINMENU_ONEDITORSTARTUP_LOADSTARTUPPLUGIN:
									if (settings::startup::kLoadPlugin().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_ONEDITORSTARTUP_SETSTARTUPWORKSPACE:
									if (settings::startup::kSetWorkspace().i)
										CheckItem = true;

									break;
								case IDC_MAINMENU_ONEDITORSTARTUP_OPENSTARTUPSCRIPT:
									if (settings::startup::kOpenScriptWindow().i)
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

									if (DisableItem)
									{
										CurrentItem.fState &= ~MFS_ENABLED;
										CurrentItem.fState |= MFS_DISABLED;
									}
									else
									{
										CurrentItem.fState &= ~MFS_DISABLED;
										CurrentItem.fState |= MFS_ENABLED;
									}

									CurrentItem.fMask = MIIM_STATE;
									SetMenuItemInfo(Popup, i, TRUE, &CurrentItem);
								}
							}
						}
					}

					SubclassParams->Out.MarkMessageAsHandled = true;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowMenuSelectSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														  bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = FALSE;

			switch (uMsg)
			{
			case WM_COMMAND:
				SubclassParams->Out.MarkMessageAsHandled = true;

				switch (LOWORD(wParam))
				{
				case TESCSMain::kMainMenu_View_PreviewWindow:
					if (PreviewWindowImposterManager::Instance.GetEnabled())
						BGSEEUI->MsgBoxI("Use the Object Window's context menu to preview objects when multiple preview windows are enabled.");
					else
						SubclassParams->Out.MarkMessageAsHandled = false;

					break;
				case TESCSMain::kMainMenu_Help_Contents:
				case TESCSMain::kMainMenu_Character_ExportDialogue:
					{
						if (achievements::kOldestTrickInTheBook->GetUnlocked() == false)
						{
							ShellExecute(nullptr, "open", "http://www.youtube.com/watch?v=oHg5SJYRHA0", nullptr, nullptr, SW_SHOWNORMAL);
							BGSEEACHIEVEMENTS->Unlock(achievements::kOldestTrickInTheBook);
						}
						else
							SubclassParams->Out.MarkMessageAsHandled = false;
					}

					break;
				case IDC_MAINMENU_SAVEAS:
					{
						if (_DATAHANDLER->activeFile == nullptr)
						{
							BGSEEUI->MsgBoxE("An active plugin must be set before using this tool.");
							break;
						}

						*TESCSMain::AllowAutoSaveFlag = 0;
						char FileName[MAX_PATH] = { 0 };

						if (TESDialog::SelectTESFileCommonDialog(hWnd,
																 INISettingCollection::Instance->LookupByName("sLocalMasterPath:General")->value.s,
																 0,
																 FileName,
																 sizeof(FileName)))
						{
							TESFile* SaveAsBuffer = _DATAHANDLER->activeFile;

							SaveAsBuffer->SetActive(false);
							SaveAsBuffer->SetLoaded(false);

							_DATAHANDLER->activeFile = nullptr;

							if (SendMessage(hWnd, TESCSMain::kWindowMessage_Save, NULL, (LPARAM)FileName))
								TESCSMain::SetTitleModified(false);
							else
							{
								_DATAHANDLER->activeFile = SaveAsBuffer;

								SaveAsBuffer->SetActive(true);
								SaveAsBuffer->SetLoaded(true);
							}
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_SaveAs);
						}

						*TESCSMain::AllowAutoSaveFlag = 1;
					}

					break;
				case IDC_MAINMENU_CONSOLE:
					BGSEECONSOLE->ToggleVisibility();

					break;
				case IDC_MAINMENU_HIDEDELETEDFORMS:
					FormEnumerationManager::Instance.ToggleVisibilityDeletedForms();

					break;
				case IDC_MAINMENU_HIDEUNMODIFIEDFORMS:
					FormEnumerationManager::Instance.ToggleVisibilityUnmodifiedForms();

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

						ShellExecute(nullptr, "open", (LPCSTR)AppPath.c_str(), nullptr, nullptr, SW_SHOW);
						BGSEEACHIEVEMENTS->Unlock(achievements::kLazyBum);
					}

					break;
				case IDC_MAINMENU_CREATEGLOBALSCRIPT:
					BGSEEUI->ModelessDialog(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_GLOBALSCRIPT), hWnd, (DLGPROC)CreateGlobalScriptDlgProc);

					break;
				case IDC_MAINMENU_TAGBROWSER:
					cliWrapper::interfaces::TAG->ShowTagBrowserDialog(nullptr);
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_TagBrowser);

					break;
				case IDC_MAINMENU_SETWORKSPACE:
					BGSEEWORKSPACE->SelectCurrentWorkspace();
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_Workspace);

					break;
				case IDC_MAINMENU_TOOLS:
					BGSEETOOLBOX->ShowToolListMenu(BGSEEMAIN->GetExtenderHandle(), hWnd);
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_Toolbox);

					break;
				case IDC_MAINMENU_CODAMANAGEGLOBALDATASTORE:
					CODAVM->ShowGlobalStoreEditDialog(BGSEEMAIN->GetExtenderHandle(), hWnd);

					break;
				case IDC_MAINMENU_CODABACKGROUNDER:
					if (CODAVM->GetBackgroundDaemon()->IsEnabled())
						CODAVM->GetBackgroundDaemon()->Suspend();
					else
						CODAVM->GetBackgroundDaemon()->Resume();

					break;
				case IDC_MAINMENU_SAVEOPTIONS_SAVEESPMASTERS:
					settings::plugins::kSaveLoadedESPsAsMasters.ToggleData();

					break;
				case IDC_MAINMENU_SAVEOPTIONS_PREVENTCHANGESTOFILETIMESTAMPS:
					settings::plugins::kPreventTimeStampChanges.ToggleData();

					break;
				case IDC_MAINMENU_AUXVIEWPORT:
					AUXVIEWPORT->ToggleVisibility();

					break;
				case IDC_MAINMENU_USEINFOLISTING:
					cliWrapper::interfaces::USE->ShowUseInfoListDialog(nullptr);
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_UseInfoListing);

					break;
				case IDC_MAINMENU_BATCHLIPGENERATOR:
					{
						if (*TESQuest::WindowHandle != NULL || *TESQuest::FilteredDialogWindowHandle != NULL)
						{
							// will cause a CTD if the tool is executed when either of the above windows are open
							BGSEEUI->MsgBoxW("Please close any open Quest or Filtered Dialog windows before using this tool.");
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

						HWND IdleWindow = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), hWnd, nullptr, NULL);
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

													char VoiceFilePath[MAX_PATH] = { 0 };

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
																if (CSIOM.GenerateLIPSyncFile(VoiceFilePath, Response->responseText.c_str()))
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

						achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GenerateLIP);
						DestroyWindow(IdleWindow);

						if (HasError)
							BGSEEUI->MsgBoxW("Batch generation completed with some errors!\n\nGenerated: %d files\nFailed: %d Files",
											 BatchGenCounter, FailedCounter);
						else
							BGSEEUI->MsgBoxI("Batch generation completed successfully!\n\nGenerated: %d files.", BatchGenCounter);
					}

					break;
				case IDC_MAINMENU_SAVEOPTIONS_CREATEBACKUPBEFORESAVING:
					settings::versionControl::kBackupOnSave.ToggleData();

					break;
				case IDC_MAINMENU_SORTACTIVEFORMSFIRST:
					settings::dialogs::kSortFormListsByActiveForm.ToggleData();

					break;
				case IDC_MAINMENU_COLORIZEACTIVEFORMS:
					settings::dialogs::kColorizeActiveForms.ToggleData();

					break;
				case IDC_MAINMENU_COLORIZEFORMOVERRIDES:
					settings::dialogs::kColorizeFormOverrides.ToggleData();

					break;
				case IDC_MAINMENU_GLOBALCLIPBOARDCONTENTS:
					BGSEECLIPBOARD->DisplayContents();

					break;
				case IDC_MAINMENU_PASTEFROMGLOBALCLIPBOARD:
					BGSEECLIPBOARD->Paste();
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GlobalClipboard);

					break;
				case IDC_MAINMENU_GLOBALUNDO:
					BGSEEUNDOSTACK->PerformUndo();
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GlobalUndo);

					break;
				case IDC_MAINMENU_GLOBALREDO:
					BGSEEUNDOSTACK->PerformRedo();
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GlobalUndo);

					break;
				case IDC_MAINMENU_PURGELOADEDRESOURCES:
					{
						BGSEECONSOLE_MESSAGE("Purging resources...");
						BGSEECONSOLE->Indent();
						PROCESS_MEMORY_COUNTERS_EX MemCounter = { 0 };
						UInt32 RAMUsage = 0;

						GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter));
						RAMUsage = MemCounter.WorkingSetSize / (1024 * 1024);
						BGSEECONSOLE_MESSAGE("Current RAM Usage: %d MB", RAMUsage);
						_TES->PurgeLoadedResources();
						BGSEECONSOLE_MESSAGE("Resources purged!");

						GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&MemCounter, sizeof(MemCounter));
						RAMUsage = MemCounter.WorkingSetSize / (1024 * 1024);
						BGSEECONSOLE_MESSAGE("Current RAM Usage: %d MB", RAMUsage);
						BGSEECONSOLE->Outdent();
					}

					break;
				case IDC_MAINMENU_INITIALLYDISABLEDREFERENCES:
					_RENDERWIN_XSTATE.ShowInitiallyDisabledRefs = _RENDERWIN_XSTATE.ShowInitiallyDisabledRefs == false;
					TESRenderWindow::Redraw();

					break;
				case IDC_MAINMENU_CHILDREFERENCESOFTHEDISABLED:
					_RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren = _RENDERWIN_XSTATE.ShowInitiallyDisabledRefChildren == false;
					TESRenderWindow::Redraw();

					break;
				case IDC_MAINMENU_CODAOPENSCRIPTREPOSITORY:
					CODAVM->OpenScriptRepository();

					break;
				case IDC_MAINMENU_SPAWNEXTRAOBJECTWINDOW:
					ObjectWindowImposterManager::Instance.SpawnImposter();
					achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_MultipleObjectWindows);

					break;
				case IDC_MAINMENU_MULTIPLEPREVIEWWINDOWS:
					if (PreviewWindowImposterManager::Instance.GetEnabled())
					{
						PreviewWindowImposterManager::Instance.SetEnabled(false);
						settings::dialogs::kMultiplePreviewWindows.SetInt(0);
					}
					else
					{
						PreviewWindowImposterManager::Instance.SetEnabled(true);
						settings::dialogs::kMultiplePreviewWindows.SetInt(1);
					}

					break;
				case IDC_MAINMENU_OBJECTPALETTE:
					objectPalette::ObjectPaletteManager::Instance.Show();

					break;
				case IDC_MAINMENU_OBJECTPREFABS:
					objectPrefabs::ObjectPrefabManager::Instance.Show();

					break;
				case IDC_MAINMENU_PARENTCHILDINDICATORS:
					settings::renderer::kParentChildVisualIndicator.ToggleData();
					TESRenderWindow::Redraw();

					break;
				case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORS:
					settings::renderer::kPathGridLinkedRefIndicator.ToggleData();
					TESRenderWindow::Redraw(true);

					break;
				case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDEBOUNDINGBOX:
				case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDELINKEDREFNODE:
				case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDELINECONNECTOR:
					{
						UInt32 Flags = settings::renderer::kPathGridLinkedRefIndicatorFlags().u;
						UInt32 Comperand = 0;

						switch (LOWORD(wParam))
						{
						case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDEBOUNDINGBOX:
							Comperand = settings::renderer::kPathGridLinkedRefIndicatorFlag_HidePointBoundingBox;
							break;
						case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDELINKEDREFNODE:
							Comperand = settings::renderer::kPathGridLinkedRefIndicatorFlag_HideLinkedRefNode;
							break;
						case IDC_MAINMENU_PATHGRIDLINKEDREFINDICATORSETTINGS_HIDELINECONNECTOR:
							Comperand = settings::renderer::kPathGridLinkedRefIndicatorFlag_HideLineConnector;
							break;
						}

						if ((Flags & Comperand))
							Flags &= ~Comperand;
						else
							Flags |= Comperand;

						settings::renderer::kPathGridLinkedRefIndicatorFlags.SetUInt(Flags);
						if (settings::renderer::kPathGridLinkedRefIndicator().i == 0)
							TESRenderWindow::Redraw(true);
					}

					break;
				case IDC_MAINMENU_CODARELOADBACKGROUNDSCRIPTS:
					CODAVM->GetBackgroundDaemon()->Rebuild();
					break;
				case IDC_MAINMENU_CODARESETSCRIPTCACHE:
					CODAVM->GetProgramCache()->Invalidate();
					break;
				case IDC_MAINMENU_SYNCSCRIPTSTODISK:
					cliWrapper::interfaces::SE->ShowDiskSyncDialog();
					break;
				case IDC_MAINMENU_CUSTOMCOLORTHEMEMODE:
					if (BGSEEUI->GetColorThemer()->IsEnabled())
						BGSEEUI->GetColorThemer()->Disable();
					else
						BGSEEUI->GetColorThemer()->Enable();

					break;
				case IDC_MAINMENU_ONEDITORSTARTUP_LOADSTARTUPPLUGIN:
					settings::startup::kLoadPlugin.ToggleData();
					break;
				case IDC_MAINMENU_ONEDITORSTARTUP_SETSTARTUPWORKSPACE:
					settings::startup::kSetWorkspace.ToggleData();
					break;
				case IDC_MAINMENU_ONEDITORSTARTUP_OPENSTARTUPSCRIPT:
					settings::startup::kOpenScriptWindow.ToggleData();
					break;
				default:
					SubclassParams->Out.MarkMessageAsHandled = false;

					break;
				}

				break;
			}

			return DlgProcResult;
		}

#define ID_PATHGRIDTOOLBARBUTTION_TIMERID		0x99

		LRESULT CALLBACK MainWindowMiscSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = FALSE;

			switch (uMsg)
			{
			case TESCSMain::kWindowMessage_Unk413:
				// if multiple editor instances are disabled, the secondary instance will send this message to the primary
				// strangely, when the primary instance has an active/open preview control, the call crashes (presumably due to a corrupt lParam pointer)
				// as far as I can tell, the corresponding code path is never executed, so we can consume this message entirely

				SubclassParams->Out.MarkMessageAsHandled = true;
				break;
			case WM_MAINWINDOW_INIT_DIALOG:
				{
					SetTimer(hWnd, ID_PATHGRIDTOOLBARBUTTION_TIMERID, 500, nullptr);
					SubclassParams->Out.MarkMessageAsHandled = true;
				}

				break;
			case WM_DESTROY:
				{
					KillTimer(hWnd, ID_PATHGRIDTOOLBARBUTTION_TIMERID);

					MainWindowMiscData* xData = BGSEE_GETWINDOWXDATA(MainWindowMiscData, SubclassParams->In.ExtraData);
					if (xData)
					{
						SubclassParams->In.ExtraData->Remove(MainWindowMiscData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_MAINWINDOW_INIT_EXTRADATA:
				{
					MainWindowMiscData* xData = BGSEE_GETWINDOWXDATA(MainWindowMiscData, SubclassParams->In.ExtraData);
					if (xData == nullptr)
					{
						xData = new MainWindowMiscData();
						SubclassParams->In.ExtraData->Add(xData);

						xData->ToolbarExtras->hInstance = BGSEEMAIN->GetExtenderHandle();
						xData->ToolbarExtras->hDialog = *TESCSMain::MainToolbarHandle;
						xData->ToolbarExtras->hContainer = *TESCSMain::MainToolbarHandle;
						xData->ToolbarExtras->position.x = 485;
						xData->ToolbarExtras->position.y = 0;

						if (xData->ToolbarExtras->Build(IDD_TOOLBAREXTRAS) == false)
							BGSEECONSOLE_ERROR("Couldn't build main window toolbar subwindow!");
						else
						{
							SubclassParams->In.Subclasser->RegisterSubclassForWindow(*TESCSMain::MainToolbarHandle, MainWindowToolbarSubClassProc);
							SendMessage(*TESCSMain::MainToolbarHandle, WM_MAINTOOLBAR_INIT, NULL, NULL);

							HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
							HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

							TESDialog::ClampDlgEditField(TODEdit, 0.0, 24.0);

							SendMessage(TODSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 23));
							SendMessage(TODSlider, TBM_SETLINESIZE, NULL, 1);
							SendMessage(TODSlider, TBM_SETPAGESIZE, NULL, 4);

							SendMessage(*TESCSMain::MainToolbarHandle, WM_MAINTOOLBAR_SETTOD, _TES->GetSkyTOD() * 4.0, NULL);
						}
					}
				}

				break;
			case WM_TIMER:
				DlgProcResult = TRUE;
				SubclassParams->Out.MarkMessageAsHandled = true;

				switch (wParam)
				{
				case TESCSMain::kTimer_Autosave:
					// autosave timer, needs to be handled here as the org wndproc doesn't compare the timerID
					if (*TESCSMain::AllowAutoSaveFlag != 0 && *TESCSMain::ExittingCSFlag == 0)
						TESCSMain::AutoSave();

					break;
				case ID_PATHGRIDTOOLBARBUTTION_TIMERID:
					{
						TBBUTTONINFO PathGridData = { 0 };
						PathGridData.cbSize = sizeof(TBBUTTONINFO);
						PathGridData.dwMask = TBIF_STATE;

						SendMessage(*TESCSMain::MainToolbarHandle, TB_GETBUTTONINFO, TESCSMain::kToolbar_PathGridEdit, (LPARAM)&PathGridData);
						if ((PathGridData.fsState & TBSTATE_CHECKED) == false && *TESRenderWindow::PathGridEditFlag)
						{
							PathGridData.fsState |= TBSTATE_CHECKED;
							SendMessage(*TESCSMain::MainToolbarHandle, TB_SETBUTTONINFO, TESCSMain::kToolbar_PathGridEdit, (LPARAM)&PathGridData);
						}
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK MainWindowToolbarSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													   bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = FALSE;

			HWND TODSlider = GetDlgItem(hWnd, IDC_TOOLBAR_TODSLIDER);
			HWND TODEdit = GetDlgItem(hWnd, IDC_TOOLBAR_TODCURRENT);

			switch (uMsg)
			{
			case WM_MAINTOOLBAR_INIT:
				{
					MainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(MainWindowToolbarData, SubclassParams->In.ExtraData);
					if (xData == nullptr)
					{
						xData = new MainWindowToolbarData();
						SubclassParams->In.ExtraData->Add(xData);
					}
				}

				break;
			case WM_DESTROY:
				{
					MainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(MainWindowToolbarData, SubclassParams->In.ExtraData);
					if (xData)
					{
						SubclassParams->In.ExtraData->Remove(MainWindowToolbarData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_COMMAND:
				{
					MainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(MainWindowToolbarData, SubclassParams->In.ExtraData);
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
							BreakOut = false;

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

					MainWindowToolbarData* xData = BGSEE_GETWINDOWXDATA(MainWindowToolbarData, SubclassParams->In.ExtraData);
					SME_ASSERT(xData);

					if (xData->SettingTODSlider == false)
						TESDialog::SetDlgItemFloat(hWnd, IDC_TOOLBAR_TODCURRENT, TOD, 2);

					TESPreviewControl::UpdatePreviewWindows();
				}

				break;
			}

			return DlgProcResult;
		}

		void InitializeMainWindowOverrides()
		{
			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(BGSEEUI->GetMainWindow(), uiManager::MainWindowMenuInitSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(BGSEEUI->GetMainWindow(), uiManager::MainWindowMenuSelectSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForWindow(BGSEEUI->GetMainWindow(), uiManager::MainWindowMiscSubclassProc);

			SendMessage(BGSEEUI->GetMainWindow(), WM_MAINWINDOW_INIT_DIALOG, NULL, NULL);
		}

	}
}