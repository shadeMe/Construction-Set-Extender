#pragma once

#include "[Common]\HandshakeStructs.h"
#include "[Common]\NativeWrapper.h"

using namespace DevComponents;
using namespace GlobalInputMonitor;

namespace cse
{
	ref class DragonDropForm : public Form
	{
	protected:
		virtual	void										WndProc(Message% m) override;
	};

	ref class TagDatabase
	{
		static String^										SerializedDataMidWayMarker = "<Tag Database Marker>";
		Dictionary<AdvTree::Node^, List<String^>^>^			Database;
		AdvTree::AdvTree^									ParentTree;

		void												AddTagNodeToDatabase(AdvTree::Node^ Tag);
		bool												TagItem(String^% TagName, componentDLLInterface::FormData* Data);
	public:
		TagDatabase(AdvTree::AdvTree^ Tree);

		void												AddTag(AdvTree::Node^ ParentTag, String^% Name);
		void												RemoveTag(AdvTree::Node^ Tag);

		bool												TagItem(AdvTree::Node^ Tag, componentDLLInterface::FormData* Data);
		void												UntagItem(AdvTree::Node^ Tag, String^% Name);

		bool												GetItemExistsInTag(AdvTree::Node^ Tag, String^% Name);
		bool												GetTagExists(String^% Name);

		void												Clear(void);
		String^												SerializeDatabase(void);
		bool												DeserializeDatabase(String^ SerializedData);

		bool												GetItemsForTag(AdvTree::Node^ Tag, List<String^>^% Destination) { return Database->TryGetValue(Tag, Destination); }
	};

	ref class TagBrowser
	{
		static TagBrowser^									Singleton = nullptr;

		void												GlobalInputMonitor_MouseUp(Object^ Sender, MouseEventArgs^ E);
		MouseEventHandler^									GlobalMouseHook_MouseUpHandler;

		void												TagBrowserBox_Cancel(Object^ Sender, CancelEventArgs^ E);

		void												FormSelectionList_ItemActivate(Object^ Sender, EventArgs^ E);
		void												FormSelectionList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);

		void												TagTree_NodeClick(Object^ Sender, AdvTree::TreeNodeMouseEventArgs^ E);
		void												TagTree_AfterCellEdit(Object^ Sender, AdvTree::CellEditEventArgs ^ E);

		void												FormList_MouseDown(Object^ Sender, MouseEventArgs^ E);

		void												SaveTags_Click(Object^ Sender, EventArgs^ E);
		void												LoadTags_Click(Object^ Sender, EventArgs^ E);

		void												SearchBox_TextChanged(Object^ Sender, EventArgs^ E);

		void												TagTreeContextMenuAdd_Click(Object^ Sender, EventArgs^ E);
		void												TagTreeContextMenuRemove_Click(Object^ Sender, EventArgs^ E);
		void												FormListContextMenuRemove_Click(Object^ Sender, EventArgs^ E);

		TagBrowser();

		Form^												TagBrowserBox;
		TextBox^											SearchBox;
		ListView^											FormList;
		ColumnHeader^										FormListHEditorID;
		ColumnHeader^										FormListHRefID;
		ColumnHeader^										FormListHType;

		AdvTree::AdvTree^									TagTree;
		AdvTree::NodeConnector^								TagTreeNodeConnector;
		DotNetBar::ElementStyle^							TagTreeElementStyle1;
		DotNetBar::ElementStyle^							TagTreeElementStyle2;

		Button^												SaveTags;
		Button^												LoadTags;
		ContextMenuStrip^									TagTreeContextMenu;
		ToolStripMenuItem^									TagTreeContextMenuAdd;
		ToolStripMenuItem^									TagTreeContextMenuRemove;
		ContextMenuStrip^									FormListContextMenu;
		ToolStripMenuItem^									FormListContextMenuRemove;

		TagDatabase^										Database;
		bool												MouseDragInProgress;

		void												UpdateFormListForTag(AdvTree::Node^ Tag);
		void												AddItemToFormList(componentDLLInterface::FormData* Data);
	public:
		static TagBrowser^%									GetSingleton();

		void												Show(IntPtr Handle);
		void												Hide();
		bool												AddItemToActiveTag(componentDLLInterface::FormData* Data);
		IntPtr												GetFormListHandle();
		IntPtr												GetWindowHandle();
		bool												GetDragInProgress();
	};

#define TAGBRWR											TagBrowser::GetSingleton()
}