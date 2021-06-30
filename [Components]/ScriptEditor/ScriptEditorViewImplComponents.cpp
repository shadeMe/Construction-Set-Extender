#include "ScriptEditorViewImplComponents.h"

namespace cse
{


namespace scriptEditor
{


namespace viewImpl
{


namespace components
{


void ViewComponent::RaiseEvent(Object^ EventType, Object^ EventArgs)
{
	auto NewEvent = gcnew ViewComponentEvent;
	NewEvent->Component = this;
	NewEvent->EventType = EventType;
	NewEvent->EventArgs = EventArgs;

	EventRouter(NewEvent);
}

void ViewComponent::RaiseEvent(ViewComponent^ Component, Object^ EventType, Object^ EventArgs)
{
	Component->RaiseEvent(EventType, EventArgs);
}

ViewComponent::ViewComponent(eComponentType ComponentType, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: Type_(ComponentType), Role_(ViewRole), EventRouter(EventRouter)
{
}

ViewComponent::~ViewComponent()
{
	EventRouter = nullptr;
}

IForm^ ViewComponent::AsForm()
{
	if (Type_ != eComponentType::Form)
		return nullptr;

	return safe_cast<IForm^>(this);
}

IButton^ ViewComponent::AsButton()
{
	if (Type_ != eComponentType::Button)
		return nullptr;

	return safe_cast<IButton^>(this);
}

IComboBox^ ViewComponent::AsComboBox()
{
	if (Type_ != eComponentType::ComboBox)
		return nullptr;

	return safe_cast<IComboBox^>(this);
}

ILabel^ ViewComponent::AsLabel()
{
	if (Type_ != eComponentType::Label)
		return nullptr;

	return safe_cast<ILabel^>(this);
}

ITabStrip^ ViewComponent::AsTabStrip()
{
	if (Type_ != eComponentType::TabStrip)
		return nullptr;

	return safe_cast<ITabStrip^>(this);
}

ITabStripItem^ ViewComponent::AsTabStripItem()
{
	if (Type_ != eComponentType::TabStripItem)
		return nullptr;

	return safe_cast<ITabStripItem^>(this);
}

IObjectListView^ ViewComponent::AsObjectListView()
{
	if (Type_ != eComponentType::ObjectListView)
		return nullptr;

	return safe_cast<IObjectListView^>(this);
}

IObjectListViewColumn^ ViewComponent::AsObjectListViewColumn()
{
	if (Type_ != eComponentType::ObjectListViewColumn)
		return nullptr;

	return safe_cast<IObjectListViewColumn^>(this);
}

IProgressBar^ ViewComponent::AsProgressBar()
{
	if (Type_ != eComponentType::ProgressBar)
		return nullptr;

	return safe_cast<IProgressBar^>(this);
}

IDockablePane^ ViewComponent::AsDockablePane()
{
	if (Type_ != eComponentType::DockablePane)
		return nullptr;

	return safe_cast<IDockablePane^>(this);
}

ICrumbBar^ ViewComponent::AsCrumbBar()
{
	if (Type_ != eComponentType::CrumbBar)
		return nullptr;

	return safe_cast<ICrumbBar^>(this);
}

ICrumbBarItem^ ViewComponent::AsCrumbBarItem()
{
	if (Type_ != eComponentType::CrumbBarItem)
		return nullptr;

	return safe_cast<ICrumbBarItem^>(this);
}

IContainer^ ViewComponent::AsContainer()
{
	if (Type_ != eComponentType::Container)
		return nullptr;

	return safe_cast<IContainer^>(this);
}

ViewComponent^ ViewComponent::FromControl(Control^ Control)
{
	return safe_cast<ViewComponent^>(Control->Tag);
}

void Form::Handler_Closing(Object^ Sender, CancelEventArgs^ E)
{
	auto EventArgs = gcnew IForm::ClosingEventArgs;
	EventArgs->Cancel = false;

	RaiseEvent(IForm::eEvent::Closing, EventArgs);

	E->Cancel = EventArgs->Cancel;
}

void Form::Handler_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	auto EventArgs = gcnew IForm::KeyDownEventArgs;
	EventArgs->KeyEvent = E;

	RaiseEvent(IForm::eEvent::KeyDown, EventArgs);
}

Form::Form(Forms::Form^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Form, ViewRole, EventRouter)
{
	this->Source = Source;

	DelegateClosing = gcnew CancelEventHandler(this, &Form::Handler_Closing);
	DelegateKeyDown = gcnew KeyEventHandler(this, &Form::Handler_KeyDown);

	Source->Closing += DelegateClosing;
	Source->KeyDown += DelegateKeyDown;
}

Form::~Form()
{
	Source->Closing -= DelegateClosing;
	Source->KeyDown -= DelegateKeyDown;

	SAFEDELETE_CLR(DelegateClosing);
	SAFEDELETE_CLR(DelegateKeyDown);

	Source = nullptr;
}

Rectangle Form::Bounds::get()
{
	if (Source->WindowState == FormWindowState::Normal)
		return Source->Bounds;
	else
		return Source->RestoreBounds;
}

void Form::Bounds::set(Rectangle v)
{
	if (Source->WindowState == FormWindowState::Normal)
		Source->Bounds = v;
	else
		throw gcnew InvalidOperationException("Cannot set form bounds when maximised or minimised");
}

void Form::BeginUpdate()
{
	Source->SuspendLayout();
}

void Form::EndUpdate()
{
	Source->ResumeLayout(true);
}

void Form::Redraw()
{
	Source->Invalidate(true);
}

void Form::Focus()
{
	Source->Focus();
}

void Form::Close(bool Force)
{
	throw gcnew System::NotImplementedException();
}

void Button::Handler_Click(Object^ Sender, EventArgs^ E)
{
	auto EventArgs = gcnew IButton::ClickEventArgs;

	RaiseEvent(IButton::eEvent::Click, EventArgs);
}

void Button::Handler_PopupOpen(Object^ Sender, PopupOpenEventArgs^ E)
{
	// raise the first event for the button being opened
	// then, raise the same event for all of the button's subitems (if the popup was not canceled)
	auto EventArgs = gcnew IButton::PopupOpeningEventArgs;
	EventArgs->Cancel = false;
	RaiseEvent(IButton::eEvent::PopupOpening, EventArgs);
	if (EventArgs->Cancel)
	{
		E->Cancel = true;
		return;
	}

	for each (auto Itr in ButtonItem->SubItems)
	{
		auto Component = ViewComponent::FromControl(safe_cast<Control^>(Itr));
		RaiseEvent(Component, IButton::eEvent::PopupOpening, EventArgs);
	}
}

void Button::InitEventHandlers()
{
	DelegateClick = gcnew EventHandler(this, &Button::Handler_Click);
	DelegatePopupOpen = gcnew DotNetBarManager::PopupOpenEventHandler(this, &Button::Handler_PopupOpen);

	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->Click += DelegateClick;
		ButtonItem->PopupOpen += DelegatePopupOpen;
		break;
	case eSourceType::ButtonX:
		ButtonX->Click += DelegateClick;
		break;
	case eSourceType::CheckBoxX:
		CheckBoxX->Click += DelegateClick;
		break;
	}
}

void Button::DeinitEventHandlers()
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->Click -= DelegateClick;
		ButtonItem->PopupOpen -= DelegatePopupOpen;
		break;
	case eSourceType::ButtonX:
		ButtonX->Click -= DelegateClick;
		break;
	case eSourceType::CheckBoxX:
		CheckBoxX->Click -= DelegateClick;
		break;
	}

	SAFEDELETE_CLR(DelegateClick);
	SAFEDELETE_CLR(DelegatePopupOpen);
}

System::String^ Button::GetterText()
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		return ButtonItem->Text;
	case eSourceType::ButtonX:
		return ButtonX->Text;
	case eSourceType::CheckBoxX:
		return CheckBoxX->Text;
	}

	return nullptr;
}

System::String^ Button::GetterShortcutKey()
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		return ButtonItem->AlternateShortCutText;
	case eSourceType::ButtonX:
	case eSourceType::CheckBoxX:
		throw gcnew NotImplementedException();
	}

	return nullptr;
}

System::String^ Button::GetterTooltip()
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		return ButtonItem->Tooltip;
	case eSourceType::ButtonX:
		return ButtonX->Tooltip;
	case eSourceType::CheckBoxX:
		throw gcnew NotImplementedException();
	}

	return nullptr;
}

bool Button::GetterChecked()
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		return ButtonItem->Checked;
	case eSourceType::ButtonX:
		return ButtonX->Checked;
	case eSourceType::CheckBoxX:
		return CheckBoxX->Checked;
	}

	return false;
}

bool Button::GetterVisible()
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		return ButtonItem->Visible;
	case eSourceType::ButtonX:
		return ButtonX->Visible;
	case eSourceType::CheckBoxX:
		return CheckBoxX->Visible;
	}

	return false;
}

bool Button::GetterEnabled()
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		return ButtonItem->Enabled;
	case eSourceType::ButtonX:
		return ButtonX->Enabled;
	case eSourceType::CheckBoxX:
		return CheckBoxX->Enabled;
	}

	return false;
}

void Button::SetterText(String^ Value)
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->Text = Value;
		break;
	case eSourceType::ButtonX:
		ButtonX->Text = Value;
		break;
	case eSourceType::CheckBoxX:
		CheckBoxX->Text = Value;
		break;
	}
}

void Button::SetterShortcutKey(String^ Value)
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->AlternateShortCutText = Value;
		break;
	case eSourceType::ButtonX:
	case eSourceType::CheckBoxX:
		throw gcnew NotImplementedException();
	}
}

void Button::SetterTooltip(String^ Value)
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->Tooltip = Value;
		break;
	case eSourceType::ButtonX:
		ButtonX->Tooltip = Value;
		break;
	case eSourceType::CheckBoxX:
		throw gcnew NotImplementedException();
	}
}

void Button::SetterChecked(bool Value)
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->Checked = Value;
		break;
	case eSourceType::ButtonX:
		break;
		ButtonX->Checked = Value;
	case eSourceType::CheckBoxX:
		CheckBoxX->Checked = Value;
		break;
	}
}

void Button::SetterVisible(bool Value)
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->Visible = Value;
		break;
	case eSourceType::ButtonX:
		ButtonX->Visible = Value;
		break;
	case eSourceType::CheckBoxX:
		CheckBoxX->Visible = Value;
		break;
	}
}

void Button::SetterEnabled(bool Value)
{
	switch (SourceType)
	{
	case eSourceType::ButtonItem:
		ButtonItem->Enabled = Value;
		break;
	case eSourceType::ButtonX:
		ButtonX->Enabled = Value;
		break;
	case eSourceType::CheckBoxX:
		CheckBoxX->Enabled = Value;
		break;
	}
}

Button::Button(DotNetBar::ButtonItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Button, ViewRole, EventRouter)
{
	SourceType = eSourceType::ButtonItem;
	ButtonItem = Source;
	Tag_ = nullptr;
	InitEventHandlers();
}

Button::Button(DotNetBar::ButtonX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Button, ViewRole, EventRouter)
{
	SourceType = eSourceType::ButtonX;
	ButtonX = Source;
	Tag_ = nullptr;
	InitEventHandlers();
}

Button::Button(DotNetBar::Controls::CheckBoxX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Button, ViewRole, EventRouter)
{
	SourceType = eSourceType::CheckBoxX;
	CheckBoxX = Source;
	Tag_ = nullptr;
	InitEventHandlers();
}

Button::~Button()
{
	DeinitEventHandlers();
	ButtonItem = nullptr;
	ButtonX = nullptr;
	CheckBoxX = nullptr;
	Tag_ = nullptr;
}

void Button::PerformClick()
{
	if (!Enabled)
		return;

	Handler_Click(nullptr, nullptr);
}

void ComboBox::Handler_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
{
	auto EventArgs = gcnew IComboBox::SelectionChangedEventArgs;

	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		EventArgs->SelectedItem = ComboBoxItem->SelectedItem;
		break;
	case eSourceType::ComboBoxEx:
		EventArgs->SelectedItem = ComboBoxEx->SelectedItem;
		break;
	}

	RaiseEvent(IComboBox::eEvent::SelectionChanged, EventArgs);
}

void ComboBox::Handler_KeyDown(Object^ Sender, KeyEventArgs^ E)
{
	auto EventArgs = gcnew IComboBox::KeyDownEventArgs;
	EventArgs->KeyEvent = E;

	RaiseEvent(IComboBox::eEvent::KeyDown, EventArgs);
}

void ComboBox::InitEventHandlers()
{
	DelegateSelectedIndexChanged = gcnew EventHandler(this, &ComboBox::Handler_SelectedIndexChanged);
	DelegateKeyDown = gcnew KeyEventHandler(this, &ComboBox::Handler_KeyDown);

	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->SelectedIndexChanged += DelegateSelectedIndexChanged;
		break;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->SelectedIndexChanged += DelegateSelectedIndexChanged;
		ComboBoxEx->KeyDown += DelegateKeyDown;
		break;
	}
}

void ComboBox::DeinitEventHandlers()
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->SelectedIndexChanged -= DelegateSelectedIndexChanged;
		break;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->SelectedIndexChanged -= DelegateSelectedIndexChanged;
		ComboBoxEx->KeyDown -= DelegateKeyDown;
		break;
	}

	SAFEDELETE_CLR(DelegateSelectedIndexChanged);
	SAFEDELETE_CLR(DelegateKeyDown);
}

System::String^ ComboBox::GetterText()
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		return ComboBoxItem->Text;
	case eSourceType::ComboBoxEx:
		return ComboBoxEx->Text;
	}

	return nullptr;
}

System::Object^ ComboBox::GetterSelection()
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		return ComboBoxItem->SelectedItem;
	case eSourceType::ComboBoxEx:
		return ComboBoxEx->SelectedItem;
	}

	return nullptr;
}

System::Collections::IEnumerable^ ComboBox::GetterItems()
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		return ComboBoxItem->Items;
	case eSourceType::ComboBoxEx:
		return ComboBoxEx->Items;
	}

	return nullptr;
}

bool ComboBox::GetterEnabled()
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		return ComboBoxItem->Enabled;
	case eSourceType::ComboBoxEx:
		return ComboBoxEx->Enabled;
	}

	return false;
}

void ComboBox::SetterText(String^ Value)
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->Text = Value;
		break;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->Text = Value;
		break;
	}
}

void ComboBox::SetterSelection(Object^ Value)
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->SelectedItem = Value;
		break;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->SelectedItem = Value;
		break;
	}
}

void ComboBox::SetterEnabled(bool Value)
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->Enabled = Value;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->Enabled = Value;
	}
}

ComboBox::ComboBox(DotNetBar::ComboBoxItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::ComboBox, ViewRole, EventRouter)
{
	ComboBoxItem = Source;
	SourceType = eSourceType::ComboBoxItem;
	InitEventHandlers();
}

ComboBox::ComboBox(DotNetBar::Controls::ComboBoxEx^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::ComboBox, ViewRole, EventRouter)
{
	ComboBoxEx = Source;
	SourceType = eSourceType::ComboBoxEx;
	InitEventHandlers();
}

ComboBox::~ComboBox()
{
	DeinitEventHandlers();
	ComboBoxEx = nullptr;
	ComboBoxItem = nullptr;
}

void ComboBox::AddDropdownItem(Object^ NewItem, bool AtFirstPosition)
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		if (AtFirstPosition)
			ComboBoxItem->Items->Insert(0, NewItem);
		else
			ComboBoxItem->Items->Add(NewItem);
		break;
	case eSourceType::ComboBoxEx:
		if (AtFirstPosition)
			ComboBoxEx->Items->Insert(0, NewItem);
		else
			ComboBoxEx->Items->Add(NewItem);
		break;
	}
}

void ComboBox::ClearDropdownItems()
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->Items->Clear();
		break;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->Items->Clear();
		break;
	}
}

System::Object^ ComboBox::LookupDropdownItem(String^ DropdownItemText)
{
	for each (auto Item in GetterItems())
	{
		if (String::Equals(Item->ToString(), DropdownItemText))
			return Item;
	}

	return nullptr;
}

Label::Label(DotNetBar::LabelItem^ Source, eViewRole ViewRole)
	: ViewComponent(eComponentType::Label, ViewRole, nullptr)
{
	this->Source = Source;
}

Label::~Label()
{
	Source = nullptr;
}

TabStripItem::TabStripItem(DotNetBar::SuperTabItem^ Source, ITabStrip^ ParentTabStrip)
	: ViewComponent(eComponentType::TabStripItem, eViewRole::None, nullptr)
{
	this->Source_ = Source;
	this->Source_->Tag = this;
	this->ParentTabStrip_ = ParentTabStrip;
	Tag_ = nullptr;
}

TabStripItem::~TabStripItem()
{
	Tag_ = nullptr;
	Source_->Tag = nullptr;
	Source_ = nullptr;
	ParentTabStrip_ = nullptr;
}

TabStripItem^ TabStripItem::FromSuperTabItem(SuperTabItem^ SuperTabItem)
{
	return safe_cast<TabStripItem^>(SuperTabItem->Tag);
}

TabStripItem^ TabStripItem::New(ITabStrip^ ParentTabStrip)
{
	auto Source = gcnew SuperTabItem;
	auto Wrapper = gcnew TabStripItem(Source, ParentTabStrip);
	return Wrapper;
}

void TabStrip::Handler_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs^ E)
{
	auto EventArgs = gcnew ITabStrip::TabClosingEventArgs;
	EventArgs->Cancel = false;
	EventArgs->Tab = TabStripItem::FromSuperTabItem(safe_cast<SuperTabItem^>(E->Tab));

	RaiseEvent(ITabStrip::eEvent::TabClosing, EventArgs);

	E->Cancel = EventArgs->Cancel;
}

void TabStrip::Handler_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E)
{
	auto EventArgs = gcnew ITabStrip::ActiveTabChangedEventArgs;
	EventArgs->OldValue = TabStripItem::FromSuperTabItem(safe_cast<SuperTabItem^>(E->OldValue));
	EventArgs->NewValue = TabStripItem::FromSuperTabItem(safe_cast<SuperTabItem^>(E->NewValue));

	RaiseEvent(ITabStrip::eEvent::ActiveTabChanged, EventArgs);
}

void TabStrip::Handler_TabStripMouseClick(Object^ Sender, MouseEventArgs^ E)
{
	auto EventArgs = gcnew ITabStrip::TabClickEventArgs;
	EventArgs->MouseEvent = E;
	EventArgs->MouseOverTab = TabStripItem::FromSuperTabItem(GetMouseOverTab());

	RaiseEvent(ITabStrip::eEvent::TabClick, EventArgs);
}

void TabStrip::Handler_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E)
{
	// prevent the tab from moving behind any non-tab items
	if (E->InsertBefore)
	{
		if (E->InsertTab == nullptr || E->InsertTab->GetType() != DotNetBar::SuperTabItem::typeid)
		{
			E->CanMove = false;
			return;
		}
	}

	auto EventArgs = gcnew ITabStrip::TabMovingEventArgs;
	EventArgs->Tab = TabStripItem::FromSuperTabItem(safe_cast<SuperTabItem^>(E->MoveTab));

	RaiseEvent(ITabStrip::eEvent::TabMoving, EventArgs);
}

SuperTabItem^ TabStrip::GetMouseOverTab()
{
	for each (auto Itr in Source->Tabs)
	{
		if (Itr->GetType() != DotNetBar::SuperTabItem::typeid)
			continue;

		auto Item = safe_cast<DotNetBar::SuperTabItem^>(Itr);
		if (Item->IsMouseOver)
			return Item;
	}

	return nullptr;
}

void TabStrip::SelectTab(SuperTabItem^ Tab)
{
	Source->SelectedTab = Tab;
	Source->TabStrip->EnsureVisible(Tab);
}

SuperTabItem^ TabStrip::GetFirstTabItem()
{
	for each (auto Itr in Source->Tabs)
	{
		if (Itr->GetType() != DotNetBar::SuperTabItem::typeid)
			continue;

		return safe_cast<DotNetBar::SuperTabItem^>(Itr);
	}

	return nullptr;
}

TabStrip::TabStrip(DotNetBar::SuperTabControl^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::TabStrip, ViewRole, EventRouter)
{
	this->Source = Source;

	Source->TabItemClose += DelegateTabItemClose;
	Source->SelectedTabChanged += DelegateSelectedTabChanged;
	Source->TabStripMouseClick += DelegateTabStripMouseClick;
	Source->TabMoving += DelegateTabMoving;

	Source->TabStrip->Tag = this;
}

TabStrip::~TabStrip()
{
	Source->TabItemClose -= DelegateTabItemClose;
	Source->SelectedTabChanged -= DelegateSelectedTabChanged;
	Source->TabStripMouseClick -= DelegateTabStripMouseClick;
	Source->TabMoving -= DelegateTabMoving;

	SAFEDELETE_CLR(DelegateTabItemClose);
	SAFEDELETE_CLR(DelegateSelectedTabChanged);
	SAFEDELETE_CLR(DelegateTabStripMouseClick);
	SAFEDELETE_CLR(DelegateTabMoving);

	Source->TabStrip->Tag = nullptr;
	Source = nullptr;
}


ITabStripItem^ TabStrip::ActiveTab::get()
{
	if (Source->SelectedTab == nullptr)
		return nullptr;

	return TabStripItem::FromSuperTabItem(Source->SelectedTab);
}

void TabStrip::ActiveTab::set(ITabStripItem^ v)
{
	auto TabItem = safe_cast<TabStripItem^>(v);
	Debug::Assert(TabItem->Parent == this);
	Debug::Assert(Source->Tabs->Contains(TabItem->Source));

	SelectTab(TabItem->Source);
}

UInt32 TabStrip::TabCount::get()
{
	UInt32 Count = 0;
	for each (auto Itr in Source->Tabs)
	{
		if (Itr->GetType() == DotNetBar::SuperTabItem::typeid)
			++Count;
	}
	return Count;
}


ITabStripItem^ TabStrip::AllocateNewTab()
{
	return TabStripItem::New(this);
}

void TabStrip::AddTab(ITabStripItem^ Tab)
{
	auto TabItem = safe_cast<TabStripItem^>(Tab);
	Debug::Assert(Tab->Parent == this);
	Source->Tabs->Add(TabItem->Source);
}

void TabStrip::RemoveTab(ITabStripItem^ Tab)
{
	auto TabItem = safe_cast<TabStripItem^>(Tab);
	Debug::Assert(Tab->Parent == this);
	Source->Tabs->Remove(TabItem->Source);
}

void TabStrip::SelectPreviousTab()
{
	if (Source->Tabs->Count < 2)
		return;

	if (!Source->SelectPreviousTab())
	{
		auto Tab = safe_cast<DotNetBar::SuperTabItem^>(Source->Tabs[Source->Tabs->Count - 1]);
		SelectTab(Tab);
	}
}

ITabStripItem^ TabStrip::LookupTabByTag(Object^ Tag)
{
	for each (auto Itr in Source->Tabs)
	{
		if (Itr->GetType() != DotNetBar::SuperTabItem::typeid)
			continue;

		auto TabItem = TabStripItem::FromSuperTabItem(safe_cast<DotNetBar::SuperTabItem^>(Itr));
		Debug::Assert(TabItem != nullptr);

		if (TabItem->Tag == Tag)
			return TabItem;
	}

	return nullptr;
}

void TabStrip::SelectNextTab()
{
	if (Source->Tabs->Count < 2)
		return;

	if (!Source->SelectNextTab())
	{
		auto Tab = GetFirstTabItem();
		SelectTab(Tab);
	}
}


ObjectListViewColumn::ObjectListViewColumn(BrightIdeasSoftware::OLVColumn^ Source, IObjectListView^ ParentListView)
	: ViewComponent(eComponentType::ObjectListViewColumn, eViewRole::None, nullptr)
{
	this->Source_ = Source;
	this->Source_->Tag = this;
	this->ParentListView_ = ParentListView;

	DelegateAspectGetter = nullptr;
	DelegateAspectToStringGetter = nullptr;
	DelegateImageGetter = nullptr;

	DelegateWrapperAspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(this, &ObjectListViewColumn::Wrapper_AspectGetter);
	DelegateWrapperAspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(this, &ObjectListViewColumn::Wrapper_AspectToStringConverter);
	DelegateWrapperImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(this, &ObjectListViewColumn::Wrapper_ImageGetter);
}

ObjectListViewColumn::~ObjectListViewColumn()
{
	if (DelegateAspectGetter != nullptr)
	{
		Source_->AspectGetter = nullptr;
		DelegateAspectGetter = nullptr;
	}

	if (DelegateAspectToStringGetter != nullptr)
	{
		Source_->AspectToStringConverter = nullptr;
		DelegateAspectToStringGetter = nullptr;
	}

	if (DelegateImageGetter != nullptr)
	{
		Source_->ImageGetter = nullptr;
		DelegateImageGetter = nullptr;
	}

	ParentListView_ = nullptr;

	SAFEDELETE_CLR(DelegateWrapperAspectGetter);
	SAFEDELETE_CLR(DelegateWrapperAspectToStringConverter);
	SAFEDELETE_CLR(DelegateWrapperImageGetter);

	Source_->Tag = nullptr;
	Source_ = nullptr;
}

void ObjectListViewColumn::SetAspectGetter(IObjectListViewColumn::AspectGetter^ Delegate)
{
	DelegateAspectGetter = Delegate;
	Source_->AspectGetter = DelegateWrapperAspectGetter;
}

void ObjectListViewColumn::SetAspectToStringGetter(IObjectListViewColumn::AspectToStringGetter^ Delegate)
{
	DelegateAspectToStringGetter = Delegate;
	Source_->AspectToStringConverter = DelegateWrapperAspectToStringConverter;
}

void ObjectListViewColumn::SetImageGetter(IObjectListViewColumn::ImageGetter^ Delegate)
{
	DelegateImageGetter = Delegate;
	Source_->ImageGetter = DelegateWrapperImageGetter;
}

ObjectListViewColumn^ ObjectListViewColumn::FromOLVColumn(BrightIdeasSoftware::OLVColumn^ Column)
{
	return safe_cast<ObjectListViewColumn^>(Column->Tag);
}

ObjectListViewColumn^ ObjectListViewColumn::New(IObjectListView^ Parent)
{
	auto Source = gcnew BrightIdeasSoftware::OLVColumn;
	auto Wrapper = gcnew ObjectListViewColumn(Source, Parent);
	return Wrapper;
}

System::Object^ ObjectListViewColumn::Wrapper_AspectGetter(Object^ Model)
{
	return DelegateAspectGetter(Model);
}

System::String^ ObjectListViewColumn::Wrapper_AspectToStringConverter(Object^ Aspect)
{
	return DelegateAspectToStringGetter(Aspect);
}

System::Object^ ObjectListViewColumn::Wrapper_ImageGetter(Object^ Model)
{
	return DelegateImageGetter(Model);
}


void ObjectListView::Handler_ItemActivate(Object^ Sender, EventArgs^ E)
{
	auto EventArgs = gcnew IObjectListView::ItemActivateEventArgs;
	EventArgs->ItemModel = Source->SelectedObject;
	if (Source->GetType() == BrightIdeasSoftware::TreeListView::typeid)
		EventArgs->ParentItemModel = safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->GetParent(EventArgs->ItemModel);

	RaiseEvent(IObjectListView::eEvent::ItemActivate, EventArgs);
}

bool ObjectListView::Wrapper_CanExpandGetter(Object^ Model)
{
	return DelegateCanExpandGetter(Model);
}

System::Collections::IEnumerable^ ObjectListView::Wrapper_ChildrenGetter(Object^ Model)
{
	return DelegateChildrenGetter(Model);
}

ObjectListView::ObjectListView(BrightIdeasSoftware::ObjectListView^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::ObjectListView, ViewRole, EventRouter)
{
	this->Source = Source;

	DelegateCanExpandGetter = nullptr;
	DelegateChildrenGetter = nullptr;

	DelegateItemActivate = gcnew EventHandler(this, &ObjectListView::Handler_ItemActivate);
	DelegateWrapperCanExpandGetter = gcnew BrightIdeasSoftware::TreeListView::CanExpandGetterDelegate(this, &ObjectListView::Wrapper_CanExpandGetter);
	DelegateWrapperChildrenGetter = gcnew BrightIdeasSoftware::TreeListView::ChildrenGetterDelegate(this, &ObjectListView::Wrapper_ChildrenGetter);

	Source->ItemActivate += DelegateItemActivate;
}

ObjectListView::~ObjectListView()
{
	Source->ItemActivate -= DelegateItemActivate;

	if (DelegateCanExpandGetter != nullptr)
	{
		safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->CanExpandGetter = nullptr;
		DelegateCanExpandGetter = nullptr;
	}

	if (DelegateChildrenGetter != nullptr)
	{
		safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->ChildrenGetter = nullptr;
		DelegateChildrenGetter = nullptr;
	}

	SAFEDELETE_CLR(DelegateItemActivate);
	SAFEDELETE_CLR(DelegateWrapperCanExpandGetter);
	SAFEDELETE_CLR(DelegateWrapperChildrenGetter);

	for each (auto Column in Source->AllColumns)
	{
		auto ColumnItem = ObjectListViewColumn::FromOLVColumn(Column);
		delete ColumnItem;
	}

	Source = nullptr;
}

void ObjectListView::SetObjects(System::Collections::IEnumerable^ Collection, bool PreserveState)
{
	Source->SetObjects(Collection, PreserveState);
}

IObjectListViewColumn^ ObjectListView::AllocateNewColumn()
{
	return ObjectListViewColumn::New(this);
}

void ObjectListView::AddColumn(IObjectListViewColumn^ Column)
{
	auto ColData = safe_cast<ObjectListViewColumn^>(Column);
	Debug::Assert(ColData->Parent == this);

	Source->AllColumns->Add(ColData->Source);
	Source->Columns->Add(ColData->Source);
}

List<IObjectListViewColumn^>^ ObjectListView::GetColumns()
{
	auto Out = gcnew List<IObjectListViewColumn^>;
	for each (auto Itr in Source->AllColumns)
		Out->Add(safe_cast<IObjectListViewColumn^>(Itr->Tag));
	return Out;
}

void ObjectListView::SetCanExpandGetter(IObjectListView::CanExpandGetter^ Delegate)
{
	if (Source->GetType() != BrightIdeasSoftware::TreeListView::typeid)
		throw gcnew InvalidOperationException("Can only be called on a tree listview");

	DelegateCanExpandGetter = Delegate;
	safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->CanExpandGetter = DelegateWrapperCanExpandGetter;
}

void ObjectListView::SetChildrenGetter(IObjectListView::ChildrenGetter^ Delegate)
{
	if (Source->GetType() != BrightIdeasSoftware::TreeListView::typeid)
		throw gcnew InvalidOperationException("Can only be called on a tree listview");

	DelegateChildrenGetter = Delegate;
	safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->ChildrenGetter = DelegateWrapperChildrenGetter;
}


void ObjectListView::EnsureItemVisible(Object^ Item)
{
	Source->EnsureModelVisible(Item);
}

void ObjectListView::ExpandAll()
{
	if (Source->GetType() != BrightIdeasSoftware::TreeListView::typeid)
		throw gcnew InvalidOperationException("Can only be called on a tree listview");

	safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->ExpandAll();
}

void ObjectListView::CollapseAll()
{
	if (Source->GetType() != BrightIdeasSoftware::TreeListView::typeid)
		throw gcnew InvalidOperationException("Can only be called on a tree listview");

	safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->CollapseAll();
}

ProgressBar::ProgressBar(DotNetBar::CircularProgressItem^ Source, eViewRole ViewRole)
	: ViewComponent(eComponentType::ProgressBar, ViewRole, nullptr)
{
	this->Source = Source;
}

ProgressBar::~ProgressBar()
{
	Source = nullptr;
}

DockablePane::DockablePane(DockContainerItem^ Source, eViewRole ViewRole)
	: ViewComponent(eComponentType::DockablePane, ViewRole, nullptr)
{
	this->Source = Source;
}

DockablePane::~DockablePane()
{
	Source = nullptr;
}

bool DockablePane::Visible::get()
{
	return Source->Visible;
}

void DockablePane::Visible::set(bool v)
{
	DevComponents::DotNetBar::BarUtilities::SetDockContainerVisible(Source, v);
}
void DockablePane::Focus()
{
	auto ParentBar = safe_cast<DevComponents::DotNetBar::Bar^>(Source->ContainerControl);
	if (ParentBar == nullptr)
		return;

	ParentBar->SelectedDockContainerItem = Source;
	if (!ParentBar->AutoHideVisible)
		ParentBar->AutoHideVisible = true;

	Source->Focus();
	//Source->Control->Focus();
}


CrumbBarItem::CrumbBarItem(DotNetBar::CrumbBarItem^ Source)
	: ViewComponent(eComponentType::CrumbBarItem, eViewRole::None, nullptr)
{
	Source_ = Source;
	Tag_ = nullptr;
}

CrumbBarItem::~CrumbBarItem()
{
	Tag_ = nullptr;
	Source_ = nullptr;
}

void CrumbBarItem::AddChild(ICrumbBarItem^ Child)
{
	auto ToAdd = safe_cast<CrumbBarItem^>(Child);
	Debug::Assert(ToAdd->Source != this->Source);

	Source->SubItems->Add(ToAdd->Source);
}

void CrumbBarItem::ClearChildren()
{
	Source->SubItems->Clear();
}

CrumbBarItem^ CrumbBarItem::FromCrumbBarItem(DotNetBar::CrumbBarItem^ Crumb)
{
	return safe_cast<CrumbBarItem^>(Crumb->Tag);
}

CrumbBarItem^ CrumbBarItem::New()
{
	auto Source = gcnew DotNetBar::CrumbBarItem;
	auto Wrapper = gcnew CrumbBarItem(Source);
	Source->Tag = Wrapper;
	return Wrapper;
}

void CrumbBar::Handler_ItemClick(Object^ Sender, EventArgs^ E)
{
	DotNetBar::CrumbBarItem^ Selection = nullptr;
	if (Sender->GetType() == DotNetBar::CrumbBarItemView::typeid)
		Selection = safe_cast<DotNetBar::CrumbBarItemView^>(Sender)->AttachedItem;
	else if (Sender->GetType() == DotNetBar::CrumbBarItem::typeid)
		Selection = safe_cast<DotNetBar::CrumbBarItem^>(Sender);

	if (Selection == nullptr)
		return;

	auto EventArgs = gcnew ICrumbBar::ItemClickEventArgs;
	EventArgs->Item = CrumbBarItem::FromCrumbBarItem(Selection);

	RaiseEvent(ICrumbBar::eEvent::ItemClick, EventArgs);
}

CrumbBar::CrumbBar(DotNetBar::CrumbBar^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::CrumbBar, ViewRole, EventRouter)
{
	this->Source = Source;

	DelegateItemClick = gcnew EventHandler(this, &CrumbBar::Handler_ItemClick);

	Source->ItemClick += DelegateItemClick;
}

CrumbBar::~CrumbBar()
{
	Source->ItemClick -= DelegateItemClick;

	SAFEDELETE_CLR(DelegateItemClick);
	Source = nullptr;
}

ICrumbBarItem^ CrumbBar::SelectedItem::get()
{
	if (Source->SelectedItem == nullptr)
		return nullptr;

	return CrumbBarItem::FromCrumbBarItem(Source->SelectedItem);
}

void CrumbBar::SelectedItem::set(ICrumbBarItem^ v)
{
	auto CrumbItem = safe_cast<CrumbBarItem^>(v);
	Debug::Assert(Source->Items->Contains(CrumbItem->Source));

	Source->SelectedItem = CrumbItem->Source;
}

ICrumbBarItem^ CrumbBar::AllocateNewItem()
{
	return CrumbBarItem::New();
}

void CrumbBar::ClearItems()
{
	Source->Items->Clear();
}

void CrumbBar::AddItem(ICrumbBarItem^ Item)
{
	auto CrumbItem = safe_cast<CrumbBarItem^>(Item);
	Source->Items->Add(CrumbItem->Source);
}


Container::Container(Control^ Source, eViewRole ViewRole)
	: ViewComponent(eComponentType::Container, ViewRole, nullptr)
{
	this->Source = Source;
}

Container::~Container()
{
	Source = nullptr;
}

void Container::AddControl(Control^ Control)
{
	Source->Controls->Add(Control);
}

void Container::RemoveControl(Control^ Control)
{
	Source->Controls->Remove(Control);
}


} // namespace components


} // namespace viewImpl


} // namespace scriptEditor


} // namespace cse

