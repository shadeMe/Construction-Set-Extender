#pragma once
#include "Common.h"

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		enum
		{
			kDialogs_RenderWindowState = 0,
			kDialogs_ObjectWindowState,
			kDialogs_CellViewWindowState,
			kDialogs_SortFormListsByActiveForm,
			kDialogs_ColorizeActiveForms,
			kDialogs_ActiveFormForeColor,
			kDialogs_ActiveFormBackColor,
			kDialogs_ShowMainWindowsInTaskbar,
			kDialogs_ShowEditDialogsInTaskbar,

			kDialogs__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kDialogsINISettings[kDialogs__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetDialogs(void);
	}

	namespace Hooks
	{
		// hooks that modify how CS dialogs behave and add interfaces for new tools
		void PatchDialogHooks(void);

		extern HANDLE	g_CSESplashImage;

		_DeclareNopHdlr(ResponseEditorMic, "nops out the call to the delinquent sound struct initializer sub, fixing a crash");
		_DeclareMemHdlr(TopicResultScriptReset, "fixes the bug that clears all flags and the result script of a selected response when adding a new topic");
		_DeclareMemHdlr(NPCFaceGen, "fixes the facegen crash by getting the CS to correctly render the model at dialog init");
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
		_DeclareMemHdlr(FormIDListViewInit, "changes the text of the OK and CANCEL buttons");
		_DeclareMemHdlr(FormIDListViewSaveChanges, "prevents the OK button from closing the list view dialog");
		_DeclareMemHdlr(FormIDListViewItemChange, "displays a confirmation message before committing changes made to the active item when switching to another");
		_DeclareMemHdlr(FormIDListViewSelectItem, "fixes a bug that prevents the correct selection of newly created list view items");
		_DeclareMemHdlr(FormIDListViewDuplicateSelection, "increments the item index returned by TESDialog::LookupListViewItemByData when duplicating forms from the popup menu");
		_DeclareMemHdlr(TESRaceCopyHairEyeDataMessageHandler, "handles the WM_COMMAND messages sent by the newly added controls");
		_DeclareMemHdlr(TESDialogGetIsWindowDragDropRecipient, "allows custom windows to receive form drag-drop notifications");
		_DeclareNopHdlr(MissingTextureWarning, "removes the ostentatious warning");
		_DeclareMemHdlr(AboutDialog, "adds a mention of CSE");
		_DeclareMemHdlr(TESQuestStageResultScript, "fixes a bug that prevented quest stage result scripts from being compiled when their text was empty");
		_DeclareMemHdlr(TESNPCUpdatePreviewControl, "fixes a bug that prevented NPC models from being updated correctly after an item was removed from their inventory");
		_DeclareMemHdlr(TESParametersFillAndInitSelectionComboBoxOwners, "allows factions to be passed as arguments to condition commands that take ownership parameters");
		_DeclareMemHdlr(SearchReplaceDialog, "patches the search and replace dialog to stay open after a replace operation");
		_DeclareMemHdlr(ObjectWindowPopulateFormListInvalidate, "invalidates the formList before items are added to it to increase performance");
		_DeclareMemHdlr(DialogEditorCommandMessageCallback, "handles messages sent by the edit result script button");
		_DeclareMemHdlr(TESQuestCommandMessageCallback, "");
		_DeclareMemHdlr(CellViewWindowResizeFix, "fixes a bug that kept eating the cell view's list view control heights after each reinit");
		_DeclareMemHdlr(TESSoundPlayFile, "patches the sound file sampler code to play files found in archives");
		_DeclareMemHdlr(FormEditDialogTitle, "appends form editorIDs to the title bar of form edit dialogs");
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
		_DeclareMemHdlr(FormListEditorIDComparatorA, "allows form lists to be sorted by name and active record status");

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
	}
}
