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

	static ViewComponent^ FromControl(Control^ Control);
};


ref class Form : public ViewComponent, IForm
{
	Forms::Form^ Source;
	CancelEventHandler^ DelegateClosing;
	KeyEventHandler^ DelegateKeyDown;

	void Handler_Closing(Object^ Sender, CancelEventArgs^ E);
	void Handler_KeyDown(Object^ Sender, KeyEventArgs^ E);
public:
	Form(Forms::Form^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~Form();

	ImplPropertySimple(String^, Text, Source->Text);
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

	EventHandler^ DelegateClick;
	DotNetBarManager::PopupOpenEventHandler^ DelegatePopupOpen;

	void Handler_Click(Object^ Sender, EventArgs^ E);
	void Handler_PopupOpen(Object^ Sender, PopupOpenEventArgs^ E);

	void InitEventHandlers();
	void DeinitEventHandlers();

	String^ GetterText();
	String^ GetterShortcutKey();
	String^ GetterTooltip();
	bool GetterChecked();
	bool GetterVisible();

	void SetterText(String^ Value);
	void SetterShortcutKey(String^ Value);
	void SetterTooltip(String^ Value);
	void SetterChecked(bool Value);
	void SetterVisible(bool Value);
public:
	Button(DotNetBar::ButtonItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	Button(DotNetBar::ButtonX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	Button(DotNetBar::Controls::CheckBoxX^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~Button();

	ImplPropertyWithAccessors(String^, Text);
	ImplPropertyWithAccessors(String^, Tooltip);
	ImplPropertyWithAccessors(String^, ShortcutKey);
	ImplPropertyWithAccessors(bool, Checked);
	ImplPropertyWithAccessors(bool, Visible);
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

	void Handler_SelectedIndexChanged(Object^ Sender, EventArgs^ E);

	void InitEventHandlers();
	void DeinitEventHandlers();

	String^ GetterSelectionText();
	Object^ GetterSelection();

	void SetterSelectionText(String^ Value);
	void SetterSelection(Object^ Value);
public:
	ComboBox(DotNetBar::ComboBoxItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	ComboBox(DotNetBar::Controls::ComboBoxEx^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~ComboBox();

	ImplPropertyWithAccessors(Object^, Selection);
	ImplPropertyWithAccessors(String^, SelectionText);

	virtual void AddDropdownItem(String^ NewItem);
	virtual void ClearDropdownItems();
};


ref class Label : public ViewComponent, ILabel
{
	LabelItem^ Source;
public:
	Label(DotNetBar::LabelItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~Label();

	ImplPropertySimple(String^, Text, Source->Text);
};


ref class TabStripItem : public ViewComponent, ITabStripItem
{
	SuperTabItem^ Source_;
	Object^ Tag_;
public:
	TabStripItem(DotNetBar::SuperTabItem^ Source);
	virtual ~TabStripItem();

	ImplPropertySimple(String^, Text, Source_->Text);
	ImplPropertySimple(String^, Tooltip, Source_->Tooltip);
	ImplPropertySimple(Drawing::Image^, Image, Source_->Image);
	ImplPropertySimple(Object^, Tag, Tag_);
	ImplPropertyGetOnly(SuperTabItem^, Source, Source_);

	static TabStripItem^ FromSuperTabItem(SuperTabItem^ SuperTabItem);
	static TabStripItem^ New();
};


ref class TabStrip : public ViewComponent, ITabStrip
{
	SuperTabControl^ Source;

	EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>^ DelegateTabItemClose;
	EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>^ DelegateSelectedTabChanged;
	EventHandler<MouseEventArgs^>^ DelegateTabStripMouseClick;
	EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>^ DelegateTabMoving;

	void Handler_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs ^ E);
	void Handler_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E);
	void Handler_TabStripMouseClick(Object^ Sender, MouseEventArgs^ E);
	void Handler_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E);

	SuperTabItem^ GetMouseOverTab();
	void SelectTab(SuperTabItem^ Tab);
public:
	TabStrip(DotNetBar::SuperTabControl^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~TabStrip();

	property ITabStripItem^ ActiveTab
	{
		virtual ITabStripItem^ get();
		virtual void set(ITabStripItem^ v);
	}

	virtual ITabStripItem^ AllocateNewTab();
	virtual void AddTab(ITabStripItem^ Tab);
	virtual void RemoveTab(ITabStripItem^ Tab);
	virtual ITabStripItem^ GetNthTab(UInt32 Index);
	virtual void SelectNextTab();
	virtual void SelectPreviousTab();
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

	IObjectListView::CanExpandGetter^ DelegateCanExpandGetter;
	IObjectListView::ChildrenGetter^ DelegateChildrenGetter;

	EventHandler^ DelegateItemActivate;
	BrightIdeasSoftware::TreeListView::CanExpandGetterDelegate^ DelegateWrapperCanExpandGetter;
	BrightIdeasSoftware::TreeListView::ChildrenGetterDelegate^ DelegateWrapperChildrenGetter;

	void Handler_ItemActivate(Object^ Sender, EventArgs^ E);
	bool Wrapper_CanExpandGetter(Object^ Model);
	Collections::IEnumerable^ Wrapper_ChildrenGetter(Object^ Model);
public:
	ObjectListView(BrightIdeasSoftware::ObjectListView^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~ObjectListView();

	ImplPropertySimple(Object^, SelectedObject, Source->SelectedObject);
	ImplPropertySimple(System::Collections::IList^, SelectedObjects, Source->SelectedObjects);

	virtual void SetObjects(System::Collections::IEnumerable^ Collection, bool PreserveState);
	virtual IObjectListViewColumn^ AllocateNewColumn();
	virtual void AddColumn(IObjectListViewColumn^ Column);
	virtual List<IObjectListViewColumn^>^ GetColumns();
	virtual void SetCanExpandGetter(IObjectListView::CanExpandGetter^ Delegate);
	virtual void SetChildrenGetter(IObjectListView::ChildrenGetter^ Delegate);
};


ref class CircularProgress : public ViewComponent, ICircularProgress
{
	DotNetBar::CircularProgressItem^ Source;
public:
	CircularProgress(DotNetBar::CircularProgressItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
	virtual ~CircularProgress();

	ImplPropertySimple(String^, Text, Source->Text);
	ImplPropertySimple(int, Value, Source->Value);
	ImplPropertySimple(int, Minimum, Source->Minimum);
	ImplPropertySimple(int, Maximum, Source->Maximum);
};


ref class DockablePane : public ViewComponent, IDockablePane
{
	DockContainerItem^ Source;
public:
	DockablePane(DockContainerItem^ Source, eViewRole ViewRole, ViewComponentEventRaiser^ EventRouter);
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

	virtual ICrumbBarItem^ AllocateNewItem();
	virtual void ClearItems();
	virtual void AddItem(ICrumbBarItem^ Item);
};


ref class Container : public ViewComponent, IContainer
{
	Control^ Source;
public:
	Container(Control^ Source, eViewRole ViewRole);
	virtual ~Container();

	virtual void AddControl(Control^ Control);
	virtual void RemoveControl(Control^ Control);
};


} // namespace components


} // namespace viewImpl


} // namespace scriptEditor


} // namespace cse