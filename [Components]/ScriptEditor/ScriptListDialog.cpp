#include "ScriptListDialog.h"
#include "Globals.h"
#include "[Common]\NativeWrapper.h"
#include "WorkspaceModelInterface.h"
#include "[Common]\ListViewUtilities.h"

namespace ConstructionSetExtender
{
	namespace ScriptEditor
	{
		ScriptListDialog::ScriptListDialog()
		{
			ScriptBox = gcnew AnimatedForm(0.20);
			PreviewBox = gcnew TextBox();
			ScriptList = gcnew ListView();
			ScriptListCFlags = gcnew ColumnHeader();
			ScriptListCScriptName = gcnew ColumnHeader();
			ScriptListCFormID = gcnew ColumnHeader();
			ScriptListCScriptType= gcnew ColumnHeader();
			SelectBox = gcnew Button();
			SearchBox = gcnew TextBox();
			SelectedEditorIDs = gcnew List < String^ > ;

			ScriptListSelectedIndexChangedHandler = gcnew EventHandler(this, &ScriptListDialog::ScriptList_SelectedIndexChanged);
			ScriptListKeyDownHandler = gcnew KeyEventHandler(this, &ScriptListDialog::ScriptList_KeyDown);
			ScriptListKeyPressHandler = gcnew KeyPressEventHandler(this, &ScriptListDialog::ScriptList_KeyPress);
			ScriptListItemActivateHandler = gcnew EventHandler(this, &ScriptListDialog::ScriptList_ItemActivate);
			ScriptListColumnClickHandler = gcnew ColumnClickEventHandler(this, &ScriptListDialog::ScriptList_ColumnClick);
			ScriptBoxCancelHandler = gcnew CancelEventHandler(this, &ScriptListDialog::ScriptBox_Cancel);
			SelectBoxClickHandler = gcnew EventHandler(this, &ScriptListDialog::SelectBox_Click);
			SearchBoxTextChangedHandler = gcnew EventHandler(this, &ScriptListDialog::SearchBox_TextChanged);
			SearchBoxKeyDownHandler = gcnew KeyEventHandler(this, &ScriptListDialog::SearchBox_KeyDown);

			PreviewBox->Font = gcnew Font("Consolas", 9, FontStyle::Regular);
			PreviewBox->Location = System::Drawing::Point(462, 12);
			PreviewBox->Multiline = true;
			PreviewBox->ReadOnly = true;
			PreviewBox->ScrollBars = ScrollBars::Both;
			PreviewBox->WordWrap = false;
			PreviewBox->Size = System::Drawing::Size(444, 520);
			PreviewBox->Anchor = AnchorStyles::Top|AnchorStyles::Right|AnchorStyles::Bottom|AnchorStyles::Left;

			ScriptList->Columns->AddRange(gcnew cli::array< ColumnHeader^  >(4) {ScriptListCFlags,
				ScriptListCScriptName,
				ScriptListCFormID,
				ScriptListCScriptType});
		//	ScriptList->Font = gcnew Font("Consolas", 9, FontStyle::Regular);
			ScriptList->Location = System::Drawing::Point(12, 12);
			ScriptList->Size = System::Drawing::Size(444, 485);
			ScriptList->UseCompatibleStateImageBehavior = false;
			ScriptList->View = View::Details;
			ScriptList->AutoSize = false;
			ScriptList->MultiSelect = false;
			ScriptList->CheckBoxes = false;
			ScriptList->FullRowSelect = true;
			ScriptList->HideSelection = false;
			ScriptList->SmallImageList = gcnew ImageList();
			ScriptList->SmallImageList->TransparentColor = Color::White;
			ScriptList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ScriptListDialogFlagDeleted"));
			ScriptList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ScriptListDialogFlagUncompiled"));
			ScriptList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("ScriptListDialogFlagActive"));
			ScriptList->Anchor = AnchorStyles::Top|AnchorStyles::Left|AnchorStyles::Bottom;

			ScriptListCFlags->Text = "";
			ScriptListCFlags->Width = 20;

			ScriptListCScriptName->Text = "Name";
			ScriptListCScriptName->Width = 196;

			ScriptListCFormID->Text = "FormID";
			ScriptListCFormID->Width = 73;

			ScriptListCScriptType->Text = "Type";
			ScriptListCScriptType->Width = 87;

			SearchBox->Font = gcnew Font("Consolas", 14.25F, FontStyle::Regular);
			SearchBox->Location = System::Drawing::Point(12, 503);
			SearchBox->MaxLength = 512;
			SearchBox->Size = System::Drawing::Size(312, 30);
			SearchBox->Anchor = AnchorStyles::Left|AnchorStyles::Bottom;

		//	SelectBox->Font = gcnew Font("Segoe UI", 10);
			SelectBox->Location = System::Drawing::Point(330, 503);
			SelectBox->Text = "Select Script(s)";
			SelectBox->Size = System::Drawing::Size(126, 29);
			SelectBox->Anchor = AnchorStyles::Left|AnchorStyles::Bottom;

			ScriptBox->ClientSize = System::Drawing::Size(916, 541);
			ScriptBox->MinimumSize = System::Drawing::Size(935, 320);
		//	ScriptBox->MaximumSize = System::Drawing::Size(935, 2000);
			ScriptBox->Controls->Add(ScriptList);
			ScriptBox->Controls->Add(PreviewBox);
			ScriptBox->Controls->Add(SelectBox);
			ScriptBox->Controls->Add(SearchBox);
			ScriptBox->FormBorderStyle = FormBorderStyle::SizableToolWindow;
			ScriptBox->StartPosition = FormStartPosition::Manual;
			ScriptBox->MaximizeBox = false;
			ScriptBox->MinimizeBox = false;
			ScriptBox->Text = "Select Script";
			ScriptBox->AcceptButton = nullptr;

			ScriptBox->Hide();
			ScriptList->Tag = (int)0;
			ScriptList->Sorting = SortOrder::Descending;
			ScriptList->Sort();
			ScriptList->ListViewItemSorter = gcnew ListViewImgSorter((int)ScriptList->Tag, ScriptList->Sorting);

			ScriptList->SelectedIndexChanged += ScriptListSelectedIndexChangedHandler;
			ScriptList->KeyDown += ScriptListKeyDownHandler;
			ScriptList->ItemActivate += ScriptListItemActivateHandler;
			ScriptList->ColumnClick += ScriptListColumnClickHandler;
			ScriptList->KeyPress += ScriptListKeyPressHandler;
			SearchBox->TextChanged += SearchBoxTextChangedHandler;
			SearchBox->KeyDown += SearchBoxKeyDownHandler;
			SelectBox->Click += SelectBoxClickHandler;
			ScriptBox->Closing += ScriptBoxCancelHandler;
		}

		ScriptListDialog::~ScriptListDialog()
		{
			ScriptList->SelectedIndexChanged -= ScriptListSelectedIndexChangedHandler;
			ScriptList->KeyDown -= ScriptListKeyDownHandler;
			ScriptList->ItemActivate -= ScriptListItemActivateHandler;
			ScriptList->ColumnClick -= ScriptListColumnClickHandler;
			ScriptList->KeyPress -= ScriptListKeyPressHandler;
			SearchBox->TextChanged -= SearchBoxTextChangedHandler;
			SearchBox->KeyDown -= SearchBoxKeyDownHandler;
			SelectBox->Click -= SelectBoxClickHandler;
			ScriptBox->Closing -= ScriptBoxCancelHandler;

			CleanupDialog(false);

			for each (Image^ Itr in ScriptList->SmallImageList->Images)
				delete Itr;

			ScriptList->SmallImageList->Images->Clear();
			ScriptList->SmallImageList = nullptr;

			delete ScriptBox;
			delete PreviewBox;
			delete ScriptList;
			delete ScriptListCFlags;
			delete ScriptListCScriptName;
			delete ScriptListCFormID;
			delete ScriptListCScriptType;
			delete SearchBox;
			delete SelectBox;

			SelectedEditorIDs->Clear();
		}

		bool ScriptListDialog::Show(ShowOperation Operation, String^ FilterString, List<String^>^% OutSelectedScriptEditorIDs)
		{
			CurrentOperation = Operation;

			if (Operation == ShowOperation::Open)
				ScriptList->MultiSelect = true;
			else
				ScriptList->MultiSelect = false;

			ScriptList->BeginUpdate();

			ComponentDLLInterface::ScriptListData* Data = ScriptListCache = NativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetScriptList();
			if (Data)
			{
				for (int i = 0; i < Data->ScriptCount; i++)
				{
					ComponentDLLInterface::ScriptData* ThisScript = &Data->ScriptListHead[i];
					if (ThisScript->IsValid() == false)
						continue;

					ListViewItem^ NewScript = gcnew ListViewItem("");
					NewScript->SubItems->Add(gcnew String(ThisScript->EditorID));
					NewScript->SubItems->Add(ThisScript->FormID.ToString("X8"));
					if (ThisScript->UDF)
						NewScript->SubItems->Add("Function");
					else
					{
						switch ((IWorkspaceModel::ScriptType)ThisScript->Type)
						{
						case IWorkspaceModel::ScriptType::Object:
							NewScript->SubItems->Add("Object");
							break;
						case IWorkspaceModel::ScriptType::Quest:
							NewScript->SubItems->Add("Quest");
							break;
						case IWorkspaceModel::ScriptType::MagicEffect:
							NewScript->SubItems->Add("Magic Effect");
							break;
						}
					}

					if (ThisScript->Flags & 0x20)
						NewScript->ImageIndex = (int)FlagIcons::Deleted;
					else if (ThisScript->Compiled == false)
						NewScript->ImageIndex = (int)FlagIcons::Uncompiled;
					else if (ThisScript->Flags & 0x2)
						NewScript->ImageIndex = (int)FlagIcons::Active;

					NewScript->Tag = (UInt32)ThisScript;
					ScriptList->Items->Add(NewScript);
				}
			}
			ScriptList->EndUpdate();

			if (ScriptList->Items->Count > 0)
			{
				SearchBox->Text = FilterString;
				SearchBox->SelectAll();
			}
			else
			{
				ScriptList->Enabled = false;
				SearchBox->Enabled = false;
			}

			SearchBox->Focus();

			char Buffer[0x200] = { 0 };
			int X, Y, W, H;
			NativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("W", "ScriptEditor::ScriptListDialog", "916", Buffer, sizeof(Buffer));
			W = Int32::Parse(gcnew String(Buffer));
			NativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("H", "ScriptEditor::ScriptListDialog", "541", Buffer, sizeof(Buffer));
			H = Int32::Parse(gcnew String(Buffer));
			NativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("X", "ScriptEditor::ScriptListDialog", "0", Buffer, sizeof(Buffer));
			X = Int32::Parse(gcnew String(Buffer));
			NativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("Y", "ScriptEditor::ScriptListDialog", "0", Buffer, sizeof(Buffer));
			Y = Int32::Parse(gcnew String(Buffer));

			ScriptBox->ClientSize = Size(W, H);
			ScriptBox->DesktopLocation = Point(X, Y);

			Closing = false;
			SelectionComplete = false;
			ScriptBox->ShowDialog();

			if (SelectionComplete)
				OutSelectedScriptEditorIDs->AddRange(SelectedEditorIDs);

			return SelectionComplete;
		}

		void ScriptListDialog::CleanupDialog(bool SaveBoundsToINI)
		{
			ScriptList->Items->Clear();
			ScriptList->Enabled = true;
			SearchBox->Enabled = true;
			PreviewBox->Text = "";
			SearchBox->Text = "";
			ScriptList->MultiSelect = false;
			NativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(ScriptListCache, false);
			ScriptListCache = 0;
			SelectedEditorIDs->Clear();

			if (SaveBoundsToINI)
			{
				NativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("W", "ScriptEditor::ScriptListDialog", (CString(ScriptBox->ClientSize.Width.ToString())).c_str());
				NativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("H", "ScriptEditor::ScriptListDialog", (CString(ScriptBox->ClientSize.Height.ToString())).c_str());
				NativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("X", "ScriptEditor::ScriptListDialog", (CString(ScriptBox->DesktopLocation.X.ToString())).c_str());
				NativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("Y", "ScriptEditor::ScriptListDialog", (CString(ScriptBox->DesktopLocation.Y.ToString())).c_str());
			}
		}

		void ScriptListDialog::CompleteSelection()
		{
			if (GetListViewSelectedItem(ScriptList) == nullptr || SelectionComplete)
				return;

			for (UInt32 i = 0; i < ScriptList->SelectedItems->Count; i++)
			{
				ListViewItem^ Itr = ScriptList->SelectedItems[i];
				ComponentDLLInterface::ScriptData* Data = (ComponentDLLInterface::ScriptData*)((UInt32)Itr->Tag);

				SelectedEditorIDs->Add(gcnew String(Data->EditorID));
			}

			Closing = true;
			SelectionComplete = true;
			ScriptBox->Close();
		}

		void ScriptListDialog::ShowUseReport()
		{
			ListViewItem^ First = GetListViewSelectedItem(ScriptList);
			if (First == nullptr)
				return;

			ComponentDLLInterface::ScriptData* Data = (ComponentDLLInterface::ScriptData*)((UInt32)First->Tag);
			NativeWrapper::g_CSEInterfaceTable->EditorAPI.ShowUseReportDialog(Data->EditorID);
		}

		void ScriptListDialog::ScriptBox_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			CleanupDialog(true);

			if (Closing == false)
			{
				E->Cancel = true;
				Closing = true;
				ScriptBox->Close();
			}
		}

		void ScriptListDialog::SelectBox_Click(Object^ Sender, EventArgs^ E)
		{
			CompleteSelection();
		}

		void ScriptListDialog::ScriptList_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
		{
			if (GetListViewSelectedItem(ScriptList) == nullptr)
				return;

			ComponentDLLInterface::ScriptData* Data = (ComponentDLLInterface::ScriptData*)((UInt32)GetListViewSelectedItem(ScriptList)->Tag);
			String^ ScriptText = gcnew String(Data->Text);

			PreviewBox->Text = ScriptText->Replace("\n", "\r\n");
		}

		void ScriptListDialog::ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::Enter:
				CompleteSelection();
				break;
			case Keys::F1:
				ShowUseReport();
				break;
			case Keys::Escape:
				Closing = true;
				ScriptBox->Close();
				break;
			case Keys::Back:
				if (SearchBox->Text->Length >= 1)
				{
					SearchBox->Text = SearchBox->Text->Remove(SearchBox->Text->Length - 1);
					ScriptList->Focus();
				}

				E->Handled = true;
				break;
			}
		}

		void ScriptListDialog::ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
		{
			if ((E->KeyChar > 0x29 && E->KeyChar < 0x3A) ||
				(E->KeyChar > 0x60 && E->KeyChar < 0x7B))
			{
				SearchBox->Text += E->KeyChar.ToString();
				ScriptList->Focus();
				E->Handled = true;
			}
		}

		void ScriptListDialog::ScriptList_ItemActivate(Object^ Sender, EventArgs^ E)
		{
			CompleteSelection();
		}

		void ScriptListDialog::ScriptList_ColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
		{
			if (E->Column != (int)ScriptList->Tag)
			{
				ScriptList->Tag = (int)E->Column;
				ScriptList->Sorting = SortOrder::Descending;
			}
			else
			{
				if (ScriptList->Sorting == SortOrder::Ascending)
					ScriptList->Sorting = SortOrder::Descending;
				else
					ScriptList->Sorting = SortOrder::Ascending;
			}

			ScriptList->Sort();
			System::Collections::IComparer^ Sorter;
			switch (E->Column)
			{
			case 0:							// Flags
				Sorter = gcnew ListViewImgSorter(E->Column, ScriptList->Sorting);
				break;
			case 2:							// FormID
				Sorter = gcnew ListViewIntSorter(E->Column, ScriptList->Sorting, true);
				break;
			default:
				Sorter = gcnew ListViewStringSorter(E->Column, ScriptList->Sorting);
				break;
			}
			ScriptList->ListViewItemSorter = Sorter;
		}

		void ScriptListDialog::SearchBox_TextChanged(Object^ Sender, EventArgs^ E)
		{
			if (SearchBox->Text != "")
			{
				for each (ListViewItem^ Itr in ScriptList->SelectedItems)
					Itr->Selected = false;

				ListViewItem^ Result = FindItemWithText(ScriptList, SearchBox->Text, true, true);
				if (Result != nullptr)
				{
					Result->Selected = true;
					ScriptList->TopItem = Result;
				}
			}
		}

		void ScriptListDialog::SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::Back:
				return;
			}

			ScriptListDialog::ScriptList_KeyDown(nullptr, E);
		}
	}
}