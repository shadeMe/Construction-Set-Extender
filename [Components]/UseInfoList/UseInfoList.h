#pragma once

#include "[Common]\HandshakeStructs.h"

public ref class UseInfoList
{
	static UseInfoList^									Singleton = nullptr;

	UseInfoList();

	void												FormList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
	void												FormList_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void												FormList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);
	void												FormList_MouseUp(Object^ Sender, MouseEventArgs^ E);
	void												FormList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);

	void												UseListObject_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);
	void												UseListCell_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);

	void												UseInfoListBox_Cancel(Object^ Sender, CancelEventArgs^ E);
	void												UseInfoListBox_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
	void												UseInfoListBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

	void												SearchBox_TextChanged(Object^ Sender, EventArgs^ E);
	void												SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

	int													LastSortColumn;

	ListView^  											FormList;
		ColumnHeader^										FormListCType;
		ColumnHeader^										FormListCEditorID;
		ColumnHeader^										FormListCFormID;
	GroupBox^											UseListObjectGroup;
		ListView^											UseListObject;
			ColumnHeader^										UseListObjectCType;
			ColumnHeader^										UseListObjectCEditorID;
			ColumnHeader^										UseListObjectCFormID;
	GroupBox^											UseListCellGroup;
		ListView^											UseListCell;
			ColumnHeader^										UseListCellCWorldEditorID;
			ColumnHeader^										UseListCellCCellFormID;
			ColumnHeader^										UseListCellCCellEditorID;
			ColumnHeader^										UseListCellCCellGrid;
			ColumnHeader^										UseListCellCFirstRef;
			ColumnHeader^										UseListCellCUseCount;
	TextBox^											SearchBox;
	Button^												FilterLabel;
	Form^												UseInfoListBox;

	void												ClearLists();
	void												Close();

	void												PopulateFormList();
	void												PopulateUseLists(const char* EditorID);

	static array<String^>^								TypeIdentifier =			// uses TESForm::typeID as its index
															{
																"None",
																"TES4",
																"Group",
																"GMST",
																"Global",
																"Class",
																"Faction",
																"Hair",
																"Eyes",
																"Race",
																"Sound",
																"Skill",
																"Effect",
																"Script",
																"LandTexture",
																"Enchantment",
																"Spell",
																"BirthSign",
																"Activator",
																"Apparatus",
																"Armor",
																"Book",
																"Clothing",
																"Container",
																"Door",
																"Ingredient",
																"Light",
																"MiscItem",
																"Static",
																"Grass",
																"Tree",
																"Flora",
																"Furniture",
																"Weapon",
																"Ammo",
																"NPC",
																"Creature",
																"LeveledCreature",
																"SoulGem",
																"Key",
																"AlchemyItem",
																"SubSpace",
																"SigilStone",
																"LeveledItem",
																"SNDG",
																"Weather",
																"Climate",
																"Region",
																"Cell",
																"Reference",
																"Reference",			// ACHR
																"Reference",			// ACRE
																"PathGrid",
																"World Space",
																"Land",
																"TLOD",
																"Road",
																"Dialog",
																"Dialog Info",
																"Quest",
																"Idle",
																"AI Package",
																"CombatStyle",
																"LoadScreen",
																"LeveledSpell",
																"AnimObject",
																"WaterType",
																"EffectShader",
																"TOFT"
															};
public:
	static UseInfoList^%								GetSingleton(void);

	void												Open(const char* InitForm);

	void												AddFormListItem(String^% EditorID, String^% FormID, UInt32 Type);
	void												AddObjectListItem(String^% EditorID, String^% FormID, UInt32 Type);
	void												AddCellListItem(String^% RefID, String^% WorldEditorID, String^% CellFormID, String^% CellEditorID, String^% CellGrid, UInt32 UseCount);
};

#define USELST											UseInfoList::GetSingleton()