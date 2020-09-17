#include "SelectScriptDialog.h"
#include "Globals.h"
#include "[Common]\NativeWrapper.h"
#include "WorkspaceModelInterface.h"
#include "[Common]\ListViewUtilities.h"
#include "ScriptSync.h"

namespace cse
{
	namespace scriptEditor
	{
		NativeScriptDataWrapper::NativeScriptDataWrapper(componentDLLInterface::ScriptData* Data)
		{
			ScriptData = Data;
			EditorID = gcnew String(Data->EditorID);
			ParentPluginName = gcnew String(Data->ParentPluginName);

			if (Data->UDF)
				Type = "User-Defined Function";
			else switch (Data->Type)
			{
			case componentDLLInterface::ScriptData::kScriptType_Object:
				Type = "Object";
				break;
			case componentDLLInterface::ScriptData::kScriptType_Quest:
				Type = "Quest";
				break;
			case componentDLLInterface::ScriptData::kScriptType_Magic:
				Type = "Magic Effect";
				break;
			}
		}

		void SelectScriptDialog::ScriptList_SelectionChanged(Object^ Sender, EventArgs^ E)
		{
			DeferredSelectionUpdateTimer->Start();
		}

		void SelectScriptDialog::ScriptList_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::F1:
				ShowUseReport();
				break;
			case Keys::Escape:
				this->Close();
				E->Handled = true;
				E->SuppressKeyPress = true;
				break;
			case Keys::Back:
				if (ToolStripFilterTextBox->Text->Length >= 1)
				{
					ToolStripFilterTextBox->Text = ToolStripFilterTextBox->Text->Remove(ToolStripFilterTextBox->Text->Length - 1);
					ScriptList->Focus();
				}

				E->Handled = true;
				break;
			}
		}

		void SelectScriptDialog::ScriptList_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
		{
			if ((E->KeyChar > 0x29 && E->KeyChar < 0x3A) ||
				(E->KeyChar > 0x60 && E->KeyChar < 0x7B))
			{
				ToolStripFilterTextBox->Text += E->KeyChar.ToString();
				ScriptList->Focus();
				E->Handled = true;
			}
		}

		void SelectScriptDialog::ScriptList_ItemActivate(Object^ Sender, EventArgs^ E)
		{
			CompleteSelection();
		}

		void SelectScriptDialog::ButtonCompleteSelection_Click(Object^ Sender, EventArgs^ E)
		{
			CompleteSelection();
		}

		void SelectScriptDialog::ActiveScriptsToolStripMenuItem_Click(Object^ Sender, EventArgs^ E)
		{
			ScriptList->DeselectAll();

			List<NativeScriptDataWrapper^>^ NewSelection = gcnew List<NativeScriptDataWrapper ^>;
			for each (NativeScriptDataWrapper ^ Itr in ListDataSource)
			{
				if (Itr->ScriptData->IsActive())
					NewSelection->Add(Itr);
			}

			ScriptList->SelectObjects(NewSelection);
		}

		void SelectScriptDialog::UncompiledScriptsToolStripMenuItem_Click(Object^ Sender, EventArgs^ E)
		{
			ScriptList->DeselectAll();

			List<NativeScriptDataWrapper^>^ NewSelection = gcnew List<NativeScriptDataWrapper^>;
			for each (NativeScriptDataWrapper ^ Itr in ListDataSource)
			{
				if (Itr->ScriptData->Compiled == false)
					NewSelection->Add(Itr);
			}

			ScriptList->SelectObjects(NewSelection);
		}

		void SelectScriptDialog::ToolStripFilterTextBox_TextChanged(Object^ Sender, EventArgs^ E)
		{
			PopulateLoadedScripts(ToolStripFilterTextBox->Text, false);
		}

		void SelectScriptDialog::ToolStripFilterTextBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::Enter:
				CompleteSelection();
				return;
			case Keys::Back:
				return;
			}

			ScriptList_KeyDown(ScriptList, E);
		}

		void SelectScriptDialog::DeferredSelectionUpdateTimer_Tick(Object^ Sender, EventArgs^ E)
		{
			ToolStripLabelSelectionCount->Text = ScriptList->SelectedObjects->Count + " selected";

			if (ScriptList->SelectedObjects->Count == 1)
			{
				NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)ScriptList->SelectedObject;
				String^ ScriptText = gcnew String(Data->ScriptData->Text);

				PreviewBox->Text = ScriptText->Replace("\n", "\r\n");
			}
			else
				PreviewBox->Text = "";

			if (ScriptList->SelectedObjects->Count > 0)
				ButtonCompleteSelection->Enabled = true;
			else
				ButtonCompleteSelection->Enabled = false;

			DeferredSelectionUpdateTimer->Stop();
		}

		void SelectScriptDialog::Dialog_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			SaveBoundsToINI();
		}

		void SelectScriptDialog::InitializeComponent()
		{
			this->PreviewBox = (gcnew System::Windows::Forms::TextBox());
			this->ScriptList = (gcnew BrightIdeasSoftware::FastObjectListView());
			this->ScriptListCFlags = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCScriptName = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCFormID = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCType = (gcnew BrightIdeasSoftware::OLVColumn());
			this->ScriptListCParentPlugin = (gcnew BrightIdeasSoftware::OLVColumn());
			this->BottomToolStrip = (gcnew System::Windows::Forms::ToolStrip());
			this->ToolStripLabelFilter = (gcnew System::Windows::Forms::ToolStripLabel());
			this->ToolStripFilterTextBox = (gcnew System::Windows::Forms::ToolStripTextBox());
			this->ToolStripLabelSelect = (gcnew System::Windows::Forms::ToolStripDropDownButton());
			this->ActiveScriptsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->UncompiledScriptsToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->ToolStripLabelSelectionCount = (gcnew System::Windows::Forms::ToolStripLabel());
			this->ButtonCompleteSelection = (gcnew System::Windows::Forms::Button());
			this->DeferredSelectionUpdateTimer = (gcnew System::Windows::Forms::Timer());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->BeginInit();
			this->BottomToolStrip->SuspendLayout();
			this->SuspendLayout();
			//
			// PreviewBox
			//
			this->PreviewBox->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->PreviewBox->Font = (gcnew System::Drawing::Font(L"Consolas", 9, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
				static_cast<System::Byte>(0)));
			this->PreviewBox->Location = System::Drawing::Point(484, 12);
			this->PreviewBox->Multiline = true;
			this->PreviewBox->Name = L"PreviewBox";
			this->PreviewBox->ReadOnly = true;
			this->PreviewBox->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->PreviewBox->Size = System::Drawing::Size(406, 552);
			this->PreviewBox->WordWrap = false;
			//
			// ScriptList
			//
			this->ScriptList->AllColumns->Add(this->ScriptListCFlags);
			this->ScriptList->AllColumns->Add(this->ScriptListCScriptName);
			this->ScriptList->AllColumns->Add(this->ScriptListCFormID);
			this->ScriptList->AllColumns->Add(this->ScriptListCType);
			this->ScriptList->AllColumns->Add(this->ScriptListCParentPlugin);
			this->ScriptList->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left));
			this->ScriptList->CellEditUseWholeCell = false;
			this->ScriptList->Columns->AddRange(gcnew cli::array< System::Windows::Forms::ColumnHeader^  >(5) {
				this->ScriptListCFlags,
					this->ScriptListCScriptName, this->ScriptListCFormID, this->ScriptListCType, this->ScriptListCParentPlugin
			});
			this->ScriptList->Cursor = System::Windows::Forms::Cursors::Default;
			this->ScriptList->FullRowSelect = true;
			this->ScriptList->Location = System::Drawing::Point(12, 13);
			this->ScriptList->Name = L"ScriptList";
			this->ScriptList->ShowGroups = false;
			this->ScriptList->Size = System::Drawing::Size(466, 505);
			this->ScriptList->TabIndex = 0;
			this->ScriptList->UseCompatibleStateImageBehavior = false;
			this->ScriptList->View = System::Windows::Forms::View::Details;
			this->ScriptList->VirtualMode = true;
			this->ScriptList->SmallImageList = gcnew ImageList();
			this->ScriptList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->
														CreateImage("ScriptListDialogFlagSyncing"));
			this->ScriptList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->
														CreateImage("ScriptListDialogFlagUncompiled"));
			this->ScriptList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->
														CreateImage("ScriptListDialogFlagActive"));
			this->ScriptList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->
														CreateImage("ScriptListDialogFlagDeleted"));
			//
			// ScriptListCFlags
			//
			this->ScriptListCFlags->MaximumWidth = 20;
			this->ScriptListCFlags->MinimumWidth = 20;
			this->ScriptListCFlags->Text = L"";
			this->ScriptListCFlags->Width = 20;
			this->ScriptListCFlags->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&SelectScriptDialog::ScriptListAspectFlagsGetter);
			this->ScriptListCFlags->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&SelectScriptDialog::ScriptListImageFlagsGetter);
			this->ScriptListCFlags->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&SelectScriptDialog::ScriptListAspectToStringConverterFlags);

			//
			// ScriptListCScriptName
			//
			this->ScriptListCScriptName->Text = L"Name";
			this->ScriptListCScriptName->Width = 180;
			this->ScriptListCScriptName->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&SelectScriptDialog::ScriptListAspectScriptNameGetter);
			//
			// ScriptListCFormID
			//
			this->ScriptListCFormID->Text = L"Form ID";
			this->ScriptListCFormID->Width = 75;
			this->ScriptListCFormID->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&SelectScriptDialog::ScriptListAspectFormIDGetter);
			this->ScriptListCFormID->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&SelectScriptDialog::ScriptListAspectToStringConverterFormID);
			//
			// ScriptListCType
			//
			this->ScriptListCType->Text = L"Type";
			this->ScriptListCType->Width = 54;
			this->ScriptListCType->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&SelectScriptDialog::ScriptListAspectTypeGetter);
			//
			// ScriptListCParentPlugin
			//
			this->ScriptListCParentPlugin->Text = L"Parent Plugin";
			this->ScriptListCParentPlugin->Width = 114;
			this->ScriptListCParentPlugin->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&SelectScriptDialog::ScriptListAspectParentPluginGetter);
			//
			// BottomToolStrip
			//
			this->BottomToolStrip->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->BottomToolStrip->AutoSize = false;
			this->BottomToolStrip->Dock = System::Windows::Forms::DockStyle::None;
			this->BottomToolStrip->GripStyle = System::Windows::Forms::ToolStripGripStyle::Hidden;
			this->BottomToolStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(4) {
				this->ToolStripLabelFilter,
					this->ToolStripFilterTextBox, this->ToolStripLabelSelect, this->ToolStripLabelSelectionCount
			});
			this->BottomToolStrip->Location = System::Drawing::Point(12, 532);
			this->BottomToolStrip->Name = L"BottomToolStrip";
			this->BottomToolStrip->RenderMode = System::Windows::Forms::ToolStripRenderMode::System;
			this->BottomToolStrip->Size = System::Drawing::Size(379, 32);
			this->BottomToolStrip->TabIndex = 0;
			//
			// ToolStripLabelFilter
			//
			this->ToolStripLabelFilter->Name = L"ToolStripLabelFilter";
			this->ToolStripLabelFilter->Size = System::Drawing::Size(33, 29);
			this->ToolStripLabelFilter->Text = L"Filter";
			//
			// ToolStripFilterTextBox
			//
			this->ToolStripFilterTextBox->AutoSize = false;
			this->ToolStripFilterTextBox->BorderStyle = System::Windows::Forms::BorderStyle::Fixed3D;
			this->ToolStripFilterTextBox->Margin = System::Windows::Forms::Padding(5, 0, 5, 0);
			this->ToolStripFilterTextBox->Name = L"ToolStripFilterTextBox";
			this->ToolStripFilterTextBox->Size = System::Drawing::Size(150, 23);
			//
			// ToolStripLabelSelect
			//
			this->ToolStripLabelSelect->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
				this->ActiveScriptsToolStripMenuItem,
					this->UncompiledScriptsToolStripMenuItem
			});
			this->ToolStripLabelSelect->Name = L"ToolStripLabelSelect";
			this->ToolStripLabelSelect->Size = System::Drawing::Size(75, 29);
			this->ToolStripLabelSelect->Text = L"Select all...";
			//
			// ActiveScriptsToolStripMenuItem
			//
			this->ActiveScriptsToolStripMenuItem->Name = L"ActiveScriptsToolStripMenuItem";
			this->ActiveScriptsToolStripMenuItem->Size = System::Drawing::Size(180, 22);
			this->ActiveScriptsToolStripMenuItem->Text = L"Active Scripts";
			this->ActiveScriptsToolStripMenuItem->DisplayStyle = ToolStripItemDisplayStyle::Text;
			//
			// UncompiledScriptsToolStripMenuItem
			//
			this->UncompiledScriptsToolStripMenuItem->Name = L"UncompiledScriptsToolStripMenuItem";
			this->UncompiledScriptsToolStripMenuItem->Size = System::Drawing::Size(180, 22);
			this->UncompiledScriptsToolStripMenuItem->Text = L"Uncompiled Scripts";
			this->UncompiledScriptsToolStripMenuItem->DisplayStyle = ToolStripItemDisplayStyle::Text;
			//
			// ToolStripLabelSelectionCount
			//
			this->ToolStripLabelSelectionCount->Alignment = System::Windows::Forms::ToolStripItemAlignment::Right;
			this->ToolStripLabelSelectionCount->Name = L"ToolStripLabelSelectionCount";
			this->ToolStripLabelSelectionCount->Size = System::Drawing::Size(51, 29);
			this->ToolStripLabelSelectionCount->Text = L"0 selected";
			//
			// ButtonCompleteSelection
			//
			this->ButtonCompleteSelection->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((System::Windows::Forms::AnchorStyles::Bottom | System::Windows::Forms::AnchorStyles::Left));
			this->ButtonCompleteSelection->ImageAlign = System::Drawing::ContentAlignment::MiddleLeft;
			this->ButtonCompleteSelection->Location = System::Drawing::Point(412, 532);
			this->ButtonCompleteSelection->Name = L"ButtonCompleteSelection";
			this->ButtonCompleteSelection->Size = System::Drawing::Size(66, 32);
			this->ButtonCompleteSelection->TabIndex = 5;
			this->ButtonCompleteSelection->Text = L"OK";
			this->ButtonCompleteSelection->UseVisualStyleBackColor = true;
			this->ButtonCompleteSelection->Image = Globals::ScriptEditorImageResourceManager->CreateImage("ScriptListDialogCompleteSelection");
			//
			// DeferredSelectionUpdateTimer
			//
			this->DeferredSelectionUpdateTimer->Enabled = false;
			this->DeferredSelectionUpdateTimer->Interval = 50;

			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(900, 573);
			this->StartPosition = FormStartPosition::Manual;
			this->Controls->Add(this->ButtonCompleteSelection);
			this->Controls->Add(this->BottomToolStrip);
			this->Controls->Add(this->ScriptList);
			this->Controls->Add(this->PreviewBox);
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Text = L"Select Scripts";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->ScriptList))->EndInit();
			this->BottomToolStrip->ResumeLayout(false);
			this->BottomToolStrip->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

			// ### The invocation of the SelectionChanged event apparently is tied to the parent thread's idle message loop
			// ### This breaks event handling for this event when the idle loop isn't active (which are usually active when thread-modal dialog is being shown)
			// ### We need to use SelectedIndexChanged instead.
			ScriptList->SelectedIndexChanged += gcnew EventHandler(this, &SelectScriptDialog::ScriptList_SelectionChanged);
			// ### The SelectedIndexChanged event does not update the SelectedObjects collection when the Shift-key selection triggers the event
			// ### We need to workaround this by deferring the original handler by using a timer
			DeferredSelectionUpdateTimer->Tick += gcnew EventHandler(this, &SelectScriptDialog::DeferredSelectionUpdateTimer_Tick);
			ScriptList->KeyDown += gcnew KeyEventHandler(this, &SelectScriptDialog::ScriptList_KeyDown);
			ScriptList->KeyPress += gcnew KeyPressEventHandler(this, &SelectScriptDialog::ScriptList_KeyPress);
			ScriptList->ItemActivate += gcnew EventHandler(this, &SelectScriptDialog::ScriptList_ItemActivate);
			ButtonCompleteSelection->Click += gcnew EventHandler(this, &SelectScriptDialog::ButtonCompleteSelection_Click);
			ActiveScriptsToolStripMenuItem->Click += gcnew EventHandler(this, &SelectScriptDialog::ActiveScriptsToolStripMenuItem_Click);
			UncompiledScriptsToolStripMenuItem->Click += gcnew EventHandler(this, &SelectScriptDialog::UncompiledScriptsToolStripMenuItem_Click);
			ToolStripFilterTextBox->TextChanged += gcnew EventHandler(this, &SelectScriptDialog::ToolStripFilterTextBox_TextChanged);
			ToolStripFilterTextBox->KeyDown += gcnew KeyEventHandler(this, &SelectScriptDialog::ToolStripFilterTextBox_KeyDown);
			this->Closing += gcnew CancelEventHandler(this, &SelectScriptDialog::Dialog_Cancel);


			BottomToolStrip->Renderer = gcnew CustomToolStripSystemRenderer;
		}


		void SelectScriptDialog::ShowUseReport()
		{
			if (ScriptList->SelectedObject != nullptr)
			{
				NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)ScriptList->SelectedObject;
				nativeWrapper::g_CSEInterfaceTable->EditorAPI.ShowUseReportDialog(Data->ScriptData->EditorID);
			}
		}

		void SelectScriptDialog::SaveBoundsToINI()
		{
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("W", "ScriptEditor::ScriptListDialog",
				(CString(ClientSize.Width.ToString())).c_str());
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("H", "ScriptEditor::ScriptListDialog",
				(CString(ClientSize.Height.ToString())).c_str());
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("X", "ScriptEditor::ScriptListDialog",
				(CString(DesktopLocation.X.ToString())).c_str());
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.WriteToINI("Y", "ScriptEditor::ScriptListDialog",
				(CString(DesktopLocation.Y.ToString())).c_str());

		}

		void SelectScriptDialog::LoadBoundsFromINI()
		{
			char Buffer[0x200] = { 0 };
			int X, Y, W, H;
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("W", "ScriptEditor::ScriptListDialog",
				"916", Buffer, sizeof(Buffer));
			W = Int32::Parse(gcnew String(Buffer));
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("H", "ScriptEditor::ScriptListDialog",
				"541", Buffer, sizeof(Buffer));
			H = Int32::Parse(gcnew String(Buffer));
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("X", "ScriptEditor::ScriptListDialog",
				"0", Buffer, sizeof(Buffer));
			X = Int32::Parse(gcnew String(Buffer));
			nativeWrapper::g_CSEInterfaceTable->EditorAPI.ReadFromINI("Y", "ScriptEditor::ScriptListDialog",
				"0", Buffer, sizeof(Buffer));
			Y = Int32::Parse(gcnew String(Buffer));

			ClientSize = System::Drawing::Size(W, H);
			DesktopLocation = Point(X, Y);
			Location = DesktopLocation;
		}

		void SelectScriptDialog::CompleteSelection()
		{
			if (ScriptList->SelectedObjects->Count == 0)
			{
				MessageBox::Show("Please select one or more scripts to continue.",
					SCRIPTEDITOR_TITLE,
					MessageBoxButtons::OK,
					MessageBoxIcon::Information);
				return;
			}

			if (HasResult)
				return;

			HasResult = true;
			for each (Object ^ Itr in ScriptList->SelectedObjects)
			{
				NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)Itr;
				ResultData->SelectedScriptEditorIDs->Add(Data->EditorID);
			}

			this->Close();
		}

		void SelectScriptDialog::PopulateLoadedScripts(String^ FilterString, bool DefaultSelection)
		{
			ScriptList->DeselectAll();
			ScriptList->ClearObjects();
			ListDataSource->Clear();
			PreviewBox->Text = "";

			List<NativeScriptDataWrapper^>^ ScriptsInSync = gcnew List<NativeScriptDataWrapper^>;
			NativeScriptDataWrapper^ DefaultSelectedItem = nullptr;
			if (LoadedScripts)
			{
				for (int i = 0; i < LoadedScripts->ScriptCount; i++)
				{
					componentDLLInterface::ScriptData* ThisScript = &LoadedScripts->ScriptListHead[i];
					if (ThisScript->IsValid() == false)
						continue;
					else if (Parameters->ShowDeletedScripts == false && ThisScript->IsDeleted())
						continue;

					String^ ScriptEID = gcnew String(ThisScript->EditorID);
					if (FilterString->Length > 0)
					{
						String^ ScriptFormID = ThisScript->FormID.ToString("X8");
						String^ ScriptParent = gcnew String(ThisScript->ParentPluginName);

						if (ScriptEID->IndexOf(FilterString, StringComparison::CurrentCultureIgnoreCase) == -1
							&& ScriptFormID->IndexOf(FilterString, StringComparison::CurrentCultureIgnoreCase) == -1
							&& ScriptParent->IndexOf(FilterString, StringComparison::CurrentCultureIgnoreCase) == -1)
							continue;
					}

					NativeScriptDataWrapper^ NewItem = gcnew NativeScriptDataWrapper(ThisScript);
					ListDataSource->Add(NewItem);
					if (scriptSync::DiskSync::Get()->IsScriptBeingSynced(ScriptEID))
						ScriptsInSync->Add(NewItem);

					if (DefaultSelection && ScriptEID->Equals(Parameters->SelectedScriptEditorID))
						DefaultSelectedItem = NewItem;
				}
			}

			ScriptList->SetObjects(ListDataSource);
			//ScriptList->DisableObjects(ScriptsInSync);

			if (ScriptList->GetItemCount())
			{
				if (DefaultSelectedItem != nullptr)
				{
					ScriptList->SelectedObject = DefaultSelectedItem;
					ScriptList->EnsureModelVisible(DefaultSelectedItem);
				}
				else
					ScriptList->SelectedIndex = 0;
			}
		}

		SelectScriptDialog::FlagType SelectScriptDialog::GetFlagType(componentDLLInterface::ScriptData* Script)
		{
			String^ ScriptEID = gcnew String(Script->EditorID);
			if (scriptSync::DiskSync::Get()->IsScriptBeingSynced(ScriptEID))
				return FlagType::Syncing;
			else if (Script->Compiled == false)
				return FlagType::Uncompiled;
			else if (Script->IsActive())
				return FlagType::Active;
			else if (Script->IsDeleted())
				return FlagType::Deleted;
			else
				return FlagType::Default;
		}

		System::Object^ SelectScriptDialog::ScriptListAspectScriptNameGetter(Object^ RowObject)
		{
			if (RowObject == nullptr)
				return nullptr;

			NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
			return Data->EditorID;
		}

		System::Object^ SelectScriptDialog::ScriptListAspectFormIDGetter(Object^ RowObject)
		{
			if (RowObject == nullptr)
				return nullptr;

			NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
			return Data->ScriptData->FormID;
		}

		System::String^ SelectScriptDialog::ScriptListAspectToStringConverterFormID(Object^ RowObject)
		{
			if (RowObject == nullptr)
				return nullptr;

			return ((UInt32)RowObject).ToString("X8");
		}

		System::Object^ SelectScriptDialog::ScriptListAspectTypeGetter(Object^ RowObject)
		{
			if (RowObject == nullptr)
				return nullptr;

			NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
			return Data->Type;
		}

		System::Object^ SelectScriptDialog::ScriptListAspectParentPluginGetter(Object^ RowObject)
		{
			if (RowObject == nullptr)
				return nullptr;

			NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
			return Data->ParentPluginName;
		}

		System::Object^ SelectScriptDialog::ScriptListAspectFlagsGetter(Object^ RowObject)
		{
			if (RowObject == nullptr)
				return nullptr;

			NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
			return GetFlagType(Data->ScriptData);
		}

		System::String^ SelectScriptDialog::ScriptListAspectToStringConverterFlags(Object^ RowObject)
		{
			return String::Empty;
		}

		System::Object^ SelectScriptDialog::ScriptListImageFlagsGetter(Object^ RowObject)
		{
			if (RowObject == nullptr)
				return nullptr;

			NativeScriptDataWrapper^ Data = (NativeScriptDataWrapper^)RowObject;
			FlagType Flag = GetFlagType(Data->ScriptData);
			if (Flag != FlagType::Default)
				return (int)Flag;
			else
				return nullptr;
		}

		SelectScriptDialog::SelectScriptDialog(SelectScriptDialogParams^ Params) :
			AnimatedForm(0.125)
		{
			InitializeComponent();

			HasResult = false;
			ResultData = gcnew SelectScriptDialogData;

			Parameters = Params;
			LoadedScripts = nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetScriptList();
			ListDataSource = gcnew List<NativeScriptDataWrapper ^>;

			PopulateLoadedScripts(Parameters->FilterString, true);
			ButtonCompleteSelection->Enabled = false;

			LoadBoundsFromINI();

			this->Hide();
			this->ShowDialog();
		}

		SelectScriptDialog::~SelectScriptDialog()
		{
			if (LoadedScripts)
				nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(LoadedScripts, false);

			if (components)
			{
				delete components;
			}
		}

	}
}
