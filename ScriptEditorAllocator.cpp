#include "ScriptEditorAllocator.h"

EditorAllocator*					EditorAllocator::Singleton = NULL;

EditorAllocator* EditorAllocator::GetSingleton(void)
{
	if (!Singleton)
	{
		EditorAllocator::Singleton = new EditorAllocator;
		Singleton->NextIndex = 1;
	}
	return Singleton;
}

UInt32 EditorAllocator::TrackNewEditor(HWND EditorDialog)
{
	UInt32 Result = NextIndex++;
	AllocationMap.insert(std::make_pair<HWND, SEAlloc*>(EditorDialog, new SEAlloc(GetDlgItem(EditorDialog, 1166), GetDlgItem(EditorDialog, 2259), Result)));
	if (NextIndex == 2147483648)
		MessageBox(*g_HWND_CSParent, "Holy crap, mate! I have no idea how you managed to create 2147483648 editor workspaces xO I'd suggest that you pack up some essentials and head to the Andes as the next allocation is certain to warp the space-time continuum in unimaginable ways.\n\nDamn you...", "The Developer Speaks", MB_HELP|MB_ICONSTOP);

	return Result;
}

void EditorAllocator::DeleteTrackedEditor(UInt32 TrackedEditorIndex)
{
	for (AlMap::iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++)
	{
		if (Itr->second->Index == TrackedEditorIndex)
		{
			delete Itr->second;
			AllocationMap.erase(Itr);
			break;
		}
	}
}

void EditorAllocator::DeleteAllTrackedEditors(void)
{
	for (AlMap::iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++)
	{
		delete Itr->second;
	}
	AllocationMap.clear();
}

void EditorAllocator::DestroyVanillaDialogs(void)
{
	for (AlMap::iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++)
		DestroyWindow(Itr->first);
}

HWND EditorAllocator::GetTrackedREC(HWND TrackedEditorDialog)
{
	AlMap::const_iterator Itr = AllocationMap.find(TrackedEditorDialog);
	if (Itr == AllocationMap.end())
		return NULL;
	else
		return Itr->second->RichEditControl;
}

HWND EditorAllocator::GetTrackedLBC(HWND TrackedEditorDialog)
{
	AlMap::const_iterator Itr = AllocationMap.find(TrackedEditorDialog);
	if (Itr == AllocationMap.end())
		return NULL;
	else
		return Itr->second->ListBoxControl;
}

UInt32 EditorAllocator::GetTrackedIndex(HWND TrackedEditorDialog)
{
	UInt32 Result = 0;
	AlMap::const_iterator Itr = AllocationMap.find(TrackedEditorDialog);
	if (Itr == AllocationMap.end())
		return 0;
	else
		return Itr->second->Index;
}

HWND EditorAllocator::GetTrackedDialog(UInt32 TrackedEditorIndex)
{
	HWND Result= NULL;
	for (AlMap::const_iterator Itr = AllocationMap.begin(); Itr != AllocationMap.end(); Itr++)
	{
		Result = Itr->first;
		if (Itr->second->Index == TrackedEditorIndex)
			break;
	}
	return Result;
}