#pragma once

//	EditorAPI: MemoryHeap class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    Memory heap class and it's main global instance, the FormHeap.
    This appears to be Bethesda's answer to the global new & delete operators.
*/

// 170 (actually 16E, but aligned to word boundary)
class MemoryHeap
{
public:
    // memory panic callback
    typedef void (*Unk164Callback)(UInt32 unk0, UInt32 unk1, UInt32 unk2);

    // members
    //     /*000*/ void**        vtbl;    
    /*004*/ UInt32               unk004; // alignment
    /*008*/ UInt32               unk008;
    /*00C*/ UInt32               unk00C; // size of main memory block
    /*010*/ UInt32               unk010;
    /*014*/ UInt32               unk014;
    /*018*/ void*                unk018; // main memory block
    /*01C*/ UInt32               unk01C;
    /*020*/ UInt32               unk020;
    /*024*/ UInt32               unk024;
    /*028*/ UInt32               unk028;
    /*02C*/ UInt32               unk02C;
    /*030*/ UInt32               unk030; // size of unk034 / 8
    /*034*/ void*                unk034; // 0x2000 byte buffer
    /*038*/ void*                unk038; // end of unk034
    /*03C*/ UInt32               unk03C;
    /*040*/ UInt32               unk040;
    /*044*/ void*                unk044;
    /*048*/ UInt32               unk048;
    /*04C*/ UInt32               unk04C;
    /*050*/ UInt32               unk050;
    /*054*/ UInt32               unk054; // available memory at startup
    /*058*/ UInt32               unk058;
    /*05C*/ UInt32               unk05C;
    /*060*/ UInt32               unk060;
    /*064*/ UInt32               unk064;
    /*068*/ UInt8                unk068[0x164-0x068];
    /*164*/ Unk164Callback       unk164;
    /*168*/ UInt32               unk168; // used memory at startup
    /*16C*/ UInt8                unk16C;
    /*16D*/ UInt8                unk16D;
    /*16E*/ UInt8                pad16E[2];
};

void*	FormHeap_Allocate(UInt32 Size);
void	FormHeap_Free(void* Ptr);

extern MemoryHeap*	g_FormHeap;

// macro for overloading new & delete operators to use the FormHeap
#define USEFORMHEAP inline void* operator new (size_t size) {return FormHeap_Allocate(size);} \
                    inline void* operator new[] (size_t size) {return FormHeap_Allocate(size);} \
                    inline void operator delete (void *object) {FormHeap_Free(object);} \
                    inline void operator delete[] (void *object) {FormHeap_Free(object);}