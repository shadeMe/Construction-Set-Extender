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
	// then, raise the same event for all of the button's subitems
	auto EventArgs = gcnew IButton::PopupOpeningEventArgs;
	RaiseEvent(IButton::eEvent::PopupOpening, EventArgs);

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

	return nullptr;
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

	return nullptr;
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

Button::Button(DotNetBar::ButtonItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Button, ViewRole, EventRouter)
{
	SourceType = eSourceType::ButtonItem;
	ButtonItem = Source;
	InitEventHandlers();
}

Button::Button(DotNetBar::ButtonX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Button, ViewRole, EventRouter)
{
	SourceType = eSourceType::ButtonX;
	ButtonX = Source;
	InitEventHandlers();
}

Button::Button(DotNetBar::Controls::CheckBoxX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Button, ViewRole, EventRouter)
{
	SourceType = eSourceType::CheckBoxX;
	CheckBoxX = Source;
	InitEventHandlers();
}

Button::~Button()
{
	DeinitEventHandlers();
	ButtonItem = nullptr;
	ButtonX = nullptr;
	CheckBoxX = nullptr;
}


void ComboBox::Handler_SelectedIndexChanged(Object^ Sender, EventArgs^ E)
{
	auto EventArgs = gcnew IComboBox::SelectionChangedEventArgs;

	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		EventArgs->SelectedItem = ComboBoxItem->SelectedItem;
		EventArgs->SelectedItemText = ComboBoxItem->SelectedText;
		break;
	case eSourceType::ComboBoxEx:
		EventArgs->SelectedItem = ComboBoxEx->SelectedItem;
		EventArgs->SelectedItemText = ComboBoxEx->SelectedText;
		break;
	}

	RaiseEvent(IComboBox::eEvent::SelectionChanged, EventArgs);
}

void ComboBox::InitEventHandlers()
{
	DelegateSelectedIndexChanged = gcnew EventHandler(this, &ComboBox::Handler_SelectedIndexChanged);

	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->SelectedIndexChanged += DelegateSelectedIndexChanged;
		break;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->SelectedIndexChanged += DelegateSelectedIndexChanged;
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
		break;
	}

	SAFEDELETE_CLR(DelegateSelectedIndexChanged);
}

System::String^ ComboBox::GetterSelectionText()
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		return ComboBoxItem->SelectedText;
	case eSourceType::ComboBoxEx:
		return ComboBoxEx->SelectedText;
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

void ComboBox::SetterSelectionText(String^ Value)
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->SelectedText = Value;
		break;
	case eSourceType::ComboBoxEx:
		ComboBoxEx->SelectedText = Value;
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

void ComboBox::AddDropdownItem(String^ NewItem)
{
	switch (SourceType)
	{
	case eSourceType::ComboBoxItem:
		ComboBoxItem->Items->Add(NewItem);
		break;
	case eSourceType::ComboBoxEx:
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

Label::Label(DotNetBar::LabelItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::Label, ViewRole, EventRouter)
{
	this->Source = Source;
}

Label::~Label()
{
	Source = nullptr;
}

TabStripItem::TabStripItem(DotNetBar::SuperTabItem^ Source)
	: ViewComponent(eComponentType::TabStripItem, eViewRole::None, nullptr)
{
	this->Source_ = Source;
	Tag_ = nullptr;
}

TabStripItem::~TabStripItem()
{
	Tag_ = nullptr;
	Source_ = nullptr;
}

TabStripItem^ TabStripItem::FromSuperTabItem(SuperTabItem^ SuperTabItem)
{
	return safe_cast<TabStripItem^>(SuperTabItem->Tag);
}

TabStripItem^ TabStripItem::New()
{
	auto Source = gcnew SuperTabItem;
	auto Wrapper = gcnew TabStripItem(Source);
	Source->Tag = Wrapper;
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
	auto EventArgs = gcnew ITabStrip::ActiveTabChanged;
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
	auto EventArgs = gcnew ITabStrip::TabMovingEventArgs;
	EventArgs->Tab = TabStripItem::FromSuperTabItem(safe_cast<SuperTabItem^>(E->MoveTab));

	RaiseEvent(ITabStrip::eEvent::TabMoving, EventArgs);
}

SuperTabItem^ TabStrip::GetMouseOverTab()
{
	for each (DotNetBar::SuperTabItem^ Itr in Source->Tabs)
	{
		if (Itr->IsMouseOver)
			return Itr;
	}

	return nullptr;
}

void TabStrip::SelectTab(SuperTabItem^ Tab)
{
	Source->SelectedTab = Tab;
	Source->TabStrip->EnsureVisible(Tab);
}

TabStrip::TabStrip(DotNetBar::SuperTabControl^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::TabStrip, ViewRole, EventRouter)
{
	this->Source = Source;

	Source->TabItemClose += DelegateTabItemClose;
	Source->SelectedTabChanged += DelegateSelectedTabChanged;
	Source->TabStripMouseClick += DelegateTabStripMouseClick;
	Source->TabMoving += DelegateTabMoving;
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

	Source = nullptr;
}


ITabStripItem^ TabStrip::ActiveTab::get()
{
	if (Source->SelectedTab == nullptr)
		return nullptr;

	return safe_cast<ITabStripItem^>(Source->SelectedTab->Tag);
}

void TabStrip::ActiveTab::set(ITabStripItem^ v)
{
	auto TabItem = safe_cast<TabStripItem^>(v);
	Source->SelectedTab = TabItem->Source;
}

ITabStripItem^ TabStrip::AllocateNewTab()
{
	return TabStripItem::New();
}

void TabStrip::AddTab(ITabStripItem^ Tab)
{
	auto TabItem = safe_cast<TabStripItem^>(Tab);
	Source->Tabs->Add(TabItem->Source);
}

void TabStrip::RemoveTab(ITabStripItem^ Tab)
{
	auto TabItem = safe_cast<TabStripItem^>(Tab);
	Source->Tabs->Remove(TabItem->Source);
}

ITabStripItem^ TabStrip::GetNthTab(UInt32 Index)
{
	if (Index >= Source->Tabs->Count)
		return nullptr;

	auto TabItem = safe_cast<DotNetBar::SuperTabItem^>(Source->Tabs[Index]);
	return TabStripItem::FromSuperTabItem(TabItem);
}

void TabStrip::SelectNextTab()
{
	if (Source->Tabs->Count == 1)
		return;

	if (!Source->SelectPreviousTab())
	{
		auto Tab = safe_cast<DotNetBar::SuperTabItem^>(Source->Tabs[Source->Tabs->Count - 1]);
		SelectTab(Tab);
	}
}

void TabStrip::SelectPreviousTab()
{
	if (Source->Tabs->Count == 1)
		return;

	if (!Source->SelectNextTab())
	{
		auto Tab = safe_cast<DotNetBar::SuperTabItem^>(Source->Tabs[0]);
		SelectTab(Tab);
	}
}


ObjectListViewColumn::ObjectListViewColumn(BrightIdeasSoftware::OLVColumn^ Source, IObjectListView^ ParentListView)
	: ViewComponent(eComponentType::ObjectListViewColumn, eViewRole::None, nullptr)
{
	this->Source = Source;
	this->ParentListView = ParentListView;

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
		Source->AspectGetter = nullptr;
		DelegateAspectGetter = nullptr;
	}

	if (DelegateAspectToStringGetter != nullptr)
	{
		Source->AspectToStringConverter = nullptr;
		DelegateAspectToStringGetter = nullptr;
	}

	if (DelegateImageGetter != nullptr)
	{
		Source->ImageGetter = nullptr;
		DelegateImageGetter = nullptr;
	}

	ParentListView = nullptr;

	SAFEDELETE_CLR(DelegateWrapperAspectGetter);
	SAFEDELETE_CLR(DelegateWrapperAspectToStringConverter);
	SAFEDELETE_CLR(DelegateWrapperImageGetter);

	Source = nullptr;
}

void ObjectListViewColumn::SetAspectGetter(IObjectListViewColumn::AspectGetter^ Delegate)
{
	DelegateAspectGetter = Delegate;
	Source->AspectGetter = DelegateWrapperAspectGetter;
}

void ObjectListViewColumn::SetAspectToStringGetter(IObjectListViewColumn::AspectToStringGetter^ Delegate)
{
	DelegateAspectToStringGetter = Delegate;
	Source->AspectToStringConverter = DelegateWrapperAspectToStringConverter;
}

void ObjectListViewColumn::SetImageGetter(IObjectListViewColumn::ImageGetter^ Delegate)
{
	DelegateImageGetter = Delegate;
	Source->ImageGetter = DelegateWrapperImageGetter;
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

	Source = nullptr;
}

void ObjectListView::SetObjects(System::Collections::IEnumerable^ Collection, bool PreserveState)
{
	Source->SetObjects(Collection, PreserveState);
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
	DelegateCanExpandGetter = Delegate;
	safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->CanExpandGetter = DelegateWrapperCanExpandGetter;
}

void ObjectListView::SetChildrenGetter(IObjectListView::ChildrenGetter^ Delegate)
{
	DelegateChildrenGetter = Delegate;
	safe_cast<BrightIdeasSoftware::TreeListView^>(Source)->ChildrenGetter = DelegateWrapperChildrenGetter;
}


CircularProgress::CircularProgress(DotNetBar::Controls::CircularProgress^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::CircularProgress, ViewRole, EventRouter)
{
	this->Source = Source;
}

CircularProgress::~CircularProgress()
{
	Source = nullptr;
}

DockablePane::DockablePane(DockContainerItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter)
	: ViewComponent(eComponentType::DockablePane, ViewRole, EventRouter)
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

