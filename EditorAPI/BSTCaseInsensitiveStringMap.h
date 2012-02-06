#pragma once
#include "NiTMap.h"

//	EditorAPI: BSTCaseInsensitiveStringMap class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding


namespace CSE_GlobalClasses
{
	template <class TVAL> class BSTCaseInsensitiveStringMap : public NiTStringMap<TVAL>
	{// size 14

		// virtual methods           
	public:     virtual                     ~BSTCaseInsensitiveStringMap() {}                                               // 000     
	protected:  virtual UInt32              KeyToHashIndex(const char* key) const;                                          // 004
				virtual bool                IsKeysEqual(const char* key1, const char* key2) const;                          // 008

				// methods            
	public:     BSTCaseInsensitiveStringMap(UInt32 uiHashSize = 37, bool bCopy = true) : NiTStringMap<TVAL>(uiHashSize,bCopy) {}

	};

	//---------------------------------------------------------------------------
	template <class TVAL> 
	UInt32 BSTCaseInsensitiveStringMap<TVAL>::KeyToHashIndex(const char* pKey) const
	{
		unsigned int len = strlen(pKey);
		if (len == 0 || pKey == 0) return 0;
		char* buffer = (char*)alloca(len+1);  // NOTE: use of alloca() for large string buffers will cause CTDs

		// convert to lower case
		for (int c = 0; c < len; c++) buffer[c] = tolower(pKey[c]);
		buffer[len] = 0;

		// invoke base hash method
		return NiTStringMap<TVAL>::KeyToHashIndex(buffer);
	}
	//---------------------------------------------------------------------------
	template <class TVAL> 
	inline bool BSTCaseInsensitiveStringMap<TVAL>::IsKeysEqual(const char* pcKey1, const char* pcKey2) const
	{
		return _stricmp(pcKey1, pcKey2) == 0;
	}
}