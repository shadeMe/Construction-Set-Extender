#pragma once
#include "ExtenderInternals.h"

class TESFormReferenceData
{
	TESForm*		Form;
	UInt32			Count;
public:
	void			Initialize(TESForm* Form)	{ this->Form = Form; Count = 0; }
	UInt32			GetReferenceCount() const	{ return Count; }
	UInt32			IncrementRefCount()			{ return ++Count; }
	UInt32			DecrementRefCount()			{ if (Count-- == 0)	Count = 0; return Count; }
	TESForm*		GetForm() const				{ return Form; }

	static TESFormReferenceData* FindDataInRefList(GenericNode<TESFormReferenceData>* RefList, TESForm* Form);
};