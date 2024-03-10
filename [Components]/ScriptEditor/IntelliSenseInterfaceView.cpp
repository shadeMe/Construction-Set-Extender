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
	auto ImageResources = view::components::CommonIcons::Get()->ResourceManager;

	this->BoundModel = nullptr;
	this->ParentViewHandle = ParentViewHandle;

	Form = gcnew utilities::AnimatedForm(true);
	ColorManager = gcnew DotNetBar::StyleManagerAmbient;

	ListView = gcnew BrightIdeasSoftware::FastObjectListView;
	BottomToolbar = gcnew DevComponents::DotNetBar::Bar;
	ToolbarFilterScriptCommand = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterScriptVariable = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterQuest = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterScript = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterUserFunction = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterGameSetting = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterGlobalVariable = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterForm = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFilterObjectReference = gcnew DevComponents::DotNetBar::ButtonItem;
	ToolbarFuzzySearch = gcnew DevComponents::DotNetBar::ButtonItem;

	ListViewSelectionChangedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_SelectionChanged);
	ListViewKeyDownHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyDown);
	ListViewKeyUpHandler = gcnew KeyEventHandler(this, &IntelliSenseInterfaceView::ListView_KeyUp);
	ListViewItemActivateHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ListView_ItemActivate);
	ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ScriptEditorPreferences_Saved);
	ListViewFormatRowHandler = gcnew EventHandler<BrightIdeasSoftware::FormatRowEventArgs^>(this, &IntelliSenseInterfaceView::ListView_FormatRow);
	SelectFirstItemOnShowHandler = gcnew utilities::AnimatedForm::TransitionCompleteHandler(this, &IntelliSenseInterfaceView::SelectFirstItemOnShow);
	ToolbarFilterClickHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ToolbarFilter_Click);
	ToolbarFuzzySearchClickHandler = gcnew EventHandler(this, &IntelliSenseInterfaceView::ToolbarFuzzySearch_Click);
	FilterPredicate = gcnew BrightIdeasSoftware::ModelFilter(gcnew Predicate<Object^>(this, &IntelliSenseInterfaceView::ListViewFilterPredicate));

	ListView->KeyDown += ListViewKeyDownHandler;
	ListView->KeyUp += ListViewKeyUpHandler;
	ListView->ItemActivate += ListViewItemActivateHandler;
	ListView->SelectedIndexChanged += ListViewSelectionChangedHandler;
	ListView->FormatRow += ListViewFormatRowHandler;
	preferences::SettingsHolder::Get()->PreferencesChanged += ScriptEditorPreferencesSavedHandler;

	ToolbarFilterScriptCommand->ImagePaddingHorizontal = 10;
	ToolbarFilterScriptCommand->ImagePaddingVertical = 15;
	ToolbarFilterScriptCommand->Tooltip = L"Script Commands";
	ToolbarFilterScriptCommand->Click += ToolbarFilterClickHandler;
	ToolbarFilterScriptCommand->Tag = IIntelliSenseInterfaceView::eItemFilter::ScriptCommand;
	ToolbarFilterScriptCommand->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::ScriptCommand);

	ToolbarFilterScriptVariable->BeginGroup = true;
	ToolbarFilterScriptVariable->ImagePaddingHorizontal = 10;
	ToolbarFilterScriptVariable->ImagePaddingVertical = 15;
	ToolbarFilterScriptVariable->Tooltip = L"Script Variables";
	ToolbarFilterScriptVariable->Click += ToolbarFilterClickHandler;
	ToolbarFilterScriptVariable->Tag = IIntelliSenseInterfaceView::eItemFilter::ScriptVariable;
	ToolbarFilterScriptVariable->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::ScriptVariable);

	ToolbarFilterQuest->ImagePaddingHorizontal = 10;
	ToolbarFilterQuest->ImagePaddingVertical = 15;
	ToolbarFilterQuest->Tooltip = L"Quests";
	ToolbarFilterQuest->Click += ToolbarFilterClickHandler;
	ToolbarFilterQuest->Tag = IIntelliSenseInterfaceView::eItemFilter::Quest;  
	ToolbarFilterQuest->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::Quest);

	ToolbarFilterScript->ImagePaddingHorizontal = 10;
	ToolbarFilterScript->ImagePaddingVertical = 15;
	ToolbarFilterScript->Tooltip = L"Scripts";
	ToolbarFilterScript->Click += ToolbarFilterClickHandler;
	ToolbarFilterScript->Tag = IIntelliSenseInterfaceView::eItemFilter::Script;
	ToolbarFilterScript->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::Script);

	ToolbarFilterUserFunction->ImagePaddingHorizontal = 10;
	ToolbarFilterUserFunction->ImagePaddingVertical = 15;
	ToolbarFilterUserFunction->Tooltip = L"User-Defined Functions";
	ToolbarFilterUserFunction->Click += ToolbarFilterClickHandler;
	ToolbarFilterUserFunction->Tag = IIntelliSenseInterfaceView::eItemFilter::UserFunction;
	ToolbarFilterUserFunction->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::UserFunction);

	ToolbarFilterGameSetting->ImagePaddingHorizontal = 10;
	ToolbarFilterGameSetting->ImagePaddingVertical = 15;
	ToolbarFilterGameSetting->Tooltip = L"Game Settings";
	ToolbarFilterGameSetting->Click += ToolbarFilterClickHandler;
	ToolbarFilterGameSetting->Tag = IIntelliSenseInterfaceView::eItemFilter::GameSetting;
	ToolbarFilterGameSetting->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::GameSetting);

	ToolbarFilterGlobalVariable->ImagePaddingHorizontal = 10;
	ToolbarFilterGlobalVariable->ImagePaddingVertical = 15;
	ToolbarFilterGlobalVariable->Tooltip = L"Global Variables";
	ToolbarFilterGlobalVariable->Click += ToolbarFilterClickHandler;
	ToolbarFilterGlobalVariable->Tag = IIntelliSenseInterfaceView::eItemFilter::GlobalVariable;
	ToolbarFilterGlobalVariable->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::GlobalVariable);

	ToolbarFilterForm->ImagePaddingHorizontal = 10;
	ToolbarFilterForm->ImagePaddingVertical = 15;
	ToolbarFilterForm->Tooltip = L"Forms";
	ToolbarFilterForm->Click += ToolbarFilterClickHandler;
	ToolbarFilterForm->Tag = IIntelliSenseInterfaceView::eItemFilter::Form;
	ToolbarFilterForm->Image = IntelliSenseItem::GetItemTypeIcon(IntelliSenseItem::eItemType::Form);

	ToolbarFilterObjectReference->ImagePaddingHorizontal = 10;
	ToolbarFilterObjectReference->ImagePaddingVertical = 15;
	ToolbarFilterObjectReference->Tooltip = L"Object References";
	ToolbarFilterObjectReference->Click += ToolbarFilterClickHandler;
	ToolbarFilterObjectReference->Tag = IIntelliSenseInterfaceView::eItemFilter::ObjectReference;
	ToolbarFilterObjectReference->Image = IntelliSenseItemForm::GetFormTypeIcon(IntelliSenseItemForm::eFormType::REFR);

	ToolbarFuzzySearch->ImagePaddingHorizontal = 15;
	ToolbarFuzzySearch->ImagePaddingVertical = 15;
	ToolbarFuzzySearch->Tooltip = L"Fuzzy Matching (Ctrl + U)";
	ToolbarFuzzySearch->Click += ToolbarFuzzySearchClickHandler;
	ToolbarFuzzySearch->Image = ImageResources->CreateImage("FuzzySearch");

	ToolbarFilterShortcutKeys = gcnew List<Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>^>;
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterScriptVariable, Keys::D1));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterScriptCommand, Keys::D2));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterForm, Keys::D3));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterObjectReference, Keys::D4));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterUserFunction, Keys::D5));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterGlobalVariable, Keys::D6));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterQuest, Keys::D7));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterScript, Keys::D8));
	ToolbarFilterShortcutKeys->Add(gcnew Tuple<DevComponents::DotNetBar::ButtonItem^, Keys>(ToolbarFilterGameSetting, Keys::D9));

	BottomToolbar->AntiAlias = true;
	BottomToolbar->Dock = System::Windows::Forms::DockStyle::Bottom;
	BottomToolbar->DockSide = DevComponents::DotNetBar::eDockSide::Bottom;
	BottomToolbar->IsMaximized = false;
	BottomToolbar->Stretch = true;
	BottomToolbar->Style = DevComponents::DotNetBar::eDotNetBarStyle::StyleManagerControlled;
	BottomToolbar->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;

	BottomToolbar->Items->Add(ToolbarFuzzySearch);
	for each (auto Button in ToolbarFilterShortcutKeys)
	{
		Button->Item1->Tooltip = Button->Item1->Tooltip + String::Format(" (Ctrl + {0})", Button->Item2.ToString());
		BottomToolbar->Items->Add(Button->Item1);
	}

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
	ListView->Font = preferences::SettingsHolder::Get()->Appearance->TextEditorFont;
	ListView->Margin = Padding(0);
	ListView->ForeColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	ListView->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
	ListView->ModelFilter = FilterPredicate;
	ListView->EmptyListMsg = L"No suggestions for the current filters";
	ListView->EmptyListMsgFont = (gcnew System::Drawing::Font(L"Segoe UI caps", 9.75F, System::Drawing::FontStyle::Regular,
		System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
	auto EmptyMsgOverlay = safe_cast<BrightIdeasSoftware::TextOverlay^>(ListView->EmptyListMsgOverlay);
	EmptyMsgOverlay->TextColor = Color::White;
	EmptyMsgOverlay->BackColor = Color::FromArgb(75, 29, 32, 33);
	ListView->ShowFilterMenuOnRightClick = false;
	ListView->UseFiltering = true;
	ListView->VirtualMode = true;
	ColorManager->SetEnableAmbientSettings(ListView, DotNetBar::eAmbientSettings::None);

	ListViewDefaultColumn = gcnew BrightIdeasSoftware::OLVColumn;
	ListViewDefaultColumn->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&IntelliSenseInterfaceView::ListViewAspectGetter);
	ListViewDefaultColumn->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&IntelliSenseInterfaceView::ListViewImageGetter);
	ListViewDefaultColumn->FillsFreeSpace = true;
	ListViewDefaultColumn->Width = preferences::SettingsHolder::Get()->IntelliSense->WindowWidth - 10;
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

	ListViewItemRectCache = Rectangle();
	MaximumVisibleItemCount = preferences::SettingsHolder::Get()->IntelliSense->MaxVisiblePopupItems;
	InsightPopupDisplayDuration = preferences::SettingsHolder::Get()->IntelliSense->InsightToolTipDisplayDuration;
	WindowWidth = preferences::SettingsHolder::Get()->IntelliSense->WindowWidth;
	Filters = IIntelliSenseInterfaceView::eItemFilter::None;

	/*Form->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
	Form->AutoScaleMode = AutoScaleMode::Font;*/
	utilities::DisableFormAutoScale(Form);
	Form->FormBorderStyle = FormBorderStyle::SizableToolWindow;
	Form->ShowInTaskbar = false;
	Form->ShowIcon = false;
	Form->ControlBox = false;
	Form->Controls->Add(ListView);
	Form->Controls->Add(BottomToolbar);
	Form->Margin = Padding(0);
	Form->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;

	Form->SetSize(Size(0, 0));
	Form->Show(Point(0, 0), Form->Handle, false);
	Form->Hide(false);
}

IntelliSenseInterfaceView::~IntelliSenseInterfaceView()
{
	Debug::Assert(Bound == false);

	ToolbarFilterScriptCommand->Click -= ToolbarFilterClickHandler;
	ToolbarFilterScriptVariable->Click -= ToolbarFilterClickHandler;
	ToolbarFilterQuest->Click -= ToolbarFilterClickHandler;
	ToolbarFilterScript->Click -= ToolbarFilterClickHandler;
	ToolbarFilterUserFunction->Click -= ToolbarFilterClickHandler;
	ToolbarFilterGameSetting->Click -= ToolbarFilterClickHandler;
	ToolbarFilterGlobalVariable->Click -= ToolbarFilterClickHandler;
	ToolbarFilterForm->Click -= ToolbarFilterClickHandler;
	ToolbarFilterObjectReference->Click -= ToolbarFilterClickHandler;
	ToolbarFuzzySearch->Click -= ToolbarFilterClickHandler;

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
	SAFEDELETE_CLR(ToolbarFilterClickHandler);
	SAFEDELETE_CLR(ToolbarFuzzySearchClickHandler);
	FilterPredicate = nullptr;

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
	ListView->Font = preferences::SettingsHolder::Get()->Appearance->TextEditorFont;
	BottomToolbar->BackColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
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

void IntelliSenseInterfaceView::ToolbarFilter_Click(Object^ Sender, EventArgs^ E)
{
	auto Button = safe_cast<DevComponents::DotNetBar::ButtonItem^>(Sender);
	auto SelectedFilter = safe_cast<IIntelliSenseInterfaceView::eItemFilter>(Button->Tag);
	ToggleFilter(SelectedFilter);
}

void IntelliSenseInterfaceView::ToolbarFuzzySearch_Click(Object^ Sender, EventArgs^ E)
{
	FuzzySearchToggled(this, E);
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

bool IntelliSenseInterfaceView::ListViewFilterPredicate(Object^ Model)
{
    if (Filters == IIntelliSenseInterfaceView::eItemFilter::None)
        return true;

    auto Item = safe_cast<IntelliSenseItem^>(Model);
    switch (Item->GetItemType())
    {
        case IntelliSenseItem::eItemType::ScriptCommand:
            return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::ScriptCommand);
        case IntelliSenseItem::eItemType::ScriptVariable:
            return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::ScriptVariable);
        case IntelliSenseItem::eItemType::Quest:
            return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::Quest);
        case IntelliSenseItem::eItemType::Script:
            return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::Script);
        case IntelliSenseItem::eItemType::UserFunction:
            return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::UserFunction);
        case IntelliSenseItem::eItemType::GameSetting:
            return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::GameSetting);
        case IntelliSenseItem::eItemType::GlobalVariable:
            return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::GlobalVariable);
        case IntelliSenseItem::eItemType::Form:
		{
			auto FormItem = safe_cast<IntelliSenseItemForm^>(Item);
			if (FormItem->IsObjectReference())
				return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::ObjectReference);
			else
				return Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::Form);
		}
        default:
            return true;
    }
}

void IntelliSenseInterfaceView::UpdateToolbarState()
{
	ToolbarFilterScriptCommand->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::ScriptCommand);
	ToolbarFilterScriptVariable->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::ScriptVariable);
	ToolbarFilterQuest->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::Quest);
	ToolbarFilterScript->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::Script);
	ToolbarFilterUserFunction->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::UserFunction);
	ToolbarFilterGameSetting->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::GameSetting);
	ToolbarFilterGlobalVariable->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::GlobalVariable);
	ToolbarFilterForm->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::Form);
	ToolbarFilterObjectReference->Checked = Filters.HasFlag(IIntelliSenseInterfaceView::eItemFilter::ObjectReference);

	ToolbarFuzzySearch->Checked = preferences::SettingsHolder::Get()->IntelliSense->SuggestionsFilter == eFilterMode::Fuzzy;
}

void IntelliSenseInterfaceView::ToggleFilter(IIntelliSenseInterfaceView::eItemFilter SelectedFilter)
{
	if (SelectedFilter == IIntelliSenseInterfaceView::eItemFilter::None)
		Filters = SelectedFilter;
	else
		Filters = Filters ^ SelectedFilter;

	Form->SuspendLayout();
	{
		ListView->UseFiltering = false;
		ListView->UseFiltering = true;
		UpdateToolbarState();
	}
	Form->ResumeLayout(true);
}

void IntelliSenseInterfaceView::ShowListViewToolTip(IntelliSenseItem^ Item)
{
	// Ignore if the all items have been filtered away.
	if (ListView->VirtualListSize == 0)
		return;

	auto TooltipData = gcnew DotNetBar::SuperTooltipInfo;
	TooltipData->HeaderText = Item->TooltipHeaderText;
	TooltipData->BodyText = Item->TooltipBodyText;
	TooltipData->FooterText = Item->TooltipFooterText;
	TooltipData->FooterImage = Item->TooltipFooterImage;
	TooltipData->Color = DevComponents::DotNetBar::eTooltipColor::System;

	auto DesktopLocation = Point(Form->DesktopBounds.Left + Form->DesktopBounds.Width + 2, Form->DesktopBounds.Top);
	auto TooltipColorSwapper = gcnew utilities::SuperTooltipColorSwapper(Item->TooltipTextColor, Item->TooltipBgColor);

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
		ListView->EnsureModelVisible(DefaultSelection);

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

void IntelliSenseInterfaceView::ResetFilters()
{
	Filters = IIntelliSenseInterfaceView::eItemFilter::None;
	UpdateToolbarState();
}

void IntelliSenseInterfaceView::HandleFilterShortcutKey(Keys ShortcutKey)
{
	if (ShortcutKey == Keys::D0)
	{
		ToggleFilter(IIntelliSenseInterfaceView::eItemFilter::None);
		return;
	}
	
	for each (auto Button in ToolbarFilterShortcutKeys)
	{
		if (ShortcutKey == Button->Item2)
		{
			Button->Item1->RaiseClick();
			return;
		}
	}

	throw gcnew ArgumentException("Unexpected IntelliSense filter shortcut key");
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
			ListView->DeselectAll();
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
	TooltipData->Color = DevComponents::DotNetBar::eTooltipColor::System;

	auto Control = Control::FromHandle(Args->ParentWindowHandle);
	InsightPopup->SetSuperTooltip(Control, TooltipData);
	InsightPopup->TooltipDuration = InsightPopupDisplayDuration * 1000;

	auto TooltipColorSwapper = gcnew utilities::SuperTooltipColorSwapper(Args->TooltipTextColor, Args->TooltipBgColor);
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

	Form->SuspendLayout();
	{
		ListView->SetObjects(BoundModel->DataStore);
		ListView->DeselectAll();
		UpdateToolbarState();

		if (BoundModel->DataStore->Count)
		{
			int ItemCount = BoundModel->DataStore->Count;
			if (ItemCount > MaximumVisibleItemCount)
				ItemCount = MaximumVisibleItemCount;

			if (ListView->VirtualListSize > 0)
				ListViewItemRectCache = ListView->GetItemRect(0);

			int ItemHeight = ListViewItemRectCache.Height;
			auto MaxHeight = MaximumVisibleItemCount * ItemHeight + ItemHeight;
			auto ExtraHeight = (MaximumVisibleItemCount - ItemCount) * ItemHeight;
			Size DisplaySize = Size(WindowWidth, MaxHeight - ExtraHeight + BottomToolbar->Height);

			Form->SetSize(DisplaySize);
			// the column width needs to be (re)set after the form (and its docked listview) have been resized
			ListViewDefaultColumn->Width = ListView->Width - 4 - (BoundModel->DataStore->Count > MaximumVisibleItemCount ? SystemInformation::VerticalScrollBarWidth : 0);
		}
	}
	Form->ResumeLayout(true);
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