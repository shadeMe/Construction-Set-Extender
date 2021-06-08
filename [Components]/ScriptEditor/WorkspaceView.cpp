#include "WorkspaceView.h"
#include "[Common]\CustomInputBox.h"
#include "IntelliSenseDatabase.h"
#include "Preferences.h"
#include "IntelliSenseInterfaceView.h"
#include "[Common]/ListViewUtilities.h"
#include "ScriptSync.h"

using namespace GlobalInputMonitor;

namespace cse
{
	namespace scriptEditor
	{
		void WorkspaceViewTabTearingHelper::TearingEventHandler(Object^ Sender, MouseEventArgs^ E)
		{
			if (ProcessingMouseMessage)
				return;

			ProcessingMouseMessage = true;

			switch (E->Button)
			{
			case MouseButtons::Left:
				{
					Debug::Assert(Torn != nullptr);
					Debug::Assert(Source != nullptr);

					bool Relocated = false;
					bool SameTabStrip = false;
					IntPtr Wnd = nativeWrapper::WindowFromPoint(E->Location);
					if (Wnd != IntPtr::Zero)
					{
						Control^ UnderMouse = Control::FromHandle(Wnd);
						if (UnderMouse)
						{
							DotNetBar::SuperTabStrip^ Strip = nullptr;
							try { Strip = (DotNetBar::SuperTabStrip^)UnderMouse; }
							catch (...) {}

							if (Strip && Strip->Tag)
							{
								ConcreteWorkspaceView^ Parent = (ConcreteWorkspaceView^)Strip->Tag;
								Debug::Assert(Parent != nullptr);

								if (Parent != Source)
								{
									// relocate to the view under the mouse
									Source->DissociateModel(Torn, false);
									Parent->AssociateModel(Torn, true);
									Parent->Focus();

									Relocated = true;
								}
								else
									SameTabStrip = true;
							}
						}
					}

					if (Relocated == false && SameTabStrip == false)
					{
						// create new view and relocate
						Rectangle Bounds = Source->GetBounds(true);
						ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)Source->ViewFactory->CreateView(E->Location.X, E->Location.Y,
																											 Bounds.Width, Bounds.Height);
						Source->DissociateModel(Torn, false);
						New->AssociateModel(Torn, true);
						New->Focus();

						if (Torn->Initialized)
							New->Enabled = true;
					}

					End();
				}

				break;
			default:
				End();
			}
		}

		void WorkspaceViewTabTearingHelper::End()
		{
			Debug::Assert(Active == true);

			Active = false;
			Torn = nullptr;
			Source = nullptr;
			ProcessingMouseMessage = false;

			HookManager::MouseUp -= TearingEventDelegate;
		}

		WorkspaceViewTabTearingHelper::WorkspaceViewTabTearingHelper()
		{
			Torn = nullptr;
			Source = nullptr;
			Active = false;
			ProcessingMouseMessage = false;

			TearingEventDelegate = gcnew MouseEventHandler(this, &WorkspaceViewTabTearingHelper::TearingEventHandler);
		}

		WorkspaceViewTabTearingHelper::~WorkspaceViewTabTearingHelper()
		{
			if (Active)
				End();

			Torn = nullptr;
			Source = nullptr;
			SAFEDELETE_CLR(TearingEventDelegate);
		}

		void WorkspaceViewTabTearingHelper::InitiateHandling(IWorkspaceModel^ Tearing, ConcreteWorkspaceView^ From)
		{
			Debug::Assert(Active == false);
			Debug::Assert(ProcessingMouseMessage == false);
			Debug::Assert(Tearing != nullptr);
			Debug::Assert(From != nullptr);

			Active = true;
			Torn = Tearing;
			Source = From;

			HookManager::MouseUp += TearingEventDelegate;
		}

		WorkspaceViewTabTearingHelper^ WorkspaceViewTabTearingHelper::Get()
		{
			if (Singleton == nullptr)
				Singleton = gcnew WorkspaceViewTabTearingHelper();

			return Singleton;
		}

		void WorkspaceViewTabFilter::ListView_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::Enter:
				CompleteSelection();
				break;
			case Keys::Escape:
				Hide();
				break;
			case Keys::Back:
				if (SearchBox->Text->Length >= 1)
				{
					SearchBox->Text = SearchBox->Text->Remove(SearchBox->Text->Length - 1);
					ListView->Focus();
				}

				E->Handled = true;
				break;
			}
		}

		void WorkspaceViewTabFilter::ListView_KeyPress(Object^ Sender, KeyPressEventArgs^ E)
		{
			if ((E->KeyChar > 0x29 && E->KeyChar < 0x3A) ||
				(E->KeyChar > 0x60 && E->KeyChar < 0x7B))
			{
				SearchBox->Text += E->KeyChar.ToString();
				ListView->Focus();
				E->Handled = true;
			}
		}

		void WorkspaceViewTabFilter::ListView_ItemActivate(Object^ Sender, EventArgs^ E)
		{
			CompleteSelection();
		}

		void WorkspaceViewTabFilter::SearchBox_TextChanged(Object^ Sender, EventArgs^ E)
		{
			UpdateFilterResults();
		}

		void WorkspaceViewTabFilter::SearchBox_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			switch (E->KeyCode)
			{
			case Keys::Back:
				return;
			}

			ListView_KeyDown(ListView, E);
		}

		void WorkspaceViewTabFilter::Form_Deactivate(Object^ Sender, EventArgs^ E)
		{
			Hide();
		}

		void WorkspaceViewTabFilter::UpdateFilterResults()
		{
			FilterResults->Clear();
			ListView->ClearObjects();

			String^ Check = SearchBox->Text;

			for each (DotNetBar::SuperTabItem^ Itr in Parent->EditorTabStrip->Tabs)
			{
				if (Itr->Text->IndexOf(Check, System::StringComparison::CurrentCultureIgnoreCase) != -1)
					FilterResults->Add(Itr);
			}

			ListView->SetObjects(FilterResults);
			if (ListView->GetItemCount())
				ListView->SelectedIndex = 0;
		}

		void WorkspaceViewTabFilter::CompleteSelection()
		{
			if (ListView->SelectedObject)
			{
				DotNetBar::SuperTabItem^ Selected = (DotNetBar::SuperTabItem^)ListView->SelectedObject;
				if (Selected->IsSelected == false)
					Parent->SelectTab(Selected);

				Parent->Focus();
			}
		}

		Object^ WorkspaceViewTabFilter::ListViewAspectGetter(Object^ RowObject)
		{
			if (RowObject)
			{
				DotNetBar::SuperTabItem^ Item = (DotNetBar::SuperTabItem^)RowObject;
				Debug::Assert(Item != nullptr);

				return Item->Text;
			}
			else
				return nullptr;
		}

		Object^ WorkspaceViewTabFilter::ListViewImageGetter(Object^ RowObject)
		{
			if (RowObject)
			{
				DotNetBar::SuperTabItem^ Item = (DotNetBar::SuperTabItem^)RowObject;
				Debug::Assert(Item != nullptr);

				return (int)Item->ImageIndex;
			}
			else
				return nullptr;
		}

		WorkspaceViewTabFilter::WorkspaceViewTabFilter(ConcreteWorkspaceView^ ParentView)
		{
			Debug::Assert(ParentView != nullptr);

			Parent = ParentView;

			Form = gcnew AnimatedForm(false);
			ListView = gcnew BrightIdeasSoftware::FastObjectListView;
			SearchBox = gcnew TextBox;
			FilterResults = gcnew List < DotNetBar::SuperTabItem^ > ;

			ListViewKeyDownHandler = gcnew KeyEventHandler(this, &WorkspaceViewTabFilter::ListView_KeyDown);
			ListViewKeyPressHandler = gcnew KeyPressEventHandler(this, &WorkspaceViewTabFilter::ListView_KeyPress);
			ListViewItemActivateHandler = gcnew EventHandler(this, &WorkspaceViewTabFilter::ListView_ItemActivate);
			SearchBoxTextChangedHandler = gcnew EventHandler(this, &WorkspaceViewTabFilter::SearchBox_TextChanged);
			SearchBoxKeyDownHandler = gcnew KeyEventHandler(this, &WorkspaceViewTabFilter::SearchBox_KeyDown);
			FormDeactivateHandler = gcnew EventHandler(this, &WorkspaceViewTabFilter::Form_Deactivate);

			SearchBox->Font = gcnew Font("Lucida Grande", 14.25F, FontStyle::Regular);
			SearchBox->MaxLength = 512;
			SearchBox->BorderStyle = BorderStyle::None;
			SearchBox->Dock = DockStyle::Bottom;
			SearchBox->Location = Point(0, 88);
			SearchBox->Size = Size(285, 32);
			SearchBox->Text = "";
			SearchBox->TextAlign = HorizontalAlignment::Center;

			ListView->View = View::Details;
			ListView->Dock = DockStyle::Fill;
			ListView->MultiSelect = false;
			ListView->BorderStyle = BorderStyle::None;
			ListView->Size = Size(285, 89);
			ListView->SmallImageList = gcnew ImageList();
			ListView->SmallImageList->Images->Add(Globals::ImageResources()->CreateImage("ModifiedFlagOff"));
			ListView->SmallImageList->Images->Add(Globals::ImageResources()->CreateImage("ModifiedFlagOn"));
			ListView->Location = Point(0, 0);
			ListView->Font = gcnew Font("Lucida Grande", 10, FontStyle::Regular);
			ListView->LabelEdit = false;
			ListView->CheckBoxes = false;
			ListView->FullRowSelect = true;
			ListView->GridLines = false;
			ListView->HeaderStyle = ColumnHeaderStyle::None;
			ListView->HideSelection = false;

			BrightIdeasSoftware::OLVColumn^ Column = gcnew BrightIdeasSoftware::OLVColumn;
			Column->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&WorkspaceViewTabFilter::ListViewAspectGetter);
			Column->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&WorkspaceViewTabFilter::ListViewImageGetter);
			Column->Text = "Tabs";
			Column->Width = 250;
			Column->TextAlign = HorizontalAlignment::Center;
			ListView->AllColumns->Add(Column);
			ListView->Columns->Add(Column);

			Form->FormBorderStyle = FormBorderStyle::SizableToolWindow;
			Form->StartPosition = FormStartPosition::Manual;
			Form->ShowInTaskbar = false;
			Form->ShowIcon = false;
			Form->ControlBox = false;
			Form->Controls->Add(ListView);
			Form->Controls->Add(SearchBox);

			Form->Size = Size(1, 1);
			Form->Show(Point(0, 0), IntPtr::Zero, false);
			Form->Hide(false);
			Form->Size = Size(285, 120);
			Form->MaximumSize = Size(285, 120);
			Form->MinimumSize = Size(285, 120);

			ListView->KeyDown += ListViewKeyDownHandler;
			ListView->ItemActivate += ListViewItemActivateHandler;
			ListView->KeyPress += ListViewKeyPressHandler;
			SearchBox->TextChanged += SearchBoxTextChangedHandler;
			SearchBox->KeyDown += SearchBoxKeyDownHandler;
			Form->Deactivate += FormDeactivateHandler;
		}

		WorkspaceViewTabFilter::~WorkspaceViewTabFilter()
		{
			FilterResults->Clear();

			ListView->KeyDown -= ListViewKeyDownHandler;
			ListView->ItemActivate -= ListViewItemActivateHandler;
			ListView->KeyPress -= ListViewKeyPressHandler;
			SearchBox->TextChanged -= SearchBoxTextChangedHandler;
			SearchBox->KeyDown -= SearchBoxKeyDownHandler;
			Form->Deactivate -= FormDeactivateHandler;

			for each (Image^ Itr in ListView->SmallImageList->Images)
				delete Itr;

			Form->ForceClose();

			SAFEDELETE_CLR(Form);
			SAFEDELETE_CLR(ListView);

			Parent = nullptr;
		}

		void WorkspaceViewTabFilter::Show()
		{
			Point DisplayLocation = Point(Parent->EditorForm->Location.X + (Parent->EditorForm->Width - Form->Width) / 2,
										  Parent->EditorForm->Location.Y + (Parent->EditorForm->Height - Form->Height) / 2);

			Form->Location = DisplayLocation;
			Form->Show(Parent->WindowHandle);
			UpdateFilterResults();
			Form->Focus();
		}

		void WorkspaceViewTabFilter::Hide()
		{
			FilterResults->Clear();
			ListView->ClearObjects();
			Form->Hide();
		}


		void WorkspaceViewOutlineView::ListView_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			obScriptParsing::Structurizer::Node^ Selection = (obScriptParsing::Structurizer::Node^)ListView->SelectedObject;
			if (Selection == nullptr)
				return;

			switch (E->KeyCode)
			{
			case Keys::Enter:
				JumpToLine(Selection->StartLine);
				break;
			case Keys::Escape:
				Hide();
				break;
			}
		}

		void WorkspaceViewOutlineView::ListView_ItemActivate(Object^ Sender, EventArgs^ E)
		{
			obScriptParsing::Structurizer::Node^ Selection = (obScriptParsing::Structurizer::Node^)ListView->SelectedObject;
			if (Selection == nullptr)
				return;

			JumpToLine(Selection->StartLine);
		}

		void WorkspaceViewOutlineView::Form_Deactivate(Object^ Sender, EventArgs^ E)
		{
			Hide();
		}

		void WorkspaceViewOutlineView::JumpToLine(UInt32 Line)
		{
			Debug::Assert(AssociatedModel != nullptr);
			AssociatedModel->Controller->GotoLine(AssociatedModel, Line);
			Hide();
		}

		void WorkspaceViewOutlineView::ResetState()
		{
			StructureData = nullptr;
			AssociatedModel = nullptr;
			ListView->ClearObjects();
		}

		Object^ WorkspaceViewOutlineView::ListViewAspectGetter(Object^ RowObject)
		{
			if (RowObject)
			{
				obScriptParsing::Structurizer::Node^ Item = (obScriptParsing::Structurizer::Node^)RowObject;
				Debug::Assert(Item != nullptr);

				return Item->Description;
			}
			else
				return nullptr;
		}

		Object^ WorkspaceViewOutlineView::ListViewImageGetter(Object^ RowObject)
		{
			if (RowObject)
			{
				obScriptParsing::Structurizer::Node^ Item = (obScriptParsing::Structurizer::Node^)RowObject;
				Debug::Assert(Item != nullptr);

				return (int)Item->Type;
			}
			else
				return nullptr;
		}

		bool WorkspaceViewOutlineView::ListViewCanExpandGetter(Object^ E)
		{
			obScriptParsing::Structurizer::Node^ Item = (obScriptParsing::Structurizer::Node^)E;
			Debug::Assert(Item != nullptr);

			return Item->Children->Count > 0;
		}

		Collections::IEnumerable^ WorkspaceViewOutlineView::ListViewChildrenGetter(Object^ E)
		{
			obScriptParsing::Structurizer::Node^ Item = (obScriptParsing::Structurizer::Node^)E;
			Debug::Assert(Item != nullptr);

			return Item->Children;
		}

		WorkspaceViewOutlineView::WorkspaceViewOutlineView(ConcreteWorkspaceView^ ParentView)
		{
			Debug::Assert(ParentView != nullptr);

			Parent = ParentView;

			Form = gcnew AnimatedForm(false);
			ListView = gcnew BrightIdeasSoftware::TreeListView;

			ListViewKeyDownHandler = gcnew KeyEventHandler(this, &WorkspaceViewOutlineView::ListView_KeyDown);
			ListViewItemActivateHandler = gcnew EventHandler(this, &WorkspaceViewOutlineView::ListView_ItemActivate);
			FormDeactivateHandler = gcnew EventHandler(this, &WorkspaceViewOutlineView::Form_Deactivate);

			ListView->View = View::Details;
			ListView->Dock = DockStyle::Fill;
			ListView->MultiSelect = false;
			ListView->BorderStyle = BorderStyle::None;
			ListView->Size = Size(285, 89);
			ListView->SmallImageList = gcnew ImageList();
			ListView->SmallImageList->Images->Add(Globals::ImageResources()->CreateImage("MessageListError"));
			ListView->SmallImageList->Images->Add(Globals::ImageResources()->CreateImage("OutlineViewVariables"));
			ListView->SmallImageList->Images->Add(Globals::ImageResources()->CreateImage("OutlineViewScriptBlock"));
			ListView->SmallImageList->Images->Add(Globals::ImageResources()->CreateImage("OutlineViewConditionalBlock"));
			ListView->SmallImageList->Images->Add(Globals::ImageResources()->CreateImage("OutlineViewLoopBlock"));
			ListView->Location = Point(0, 0);
			ListView->LabelEdit = false;
			ListView->CheckBoxes = false;
			ListView->FullRowSelect = true;
			ListView->GridLines = false;
			ListView->HeaderStyle = ColumnHeaderStyle::None;
			ListView->HideSelection = false;
			ListView->CanExpandGetter = gcnew BrightIdeasSoftware::TreeListView::CanExpandGetterDelegate(&WorkspaceViewOutlineView::ListViewCanExpandGetter);
			ListView->ChildrenGetter = gcnew BrightIdeasSoftware::TreeListView::ChildrenGetterDelegate(&WorkspaceViewOutlineView::ListViewChildrenGetter);

			BrightIdeasSoftware::OLVColumn^ Column = gcnew BrightIdeasSoftware::OLVColumn;
			Column->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&WorkspaceViewOutlineView::ListViewAspectGetter);
			Column->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&WorkspaceViewOutlineView::ListViewImageGetter);
			Column->Text = "Structure";
			Column->TextAlign = HorizontalAlignment::Center;
			Column->Width = 400;
			ListView->AllColumns->Add(Column);
			ListView->Columns->Add(Column);

			Form->FormBorderStyle = FormBorderStyle::SizableToolWindow;
			Form->StartPosition = FormStartPosition::Manual;
			Form->ShowInTaskbar = false;
			Form->ShowIcon = false;
			Form->ControlBox = false;
			Form->Controls->Add(ListView);

			Form->Size = Size(1, 1);
			Form->Show(Point(0, 0), IntPtr::Zero, false);
			Form->Hide(false);
			Form->Size = Size(400, 400);
			Form->MaximumSize = Size(400, 400);
			Form->MinimumSize = Size(400, 400);

			ListView->KeyDown += ListViewKeyDownHandler;
			ListView->ItemActivate += ListViewItemActivateHandler;
			Form->Deactivate += FormDeactivateHandler;
		}

		WorkspaceViewOutlineView::~WorkspaceViewOutlineView()
		{
			ResetState();

			ListView->KeyDown -= ListViewKeyDownHandler;
			ListView->ItemActivate -= ListViewItemActivateHandler;
			Form->Deactivate -= FormDeactivateHandler;

			for each (Image^ Itr in ListView->SmallImageList->Images)
				delete Itr;

			Form->ForceClose();

			SAFEDELETE_CLR(Form);
			SAFEDELETE_CLR(ListView);

			Parent = nullptr;
		}

		void WorkspaceViewOutlineView::Show(obScriptParsing::Structurizer^ Data, IWorkspaceModel^ Model)
		{
			Debug::Assert(AssociatedModel == nullptr && StructureData == nullptr);

			Point DisplayLocation = Point(Parent->EditorForm->Location.X + (Parent->EditorForm->Width - Form->Width) / 2,
										  Parent->EditorForm->Location.Y + (Parent->EditorForm->Height - Form->Height) / 2);

			Form->Location = DisplayLocation;

			AssociatedModel = Model;
			StructureData = Data;

			ListView->ClearObjects();
			ListView->SetObjects(StructureData->Output);
			Debug::Assert(ListView->GetItemCount() != 0);

	//		ListView->Columns[0]->AutoResize(ColumnHeaderAutoResizeStyle::ColumnContent);
			ListView->ExpandAll();
			if (StructureData->CurrentScope)
			{
				ListView->SelectObject(StructureData->CurrentScope, true);
				ListView->SelectedItem->EnsureVisible();
			}

			Form->Show(Parent->WindowHandle);
			Form->Focus();
		}

		void WorkspaceViewOutlineView::Hide()
		{
			ResetState();
			Form->Hide();
		}


		bool FindReplaceAllResults::GenericCanExpandGetter(Object^ E)
		{
			if (E->GetType() == FindReplaceAllResults::typeid)
				return true;
			else if (E->GetType() == FindReplaceAllResults::PerScriptData::typeid)
				return true;
			else
				return false;
		}

		Collections::IEnumerable^ FindReplaceAllResults::GenericChildrenGetter(Object^ E)
		{
			if (E->GetType() == FindReplaceAllResults::typeid)
			{
				auto TopItem = safe_cast<FindReplaceAllResults^>(E);
				return TopItem->ScriptsWithHits;
			}

			if (E->GetType() == FindReplaceAllResults::PerScriptData::typeid)
			{
				auto PerScript = safe_cast<FindReplaceAllResults::PerScriptData^>(E);
				return PerScript->Results->Hits;
			}

			return nullptr;
		}

		Object^ FindReplaceAllResults::TextAspectGetter(Object^ E)
		{
			if (E->GetType() == FindReplaceAllResults::typeid)
			{
				auto TopItem = safe_cast<FindReplaceAllResults^>(E);
				switch (TopItem->Operation)
				{
				case textEditors::IScriptTextEditor::FindReplaceOperation::Replace:
					return "Replace \"" + TopItem->Query + "\" with \"" + TopItem->Replacement + "\" (" + TopItem->TotalHitCount + " hits in " + TopItem->ScriptsWithHits->Count + " script(s))";
				default:
					return "Search \"" + TopItem->Query + "\" (" + TopItem->TotalHitCount + " hits in " + TopItem->ScriptsWithHits->Count + " script(s))";
				}
			}

			if (E->GetType() == FindReplaceAllResults::PerScriptData::typeid)
			{
				auto PerScript = safe_cast<FindReplaceAllResults::PerScriptData^>(E);
				return "Script " + PerScript->ParentDescription;
			}

			if (E->GetType() == textEditors::IScriptTextEditor::FindReplaceResult::HitData::typeid)
			{
				auto PerLine = safe_cast<textEditors::IScriptTextEditor::FindReplaceResult::HitData^>(E);
				return PerLine->Text;
			}

			return String::Empty;
		}

		System::Object^ FindReplaceAllResults::LineAspectGetter(Object^ E)
		{
			if (E->GetType() != textEditors::IScriptTextEditor::FindReplaceResult::HitData::typeid)
				return String::Empty;

			auto Model = safe_cast<textEditors::IScriptTextEditor::FindReplaceResult::HitData^>(E);
			return Model->Line;
		}

		System::Object^ FindReplaceAllResults::HitsAspectGetter(Object^ E)
		{
			if (E->GetType() == FindReplaceAllResults::PerScriptData::typeid)
			{
				auto PerScript = safe_cast<FindReplaceAllResults::PerScriptData^>(E);
				return PerScript->TotalHitCount;
			}

			if (E->GetType() == textEditors::IScriptTextEditor::FindReplaceResult::HitData::typeid)
			{
				auto PerLine = safe_cast<textEditors::IScriptTextEditor::FindReplaceResult::HitData^>(E);
				return PerLine->Hits;
			}

			return String::Empty;
		}

		ConcreteWorkspaceView::ConcreteWorkspaceView(ConcreteWorkspaceViewController^ Controller, ConcreteWorkspaceViewFactory^ Factory, Rectangle Bounds)
		{
			ViewController = Controller;
			ViewFactory = Factory;

			EditorFormCancelHandler = gcnew CancelEventHandler(this, &ConcreteWorkspaceView::EditorForm_Cancel);
			EditorFormKeyDownHandler = gcnew KeyEventHandler(this, &ConcreteWorkspaceView::EditorForm_KeyDown);
			EditorFormPositionChangedHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::EditorForm_PositionChanged);
			EditorFormSizeChangedHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::EditorForm_SizeChanged);
			EditorFormActivated = gcnew EventHandler(this, &ConcreteWorkspaceView::EditorForm_Activated);

			ScriptStripTabItemCloseHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabItemCloseEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_TabItemClose);
			ScriptStripSelectedTabChangedHandler = gcnew EventHandler<DotNetBar::SuperTabStripSelectedTabChangedEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_SelectedTabChanged);
			ScriptStripTabRemovedHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabRemovedEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_TabRemoved);
			ScriptStripMouseClickHandler = gcnew EventHandler<MouseEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_MouseClick);
			ScriptStripTabMovingHandler = gcnew EventHandler<DotNetBar::SuperTabStripTabMovingEventArgs^>(this, &ConcreteWorkspaceView::ScriptStrip_TabMoving);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ConcreteWorkspaceView::ScriptEditorPreferences_Saved);

			NavigationStackBackward = gcnew Stack < IWorkspaceModel^ > ;
			NavigationStackForward = gcnew Stack < IWorkspaceModel^ > ;
			FreezeNavigationStacks = false;

			EditorForm = gcnew AnimatedForm(false);
			EditorForm->SuspendLayout();

			EditorForm->FormBorderStyle = FormBorderStyle::Sizable;
			EditorForm->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			EditorForm->AutoScaleMode = AutoScaleMode::Font;
			EditorForm->Size = Size(Bounds.Width, Bounds.Height);
			EditorForm->KeyPreview = true;
			EditorForm->TabStop = false;

			DotNetBar::RibbonPredefinedColorSchemes::ChangeOffice2010ColorTable(EditorForm, DotNetBar::Rendering::eOffice2010ColorScheme::Black);

			EditorTabStrip = gcnew DotNetBar::SuperTabControl();
			EditorTabStrip->SuspendLayout();
			EditorTabStrip->Dock = DockStyle::Top;
			EditorTabStrip->MaximumSize = Size(32000, 26);
			EditorTabStrip->Location = Point(0, 0);
			EditorTabStrip->TabAlignment = DotNetBar::eTabStripAlignment::Top;
			EditorTabStrip->TabLayoutType = DotNetBar::eSuperTabLayoutType::SingleLine;
			EditorTabStrip->TextAlignment = DotNetBar::eItemAlignment::Far;
			EditorTabStrip->TabFont = gcnew Font(SystemFonts::MessageBoxFont->FontFamily->Name, 9, FontStyle::Bold);
			EditorTabStrip->SelectedTabFont = gcnew Font(SystemFonts::MessageBoxFont->FontFamily->Name, 9, FontStyle::Bold);
			EditorTabStrip->FixedTabSize = Size(0, 23);

			Color TabStripGradientColorStart = Color::FromArgb(255, 25, 25, 25);
			Color TabStripGradientColorEnd = Color::FromArgb(255, 25, 25, 25);

			EditorTabStrip->CloseButtonOnTabsVisible = true;
			EditorTabStrip->CloseButtonOnTabsAlwaysDisplayed = false;
			EditorTabStrip->AntiAlias = true;
			EditorTabStrip->TabStop = false;
			EditorTabStrip->ImageList = gcnew ImageList;
			EditorTabStrip->ImageList->Images->Add(Globals::ImageResources()->CreateImage("ModifiedFlagOff"));
			EditorTabStrip->ImageList->Images->Add(Globals::ImageResources()->CreateImage("ModifiedFlagOn"));
			EditorTabStrip->ImageList->ImageSize = Size(12, 12);
			EditorTabStrip->ReorderTabsEnabled = true;
			EditorTabStrip->TabStyle = DotNetBar::eSuperTabStyle::Office2010BackstageBlue;
			EditorTabStrip->TabStripColor->OuterBorder = TabStripGradientColorStart;
			EditorTabStrip->TabStripColor->InnerBorder = TabStripGradientColorStart;

			EditorTabStrip->TabStrip->Tag = this;

			NewTabButton = gcnew DotNetBar::ButtonItem();
			NewTabButton->Image = Globals::ImageResources()->CreateImage("NewTabButton");
			NewTabButton->ButtonStyle = DotNetBar::eButtonStyle::Default;
			NewTabButton->Style = DotNetBar::eDotNetBarStyle::Office2010;
			NewTabButton->ColorTable = DotNetBar::eButtonColor::Flat;
			NewTabButton->Text = "New Tab";
			NewTabButton->Tooltip = "New Tab";

			SortTabsButton = gcnew DotNetBar::ButtonItem();
			SortTabsButton->ButtonStyle = DotNetBar::eButtonStyle::TextOnlyAlways;
			SortTabsButton->Text = "Sort Tabs";
			SortTabsButton->Tooltip = "Sort Tabs";

			NewTabButton->SubItems->Add(SortTabsButton);
			EditorTabStrip->ControlBox->SubItems->Add(NewTabButton);
			EditorTabStrip->ControlBox->Visible = true;

			ConcreteWorkspaceViewDefineClickHandler(NewTabButton);
			ConcreteWorkspaceViewDefineClickHandler(SortTabsButton);

			EditorTabStrip->TabStripColor->Background = gcnew DotNetBar::Rendering::SuperTabLinearGradientColorTable(TabStripGradientColorStart,
																													 TabStripGradientColorEnd);

			EditorForm->HelpButton = false;
			EditorForm->Text = SCRIPTEDITOR_TITLE;

			AllowDisposal = false;
			DisallowBinding = false;

			ModelStateChangedHandler = gcnew IWorkspaceModel::StateChangeEventHandler(this, &ConcreteWorkspaceView::Model_StateChangeHandler);

			EditorForm->Closing += EditorFormCancelHandler;
			EditorForm->KeyDown += EditorFormKeyDownHandler;
			EditorForm->Move += EditorFormPositionChangedHandler;
			EditorForm->SizeChanged += EditorFormSizeChangedHandler;
			EditorForm->MaximizedBoundsChanged += EditorFormSizeChangedHandler;
			EditorForm->Activated += EditorFormActivated;
			EditorTabStrip->TabItemClose += ScriptStripTabItemCloseHandler;
			EditorTabStrip->SelectedTabChanged += ScriptStripSelectedTabChangedHandler;
			EditorTabStrip->TabRemoved += ScriptStripTabRemovedHandler;
			EditorTabStrip->TabStripMouseClick += ScriptStripMouseClickHandler;
			EditorTabStrip->TabMoving += ScriptStripTabMovingHandler;
			preferences::SettingsHolder::Get()->SavedToDisk += ScriptEditorPreferencesSavedHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(NewTabButton);
			ConcreteWorkspaceViewSubscribeClickEvent(SortTabsButton);

			WorkspaceSplitter = gcnew SplitContainer();
			WorkspaceSplitter->TabStop = false;
			WorkspaceSplitter->Panel1->TabStop = false;
			WorkspaceSplitter->Panel2->TabStop = false;

			MessageList = gcnew BrightIdeasSoftware::FastObjectListView();
			MessageList->TabStop = false;
			FindResultsList = gcnew BrightIdeasSoftware::FastObjectListView();
			FindResultsList->TabStop = false;
			BookmarkList = gcnew BrightIdeasSoftware::FastObjectListView();
			BookmarkList->TabStop = false;
			GlobalFindList = gcnew BrightIdeasSoftware::TreeListView;
			GlobalFindList->TabStop = false;
			SpoilerText = gcnew Label();

			ScriptAnnotationListItemActivate = gcnew EventHandler(this, &ConcreteWorkspaceView::ScriptAnnotationList_ItemActivate);
			GlobalFindListItemActivate = gcnew EventHandler(this, &ConcreteWorkspaceView::GlobalFindList_ItemActivate);
			BookmarksListButtonClick = gcnew EventHandler<BrightIdeasSoftware::CellClickEventArgs^>(this, &ConcreteWorkspaceView::BookmarksList_ButtonClick);

			AttachPanel = gcnew Panel();
			AttachPanel->Dock = DockStyle::Fill;
			AttachPanel->BorderStyle = BorderStyle::None;
			AttachPanel->TabStop = false;

			WorkspaceMainToolBar = gcnew ToolStrip();
			WorkspaceMainToolBar->TabStop = false;
			ToolBarNewScript = gcnew ToolStripButton();
			ToolBarOpenScript = gcnew ToolStripButton();
			ToolBarPreviousScript = gcnew ToolStripButton();
			ToolBarNextScript = gcnew ToolStripButton();
			ToolBarSaveScript = gcnew ToolStripSplitButton();
			ToolBarSaveScriptDropDown = gcnew ToolStripDropDown();
			ToolBarSaveScriptNoCompile = gcnew ToolStripButton();
			ToolBarSaveScriptAndPlugin = gcnew ToolStripButton();
			ToolBarRecompileScripts = gcnew ToolStripButton();
			ToolBarCompileDependencies = gcnew ToolStripButton();
			ToolBarDeleteScript = gcnew ToolStripButton();
			ToolBarNavigationBack = gcnew ToolStripButton();
			ToolBarNavigationForward = gcnew ToolStripButton();
			ToolBarSaveAll = gcnew ToolStripButton();
			ToolBarGlobalFindList = gcnew ToolStripButton;
			ToolBarOptions = gcnew ToolStripButton();

			ToolBarScriptType = gcnew ToolStripDropDownButton();
			ToolBarScriptTypeContents = gcnew ToolStripDropDown();
			ToolBarScriptTypeContentsObject = gcnew ToolStripButton();
			ToolBarScriptTypeContentsQuest = gcnew ToolStripButton();
			ToolBarScriptTypeContentsMagicEffect = gcnew ToolStripButton();

			ToolBarEditMenu = gcnew ToolStripDropDownButton();
			ToolBarEditMenuContents = gcnew ToolStripDropDown();
			ToolBarEditMenuContentsFindReplace = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoLine = gcnew ToolStripButton();
			ToolBarEditMenuContentsGotoOffset = gcnew ToolStripButton();

			ToolBarRefactorMenu = gcnew ToolStripDropDownButton();
			ToolBarRefactorMenuContents = gcnew ToolStripDropDown();
			ToolBarRefactorMenuContentsDocumentScript = gcnew ToolStripButton();
			ToolBarRefactorMenuContentsRenameVariables = gcnew ToolStripButton();
			ToolBarRefactorMenuContentsModifyVariableIndices = gcnew ToolStripButton();

			WorkspaceSecondaryToolBar = gcnew ToolStrip();
			WorkspaceSecondaryToolBar->TabStop = false;
			ToolBarMessageList = gcnew ToolStripButton();
			ToolBarFindList = gcnew ToolStripButton();
			ToolBarBookmarkList = gcnew ToolStripButton();
			ToolBarDumpScript = gcnew ToolStripSplitButton();
			ToolBarDumpScriptDropDown = gcnew ToolStripDropDown();
			ToolBarDumpAllScripts = gcnew ToolStripButton();
			ToolBarLoadScript = gcnew ToolStripSplitButton();
			ToolBarLoadScriptDropDown = gcnew ToolStripDropDown();
			ToolBarLoadScriptsToTabs = gcnew ToolStripButton();
			ToolBarShowOffsets = gcnew ToolStripButton();
			ToolBarShowPreprocessedText = gcnew ToolStripButton();
			ToolBarSanitizeScriptText = gcnew ToolStripButton();
			ToolBarBindScript = gcnew ToolStripButton();
			ToolBarSnippetManager = gcnew ToolStripButton();
			ToolBarSyncScriptsToDisk = gcnew ToolStripButton();
			ToolBarByteCodeSize = gcnew ToolStripProgressBar();

			FindReplaceBox = gcnew FindReplaceDialog(this);

			Color ForegroundColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
			Color BackgroundColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
			Color HighlightColor = Color::Maroon;
			Font^ CustomFont = safe_cast<Font^>(preferences::SettingsHolder::Get()->Appearance->TextFont->Clone());

			OffsetTextViewer = gcnew textEditors::ScriptOffsetViewer(CustomFont, ForegroundColor, BackgroundColor, HighlightColor, WorkspaceSplitter->Panel1);
			PreprocessorTextViewer = gcnew textEditors::SimpleTextViewer(CustomFont, ForegroundColor, BackgroundColor, HighlightColor, WorkspaceSplitter->Panel1);
			IntelliSenseView = gcnew intellisense::IntelliSenseInterfaceView;
			TabStripFilter = gcnew WorkspaceViewTabFilter(this);

			CachedFindReplaceAllResults = gcnew List < FindReplaceAllResults^ > ;
			OutlineView = gcnew WorkspaceViewOutlineView(this);

			ScopeCrumbBar = gcnew DotNetBar::CrumbBar();
			ScopeCrumbBar->AutoSize = false;
			ScopeCrumbBar->Dock = DockStyle::Top;
	//		ScopeCrumbBar->BackgroundStyle->BackColor = BackgroundColor;
	//		ScopeCrumbBar->BackgroundStyle->BorderColor = System::Drawing::Color::FromArgb(200, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B);
	//		ScopeCrumbBar->BackgroundStyle->BorderColor2 = System::Drawing::Color::FromArgb(200, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B);
			ScopeCrumbBar->BackgroundStyle->BorderBottom = DevComponents::DotNetBar::eStyleBorderType::DashDotDot;
			ScopeCrumbBar->BackgroundStyle->BorderBottomWidth = 2;
			ScopeCrumbBar->BackgroundStyle->BorderLeft = DevComponents::DotNetBar::eStyleBorderType::None;
			ScopeCrumbBar->BackgroundStyle->BorderRight = DevComponents::DotNetBar::eStyleBorderType::None;
			ScopeCrumbBar->BackgroundStyle->BorderTop = DevComponents::DotNetBar::eStyleBorderType::None;
			ScopeCrumbBar->FadeEffect = false;
			ScopeCrumbBar->ThemeAware = true;
			ScopeCrumbBar->Font = gcnew Font("Segoe UI", 9);
			ScopeCrumbBar->Margin = Padding(20, 0, 20, 0);
			ScopeCrumbBar->Padding = Padding(20, 0, 20, 0);
	//		ScopeCrumbBar->ResetBackgroundStyle();

			ScopeCrumbManager = gcnew textEditors::ScopeBreadcrumbManager(ScopeCrumbBar);

			SetupControlImage(ToolBarNewScript);
			SetupControlImage(ToolBarOpenScript);
			SetupControlImage(ToolBarPreviousScript);
			SetupControlImage(ToolBarNextScript);
			SetupControlImage(ToolBarSaveScript);
			SetupControlImage(ToolBarRecompileScripts);
			SetupControlImage(ToolBarCompileDependencies);
			SetupControlImage(ToolBarDeleteScript);
			SetupControlImage(ToolBarNavigationBack);
			SetupControlImage(ToolBarNavigationForward);
			SetupControlImage(ToolBarSaveAll);
			SetupControlImage(ToolBarGlobalFindList);
			SetupControlImage(ToolBarOptions);

			SetupControlImage(ToolBarScriptTypeContentsObject);
			SetupControlImage(ToolBarScriptTypeContentsQuest);
			SetupControlImage(ToolBarScriptTypeContentsMagicEffect);

			SetupControlImage(ToolBarEditMenu);
			SetupControlImage(ToolBarEditMenuContentsFindReplace);
			SetupControlImage(ToolBarEditMenuContentsGotoLine);
			SetupControlImage(ToolBarEditMenuContentsGotoOffset);

			SetupControlImage(ToolBarRefactorMenu);
			SetupControlImage(ToolBarRefactorMenuContentsDocumentScript);
			SetupControlImage(ToolBarRefactorMenuContentsRenameVariables);
			SetupControlImage(ToolBarRefactorMenuContentsModifyVariableIndices);

			SetupControlImage(ToolBarMessageList);
			SetupControlImage(ToolBarFindList);
			SetupControlImage(ToolBarBookmarkList);
			SetupControlImage(ToolBarDumpScript);
			SetupControlImage(ToolBarLoadScript);
			SetupControlImage(ToolBarShowOffsets);
			SetupControlImage(ToolBarShowPreprocessedText);
			SetupControlImage(ToolBarSanitizeScriptText);
			SetupControlImage(ToolBarBindScript);
			SetupControlImage(ToolBarSnippetManager);
			SetupControlImage(ToolBarSyncScriptsToDisk);

			ConcreteWorkspaceViewDefineClickHandler(NewTabButton);
			ConcreteWorkspaceViewDefineClickHandler(SortTabsButton);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarNavigationBack);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarNavigationForward);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveAll);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarGlobalFindList);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarOptions);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarDumpAllScripts);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarLoadScriptsToTabs);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarMessageList);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarFindList);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarBookmarkList);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarNewScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarOpenScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarPreviousScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarNextScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveScriptNoCompile);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSaveScriptAndPlugin);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarRecompileScripts);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarCompileDependencies);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarDeleteScript);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarScriptTypeContentsObject);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarScriptTypeContentsQuest);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarScriptTypeContentsMagicEffect);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarEditMenuContentsFindReplace);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarEditMenuContentsGotoLine);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarEditMenuContentsGotoOffset);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarRefactorMenuContentsDocumentScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarRefactorMenuContentsRenameVariables);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarRefactorMenuContentsModifyVariableIndices);

			ConcreteWorkspaceViewDefineClickHandler(ToolBarDumpScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarLoadScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarShowOffsets);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarShowPreprocessedText);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSanitizeScriptText);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarBindScript);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSnippetManager);
			ConcreteWorkspaceViewDefineClickHandler(ToolBarSyncScriptsToDisk);

			Padding ToolBarButtonPaddingLarge = Padding(16, 0, 16, 0);
			Padding ToolBarButtonPaddingRegular = Padding(10, 0, 10, 0);

			ToolStripStatusLabel^ ToolBarSpacerA = gcnew ToolStripStatusLabel();
			ToolBarSpacerA->Spring = true;

			WorkspaceSplitter->Dock = DockStyle::Fill;
			WorkspaceSplitter->SplitterWidth = 5;
			WorkspaceSplitter->Orientation = Orientation::Horizontal;

			WorkspaceMainToolBar->GripStyle = ToolStripGripStyle::Hidden;

			ToolBarNewScript->ToolTipText = "New Script";
			ToolBarNewScript->AutoSize = true;
			ToolBarNewScript->Margin = Padding(0, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarOpenScript->ToolTipText = "Open Script";
			ToolBarOpenScript->AutoSize = true;
			ToolBarOpenScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarSaveScript->ToolTipText = "Save And Compile Script";
			ToolBarSaveScript->AutoSize = true;
			ToolBarSaveScript->Margin = ToolBarButtonPaddingLarge;
			ToolBarSaveScriptNoCompile->ToolTipText = "Save But Do Not Compile Script";
			ToolBarSaveScriptNoCompile->Text = "Save But Do Not Compile Script";
			ToolBarSaveScriptNoCompile->AutoSize = true;
			ToolBarSaveScriptAndPlugin->Text = "Save Script And Active Plugin";
			ToolBarSaveScriptAndPlugin->ToolTipText = "Save Script And Active Plugin";
			ToolBarSaveScriptAndPlugin->AutoSize = true;
			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptNoCompile);
			ToolBarSaveScriptDropDown->Items->Add(ToolBarSaveScriptAndPlugin);
			ToolBarSaveScript->DropDown = ToolBarSaveScriptDropDown;

			ToolBarPreviousScript->ToolTipText = "Previous Script";
			ToolBarPreviousScript->AutoSize = true;
			ToolBarPreviousScript->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarNextScript->ToolTipText = "Next Script";
			ToolBarNextScript->AutoSize = true;
			ToolBarNextScript->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarRecompileScripts->ToolTipText = "Recompile Active Scripts";
			ToolBarRecompileScripts->AutoSize = true;
			ToolBarRecompileScripts->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarCompileDependencies->ToolTipText = "Recompile Script Dependencies";
			ToolBarCompileDependencies->AutoSize = true;
			ToolBarCompileDependencies->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarDeleteScript->ToolTipText = "Delete Script";
			ToolBarDeleteScript->AutoSize = true;
			ToolBarDeleteScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarGlobalFindList->ToolTipText = "Global Find Result List";
			ToolBarGlobalFindList->AutoSize = true;
			ToolBarGlobalFindList->Margin = ToolBarButtonPaddingRegular;

			ToolBarSaveAll->ToolTipText = "Save All Open Scripts";
			ToolBarSaveAll->AutoSize = true;
			ToolBarSaveAll->Margin = ToolBarButtonPaddingLarge;
			ToolBarSaveAll->Alignment = ToolStripItemAlignment::Right;

			ToolBarNavigationBack->ToolTipText = "Navigate Backward";
			ToolBarNavigationBack->AutoSize = true;
			ToolBarNavigationBack->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);
			ToolBarNavigationBack->Alignment = ToolStripItemAlignment::Right;

			ToolBarNavigationForward->ToolTipText = "Navigate Forward";
			ToolBarNavigationForward->AutoSize = true;
			ToolBarNavigationForward->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);
			ToolBarNavigationForward->Alignment = ToolStripItemAlignment::Right;

			ToolBarOptions->ToolTipText = "Preferences";
			ToolBarOptions->Alignment = ToolStripItemAlignment::Right;
			ToolBarOptions->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, 0, 0);

			ToolBarMessageList->ToolTipText = "Messages";
			ToolBarMessageList->AutoSize = true;
			ToolBarMessageList->Margin = Padding(0, 0, ToolBarButtonPaddingRegular.Right, 0);
			ToolBarMessageList->Font = gcnew Font(SystemFonts::DefaultFont->FontFamily, 9.25, FontStyle::Bold);
			ToolBarMessageList->TextAlign = ContentAlignment::MiddleCenter;
			ToolBarMessageList->ImageAlign = ContentAlignment::MiddleLeft;

			ToolBarFindList->ToolTipText = "Find/Replace Results";
			ToolBarFindList->AutoSize = true;
			ToolBarFindList->Margin = ToolBarButtonPaddingRegular;
			ToolBarFindList->Font = gcnew Font(SystemFonts::DefaultFont->FontFamily, 9.25, FontStyle::Bold);
			ToolBarFindList->TextAlign = ContentAlignment::MiddleCenter;
			ToolBarFindList->ImageAlign = ContentAlignment::MiddleLeft;

			ToolBarBookmarkList->ToolTipText = "Bookmarks";
			ToolBarBookmarkList->AutoSize = true;
			ToolBarBookmarkList->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);
			ToolBarBookmarkList->Font = gcnew Font(SystemFonts::DefaultFont->FontFamily, 9.25, FontStyle::Bold);
			ToolBarBookmarkList->TextAlign = ContentAlignment::MiddleCenter;
			ToolBarBookmarkList->ImageAlign = ContentAlignment::MiddleLeft;

			ToolBarDumpScript->ToolTipText = "Dump Script";
			ToolBarDumpScript->AutoSize = true;
			ToolBarDumpScript->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);
			ToolBarDumpAllScripts->ToolTipText = "Dump All Tabs";
			ToolBarDumpAllScripts->Text = "Dump All Tabs";
			ToolBarDumpAllScripts->AutoSize = true;
			ToolBarDumpScriptDropDown->Items->Add(ToolBarDumpAllScripts);
			ToolBarDumpScript->DropDown = ToolBarDumpScriptDropDown;

			ToolBarLoadScript->ToolTipText = "Load Script";
			ToolBarLoadScript->AutoSize = true;
			ToolBarLoadScript->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);
			ToolBarLoadScriptsToTabs->Text = "Load Multiple Scripts Into Tabs";
			ToolBarLoadScriptsToTabs->ToolTipText = "Load Multiple Scripts Into Tabs";
			ToolBarLoadScriptsToTabs->AutoSize = true;
			ToolBarLoadScriptDropDown->Items->Add(ToolBarLoadScriptsToTabs);
			ToolBarLoadScript->DropDown = ToolBarLoadScriptDropDown;

			ToolBarShowOffsets->ToolTipText = "Toggle Offset Viewer";
			ToolBarShowOffsets->AutoSize = true;
			ToolBarShowOffsets->Margin = Padding(ToolBarButtonPaddingLarge.Left, 0, ToolBarButtonPaddingRegular.Right, 0);

			ToolBarShowPreprocessedText->ToolTipText = "Toggle Preprocessed Text Viewer";
			ToolBarShowPreprocessedText->AutoSize = true;
			ToolBarShowPreprocessedText->Margin = Padding(ToolBarButtonPaddingRegular.Left, 0, ToolBarButtonPaddingLarge.Right, 0);

			ToolBarSanitizeScriptText->ToolTipText = "Sanitize Script Text";
			ToolBarSanitizeScriptText->AutoSize = true;
			ToolBarSanitizeScriptText->Margin = ToolBarButtonPaddingLarge;

			ToolBarBindScript->ToolTipText = "Bind Script";
			ToolBarBindScript->AutoSize = true;
			ToolBarBindScript->Margin = ToolBarButtonPaddingLarge;

			ToolBarSnippetManager->ToolTipText = "Code Snippet Manager";
			ToolBarSnippetManager->AutoSize = true;
			ToolBarSnippetManager->Margin = ToolBarButtonPaddingLarge;

			ToolBarSyncScriptsToDisk->ToolTipText = "Sync Scripts To Disk";
			ToolBarSyncScriptsToDisk->AutoSize = true;
			ToolBarSyncScriptsToDisk->Margin = ToolBarButtonPaddingLarge;

			ToolBarByteCodeSize->Minimum = 0;
			ToolBarByteCodeSize->Maximum = 0x8000;
			ToolBarByteCodeSize->AutoSize = false;
			ToolBarByteCodeSize->Size = Size(125, 14);
			ToolBarByteCodeSize->ToolTipText = "Compiled Script Size";
			ToolBarByteCodeSize->Alignment = ToolStripItemAlignment::Left;
			ToolBarByteCodeSize->Margin = Padding(8, 0, 8, 0);

			ToolBarScriptTypeContentsObject->Text = "Object                   ";
			ToolBarScriptTypeContentsObject->ToolTipText = "Object";
			ToolBarScriptTypeContentsQuest->Text = "Quest                    ";
			ToolBarScriptTypeContentsQuest->ToolTipText = "Quest";
			ToolBarScriptTypeContentsMagicEffect->Text = "Magic Effect        ";
			ToolBarScriptTypeContentsMagicEffect->ToolTipText = "Magic Effect";
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsObject);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsQuest);
			ToolBarScriptTypeContents->Items->Add(ToolBarScriptTypeContentsMagicEffect);
			ToolBarScriptType->DropDown = ToolBarScriptTypeContents;
			ToolBarScriptType->Alignment = ToolStripItemAlignment::Right;
			ToolBarScriptType->Overflow = ToolStripItemOverflow::Never;
		//	ToolBarScriptType->Padding = ToolBarButtonPaddingLarge;

			UpdateScriptTypeControls(IWorkspaceModel::ScriptType::Object);

			ToolBarEditMenuContentsFindReplace->Text = "Find/Replace";
			ToolBarEditMenuContentsGotoLine->Text = "Goto Line";
			ToolBarEditMenuContentsGotoOffset->Text = "Goto Offset";
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsFindReplace);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoLine);
			ToolBarEditMenuContents->Items->Add(ToolBarEditMenuContentsGotoOffset);
			ToolBarEditMenu->Text = "Edit";
			ToolBarEditMenu->DropDown = ToolBarEditMenuContents;
			ToolBarEditMenu->Padding = Padding(0);
			ToolBarEditMenu->Alignment = ToolStripItemAlignment::Right;
			ToolBarEditMenu->Overflow = ToolStripItemOverflow::Never;

			ToolBarRefactorMenuContentsDocumentScript->Text = "Document Script";
			ToolBarRefactorMenuContentsRenameVariables->Text = "Rename Variables";
			ToolBarRefactorMenuContentsModifyVariableIndices->Text = "Modify Variable Indices";
			ToolBarRefactorMenuContents->Items->Add(ToolBarRefactorMenuContentsDocumentScript);
			ToolBarRefactorMenuContents->Items->Add(ToolBarRefactorMenuContentsRenameVariables);
			ToolBarRefactorMenuContents->Items->Add(ToolBarRefactorMenuContentsModifyVariableIndices);
			ToolBarRefactorMenu->Text = "Refactor";
			ToolBarRefactorMenu->DropDown = ToolBarRefactorMenuContents;
			ToolBarRefactorMenu->Margin = Padding(20, 0, 3, 0);
			ToolBarRefactorMenu->Alignment = ToolStripItemAlignment::Right;
			ToolBarRefactorMenu->Overflow = ToolStripItemOverflow::Never;

			WorkspaceMainToolBar->Dock = DockStyle::Top;
			WorkspaceMainToolBar->Items->Add(ToolBarNewScript);
			WorkspaceMainToolBar->Items->Add(ToolBarOpenScript);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveScript);
			WorkspaceMainToolBar->Items->Add(ToolBarPreviousScript);
			WorkspaceMainToolBar->Items->Add(ToolBarNextScript);
			WorkspaceMainToolBar->Items->Add(ToolBarRecompileScripts);
			WorkspaceMainToolBar->Items->Add(ToolBarCompileDependencies);
			WorkspaceMainToolBar->Items->Add(ToolBarDeleteScript);
			WorkspaceMainToolBar->Items->Add(ToolBarSpacerA);
			WorkspaceMainToolBar->Items->Add(ToolBarOptions);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationForward);
			WorkspaceMainToolBar->Items->Add(ToolBarNavigationBack);
			WorkspaceMainToolBar->Items->Add(ToolBarSaveAll);
			WorkspaceMainToolBar->ShowItemToolTips = true;

			WorkspaceSecondaryToolBar->GripStyle = ToolStripGripStyle::Hidden;
			WorkspaceSecondaryToolBar->Dock = DockStyle::Bottom;
			WorkspaceSecondaryToolBar->Items->Add(ToolBarMessageList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarFindList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarGlobalFindList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBookmarkList);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarDumpScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarLoadScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowOffsets);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarShowPreprocessedText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSanitizeScriptText);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarBindScript);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSnippetManager);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarSyncScriptsToDisk);
			WorkspaceSecondaryToolBar->Items->Add(gcnew ToolStripSeparator());
			WorkspaceSecondaryToolBar->Items->Add(ToolBarByteCodeSize);
			WorkspaceSecondaryToolBar->Items->Add(gcnew ToolStripSeparator());
			WorkspaceSecondaryToolBar->Items->Add(ToolBarEditMenu);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarRefactorMenu);
			WorkspaceSecondaryToolBar->Items->Add(ToolBarScriptType);
			WorkspaceSecondaryToolBar->ShowItemToolTips = true;

			SpoilerText->Dock = DockStyle::Fill;
			SpoilerText->TextAlign = ContentAlignment::MiddleCenter;
			SpoilerText->Text = "Right, everybody out! Smash the Spinning Jenny! Burn the rolling Rosalind! Destroy the going-up-and-down-a-bit-and-then-moving-along Gertrude! And death to the stupid Prince who grows fat on the profits!";

			auto EmpytListFont = gcnew Font("Segoe UI Light", 16);

			MessageList->Dock = DockStyle::Fill;
			MessageList->BorderStyle = BorderStyle::Fixed3D;
			MessageList->Visible = false;
			MessageList->View = View::Details;
			MessageList->MultiSelect = false;
			MessageList->CheckBoxes = false;
			MessageList->FullRowSelect = true;
			MessageList->HideSelection = false;
			MessageList->ShowGroups = false;	// ### disabled as it kinda breaks sorting
			MessageList->GridLines = true;
			MessageList->UseAlternatingBackColors = false;
			MessageList->IsSearchOnSortColumn = true;
			MessageList->ItemActivate += ScriptAnnotationListItemActivate;
			MessageList->EmptyListMsg = "Doesn't look like anything to me...";
			MessageList->EmptyListMsgFont = EmpytListFont;
			MessageList->SmallImageList = gcnew ImageList;
			ScriptDiagnosticMessage::PopulateImageListWithMessageTypeImages(MessageList->SmallImageList);

			auto MessageListColumnType = gcnew BrightIdeasSoftware::OLVColumn;
			MessageListColumnType->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::MessageListTypeAspectGetter);
			MessageListColumnType->AspectToStringConverter = gcnew BrightIdeasSoftware::AspectToStringConverterDelegate(&ConcreteWorkspaceView::MessageListTypeAspectToStringConverter);
			MessageListColumnType->ImageGetter = gcnew BrightIdeasSoftware::ImageGetterDelegate(&ConcreteWorkspaceView::MessageListTypeImageGetter);
			MessageListColumnType->GroupKeyGetter = gcnew BrightIdeasSoftware::GroupKeyGetterDelegate(&ConcreteWorkspaceView::MessageListTypeGroupKeyGetter);
			MessageListColumnType->GroupKeyToTitleConverter  = gcnew BrightIdeasSoftware::GroupKeyToTitleConverterDelegate(&ConcreteWorkspaceView::MessageListTypeGroupKeyToTitleConverter);
			MessageListColumnType->Text = "Type";
			MessageListColumnType->Width = 40;
			MessageListColumnType->MinimumWidth = 40;
			MessageListColumnType->MaximumWidth = 40;
			MessageListColumnType->Groupable = true;
			MessageListColumnType->UseInitialLetterForGroup = false;

			auto MessageListColumnLine = gcnew BrightIdeasSoftware::OLVColumn;
			MessageListColumnLine->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::ScriptTextAnnotationListLineNumberAspectGetter);
			MessageListColumnLine->Text = "Line";
			MessageListColumnLine->MinimumWidth = 30;
			MessageListColumnLine->Width = 40;
			MessageListColumnLine->Groupable = false;

			auto MessageListColumnText = gcnew BrightIdeasSoftware::OLVColumn;
			MessageListColumnText->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::ScriptTextAnnotationListTextAspectGetter);
			MessageListColumnText->Text = "Message";
			MessageListColumnText->MinimumWidth = 600;
			MessageListColumnText->Groupable = false;

			auto MessageListColumnSource = gcnew BrightIdeasSoftware::OLVColumn;
			MessageListColumnSource->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::MessageListSourceAspectGetter);
			MessageListColumnSource->GroupKeyGetter = gcnew BrightIdeasSoftware::GroupKeyGetterDelegate(&ConcreteWorkspaceView::MessageListSourceGroupKeyGetter);
			MessageListColumnSource->GroupKeyToTitleConverter  = gcnew BrightIdeasSoftware::GroupKeyToTitleConverterDelegate(&ConcreteWorkspaceView::MessageListSourceGroupKeyToTitleConverter);
			MessageListColumnSource->Text = "Message Source";
			MessageListColumnSource->MinimumWidth = 100;
			MessageListColumnSource->Groupable = true;

			MessageList->AllColumns->Add(MessageListColumnType);
			MessageList->Columns->Add(MessageListColumnType);
			MessageList->AllColumns->Add(MessageListColumnLine);
			MessageList->Columns->Add(MessageListColumnLine);
			MessageList->AllColumns->Add(MessageListColumnText);
			MessageList->Columns->Add(MessageListColumnText);
			MessageList->AllColumns->Add(MessageListColumnSource);
			MessageList->Columns->Add(MessageListColumnSource);

			FindResultsList->Dock = DockStyle::Fill;
			FindResultsList->BorderStyle = BorderStyle::Fixed3D;
			FindResultsList->Visible = false;
			FindResultsList->View = View::Details;
			FindResultsList->MultiSelect = false;
			FindResultsList->CheckBoxes = false;
			FindResultsList->FullRowSelect = true;
			FindResultsList->HideSelection = false;
			FindResultsList->GridLines = true;
			FindResultsList->UseAlternatingBackColors = false;
			FindResultsList->IsSearchOnSortColumn = true;
			FindResultsList->ItemActivate += ScriptAnnotationListItemActivate;
			FindResultsList->EmptyListMsg = "Doesn't look like anything to me...";
			FindResultsList->EmptyListMsgFont = EmpytListFont;

			auto FindResultsListColumnLine = gcnew BrightIdeasSoftware::OLVColumn;
			FindResultsListColumnLine->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::ScriptTextAnnotationListLineNumberAspectGetter);
			FindResultsListColumnLine->Text = "Line";
			FindResultsListColumnLine->MinimumWidth = 16;

			auto FindResultsListColumnText = gcnew BrightIdeasSoftware::OLVColumn;
			FindResultsListColumnText->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::ScriptTextAnnotationListTextAspectGetter);
			FindResultsListColumnText->Text = "Code";
			FindResultsListColumnText->MinimumWidth = 250;

			auto FindResultsListColumnHits = gcnew BrightIdeasSoftware::OLVColumn;
			FindResultsListColumnHits->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::FindResultsListHitsAspectGetter);
			FindResultsListColumnHits->Text = "Hits";
			FindResultsListColumnHits->MinimumWidth = 20;

			FindResultsList->AllColumns->Add(FindResultsListColumnLine);
			FindResultsList->Columns->Add(FindResultsListColumnLine);
			FindResultsList->AllColumns->Add(FindResultsListColumnText);
			FindResultsList->Columns->Add(FindResultsListColumnText);
			FindResultsList->AllColumns->Add(FindResultsListColumnHits);
			FindResultsList->Columns->Add(FindResultsListColumnHits);

			BookmarkList->Dock = DockStyle::Fill;
			BookmarkList->BorderStyle = BorderStyle::Fixed3D;
			BookmarkList->Visible = false;
			BookmarkList->View = View::Details;
			BookmarkList->MultiSelect = false;
			BookmarkList->CheckBoxes = false;
			BookmarkList->FullRowSelect = true;
			BookmarkList->HideSelection = false;
			BookmarkList->IsSearchOnSortColumn = true;
			BookmarkList->GridLines = true;
			BookmarkList->UseAlternatingBackColors = false;
			BookmarkList->ItemActivate += ScriptAnnotationListItemActivate;
			BookmarkList->ButtonClick += BookmarksListButtonClick;
			BookmarkList->EmptyListMsg = "Doesn't look like anything to me...";
			BookmarkList->EmptyListMsgFont = EmpytListFont;

			auto BookmarksListColumnLine = gcnew BrightIdeasSoftware::OLVColumn;
			BookmarksListColumnLine->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::ScriptTextAnnotationListLineNumberAspectGetter);
			BookmarksListColumnLine->Text = "Line";
			BookmarksListColumnLine->MinimumWidth = 16;

			auto BookmarksListColumnText = gcnew BrightIdeasSoftware::OLVColumn;
			BookmarksListColumnText->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::ScriptTextAnnotationListTextAspectGetter);
			BookmarksListColumnText->Text = "Description";
			BookmarksListColumnText->MinimumWidth = 250;

			auto BookmarksListColumnAction = gcnew BrightIdeasSoftware::OLVColumn;
			BookmarksListColumnAction->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&ConcreteWorkspaceView::BookmarksListActionAspectGetter);
			BookmarksListColumnAction->Text = "Action";
			BookmarksListColumnAction->MinimumWidth = 75;
			BookmarksListColumnAction->MaximumWidth = 75;
			BookmarksListColumnAction->CellVerticalAlignment = StringAlignment::Center;
			BookmarksListColumnAction->TextAlign = HorizontalAlignment::Center;
			BookmarksListColumnAction->IsButton = true;
			BookmarksListColumnAction->ButtonSizing = BrightIdeasSoftware::OLVColumn::ButtonSizingMode::TextBounds;
			BookmarksListColumnAction->ButtonPadding = Size(3, 1.5);

			BookmarkList->AllColumns->Add(BookmarksListColumnLine);
			BookmarkList->Columns->Add(BookmarksListColumnLine);
			BookmarkList->AllColumns->Add(BookmarksListColumnText);
			BookmarkList->Columns->Add(BookmarksListColumnText);
			BookmarkList->AllColumns->Add(BookmarksListColumnAction);
			BookmarkList->Columns->Add(BookmarksListColumnAction);

			GlobalFindList->Dock = DockStyle::Fill;
			GlobalFindList->BorderStyle = BorderStyle::Fixed3D;
			GlobalFindList->Visible = false;
			GlobalFindList->View = View::Details;
			GlobalFindList->MultiSelect = false;
			GlobalFindList->CheckBoxes = false;
			GlobalFindList->FullRowSelect = true;
			GlobalFindList->HideSelection = false;
			GlobalFindList->LabelEdit = false;
			GlobalFindList->GridLines = false;
			GlobalFindList->HeaderStyle = ColumnHeaderStyle::Nonclickable;
			GlobalFindList->CanExpandGetter = gcnew BrightIdeasSoftware::TreeListView::CanExpandGetterDelegate(&FindReplaceAllResults::GenericCanExpandGetter);
			GlobalFindList->ChildrenGetter = gcnew BrightIdeasSoftware::TreeListView::ChildrenGetterDelegate(&FindReplaceAllResults::GenericChildrenGetter);
			GlobalFindList->ItemActivate += GlobalFindListItemActivate;
			GlobalFindList->EmptyListMsg = "Doesn't look like anything to me...";
			GlobalFindList->EmptyListMsgFont = EmpytListFont;

			auto GlobalFindListColumnText = gcnew BrightIdeasSoftware::OLVColumn;
			GlobalFindListColumnText->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&FindReplaceAllResults::TextAspectGetter);
			GlobalFindListColumnText->Text = "Code";
			GlobalFindListColumnText->Width = 500;

			auto GlobalFindListColumnLine = gcnew BrightIdeasSoftware::OLVColumn;
			GlobalFindListColumnLine->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&FindReplaceAllResults::LineAspectGetter);
			GlobalFindListColumnLine->Text = "Line";
			GlobalFindListColumnLine->MinimumWidth = 20;

			auto GlobalFindListColumnHits = gcnew BrightIdeasSoftware::OLVColumn;
			GlobalFindListColumnHits->AspectGetter = gcnew BrightIdeasSoftware::AspectGetterDelegate(&FindReplaceAllResults::HitsAspectGetter);
			GlobalFindListColumnHits->Text = "Hits";
			GlobalFindListColumnHits->MinimumWidth = 20;


			GlobalFindList->Columns->Add(GlobalFindListColumnText);
			GlobalFindList->Columns->Add(GlobalFindListColumnLine);
			GlobalFindList->Columns->Add(GlobalFindListColumnHits);
			GlobalFindList->AllColumns->Add(GlobalFindListColumnText);
			GlobalFindList->AllColumns->Add(GlobalFindListColumnLine);
			GlobalFindList->AllColumns->Add(GlobalFindListColumnHits);

			// ideally, the main toolbar should be the main form's child but that screws with the tab strip's layout
			WorkspaceSplitter->Panel1->Controls->Add(AttachPanel);
			WorkspaceSplitter->Panel1->Controls->Add(ScopeCrumbBar);
			WorkspaceSplitter->Panel1->Controls->Add(WorkspaceMainToolBar);

			WorkspaceSplitter->Panel2->Controls->Add(WorkspaceSecondaryToolBar);
			WorkspaceSplitter->Panel2->Controls->Add(MessageList);
			WorkspaceSplitter->Panel2->Controls->Add(FindResultsList);
			WorkspaceSplitter->Panel2->Controls->Add(BookmarkList);
			WorkspaceSplitter->Panel2->Controls->Add(GlobalFindList);
			WorkspaceSplitter->Panel2->Controls->Add(SpoilerText);

			EditorForm->Controls->Add(WorkspaceSplitter);
			EditorForm->Controls->Add(EditorTabStrip);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNewScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarOpenScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarPreviousScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNextScript);
			ToolBarSaveScript->ButtonClick += ToolBarSaveScriptClickHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSaveScriptNoCompile);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSaveScriptAndPlugin);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRecompileScripts);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarDeleteScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarCompileDependencies);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNavigationBack);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarNavigationForward);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSaveAll);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarGlobalFindList);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarOptions);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarEditMenuContentsFindReplace);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarEditMenuContentsGotoLine);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarEditMenuContentsGotoOffset);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarScriptTypeContentsObject);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarScriptTypeContentsQuest);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarScriptTypeContentsMagicEffect);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRefactorMenuContentsDocumentScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRefactorMenuContentsRenameVariables);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarRefactorMenuContentsModifyVariableIndices);

			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarMessageList);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarFindList);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarBookmarkList);

			ToolBarDumpScript->ButtonClick += ToolBarDumpScriptClickHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarDumpAllScripts);
			ToolBarLoadScript->ButtonClick += ToolBarLoadScriptClickHandler;
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarLoadScriptsToTabs);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarShowOffsets);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarShowPreprocessedText);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSanitizeScriptText);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarBindScript);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSnippetManager);
			ConcreteWorkspaceViewSubscribeClickEvent(ToolBarSyncScriptsToDisk);

			if (preferences::SettingsHolder::Get()->General->HideInTaskbar)
			{
				EditorForm->ShowInTaskbar = false;
				EditorForm->Show(safe_cast<IntPtr>(nativeWrapper::g_CSEInterfaceTable->EditorAPI.GetMainWindowHandle()));
			}
			else
				EditorForm->Show();

			Enabled = false;

			EditorForm->Location = Point(Bounds.Left, Bounds.Top);
			EditorTabStrip->ResumeLayout();
			EditorForm->ResumeLayout();

			EditorForm->Focus();
			EditorForm->BringToFront();

			try { WorkspaceSplitter->SplitterDistance = GetBounds(true).Height; }
			catch (...) {}

			AssociatedModels = gcnew ModelTabTableT;

			// needs to be deferred until the form has been init'ed/layout is complete
			// otherwise the breadcrumb bar turns up above the tab control
			ScopeCrumbManager->Visible = preferences::SettingsHolder::Get()->Appearance->ShowScopeBar;
		}

		ConcreteWorkspaceView::~ConcreteWorkspaceView()
		{
			CachedFindReplaceAllResults->Clear();

			NavigationStackBackward->Clear();
			NavigationStackForward->Clear();
			FreezeNavigationStacks = true;

			for each (auto Itr in AssociatedModels)
			{
				Itr.Value->Tag = nullptr;
				ModelUnsubscribeEvents(Itr.Key);
				delete Itr.Key;
			}

			AssociatedModels->Clear();

			SAFEDELETE_CLR(ModelStateChangedHandler);

			for each (Image^ Itr in EditorTabStrip->ImageList->Images)
				delete Itr;

			EditorTabStrip->ImageList->Images->Clear();
			EditorTabStrip->ImageList = nullptr;
			EditorTabStrip->TabStrip->Tag = nullptr;
			EditorTabStrip->Tabs->Clear();

			EditorForm->Closing -= EditorFormCancelHandler;
			EditorForm->KeyDown -= EditorFormKeyDownHandler;
			EditorForm->Move -= EditorFormPositionChangedHandler;
			EditorForm->SizeChanged -= EditorFormSizeChangedHandler;
			EditorForm->MaximizedBoundsChanged -= EditorFormSizeChangedHandler;
			EditorForm->Activated -= EditorFormActivated;
			EditorTabStrip->TabItemClose -= ScriptStripTabItemCloseHandler;
			EditorTabStrip->SelectedTabChanged -= ScriptStripSelectedTabChangedHandler;
			EditorTabStrip->TabRemoved -= ScriptStripTabRemovedHandler;
			EditorTabStrip->TabStripMouseClick -= ScriptStripMouseClickHandler;
			EditorTabStrip->TabMoving -= ScriptStripTabMovingHandler;
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(NewTabButton);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(SortTabsButton);
			preferences::SettingsHolder::Get()->SavedToDisk -= ScriptEditorPreferencesSavedHandler;

			SAFEDELETE_CLR(EditorFormCancelHandler);
			SAFEDELETE_CLR(EditorFormKeyDownHandler);
			SAFEDELETE_CLR(EditorFormPositionChangedHandler);
			SAFEDELETE_CLR(EditorFormSizeChangedHandler);
			SAFEDELETE_CLR(EditorFormActivated);
			SAFEDELETE_CLR(ScriptStripTabItemCloseHandler);
			SAFEDELETE_CLR(ScriptStripSelectedTabChangedHandler);
			SAFEDELETE_CLR(ScriptStripTabRemovedHandler);
			SAFEDELETE_CLR(ScriptStripMouseClickHandler);
			SAFEDELETE_CLR(ScriptStripTabMovingHandler);
			SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);

			SAFEDELETE_CLR(EditorTabStrip);
			SAFEDELETE_CLR(NewTabButton->Image);
			SAFEDELETE_CLR(NewTabButton);
			SAFEDELETE_CLR(SortTabsButton);

			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarNewScript);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarOpenScript);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarPreviousScript);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarNextScript);
			ToolBarSaveScript->ButtonClick -= ToolBarSaveScriptClickHandler;
			SAFEDELETE_CLR(ToolBarSaveScriptClickHandler);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarSaveScriptNoCompile);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarSaveScriptAndPlugin);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarRecompileScripts);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarDeleteScript);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarCompileDependencies);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarNavigationBack);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarNavigationForward);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarSaveAll);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarGlobalFindList);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarOptions);

			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarEditMenuContentsFindReplace);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarEditMenuContentsGotoLine);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarEditMenuContentsGotoOffset);

			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarScriptTypeContentsObject);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarScriptTypeContentsQuest);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarScriptTypeContentsMagicEffect);

			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarRefactorMenuContentsDocumentScript);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarRefactorMenuContentsRenameVariables);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarRefactorMenuContentsModifyVariableIndices);

			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarMessageList);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarFindList);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarBookmarkList);

			ToolBarDumpScript->ButtonClick -= ToolBarDumpScriptClickHandler;
			SAFEDELETE_CLR(ToolBarDumpScriptClickHandler);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarDumpAllScripts);
			ToolBarLoadScript->ButtonClick -= ToolBarLoadScriptClickHandler;
			SAFEDELETE_CLR(ToolBarLoadScriptClickHandler);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarLoadScriptsToTabs);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarShowOffsets);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarShowPreprocessedText);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarSanitizeScriptText);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarBindScript);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarSnippetManager);
			ConcreteWorkspaceViewUnsubscribeDeleteClickEvent(ToolBarSyncScriptsToDisk);

			GlobalFindList->ItemActivate -= GlobalFindListItemActivate;
			SAFEDELETE_CLR(GlobalFindListItemActivate);

			MessageList->ItemActivate -= ScriptAnnotationListItemActivate;
			BookmarkList->ItemActivate -= ScriptAnnotationListItemActivate;
			BookmarkList->ButtonClick -= BookmarksListButtonClick;
			FindResultsList->ItemActivate -= ScriptAnnotationListItemActivate;
			SAFEDELETE_CLR(ScriptAnnotationListItemActivate);
			SAFEDELETE_CLR(BookmarksListButtonClick);

			DisposeControlImage(ToolBarNewScript);
			DisposeControlImage(ToolBarOpenScript);
			DisposeControlImage(ToolBarPreviousScript);
			DisposeControlImage(ToolBarNextScript);
			DisposeControlImage(ToolBarSaveScript);
			DisposeControlImage(ToolBarRecompileScripts);
			DisposeControlImage(ToolBarCompileDependencies);
			DisposeControlImage(ToolBarDeleteScript);
			DisposeControlImage(ToolBarNavigationBack);
			DisposeControlImage(ToolBarNavigationForward);
			DisposeControlImage(ToolBarSaveAll);
			DisposeControlImage(ToolBarGlobalFindList);
			DisposeControlImage(ToolBarOptions);

			DisposeControlImage(ToolBarEditMenu);
			DisposeControlImage(ToolBarEditMenuContentsFindReplace);
			DisposeControlImage(ToolBarEditMenuContentsGotoLine);
			DisposeControlImage(ToolBarEditMenuContentsGotoOffset);

			DisposeControlImage(ToolBarMessageList);
			DisposeControlImage(ToolBarFindList);
			DisposeControlImage(ToolBarBookmarkList);
			DisposeControlImage(ToolBarDumpScript);
			DisposeControlImage(ToolBarLoadScript);
			DisposeControlImage(ToolBarShowOffsets);
			DisposeControlImage(ToolBarShowPreprocessedText);
			DisposeControlImage(ToolBarSanitizeScriptText);
			DisposeControlImage(ToolBarBindScript);
			DisposeControlImage(ToolBarSnippetManager);
			DisposeControlImage(ToolBarSyncScriptsToDisk);

			DisposeControlImage(ToolBarScriptTypeContentsObject);
			DisposeControlImage(ToolBarScriptTypeContentsQuest);
			DisposeControlImage(ToolBarScriptTypeContentsMagicEffect);

			SAFEDELETE_CLR(WorkspaceMainToolBar);
			SAFEDELETE_CLR(ToolBarNewScript);
			SAFEDELETE_CLR(ToolBarOpenScript);
			SAFEDELETE_CLR(ToolBarPreviousScript);
			SAFEDELETE_CLR(ToolBarNextScript);
			SAFEDELETE_CLR(ToolBarSaveScript);
			SAFEDELETE_CLR(ToolBarSaveScriptDropDown);
			SAFEDELETE_CLR(ToolBarSaveScriptNoCompile);
			SAFEDELETE_CLR(ToolBarSaveScriptAndPlugin);
			SAFEDELETE_CLR(ToolBarRecompileScripts);
			SAFEDELETE_CLR(ToolBarCompileDependencies);
			SAFEDELETE_CLR(ToolBarDeleteScript);
			SAFEDELETE_CLR(ToolBarNavigationBack);
			SAFEDELETE_CLR(ToolBarNavigationForward);
			SAFEDELETE_CLR(ToolBarSaveAll);
			SAFEDELETE_CLR(ToolBarOptions);

			SAFEDELETE_CLR(ToolBarScriptType);
			SAFEDELETE_CLR(ToolBarScriptTypeContents);
			SAFEDELETE_CLR(ToolBarScriptTypeContentsObject);
			SAFEDELETE_CLR(ToolBarScriptTypeContentsQuest);
			SAFEDELETE_CLR(ToolBarScriptTypeContentsMagicEffect);

			SAFEDELETE_CLR(WorkspaceSecondaryToolBar);
			SAFEDELETE_CLR(ToolBarEditMenu);
			SAFEDELETE_CLR(ToolBarEditMenuContents);
			SAFEDELETE_CLR(ToolBarEditMenuContentsFindReplace);
			SAFEDELETE_CLR(ToolBarEditMenuContentsGotoLine);
			SAFEDELETE_CLR(ToolBarEditMenuContentsGotoOffset);
			SAFEDELETE_CLR(ToolBarMessageList);
			SAFEDELETE_CLR(ToolBarFindList);
			SAFEDELETE_CLR(ToolBarBookmarkList);
			SAFEDELETE_CLR(ToolBarDumpScript);
			SAFEDELETE_CLR(ToolBarDumpScriptDropDown);
			SAFEDELETE_CLR(ToolBarDumpAllScripts);
			SAFEDELETE_CLR(ToolBarLoadScript);
			SAFEDELETE_CLR(ToolBarLoadScriptDropDown);
			SAFEDELETE_CLR(ToolBarLoadScriptsToTabs);
			SAFEDELETE_CLR(ToolBarShowOffsets);
			SAFEDELETE_CLR(ToolBarShowPreprocessedText);
			SAFEDELETE_CLR(ToolBarSanitizeScriptText);
			SAFEDELETE_CLR(ToolBarBindScript);
			SAFEDELETE_CLR(ToolBarSnippetManager);
			SAFEDELETE_CLR(ToolBarSyncScriptsToDisk);
			SAFEDELETE_CLR(ToolBarByteCodeSize);

			SAFEDELETE_CLR(OffsetTextViewer);
			SAFEDELETE_CLR(PreprocessorTextViewer);
			SAFEDELETE_CLR(FindReplaceBox);
			SAFEDELETE_CLR(IntelliSenseView);
			SAFEDELETE_CLR(TabStripFilter);
			SAFEDELETE_CLR(OutlineView);
			SAFEDELETE_CLR(ScopeCrumbManager);
			SAFEDELETE_CLR(ScopeCrumbBar);

			SAFEDELETE_CLR(AttachPanel);

			WorkspaceSplitter->Panel1->Controls->Clear();
			WorkspaceSplitter->Panel2->Controls->Clear();

			SAFEDELETE_CLR(WorkspaceSplitter);
			SAFEDELETE_CLR(MessageList);
			SAFEDELETE_CLR(FindResultsList);
			SAFEDELETE_CLR(BookmarkList);
			SAFEDELETE_CLR(GlobalFindList);
			SAFEDELETE_CLR(SpoilerText);

			ViewFactory->Remove(this);

			ViewController = nullptr;
			ViewFactory = nullptr;

			preferences::SettingsHolder::Get()->SaveToDisk();

			EditorForm->ForceClose();
			SAFEDELETE_CLR(EditorForm);
		}

		void ConcreteWorkspaceView::EditorForm_Cancel(Object^ Sender, CancelEventArgs^ E)
		{
			if (AllowDisposal)
			{
				Debug::Assert(EditorTabStrip == nullptr || GetTabCount() == 0);

				Rectangle Bounds = GetBounds(true);
				nativeWrapper::g_CSEInterfaceTable->ScriptEditor.SaveEditorBoundsToINI(Bounds.Left, Bounds.Top, Bounds.Width, Bounds.Height);

				// yuck!
				delete this;
				return;
			}

			E->Cancel = true;
			if (GetTabCount() > 1)
			{
				if (MessageBox::Show("Are you sure you want to close all open tabs?",
					SCRIPTEDITOR_TITLE,
					MessageBoxButtons::YesNo,
					MessageBoxIcon::Question,
					MessageBoxDefaultButton::Button2) == DialogResult::No)
				{
					return;
				}
			}

			CloseAll();
		}

		void ConcreteWorkspaceView::EditorForm_KeyDown(Object^ Sender, KeyEventArgs^ E)
		{
			Controller->BubbleKeyDownEvent(this, E);
		}

		void ConcreteWorkspaceView::EditorForm_SizeChanged(Object^ Sender, EventArgs^ E)
		{
			;//
		}

		void ConcreteWorkspaceView::EditorForm_PositionChanged(Object^ Sender, EventArgs^ E)
		{
			;//
		}

		void ConcreteWorkspaceView::EditorForm_Activated(Object^ Sender, EventArgs^ E)
		{
			;//
		}

		void ConcreteWorkspaceView::ScriptStrip_TabItemClose(Object^ Sender, DotNetBar::SuperTabStripTabItemCloseEventArgs ^ E)
		{
			E->Cancel = true;

			IWorkspaceModel^ Model = GetModel((DotNetBar::SuperTabItem^)E->Tab);	// clicking on the close button doesn't change the active tab
			if (Model->Dirty)
				SelectTab(GetTab(Model));

			bool Cancelled = false;
			if (Model->Controller->Close(Model, Cancelled))
				DissociateModel(Model, true);
		}

		void ConcreteWorkspaceView::ScriptStrip_SelectedTabChanged(Object^ Sender, DotNetBar::SuperTabStripSelectedTabChangedEventArgs^ E)
		{
			if (DisallowBinding)
				return;

			if (E->NewValue == nullptr)
				return;

			IWorkspaceModel^ Old = nullptr;
			IWorkspaceModel^ New = GetModel((DotNetBar::SuperTabItem^)E->NewValue);

			if (E->OldValue)
				Old = GetModel((DotNetBar::SuperTabItem^)E->OldValue);

			if (FreezeNavigationStacks == false)
			{
				if (Old)
					NavigationStackBackward->Push(Old);

				NavigationStackForward->Clear();
			}

			BeginUpdate();

			if (Old)
				Old->Controller->Unbind(Old);

			New->Controller->Bind(New, this);

#if 0
			IntPtr FocusCtrl = NativeWrapper::GetFocus();
			if (FocusCtrl == IntPtr::Zero)
				DebugPrint("no focus");
			else
			{
				Control^ Focus = Control::FromHandle(FocusCtrl);
				if (Focus == nullptr)
					DebugPrint("handle found but no control - " + FocusCtrl.ToString("X8"));
				else
					DebugPrint("focus control = " + Focus->GetType()->ToString() + ", handle = " + FocusCtrl.ToString("X8"));
			}
#endif
			EndUpdate();
		}

		void ConcreteWorkspaceView::ScriptStrip_TabRemoved(Object^ Sender, DotNetBar::SuperTabStripTabRemovedEventArgs^ E)
		{
			;//
		}

		void ConcreteWorkspaceView::ScriptStrip_MouseClick(Object^ Sender, MouseEventArgs^ E)
		{
			switch (E->Button)
			{
			case MouseButtons::Middle:
				{
					DotNetBar::SuperTabItem^ MouseOverTab = GetMouseOverTab();
					if (MouseOverTab)
					{
						IWorkspaceModel^ Model = GetModel(MouseOverTab);
						if (Model->Dirty)
							SelectTab(GetTab(Model));

						bool Cancelled = false;
						if (ModelController()->Close(Model, Cancelled))
							DissociateModel(Model, true);
					}

					break;
				}
			}
		}

		void ConcreteWorkspaceView::ScriptStrip_TabMoving(Object^ Sender, DotNetBar::SuperTabStripTabMovingEventArgs^ E)
		{
			if (WorkspaceViewTabTearingHelper::Get()->InProgress == false)
			{
				DotNetBar::SuperTabItem^ MouseOverTab = GetMouseOverTab();
				if (MouseOverTab)
					WorkspaceViewTabTearingHelper::Get()->InitiateHandling(GetModel(MouseOverTab), this);
			}
		}

		void ConcreteWorkspaceView::NewTabButton_Click(Object^ Sender, EventArgs^ E)
		{
			NewTab(gcnew NewTabOperationArgs);
		}

		void ConcreteWorkspaceView::SortTabsButton_Click(Object^ Sender, EventArgs^ E)
		{
			array<String^>^ Keys = gcnew array<String^>(EditorTabStrip->Tabs->Count);
			array<DotNetBar::BaseItem^>^ Tabs = gcnew array<DotNetBar::BaseItem^>(EditorTabStrip->Tabs->Count);

			for (int i = 0; i < EditorTabStrip->Tabs->Count; i++)
			{
				Tabs[i] = EditorTabStrip->Tabs[i];
				Keys[i] = EditorTabStrip->Tabs[i]->PlainText;
			}

			Array::Sort(Keys, Tabs);

			for (int i = 0; i < EditorTabStrip->Tabs->Count; i++)
				EditorTabStrip->Tabs[i] = Tabs[i];

			EditorTabStrip->RecalcLayout();
		}

		void ConcreteWorkspaceView::ToolBarNavigationBack_Click(Object^ Sender, EventArgs^ E)
		{
			try
			{
				FreezeNavigationStacks = true;

				IWorkspaceModel^ Current = GetActiveModel();
				if (NavigationStackBackward->Count)
				{
					IWorkspaceModel^ Previous = NavigationStackBackward->Pop();

					NavigationStackForward->Push(Current);
					SelectTab(GetTab(Previous));
				}
			}
			finally
			{
				FreezeNavigationStacks = false;
			}
		}

		void ConcreteWorkspaceView::ToolBarNavigationForward_Click(Object^ Sender, EventArgs^ E)
		{
			try
			{
				FreezeNavigationStacks = true;

				IWorkspaceModel^ Current = GetActiveModel();
				if (NavigationStackForward->Count)
				{
					IWorkspaceModel^ Next = NavigationStackForward->Pop();

					NavigationStackBackward->Push(Current);
					SelectTab(GetTab(Next));
				}
			}
			finally
			{
				FreezeNavigationStacks = false;
			}
		}

		void ConcreteWorkspaceView::ToolBarSaveAll_Click(Object^ Sender, EventArgs^ E)
		{
			SaveAll();
		}

		void ConcreteWorkspaceView::ToolBarOptions_Click(Object^ Sender, EventArgs^ E)
		{
			preferences::PreferencesDialog PreferencesDialog;
		}

		void ConcreteWorkspaceView::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			if (AssociatedModels->Count == 0)
			{
				// this can happen if the editor shuts down whilst the view is being/about to be disposed
				Debug::Assert(AllowDisposal == true);
				return;
			}

			Color ForegroundColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
			Color BackgroundColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
			Font^ CustomFont = safe_cast<Font^>(preferences::SettingsHolder::Get()->Appearance->TextFont->Clone());

			OffsetTextViewer->SetFont(CustomFont);
			OffsetTextViewer->SetForegroundColor(ForegroundColor);
			OffsetTextViewer->SetBackgroundColor(BackgroundColor);

			PreprocessorTextViewer->SetFont(CustomFont);
			PreprocessorTextViewer->SetForegroundColor(ForegroundColor);
			PreprocessorTextViewer->SetBackgroundColor(BackgroundColor);

		//	ScopeCrumbBar->BackgroundStyle->BackColor = BackgroundColor;
		//	ScopeCrumbBar->BackgroundStyle->BorderColor = System::Drawing::Color::FromArgb(200, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B);
		//	ScopeCrumbBar->BackgroundStyle->BorderColor2 = System::Drawing::Color::FromArgb(200, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B);
		//	ScopeCrumbBar->Font = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"), 9);
			ScopeCrumbManager->Visible = preferences::SettingsHolder::Get()->Appearance->ShowScopeBar;
			ScopeCrumbManager->RefreshCrumbs();

			Redraw();
		}

		void ConcreteWorkspaceView::ToolBarDumpAllScripts_Click(Object^ Sender, EventArgs^ E)
		{
			FolderBrowserDialog^ SaveManager = gcnew FolderBrowserDialog();

			SaveManager->Description = "All open scripts in this window will be dumped to the selected folder.";
			SaveManager->ShowNewFolderButton = true;
			SaveManager->SelectedPath = gcnew String(nativeWrapper::g_CSEInterfaceTable->ScriptEditor.GetDefaultCachePath());

			if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->SelectedPath->Length > 0)
			{
				String^ FileExtension = "txt";
				inputBoxes::InputBoxResult^ Result = inputBoxes::InputBox::Show("Enter The File Extension To Use", "Dump Scripts", FileExtension);
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;
				else
					FileExtension = Result->Text;

				DumpAllToDisk(SaveManager->SelectedPath, FileExtension);
			}
		}

		void ConcreteWorkspaceView::ToolBarLoadScriptsToTabs_Click(Object^ Sender, EventArgs^ E)
		{
			OpenFileDialog^ LoadManager = gcnew OpenFileDialog();

			LoadManager->DefaultExt = "*.txt";
			LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			LoadManager->Multiselect = true;
			LoadManager->RestoreDirectory = true;

			if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileNames->Length > 0)
				LoadAllFromDisk(LoadManager->FileNames);
		}

		void ConcreteWorkspaceView::ToolBarMessageList_Click(Object^ Sender, EventArgs^ E)
		{
			if (ToolBarMessageList->Checked == false)
				ShowMessageList();
			else
				HideAllLists();
		}

		void ConcreteWorkspaceView::ToolBarFindList_Click(Object^ Sender, EventArgs^ E)
		{
			if (ToolBarFindList->Checked == false)
				ShowFindResultList();
			else
				HideAllLists();
		}

		void ConcreteWorkspaceView::ToolBarBookmarkList_Click(Object^ Sender, EventArgs^ E)
		{
			if (ToolBarBookmarkList->Checked == false)
				ShowBookmarkList();
			else
				HideAllLists();
		}

		void ConcreteWorkspaceView::ToolBarGlobalFindList_Click(Object^ Sender, EventArgs^ E)
		{
			if (ToolBarGlobalFindList->Checked == false)
				ShowGlobalFindResultList();
			else
				HideAllLists();
		}

		void ConcreteWorkspaceView::ToolBarNewScript_Click(Object^ Sender, EventArgs^ E)
		{
			switch (Control::ModifierKeys)
			{
			case Keys::Control:
				{
					NewTabOperationArgs^ Args = gcnew NewTabOperationArgs;
					Args->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::New;
					NewTab(Args);
				}

				break;
			case Keys::Shift:
				{
					Rectangle Bounds = GetBounds(false);
					ConcreteWorkspaceView^ New = (ConcreteWorkspaceView^)ViewFactory->CreateView(Bounds.X, Bounds.Y, Bounds.Width, Bounds.Height);
					IWorkspaceModel^ Model = ModelFactory()->CreateModel(nullptr);

					New->AssociateModel(Model, true);
				}

				break;
			default:
				ModelController()->New(GetActiveModel());
				break;
			}
		}

		void ConcreteWorkspaceView::ToolBarOpenScript_Click(Object^ Sender, EventArgs^ E)
		{
			switch (Control::ModifierKeys)
			{
			case Keys::Control:
				{
					NewTabOperationArgs^ Args = gcnew NewTabOperationArgs;
					Args->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::OpenList;
					NewTab(Args);
				}

				break;
			default:
				ShowOpenDialog();
				break;
			}
		}

		void ConcreteWorkspaceView::ToolBarPreviousScript_Click(Object^ Sender, EventArgs^ E)
		{
			ModelController()->Previous(GetActiveModel());
		}

		void ConcreteWorkspaceView::ToolBarNextScript_Click(Object^ Sender, EventArgs^ E)
		{
			ModelController()->Next(GetActiveModel());
		}

		void ConcreteWorkspaceView::ToolBarSaveScript_Click(Object^ Sender, EventArgs^ E)
		{
			bool HasWarnings = false;
			if (ModelController()->Save(GetActiveModel(), IWorkspaceModel::SaveOperation::Default, HasWarnings) == false || HasWarnings)
				ShowMessageList();
		}

		void ConcreteWorkspaceView::ToolBarSaveScriptNoCompile_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (Active->New == true || Active->Initialized == false)
			{
				MessageBox::Show("You may only perform this operation on an existing script.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			bool HasWarnings = false;
			if (ModelController()->Save(GetActiveModel(), IWorkspaceModel::SaveOperation::NoCompile, HasWarnings) == false || HasWarnings)
				ShowMessageList();
		}

		void ConcreteWorkspaceView::ToolBarSaveScriptAndPlugin_Click(Object^ Sender, EventArgs^ E)
		{
			bool HasWarnings = false;
			if (ModelController()->Save(GetActiveModel(), IWorkspaceModel::SaveOperation::SavePlugin, HasWarnings) == false || HasWarnings)
				ShowMessageList();
		}

		void ConcreteWorkspaceView::ToolBarRecompileScripts_Click(Object^ Sender, EventArgs^ E)
		{
			if (MessageBox::Show("Are you sure you want to recompile all the scripts in the active plugin?",
				SCRIPTEDITOR_TITLE,
				MessageBoxButtons::YesNo,
				MessageBoxIcon::Exclamation) == DialogResult::Yes)
			{
				nativeWrapper::g_CSEInterfaceTable->ScriptEditor.RecompileScripts();
				MessageBox::Show("All active scripts recompiled. Results have been logged to the console.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
			}
		}

		void ConcreteWorkspaceView::ToolBarCompileDependencies_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (Active->New == false && Active->Initialized == true)
			{
				ModelController()->CompileDepends(Active);
				MessageBox::Show("Operation complete!\n\nScript variables used as condition parameters will need to be corrected manually. The results have been logged to the console.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
			}
			else if (Active->Initialized == true)
			{
				MessageBox::Show("The current script needs to be compiled before its dependencies can be updated.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceView::ToolBarDeleteScript_Click(Object^ Sender, EventArgs^ E)
		{
			ShowDeleteDialog();
		}

		void ConcreteWorkspaceView::ToolBarScriptTypeContentsObject_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			ModelController()->SetType(Active, IWorkspaceModel::ScriptType::Object);
		}

		void ConcreteWorkspaceView::ToolBarScriptTypeContentsQuest_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			ModelController()->SetType(Active, IWorkspaceModel::ScriptType::Quest);
		}

		void ConcreteWorkspaceView::ToolBarScriptTypeContentsMagicEffect_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			ModelController()->SetType(Active, IWorkspaceModel::ScriptType::MagicEffect);
		}

		void ConcreteWorkspaceView::ToolBarEditMenuContentsFindReplace_Click(Object^ Sender, EventArgs^ E)
		{
			ShowFindReplaceDialog(false);
		}

		void ConcreteWorkspaceView::ToolBarEditMenuContentsGotoLine_Click(Object^ Sender, EventArgs^ E)
		{
			GotoLine();
		}

		void ConcreteWorkspaceView::ToolBarEditMenuContentsGotoOffset_Click(Object^ Sender, EventArgs^ E)
		{
			GotoOffset();
		}

		void ConcreteWorkspaceView::ToolBarDumpScript_Click(Object^ Sender, EventArgs^ E)
		{
			SaveFileDialog^ SaveManager = gcnew SaveFileDialog();
			IWorkspaceModel^ Active = GetActiveModel();

			SaveManager->DefaultExt = "*.txt";
			SaveManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			SaveManager->FileName = Active->LongDescription;
			SaveManager->RestoreDirectory = true;

			if (SaveManager->ShowDialog() == DialogResult::OK && SaveManager->FileName->Length > 0)
				ModelController()->SaveToDisk(Active, SaveManager->FileName, true, gcnew String("txt"));
		}

		void ConcreteWorkspaceView::ToolBarLoadScript_Click(Object^ Sender, EventArgs^ E)
		{
			OpenFileDialog^ LoadManager = gcnew OpenFileDialog();
			IWorkspaceModel^ Active = GetActiveModel();

			LoadManager->DefaultExt = "*.txt";
			LoadManager->Filter = "Text Files|*.txt|All files (*.*)|*.*";
			LoadManager->RestoreDirectory = true;

			if (LoadManager->ShowDialog() == DialogResult::OK && LoadManager->FileName->Length > 0)
				ModelController()->LoadFromDisk(Active, LoadManager->FileName);
		}

		void ConcreteWorkspaceView::ToolBarShowOffsets_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (ToolBarShowOffsets->Checked)
			{
				int Caret = OffsetTextViewer->Hide();
				ModelController()->SetCaret(Active, Caret);
				ModelController()->AcquireInputFocus(Active);
				ToolBarShowOffsets->Checked = false;
			}
			else
			{
				String^ Text = "";
				void* Data = nullptr;
				UInt32 Length = 0;

				if (ModelController()->GetOffsetViewerData(Active, Text, &Data, Length))
				{
					OffsetTextViewer->InitializeViewer(Text, Data, Length);
					int Caret = ModelController()->GetCaret(Active);
					if (Caret == -1)
						Caret = 0;

					if (OffsetTextViewer->Show(Caret))
					{
						ToolBarShowOffsets->Checked = true;
						return;
					}
				}

				MessageBox::Show("Offset Viewer couldn't be initialized!\n\nPlease recompile the current script.",
								 SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceView::ToolBarShowPreprocessedText_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (ToolBarShowPreprocessedText->Checked)
			{
				PreprocessorTextViewer->Hide();
				ModelController()->AcquireInputFocus(Active);
				ToolBarShowPreprocessedText->Checked = false;
			}
			else
			{
				bool Preprocess = false;
				String^ PreprocessedText = ModelController()->GetText(Active, true, Preprocess, false);
				if (Preprocess)
				{
					int Caret = ModelController()->GetCaret(Active);
					if (Caret == -1)
						Caret = 0;

					PreprocessorTextViewer->Show(PreprocessedText, Caret);
					ToolBarShowPreprocessedText->Checked = true;
				}
				else
				{
					MessageBox::Show("The preprocessing operation was unsuccessful.", SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Error);
					ShowMessageList();
				}
			}
		}

		void ConcreteWorkspaceView::ToolBarSanitizeScriptText_Click(Object^ Sender, EventArgs^ E)
		{
			if (ModelController()->Sanitize(GetActiveModel()) == false)
			{
				MessageBox::Show("The script contains structural errors that must be fixed before this operation can be performed.",
								 SCRIPTEDITOR_TITLE, MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}

		void ConcreteWorkspaceView::ToolBarBindScript_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			if (Active->New == true || Active->Initialized == false)
			{
				MessageBox::Show("You may only perform this operation on an existing script.", SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK, MessageBoxIcon::Exclamation);
			}
			else
			{
				ModelController()->BindToForm(Active);
			}
		}

		void ConcreteWorkspaceView::ToolBarSnippetManager_Click(Object^ Sender, EventArgs^ E)
		{
			intellisense::IntelliSenseBackend::Get()->ShowCodeSnippetManager();
		}

		void ConcreteWorkspaceView::ToolBarSyncScriptsToDisk_Click(Object^ Sender, EventArgs^ E)
		{
			scriptSync::DiskSyncDialog::Show();
		}

		void ConcreteWorkspaceView::ToolBarRefactorMenuContentsDocumentScript_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();

			if (Active->Dirty)
			{
				MessageBox::Show("The current script must be compiled to perform this operation.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			ModelController()->ApplyRefactor(Active, IWorkspaceModel::RefactorOperation::DocumentScript, nullptr);
		}

		void ConcreteWorkspaceView::ToolBarRefactorMenuContentsRenameVariables_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();

			if (Active->Dirty)
			{
				MessageBox::Show("The current script must be compiled to perform this operation.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			if (ModelController()->ApplyRefactor(Active, IWorkspaceModel::RefactorOperation::RenameVariables, nullptr))
			{
				MessageBox::Show("Variables have been renamed. Scripts referencing them (current script included) will have to be manually updated with the new identifiers.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Information);
			}
		}

		void ConcreteWorkspaceView::ToolBarRefactorMenuContentsModifyVariableIndices_Click(Object^ Sender, EventArgs^ E)
		{
			IWorkspaceModel^ Active = GetActiveModel();

			if (Active->Dirty)
			{
				MessageBox::Show("The current script must be compiled to perform this operation.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);

				return;
			}

			if (ModelController()->ApplyRefactor(Active, IWorkspaceModel::RefactorOperation::ModifyVariableIndices, nullptr))
			{
				if (preferences::SettingsHolder::Get()->General->RecompileDependsOnVarIdxMod)
				{
					ToolBarSaveScript->PerformButtonClick();
					ToolBarCompileDependencies->PerformClick();
				}
			}
		}

		void ConcreteWorkspaceView::Model_StateChangeHandler(IWorkspaceModel^ Sender, IWorkspaceModel::StateChangeEventArgs^ E)
		{
			switch (E->EventType)
			{
			case IWorkspaceModel::StateChangeEventArgs::Type::Dirty:
			{
				DotNetBar::SuperTabItem^ Tab = GetTab(Sender);
				Debug::Assert(Tab != nullptr);

				Tab->ImageIndex = (int)E->Dirty;
				break;
			}
			case IWorkspaceModel::StateChangeEventArgs::Type::ByteCodeSize:
			{
				if (GetActiveModel() == Sender)
				{
					ToolBarByteCodeSize->Value = E->ByteCodeSize;
					ToolBarByteCodeSize->ToolTipText = String::Format("Compiled Script Size: {0:F2} KB", (float)(E->ByteCodeSize / 1024.0));
				}

				break;
			}
			case IWorkspaceModel::StateChangeEventArgs::Type::ScriptType:
			{
				if (GetActiveModel() == Sender)
					UpdateScriptTypeControls(E->ScriptType);

				break;
			}
			case IWorkspaceModel::StateChangeEventArgs::Type::Description:
			{
				DotNetBar::SuperTabItem^ Tab = GetTab(Sender);
				Debug::Assert(Tab != nullptr);

				Tab->Text = E->ShortDescription;
				Tab->Tooltip = E->LongDescription;

				if (GetActiveModel() == Sender)
					EditorForm->Text = E->LongDescription + " - " + SCRIPTEDITOR_TITLE;

				break;
			}
			case IWorkspaceModel::StateChangeEventArgs::Type::Messages:
			{
				MessageList->SetObjects(E->Messages, true);

				if (E->Messages->Count == 0)
				{
					ToolBarMessageList->DisplayStyle = ToolStripItemDisplayStyle::Image;
					ToolBarMessageList->ToolTipText = "Messages";
				}
				else
				{
					ToolBarMessageList->DisplayStyle = ToolStripItemDisplayStyle::ImageAndText;
					ToolBarMessageList->Text = "(" + E->Messages->Count + ")";
					ToolBarMessageList->ToolTipText = E->Messages->Count + " Message(s)";
				}

				break;
			}
			case IWorkspaceModel::StateChangeEventArgs::Type::Bookmarks:
			{
				BookmarkList->SetObjects(E->Bookmarks, true);

				if (E->Bookmarks->Count == 0)
				{
					ToolBarBookmarkList->DisplayStyle = ToolStripItemDisplayStyle::Image;
					ToolBarBookmarkList->ToolTipText = "Bookmarks";
				}
				else
				{
					ToolBarBookmarkList->DisplayStyle = ToolStripItemDisplayStyle::ImageAndText;
					ToolBarBookmarkList->Text = "(" + E->Bookmarks->Count + ")";
					ToolBarBookmarkList->ToolTipText = E->Bookmarks->Count + " Bookmark(s)";
				}

				break;
			}
			case IWorkspaceModel::StateChangeEventArgs::Type::FindResults:
			{
				FindResultsList->SetObjects(E->FindResults, true);

				if (E->FindResults->Count == 0)
				{
					ToolBarFindList->DisplayStyle = ToolStripItemDisplayStyle::Image;
					ToolBarFindList->ToolTipText = "Find/Replace Results";
				}
				else
				{
					ToolBarFindList->DisplayStyle = ToolStripItemDisplayStyle::ImageAndText;
					ToolBarFindList->Text = "(" + E->FindResults->Count + ")";
					ToolBarFindList->ToolTipText = E->FindResults->Count + " Find/Replace Result(s)";
				}

				break;
			}
			default:
				break;
			}
		}

		void ConcreteWorkspaceView::GlobalFindList_ItemActivate(Object^ Sender, EventArgs^ E)
		{
			Object^ Selection = GlobalFindList->SelectedObject;
			if (Selection)
			{
				if (Selection->GetType() == textEditors::IScriptTextEditor::FindReplaceResult::HitData::typeid)
				{
					textEditors::IScriptTextEditor::FindReplaceResult::HitData^ Data = (textEditors::IScriptTextEditor::FindReplaceResult::HitData^)Selection;
					FindReplaceAllResults::PerScriptData^ ParentData = (FindReplaceAllResults::PerScriptData^)GlobalFindList->GetParent(Selection);

					if (ParentData->ParentModel)
					{
						SelectTab(GetTab(ParentData->ParentModel));
						ParentData->ParentModel->Controller->GotoLine(ParentData->ParentModel, Data->Line);
					}
				}
				else if (GlobalFindList->CanExpand(Selection))
				{
					if (GlobalFindList->IsExpanded(Selection) == false)
						GlobalFindList->Expand(Selection);
					else
						GlobalFindList->Collapse(Selection);
				}
			}
		}

		void ConcreteWorkspaceView::ScriptAnnotationList_ItemActivate(Object^ Sender, EventArgs^ E)
		{
			auto ListView = safe_cast<BrightIdeasSoftware::ObjectListView^>(Sender);
			auto Annotation = safe_cast<ScriptLineAnnotation^>(ListView->SelectedObject);
			if (Annotation == nullptr)
				return;

			auto Model = GetActiveModel();
			Model->Controller->GotoLine(Model, Annotation->Line);
		}

		void ConcreteWorkspaceView::BookmarksList_ButtonClick(Object^ Sender, BrightIdeasSoftware::CellClickEventArgs^ E)
		{
			auto Bookmark = safe_cast<ScriptBookmark^>(E->Model);
			if (Bookmark == nullptr)
				return;

			auto Model = GetActiveModel();
			Model->Controller->RemoveBookmark(Model, Bookmark->Line, Bookmark->Text);
		}

		System::Object^ ConcreteWorkspaceView::ScriptTextAnnotationListLineNumberAspectGetter(Object^ E)
		{
			auto Model = safe_cast<ScriptLineAnnotation^>(E);
			if (Model == nullptr)
				return nullptr;

			return Model->Line;
		}

		System::Object^ ConcreteWorkspaceView::ScriptTextAnnotationListTextAspectGetter(Object^ E)
		{
			auto Model = safe_cast<ScriptLineAnnotation^>(E);
			if (Model == nullptr)
				return nullptr;

			return Model->Text;
		}

		System::Object^ ConcreteWorkspaceView::MessageListTypeAspectGetter(Object^ E)
		{
			auto Model = safe_cast<ScriptDiagnosticMessage^>(E);
			if (Model == nullptr)
				return nullptr;

			return Model->Type;
		}

		System::Object^ ConcreteWorkspaceView::MessageListTypeImageGetter(Object^ RowObject)
		{
			auto Model = safe_cast<ScriptDiagnosticMessage^>(RowObject);
			if (Model == nullptr)
				return nullptr;

			return safe_cast<int>(Model->Type);
		}

		System::String^ ConcreteWorkspaceView::MessageListTypeAspectToStringConverter(Object^ E)
		{
			return String::Empty;
		}

		System::Object^ ConcreteWorkspaceView::MessageListTypeGroupKeyGetter(Object^ RowObject)
		{
			auto Model = safe_cast<ScriptDiagnosticMessage^>(RowObject);
			if (Model == nullptr)
				return nullptr;

			return Model->Type;
		}

		System::String^ ConcreteWorkspaceView::MessageListTypeGroupKeyToTitleConverter(System::Object^ GroupKey)
		{
			auto Key = safe_cast<ScriptDiagnosticMessage::MessageType>(GroupKey);
			return Key.ToString();
		}

		System::Object^ ConcreteWorkspaceView::MessageListSourceAspectGetter(Object^ E)
		{
			auto Model = safe_cast<ScriptDiagnosticMessage^>(E);
			if (Model == nullptr)
				return nullptr;

			return Model->Source;
		}

		System::String^ ConcreteWorkspaceView::MessageListSourceAspectToStringConverter(Object^ E)
		{
			auto Key = safe_cast<ScriptDiagnosticMessage::MessageSource>(E);
			return Key.ToString();
		}

		System::Object^ ConcreteWorkspaceView::MessageListSourceGroupKeyGetter(Object^ RowObject)
		{
			auto Model = safe_cast<ScriptDiagnosticMessage^>(RowObject);
			if (Model == nullptr)
				return nullptr;

			return Model->Source;
		}

		System::String^ ConcreteWorkspaceView::MessageListSourceGroupKeyToTitleConverter(System::Object^ GroupKey)
		{
			auto Key = safe_cast<ScriptDiagnosticMessage::MessageSource>(GroupKey);
			return Key.ToString();
		}

		System::Object^ ConcreteWorkspaceView::BookmarksListActionAspectGetter(Object^ E)
		{
			return "Remove";
		}

		System::Object^ ConcreteWorkspaceView::FindResultsListHitsAspectGetter(Object^ E)
		{
			auto Model = safe_cast<ScriptFindResult^>(E);
			if (Model == nullptr)
				return nullptr;

			return Model->Hits;
		}

		IWorkspaceModelController^ ConcreteWorkspaceView::ModelController()
		{
			Debug::Assert(AssociatedModels->Count != 0);
			ModelTabTableT::Enumerator Itr = AssociatedModels->GetEnumerator();
			Itr.MoveNext();

			return Itr.Current.Key->Controller;
		}

		IWorkspaceModelFactory^ ConcreteWorkspaceView::ModelFactory()
		{
			Debug::Assert(AssociatedModels->Count != 0);
			ModelTabTableT::Enumerator Itr = AssociatedModels->GetEnumerator();
			Itr.MoveNext();

			return Itr.Current.Key->Factory;
		}

		bool ConcreteWorkspaceView::IsModelAssociated(IWorkspaceModel^ Model)
		{
			return AssociatedModels->ContainsKey(Model);
		}

		void ConcreteWorkspaceView::AssociateModel(IWorkspaceModel^ Model, bool Bind)
		{
			Debug::Assert(IsModelAssociated(Model) == false);

			DotNetBar::SuperTabItem^ Tab = gcnew DotNetBar::SuperTabItem;
			Tab->Text = Model->ShortDescription;
			Tab->ImageIndex = (int)Model->Dirty;
			Tab->Tag = Model;

			ModelSubscribeEvents(Model);
			AssociatedModels->Add(Model, Tab);

			// when the model being associated is the very first one (for the view), the addition of the tab to the strip automatically triggers its selection
			// so it must be added to the table beforehand
			EditorTabStrip->Tabs->Add(Tab);

			if (Bind)
				SelectTab(Tab);
		}

		void ConcreteWorkspaceView::DissociateModel(IWorkspaceModel^ Model, bool Destroy)
		{
			Debug::Assert(IsModelAssociated(Model) == true);

			Model->Controller->Unbind(Model);

			RemoveFromNavigationStacks(Model);
			RemoveFromFindReplaceAllResultCache(Model);
			ModelUnsubscribeEvents(Model);

			DotNetBar::SuperTabItem^ Tab = GetTab(Model);

			Tab->Tag = nullptr;
			delete Tab;

			AssociatedModels->Remove(Model);

			if (Destroy)
				delete Model;

			EditorTabStrip->Tabs->Remove(Tab);

			// close form if there are no more tabs left
			if (GetTabCount() == 0 && DisallowBinding == false)
			{
				AllowDisposal = true;
				EditorForm->Close();
			}
		}

		IWorkspaceModel^ ConcreteWorkspaceView::GetModel(DotNetBar::SuperTabItem^ Tab)
		{
			return (IWorkspaceModel^)Tab->Tag;
		}

		IWorkspaceModel^ ConcreteWorkspaceView::GetModel(String^ Description)
		{
			for each (auto Itr in AssociatedModels)
			{
				if (String::Compare(Itr.Key->ShortDescription, Description, true) == 0)
					return Itr.Key;
			}

			return nullptr;
		}

		DotNetBar::SuperTabItem^ ConcreteWorkspaceView::GetTab(IWorkspaceModel^ Model)
		{
			if (IsModelAssociated(Model))
				return AssociatedModels[Model];
			else
				return nullptr;
		}

		DotNetBar::SuperTabItem^ ConcreteWorkspaceView::GetMouseOverTab()
		{
			for each (DotNetBar::SuperTabItem^ Itr in EditorTabStrip->Tabs)
			{
				if (Itr->IsMouseOver)
					return Itr;
			}

			return nullptr;
		}

		DotNetBar::SuperTabItem^ ConcreteWorkspaceView::GetActiveTab()
		{
			return EditorTabStrip->SelectedTab;
		}

		IWorkspaceModel^ ConcreteWorkspaceView::GetActiveModel()
		{
			return GetModel(GetActiveTab());
		}

		Rectangle ConcreteWorkspaceView::GetBounds(bool UseRestoreBounds)
		{
			if (EditorForm->WindowState == FormWindowState::Normal || UseRestoreBounds == false)
				return EditorForm->Bounds;
			else
				return EditorForm->RestoreBounds;
		}

		void ConcreteWorkspaceView::ToggleSecondaryPanel(bool State)
		{
			if (State)
				WorkspaceSplitter->SplitterDistance = GetBounds(false).Height / 1.5;
			else
				WorkspaceSplitter->SplitterDistance = GetBounds(false).Height;
		}

		void ConcreteWorkspaceView::UpdateScriptTypeControls(IWorkspaceModel::ScriptType New)
		{
			switch (New)
			{
			case IWorkspaceModel::ScriptType::Object:
				ToolBarScriptType->Text = "Object Script";
				ToolBarScriptType->Image = ToolBarScriptTypeContentsObject->Image;
				break;
			case IWorkspaceModel::ScriptType::Quest:
				ToolBarScriptType->Text = "Quest Script";
				ToolBarScriptType->Image = ToolBarScriptTypeContentsQuest->Image;
				break;
			case IWorkspaceModel::ScriptType::MagicEffect:
				ToolBarScriptType->Text = "Magic Effect Script";
				ToolBarScriptType->Image = ToolBarScriptTypeContentsMagicEffect->Image;
				break;
			}
		}

		void ConcreteWorkspaceView::ShowOpenDialog()
		{
			SelectScriptDialogParams^ Params = gcnew SelectScriptDialogParams;
			Params->SelectedScriptEditorID = GetActiveModel()->ShortDescription;

			SelectScriptDialog ScriptSelection(Params);
			if (ScriptSelection.HasResult == false)
				return;

			BeginUpdate();
			{
				NewTabOperationArgs^ E = gcnew NewTabOperationArgs;
				int i = 0;
				for each (auto Itr in ScriptSelection.ResultData->SelectedScriptEditorIDs)
				{
					CString EID(Itr);

					if (i == 0)
					{
						ModelController()->Open(GetActiveModel(),
							nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str()));
					}
					else
					{
						E->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::Open;
						E->OpenArgs = nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str());
						E->BindAfterCreation = false;
						Debug::Assert(E->OpenArgs != nullptr);
						NewTab(E);
					}

					i++;
				}
			}
			EndUpdate();
		}

		void ConcreteWorkspaceView::ShowDeleteDialog()
		{
			SelectScriptDialogParams^ Params = gcnew SelectScriptDialogParams;
			Params->SelectedScriptEditorID = GetActiveModel()->ShortDescription;

			SelectScriptDialog ScriptSelection(Params);
			if (ScriptSelection.HasResult == false)
				return;

			for each (auto Itr in ScriptSelection.ResultData->SelectedScriptEditorIDs)
			{
				CString EID(Itr);
				nativeWrapper::g_CSEInterfaceTable->ScriptEditor.DeleteScript(EID.c_str());
			}
		}

		void ConcreteWorkspaceView::ShowFindReplaceDialog(bool PerformSearch)
		{
			IWorkspaceModel^ Active = GetActiveModel();
			String^ Query = Active->Controller->GetSelection(Active);
			if (Query == "")
				Query = Active->Controller->GetCaretToken(Active);

			FindReplaceBox->Show(Query, false, PerformSearch);
		}

		void ConcreteWorkspaceView::GotoLine()
		{
			if (ToolBarShowOffsets->Checked)
			{
				MessageBox::Show("This operation can only be performed in the text editor and the preprocessed text viewer.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}
			else
			{
				IWorkspaceModel^ Active = GetActiveModel();
				UInt32 LineCount = 0;
				if (ToolBarShowPreprocessedText->Checked)
					LineCount = PreprocessorTextViewer->GetLineCount();
				else
					LineCount = ModelController()->GetLineCount(Active);

				inputBoxes::InputBoxResult^ Result = inputBoxes::InputBox::Show("Line Number (1 - " + LineCount + ")", "Go To Line");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;

				if (ToolBarShowPreprocessedText->Checked)
					PreprocessorTextViewer->JumpToLine(Result->Text);
				else
				{
					int Line = 0;
					if (int::TryParse(Result->Text, Line))
						ModelController()->GotoLine(Active, Line);
				}
			}
		}

		void ConcreteWorkspaceView::GotoOffset()
		{
			if (ToolBarShowOffsets->Checked)
			{
				inputBoxes::InputBoxResult^ Result = inputBoxes::InputBox::Show("Offset (0000 - " + OffsetTextViewer->GetLastOffset().ToString("X4") + ")", "Go To Offset");
				if (Result->ReturnCode == DialogResult::Cancel || Result->Text == "")
					return;

				OffsetTextViewer->JumpToLine(Result->Text);
			}
			else
			{
				MessageBox::Show("This operation can only be performed in the offset viewer.",
								 SCRIPTEDITOR_TITLE,
								 MessageBoxButtons::OK,
								 MessageBoxIcon::Exclamation);
			}
		}

		void ConcreteWorkspaceView::SaveAll()
		{
			bool HasWarnings = false;
			for each (auto Itr in AssociatedModels)
			{
				if (ModelController()->Save(Itr.Key, IWorkspaceModel::SaveOperation::Default, HasWarnings) == false || HasWarnings)
				{
					if (Itr.Key == GetActiveModel())
						ShowMessageList();
				}
			}
		}

		void ConcreteWorkspaceView::CloseAll()
		{
			List<IWorkspaceModel^>^ Dissociate = gcnew List < IWorkspaceModel^ > ;
			bool Cancelled = false;
			for each (auto Itr in AssociatedModels)
			{
				// switch to dirty models to enable user interaction
				if (Itr.Key->Dirty)
					SelectTab(GetTab(Itr.Key));

				if (ModelController()->Close(Itr.Key, Cancelled))
					Dissociate->Add(Itr.Key);

				if (Cancelled)
					break;
			}

			DisallowBinding = true;
			BeginUpdate();

			for each (auto Itr in Dissociate)
				DissociateModel(Itr, true);

			Dissociate->Clear();
			DisallowBinding = false;
			EndUpdate();

			if (GetTabCount() == 0)
			{
				AllowDisposal = true;
				EditorForm->Close();
			}
		}

		void ConcreteWorkspaceView::DumpAllToDisk(String^ OutputDirectory, String^ FileExtension)
		{
			for each (auto Itr in AssociatedModels)
			{
				if (Itr.Key->New == false && Itr.Key->Initialized == true)
					ModelController()->SaveToDisk(Itr.Key, OutputDirectory, false, FileExtension);
			}
		}

		void ConcreteWorkspaceView::LoadAllFromDisk(array<String^>^ FilePaths)
		{
			NewTabOperationArgs^ E = gcnew NewTabOperationArgs;
			BeginUpdate();
			for each (auto Path in FilePaths)
			{
				E->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::LoadFromDisk;
				E->PathToFile = Path;
				E->BindAfterCreation = false;
				NewTab(E);
			}
			EndUpdate();
		}

		void ConcreteWorkspaceView::BeginUpdate()
		{
			EditorForm->SuspendLayout();
		}

		void ConcreteWorkspaceView::EndUpdate()
		{
			EditorForm->ResumeLayout(true);
		}

		void ConcreteWorkspaceView::Redraw()
		{
			EditorForm->Invalidate(true);
		}

		int ConcreteWorkspaceView::GetTabCount()
		{
			return EditorTabStrip->Tabs->Count;
		}

		void ConcreteWorkspaceView::SelectTab(DotNetBar::SuperTabItem^ Tab)
		{
			EditorTabStrip->SelectedTab = Tab;
			EditorTabStrip->TabStrip->EnsureVisible(Tab);
		}

		void ConcreteWorkspaceView::SelectTab(Keys Index)
		{
			int TabIndex = -1;

			switch (Index)
			{
			case Keys::D1:
				TabIndex = 0;
				break;
			case Keys::D2:
				TabIndex = 1;
				break;
			case Keys::D3:
				TabIndex = 2;
				break;
			case Keys::D4:
				TabIndex = 3;
				break;
			case Keys::D5:
				TabIndex = 4;
				break;
			case Keys::D6:
				TabIndex = 5;
				break;
			case Keys::D7:
				TabIndex = 6;
				break;
			case Keys::D8:
				TabIndex = 7;
				break;
			case Keys::D9:
				TabIndex = 8;
				break;
			default:
				break;
			}

			if (TabIndex != -1)
				SelectTab(TabIndex);
		}

		void ConcreteWorkspaceView::SelectTab(int Index)
		{
			if (Index == -1)
				SelectTab((DotNetBar::SuperTabItem^)(EditorTabStrip->Tabs[GetTabCount() - 1]));
			else if (Index < EditorTabStrip->Tabs->Count)
				SelectTab((DotNetBar::SuperTabItem^)(EditorTabStrip->Tabs[Index]));
		}

		void ConcreteWorkspaceView::SelectPreviousTab()
		{
			if (GetTabCount() < 2)
				return;

			if (EditorTabStrip->SelectPreviousTab() == false)
				SelectTab(-1);
		}

		void ConcreteWorkspaceView::SelectNextTab()
		{
			if (GetTabCount() < 2)
				return;

			if (EditorTabStrip->SelectNextTab() == false)
				SelectTab(0);
		}

		void ConcreteWorkspaceView::NewTab(NewTabOperationArgs^ E)
		{
			IWorkspaceModel^ New = nullptr;

			if (E->PostCreationOperation != NewTabOperationArgs::PostNewTabOperation::LoadFromDisk)
			{
				New = ModelFactory()->CreateModel(E->OpenArgs);
				AssociateModel(New, E->BindAfterCreation);
			}

			switch (E->PostCreationOperation)
			{
			case NewTabOperationArgs::PostNewTabOperation::None:
				break;
			case NewTabOperationArgs::PostNewTabOperation::New:
				ModelController()->New(New);
				break;
			case NewTabOperationArgs::PostNewTabOperation::Open:
				Debug::Assert(E->OpenArgs != nullptr);
				break;
			case NewTabOperationArgs::PostNewTabOperation::OpenList:
				ShowOpenDialog();
				break;
			case NewTabOperationArgs::PostNewTabOperation::LoadFromDisk:
				{
					if (preferences::SettingsHolder::Get()->General->LoadScriptUpdatesExistingScripts == false)
					{
						New = ModelFactory()->CreateModel(nullptr);
						ModelController()->New(New);
						AssociateModel(New, E->BindAfterCreation);
					}
					else
					{
						componentDLLInterface::ScriptData* ScriptData = nullptr;
						try
						{
							StreamReader^ FileParser = gcnew StreamReader(E->PathToFile);
							String^ Contents = FileParser->ReadToEnd()->Replace("\r\n", "\n");
							FileParser->Close();

							obScriptParsing::AnalysisData^ Data = gcnew obScriptParsing::AnalysisData;
							Data->PerformAnalysis(gcnew obScriptParsing::AnalysisData::Params);

							if (Data->Name != "")
							{
								CString CEID(Data->Name);
								ScriptData = nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(CEID.c_str());

								if (ScriptData)
									New = ModelFactory()->CreateModel(ScriptData);
								else
								{
									New = ModelFactory()->CreateModel(nullptr);
									ModelController()->New(New);
								}
							}
						}
						catch (Exception^ Excp)
						{
							DebugPrint("Couldn't read script name from file " + E->PathToFile + " for script updating!\n\tException: " + Excp->Message, true);
							New = ModelFactory()->CreateModel(nullptr);
							ModelController()->New(New);

							if (ScriptData)
								nativeWrapper::g_CSEInterfaceTable->DeleteInterOpData(ScriptData, false);
						}

						ModelController()->LoadFromDisk(New, E->PathToFile);
						AssociateModel(New, E->BindAfterCreation);
					}
				}

				break;
			case NewTabOperationArgs::PostNewTabOperation::SetText:
				ModelController()->New(New);
				ModelController()->SetText(New, E->NewText, true);
				break;
			}
		}

		void ConcreteWorkspaceView::ModelSubscribeEvents(IWorkspaceModel^ Model)
		{
			Model->StateChanged += ModelStateChangedHandler;
		}

		void ConcreteWorkspaceView::ModelUnsubscribeEvents(IWorkspaceModel^ Model)
		{
			Model->StateChanged -= ModelStateChangedHandler;
		}

		void ConcreteWorkspaceView::RemoveFromNavigationStacks(IWorkspaceModel^ Model)
		{
			Stack<IWorkspaceModel^>^ Buffer = gcnew Stack < IWorkspaceModel^ > ;

			while (NavigationStackBackward->Count)
			{
				IWorkspaceModel^ Current = NavigationStackBackward->Pop();
				if (Current != Model)
					Buffer->Push(Current);
			}

			while (Buffer->Count)
				NavigationStackBackward->Push(Buffer->Pop());

			while (NavigationStackForward->Count)
			{
				IWorkspaceModel^ Current = NavigationStackForward->Pop();
				if (Current != Model)
					Buffer->Push(Current);
			}

			while (Buffer->Count)
				NavigationStackForward->Push(Buffer->Pop());
		}

		void ConcreteWorkspaceView::RemoveFromFindReplaceAllResultCache(IWorkspaceModel^ Model)
		{
			for each (auto Itr in CachedFindReplaceAllResults)
			{
				for each (auto Data in Itr->ScriptsWithHits)
				{
					if (Data->ParentModel == Model)
						Data->ParentModel = nullptr;
				}
			}
		}

		void ConcreteWorkspaceView::Focus()
		{
			EditorForm->Focus();
		}

		void ConcreteWorkspaceView::ToggleMessageList(bool State)
		{
			if (State)
			{
				ToolBarMessageList->Checked = true;
				MessageList->Visible = true;
				MessageList->BringToFront();
			}
			else
			{
				ToolBarMessageList->Checked = false;
				MessageList->Visible = false;
				MessageList->SendToBack();
			}
		}

		void ConcreteWorkspaceView::ToggleBookmarkList(bool State)
		{
			if (State)
			{
				ToolBarBookmarkList->Checked = true;
				BookmarkList->Visible = true;
				BookmarkList->BringToFront();
			}
			else
			{
				ToolBarBookmarkList->Checked = false;
				BookmarkList->Visible = false;
				BookmarkList->SendToBack();
			}
		}

		void ConcreteWorkspaceView::ToggleFindResultList(bool State)
		{
			if (State)
			{
				ToolBarFindList->Checked = true;
				FindResultsList->Visible = true;
				FindResultsList->BringToFront();
			}
			else
			{
				ToolBarFindList->Checked = false;
				FindResultsList->Visible = false;
				FindResultsList->SendToBack();
			}
		}

		void ConcreteWorkspaceView::ToggleGlobalFindResultList(bool State)
		{
			if (State)
			{
				ToolBarGlobalFindList->Checked = true;
				GlobalFindList->Visible = true;
				GlobalFindList->BringToFront();
			}
			else
			{
				ToolBarGlobalFindList->Checked = false;
				GlobalFindList->Visible = false;
				GlobalFindList->SendToBack();
			}
		}

		void ConcreteWorkspaceView::ShowMessageList()
		{
			if (!Enabled)
				return;

			BeginUpdate();

			ToggleBookmarkList(false);
			ToggleFindResultList(false);
			ToggleGlobalFindResultList(false);

			ToggleMessageList(true);
			ToggleSecondaryPanel(true);

			EndUpdate();
		}

		void ConcreteWorkspaceView::ShowGlobalFindResultList()
		{
			if (!Enabled)
				return;

			BeginUpdate();

			ToggleBookmarkList(false);
			ToggleMessageList(false);
			ToggleFindResultList(false);

			ToggleGlobalFindResultList(true);
			ToggleSecondaryPanel(true);

			EndUpdate();
		}

		void ConcreteWorkspaceView::ShowFindResultList()
		{
			if (!Enabled)
				return;

			BeginUpdate();

			ToggleBookmarkList(false);
			ToggleMessageList(false);
			ToggleGlobalFindResultList(false);

			ToggleFindResultList(true);
			ToggleSecondaryPanel(true);

			EndUpdate();
		}

		void ConcreteWorkspaceView::ShowBookmarkList()
		{
			if (!Enabled)
				return;

			BeginUpdate();

			ToggleFindResultList(false);
			ToggleMessageList(false);
			ToggleGlobalFindResultList(false);

			ToggleBookmarkList(true);
			ToggleSecondaryPanel(true);

			EndUpdate();
		}

		void ConcreteWorkspaceView::HideAllLists()
		{
			if (!Enabled)
				return;

			BeginUpdate();

			ToggleMessageList(false);
			ToggleFindResultList(false);
			ToggleGlobalFindResultList(false);
			ToggleBookmarkList(false);
			ToggleSecondaryPanel(false);

			EndUpdate();
		}

		// ConcreteWorkspaceViewController
		void ConcreteWorkspaceViewController::AttachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model)
		{
			Debug::Assert(View != nullptr);
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			// we need multiple levels of indirection when swapping the textview to keep the form's layout intact
			// directly adding it to the splitter panel will screw up the toolbar's docking, among other things
			Concrete->AttachPanel->Controls->Add(Model->InternalView);
		}

		void ConcreteWorkspaceViewController::DetachModelInternalView(IWorkspaceView^ View, IWorkspaceModel^ Model)
		{
			Debug::Assert(View != nullptr);
			Debug::Assert(Model != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			Concrete->AttachPanel->Controls->Remove(Model->InternalView);
		}

		void ConcreteWorkspaceViewController::BubbleKeyDownEvent(IWorkspaceView^ View, KeyEventArgs^ E)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			IWorkspaceModel^ Active = Concrete->GetActiveModel();

			switch (E->KeyCode)
			{
			case Keys::N:
				if (E->Modifiers == Keys::Control)
					Concrete->ModelController()->New(Active);

				break;
			case Keys::O:
				if (E->Modifiers == Keys::Control)
					Concrete->ShowOpenDialog();

				break;
			case Keys::S:
				if (E->Modifiers == Keys::Control)
				{
					bool HasWarnings = false;
					if (Concrete->ModelController()->Save(Active, IWorkspaceModel::SaveOperation::Default, HasWarnings) == false || HasWarnings)
						Concrete->ShowMessageList();
				}
				else if (E->Control && E->Shift)
					Concrete->SaveAll();

				break;
			case Keys::D:
				if (E->Modifiers == Keys::Control)
					Concrete->ShowDeleteDialog();

				break;
			case Keys::Left:
				if (E->Control && E->Alt)
					Concrete->ModelController()->Previous(Active);

				break;
			case Keys::Right:
				if (E->Control && E->Alt)
					Concrete->ModelController()->Next(Active);

				break;
			case Keys::F:
			case Keys::H:
				if (E->Control)
					Concrete->ShowFindReplaceDialog(true);

				break;
			case Keys::G:
				if (E->Modifiers == Keys::Control)
					Concrete->GotoLine();

				break;
			case Keys::E:
				if (E->Modifiers == Keys::Control)
					Concrete->GotoOffset();

				break;
			case Keys::T:
				if (E->Modifiers == Keys::Control)
					Concrete->NewTab(gcnew NewTabOperationArgs);

				break;
			case Keys::PageDown:
				if (E->Modifiers == Keys::Control)
					Concrete->SelectNextTab();

				break;
			case Keys::PageUp:
				if (E->Modifiers == Keys::Control)
					Concrete->SelectPreviousTab();

				break;
			case Keys::Tab:
				if (E->Control == true && E->Shift == false)
					Concrete->SelectNextTab();
				else if (E->Control == true && E->Shift == true)
					Concrete->SelectPreviousTab();

				break;
			case Keys::F4:
				if (E->Modifiers == Keys::Control)
				{
					IWorkspaceModel^ Active = Concrete->GetActiveModel();
					bool Cancelled = false;
					if (Active->Controller->Close(Active, Cancelled))
						Concrete->DissociateModel(Active, true);
				}

				break;
			case Keys::D1:
			case Keys::D2:
			case Keys::D3:
			case Keys::D4:
			case Keys::D5:
			case Keys::D6:
			case Keys::D7:
			case Keys::D8:
			case Keys::D9:
				if (E->Modifiers == Keys::Control)
					Concrete->SelectTab(E->KeyCode);

				break;
			case Keys::Space:
				if (E->Control && E->Alt == false)
				{
					Concrete->TabStripFilter->Show();
					E->Handled = true;
				}

				break;
			}
		}

		void ConcreteWorkspaceViewController::Jump(IWorkspaceView^ View, IWorkspaceModel^ From, String^ ScriptEditorID)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			IWorkspaceModel^ Existing = Concrete->GetModel(ScriptEditorID);
			if (Existing)
				Concrete->SelectTab(Concrete->GetTab(Existing));
			else
			{
				CString EID(ScriptEditorID);
				NewTabOperationArgs^ E = gcnew NewTabOperationArgs;

				E->PostCreationOperation = NewTabOperationArgs::PostNewTabOperation::Open;
				E->OpenArgs = nativeWrapper::g_CSEInterfaceTable->EditorAPI.LookupScriptableFormByEditorID(EID.c_str());

				if (E->OpenArgs);
					Concrete->NewTab(E);
			}
		}

		int ConcreteWorkspaceViewController::FindReplace(IWorkspaceView^ View, textEditors::IScriptTextEditor::FindReplaceOperation Operation,
														  String^ Query, String^ Replacement, textEditors::IScriptTextEditor::FindReplaceOptions Options, bool Global)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;

			if (Query->Length == 0)
				return -1;

			if (Global)
			{
				FindReplaceAllResults^ GlobalResult = gcnew FindReplaceAllResults;
				GlobalResult->Operation = Operation;
				GlobalResult->Query = Query;
				GlobalResult->Replacement = Replacement;
				GlobalResult->Options = Options;

				for each (auto Itr in Concrete->AssociatedModels)
				{
					IWorkspaceModel^ Model = Itr.Key;
					textEditors::IScriptTextEditor::FindReplaceResult^ Result = Concrete->ModelController()->FindReplace(Model,
																														 Operation,
																														 Query,
																														 Replacement,
																														 Options);
					if (Result->HasError)
						break;

					if (Result->TotalHitCount)
						GlobalResult->Add(Model, Result);
				}

				if (GlobalResult->TotalHitCount)
				{
					// we want the newest result to show first
					Concrete->CachedFindReplaceAllResults->Insert(0, GlobalResult);
					Concrete->GlobalFindList->SetObjects(Concrete->CachedFindReplaceAllResults);
					Concrete->ShowGlobalFindResultList();
				}
				else
				{
					MessageBox("No matches were found.", MessageBoxButtons::OK, MessageBoxIcon::Information);
				}

				return -1;
			}
			else
			{
				textEditors::IScriptTextEditor::FindReplaceResult^ Result = Concrete->GetActiveModel()->Controller->FindReplace(Concrete->GetActiveModel(),
																																Operation,
																																Query,
																																Replacement,
																																Options);
				if (Result->TotalHitCount && Operation != textEditors::IScriptTextEditor::FindReplaceOperation::CountMatches)
					Concrete->ShowFindResultList();

				return Result->TotalHitCount;
			}
		}

		void ConcreteWorkspaceViewController::ShowOutline(IWorkspaceView^ View, obScriptParsing::Structurizer^ Data, IWorkspaceModel^ Model)
		{
			Debug::Assert(View != nullptr && Model != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			Concrete->OutlineView->Show(Data, Model);
		}

		void ConcreteWorkspaceViewController::Redraw(IWorkspaceView^ View)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			Concrete->Redraw();
		}

		System::Windows::Forms::DialogResult ConcreteWorkspaceViewController::MessageBox(String^ Message, MessageBoxButtons Buttons, MessageBoxIcon Icon)
		{
			return MessageBox::Show(Message, SCRIPTEDITOR_TITLE, Buttons, Icon);
		}

		void ConcreteWorkspaceViewController::NewTab(IWorkspaceView^ View, NewTabOperationArgs^ E)
		{
			Debug::Assert(View != nullptr);
			ConcreteWorkspaceView^ Concrete = (ConcreteWorkspaceView^)View;
			Concrete->NewTab(E);
		}

		// ConcreteWorkspaceViewFactory
		ConcreteWorkspaceViewFactory::ConcreteWorkspaceViewFactory()
		{
			Allocations = gcnew List < ConcreteWorkspaceView^ >;
		}

		ConcreteWorkspaceViewFactory::~ConcreteWorkspaceViewFactory()
		{
			for each (auto Itr in Allocations)
				delete Itr;

			Allocations->Clear();
		}

		IWorkspaceView^ ConcreteWorkspaceViewFactory::CreateView(int X, int Y, int Width, int Height)
		{
			Rectangle Bounds(X, Y, Width, Height);
			ConcreteWorkspaceView^ New = gcnew ConcreteWorkspaceView((ConcreteWorkspaceViewController^)CreateController(),
																	 this,
																	 Bounds);
			Allocations->Add(New);
			return New;
		}

		IWorkspaceViewController^ ConcreteWorkspaceViewFactory::CreateController()
		{
			return gcnew ConcreteWorkspaceViewController();
		}

		void ConcreteWorkspaceViewFactory::Remove(ConcreteWorkspaceView^ Allocation)
		{
			Allocations->Remove(Allocation);
		}

		void ConcreteWorkspaceViewFactory::Clear()
		{
			// this is totally unbecoming...
			List<ConcreteWorkspaceView^>^ Buffer = gcnew List<ConcreteWorkspaceView^>(Allocations);
			for each (ConcreteWorkspaceView^ Itr in Buffer)
				delete Itr;

			Debug::Assert(Allocations->Count == 0);
			Buffer->Clear();
		}

	}
}