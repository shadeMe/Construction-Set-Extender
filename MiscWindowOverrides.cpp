#include "MiscWindowOverrides.h"
#include "Construction Set Extender_Resource.h"
#include "UIManager.h"
#include "[Common]\CLIWrapper.h"
#include "Achievements.h"
#include "OldCSInteropManager.h"
#include "CustomDialogProcs.h"
#include "Render Window\RenderWindowManager.h"

namespace cse
{
	namespace uiManager
	{
		FaceGenWindowData::FaceGenWindowData() :
			bgsee::WindowExtraData(kTypeID)
		{
			TunnelingTabSelectMessage = false;
			AllowPreviewUpdates = true;
			VoicePlaybackFilePath = "";
		}

		FaceGenWindowData::~FaceGenWindowData()
		{
			;//
		}

		void LoadPluginsWindowData::BuildPluginFlagCache()
		{
			PluginFlagsInitialState.clear();

			for (auto Itr = _DATAHANDLER->fileList.Begin(); Itr.Get() && Itr.End() == false; ++Itr)
				PluginFlagsInitialState.emplace_back(Itr.Get());
		}


		void LoadPluginsWindowData::UpdatePluginFlagsFromCache()
		{
			for (auto& Itr : PluginFlagsInitialState)
			{
				Itr.Plugin->SetLoaded(Itr.Loaded);
				Itr.Plugin->SetActive(Itr.Active);
			}
		}

#define IDC_FINDTEXT_OPENSCRIPTS				9014

		LRESULT CALLBACK FindTextDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												 bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			static bool kDraggingForms = false;

			switch (uMsg)
			{
			case WM_MOUSEMOVE:
				if (kDraggingForms)
				{
					POINT Pos = { 0 };
					GetCursorPos(&Pos);
					HWND Window = WindowFromPoint(Pos);

					if (_RENDERSEL->selectionList && _RENDERSEL->selectionList->Data)
					{
						if (TESDialog::GetIsWindowDragDropRecipient(_RENDERSEL->selectionList->Data->formType, Window))
							SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xBA));
						else
							SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xB8));
					}
				}

				break;
			case WM_LBUTTONUP:
				if (kDraggingForms)
				{
					POINT Pos = { 0 };
					GetCursorPos(&Pos);
					HWND Window = WindowFromPoint(Pos);

					if (_RENDERSEL->selectionList && _RENDERSEL->selectionList->Data)
					{
						if (TESDialog::GetIsWindowDragDropRecipient(_RENDERSEL->selectionList->Data->formType, Window))
						{
							HWND Parent = GetParent(Window);
							if (Parent == nullptr || Parent == *TESCSMain::WindowHandle)
								Parent = Window;

							SendMessage(Parent, TESDialog::kWindowMessage_HandleDragDrop, NULL, (LPARAM)&Pos);
						}
						else
							_RENDERSEL->ClearSelection();

						kDraggingForms = false;
						SetCursor(LoadCursor(nullptr, IDC_ARROW));
						ReleaseCapture();
					}
				}

				break;
			case WM_NOTIFY:
				switch (((LPNMHDR)lParam)->code)
				{
				case NM_RCLICK:
					{
						NMHDR* Data = (NMHDR*)lParam;
						TESFormArrayT Selection;

						if (Data->idFrom == kFindTextListView_Objects)
						{
							int Index = -1;
							while ((Index = ListView_GetNextItem(Data->hwndFrom, Index, LVNI_SELECTED)) != -1)
							{
								TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hwndFrom, Index);
								SME_ASSERT(Form);

								Script* FoundScript = CS_CAST(Form, TESForm, Script);
								if (FoundScript)
									Selection.push_back(FoundScript);
							}

							if (Selection.size())
							{
								HMENU Popup = CreatePopupMenu();
								MENUITEMINFO Item = { 0 };
								Item.cbSize = sizeof(MENUITEMINFO);
								Item.fMask = MIIM_STATE | MIIM_TYPE | MIIM_ID;
								Item.fType = MFT_STRING;

								Item.fState = MFS_ENABLED;
								Item.dwTypeData = "Open in Script Editor";
								Item.cch = strlen((const char*)Item.dwTypeData);
								Item.wID = IDC_FINDTEXT_OPENSCRIPTS;
								InsertMenuItem(Popup, -1, TRUE, &Item);

								POINT CursorPos = { 0 };
								GetCursorPos(&CursorPos);

								switch (TrackPopupMenu(Popup, TPM_RETURNCMD, CursorPos.x, CursorPos.y, NULL, hWnd, nullptr))
								{
								case IDC_FINDTEXT_OPENSCRIPTS:
									{
										componentDLLInterface::ScriptData** Data = new componentDLLInterface::ScriptData*[Selection.size()];
										for (int i = 0; i < Selection.size(); i++)
											Data[i] = new componentDLLInterface::ScriptData((Script*)Selection[i]);

										RECT ScriptEditorLoc;
										TESDialog::ReadBoundsFromINI("Script Edit", &ScriptEditorLoc);
										char Buffer[0x100] = { 0 };
										GetDlgItemText(hWnd, kFindTextTextBox_Query, Buffer, sizeof(Buffer));
										cse::cliWrapper::interfaces::SE->InstantiateEditorsAndHighlight(Data,
																										Selection.size(),
																										Buffer,
																										ScriptEditorLoc.left, ScriptEditorLoc.top,
																										ScriptEditorLoc.right, ScriptEditorLoc.bottom);
									}

									break;
								}

								DestroyMenu(Popup);
							}
						}

						break;
					}

				case LVN_BEGINDRAG:
					{
						NMLISTVIEW* Data = (NMLISTVIEW*)lParam;
						TESFormArrayT Selection;

						switch (Data->hdr.idFrom)
						{
						case kFindTextListView_Objects:
							{
								int Index = -1;
								while ((Index = ListView_GetNextItem(Data->hdr.hwndFrom, Index, LVNI_SELECTED)) != -1)
								{
									TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hdr.hwndFrom, Index);
									SME_ASSERT(Form);

									Selection.push_back(Form);
								}
							}

							break;
						}

						kDraggingForms = false;
						if (Selection.size())
						{
							_RENDERSEL->ClearSelection(true);

							for (TESFormArrayT::iterator Itr = Selection.begin(); Itr != Selection.end(); ++Itr)
								_RENDERSEL->AddToSelection(*Itr);

							kDraggingForms = true;
							SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xB8));
							SetCapture(hWnd);
						}
					}

					break;
				case LVN_ITEMACTIVATE:
					NMITEMACTIVATE* Data = (NMITEMACTIVATE*)lParam;
					TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hdr.hwndFrom, Data->iItem);

					if (Data->hdr.idFrom == kFindTextListView_Objects && Form)
					{
						switch (Form->formType)
						{
						case TESForm::kFormType_Script:
							{
								componentDLLInterface::ScriptData* Data = new componentDLLInterface::ScriptData(CS_CAST(Form, TESForm, Script));

								RECT ScriptEditorLoc;
								TESDialog::ReadBoundsFromINI("Script Edit", &ScriptEditorLoc);
								char Buffer[0x100] = { 0 };
								GetDlgItemText(hWnd, kFindTextTextBox_Query, Buffer, sizeof(Buffer));
								cse::cliWrapper::interfaces::SE->InstantiateEditorAndHighlight(Data,
																							   Buffer,
																							   ScriptEditorLoc.left, ScriptEditorLoc.top,
																							   ScriptEditorLoc.right, ScriptEditorLoc.bottom);
							}

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
			case WM_INITDIALOG:
				break;
			case WM_DESTROY:
				break;
			}

			return DlgProcResult;
		}



		LRESULT CALLBACK DataDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											 bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			TESFile* ActiveTESFile = *DataDialog::ActivePlugin;
			HWND PluginList = GetDlgItem(hWnd, DataDialog::kCtrlId_PluginFileList);

			switch (uMsg)
			{
			case WM_DATADLG_RECURSEMASTERS:
				{
					Return = true;

					TESFile* PluginFile = (TESFile*)lParam;

					if (PluginFile)
					{
						if (PluginFile->IsLoaded() == false)
						{
							PluginFile->SetLoaded(true);
							for (int i = 0; i < PluginFile->masterCount; i++)
								SendMessage(hWnd, WM_DATADLG_RECURSEMASTERS, NULL, (LPARAM)PluginFile->masterFiles[i]);
						}
					}
				}

				break;
			case WM_INITDIALOG:
				{
					LVCOLUMN ColumnData = { 0 };
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 305;
					ListView_SetColumn(PluginList, 0, &ColumnData);

					ColumnData.cx = 65;
					ListView_SetColumn(PluginList, 1, &ColumnData);

					bool LoadStartupPlugin = settings::startup::kLoadPlugin.GetData().i;
					if (LoadStartupPlugin)
						CheckDlgButton(hWnd, IDC_CSE_DATA_LOADSTARTUPPLUGIN, BST_CHECKED);

					LoadPluginsWindowData* xData = BGSEE_GETWINDOWXDATA(LoadPluginsWindowData, ExtraData);
					if (xData == nullptr)
					{
						xData = new LoadPluginsWindowData();
						ExtraData->Add(xData);
					}

					// cache the flags of all plugins for later restoration if the active file needs to be saved
					// necessary if the user unselects the masters of the currently active plugin
					for (auto Itr = _DATAHANDLER->fileList.Begin(); Itr.Get() && Itr.End() == false; ++Itr)
						xData->PluginFlagsInitialState.emplace_back(Itr.Get());
				}

				break;
			case WM_DESTROY:
				{
					TESFile* ActiveFile = _DATAHANDLER->activeFile;

					// required for correct ESM handling
					if (ActiveFile)
						ActiveFile->SetMaster(false);


					LoadPluginsWindowData* xData = BGSEE_GETWINDOWXDATA(LoadPluginsWindowData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(LoadPluginsWindowData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case NM_CUSTOMDRAW:
						if (wParam == DataDialog::kCtrlId_PluginFileList ||
							wParam == DataDialog::kCtrlId_ParentMasterFileList)
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
									bool Update = true;
									if (wParam == DataDialog::kCtrlId_PluginFileList)
									{
										UInt32 PluginIndex = (UInt32)DrawData->nmcd.lItemlParam;
										TESFile* CurrentFile = _DATAHANDLER->LookupPluginByIndex(PluginIndex);

										if (CurrentFile)
										{
											if (CurrentFile->IsMaster())
											{
												DrawData->clrTextBk = RGB(242, 247, 243);
												DrawData->clrText = RGB(0, 0, 0);
											}

											if (CurrentFile == ActiveTESFile)
											{
												DrawData->clrTextBk = RGB(227, 183, 251);
												DrawData->clrText = RGB(0, 0, 0);
											}
											else if (!_stricmp(settings::startup::kPluginName.GetData().s, CurrentFile->fileName))
											{
												DrawData->clrTextBk = RGB(248, 227, 186);
												DrawData->clrText = RGB(0, 0, 0);
											}
											else if (CurrentFile->authorName.c_str() &&
													 !_stricmp(CurrentFile->authorName.c_str(), "shadeMe"))
											{
												DrawData->clrTextBk = RGB(249, 255, 255);
												DrawData->clrText = RGB(0, 0, 0);
											}
											else
												Update = false;
										}
									}
									else
									{
										if (DrawData->nmcd.lItemlParam == NULL)
										{
											// missing master
											DrawData->clrTextBk = RGB(244, 75, 66);
											DrawData->clrText = RGB(0, 0, 0);
										}
										else
											Update = false;
									}

									if (Update)
									{
										SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NEWFONT);
										DlgProcResult = TRUE;
										Return = true;
									}
								}

								break;
							}
						}

						break;
					case LVN_KEYDOWN:
						if (wParam == DataDialog::kCtrlId_ParentMasterFileList)
						{
							NMLVKEYDOWN* KeyData = (NMLVKEYDOWN*)lParam;

							switch (KeyData->wVKey)
							{
							case VK_DELETE:		// delete master
								{
									TESFile* Selection = (TESFile*)TESListView::GetSelectedItemData(KeyData->hdr.hwndFrom);
									if (Selection)
									{
										if (BGSEEUI->MsgBoxW(hWnd,
															 MB_YESNO,
															 "You are about to remove a master file from the selected plugin.\n\nAre you sure you'd like to proceed?") == IDNO)
										{
											Return = true;
										}
									}
								}

								break;
							case VK_INSERT:		// insert master
								{
									;//
								}

								break;
							}
						}

						break;
					case LVN_GETDISPINFO:
						{
							if (wParam == DataDialog::kCtrlId_ParentMasterFileList)
							{
								Return = true;

								auto Info = (NMLVDISPINFO*)lParam;
								auto Plugin = (TESFile*)Info->item.lParam;
								if (Plugin)
									sprintf_s(Info->item.pszText, Info->item.cchTextMax, "%s", Plugin->fileName);
								else
								{
									// the masters are added in the same order as in the header
									// use the index of the list view item to get the corresponding missing master's name
									auto MissingMasterName = (*DataDialog::CurrentSelection)->masterNames.GetNthItem(Info->item.iItem);
									sprintf_s(Info->item.pszText, Info->item.cchTextMax, "%s", MissingMasterName);
								}
							}
						}

						break;
					}
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case IDC_CSE_DATA_SELECTLOADORDER:
					{
						std::string PluginListPath(TESCSMain::ProfileFolderPath);
						PluginListPath += "Plugins.txt";

						std::fstream Read(PluginListPath.c_str(), std::ios::in);
						if (Read.good())
						{
							char Buffer[0x100] = { 0 };
							while (Read.eof() == false)
							{
								Read.getline(Buffer, sizeof(Buffer));
								if (Buffer[0] != '#' && Buffer[0] != 0 && Buffer[0] != '\n')
								{
									TESFile* Plugin = _DATAHANDLER->LookupPluginByName(Buffer);
									if (Plugin)
										Plugin->SetLoaded(true);
								}
							}
						}

						BGSEEUI->GetInvalidationManager()->Redraw(PluginList);
					}

					break;
				case IDC_CSE_DATA_SELECTNONE:
					{
						for (tList<TESFile>::Iterator Itr = _DATAHANDLER->fileList.Begin(); Itr.Get() && Itr.End() == false; ++Itr)
							Itr.Get()->SetLoaded(false);

						*DataDialog::ActivePlugin = nullptr;
						ActiveTESFile = nullptr;
						BGSEEUI->GetInvalidationManager()->Redraw(PluginList);
					}

					break;
				case IDC_CSE_DATA_LOADSTARTUPPLUGIN:
					settings::startup::kLoadPlugin.SetInt((IsDlgButtonChecked(hWnd, IDC_CSE_DATA_LOADSTARTUPPLUGIN) == BST_CHECKED));

					break;
				case IDC_CSE_DATA_SETSTARTUPPLUGIN:
					{
						int SelectedItem = ListView_GetNextItem(PluginList, -1, LVNI_SELECTED);

						if (SelectedItem != -1)
						{
							LVITEM SelectedPluginItem;
							char PluginNameBuffer[0x200] = { 0 };

							SelectedPluginItem.iItem = SelectedItem;
							SelectedPluginItem.iSubItem = 0;
							SelectedPluginItem.mask = LVIF_TEXT;
							SelectedPluginItem.pszText = PluginNameBuffer;
							SelectedPluginItem.cchTextMax = sizeof(PluginNameBuffer);

							if (ListView_GetItem(PluginList, &SelectedPluginItem) == TRUE)
							{
								settings::startup::kPluginName.SetString(PluginNameBuffer);

								BGSEEUI->MsgBoxI(hWnd, 0, "Startup plugin set to '%s'.", PluginNameBuffer);
								achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_StartupLoad);
							}
						}
					}

					break;
				case TESDialog::kStandardButton_Ok:
					if (*TESCSMain::UnsavedChangesFlag)
					{
						// restore the original state of plugin flags to prevent save corruption
						LoadPluginsWindowData CurrentState;
						CurrentState.BuildPluginFlagCache();

						LoadPluginsWindowData* xData = BGSEE_GETWINDOWXDATA(LoadPluginsWindowData, ExtraData);
						xData->UpdatePluginFlagsFromCache();

						if (TESCSMain::ConfirmUnsavedChanges() == false)
							Return = true;
						else
						{
							// reset the unsaved changes flag to prevent the prompt from showing up a second time
							// when the original message handler is invoked
							*TESCSMain::UnsavedChangesFlag = 0;
						}

						CurrentState.UpdatePluginFlagsFromCache();

						if (Return)
							break;
					}

					if (cliWrapper::interfaces::SE->GetOpenEditorCount())
					{
						if (BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "There are open script windows. Are you sure you'd like to proceed?") == IDNO)
							Return = true;
					}
					else if (cliWrapper::interfaces::SE->IsDiskSyncInProgress())
					{
						BGSEEUI->MsgBoxE(hWnd, 0, "Script syncing is currently in progress. The syncing process must be stopped before (re)loading plugins.");
						Return = true;
					}
					else if (ActiveTESFile != nullptr && !_stricmp(ActiveTESFile->fileName, "oblivion.esm"))
					{
						if (BGSEEUI->MsgBoxW(hWnd,
											 MB_YESNO,
											 "You have set Oblvion.esm as an active file. Are you absolutely sure this is the end of the world?") == IDNO)
						{
							Return = true;
						}
						else if (BGSEEUI->MsgBoxW(hWnd,
												  MB_YESNO,
												  "What you're about to do is tantamount to using the Osterhagen Key.\n\nThis is the Point Of No Return. Proceed?") == IDNO)
						{
							Return = true;
							BGSEEACHIEVEMENTS->Unlock(achievements::kChicken);
						}
						else
							BGSEEACHIEVEMENTS->Unlock(achievements::kFearless);
					}

					if (Return == false)
					{
						FormEnumerationManager::Instance.ResetVisibility();

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
							} while (true);
						}
					}

					break;
				case TESDialog::kStandardButton_Cancel:
					{
						// revert any changes to the loaded flags of plugins that were made in the dialog
						LoadPluginsWindowData* xData = BGSEE_GETWINDOWXDATA(LoadPluginsWindowData, ExtraData);
						xData->UpdatePluginFlagsFromCache();
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}


		LRESULT CALLBACK ResponseDlgSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												 bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND VoiceList = GetDlgItem(hWnd, ResponseEditorData::kVoiceFileListView);

			switch (uMsg)
			{
			case WM_DESTROY:
				settings::general::kFaceGenPreviewVoiceDelay.SetInt(TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY));

				break;
			case WM_INITDIALOG:
				{
					EnableWindow(GetDlgItem(hWnd, ResponseEditorData::kGenerateLIPFileButton), TRUE);

					CheckDlgButton(hWnd,
								   IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW,
								   (settings::general::kFaceGenPreviewResponseWindow.GetData().i ? BST_CHECKED : BST_UNCHECKED));

					TESDialog::ClampDlgEditField(GetDlgItem(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY), 0.0, 5000.0, true);
					TESDialog::SetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY, settings::general::kFaceGenPreviewVoiceDelay.GetData().i, 0);

					SetForegroundWindow(hWnd);
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					switch (NotificationData->idFrom)
					{
					case ResponseEditorData::kVoiceFileListView:
						{
							if (NotificationData->code == LVN_ITEMACTIVATE)
							{
								if (settings::general::kFaceGenPreviewResponseWindow.GetData().i == 0)
									break;

								int SelectedItem = ListView_GetNextItem(VoiceList, -1, LVNI_SELECTED);

								char VoicePath[0x200] = { 0 };
								LVITEM SelectedVoiceItem;

								SelectedVoiceItem.iItem = SelectedItem;
								SelectedVoiceItem.iSubItem = 6;
								SelectedVoiceItem.mask = LVIF_TEXT;
								SelectedVoiceItem.pszText = VoicePath;
								SelectedVoiceItem.cchTextMax = sizeof(VoicePath);
								ListView_GetItem(VoiceList, &SelectedVoiceItem);

								std::string LipPath(VoicePath);
								LipPath = LipPath.substr(0, LipPath.rfind("."));
								LipPath += ".lip";

								HWND NPCEditDlg = nullptr;
								for (tList<HWND>::Iterator Itr = TESDialog::OpenDialogWindows->Begin(); Itr.End() == false && Itr.Get(); ++Itr)
								{
									HWND Current = (HWND)Itr.Get();
									TESForm* Form = TESDialog::GetDialogExtraParam(Current);
									if (Form && Form->formType == TESForm::kFormType_NPC)
									{
										NPCEditDlg = Current;
										break;
									}
								}

								if (NPCEditDlg)
								{
									Return = true;

									FaceGenVoicePreviewData PreviewData = { 0 };
									FORMAT_STR(PreviewData.VoicePath, "%s", VoicePath);
									FORMAT_STR(PreviewData.LipPath, "%s", LipPath.c_str());
									PreviewData.DelayTime = TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY);

									SendMessage(NPCEditDlg, WM_FACEGENPREVIEW_PLAYVOICE, (WPARAM)&PreviewData, NULL);
								}
							}
						}

						break;
					}
				}

				break;
			case WM_COMMAND:
				{
					int SelectedItem = ListView_GetNextItem(VoiceList, -1, LVNI_SELECTED);

					char VoicePath[0x200] = { 0 };
					LVITEM SelectedVoiceItem;

					SelectedVoiceItem.iItem = SelectedItem;
					SelectedVoiceItem.iSubItem = 6;
					SelectedVoiceItem.mask = LVIF_TEXT;
					SelectedVoiceItem.pszText = VoicePath;
					SelectedVoiceItem.cchTextMax = sizeof(VoicePath);

					switch (LOWORD(wParam))
					{
					case IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW:
						{
							settings::general::kFaceGenPreviewResponseWindow.SetInt(IsDlgButtonChecked(hWnd, IDC_CSE_RESPONSEWINDOW_FACEGENPREVIEW) == BST_CHECKED);
						}

						break;
					case ResponseEditorData::kCopyExternalFileButton:
						{
							if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
							{
								BGSEEUI->MsgBoxI(hWnd, NULL, "Please select a voice file first from the list above.");
								Return = true;
								break;
							}

							char FilePath[MAX_PATH] = { 0 };
							OPENFILENAME SelectFile = { 0 };
							SelectFile.lStructSize = sizeof(OPENFILENAME);
							SelectFile.hwndOwner = hWnd;
							SelectFile.lpstrFilter = "MP3 Files\0*.mp3\0\0";
							SelectFile.lpstrCustomFilter = nullptr;
							SelectFile.nFilterIndex = 0;
							SelectFile.lpstrFile = FilePath;
							SelectFile.nMaxFile = sizeof(FilePath);
							SelectFile.lpstrFileTitle = nullptr;
							SelectFile.lpstrInitialDir = nullptr;
							SelectFile.lpstrTitle = "Select an audio file to use as the current response's voice";
							SelectFile.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR;

							if (GetOpenFileName(&SelectFile))
							{
								std::string Destination(std::string(BGSEEMAIN->GetAPPPath()) + "\\" + std::string(VoicePath)), DirPath(Destination);
								DirPath = DirPath.substr(0, DirPath.rfind("\\") + 1);

								if (SHCreateDirectoryEx(nullptr, DirPath.c_str(), nullptr) &&
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
							}

							Return = true;
						}

						break;
					case ResponseEditorData::kGenerateLIPFileButton:
						if (ListView_GetItem(VoiceList, &SelectedVoiceItem) != TRUE)
						{
							BGSEEUI->MsgBoxI(hWnd, NULL, "Please select a voice file first from the list above.");
							Return = true;
							break;
						}

						std::string Path(VoicePath);
						Path = Path.substr(0, Path.rfind("."));

						HWND IdleWindow = CreateDialogParam(BGSEEMAIN->GetExtenderHandle(), MAKEINTRESOURCE(IDD_IDLE), BGSEEUI->GetMainWindow(), nullptr, NULL);

						if (CSIOM->GenerateLIPSyncFile(Path.c_str(), (*ResponseEditorData::EditorCache)->responseLocalCopy->responseText.c_str()) == false)
							BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't generate LIP file for the selected voice.\n\nCheck the console for more information.");
						else
						{
							BGSEECONSOLE_MESSAGE("Successfully generated LIP file for the selected voice");
							BGSEEACHIEVEMENTS->Unlock(achievements::kSoprano);
							achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_GenerateLIP);
						}

						DestroyWindow(IdleWindow);

						Return = true;
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK LandscapeTextureUseDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
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
					char Buffer[0x200] = { 0 };
					ListView_GetItemText(Data->hdr.hwndFrom, Data->iItem, 0, Buffer, sizeof(Buffer));

					TESForm* Texture = TESForm::LookupByEditorID(Buffer);

					if (Texture)
					{
						*TESRenderWindow::ActiveLandscapeTexture = CS_CAST(Texture, TESForm, TESLandTexture);
						// select the new texture in the landscape edit dialog
						SendMessage(*TESObjectLAND::WindowHandle, TESDialog::kWindowMessage_Refresh, NULL, NULL);
						SetForegroundWindow(*TESRenderWindow::WindowHandle);

						renderWindow::NotificationOSDLayer::Instance.ShowNotification("Active landscape texture changed");
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK FilteredDialogQuestDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					if (Subclasser->GetDialogTemplate(hWnd) == TESDialog::kDialogTemplate_Quest)
					{
						LVCOLUMN ColumnData = { 0 };
						ColumnData.mask = LVCF_WIDTH;
						HWND QuestList = GetDlgItem(hWnd, kFormList_TESFormIDListView);

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
					if (IsWindowEnabled(GetDlgItem(hWnd, kDialogEditor_ResultScriptTextBox)))
					{
						if (BGSEEUI->ModalDialog(BGSEEMAIN->GetExtenderHandle(),
												 MAKEINTRESOURCE(IDD_EDITRESULTSCRIPT),
												 hWnd,
												 uiManager::EditResultScriptDlgProc,
												 (LPARAM)hWnd))
						{
							SendMessage(hWnd, WM_COMMAND, kDialogEditor_CompileResultScriptButton, NULL);		// compile result script
						}
					}

					Return = true;
					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK AboutDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											  bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
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

						HWND PictureControl = GetDlgItem(hWnd, kAboutDialog_LogoPictureControl);
						SendMessage(PictureControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)Image);
					}

					DWORD FileVersionHandle = 0;
					DWORD FileVersionSize = GetFileVersionInfoSize(BGSEEMAIN->GetDLLPath(), &FileVersionHandle);

					if (FileVersionSize)
					{
						char* Buffer = new char[FileVersionSize];
						char VersionString[0x100] = { 0 };
						void* VersionStringPtr = nullptr;

						GetFileVersionInfo(BGSEEMAIN->GetDLLPath(), FileVersionHandle, FileVersionSize, Buffer);
						VerQueryValue(Buffer, "\\StringFileInfo\\040904b0\\ProductVersion", &VersionStringPtr, (PUINT)FileVersionHandle);
						FORMAT_STR(VersionString, "%s v%s\r\n\"%s\"", BGSEEMAIN->ExtenderGetDisplayName(),
							(char*)VersionStringPtr, BGSEEMAIN->ExtenderGetReleaseName());

						std::string ReplacedString(VersionString);
						std::replace(ReplacedString.begin(), ReplacedString.end(), ',', '.');

						SetDlgItemText(hWnd, kAboutDialog_VersionLabel, (LPCSTR)ReplacedString.c_str());

						if (IsWarholAGenius)
						{
							Edit_SetText(GetDlgItem(hWnd, kAboutDialog_CopyrightTextBox),
										 "Pain, day, sky, beauty, black, die, joy,\n love, empty, time, sun, hurt, trust, peace, dark, rage, sad, white, rain, hate,\n anger, hope, sacred, passion, life, night,\n ache, soft light");
						}

						delete[] Buffer;
					}
				}

				break;
			case WM_DESTROY:
				{
					HANDLE Image = (HANDLE)SendDlgItemMessage(hWnd, kAboutDialog_LogoPictureControl, STM_GETIMAGE, IMAGE_BITMAP, NULL);

					if (Image)
						DeleteBitmap(Image);
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK RaceDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											 bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
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
																			(DLGPROC)uiManager::TESComboBoxDlgProc,
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


		LRESULT CALLBACK LandscapeEditDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													  bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					// immediately close the dialog if no exterior cell is loaded
					// otherwise, the editor will crash as soon as the render window acquires input focus
					if (*TESRenderWindow::ActiveCell == nullptr || (*TESRenderWindow::ActiveCell)->IsInterior())
						SendMessage(hWnd, WM_COMMAND, TESDialog::kStandardButton_Cancel, NULL);
					else
						SendDlgItemMessage(hWnd, kFormList_LandTextures, LVM_SORTITEMS, 0, (LPARAM)TESDialog::LandscapeTextureSortComparator);
				}

				break;
			case TESDialog::kWindowMessage_Refresh:		// update active landscape texture
				{
					Return = true;

					HWND TexList = GetDlgItem(hWnd, kFormList_LandTextures);
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

		LRESULT CALLBACK AIPackagesDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												   bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FormList = GetDlgItem(hWnd, kFormList_TESPackage);

			switch (uMsg)
			{
			case WM_DESTROY:
				FilterableFormListManager::Instance.Unregister(FilterEditBox);

				break;
			case WM_INITDIALOG:
				{
					HWND PackageListView = GetDlgItem(hWnd, kFormList_TESPackage);
					LVCOLUMN ColumnData = { 0 };
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 175;
					ListView_SetColumn(PackageListView, 0, &ColumnData);

					FilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), FormList, hWnd);
				}

				break;
			case WM_COMMAND:
				if (LOWORD(wParam) == TESDialog::kStandardButton_Cancel)		// prevents the dialog from closing itself on renaming an AI package
				{
					if (HIWORD(wParam) == EN_KILLFOCUS || HIWORD(wParam) == EN_SETFOCUS)
					{
						Return = true;
						DlgProcResult = TRUE;
					}
				}

				break;
			}

			if (FilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
				SendMessage(hWnd, TESDialog::kWindowMessage_Refresh, 0, 0);

			return DlgProcResult;
		}

		LRESULT CALLBACK AIFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
											   bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					HWND PackageListView = GetDlgItem(hWnd, kFormList_TESPackage);

					TESPackage::InitializeListViewColumns(PackageListView);	// do this just once, not every time the listview is refreshed

					LVCOLUMN ColumnData = { 0 };
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 150;
					ListView_SetColumn(PackageListView, 0, &ColumnData);
				}

				break;
			}

			return DlgProcResult;
		}

#define IDT_FACEGENPREVIEW_VOICEPLAYBACK			0x6FF
#define IDT_FACEGENPREVIEW_PREVIEWUPDATE			0x7FF

		LRESULT CALLBACK FaceGenParamListSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													  bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_MOUSEWHEEL:
				{
					if (GetAsyncKeyState(VK_RBUTTON))
					{
						// RMB is held down, forward the message to the trackbar and then consume it
						Return = true;

						HWND Parent = nullptr;
						if ((Parent = GetParent(hWnd)))
						{
							HWND Trackbar = GetDlgItem(Parent, kFaceGenControl_AdvancedTrackbar);
							if (Trackbar)
								SendMessage(Trackbar, uMsg, wParam, lParam);
						}
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK FaceGenDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_TIMER:
				{
					switch (wParam)
					{
					case IDT_FACEGENPREVIEW_VOICEPLAYBACK:
						{
							Return = true;

							FaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(FaceGenWindowData, ExtraData);
							SME_ASSERT(xData);

							TESSound::PlaySoundFile(xData->VoicePlaybackFilePath.c_str());
							KillTimer(hWnd, IDT_FACEGENPREVIEW_VOICEPLAYBACK);
						}

						break;
					case IDT_FACEGENPREVIEW_PREVIEWUPDATE:
						{
							Return = true;

							FaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(FaceGenWindowData, ExtraData);
							SME_ASSERT(xData);

							// re-enable updates now that most, if not all, of the init is done
							xData->AllowPreviewUpdates = true;
							KillTimer(hWnd, IDT_FACEGENPREVIEW_PREVIEWUPDATE);
						}

						break;
					}
				}

				break;
			case WM_FACEGENPREVIEW_PLAYVOICE:
				{
					FaceGenVoicePreviewData* Data = (FaceGenVoicePreviewData*)wParam;
					SME_ASSERT(Data);

					const char* VoicePath = Data->VoicePath;
					const char* LipPath = Data->LipPath;

					if (LipPath)
					{
						std::string RelativeLipPath(_FILEFINDER->GetRelativePath(LipPath, "Data\\Sound\\Voice\\"));
						std::string RelativeVoicePath = "";
						if (strlen(VoicePath))
							RelativeVoicePath = std::string(_FILEFINDER->GetRelativePath(VoicePath, "Data\\Sound\\Voice\\"));

						if (_FILEFINDER->FindFile(RelativeLipPath.c_str()) == FileFinder::kFileStatus_NotFound)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't find lip file at '%s'.", RelativeLipPath.c_str());
							break;
						}
						else if (!RelativeVoicePath.empty() && _FILEFINDER->FindFile(RelativeVoicePath.c_str()) == FileFinder::kFileStatus_NotFound)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't find voice file at '%s'.", RelativeVoicePath.c_str());
							break;
						}

						Setting* FaceTestSoundName = INISettingCollection::Instance->LookupByName("sFaceTestSoundName:General");
						SME_ASSERT(FaceTestSoundName);

						FaceTestSoundName->SetStringValue(RelativeLipPath.c_str());

						SetForegroundWindow(hWnd);

						// make sure the head preview is selected
						if (IsDlgButtonChecked(hWnd, TESNPC::kHeadPeviewCheckBox) != BST_CHECKED)
							SendMessage(hWnd, WM_COMMAND, TESNPC::kHeadPeviewCheckBox, NULL);

						SetActiveWindow(GetDlgItem(hWnd, kFaceGenControl_PreviewCtrl));
						SendDlgItemMessage(hWnd, kFaceGenControl_PreviewCtrl, WM_KEYDOWN, 0x4C, NULL);		// L key

																											// delay voice file playback to account for synchronization
						if (!RelativeVoicePath.empty())
						{
							FaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(FaceGenWindowData, ExtraData);
							SME_ASSERT(xData);

							if (Data->DelayTime < 8 || Data->DelayTime > 5000)
								Data->DelayTime = 10;

							xData->VoicePlaybackFilePath = RelativeVoicePath;
							SetTimer(hWnd, IDT_FACEGENPREVIEW_VOICEPLAYBACK, Data->DelayTime, nullptr);
						}
					}
				}

				break;
			case WM_DESTROY:
				{
					FaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(FaceGenWindowData, ExtraData);

					if (xData)
					{
						ExtraData->Remove(FaceGenWindowData::kTypeID);
						delete xData;
					}

					DragAcceptFiles(hWnd, FALSE);
					settings::general::kFaceGenPreviewVoiceDelay.SetInt(TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY));
				}

				break;
			case WM_INITDIALOG:
				{
					FaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(FaceGenWindowData, ExtraData);
					if (xData == nullptr)
					{
						xData = new FaceGenWindowData();
						ExtraData->Add(xData);
					}

					DragAcceptFiles(hWnd, TRUE);
					TESDialog::ClampDlgEditField(GetDlgItem(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY), 0.0, 5000.0, true);
					TESDialog::SetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY, settings::general::kFaceGenPreviewVoiceDelay.GetData().i, 0);
				}

				break;
			case WM_DROPFILES:
				{
					HDROP DropData = (HDROP)wParam;
					char FilePathVoice[MAX_PATH] = { 0 }, FilePathLip[MAX_PATH] = { 0 }, Buffer[MAX_PATH] = { 0 };
					UInt32 FileCount = DragQueryFile(DropData, 0xFFFFFFFF, FilePathLip, sizeof(FilePathLip));

					if (FileCount == 1 || FileCount == 2)
					{
						for (int i = 0; i < FileCount; i++)
						{
							if (DragQueryFile(DropData, i, Buffer, sizeof(Buffer)))
							{
								if (_FILEFINDER->GetRelativePath(Buffer, "Data\\Sound\\Voice\\") && strrchr(Buffer, '.'))
								{
									char* Extension = strrchr(Buffer, '.') + 1;
									if (!_stricmp(Extension, "lip"))
										memcpy(FilePathLip, Buffer, sizeof(Buffer));
									else if (!_stricmp(Extension, "wav") || !_stricmp(Extension, "mp3"))
										memcpy(FilePathVoice, Buffer, sizeof(Buffer));
									else
										BGSEEUI->MsgBoxE(hWnd, 0, "Invalid extension '%s' - Must be one of the following: mp3, wav, lip.", Extension);
								}
								else
									BGSEEUI->MsgBoxE(hWnd, 0, "Path '%s' is invalid - Must be inside the Data\\Sound\\Voice directory.", Buffer);
							}
							else
								BGSEEUI->MsgBoxE(hWnd, 0, "Couldn't retrieve dropped file path at index %d.", i);
						}

						if (strlen(FilePathLip))
						{
							FaceGenVoicePreviewData PreviewData = { 0 };
							FORMAT_STR(PreviewData.VoicePath, "%s", FilePathVoice);
							FORMAT_STR(PreviewData.LipPath, "%s", FilePathLip);
							PreviewData.DelayTime = TESDialog::GetDlgItemFloat(hWnd, IDC_CSE_RESPONSEWINDOW_VOICEDELAY);

							SendMessage(hWnd, WM_FACEGENPREVIEW_PLAYVOICE, (WPARAM)&PreviewData, NULL);
						}
					}
					else if (FileCount)
						BGSEEUI->MsgBoxW(hWnd, 0, "No more than two files may be dropped into the preview control.");

					DragFinish(DropData);
				}

				break;
				{
			case WM_NOTIFY:
				NMHDR* NotificationData = (NMHDR*)lParam;

				switch (NotificationData->idFrom)
				{
				case TESTabControl::kTabControl:		// tab control, same for both the NPC and Race dialogs
					{
						if (NotificationData->code == TCN_SELCHANGE)
						{
							FaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(FaceGenWindowData, ExtraData);

							// consume the original notification and disable preview control updates
							// otherwise, the morph values will get reset upon subwindow init
							if (xData && xData->TunnelingTabSelectMessage == false)
							{
								Return = true;

								xData->TunnelingTabSelectMessage = true;
								xData->AllowPreviewUpdates = false;

								SendMessage(hWnd, uMsg, wParam, lParam);

								xData->TunnelingTabSelectMessage = false;

								// don't allow updates immediately, use a cool-down timer
								static const UInt32 kPreviewUpdateCoolDownPeriod = 1000;		// in ms

								SetTimer(hWnd, IDT_FACEGENPREVIEW_PREVIEWUPDATE, kPreviewUpdateCoolDownPeriod, nullptr);

								HWND AdvancedParamsList = GetDlgItem(hWnd, kFaceGenControl_AdvancedParamsListView);
								if (AdvancedParamsList)
								{
									char Buffer[0x200] = { 0 };
									GetClassName(AdvancedParamsList, Buffer, sizeof(Buffer));

									if (!_stricmp("SysListView32", Buffer))
									{
										// right, subclass it to forward mouse wheel messages to the trackbar
										Subclasser->RegisterSubclassForWindow(AdvancedParamsList, FaceGenParamListSubClassProc);
									}
								}
							}
						}
					}

					break;
				}
				}

				break;
			case WM_COMMAND:
				switch (HIWORD(wParam))
				{
				case EN_CHANGE:
					{
						// the method used here is actually rather buggy
						// the 0x41A message updates the NPC/Race form before regenerating the face model
						// this causes the form's compare method to return true, leading to false changes
						// the issue is only noticeable in the race edit dialog as it's a FormIDListView dlg
						// we HACK around this by preventing updates for a short time after the current tab's been switched
						// during this time, updates to the edit controls' text will be ignored by this handler
						FaceGenWindowData* xData = BGSEE_GETWINDOWXDATA(FaceGenWindowData, ExtraData);
						if (xData == nullptr || xData->AllowPreviewUpdates == false)
							break;

						if (LOWORD(wParam) == kFaceGenControl_AgeEditCtrl ||
							LOWORD(wParam) == kFaceGenControl_ComplexionEditCtrl ||
							LOWORD(wParam) == kFaceGenControl_HairLengthEditCtrl ||
							LOWORD(wParam) == kFaceGenControl_AdvancedEditCtrl)
						{
							// refresh the preview control
							SendMessage(hWnd, TESDialog::kWindowMessage_Refresh, NULL, NULL);
						}
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}


		LRESULT CALLBACK MagicItemFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													  bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_NOTIFY:
				{
					HWND EffectItemListView = GetDlgItem(hWnd, EffectItemList::kEffectItemListView);
					NMHDR* NotificationData = (NMHDR*)lParam;

					if (NotificationData->hwndFrom != EffectItemListView)
						break;

					switch (NotificationData->code)
					{
					case LVN_COLUMNCLICK:
						Return = true;				// no sorting!

						break;
					case LVN_KEYDOWN:
						if (GetAsyncKeyState(VK_CONTROL))
						{
							Return = true;

							NMLVKEYDOWN* KeyData = (NMLVKEYDOWN*)lParam;
							TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);

							if (LocalCopy && (KeyData->wVKey == VK_UP || KeyData->wVKey == VK_DOWN))
							{
								EffectItemList* EffectItems = CS_CAST(LocalCopy, TESForm, EffectItemList);
								EffectItem* Selection = (EffectItem*)TESListView::GetSelectedItemData(EffectItemListView);

								if (EffectItems && Selection)
								{
									EffectItemList::EffectItemListT::_Node* CurrentNode = nullptr;
									EffectItemList::EffectItemListT::_Node* PreviousNode = nullptr;
									EffectItemList::EffectItemListT::_Node* NextNode = nullptr;

									for (EffectItemList::EffectItemListT::_Node* Itr = EffectItems->effects.Head();
										 Itr; PreviousNode = Itr, Itr = Itr->Next())
									{
										CurrentNode = Itr;
										NextNode = Itr->Next();

										if (Itr->Item() == Selection)
											break;
									}

									bool SelectionMoved = true, Up = false;
									switch (KeyData->wVKey)
									{
									case VK_UP:
										Up = true;

										if (PreviousNode == nullptr)
											SelectionMoved = false;
										else
										{
											EffectItem* Buffer = CurrentNode->item;
											CurrentNode->item = PreviousNode->item;
											PreviousNode->item = Buffer;
										}

										break;
									case VK_DOWN:
										if (NextNode == nullptr)
											SelectionMoved = false;
										else
										{
											EffectItem* Buffer = NextNode->item;
											NextNode->item = CurrentNode->item;
											CurrentNode->item = Buffer;
										}

										break;
									}

									if (SelectionMoved)
									{
										EffectItems->PopulateListView(EffectItemListView);
										TESListView::SetSelectedItem(EffectItemListView,
																	 TESListView::GetItemByData(EffectItemListView, Selection) + (Up ? 1 : -1));
									}
								}
							}
						}
					}
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK LeveledItemFormDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case TESDialog::kStandardButton_Ok:
					{
						// validate list
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						// use a temp copy as the local copy is used to determine if changes were made
						TESForm* TempCopy = TESForm::CreateTemporaryCopy(LocalCopy);
						TempCopy->GetDataFromDialog(hWnd);

						std::string ValidationOutput = "";
						if ((CS_CAST(TempCopy, TESForm, TESLeveledList))->CheckForCircularPaths(ValidationOutput) == false)
						{
							BGSEEUI->MsgBoxE(hWnd, 0, "The leveled list contents are invalid!\n\nA circular link was found at:\n%s", ValidationOutput.c_str());
							Return = true;
						}

						TempCopy->DeleteInstance();
					}

					break;
				}

				break;
			}

			return DlgProcResult;
		}

		LRESULT CALLBACK TESObjectCELLDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													  bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND FormList = GetDlgItem(hWnd, kFormList_TESFormIDListView);

			switch (uMsg)
			{
			case WM_COMMAND:
				if (LOWORD(wParam) == TESCellViewWindow::kWorldspaceComboBox && HIWORD(wParam) == 1)
				{
					Return = true;

					if ((HWND)lParam == GetDlgItem(hWnd, TESCellViewWindow::kWorldspaceComboBox))
					{
						// we didn't send this message, so clear the filter string
						// this is done to prevent the dialog's controls from being disabled if the active filter string doesn't match any forms in the new worldspace
						FilterableFormListManager::Instance.SetEnabledState(FilterEditBox, false);
						SetWindowText(FilterEditBox, "");
						Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
						FilterableFormListManager::Instance.SetEnabledState(FilterEditBox, true);
					}
					else if (lParam == NULL)
					{
						// filter refresh
						// prevent filter controls from being disabled
						Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
						EnableWindow(FilterEditBox, TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), TRUE);
					}
				}

				break;
			case WM_DESTROY:
				FilterableFormListManager::Instance.Unregister(FilterEditBox);

				break;
			case WM_INITDIALOG:
				{
					SME_ASSERT(FilterEditBox);
					FilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL), FormList, hWnd);
				}

				break;
			}

			if (FilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				// pass NULL as lParam to indicate a filter refresh
				SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(TESCellViewWindow::kWorldspaceComboBox, 1), NULL);
			}

			return DlgProcResult;
		}

		int CALLBACK SelectTopicsQuestsFormListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

					Result = FormEnumerationManager::Instance.CompareActiveForms(FormA, FormB, Result);
				}
			}

			return Result;
		}

		LRESULT CALLBACK SelectTopicsQuestsSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			HWND FormList = GetDlgItem(hWnd, kFormList_Generic);
			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					LVCOLUMN ColumnData = { 0 };
					ColumnData.mask = LVCF_WIDTH;

					ColumnData.cx = 360;
					ListView_SetColumn(FormList, 0, &ColumnData);

					SetWindowLongPtr(FormList, GWL_USERDATA, NULL);

					SME_ASSERT(FilterEditBox);
					FilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL),
																 FormList, hWnd);
				}

				break;
			case WM_DESTROY:
				FilterableFormListManager::Instance.Unregister(FilterEditBox);

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

			if (Return == false && FilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				if (Subclasser->GetDialogTemplate(hWnd) == TESDialog::kDialogTemplate_SelectTopic)
					(*SelectTopicWindowData::Singleton)->RefreshListView(hWnd);
				else
					(*SelectQuestWindowData::Singleton)->RefreshListView(hWnd);
			}

			return DlgProcResult;
		}


		void InitializeMiscWindowOverrides()
		{
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_FindText, FindTextDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Data, DataDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_ResponseEditor, ResponseDlgSubclassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_TextureUse, LandscapeTextureUseDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Quest, FilteredDialogQuestDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_FilteredDialog, FilteredDialogQuestDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_About, AboutDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Race, RaceDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SelectTopic, SelectTopicsQuestsSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SelectQuests, SelectTopicsQuestsSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LandscapeEdit, LandscapeEditDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AIPackages, AIPackagesDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AIForm, AIFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_NPC, FaceGenDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Race, FaceGenDlgSubClassProc);

			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Ingredient, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SigilStone, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Enchantment, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Potion, MagicItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Spell, MagicItemFormDlgSubClassProc);

			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledItem, LeveledItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledCreature, LeveledItemFormDlgSubClassProc);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledSpell, LeveledItemFormDlgSubClassProc);

			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CellEdit, TESObjectCELLDlgSubClassProc);
		}


	}
}