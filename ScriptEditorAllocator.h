#pragma once
#include "ExtenderInternals.h"

class EditorAllocator
{
	static EditorAllocator*													Singleton;

	struct SEAlloc
	{
		HWND																RichEditControl;
		HWND																ListBoxControl;
		UInt32																Index;

																			SEAlloc(HWND REC, HWND LBC, UInt32 IDX):
																			RichEditControl(REC), ListBoxControl(LBC), Index(IDX) {};
	};

	typedef std::map<HWND, SEAlloc*>										AlMap;
	AlMap																	AllocationMap;
	UInt32																	NextIndex;
	UInt32																	LastContactedEditor;
public:
	static EditorAllocator*													GetSingleton(void);
	UInt32																	TrackNewEditor(HWND EditorDialog);
	void																	DeleteTrackedEditor(UInt32 TrackedEditorIndex);
	void																	DeleteAllTrackedEditors(void);
	void																	DestroyVanillaDialogs(void);

	HWND																	GetTrackedREC(HWND TrackedEditorDialog);
	HWND																	GetTrackedLBC(HWND TrackedEditorDialog);
	UInt32																	GetTrackedIndex(HWND TrackedEditorDialog);
	HWND																	GetTrackedDialog(UInt32 TrackedEditorIndex);
	UInt32																	GetTrackedEditorCount() { return AllocationMap.size(); }
	UInt32																	GetLastContactedEditor() { return LastContactedEditor; }
	void																	SetLastContactedEditor(UInt32 TrackedEditorIndex) { LastContactedEditor = TrackedEditorIndex; }
};

#define EDAL																EditorAllocator::GetSingleton()