#include "Array.h"

namespace CSAutomationScript
{
	void ArrayElement::GetAsMUPValue(mup::Value& ValueOut)
	{
		switch (Type)
		{
		case kParamType_Invalid:
			ValueOut = (double)0.0;
			break;
		case kParamType_String:
			ValueOut = std::string(StringData);
			break;
		case kParamType_Array:
			GetArray()->ConvertToMUPArray(ValueOut);
			break;
		case kParamType_Reference:
			ValueOut = (int)RefData->formID;
			break;
		case kParamType_Numeric:
			ValueOut = NumericData;
			break;
		}
	}

	UInt32		Array::GlobalInstanceCount = 0;

	void Array::Release()
	{
		for (ArrayElementList::iterator Itr = Elements.begin(); Itr != Elements.end(); Itr++)
		{
			delete (*Itr);
		}

		Elements.clear();

		--GlobalInstanceCount;
	}

	void Array::CopyElementsFrom(Array* Source)
	{
		for (int i = 0; i < Source->Size(); i++)
		{
			ArrayElement* SourceElement = NULL;
			if (Source->At(i, &SourceElement))
			{
				Insert(SourceElement);
			}
			else
				assertR(0);
		}
	}

	template<typename T>
	bool Array::AddElement(T Data, int Index)
	{
		if (Index == -1)
		{
			Elements.push_back(new ArrayElement(Data));
			return true;
		}
		else if (Index < Size())
		{
			ArrayElement* Existing = Elements[Index];
			if (Existing)
				delete Existing;

			Elements[Index] = new ArrayElement(Data);
			return true;
		}
		else
			return false;
	}

	bool Array::Insert(Array* Data, int Index /* = -1 */)
	{
		return AddElement<Array*>(Data, Index);
	}

	bool Array::Insert(TESForm* Data, int Index /* = -1 */)
	{
		return AddElement<TESForm*>(Data, Index);
	}

	bool Array::Insert(const char* Data, int Index /* = -1 */)
	{
		return AddElement<const char*>(Data, Index);
	}

	bool Array::Insert(double Data, int Index /* = -1 */)
	{
		return AddElement<double>(Data, Index);
	}

	bool Array::Insert(CSASDataElement* Data, int Index /* = -1 */)
	{
		return AddElement<CSASDataElement*>(Data, Index);
	}

	bool Array::Erase(UInt32 Index)
	{
		if (Index >= Size())
			return false;

		ArrayElementList::iterator Itr = Elements.begin() + Index;
		Elements.erase(Itr);

		return true;
	}

	bool Array::At(UInt32 Index, ArrayElement** ElementOut) const
	{
		if (Index >= Size())
			return false;

		*ElementOut = Elements[Index];
		return true;
	}

	UInt32 Array::Size() const
	{
		return Elements.size();
	}

	void Array::Clear()
	{
		Release();
	}

	void Array::ConvertToMUPArray(mup::IValue& ArrayOut)
	{
		mup::Value Buffer(Size(), 0);

		for (int i = 0; i < Size(); i++)
		{
			ArrayElement* ThisElement = Elements[i];
			ThisElement->GetAsMUPValue(dynamic_cast<mup::Value&>(Buffer.At(i, 0)));
		}

		ArrayOut = Buffer;
	}

	Array* Array::AllocateArray(Array* Source)
	{
		return new Array(Source);
	}

	Array* Array::AllocateArray(CSASDataElement* Element, UInt32 Size)
	{
		Array* Result = new Array();
		Result->Elements.resize(Size);

		for (int i = 0; i < Size; i++)
		{
			CSASDataElement* CurrentElement = &Element[i];
			Result->Insert(CurrentElement, i);
		}

		return Result;
	}

	void Array::ReleaseArray(Array* Source)
	{
		delete Source;
	}

	ArrayIterator::ArrayIterator(Array* ParentArray)
	{
		ArraySize = ParentArray->Elements.size();
		Iterator = Bookend = ParentArray->Elements.end();

		CurrentElement = NULL;
		this->ParentArray = ParentArray;

		if (ArraySize)
		{
			Iterator = ParentArray->Elements.begin();
			CurrentElement = *Iterator;
		}
	}

	ArrayElement* ArrayIterator::GetNextElement()
	{
		if (Iterator != ParentArray->Elements.end() && ++Iterator != ParentArray->Elements.end())
			CurrentElement = *Iterator;
		else
			CurrentElement = NULL;

		return CurrentElement;
	}
}