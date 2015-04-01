#include "CSEWrappers.h"
#include "CSEGlobalClipboard.h"
#include "Hooks\Hooks-Plugins.h"

namespace ConstructionSetExtender
{
	CSEFormWrapper::CSEFormWrapper( TESForm* Form ) :
		BGSEditorExtender::BGSEEFormWrapper(),
		WrappedForm(Form)
	{
		SME_ASSERT(WrappedForm);
	}

	CSEFormWrapper::~CSEFormWrapper()
	{
		WrappedForm = NULL;
	}

	UInt32 CSEFormWrapper::GetFormID( void ) const
	{
		return WrappedForm->formID;
	}

	const char* CSEFormWrapper::GetEditorID( void ) const
	{
		return WrappedForm->GetEditorID();
	}

	UInt8 CSEFormWrapper::GetType( void ) const
	{
		return WrappedForm->formType;
	}

	const char* CSEFormWrapper::GetTypeString( void ) const
	{
		return WrappedForm->GetTypeIDString();
	}

	UInt32 CSEFormWrapper::GetFlags( void ) const
	{
		return WrappedForm->formFlags;
	}

	bool CSEFormWrapper::GetIsDeleted( void ) const
	{
		return (WrappedForm->formFlags & TESForm::kFormFlags_Deleted);
	}

	TESForm* CSEFormWrapper::GetWrappedForm( void ) const
	{
		return WrappedForm;
	}

	void CSEPluginFileWrapper::CreateTempFile( void )
	{
		if (PluginPath.length())
		{
			DeleteFile(PluginPath.c_str());
			DeleteFile((std::string(PluginPath + ".tes")).c_str());

			BSFile* TempFile = BSFile::CreateInstance(PluginPath.c_str(), NiFile::kFileMode_WriteOnly);
			TempFile->DeleteInstance();
		}
	}

	CSEPluginFileWrapper::CSEPluginFileWrapper() :
		BGSEditorExtender::BGSEEPluginFileWrapper(),
		WrappedPlugin(NULL),
		PluginPath("")
	{
		;//
	}

	CSEPluginFileWrapper::~CSEPluginFileWrapper()
	{
		if (WrappedPlugin)
			WrappedPlugin->DeleteInstance();
	}

	bool CSEPluginFileWrapper::Construct( const char* FileName )
	{
		SME_ASSERT(WrappedPlugin == NULL);

		char Buffer[0x200] = {0};
		FORMAT_STR(Buffer, "%s\\%s", BGSEEWORKSPACE->GetCurrentWorkspace(), FileName);
		PluginPath = Buffer;

		CreateTempFile();
		WrappedPlugin = TESFile::CreateInstance(BGSEEWORKSPACE->GetCurrentWorkspace(), FileName);
		if (WrappedPlugin)
		{
			WrappedPlugin->SetFileIndex(0);
		}

		return WrappedPlugin != NULL;
	}

	void CSEPluginFileWrapper::Purge( void )
	{
		SME_ASSERT(WrappedPlugin);

		CreateTempFile();
	}

	bool CSEPluginFileWrapper::Open( bool ForWriting )
	{
		SME_ASSERT(WrappedPlugin);

		if (ForWriting)
			WrappedPlugin->CreateTempFile();

		UInt8 ErrorState = WrappedPlugin->Open();
		return (ErrorState == TESFile::kFileState_None || ErrorState == TESFile::kFileState_NoHeader);
	}

	bool CSEPluginFileWrapper::SaveHeader( void )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->SaveHeader() == TESFile::kFileState_None;
	}

	bool CSEPluginFileWrapper::CorrectHeader( UInt32 RecordCount )
	{
		SME_ASSERT(WrappedPlugin);

		WrappedPlugin->fileHeader.numRecords = RecordCount;
		return WrappedPlugin->CorrectHeader() == TESFile::kFileState_None;
	}

	bool CSEPluginFileWrapper::Close( void )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->Close();
	}

	UInt8 CSEPluginFileWrapper::GetRecordType( void )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->GetRecordType();
	}

	bool CSEPluginFileWrapper::GetNextRecord( bool SkipIgnoredRecords )
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->GetNextRecord(SkipIgnoredRecords);
	}

	int CSEPluginFileWrapper::GetErrorState( void ) const
	{
		SME_ASSERT(WrappedPlugin);

		return WrappedPlugin->errorState;
	}

	TESFile* CSEPluginFileWrapper::GetWrappedPlugin( void ) const
	{
		return WrappedPlugin;
	}

	const char* CSEPluginFileWrapper::GetFileName(void) const
	{
		SME_ASSERT(WrappedPlugin);

		return PluginPath.c_str();
	}

	void ICSEFormCollectionSerializer::FreeBuffer(void)
	{
		for each (auto Itr in LoadedFormBuffer)
			Itr->DeleteInstance();

		LoadedFormBuffer.clear();
	}

	void ICSEFormCollectionSerializer::SaveForm(BGSEditorExtender::BGSEEPluginFileWrapper* File, BGSEditorExtender::BGSEEFormWrapper* Form)
	{
		CSEFormWrapper* FormW = dynamic_cast<CSEFormWrapper*>(Form);
		CSEPluginFileWrapper* FileW = dynamic_cast<CSEPluginFileWrapper*>(File);

		SME_ASSERT(FormW && FormW->GetWrappedForm() && FileW && FileW->GetWrappedPlugin());

		FormW->GetWrappedForm()->SaveFormRecord(FileW->GetWrappedPlugin());

#ifdef _DEBUG
		BGSEECONSOLE_MESSAGE("Wrote form %s %08X to buffer", Form->GetEditorID(), Form->GetFormID());
#endif
	}

	ICSEFormCollectionSerializer::ICSEFormCollectionSerializer() :
		BGSEditorExtender::BGSEEFormCollectionSerializer(),
		LoadedFormBuffer()
	{
		;//
	}

	ICSEFormCollectionSerializer::~ICSEFormCollectionSerializer()
	{
		FreeBuffer();
	}

	UInt8 ICSEFormCollectionSerializer::GetFileSerializerType(BGSEditorExtender::BGSEEPluginFileWrapper* File)
	{
		CSEPluginFileWrapper* CSEFile = dynamic_cast<CSEPluginFileWrapper*>(File);
		SME_ASSERT(CSEFile);

		TESFile* Wrapped = CSEFile->GetWrappedPlugin();
		SME_ASSERT(Wrapped);

		UInt8 Result = kSerializer_Unknown;
		if (Wrapped->authorName.c_str())
		{
			if (_stricmp(Wrapped->authorName.c_str(), "CSE_SERIALIZER_DEFAULTFORM") == 0)
				Result = kSerializer_DefaultForm;
			else if (_stricmp(Wrapped->authorName.c_str(), "CSE_SERIALIZER_OBJECTREF") == 0)
				Result = kSerializer_ObjectRef;
		}

		return Result;
	}

	void ICSEFormCollectionSerializer::SetFileSerializerType(BGSEditorExtender::BGSEEPluginFileWrapper* File, UInt8 Type)
	{
		CSEPluginFileWrapper* CSEFile = dynamic_cast<CSEPluginFileWrapper*>(File);
		SME_ASSERT(CSEFile);
		SME_ASSERT(Type > kSerializer_Unknown && Type < kSerializer__MAX);

		TESFile* Wrapped = CSEFile->GetWrappedPlugin();
		SME_ASSERT(Wrapped);

		switch (Type)
		{
		case kSerializer_DefaultForm:
			Wrapped->authorName.Set("CSE_SERIALIZER_DEFAULTFORM");
			break;
		case kSerializer_ObjectRef:
			Wrapped->authorName.Set("CSE_SERIALIZER_OBJECTREF");
			break;
		}
	}

	CSEDefaultFormCollectionInstantiator::~CSEDefaultFormCollectionInstantiator()
	{
		;//
	}

	bool CSEDefaultFormCollectionInstantiator::Instantiate(ICSEFormCollectionSerializer* Data, bool FreeTempData)
	{
		SME_ASSERT(Data);
		SME_ASSERT(Data->GetType() == ICSEFormCollectionSerializer::kSerializer_DefaultForm);

		CSEDefaultFormCollectionSerializer* Serializer = dynamic_cast<CSEDefaultFormCollectionSerializer*>(Data);
		SME_ASSERT(Serializer);

		if (Serializer->LoadedFormBuffer.size())
		{
			SME_ASSERT((*Serializer->LoadedFormBuffer.begin())->IsReference() == false);

			bool ReplaceAll = false;
			for each (auto Itr in Serializer->LoadedFormBuffer)
			{
				TESForm* TempForm = Itr;
				bool FormExists = false;
				TESForm* CurrentForm = TESForm::LookupByEditorID(TempForm->GetEditorID());

				if (CurrentForm == NULL)
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
						switch (BGSEEUI->MsgBoxI(NULL,
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

	bool CSEDefaultFormCollectionSerializer::LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File)
	{
		CSEPluginFileWrapper* Wrapper = dynamic_cast<CSEPluginFileWrapper*>(File);
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

	CSEDefaultFormCollectionSerializer::~CSEDefaultFormCollectionSerializer()
	{
		;//
	}

	bool CSEDefaultFormCollectionSerializer::Serialize(BGSEditorExtender::BGSEEFormListT& Forms, BGSEditorExtender::BGSEEPluginFileWrapper* OutputStream)
	{
		bool FoundRefs = false;
		bool Result = false;

		for each (auto Itr in Forms)
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
			for each (auto Itr in Forms)
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
					for each (auto Itr in Forms)
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

		return Result;
	}

	bool CSEDefaultFormCollectionSerializer::Deserialize(BGSEditorExtender::BGSEEPluginFileWrapper* InputStream, int& OutDeserializedFormCount)
	{
		bool Result = false;

		while (true)
		{
			if (InputStream->Open(false) == false)
			{
				BGSEECONSOLE_MESSAGE("Failed to open buffer!");
				break;
			}

			SME_ASSERT(GetFileSerializerType(InputStream) == kSerializer_DefaultForm);

			FreeBuffer();
			Result = true;
			do
			{
				if (LoadForm(InputStream) == false)
					Result = false;
			}
			while (InputStream->GetNextRecord(true));

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

		return Result;
	}

	UInt8 CSEDefaultFormCollectionSerializer::GetType()
	{
		return kSerializer_DefaultForm;
	}

	void CSEObjectRefCollectionInstantiator::GetPositionOffset(TESObjectREFRListT& InData, NiNode* CameraNode, Vector3& OutOffset)
	{
		TESObjectSelection* Buffer = TESObjectSelection::CreateInstance();
		for each (auto Itr in InData)
		{
			// need to generate the ref's 3D first, otherwise it won't get added to the selection
			Itr->GenerateNiNode();
			Buffer->AddToSelection(Itr);
			Itr->SetNiNode(NULL);
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

	CSEObjectRefCollectionInstantiator::~CSEObjectRefCollectionInstantiator()
	{
		;//
	}

	bool CSEObjectRefCollectionInstantiator::Instantiate(ICSEFormCollectionSerializer* Data, bool FreeTempData /*= true*/)
	{
		SME_ASSERT(Data);
		SME_ASSERT(Data->GetType() == ICSEFormCollectionSerializer::kSerializer_ObjectRef);

		CSEObjectRefCollectionSerializer* Serializer = dynamic_cast<CSEObjectRefCollectionSerializer*>(Data);
		SME_ASSERT(Serializer);
		bool Result = true;

		if (Serializer->LoadedFormBuffer.size())
		{
			if ((*TESRenderWindow::ActiveCell == NULL && _TES->currentInteriorCell == NULL) ||
				*TESRenderWindow::PathGridEditFlag ||
				*TESRenderWindow::LandscapeEditFlag)
			{
				BGSEECONSOLE_MESSAGE("Cannot instantiate references! Possible reasons: No cell loaded in the render window, pathgrid/landscape edit mode enabled");
				Result = false;
			}
			else
			{
				std::map<TESObjectREFR*, const char*> RefEditorIDMap;
				TESObjectREFRListT RefBuffer;

				for each (auto Itr in Serializer->LoadedFormBuffer)
				{
					TESForm* TempForm = Itr;
					bool FormExists = false;

					TESObjectREFR* Ref = CS_CAST(TempForm, TESForm, TESObjectREFR);
					SME_ASSERT(Ref);

					if (Ref->baseForm == NULL)
					{
						BGSEECONSOLE_MESSAGE("Couldn't instantiate reference %08X '%s' - Unresolved base form", TempForm->formID, TempForm->GetEditorID());
						Result = false;
						continue;
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
						Worldspace = NULL;

					for (std::map<TESObjectREFR*, const char*>::iterator Itr = RefEditorIDMap.begin(); Itr != RefEditorIDMap.end(); Itr++)
					{
						TESObjectREFR* TempRef = Itr->first;
						TESObject* Base = CS_CAST(Itr->first->baseForm, TESForm, TESObject);
						const char* EditorID = Itr->second;
						SME_ASSERT(Base);

						Vector3 NewPosition(TempRef->position);
						Vector3 NewRotation(TempRef->rotation);
						NewPosition += NewOrigin;

						TESObjectREFR* NewRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
						if (NewRef == NULL)
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
								EditorID = NULL;
								TempRef->SetEditorID(NULL);
								Result = false;
							}

							NewRef->CopyFrom(TempRef);
							NewRef->SetFromActiveFile(true);
							_DATAHANDLER->PlaceObjectRef(Base, &NewPosition, &NewRotation, Interior, Worldspace, NewRef);

							AllocationMap[TempRef] = NewRef;

							if (EditorID)
								NewRef->SetEditorID(EditorID);
						}
					}

					// update extra data ref pointers to the allocated instances
					for each (auto i in AllocationMap)
					{
						TESObjectREFR* NewRef = i.second;
						TESObjectREFR* TempRef = i.first;

						ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)TempRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
						if (xParent && xParent->parent)
						{
							TESObjectREFR* NewParent = NULL;
							for each (auto j in AllocationMap)
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
						}
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

	bool CSEObjectRefCollectionInstantiator::CreatePreviewNode(CSEObjectRefCollectionSerializer* Data,
															  TESPreviewControl* PreviewControl,
															  TESObjectREFRListT& OutPreviewRefs,
															  NiNode** OutPreviewNode)
	{
		SME_ASSERT(Data && PreviewControl);

		TESObjectREFRListT RefBuffer;
		bool Result = true;

		for each (auto Itr in Data->LoadedFormBuffer)
		{
			TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
			if (ThisRef->baseForm == NULL)
			{
				Result = false;
				break;
			}

			RefBuffer.push_back(ThisRef);
		}

		if (Result)
		{
			Vector3 PosOffset;
			GetPositionOffset(RefBuffer, PreviewControl->cameraNode, PosOffset);
			NiNode* Root = *OutPreviewNode = TESRender::CreateNiNode();

			for each (auto Itr in RefBuffer)
			{
				TESObjectREFR* NewRef = CS_CAST(TESForm::CreateInstance(TESForm::kFormType_REFR), TESForm, TESObjectREFR);
				SME_ASSERT(NewRef);

				NewRef->MarkAsTemporary();
				NewRef->CopyFrom(Itr);

				PosOffset += NewRef->position;
				NewRef->SetPosition(PosOffset);
				NiNode* Ref3D = NewRef->GenerateNiNode();
				if (Ref3D == NULL)
				{
					TESRender::DeleteNiNode(Root);
					for each (auto Itr in OutPreviewRefs)
						Itr->DeleteInstance();
					OutPreviewRefs.clear();
					*OutPreviewNode = NULL;

					Result = false;
					break;
				}

				OutPreviewRefs.push_back(NewRef);
				TESRender::AddToNiNode(Root, Ref3D);
			}
		}

		return Result;
	}

	bool CSEObjectRefCollectionSerializer::LoadForm(BGSEditorExtender::BGSEEPluginFileWrapper* File)
	{
		CSEPluginFileWrapper* Wrapper = dynamic_cast<CSEPluginFileWrapper*>(File);
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
				TESForm* TempForm = NULL;
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
					if (BaseForm == false)
					{
						UInt32 BaseFormID = 0;
						TESFile* Plugin = Wrapper->GetWrappedPlugin();
						Result = false;
						if (Plugin->JumpToBeginningOfRecord())
						{
							do
							{
								if (BaseFormID == 0 && Plugin->currentChunk.chunkType == 'EMAN')
									Plugin->GetChunkData4Bytes(&BaseFormID);
							} while (Plugin->GetNextChunk());

							SME_ASSERT(Plugin->GetNextChunk() == false);

							if (BaseFormID)
							{
								for each (auto Itr in BaseFormDeserializatonBuffer)
								{
									if ((Itr->formID & 0xFFFFFF) == (BaseFormID & 0xFFFFFF))
									{
										Result = true;
										((TESObjectREFR*)TempForm)->SetBaseForm(Itr);
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
						BaseFormDeserializatonBuffer.push_back(TempForm);
				}
			}

			break;
		}

		return Result;
	}

	void CSEObjectRefCollectionSerializer::FreeDeserializationBuffers()
	{
		for each (auto Itr in BaseFormDeserializatonBuffer)
			Itr->DeleteInstance();

		BaseFormDeserializatonBuffer.clear();
	}

	CSEObjectRefCollectionSerializer::CSEObjectRefCollectionSerializer() :
		ICSEFormCollectionSerializer(),
		BaseFormDeserializatonBuffer()
	{
		;//
	}

	CSEObjectRefCollectionSerializer::~CSEObjectRefCollectionSerializer()
	{
		FreeDeserializationBuffers();
	}

	bool CSEObjectRefCollectionSerializer::Serialize(BGSEditorExtender::BGSEEFormListT& Forms, BGSEditorExtender::BGSEEPluginFileWrapper* OutputStream)
	{
		bool FoundNoneRefs = false;
		bool Result = true;

		for each (auto Itr in Forms)
		{
			if (Itr->GetType() != TESForm::kFormType_REFR &&
				Itr->GetType() != TESForm::kFormType_ACHR &&
				Itr->GetType() != TESForm::kFormType_ACRE)
			{
				FoundNoneRefs = true;
				break;
			}
		}

		if (FoundNoneRefs)
		{
			BGSEECONSOLE_MESSAGE("Form type mismatch! Selection contains non-object reference forms");
			Result = false;
		}

		if (Result && Forms.size())
		{
			bool FormCheck = true;
			TESFormListT RefBaseForms;

			for each (auto Itr in Forms)
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

				TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
				if (ThisRef->baseForm == NULL)
				{
					BGSEECONSOLE_MESSAGE("Reference %08X has no base form!", Itr->GetFormID());
					FormCheck = false;
					break;
				}
				else if (ThisRef->baseForm->GetEditorID() == NULL || strlen(ThisRef->baseForm->GetEditorID()) < 1)
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
				else if (ThisRef->baseForm->formID < 0x800)
				{
					BGSEECONSOLE_MESSAGE("Reference %08X's base form is a default object!", Itr->GetFormID());
					FormCheck = false;
					break;
				}

				ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)ThisRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				if (xParent && xParent->parent)
				{
					bool MissingParent = true;
					for each (auto j in Forms)
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

				RefBaseForms.push_back(ThisRef->baseForm);
			}

			for each (auto Base in RefBaseForms)
			{
				for each (auto Ref in Forms)
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
					for each (auto Itr in RefBaseForms)
					{
						CSEFormWrapper Box(Itr);
						SaveForm(OutputStream, &Box);
					}

					// save the refs next
					for each (auto Itr in Forms)
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

		return Result;
	}

	bool CSEObjectRefCollectionSerializer::Deserialize(BGSEditorExtender::BGSEEPluginFileWrapper* InputStream, int& OutDeserializedFormCount)
	{
		bool Result = false;

		while (true)
		{
			if (InputStream->Open(false) == false)
			{
				BGSEECONSOLE_MESSAGE("Failed to open buffer!");
				break;
			}

			SME_ASSERT(GetFileSerializerType(InputStream) == kSerializer_ObjectRef);

			FreeBuffer();
			FreeDeserializationBuffers();
			Result = true;

			do
			{
				if (LoadForm(InputStream) == false)
					Result = false;
			} while (InputStream->GetNextRecord(true));

			// resolve extra data with formID fields
			for each (auto Itr in LoadedFormBuffer)
			{
				TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
				ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)ThisRef->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				if (xParent && xParent->parent)
				{
					UInt32 FormID = (UInt32)xParent->parent & 0xFFFFFF;
					for each (auto j in LoadedFormBuffer)
					{
						if ((j->formID & 0xFFFFFF) == FormID)
						{
							ParentDeserializationBuffer[ThisRef] = (TESObjectREFR*)j;
							break;
						}
					}
				}
			}

			// link the refs
			for each (auto Itr in LoadedFormBuffer)
				Itr->LinkForm();

			// update extra data
			for each (auto Itr in ParentDeserializationBuffer)
			{
				ExtraEnableStateParent* xParent = (ExtraEnableStateParent*)Itr.first->extraData.GetExtraDataByType(BSExtraData::kExtra_EnableStateParent);
				SME_ASSERT(xParent && xParent->parent != Itr.second);
				xParent->parent = Itr.second;
			}

			// finally, check and update the refs' base forms
			TESFormListT Validated;
			for each (auto Itr in LoadedFormBuffer)
			{
				TESObjectREFR* ThisRef = (TESObjectREFR*)Itr;
				SME_ASSERT(ThisRef->baseForm && ThisRef->baseForm->GetEditorID());

				TESForm* Existing = TESForm::LookupByEditorID(ThisRef->baseForm->GetEditorID());
				bool Delinquent = false;
				if (Existing == NULL)
				{
					BGSEECONSOLE_MESSAGE("Reference %08X requires base form '%s' of type '%s'!", ThisRef->formID,
										 ThisRef->baseForm->GetEditorID(),
										 ThisRef->baseForm->GetTypeIDString());
					Delinquent = true;
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
					ThisRef->SetBaseForm(Existing);
					Validated.push_back(ThisRef);
				}
				else
					ThisRef->DeleteInstance();
			}

			LoadedFormBuffer.clear();
			LoadedFormBuffer = Validated;

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

		return Result;
	}

	UInt8 CSEObjectRefCollectionSerializer::GetType()
	{
		return kSerializer_ObjectRef;
	}
}