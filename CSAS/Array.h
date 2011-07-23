#pragma once

#include "ScriptTypes.h"
#include "mpValue.h"

namespace CSAutomationScript
{
	class ArrayElement : public CSASDataElement
	{
	public:
		friend class Array;

		ArrayElement() : CSASDataElement() {}
		ArrayElement(CSASDataElement* Data) : CSASDataElement(Data) {}
		ArrayElement(double Num) : CSASDataElement(Num) {}
		ArrayElement(const char* Str) : CSASDataElement(Str) {}
		ArrayElement(TESForm* Form) : CSASDataElement(Form) {}
		ArrayElement(Array* Array) : CSASDataElement(Array) {}
		ArrayElement(CSASDataElement* Elements, UInt32 Size) : CSASDataElement(Elements, Size) {}

		virtual ~ArrayElement()
		{
			;//
		}

		void								GetAsMUPValue(mup::Value& ValueOut);
	};

	class ArrayIterator;

	class Array
	{
		static UInt32						GlobalInstanceCount;

		typedef std::vector<ArrayElement*>	ArrayElementList;
		ArrayElementList					Elements;

		void								Release();
		void								CopyElementsFrom(Array* Source);

		Array(const Array& rhs);
		Array& operator=(const Array& rhs);

		template<typename T>
		bool								AddElement(T Data, int Index);
	public:
		friend class ArrayIterator;

		Array() { Elements.reserve(5); GlobalInstanceCount++; }
		Array(Array* Source) { Elements.reserve(5); CopyElementsFrom(Source); GlobalInstanceCount++; }
		~Array() { Release(); }

		bool								Insert(CSASDataElement* Data, int Index = -1);
		bool								Insert(double Data, int Index = -1);
		bool								Insert(const char* Data, int Index = -1);
		bool								Insert(TESForm* Data, int Index = -1);
		bool								Insert(Array* Data, int Index = -1);

		bool								Erase(UInt32 Index);

		bool								At(UInt32 Index, ArrayElement** ElementOut) const;
		void								Clear();
		UInt32								Size() const;
		void								ConvertToMUPArray(mup::IValue& ArrayOut);

		static Array*						AllocateArray(CSASDataElement* Element, UInt32 Size);
		static Array*						AllocateArray(Array* Source);
		static void							ReleaseArray(Array* Source);

		static UInt32						GetGlobalInstanceCount() { return GlobalInstanceCount; }
	};

	class ArrayIterator
	{
		UInt32												ArraySize;
		Array::ArrayElementList::const_iterator				Bookend;
		Array::ArrayElementList::const_iterator				Iterator;

		ArrayElement*										CurrentElement;
		Array*												ParentArray;
	public:
		ArrayIterator(Array* ParentArray);

		ArrayElement*										GetNextElement();
		bool												GetDone() { return CurrentElement == 0; }
		ArrayElement*										GetCurrentElement() { return CurrentElement; }
	};
}