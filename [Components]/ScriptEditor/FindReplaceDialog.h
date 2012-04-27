#pragma once
#include "[Common]\AuxiliaryWindowsForm.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ref class FindReplaceDialog
		{
		protected:
			AnimatedForm^								FindReplaceBox;
			Label^										LabelFind;
			Label^										LabelReplace;
			ComboBox^									QueryBox;
			ComboBox^									ReplacementBox;
			Button^										FindButton;
			Button^										ReplaceButton;
			Button^										FindInTabsButton;
			Button^										ReplaceInTabsButton;
			Button^										CountMatchesButton;
			CheckBox^									MatchWholeWord;
			CheckBox^									CaseInsensitiveSearch;
			CheckBox^									InSelection;
			CheckBox^									UseRegEx;

			UInt32										ParentWorkspaceIndex;
			bool										Closing;

			EventHandler^								FindButtonClickHandler;
			EventHandler^								ReplaceButtonClickHandler;
			EventHandler^								FindInTabsButtonClickHandler;
			EventHandler^								ReplaceInTabsButtonClickHandler;
			EventHandler^								CountMatchesButtonClickHandler;
			CancelEventHandler^							FindReplaceBoxCancelHandler;
			KeyEventHandler^							FindReplaceBoxKeyDownHandler;

			void										FindButton_Click(Object^ Sender, EventArgs^ E);
			void										ReplaceButton_Click(Object^ Sender, EventArgs^ E);
			void										FindInTabsButton_Click(Object^ Sender, EventArgs^ E);
			void										ReplaceInTabsButton_Click(Object^ Sender, EventArgs^ E);
			void										CountMatchesButton_Click(Object^ Sender, EventArgs^ E);

			void										FindReplaceBox_Cancel(Object^ Sender, CancelEventArgs^ E);
			void										FindReplaceBox_KeyDown(Object^ Sender, KeyEventArgs^ E);

			void										CacheComboBoxStrings();
			UInt32										GetSelectedOptions();

			void										LoadOptions();
			void										SaveOptions();

			virtual void								Destroy();
		public:
			FindReplaceDialog(UInt32 ParentWorkspaceIndex);
			virtual ~FindReplaceDialog()
			{
				FindReplaceDialog::Destroy();
			}

			void										Show(IntPtr ParentHandle, String^ Query, bool DefaultInSelection);
			void										Hide();
		};
	}
}