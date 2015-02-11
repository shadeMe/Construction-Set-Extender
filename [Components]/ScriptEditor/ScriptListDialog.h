#pragma once
#include "[Common]\HandShakeStructs.h"
#include "[Common]\AuxiliaryWindowsForm.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ref class ScriptListDialog
		{
		public:
			static enum class ShowOperation
			{
				Open,
				Delete
			};
		protected:
			static enum class FlagIcons
			{
				Deleted = 0,
				Uncompiled,
				Active
			};

			void												ScriptList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
			void												ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void												ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
			void												ScriptList_ItemActivate(Object^ Sender, EventArgs^ E);
			void												ScriptList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);

			void												ScriptBox_Cancel(Object^ Sender, CancelEventArgs^ E);
			void												SelectBox_Click(Object^ Sender, EventArgs^ E);

			void												SearchBox_TextChanged(Object^ Sender, EventArgs^ E);
			void												SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

			void												ShowUseReport();
			void												CompleteSelection();
			void												CleanupDialog(bool SaveBoundsToINI);

			ShowOperation										CurrentOperation;
			ComponentDLLInterface::ScriptListData*				ScriptListCache;
			List<String^>^										SelectedEditorIDs;
			bool												Closing;
			bool												SelectionComplete;		// stoopid workaround for the form's obsession with an accept button

			AnimatedForm^										ScriptBox;
			TextBox^											PreviewBox;
			ListView^											ScriptList;
			ColumnHeader^										ScriptListCFlags;
			ColumnHeader^										ScriptListCScriptName;
			ColumnHeader^										ScriptListCFormID;
			ColumnHeader^										ScriptListCScriptType;
			TextBox^											SearchBox;
			Button^												SelectBox;

			EventHandler^										ScriptListSelectedIndexChangedHandler;
			KeyEventHandler^									ScriptListKeyDownHandler;
			KeyPressEventHandler^								ScriptListKeyPressHandler;
			EventHandler^										ScriptListItemActivateHandler;
			ColumnClickEventHandler^							ScriptListColumnClickHandler;
			CancelEventHandler^									ScriptBoxCancelHandler;
			EventHandler^										SelectBoxClickHandler;
			EventHandler^										SearchBoxTextChangedHandler;
			KeyEventHandler^									SearchBoxKeyDownHandler;
		public:
			ScriptListDialog();
			~ScriptListDialog();

																// returns true if at least one script was selected
			bool												Show(ShowOperation Operation, String^ FilterString, List<String^>^% OutSelectedScriptEditorIDs);
		};
	}
}