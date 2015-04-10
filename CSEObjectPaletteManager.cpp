#include "CSEObjectPaletteManager.h"
#include "Construction Set Extender_Resource.h"
#include "CSEMain.h"
#include "CSEAchievements.h"

namespace ConstructionSetExtender
{
	namespace ObjectPalette
	{
		static const char* kFieldMarkers[] =
		{
			"NAME",
			"BASEFORM",

			"ANGLE_BASE_X",
			"ANGLE_BASE_Y",
			"ANGLE_BASE_Z",

			"ANGLE_OFFSET_X",
			"ANGLE_OFFSET_Y",
			"ANGLE_OFFSET_Z",

			"SINK_BASE",
			"SINK_OFFSET",

			"SCALE_BASE",
			"SCALE_OFFSET",

			"CONFORM_SLOPE"
		};

		enum
		{
			kField_Name,
			kField_BaseForm,

			kField_AngleBase_X,
			kField_AngleBase_Y,
			kField_AngleBase_Z,

			kField_AngleOffset_X,
			kField_AngleOffset_Y,
			kField_AngleOffset_Z,

			kField_SinkBase,
			kField_SinkOffset,

			kField_ScaleBase,
			kField_ScaleOffset,

			kField_ConformToSlope,

			kField__MAX
		};

		void PaletteObject::Serialize(SerializedData& OutData) const
		{
			OutData = "";

			OutData += kFieldMarkers[kField_Name] + SerializedData("=") + Name;
			OutData += "\n";
			SME_ASSERT(BaseObject->GetEditorID());
			OutData += kFieldMarkers[kField_BaseForm] + SerializedData("=") + BaseObject->GetEditorID();
			OutData += "\n";

			char Format[0x50] = { 0 };

			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_AngleBase_X], AngleBase.x);
			OutData += Format;
			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_AngleBase_Y], AngleBase.y);
			OutData += Format;
			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_AngleBase_Z], AngleBase.z);
			OutData += Format;

			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_AngleOffset_X], AngleOffset.x);
			OutData += Format;
			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_AngleOffset_Y], AngleOffset.y);
			OutData += Format;
			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_AngleOffset_Z], AngleOffset.z);
			OutData += Format;

			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_SinkBase], Sink.x);
			OutData += Format;
			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_SinkOffset], Sink.y);
			OutData += Format;

			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_ScaleBase], Scale.x);
			OutData += Format;
			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_ScaleOffset], Scale.y);
			OutData += Format;

			FORMAT_STR(Format, "%s=%f\n", kFieldMarkers[kField_ConformToSlope], float(ConformToSlope));
			OutData += Format;
		}

		void PaletteObject::Deserialize(const SerializedData& Data)
		{
			Reset();

			SME::StringHelpers::Tokenizer LineParser(Data.c_str(), "\n\0");
			SerializedData Line, Field;

			while (LineParser.NextToken(Line) != -1)
			{
				SME::StringHelpers::Tokenizer FieldParser(Line.c_str(), "=");
				if (FieldParser.NextToken(Field) != -1)
				{
					for (int i = kField_Name; i < kField__MAX; i++)
					{
						if (!_stricmp(kFieldMarkers[i], Field.c_str()))
						{
							if (FieldParser.NextToken(Field) != -1)
							{
								float FloatData = 0.f;
								sscanf_s(Field.c_str(), "%f", &FloatData);

								switch (i)
								{
								case kField_Name:
									Name = Field;
									break;
								case kField_BaseForm:
									BaseObject = TESForm::LookupByEditorID(Field.c_str());
									break;
								case kField_AngleBase_X:
									AngleBase.x = FloatData;
									break;
								case kField_AngleBase_Y:
									AngleBase.y = FloatData;
									break;
								case kField_AngleBase_Z:
									AngleBase.z = FloatData;
									break;
								case kField_AngleOffset_X:
									AngleOffset.x = FloatData;
									break;
								case kField_AngleOffset_Y:
									AngleOffset.y = FloatData;
									break;
								case kField_AngleOffset_Z:
									AngleOffset.z = FloatData;
									break;
								case kField_SinkBase:
									Sink.x = FloatData;
									break;
								case kField_SinkOffset:
									Sink.y = FloatData;
									break;
								case kField_ScaleBase:
									if (FloatData == 0)
										FloatData = 1;

									Scale.x = FloatData;
									break;
								case kField_ScaleOffset:
									Scale.y = FloatData;
									break;
								case kField_ConformToSlope:
									ConformToSlope = FloatData;
									break;
								}
							}
						}
					}
				}
			}
		}

		void PaletteObject::GetFromDialog(HWND Dialog)
		{
			char Buffer[0x100] = { 0 };

			GetDlgItemText(Dialog, IDC_OBJECTPALETTE_NAME, Buffer, sizeof(Buffer));
			Name = Buffer;

			TESForm* Base = (TESForm*)TESComboBox::GetSelectedItemData(GetDlgItem(Dialog, IDC_OBJECTPALETTE_BASEOBJECT));
			SME_ASSERT(Base);
			BaseObject = Base;

			AngleBase.x = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEBASE_X);
			AngleBase.y = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEBASE_Y);
			AngleBase.z = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEBASE_Z);

			AngleOffset.x = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEOFFSET_X);
			AngleOffset.y = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEOFFSET_Y);
			AngleOffset.z = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEOFFSET_Z);

			Sink.x = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SINKBASE);
			Sink.y = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SINKOFFSET);

			Scale.x = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SCALEBASE);
			Scale.y = TESDialog::GetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SCALEOFFSET);

			if (Scale.x == 0)
				Scale.x = 1;

			if (IsDlgButtonChecked(Dialog, IDC_OBJECTPALETTE_CONFORMSLOPE))
				ConformToSlope = true;
			else
				ConformToSlope = false;
		}

		void PaletteObject::SetInDialog(HWND Dialog) const
		{
			SetDlgItemText(Dialog, IDC_OBJECTPALETTE_NAME, Name.c_str());

			TESComboBox::SetSelectedItemByData(GetDlgItem(Dialog, IDC_OBJECTPALETTE_BASEOBJECT), BaseObject);

			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEBASE_X, AngleBase.x, 2);
			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEBASE_Y, AngleBase.y, 2);
			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEBASE_Z, AngleBase.z, 2);

			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEOFFSET_X, AngleOffset.x, 2);
			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEOFFSET_Y, AngleOffset.y, 2);
			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_ANGLEOFFSET_Z, AngleOffset.z, 2);

			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SINKBASE, Sink.x, 2);
			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SINKOFFSET, Sink.y, 2);

			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SCALEBASE, Scale.x, 2);
			TESDialog::SetDlgItemFloat(Dialog, IDC_OBJECTPALETTE_SCALEOFFSET, Scale.y, 2);

			if (ConformToSlope)
				CheckDlgButton(Dialog, IDC_OBJECTPALETTE_CONFORMSLOPE, BST_CHECKED);
			else
				CheckDlgButton(Dialog, IDC_OBJECTPALETTE_CONFORMSLOPE, BST_UNCHECKED);
		}

		void PaletteObject::Reset()
		{
			Name = "";
			BaseObject = NULL;
			AngleBase.x = AngleBase.y = AngleBase.z = 0;
			AngleOffset.x = AngleOffset.y = AngleOffset.z = 0;
			Sink.x = Sink.y = 0;
			Scale.x = 1; Scale.y = 0;
			ConformToSlope = false;
		}

		PaletteObject::PaletteObject()
		{
			Reset();
		}

		PaletteObject::PaletteObject(const SerializedData& Data)
		{
			Deserialize(Data);
		}

		PaletteObject::PaletteObject(TESForm* Base)
		{
			SME_ASSERT(Base);

			Reset();
			BaseObject = Base;
		}

		TESObjectREFR* PaletteObject::Instantiate(const Vector3& Position, bool MarkAsTemporary) const
		{
			SME_ASSERT(BaseObject);

			TESObjectREFR* NewRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
			if (MarkAsTemporary)
				NewRef->MarkAsTemporary();

			NewRef->SetBaseForm(BaseObject);

			Vector3 Pos(Position);
			SME::MersenneTwister::init_genrand(GetTickCount());
			Pos.z += Sink.x + (-Sink.y + SME::MersenneTwister::genrand_real2() * 2.f * Sink.y);
			NewRef->SetPosition(Pos);

			SME::MersenneTwister::init_genrand(GetTickCount());
			Vector3 Rot(AngleBase);
			Vector3 Offset(-AngleOffset.x + SME::MersenneTwister::genrand_real2() * 2.f * AngleOffset.x,
						   -AngleOffset.y + SME::MersenneTwister::genrand_real2() * 2.f * AngleOffset.y,
						   -AngleOffset.z + SME::MersenneTwister::genrand_real2() * 2.f * AngleOffset.z);
			Rot += Offset;
			NewRef->SetRotation(Rot, false);

			SME::MersenneTwister::init_genrand(GetTickCount());
			NewRef->SetScale(Scale.x + (-Scale.y + SME::MersenneTwister::genrand_real2() * 2.f * Scale.y));

			return NewRef;
		}

		PaletteObject::~PaletteObject()
		{
			BaseObject = NULL;
		}

		CSEObjectPaletteManager							CSEObjectPaletteManager::Instance;
		const char*										CSEObjectPaletteManager::kPaletteFileExtension = "cseopal";
		const BGSEditorExtender::BGSEEResourceLocation	CSEObjectPaletteManager::kRepositoryPath(CSE_OPALDEPOT);

#define IDC_OBJECTPALETTE_FILTERINPUTTIMERID		0x200
#define IDC_OBJECTPALETTE_PREVIEWTIMERID			0x201
#define IDC_OBJECTPALETTE_LISTSTATETIMERID			0x202

		INT_PTR CALLBACK CSEObjectPaletteManager::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			HWND FilterBox = GetDlgItem(hWnd, IDC_FILTERBOX);
			HWND LoadedObjectsList = GetDlgItem(hWnd, IDC_PALETTEOBJECTS_LOADED);
			HWND SelectedObjectsList = GetDlgItem(hWnd, IDC_PALETTEOBJECTS_SELECTION);
			LRESULT Result = FALSE;

			switch (uMsg)
			{
			case TESDialog::kWindowMessage_HandleDragDrop:
				Instance.HandleDragDrop();
				Result = TRUE;
				break;
			case WM_COMMAND:
				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == FilterBox)
					Instance.TimeCounter = 0;

				switch (wParam)
				{
				case IDC_OBJECTPALETTE_APPLYPREVIEW:
					if (Instance.ActiveObject)
						Instance.ActiveObject->GetFromDialog(hWnd);

					Instance.UpdatePreview();
					break;
				case IDC_OBJECTPALETTE_NEW:
					Instance.NewPalette();
					break;
				case IDC_OBJECTPALETTE_SAVE:
					Instance.SavePalette(false);
					break;
				case IDC_OBJECTPALETTE_SAVEAS:
					Instance.SavePalette(true);
					break;
				case IDC_OBJECTPALETTE_LOAD:
					Instance.LoadPalette();
					break;
				case IDC_OBJECTPALETTE_MERGE:
					Instance.MergePalette();
					break;
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case IDC_OBJECTPALETTE_FILTERINPUTTIMERID:
					Result = TRUE;
					if (Instance.TimeCounter && Instance.TimeCounter != -1 && Instance.TimeCounter >= 500 * 1.5)
					{
						Instance.RefreshObjectList();
						Instance.TimeCounter = -1;
					}
					else if (Instance.TimeCounter != -1)
						Instance.TimeCounter += 500;

					break;
				case IDC_OBJECTPALETTE_PREVIEWTIMERID:
					Result = TRUE;
					if (Instance.PreviewRef && Instance.PreviewRef->GetNiNode())
						Instance.Renderer->Present(0.0);

					break;
				case IDC_OBJECTPALETTE_LISTSTATETIMERID:
					Result = TRUE;

					if (TESListView::GetSelectedItemCount(LoadedObjectsList) == 0)
					{
						if (Instance.ActiveObject)
							Instance.ActiveObject->GetFromDialog(hWnd);

						Instance.ActiveObject = NULL;
						Instance.EnableControls(false);
						Instance.UpdateSelectionList();
						Instance.UpdatePreview();
					}

					KillTimer(hWnd, IDC_OBJECTPALETTE_LISTSTATETIMERID);
					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					switch (NotificationData->code)
					{
					case LVN_COLUMNCLICK:
						if (NotificationData->hwndFrom == LoadedObjectsList)
						{
							LONG_PTR SortOrder = GetWindowLongPtr(LoadedObjectsList, GWL_USERDATA);
							ListView_SortItems(LoadedObjectsList, SortComparator, SortOrder);
							SortOrder = SortOrder == NULL;
							SetWindowLongPtr(LoadedObjectsList, GWL_USERDATA, SortOrder);
						}

						break;
					case LVN_KEYDOWN:
						if (NotificationData->hwndFrom == LoadedObjectsList)
						{
							NMLVKEYDOWN* KeyData = (NMLVKEYDOWN*)lParam;

							switch (KeyData->wVKey)
							{
							case VK_DELETE:
								Instance.RemoveSelection();
								break;
							}
						}

						break;
					case LVN_GETDISPINFO:
						if (Instance.RefreshingList == false)
						{
							NMLVDISPINFO* DispInfo = (NMLVDISPINFO*)lParam;
							if ((DispInfo->item.mask & LVIF_TEXT) && DispInfo->item.lParam)
							{
								PaletteObject* Current = (PaletteObject*)DispInfo->item.lParam;
								SME_ASSERT(Current && Current->BaseObject);

								if (Current->Name.length())
									sprintf_s(DispInfo->item.pszText, DispInfo->item.cchTextMax, "%s", Current->Name.c_str());
								else
									sprintf_s(DispInfo->item.pszText, DispInfo->item.cchTextMax, "%s", Current->BaseObject->GetEditorID());
							}
						}

						break;
					case LVN_ITEMCHANGED:
						{
							NMLISTVIEW* ChangeData = (NMLISTVIEW*)lParam;

							if ((ChangeData->uChanged & LVIF_STATE) &&
								(ChangeData->uOldState & LVIS_FOCUSED) &&
								(ChangeData->uNewState & LVIS_FOCUSED) == false)
							{
								if (ChangeData->hdr.hwndFrom == LoadedObjectsList)
								{
									// before the new listview item is selected
									Result = TRUE;

									if (Instance.ActiveObject)
										Instance.ActiveObject->GetFromDialog(hWnd);
								}
							}
							else if ((ChangeData->uChanged & LVIF_STATE) &&
									 (((ChangeData->uOldState & LVIS_FOCUSED) == false &&
									 (ChangeData->uNewState & LVIS_FOCUSED)) ||
									 ((ChangeData->uOldState & LVIS_SELECTED) == false &&
									 (ChangeData->uNewState & LVIS_SELECTED) &&
									 TESListView::GetSelectedItemCount(LoadedObjectsList) == 1)))
							{
								if (ChangeData->hdr.hwndFrom == LoadedObjectsList)
								{
									// after the new listview item is selected
									Result = TRUE;

									int NewIndex = ChangeData->iItem;
									Instance.ActiveObject = NULL;

									PaletteObject* NewSelection = (PaletteObject*)ChangeData->lParam;
									if (NewIndex != -1 && NewSelection)
									{
										Instance.ActiveObject = NewSelection;
										Instance.ActiveObject->SetInDialog(hWnd);
										Instance.EnableControls(true);
									}
									else
										Instance.EnableControls(false);

									Instance.UpdateSelectionList();
									Instance.UpdatePreview();
								}
							}
							else if (ChangeData->hdr.hwndFrom == LoadedObjectsList &&
									 TESListView::GetSelectedItemCount(LoadedObjectsList) == 0)
							{
								// this needs to be deferred until the old selection/deselection message is processed
								SetTimer(hWnd, IDC_OBJECTPALETTE_LISTSTATETIMERID, 200, NULL);
							}
						}

						break;
					}
				}

				break;
			case WM_CLOSE:
				Instance.Close();
				Result = TRUE;
				break;
			case WM_INITDIALOG:
				Instance.InitializeDialog(hWnd);
				break;
			case WM_DESTROY:
				Instance.DeinitializeDialog(hWnd);
				break;
			}

			int Throwaway = 0;
			if (Instance.Renderer)
				Instance.Renderer->DialogMessageCallback(hWnd, uMsg, wParam, lParam, (LONG_PTR)&Throwaway);

			return Result;
		}

		int CALLBACK CSEObjectPaletteManager::SortComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
		{
			PaletteObject* One = (PaletteObject*)lParam1;
			PaletteObject* Two = (PaletteObject*)lParam2;

			SME_ASSERT(One && Two);
			SME_ASSERT(One->BaseObject && Two->BaseObject);

			const char* NameOne = One->Name.c_str();
			if (One->Name.length() == 0)
				NameOne = One->BaseObject->GetEditorID();
			const char* NameTwo = Two->Name.c_str();
			if (Two->Name.length() == 0)
				NameTwo = Two->BaseObject->GetEditorID();

			int Compare = _stricmp(NameOne, NameTwo);
			if (lParamSort)
				Compare *= -1;

			return Compare;
		}

		void CSEObjectPaletteManager::InitializeDialog(HWND Dialog)
		{
			HWND LoadedObjectsList = GetDlgItem(Dialog, IDC_PALETTEOBJECTS_LOADED);
			HWND SelectedObjectsList = GetDlgItem(Dialog, IDC_PALETTEOBJECTS_SELECTION);
			HWND BaseFormList = GetDlgItem(Dialog, IDC_OBJECTPALETTE_BASEOBJECT);

			TESListView::AddColumnHeader(LoadedObjectsList, 0, "Palette Objects", 225);
			TESListView::AddColumnHeader(SelectedObjectsList, 0, "Selection", 225);

			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Activator, true, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Apparatus, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Armor, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Book, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Clothing, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Container, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Ingredient, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Light, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Misc, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Furniture, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Weapon, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Ammo, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_SoulGem, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Key, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_AlchemyItem, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_SigilStone, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Static, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Tree, false, false);
			TESComboBox::PopulateWithForms(BaseFormList, TESForm::kFormType_Flora, false, false);

			MainDialog = Dialog;

			ExtraDataList = TESDialog::CreateDialogExtraDataList(Dialog);
			TESRenderControl::Parameters Params = { 0 };
			Params.previewOutputCtrlID = IDC_OBJECTPALETTE_PREVIEW;
			Params.renderTargetWidth = Params.renderTargetHeight = 1024.f;
			Renderer = TESPreviewControl::CreatePreviewControl(Dialog, &Params);

			BGSEEUI->GetWindowHandleCollection(BGSEditorExtender::BGSEEUIManager::kHandleCollection_DragDropableWindows)->Add(LoadedObjectsList);

			EnableControls(false);
			SetTimer(Dialog, IDC_OBJECTPALETTE_FILTERINPUTTIMERID, 500, NULL);
			SetTimer(Dialog, IDC_OBJECTPALETTE_PREVIEWTIMERID, 5, NULL);

			TESDialog::AddDialogToOpenList(Dialog);
		}

		void CSEObjectPaletteManager::DeinitializeDialog(HWND Dialog)
		{
			HWND LoadedObjectsList = GetDlgItem(Dialog, IDC_PALETTEOBJECTS_LOADED);
			HWND SelectedObjectsList = GetDlgItem(Dialog, IDC_PALETTEOBJECTS_SELECTION);
			HWND BaseFormList = GetDlgItem(Dialog, IDC_OBJECTPALETTE_BASEOBJECT);

			TESComboBox::ClearItems(BaseFormList);
			TESListView::ClearColumnHeaders(LoadedObjectsList);
			TESListView::ClearColumnHeaders(SelectedObjectsList);

			MainDialog = NULL;

			CurrentPaletteFilename = "";
			TimeCounter = 0;
			ActiveObject = NULL;
			Renderer = NULL;
			ExtraDataList = NULL;

			LoadedObjects.clear();
			CurrentSelection.clear();

			if (PreviewRef)
			{
				PreviewRef->DeleteInstance();
				PreviewRef = NULL;
			}

			BGSEEUI->GetWindowHandleCollection(BGSEditorExtender::BGSEEUIManager::kHandleCollection_DragDropableWindows)->Remove(LoadedObjectsList);
			TESDialog::DestroyDialogExtraDataList(Dialog);

			KillTimer(Dialog, IDC_OBJECTPALETTE_FILTERINPUTTIMERID);
			KillTimer(Dialog, IDC_OBJECTPALETTE_PREVIEWTIMERID);
			KillTimer(Dialog, IDC_OBJECTPALETTE_LISTSTATETIMERID);

			TESDialog::RemoveDialogFromOpenList(Dialog);
		}

		void CSEObjectPaletteManager::RefreshObjectList()
		{
			SME_ASSERT(MainDialog);

			HWND LoadedObjectsList = GetDlgItem(MainDialog, IDC_PALETTEOBJECTS_LOADED);
			HWND FilterBox = GetDlgItem(MainDialog, IDC_FILTERBOX);

			SME_ASSERT(RefreshingList == false);
			RefreshingList = true;

			TESListView::ClearItems(LoadedObjectsList);
			ActiveObject = NULL;

			char Buffer[0x100] = { 0 };
			GetWindowText(FilterBox, Buffer, sizeof(Buffer));
			std::string Filter(Buffer);
			SME::StringHelpers::MakeLower(Filter);
			bool SelectDefault = false;

			for each (auto Itr in LoadedObjects)
			{
				SME_ASSERT(Itr->BaseObject && Itr->BaseObject->GetEditorID());

				std::string Compare(Itr->Name);
				if (Compare.length() == 0)
					Compare = Itr->BaseObject->GetEditorID();

				SME::StringHelpers::MakeLower(Compare);
				if (Filter.length() == 0 || Compare.find(Filter) != std::string::npos)
				{
					TESListView::InsertItem(LoadedObjectsList, Itr.get());
					SelectDefault = true;
				}
			}

			RefreshingList = false;
			if (SelectDefault)
				TESListView::SetSelectedItem(LoadedObjectsList, 0);
			else
				EnableControls(false);

			BGSEEUI->GetInvalidationManager()->Redraw(LoadedObjectsList);
		}

		void CSEObjectPaletteManager::UpdateSelectionList()
		{
			SME_ASSERT(MainDialog);

			HWND LoadedObjectsList = GetDlgItem(MainDialog, IDC_PALETTEOBJECTS_LOADED);
			HWND SelectedObjectsList = GetDlgItem(MainDialog, IDC_PALETTEOBJECTS_SELECTION);

			SME_ASSERT(RefreshingList == false);
			RefreshingList = true;

			TESListView::ClearItems(SelectedObjectsList);
			CurrentSelection.clear();

			int Selection = -1;
			do
			{
				Selection = ListView_GetNextItem(LoadedObjectsList, Selection, LVNI_SELECTED);
				if (Selection != -1)
				{
					PaletteObject* Data = (PaletteObject*)TESListView::GetItemData(LoadedObjectsList, Selection);
					if (Data)
					{
						PaletteObjectHandleT Copy;
						bool HandleFound = GetBaseHandle(Data, Copy);
						SME_ASSERT(HandleFound);

						CurrentSelection.push_back(Copy);
					}
				}
			} while (Selection != -1);

			for each (auto Itr in CurrentSelection)
				TESListView::InsertItem(SelectedObjectsList, Itr.get());

			RefreshingList = false;
			BGSEEUI->GetInvalidationManager()->Redraw(SelectedObjectsList);
		}

		void CSEObjectPaletteManager::RemoveSelection()
		{
			HWND LoadedObjectsList = GetDlgItem(MainDialog, IDC_PALETTEOBJECTS_LOADED);
			ActiveObject = NULL;
			int Selection = -1;
			do
			{
				Selection = ListView_GetNextItem(LoadedObjectsList, Selection, LVNI_SELECTED);
				if (Selection != -1)
				{
					PaletteObject* Data = (PaletteObject*)TESListView::GetItemData(LoadedObjectsList, Selection);
					if (Data)
						RemoveLoadedObject(Data);
				}
			} while (Selection != -1);

			RefreshObjectList();
			UpdateSelectionList();
			UpdatePreview();
		}

		void CSEObjectPaletteManager::SaveObjects(const PaletteObjectListT& Objects, const char* Path) const
		{
			DeleteFile(Path);

			int i = 0;
			char Buffer[0x5] = { 0 };
			SerializedData FileBuffer;

			for each (auto Itr in Objects)
			{
				FORMAT_STR(Buffer, "%d", i);
				FileBuffer += "[" + SerializedData(Buffer) + "]\n";

				SerializedData Out = "";
				Itr->Serialize(Out);
				FileBuffer += Out + "\n";

				i++;
			}

			IFileStream File;
			if (File.Create(Path) == false)
				BGSEEUI->MsgBoxE(MainDialog, MB_OK, "Couldn't create OPAL file at '%s'.", Path);
			else
				File.WriteBuf(FileBuffer.c_str(), FileBuffer.length());
		}

		bool CSEObjectPaletteManager::LoadObjects(PaletteObjectListT& OutObjects, const char* Path) const
		{
			OutObjects.clear();

			IFileStream File;
			char Sections[0x4000] = { 0 };
			char Data[0x4000] = { 0 };
			bool Result = true;

			if (File.Open(Path))
			{
				if (GetPrivateProfileSectionNames(Sections, sizeof(Sections), Path))
				{
					for (const char* Section = Sections; *Section != '\0'; Section += strlen(Section) + 1)
					{
						ZeroMemory(Data, sizeof(Data));
						GetPrivateProfileSection(Section, Data, sizeof(Data), Path);

						SerializedData Extract;
						for (const char* Line = Data; *Line != '\0'; Line += strlen(Line) + 1)
						{
							Extract += Line;
							Extract += "\n";
						}

						PaletteObject* NewObject = new PaletteObject(Extract);
						if (NewObject->BaseObject == NULL)
						{
							Result = false;
							BGSEECONSOLE_MESSAGE("Couldn't resolve base form for palette object '%s' (ID=%s) in file '%s'",
												 NewObject->Name.c_str(),
												 Section,
												 Path);
						}
						else
							OutObjects.push_back(PaletteObjectHandleT(NewObject));
					}
				}
			}

			return Result;
		}

		void CSEObjectPaletteManager::NewPalette()
		{
			SME_ASSERT(MainDialog);

			LoadedObjects.clear();
			CurrentSelection.clear();

			RefreshObjectList();
			UpdateSelectionList();

			CurrentPaletteFilename = "";

			SetWindowText(MainDialog, "Object Palette");
		}

		void CSEObjectPaletteManager::SavePalette(bool NewFile)
		{
			SME_ASSERT(MainDialog);

			if (NewFile || CurrentPaletteFilename.length() == 0)
			{
				std::string FilePath;
				if (ShowFileDialog(true, FilePath, CurrentPaletteFilename))
				{
					SaveObjects(LoadedObjects, FilePath.c_str());

					std::string Title("Object Palette");
					Title += ": " + CurrentPaletteFilename;

					SetWindowText(MainDialog, Title.c_str());
				}
			}
			else
			{
				std::string FilePath(kRepositoryPath());
				FilePath += CurrentPaletteFilename + std::string(".") + std::string(kPaletteFileExtension);

				SaveObjects(LoadedObjects, FilePath.c_str());
			}
		}

		void CSEObjectPaletteManager::LoadPalette()
		{
			SME_ASSERT(MainDialog);

			std::string FilePath;
			if (ShowFileDialog(false, FilePath, CurrentPaletteFilename))
			{
				if (LoadObjects(LoadedObjects, FilePath.c_str()) == false)
					BGSEEUI->MsgBoxW(MainDialog, MB_OK, "Errors were encountered when loading the object palette file. Check the console for more information.");

				RefreshObjectList();

				std::string Title("Object Palette");
				Title += ": " + CurrentPaletteFilename;

				SetWindowText(MainDialog, Title.c_str());
			}
		}

		void CSEObjectPaletteManager::MergePalette()
		{
			SME_ASSERT(MainDialog);

			std::string FilePath, Filename;
			if (ShowFileDialog(false, FilePath, Filename))
			{
				PaletteObjectListT Temp;
				if (LoadObjects(Temp, FilePath.c_str()) == false)
					BGSEEUI->MsgBoxW(MainDialog, MB_OK, "Errors were encountered when loading the object palette file. Check the console for more information.\n\nThe merge operation did not complete successfully.");
				else
				{
					for each (auto Itr in Temp)
						LoadedObjects.push_back(Itr);

					RefreshObjectList();
					BGSEEUI->MsgBoxI(MainDialog, MB_OK, "The merge operation will be complete upon saving the current file.");
				}
			}
		}

		void CSEObjectPaletteManager::UpdatePreview()
		{
			SME_ASSERT(MainDialog);

			if (PreviewRef)
			{
				NiNode* Node = PreviewRef->GetNiNode();
				Renderer->TESPreviewControl::RemovePreviewNode(Node);
				PreviewRef->SetNiNode(NULL);

				PreviewRef->DeleteInstance();
				PreviewRef = NULL;
			}

			if (ActiveObject)
			{
				_TEXTUREPALETTE->ReleaseTextures();

				Vector3 Zero(0, 0, 0);
				PreviewRef = ActiveObject->Instantiate(Zero, true);
				NiNode* Node = PreviewRef->GenerateNiNode();
				if (Node)
					Renderer->AddPreviewNode(Node);
				else
				{
					PreviewRef->DeleteInstance();
					PreviewRef = NULL;
				}
			}

			Renderer->Present();
		}

		void CSEObjectPaletteManager::HandleDragDrop()
		{
			SME_ASSERT(MainDialog);
			for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
			{
				TESForm* Form = Itr->Data;
				if (GetFormTypeAllowed(Form->formType) && Form->GetEditorID())
					LoadedObjects.push_back(PaletteObjectHandleT(new PaletteObject(Form)));
			}

			_RENDERSEL->ClearSelection();

			RefreshObjectList();
		}

		void CSEObjectPaletteManager::EnableControls(bool State) const
		{
			SME_ASSERT(MainDialog);

			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_NAME), State);
			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_BASEOBJECT), State);

			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_ANGLEBASE_X), State);
			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_ANGLEBASE_Y), State);
			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_ANGLEBASE_Z), State);

			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_ANGLEOFFSET_X), State);
			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_ANGLEOFFSET_Y), State);
			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_ANGLEOFFSET_Z), State);

			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_SINKBASE), State);
			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_SINKOFFSET), State);

			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_SCALEBASE), State);
			EnableWindow(GetDlgItem(MainDialog, IDC_OBJECTPALETTE_SCALEOFFSET), State);
		}

		CSEObjectPaletteManager::CSEObjectPaletteManager() :
			LoadedObjects(),
			CurrentSelection(),
			PreviewRef(NULL),
			MainDialog(NULL),
			CurrentPaletteFilename(""),
			TimeCounter(0),
			ExtraDataList(NULL),
			Renderer(NULL),
			ActiveObject(NULL),
			RefreshingList(false)
		{
			;//
		}

		CSEObjectPaletteManager::~CSEObjectPaletteManager()
		{
			Close();
		}

		bool CSEObjectPaletteManager::PlaceObject(int X, int Y) const
		{
			if (MainDialog == NULL)
				return false;

			if (CurrentSelection.size())
			{
				if ((*TESRenderWindow::ActiveCell == NULL && _TES->currentInteriorCell == NULL) ||
					*TESRenderWindow::PathGridEditFlag ||
					*TESRenderWindow::LandscapeEditFlag)
				{
					BGSEEUI->MsgBoxE(*TESRenderWindow::WindowHandle,
									 MB_OK,
									 "Cannot place object! Possible reasons: No cell loaded in the render window, pathgrid/landscape edit mode enabled");
					return false;
				}

				TESObjectCELL* Interior = _TES->currentInteriorCell;
				TESWorldSpace* Worldspace = _TES->currentWorldSpace;
				if (Interior)
					Worldspace = NULL;

				if (Interior || Worldspace)
				{
					NiCamera* MainCamera = _PRIMARYRENDERER->primaryCamera;
					Vector3 Position, Rotation;

					// get initial click position and rotation
					thisCall<bool>(0x006FF1A0, MainCamera, X, Y, &Position, &Rotation);

					// I really need to decode this stuff...
					thisCall<void>(0x00417C40, 0x00A0BC64, _TES->sceneGraphObjectRoot);
					if (thisCall<bool>(0x005E6030, 0x00A0BC64, &Position, &Rotation, 0))
					{
						float*** NewPosition = (float***)0x00A0BC80;
						Position.x = *(float*)((UInt32)**NewPosition + 0x8);
						Position.y = *(float*)((UInt32)**NewPosition + 0xC);
						Position.z = *(float*)((UInt32)**NewPosition + 0x10);
					}

					// sanity check to ensure exterior coords are valid
					if (_TES->currentInteriorCell == NULL)
					{
						TESWorldSpace* CurrentWorldspace = _TES->currentWorldSpace;
						if (CurrentWorldspace == NULL || _DATAHANDLER->GetExteriorCell(Position.x, Position.y, CurrentWorldspace) == NULL)
							thisCall<bool>(0x006FF1A0, MainCamera, X, Y, &Position, &Rotation);
					}

					// randomly pick one from the selection list
					SME::MersenneTwister::init_genrand(GetTickCount());
					int Index = SME::MersenneTwister::genrand_real2() * (CurrentSelection.size() - 1);
					if (Index < CurrentSelection.size())
					{
						TESObjectREFR* NewRef = CurrentSelection.at(Index)->Instantiate(Position, false);
						NewRef->SetFromActiveFile(true);
						Rotation = NewRef->rotation;

						_DATAHANDLER->PlaceObjectRef((TESObject*)NewRef->baseForm,
													 &Position,
													 &Rotation,
													 Interior,
													 Worldspace,
													 NewRef);

						_RENDERSEL->ClearSelection(true);
						_RENDERSEL->AddToSelection(NewRef, true);

						Achievements::kPowerUser->UnlockTool(Achievements::CSEAchievementPowerUser::kTool_ObjectPalette);
						TESRenderWindow::Redraw();
						return true;
					}
				}
			}

			return false;
		}

		void CSEObjectPaletteManager::Show()
		{
			if (MainDialog)
				SetForegroundWindow(MainDialog);
			else
			{
				HWND Dialog = BGSEEUI->ModelessDialog(BGSEEMAIN->GetExtenderHandle(),
										MAKEINTRESOURCE(IDD_OBJECTPALETTE),
										*TESCSMain::WindowHandle,
										DlgProc,
										NULL);
				SME_ASSERT(Dialog);

				if (Settings::Dialogs::kShowMainWindowsInTaskbar.GetData().i)
				{
					BGSEditorExtender::BGSEEWindowStyler::StyleData RegularAppWindow = { 0 };
					RegularAppWindow.Extended = WS_EX_APPWINDOW;
					RegularAppWindow.ExtendedOp = BGSEditorExtender::BGSEEWindowStyler::StyleData::kOperation_OR;

					BGSEEUI->GetWindowStyler()->StyleWindow(Dialog, RegularAppWindow);
				}
			}
		}

		void CSEObjectPaletteManager::Close()
		{
			if (MainDialog)
				DestroyWindow(MainDialog);
		}

		bool CSEObjectPaletteManager::GetFormTypeAllowed(UInt8 Type) const
		{
			switch (Type)
			{
			case TESForm::kFormType_Activator:
			case TESForm::kFormType_Apparatus:
			case TESForm::kFormType_Armor:
			case TESForm::kFormType_Book:
			case TESForm::kFormType_Clothing:
			case TESForm::kFormType_Container:
			case TESForm::kFormType_Ingredient:
			case TESForm::kFormType_Light:
			case TESForm::kFormType_Misc:
			case TESForm::kFormType_Static:
			case TESForm::kFormType_Tree:
			case TESForm::kFormType_Flora:
			case TESForm::kFormType_Furniture:
			case TESForm::kFormType_Weapon:
			case TESForm::kFormType_Ammo:
			case TESForm::kFormType_SoulGem:
			case TESForm::kFormType_Key:
			case TESForm::kFormType_AlchemyItem:
			case TESForm::kFormType_SigilStone:
				return true;
			default:
				return false;
			}
		}

		bool CSEObjectPaletteManager::ShowFileDialog(bool Save, std::string& OutPath, std::string& OutName)
		{
			char SelectPath[MAX_PATH] = { 0 };
			char FilterString[0x100] = { 0 };

			// can't format the file extension in the filter string because printf adds padding to the string after the first null character
			// at least in debug mode, can't be bothered to hack around it
			// ### remember to change the filter string manually if the extension changes
			if (TESDialog::ShowFileSelect(MainDialog,
				kRepositoryPath().c_str(),
				"Object Palette Files\0*.cseopal\0\0",
				"Select Object Palette File",
				NULL,
				NULL,
				Save == false,
				Save,
				SelectPath,
				MAX_PATH))
			{
				std::string SanitizedFilePath(SelectPath);
				int ExtIdx = SanitizedFilePath.rfind(kPaletteFileExtension);
				if (ExtIdx != -1)
					SanitizedFilePath.erase(ExtIdx - 1);		// remove the dot too

				OutPath = kRepositoryPath();
				OutPath += SanitizedFilePath + std::string(".") + std::string(kPaletteFileExtension);
				OutName = SanitizedFilePath;

				return true;
			}
			else
				return false;
		}

		bool CSEObjectPaletteManager::GetBaseHandle(PaletteObject* Data, PaletteObjectHandleT& Out) const
		{
			SME_ASSERT(Data);

			for each (auto Itr in LoadedObjects)
			{
				if (Itr.get() == Data)
				{
					Out = Itr;
					return true;
				}
			}

			return false;
		}

		bool CSEObjectPaletteManager::RemoveLoadedObject(PaletteObject* Data)
		{
			SME_ASSERT(Data);

			for (PaletteObjectListT::iterator Itr = LoadedObjects.begin(); Itr != LoadedObjects.end(); Itr++)
			{
				if ((*Itr).get() == Data)
				{
					LoadedObjects.erase(Itr);
					return true;
				}
			}

			return false;
		}
	}
}