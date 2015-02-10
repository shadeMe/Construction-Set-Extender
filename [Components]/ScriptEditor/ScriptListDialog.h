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
			static enum class									Operation
			{
				e_Open = 0,
				e_Delete
			};

			static enum class									FlagIcons
			{
				e_Deleted = 0,
				e_Active
			};
		protected:
			void												ScriptList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
			void												ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E);
			void												ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E);
			void												ScriptList_ItemActivate(Object^ Sender, EventArgs^ E);
			void												ScriptList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E);

			void												ScriptBox_Cancel(Object^ Sender, CancelEventArgs^ E);
			void												SelectBox_Click(Object^ Sender, EventArgs^ E);

			void												SearchBox_TextChanged(Object^ Sender, EventArgs^ E);
			void												SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

			virtual void										ShowUseReportForSelection();
			void												PerformOperationOnSelection();
			void												CleanupDialog(bool SaveBoundsToINI);

			UInt32												ParentWorkspaceIndex;
			Operation											CurrentOperation;
			ComponentDLLInterface::ScriptListData*				ScriptListCache;
			ComponentDLLInterface::ScriptData*					FirstSelectionCache;
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

			virtual void										Destroy();
		public:
			ScriptListDialog();
			virtual ~ScriptListDialog()
			{
				ScriptListDialog::Destroy();
			}

			virtual ComponentDLLInterface::ScriptData*			Show(Operation Op, String^ FilterString);		// caller takes ownership of pointer
		};
	}
}