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

			Form = gcnew AnimatedForm(true);
			ListView = gcnew BrightIdeasSoftware::FastObjectListView;

			ListViewSelectionChangedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_SelectionChanged);
			ListViewKeyDownHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyDown);
			ListViewKeyUpHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyUp);
			ListViewItemActivateHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_ItemActivate);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ScriptEditorPreferences_Saved);
			ListViewFormatRowHandler = gcnew EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>(this, &IntelliSenseInterfaceView::ListView_FormatRow);
			SelectFirstItemOnShowHandler = gcnew AnimatedForm::TransitionCompleteHandler(this, &IntelliSenseInterfaceView::SelectFirstItemOnShow);

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

			IntelliSenseItemImages = gcnew ImageList();
			IntelliSenseItem::PopulateImageListWithItemTypeImages(IntelliSenseItemImages);

			ListView->View = View::Details;
			ListView->Dock = DockStyle::Fill;
			ListView->MultiSelect = false;
			ListView->SmallImageList = IntelliSenseItemImages;
			ListView->Location = Point(0, 0);
			ListView->LabelEdit = false;
			ListView->CheckBoxes = false;
			ListView->FullRowSelect = true;
			ListView->GridLines = false;
			ListView->HeaderStyle = ColumnHeaderStyle::None;
			ListView->HideSelection = true;
			ListView->Font = gcnew Font(SystemFonts::DialogFont->FontFamily, 9.25);

			ListViewDefaultColumn = gcnew BrightIdeasSoftware::OLVColumn;
			ListViewDefaultColumn->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&IntelliSenseInterfaceView::ListViewAspectGetter);
			ListViewDefaultColumn->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&IntelliSenseInterfaceView::ListViewImageGetter);
			ListView->AllColumns->Add(ListViewDefaultColumn);
			ListView->Columns->Add(ListViewDefaultColumn);

			ListViewPopup = gcnew DotNetBar::SuperTooltip;
			ListViewPopup->CheckTooltipPosition = false;
			ListViewPopup->DelayTooltipHideDuration = 9999999;
			ListViewPopup->TooltipDuration = 0;
			ListViewPopup->IgnoreFormActiveState = true;
			ListViewPopup->ShowTooltipImmediately = true;
			ListViewPopup->PositionBelowControl = false;
			ListViewPopup->HoverDelayMultiplier = 0;
			ListViewPopup->DefaultFont = gcnew Font(SystemFonts::DialogFont->FontFamily, 9.25);
			ListViewPopup->MinimumTooltipSize = Size(180, 25);
			ListViewPopup->MarkupLinkClick += gcnew DotNetBar::MarkupLinkClickEventHandler(&IntelliSenseInterfaceView::SuperTooltip_MarkupLinkClick);

			InsightPopup = gcnew DotNetBar::SuperTooltip;
			InsightPopup->DelayTooltipHideDuration = 500;
			InsightPopup->TooltipDuration = 0;
			InsightPopup->CheckTooltipPosition = false;
			InsightPopup->DefaultFont = gcnew Font(SystemFonts::DialogFont->FontFamily, 9.25);
			InsightPopup->MinimumTooltipSize = Size(180, 25);
			InsightPopup->MarkupLinkClick += gcnew DotNetBar::MarkupLinkClickEventHandler(&IntelliSenseInterfaceView::SuperTooltip_MarkupLinkClick);

			MaximumVisibleItemCount = preferences::SettingsHolder::Get()->IntelliSense->MaxSuggestionsToDisplay;
			InsightPopupDisplayDuration = preferences::SettingsHolder::Get()->IntelliSense->InsightToolTipDisplayDuration;
			WindowWidth = preferences::SettingsHolder::Get()->IntelliSense->WindowWidth;

			Form->SetSize(Size(0, 0));
			Form->Show(Point(0, 0), Form->Handle, false);
			Form->Hide(false);
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
			SAFEDELETE_CLR(SelectFirstItemOnShowHandler);

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
			WindowWidth = preferences::SettingsHolder::Get()->IntelliSense->WindowWidth;
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

			auto Item = safe_cast<IntelliSenseItem^>(E->Model);
			switch (Item->GetItemType())
			{
			case IntelliSenseItem::ItemType::ScriptVariable:
				if (BoundModel->IsLocalVariable(Item->GetIdentifier()))
				{
					E->Item->ForeColor = preferences::SettingsHolder::Get()->Appearance->ForeColorLocalVariables;
					break;
				}
			default:
				E->Item->ForeColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
			}
			E->Item->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;

			if (E->Model == E->ListView->SelectedObject)
			{
				auto Temp = E->Item->ForeColor;
				E->Item->ForeColor = E->Item->BackColor;
				E->Item->BackColor = Temp;

				E->Item->SelectedForeColor = E->Item->ForeColor;
				E->Item->SelectedBackColor = E->Item->BackColor;
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

		void IntelliSenseInterfaceView::SuperTooltip_MarkupLinkClick(Object^ Sender, DotNetBar::MarkupLinkClickEventArgs^ E)
		{
			Process::Start(E->HRef);
		}

		void IntelliSenseInterfaceView::ShowListViewToolTip(IntelliSenseItem^ Item)
		{
			auto TooltipData = gcnew DotNetBar::SuperTooltipInfo;
			TooltipData->HeaderText = Item->TooltipHeaderText;
			TooltipData->BodyText = Item->TooltipBodyText;
			TooltipData->FooterText = Item->TooltipFooterText;
			TooltipData->FooterImage = Item->TooltipFooterImage;
			TooltipData->Color = MapRichTooltipBackgroundColorToDotNetBar(Item->TooltipBgColor);

			auto DesktopLocation = Point(Form->DesktopBounds.Left + Form->DesktopBounds.Width, Form->DesktopBounds.Top);

			if (ListViewPopup->IsTooltipVisible)
			{
				ListViewPopup->SuperTooltipControl->UpdateWithSuperTooltipInfo(TooltipData);
				ListViewPopup->SuperTooltipControl->RecalcSize();
				ListViewPopup->SuperTooltipControl->UpdateShadow();

				auto Height = ListViewPopup->SuperTooltipControl->Bounds.Height;
				int CoordY = DesktopLocation.Y;
				if (CoordY + Height > Screen::PrimaryScreen->Bounds.Height)
				{
					CoordY += Screen::PrimaryScreen->Bounds.Height - CoordY - Height;
					if (CoordY < 0)
						CoordY = 0;
				}

				ListViewPopup->SuperTooltipControl->SetBounds(DesktopLocation.X, CoordY, 0, 0, System::Windows::Forms::BoundsSpecified::Location);
			}
			else
			{
				ListViewPopup->SetSuperTooltip(Form, TooltipData);
				ListViewPopup->ShowTooltip(Form, DesktopLocation);
			}
		}

		void IntelliSenseInterfaceView::HideListViewToolTip()
		{
			ListViewPopup->HideTooltip();
		}

		void IntelliSenseInterfaceView::SelectFirstItemOnShow(AnimatedForm^ Sender)
		{
			if (BoundModel->DataStore->Count)
			{
				auto DefaultSelection = BoundModel->DataStore[0];
				ListView->SelectObject(DefaultSelection);

				// The SelectionChanged event doesn't get raised consistently at this point
				// So, we ensure that the tooltip is shown
				ShowListViewToolTip(DefaultSelection);
			}
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
				ListView->ClearObjects();

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
			if (!Visible || Form->IsFadingIn)
				return;

			Form->Opacity = DimmedOpacity;
		}

		void IntelliSenseInterfaceView::ResetOpacity()
		{
			if (!Visible || Form->IsFadingIn)
				return;

			Form->Opacity = 1.f;
		}

		void IntelliSenseInterfaceView::ShowInsightToolTip(IntelliSenseShowInsightToolTipArgs^ Args)
		{
			auto TooltipData = gcnew DotNetBar::SuperTooltipInfo;
			TooltipData->HeaderText = Args->TooltipHeaderText;
			TooltipData->BodyText = Args->TooltipBodyText;
			TooltipData->BodyImage = Args->TooltipBodyImage;
			TooltipData->FooterText = Args->TooltipFooterText;
			TooltipData->FooterImage = Args->TooltipFooterImage;
			TooltipData->Color = MapRichTooltipBackgroundColorToDotNetBar(Args->TooltipBgColor);

			auto Control = Control::FromHandle(Args->ParentWindowHandle);
			InsightPopup->SetSuperTooltip(Control, TooltipData);
			InsightPopup->ShowTooltip(Control, Args->DisplayScreenCoords);
			InsightPopup->TooltipDuration = InsightPopupDisplayDuration * 1000;
		}

		void IntelliSenseInterfaceView::HideInsightToolTip()
		{
			InsightPopup->HideTooltip();
		}

		void IntelliSenseInterfaceView::Update()
		{
			Debug::Assert(Bound == true);

			ListView->SetObjects(BoundModel->DataStore);
			ListView->DeselectAll();

			if (BoundModel->DataStore->Count)
			{
				int ItemCount = BoundModel->DataStore->Count;
				if (ItemCount > MaximumVisibleItemCount)
					ItemCount = MaximumVisibleItemCount;
				int ItemHeight = ListView->GetItemRect(0).Height;

				auto MaxHeight = MaximumVisibleItemCount * ItemHeight + ItemHeight;
				auto ExtraHeight = (MaximumVisibleItemCount - ItemCount) * ItemHeight;
				Size DisplaySize = Size(WindowWidth, MaxHeight - ExtraHeight);

				// ### HACK!
				// The SetSize call crashes consistently under certain conditions (which are yet to be decoded) due to an invalid window handle
				// Might have something to do with how the call is invoked (multiple levels of interop b'ween WinForms and WPF)
				try {

					Form->SetSize(DisplaySize);

					// the column width needs to be (re)set after the form (and its docked listview) have been resized
					ListViewDefaultColumn->Width = ListView->Width - 4 - SystemInformation::HorizontalScrollBarHeight;
				}
				catch (Exception^ E) {
					DebugPrint("IntelliSenseInterfaceView::Update Exception! Message - " + E->Message);
#ifndef NDEBUG
					Debugger::Break();
#endif // !NDEBUG
				}
			}
		}

		void IntelliSenseInterfaceView::Show(Point Location, IntPtr Parent)
		{
			try
			{
				auto PopupHidden = !Form->Visible;

				if (PopupHidden)
					Form->SetNextActiveTransitionCompleteHandler(SelectFirstItemOnShowHandler);

				Form->Show(Location, Parent, PopupHidden);

				if (!PopupHidden)
					SelectFirstItemOnShow(Form);
			}
			catch (Exception^ E) {
				DebugPrint("IntelliSenseInterfaceView::Show Exception! Message - " + E->Message);
#ifndef NDEBUG
				Debugger::Break();
#endif // !NDEBUG
			}
		}

		void IntelliSenseInterfaceView::Hide()
		{
			HideListViewToolTip();

			if (!Form->Visible)
				return;

			try {
				Form->Hide();
			}
			catch (Exception^ E) {
				DebugPrint("IntelliSenseInterfaceView::Hide Exception! Message - " + E->Message);
#ifndef NDEBUG
				Debugger::Break();
#endif // !NDEBUG
			}
		}
	}
}