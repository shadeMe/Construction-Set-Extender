#pragma once

namespace bgsee
{
	namespace util
	{
		struct IATPatchData
		{
			UInt8*				ModuleBase;
			const char*			ImportModule;
			const char*			ImportName;
			bool				Delayed;

			void*				Location;
			void*				OriginalFunction;
			void*				CallbackFunction;

			IATPatchData(const char* ModuleName = nullptr);

			bool				Initialize();
			void				Replace(void);
			void				Reset(void);
		};


		class ThunkBase
		{
		protected:
			struct Bytecode
			{
#pragma pack(push, 1)
				unsigned short stub1;      // lea ecx,
				unsigned long  nThisPtr;   // this
				unsigned char  stub2;      // mov eax,
				unsigned long  nJumpProc;  // pointer to destination function
				unsigned short stub3;      // jmp eax
#pragma pack(pop)
				Bytecode() :
					stub1	  (0x0D8D),
					nThisPtr  (0),
					stub2	  (0xB8),
					nJumpProc (0),
					stub3	  (0xE0FF)
				{}
			};

			class Heap
			{
				HANDLE		HeapPtr;

				Heap();
			public:
				~Heap();

				Bytecode*	Allocate();
				void		Free(Bytecode* Mem);

				static Heap* Get();
			};


			Bytecode*		ThunkBytecode;
		public:
			ThunkBase();
			virtual ~ThunkBase();

			ThunkBase(const ThunkBase&) = delete;
			const ThunkBase& operator=(const ThunkBase&) = delete;
		};

		template<class T, typename R, typename ...Args>
		class ThunkStdCall : public ThunkBase
		{
		public:
			using CallbackT			= R (__stdcall *)(Args...);
			using MemberFunctionT	= R (__thiscall T::*)(Args...);

			void Bind(T* Instance, MemberFunctionT MemberFunction)
			{
				STATIC_ASSERT(sizeof(Instance) == sizeof(ULONG));

				ThunkBytecode->nThisPtr = reinterpret_cast<ULONG>(Instance);
				ThunkBytecode->nJumpProc = *reinterpret_cast<ULONG*>(&MemberFunction);

				FlushInstructionCache(GetCurrentProcess(), ThunkBytecode, sizeof(Bytecode));
			}

			ThunkStdCall() = default;
			ThunkStdCall(T* Instance, MemberFunctionT MemberFunction) : ThunkBase()
			{
				Bind(Instance, MemberFunction);
			}
			virtual ~ThunkStdCall() = default;

			CallbackT operator()() const
			{
				SME_ASSERT(ThunkBytecode->nThisPtr != 0);
				SME_ASSERT(ThunkBytecode->nJumpProc != 0);
				return reinterpret_cast<CallbackT>(ThunkBytecode);
			}
		};
	}
}
