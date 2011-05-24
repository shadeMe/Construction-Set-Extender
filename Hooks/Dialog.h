#pragma once 
#include "Common.h"

namespace Hooks
{
	// hooks that modify how CS dialogs behave and add interfaces for new functionality
	void PatchDialogHooks(void);

	extern bool g_QuickLoadToggle;

	_DeclareNopHdlr(ResponseEditorMic, "nops out the call to the delinquent sound struct initializer sub, fixing a crash");
	_DeclareMemHdlr(FindTextInit, "hooks the find text window for subclassing");
	_DeclareMemHdlr(UseInfoListInit, "replaces the redundant Recreate facial animation files menu item with Use Info Listings");
	_DeclareMemHdlr(TopicResultScriptReset, "fixes the bug that clears all flags and the result script of a selected response when adding a new topic");
	_DeclareMemHdlr(NPCFaceGen, "fixes the facegen crash by getting the CS to correctly render the model at dialog init");
	_DeclareMemHdlr(DataDlgInit, "hooks the data window for subclassing");
	_DeclareMemHdlr(CustomCSWindow, "keeps custom child windows of the CS main window from being closed on plugin load");
	_DeclareMemHdlr(RaceDescriptionDirtyEdit, "prevent dirty edits occuring when you edit a race's text description and click directly to another race without switching tabs first, if the spellchecker pops up (which it will), the description for the race you were previously working on gets copied into the one you just selected.");
	_DeclareMemHdlr(AddListViewItem, "patches various routines to check for the 'Hide UnModified Forms' flag before populating controls with forms");
	_DeclareMemHdlr(AddComboBoxItem, "");
	_DeclareMemHdlr(ObjectListPopulateListViewItems, "");
	_DeclareMemHdlr(CellViewPopulateObjectList, "");
	_DeclareMemHdlr(CellObjectListShadeMeRefAppend, "");
	_DeclareMemHdlr(DeathToTheCloseOpenDialogsMessage, "gets rid of it");
	_DeclareMemHdlr(TESDialogPopupMenu, "hooks popup menu instantiation for new menu items");
	_DeclareMemHdlr(ResponseWindowLipButtonPatch, "adds suport for the 'fixed' lipsync tool");
	_DeclareMemHdlr(ResponseWindowInit, "hooks the response editor dialog for subclassing");
	_DeclareMemHdlr(DataDlgZOrder, "prevents the data dlg from staying ontop of all other windows");
	_DeclareMemHdlr(FormIDListViewInit, "changes the text of the OK and CANCEL buttons");
	_DeclareMemHdlr(FormIDListViewSaveChanges, "prevents the OK button from closing the list view dialog");
	_DeclareMemHdlr(FormIDListViewItemChange, "displays a confirmation message before comitting changes made to the active item when switching to another");
	_DeclareMemHdlr(FormIDListViewSelectItem, "fixes a bug that prevents the correct selection of newly created list view items");
	_DeclareMemHdlr(FormIDListViewDuplicateSelection, "increments the item index returned by TESDialog::LookupListViewItemByData when duplicating forms from the popup menu");
	_DeclareMemHdlr(TESRaceCopyHairEyeDataInit, "adds buttons to the face data tab page of the TESRace formIDListView dialog");
	_DeclareMemHdlr(TESRaceCopyHairEyeDataMessageHandler, "handles the WM_COMMAND messages sent by the newly added controls");
	_DeclareNopHdlr(TESDialogSubwindowEnumChildCallback, "patches the TESDialogSubWindow::EnumChildWindowsCallback function to keep it from overwriting the subwindow object's container member. (the patch)causes issues with control placement and z-ordering");
	_DeclareMemHdlr(TESDialogGetIsWindowDragDropRecipient, "allows custom windows to receive form drag-drop notifications");
	_DeclareNopHdlr(MissingTextureWarning, "removes the ostentatious warning");
	_DeclareMemHdlr(AboutDialog, "add a mention of CSE");
}
