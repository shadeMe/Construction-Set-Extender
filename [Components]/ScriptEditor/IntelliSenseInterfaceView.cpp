#include "IntelliSenseInterfaceView.h"
#include "IntelliSenseItem.h"
#include "Preferences.h"
#include "Globals.h"

namespace cse
{
	namespace intellisense
	{
		IntelliSenseInterfaceView::IntelliSenseInterfaceView()
		{
			BoundModel = nullptr;

			Form = gcnew AnimatedForm(0.1, true);
			ListView = gcnew BrightIdeasSoftware::FastObjectListView;

			ListViewSelectionChangedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_SelectionChanged);
			ListViewKeyDownHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyDown);
			ListViewKeyUpHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyUp);
			ListViewItemActivateHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_ItemActivate);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ScriptEditorPreferences_Saved);
			ListViewFormatRowHandler = gcnew EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>(this, &IntelliSenseInterfaceView::ListView_FormatRow);

			ListView->KeyDown += ListViewKeyDownHandler;
			ListView->KeyUp += ListViewKeyUpHandler;
			ListView->ItemActivate += ListViewItemActivateHandler;
			ListView->SelectedIndexChanged += ListViewSelectionChangedHandler;
			ListView->FormatRow += ListViewFormatRowHandler;
			preferences::SettingsHolder::Get()->SavedToDisk += ScriptEditorPreferencesSavedHandler;

			Form->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			Form->AutoScaleMode = AutoScaleMode::Font;
			Form->FormBorderStyle = FormBorderStyle::SizableToolWindow;
			Form->ShowInTaskbar = false;
			Form->ShowIcon = false;
			Form->ControlBox = false;
			Form->Controls->Add(ListView);
			//Form->PreventActivation = true;

			ListView->View = View::Details;
			ListView->Dock = DockStyle::Fill;
			ListView->MultiSelect = false;
			ListView->SmallImageList = gcnew ImageList();
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemEmpty"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemCommand"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemLocalVar"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemQuest"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemUDF"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemUDF"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemGMST"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemGlobalVar"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemForm"));
			ListView->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("IntelliSenseItemSnippet"));
			ListView->Location = Point(0, 0);
			ListView->LabelEdit = false;
			ListView->CheckBoxes = false;
			ListView->FullRowSelect = true;
			ListView->GridLines = false;
			ListView->HeaderStyle = ColumnHeaderStyle::None;
			ListView->HideSelection = true;

			BrightIdeasSoftware::OLVColumn^ Column = gcnew BrightIdeasSoftware::OLVColumn;
			Column->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&IntelliSenseInterfaceView::ListViewAspectGetter);
			Column->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&IntelliSenseInterfaceView::ListViewImageGetter);
			Column->Text = "IntelliSense Item";
			Column->Width = 203;
			ListView->AllColumns->Add(Column);
			ListView->Columns->Add(Column);

			ListViewPopup = gcnew ModalToolTip;
			ListViewPopup->AutoPopDelay = 0;
			ListViewPopup->InitialDelay = 0;
			ListViewPopup->ReshowDelay = 0;
			ListViewPopup->ToolTipIcon = ToolTipIcon::None;
			ListViewPopup->Tag = nullptr;
			ListViewPopup->ShowAlways = true;
			//ListViewPopup->UseAnimation = false;
			//ListViewPopup->UseFading = false;

			InsightPopup = gcnew ToolTip;
			InsightPopup->InitialDelay = 0;
			InsightPopup->ReshowDelay = 0;
			InsightPopup->ToolTipIcon = ToolTipIcon::None;
			InsightPopup->Tag = nullptr;

			MaximumVisibleItemCount = preferences::SettingsHolder::Get()->IntelliSense->MaxSuggestionsToDisplay;
			InsightPopupDisplayDuration = preferences::SettingsHolder::Get()->IntelliSense->InsightToolTipDisplayDuration;

			Form->SetSize(Size(0, 0));
			Form->Show(Point(0, 0), Form->Handle, false);
			Form->Hide();
		}

		IntelliSenseInterfaceView::~IntelliSenseInterfaceView()
		{
			Debug::Assert(Bound == false);

			ListView->KeyDown -= ListViewKeyDownHandler;
			ListView->KeyUp -= ListViewKeyUpHandler;
			ListView->ItemActivate -= ListViewItemActivateHandler;
			ListView->SelectedIndexChanged -= ListViewSelectionChangedHandler;
			ListView->FormatRow -= ListViewFormatRowHandler;
			preferences::SettingsHolder::Get()->SavedToDisk -= ScriptEditorPreferencesSavedHandler;

			SAFEDELETE_CLR(ListViewKeyDownHandler);
			SAFEDELETE_CLR(ListViewKeyUpHandler);
			SAFEDELETE_CLR(ListViewItemActivateHandler);
			SAFEDELETE_CLR(ListViewSelectionChangedHandler);
			SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);

			for each (Image^ Itr in ListView->SmallImageList->Images)
				delete Itr;

			HideListViewToolTip();
			HideInsightToolTip();
			Form->ForceClose();

			delete InsightPopup;
			delete ListViewPopup;
			delete Form;
			delete ListView;
		}

		void IntelliSenseInterfaceView::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			MaximumVisibleItemCount = preferences::SettingsHolder::Get()->IntelliSense->MaxSuggestionsToDisplay;
			InsightPopupDisplayDuration = preferences::SettingsHolder::Get()->IntelliSense->InsightToolTipDisplayDuration;
		}

		void IntelliSenseInterfaceView::ListView_SelectionChanged(Object^ Sender, EventArgs^ E)
		{
			if (Form->Visible)
			{
				IntelliSenseItem^ Current = (IntelliSenseItem^)ListView->SelectedObject;
				if (Current)
					ShowListViewToolTip(Current);
			}
		}

		void IntelliSenseInterfaceView::ListView_ItemActivate(Object^ Sender, EventArgs^ E)
		{
			Debug::Assert(Bound == true);

			ItemSelected(this, E);
		}

		void IntelliSenseInterfaceView::ListView_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			Debug::Assert(Bound == true);

			switch (E->KeyCode)
			{
			case Keys::Escape:
				Dismissed(this, E);
				break;
			case Keys::Tab:
			case Keys::Enter:
				ItemSelected(this, E);
				break;
			case Keys::ControlKey:
				DimOpacity();
				break;
			}
		}

		void IntelliSenseInterfaceView::ListView_KeyUp(Object^ Sender, KeyEventArgs^ E)
		{
			Debug::Assert(Bound == true);

			switch (E->KeyCode)
			{
			case Keys::ControlKey:
				ResetOpacity();
				break;
			}
		}

		void IntelliSenseInterfaceView::ListView_FormatRow(Object^ Sender, BrightIdeasSoftware::FormatRowEventArgs^ E)
		{
			if (E->Model == nullptr)
				return;

			if (E->Model == E->ListView->SelectedObject)
			{
				auto Temp = E->Item->ForeColor;
				E->Item->ForeColor = E->Item->BackColor;
				E->Item->BackColor = Temp;

				E->Item->SelectedForeColor = E->Item->ForeColor;
				E->Item->SelectedBackColor = E->Item->BackColor;

				//E->Item->Font = gcnew Font(E->Item->Font->FontFamily, E->Item->Font->Size, E->Item->Font->Style | FontStyle::Bold);
			}
		}

		Object^ IntelliSenseInterfaceView::ListViewAspectGetter(Object^ RowObject)
		{
			if (RowObject)
			{
				IntelliSenseItem^ Item = (IntelliSenseItem^)RowObject;
				Debug::Assert(Item != nullptr);

				return Item->GetIdentifier();
			}
			else
				return nullptr;
		}

		Object^ IntelliSenseInterfaceView::ListViewImageGetter(Object^ RowObject)
		{
			if (RowObject)
			{
				IntelliSenseItem^ Item = (IntelliSenseItem^)RowObject;
				Debug::Assert(Item != nullptr);

				return (int)Item->GetItemType();
			}
			else
				return nullptr;
		}

		void IntelliSenseInterfaceView::ShowListViewToolTip(IntelliSenseItem^ Item)
		{
			const UInt32 kDefaultDisplayDuration = 30 * 1000;

			ListViewPopup->Tag = Form->Handle;
			ListViewPopup->ToolTipTitle = Item->GetItemTypeName();
			ListViewPopup->Show(Item->Describe(), Control::FromHandle(Form->Handle), Point(ListView->Size.Width + 17, 0), kDefaultDisplayDuration);
			//ListViewPopup->StopHideTimer();
		}

		void IntelliSenseInterfaceView::HideListViewToolTip()
		{
			ListViewPopup->Hide(Control::FromHandle(Form->Handle));
			ListViewPopup->Tag = nullptr;
		}

		void IntelliSenseInterfaceView::Bind(IIntelliSenseInterfaceModel^ To)
		{
			Debug::Assert(Bound == false);

			BoundModel = To;
		}

		void IntelliSenseInterfaceView::Unbind()
		{
			if (Bound)
			{
				BoundModel = nullptr;
				Hide();
				HideInsightToolTip();
			}
		}

		void IntelliSenseInterfaceView::ChangeSelection(IIntelliSenseInterfaceView::MoveDirection Direction)
		{
			if (!Visible)
				return;

			int SelectedIndex = ListView->SelectedIndex;
			if (SelectedIndex == -1)
				return;

			switch (Direction)
			{
			case IIntelliSenseInterfaceView::MoveDirection::Down:
				if (SelectedIndex < ListView->GetItemCount() - 1)
					++SelectedIndex;

				break;
			case IIntelliSenseInterfaceView::MoveDirection::Up:
				if (SelectedIndex > 0)
					--SelectedIndex;

				break;
			}

			if (SelectedIndex != ListView->SelectedIndex)
			{
				auto ModelObject = ListView->GetModelObject(SelectedIndex);
				if (ModelObject)
				{
					ListView->SelectObject(ModelObject);
					ListView->EnsureModelVisible(ModelObject);
				}
			}
		}

		void IntelliSenseInterfaceView::DimOpacity()
		{
			if (!Visible)
				return;

			Form->Opacity = DimmedOpacity;
		}

		void IntelliSenseInterfaceView::ResetOpacity()
		{
			if (!Visible)
				return;

			Form->Opacity = 1.f;
		}

		void IntelliSenseInterfaceView::ShowInsightToolTip(IntelliSenseShowInsightToolTipArgs^ Args)
		{
			InsightPopup->ToolTipTitle = Args->Title;
			InsightPopup->ToolTipIcon = Args->Icon;
			InsightPopup->Tag = Args->ParentWindowHandle;
			InsightPopup->Show(Args->Text,
							Control::FromHandle(Args->ParentWindowHandle),
							Args->DisplayScreenCoords, InsightPopupDisplayDuration * 1000);
		}

		void IntelliSenseInterfaceView::HideInsightToolTip()
		{
			if (InsightPopup->Tag)
			{
				InsightPopup->Hide(Control::FromHandle((IntPtr)InsightPopup->Tag));
				InsightPopup->Tag = nullptr;
			}
		}

		void IntelliSenseInterfaceView::Update()
		{
			Debug::Assert(Bound == true);

			ListView->SetObjects(BoundModel->DataStore);

			if (BoundModel->DataStore->Count)
			{
				int ItemCount = BoundModel->DataStore->Count;
				if (ItemCount > MaximumVisibleItemCount)
					ItemCount = MaximumVisibleItemCount;
				int ItemHeight = ListView->Items[0]->Bounds.Height;

				// mucking about with the CS' compatibility settings (setting it to Windows 7, for instance)
				// yields a different result than when the setting's disabled
				Size DisplaySize = Size(240, (MaximumVisibleItemCount * ItemHeight + ItemHeight) - ((MaximumVisibleItemCount - ItemCount) * ItemHeight));

				Form->BeginInvoke(gcnew UIInvokeDelegate_FormSetSize(&IntelliSenseInterfaceView::UIInvoke_FormSetSize), gcnew array < Object^ > { this, Form, DisplaySize });
			}
		}

		void IntelliSenseInterfaceView::Show(Point Location, IntPtr Parent)
		{
			Form->BeginInvoke(gcnew UIInvokeDelegate_FormShow(&IntelliSenseInterfaceView::UIInvoke_FormShow), gcnew array < Object^ > { this, Form, Location, Parent });
		}

		void IntelliSenseInterfaceView::Hide()
		{
			ListView->ClearObjects();

			if (Form->Visible)
			{
				HideListViewToolTip();

				Form->BeginInvoke(gcnew UIInvokeDelegate_FormHide(&IntelliSenseInterfaceView::UIInvoke_FormHide), gcnew array < Object^ > { this, Form });
			}

		}

		// HACK!
		// The SetSize call crashes consistently under certain conditions (which are yet to be decoded) due to an invalid window handle
		// Methinks it has something to do with how the call is invoked (multiple levels of interop b'ween WinForms and WPF)
		// Delegating it to the UI thread through BeginInvoke seems to help apparently, but we still need to wrap it in an exception handler

		void IntelliSenseInterfaceView::UIInvoke_FormShow(IntelliSenseInterfaceView^ Sender,
			AnimatedForm^ ToInvoke, Point Location, IntPtr Parent)
		{
			try
			{
				ToInvoke->Show(Location, Parent, (ToInvoke->Visible == false));
				Sender->HideListViewToolTip();

				if (Sender->BoundModel->DataStore->Count)
				{
					auto DefaultSelection = Sender->BoundModel->DataStore[0];
					Sender->ListView->SelectObject(DefaultSelection);

					// The SelectionChanged event doesn't get raised consistently at this point
					// So, we ensure that the tooltip is shown
					Sender->ShowListViewToolTip(DefaultSelection);
				}
			}
			catch (Exception^ E) {
#ifndef NDEBUG
				DebugPrint("IntelliSenseInterfaceView::UIInvoke_FormShow Exception! Message - " + E->Message);
				Debugger::Break();
#endif // !NDEBUG
			}
		}

		void IntelliSenseInterfaceView::UIInvoke_FormSetSize(IntelliSenseInterfaceView^ Sender,
			AnimatedForm^ ToInvoke, Size ToSet)
		{
			try {
				ToInvoke->SetSize(ToSet);
			}
			catch (Exception^ E) {
#ifndef NDEBUG
				DebugPrint("IntelliSenseInterfaceView::UIInvoke_FormSetSize Exception! Message - " + E->Message);
				Debugger::Break();
#endif // !NDEBUG
			}
		}

		void IntelliSenseInterfaceView::UIInvoke_FormHide(IntelliSenseInterfaceView^ Sender, AnimatedForm^ ToInvoke)
		{
			try {
				ToInvoke->Hide();
			}
			catch (Exception^ E) {
#ifndef NDEBUG
				DebugPrint("IntelliSenseInterfaceView::UIInvoke_FormHide Exception! Message - " + E->Message);
				Debugger::Break();
#endif // !NDEBUG
			}
		}
	}
}