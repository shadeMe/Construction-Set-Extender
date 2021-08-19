#pragma once
#include "Macros.h"
#include "IScriptEditorView.h"


namespace cse
{


namespace scriptEditor
{


namespace viewImpl
{


namespace components
{


using namespace cse::scriptEditor::view;
using namespace DevComponents;
using namespace DevComponents::DotNetBar;
using namespace DevComponents::DotNetBar::Controls;
using namespace DevComponents::DotNetBar::Metro;
using namespace DevComponents::DotNetBar::Events;
using namespace System::Windows;
using namespace System::Drawing;


delegate void ViewComponentEventRaiser(ViewComponentEvent^ E);


ref class ViewComponent : public IViewComponent
{
	eComponentType Type_;
	eViewRole Role_;
	ViewComponentEventRaiser^ EventRouter;
protected:
	void RaiseEvent(Object^ EventType, Object^ EventArgs);
	void RaiseEvent(ViewComponent^ Component, Object^ EventType, Object^ EventArgs);
public:
	ViewComponent(eComponentType ComponentType, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~ViewComponent();

	property eComponentType Type
	{
		ImplPropertyGetSimple(eComponentType, Type_);
		ImplPropertySetInvalid(eComponentType);
	}
	property eViewRole Role
	{
		ImplPropertyGetSimple(eViewRole, Role_);
		ImplPropertySetInvalid(eViewRole);
	}

	virtual IForm^ AsForm();
	virtual IButton^ AsButton();
	virtual IComboBox^ AsComboBox();
	virtual ILabel^ AsLabel();
	virtual ITabStrip^ AsTabStrip();
	virtual ITabStripItem^ AsTabStripItem();
	virtual IObjectListView^ AsObjectListView();
	virtual IObjectListViewColumn^ AsObjectListViewColumn();
	virtual IProgressBar^ AsProgressBar();
	virtual IDockablePane^ AsDockablePane();
	virtual ICrumbBar^ AsCrumbBar();
	virtual ICrumbBarItem^ AsCrumbBarItem();
	virtual IContainer^ AsContainer();
	virtual IContextMenu^ AsContextMenu();
};


ref class Form : public ViewComponent, IForm
{
	Forms::Form^ Source;
	int UpdateCounter;
	CancelEventHandler^ DelegateClosing;
	KeyEventHandler^ DelegateKeyDown;
	EventHandler^ DelegateLocationChanged;
	EventHandler^ DelegateSizeChanged;
	EventHandler^ DelegateActivated;
	EventHandler^ DelegateDeactivate;

	void Handler_Closing(Object^ Sender, CancelEventArgs^ E);
	void Handler_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void Handler_LocationChanged(Object^ Sender, EventArgs^ E);
	void Handler_SizeChanged(Object^ Sender, EventArgs^ E);
	void Handler_Activated(Object^ Sender, EventArgs^ E);
	void Handler_Deactivate(Object^ Sender, EventArgs^ E);
public:
	Form(Forms::Form^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~Form();

	property String^ Text
	{
		virtual String^ get();
		virtual void set(String^ v);
	}
	property Rectangle Bounds
	{
		virtual Rectangle get();
		virtual void set(Rectangle v);
	}

	virtual void BeginUpdate();
	virtual void EndUpdate();
	virtual void Redraw();
	virtual void Focus();
	virtual void Close(bool Force);
};


ref class Button : public ViewComponent, IButton
{
	static enum class eSourceType
	{
		ButtonItem,
		ButtonX,
		CheckBoxX
	};

	ButtonItem^ ButtonItem;
	ButtonX^ ButtonX;
	CheckBoxX^ CheckBoxX;
	eSourceType SourceType;
	Object^ Tag_;
	String^ TextBuffer;
	String^ TooltipBuffer;
	String^ ShortcutKeyBuffer;
	bool MouseOver;

	EventHandler^ DelegateClick;
	DotNetBarManager::PopupOpenEventHandler^ DelegatePopupOpen;
	EventHandler^ DelegateMouseEnter;
	EventHandler^ DelegateMouseLeave;

	void Handler_Click(Object^ Sender, EventArgs^ E);
	void Handler_PopupOpen(Object^ Sender, PopupOpenEventArgs^ E);
	void Handler_MouseEnter(Object^ Sender, EventArgs^ E);
	void Handler_MouseLeave(Object^ Sender, EventArgs^ E);

	void InitEventHandlers();
	void DeinitEventHandlers();

	String^ GetterText();
	String^ GetterShortcutKey();
	String^ GetterTooltip();
	Image^ GetterImage();
	bool GetterChecked();
	bool GetterVisible();
	bool GetterEnabled();

	void SetterText(String^ Value);
	void SetterShortcutKey(String^ Value);
	void SetterTooltip(String^ Value);
	void SetterImage(Image^ Value);
	void SetterChecked(bool Value);
	void SetterVisible(bool Value);
	void SetterEnabled(bool Value);

	void CombineTextAndShortcut();
	void CombineTooltipAndShortcut();
public:
	Button(DotNetBar::ButtonItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	Button(DotNetBar::ButtonX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	Button(DotNetBar::Controls::CheckBoxX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~Button();

	ImplPropertyWithAccessors(String^, Text);
	ImplPropertyWithAccessors(String^, Tooltip);
	ImplPropertyWithAccessors(String^, ShortcutKey);
	ImplPropertyWithAccessors(Drawing::Image^, Image);
	ImplPropertySimple(Object^, Tag, Tag_);
	ImplPropertyWithAccessors(bool, Checked);
	ImplPropertyWithAccessors(bool, Visible);
	ImplPropertyWithAccessors(bool, Enabled);
	ImplPropertyGetOnly(bool, IsMouseOver, MouseOver);
	ImplPropertyGetOnly(DotNetBar::ButtonItem^, SourceButtonItem, ButtonItem);

	virtual void PerformClick();

	static Button^ FromDotNetBarBaseItem(BaseItem^ Item);
};


ref class ComboBox : public ViewComponent, IComboBox
{
	static enum class eSourceType
	{
		ComboBoxItem,
		ComboBoxEx
	};

	DotNetBar::ComboBoxItem^ ComboBoxItem;
	ComboBoxEx^ ComboBoxEx;
	eSourceType SourceType;

	EventHandler^ DelegateSelectedIndexChanged;
	KeyEventHandler^ DelegateKeyDown;

	void Handler_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
	void Handler_KeyDown(Object^ Sender, KeyEventArgs^ E);

	void InitEventHandlers();
	void DeinitEventHandlers();

	String^ GetterText();
	Object^ GetterSelection();
	Collections::IEnumerable^ GetterItems();
	bool GetterEnabled();

	void SetterText(String^ Value);
	void SetterSelection(Object^ Value);
	void SetterEnabled(bool Value);
public:
	ComboBox(DotNetBar::ComboBoxItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	ComboBox(DotNetBar::Controls::ComboBoxEx^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~ComboBox();

	ImplPropertyWithAccessors(Object^, Selection);
	ImplPropertyWithAccessors(String^, Text);
	ImplPropertyGetOnlyAccessor(Collections::IEnumerable^, Items);
	ImplPropertyWithAccessors(bool, Enabled);

	virtual void AddDropdownItem(Object^ NewItem, bool AtFirstPosition);
	virtual void ClearDropdownItems();
	virtual Object^ LookupDropdownItem(String^ DropdownItemText);
	virtual void Focus();
};


ref class Label : public ViewComponent, ILabel
{
	LabelItem^ Source;
public:
	Label(DotNetBar::LabelItem^ Source, eViewRole ViewRole);
	virtual ~Label();

	ImplPropertySimple(String^, Text, Source->Text);
	ImplPropertySimple(Drawing::Image^, Image, Source->Image);
	ImplPropertySimple(bool, Visible, Source->Visible);
};


ref class TabStripItem : public ViewComponent, ITabStripItem
{
	SuperTabItem^ Source_;
	Object^ Tag_;
	ITabStrip^ ParentTabStrip_;
public:
	TabStripItem(DotNetBar::SuperTabItem^ Source, ITabStrip^ ParentTabStrip);
	virtual ~TabStripItem();

	ImplPropertySimple(String^, Text, Source_->Text);
	ImplPropertySimple(String^, Tooltip, Source_->Tooltip);
	ImplPropertySimple(Drawing::Image^, Image, Source_->Image);
	ImplPropertySimple(Object^, Tag, Tag_);
	ImplPropertyGetOnly(SuperTabItem^, Source, Source_);
	ImplPropertyGetOnly(ITabStrip^, Parent, ParentTabStrip_);

	virtual void Close();

	static TabStripItem^ FromSuperTabItem(SuperTabItem^ SuperTabItem);
	static TabStripItem^ New(ITabStrip^ ParentTabStrip);
};


ref class TabStrip : public ViewComponent, ITabStrip
{
	SuperTabControl^ Source;
	int UpdateCounter;

	EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>^ DelegateTabItemClose;
	EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>^ DelegateSelectedTabChanged;
	EventHandler<MouseEventArgs^>^ DelegateTabStripMouseClick;
	EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>^ DelegateTabMoving;
	EventHandler<DotNetBar::SuperTabStripTabMovedEventArgs^>^ DelegateTabMoved;
	EventHandler<MouseEventArgs^>^ DelegateTabStripMouseDown;
	EventHandler<MouseEventArgs^>^ DelegateTabStripMouseUp;
	EventHandler<MouseEventArgs^>^ DelegateTabStripMouseMove;
	EventHandler<MouseEventArgs^>^ DelegateTabStripDoubleClick;

	void Handler_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs ^ E);
	void Handler_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E);
	void Handler_TabStripMouseClick(Object^ Sender, MouseEventArgs^ E);
	void Handler_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E);
	void Handler_TabMoved(Object^ Sender, DotNetBar::SuperTabStripTabMovedEventArgs^ E);
	void Handler_TabStripMouseDown(Object^ Sender, MouseEventArgs^ E);
	void Handler_TabStripMouseUp(Object^ Sender, MouseEventArgs^ E);
	void Handler_TabStripMouseMove(Object^ Sender, MouseEventArgs^ E);
	void Handler_TabStripDoubleClick(Object^ Sender, MouseEventArgs^ E);
	void HandleTabStripMouseEvent(ITabStrip::eEvent Event, MouseEventArgs^ E);

	SuperTabItem^ GetMouseOverTab();
	BaseItem^ GetMouseOverBaseItem(bool IgnoreTabItems);
	void SelectTab(SuperTabItem^ Tab);
	SuperTabItem^ GetFirstTabItem();
public:
	TabStrip(DotNetBar::SuperTabControl^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~TabStrip();

	property ITabStripItem^ ActiveTab
	{
		virtual ITabStripItem^ get();
		virtual void set(ITabStripItem^ v);
	}
	property ITabStripItem^ MouseOverTab
	{
		virtual ITabStripItem^ get();
		ImplPropertySetInvalid(ITabStripItem^);
	}
	property UInt32 TabCount
	{
		virtual UInt32 get();
		ImplPropertySetInvalid(UInt32);
	}
	property IEnumerable<ITabStripItem^>^ Tabs
	{
		virtual IEnumerable<ITabStripItem^>^ get();
		ImplPropertySetInvalid(IEnumerable<ITabStripItem^>^);
	}

	virtual ITabStripItem^ AllocateNewTab();
	virtual void AddTab(ITabStripItem^ Tab);
	virtual void RemoveTab(ITabStripItem^ Tab);
	virtual void SelectNextTab();
	virtual void SelectPreviousTab();
	virtual ITabStripItem^ LookupTabByTag(Object^ Tag);
	virtual void BeginUpdate();
	virtual void EndUpdate();
};


ref class ObjectListViewColumn : public ViewComponent, IObjectListViewColumn
{
	BrightIdeasSoftware::OLVColumn^ Source_;
	IObjectListView^ ParentListView_;

	IObjectListViewColumn::AspectGetter^ DelegateAspectGetter;
	IObjectListViewColumn::AspectToStringGetter^ DelegateAspectToStringGetter;
	IObjectListViewColumn::ImageGetter^ DelegateImageGetter;

	BrightIdeasSoftware::AspectGetterDelegate^ DelegateWrapperAspectGetter;
	BrightIdeasSoftware::AspectToStringConverterDelegate^ DelegateWrapperAspectToStringConverter;
	BrightIdeasSoftware::ImageGetterDelegate^ DelegateWrapperImageGetter;

	Object^ Wrapper_AspectGetter(Object^ Model);
	Object^ Wrapper_ImageGetter(Object^ Model);
	String^ Wrapper_AspectToStringConverter(Object^ Aspect);
public:
	ObjectListViewColumn(BrightIdeasSoftware::OLVColumn^ Source, IObjectListView^ ParentListView);
	virtual ~ObjectListViewColumn();

	ImplPropertyGetOnly(int, Index, Source_->Index);
	ImplPropertySimple(String^, Text, Source_->Text);
	ImplPropertyGetOnly(IObjectListView^, Parent, ParentListView_);
	ImplPropertySimple(int, MinimumWidth, Source_->MinimumWidth);
	ImplPropertySimple(int, MaximumWidth, Source_->MaximumWidth);
	ImplPropertySimple(int, Width, Source_->Width);
	ImplPropertySimple(bool, FillsFreeSpace, Source_->FillsFreeSpace);
	ImplPropertyGetOnly(BrightIdeasSoftware::OLVColumn^, Source, Source_);

	virtual void SetAspectGetter(IObjectListViewColumn::AspectGetter^ Delegate);
	virtual void SetAspectToStringGetter(IObjectListViewColumn::AspectToStringGetter^ Delegate);
	virtual void SetImageGetter(IObjectListViewColumn::ImageGetter^ Delegate);

	static ObjectListViewColumn^ FromOLVColumn(BrightIdeasSoftware::OLVColumn^ Column);
	static ObjectListViewColumn^ New(IObjectListView^ Parent);
};


ref class ObjectListView : public ViewComponent, IObjectListView
{
	BrightIdeasSoftware::ObjectListView^ Source;
	StyleManagerAmbient^ ColorManager;

	IObjectListView::CanExpandGetter^ DelegateCanExpandGetter;
	IObjectListView::ChildrenGetter^ DelegateChildrenGetter;
	IObjectListView::CheckStateGetter^ DelegateCheckStateGetter;
	IObjectListView::CheckStateSetter^ DelegateCheckStateSetter;

	EventHandler^ DelegateItemActivate;
	EventHandler^ DelegatePreferenceChanged;
	BrightIdeasSoftware::TreeListView::CanExpandGetterDelegate^ DelegateWrapperCanExpandGetter;
	BrightIdeasSoftware::TreeListView::ChildrenGetterDelegate^ DelegateWrapperChildrenGetter;
	BrightIdeasSoftware::BooleanCheckStateGetterDelegate^ DelegateWrapperCheckStateGetter;
	BrightIdeasSoftware::BooleanCheckStatePutterDelegate^ DelegateWrapperCheckStateSetter;

	void Handler_ItemActivate(Object^ Sender, EventArgs^ E);
	void Handler_PreferencesChanged(Object^ Sender, EventArgs^ E);
	bool Wrapper_CanExpandGetter(Object^ Model);
	Collections::IEnumerable^ Wrapper_ChildrenGetter(Object^ Model);
	bool Wrapper_CheckStateGetter(Object^ Model);
	bool Wrapper_CheckStateSetter(Object^ Model, bool NewValue);

	void SetLastColumnToFillFreeSpace();
public:
	ObjectListView(BrightIdeasSoftware::ObjectListView^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~ObjectListView();

	ImplPropertySimple(Object^, SelectedObject, Source->SelectedObject);
	ImplPropertySimple(System::Collections::IList^, SelectedObjects, Source->SelectedObjects);
	property bool HeaderVisible
	{
		virtual bool get();
		virtual void set(bool v);
	}
	ImplPropertySimple(bool, UseFiltering, Source->UseFiltering);
	ImplPropertySimple(bool, UseCheckBoxes, Source->CheckBoxes);

	virtual void SetObjects(System::Collections::IEnumerable^ Collection, bool PreserveState);
	virtual void ClearObjects();
	virtual IObjectListViewColumn^ AllocateNewColumn();
	virtual void AddColumn(IObjectListViewColumn^ Column);
	virtual List<IObjectListViewColumn^>^ GetColumns();
	virtual void SetCanExpandGetter(IObjectListView::CanExpandGetter^ Delegate);
	virtual void SetChildrenGetter(IObjectListView::ChildrenGetter^ Delegate);
	virtual void SetModelFilter(Predicate<Object^>^ Predicate);
	virtual void SetCheckStateGetter(IObjectListView::CheckStateGetter^ Delegate);
	virtual void SetCheckStateSetter(IObjectListView::CheckStateSetter^ Delegate);
	virtual void EnsureItemVisible(Object^ Item);
	virtual void ExpandAll();
	virtual void CollapseAll();
};


ref class ProgressBar : public ViewComponent, IProgressBar
{
	DotNetBar::CircularProgressItem^ Source;
public:
	ProgressBar(DotNetBar::CircularProgressItem^ Source, eViewRole ViewRole);
	virtual ~ProgressBar();

	ImplPropertySimple(String^, Text, Source->Text);
	ImplPropertySimple(String^, Tooltip, Source->Tooltip);
	ImplPropertySimple(int, Value, Source->Value);
	ImplPropertySimple(int, Minimum, Source->Minimum);
	ImplPropertySimple(int, Maximum, Source->Maximum);
	ImplPropertySimple(bool, Visible, Source->Visible);
};


ref class DockablePane : public ViewComponent, IDockablePane
{
	DockContainerItem^ Source;
public:
	DockablePane(DockContainerItem^ Source, eViewRole ViewRole);
	virtual ~DockablePane();

	property bool Visible
	{
		virtual bool get();
		virtual void set(bool v);
	}

	virtual void Focus();
};


ref class CrumbBarItem : public ViewComponent, ICrumbBarItem
{
	DotNetBar::CrumbBarItem^ Source_;
	Object^ Tag_;
public:
	CrumbBarItem(DotNetBar::CrumbBarItem^ Source);
	virtual ~CrumbBarItem();

	ImplPropertySimple(String^, Text, Source_->Text);
	ImplPropertySimple(String^, Tooltip, Source_->Tooltip);
	ImplPropertySimple(Drawing::Image^, Image, Source_->Image);
	ImplPropertySimple(Object^, Tag, Tag_);
	ImplPropertyGetOnly(DotNetBar::CrumbBarItem^, Source, Source_);

	virtual void AddChild(ICrumbBarItem^ Child);
	virtual void ClearChildren();

	static CrumbBarItem^ FromCrumbBarItem(DotNetBar::CrumbBarItem^ Crumb);
	static CrumbBarItem^ New();
};


ref class CrumbBar : public ViewComponent, ICrumbBar
{
	DotNetBar::CrumbBar^ Source;

	EventHandler^ DelegateItemClick;

	void Handler_ItemClick(Object^ Sender, EventArgs^ E);
public:
	CrumbBar(DotNetBar::CrumbBar^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~CrumbBar();

	property ICrumbBarItem^ SelectedItem
	{
		virtual ICrumbBarItem^ get();
		virtual void set(ICrumbBarItem^ v);
	}
	ImplPropertySimple(bool, Visible, Source->Visible);

	virtual ICrumbBarItem^ AllocateNewItem();
	virtual void ClearItems();
	virtual void AddItem(ICrumbBarItem^ Item);
};


ref class Container : public ViewComponent, IContainer
{
	static enum class eSourceType
	{
		Control,
		BaseItem,
	};

	Control^ BasicControl;
	DotNetBar::BaseItem^ DotNetBarBaseItem;
	eSourceType SourceType;

	MouseEventHandler^ DelegateMouseDown;
	MouseEventHandler^ DelegateMouseUp;
	MouseEventHandler^ DelegateMouseMove;
	EventHandler^ DelegateDoubleClick;

	void Handler_MouseDown(Object^ Sender, MouseEventArgs^ E);
	void Handler_MouseUp(Object^ Sender, MouseEventArgs^ E);
	void Handler_MouseMove(Object^ Sender, MouseEventArgs^ E);
	void Handler_DoubleClick(Object^ Sender, EventArgs^ E);

	void InitEventHandlers();
	void DeinitEventHandlers();

	bool GetterVisible();
	bool GetterFocused();
	void SetterVisible(bool Value);
public:
	Container(Control^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	Container(DotNetBar::BaseItem^ Source, eViewRole ViewRole,ViewComponentEventRaiser^ EventRouter );
	virtual ~Container();

	ImplPropertyWithAccessors(bool, Visible);
	ImplPropertyGetOnlyAccessor(bool, Focused);

	virtual void AddControl(Control^ Control);
	virtual void RemoveControl(Control^ Control);
	virtual void Invalidate();
	virtual void Focus();
};


ref class ContextMenu : public ViewComponent, IContextMenu
{
	ContextMenuBar^ Provider;
	Button^ RootWrapper;
	PopupItem^ Popup;
public:
	ContextMenu(ContextMenuBar^ Provider, ButtonItem^ Root, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~ContextMenu();

	ImplPropertySimple(bool, Expanded, Popup->Expanded);

	virtual void Show(Drawing::Point ScreenCoords);
	virtual void Hide();
};


} // namespace components


} // namespace viewImpl


} // namespace scriptEditor


} // namespace cse