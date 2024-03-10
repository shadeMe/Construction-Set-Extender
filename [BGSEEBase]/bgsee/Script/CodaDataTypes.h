#pragma once
#include "CodaForwardDecls.inl"
#include "CodaBaseInterfaces.inl"
#include "CodaUtilities.h"

namespace bgsee
{
	namespace script
	{
		class ICodaScriptArrayDataType
		{
		public:
			virtual ~ICodaScriptArrayDataType() = 0;

			typedef std::shared_ptr<ICodaScriptArrayDataType>		SharedPtrT;

			virtual bool											Insert(CodaScriptBackingStore* Data, int Index = -1, bool Replace = true) = 0;
			virtual bool											Insert(CodaScriptNumericDataTypeT Data, int Index = -1, bool Replace = true) = 0;
			virtual bool											Insert(CodaScriptStringParameterTypeT Data, int Index = -1, bool Replace = true) = 0;
			virtual bool											Insert(CodaScriptReferenceDataTypeT Data, int Index = -1, bool Replace = true) = 0;
			virtual bool											Insert(SharedPtrT Data, int Index = -1, bool Replace = true) = 0;

			virtual bool											Erase(UInt32 Index) = 0;
			virtual void											Clear(void) = 0;

			virtual bool											At(UInt32 Index, CodaScriptBackingStore& OutBuffer) const = 0;
			virtual UInt32											Size(void) const = 0;
		};

		class CodaScriptBackingStore : public ICodaScriptDataStore
		{
			static int								GIC;
		protected:
			union
			{
				CodaScriptNumericDataTypeT			NumericData;
				CodaScriptReferenceDataTypeT		RefData;
				CodaScriptStringDataTypeT			StringData;
			};
			ICodaScriptArrayDataType::SharedPtrT	ArrayData;				// not a trivial data type, so can't be a part of the union

			void									Reset(void);
			void									Copy(const CodaScriptBackingStore& Source);

			static bool								CompareString(CodaScriptStringParameterTypeT lhs, CodaScriptStringParameterTypeT rhs);
			static bool								CompareNumber(const CodaScriptNumericDataTypeT& lhs, const CodaScriptNumericDataTypeT& rhs);
			static bool								CompareReference(const CodaScriptReferenceDataTypeT& lhs, const CodaScriptReferenceDataTypeT& rhs);
			static bool								CompareArray(const ICodaScriptArrayDataType::SharedPtrT& lhs,
																 const ICodaScriptArrayDataType::SharedPtrT& rhs);	// shallow/pointer comparison
		public:
			CodaScriptBackingStore();
			explicit CodaScriptBackingStore(CodaScriptBackingStore* Data);
			explicit CodaScriptBackingStore(CodaScriptNumericDataTypeT Num);
			explicit CodaScriptBackingStore(CodaScriptStringParameterTypeT Str);
			explicit CodaScriptBackingStore(CodaScriptReferenceDataTypeT Form);
			explicit CodaScriptBackingStore(ICodaScriptArrayDataType::SharedPtrT Array);
			virtual ~CodaScriptBackingStore();

			explicit CodaScriptBackingStore(const CodaScriptBackingStore& rhs);
			CodaScriptBackingStore& operator=(const CodaScriptBackingStore& rhs);

			CodaScriptBackingStore& operator+=(const CodaScriptBackingStore &rhs);
			CodaScriptBackingStore& operator-=(const CodaScriptBackingStore &rhs);
			CodaScriptBackingStore& operator*=(const CodaScriptBackingStore &rhs);

			virtual bool											HasImplicitCast(DataType NewType) const;

			virtual CodaScriptReferenceDataTypeT					GetFormID() const;
			virtual CodaScriptNumericDataTypeT						GetNumber() const;
			virtual CodaScriptStringParameterTypeT					GetString() const;
			ICodaScriptArrayDataType::SharedPtrT					GetArray() const;

			virtual void											SetFormID(CodaScriptReferenceDataTypeT Data);
			virtual void											SetNumber(CodaScriptNumericDataTypeT Data);
			virtual void											SetString(CodaScriptStringParameterTypeT Data);
			virtual void											SetArray(ICodaScriptDataStore* Data);					// ugly workaround for CRT state inconsistencies during runtime
			void													SetArray(ICodaScriptArrayDataType::SharedPtrT Data);

			virtual ICodaScriptDataStore&							operator=(const ICodaScriptDataStore& rhs);
			virtual ICodaScriptDataStore&							operator=(CodaScriptNumericDataTypeT Num);
			virtual ICodaScriptDataStore&							operator=(CodaScriptStringParameterTypeT Str);
			virtual ICodaScriptDataStore&							operator=(CodaScriptReferenceDataTypeT Form);


			static const int&										GetGIC() { return GIC; }

			virtual bool											operator ==(const ICodaScriptDataStore& rhs) const override;
			virtual bool											operator ==(const CodaScriptNumericDataTypeT& rhs) const override;
			virtual bool											operator ==(CodaScriptStringParameterTypeT& rhs) const override;
			virtual bool											operator ==(const CodaScriptReferenceDataTypeT& rhs) const override;

			typedef std::vector<CodaScriptBackingStore>				NonPtrArrayT;
		};

		class CodaScriptVariable
		{
			static int												GIC;

			CodaScriptVariable(const CodaScriptVariable& rhs);
			CodaScriptVariable& operator=(const CodaScriptVariable& rhs);
		protected:
			CodaScriptSourceCodeT									Name;
			ICodaScriptDataStoreOwner::PtrT							BoundOwner;
		public:
			CodaScriptVariable(CodaScriptSourceCodeT& Name, ICodaScriptDataStoreOwner* Storage);		// claims ownership of the owner
			virtual ~CodaScriptVariable();

			const char*												GetName() const;
			void													SetName(CodaScriptSourceCodeT& Name);
			ICodaScriptDataStoreOwner*								GetStoreOwner() const;

			static const int&										GetGIC() { return GIC; }

			typedef std::vector<CodaScriptVariable*>				ArrayT;
			typedef std::unique_ptr<CodaScriptVariable>				PtrT;
		};

		enum class ObjectFactoryType
		{
			MUP
		};

		class IObjectFactory
		{
		protected:
			ObjectFactoryType	FactoryType;
		public:
			IObjectFactory(ObjectFactoryType Type) : FactoryType(Type) {}
			virtual ~IObjectFactory() = 0 {}

			virtual ICodaScriptExpressionParser*			BuildExpressionParser() = 0;
			virtual ICodaScriptDataStoreOwner*				BuildDataStoreOwner() = 0;
			virtual ICodaScriptArrayDataType::SharedPtrT	BuildArray(UInt32 InitialSize = 0) = 0;
		};
	}
}