#pragma once

#include "[Common]\HandshakeStructs.h"
#include "[Common]\NativeWrapper.h"
using namespace DevComponents;

public ref class TagDatabase
{
	static String^										SerializedDataMidWayMarker = "<CSE Tag Database Marker>";
	Dictionary<AdvTree::Node^, List<String^>^>^			Database;
	AdvTree::AdvTree^									ParentTree;

	void												AddTagNodeToDatabase(AdvTree::Node^ Tag);
	bool												TagItem(String^% TagName, FormData* Data);
public:
	TagDatabase(AdvTree::AdvTree^ Tree);

	void												AddTag(AdvTree::Node^ ParentTag, String^% Name);
	void												RemoveTag(AdvTree::Node^ Tag);

	bool												TagItem(AdvTree::Node^ Tag, FormData* Data);
	void												UntagItem(AdvTree::Node^ Tag, String^% Name);
	
	bool												GetItemExistsInTag(AdvTree::Node^ Tag, String^% Name);
	bool												GetTagExists(String^% Name);

	void												Clear(void);	
	String^												SerializeDatabase(void);
	bool												DeserializeDatabase(String^ SerializedData);

	bool												GetItemsForTag(AdvTree::Node^ Tag, List<String^>^% Destination) { return Database->TryGetValue(Tag, Destination); }
};


public ref class TagBrowser
{
	static TagBrowser^									Singleton = nullptr;


	void												TagBrowserBox_Cancel(Object^ Sender, CancelEventArgs^ E);

	void												FormSelectionList_ItemActivate(Object^ Sender, EventArgs^ E);
	void												FormSelectionList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);

	void												TagTree_NodeClick(Object^ Sender, AdvTree::TreeNodeMouseEventArgs^ E);
	void												TagTree_AfterCellEdit(Object^ Sender, AdvTree::CellEditEventArgs ^ E);

	void												PlaceSelection_MouseDown(Object^ Sender, MouseEventArgs^ E);
	void												PlaceSelection_MousUp(Object^ Sender, MouseEventArgs^ E);

	void												AddObjectSelection_Click(Object^ Sender, EventArgs^ E);
	void												PlaceSelection_Click(Object^ Sender, EventArgs^ E);
	void												ShowSelectionControls_Click(Object^ Sender, EventArgs^ E);
	void												SaveTags_Click(Object^ Sender, EventArgs^ E);
	void												LoadTags_Click(Object^ Sender, EventArgs^ E);

	void												SearchBox_TextChanged(Object^ Sender, EventArgs^ E);

	void												TagTreeContextMenuAdd_Click(Object^ Sender, EventArgs^ E);
	void												TagTreeContextMenuRemove_Click(Object^ Sender, EventArgs^ E);
	void												FormListContextMenuAdd_Click(Object^ Sender, EventArgs^ E);
	void												FormListContextMenuRemove_Click(Object^ Sender, EventArgs^ E);
	void												SelectionListContextMenuRemove_Click(Object^ Sender, EventArgs^ E);

	TagBrowser();

	Form^												TagBrowserBox;
		TextBox^											SearchBox;
		ListView^											FormList;
			ColumnHeader^										FormListHEditorID;
			ColumnHeader^										FormListHRefID;
			ColumnHeader^										FormListHType;
	ListView^												SelectionList;
		ColumnHeader^											SelectionListHEditorId;
		ColumnHeader^											SelectionListHRefID;
		ColumnHeader^											SelectionListHType;

	AdvTree::AdvTree^										TagTree;
		AdvTree::NodeConnector^									TagTreeNodeConnector;
		DotNetBar::ElementStyle^								TagTreeElementStyle1;
		DotNetBar::ElementStyle^								TagTreeElementStyle2;

	Button^													AddObjectSelection;
	Button^													PlaceSelection;
	Label^													LabelSelectionList;
	CheckBox^												ClearSelectionOnInstantiation;
	CheckBox^												CustomPos;
		TextBox^												PositionX;
		TextBox^												PositionY;
		TextBox^												PositionZ;
	CheckBox^												CustomRot;
		TextBox^												RotationX;
		TextBox^												RotationY;
		TextBox^												RotationZ;
	Button^													ShowSelectionControls;
	Button^													SaveTags;
	Button^													LoadTags;
	ContextMenuStrip^										TagTreeContextMenu;
		ToolStripMenuItem^										TagTreeContextMenuAdd;
		ToolStripMenuItem^										TagTreeContextMenuRemove;
	ContextMenuStrip^										FormListContextMenu;
		ToolStripMenuItem^										FormListContextMenuAdd;
		ToolStripMenuItem^										FormListContextMenuRemove;
	ContextMenuStrip^										SelectionListContextMenu;
		ToolStripMenuItem^										SelectionListContextMenuRemove;

	TagDatabase^											Database;
	

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


	void												UpdateFormListForTag(AdvTree::Node^ Tag);
	void												AddItemToFormList(FormData* Data);
public:
	static TagBrowser^%									GetSingleton();

	void												Show(IntPtr Handle)	{ TagBrowserBox->Show(gcnew WindowHandleWrapper(Handle)); }
	void												Hide()	{ TagBrowserBox->Hide(); }
	void												AddItemToActiveTag(FormData* Data);

};

#define TAGBRWR											TagBrowser::GetSingleton()