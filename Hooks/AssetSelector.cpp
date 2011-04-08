#include "AssetSelector.h"

const char* g_AssetSelectorReturnPath = NULL;

_DefineHookHdlr(TESDialogShowDDSCommonDialogProlog, 0x004A414B);
_DefineHookHdlr(TESDialogShowNIFCommonDialogProlog, 0x0049BDAB);
_DefineHookHdlr(TESDialogShowDDSCommonDialogEpilog, 0x00446CDC);
_DefineHookHdlr(TESDialogShowNIFCommonDialogEpilog, 0x00446C94);

void PatchAssetSelectorHooks(void)
{
//	PatchCommonDialogCancelHandler(Model);
	PatchCommonDialogCancelHandler(Animation);
	PatchCommonDialogCancelHandler(Sound);
//	PatchCommonDialogCancelHandler(Texture);
	PatchCommonDialogCancelHandler(SPT);

//	PatchCommonDialogPrologHandler(Model);
	PatchCommonDialogPrologHandler(Animation);
	PatchCommonDialogPrologHandler(Sound);
//	PatchCommonDialogPrologHandler(Texture);
	PatchCommonDialogPrologHandler(SPT);

//	PatchCommonDialogEpilogHandler(Model);
	PatchCommonDialogEpilogHandler(Animation);
	PatchCommonDialogEpilogHandler(Sound);
//	PatchCommonDialogEpilogHandler(Texture);
	PatchCommonDialogEpilogHandler(SPT);

	_MemoryHandler(TESDialogShowDDSCommonDialogProlog).WriteJump();
	_MemoryHandler(TESDialogShowNIFCommonDialogProlog).WriteJump();
	_MemoryHandler(TESDialogShowDDSCommonDialogEpilog).WriteJump();
	_MemoryHandler(TESDialogShowNIFCommonDialogEpilog).WriteJump();
}


DefineCommonDialogCancelHandler(Model)
DefineCommonDialogCancelHandler(Animation)
DefineCommonDialogCancelHandler(Sound)
DefineCommonDialogCancelHandler(Texture)
DefineCommonDialogCancelHandler(SPT)

UInt32 __stdcall InitBSAViewer(UInt32 Filter)
{
	switch (Filter)
	{
	case e_NIF:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "nif");
		break;
	case e_KF:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "kf");
		break;
	case e_WAV:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "wav");
		break;
	case e_DDS:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "dds");
		break;
	case e_SPT:
		g_AssetSelectorReturnPath = CLIWrapper::BSAViewer::InitializeViewer(g_AppPath.c_str(), "spt");
		break;
	}

	if (!g_AssetSelectorReturnPath)
		return 0;
	else
		return e_FetchPath;
}

UInt32 __stdcall InitPathEditor(int ID, const char* ExistingPath, HWND Dialog)
{
	if (!ExistingPath)
		GetDlgItemText(Dialog, ID, g_Buffer, sizeof(g_Buffer));
	else
		PrintToBuffer("%s", ExistingPath);

	g_AssetSelectorReturnPath = (const char*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_TEXTEDIT), Dialog, (DLGPROC)TextEditDlgProc, (LPARAM)g_Buffer);

	if (!g_AssetSelectorReturnPath)
		return 0;
	else
		return e_FetchPath;
}

UInt32 __stdcall InitPathCopier(UInt32 Filter, HWND Dialog)
{
	g_AssetSelectorReturnPath = (const char*)DialogBoxParam(g_DLLInstance, MAKEINTRESOURCE(DLG_COPYPATH), Dialog, (DLGPROC)CopyPathDlgProc, (LPARAM)Filter);
	if (!g_AssetSelectorReturnPath)
		return 0;
	else
		return e_FetchPath;
}

UInt32 __stdcall InitAssetSelectorDlg(HWND Dialog)
{
	return DialogBox(g_DLLInstance, MAKEINTRESOURCE(DLG_ASSETSEL), Dialog, (DLGPROC)AssetSelectorDlgProc);
}

DefineCommonDialogPrologHandler(Model)
DefineCommonDialogPrologHandler(Animation)
DefineCommonDialogPrologHandler(Sound)
DefineCommonDialogPrologHandler(Texture)
DefineCommonDialogPrologHandler(SPT)

void __declspec(naked) ModelPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath	
		jz		SELECT

		lea		eax, [esp + 0x14]
        jmp     [kModelPostCommonDialogRetnAddr]
	SELECT:
		mov		eax, g_AssetSelectorReturnPath
        jmp     [kModelPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) AnimationPostCommonDialogHook(void)     
{
    __asm
    {
		mov		ebx, eax

		mov     eax, [esi + 0x24]
		push    ebx
		push    eax
		lea     ecx, [ebp - 0x14]
		mov     byte ptr [ebp - 0x4], 1
		call    kBSString_Set

		cmp		ebx, e_FetchPath
		jz		SELECT

		lea		edx, [ebp]
        jmp		POST
	SELECT:
		mov		edx, g_AssetSelectorReturnPath
	POST:
		push	edx
		lea		ecx, [esp + 0x24]
        jmp     [kAnimationPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) SoundPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath	
		jz		SELECT

		lea		ecx, [esp + 8]
		push	ecx
        jmp     [kSoundPostCommonDialogRetnAddr]
	SELECT:
		mov		ecx, g_AssetSelectorReturnPath
		push	ecx
        jmp     [kSoundPostCommonDialogRetnAddr]
    }
}
void __declspec(naked) TexturePostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath
		jz		SELECT

		lea		eax, [ebp]
        jmp		POST
	SELECT:
		mov		eax, g_AssetSelectorReturnPath
	POST:
		push	eax
		lea		ecx, [ebp - 0x14]
        jmp     [kTexturePostCommonDialogRetnAddr]
    }
}
void __declspec(naked) SPTPostCommonDialogHook(void)     
{
    __asm
    {
		cmp		eax, e_FetchPath
		jz		SELECT

		lea		ecx, [esp + 0x14]
        jmp     [kSPTPostCommonDialogRetnAddr]
	SELECT:
		mov		ecx, g_AssetSelectorReturnPath
        jmp     [kSPTPostCommonDialogRetnAddr]
    }
}


void __stdcall DoTESDialogShowCommonDialogPrologHook(HWND Dialog, int Identifier, char* Buffer)
{
	GetDlgItemText(Dialog, Identifier, Buffer, 0x104);
}

_BeginHookHdlrFn(TESDialogShowDDSCommonDialogProlog)
{
	_DeclareHookHdlrFnVariable(TESDialogShowDDSCommonDialogProlog, Retn, 0x004A4150);
	_DeclareHookHdlrFnVariable(TESDialogShowDDSCommonDialogProlog, Call, 0x00446CA0);
	__asm
	{
		pushad
		push	ebp
		push	[esi + 0x10]
		push	edi
		call	DoTESDialogShowCommonDialogPrologHook
		popad

		call	[_HookHdlrFnVariable(TESDialogShowDDSCommonDialogProlog, Call)]
		jmp		[_HookHdlrFnVariable(TESDialogShowDDSCommonDialogProlog, Retn)]
	}
}

_BeginHookHdlrFn(TESDialogShowNIFCommonDialogProlog)
{
	_DeclareHookHdlrFnVariable(TESDialogShowNIFCommonDialogProlog, Retn, 0x0049BDB0);
	_DeclareHookHdlrFnVariable(TESDialogShowNIFCommonDialogProlog, Call, 0x00446C60);
	__asm
	{
		pushad
		push	edx
		push	[edi + 0x20]
		push	esi
		call	DoTESDialogShowCommonDialogPrologHook
		popad

		call	[_HookHdlrFnVariable(TESDialogShowNIFCommonDialogProlog, Call)]
		jmp		[_HookHdlrFnVariable(TESDialogShowNIFCommonDialogProlog, Retn)]
	}
}

void __stdcall DoMemCpy(void* Source, void* Dest, size_t Size)
{
	ZeroMemory(Dest, Size);
	memcpy(Dest, Source, Size);
}

static char* s_AssetSelectorPathBuffer = new char[0x104];		// flabbergastingly hacky, but that's the price of being a black sheep

_BeginHookHdlrFn(TESDialogShowDDSCommonDialogEpilog)
{
	_DeclareHookHdlrFnVariable(TESDialogShowDDSCommonDialogEpilog, Retn, 0x00446CE1);
	_DeclareHookHdlrFnVariable(TESDialogShowDDSCommonDialogEpilog, Call, 0x00446A30);
	__asm
	{
		mov		eax, [esp]	// parent HWND
		push	esi
		mov		esi, ebp 	// path buffer

		pushad  
		push	eax 
		call	InitAssetSelectorDlg  
		cmp		eax, e_Close 
		jz		CLOSE 
		cmp		eax, e_FileBrowser  
		jz		FILEB  
		cmp		eax, e_BSABrowser 
		jz		BSAB 
		cmp		eax, e_EditPath 
		jz		EDITP 
		cmp		eax, e_CopyPath 
		jz		COPYP 
		popad  

		// clear path
		xor		eax, eax
		jmp		EXIT
	CLOSE: 
		popad 
		mov		eax, 1
		jmp		EXIT
	FILEB: 
		push	0x104
		push	s_AssetSelectorPathBuffer
		push	esi
		call	DoMemCpy
		popad 

		pop		esi
		call	[_HookHdlrFnVariable(TESDialogShowDDSCommonDialogEpilog, Call)]
		test	al, al
		jz		RESETPATH
		
		mov		eax, 1
		jmp		[_HookHdlrFnVariable(TESDialogShowDDSCommonDialogEpilog, Retn)]
	RESETPATH:
		push	0x104
		push	ebp
		push	s_AssetSelectorPathBuffer
		call	DoMemCpy

		mov		eax, 1
		jmp		[_HookHdlrFnVariable(TESDialogShowDDSCommonDialogEpilog, Retn)]
	BSAB:  
		popad  
		push	kTextureSelectorCommonDialogFilterType
		call	InitBSAViewer 
		jmp		FETCH
	EDITP: 
		popad 
		push	eax 
		push	esi 
		push	0
		call	InitPathEditor 
		jmp		FETCH   
	COPYP: 
		popad 
		push	eax 
		push	kTextureSelectorCommonDialogFilterType
		call	InitPathCopier 
		jmp		FETCH
	FETCH:
		cmp		eax, e_FetchPath
		mov		eax, 1
		jz		FIXPATH
		jmp		EXIT 	
	FIXPATH:
		push	0x104
		push	esi
		push	g_AssetSelectorReturnPath
		call	DoMemCpy
	EXIT:
		pop		esi
		jmp		[_HookHdlrFnVariable(TESDialogShowDDSCommonDialogEpilog, Retn)]
	}
}

_BeginHookHdlrFn(TESDialogShowNIFCommonDialogEpilog)
{
	_DeclareHookHdlrFnVariable(TESDialogShowNIFCommonDialogEpilog, Retn, 0x00446C99);
	_DeclareHookHdlrFnVariable(TESDialogShowNIFCommonDialogEpilog, Call, 0x00446A30);
	__asm
	{
		mov		eax, [esp]
		push	esi
		mov		esi, [esp + 0x38]

		pushad  
		push	eax 
		call	InitAssetSelectorDlg  
		cmp		eax, e_Close 
		jz		CLOSE 
		cmp		eax, e_FileBrowser  
		jz		FILEB  
		cmp		eax, e_BSABrowser 
		jz		BSAB 
		cmp		eax, e_EditPath 
		jz		EDITP 
		cmp		eax, e_CopyPath 
		jz		COPYP 
		popad  

		// clear path
		xor		eax, eax
		jmp		EXIT
	CLOSE: 
		popad 
		mov		eax, 1
		jmp		EXIT
	FILEB: 
		push	0x104
		push	s_AssetSelectorPathBuffer
		push	esi
		call	DoMemCpy
		popad 

		pop		esi
		call	[_HookHdlrFnVariable(TESDialogShowNIFCommonDialogEpilog, Call)] 
		test	al, al
		jz		RESETPATH

		mov		eax, 1
		jmp		[_HookHdlrFnVariable(TESDialogShowNIFCommonDialogEpilog, Retn)]
	RESETPATH:
		push	0x104
		mov		eax, esp
		add		eax, 0x54
		push	eax
		push	s_AssetSelectorPathBuffer
		call	DoMemCpy

		mov		eax, 1
		jmp		[_HookHdlrFnVariable(TESDialogShowNIFCommonDialogEpilog, Retn)]
	BSAB:  
		popad  
		push	kModelSelectorCommonDialogFilterType
		call	InitBSAViewer 
		jmp		FETCH
	EDITP: 
		popad 
		push	eax 
		push	esi 
		push	0
		call	InitPathEditor 
		jmp		FETCH   
	COPYP: 
		popad 
		push	eax 
		push	kModelSelectorCommonDialogFilterType
		call	InitPathCopier 
		jmp		FETCH
	FETCH:
		cmp		eax, e_FetchPath
		mov		eax, 1
		jz		FIXPATH
		jmp		EXIT  	
	FIXPATH:
		push	0x104
		push	esi
		push	g_AssetSelectorReturnPath
		call	DoMemCpy
	EXIT:
		pop		esi
		jmp		[_HookHdlrFnVariable(TESDialogShowNIFCommonDialogEpilog, Retn)]
	}
}