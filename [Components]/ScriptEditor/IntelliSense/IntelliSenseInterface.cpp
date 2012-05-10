#include "IntelliSenseItem.h"
#include "IntelliSenseInterface.h"
#include "IntelliSenseDatabase.h"
#include "[Common]\ListViewUtilities.h"
#include "..\Globals.h"
#include "..\ScriptEditorPreferences.h"
#include "..\ScriptEditorManager.h"

namespace ConstructionSetExtender
{
	using namespace ScriptEditor;

	namespace IntelliSense
	{
		int IntelliSenseItemSorter::Compare( ListViewItem^ X, ListViewItem^ Y )
		{
			int Result = -1;
			Result = String::Compare((dynamic_cast<IntelliSenseItem^>(X->Tag))->GetIdentifier(),
									(dynamic_cast<IntelliSenseItem^>(Y->Tag))->GetIdentifier(),
									true);

			if (Order == SortOrder::Descending)
				Result *= -1;

			return Result;
		}

		IntelliSenseInterface::IntelliSenseInterface(UInt32 ParentWorkspaceIndex)
		{
			this->ParentWorkspaceIndex = ParentWorkspaceIndex;

			IntelliSenseBox = gcnew NonActivatingImmovableAnimatedForm();
			LocalVariableDatabase = gcnew List<IntelliSenseItem^>();
			VirtualListCache = gcnew List<ListViewItem^>();
			IntelliSenseList = gcnew ListView();

			IntelliSenseListSelectedIndexChangedHandler = gcnew EventHandler(this, &IntelliSenseInterface::IntelliSenseList_SelectedIndexChanged);
			IntelliSenseListKeyDownHandler = gcnew KeyEventHandler(this, &IntelliSenseInterface::IntelliSenseList_KeyDown);
			IntelliSenseListMouseDoubleClickHandler = gcnew MouseEventHandler(this, &IntelliSenseInterface::IntelliSenseList_MouseDoubleClick);
			IntelliSenseListRetrieveVirtualItemEventHandler = gcnew RetrieveVirtualItemEventHandler(this, &IntelliSenseInterface::IntelliSenseList_RetrieveVirtualItem);
			IntelliSenseBoxCancelHandler = gcnew CancelEventHandler(this, &IntelliSenseInterface::IntelliSenseBox_Cancel);

			IntelliSenseBox->FormBorderStyle = FormBorderStyle::SizableToolWindow;
			IntelliSenseBox->ShowInTaskbar = false;
			IntelliSenseBox->ShowIcon = false;
			IntelliSenseBox->ControlBox = false;
			IntelliSenseBox->Controls->Add(IntelliSenseList);

			IntelliSenseList->View = View::Details;
			IntelliSenseList->Dock = DockStyle::Fill;
			IntelliSenseList->MultiSelect = false;
			IntelliSenseList->SmallImageList = gcnew ImageList();
			IntelliSenseList->SmallImageList->TransparentColor = Color::White;
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemEmpty"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemCommand"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemLocalVar"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemRemoteVar"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemUDF"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemQuest"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemGlobalVar"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemGMST"));
			IntelliSenseList->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("IntelliSenseItemForm"));
			IntelliSenseList->Location = Point(0, 0);
			IntelliSenseList->Font = gcnew Font("Lucida Grande", 9, FontStyle::Regular);
			IntelliSenseList->LabelEdit = false;
			IntelliSenseList->CheckBoxes = false;
			IntelliSenseList->FullRowSelect = true;
			IntelliSenseList->GridLines = false;
			IntelliSenseList->Sorting = SortOrder::None;
			IntelliSenseList->HeaderStyle = ColumnHeaderStyle::None;
			IntelliSenseList->HideSelection = false;
			IntelliSenseList->Columns->Add("IntelliSense Object", 203);
			IntelliSenseList->Sorting = SortOrder::Ascending;

			InfoToolTip->AutoPopDelay = 500;
			InfoToolTip->InitialDelay = 500;
			InfoToolTip->ReshowDelay = 0;
			InfoToolTip->ToolTipIcon = ToolTipIcon::None;

			Enabled = true;
			LastOperation = Operation::e_Default;
			Destroying = false;
			MaximumVisibleItemCount = 10;
			PreventActivation = false;

			IntelliSenseBox->SetSize(Size(0, 0));
			IntelliSenseBox->ShowForm(Point(0,0), IntelliSenseBox->Handle, false);
			IntelliSenseBox->Hide();

			RemoteScript = Script::NullScript;
			CallingObjectIsRef = false;

			IntelliSenseBox->Closing += IntelliSenseBoxCancelHandler;
			IntelliSenseList->SelectedIndexChanged += IntelliSenseListSelectedIndexChangedHandler;
			IntelliSenseList->KeyDown += IntelliSenseListKeyDownHandler;
			IntelliSenseList->MouseDoubleClick += IntelliSenseListMouseDoubleClickHandler;
			IntelliSenseList->RetrieveVirtualItem += IntelliSenseListRetrieveVirtualItemEventHandler;
		}

		void IntelliSenseInterface::DisplayInfoToolTip(String^ Title, String^ Message, Point Location, IntPtr ParentHandle, UInt32 Duration)
		{
			InfoToolTip->Tag = ParentHandle;
			InfoToolTip->ToolTipTitle = Title;
			InfoToolTip->Show(Message, Control::FromHandle(ParentHandle), Location, Duration);
		}

		void IntelliSenseInterface::HideInfoToolTip()
		{
			try		// to account for disposed controls
			{
				if (InfoToolTip->Tag != nullptr && Control::FromHandle((IntPtr)InfoToolTip->Tag) != nullptr)
					InfoToolTip->Hide(Control::FromHandle((IntPtr)InfoToolTip->Tag));
			}
			catch (...) {}

			InfoToolTip->Tag = nullptr;
		}

		void IntelliSenseInterface::ShowInterface( IntelliSenseInterface::Operation DisplayOperation, bool ForceDisplay, bool ShowAllItems )
		{
			if (Enabled == false && ForceDisplay == false)
				return;

			UInt32 ItemCount = 0;
			CleanupInterface();

			ScriptEditor::Workspace^ ParentEditor = SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex);
			String^ Extract = ParentEditor->GetCurrentToken();

			switch (DisplayOperation)
			{
			case Operation::e_Default:
				if (Extract->Length >= PREFERENCES->FetchSettingAsInt("ThresholdLength", "IntelliSense") || ForceDisplay)
				{
					for each (IntelliSenseItem^ Itr in LocalVariableDatabase)
					{
						if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr))
						{
							ListViewItem^ Item = gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetIntelliSenseItemType());
							Item->Tag = Itr;
							VirtualListCache->Add(Item);
							ItemCount++;
						}
					}

					for each (IntelliSenseItem^ Itr in ISDB->Enumerables)
					{
						if ((Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Cmd &&
							dynamic_cast<IntelliSenseItemScriptCommand^>(Itr)->GetRequiresParent() == false) ||
							Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Quest ||
							Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_GlobalVar ||
							Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_GMST ||
							Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Form)
						{
							if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr))
							{
								ListViewItem^ Item = gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetIntelliSenseItemType());
								Item->Tag = Itr;
								VirtualListCache->Add(Item);
								ItemCount++;
							}
						}
					}
				}

				break;
			case Operation::e_Call:
				for each (IntelliSenseItem^ Itr in ISDB->Enumerables)
				{
					if (Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_UserFunct)
					{
						if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
						{
							ListViewItem^ Item = gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetIntelliSenseItemType());
							Item->Tag = Itr;
							VirtualListCache->Add(Item);
							ItemCount++;
						}
					}
				}

				break;
			case Operation::e_Dot:
				if (ShowAllItems)
				{
					IntelliSenseItemVariable^ RefVar = LookupLocalVariableByIdentifier(Extract);
					if (RefVar != nullptr && RefVar->GetDataType() == IntelliSenseItemVariable::IntelliSenseItemVariableDataType::e_Ref)
					{
						CallingObjectIsRef = true;
					}
					else if (!String::Compare(Extract, "player", true) || !String::Compare(Extract, "playerref", true))
					{
						CallingObjectIsRef = true;
					}
					else
					{
						CString CStr(Extract);						// extract = calling ref
						ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CStr.c_str());
						if (Data && !Data->IsValid())
						{
							LastOperation = Operation::e_Default;
							break;
						}
						else if (Data)
						{
							RemoteScript = ISDB->CacheRemoteScript(gcnew String(Data->ParentID), gcnew String(Data->Text));
							CallingObjectIsRef = NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetIsFormReference(CStr.c_str());
						}
						else
						{
							CallingObjectIsRef = false;
							RemoteScript = Script::NullScript;
						}
						NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);
					}
				}

				for (Script::VarListT::Enumerator^ RemoteVarItr = RemoteScript->GetVariableListEnumerator(); RemoteVarItr->MoveNext();)
				{
					if (RemoteVarItr->Current->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_RemoteVar)
					{
						if (RemoteVarItr->Current->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
						{
							ListViewItem^ Item = gcnew ListViewItem(RemoteVarItr->Current->GetIdentifier(), (int)RemoteVarItr->Current->GetIntelliSenseItemType());
							Item->Tag = RemoteVarItr->Current;
							VirtualListCache->Add(Item);
							ItemCount++;
						}
					}
				}

				for each (IntelliSenseItem^ Itr in ISDB->Enumerables)
				{
					if (Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Cmd && CallingObjectIsRef)
					{
						if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
						{
							ListViewItem^ Item = gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetIntelliSenseItemType());
							Item->Tag = Itr;
							VirtualListCache->Add(Item);
							ItemCount++;
						}
					}
				}

				break;
			case Operation::e_Assign:
				for each (IntelliSenseItem^ Itr in LocalVariableDatabase)
				{
					if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
					{
						ListViewItem^ Item = gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetIntelliSenseItemType());
						Item->Tag = Itr;
						VirtualListCache->Add(Item);
						ItemCount++;
					}
				}

				for each (IntelliSenseItem^ Itr in ISDB->Enumerables)
				{
					if (Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_Quest ||
						Itr->GetIntelliSenseItemType() == IntelliSenseItem::IntelliSenseItemType::e_GlobalVar)
					{
						if (Itr->GetIdentifier()->StartsWith(Extract, true, nullptr) || ShowAllItems)
						{
							ListViewItem^ Item = gcnew ListViewItem(Itr->GetIdentifier(), (int)Itr->GetIntelliSenseItemType());
							Item->Tag = Itr;
							VirtualListCache->Add(Item);
							ItemCount++;
						}
					}
				}

				break;
			}

			VirtualListCache->Sort(gcnew IntelliSenseItemSorter(SortOrder::Ascending));

			if (ItemCount == 1 &&
				String::Compare((dynamic_cast<IntelliSenseItem^>(VirtualListCache[0]->Tag))->GetIdentifier(),
								Extract,
								true) == 0)
			{
				HideInterface();		// do not show when enumerable == extract
			}
			else if (ItemCount > 0)
			{
				IntelliSenseList->VirtualListSize = ItemCount;

				Point Location = ParentEditor->GetCaretLocation(true);
				Location.X += 3; Location.Y += PREFERENCES->FetchSettingAsInt("FontSize", "Appearance") + 3;

				if (ItemCount > MaximumVisibleItemCount)
					ItemCount = MaximumVisibleItemCount;

				Size DisplaySize = Size(240, (MaximumVisibleItemCount * 19) + 17 - ((MaximumVisibleItemCount - ItemCount) * 19));
				IntelliSenseBox->SetSize(DisplaySize);
				IntelliSenseBox->ShowForm(ParentEditor->GetScreenPoint(Location), ParentEditor->GetControlBoxHandle(), !IntelliSenseBox->Visible);

				ParentEditor->Focus();

				IntelliSenseList->SelectedIndices->Add(0);
			}
			else
			{
				HideInterface();
			}

			LastOperation = DisplayOperation;
		}

		IntelliSenseItemVariable^ IntelliSenseInterface::LookupLocalVariableByIdentifier(String^% Identifier)
		{
			for each (IntelliSenseItem^ Itr in LocalVariableDatabase)
			{
				if (!String::Compare(Itr->GetIdentifier(), Identifier, true))
					return dynamic_cast<IntelliSenseItemVariable^>(Itr);
			}

			return nullptr;
		}

		void IntelliSenseInterface::IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
		{
			ScriptEditor::Workspace^ ParentEditor = SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex);

			if (Visible)
			{
				int Index = GetListViewSelectedItemIndex(IntelliSenseList);
				if (Index == -1)
					return;

				Point Location = Point(IntelliSenseList->Size.Width + 17, 0);

				DisplayInfoToolTip((dynamic_cast<IntelliSenseItem^>(VirtualListCache[Index]->Tag))->GetIntelliSenseItemTypeID(),
									(dynamic_cast<IntelliSenseItem^>(VirtualListCache[Index]->Tag))->Describe(),
									Location,
									IntelliSenseBox->Handle,
									15000);
			}
		}

		void IntelliSenseInterface::IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::Escape:
				HideInterface();
				break;
			case Keys::Tab:
			case Keys::Enter:
				PickSelection();
				break;
			}
		}

		void IntelliSenseInterface::IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E)
		{
			if (GetListViewSelectedItemIndex(IntelliSenseList) == -1)
				return;

			PickSelection();
		}

		void IntelliSenseInterface::IntelliSenseList_RetrieveVirtualItem( Object^ Sender, RetrieveVirtualItemEventArgs^ E )
		{
			E->Item = VirtualListCache[E->ItemIndex];
		}

		void IntelliSenseInterface::IntelliSenseBox_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			if (Destroying == false)
				E->Cancel = true;
		}

		void IntelliSenseInterface::ChangeCurrentSelection(IntelliSenseInterface::MoveDirection Direction)
		{
			int SelectedIndex = GetListViewSelectedItemIndex(IntelliSenseList);
			if (SelectedIndex == -1)
				return;

			switch (Direction)
			{
			case MoveDirection::e_Down:
				if (SelectedIndex < (VirtualListCache->Count - 1))
				{
					VirtualListCache[SelectedIndex]->Selected = false;
					VirtualListCache[SelectedIndex + 1]->Selected = true;

					if (IntelliSenseList->TopItem->Index < VirtualListCache->Count - 1)
						IntelliSenseList->TopItem = VirtualListCache[IntelliSenseList->TopItem->Index + 1];
				}

				break;
			case MoveDirection::e_Up:
				if (SelectedIndex > 0)
				{
					VirtualListCache[SelectedIndex]->Selected = false;
					VirtualListCache[SelectedIndex - 1]->Selected = true;

					if (IntelliSenseList->TopItem->Index > 0 )
						IntelliSenseList->TopItem = VirtualListCache[IntelliSenseList->TopItem->Index - 1];
				}

				break;
			}
		}

		void IntelliSenseInterface::UpdateLocalVariableDatabase()
		{
			ScriptEditor::Workspace^ ParentEditor = SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex);
			if (ParentEditor == nullptr)		// will return true when called inside a workspace ctor
				return;

			ISDB->ParseScript(ParentEditor->GetScriptText(), gcnew IntelliSenseParseScriptData(this));
		}

		void IntelliSenseInterface::PickSelection()
		{
			ScriptEditor::Workspace^ ParentEditor = SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex);

			String^ Result;
			ParentEditor->Focus();

			int Index = GetListViewSelectedItemIndex(IntelliSenseList);
			if (Index != -1)
			{
				Result = (dynamic_cast<IntelliSenseItem^>(VirtualListCache[Index]->Tag))->GetIdentifier();
				CleanupInterface();
				HideInterface();
			}
			else
				return;

			ParentEditor->SetCurrentToken(Result);
		}

		void IntelliSenseInterface::HideInterface()
		{
			if (Visible)
			{
				IntelliSenseBox->HideForm(true);
				HideInfoToolTip();
			}
		}

		void IntelliSenseInterface::CleanupInterface()
		{
			IntelliSenseList->VirtualListSize = 0;
			IntelliSenseList->VirtualMode = true;
			VirtualListCache->Clear();
		}

		bool IntelliSenseInterface::ShowQuickInfoTip(String^ MainToken, String^ ParentToken, Point Location)
		{
			if (PREFERENCES->FetchSettingAsInt("UseQuickView", "IntelliSense") == 0)
				return false;

			ScriptEditor::Workspace^ ParentEditor = SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex);

			CString CStr(ParentToken);
			ComponentDLLInterface::ScriptData* Data = NativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CStr.c_str());
			if (Data && Data->IsValid())
			{
				ParentToken = "" + gcnew String(Data->ParentID);
				ISDB->CacheRemoteScript(gcnew String(Data->ParentID), gcnew String(Data->Text));
			}
			NativeWrapper::g_CSEInterfaceTable->DeleteNativeHeapPointer(Data, false);

			IntelliSenseItem^ Item = ISDB->LookupRemoteScriptVariable(ParentToken, MainToken);

			if (Item == nullptr)
				Item = LookupLocalVariableByIdentifier(MainToken);

			if (Item == nullptr)
			{
				for each (IntelliSenseItem^ Itr in ISDB->Enumerables)
				{
					if (!String::Compare(Itr->GetIdentifier(), MainToken, true))
					{
						Item = Itr;
						break;
					}
				}
			}

			if (Item != nullptr)
			{
				Location.Y += PREFERENCES->FetchSettingAsInt("FontSize", "Appearance");
				DisplayInfoToolTip(Item->GetIntelliSenseItemTypeID(),
									Item->Describe(),
									Location,
									ParentEditor->GetEditorBoxHandle(),
									8000);

				return true;
			}
			else
				return false;
		}

		bool IntelliSenseInterface::ShowQuickViewTooltip(String^ MainToken, String^ ParentToken)
		{
			ScriptEditor::Workspace^ ParentEditor = SEMGR->GetAllocatedWorkspace(ParentWorkspaceIndex);

			return ShowQuickInfoTip(MainToken, ParentToken, ParentEditor->GetCaretLocation(true));
		}

		bool IntelliSenseInterface::ShowQuickViewTooltip(String^ MainToken, String^ ParentToken, Point MouseLocation)
		{
			return ShowQuickInfoTip(MainToken, ParentToken, MouseLocation);
		}

		void IntelliSenseInterface::Destroy()
		{
			Destroying = true;
			RemoteScript = nullptr;

			IntelliSenseBox->Closing -= IntelliSenseBoxCancelHandler;
			IntelliSenseList->SelectedIndexChanged -= IntelliSenseListSelectedIndexChangedHandler;
			IntelliSenseList->KeyDown -= IntelliSenseListKeyDownHandler;
			IntelliSenseList->MouseDoubleClick -= IntelliSenseListMouseDoubleClickHandler;
			IntelliSenseList->RetrieveVirtualItem -= IntelliSenseListRetrieveVirtualItemEventHandler;

			for each (Image^ Itr in IntelliSenseList->SmallImageList->Images)
				delete Itr;

			IntelliSenseList->SmallImageList->Images->Clear();
			IntelliSenseList->SmallImageList = nullptr;
			CleanupInterface();
			HideInfoToolTip();
			LocalVariableDatabase->Clear();
			VirtualListCache->Clear();
			IntelliSenseBox->Close();

			delete IntelliSenseBox;
			delete IntelliSenseList;
		}

		void IntelliSenseInterface::AddLocalVariableToDatabase( IntelliSenseItemVariable^ Variable )
		{
			LocalVariableDatabase->Add(Variable);
		}

		void IntelliSenseInterface::ClearLocalVariableDatabase()
		{
			LocalVariableDatabase->Clear();
		}
}
}