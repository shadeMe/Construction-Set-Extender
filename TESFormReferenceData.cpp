#include "TESFormReferenceData.h"

TESFormReferenceData* TESFormReferenceData::FindDataInRefList(GenericNode<TESFormReferenceData>* RefList, TESForm* Form)
{
	for (; RefList && RefList->data; RefList = RefList->next)
	{
		TESFormReferenceData* Data = RefList->data;
		if (Data->Form == Form)
			return Data;
	}

	return NULL;
}