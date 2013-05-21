#include "FindReplaceDialog.h"
#include "ScriptEditor.h"
#include "ScriptEditorManager.h"
#include "ScriptEditorPreferences.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		FindReplaceDialog::FindReplaceDialog( UInt32 ParentWorkspaceIndex )
		{
			this->ParentWorkspaceIndex = ParentWorkspaceIndex;
			Closing = false;

			FindReplaceBox = gcnew AnimatedForm(0.10);
			this->LabelFind = (gcnew Label());
			this->LabelReplace = (gcnew Label());
			this->QueryBox = (gcnew ComboBox());
			this->ReplacementBox = (gcnew ComboBox());
			this->FindButton = (gcnew Button());
			this->ReplaceButton = (gcnew Button());
			this->FindInTabsButton = (gcnew Button());
			this->ReplaceInTabsButton = (gcnew Button());
			this->CountMatchesButton = (gcnew Button());
			this->MatchWholeWord = (gcnew CheckBox());
			this->CaseInsensitiveSearch = (gcnew CheckBox());
			this->InSelection = (gcnew CheckBox());
			UseRegEx = gcnew CheckBox();
			FindReplaceBox->SuspendLayout();

			FindButtonClickHandler = gcnew EventHandler(this, &FindReplaceDialog::FindButton_Click);
			ReplaceButtonClickHandler = gcnew EventHandler(this, &FindReplaceDialog::ReplaceButton_Click);
			FindInTabsButtonClickHandler = gcnew EventHandler(this, &FindReplaceDialog::FindInTabsButton_Click);
			ReplaceInTabsButtonClickHandler = gcnew EventHandler(this, &FindReplaceDialog::ReplaceInTabsButton_Click);
			CountMatchesButtonClickHandler = gcnew EventHandler(this, &FindReplaceDialog::CountMatchesButton_Click);
			FindReplaceBoxCancelHandler = gcnew CancelEventHandler(this, &FindReplaceDialog::FindReplaceBox_Cancel);
			FindReplaceBoxKeyDownHandler = gcnew KeyEventHandler(this, &FindReplaceDialog::FindReplaceBox_KeyDown);

			FindButton->Click += FindButtonClickHandler;
			ReplaceButton->Click += ReplaceButtonClickHandler;
			FindInTabsButton->Click += FindInTabsButtonClickHandler;
			ReplaceInTabsButton->Click += ReplaceInTabsButtonClickHandler;
			CountMatchesButton->Click += CountMatchesButtonClickHandler;
			FindReplaceBox->Closing += FindReplaceBoxCancelHandler;
			FindReplaceBox->KeyDown += FindReplaceBoxKeyDownHandler;

			//
			// LabelFind
			//
			this->LabelFind->Location = System::Drawing::Point(12, 21);
			this->LabelFind->Name = L"LabelFind";
			this->LabelFind->Size = System::Drawing::Size(78, 13);
			this->LabelFind->TabIndex = 0;
			this->LabelFind->Text = L"Find What:";
			this->LabelFind->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			//
			// LabelReplace
			//
			this->LabelReplace->Location = System::Drawing::Point(12, 48);
			this->LabelReplace->Name = L"LabelReplace";
			this->LabelReplace->Size = System::Drawing::Size(78, 13);
			this->LabelReplace->TabIndex = 1;
			this->LabelReplace->Text = L"Replace With:";
			this->LabelReplace->TextAlign = System::Drawing::ContentAlignment::MiddleRight;
			//
			// QueryBox
			//
			this->QueryBox->DropDownHeight = 200;
			this->QueryBox->FormattingEnabled = true;
			this->QueryBox->IntegralHeight = false;
			this->QueryBox->Location = System::Drawing::Point(96, 18);
			this->QueryBox->Name = L"QueryBox";
			this->QueryBox->Size = System::Drawing::Size(307, 21);
			this->QueryBox->TabIndex = 2;
			//
			// ReplacementBox
			//
			this->ReplacementBox->DropDownHeight = 200;
			this->ReplacementBox->FormattingEnabled = true;
			this->ReplacementBox->IntegralHeight = false;
			this->ReplacementBox->Location = System::Drawing::Point(96, 45);
			this->ReplacementBox->Name = L"ReplacementBox";
			this->ReplacementBox->Size = System::Drawing::Size(307, 21);
			this->ReplacementBox->TabIndex = 3;
			//
			// FindButton
			//
			this->FindButton->Location = System::Drawing::Point(409, 19);
			this->FindButton->Name = L"FindButton";
			this->FindButton->Size = System::Drawing::Size(72, 21);
			this->FindButton->TabIndex = 4;
			this->FindButton->Text = L"Find All";
			this->FindButton->UseVisualStyleBackColor = true;
			//
			// ReplaceButton
			//
			this->ReplaceButton->Location = System::Drawing::Point(409, 46);
			this->ReplaceButton->Name = L"ReplaceButton";
			this->ReplaceButton->Size = System::Drawing::Size(72, 21);
			this->ReplaceButton->TabIndex = 5;
			this->ReplaceButton->Text = L"Replace All";
			this->ReplaceButton->UseVisualStyleBackColor = true;

			//
			// FindInTabsButton
			//
			this->FindInTabsButton->Location = System::Drawing::Point(487, 19);
			this->FindInTabsButton->Name = L"FindInTabsButton";
			this->FindInTabsButton->Size = System::Drawing::Size(106, 24);
			this->FindInTabsButton->TabIndex = 6;
			this->FindInTabsButton->Text = L"In All Open Scripts";
			this->FindInTabsButton->UseVisualStyleBackColor = true;
			//
			// ReplaceInTabsButton
			//
			this->ReplaceInTabsButton->Location = System::Drawing::Point(487, 46);
			this->ReplaceInTabsButton->Name = L"ReplaceInTabsButton";
			this->ReplaceInTabsButton->Size = System::Drawing::Size(106, 24);
			this->ReplaceInTabsButton->TabIndex = 7;
			this->ReplaceInTabsButton->Text = L"In All Open Scripts";
			this->ReplaceInTabsButton->UseVisualStyleBackColor = true;

			//
			// CountMatchesButton
			//
			this->CountMatchesButton->Location = System::Drawing::Point(487, 73);
			this->CountMatchesButton->Name = L"CountMatchesButton";
			this->CountMatchesButton->Size = System::Drawing::Size(106, 26);
			this->CountMatchesButton->TabIndex = 8;
			this->CountMatchesButton->Text = L"Count Matches";
			this->CountMatchesButton->UseVisualStyleBackColor = true;

			//
			// MatchWholeWord
			//
			this->MatchWholeWord->AutoSize = true;
			this->MatchWholeWord->Location = System::Drawing::Point(15, 73);
			this->MatchWholeWord->Name = L"MatchWholeWord";
			this->MatchWholeWord->Size = System::Drawing::Size(143, 17);
			this->MatchWholeWord->TabIndex = 9;
			this->MatchWholeWord->Text = L"Match Whole Word Only";
			this->MatchWholeWord->UseVisualStyleBackColor = true;
			//
			// CaseInsensitiveSearch
			//
			this->CaseInsensitiveSearch->AutoSize = true;
			this->CaseInsensitiveSearch->Location = System::Drawing::Point(15, 96);
			this->CaseInsensitiveSearch->Name = L"CaseInsensitiveSearch";
			this->CaseInsensitiveSearch->Size = System::Drawing::Size(140, 17);
			this->CaseInsensitiveSearch->TabIndex = 10;
			this->CaseInsensitiveSearch->Text = L"Case Insensitive Search";
			this->CaseInsensitiveSearch->UseVisualStyleBackColor = true;
			//
			// InSelection
			//
			this->InSelection->AutoSize = true;
			this->InSelection->Location = System::Drawing::Point(321, 73);
			this->InSelection->Name = L"InSelection";
			this->InSelection->Size = System::Drawing::Size(82, 17);
			this->InSelection->TabIndex = 11;
			this->InSelection->Text = L"In Selection";
			this->InSelection->UseVisualStyleBackColor = true;
			//
			// UseRegEx
			//
			this->UseRegEx->AutoSize = true;
			this->UseRegEx->Location = System::Drawing::Point(161, 96);
			this->UseRegEx->Name = L"UseRegEx";
			this->UseRegEx->Size = System::Drawing::Size(144, 17);
			this->UseRegEx->TabIndex = 12;
			this->UseRegEx->Text = L"Use Regular Expressions";
			this->UseRegEx->UseVisualStyleBackColor = true;
			//
			// SEFindReplace
			//
			FindReplaceBox->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			FindReplaceBox->AutoScaleMode = AutoScaleMode::Font;
			FindReplaceBox->ClientSize = System::Drawing::Size(607, 126);
			FindReplaceBox->KeyPreview = true;
			FindReplaceBox->ShowInTaskbar = false;
			FindReplaceBox->Controls->Add(this->UseRegEx);
			FindReplaceBox->Controls->Add(this->InSelection);
			FindReplaceBox->Controls->Add(this->CaseInsensitiveSearch);
			FindReplaceBox->Controls->Add(this->MatchWholeWord);
			FindReplaceBox->Controls->Add(this->CountMatchesButton);
			FindReplaceBox->Controls->Add(this->ReplaceInTabsButton);
			FindReplaceBox->Controls->Add(this->FindInTabsButton);
			FindReplaceBox->Controls->Add(this->ReplaceButton);
			FindReplaceBox->Controls->Add(this->FindButton);
			FindReplaceBox->Controls->Add(this->ReplacementBox);
			FindReplaceBox->Controls->Add(this->QueryBox);
			FindReplaceBox->Controls->Add(this->LabelReplace);
			FindReplaceBox->Controls->Add(this->LabelFind);
			FindReplaceBox->FormBorderStyle = FormBorderStyle::FixedToolWindow;
			FindReplaceBox->Name = L"SEFindReplace";
			FindReplaceBox->StartPosition = FormStartPosition::CenterScreen;
			FindReplaceBox->Text = L"Find/Replace";
			FindReplaceBox->ResumeLayout(false);
			FindReplaceBox->PerformLayout();

			FindReplaceBox->Hide();
		}

		void FindReplaceDialog::Destroy()
		{
			FindButton->Click -= FindButtonClickHandler;
			ReplaceButton->Click -= ReplaceButtonClickHandler;
			FindInTabsButton->Click -= FindInTabsButtonClickHandler;
			ReplaceInTabsButton->Click -= ReplaceInTabsButtonClickHandler;
			CountMatchesButton->Click -= CountMatchesButtonClickHandler;
			FindReplaceBox->Closing -= FindReplaceBoxCancelHandler;
			FindReplaceBox->KeyDown -= FindReplaceBoxKeyDownHandler;

			delete FindReplaceBox;
			delete LabelFind;
			delete LabelReplace;
			delete QueryBox;
			delete ReplacementBox;
			delete FindButton;
			delete ReplaceButton;
			delete FindInTabsButton;
			delete ReplaceInTabsButton;
			delete CountMatchesButton;
			delete MatchWholeWord;
			delete CaseInsensitiveSearch;
			delete InSelection;
		}

		void FindReplaceDialog::FindButton_Click( Object^ Sender, EventArgs^ E )
		{
			String^ Query = QueryBox->Text;
			if (Query == "")
				return;

			CacheComboBoxStrings();
			SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex)->PerformFindReplace(TextEditors::IScriptTextEditor::FindReplaceOperation::e_Find,
																					Query,
																					"",
																					GetSelectedOptions());
		}

		void FindReplaceDialog::ReplaceButton_Click( Object^ Sender, EventArgs^ E )
		{
			String^ Query = QueryBox->Text;
			String^ Replacement = ReplacementBox->Text;
			if (Query == "")
				return;

			CacheComboBoxStrings();
			SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex)->PerformFindReplace(TextEditors::IScriptTextEditor::FindReplaceOperation::e_Replace,
																					Query,
																					Replacement,
																					GetSelectedOptions());
		}

		void FindReplaceDialog::FindInTabsButton_Click( Object^ Sender, EventArgs^ E )
		{
			String^ Query = QueryBox->Text;
			if (Query == "")
				return;

			CacheComboBoxStrings();
			List<Object^>^ RemoteOpParameters = gcnew List<Object^>();
			RemoteOpParameters->Add(TextEditors::IScriptTextEditor::FindReplaceOperation::e_Find);
			RemoteOpParameters->Add(Query);
			RemoteOpParameters->Add("");
			RemoteOpParameters->Add(GetSelectedOptions());

			SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex)->GetParentContainer()->PerformRemoteWorkspaceOperation(
										WorkspaceContainer::RemoteWorkspaceOperation::e_FindReplaceInOpenWorkspaces,
										RemoteOpParameters);
		}

		void FindReplaceDialog::ReplaceInTabsButton_Click( Object^ Sender, EventArgs^ E )
		{
			String^ Query = QueryBox->Text;
			String^ Replacement = ReplacementBox->Text;
			if (Query == "")
				return;

			CacheComboBoxStrings();
			List<Object^>^ RemoteOpParameters = gcnew List<Object^>();
			RemoteOpParameters->Add(TextEditors::IScriptTextEditor::FindReplaceOperation::e_Replace);
			RemoteOpParameters->Add(Query);
			RemoteOpParameters->Add(Replacement);
			RemoteOpParameters->Add(GetSelectedOptions());

			SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex)->GetParentContainer()->PerformRemoteWorkspaceOperation(
										WorkspaceContainer::RemoteWorkspaceOperation::e_FindReplaceInOpenWorkspaces,
										RemoteOpParameters);
		}

		void FindReplaceDialog::CountMatchesButton_Click( Object^ Sender, EventArgs^ E )
		{
			String^ Query = QueryBox->Text;
			if (Query == "")
				return;

			CacheComboBoxStrings();
			int Hits = SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex)->PerformFindReplace(TextEditors::IScriptTextEditor::FindReplaceOperation::e_CountMatches,
				Query,
				"",
				GetSelectedOptions());

			if (Hits != -1)
				MessageBox::Show("Found " + Hits + " matches.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Information);
		}

		void FindReplaceDialog::FindReplaceBox_Cancel( Object^ Sender, CancelEventArgs^ E )
		{
			if (Closing == false)
			{
				E->Cancel = true;
				Hide();
			}
		}

		void FindReplaceDialog::FindReplaceBox_KeyDown( Object^ Sender, KeyEventArgs^ E )
		{
			switch (E->KeyCode)
			{
			case Keys::Enter:
				if (FindReplaceBox->ActiveControl == QueryBox)
					FindButton->PerformClick();
				else if (FindReplaceBox->ActiveControl == ReplacementBox)
					ReplaceButton->PerformClick();

				E->Handled = true;

				break;
			case Keys::Escape:
				Hide();
				E->Handled = true;
				
				break;
			}
		}

		void FindReplaceDialog::CacheComboBoxStrings()
		{
			String^ Value = QueryBox->Text;
			bool MatchFound = false;

			if (Value != "")
			{
				for each (Object^ Itr in QueryBox->Items)
				{
					String^ Item = Itr->ToString();
					if (!String::Compare(Value, Item))
					{
						MatchFound = true;
						break;
					}
				}

				if (MatchFound == false)
					QueryBox->Items->Add(Value);
			}

			Value = ReplacementBox->Text;
			MatchFound = false;

			if (Value != "")
			{
				for each (Object^ Itr in ReplacementBox->Items)
				{
					String^ Item = Itr->ToString();
					if (!String::Compare(Value, Item))
					{
						MatchFound = true;
						break;
					}
				}

				if (MatchFound == false)
					ReplacementBox->Items->Add(Value);
			}
		}

		void FindReplaceDialog::Hide()
		{
			Closing = true;
			SaveOptions();
			FindReplaceBox->Hide();
		}

		void FindReplaceDialog::Show(IntPtr ParentHandle, String^ Query, bool DefaultInSelection, bool PerformSearch)
		{
			LoadOptions();

			if (DefaultInSelection)
				InSelection->Checked = true;
			else
				InSelection->Checked = false;

			if (Query != "")
				QueryBox->Text = Query;

			QueryBox->Focus();

			if (FindReplaceBox->Visible == false)
			{
				Closing = false;
				if (ParentHandle != IntPtr::Zero)
					FindReplaceBox->Show(gcnew WindowHandleWrapper(ParentHandle));
				else
					FindReplaceBox->Show();
			}

			if (PerformSearch)
				FindButton->PerformClick();

			QueryBox->Focus();
		}

		UInt32 FindReplaceDialog::GetSelectedOptions()
		{
			UInt32 Options = 0;
			if (CaseInsensitiveSearch->Checked)
				Options |= (UInt32)TextEditors::IScriptTextEditor::FindReplaceOptions::e_CaseInsensitive;
			if (MatchWholeWord->Checked)
				Options |= (UInt32)TextEditors::IScriptTextEditor::FindReplaceOptions::e_MatchWholeWord;
			if (UseRegEx->Checked)
				Options |= (UInt32)TextEditors::IScriptTextEditor::FindReplaceOptions::e_RegEx;
			if (InSelection->Checked)
				Options |= (UInt32)TextEditors::IScriptTextEditor::FindReplaceOptions::e_InSelection;

			return Options;
		}

		void FindReplaceDialog::LoadOptions()
		{
			CaseInsensitiveSearch->Checked = PREFERENCES->FetchSettingAsInt("CaseInsensitive", "FindReplace");
			MatchWholeWord->Checked = PREFERENCES->FetchSettingAsInt("MatchWholeWord", "FindReplace");
			UseRegEx->Checked = PREFERENCES->FetchSettingAsInt("UseRegEx", "FindReplace");
		}

		void FindReplaceDialog::SaveOptions()
		{
			PREFERENCES->FetchSetting("CaseInsensitive", "FindReplace")->SetValue(((int)CaseInsensitiveSearch->Checked).ToString());
			PREFERENCES->FetchSetting("MatchWholeWord", "FindReplace")->SetValue(((int)MatchWholeWord->Checked).ToString());
			PREFERENCES->FetchSetting("UseRegEx", "FindReplace")->SetValue(((int)UseRegEx->Checked).ToString());

			PREFERENCES->SaveINI();
		}
	}
}