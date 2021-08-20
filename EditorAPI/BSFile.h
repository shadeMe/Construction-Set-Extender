#pragma once

#include "NiClasses.h"

//	EditorAPI: BSFile class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	BSFile: basic Bethesda file class
*/

// 154
class BSFile : public NiFile
{
public:
	// members
	//     /*000*/ NiFile
	/*028*/ UInt32			unk28;
	/*02C*/ UInt32			unk2C;
	/*030*/ UInt32			unk30;  // init to -1
	/*034*/ UInt32			unk34;
	/*038*/ UInt32			unk38;
	/*03C*/ char			fileName[0x104];
	/*140*/ UInt32			unk140;
	/*144*/ UInt32			unk144;
	/*148*/ UInt32			bsPos;  // (?) tracks current offset like NiFile::m_uiPos, but not reset by Flush()
	/*14C*/ UInt32			unk14C;
	/*150*/ UInt32			fileSize;

	// methods
	void					Flush();
	UInt32					GetFileSize();
	UInt32					DirectRead(void* buffer, UInt32 size);
	UInt32					DirectWrite(const void* buffer, UInt32 size);

	static BSFile*			CreateInstance(const char* fileName, UInt32 mode, UInt32 bufferSize = 0x8000, bool isTextFile = false);

	// probably belongs elswhere
	static const char*		GetRelativePath(const char* FilePath, const char* RelativeTo);

	void					DeleteInstance(bool ReleaseMemory = true);
};
STATIC_ASSERT(sizeof(BSFile) == 0x154);