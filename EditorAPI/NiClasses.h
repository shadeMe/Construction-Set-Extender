#pragma once
#include "obse\NiNodes.h"

//	EditorAPI: Misc. NetImmerse classes.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding
//

/*
    NiBinaryStream: generalized platform-independent binary i/o

    [Original] Notes:
    This abstract base class for all binary streams, such as memory streams or files, is designed
    to wrap platform-dependent implementations of streaming systems.

    Derived classes need to implement some particular behavior to support endian swapping.  Such
    details are described in the documentation about how to derive from BinaryStream.  By default,
    all files will be written in the endianness of the current platform.
*/

// 0C
class NiBinaryStream
{
public:
    // read/write function callbacks.  Component size and number are for custom byte ordering
    typedef UInt32 (*ReadFNT)(NiBinaryStream* pkThis, void* pvBuffer, UInt32 uiBytes, UInt32* puiComponentSizes, UInt32 uiNumComponents);
    typedef UInt32 (*WriteFNT)(NiBinaryStream* pkThis, const void* pvBuffer, UInt32 uiBytes, UInt32* puiComponentSizes, UInt32 uiNumComponents);

    // members
    //     /*00*/ void**        vtbl;
    /*04*/ ReadFNT				m_pfnRead;  // called to actually read from stream
    /*08*/ WriteFNT				m_pfnWrite; // called to actually write to stream

	// virtual methods
	virtual void				Dtor();
};
STATIC_ASSERT(sizeof(NiBinaryStream) == 0xC);

/*
    NiFile: Basic NetImmerse file class

    [Original] Notes:
    Platform-independent abstract class that defines the interface for writing to
    and reading from files.  In addition to defining the interface it also provides
    a mechanism to replace the default concrete implementation of the File interface.
*/

// 28
class NiFile : public NiBinaryStream
{
public:
    enum FileModes
    {
        kFileMode_ReadOnly      = 0,
        kFileMode_WriteOnly     = 1,
        kFileMode_AppendOnly    = 2,
    };

    // members
    //     /*00*/ NiBinaryStream
    /*0C*/ UInt32        m_uiBufferAllocSize;	// size of buffer
    /*10*/ UInt32        m_uiBufferReadSize;	// init to 0
    /*14*/ UInt32        m_uiPos;				// init to 0
    /*18*/ void*         m_pBuffer;				// allocated/deallocated on form heap by constructor/destructor
    /*1C*/ FILE*         m_pFile;
    /*20*/ UInt32        m_eMode;				// file mode
    /*24*/ bool          m_bGood;				// true if file is open
};
STATIC_ASSERT(sizeof(NiFile) == 0x28);

class	Ni2DBuffer;

// 14
class NiDX92DBufferData : public NiRefObject
{
public:
	// members
    //     /*00*/ NiRefObject
	/*08*/ Ni2DBuffer*			parentData;
	/*0C*/ IDirect3DSurface9*	surface;
	/*10*/ TextureFormat*		surfaceData;
};
STATIC_ASSERT(sizeof(NiDX92DBufferData) == 0x14);