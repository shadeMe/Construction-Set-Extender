#include "CSEWrappers.h"

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

}