#include "Core.h"
#include "Hooks\Hooks-Renderer.h"
#include "Hooks\Hooks-LOD.h"

using namespace cse;

TESDataHandler**					TESDataHandler::Singleton = (TESDataHandler **)0x00A0E064;
bool								TESDataHandler::PluginLoadSaveInProgress = false;

TES**								TES::Singleton = (TES**)0x00A0ABB0;
FileFinder**						FileFinder::Singleton = (FileFinder**)0x00A0DE8C;
BSTexturePalette**					BSTexturePalette::Singleton = (BSTexturePalette**)0x00A10004;
BSTextureManager**					BSTextureManager::Singleton = (BSTextureManager**)0x00A8E760;

UInt8								TESLODTextureGenerator::GeneratorState = TESLODTextureGenerator::kLODDiffuseMapGeneratorState_NotInUse;
const char*							TESLODTextureGenerator::LODFullTexturePath = ".\\Data\\Textures\\LandscapeLOD\\Generated\\%i.%02i.%02i.%i.dds";
const char*							TESLODTextureGenerator::ExteriorSnapshotPathBuffer = NULL;
TESObjectCELL*						TESLODTextureGenerator::ExteriorSnapshotSource = NULL;

LPDIRECT3DTEXTURE9*					TESLODTextureGenerator::D3DTexture32x = (LPDIRECT3DTEXTURE9*)0x00A0AAC4;
LPDIRECT3DTEXTURE9*					TESLODTextureGenerator::D3DTexture64x = (LPDIRECT3DTEXTURE9*)0x00A0AAC0;
LPDIRECT3DTEXTURE9*					TESLODTextureGenerator::D3DTexture128x = (LPDIRECT3DTEXTURE9*)0x00A0AABC;
LPDIRECT3DTEXTURE9*					TESLODTextureGenerator::D3DTexture512x = (LPDIRECT3DTEXTURE9*)0x00A0AAC8;
LPDIRECT3DTEXTURE9*					TESLODTextureGenerator::D3DTexture1024x = (LPDIRECT3DTEXTURE9*)0x00A0AAD0;
LPDIRECT3DTEXTURE9*					TESLODTextureGenerator::D3DTexture2048x = (LPDIRECT3DTEXTURE9*)0x00A0AACC;

BSRenderedTexture**					TESLODTextureGenerator::BSTexture32x = (BSRenderedTexture**)0x00A0AADC;
BSRenderedTexture**					TESLODTextureGenerator::BSTexture64x = (BSRenderedTexture**)0x00A0AAD8;
BSRenderedTexture**					TESLODTextureGenerator::BSTexture128x = (BSRenderedTexture**)0x00A0AAD4;
BSRenderedTexture**					TESLODTextureGenerator::BSTexture512x = (BSRenderedTexture**)0x00A0AAE0;
BSRenderedTexture**					TESLODTextureGenerator::BSTexture1024x = (BSRenderedTexture**)0x00A0AAE8;
BSRenderedTexture**					TESLODTextureGenerator::BSTexture2048x = (BSRenderedTexture**)0x00A0AAE4;

LPDIRECT3DTEXTURE9					TESLODTextureGenerator::D3DTexture256x = NULL;
BSRenderedTexture*					TESLODTextureGenerator::BSTexture256x = NULL;
LPDIRECT3DTEXTURE9					TESLODTextureGenerator::D3DTexture384x = NULL;
BSRenderedTexture*					TESLODTextureGenerator::BSTexture384x = NULL;
LPDIRECT3DTEXTURE9					TESLODTextureGenerator::D3DTexture4096x = NULL;
BSRenderedTexture*					TESLODTextureGenerator::BSTexture4096x = NULL;
LPDIRECT3DTEXTURE9					TESLODTextureGenerator::D3DTexture6144x = NULL;
BSRenderedTexture*					TESLODTextureGenerator::BSTexture6144x = NULL;

ModelLoader**						ModelLoader::Singleton = (ModelLoader**)0x00A0DEAC;
DWORD*								ThreadLocalData::TLSIndex = (DWORD*)0x00A95534;

TESFile* TESDataHandler::LookupPluginByName(const char* PluginName)
{
	for (tList<TESFile>::Iterator Itr = fileList.Begin(); !Itr.End(); ++Itr)
	{
		TESFile* Plugin = Itr.Get();
		if (!Plugin)
			break;

		if (!_stricmp(Plugin->fileName, PluginName))
			return Plugin;
	}

	return NULL;
}

TESFile* TESDataHandler::LookupPluginByIndex(UInt32 Index)
{
	return thisCall<TESFile*>(0x0047BEC0, this, Index);
}

void TESDataHandler::SortScripts()
{
	thisCall<UInt32>(0x0047BA30, this);
}

TESForm* TESDataHandler::CreateForm(UInt8 TypeID)
{
	return thisCall<TESForm*>(0x004793F0, this, TypeID);
}

void TESDataHandler::PopulatePluginArray(const char* WorkingDirectory)
{
	thisCall<UInt32>(0x0047E4C0, this, WorkingDirectory);
}

void TESDataHandler::AddTESObject(TESObject* Object)
{
	this->objects->AddObject(Object);
}

TESObjectREFR* TESDataHandler::PlaceObjectRef( TESObject* BaseObject, const Vector3* Position, const Vector3* Rotation, TESObjectCELL* Cell, TESWorldSpace* WorldSpace, TESObjectREFR* ExistingRef )
{
	return thisCall<TESObjectREFR*>(0x0047A060, this, BaseObject, Position, Rotation, Cell, WorldSpace, ExistingRef);
}

void TESDataHandler::ClearPluginArray()
{
	for (tList<TESFile>::Iterator Itr = fileList.Begin(); !Itr.End(); ++Itr)
	{
		if (!Itr.Get())
			break;

		Itr.Get()->DeleteInstance();
	}
	fileList.RemoveAll();
}

void TESDataHandler::CleanCellWaterExtraData( void )
{
	for (cseOverride::NiTMapIterator Itr = TESForm::FormIDMap->GetFirstPos(); Itr;)
	{
		UInt32 FormID = NULL;
		TESForm* Form = NULL;

		TESForm::FormIDMap->GetNext(Itr, FormID, Form);
		if (FormID && Form)
		{
			if (Form->formType == TESForm::kFormType_Cell)
			{
				TESObjectCELL* Cell = CS_CAST(Form, TESForm, TESObjectCELL);
				SME_ASSERT(Cell);

				if ((Cell->cellFlags & TESObjectCELL::kCellFlags_HasWater) == 0)
				{
					Cell->ModExtraCellWaterHeight(0.0);
					Cell->ModExtraCellWaterType(NULL);
				}
			}
		}
	}
}

void TESDataHandler::AutoSave( void )
{
	thisCall<void>(0x00481ED0, this);
}

bool TESDataHandler::PanicSave( bool Initialize /*= false*/ )
{
	static TESFile* kSaveFile = NULL;

	if (kSaveFile == NULL && Initialize)
	{
		kSaveFile = TESFile::CreateInstance("Data\\Backup\\", "PanicSave.bak");

		return kSaveFile != NULL;
	}
	else if (kSaveFile)
	{
		// wrap this in SEH to prevent the crash handler from being invoked again (we won't be here unless the editor has already crashed)
		__try
		{
			this->unkCD2 = 1;
			TESFile* ActiveFile = this->activeFile;

			if (ActiveFile)
			{
				ActiveFile->SetActive(false);
				ActiveFile->SetLoaded(false);
			}

			kSaveFile->SetActive(true);
			kSaveFile->SetLoaded(true);

			this->activeFile = kSaveFile;
			bool Result = this->SavePlugin();
			kSaveFile->DeleteInstance();
			kSaveFile = NULL;

			if (ActiveFile)
			{
				ActiveFile->SetActive(true);
				ActiveFile->SetLoaded(true);
			}

			this->activeFile = ActiveFile;
			this->unkCD2 = 0;

			return Result;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			return false;
		}
	}
	else
		return false;
}

bool TESDataHandler::SavePlugin( const char* FileName /*= NULL*/, bool AsESM /*= false*/ )
{
	return thisCall<bool>(0x0047E9B0, this, FileName, AsESM);
}

TESObjectCELL* TESDataHandler::GetExteriorCell( float XCoord, float YCoord, TESWorldSpace* Worldspace, bool Create /*= false*/ )
{
	return thisCall<TESObjectCELL*>(0x0047E480, this, XCoord, YCoord, Worldspace, Create);
}

TESObjectCELL* TESDataHandler::GetExteriorCell( float XCoord, float YCoord, float ZCoord, bool* Create, TESWorldSpace* Worldspace )
{
	return thisCall<TESObjectCELL*>(0x0047C7D0, this, XCoord, YCoord, ZCoord, Create, Worldspace);
}

bool TESDataHandler::MoveReference( TESObjectCELL* Cell, TESObjectREFR* Reference )
{
	return thisCall<bool>(0x0047F3F0, this, Cell, Reference);
}

bool TESDataHandler::AddForm( TESForm* Form )
{
	return thisCall<bool>(0x004818F0, this, Form);
}

void TESDataHandler::RemoveInvalidScripts( void )
{
	std::list<Script*> Delinquents;

	for (tList<Script>::Iterator Itr = scripts.Begin(); !Itr.End(); ++Itr)
	{
		Script* Current = Itr.Get();

		if (Current)
		{
			if (Current->editorID.c_str() == NULL && Current->text == NULL && Current->data == NULL)
			{
				Delinquents.push_back(Current);
			}
		}
	}

	for (std::list<Script*>::iterator Itr = Delinquents.begin(); Itr != Delinquents.end(); ++Itr)
	{
		thisCall<void>(0x00452AE0, &scripts, *Itr);		// remove from list
		(*Itr)->DeleteInstance();
	}

	SortScripts();
}

void TESDataHandler::FixInteriorCellFogPlane( void )
{
	for (cseOverride::NiTMapIterator Itr = TESForm::FormIDMap->GetFirstPos(); Itr;)
	{
		UInt32 FormID = NULL;
		TESForm* Form = NULL;

		TESForm::FormIDMap->GetNext(Itr, FormID, Form);
		if (FormID && Form)
		{
			if (Form->formType == TESForm::kFormType_Cell)
			{
				TESObjectCELL* Cell = CS_CAST(Form, TESForm, TESObjectCELL);
				SME_ASSERT(Cell);

				if ((Cell->cellFlags & TESObjectCELL::kCellFlags_Interior))
				{
					if (Cell->cellData.lighting)
					{
						if (Cell->cellData.lighting->fogNear < 0.0001)
							Cell->cellData.lighting->fogNear = 0.0001;
					}
				}
			}
		}
	}
}

void TESDataHandler::PerformPostLoadTasks( void )
{
	CleanCellWaterExtraData();
	RemoveInvalidScripts();
	FixInteriorCellFogPlane();
}

void TES::LoadCellIntoViewPort(const Vector3* CameraCoordData, TESObjectREFR* Reference)
{
	cdeclCall<UInt32>(0x00430F40, CameraCoordData, Reference);
}

void TES::SetSkyTOD( float TOD )
{
	if (sky)
		thisCall<void>(0x00422720, sky, TOD);
}

float TES::GetSkyTOD( void )
{
	if (sky)
		return *((float*)((UInt32)sky + 0xD4));
	else
		return 0.0;
}

UInt8 FileFinder::FindFile(const char* Path, UInt32 Unk02, UInt32 Unk03, int Unk04)
{
	return thisVirtualCall<UInt8>(0x4, this, Path, Unk02, Unk03, Unk04);
}

void FileFinder::AddSearchPath(const char* Path)
{
	thisVirtualCall<UInt32>(0x8, this, Path);
}

NiBinaryStream* FileFinder::GetFileStream(const char* Path, bool WriteAccess, UInt32 BufferSize)
{
	return cdeclCall<NiBinaryStream*>(0x00468400, Path, WriteAccess, BufferSize);
}

const char* FileFinder::GetRelativePath( const char* Path, const char* RelativeTo )
{
	return cdeclCall<const char*>(0x004308B0, Path, RelativeTo);
}

BSRenderedTexture* BSTextureManager::CreateTexture( NiRenderer* Renderer, UInt32 Size, UInt32 Flags, UInt32 Unk04 /*= 0*/, UInt32 Unk05 /*= 0*/ )
{
	return thisCall<BSRenderedTexture*>(0x00773080, this, Renderer, Size, Flags, Unk04, Unk05);
}

LPDIRECT3DTEXTURE9 BSRenderedTexture::ConvertToD3DTexture(UInt32 Width, UInt32 Height)
{
	LPDIRECT3DTEXTURE9 D3DTexture = NULL, Result = NULL;
	if (Width == 0)
		Width = this->renderedTexture->unk030->width;
	if (Height == 0)
		Height = this->renderedTexture->unk030->height;

	D3DXCreateTexture(_NIRENDERER->device, Width, Height, 1, 0, D3DFMT_R8G8B8, D3DPOOL_SYSTEMMEM, &D3DTexture);
	hooks::_MemHdlr(ConvertNiRenderedTexToD3DBaseTex).WriteJump();
	Result = cdeclCall<LPDIRECT3DTEXTURE9>(0x004113E0, this->renderedTexture, 0, 0, Width, D3DTexture, 0, 1, NULL);
	hooks::_MemHdlr(ConvertNiRenderedTexToD3DBaseTex).WriteBuffer();
	SAFERELEASE_D3D(D3DTexture);

	return Result;
}

void BSRenderedTexture::DeleteInstance( bool ReleaseMemory /*= 0*/ )
{
	thisVirtualCall<void>(0x0, this, ReleaseMemory);
}

GridCellArray::GridEntry* GridCellArray::GetCellEntry( SInt32 X, SInt32 Y )
{
	return &grid[Y + X * size];
}

void TESLODTextureGenerator::SaveExteriorSnapshot( TESObjectCELL* Exterior, UInt32 Resolution, const char* SavePath )
{
	SME_ASSERT(Exterior && Exterior->GetIsInterior() == false && Resolution);
	SME_ASSERT(GeneratorState == kLODDiffuseMapGeneratorState_NotInUse);

	ExteriorSnapshotSource = Exterior;
	SInt32 XCoord = Exterior->cellData.coords->x;
	SInt32 YCoord = Exterior->cellData.coords->y;

	// load the cell into the render window first
	Vector3 Coords((XCoord << 12) + 2048.0, (YCoord << 12) + 2048.0, 0);
	_TES->LoadCellIntoViewPort(&Coords, NULL);

	if (Resolution > 6144)
		Resolution = 6144;
	else if (Resolution < 32)
		Resolution = 32;
	else switch (Resolution)
	{
	case 32:
	case 64:
	case 128:
	case 256:
	case 512:
	case 1024:
	case 2048:
	case 4096:
	case 6144:
		break;
	default:
		BGSEECONSOLE_MESSAGE("Invalid resolution %d for exterior snapshot! Reset to 1024...", Resolution);
		Resolution = 1024;
		break;
	}

	char PathBuffer[MAX_PATH] = {0};
	if (SavePath == NULL)
	{
		CreateDirectory("Data\\Textures\\Landscape\\", NULL);
		FORMAT_STR(PathBuffer, "Data\\Textures\\Landscape\\Snapshot_%i.%i.dds", XCoord, YCoord);
		ExteriorSnapshotPathBuffer = PathBuffer;
	}
	else
		ExteriorSnapshotPathBuffer = SavePath;

	hooks::_MemHdlr(GeneratePartialLODFilePath).WriteJump();
	CreateTextureBuffers();
	cdeclCall<void>(0x00412480, XCoord, YCoord, Resolution);
	ReleaseTextureBuffers();
	hooks::_MemHdlr(GeneratePartialLODFilePath).WriteBuffer();
	BGSEECONSOLE_MESSAGE("Saved exterior cell %i,%i snapshot to %s", XCoord, YCoord, ExteriorSnapshotPathBuffer);

	ExteriorSnapshotPathBuffer = NULL;
	ExteriorSnapshotSource = NULL;
}

void TESLODTextureGenerator::CreateTextureBuffers( void )
{
	cdeclCall<void>(0x00410CD0);
}

void TESLODTextureGenerator::ReleaseTextureBuffers( void )
{
	cdeclCall<void>(0x00410A30);
}

void BSTexturePalette::ReleaseTextures()
{
	thisCall<void>(0x004BD5B0, this);
}

ThreadLocalData* ThreadLocalData::Get(void)
{
	ThreadLocalData* OutLocal;
	__asm
	{
		// since the MSVC++ inline assembler is useless
		// get TLS array (mov ecx, large fs:0x2C)
		_emit	0x64
		_emit	0x8B
		_emit	0x0D
		_emit	0x2C
		_emit	0x0
		_emit	0x0
		_emit	0x0

		mov		eax, TLSIndex
		mov		eax, [eax]
		mov		eax, [ecx + eax * 4]
		mov		OutLocal, eax
	}

	return OutLocal;
}