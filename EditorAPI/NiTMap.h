#pragma once

//	EditorAPI: NiTMap class and derivatives.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*       
    NiTMap class hierarchy: hashtable-based associative containers   

    [ NOTICE: This is a templated class ]
    Because it uses templates, this class requires a separate set of imports for every specialization.  
    In practice, most of the specializations map back to a few basic methods, but we'd still need to
    import the appropriate symbols for each.  This makes it difficult to hook and patch - but it also means
    that no one is likely to patch it, so the hooking is not strictly necessary.  We instead attempt to
    reproduce the original templated source code.
    CAUTIONS:
    -   Great care must be taken for some functions (e.g. hash computation) to make sure they match the game code
    -   Because of simplification to template & inheritance structure, dynamic_cast may fail for this class

    Alterations from original code:
    -   Removed all template parameters for the allocator, hash & comparison functors
        The hash & comparison functors actually manifest as virtual methods
    -   Removed all inheritance from NiMemObject
    -   Switched NIASSERT to cassert
    -   Switched NiMalloc & NiFree to FormHeap

    [Original] Notes on Usage:

     The map class implements a hash table of TKEY to store values of TVAL.
     It uses modular arithmetic for building the hash keys with a default
     table size of 37.  If you want a larger table size, the best bet is to
     us a large prime number.  Consult a standard text on hashing for the
     basic theory.

     TKEY MUST BE THE SAME SIZE AS A POINTER!

     The template class assumes that type TKEY has the following:
       1.  Default constructor, TKEY::TKEY();
       2.  Copy constructor, TKEY::TKEY(const TKEY&);
       3.  Assignment, TKEY& operator=(const TKEY&);
       4.  Comparison, bool TKEY::operator==(const TKEY&), or supply a
           specialized equality testing class in your template.
       5.  Implicit conversion, TKEY::operator long(), for building hash key,
           or you must pass in your own hash function class in your template.

     The template class assumes that type TVAL has the following:
       1.  Default constructor, TVAL::TVAL();
       2.  Copy constructor, TVAL::TVAL(const TVAL&);
       3.  Assignment, TVAL& operator=(const TVAL&);

     In both cases, the compiler-generated default constructor, copy
     constructor, and assignment operator are acceptable.

     Example of iteration over map

         NiTMap<TKEY,TVAL> kMap;
         NiTMapIterator pos = kMap.GetFirstPos();
         while (pos)
         {
             TKEY key;
             TVAL val;
             kMap.GetNext(pos,key,val);
             <process key and val here>;
         }

*/
namespace CSE_GlobalClasses			// can't be in the global namespace as it conflicts with OBSE's definition of the classes
{
	typedef void* NiTMapIterator;

	template <class TKEY, class TVAL> class NiTMapItem
	{// size 0C/0C
	public:
		NiTMapItem* m_pkNext;
		TKEY        m_key;
		TVAL        m_val;
		// use FormHeap for class new & delete
		USEFORMHEAP 
	};

	template <class TKEY, class TVAL> class NiTMapBase
	{// size 10/10 

		// members
		//void**                vtbl;               // 00
	protected:  UInt32                  m_uiHashSize;       // 04 maximum slots in hash table
				NiTMapItem<TKEY,TVAL>** m_ppkHashTable;     // 08 hash table storage
				UInt32                  m_uiCount;          // 0C number of elements in list

				// virtual methods              
	public:     virtual                         ~NiTMapBase();                                                  // 000 
				// vtbl entry is actually 'scalar deleting destructor', which calls this method       
	protected:  virtual UInt32                  KeyToHashIndex(TKEY key) const;                                 // 004 actually a templated functor
				virtual bool                    IsKeysEqual(TKEY key1, TKEY key2) const;                        // 008 actually a templated functor
				virtual void                    SetValue(NiTMapItem<TKEY,TVAL>* pkItem, TKEY key, TVAL val);    // 00C
				virtual void                    ClearValue(NiTMapItem<TKEY,TVAL>* pkItem);                      // 010
				virtual NiTMapItem<TKEY, TVAL>* NewItem() = 0;                                                  // 014
				virtual void                    DeleteItem(NiTMapItem<TKEY, TVAL>* pkItem) = 0;                 // 018

				// methods - counting elements in map
	public:     inline UInt32           GetCount() const;
				inline bool             IsEmpty() const;
				// methods - add or remove elements
				inline void             SetAt(TKEY key, TVAL val);
				inline bool             RemoveAt(TKEY key);
				inline void             RemoveAll();
				// methods - element access
				inline bool             GetAt(TKEY key, TVAL& val) const;
				// methods - map traversal
				inline NiTMapIterator   GetFirstPos() const;
				inline void             GetNext(NiTMapIterator& pos, TKEY& key, TVAL& val) const;  
				// methods - Resize the map (e.g. the underlying hash table)
				inline void             Resize(UInt32 uiNewHashSize);         
				// methods - construction
	public:     NiTMapBase(UInt32 uiHashSize = 37); 
				// use FormHeap for class new & delete
				USEFORMHEAP 
					// prevent compiler from generating the default copy constructor or default assignment operator 
	private:    NiTMapBase(const NiTMapBase&);              
				NiTMapBase& operator=(const NiTMapBase&);           
	};

	template <class TKEY, class TVAL> class NiTMap : public NiTMapBase<TKEY, TVAL>
	{// size 10/10 
		// members
	public:     static const UInt32 NUM_PRIMES;
				static const UInt32 PRIMES[];

				// virtual methods
				virtual ~NiTMap();                                          // 000
	protected:  virtual NiTMapItem<TKEY, TVAL>* NewItem();                  // 014
				virtual void DeleteItem(NiTMapItem<TKEY, TVAL>* pkItem);    // 018

				// methods
	public:     inline NiTMap(UInt32 uiHashSize = 37) :  NiTMapBase<TKEY, TVAL>(uiHashSize) {};
				static UInt32 NextPrime(const UInt32 uiTarget);
	};

	template <class TKEY, class TVAL> class NiTPointerMap : public NiTMapBase<TKEY, TVAL>
	{// size 10/10 
		// virtual methods
	public:     virtual ~NiTPointerMap();                                   // 000
	protected:  virtual NiTMapItem<TKEY, TVAL>* NewItem();                  // 014
				virtual void DeleteItem(NiTMapItem<TKEY, TVAL>* pkItem);    // 018

				// methods
	public:     NiTPointerMap(UInt32 uiHashSize = 37) :  NiTMapBase<TKEY, TVAL>(uiHashSize) {};
				// use FormHeap for class new & delete
				USEFORMHEAP 
	};

	template <class TPARENT, class TVAL> class NiTStringTemplateMap : public TPARENT
	{// size 14/14 
		// members
	protected:  bool m_bCopy;       // 10

				// virtual methods

	public:     virtual                     ~NiTStringTemplateMap();                                                        // 000     
	protected:  virtual UInt32              KeyToHashIndex(const char* key) const;                                          // 004
				virtual bool                IsKeysEqual(const char* key1, const char* key2) const;                          // 008
				// NOTE: Bethesda used a case-insensitive string comparison for this field, even though the default
				// hash function is case sensitive.  This violates the core invariant of hash tables - that key equality
				// implies hash equality.  As a result, some parts of the key space will theoretically overlap.
				// In practice with a hash table of reasonable size, however, this doesn't seem to be a problem.  
				// For correctness, this definition uses the intended case sensitive comparator.
				virtual void                SetValue(NiTMapItem<const char*, TVAL>* pkItem, const char* pcKey, TVAL val);   // 00C
				virtual void                ClearValue(NiTMapItem<const char*, TVAL>* pkItem);                              // 010

				// methods            
	public:     NiTStringTemplateMap(UInt32 uiHashSize = 37, bool bCopy = true) : TPARENT(uiHashSize), m_bCopy(bCopy) {}
				// use FormHeap for class new & delete
				USEFORMHEAP 
	};

	template <class TVAL> class NiTStringMap : public NiTStringTemplateMap<NiTMap<const char*, TVAL>,TVAL>
	{// size 14/14 
	public:     
		// methods
		NiTStringMap(UInt32 uiHashSize = 37, bool bCopy = true) : NiTStringTemplateMap<NiTMap<const char*, TVAL>,TVAL>(uiHashSize, bCopy) {}
		// use FormHeap for class new & delete
		USEFORMHEAP 
	};

	template <class TVAL> class NiTStringPointerMap : public NiTStringTemplateMap<NiTPointerMap<const char*, TVAL>, TVAL>
	{// size 14/14 
	public:     
		// methods
		NiTStringPointerMap(UInt32 uiHashSize = 37, bool bCopy = true) : NiTStringTemplateMap<NiTPointerMap<const char*, TVAL>, TVAL>(uiHashSize, bCopy) {}
		// use FormHeap for class new & delete
		USEFORMHEAP 
	};

	// NiTMapBase
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	inline NiTMapBase<TKEY, TVAL>::NiTMapBase(UInt32 uiHashSize)
	{
		assert(uiHashSize > 0);

		m_uiHashSize = uiHashSize;
		m_uiCount = 0;

		UInt32 uiSize = sizeof(NiTMapItem<TKEY,TVAL>*) * m_uiHashSize;
		m_ppkHashTable = (NiTMapItem<TKEY,TVAL>**)MemoryHeap::FormHeapAlloc(uiSize);
		assert(m_ppkHashTable);
		memset(m_ppkHashTable, 0, uiSize);
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	inline NiTMapBase<TKEY, TVAL>::~NiTMapBase()
	{
		RemoveAll();
		FormHeap_Free(m_ppkHashTable);
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline UInt32 NiTMapBase<TKEY, TVAL>::GetCount() const
	{ 
		return m_uiCount;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline bool NiTMapBase<TKEY,TVAL>::IsEmpty() const
	{ 
		return m_uiCount == 0;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline void NiTMapBase<TKEY,TVAL>::SetAt(TKEY key, TVAL val)
	{
		if (!m_ppkHashTable) return;  // no hash table allocated

		// look up hash table location for key
		UInt32 uiIndex = KeyToHashIndex(key);
		NiTMapItem<TKEY, TVAL>* pkItem = m_ppkHashTable[uiIndex];

		UInt32 uiDepth = 0;

		// search list at hash table location for key
		while (pkItem)
		{
			uiDepth++;
			if (IsKeysEqual(key, pkItem->m_key))
			{
				// item already in hash table, set its new value
				pkItem->m_val = val;
				return;
			}
			pkItem = pkItem->m_pkNext;
		}

		// add object to beginning of list for this hash table index
		pkItem = (NiTMapItem<TKEY, TVAL>*)NewItem();

		assert(pkItem);
		SetValue(pkItem, key, val);
		pkItem->m_pkNext = m_ppkHashTable[uiIndex];
		m_ppkHashTable[uiIndex] = pkItem;
		m_uiCount++;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline bool NiTMapBase<TKEY,TVAL>::RemoveAt(TKEY key)
	{
		if (!m_ppkHashTable) return false;  // no hash table allocated

		// look up hash table location for key
		UInt32 uiIndex = KeyToHashIndex(key);
		NiTMapItem<TKEY, TVAL>* pkItem = m_ppkHashTable[uiIndex];

		// search list at hash table location for key
		if (pkItem)
		{
			if (IsKeysEqual(key, pkItem->m_key))
			{
				// item at front of list, remove it
				m_ppkHashTable[uiIndex] = pkItem->m_pkNext;
				ClearValue(pkItem);

				DeleteItem(pkItem);

				m_uiCount--;
				return true;
			}
			else
			{
				// search rest of list for item
				NiTMapItem<TKEY, TVAL>* pkPrev = pkItem;
				NiTMapItem<TKEY, TVAL>* pkCurr = pkPrev->m_pkNext;
				while (pkCurr && !IsKeysEqual(key, pkCurr->m_key))
				{
					pkPrev = pkCurr;
					pkCurr = pkCurr->m_pkNext;
				}
				if (pkCurr)
				{
					// found the item, remove it
					pkPrev->m_pkNext = pkCurr->m_pkNext;
					ClearValue(pkCurr);

					DeleteItem(pkCurr);

					m_uiCount--;
					return true;
				}
			}
		}

		return false;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline void NiTMapBase<TKEY, TVAL>::RemoveAll()
	{
		if (!m_ppkHashTable) return;  // no hash table allocated

		for (UInt32 ui = 0; ui < m_uiHashSize; ui++) 
		{
			while (m_ppkHashTable[ui])
			{
				NiTMapItem<TKEY, TVAL>* pkSave = m_ppkHashTable[ui];
				m_ppkHashTable[ui] = m_ppkHashTable[ui]->m_pkNext;
				ClearValue(pkSave);

				DeleteItem(pkSave);
			}
		}

		m_uiCount = 0;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline bool NiTMapBase<TKEY,TVAL>::GetAt(TKEY key, TVAL& val) const
	{
		if (!m_ppkHashTable) return false;  // no hash table allocated
		// look up hash table location for key
		UInt32 uiIndex = KeyToHashIndex(key);
		NiTMapItem<TKEY, TVAL>* pkItem = m_ppkHashTable[uiIndex];

		// search list at hash table location for key
		while (pkItem)
		{
			if (IsKeysEqual(key, pkItem->m_key))
			{
				// item found
				val = pkItem->m_val;
				return true;
			}
			pkItem = pkItem->m_pkNext;
		}

		return false;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	inline UInt32 NiTMapBase<TKEY,TVAL>::KeyToHashIndex(TKEY key) const
	{
		return (UInt32) (((size_t) key) % m_uiHashSize);
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	inline bool NiTMapBase<TKEY,TVAL>::IsKeysEqual(TKEY key1, TKEY key2) const
	{
		return key1 == key2;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline void NiTMapBase<TKEY,TVAL>::SetValue(NiTMapItem<TKEY, TVAL>* pkItem, TKEY key, TVAL val)
	{
		pkItem->m_key = key;
		pkItem->m_val = val;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	inline void NiTMapBase<TKEY, TVAL>::ClearValue(NiTMapItem<TKEY, TVAL>* /* pkItem */)
	{
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline NiTMapIterator NiTMapBase<TKEY,TVAL>::GetFirstPos() const
	{
		for (UInt32 ui = 0; ui < m_uiHashSize; ui++) 
		{
			if (m_ppkHashTable[ui])
				return m_ppkHashTable[ui];
		}
		return 0;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL> 
	inline void NiTMapBase<TKEY,TVAL>::GetNext(NiTMapIterator& pos, TKEY& key, TVAL& val) const
	{
		NiTMapItem<TKEY, TVAL>* pkItem = (NiTMapItem<TKEY, TVAL>*) pos;

		key = pkItem->m_key;
		val = pkItem->m_val;

		if (pkItem->m_pkNext)
		{
			pos = pkItem->m_pkNext;
			return;
		}

		UInt32 ui = KeyToHashIndex(pkItem->m_key);
		for (++ui; ui < m_uiHashSize; ui++) 
		{
			pkItem = m_ppkHashTable[ui];
			if (pkItem) 
			{
				pos = pkItem;
				return;
			}
		}

		pos = 0;
	}
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	inline void NiTMapBase<TKEY,TVAL>::Resize(UInt32 uiNewHashSize)
	{
		assert(uiNewHashSize > 0);

		// store old members
		NiTMapItem<TKEY,TVAL>** oldHashTable = m_ppkHashTable;
		UInt32 oldHashSize = m_uiHashSize;
		UInt32 oldCount = m_uiCount;    

		// re-initialize 
		m_uiHashSize = uiNewHashSize;
		m_uiCount = 0;
		UInt32 uiSize = sizeof(NiTMapItem<TKEY,TVAL>*) * m_uiHashSize;
		m_ppkHashTable = (NiTMapItem<TKEY,TVAL>**)MemoryHeap::FormHeapAlloc(uiSize);
		assert(m_ppkHashTable);
		memset(m_ppkHashTable, 0, uiSize);

		// Go through all entries in the old hash array and transfer to the new hash array.
		for (UInt32 ui = 0; ui < oldHashSize; ui++) 
		{
			while (oldHashTable[ui])
			{
				// Remove the entry from the old array
				NiTMapItem<TKEY, TVAL>* pkItem = oldHashTable[ui];
				oldHashTable[ui] = pkItem->m_pkNext;
				oldCount--;

				// Clear values in the entry
				pkItem->m_pkNext = 0;

				// Insert the entry in the new map
				UInt32 uiIndex = KeyToHashIndex(pkItem->m_key);
				pkItem->m_pkNext = m_ppkHashTable[uiIndex];
				m_ppkHashTable[uiIndex] = pkItem;
				m_uiCount++;
			}
		}

		// At the end, the existing hash array should be empty, and we can just delete it.
		assert(oldCount == 0);
		MemoryHeap::FormHeapFree(oldHashTable);
	}

	// NiTMap
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	const UInt32 NiTMap<TKEY, TVAL>::NUM_PRIMES = 18;
	//---------------------------------------------------------------------------
	template <class TKEY, class TVAL>
	const UInt32 NiTMap<TKEY, TVAL>::PRIMES[18] = {
		7, 13, 31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 32749,
		65521, 131071, 262139, 524287, 999983};
		//---------------------------------------------------------------------------
		template <class TKEY, class TVAL> 
		inline NiTMap<TKEY, TVAL>::~NiTMap()
		{
			// RemoveAll is called from here because it depends on virtual functions
			// implemented in NiTAllocatorMap.  It will also be called in the 
			// parent destructor, but the map will already be empty.
			NiTMap<TKEY, TVAL>::RemoveAll();
		}
		//---------------------------------------------------------------------------
		template <class TKEY, class TVAL> 
		inline NiTMapItem<TKEY, TVAL>* NiTMap<TKEY, TVAL>::NewItem()
		{
			return new NiTMapItem<TKEY, TVAL>;
		}
		//---------------------------------------------------------------------------
		template <class TKEY, class TVAL> 
		inline void NiTMap<TKEY, TVAL>::DeleteItem(NiTMapItem<TKEY, TVAL>* pkItem)
		{
			// set key and val to zero so that if they are smart pointers
			// their references will be decremented.
			pkItem->m_val = 0;
			delete pkItem;
		}
		//---------------------------------------------------------------------------
		template <class TKEY, class TVAL>
		inline UInt32 NiTMap<TKEY, TVAL>::NextPrime(const UInt32 uiTarget)
		{
			UInt32 ui;
			for (ui = 0; ui < NUM_PRIMES - 1 && PRIMES[ui] < uiTarget; ui++) { }
			return PRIMES[ui];
		}

		// NiTPointerMap
		//---------------------------------------------------------------------------
		template <class TKEY, class TVAL> 
		inline NiTPointerMap<TKEY,TVAL>::~NiTPointerMap()
		{
			// RemoveAll is called from here because it depends on virtual functions
			// implemented in NiTAllocatorMap.  It will also be called in the 
			// parent destructor, but the map will already be empty.
			NiTPointerMap<TKEY,TVAL>::RemoveAll();
		}
		//---------------------------------------------------------------------------
		template <class TKEY, class TVAL> 
		inline NiTMapItem<TKEY, TVAL>* NiTPointerMap<TKEY, TVAL>::NewItem()
		{
			return new NiTMapItem<TKEY, TVAL>;
		}
		//---------------------------------------------------------------------------
		template <class TKEY, class TVAL> 
		inline void NiTPointerMap<TKEY, TVAL>::DeleteItem(NiTMapItem<TKEY, TVAL>* pkItem)
		{
			// set key and val to zero so that if they are smart pointers
			// their references will be decremented.
			pkItem->m_val = 0;
			delete pkItem;
		}

		// NiTStringTemplateMap
		//---------------------------------------------------------------------------
		template <class TPARENT, class TVAL>  
		inline NiTStringTemplateMap<TPARENT,TVAL>::~NiTStringTemplateMap()
		{
			if (m_bCopy)
			{
				for (unsigned int i = 0; i < TPARENT::m_uiHashSize; i++) 
				{
					NiTMapItem<const char*, TVAL>* pkItem = TPARENT::m_ppkHashTable[i];
					while (pkItem) 
					{
						NiTMapItem<const char*, TVAL>* pkSave = pkItem;
						pkItem = pkItem->m_pkNext;
						FormHeap_Free((void*)pkSave->m_key);
					}
				}
			}
		}
		//---------------------------------------------------------------------------
		template <class TPARENT, class TVAL> 
		inline void NiTStringTemplateMap<TPARENT,TVAL>::SetValue(NiTMapItem<const char*, TVAL>* pkItem, const char* pcKey, TVAL val)
		{
			if (m_bCopy)
			{
				size_t stLen = strlen(pcKey) + 1;
				pkItem->m_key = (char*)FormHeap_Allocate(stLen);
				assert(pkItem->m_key);
				strcpy_s((char*)pkItem->m_key, stLen, pcKey);
			}
			else
			{
				pkItem->m_key = pcKey;
			}
			pkItem->m_val = val;
		}
		//---------------------------------------------------------------------------
		template <class TPARENT, class TVAL>  
		inline void NiTStringTemplateMap<TPARENT,TVAL>::ClearValue(NiTMapItem<const char*, TVAL>* pkItem)
		{
			if (m_bCopy)
			{
				FormHeap_Free((void*)pkItem->m_key);
			}
		}
		//---------------------------------------------------------------------------
		template <class TPARENT, class TVAL> 
		inline UInt32 NiTStringTemplateMap<TPARENT,TVAL>::KeyToHashIndex(const char* pKey) const
		{
			UInt32 uiHash = 0;

			while (*pKey)
				uiHash = (uiHash << 5) + uiHash + *pKey++;

			return uiHash % m_uiHashSize;
		}
		//---------------------------------------------------------------------------
		template <class TPARENT, class TVAL> 
		inline bool NiTStringTemplateMap<TPARENT,TVAL>::IsKeysEqual(const char* pcKey1, const char* pcKey2) const
		{
			return strcmp(pcKey1, pcKey2) == 0;
			// NOTE - this implementation uses the intended case sensitive string comparison
			// However, Bethesda's code uses a case-insensitive comparison (see associated notes in NiTMap.h).
			// This version is more correct, however it may in rare cases not agree with the Bethesda code.
		}
		//---------------------------------------------------------------------------

}