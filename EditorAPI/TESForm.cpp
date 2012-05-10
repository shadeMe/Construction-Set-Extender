#include "TESForm.h"
#include "Core.h"

CSE_GlobalClasses::BSTCaseInsensitiveStringMap<TESForm*>*		g_TESFormEditorIDMap = (CSE_GlobalClasses::BSTCaseInsensitiveStringMap<TESForm*>*)0x009EE18C;
CSE_GlobalClasses::NiTMapBase<UInt32, TESForm*>*				g_TESFormFormIDMap = (CSE_GlobalClasses::NiTMapBase<UInt32, TESForm*>*)0x009EE164;

FormCrossReferenceData* FormCrossReferenceData::LookupFormInCrossReferenceList( FormCrossReferenceListT* CrossReferenceList, TESForm* CrossReferencedForm )
{
	for (FormCrossReferenceListT::Iterator Itr = CrossReferenceList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		FormCrossReferenceData* Data = Itr.Get();
		if (Data->Form == CrossReferencedForm)
			return Data;
	}

	return NULL;
}

FormCrossReferenceData* FormCrossReferenceData::CreateInstance(TESForm* Reference, bool IncrementRefCount)
{
	FormCrossReferenceData* NewInstance = (FormCrossReferenceData*)FormHeap_Allocate(sizeof(FormCrossReferenceData));
	NewInstance->Initialize(Reference);
	if (IncrementRefCount)		NewInstance->IncrementRefCount();
	return NewInstance;
}

void FormCrossReferenceData::DeleteInstance()
{
	FormHeap_Free(this);
}

void FormCrossReferenceData::Initialize( TESForm* Form )
{
	this->Form = Form; Count = 0;
}

UInt32 FormCrossReferenceData::GetReferenceCount() const
{
	return Count;
}

UInt32 FormCrossReferenceData::IncrementRefCount()
{
	return ++Count;
}

UInt32 FormCrossReferenceData::DecrementRefCount()
{
	if (Count-- == 0)
		Count = 0;

	return Count;
}

TESForm* FormCrossReferenceData::GetForm() const
{
	return Form;
}

bool TESForm::SetEditorID(const char* EditorID)
{
	return thisCall<bool>(0x00497670, this, EditorID);
}

TESFile* TESForm::GetOverrideFile(int Index)
{
	return thisCall<TESFile*>(0x00495FE0, this, Index);
}

void TESForm::MarkAsTemporary()
{
	thisCall<UInt32>(0x004972A0, this);
}

TESForm* TESForm::CreateInstance(UInt8 TypeID)
{
	return _DATAHANDLER->CreateForm(TypeID);
}

void TESForm::DeleteInstance(bool ReleaseMemory)
{
	thisVirtualCall<UInt32>(0x34, this, ReleaseMemory);
}

void TESForm::SetFormID(UInt32 FormID, bool Unk02)
{
	thisCall<UInt32>(0x00497E50, this, FormID, Unk02);
}

void TESForm::AddCrossReference(TESForm* Form)
{
	thisCall<UInt32>(0x00496430, this, Form);
}

void TESForm::RemoveCrossReference(TESForm* Form)
{
	thisCall<UInt32>(0x00496470, this, Form);
}

FormCrossReferenceListT* TESForm::GetCrossReferenceList(bool CreateNew)
{
	return thisCall<FormCrossReferenceListT*>(0x00496380, this, CreateNew);
}

void TESForm::CleanupCrossReferenceList()
{
	thisCall<UInt32>(0x00496400, this);
}

void TESForm::SetFromActiveFile(bool State)
{
	thisVirtualCall<UInt32>(0x94, this, State);
}

void TESForm::SetDeleted(bool State)
{
	thisVirtualCall<UInt32>(0x90, this, State);
}

bool TESForm::CopyFrom(TESForm* Form)
{
	return thisVirtualCall<bool>(0xB8, this, Form);
}

bool TESForm::CompareTo(TESForm* Form)
{
	return thisVirtualCall<bool>(0xBC, this, Form);
}

bool TESForm::UpdateUsageInfo()
{
	return thisVirtualCall<bool>(0x104, this);
}

void TESForm::Link()
{
	thisVirtualCall<UInt32>(0x70, this);
}

bool TESForm::SaveForm(TESFile* File)
{
	return thisVirtualCall<bool>(0x50, this, File);
}

TESForm* TESForm::LookupByFormID(UInt32 FormID)
{
	return cdeclCall<TESForm*>(0x00495EF0, FormID);
}

TESForm* TESForm::LookupByEditorID(const char* EditorID)
{
	return cdeclCall<TESForm*>(0x00495F20, EditorID);
}

const char* TESForm::GetTypeIDString( void )
{
	return cdeclCall<const char*>(0x004AC1B0, this->formType);
}