#include "ObjectPrefabManager.h"
#include "Construction Set Extender_Resource.h"
#include "Main.h"
#include "Achievements.h"

namespace cse
{
	namespace objectPrefabs
	{
		PrefabObjectPreviewData::PrefabObjectPreviewData() :
			Parent(nullptr),
			BaseForms(),
			References(),
			RootNode(nullptr)
		{
			;//
		}

		PrefabObjectPreviewData::~PrefabObjectPreviewData()
		{
			Detach();

			if (RootNode)
				TESRender::DeleteNiRefObject(RootNode);

			for (auto Itr : References)
				Itr->DeleteInstance();

			for (auto Itr : BaseForms)
				Itr->DeleteInstance();

			References.clear();
			BaseForms.clear();
		}

		void PrefabObjectPreviewData::Attach(TESPreviewControl* To)
		{
			SME_ASSERT(To);
			SME_ASSERT(Parent == nullptr);
			SME_ASSERT(RootNode);

			Parent = To;
			Parent->AddPreviewNode(RootNode);
			Parent->ResetCamera();
		}

		void PrefabObjectPreviewData::Detach()
		{
			if (Parent)
			{
				SME_ASSERT(RootNode->m_uiRefCount == 1);
				Parent->TESPreviewControl::RemovePreviewNode(RootNode);
				Parent->ResetCamera();
				Parent->Present();

				Parent = nullptr;
				RootNode = nullptr;
			}
		}

		void PrefabObject::SetInDialog(HWND Dialog)
		{
			HWND DetailsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_CURRENTDETAILSLIST);

			if (Deserialize() == kState_Good)
			{
				ObjectRefCollectionDescriptorArrayT Description;
				Serializer->GetDescription(Description);

				UInt32 Index = 0;
				TESListView::ClearItems(DetailsList);

				for (const auto& Itr : Description)
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
				if (Serializer->GetFileSerializerType(SourceFile) != IFormCollectionSerializer::kSerializer_ObjectRef)
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

		UInt8 PrefabObject::Serialize(bgsee::FormListT& Forms, bool Force)
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

		PrefabObject::PrefabObject(const char* SourceFilePath, const char* RepositoryPath, bool OverwriteExisting) :
			SourceFile(new TESFileWrapper),
			Serializer(new ObjectRefCollectionSerializer(false)),
			Instantiator(new ObjectRefCollectionInstantiator),
			SerializationState(kState_None),
			DeserializationState(kState_None),
			FileName(""),
			FilePath("")
		{
			SME_ASSERT(SourceFilePath && RepositoryPath);

			if (SourceFile->Construct(SourceFilePath, OverwriteExisting) == false)
			{
				DeserializationState = kState_Bad;
				SerializationState = kState_Bad;
			}

			FilePath = SourceFilePath;

			while (FilePath.find("\\\\") != -1)
				FilePath.replace(FilePath.find("\\\\"), 2, "\\");

			std::string RepoPath(RepositoryPath);
			while (RepoPath.find("\\\\") != -1)
				RepoPath.replace(RepoPath.find("\\\\"), 2, "\\");

			SME_ASSERT(FilePath.find(RepoPath) == 0);
			FileName = FilePath.substr(RepoPath.length());
			FileName = FileName.substr(0, FileName.rfind("."));
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
				return nullptr;
			else
			{
				PrefabObjectPreviewData* Data = new PrefabObjectPreviewData;
				if (Instantiator->CreatePreviewNode(Serializer, PreviewControl, Data->BaseForms, Data->References, &Data->RootNode))
					return Data;
				else
				{
					delete Data;
					return nullptr;
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

		ObjectPrefabManager				ObjectPrefabManager::Instance;
		const char*						ObjectPrefabManager::kPrefabFileExtension = "cseprefab";
		const bgsee::ResourceLocation	ObjectPrefabManager::kRepositoryPath(CSE_PREFABDEPOT);

#define IDC_OBJECTPREFAB_FILTERINPUTTIMERID		0x200
#define IDC_OBJECTPREFAB_PREVIEWTIMERID			0x201

		INT_PTR CALLBACK ObjectPrefabManager::DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
						Instance.Renderer->Present();

					break;
				}

				break;
			case WM_NOTIFY:
				{
					NMHDR* NotificationData = (NMHDR*)lParam;

					switch (NotificationData->code)
					{
					case LVN_COLUMNCLICK:
						if (NotificationData->hwndFrom == LoadedPrefabsList)
						{
							LONG_PTR SortOrder = GetWindowLongPtr(LoadedPrefabsList, GWL_USERDATA);
							ListView_SortItems(LoadedPrefabsList, SortComparator, SortOrder);
							SortOrder = SortOrder == NULL;
							SetWindowLongPtr(LoadedPrefabsList, GWL_USERDATA, SortOrder);
						}

						break;
					case LVN_KEYDOWN:
						if (NotificationData->hwndFrom == LoadedPrefabsList)
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
							if (Instance.RefreshingList)
								break;

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
									Instance.CurrentSelection = nullptr;
									SAFEDELETE(Instance.PreviewData);

									PrefabObject* NewSelection = (PrefabObject*)ChangeData->lParam;
									if (NewIndex != -1 && NewSelection)
									{
										Instance.CurrentSelection = NewSelection;

										NewSelection->SetInDialog(hWnd);
										Instance.PreviewData = NewSelection->GeneratePreviewData(Instance.Renderer);
										if (Instance.PreviewData == nullptr)
											BGSEECONSOLE_MESSAGE("Couldn't generate preview data for prefab %s", NewSelection->FileName.c_str());
										else
											Instance.PreviewData->Attach(Instance.Renderer);
									}
									else
										TESListView::ClearItems(PrefabDetailsList);

									Instance.Renderer->Present();
								}
								else if (ChangeData->hdr.hwndFrom == PrefabDetailsList)
								{
									// toggle the selection box the ref
									Result = TRUE;
									int NewIndex = ChangeData->iItem;
									if (NewIndex != -1 && Instance.PreviewData)
									{
										if (NewIndex < Instance.PreviewData->References.size())
										{
											TESObjectREFR* Selection = Instance.PreviewData->References[NewIndex];
											for (auto Itr : Instance.PreviewData->References)
												Itr->ToggleSelectionBox(false);

											Selection->ToggleSelectionBox(true);
										}
									}
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

		int CALLBACK ObjectPrefabManager::SortComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
		{
			PrefabObject* One = (PrefabObject*)lParam1;
			PrefabObject* Two = (PrefabObject*)lParam2;

			SME_ASSERT(One && Two);

			int Compare = _stricmp(One->FileName.c_str(), Two->FileName.c_str());
			if (lParamSort)
				Compare *= -1;

			return Compare;
		}

		void ObjectPrefabManager::InitializeDialog(HWND Dialog)
		{
			HWND LoadedPrefabsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_LOADEDPREFABSLIST);
			HWND PrefabDetailsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_CURRENTDETAILSLIST);

			TESListView::AddColumnHeader(LoadedPrefabsList, 0, "Prefab Collection", 200);

			TESListView::AddColumnHeader(PrefabDetailsList, 0, "Reference", 100);
			TESListView::AddColumnHeader(PrefabDetailsList, 1, "Position", 1);
			TESListView::AddColumnHeader(PrefabDetailsList, 2, "Rotation", 1);
			TESListView::AddColumnHeader(PrefabDetailsList, 3, "Scale", 1);
			TESListView::AddColumnHeader(PrefabDetailsList, 4, "Base Form", 150);
			TESListView::AddColumnHeader(PrefabDetailsList, 5, "Type", 100);
			TESListView::AddColumnHeader(PrefabDetailsList, 6, "Existing Base Form", 25);
			TESListView::AddColumnHeader(PrefabDetailsList, 7, "Enable State Parent", 100);
			TESListView::AddColumnHeader(PrefabDetailsList, 8, "Parent Opposite State", 25);

			MainDialog = Dialog;

			ExtraDataList = TESDialog::CreateDialogExtraDataList(Dialog);
			TESRenderControl::Parameters Params = { 0 };
			Params.previewOutputCtrlID = IDC_OBJECTPREFAB_PREVIEW;
			Params.renderTargetWidth = Params.renderTargetHeight = 1024.f;
			Renderer = TESPreviewControl::CreatePreviewControl(Dialog, &Params);
			TimeCounter = -1;
			Renderer->RemoveGroundPlane();

			SetTimer(Dialog, IDC_OBJECTPREFAB_FILTERINPUTTIMERID, 500, nullptr);
			SetTimer(Dialog, IDC_OBJECTPREFAB_PREVIEWTIMERID, 5, nullptr);

			ReloadPrefabs();

			TESDialog::AddDialogToOpenList(Dialog);
		}

		void ObjectPrefabManager::DeinitializeDialog(HWND Dialog)
		{
			HWND LoadedPrefabsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_LOADEDPREFABSLIST);
			HWND PrefabDetailsList = GetDlgItem(Dialog, IDC_OBJECTPREFAB_CURRENTDETAILSLIST);

			TESListView::ClearColumnHeaders(LoadedPrefabsList);
			TESListView::ClearColumnHeaders(PrefabDetailsList);

			SAFEDELETE(PreviewData);
			CurrentSelection = nullptr;

			MainDialog = nullptr;

			TimeCounter = -1;
			Renderer = nullptr;
			ExtraDataList = nullptr;

			LoadedPrefabs.clear();

			TESDialog::DestroyDialogExtraDataList(Dialog);

			KillTimer(Dialog, IDC_OBJECTPREFAB_FILTERINPUTTIMERID);
			KillTimer(Dialog, IDC_OBJECTPREFAB_PREVIEWTIMERID);

			TESDialog::RemoveDialogFromOpenList(Dialog);
		}

		void ObjectPrefabManager::RefreshPrefabList()
		{
			SME_ASSERT(MainDialog);

			HWND LoadedPrefabsList = GetDlgItem(MainDialog, IDC_OBJECTPREFAB_LOADEDPREFABSLIST);
			HWND FilterBox = GetDlgItem(MainDialog, IDC_FILTERBOX);

			SME_ASSERT(RefreshingList == false);
			RefreshingList = true;

			TESListView::ClearItems(LoadedPrefabsList);
			SAFEDELETE(PreviewData);
			CurrentSelection = nullptr;

			char Buffer[0x100] = { 0 };
			GetWindowText(FilterBox, Buffer, sizeof(Buffer));
			std::string Filter(Buffer);
			SME::StringHelpers::MakeLower(Filter);
			bool SelectDefault = false;

			for (const auto& Itr : LoadedPrefabs)
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

		void ObjectPrefabManager::LoadPrefabsInDirectory(const char* DirectoryPath)
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
						std::unique_ptr<PrefabObject> Prefab(new PrefabObject(FilePath.c_str(), kRepositoryPath().c_str(), false));

						if (Prefab->DeserializationState == PrefabObject::kState_Bad || Prefab->Deserialize() == PrefabObject::kState_Bad)
							BGSEECONSOLE_MESSAGE("Couldn't load prefab collection at %s", FilePath.c_str());
						else
							LoadedPrefabs.emplace_back(std::move(Prefab));
					}
				}
			}
		}

		void ObjectPrefabManager::ReloadPrefabs()
		{
			SME_ASSERT(MainDialog);

			SAFEDELETE(PreviewData);
			CurrentSelection = nullptr;

			LoadedPrefabs.clear();
			LoadPrefabsInDirectory(kRepositoryPath().c_str());

			RefreshPrefabList();
		}

		void ObjectPrefabManager::NewPrefab()
		{
			SME_ASSERT(MainDialog);

			if (_RENDERSEL->selectionCount == 0)
			{
				BGSEEUI->MsgBoxE(MainDialog, MB_OK, "The Render Window Selection is empty.");
				return;
			}

			std::string FilePath, FileName;
			if (ShowFileDialog(true, FilePath, FileName))
			{
				bgsee::FormListT Selection;
				bool Result = true;
				for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
				{
					TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
					if (Ref == nullptr)
					{
						BGSEECONSOLE_MESSAGE("Object %08X is not a reference!", Itr->Data->formID);
						Result = false;
					}

					if (Result)
						Selection.push_back(new TESFormWrapper(Ref));
				}

				if (Result)
				{
					PrefabObject* ExistingPrefab = nullptr;
					std::unique_ptr<PrefabObject> NewPrefab;
					bool PrefabExists = false;

					if ((PrefabExists = GetExistingPrefab(FilePath.c_str(), &ExistingPrefab)) == false)
						NewPrefab.reset(new PrefabObject(FilePath.c_str(), kRepositoryPath().c_str(), false));
					else
					{
						// create a temp file to check if the serialization works out
						// so the old file will not get overwritten if there are errors
						std::string TempFilePath(kRepositoryPath());
						TempFilePath += "TempPrefab.";
						TempFilePath += kPrefabFileExtension;

						NewPrefab.reset(new PrefabObject(TempFilePath.c_str(), kRepositoryPath().c_str(), true));
					}

					if (NewPrefab->Serialize(Selection, true) == PrefabObject::kState_Good)
					{
						if (PrefabExists)
						{
							// delete temp file and update the existing prefab
							NewPrefab->SourceFile->Delete();
							ExistingPrefab->Serialize(Selection, true);
							SME_ASSERT(ExistingPrefab->SerializationState == PrefabObject::kState_Good);
							ExistingPrefab->Deserialize(true);
							SME_ASSERT(ExistingPrefab->DeserializationState == PrefabObject::kState_Good);
						}
						else
							LoadedPrefabs.emplace_back(std::move(NewPrefab));

						RefreshPrefabList();
					}
					else
					{
						Result = false;
						if (PrefabExists)
							NewPrefab->SourceFile->Delete();
					}
				}

				if (Result == false)
				{
					BGSEEUI->MsgBoxE(MainDialog, MB_OK,
									 "Couldn't create a prefab collection from the current Render Window selection.\n\nCheck the console for more information.");
				}

				for (auto Itr : Selection)
					delete Itr;

				Selection.clear();
			}
		}

		void ObjectPrefabManager::InstatiateSelection()
		{
			SME_ASSERT(MainDialog);

			if (CurrentSelection)
			{
				achievements::kPowerUser->UnlockTool(achievements::AchievementPowerUser::kTool_ObjectPrefabs);
				if (CurrentSelection->Instantiate() == false)
					BGSEEUI->MsgBoxE(MainDialog, MB_OK, "Couldn't instantiate the current selection.\n\nCheck the console for more information.");
				else
				{
					// the selection should contain the new refs, raise the PlaceRef event
					for (TESRenderSelection::SelectedObjectsEntry* Itr = _RENDERSEL->selectionList; Itr && Itr->Data; Itr = Itr->Next)
					{
						TESObjectREFR* Ref = CS_CAST(Itr->Data, TESForm, TESObjectREFR);
						events::dialog::renderWindow::kPlaceRef.HandlePlaceRef(Ref);
					}
				}
			}
		}

		bool ObjectPrefabManager::ShowFileDialog(bool Save, std::string& OutPath, std::string& OutName)
		{
			char SelectPath[MAX_PATH] = { 0 };
			char FilterString[0x100] = { 0 };

			// ### remember to change the filter string manually if the extension changes
			if (TESDialog::ShowFileSelect(MainDialog,
				kRepositoryPath().c_str(),
				"Object Prefab Files\0*.cseprefab\0\0",
				"Select Object Prefab File",
				nullptr,
				nullptr,
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

		bool ObjectPrefabManager::GetExistingPrefab(const char* FilePath, PrefabObject** Out)
		{
			for (const auto& Itr : LoadedPrefabs)
			{
				if (!_stricmp(Itr->FilePath.c_str(), FilePath))
				{
					*Out = Itr.get();
					return true;
				}
			}

			return false;
		}

		bool ObjectPrefabManager::RemoveLoadedPrefab(PrefabObject* Data)
		{
			SME_ASSERT(Data);

			for (PrefabObjectArrayT::iterator Itr = LoadedPrefabs.begin(); Itr != LoadedPrefabs.end(); Itr++)
			{
				if ((*Itr).get() == Data)
				{
					if (DeleteFile((*Itr)->FilePath.c_str()) == FALSE)
					{
						BGSEECONSOLE_ERROR("Couldn't delete prefab file %s", (*Itr)->FileName.c_str());
						return false;
					}

					LoadedPrefabs.erase(Itr);
					if (CurrentSelection == Data)
						CurrentSelection = nullptr;

					return true;
				}
			}

			return false;
		}

		void ObjectPrefabManager::RemoveSelection()
		{
			if (CurrentSelection)
			{
				if (BGSEEUI->MsgBoxW(MainDialog,
					MB_YESNO,
					"Delete prefab file '%s'? This operation cannot be undone.", CurrentSelection->FileName.c_str()) == IDYES)
				{
					if (RemoveLoadedPrefab(CurrentSelection))
						ReloadPrefabs();
				}
			}
		}

		ObjectPrefabManager::ObjectPrefabManager() :
			LoadedPrefabs(),
			CurrentSelection(nullptr),
			PreviewData(nullptr),
			MainDialog(nullptr),
			TimeCounter(0),
			ExtraDataList(nullptr),
			Renderer(nullptr),
			RefreshingList(false)
		{
			;//
		}

		ObjectPrefabManager::~ObjectPrefabManager()
		{
			Close();
		}

		void ObjectPrefabManager::Show()
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

				if (settings::dialogs::kShowMainWindowsInTaskbar.GetData().i)
				{
					bgsee::WindowStyler::StyleData RegularAppWindow = { 0 };
					RegularAppWindow.Extended = WS_EX_APPWINDOW;
					RegularAppWindow.ExtendedOp = bgsee::WindowStyler::StyleData::kOperation_OR;

					BGSEEUI->GetWindowStyler()->StyleWindow(Dialog, RegularAppWindow);
				}
			}
		}

		void ObjectPrefabManager::Close()
		{
			if (MainDialog)
				DestroyWindow(MainDialog);
		}
	}
}