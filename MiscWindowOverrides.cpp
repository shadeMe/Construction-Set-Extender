#include "MiscWindowOverrides.h"
#include "Construction Set Extender_Resource.h"
#include "Main.h"
#include "UIManager.h"
#include "Achievements.h"
#include "OldCSInteropManager.h"
#include "CustomDialogProcs.h"
#include "Render Window\RenderWindowManager.h"
#include "Construction Set Extender_Resource.h"
#include "DialogImposterManager.h"
#include "Achievements.h"
#include "HallOfFame.h"
#include "FormUndoStack.h"
#include "[Common]\CLIWrapper.h"

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

						if (CSIOM.GenerateLIPSyncFile(Path.c_str(), (*ResponseEditorData::EditorCache)->responseLocalCopy->responseText.c_str()) == false)
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
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					HWND PackageListView = GetDlgItem(hWnd, kFormList_TESPackage);

					switch (NotificationData->code)
					{
					case LVN_KEYDOWN:
						if (NotificationData->hwndFrom == PackageListView)
						{
							NMLVKEYDOWN* KeyData = (NMLVKEYDOWN*)lParam;
							if (KeyData->wVKey == VK_F1)
							{
								auto SelectedPackage = TESListView::GetSelectedItemData(PackageListView);
								if (SelectedPackage)
									TESDialog::ShowUseReportDialog(reinterpret_cast<TESForm*>(SelectedPackage));
							}
						}

						break;
					}

					break;
				}
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
						FilterableFormListManager::Instance.SetEnabled(FilterEditBox, false);
						SetWindowText(FilterEditBox, "");
						Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);
						FilterableFormListManager::Instance.SetEnabled(FilterEditBox, true);
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


		DialogExtraFittingsData::DialogExtraFittingsData() :
			bgsee::WindowExtraData(kTypeID)
		{
			LastCursorPos.x = LastCursorPos.y = 0;
			LastCursorPosWindow = nullptr;
			QuickViewTriggered = false;

			AssetControlToolTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, nullptr,
												TTS_ALWAYSTIP|TTS_NOPREFIX,
												CW_USEDEFAULT, CW_USEDEFAULT,
												CW_USEDEFAULT, CW_USEDEFAULT,
												nullptr, nullptr, nullptr, nullptr);
			ZeroMemory(&AssetControlToolData, sizeof(AssetControlToolData));
			AssetControlToolData.cbSize = sizeof(AssetControlToolData);
			LastTrackedTool = nullptr;
			TrackingToolTip = false;
		}

		DialogExtraFittingsData::~DialogExtraFittingsData()
		{
			if (AssetControlToolTip)
				DestroyWindow(AssetControlToolTip);
		}

		TESFormEditData::TESFormEditData() :
			bgsee::WindowExtraData(kTypeID)
		{
			Buffer = nullptr;
		}

		TESFormEditData::~TESFormEditData()
		{
			if (Buffer)
				Buffer->DeleteInstance();
		}

		void TESFormEditData::FillBuffer( TESForm* Parent )
		{
			SME_ASSERT(Buffer == nullptr);

			Buffer = TESForm::CreateTemporaryCopy(Parent);
		}

		bool TESFormEditData::HasChanges( TESForm* Parent )
		{
			SME_ASSERT(Buffer && Buffer->formType == Parent->formType);

			return Buffer->CompareTo(Parent);
		}


		TESFormIDListViewData::TESFormIDListViewData() :
			bgsee::WindowExtraData(kTypeID)
		{
			DisableDragHandling = false;
		}

		TESFormIDListViewData::~TESFormIDListViewData()
		{
			;//
		}


#define ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID						0x108
#define ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID					0x109

		BOOL CALLBACK DoubleBufferListViewControls(HWND hwnd, LPARAM lParam)
		{
			char Buffer[100];
			GetClassName(hwnd, Buffer, sizeof(Buffer));

			if (!strcmp("SysListView32", Buffer))
				ListView_SetExtendedListViewStyleEx(hwnd, LVS_EX_DOUBLEBUFFER, LVS_EX_DOUBLEBUFFER);

			return TRUE;
		}


		LRESULT CALLBACK CommonDialogExtraFittingsSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
															bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;
			DialogExtraFittingsData* xData = BGSEE_GETWINDOWXDATA(DialogExtraFittingsData, ExtraData);

			switch (uMsg)
			{
			case WM_INITDIALOG:
			case WM_OBJECTWINDOWIMPOSTER_INITIALIZEXTRA:
				{
					xData = BGSEE_GETWINDOWXDATA(DialogExtraFittingsData, ExtraData);
					if (xData == nullptr)
					{
						xData = new DialogExtraFittingsData();
						ExtraData->Add(xData);
					}

					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_QUICKVIEWTIMERID, 100, nullptr);
					SetTimer(hWnd, ID_COMMONDLGEXTRAFITTINGS_ASSETTOOLTIPTIMERID, 650, nullptr);


					EnumChildWindows(hWnd, DoubleBufferListViewControls, NULL);
				}

				break;
			case WM_DESTROY:
				{
					xData = BGSEE_GETWINDOWXDATA(DialogExtraFittingsData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(DialogExtraFittingsData::kTypeID);
						delete xData;

						xData = nullptr;
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
							bool Viewable = false;
							char Buffer[0x200] = {0}, ControlText[0x200] = {0};

							if (IsWindowEnabled(WindowAtPoint))
							{
								int CtrlID = GetDlgCtrlID(WindowAtPoint);
								LONG_PTR WindowStyle = GetWindowLongPtr(WindowAtPoint, GWL_STYLE);

								GetClassName(WindowAtPoint, Buffer, sizeof(Buffer));
								GetWindowText(WindowAtPoint, (LPSTR)ControlText, sizeof(ControlText));

								if ((!_stricmp("Button", Buffer) &&
									CtrlID != kFaceGenControl_PreviewCtrl &&
									(WindowStyle & BS_AUTOCHECKBOX) == false &&
									CtrlID > 5 && strlen(ControlText) > 3) ||
									!_stricmp("ComboBox", Buffer))
								{
									Viewable = true;
								}
							}

							if (Viewable)
							{
								// valid control, show tooltip
								if (strlen(ControlText))
								{
									ZeroMemory(&xData->AssetControlToolData, sizeof(TOOLINFO));
									xData->AssetControlToolData.cbSize = sizeof(TOOLINFO);
									xData->AssetControlToolData.uFlags = TTF_TRACK;
									xData->AssetControlToolData.lpszText = ControlText;

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
							TESForm* Form = nullptr;

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
								GetWindowText(WindowAtPoint, Buffer, sizeof(Buffer));

							if (strlen(Buffer) < 2)
								break;

							std::string PotentialEditorID(Buffer);
							int StatusIndicatorOffset = -1;

							if ((StatusIndicatorOffset = PotentialEditorID.find(" *")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);
							if ((StatusIndicatorOffset = PotentialEditorID.find(" D")) != std::string::npos)
								PotentialEditorID.erase(StatusIndicatorOffset, 2);

							Form = TESForm::LookupByEditorID(PotentialEditorID.c_str());
							xData->LastCursorPosWindow = nullptr;

							if (Form)
							{
								achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_QuickLookup);
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
							wParam == kFormList_LandTextures ||
							wParam == kFormList_CrossReferences ||
							wParam == kFormList_CellUseList)
						{
							NMLVCUSTOMDRAW* DrawData = (NMLVCUSTOMDRAW*)lParam;

							switch (DrawData->nmcd.dwDrawStage)
							{
							case CDDS_PREPAINT:
								{
									SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
									DlgProcResult = CDRF_NOTIFYITEMDRAW;
									Return = true;
								}

								break;
							case CDDS_ITEMPREPAINT:
								{
									TESForm* Form = nullptr;

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
											auto TemplateID = Subclasser->GetDialogTemplate(hWnd);

											switch (TemplateID)
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
									case kFormList_CellUseList:
										{
											TESCellUseList::CellUseInfo* Info = (TESCellUseList::CellUseInfo*)DrawData->nmcd.lItemlParam;
											SME_ASSERT(Info);
											Form = Info->cell;
										}

										break;
									default:
										Form = (TESForm*)DrawData->nmcd.lItemlParam;

										break;
									}

									bool ColorizeActiveFormsEnabled = settings::dialogs::kColorizeActiveForms.GetData().i &&
																	FormEnumerationManager::Instance.GetVisibleUnmodifiedForms();

									bool ColorizeFormOverridesEnabled = settings::dialogs::kColorizeFormOverrides.GetData().i;

									if (Form && (ColorizeActiveFormsEnabled || ColorizeFormOverridesEnabled))
									{
										COLORREF ForeColor, BackColor;
										bool ColorOverridden = false;

										if (ColorizeFormOverridesEnabled)
										{
											ColorOverridden = true;

											switch (Form->fileList.Count())
											{
											case 0:
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel0ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel0BackColor.GetData().s);

												break;
											case 1:
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel1ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel1BackColor.GetData().s);

												break;
											case 2:
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel2ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel2BackColor.GetData().s);

												break;
											default:
												ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel3ForeColor.GetData().s);
												BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kFormOverrideLevel3BackColor.GetData().s);

												break;
											}
										}

										if (Form->IsActive() && ColorizeActiveFormsEnabled)
										{
											ColorOverridden = true;

											ForeColor = SME::StringHelpers::GetRGB(settings::dialogs::kActiveFormForeColor.GetData().s);
											BackColor = SME::StringHelpers::GetRGB(settings::dialogs::kActiveFormBackColor.GetData().s);
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
							break;
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


// returns TRUE if there are changes
#define WM_TESFORMIDLISTVIEW_HASCHANGES							(WM_USER + 2006)
// lParam = NMLISTVIEW*
#define WM_TESFORMIDLISTVIEW_SAVECHANGES						(WM_USER + 2007)
#define ID_TESFORMIDLISTVIEW_DRAGTIMER							(WM_USER + 2008)

		LRESULT CALLBACK TESFormIDListViewDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
														bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser )
		{
			LRESULT DlgProcResult = FALSE;

			HWND FilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);

			Return = false;

			switch (uMsg)
			{
			case WM_TESFORMIDLISTVIEW_HASCHANGES:
				{
					// doesn't reliably work on TESGlobal forms
					// given their bollocks'd-up way of copying data from the dialog
					Return = true;

					if (IsWindowEnabled(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok)))
					{
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

						if (WorkingCopy)
						{
							LocalCopy->GetDataFromDialog(hWnd);

							if (WorkingCopy->CompareTo(LocalCopy))
								DlgProcResult = TRUE;
						}
					}

					SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
				}

				break;
			case WM_TESFORMIDLISTVIEW_SAVECHANGES:
				{
					if (IsWindowEnabled(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok)))
					{
						TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);
						NMLISTVIEW* ChangeData = (NMLISTVIEW*)lParam;

						if (WorkingCopy)
						{
							LocalCopy->GetDataFromDialog(hWnd);

							if (WorkingCopy->CompareTo(LocalCopy))
							{
								// fill in a new proxy from the working copy
								BGSEEUNDOSTACK->Record(new formUndoStack::FormUndoProxy(WorkingCopy));

								if (WorkingCopy->UpdateUsageInfo())
								{
									WorkingCopy->SetFromActiveFile(true);
									WorkingCopy->CopyFrom(LocalCopy);

									if (WorkingCopy->formType == TESForm::kFormType_EffectSetting)
										BGSEEACHIEVEMENTS->Unlock(achievements::kMagister);

									if (ChangeData)
										SendMessage(ChangeData->hdr.hwndFrom, LVM_REDRAWITEMS, ChangeData->iItem, ChangeData->iItem);
								}
							}
						}
					}

					Return = true;
				}

				break;
			case WM_INITDIALOG:
				{
					if (Subclasser->GetDialogTemplate(hWnd) != TESDialog::kDialogTemplate_Quest)
					{
						SetWindowText(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok), "Apply");
						SetWindowText(GetDlgItem(hWnd, TESDialog::kStandardButton_Cancel), "Close");
					}

					// ideally, we'd be changing the form listview's style to allow multiple selections
					// unfortunately, adding/removing the LVS_SINGLESEL style post-window creation has no effect
					// so we tuck in our tail and create replacement templates for all TESFormIDListView forms
					// PS: Dammit!

					SME_ASSERT(FilterEditBox);
					FilterableFormListManager::Instance.Register(FilterEditBox, GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL),
																	GetDlgItem(hWnd, kFormList_TESFormIDListView), hWnd);

					TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
					if (xData == nullptr)
					{
						xData = new TESFormIDListViewData();
						ExtraData->Add(xData);
					}
				}

				break;
			case WM_DESTROY:
				{
					FilterableFormListManager::Instance.Unregister(FilterEditBox);
					TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
					if (xData)
					{
						ExtraData->Remove(TESFormIDListViewData::kTypeID);
						delete xData;
					}
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_TESFORMIDLISTVIEW_DRAGTIMER:
					{
						// reenable dragging after a single tick
						KillTimer(hWnd, ID_TESFORMIDLISTVIEW_DRAGTIMER);
						TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
						SME_ASSERT(xData);
						xData->DisableDragHandling = false;
					}

					break;
				}

				break;
			case WM_COMMAND:
				if (HIWORD(wParam))		// to keep EN_KILLFOCUS notifications from inadvertently calling the button handlers
					break;				// ### could cause weird behaviour later on

				if (Subclasser->GetDialogTemplate(hWnd) == TESDialog::kDialogTemplate_Quest)
					break;

				switch (LOWORD(wParam))
				{
				case TESDialog::kStandardButton_Ok:			// OK/Apply button
					{
						Return = true;

						SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, NULL);
					}

					break;
				case TESDialog::kStandardButton_Cancel:			// Cancel/Close button
					{
						Return = true;
						bool Cancelled = false;

						if (SendMessage(hWnd, WM_TESFORMIDLISTVIEW_HASCHANGES, NULL, NULL) == TRUE)
						{
							int MsgResult = BGSEEUI->MsgBoxW(hWnd, MB_YESNOCANCEL, "Save changes made to the active form before closing the dialog?");

							switch (MsgResult)
							{
							case IDCANCEL:
								Cancelled = true;

								break;
							case IDYES:
								SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, NULL);

								break;
							}

							if (Cancelled)
								break;
						}

						DestroyWindow(hWnd);
					}

					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					if (NotificationData->idFrom != kFormList_TESFormIDListView)
						break;		// only interested in the main listview control

					if (Subclasser->GetDialogTemplate(hWnd) == TESDialog::kDialogTemplate_Quest)
						break;

					switch (NotificationData->code)
					{
					case LVN_BEGINLABELEDIT:
						{
							// prevent the modification of GMST editorIDs, they aren't meant to be modified
							DialogExtraParam* xParam = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param), BSExtraData, DialogExtraParam);
							SME_ASSERT(xParam);

							if (xParam->formType == TESForm::kFormType_GMST)
							{
								Return = true;

								DlgProcResult = TRUE;
								SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
							}
						}

						break;
					case LVN_BEGINDRAG:
						{
							TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
							SME_ASSERT(xData);
							if (xData->DisableDragHandling == false)
							{
								// override the vanilla handler to allow multiple selections
								NMLISTVIEW* Data = (NMLISTVIEW*)lParam;
								std::list<TESForm*> Selection;

								int Index = -1;
								while ((Index = ListView_GetNextItem(Data->hdr.hwndFrom, Index, LVNI_SELECTED)) != -1)
								{
									TESForm* Form = (TESForm*)TESListView::GetItemData(Data->hdr.hwndFrom, Index);
									SME_ASSERT(Form);

									Selection.push_back(Form);
								}

								if (Selection.size())
								{
									*TESDialog::TESFormIDListViewDragDropInProgress = 1;

									_RENDERSEL->ClearSelection(true);
									TESObjectWindow::SetSplitterEnabled(*TESObjectWindow::SplitterHandle, false);

									for (std::list<TESForm*>::iterator Itr = Selection.begin(); Itr != Selection.end(); ++Itr)
										_RENDERSEL->AddToSelection(*Itr);

									SetCursor(LoadCursor(*TESCSMain::Instance, (LPCSTR)0xB8));
									SetCapture(hWnd);

									Return = true;
								}
							}
							else
								Return = true;
						}

						break;
					case LVN_ITEMCHANGED:
						{
							NMLISTVIEW* ChangeData = (NMLISTVIEW*)lParam;

							if ((ChangeData->uChanged & LVIF_STATE) &&
								(ChangeData->uOldState & LVIS_FOCUSED) &&
								(ChangeData->uNewState & LVIS_FOCUSED) == false)
							{
								// before the new listview item is selected

								if (SendMessage(hWnd, WM_TESFORMIDLISTVIEW_HASCHANGES, NULL, NULL) == TRUE)
								{
									int MsgResult = BGSEEUI->MsgBoxW(hWnd, MB_YESNO, "Save changes made to the active form?");

									switch (MsgResult)
									{
									case IDYES:
										SendMessage(hWnd, WM_TESFORMIDLISTVIEW_SAVECHANGES, NULL, lParam);

										break;
									}
								}

								// we need to disable the listview begin drag handler for a bit to prevent it from getting triggered after the selection
								TESFormIDListViewData* xData = BGSEE_GETWINDOWXDATA(TESFormIDListViewData, ExtraData);
								SME_ASSERT(xData);
								xData->DisableDragHandling = true;
								SetTimer(hWnd, ID_TESFORMIDLISTVIEW_DRAGTIMER, 1000, nullptr);

								Return = true;
								SetWindowLongPtr(hWnd, DWL_MSGRESULT, DlgProcResult);
							}
							else if ((ChangeData->uChanged & LVIF_STATE) &&
									 (ChangeData->uOldState & LVIS_FOCUSED) == false &&
									 (ChangeData->uNewState & LVIS_FOCUSED) == false)
							{
								// after the new listview item is selected

								int NewIndex = ChangeData->iItem;
								HWND ListView = ChangeData->hdr.hwndFrom;
								DialogExtraParam* xParam = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param), BSExtraData, DialogExtraParam);
								DialogExtraLocalCopy* xLocalCopy = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param), BSExtraData, DialogExtraLocalCopy);
								TESForm* NewSelection = nullptr;

								if (xParam && xLocalCopy && NewIndex != -1 && (NewSelection = (TESForm*)TESListView::GetItemData(ListView, NewIndex)))
								{
									xParam->form = NewSelection;
									xLocalCopy->localCopy->CopyFrom(NewSelection);
									xLocalCopy->localCopy->SetFromActiveFile(true);
									xLocalCopy->localCopy->SetDataInDialog(hWnd);

									char WindowTitle[0x100] = {0};
									if (NewSelection->editorID.Size())
										FORMAT_STR(WindowTitle, "%s : %s", NewSelection->GetTypeIDString(), NewSelection->GetEditorID());
									else
										FORMAT_STR(WindowTitle, "%s : EMPTY", NewSelection->GetTypeIDString());

									SetWindowText(hWnd, WindowTitle);
									if (IsWindowEnabled(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok)) == FALSE)
									{
										EnableWindow(GetDlgItem(hWnd, TESDialog::kStandardButton_Ok), TRUE);
										InvalidateRect(hWnd, nullptr, TRUE);
									}

									SetFocus(ListView);

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

			if (Return == false && FilterableFormListManager::Instance.HandleMessages(FilterEditBox, uMsg, wParam, lParam))
			{
				TESForm* LocalCopy = TESDialog::GetDialogExtraLocalCopy(hWnd);
				if (LocalCopy)
				{
					TESFormIDListView* Item = CS_CAST(LocalCopy, TESForm, TESFormIDListView);
					Item->RefreshFormList(GetDlgItem(hWnd, kFormList_TESFormIDListView));
				}
			}

			return DlgProcResult;
		}


		LRESULT CALLBACK TESFormEditDlgSubClassProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser )
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				{
					TESFormEditData* xData = BGSEE_GETWINDOWXDATA(TESFormEditData, ExtraData);
					TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

					if (xData == nullptr)
					{
						xData = new TESFormEditData();
						ExtraData->Add(xData);

						if (WorkingCopy)		// can be NULL when creating new forms
							xData->FillBuffer(WorkingCopy);
					}

					if (WorkingCopy)
					{
						std::string Desc(WorkingCopy->GetTypeIDString());
						if (WorkingCopy->GetEditorID())
							Desc += " [" + std::string(WorkingCopy->GetEditorID()) + "]";

						std::string WndTitle = Desc;

						if (settings::general::kShowHallOfFameMembersInTitleBar().i == hallOfFame::kDisplayESMember_ObjectPreviewEdit)
						{
							hallOfFame::GetRandomESMember(WndTitle);
							WndTitle += " " + Desc;
						}
						SetWindowText(hWnd, WndTitle.c_str());
					}
				}

				break;
			case WM_DESTROY:
				{
					TESFormEditData* xData = BGSEE_GETWINDOWXDATA(TESFormEditData, ExtraData);
					if (xData)
					{
						// at this point, the working copy is already modified (if there were changes and the user confirmed them)
						TESForm* WorkingCopy = TESDialog::GetDialogExtraParam(hWnd);

						if (WorkingCopy && xData->HasChanges(WorkingCopy))
						{
							// create a proxy from the pre-filled buffer
							BGSEEUNDOSTACK->Record(new formUndoStack::FormUndoProxy(WorkingCopy, xData->Buffer));
						}

						ExtraData->Remove(TESFormEditData::kTypeID);
						delete xData;
					}

					ObjectWindowImposterManager::Instance.RefreshImposters();
				}

				break;
			}

			return DlgProcResult;
		}


		std::string GetWindowClassNameForWindowPosition(HWND hWnd, bgsee::WindowSubclasser* Subclasser)
		{
			auto Template = Subclasser->GetDialogTemplate(hWnd);

			switch (Template)
			{
				// TESBoundObject/FormEdit Dialogs
			case TESDialog::kDialogTemplate_Weapon:
			case TESDialog::kDialogTemplate_Armor:
			case TESDialog::kDialogTemplate_Clothing:
			case TESDialog::kDialogTemplate_MiscItem:
			case TESDialog::kDialogTemplate_Static:
			case TESDialog::kDialogTemplate_Reference:
			case TESDialog::kDialogTemplate_Apparatus:
			case TESDialog::kDialogTemplate_Book:
			case TESDialog::kDialogTemplate_Container:
			case TESDialog::kDialogTemplate_Activator:
			case TESDialog::kDialogTemplate_AIForm:
			case TESDialog::kDialogTemplate_Light:
			case TESDialog::kDialogTemplate_Potion:
			case TESDialog::kDialogTemplate_Enchantment:
			case TESDialog::kDialogTemplate_LeveledCreature:
			case TESDialog::kDialogTemplate_Sound:
			case TESDialog::kDialogTemplate_Door:
			case TESDialog::kDialogTemplate_LeveledItem:
			case TESDialog::kDialogTemplate_LandTexture:
			case TESDialog::kDialogTemplate_SoulGem:
			case TESDialog::kDialogTemplate_Ammo:
			case TESDialog::kDialogTemplate_Spell:
			case TESDialog::kDialogTemplate_Flora:
			case TESDialog::kDialogTemplate_Tree:
			case TESDialog::kDialogTemplate_CombatStyle:
			case TESDialog::kDialogTemplate_Water:
			case TESDialog::kDialogTemplate_NPC:
			case TESDialog::kDialogTemplate_Creature:
			case TESDialog::kDialogTemplate_Grass:
			case TESDialog::kDialogTemplate_Furniture:
			case TESDialog::kDialogTemplate_LoadingScreen:
			case TESDialog::kDialogTemplate_Ingredient:
			case TESDialog::kDialogTemplate_LeveledSpell:
			case TESDialog::kDialogTemplate_AnimObject:
			case TESDialog::kDialogTemplate_Subspace:
			case TESDialog::kDialogTemplate_EffectShader:
			case TESDialog::kDialogTemplate_SigilStone:
				// TESFormIDListView Dialogs
			case TESDialog::kDialogTemplate_Faction:
			case TESDialog::kDialogTemplate_Race:
			case TESDialog::kDialogTemplate_Class:
			case TESDialog::kDialogTemplate_Skill:
			case TESDialog::kDialogTemplate_EffectSetting:
			case TESDialog::kDialogTemplate_GameSetting:
			case TESDialog::kDialogTemplate_Globals:
			case TESDialog::kDialogTemplate_Birthsign:
			case TESDialog::kDialogTemplate_Climate:
			case TESDialog::kDialogTemplate_Worldspace:
			case TESDialog::kDialogTemplate_Hair:
			case TESDialog::kDialogTemplate_Eyes:
			case TESDialog::kDialogTemplate_Weather:
			{
				DialogExtraParam* xParam = CS_CAST(TESDialog::GetDialogExtraByType(hWnd, BSExtraData::kDialogExtra_Param),
					BSExtraData, DialogExtraParam);
				if (xParam)
					return TESForm::GetFormTypeIDLongName(xParam->formType);
			}
			break;
			// Misc Dialogs
			case TESDialog::kDialogTemplate_CellEdit:
				return "Cell Edit";
			case TESDialog::kDialogTemplate_SearchReplace:
				return "Search Replace";
			case TESDialog::kDialogTemplate_LandscapeEdit:
				return "Landscape Edit";
			case TESDialog::kDialogTemplate_FindText:
				return "Find Text";
			case TESDialog::kDialogTemplate_RegionEditor:
				return "Region Editor";
			case TESDialog::kDialogTemplate_HeightMapEditor:
				return "Height Map Editor";
			case TESDialog::kDialogTemplate_IdleAnimations:
				return "Idle Anims";
			case TESDialog::kDialogTemplate_AIPackages:
				return "AI Packages";
			case TESDialog::kDialogTemplate_TextureUse:
				return "Texture Use";
			case TESDialog::kDialogTemplate_Package:
				return "Package";
			case TESDialog::kDialogTemplate_ChooseReference:
				return "Choose Ref";
			}

			return "";
		}

		LRESULT CALLBACK WindowPosDlgSubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
												  bool& Return, bgsee::WindowExtraDataCollection* ExtraData, bgsee::WindowSubclasser* Subclasser)
		{
			LRESULT DlgProcResult = FALSE;
			Return = false;

			switch (uMsg)
			{
			case WM_INITDIALOG:
				if (settings::dialogs::kPreserveEditorDialogLocations().i)
				{
					std::string ClassName(GetWindowClassNameForWindowPosition(hWnd, Subclasser));
					if (!ClassName.empty())
					{
						RECT Bounds = { 0 };
						if (TESDialog::ReadBoundsFromINI(ClassName.c_str(), &Bounds))
							SetWindowPos(hWnd, HWND_TOP, Bounds.left, Bounds.top, 0, 0, SWP_NOSIZE);
					}
				}

				break;
			case WM_DESTROY:
				if (settings::dialogs::kPreserveEditorDialogLocations().i)
				{
					std::string ClassName(GetWindowClassNameForWindowPosition(hWnd, Subclasser));
					if (!ClassName.empty())
						TESDialog::WriteBoundsToINI(hWnd, ClassName.c_str());
				}

				break;
			}

			return DlgProcResult;
		}


		void InitializeMiscWindowOverrides()
		{
			// FormEdit subclasses
			{
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LandTexture, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Enchantment, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Spell, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Sound, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Activator, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Apparatus, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Armor, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Book, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Clothing, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Container, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Door, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Ingredient, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Light, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_MiscItem, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Static, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Grass, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Tree, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Flora, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Furniture, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Weapon, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Ammo, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_NPC, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Creature, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledCreature, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SoulGem, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Potion, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Subspace, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SigilStone, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledItem, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Package, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CombatStyle, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LoadingScreen, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledSpell, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AnimObject, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Water, TESFormEditDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_EffectShader, TESFormEditDlgSubClassProc);
			}

			// FormIDListView subclasses
			{
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Faction, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Race, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Class, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Skill, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_EffectSetting, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_GameSetting, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Globals, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Birthsign, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Climate, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Worldspace, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Hair, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Eyes, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Weather, TESFormIDListViewDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Quest, TESFormIDListViewDlgSubClassProc);
			}

			// Generic extra fittings subclasses
			{
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CellEdit, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Data, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SearchReplace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LandscapeEdit, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_FindText, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_IdleAnimations, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AIPackages, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_FilteredDialog, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Weapon, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Armor, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Clothing, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_MiscItem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Static, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Reference, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Apparatus, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Book, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Container, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Activator, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AIForm, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Light, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Potion, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Enchantment, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledCreature, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Sound, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Door, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledItem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LandTexture, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SoulGem, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Ammo, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Spell, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Flora, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Tree, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CombatStyle, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Water, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_NPC, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Creature, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Grass, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Furniture, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LoadingScreen, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Ingredient, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledSpell, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AnimObject, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Subspace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_EffectShader, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SigilStone, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Faction, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Race, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Class, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Skill, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_EffectSetting, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_GameSetting, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Globals, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Birthsign, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Climate, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Worldspace, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Hair, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Quest, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Eyes, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Weather, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SelectTopic, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SelectQuests, CommonDialogExtraFittingsSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_UseReport, CommonDialogExtraFittingsSubClassProc);
			}

			// Window position-saver subclasses
			{
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LandTexture, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Enchantment, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Spell, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Sound, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Activator, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Apparatus, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Armor, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Book, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Clothing, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Container, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Door, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Ingredient, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Light, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_MiscItem, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Static, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Grass, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Tree, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Flora, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Furniture, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Weapon, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Ammo, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_NPC, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Creature, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledCreature, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SoulGem, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Potion, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Subspace, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SigilStone, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledItem, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Package, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CombatStyle, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LoadingScreen, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LeveledSpell, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AnimObject, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Water, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_EffectShader, WindowPosDlgSubClassProc);

				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Faction, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Race, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Class, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Skill, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_EffectSetting, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_GameSetting, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Globals, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Birthsign, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Climate, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Worldspace, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Hair, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Eyes, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Weather, WindowPosDlgSubClassProc);

				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CellEdit, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_SearchReplace, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_LandscapeEdit, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_FindText, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_RegionEditor, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_HeightMapEditor, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_IdleAnimations, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_AIPackages, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_TextureUse, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_Package, WindowPosDlgSubClassProc);
				BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_ChooseReference, WindowPosDlgSubClassProc);
			}

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