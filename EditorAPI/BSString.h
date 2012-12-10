#pragma once

//	EditorAPI: BSString class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	Basic dynamically-sizeable string class.

	High level string manipulation - comparison, replacement, etc. seems to be done 'manually'
	using cstdlib functions rather than through member functions.  That doesn't mean that
	member functions for comparison, etc. don't exist, but if they do they are very hard to find.
	The exception so far is 'sprintf', which has a member function wrapper.
*/

// 08
class BSString
{
protected:
	// members
	/*00*/ char*       m_data;
	/*04*/ SInt16      m_dataLen;
	/*06*/ SInt16      m_bufLen;
public:
	// methods
	bool							Set(const char* string, SInt16 size = 0);   // size determines allocated storage? 0 to allocate automatically
	void							Clear();
	SInt16							Size() const;
	SInt16							Compare(const char* string, bool ignoreCase = true);
	const char*						c_str() const                  { return m_data; }

	static BSString*				CreateInstance(const char* String);
	void							DeleteInstance(bool ReleaseMemory = true);
};
STATIC_ASSERT(sizeof(BSString) == 0x08);