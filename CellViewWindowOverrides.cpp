#include "CellViewWindowOverrides.h"
#include "MiscWindowOverrides.h"
#include "Construction Set Extender_Resource.h"
#include "Render Window\RenderWindowManager.h"
#include "UIManager.h"

namespace cse
{
	namespace uiManager
	{
		CellViewExtraData::~CellViewExtraData()
		{
			;//
		}

		CellViewExtraData::CellViewExtraData() :
			bgsee::WindowExtraData(kTypeID),
			RefFilterEditBox(),
			RefFilterLabel(),
			XLabel(),
			YLabel(),
			XEdit(),
			YEdit(),
			GoBtn(),
			CellFilterEditBox(),
			CellFilterLabel(),
			VisibleOnlyCheckBox(),
			SelectionOnlyCheckBox(),
			RefreshRefListBtn()
		{
			;//
		}

		int CALLBACK CellViewExtraData::CustomFormListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
		{
			int Result = 0;

			TESObjectREFR* FormA = (TESObjectREFR*)lParam1;
			TESObjectREFR* FormB = (TESObjectREFR*)lParam2;

			if (FormA && FormB)
			{
				switch ((UInt32)abs(lParamSort))
				{
				case kExtraRefListColumn_Persistent:
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
				case kExtraRefListColumn_Disabled:
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
				case kExtraRefListColumn_VWD:
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
				case kExtraRefListColumn_EnableParent:
					{
						BSExtraData* AxData = FormA->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
						BSExtraData* BxData = FormB->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);

						if (AxData && BxData == nullptr)
							Result = -1;
						else if (AxData == nullptr && BxData)
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
				case kExtraRefListColumn_Count:
					{
						BSExtraData* AxData = FormA->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);
						BSExtraData* BxData = FormB->extraData.GetExtraDataByType(BSExtraData::kExtra_Count);

						if (AxData && BxData == nullptr)
							Result = -1;
						else if (AxData == nullptr && BxData)
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

				Result = FormEnumerationManager::Instance.CompareActiveForms(FormA, FormB, Result);
			}

			return Result;
		}

		bool CellViewExtraData::RefListFilter(TESForm* Form)
		{
			TESObjectREFR* Ref = (TESObjectREFR*)Form;

			bool VisibleOnly = IsDlgButtonChecked(*TESCellViewWindow::WindowHandle, IDC_CSE_CELLVIEW_VISIBLEONLYBTN) == BST_CHECKED;
			bool SelectionOnly = IsDlgButtonChecked(*TESCellViewWindow::WindowHandle, IDC_CSE_CELLVIEW_SELECTEDONLYBTN) == BST_CHECKED;

			if (VisibleOnly == false && SelectionOnly == false)
				return true;
			else if (VisibleOnly && (renderWindow::ReferenceVisibilityManager::ShouldBeInvisible(Ref) || renderWindow::ReferenceVisibilityManager::IsCulled(Ref)))
				return false;
			else if (SelectionOnly && _RENDERSEL->HasObject(Form) == false)
				return false;
			else
				return true;
		}

#define ID_CELLVIEW_HOUSEKEEPINGTIMERID							(WM_USER + 6500)


		LRESULT CALLBACK CellViewWindowSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
													bgsee::WindowSubclassProcCollection::SubclassProcExtraParams* SubclassParams)
		{
			LRESULT DlgProcResult = FALSE;

			HWND WorldspaceLabel = GetDlgItem(hWnd, TESCellViewWindow::kWorldspaceLabel);
			HWND WorldspaceCombo = GetDlgItem(hWnd, TESCellViewWindow::kWorldspaceComboBox);
			HWND CellLabel = GetDlgItem(hWnd, TESCellViewWindow::kCellLabel);
			HWND CellList = GetDlgItem(hWnd, TESCellViewWindow::kCellListView);
			HWND RefList = GetDlgItem(hWnd, TESCellViewWindow::kObjectRefListView);

			HWND RefFilterEditBox = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTEREDIT);
			HWND RefFilterLabel = GetDlgItem(hWnd, IDC_CSEFILTERABLEFORMLIST_FILTERLBL);
			HWND XLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_XLBL);
			HWND YLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_YLBL);
			HWND XEdit = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_XEDIT);
			HWND YEdit = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_YEDIT);
			HWND GoBtn = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_GOBTN);
			HWND CellFilterEditBox = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_CELLFILTEREDIT);
			HWND CellFilterLabel = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_CELLFILTERLBL);
			int* RefListSortColumn = TESCellViewWindow::ObjectListSortColumn;
			HWND VisibleOnlyCheckbox = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_VISIBLEONLYBTN);
			HWND SelectionOnlyCheckbox = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_SELECTEDONLYBTN);
			HWND RefreshRefListBtn = GetDlgItem(hWnd, IDC_CSE_CELLVIEW_REFRESHREFSBTN);

			switch (uMsg)
			{
			case TESCellViewWindow::kWindowMessage_ReloadObjects:
				// update the cell list's selection too to keep it synchronised
				SubclassParams->Out.MarkMessageAsHandled = true;
				DlgProcResult = TRUE;

				SubclassParams->In.Subclasser->TunnelMessageToOrgWndProc(hWnd, uMsg, wParam, lParam, true);

				if (*TESCellViewWindow::CurrentCellSelection)
				{
					int Index = TESListView::GetItemByData(*TESCellViewWindow::CellListHandle, *TESCellViewWindow::CurrentCellSelection);
					if (Index != -1)
						ListView_EnsureVisible(*TESCellViewWindow::CellListHandle, Index, FALSE);
				}

				break;
			case WM_ACTIVATE:
				if (LOWORD(wParam) != WA_INACTIVE)
				{
					// refresh the ref list if the visible only/selection only filter is checked
					if (IsDlgButtonChecked(hWnd, IDC_CSE_CELLVIEW_VISIBLEONLYBTN) == BST_CHECKED ||
						IsDlgButtonChecked(hWnd, IDC_CSE_CELLVIEW_SELECTEDONLYBTN) == BST_CHECKED)
					{
						TESCellViewWindow::RefreshObjectList();
					}
				}

				break;
			case WM_CLOSE:
				SendMessage(*TESCSMain::WindowHandle, WM_COMMAND, TESCSMain::kMainMenu_View_CellViewWindow, NULL);
				SubclassParams->Out.MarkMessageAsHandled = true;

				break;
			case WM_DESTROY:
				{
					CellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CellViewExtraData, SubclassParams->In.ExtraData);

					if (xData)
					{
						SubclassParams->In.ExtraData->Remove(CellViewExtraData::kTypeID);
						delete xData;
					}

					KillTimer(hWnd, ID_CELLVIEW_HOUSEKEEPINGTIMERID);
				}
			case TESDialog::kWindowMessage_Destroy:
				FilterableFormListManager::Instance.Unregister(RefFilterEditBox);
				FilterableFormListManager::Instance.Unregister(CellFilterEditBox);

				break;
			case WM_INITDIALOG:
				{
					CellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CellViewExtraData, SubclassParams->In.ExtraData);
					if (xData == nullptr)
					{
						xData = new CellViewExtraData();
						SubclassParams->In.ExtraData->Add(xData);

						RECT Bounds = { 0 };

						SME::UIHelpers::GetClientRectInitBounds(RefFilterEditBox, hWnd, &xData->RefFilterEditBox);
						SME::UIHelpers::GetClientRectInitBounds(RefFilterLabel, hWnd, &xData->RefFilterLabel);
						SME::UIHelpers::GetClientRectInitBounds(XLabel, hWnd, &xData->XLabel);
						SME::UIHelpers::GetClientRectInitBounds(YLabel, hWnd, &xData->YLabel);
						SME::UIHelpers::GetClientRectInitBounds(XEdit, hWnd, &xData->XEdit);
						SME::UIHelpers::GetClientRectInitBounds(YEdit, hWnd, &xData->YEdit);
						SME::UIHelpers::GetClientRectInitBounds(GoBtn, hWnd, &xData->GoBtn);
						SME::UIHelpers::GetClientRectInitBounds(CellFilterEditBox, hWnd, &xData->CellFilterEditBox);
						SME::UIHelpers::GetClientRectInitBounds(CellFilterLabel, hWnd, &xData->CellFilterLabel);
						SME::UIHelpers::GetClientRectInitBounds(VisibleOnlyCheckbox, hWnd, &xData->VisibleOnlyCheckBox);
						SME::UIHelpers::GetClientRectInitBounds(SelectionOnlyCheckbox, hWnd, &xData->SelectionOnlyCheckBox);
						SME::UIHelpers::GetClientRectInitBounds(RefreshRefListBtn, hWnd, &xData->RefreshRefListBtn);

						TESDialog::ReadBoundsFromINI("Cell View", &Bounds);
						SetWindowPos(hWnd, NULL, Bounds.left, Bounds.top, Bounds.right, Bounds.bottom, SWP_NOZORDER);
					}

					FilterableFormListManager::Instance.Register(RefFilterEditBox, RefFilterLabel, RefList, hWnd, CellViewExtraData::RefListFilter);
					FilterableFormListManager::Instance.Register(CellFilterEditBox, CellFilterLabel, CellList, hWnd);

					LVCOLUMN ColumnData = { 0 };
					ColumnData.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVCF_FMT;
					ColumnData.fmt = LVCFMT_CENTER;

					ColumnData.cx = 45;
					ColumnData.pszText = "Persistent";
					ColumnData.iSubItem = CellViewExtraData::kExtraRefListColumn_Persistent;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 45;
					ColumnData.pszText = "Initially Disabled";
					ColumnData.iSubItem = CellViewExtraData::kExtraRefListColumn_Disabled;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 45;
					ColumnData.pszText = "VWD";
					ColumnData.iSubItem = CellViewExtraData::kExtraRefListColumn_VWD;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 100;
					ColumnData.pszText = "Enable Parent";
					ColumnData.iSubItem = CellViewExtraData::kExtraRefListColumn_EnableParent;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					ColumnData.cx = 40;
					ColumnData.pszText = "Count";
					ColumnData.iSubItem = CellViewExtraData::kExtraRefListColumn_Count;
					ListView_InsertColumn(RefList, ColumnData.iSubItem, &ColumnData);

					SetTimer(hWnd, ID_CELLVIEW_HOUSEKEEPINGTIMERID, 500, nullptr);
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case ID_CELLVIEW_HOUSEKEEPINGTIMERID:
					// prevent activation of the ref list when editing landscape or pathgrids
					if (*TESRenderWindow::LandscapeEditFlag || *TESRenderWindow::PathGridEditFlag)
					{
						if (IsWindowEnabled(RefList))
							EnableWindow(RefList, FALSE);
					}
					else
					{
						if (!IsWindowEnabled(RefList))
							EnableWindow(RefList, TRUE);
					}

					DlgProcResult = TRUE;
					SubclassParams->Out.MarkMessageAsHandled = true;

					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;
					switch (NotificationData->code)
					{
					case NM_CUSTOMDRAW:
						if (wParam == TESCellViewWindow::kCellListView)
						{
							NMLVCUSTOMDRAW* DrawData = (NMLVCUSTOMDRAW*)lParam;

							switch (DrawData->nmcd.dwDrawStage)
							{
							case CDDS_PREPAINT:
							{
								SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NOTIFYITEMDRAW);
								DlgProcResult = TRUE;
								SubclassParams->Out.MarkMessageAsHandled = true;
							}

							break;
							case CDDS_ITEMPREPAINT:
							{
								if (wParam == TESCellViewWindow::kCellListView)
								{
									auto Cell = reinterpret_cast<TESObjectCELL*>(DrawData->nmcd.lItemlParam);
									if (Cell)
									{
										bool Update = true;
										if (_TES->currentInteriorCell == Cell || *TESRenderWindow::ActiveCell == Cell)
										{
											DrawData->clrText = SME::StringHelpers::GetRGB(settings::dialogs::kCellViewActiveCellForeColor.GetData().s);
											DrawData->clrTextBk = SME::StringHelpers::GetRGB(settings::dialogs::kCellViewActiveCellBackColor.GetData().s);
										}
										else if (_TES->gridCellArray->IsCellInGrid(Cell))
										{
											DrawData->clrText = SME::StringHelpers::GetRGB(settings::dialogs::kCellViewLoadedGridCellForeColor.GetData().s);
											DrawData->clrTextBk = SME::StringHelpers::GetRGB(settings::dialogs::kCellViewLoadedGridCellBackColor.GetData().s);
										}
										else
											Update = false;


										if (Update)
										{
											SetWindowLongPtr(hWnd, DWL_MSGRESULT, CDRF_NEWFONT);
											DlgProcResult = TRUE;
											SubclassParams->Out.MarkMessageAsHandled = true;
										}
									}
								}
							}

							break;
							}
						}

						break;
					case LVN_ITEMCHANGED:
						if (NotificationData->hwndFrom == CellList)
						{
							NMITEMACTIVATE* ActivateData = reinterpret_cast<NMITEMACTIVATE*>(lParam);
							if (ActivateData->iItem >= 0 && (ActivateData->uNewState & 2))
							{
								SubclassParams->Out.MarkMessageAsHandled = true;
								DlgProcResult = TRUE;

								TESCellViewWindow::OnSelectCellListItem(reinterpret_cast<TESObjectCELL*>(ActivateData->lParam), true);
							}
						}


						break;
					case LVN_GETDISPINFO:
						if (NotificationData->hwndFrom == RefList)
						{
							NMLVDISPINFO* DisplayData = (NMLVDISPINFO*)lParam;

							if ((DisplayData->item.mask & LVIF_TEXT) && DisplayData->item.lParam)
							{
								DlgProcResult = TRUE;
								SubclassParams->Out.MarkMessageAsHandled = true;

								TESObjectREFR* Current = (TESObjectREFR*)DisplayData->item.lParam;

								switch (DisplayData->item.iSubItem)
								{
								case CellViewExtraData::kExtraRefListColumn_Persistent:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
										((Current->formFlags & TESForm::kFormFlags_QuestItem) ? "Y" : ""));

									break;
								case CellViewExtraData::kExtraRefListColumn_Disabled:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
										((Current->formFlags & TESForm::kFormFlags_Disabled) ? "Y" : ""));

									break;
								case CellViewExtraData::kExtraRefListColumn_VWD:
									sprintf_s(DisplayData->item.pszText, DisplayData->item.cchTextMax, "%s",
										((Current->formFlags & TESForm::kFormFlags_VisibleWhenDistant) ? "Y" : ""));

									break;
								case CellViewExtraData::kExtraRefListColumn_EnableParent:
									{
										BSExtraData* xData = Current->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
										if (xData)
										{
											ExtraEnableStateParent* xParent = CS_CAST(xData, BSExtraData, ExtraEnableStateParent);
											SME_ASSERT(xParent->parent);

											if (xParent->parent->editorID.c_str() == nullptr)
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
								case CellViewExtraData::kExtraRefListColumn_Count:
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
									SubclassParams->Out.MarkMessageAsHandled = false;
								}
							}
						}

						break;
					case LVN_COLUMNCLICK:
						if (NotificationData->hwndFrom == RefList)
						{
							NMLISTVIEW* ListViewData = (NMLISTVIEW*)lParam;
							if (ListViewData->iSubItem >= CellViewExtraData::kExtraRefListColumn_Persistent)
							{
								if (*RefListSortColumn > 0)
								{
									*RefListSortColumn = ListViewData->iSubItem;
									*RefListSortColumn *= -1;
								}
								else
									*RefListSortColumn = ListViewData->iSubItem;

								SendMessage(RefList, LVM_SORTITEMS, *RefListSortColumn, (LPARAM)CellViewExtraData::CustomFormListComparator);

								DlgProcResult = TRUE;
								SubclassParams->Out.MarkMessageAsHandled = true;
							}
						}

						break;
					}
				}

				break;
			case WM_WINDOWPOSCHANGING:
				{
					static constexpr auto kMinWidth = 630;
					static constexpr auto kMinHeight = 330;

					auto WindowPosParams = reinterpret_cast<WINDOWPOS*>(lParam);
					if (WindowPosParams->cx < kMinWidth)
						WindowPosParams->cx = kMinWidth;

					if (WindowPosParams->cy < kMinHeight)
						WindowPosParams->cy = kMinHeight;
				}

				break;
			case WM_SIZE:
				if (wParam != SIZE_MINIMIZED)
				{
					RECT CurrentRect = { 0 };

					RECT* BaseDlgRect = (RECT*)0x00A0AA38;
					RECT* BaseCellListRect = (RECT*)0x00A0AA48;
					RECT* BaseRefListRect = (RECT*)0x00A0AA04;
					RECT* BaseWorldspaceComboRect = (RECT*)0x00A0AA24;
					RECT* BaseWorldspaceLabelRect = (RECT*)0x00A0AA14;
					RECT* BaseCellNameLabelRect = (RECT*)0x00A0A9F0;

					SetRect(&CurrentRect, 0, 0, LOWORD(lParam), HIWORD(lParam));
					int DeltaDlgWidth = (CurrentRect.right - BaseDlgRect->right) >> 1;
					HDWP DeferPosData = BeginDeferWindowPos(3);

					if (DeferPosData)
					{
						DeferPosData = DeferWindowPos(DeferPosData, CellList, 0,
							0, 0,
							DeltaDlgWidth + BaseCellListRect->right, CurrentRect.bottom + BaseCellListRect->bottom - BaseDlgRect->bottom,
							SWP_NOMOVE);
					}

					if (DeferPosData)
					{
						DeferPosData = DeferWindowPos(DeferPosData, WorldspaceCombo, 0,
							0, 0,
							BaseCellListRect->right + DeltaDlgWidth - BaseWorldspaceLabelRect->right, BaseWorldspaceComboRect->bottom,
							SWP_NOMOVE);
					}

					if (DeferPosData)
					{
						DeferPosData = DeferWindowPos(DeferPosData, RefList, 0,
							DeltaDlgWidth + BaseRefListRect->left, BaseRefListRect->top,
							DeltaDlgWidth + BaseRefListRect->right, CurrentRect.bottom + BaseRefListRect->bottom - BaseDlgRect->bottom,
							NULL);
					}

					if (DeferPosData)
					{
						DeferPosData = DeferWindowPos(DeferPosData, CellLabel, 0,
							BaseCellNameLabelRect->left + DeltaDlgWidth, BaseCellNameLabelRect->top,
							DeltaDlgWidth + BaseCellNameLabelRect->right, BaseCellNameLabelRect->bottom,
							NULL);
					}

					InvalidateRect(CellLabel, nullptr, TRUE);
					InvalidateRect(WorldspaceLabel, nullptr, TRUE);

					if (DeferPosData)
						EndDeferWindowPos(DeferPosData);

					CellViewExtraData* xData = BGSEE_GETWINDOWXDATA(CellViewExtraData, SubclassParams->In.ExtraData);

					if (xData)
					{
						DeferPosData = BeginDeferWindowPos(8);

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, XLabel, 0,
								DeltaDlgWidth + xData->XLabel.left, xData->XLabel.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, XEdit, 0,
								DeltaDlgWidth + xData->XEdit.left, xData->XEdit.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, YLabel, 0,
								DeltaDlgWidth + xData->YLabel.left, xData->YLabel.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, YEdit, 0,
								DeltaDlgWidth + xData->YEdit.left, xData->YEdit.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, GoBtn, 0,
								DeltaDlgWidth + xData->GoBtn.left, xData->GoBtn.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, RefFilterLabel, 0,
								DeltaDlgWidth + xData->RefFilterLabel.left, xData->RefFilterLabel.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, RefFilterEditBox, 0,
								DeltaDlgWidth + xData->RefFilterEditBox.left, xData->RefFilterEditBox.top,
								DeltaDlgWidth + xData->RefFilterEditBox.right, xData->RefFilterEditBox.bottom + 2,
								NULL);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, VisibleOnlyCheckbox, 0,
								DeltaDlgWidth + xData->VisibleOnlyCheckBox.left, xData->VisibleOnlyCheckBox.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, SelectionOnlyCheckbox, 0,
								DeltaDlgWidth + xData->SelectionOnlyCheckBox.left, xData->SelectionOnlyCheckBox.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, RefreshRefListBtn, 0,
								DeltaDlgWidth + xData->RefreshRefListBtn.left, xData->RefreshRefListBtn.top,
								0, 0,
								SWP_NOSIZE);
						}

						if (DeferPosData)
						{
							DeferPosData = DeferWindowPos(DeferPosData, CellFilterEditBox, 0,
								0, 0,
								BaseCellListRect->right + DeltaDlgWidth - xData->CellFilterLabel.right - 7, xData->CellFilterEditBox.bottom,
								SWP_NOMOVE);
						}

						InvalidateRect(XLabel, nullptr, TRUE);
						InvalidateRect(YLabel, nullptr, TRUE);
						InvalidateRect(RefFilterLabel, nullptr, TRUE);

						if (DeferPosData)
							EndDeferWindowPos(DeferPosData);
					}

					SubclassParams->Out.MarkMessageAsHandled = true;
				}

				break;
			case WM_COMMAND:
				switch (LOWORD(wParam))
				{
				case TESCellViewWindow::kWorldspaceComboBox:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						// override the default handler to use a common code path to update the cell list
						SubclassParams->Out.MarkMessageAsHandled = true;
						DlgProcResult = TRUE;

						TESCellViewWindow::RefreshCellList();
					}

					break;
				case IDC_CSE_CELLVIEW_GOBTN:
					{
						char XCoord[4] = { 0 }, YCoord[4] = { 0 };
						GetWindowText(XEdit, (LPSTR)XCoord, sizeof(XCoord));
						GetWindowText(YEdit, (LPSTR)YCoord, sizeof(YCoord));

						if (strlen(XCoord) && strlen(YCoord) && _TES->currentInteriorCell == nullptr)
						{
							auto X = atoi(XCoord), Y = atoi(YCoord);
							if (X > 32767 || Y > 32767 || X < -32768 || Y < -32768)
							{
								BGSEEUI->MsgBoxE(hWnd, MB_OK, "Cell coordinates are out-of-bounds!");
								break;
							}

							Vector3 Coords((X << 12) + 2048.0, (Y << 12) + 2048.0, 0);
							_TES->LoadCellIntoViewPort(&Coords, nullptr);
						}
					}

					break;
				case IDC_CSE_CELLVIEW_VISIBLEONLYBTN:
				case IDC_CSE_CELLVIEW_SELECTEDONLYBTN:
				case IDC_CSE_CELLVIEW_REFRESHREFSBTN:
					TESCellViewWindow::RefreshObjectList();

					break;
				}
			}

			if (SubclassParams->Out.MarkMessageAsHandled == false)
			{
				if (FilterableFormListManager::Instance.HandleMessages(RefFilterEditBox, uMsg, wParam, lParam))
					TESCellViewWindow::RefreshObjectList();

				if (FilterableFormListManager::Instance.HandleMessages(CellFilterEditBox, uMsg, wParam, lParam))
					TESCellViewWindow::RefreshCellList();
			}

			return DlgProcResult;
		}

		void InitializeCellViewWindowOverrides()
		{
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CellView,
				uiManager::CellViewWindowSubclassProc, 99);
			BGSEEUI->GetSubclasser()->RegisterSubclassForDialogResourceTemplate(TESDialog::kDialogTemplate_CellView,
				uiManager::CommonDialogExtraFittingsSubClassProc, 1);

			if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
			{
				bgsee::WindowStyler::StyleData RegularAppWindow = { 0 };
				RegularAppWindow.Extended = WS_EX_APPWINDOW;
				RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

				BGSEEUI->GetWindowStyler()->RegisterStyle(TESDialog::kDialogTemplate_CellView, RegularAppWindow);
			}
		}

	}
}