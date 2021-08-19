#include "IntelliSenseInterfaceView.h"
#include "IntelliSenseItem.h"
#include "Preferences.h"
#include "Utilities.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


IntelliSenseInterfaceView::IntelliSenseInterfaceView(IntPtr ParentViewHandle)
{
	this->BoundModel = nullptr;
	this->ParentViewHandle = ParentViewHandle;

	Form = gcnew utilities::AnimatedForm(true);
	ColorManager = gcnew DotNetBar::StyleManagerAmbient;

	ListView = gcnew BrightIdeasSoftware::FastObjectListView;

	ListViewSelectionChangedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_SelectionChanged);
	ListViewKeyDownHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyDown);
	ListViewKeyUpHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyUp);
	ListViewItemActivateHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_ItemActivate);
	ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ScriptEditorPreferences_Saved);
	ListViewFormatRowHandler = gcnew EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>(this, &IntelliSenseInterfaceView::ListView_FormatRow);
	SelectFirstItemOnShowHandler = gcnew utilities::AnimatedForm::TransitionCompleteHandler(this, &IntelliSenseInterfaceView::SelectFirstItemOnShow);

	ListView->KeyDown += ListViewKeyDownHandler;
	ListView->KeyUp += ListViewKeyUpHandler;
	ListView->ItemActivate += ListViewItemActivateHandler;
	ListView->SelectedIndexChanged += ListViewSelectionChangedHandler;
	ListView->FormatRow += ListViewFormatRowHandler;
	preferences::SettingsHolder::Get()->PreferencesChanged += ScriptEditorPreferencesSavedHandler;

	Form->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	Form->AutoScaleMode = AutoScaleMode::Font;
	Form->FormBorderStyle = FormBorderStyle::SizableToolWindow;
	Form->ShowInTaskbar = false;
	Form->ShowIcon = false;
	Form->ControlBox = false;
	Form->Controls->Add(ListView);
	Form->Margin = Padding(0);
	Form->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;

	ListView->View = View::Details;
	ListView->Dock = DockStyle::Fill;
	ListView->MultiSelect = false;
	ListView->Location = Point(0, 0);
	ListView->LabelEdit = false;
	ListView->CheckBoxes = false;
	ListView->FullRowSelect = true;
	ListView->GridLines = false;
	ListView->HeaderStyle = ColumnHeaderStyle::None;
	ListView->HideSelection = true;
	ListView->Font = preferences::SettingsHolder::Get()->Appearance->TextFont;
	ListView->Margin = Padding(0);
	ListView->ForeColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	ListView->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
	ColorManager->SetEnableAmbientSettings(ListView, DotNetBar::eAmbientSettings::None);

	ListViewDefaultColumn = gcnew BrightIdeasSoftware::OLVColumn;
	ListViewDefaultColumn->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&IntelliSenseInterfaceView::ListViewAspectGetter);
	ListViewDefaultColumn->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&IntelliSenseInterfaceView::ListViewImageGetter);
	ListViewDefaultColumn->FillsFreeSpace = true;
	ListViewDefaultColumn->Width = 100;
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
	ListViewPopup->MaximumWidth = Screen::PrimaryScreen->WorkingArea.Width - 150;
	ListViewPopup->MarkupLinkClick += gcnew DotNetBar::MarkupLinkClickEventHandler(&IntelliSenseInterfaceView::SuperTooltip_MarkupLinkClick);

	InsightPopup = gcnew DotNetBar::SuperTooltip;
	InsightPopup->DelayTooltipHideDuration = 500;
	InsightPopup->TooltipDuration = 0;
	InsightPopup->CheckTooltipPosition = false;
	InsightPopup->DefaultFont = gcnew Font(SystemFonts::DialogFont->FontFamily, 9.25);
	InsightPopup->MinimumTooltipSize = Size(180, 25);
	InsightPopup->MaximumWidth = Screen::PrimaryScreen->WorkingArea.Width - 150;
	InsightPopup->MarkupLinkClick += gcnew DotNetBar::MarkupLinkClickEventHandler(&IntelliSenseInterfaceView::SuperTooltip_MarkupLinkClick);

	TooltipColorSwapper = gcnew utilities::SuperTooltipColorSwapper;
	//TooltipColorSwapper = gcnew utilities::SuperTooltipColorSwapper(preferences::SettingsHolder::Get()->Appearance->ForeColor,
																	//preferences::SettingsHolder::Get()->Appearance->BackColor);

	MaximumVisibleItemCount = preferences::SettingsHolder::Get()->IntelliSense->MaxVisiblePopupItems;
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
	preferences::SettingsHolder::Get()->PreferencesChanged -= ScriptEditorPreferencesSavedHandler;

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
	MaximumVisibleItemCount = preferences::SettingsHolder::Get()->IntelliSense->MaxVisiblePopupItems;
	InsightPopupDisplayDuration = preferences::SettingsHolder::Get()->IntelliSense->InsightToolTipDisplayDuration;
	WindowWidth = preferences::SettingsHolder::Get()->IntelliSense->WindowWidth;
	Form->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
	ListView->ForeColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	ListView->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
	ListView->Font = preferences::SettingsHolder::Get()->Appearance->TextFont;

	//TooltipColorSwapper->TextColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	//TooltipColorSwapper->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
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
	case IntelliSenseItem::eItemType::ScriptVariable:
		if (BoundModel->IsLocalVariable(Item->GetIdentifier()))
		{
			auto ForeColor = preferences::SettingsHolder::Get()->Appearance->ForeColorLocalVariables;
			E->Item->ForeColor = ForeColor;
			break;
		}
	default:
	{
		auto ForeColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
		E->Item->ForeColor = ForeColor;
	}
	}
	auto BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
	E->Item->BackColor = BackColor;

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
	if (RowObject == nullptr)
		return nullptr;

	return safe_cast<IntelliSenseItem^>(RowObject)->TooltipFooterImage;
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

	auto DesktopLocation = Point(Form->DesktopBounds.Left + Form->DesktopBounds.Width + 2, Form->DesktopBounds.Top);

	if (ListViewPopup->IsTooltipVisible)
	{
		TooltipColorSwapper->UpdateWithSuperTooltipInfo(ListViewPopup, TooltipData, true);

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
		TooltipColorSwapper->ShowTooltip(ListViewPopup, Form, DesktopLocation);
	}
}

void IntelliSenseInterfaceView::HideListViewToolTip()
{
	ListViewPopup->HideTooltip();
}

void IntelliSenseInterfaceView::SelectFirstItemOnShow(utilities::AnimatedForm^ Sender)
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

void IntelliSenseInterfaceView::ChangeSelection(IIntelliSenseInterfaceView::eMoveDirection Direction)
{
	if (!Visible)
		return;

	int SelectedIndex = ListView->SelectedIndex;
	if (SelectedIndex == -1)
		return;

	switch (Direction)
	{
	case IIntelliSenseInterfaceView::eMoveDirection::Down:
		if (SelectedIndex < ListView->GetItemCount() - 1)
			++SelectedIndex;

		break;
	case IIntelliSenseInterfaceView::eMoveDirection::Up:
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

	Form->Opacity = kDimmedOpacity;
	HideListViewToolTip();
}

void IntelliSenseInterfaceView::ResetOpacity()
{
	if (!Visible || Form->IsFadingIn)
		return;

	Form->Opacity = 1.f;

	auto CurrentSelection = safe_cast<IntelliSenseItem^>(ListView->SelectedObject);
	if (CurrentSelection)
		ShowListViewToolTip(CurrentSelection);
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
	InsightPopup->TooltipDuration = InsightPopupDisplayDuration * 1000;

	TooltipColorSwapper->ShowTooltip(InsightPopup, Control, Args->DisplayScreenCoords);
}

void IntelliSenseInterfaceView::HideInsightToolTip()
{
	// only hide if the cursor is outside bounds of the tooltip
	// in this case, the tooltip will close itself after the mouse leaves its bounds
	if (InsightPopup->IsTooltipVisible && InsightPopup->SuperTooltipControl->Bounds.Contains(Control::MousePosition))
		return;

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

		Form->SetSize(DisplaySize);
		// the column width needs to be (re)set after the form (and its docked listview) have been resized
		ListViewDefaultColumn->Width = ListView->Width - 4 - (BoundModel->DataStore->Count > MaximumVisibleItemCount ? SystemInformation::HorizontalScrollBarHeight : 0);
	}
}

void IntelliSenseInterfaceView::Show(Drawing::Point Location)
{
	auto PopupHidden = !Form->Visible;

	if (PopupHidden)
		Form->SetNextActiveTransitionCompleteHandler(SelectFirstItemOnShowHandler);

	Form->Show(Location, ParentViewHandle, PopupHidden);

	if (!PopupHidden)
		SelectFirstItemOnShow(Form);
}

void IntelliSenseInterfaceView::Hide()
{
	HideListViewToolTip();

	if (!Form->Visible)
		return;

	Form->Hide(true);
}


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse