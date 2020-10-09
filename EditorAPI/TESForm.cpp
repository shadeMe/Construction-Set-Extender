#include "TESForm.h"
#include "Core.h"

cseOverride::NiTMapBase<UInt32, TESForm*>*				TESForm::FormIDMap = (cseOverride::NiTMapBase<UInt32, TESForm*>*)0x009EE164;
cseOverride::BSTCaseInsensitiveStringMap<TESForm*>*		TESForm::EditorIDMap = (cseOverride::BSTCaseInsensitiveStringMap<TESForm*>*)0x009EE18C;
TESForm::FormTypeInfo*																	TESForm::FormTypeInfoTable = (TESForm::FormTypeInfo*)0x009EDE28;

const char* TESForm::FormTypeIDLongNames[kFormType__MAX] =
{
	"None",
	"TES4",
	"Group",
	"GMST",
	"Global",
	"Class",
	"Faction",
	"Hair",
	"Eyes",
	"Race",
	"Sound",
	"Skill",
	"Effect",
	"Script",
	"Land Texture",
	"Enchantment",
	"Spell",
	"BirthSign",
	"Activator",
	"Apparatus",
	"Armor",
	"Book",
	"Clothing",
	"Container",
	"Door",
	"Ingredient",
	"Light",
	"Misc Item",
	"Static",
	"Grass",
	"Tree",
	"Flora",
	"Furniture",
	"Weapon",
	"Ammo",
	"NPC",
	"Creature",
	"Leveled Creature",
	"SoulGem",
	"Key",
	"Alchemy Item",
	"SubSpace",
	"Sigil Stone",
	"Leveled Item",
	"SNDG",
	"Weather",
	"Climate",
	"Region",
	"Cell",
	"Reference",
	"Reference",			// ACHR
	"Reference",			// ACRE
	"PathGrid",
	"World Space",
	"Land",
	"TLOD",
	"Road",
	"Dialog",
	"Dialog Info",
	"Quest",
	"Idle",
	"AI Package",
	"Combat Style",
	"Load Screen",
	"Leveled Spell",
	"Anim Object",
	"Water Type",
	"Effect Shader",
	"TOFT"
};

FormCrossReferenceData* FormCrossReferenceData::LookupFormInCrossReferenceList( FormCrossReferenceListT* CrossReferenceList, TESForm* CrossReferencedForm )
{
	for (FormCrossReferenceListT::Iterator Itr = CrossReferenceList->Begin(); !Itr.End() && Itr.Get(); ++Itr)
	{
		FormCrossReferenceData* Data = Itr.Get();
		if (Data->Form == CrossReferencedForm)
			return Data;
	}

	return nullptr;
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
	if (IsTemporary() && EditorID == nullptr)
	{
		this->editorID.Clear();
		return true;
	}

	SME_ASSERT(EditorID);
	return thisCall<bool>(0x00497670, this, EditorID);
}

TESFile* TESForm::GetOverrideFile(int Index) const
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

void TESForm::DeleteInstance()
{
	thisVirtualCall<UInt32>(0x34, this, true);
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

void TESForm::CopyFrom(TESForm* Form)
{
	thisVirtualCall<void>(0xB8, this, Form);
}

bool TESForm::CompareTo(TESForm* Form)
{
	return thisVirtualCall<bool>(0xBC, this, Form);
}

bool TESForm::UpdateUsageInfo()
{
	return thisVirtualCall<bool>(0x104, this);
}

void TESForm::LinkForm()
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
	return GetFormTypeIDLongName(this->formType);
}

bool TESForm::IsReference() const
{
	return formType >= kFormType_REFR && formType <= kFormType_ACRE;
}

bool TESForm::IsActive() const
{
	return (formFlags & kFormFlags_FromActiveFile);
}

bool TESForm::IsQuestItem() const
{
	return (formFlags & kFormFlags_QuestItem);
}

void TESForm::SetQuestItem( bool State )
{
	thisCall<void>(0x00494BA0, this, State);
}

void TESForm::GetDataFromDialog( HWND Dialog )
{
	thisVirtualCall<void>(0x118, this, Dialog);
}

const char* TESForm::GetFormTypeIDLongName( UInt8 TypeID )
{
	return cdeclCall<const char*>(0x004AC1B0, TypeID);
}

bool TESForm::LoadForm( TESFile* File )
{
	return thisVirtualCall<bool>(0x40, this, File);
}

bool TESForm::SaveFormRecord( TESFile* File )
{
	return thisVirtualCall<bool>(0x44, this, File);
}

const char* TESForm::GetEditorID() const
{
	return editorID.c_str();
}

void TESForm::SetDataInDialog( HWND Dialog )
{
	thisVirtualCall<void>(0x114, this, Dialog);
}

void TESForm::SetInitiallyDisabled(bool State)
{
	SME::MiscGunk::ToggleFlag(&formFlags, kFormFlags_Disabled, State);
}

void TESForm::SetVWD(bool State)
{
	SME::MiscGunk::ToggleFlag(&formFlags, kFormFlags_VisibleWhenDistant, State);
}

bool TESForm::GetFromActiveFile() const
{
	return (formFlags & kFormFlags_FromActiveFile);
}

bool TESForm::IsDeleted() const
{
	return (formFlags & kFormFlags_Deleted);
}

bool TESForm::IsTemporary() const
{
	return (formFlags & kFormFlags_Temporary);
}

bool TESForm::IsVWD() const
{
	return (formFlags & kFormFlags_VisibleWhenDistant);
}

bool TESForm::IsDisabled() const
{
	return (formFlags & kFormFlags_Disabled);
}

TESForm* TESForm::CreateTemporaryCopy(TESForm* Source, bool CopyModifiedState /*= true*/)
{
	SME_ASSERT(Source);

	TESForm* Buffer = TESForm::CreateInstance(Source->formType);
	Buffer->MarkAsTemporary();
	Buffer->CopyFrom(Source);

	if (CopyModifiedState)
		Buffer->SetFromActiveFile(Source->GetFromActiveFile());

	return Buffer;
}

void TESForm::PopulateCrossReferenceListView(HWND ListView)
{
	thisCall<void>(0x004964C0, this, ListView);
}

void TESFormIDListView::RefreshFormList(HWND ListView)
{
	return thisVirtualCall<void>(0x12C, this, ListView);
}