#pragma once
#include "Hooks-Common.h"

namespace ConstructionSetExtender
{
	namespace Hooks
	{
		// hooks that modify how CS dialogs behave and add interfaces for new tools
		void PatchDialogHooks(void);

		_DeclareNopHdlr(ResponseEditorMic, "nops out the call to the delinquent sound struct initializer sub, fixing a crash");
		_DeclareMemHdlr(TopicResultScriptReset, "fixes the bug that clears all flags and the result script of a selected response when adding a new topic");
		_DeclareMemHdlr(NPCFaceGen, "fixes the facegen crash by getting the CS to correctly render the model at dialog init");
		_DeclareMemHdlr(CreaturePreview, "enables the preview control in teh creature edit dialog on startup");
		_DeclareMemHdlr(CustomCSWindow, "keeps custom child windows of the CS main window from being closed on plugin load");
		_DeclareMemHdlr(RaceDescriptionDirtyEdit, "prevent dirty edits occuring when you edit a race's text description and click directly to another race without switching tabs first, if the spellchecker pops up (which it will), the description for the race you were previously working on gets copied into the one you just selected.");
		_DeclareMemHdlr(AddListViewItem, "patches various routines to check for the 'Hide UnModified/Deleted Forms' flag before populating controls with forms");
		_DeclareMemHdlr(AddComboBoxItem, "");
		_DeclareMemHdlr(ObjectListPopulateListViewItems, "");
		_DeclareMemHdlr(CellViewPopulateObjectList, "");
		_DeclareMemHdlr(CellObjectListShadeMeRefAppend, "");
		_DeclareMemHdlr(DeathToTheCloseOpenDialogsMessage, "gets rid of it");
		_DeclareMemHdlr(TESDialogPopupMenu, "hooks popup menu instantiation for new menu items");
		_DeclareMemHdlr(DataDlgZOrder, "prevents the data dlg from staying on top of all other windows");
		_DeclareMemHdlr(FormIDListViewSelectItem, "fixes a bug that prevents the correct selection of newly created list view items");
		_DeclareMemHdlr(FormIDListViewDuplicateSelection, "increments the item index returned by TESDialog::LookupListViewItemByData when duplicating forms from the popup menu");
		_DeclareMemHdlr(TESDialogGetIsWindowDragDropRecipient, "allows custom windows to receive form drag-drop notifications");
		_DeclareNopHdlr(MissingTextureWarning, "removes the ostentatious warning");
		_DeclareMemHdlr(TESQuestStageResultScript, "fixes a bug that prevented quest stage result scripts from being compiled when their text was empty");
		_DeclareMemHdlr(TESNPCUpdatePreviewControl, "fixes a bug that prevented NPC models from being updated correctly after an item was removed from their inventory");
		_DeclareMemHdlr(TESParametersFillAndInitSelectionComboBoxOwners, "allows factions to be passed as arguments to condition commands that take ownership parameters");
		_DeclareMemHdlr(SearchReplaceDialog, "patches the search and replace dialog to stay open after a replace operation");
		_DeclareMemHdlr(ObjectWindowPopulateFormListInvalidate, "invalidates the formList before items are added to it to increase performance");
		_DeclareMemHdlr(CellViewWindowResizeFix, "fixes a bug that kept eating the cell view's list view control heights after each reinit");
		_DeclareMemHdlr(TESSoundPlayFile, "patches the sound file sampler code to play files found in archives");
		_DeclareMemHdlr(FindTextFormEnumerationA, "patches the find text dialog to account for the new 'Hide XXX records' tools");
		_DeclareMemHdlr(FindTextFormEnumerationB, "");
		_DeclareMemHdlr(FindTextFormEnumerationC, "");
		_DeclareMemHdlr(TESQuestRemoveStageData, "adds a workaround for a bug that causes a CTD when deleting quest stages in the theme-enabled EXE");
		_DeclareMemHdlr(HideCSMainDialogsA, "hides the main cs windows instead of minimizing them");
		_DeclareMemHdlr(HideCSMainDialogsB, "");
		_DeclareMemHdlr(HideCSMainDialogsC, "");
		_DeclareMemHdlr(HideCSMainDialogsStartup, "preserves the window state of the main windows b'ween sessions");
		_DeclareMemHdlr(ReleaseModelessDialogsA, "fixes a long standing bug in the TES editor code (Todd-Code, I'm told) that improperly destroys modeless dialogs; DistantLOD Export");
		_DeclareMemHdlr(ReleaseModelessDialogsB, "DistantLOD Export");
		_DeclareMemHdlr(ReleaseModelessDialogsC, "TESForm Common");
		_DeclareMemHdlr(ReleaseModelessDialogsD, "");
		_DeclareMemHdlr(ReleaseModelessDialogsE, "");
		_DeclareMemHdlr(SubwindowTemplateHotSwap, "adds support for the hotswapping of subwindow dialog templates");
		_DeclareMemHdlr(CellViewInitDialog, "prevents the default handler from screwing with the new controls");
		_DeclareMemHdlr(TESQuestWindowResize, "prevents quest edit window from being resized unnecessarily");
		_DeclareMemHdlr(FilteredDialogWindowResize, "prevents filtered dialog window from being resized unnecessarily");
		_DeclareMemHdlr(DialogueEditorPopup, "prevents the addition of TESForm related context menus items in the dialogue editor");
		_DeclareMemHdlr(TESWeatherSoundListSort, "fixes a bug that causes a CTD when the sound list view in the edit dialog was sorted");
		_DeclareMemHdlr(TESFormShowCrossRefList, "shows the form cross ref list as modal dialog. proper modal");
		_DeclareNopHdlr(TESColorControlWndProc, "fixes a bug that causes large parts of the desktop screen to 'blackout' when dialogs didn't handle their TESColorControl messages correctly");
		_DeclareMemHdlr(TESPackageWndProcAddNew, "prevents the packages listview from being repopulated every time a new package is created (by renaming an existing one). Also fixes a bug in the process.");
		_DeclareMemHdlr(RegionEditorCreateDataCopy, "fixes a bug that causes a CTD when a data copy operation was attempted on a brand new region record");
		_DeclareNopHdlr(AIFormResetPackageListColumns, "fixes a bug that causes the AI data dlg's packages listview to reset its columns everytime the thing was refreshed");
		_DeclareMemHdlr(CellViewOnCellSelection, "prevents the object ref list from losing its sort status whenever a cell is selected in the cell list");
		_DeclareMemHdlr(EffectItemListViewSortingA, "prevents the sorting of the effect item list view");
		_DeclareMemHdlr(EffectItemListViewSortingB, "");
		_DeclareMemHdlr(EffectItemListViewSortingC, "");
		_DeclareMemHdlr(TESObjectLIGHClampFalloffExp, "makes sure the falloff exponent text field is clamped to its expected limits");
		_DeclareMemHdlr(TESPackageWndProBeginDrag, "caches drag drop operation state for non-object window/formID listview dialogs");
		_DeclareMemHdlr(TESPackageWndProcEndDrag, "");
		_DeclareMemHdlr(TESObjectCELLWndProcBeginDrag, "");
		_DeclareMemHdlr(TESObjectCELLWndProcEndDrag, "");
		_DeclareMemHdlr(ObjectWindowSplitterWndProcDisable, "disables the splitter when a drag-drop op is in progress, regardless of its enabled state");

		bool __cdecl TESDialogBuildSubwindowDetour(UInt16 TemplateID, Subwindow* DialogSubwindow);
		void __stdcall TESTopicEnumerateDialogDataDetour(HWND Dialog, int SubItemIndex);
		void __cdecl TESDialogEnableTopicControlsDetour(HWND Dialog, bool TopicControlState, bool ResponseControlState);
		void __cdecl TESConditionItemDisableDialogControlsDetour(HWND Dialog);
		void __stdcall TESTopicInfoSetInDialogDetour(void* DialogEditorData, HWND Dialog);

		int CALLBACK ObjectWindowFormListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK TESFormIDListViewFormListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK TESDialogCellListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK TESDialogReferenceListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK TESDialogFormListEDIDComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK FindTextGenericComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK FindTextTopicInfoComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK LandscapeTextureComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
		int CALLBACK TESDialogAIPackageListComparator(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	}
}
