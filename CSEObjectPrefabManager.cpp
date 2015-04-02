#include "CSEObjectPrefabManager.h"
#include "Construction Set Extender_Resource.h"
#include "CSEMain.h"

namespace ConstructionSetExtender
{
	namespace ObjectPrefabs
	{
		PrefabObjectPreviewData::PrefabObjectPreviewData() :
			Parent(NULL),
			BaseForms(),
			References(),
			RootNode(NULL)
		{
			;//
		}

		PrefabObjectPreviewData::~PrefabObjectPreviewData()
		{
			Detach();

			TESRender::DeleteNiNode(RootNode);

			for each (auto Itr in References)
				Itr->DeleteInstance();

			for each (auto Itr in BaseForms)
				Itr->DeleteInstance();

			References.clear();
			BaseForms.clear();
		}

		void PrefabObjectPreviewData::Attach(TESPreviewControl* To)
		{
			SME_ASSERT(To);
			SME_ASSERT(Parent == NULL);
			SME_ASSERT(RootNode);

			Parent = To;
			Parent->SetPreviewNode(RootNode);
		}

		void PrefabObjectPreviewData::Detach()
		{
			if (Parent)
			{
				Parent->VFn18(RootNode);

				Parent = NULL;
			}
		}

		void PrefabObject::SetInDialog(HWND Dialog)
		{
			HWND DetailsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_CURRENTDETAILSLIST);

			if (Deserialize() == kState_Good)
			{
				ObjectRefCollectionDescriptorListT Description;
				Serializer->GetDescription(Description);

				UInt32 Index = 0;
				TESListView::ClearItems(DetailsList);

				for each (auto Itr in Description)
				{
					for (int i = 0; i < 9; i++)
					{
						LVITEM Item;
						Item.mask = LVIF_TEXT | LVIF_STATE;
						Item.iSubItem = i;
						Item.state = 0;
						Item.stateMask = 0;
						Item.iItem = Index;

						switch (i)
						{
						case 0:
							if (Itr->EditorID.length())
								Item.pszText = (LPSTR)Itr->EditorID.c_str();
							else
								Item.pszText = (LPSTR)Itr->FormID.c_str();
							break;
						case 1:
							Item.pszText = (LPSTR)Itr->Position.c_str();
							break;
						case 2:
							Item.pszText = (LPSTR)Itr->Rotation.c_str();
							break;
						case 3:
							Item.pszText = (LPSTR)Itr->Scale.c_str();
							break;
						case 4:
							Item.pszText = (LPSTR)Itr->BaseFormEditorID.c_str();
							break;
						case 5:
							Item.pszText = (LPSTR)Itr->BaseFormType.c_str();
							break;
						case 6:
							if (Itr->TemporaryBaseForm)
								Item.pszText = "";
							else
								Item.pszText = "Y";
							break;
						case 7:
							if (Itr->HasEnableStateParent == false)
								Item.pszText = "";
							else if (Itr->EnableStateParentEditorID.length())
								Item.pszText = (LPSTR)Itr->EnableStateParentEditorID.c_str();
							else
								Item.pszText = (LPSTR)Itr->EnableStateParentFormID.c_str();
							break;
						case 8:
							if (Itr->HasEnableStateParent == false || Itr->ParentOppositeState == false)
								Item.pszText = "";
							else
								Item.pszText = "Y";
							break;
						}

						if (i == 0)
							ListView_InsertItem(DetailsList, &Item);
						else
							ListView_SetItem(DetailsList, &Item);
					}

					Index++;
				}

				Description.clear();
			}
		}

		UInt8 PrefabObject::Deserialize(bool Force)
		{
			if (Force || DeserializationState == kState_None)
			{
				if (Serializer->GetFileSerializerType(SourceFile) != ICSEFormCollectionSerializer::kSerializer_ObjectRef)
				{
					BGSEECONSOLE_MESSAGE("Prefab file %s uses an unknown serializer/deserializer!", SourceFile->GetFileName());
					DeserializationState = kState_Bad;
				}
				else
				{
					DeserializationState = kState_None;

					int Throwaway = 0;
					if (Serializer->Deserialize(SourceFile, Throwaway) == false)
						DeserializationState = kState_Bad;
					else
						DeserializationState = kState_Good;
				}
			}

			return DeserializationState;
		}

		UInt8 PrefabObject::Serialize(BGSEditorExtender::BGSEEFormListT& Forms, bool Force)
		{
			if (Force || SerializationState == kState_None)
			{
				SerializationState = kState_None;

				if (Serializer->Serialize(Forms, SourceFile) == false)
					SerializationState = kState_Bad;
				else
					SerializationState = kState_Good;
			}

			return SerializationState;
		}

		PrefabObject::PrefabObject(const char* FilePath) :
			SourceFile(new CSEPluginFileWrapper),
			Serializer(new CSEObjectRefCollectionSerializer(false)),
			Instantiator(new CSEObjectRefCollectionInstantiator),
			SerializationState(kState_None),
			DeserializationState(kState_None),
			FileName("")
		{
			SME_ASSERT(FilePath);

			if (SourceFile->Construct(FilePath) == false)
			{
				DeserializationState = kState_Bad;
				SerializationState = kState_Bad;
			}

			FileName = SourceFile->GetWrappedPlugin()->fileName;
		}

		PrefabObject::~PrefabObject()
		{
			SAFEDELETE(SourceFile);
			SAFEDELETE(Serializer);
			SAFEDELETE(Instantiator);
		}

		PrefabObjectPreviewData* PrefabObject::GeneratePreviewData(TESPreviewControl* PreviewControl)
		{
			SME_ASSERT(PreviewControl);

			if (Deserialize() != kState_Good)
				return NULL;
			else
			{
				PrefabObjectPreviewData* Data = new PrefabObjectPreviewData;
				if (Instantiator->CreatePreviewNode(Serializer, PreviewControl, Data->BaseForms, Data->References, &Data->RootNode))
					return Data;
				else
				{
					delete Data;
					return NULL;
				}
			}
		}

		bool PrefabObject::Instantiate()
		{
			bool Result = false;

			if (Deserialize() == kState_Good)
				Result = Instantiator->Instantiate(Serializer, false);

			return Result;
		}

		CSEObjectPrefabManager							CSEObjectPrefabManager::Instance;
		const char*										CSEObjectPrefabManager::kPrefabFileExtension = "cseprefab";
		const BGSEditorExtender::BGSEEResourceLocation	CSEObjectPrefabManager::kRepositoryPath(CSE_PREFABDEPOT);

#define IDC_OBJECTPREFAB_FILTERINPUTTIMERID		0x200
#define IDC_OBJECTPREFAB_PREVIEWTIMERID			0x201

		INT_PTR CALLBACK CSEObjectPrefabManager::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			HWND FilterBox = GetDlgItem(hWnd, IDC_FILTERBOX);
			HWND LoadedPrefabsList = GetDlgItem(hWnd, IDC_OBJECTPREFAB_LOADEDPREFABSLIST);
			HWND PrefabDetailsList = GetDlgItem(hWnd, IDC_OBJECTPREFAB_CURRENTDETAILSLIST);
			LRESULT Result = FALSE;

			switch (uMsg)
			{
			case WM_COMMAND:
				if (HIWORD(wParam) == EN_CHANGE && (HWND)lParam == FilterBox)
					Instance.TimeCounter = 0;

				switch (wParam)
				{
				case IDC_OBJECTPREFAB_CREATE:
					Instance.NewPrefab();
					break;
				case IDC_OBJECTPREFAB_INSTANTIATE:
					Instance.InstatiateSelection();
					break;
				case IDC_OBJECTPREFAB_REFRESH:
					Instance.ReloadPrefabs();
					break;
				}

				break;
			case WM_TIMER:
				switch (wParam)
				{
				case IDC_OBJECTPREFAB_FILTERINPUTTIMERID:
					Result = TRUE;
					if (Instance.TimeCounter && Instance.TimeCounter != -1 && Instance.TimeCounter >= 500 * 1.5)
					{
						Instance.RefreshPrefabList();
						Instance.TimeCounter = -1;
					}
					else if (Instance.TimeCounter != -1)
						Instance.TimeCounter += 500;

					break;
				case IDC_OBJECTPREFAB_PREVIEWTIMERID:
					Result = TRUE;
					if (Instance.PreviewData)
						Instance.Renderer->Present(0.0);

					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					switch (NotificationData->code)
					{
					case LVN_GETDISPINFO:
						if (Instance.RefreshingList == false)
						{
							NMLVDISPINFO* DispInfo = (NMLVDISPINFO*)lParam;
							if ((DispInfo->item.mask & LVIF_TEXT) && DispInfo->item.lParam && DispInfo->hdr.hwndFrom == LoadedPrefabsList)
							{
								PrefabObject* Current = (PrefabObject*)DispInfo->item.lParam;
								SME_ASSERT(Current);

								sprintf_s(DispInfo->item.pszText, DispInfo->item.cchTextMax, "%s", Current->FileName.c_str());
								Result = TRUE;
							}
						}

						break;
					case LVN_ITEMCHANGED:
						{
							NMLISTVIEW* ChangeData = (NMLISTVIEW*)lParam;

							if ((ChangeData->uChanged & LVIF_STATE) &&
								(ChangeData->uOldState & LVIS_FOCUSED) == false &&
								(ChangeData->uNewState & LVIS_FOCUSED))
							{
								if (ChangeData->hdr.hwndFrom == LoadedPrefabsList)
								{
									// after the new listview item is selected
									Result = TRUE;

									int NewIndex = ChangeData->iItem;
									Instance.CurrentSelection = NULL;
									if (Instance.PreviewData)
										SAFEDELETE(Instance.PreviewData);

									PrefabObject* NewSelection = (PrefabObject*)ChangeData->lParam;
									if (NewIndex != -1 && NewSelection)
									{
										Instance.CurrentSelection = NewSelection;

										NewSelection->SetInDialog(hWnd);
										Instance.PreviewData = NewSelection->GeneratePreviewData(Instance.Renderer);
										if (Instance.PreviewData == NULL)
											BGSEECONSOLE_MESSAGE("Couldn't generate preview data for prefab %s", NewSelection->FileName.c_str());
										else
											Instance.PreviewData->Attach(Instance.Renderer);
									}

									Instance.UpdatePreview();
								}
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

		void CSEObjectPrefabManager::InitializeDialog(HWND Dialog)
		{
			HWND LoadedPrefabsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_LOADEDPREFABSLIST);
			HWND PrefabDetailsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_CURRENTDETAILSLIST);

			TESListView::AddColumnHeader(LoadedPrefabsList, 0, "Prefab Collection", 225);

			TESListView::AddColumnHeader(PrefabDetailsList, 0, "Reference", 75);
			TESListView::AddColumnHeader(PrefabDetailsList, 1, "Position", 75);
			TESListView::AddColumnHeader(PrefabDetailsList, 2, "Rotation", 75);
			TESListView::AddColumnHeader(PrefabDetailsList, 3, "Scale", 30);
			TESListView::AddColumnHeader(PrefabDetailsList, 4, "Base Form", 75);
			TESListView::AddColumnHeader(PrefabDetailsList, 5, "Type", 60);
			TESListView::AddColumnHeader(PrefabDetailsList, 6, "Existing Base Form", 50);
			TESListView::AddColumnHeader(PrefabDetailsList, 7, "Enable State Parent", 75);
			TESListView::AddColumnHeader(PrefabDetailsList, 8, "Parent Opposite State", 30);

			MainDialog = Dialog;

			ExtraDataList = TESDialog::CreateDialogExtraDataList(Dialog);
			TESRenderControl::Parameters Params = { 0 };
			Params.previewOutputCtrlID = IDC_OBJECTPREFAB_PREVIEW;
			Params.renderTargetWidth = Params.renderTargetHeight = 1024.f;
			Renderer = TESPreviewControl::CreatePreviewControl(Dialog, &Params);

			SetTimer(Dialog, IDC_OBJECTPREFAB_FILTERINPUTTIMERID, 500, NULL);
			SetTimer(Dialog, IDC_OBJECTPREFAB_PREVIEWTIMERID, 5, NULL);

			ReloadPrefabs();
		}

		void CSEObjectPrefabManager::DeinitializeDialog(HWND Dialog)
		{
			HWND LoadedPrefabsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_LOADEDPREFABSLIST);
			HWND PrefabDetailsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_CURRENTDETAILSLIST);

			TESListView::ClearColumnHeaders(LoadedPrefabsList);
			TESListView::ClearColumnHeaders(PrefabDetailsList);

			SAFEDELETE(PreviewData);
			CurrentSelection = NULL;

			MainDialog = NULL;

			TimeCounter = 0;
			Renderer = NULL;
			ExtraDataList = NULL;

			LoadedPrefabs.clear();

			TESDialog::DestroyDialogExtraDataList(Dialog);

			KillTimer(Dialog, IDC_OBJECTPREFAB_FILTERINPUTTIMERID);
			KillTimer(Dialog, IDC_OBJECTPREFAB_PREVIEWTIMERID);
		}

		void CSEObjectPrefabManager::RefreshPrefabList()
		{
			SME_ASSERT(MainDialog);

			HWND LoadedPrefabsList = GetDlgItem(MainDialog, IDC_OBJECTPREFAB_LOADEDPREFABSLIST);
			HWND FilterBox = GetDlgItem(MainDialog, IDC_FILTERBOX);

			SME_ASSERT(RefreshingList == false);
			RefreshingList = true;

			TESListView::ClearItems(LoadedPrefabsList);
			SAFEDELETE(PreviewData);
			CurrentSelection = NULL;

			char Buffer[0x100] = { 0 };
			GetWindowText(FilterBox, Buffer, sizeof(Buffer));
			std::string Filter(Buffer);
			SME::StringHelpers::MakeLower(Filter);
			bool SelectDefault = false;

			for each (auto Itr in LoadedPrefabs)
			{
				std::string Compare(Itr->FileName);
				SME::StringHelpers::MakeLower(Compare);

				if (Filter.length() == 0 || Compare.find(Filter) != std::string::npos)
				{
					TESListView::InsertItem(LoadedPrefabsList, Itr.get());
					SelectDefault = true;
				}
			}

			RefreshingList = false;
			if (SelectDefault)
				TESListView::SetSelectedItem(LoadedPrefabsList, 0);

			BGSEEUI->GetInvalidationManager()->Redraw(LoadedPrefabsList);
		}

		void CSEObjectPrefabManager::LoadPrefabsInDirectory(const char* DirectoryPath)
		{
			for (IDirectoryIterator Itr(DirectoryPath, "*.*"); !Itr.Done(); Itr.Next())
			{
				if ((Itr.Get()->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
					strcmp(Itr.Get()->cFileName, ".") &&
					strcmp(Itr.Get()->cFileName, ".."))
				{
					LoadPrefabsInDirectory(Itr.GetFullPath().c_str());
				}
				else
				{
					std::string FilePath(Itr.GetFullPath());
					if (FilePath.rfind(kPrefabFileExtension) == FilePath.length() - strlen(kPrefabFileExtension))
					{
						PrefabObjectHandleT Prefab(new PrefabObject(FilePath.c_str()));
						if (Prefab->DeserializationState == PrefabObject::kState_Bad || Prefab->Deserialize() == PrefabObject::kState_Bad)
							BGSEECONSOLE_MESSAGE("Couldn't load prefab collection at %s", FilePath.c_str());
						else
							LoadedPrefabs.push_back(Prefab);
					}
				}
			}
		}

		void CSEObjectPrefabManager::ReloadPrefabs()
		{
			SME_ASSERT(MainDialog);

			LoadedPrefabs.clear();
			LoadPrefabsInDirectory(kRepositoryPath.GetFullPath().c_str());

			RefreshPrefabList();
		}

		void CSEObjectPrefabManager::NewPrefab()
		{
			SME_ASSERT(MainDialog);

			std::string FilePath, FileName;
			if (ShowFileDialog(true, FilePath, FileName))
			{
				PrefabObjectHandleT Prefab(new PrefabObject(FilePath.c_str()));
				BGSEditorExtender::BGSEEFormListT Selection;

				bool SelectionCheck = true;
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					if (Ref == NULL)
					{
						SelectionCheck = false;
						break;
					}

					Selection.push_back(new CSEFormWrapper(Ref));
				}

				if (SelectionCheck)
				{
					if (Prefab->Serialize(Selection))
					{
						LoadedPrefabs.push_back(Prefab);
						RefreshPrefabList();
					}
					else
					{
						SelectionCheck = false;
						Prefab->SourceFile->Delete();
					}
				}

				if (SelectionCheck == false)
				{
					BGSEEUI->MsgBoxE(MainDialog, MB_OK,
									 "Couldn't create a prefab collection from the current Render Window selection. Check the console for more information.");
				}

				for each (auto Itr in Selection)
					delete Itr;

				Selection.clear();
			}
		}

		void CSEObjectPrefabManager::InstatiateSelection()
		{
			SME_ASSERT(MainDialog);

			if (CurrentSelection)
			{
				if (CurrentSelection->Instantiate() == false)
					BGSEEUI->MsgBoxE(MainDialog, MB_OK, "Couldn't instantiate the current selection. Check the console for more information.");
			}
		}

		void CSEObjectPrefabManager::UpdatePreview()
		{
			SME_ASSERT(MainDialog);
			Instance.Renderer->Present(0.0);
		}

		bool CSEObjectPrefabManager::ShowFileDialog(bool Save, std::string& OutPath, std::string& OutName)
		{
			char SelectPath[MAX_PATH] = { 0 };
			char FilterString[0x100] = { 0 };

			// ### remember to change the filter string manually if the extension changes
			if (TESDialog::ShowFileSelect(MainDialog,
				kRepositoryPath().c_str(),
				"Object Prefab Files\0*.cseprefab\0\0",
				"Select Object Prefab File",
				NULL,
				NULL,
				Save == false,
				Save,
				SelectPath,
				MAX_PATH))
			{
				std::string SanitizedFilePath(SelectPath);
				int ExtIdx = SanitizedFilePath.rfind(kPrefabFileExtension);
				if (ExtIdx != -1)
					SanitizedFilePath.erase(ExtIdx - 1);		// remove the dot too

				OutPath = kRepositoryPath();
				OutPath += SanitizedFilePath + std::string(".") + std::string(kPrefabFileExtension);
				OutName = SanitizedFilePath;

				return true;
			}
			else
				return false;
		}

		CSEObjectPrefabManager::CSEObjectPrefabManager() :
			LoadedPrefabs(),
			CurrentSelection(NULL),
			PreviewData(NULL),
			MainDialog(NULL),
			TimeCounter(0),
			ExtraDataList(NULL),
			Renderer(NULL),
			RefreshingList(false)
		{
			;//
		}

		CSEObjectPrefabManager::~CSEObjectPrefabManager()
		{
			Close();
		}

		void CSEObjectPrefabManager::Show()
		{
			if (MainDialog)
				SetForegroundWindow(MainDialog);
			else
			{
				HWND Dialog = BGSEEUI->ModelessDialog(BGSEEMAIN->GetExtenderHandle(),
													  MAKEINTRESOURCE(IDD_OBJECTPREFAB),
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

		void CSEObjectPrefabManager::Close()
		{
			if (MainDialog)
				DestroyWindow(MainDialog);
		}
	}
}