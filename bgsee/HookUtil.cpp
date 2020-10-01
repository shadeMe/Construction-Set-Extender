#include "HookUtil.h"

namespace bgsee
{
	namespace util
	{
		IATPatchData::IATPatchData(const char* ModuleName) :
			ModuleBase(0),
			ImportModule(nullptr),
			ImportName(nullptr),
			Delayed(false),
			Location(0),
			OriginalFunction(0),
			CallbackFunction(0)
		{
			ModuleBase = reinterpret_cast<uintptr_t>(GetModuleHandle(ModuleName));
		}

		void IATPatchData::Replace() const
		{
			SME_ASSERT(Location && CallbackFunction);
			SME::MemoryHandler::SafeWrite32(Location, CallbackFunction);
		}

		void IATPatchData::Reset() const
		{
			SME_ASSERT(Location && OriginalFunction);
			SME::MemoryHandler::SafeWrite32(Location, OriginalFunction);
		}

		PIMAGE_THUNK_DATA GetIATThunk(uintptr_t Module, PIMAGE_THUNK_DATA NameTable, PIMAGE_THUNK_DATA ImportTable, const char* API)
		{
			for (; NameTable->u1.Ordinal != 0; ++NameTable, ++ImportTable)
			{
				if (!IMAGE_SNAP_BY_ORDINAL(NameTable->u1.Ordinal))
				{
					auto importName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(Module + NameTable->u1.ForwarderString);
					auto funcName = &importName->Name[0];

					// If this is the function name we want, hook it
					if (!strcmp(funcName, API))
					{
						return ImportTable;
					}
				}
			}

			return 0;
		}

		bool IATPatchData::Initialize()
		{
			SME_ASSERT(ModuleBase);
			SME_ASSERT(ImportModule);
			SME_ASSERT(ImportName);
			SME_ASSERT(Location == 0);
			SME_ASSERT(OriginalFunction == 0);

			auto dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(ModuleBase);
			auto ntHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(ModuleBase + dosHeader->e_lfanew);

			// Validate PE Header and (64-bit|32-bit) module type
			if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
				return false;

			if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
				return false;

			if (ntHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
				return false;

			// Get the load configuration section which holds the imports
			auto dataDirectory = ntHeaders->OptionalHeader.DataDirectory;
			auto imageDir = Delayed == false ? IMAGE_DIRECTORY_ENTRY_IMPORT : IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT;
			auto sectionRVA = dataDirectory[imageDir].VirtualAddress;
			auto sectionSize = dataDirectory[imageDir].Size;

			if (sectionRVA == 0 || sectionSize == 0)
				return false;

			if (Delayed)
			{
				auto importDescriptor = reinterpret_cast<PIMAGE_DELAYLOAD_DESCRIPTOR>(ModuleBase + sectionRVA);

				for (size_t i = 0; importDescriptor[i].DllNameRVA != 0; i++)
				{
					auto dllName = reinterpret_cast<PSTR>(ModuleBase + importDescriptor[i].DllNameRVA);
					if (!_stricmp(dllName, ImportModule))
					{
						auto nameTable = reinterpret_cast<PIMAGE_THUNK_DATA>(ModuleBase + importDescriptor[i].ImportNameTableRVA);
						auto importTable = reinterpret_cast<PIMAGE_THUNK_DATA>(ModuleBase + importDescriptor[i].ImportAddressTableRVA);

						auto thunk = GetIATThunk(ModuleBase, nameTable, importTable, ImportName);
						if (thunk)
						{
							Location = reinterpret_cast<uintptr_t>(thunk);
							OriginalFunction = thunk->u1.AddressOfData;
							break;
						}
					}
				}
			}
			else
			{
				auto importDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(ModuleBase + sectionRVA);

				for (size_t i = 0; importDescriptor[i].Name != 0; i++)
				{
					auto dllName = reinterpret_cast<PSTR>(ModuleBase + importDescriptor[i].Name);

					// Is this the specific module the user wants?
					if (!_stricmp(dllName, ImportModule))
					{
						if (!importDescriptor[i].FirstThunk)
							return false;

						auto nameTable = reinterpret_cast<PIMAGE_THUNK_DATA>(ModuleBase + importDescriptor[i].OriginalFirstThunk);
						auto importTable = reinterpret_cast<PIMAGE_THUNK_DATA>(ModuleBase + importDescriptor[i].FirstThunk);

						auto thunk = GetIATThunk(ModuleBase, nameTable, importTable, ImportName);
						if (thunk)
						{
							Location = reinterpret_cast<uintptr_t>(thunk);
							OriginalFunction = thunk->u1.AddressOfData;
							break;
						}
					}
				}
			}

			return Location != 0;
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

