#include "Wrappers.h"
#include "WorkspaceManager.h"

namespace cse
{
	TESFormWrapper::TESFormWrapper( TESForm* Form ) :
		bgsee::FormWrapper(),
		WrappedForm(Form)
	{
		SME_ASSERT(WrappedForm);
	}

	TESFormWrapper::~TESFormWrapper()
	{
		WrappedForm = nullptr;
	}

	UInt32 TESFormWrapper::GetFormID( void ) const
	{
		return WrappedForm->formID;
	}

	const char* TESFormWrapper::GetEditorID( void ) const
	{
		return WrappedForm->GetEditorID();
	}

	UInt8 TESFormWrapper::GetType( void ) const
	{
		return WrappedForm->formType;
	}

	const char* TESFormWrapper::GetTypeString( void ) const
	{
		return WrappedForm->GetTypeIDString();
	}

	UInt32 TESFormWrapper::GetFlags( void ) const
	{
		return WrappedForm->formFlags;
	}

	bool TESFormWrapper::GetIsDeleted( void ) const
	{
		return (WrappedForm->formFlags & TESForm::kFormFlags_Deleted);
	}

	TESForm* TESFormWrapper::GetWrappedForm( void ) const
	{
		return WrappedForm;
	}

	void TESFileWrapper::CreateTempFile( void )
	{
		if (PluginPath.length())
		{
			DeleteFile(PluginPath.c_str());
			DeleteFile((std::string(PluginPath + ".tes")).c_str());

			BSFile* TempFile = BSFile::CreateInstance(PluginPath.c_str(), NiFile::kFileMode_WriteOnly);
			TempFile->DeleteInstance();
		}
	}

	TESFileWrapper::TESFileWrapper() :
		bgsee::PluginFileWrapper(),
		WrappedPlugin(nullptr),
		PluginPath("")
	{
		;//
	}

	TESFileWrapper::~TESFileWrapper()
	{
		if (WrappedPlugin)
			WrappedPlugin->DeleteInstance();
	}

	bool TESFileWrapper::Construct(const char* FileName, bool OverwriteExisting)
	{
		SME_ASSERT(WrappedPlugin == nullptr);

		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s\\%s", BGSEEWORKSPACE->GetCurrentWorkspace(), FileName);
		PluginPath = Buffer;

		if (OverwriteExisting)
			CreateTempFile();

		WrappedPlugin = TESFile::CreateInstance(BGSEEWORKSPACE->GetCurrentWorkspace(), FileName);
		if (WrappedPlugin)
			WrappedPlugin->SetFileIndex(0);

		return WrappedPlugin != nullptr;
	}

	void TESFileWrapper::Purge( void )
	{
		SME_ASSERT(WrappedPlugin);

		CreateTempFile();
	}

	bool TESFileWrapper::Open( bool ForWriting )
	{
		SME_ASSERT(WrappedPlugin);

		if (ForWriting)
			WrappedPlugin->CreateTempFile();

		WrappedPlugin->Open();

		UInt8 ErrorState = WrappedPlugin->errorState;
		return (ErrorState == TESFile::kFileState_None || ErrorState == TESFile::kFileState_NoHeader);
	}

	bool TESFileWrapper::SaveHeader( void )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->SaveHeader() == TESFile::kFileState_None;
	}

	bool TESFileWrapper::CorrectHeader( UInt32 RecordCount )
	{
		SME_ASSERT(WrappedPlugin);

		WrappedPlugin->fileHeader.numRecords = RecordCount;
		return WrappedPlugin->CorrectHeader() == TESFile::kFileState_None;
	}

	bool TESFileWrapper::Close( void )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->Close();
	}

	UInt8 TESFileWrapper::GetRecordType( void )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->GetRecordType();
	}

	bool TESFileWrapper::GetNextRecord( bool SkipIgnoredRecords )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->GetNextRecord(SkipIgnoredRecords);
	}

	int TESFileWrapper::GetErrorState( void ) const
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->errorState;
	}

	TESFile* TESFileWrapper::GetWrappedPlugin( void ) const
	{
		return WrappedPlugin;
	}

	const char* TESFileWrapper::GetFileName(void) const
	{
		SME_ASSERT(WrappedPlugin);

		return PluginPath.c_str();
	}

	void TESFileWrapper::Delete(void)
	{
		SME_ASSERT(WrappedPlugin);

		Close();
		DeleteFile(PluginPath.c_str());
	}

	void IFormCollectionSerializer::FreeBuffer(void)
	{
		for (auto Itr : LoadedFormBuffer)
		{
			// clean up any cross refs that may have accumulated to suppress the deletion conformation dialog
			Itr->CleanupCrossReferenceList();
			Itr->DeleteInstance();
		}

		LoadedFormBuffer.clear();
	}

	void IFormCollectionSerializer::SaveForm(bgsee::PluginFileWrapper* File, bgsee::FormWrapper* Form)
	{
		TESFormWrapper* FormW = dynamic_cast<TESFormWrapper*>(Form);
		TESFileWrapper* FileW = dynamic_cast<TESFileWrapper*>(File);

		SME_ASSERT(FormW && FormW->GetWrappedForm() && FileW && FileW->GetWrappedPlugin());

		FormW->GetWrappedForm()->SaveFormRecord(FileW->GetWrappedPlugin());

#ifdef _DEBUG
		BGSEECONSOLE_MESSAGE("Wrote form %s %08X to buffer", Form->GetEditorID(), Form->GetFormID());
#endif
	}

	IFormCollectionSerializer::IFormCollectionSerializer() :
		bgsee::FormCollectionSerializer(),
		LoadedFormBuffer()
	{
		;//
	}

	IFormCollectionSerializer::~IFormCollectionSerializer()
	{
		FreeBuffer();
	}
	const char* IFormCollectionSerializer::kSigilDefaultForm = "CSE_SERIALIZER_DEFAULTFORM";
	const char* IFormCollectionSerializer::kSigilObjectRef = "CSE_SERIALIZER_OBJECTREF";

	UInt8 IFormCollectionSerializer::GetFileSerializerType(bgsee::PluginFileWrapper* File)
	{
		TESFileWrapper* CSEFile = dynamic_cast<TESFileWrapper*>(File);
		SME_ASSERT(CSEFile);

		TESFile* Wrapped = CSEFile->GetWrappedPlugin();
		SME_ASSERT(Wrapped);

		// refresh the file header
		SME_ASSERT(Wrapped->bsFile == nullptr);

		CSEFile->Open(false);
		CSEFile->Close();

		UInt8 Result = kSerializer_Unknown;
		if (Wrapped->authorName.c_str())
		{
			if (_stricmp(Wrapped->authorName.c_str(), kSigilDefaultForm) == 0)
				Result = kSerializer_DefaultForm;
			else if (_stricmp(Wrapped->authorName.c_str(), kSigilObjectRef) == 0)
				Result = kSerializer_ObjectRef;
		}

		return Result;
	}

	void IFormCollectionSerializer::SetFileSerializerType(bgsee::PluginFileWrapper* File, UInt8 Type)
	{
		TESFileWrapper* CSEFile = dynamic_cast<TESFileWrapper*>(File);
		SME_ASSERT(CSEFile);
		SME_ASSERT(Type > kSerializer_Unknown && Type < kSerializer__MAX);

		TESFile* Wrapped = CSEFile->GetWrappedPlugin();
		SME_ASSERT(Wrapped);

		switch (Type)
		{
		case kSerializer_DefaultForm:
			Wrapped->authorName.Set(kSigilDefaultForm);
			break;
		case kSerializer_ObjectRef:
			Wrapped->authorName.Set(kSigilObjectRef);
			break;
		}
	}

	bool IFormCollectionSerializer::GetFormInBuffer(TESForm* Form) const
	{
		SME_ASSERT(Form);

		for (auto Itr : LoadedFormBuffer)
		{
			if (Itr == Form)
				return true;
		}

		return false;
	}

	DefaultFormCollectionInstantiator::~DefaultFormCollectionInstantiator()
	{
		;//
	}

	bool DefaultFormCollectionInstantiator::Instantiate(IFormCollectionSerializer* Data, bool FreeTempData)
	{
		SME_ASSERT(Data);
		SME_ASSERT(Data->GetType() == IFormCollectionSerializer::kSerializer_DefaultForm);

		DefaultFormCollectionSerializer* Serializer = dynamic_cast<DefaultFormCollectionSerializer*>(Data);
		SME_ASSERT(Serializer);

		if (Serializer->LoadedFormBuffer.size())
		{
			SME_ASSERT((*Serializer->LoadedFormBuffer.begin())->IsReference() == false);

			bool ReplaceAll = false;
			for (auto Itr : Serializer->LoadedFormBuffer)
			{
				TESForm* TempForm = Itr;
				bool FormExists = false;
				TESForm* CurrentForm = TESForm::LookupByEditorID(TempForm->GetEditorID());
				SME_ASSERT(TempForm->IsReference() == false);

				if (CurrentForm == nullptr)
					CurrentForm = TESForm::CreateInstance(TempForm->formType);
				else
				{
					FormExists = true;

					if (TempForm->formType != CurrentForm->formType)
					{
						BGSEECONSOLE_MESSAGE("Couldn't deserialize form %s %08X - A form with the same editorID but different type exists!",
											 TempForm->GetEditorID(), TempForm->formID);
						continue;
					}
					else if (ReplaceAll == false)
					{
						bool Skip = false;
						switch (BGSEEUI->MsgBoxI(nullptr,
							MB_TASKMODAL | MB_SETFOREGROUND | MB_YESNOCANCEL,
							"Form %s already exists. Do you wish to replace it?\n\n\"Cancel\" will replace all existing forms.", CurrentForm->GetEditorID()))
						{
						case IDNO:
							Skip = true;
							break;
						case IDCANCEL:
							ReplaceAll = true;
							break;
						}

						if (Skip)
							continue;
					}
				}

				CurrentForm->CopyFrom(TempForm);
				CurrentForm->SetFromActiveFile(true);

				if (FormExists == false)
				{
					if (TempForm->GetEditorID())
						CurrentForm->SetEditorID(TempForm->GetEditorID());

					_DATAHANDLER->AddForm(CurrentForm);
				}
			}
		}

		if (FreeTempData)
			Serializer->FreeBuffer();

		return true;
	}

	bool DefaultFormCollectionSerializer::LoadForm(bgsee::PluginFileWrapper* File)
	{
		TESFileWrapper* Wrapper = dynamic_cast<TESFileWrapper*>(File);
		bool Result = true;

		SME_ASSERT(Wrapper && Wrapper->GetWrappedPlugin());

		UInt8 CurrentType = Wrapper->GetRecordType();
		SME_ASSERT(CurrentType != TESForm::kFormType_REFR && CurrentType != TESForm::kFormType_ACHR && CurrentType != TESForm::kFormType_ACRE);

		switch (CurrentType)
		{
		case TESForm::kFormType_None:
		case TESForm::kFormType_TES4:
		case TESForm::kFormType_Group:
		case TESForm::kFormType_TOFT:
			break;
		default:
			{
				TESForm* TempForm =  TESForm::CreateInstance(Wrapper->GetRecordType());
				SME_ASSERT(TempForm);

				TempForm->MarkAsTemporary();
				if (TempForm->LoadForm(Wrapper->GetWrappedPlugin()) == false)
				{
					BGSEECONSOLE_MESSAGE("Couldn't load form %08X!", Wrapper->GetWrappedPlugin()->currentRecord.recordID);
					TempForm->DeleteInstance();
					Result = false;
				}
				else
				{
#ifdef _DEBUG
					BGSEECONSOLE_MESSAGE("Read form %s %08X from buffer", TempForm->GetEditorID(), TempForm->formID);
#endif
					// save it later, for ze Precious...
					TempForm->LinkForm();
					LoadedFormBuffer.push_back(TempForm);
				}
			}

			break;
		}

		return Result;
	}

	DefaultFormCollectionSerializer::~DefaultFormCollectionSerializer()
	{
		;//
	}

	bool DefaultFormCollectionSerializer::Serialize(bgsee::FormListT& Forms, bgsee::PluginFileWrapper* OutputStream)
	{
		bool FoundRefs = false;
		bool Result = false;

		SME_ASSERT(ThreadLocalData::Get()->saveLoadInProgress == 0);
		ThreadLocalData::Get()->saveLoadInProgress = 1;
		{

			for (auto Itr : Forms)
			{
				if (Itr->GetType() == TESForm::kFormType_REFR ||
					Itr->GetType() == TESForm::kFormType_ACHR ||
					Itr->GetType() == TESForm::kFormType_ACRE)
				{
					FoundRefs = true;
					break;
				}
			}

			if (FoundRefs)
				BGSEECONSOLE_MESSAGE("Form type mismatch! Selection contains object reference forms");
			else
				Result = true;

			if (Result && Forms.size())
			{
				bool FormCheck = true;
				for (auto Itr : Forms)
				{
					if (Itr->GetEditorID() && strlen(Itr->GetEditorID()) < 1)
					{
						BGSEECONSOLE_MESSAGE("Form editorID error! Invalid editorID");
						FormCheck = false;
						break;
					}
					else if (Itr->GetType() != (*Forms.begin())->GetType())
					{
						BGSEECONSOLE_MESSAGE("Form type error! Selection contains multiple types");
						FormCheck = false;
						break;
					}
					else if (Itr->GetIsDeleted())
					{
						BGSEECONSOLE_MESSAGE("Attempting to serialize deleted form %08X!", Itr->GetFormID());
						FormCheck = false;
						break;
					}
				}

				Result = FormCheck;
				if (FormCheck)
				{
					bool Failed = false;

					while (true)
					{
						OutputStream->Purge();

						if (OutputStream->Open(true) == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to open buffer!");
							Failed = true;
							break;
						}

						SetFileSerializerType(OutputStream, kSerializer_DefaultForm);
						if (OutputStream->SaveHeader() == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to save header!");
							Failed = true;
							break;
						}

						// we can skip the form sorting as they are all of a single type
						for (auto Itr : Forms)
							SaveForm(OutputStream, Itr);

						if (OutputStream->CorrectHeader(Forms.size()) == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to correct header!");
							Failed = true;
							break;
						}

						if (OutputStream->Close() == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to close buffer!");
							Failed = true;
							break;
						}

						break;
					}

					if (Failed)
					{
						Result = false;
						BGSEECONSOLE_MESSAGE("Buffer Error state = %d", OutputStream->GetErrorState());
					}

#ifdef _DEBUG
					BGSEECONSOLE_MESSAGE("Serialized %d forms to %s", Forms.size(), OutputStream->GetFileName());
#endif
				}
			}
		}
		ThreadLocalData::Get()->saveLoadInProgress = 0;
		return Result;
	}

	bool DefaultFormCollectionSerializer::Deserialize(bgsee::PluginFileWrapper* InputStream, int& OutDeserializedFormCount)
	{
		bool Result = false;

		SME_ASSERT(ThreadLocalData::Get()->saveLoadInProgress == 0);
		ThreadLocalData::Get()->saveLoadInProgress = 1;
		{
			while (true)
			{
				SME_ASSERT(GetFileSerializerType(InputStream) == kSerializer_DefaultForm);

				if (InputStream->Open(false) == false)
				{
					BGSEECONSOLE_MESSAGE("Failed to open buffer!");
					break;
				}

				FreeBuffer();
				Result = true;
				do
				{
					if (LoadForm(InputStream) == false)
						Result = false;
				} while (InputStream->GetNextRecord(true));

				OutDeserializedFormCount = LoadedFormBuffer.size();

				if (InputStream->Close() == false)
				{
					BGSEECONSOLE_MESSAGE("Failed to close buffer!");
					Result = false;
					break;
				}

				break;
			}

			if (Result == false)
				BGSEECONSOLE_MESSAGE("Buffer Error state = %d", InputStream->GetErrorState());
		}
		ThreadLocalData::Get()->saveLoadInProgress = 0;
		return Result;
	}

	UInt8 DefaultFormCollectionSerializer::GetType()
	{
		return kSerializer_DefaultForm;
	}

	void ObjectRefCollectionInstantiator::GetPositionOffset(TESObjectREFRArrayT& InData, NiNode* CameraNode, Vector3& OutOffset)
	{
		TESObjectSelection* Buffer = TESObjectSelection::CreateInstance();
		for (auto Itr : InData)
		{
			// need to generate the ref's 3D first, otherwise it won't get added to the selection
			TESBoundObject* BaseForm = CS_CAST(Itr->baseForm, TESForm, TESBoundObject);
			SME_ASSERT(BaseForm);

			NiNode* Node3D = Itr->GenerateNiNode();
			BaseForm->CalculateBounds(Node3D);
			Buffer->AddToSelection(Itr);
		}

		Buffer->CalculatePositionVectorSum();
		Buffer->CalculateBounds();

		float Multiplier = Buffer->selectionBounds + 60.f;
		if (Multiplier > 2048)
			Multiplier = 2048.f;

		Vector3 PosVecSum(Buffer->selectionPositionVectorSum);
		Vector3 NewOrigin(CameraNode->m_worldTranslate.x, CameraNode->m_worldTranslate.y, CameraNode->m_worldTranslate.z);
		Vector3 MatRot(CameraNode->m_localRotate.data[1], CameraNode->m_localRotate.data[4], CameraNode->m_localRotate.data[7]);

		MatRot.Scale(Multiplier);
		NewOrigin += MatRot;
		NewOrigin -= PosVecSum;

		Buffer->DeleteInstance();

		OutOffset = NewOrigin;
	}

	TESObject* ObjectRefCollectionInstantiator::InstantiateBaseForm(TESObjectREFR* Ref)
	{
		SME_ASSERT(Ref && Ref->baseForm);

		TESForm* TempBase = Ref->baseForm;
		TESForm* Existing = TESForm::LookupByEditorID(TempBase->GetEditorID());

		if (Existing)
		{
			// the base form was instantiated for an earlier ref
			return CS_CAST(Existing, TESForm, TESObject);
		}
#ifdef _DEBUG
		BGSEECONSOLE_MESSAGE("Instantiating base form for reference %08X", Ref->formID);
#endif
		TESForm* NewBase = TESForm::CreateInstance(TempBase->formType);
		NewBase->CopyFrom(TempBase);
		NewBase->SetFromActiveFile(true);
		NewBase->SetEditorID(TempBase->GetEditorID());
		_DATAHANDLER->AddForm(NewBase);

		return CS_CAST(NewBase, TESForm, TESObject);
	}

	ObjectRefCollectionInstantiator::~ObjectRefCollectionInstantiator()
	{
		;//
	}

	bool ObjectRefCollectionInstantiator::Instantiate(IFormCollectionSerializer* Data, bool FreeTempData /*= true*/)
	{
		SME_ASSERT(Data);
		SME_ASSERT(Data->GetType() == IFormCollectionSerializer::kSerializer_ObjectRef);

		ObjectRefCollectionSerializer* Serializer = dynamic_cast<ObjectRefCollectionSerializer*>(Data);
		SME_ASSERT(Serializer);
		bool Result = true;

		if (Serializer->LoadedFormBuffer.size())
		{
			if ((*TESRenderWindow::ActiveCell == nullptr && _TES->currentInteriorCell == nullptr) ||
				*TESRenderWindow::PathGridEditFlag ||
				*TESRenderWindow::LandscapeEditFlag)
			{
				BGSEECONSOLE_MESSAGE("Cannot instantiate references! Possible reasons: No cell loaded in the render window, pathgrid/landscape edit mode enabled");
				Result = false;
			}
			else
			{
				std::map<TESObjectREFR*, const char*> RefEditorIDMap;
				TESObjectREFRArrayT RefBuffer;

				for (auto Itr : Serializer->LoadedFormBuffer)
				{
					TESForm* TempForm = Itr;
					bool FormExists = false;

					TESObjectREFR* Ref = CS_CAST(TempForm, TESForm, TESObjectREFR);
					SME_ASSERT(Ref);

					if (Ref->baseForm == nullptr)
					{
						BGSEECONSOLE_MESSAGE("Couldn't instantiate reference %08X '%s' - Unresolved base form", TempForm->formID, TempForm->GetEditorID());
						Result = false;
						RefEditorIDMap.clear();
						break;
					}

					RefEditorIDMap[Ref] = TempForm->GetEditorID();
					RefBuffer.push_back(Ref);
				}

				if (RefEditorIDMap.size())
				{
					// maps each loaded ref instance to its actual instantiated version
					std::map<TESObjectREFR*, TESObjectREFR*> AllocationMap;

					// the refs are instantiated in the same configuration
					Vector3 NewOrigin;
					GetPositionOffset(RefBuffer, _PRIMARYRENDERER->primaryCameraParentNode, NewOrigin);

					bool RefreshRenderWindow = false;
					TESObjectCELL* Interior = _TES->currentInteriorCell;
					TESWorldSpace* Worldspace = _TES->currentWorldSpace;
					if (Interior)
						Worldspace = nullptr;

					for (std::map<TESObjectREFR*, const char*>::iterator Itr = RefEditorIDMap.begin(); Itr != RefEditorIDMap.end(); Itr++)
					{
						TESObjectREFR* TempRef = Itr->first;
						const char* EditorID = Itr->second;
						TESObject* Base = CS_CAST(TempRef->baseForm, TESForm, TESObject);
						bool NewBaseForm = false;

						if (Serializer->IsBaseFormTemporary(Base))
						{
							// instantiate base form
							Base = InstantiateBaseForm(TempRef);
							SME_ASSERT(Base);
							NewBaseForm = true;
						}

						Vector3 NewPosition(TempRef->position);
						Vector3 NewRotation(TempRef->rotation);
						NewPosition += NewOrigin;


						auto BlankForm = TESForm::CreateInstance(TESForm::kFormType_REFR);
						TESObjectREFR* NewRef = CS_CAST(BlankForm, TESForm, TESObjectREFR);
						if (NewRef == nullptr)
						{
							BGSEECONSOLE_MESSAGE("Couldn't create reference @ %0.3f, %0.3f, %0.3f, Cell = %08X, Worldspace = %08X",
												 NewPosition.x, NewPosition.y, NewPosition.z,
												 (Interior ? Interior->formID : 0),
												 (Worldspace ? Worldspace->formID : 0));
							Result = false;
						}
						else
						{
							RefreshRenderWindow = true;

							if (EditorID && TESForm::LookupByEditorID(EditorID))
							{
								BGSEECONSOLE_MESSAGE("Couldn't set editorID '%s' on instantiated reference %08X - It's already in use", EditorID, NewRef->formID);
								EditorID = nullptr;
								TempRef->SetEditorID(nullptr);
								Result = false;
							}


							// extra shenanigans for tree references - we need to generate their 3D and fixup their base form/scale before we perform the copy
							// otherwise the code path taken by the CopyFrom() call leads to the deferencing of the new ref's (non-existent) NiNode
							// not sure if this is a vanilla bug.
							if (Base->formType == TESForm::kFormType_Tree)
							{
								NewRef->SetBaseForm(TempRef->baseForm);
								NewRef->scale = TempRef->scale;
								auto New3D = (CS_CAST(TempRef->baseForm, TESForm, TESObject))->GenerateNiNode(NewRef, false);
								NewRef->SetNiNode(New3D);
							}

							NewRef->CopyFrom(TempRef);
							NewRef->SetFromActiveFile(true);

							if (NewBaseForm)
								NewRef->SetBaseForm(Base);

							// after the copy operation is complete, we need to regenerate the tree ref's 3D
							// to correctly fill its REF extradata (will cause picking issues in the render window otherwise)
							if (Base->formType == TESForm::kFormType_Tree)
							{
								NewRef->SetNiNode(nullptr);
								NewRef->SetNiNode(NewRef->GenerateNiNode());
							}


							_DATAHANDLER->PlaceObjectRef(Base, &NewPosition, &NewRotation, Interior, Worldspace, NewRef);

							AllocationMap[TempRef] = NewRef;

							if (EditorID)
								NewRef->SetEditorID(EditorID);
						}
					}

					if (AllocationMap.size())
						_RENDERSEL->ClearSelection(true);

					// update extra data ref pointers to the allocated instances
					for (auto i : AllocationMap)
					{
						TESObjectREFR* NewRef = i.second;
						TESObjectREFR* TempRef = i.first;

						ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)TempRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
						if (xParent && xParent->parent)
						{
							TESObjectREFR* NewParent = nullptr;
							for (auto j : AllocationMap)
							{
								if (j.first == xParent->parent)
								{
									NewParent = j.second;
									break;
								}
							}

							SME_ASSERT(NewParent && NewParent != xParent->parent);
							ExtraEnableStateParent* xParentNew = (ExtraEnableStateParent*)NewRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
							SME_ASSERT(xParentNew && xParentNew->parent == xParent->parent);
							xParentNew->parent = NewParent;
							NewParent->AddCrossReference(NewRef);
						}

						NewRef->GenerateNiNode();
						_RENDERSEL->AddToSelection(NewRef, true);
					}

					if (RefreshRenderWindow)
						TESRenderWindow::Redraw();
				}
			}
		}

		if (FreeTempData)
		{
			Serializer->FreeBuffer();
			Serializer->FreeDeserializationBuffers();
		}

		return Result;
	}

	bool ObjectRefCollectionInstantiator::CreatePreviewNode(ObjectRefCollectionSerializer* Data,
															  TESPreviewControl* PreviewControl,
															  TESFormArrayT& OutPreviewBaseForms,
															  TESObjectREFRArrayT& OutPreviewRefs,
															  NiNode** OutPreviewNode)
	{
		SME_ASSERT(Data && PreviewControl);

		TESObjectREFRArrayT RefBuffer;
		bool Result = true;

		for (auto Itr : Data->LoadedFormBuffer)
		{
			TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
			if (ThisRef->baseForm == nullptr)
			{
				Result = false;
				break;
			}

			RefBuffer.push_back(ThisRef);
		}

		if (Result)
		{
			NiNode* Root = *OutPreviewNode = TESRender::CreateNiNode();

			for (auto Itr : RefBuffer)
			{
				TESObjectREFR* NewRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
				SME_ASSERT(NewRef);
				NewRef->MarkAsTemporary();
				NewRef->CopyFrom(Itr);

				TESForm* NewBase = TESForm::CreateInstance(Itr->baseForm->formType);
				SME_ASSERT(NewBase);
				NewBase->MarkAsTemporary();
				NewBase->CopyFrom(Itr->baseForm);
				NewRef->SetBaseForm(NewBase);

				NiNode* Ref3D = NewRef->GenerateNiNode();
				if (Ref3D == nullptr)
				{
					NewRef->DeleteInstance();
					NewBase->DeleteInstance();

					TESRender::DeleteNiRefObject(Root);

					for (auto Itr : OutPreviewBaseForms)
						Itr->DeleteInstance();
					OutPreviewBaseForms.clear();

					for (auto Itr : OutPreviewRefs)
						Itr->DeleteInstance();
					OutPreviewRefs.clear();

					*OutPreviewNode = nullptr;

					Result = false;
					break;
				}
				else
					NewRef->SetNiNode(nullptr);

				OutPreviewRefs.push_back(NewRef);
				OutPreviewBaseForms.push_back(NewBase);
			}

			if (Result)
			{
				PreviewControl->ResetCamera();

				Vector3 PosOffset;
				GetPositionOffset(OutPreviewRefs, PreviewControl->sceneRoot, PosOffset);
				for (auto Itr : OutPreviewRefs)
				{
					Vector3 NewPos(PosOffset);
					NewPos += Itr->position;
					Itr->SetPosition(NewPos);

					Itr->SetNiNode(nullptr);
					NiNode* Ref3D = Itr->GenerateNiNode();
					SME_ASSERT(Ref3D);
					TESRender::AddToNiNode(Root, Ref3D);
				}
			}
		}

		return Result;
	}

	bool ObjectRefCollectionSerializer::LoadForm(bgsee::PluginFileWrapper* File)
	{
		TESFileWrapper* Wrapper = dynamic_cast<TESFileWrapper*>(File);
		bool Result = true;

		SME_ASSERT(Wrapper && Wrapper->GetWrappedPlugin());

		UInt8 CurrentType = Wrapper->GetRecordType();

		switch (CurrentType)
		{
		case TESForm::kFormType_None:
		case TESForm::kFormType_TES4:
		case TESForm::kFormType_Group:
		case TESForm::kFormType_TOFT:
			break;
		default:
			{
				TESForm* TempForm = nullptr;
				bool BaseForm = false;
				switch (CurrentType)
				{
				case TESForm::kFormType_REFR:
				case TESForm::kFormType_ACHR:
				case TESForm::kFormType_ACRE:
					TempForm = TESForm::CreateInstance(TESForm::kFormType_REFR);
					break;
				default:
					TempForm = TESForm::CreateInstance(Wrapper->GetRecordType());
					BaseForm = true;
					break;
				}
				SME_ASSERT(TempForm);

				TempForm->MarkAsTemporary();
				bool LoadResult = TempForm->LoadForm(Wrapper->GetWrappedPlugin());
				if (BaseForm && LoadResult == false)
				{
					BGSEECONSOLE_MESSAGE("Couldn't load form %08X!", Wrapper->GetWrappedPlugin()->currentRecord.recordID);
					TempForm->DeleteInstance();
					Result = false;
				}
				else
				{
#ifdef _DEBUG
					BGSEECONSOLE_MESSAGE("Read form %s %08X from buffer", TempForm->GetEditorID(), TempForm->formID);
#endif
					// manually resolve base form fields for refs
					// at this point, all base forms must be loaded as they are serialized before the refs
					// also, reload the position/rotation data since they are discarded when the base form is invalid
					if (BaseForm == false)
					{
						UInt32 BaseFormID = 0;
						TESFile* Plugin = Wrapper->GetWrappedPlugin();
						Result = false;
						if (Plugin->JumpToBeginningOfRecord())
						{
							TESObjectREFR* ThisRef = (TESObjectREFR*)TempForm;
							do
							{
								if (BaseFormID == 0 && Plugin->currentChunk.chunkType == 'EMAN')
									Plugin->GetChunkData4Bytes(&BaseFormID);
								else if (Plugin->currentChunk.chunkType == 'ATAD')
								{
									struct {
										Vector3 Position;
										Vector3 Rotation;
									} Buffer;

									Plugin->GetChunkData(&Buffer, 0x18);

									ThisRef->position = Buffer.Position;
									ThisRef->rotation = Buffer.Rotation;
								}
							} while (Plugin->GetNextChunk());

							SME_ASSERT(Plugin->GetNextChunk() == false);

							if (BaseFormID)
							{
								for (auto Itr : BaseFormDeserializatonBuffer)
								{
									if ((Itr->formID & 0xFFFFFF) == (BaseFormID & 0xFFFFFF))
									{
										Result = true;
										ThisRef->SetBaseForm(Itr);
										break;
									}
								}
							}
						}

						if (Result == false)
						{
							BGSEECONSOLE_MESSAGE("Couldn't resolve base form %08X for ref %08X!", BaseFormID, TempForm->formID);
							TempForm->DeleteInstance();
						}
						else
							LoadedFormBuffer.push_back(TempForm);
					}
					else
					{
						// there can only be one instance of each base form
						SME_ASSERT(IsBaseFormTemporary(TempForm->formID) == false);
						BaseFormDeserializatonBuffer.push_back(TempForm);
					}
				}
			}

			break;
		}

		return Result;
	}

	void ObjectRefCollectionSerializer::FreeDeserializationBuffers()
	{
		for (auto Itr : BaseFormDeserializatonBuffer)
			Itr->DeleteInstance();

		BaseFormDeserializatonBuffer.clear();
		ParentDeserializationBuffer.clear();
		ParentStateDeserializationBuffer.clear();
	}

	bool ObjectRefCollectionSerializer::IsBaseFormTemporary(TESForm* Form) const
	{
		SME_ASSERT(Form);
		for (auto Itr : BaseFormDeserializatonBuffer)
		{
			if (Form == Itr)
				return true;
		}

		return false;
	}

	bool ObjectRefCollectionSerializer::IsBaseFormTemporary(UInt32 FormID) const
	{
		SME_ASSERT(FormID);
		for (auto Itr : BaseFormDeserializatonBuffer)
		{
			if (FormID == Itr->formID)
				return true;
		}

		return false;
	}

	bool ObjectRefCollectionSerializer::ResolveBaseForms()
	{
		bool Result = true;

		TESFormArrayT Validated;
		for (auto Itr : LoadedFormBuffer)
		{
			TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
			SME_ASSERT(ThisRef->baseForm && ThisRef->baseForm->GetEditorID());
			SME_ASSERT(IsBaseFormTemporary(ThisRef->baseForm) == true);

			TESForm* Existing = TESForm::LookupByEditorID(ThisRef->baseForm->GetEditorID());
			bool Delinquent = false;
			if (Existing == nullptr)
			{
				if (StrictBaseFormResolution)
				{
					BGSEECONSOLE_MESSAGE("Reference %08X requires base form '%s' of type '%s'!", ThisRef->formID,
										 ThisRef->baseForm->GetEditorID(),
										 ThisRef->baseForm->GetTypeIDString());
					Delinquent = true;
				}
			}
			else if (Existing->formType != ThisRef->baseForm->formType)
			{
				BGSEECONSOLE_MESSAGE("Reference %08X base form type mismatch - Found '%s', expected '%s'!", ThisRef->formID,
									 Existing->GetTypeIDString(),
									 ThisRef->baseForm->GetTypeIDString());
				Delinquent = true;
			}

			if (Delinquent == false)
			{
				if (Existing)
					ThisRef->SetBaseForm(Existing);

				Validated.push_back(ThisRef);
			}
			else
			{
				ThisRef->DeleteInstance();
				Result = false;
			}
		}

		LoadedFormBuffer.clear();
		LoadedFormBuffer = Validated;

		return Result;
	}

	ObjectRefDescriptor* ObjectRefCollectionSerializer::GetDescriptor(TESForm* Form) const
	{
		TESObjectREFR* For = (TESObjectREFR*)Form;

		SME_ASSERT(For && For->baseForm);
		SME_ASSERT(GetFormInBuffer(For));

		ObjectRefDescriptor* Descriptor = new ObjectRefDescriptor;
		char Buffer[0x100] = { 0 };

		FORMAT_STR(Buffer, "%08X", For->formID);
		Descriptor->FormID = Buffer;

		Descriptor->EditorID = "";
		if (For->GetEditorID())
			Descriptor->EditorID = For->GetEditorID();

		FORMAT_STR(Buffer, "(%f, %f, %f)", For->position.x, For->position.y, For->position.z);
		Descriptor->Position = Buffer;

		FORMAT_STR(Buffer, "(%f, %f, %f)", For->rotation.x * REFR_RAD2DEG, For->rotation.y * REFR_RAD2DEG, For->rotation.z * REFR_RAD2DEG);
		Descriptor->Rotation = Buffer;

		FORMAT_STR(Buffer, "%f", For->scale);
		Descriptor->Scale = Buffer;

		Descriptor->BaseFormEditorID = For->baseForm->GetEditorID();
		Descriptor->BaseFormType = For->baseForm->GetTypeIDString();
		Descriptor->TemporaryBaseForm = IsBaseFormTemporary(For->baseForm);

		Descriptor->EnableStateParentFormID = "";
		Descriptor->EnableStateParentEditorID = "";
		Descriptor->ParentOppositeState = false;
		Descriptor->HasEnableStateParent = false;
		ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)For->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
		if (xParent && xParent->parent)
		{
			Descriptor->HasEnableStateParent = true;
			FORMAT_STR(Buffer, "%08X", xParent->parent->formID);
			Descriptor->EnableStateParentFormID = Buffer;

			if (xParent->parent->GetEditorID())
				Descriptor->EnableStateParentEditorID = xParent->parent->GetEditorID();

			Descriptor->ParentOppositeState = xParent->oppositeState;
		}

		return Descriptor;
	}

	ObjectRefCollectionSerializer::ObjectRefCollectionSerializer(bool StrictBaseFormResolution) :
		IFormCollectionSerializer(),
		BaseFormDeserializatonBuffer(),
		ParentDeserializationBuffer(),
		ParentStateDeserializationBuffer(),
		StrictBaseFormResolution(StrictBaseFormResolution)
	{
		;//
	}

	ObjectRefCollectionSerializer::~ObjectRefCollectionSerializer()
	{
		// refs must be freed before their base forms
		FreeBuffer();
		FreeDeserializationBuffers();
	}

	bool ObjectRefCollectionSerializer::Serialize(bgsee::FormListT& Forms, bgsee::PluginFileWrapper* OutputStream)
	{
		bool FoundNonRefs = false;
		bool Result = true;

		SME_ASSERT(ThreadLocalData::Get()->saveLoadInProgress == 0);
		ThreadLocalData::Get()->saveLoadInProgress = 1;
		{

			for (auto Itr : Forms)
			{
				if (Itr->GetType() != TESForm::kFormType_REFR &&
					Itr->GetType() != TESForm::kFormType_ACHR &&
					Itr->GetType() != TESForm::kFormType_ACRE)
				{
					FoundNonRefs = true;
					break;
				}
			}

			if (FoundNonRefs)
			{
				BGSEECONSOLE_MESSAGE("Form type mismatch! Selection contains non-object reference forms");
				Result = false;
			}

			if (Result && Forms.size())
			{
				bool FormCheck = true;
				TESFormArrayT RefBaseForms;

				for (auto Itr : Forms)
				{
					if (Itr->GetEditorID() && strlen(Itr->GetEditorID()) < 1)
					{
						BGSEECONSOLE_MESSAGE("Form editorID error! Invalid editorID");
						FormCheck = false;
						break;
					}
					else if (Itr->GetType() != (*Forms.begin())->GetType())
					{
						BGSEECONSOLE_MESSAGE("Form type error! Selection contains multiple types");
						FormCheck = false;
						break;
					}
					else if (Itr->GetIsDeleted())
					{
						BGSEECONSOLE_MESSAGE("Attempting to serialize deleted form %08X!", Itr->GetFormID());
						FormCheck = false;
						break;
					}

					TESFormWrapper* Wrapped = dynamic_cast<TESFormWrapper*>(Itr);
					SME_ASSERT(Wrapped);
					TESObjectREFR* ThisRef = (TESObjectREFR*)Wrapped->GetWrappedForm();
					if (ThisRef->baseForm == nullptr)
					{
						BGSEECONSOLE_MESSAGE("Reference %08X has no base form!", Itr->GetFormID());
						FormCheck = false;
						break;
					}
					else if (ThisRef->baseForm->GetEditorID() == nullptr || strlen(ThisRef->baseForm->GetEditorID()) < 1)
					{
						BGSEECONSOLE_MESSAGE("Reference %08X's base form has no editorID!", Itr->GetFormID());
						FormCheck = false;
						break;
					}
					else if (ThisRef->baseForm->formType == TESForm::kFormType_Door)
					{
						BGSEECONSOLE_MESSAGE("Reference %08X is a door!", Itr->GetFormID());
						FormCheck = false;
						break;
					}
#ifdef NDEBUG
					else if (ThisRef->baseForm->formID < 0x800)
					{
						BGSEECONSOLE_MESSAGE("Reference %08X's base form is a default object!", Itr->GetFormID());
						FormCheck = false;
						break;
					}
#endif
					ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)ThisRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
					if (xParent && xParent->parent)
					{
						bool MissingParent = true;
						for (auto j : Forms)
						{
							if (j->GetFormID() == xParent->parent->formID)
							{
								MissingParent = false;
								break;
							}
						}

						if (MissingParent)
						{
							BGSEECONSOLE_MESSAGE("Reference %08X's enable state parent is not a part of the selection!", Itr->GetFormID());
							FormCheck = false;
							break;
						}
					}

					bool FoundBase = false;
					for (auto ExtantBase : RefBaseForms)
					{
						if (ExtantBase == ThisRef->baseForm)
						{
							FoundBase = true;
							break;
						}
					}

					// prevent the same base from being serialized multiple times
					if (FoundBase == false)
						RefBaseForms.push_back(ThisRef->baseForm);
				}

				for (auto Base : RefBaseForms)
				{
					for (auto Ref : Forms)
					{
						if ((Ref->GetFormID() & 0xFFFFFF) == (Base->formID & 0xFFFFFF))
						{
							BGSEECONSOLE_MESSAGE("FormID collision between reference %08X and base form %08X (ignoring mod index)!", Ref->GetFormID(), Base->formID);
							FormCheck = false;
							break;
						}
					}
				}

				Result = FormCheck;
				if (FormCheck)
				{
					bool Failed = false;
					bool WarningState = BGSEECONSOLE->GetLogsWarnings();
					BGSEECONSOLE->ToggleWarningLogging(false);			// get rid of the Failed to CreateGroupData warning

					while (true)
					{
						OutputStream->Purge();

						if (OutputStream->Open(true) == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to open buffer!");
							Failed = true;
							break;
						}

						SetFileSerializerType(OutputStream, kSerializer_ObjectRef);
						if (OutputStream->SaveHeader() == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to save header!");
							Failed = true;
							break;
						}

						// serialize base forms first
						for (auto Itr : RefBaseForms)
						{
							TESFormWrapper Box(Itr);
							SaveForm(OutputStream, &Box);
						}

						// save the refs next
						for (auto Itr : Forms)
							SaveForm(OutputStream, Itr);

						if (OutputStream->CorrectHeader(Forms.size()) == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to correct header!");
							Failed = true;
							break;
						}

						if (OutputStream->Close() == false)
						{
							BGSEECONSOLE_MESSAGE("Failed to close buffer!");
							Failed = true;
							break;
						}

						break;
					}

					if (Failed)
					{
						Result = false;
						BGSEECONSOLE_MESSAGE("Buffer Error state = %d", OutputStream->GetErrorState());
					}

#ifdef _DEBUG
					BGSEECONSOLE_MESSAGE("Serialized %d forms to %s", Forms.size() * 2, OutputStream->GetFileName());
#endif
					BGSEECONSOLE->ToggleWarningLogging(WarningState);
				}
			}
		}
		ThreadLocalData::Get()->saveLoadInProgress = 0;
		return Result;
	}

	bool ObjectRefCollectionSerializer::Deserialize(bgsee::PluginFileWrapper* InputStream, int& OutDeserializedFormCount)
	{
		bool Result = false;
		bool WarningState = BGSEECONSOLE->GetLogsWarnings();
		BGSEECONSOLE->ToggleWarningLogging(false);			// skip linking/loading errors
		bool PluginError = false;

		SME_ASSERT(ThreadLocalData::Get()->saveLoadInProgress == 0);
		ThreadLocalData::Get()->saveLoadInProgress = 1;
		{

			while (true)
			{
				SME_ASSERT(GetFileSerializerType(InputStream) == kSerializer_ObjectRef);

				if (InputStream->Open(false) == false)
				{
					BGSEECONSOLE_MESSAGE("Failed to open buffer!");
					PluginError = true;
					break;
				}

				FreeBuffer();
				FreeDeserializationBuffers();
				Result = true;

				do
				{
					if (LoadForm(InputStream) == false)
						Result = false;
				} while (InputStream->GetNextRecord(true));

				// resolve extra data with formID fields
				for (auto Itr : LoadedFormBuffer)
				{
					TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
					ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)ThisRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
					if (xParent && xParent->parent)
					{
						UInt32 FormID = (UInt32)xParent->parent & 0xFFFFFF;
						for (auto j : LoadedFormBuffer)
						{
							if ((j->formID & 0xFFFFFF) == FormID)
							{
								ParentDeserializationBuffer[ThisRef] = (TESObjectREFR*)j;
								ParentStateDeserializationBuffer[ThisRef] = xParent->oppositeState;
								break;
							}
						}
					}
				}

				// link the refs
				for (auto Itr : LoadedFormBuffer)
					Itr->LinkForm();

				// link the base forms
				for (auto Itr : BaseFormDeserializatonBuffer)
					Itr->LinkForm();

				// update extra data
				// invalid extradata would've been stripped away during linking, so re-add
				for (auto Itr : ParentDeserializationBuffer)
				{
					Itr.first->extraData.ModExtraEnableStateParent(ParentDeserializationBuffer[Itr.first]);
					Itr.first->SetExtraEnableStateParentOppositeState(ParentStateDeserializationBuffer[Itr.first]);
				}

				// finally, check and update the refs' base forms
				Result = ResolveBaseForms();
				OutDeserializedFormCount = LoadedFormBuffer.size();

				if (InputStream->Close() == false)
				{
					BGSEECONSOLE_MESSAGE("Failed to close buffer!");
					Result = false;
					PluginError = true;
					break;
				}

				break;
			}

			if (PluginError)
				BGSEECONSOLE_MESSAGE("Buffer Error state = %d", InputStream->GetErrorState());

		}
		ThreadLocalData::Get()->saveLoadInProgress = 0;
		BGSEECONSOLE->ToggleWarningLogging(WarningState);
		return Result;
	}

	UInt8 ObjectRefCollectionSerializer::GetType()
	{
		return kSerializer_ObjectRef;
	}

	bool ObjectRefCollectionSerializer::GetHasTemporaryBaseForms() const
	{
		for (auto Itr : LoadedFormBuffer)
		{
			TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
			if (IsBaseFormTemporary(ThisRef->baseForm))
				return true;
		}

		return false;
	}

	void ObjectRefCollectionSerializer::GetDescription(ObjectRefCollectionDescriptorArrayT& Out) const
	{
		Out.clear();

		for (auto Itr : LoadedFormBuffer)
		{
			ObjectRefDescriptorHandleT Item(GetDescriptor(Itr));
			Out.push_back(Item);
		}
	}
}