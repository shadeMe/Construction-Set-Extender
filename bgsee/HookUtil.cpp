#include "HookUtil.h"

namespace bgsee
{
	namespace util
	{
		IATPatchData::IATPatchData(const char* ModuleName) :
			ModuleBase(nullptr),
			ImportModule(nullptr),
			ImportName(nullptr),
			Delayed(false),
			Location(nullptr),
			OriginalFunction(nullptr),
			CallbackFunction(nullptr)
		{
			ModuleBase = (UInt8*)GetModuleHandle(ModuleName);
		}

		void IATPatchData::Replace( void )
		{
			SME_ASSERT(Location && CallbackFunction);
			SME::MemoryHandler::SafeWrite32((UInt32)Location, (UInt32)CallbackFunction);
		}

		void IATPatchData::Reset( void )
		{
			SME_ASSERT(Location && OriginalFunction);
			SME::MemoryHandler::SafeWrite32((UInt32)Location, (UInt32)OriginalFunction);
		}

		bool IATPatchData::Initialize()
		{
			SME_ASSERT(ModuleBase);
			SME_ASSERT(ImportModule);
			SME_ASSERT(ImportName);
			SME_ASSERT(Location == nullptr);
			SME_ASSERT(OriginalFunction == nullptr);

			auto Base = ModuleBase;
			IMAGE_DOS_HEADER* DOSHeader = (IMAGE_DOS_HEADER*)Base;
			IMAGE_NT_HEADERS* NTHeader = (IMAGE_NT_HEADERS*)(Base + DOSHeader->e_lfanew);

			auto ImageDir = Delayed == false ? IMAGE_DIRECTORY_ENTRY_IMPORT : IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT;
			IMAGE_IMPORT_DESCRIPTOR* IAT = (IMAGE_IMPORT_DESCRIPTOR*)(Base + NTHeader->OptionalHeader.DataDirectory[ImageDir].VirtualAddress);

			for(; IAT->Characteristics && Location == nullptr; ++IAT)
			{
				if(!_stricmp(ImportModule, (const char*)(Base + IAT->Name)))
				{
					IMAGE_THUNK_DATA* ThunkData = (IMAGE_THUNK_DATA*)(Base + IAT->OriginalFirstThunk);
					UInt32* DLLIAT = (UInt32*)(Base + IAT->FirstThunk);

					for(; ThunkData->u1.Ordinal; ++ThunkData, ++DLLIAT)
					{
						if(!IMAGE_SNAP_BY_ORDINAL(ThunkData->u1.Ordinal))
						{
							IMAGE_IMPORT_BY_NAME* ImportInfo = (IMAGE_IMPORT_BY_NAME*)(Base + ThunkData->u1.AddressOfData);

							if(!_stricmp((char *)ImportInfo->Name, ImportName))
							{
								Location = DLLIAT;
								break;
							}
						}
					}
				}
			}

			if (Location == nullptr)
				return false;

			OriginalFunction = *((void**)Location);
			return true;
		}

		ThunkBase::Heap::Heap()
		{
			HeapPtr = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 0, 0);
			if (HeapPtr == nullptr)
				OutputDebugString("ThunkBase::Heap::Heap - Bytecode heap creation failed!");
		}

		ThunkBase::Heap::~Heap()
		{
			if (HeapPtr)
				HeapDestroy(HeapPtr);
		}

		ThunkBase::Bytecode* ThunkBase::Heap::Allocate()
		{
			SME_ASSERT(HeapPtr);
			return reinterpret_cast<Bytecode*>(HeapAlloc(HeapPtr, 0, sizeof(Bytecode)));
		}

		void ThunkBase::Heap::Free(Bytecode* Mem)
		{
			SME_ASSERT(HeapPtr);
			HeapFree(HeapPtr, 0, Mem);
		}

		ThunkBase::Heap* ThunkBase::Heap::Get()
		{
			static std::mutex Mutex;
			static std::unique_ptr<Heap> Instance;

			if (Instance == nullptr)
			{
				std::lock_guard<std::mutex> Lock(Mutex);
				if (Instance == nullptr)
					Instance.reset(new Heap);
			}

			return Instance.get();
		}

		ThunkBase::ThunkBase()
		{
			ThunkBytecode = Heap::Get()->Allocate();
			SME_ASSERT(ThunkBytecode);

			new (ThunkBytecode) Bytecode;
		}

		ThunkBase::~ThunkBase()
		{
			Heap::Get()->Free(ThunkBytecode);
		}
	}
}

