#include "ScriptEditorControllerImplComponents.h"
#include "Preferences.h"
#include "IntelliSenseBackend.h"


namespace cse
{


namespace scriptEditor
{


namespace controllerImpl
{


namespace components
{


void ViewTabTearingHelper::TearingEventHandler(Object^ Sender, MouseEventArgs^ E)
{
	Debug::Assert(TornDocument != nullptr);
	Debug::Assert(SourceController != nullptr);

	auto LocalTornDocument = TornDocument;
	auto LocalSourceController = SourceController;

	End();

	switch (E->Button)
	{
	case MouseButtons::Left:
		{
			bool Relocated = false;
			bool SameTabStrip = false;

			IntPtr Wnd = nativeWrapper::WindowFromPoint(E->Location);
			if (Wnd != IntPtr::Zero)
			{
				auto UnderMouse = Control::FromHandle(Wnd);
				if (UnderMouse)
				{
					auto UserData = UnderMouse->Tag;
					if (UserData)
					{
						view::components::ITabStrip^ TabStrip = nullptr;
						try
						{
							TabStrip = safe_cast<view::components::ITabStrip^>(UserData);
						}
						catch (...) {}

						if (TabStrip)
						{
							Debug::Assert(ActiveTabStrips->ContainsKey(TabStrip));

							auto ParentController = ActiveTabStrips[TabStrip];
							if (ParentController == LocalSourceController)
								SameTabStrip = true;
							else
							{
								Relocated = true;
								ParentController->RelocateDocument(LocalTornDocument, LocalSourceController);
							}
						}
					}
				}
			}

			if (!Relocated && !SameTabStrip)
			{
				auto NewBounds = LocalSourceController->View->GetComponentByRole(view::eViewRole::MainWindow)->AsForm()->Bounds;
				NewBounds.X = E->Location.X;
				NewBounds.Y = E->Location.Y;

				auto NewController = LocalSourceController->New();
				NewController->RelocateDocument(LocalTornDocument, LocalSourceController);
				NewController->View->Reveal(NewBounds);
			}
		}

		break;
	}
}

void ViewTabTearingHelper::End()
{
	Debug::Assert(Active == true);

	Active = false;
	TornDocument = nullptr;
	SourceController = nullptr;

	GlobalInputMonitor::HookManager::MouseUp -= TearingEventDelegate;
}

ViewTabTearingHelper::ViewTabTearingHelper()
{
	TornDocument = nullptr;
	SourceController = nullptr;
	Active = false;
	ActiveTabStrips = gcnew Dictionary<view::components::ITabStrip^, IScriptEditorController^>;

	TearingEventDelegate = gcnew MouseEventHandler(this, &ViewTabTearingHelper::TearingEventHandler);
}

ViewTabTearingHelper::~ViewTabTearingHelper()
{
	if (Active)
		End();

	SAFEDELETE_CLR(TearingEventDelegate);
	ActiveTabStrips->Clear();
}

void ViewTabTearingHelper::InitiateHandling(model::IScriptDocument^ Tearing, IScriptEditorController^ ParentController)
{
	Debug::Assert(Active == false);
	Debug::Assert(Tearing != nullptr);
	Debug::Assert(ParentController != nullptr);

	Active = true;
	TornDocument = Tearing;
	SourceController = ParentController;

	GlobalInputMonitor::HookManager::MouseUp += TearingEventDelegate;
}

void ViewTabTearingHelper::RegisterTabStrip(view::components::ITabStrip^ TabStrip, IScriptEditorController^ ParentController)
{
	Debug::Assert(ActiveTabStrips->ContainsKey(TabStrip) == false);

	ActiveTabStrips->Add(TabStrip, ParentController);
}

void ViewTabTearingHelper::DeregisterTabStrip(view::components::ITabStrip^ TabStrip)
{
	Debug::Assert(ActiveTabStrips->ContainsKey(TabStrip));

	ActiveTabStrips->Remove(TabStrip);
}

ViewTabTearingHelper^ ViewTabTearingHelper::Get()
{
	if (Singleton == nullptr)
		Singleton = gcnew ViewTabTearingHelper();

	return Singleton;
}

Image^ DocumentNavigationHelper::GetNodeIcon(obScriptParsing::Structurizer::Node::eNodeType NodeType)
{
	if (NodeIcons->Count == 0)
	{
		auto ImageResources = view::components::CommonIcons::Get()->ResourceManager;
		NodeIcons->Add(obScriptParsing::Structurizer::Node::eNodeType::Invalid, ImageResources->CreateImage("Transparent"));
		NodeIcons->Add(obScriptParsing::Structurizer::Node::eNodeType::VariableDeclaration, ImageResources->CreateImage("VariableDeclarations"));
		NodeIcons->Add(obScriptParsing::Structurizer::Node::eNodeType::ScriptBlock, ImageResources->CreateImage("ScriptBlock"));
		NodeIcons->Add(obScriptParsing::Structurizer::Node::eNodeType::BasicConditionalBlock, ImageResources->CreateImage("ConditionalBlock"));
		NodeIcons->Add(obScriptParsing::Structurizer::Node::eNodeType::LoopBlock, ImageResources->CreateImage("LoopBlock"));
	}

	return NodeIcons[NodeType];
}

System::Object^ DocumentNavigationHelper::OutlineListViewAspectGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	auto Model = safe_cast<obScriptParsing::Structurizer::Node^>(RowObject);
	return Model->Description;
}

System::Object^ DocumentNavigationHelper::OutlineListViewImageGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	auto Model = safe_cast<obScriptParsing::Structurizer::Node^>(RowObject);
	return GetNodeIcon(Model->Type);
}

bool DocumentNavigationHelper::OutlineListViewCanExpandGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	auto Model = safe_cast<obScriptParsing::Structurizer::Node^>(RowObject);
	return Model->Children->Count > 0;
}

System::Collections::IEnumerable^ DocumentNavigationHelper::OutlineListViewChildrenGetter(Object^ RowObject)
{
	if (RowObject == nullptr)
		return nullptr;

	auto Model = safe_cast<obScriptParsing::Structurizer::Node^>(RowObject);
	return Model->Children;
}

DocumentNavigationHelper::CrumbItemData::CrumbItemData(model::IScriptDocument^ ParentScriptDocument, obScriptParsing::Structurizer::Node^ Scope)
{
	this->ParentScriptDocument = ParentScriptDocument;
	this->Scope = Scope;
}

void DocumentNavigationHelper::Preferences_Changed(Object^ Sender, EventArgs^ E)
{
	NavigationBarVisible = preferences::SettingsHolder::Get()->Appearance->ShowScopeBar;
}

void DocumentNavigationHelper::GenerateAllCrumbs(obScriptParsing::Structurizer^ Data, model::IScriptDocument^ SourceDocument)
{
	ResetCrumbs();

	if (!NavigationBarVisible)
		return;

	Root->Text = Data->RootName;
	Root->Tag = gcnew CrumbItemData(SourceDocument, gcnew obScriptParsing::Structurizer::Node(obScriptParsing::Structurizer::Node::eNodeType::Invalid, 1, 1, Data->RootName));

	for each (auto Itr in Data->Output)
		CreateNewCrumb(Itr, Root, SourceDocument, true);

	CachedStructureData = Data;
}

void DocumentNavigationHelper::EnumerateChildCrumbs(view::components::ICrumbBarItem^ Item, model::IScriptDocument^ SourceDocument)
{
	auto Data = safe_cast<CrumbItemData^>(Item->Tag);

	for each (auto Itr in Data->Scope->Children)
		CreateNewCrumb(Itr, Item, SourceDocument, true);
}

void DocumentNavigationHelper::CreateNewCrumb(obScriptParsing::Structurizer::Node^ Source, view::components::ICrumbBarItem^ Parent, model::IScriptDocument^ SourceDocument, bool EnumerateChildren)
{
	auto NewItem = CrumbBar->AllocateNewItem();
	String^ LineText = Source->Description;
	if (LineText->Length > MaxCrumbItemTextLength)
		LineText = LineText->Substring(0, MaxCrumbItemTextLength) + "...";

	NewItem->Text = LineText;
	NewItem->Tooltip = Source->Description;
	NewItem->Tag = gcnew CrumbItemData(SourceDocument, Source);
	NewItem->Image = GetNodeIcon(Source->Type);

	ActiveCrumbs->Add(Source, NewItem);
	Parent->AddChild(NewItem);

	if (EnumerateChildren)
		EnumerateChildCrumbs(NewItem, SourceDocument);
}

void DocumentNavigationHelper::ResetCrumbs()
{
	Root->Text = "";
	Root->Tag = nullptr;

	ActiveCrumbs->Clear();
	Root->ClearChildren();

	CrumbBar->SelectedItem = Root;
	CachedStructureData = nullptr;
}

void DocumentNavigationHelper::UpdateToLine(UInt32 Line)
{
	if (CachedStructureData == nullptr)
		return;

	auto ContainingNode = CachedStructureData->GetContainingNode(Line);
	if (ContainingNode == nullptr)
		return;

	for each (auto Itr in ActiveCrumbs)
	{
		if (Itr.Key == ContainingNode)
		{
			CrumbBar->SelectedItem = Itr.Value;
			return;
		}
	}

	OutlineListView->SelectedObject = ContainingNode;
	OutlineListView->EnsureItemVisible(ContainingNode);
}

void DocumentNavigationHelper::InitOutlineListView()
{
	auto Column = OutlineListView->AllocateNewColumn();
	Column->FillsFreeSpace = true;
	Column->Text = "Structure";
	Column->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&DocumentNavigationHelper::OutlineListViewAspectGetter));
	Column->SetImageGetter(gcnew view::components::IObjectListViewColumn::ImageGetter(&DocumentNavigationHelper::OutlineListViewImageGetter));

	OutlineListView->AddColumn(Column);
	OutlineListView->SetCanExpandGetter(gcnew view::components::IObjectListView::CanExpandGetter(&DocumentNavigationHelper::OutlineListViewCanExpandGetter));
	OutlineListView->SetChildrenGetter(gcnew view::components::IObjectListView::ChildrenGetter(&DocumentNavigationHelper::OutlineListViewChildrenGetter));
	OutlineListView->HeaderVisible = false;
}

void DocumentNavigationHelper::RefreshOutlineListView(obScriptParsing::Structurizer^ StructureData)
{
	if (StructureData)
	{
		OutlineListView->SetObjects(StructureData->Output, true);
		OutlineListView->ExpandAll();
		if (StructureData->CurrentScope)
		{
			OutlineListView->SelectedObject = StructureData->CurrentScope;
			OutlineListView->EnsureItemVisible(StructureData->CurrentScope);
		}
	}
	else
		OutlineListView->ClearObjects();

	CachedStructureData = StructureData;
}

DocumentNavigationHelper::DocumentNavigationHelper(view::IScriptEditorView^ View)
{
	CrumbBar = View->GetComponentByRole(view::eViewRole::NavigationBar)->AsCrumbBar();
	OutlineListView = View->GetComponentByRole(view::eViewRole::OutlineView_TreeView)->AsObjectListView();
	CachedStructureData = nullptr;
	Root = CrumbBar->AllocateNewItem();
	Root->Image = view::components::CommonIcons::Get()->ResourceManager->CreateImage("Script");
	ActiveCrumbs = gcnew Dictionary<obScriptParsing::Structurizer::Node^, view::components::ICrumbBarItem^>;

	PreferencesChangedEventHandler = gcnew EventHandler(this, &DocumentNavigationHelper::Preferences_Changed);
	preferences::SettingsHolder::Get()->PreferencesChanged += PreferencesChangedEventHandler;

	CrumbBar->ClearItems();
	CrumbBar->AddItem(Root);

	InitOutlineListView();
}

DocumentNavigationHelper::~DocumentNavigationHelper()
{
	preferences::SettingsHolder::Get()->PreferencesChanged -= PreferencesChangedEventHandler;

	SAFEDELETE_CLR(Root);
	SAFEDELETE_CLR(PreferencesChangedEventHandler);

	CrumbBar = nullptr;
	OutlineListView = nullptr;
}

void DocumentNavigationHelper::SyncWithDocument(model::IScriptDocument^ SourceDocument)
{
	if (SourceDocument == nullptr)
	{
		ResetCrumbs();
		RefreshOutlineListView(nullptr);
		return;
	}

	auto LastAnalysisData = SourceDocument->BackgroundAnalyzer->LastAnalysisResult;
	if (LastAnalysisData)
	{
		auto StructureData = SourceDocument->NavigationHelper->GenerateStructureData(LastAnalysisData);
		if (StructureData->Valid)
		{
			GenerateAllCrumbs(StructureData, SourceDocument);
			RefreshOutlineListView(StructureData);
			UpdateToLine(SourceDocument->TextEditor->CurrentLine);
		}
	}
}

void DocumentNavigationHelper::HandleCrumbBarEvent(view::ViewComponentEvent^ E, model::IScriptDocument^ SourceDocument)
{
	auto EventType = safe_cast<view::components::ICrumbBar::eEvent>(E->EventType);
	switch (EventType)
	{
	case view::components::ICrumbBar::eEvent::ItemClick:
	{
		auto Args = safe_cast<view::components::ICrumbBar::ItemClickEventArgs^>(E->EventArgs);
		if (Args->Item->Tag)
		{
			auto Data = safe_cast<CrumbItemData^>(Args->Item->Tag);
			Data->ParentScriptDocument->TextEditor->ScrollToLine(Data->Scope->StartLine);
		}

		break;
	}
	default:
		throw gcnew NotImplementedException();
	}
}

void DocumentNavigationHelper::HandleListViewEvent(view::ViewComponentEvent^ E, model::IScriptDocument^ SourceDocument)
{
	auto EventType = safe_cast<view::components::IObjectListView::eEvent>(E->EventType);
	switch (EventType)
	{
	case view::components::IObjectListView::eEvent::ItemActivate:
	{
		auto Args = safe_cast<view::components::IObjectListView::ItemActivateEventArgs^>(E->EventArgs);
		if (Args->ItemModel)
		{
			auto Data = safe_cast<obScriptParsing::Structurizer::Node^>(Args->ItemModel);
			SourceDocument->TextEditor->ScrollToLine(Data->StartLine);
		}

		break;
	}
	default:
		throw gcnew NotImplementedException();
	}
}

void DocumentNavigationHelper::HandleNavigationChangedEvent(model::components::INavigationHelper::NavigationChangedEventArgs^ E, model::IScriptDocument^ SourceDocument)
{
	if (E->StructureChanged)
	{
		if (E->StructureData == nullptr)
			ResetCrumbs();
		else
			GenerateAllCrumbs(E->StructureData, SourceDocument);

		RefreshOutlineListView(E->StructureData);
	}

	if (E->LineChanged)
		UpdateToLine(E->CurrentLine);
}

GlobalFindReplaceResult::ScriptDocumentData::ScriptDocumentData(model::IScriptDocument^ Source, textEditor::FindReplaceResult^ OperationResult)
{
	this->Source = Source;
	this->SourceDescription = Source->ScriptEditorID + "(" + Source->ScriptFormID.ToString("X8") + ")";
	this->OperationResult = OperationResult;
}

GlobalFindReplaceResult::GlobalFindReplaceResult()
{
	Operation = textEditor::eFindReplaceOperation::CountMatches;
	Query = "";
	Replacement = "";
	Options = textEditor::eFindReplaceOptions::None;
	DocumentsWithHits = gcnew List<ScriptDocumentData^>;
}

void GlobalFindReplaceResult::Add(model::IScriptDocument^ ScriptDocument, textEditor::FindReplaceResult^ OperationResult)
{
	Debug::Assert(OperationResult->HasError == false);
	DocumentsWithHits->Add(gcnew ScriptDocumentData(ScriptDocument, OperationResult));
}

int GlobalFindReplaceResult::TotalHitCount::get()
{
	int Count = 0;

	for each (auto Instance in DocumentsWithHits)
		Count += Instance->TotalHits;

	return Count;
}

bool GlobalFindReplaceResult::GenericCanExpandGetter(Object^ E)
{
	if (E->GetType() == GlobalFindReplaceResult::typeid)
		return true;
	else if (E->GetType() == GlobalFindReplaceResult::ScriptDocumentData::typeid)
		return true;
	else
		return false;
}

System::Collections::IEnumerable^ GlobalFindReplaceResult::GenericChildrenGetter(Object^ E)
{
	if (E->GetType() == GlobalFindReplaceResult::typeid)
	{
		auto TopItem = safe_cast<GlobalFindReplaceResult^>(E);
		return TopItem->DocumentsWithHits;
	}

	if (E->GetType() == GlobalFindReplaceResult::ScriptDocumentData::typeid)
	{
		auto PerScript = safe_cast<GlobalFindReplaceResult::ScriptDocumentData^>(E);
		return PerScript->OperationResult->Hits;
	}

	return nullptr;
}

System::Object^ GlobalFindReplaceResult::TextAspectGetter(Object^ E)
{
	if (E->GetType() == GlobalFindReplaceResult::typeid)
	{
		auto TopItem = safe_cast<GlobalFindReplaceResult^>(E);
		switch (TopItem->Operation)
		{
		case textEditor::eFindReplaceOperation::Replace:
			return "Replace \"" + TopItem->Query + "\" with \"" + TopItem->Replacement + "\" (" + TopItem->TotalHitCount + " hits in " + TopItem->DocumentsWithHits->Count + " script(s))";
		default:
			return "Search \"" + TopItem->Query + "\" (" + TopItem->TotalHitCount + " hits in " + TopItem->DocumentsWithHits->Count + " script(s))";
		}
	}

	if (E->GetType() == GlobalFindReplaceResult::ScriptDocumentData::typeid)
	{
		auto PerScript = safe_cast<GlobalFindReplaceResult::ScriptDocumentData^>(E);
		return "Script " + PerScript->SourceDescription;
	}

	if (E->GetType() == textEditor::FindReplaceResult::HitData::typeid)
	{
		auto PerLine = safe_cast<textEditor::FindReplaceResult::HitData^>(E);
		return PerLine->Text;
	}

	return String::Empty;
}

System::Object^ GlobalFindReplaceResult::LineAspectGetter(Object^ E)
{
	if (E->GetType() != textEditor::FindReplaceResult::HitData::typeid)
		return String::Empty;

	auto Model = safe_cast<textEditor::FindReplaceResult::HitData^>(E);
	return Model->Line;
}

System::Object^ GlobalFindReplaceResult::HitsAspectGetter(Object^ E)
{
	if (E->GetType() == GlobalFindReplaceResult::ScriptDocumentData::typeid)
	{
		auto PerScript = safe_cast<GlobalFindReplaceResult::ScriptDocumentData^>(E);
		return PerScript->TotalHits;
	}

	if (E->GetType() == textEditor::FindReplaceResult::HitData::typeid)
	{
		auto PerLine = safe_cast<textEditor::FindReplaceResult::HitData^>(E);
		return PerLine->HitCount;
	}

	return String::Empty;
}

FindReplaceHelper::FindReplaceSourceDropdownItem::FindReplaceSourceDropdownItem(eFindReplaceSource Source)
{
	this->Source = Source;
}

bool FindReplaceHelper::FindReplaceSourceDropdownItem::Equals(Object^ obj)
{
	if (obj == nullptr)
		return false;

	if (obj->GetType() == eFindReplaceSource::typeid)
		return safe_cast<eFindReplaceSource>(obj) == this->Source;

	if (obj->GetType() == FindReplaceHelper::FindReplaceSourceDropdownItem::typeid)
		return safe_cast<FindReplaceHelper::FindReplaceSourceDropdownItem^>(obj)->Source == this->Source;

	return false;
}

int FindReplaceHelper::FindReplaceSourceDropdownItem::GetHashCode()
{
	return Source.GetHashCode();
}

System::String^ FindReplaceHelper::FindReplaceSourceDropdownItem::ToString()
{
	switch (Source)
	{
	case eFindReplaceSource::CurrentDocumentSelection:
		return "Selection (Current Script)";
	case eFindReplaceSource::CurrentDocumentFullText:
		return "Entire Text (Current Script)";
	case eFindReplaceSource::AllDocumentsSelection:
		return "Selection (All Open Scripts)";
	case eFindReplaceSource::AllDocumentsFullText:
		return "Entire Text (All Open Scripts)";
	default:
		throw gcnew NotImplementedException;
	}

	return nullptr;
}

System::Object^ FindReplaceHelper::FindReplaceResultsListLineNumberAspectGetter(Object^ E)
{
	auto Model = safe_cast<model::components::ScriptFindResult^>(E);
	if (Model == nullptr)
		return nullptr;
	else if (!Model->Valid)
		return nullptr;

	return Model->Line;
}

System::Object^ FindReplaceHelper::FindReplaceResultsListTextAspectGetter(Object^ E)
{
	auto Model = safe_cast<model::components::ScriptFindResult^>(E);
	if (Model == nullptr)
		return nullptr;

	return Model->Text;
}

System::Object^ FindReplaceHelper::FindReplaceResultsListHitsAspectGetter(Object^ E)
{
	auto Model = safe_cast<model::components::ScriptFindResult^>(E);
	if (Model == nullptr)
		return nullptr;

	return Model->Hits;
}

void FindReplaceHelper::Preferences_Changed(Object^ Sender, EventArgs^ E)
{
	LoadPreferences();
}

void FindReplaceHelper::InitResultsListViews()
{
	{
		auto ColumnLine = FindReplaceResultsListView->AllocateNewColumn();
		auto ColumnText = FindReplaceResultsListView->AllocateNewColumn();
		auto ColumnHits = FindReplaceResultsListView->AllocateNewColumn();

		ColumnLine->Text = "Line";
		ColumnLine->MinimumWidth = 40;
		ColumnLine->MaximumWidth = 40;
		ColumnLine->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&FindReplaceHelper::FindReplaceResultsListLineNumberAspectGetter));

		ColumnText->Text = "Code";
		ColumnText->MinimumWidth = 600;
		ColumnText->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&FindReplaceHelper::FindReplaceResultsListTextAspectGetter));

		ColumnHits->Text = "Hits";
		ColumnHits->MinimumWidth = 40;
		ColumnHits->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&FindReplaceHelper::FindReplaceResultsListHitsAspectGetter));

		FindReplaceResultsListView->AddColumn(ColumnLine);
		FindReplaceResultsListView->AddColumn(ColumnText);
		FindReplaceResultsListView->AddColumn(ColumnHits);
	}

	{
		auto ColumnText = GlobalFindReplaceResultsListView->AllocateNewColumn();
		auto ColumnLine = GlobalFindReplaceResultsListView->AllocateNewColumn();
		auto ColumnHits = GlobalFindReplaceResultsListView->AllocateNewColumn();

		ColumnText->Text = "Code";
		ColumnText->Width = 500;
		ColumnText->MinimumWidth = 600;
		ColumnLine->MaximumWidth = 40;
		ColumnText->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&GlobalFindReplaceResult::TextAspectGetter));

		ColumnLine->Text = "Line";
		ColumnLine->MinimumWidth = 40;
		ColumnLine->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&GlobalFindReplaceResult::LineAspectGetter));

		ColumnHits->Text = "Hits";
		ColumnHits->MinimumWidth = 40;
		ColumnHits->SetAspectGetter(gcnew view::components::IObjectListViewColumn::AspectGetter(&GlobalFindReplaceResult::HitsAspectGetter));

		GlobalFindReplaceResultsListView->AddColumn(ColumnLine);
		GlobalFindReplaceResultsListView->AddColumn(ColumnText);
		GlobalFindReplaceResultsListView->AddColumn(ColumnHits);

		GlobalFindReplaceResultsListView->SetCanExpandGetter(gcnew view::components::IObjectListView::CanExpandGetter(&GlobalFindReplaceResult::GenericCanExpandGetter));
		GlobalFindReplaceResultsListView->SetChildrenGetter(gcnew view::components::IObjectListView::ChildrenGetter(&GlobalFindReplaceResult::GenericChildrenGetter));
	}
}

void FindReplaceHelper::InitFindReplaceLookInDropdown()
{
	FindReplaceLookInDropdown->ClearDropdownItems();

	auto SourceCurrentDoc = gcnew FindReplaceSourceDropdownItem(eFindReplaceSource::CurrentDocumentFullText);
	FindReplaceLookInDropdown->AddDropdownItem(SourceCurrentDoc, false);
	FindReplaceLookInDropdown->AddDropdownItem(gcnew FindReplaceSourceDropdownItem(eFindReplaceSource::AllDocumentsFullText), false);

	FindReplaceLookInDropdown->AddDropdownItem(gcnew FindReplaceSourceDropdownItem(eFindReplaceSource::CurrentDocumentSelection), false);
	FindReplaceLookInDropdown->AddDropdownItem(gcnew FindReplaceSourceDropdownItem(eFindReplaceSource::AllDocumentsSelection), false);

	FindReplaceLookInDropdown->Selection = SourceCurrentDoc;
}

void FindReplaceHelper::LoadPreferences()
{
	FindReplaceOptionsButtonMatchCase->Checked = !preferences::SettingsHolder::Get()->FindReplace->CaseInsensitive;
	FindReplaceOptionsButtonMatchWholeWord->Checked = preferences::SettingsHolder::Get()->FindReplace->MatchWholeWord;
	FindReplaceOptionsButtonUseRegEx->Checked = preferences::SettingsHolder::Get()->FindReplace->UseRegEx;
	FindReplaceOptionsButtonIgnoreComments->Checked = preferences::SettingsHolder::Get()->FindReplace->IgnoreComments;
}

void FindReplaceHelper::SavePreferences()
{
	preferences::SettingsHolder::Get()->FindReplace->CaseInsensitive = !FindReplaceOptionsButtonMatchCase->Checked;
	preferences::SettingsHolder::Get()->FindReplace->MatchWholeWord = FindReplaceOptionsButtonMatchWholeWord->Checked;
	preferences::SettingsHolder::Get()->FindReplace->UseRegEx = FindReplaceOptionsButtonUseRegEx->Checked;
	preferences::SettingsHolder::Get()->FindReplace->IgnoreComments = FindReplaceOptionsButtonIgnoreComments->Checked;
}

textEditor::eFindReplaceOptions FindReplaceHelper::GetSelectedOptions()
{
	auto Options = textEditor::eFindReplaceOptions::None;
	if (!FindReplaceOptionsButtonMatchCase->Checked)
		Options = Options | textEditor::eFindReplaceOptions::CaseInsensitive;

	if (FindReplaceOptionsButtonMatchWholeWord->Checked)
		Options = Options | textEditor::eFindReplaceOptions::MatchWholeWord;

	if (FindReplaceOptionsButtonUseRegEx->Checked)
		Options = Options | textEditor::eFindReplaceOptions::RegEx;

	if (FindReplaceLookInDropdown->Selection->Equals(eFindReplaceSource::CurrentDocumentSelection) ||
		FindReplaceLookInDropdown->Selection->Equals(eFindReplaceSource::AllDocumentsSelection))
	{
		Options = Options | textEditor::eFindReplaceOptions::InSelection;
	}

	if (FindReplaceOptionsButtonIgnoreComments->Checked)
		Options = Options | textEditor::eFindReplaceOptions::IgnoreComments;

	return Options;
}

void FindReplaceHelper::UpdateDropdownStrings(view::components::IComboBox^ ComboBox)
{
	auto CurrentText = ComboBox->Text;
	Object^ LastItem = "";
	auto ItemsEnumerator = ComboBox->Items->GetEnumerator();
	if (ItemsEnumerator->MoveNext())
		LastItem = ItemsEnumerator->Current;

	if (CurrentText != LastItem->ToString())
		ComboBox->AddDropdownItem(CurrentText, true);
}

void FindReplaceHelper::PerformOperation(textEditor::eFindReplaceOperation Operation, IScriptEditorController^ Controller)
{
	auto Query = FindReplaceFindDropdown->Text;
	if (Query->Length == 0)
		return;

	auto Replacement = FindReplaceReplaceDropdown->Text;

	if (Operation == textEditor::eFindReplaceOperation::Find)
		UpdateDropdownStrings(FindReplaceFindDropdown);

	if (Operation == textEditor::eFindReplaceOperation::Replace && Replacement->Length != 0)
		UpdateDropdownStrings(FindReplaceReplaceDropdown);

	auto Options = GetSelectedOptions();
	bool GlobalOperation = FindReplaceLookInDropdown->Selection->Equals(eFindReplaceSource::AllDocumentsSelection) ||
						   FindReplaceLookInDropdown->Selection->Equals(eFindReplaceSource::AllDocumentsFullText);

	auto Hits = DoOperation(Operation, Query, Replacement, Options, GlobalOperation, Controller);
	if (Operation == textEditor::eFindReplaceOperation::CountMatches)
		Controller->View->ShowNotification("Found " + Hits + " match(es).", nullptr, 4000);
}

FindReplaceHelper::FindReplaceHelper(view::IScriptEditorView^ View)
{
	FindReplaceResultsListView = View->GetComponentByRole(view::eViewRole::FindReplaceResults_ListView)->AsObjectListView();
	GlobalFindReplaceResultsListView = View->GetComponentByRole(view::eViewRole::GlobalFindReplaceResults_TreeView)->AsObjectListView();
	FindReplaceResultsPane = View->GetComponentByRole(view::eViewRole::FindReplaceResults_DockPanel)->AsDockablePane();
	GlobalFindReplaceResultsPane = View->GetComponentByRole(view::eViewRole::GlobalFindReplaceResults_DockPanel)->AsDockablePane();
	FindReplacePane = View->GetComponentByRole(view::eViewRole::FindReplace_DockPanel)->AsDockablePane();
	FindReplaceFindDropdown = View->GetComponentByRole(view::eViewRole::FindReplace_FindDropdown)->AsComboBox();
	FindReplaceReplaceDropdown = View->GetComponentByRole(view::eViewRole::FindReplace_ReplaceDropdown)->AsComboBox();
	FindReplaceLookInDropdown = View->GetComponentByRole(view::eViewRole::FindReplace_LookInDropdown)->AsComboBox();
	FindReplaceOptionsButtonMatchCase = View->GetComponentByRole(view::eViewRole::FindReplace_MatchCase)->AsButton();
	FindReplaceOptionsButtonMatchWholeWord = View->GetComponentByRole(view::eViewRole::FindReplace_MatchWholeWord)->AsButton();
	FindReplaceOptionsButtonUseRegEx = View->GetComponentByRole(view::eViewRole::FindReplace_UseRegEx)->AsButton();
	FindReplaceOptionsButtonIgnoreComments = View->GetComponentByRole(view::eViewRole::FindReplace_IgnoreComments)->AsButton();
	FindReplaceButtonFind = View->GetComponentByRole(view::eViewRole::FindReplace_FindButton)->AsButton();
	FindReplaceButtonReplace = View->GetComponentByRole(view::eViewRole::FindReplace_ReplaceButton)->AsButton();
	FindReplaceButtonCountMatches = View->GetComponentByRole(view::eViewRole::FindReplace_CountMatchesButton)->AsButton();

	CachedGlobalFindReplaceResults = gcnew List<GlobalFindReplaceResult^>;
	PreferencesChangedEventHandler = gcnew EventHandler(this, &FindReplaceHelper::Preferences_Changed);

	preferences::SettingsHolder::Get()->PreferencesChanged += PreferencesChangedEventHandler;
	InitResultsListViews();
	InitFindReplaceLookInDropdown();
	LoadPreferences();
}

FindReplaceHelper::~FindReplaceHelper()
{
	preferences::SettingsHolder::Get()->PreferencesChanged -= PreferencesChangedEventHandler;
	CachedGlobalFindReplaceResults->Clear();
	SavePreferences();

	SAFEDELETE_CLR(PreferencesChangedEventHandler);
	FindReplaceResultsListView = nullptr;
	GlobalFindReplaceResultsListView = nullptr;
	FindReplacePane = nullptr;
	FindReplaceFindDropdown = nullptr;
	FindReplaceReplaceDropdown = nullptr;
	FindReplaceLookInDropdown = nullptr;
	FindReplaceOptionsButtonMatchCase = nullptr;
	FindReplaceOptionsButtonMatchWholeWord = nullptr;
	FindReplaceOptionsButtonUseRegEx = nullptr;
	FindReplaceOptionsButtonIgnoreComments = nullptr;
	FindReplaceButtonFind = nullptr;
	FindReplaceButtonReplace = nullptr;
	FindReplaceButtonCountMatches = nullptr;
}

void FindReplaceHelper::HandleResultsViewEvent(view::ViewComponentEvent^ E, IScriptEditorController^ Controller)
{
	auto EventType = safe_cast<view::components::IObjectListView::eEvent>(E->EventType);
	switch (EventType)
	{
	case view::components::IObjectListView::eEvent::ItemActivate:
	{
		auto Args = safe_cast<view::components::IObjectListView::ItemActivateEventArgs^>(E->EventArgs);

		if (E->Component == FindReplaceResultsListView)
		{
			auto Item = safe_cast<model::components::ScriptFindResult^>(Args->ItemModel);
			if (Item != nullptr)
				Controller->ActiveDocument->TextEditor->ScrollToLine(Item->Line);
		}
		else if (E->Component == GlobalFindReplaceResultsListView)
		{
			if (Args->ItemModel->GetType() == textEditor::FindReplaceResult::HitData::typeid)
			{
				auto Item = safe_cast<textEditor::FindReplaceResult::HitData^>(Args->ItemModel);
				auto ScriptDocumentData = safe_cast<GlobalFindReplaceResult::ScriptDocumentData^>(Args->ParentItemModel);

				if (ScriptDocumentData && ScriptDocumentData->Source)
				{
					Debug::Assert(Controller->Model->ContainsDocument(ScriptDocumentData->Source));
					Controller->ActiveDocument = ScriptDocumentData->Source;
					Controller->ActiveDocument->TextEditor->ScrollToLine(Item->Line);
				}
			}
		}
		else
			gcnew NotImplementedException();

		break;
	}
	default:
		gcnew NotImplementedException();
	}
}

void FindReplaceHelper::HandleFindReplaceEvent(view::ViewComponentEvent^ E, IScriptEditorController^ Controller)
{
	switch (E->Component->Role)
	{
	case view::eViewRole::FindReplace_FindDropdown:
	case view::eViewRole::FindReplace_ReplaceDropdown:
	{
		auto Event = safe_cast<view::components::IComboBox::eEvent>(E->EventType);
		if (Event == view::components::IComboBox::eEvent::KeyDown)
		{
			auto Args = safe_cast<view::components::IComboBox::KeyDownEventArgs^>(E->EventArgs);
			switch (Args->KeyEvent->KeyCode)
			{
			case Keys::Enter:
				if (E->Component == FindReplaceFindDropdown)
					PerformOperation(textEditor::eFindReplaceOperation::Find, Controller);
				else if (E->Component == FindReplaceReplaceDropdown)
					PerformOperation(textEditor::eFindReplaceOperation::Replace, Controller);

				Args->KeyEvent->Handled = true;
				Args->KeyEvent->SuppressKeyPress = true;

				break;
			case Keys::Escape:
				FindReplacePane->Visible = false;

				Args->KeyEvent->Handled = true;
				Args->KeyEvent->SuppressKeyPress = true;

				break;
			}
		}

		break;
	}
	case view::eViewRole::FindReplace_FindButton:
		PerformOperation(textEditor::eFindReplaceOperation::Find, Controller);
		break;
	case view::eViewRole::FindReplace_ReplaceButton:
		PerformOperation(textEditor::eFindReplaceOperation::Replace, Controller);
		break;
	case view::eViewRole::FindReplace_CountMatchesButton:
		PerformOperation(textEditor::eFindReplaceOperation::CountMatches, Controller);
		break;
	case view::eViewRole::FindReplace_LookInDropdown:
	case view::eViewRole::FindReplace_MatchCase:
	case view::eViewRole::FindReplace_MatchWholeWord:
	case view::eViewRole::FindReplace_UseRegEx:
	case view::eViewRole::FindReplace_IgnoreComments:
		;// nothing to do here
	default:
		gcnew ArgumentException("Unexpected find-replace panel component " + E->Component->Role.ToString());
	}
}

void FindReplaceHelper::InvalidateScriptDocumentInGlobalResultsCache(model::IScriptDocument^ ScriptDocument)
{
	for each (auto GlobalResult in CachedGlobalFindReplaceResults)
	{
		for each (auto ScriptDocData in GlobalResult->DocumentsWithHits)
		{
			if (ScriptDocData->Source == ScriptDocument)
				ScriptDocData->Source = nullptr;
		}
	}
}

int FindReplaceHelper::DoOperation(textEditor::eFindReplaceOperation Operation, String^ Query, String^ Replacement, textEditor::eFindReplaceOptions Options, bool InAllOpenDocuments, IScriptEditorController^ Controller)
{
	if (Query->Length == 0)
		return 0;

	if (!InAllOpenDocuments)
	{
		auto Result = Controller->ActiveDocument->FindReplace(Operation, Query, Replacement, Options);
		if (!Result->HasError && Result->TotalHits > 0 && Operation != textEditor::eFindReplaceOperation::CountMatches)
		{
			FindReplaceResultsPane->Visible = true;
			FindReplaceResultsPane->Focus();
		}

		return Result->TotalHits;
	}

	auto GlobalResult = gcnew GlobalFindReplaceResult;
	GlobalResult->Operation = Operation;
	GlobalResult->Query = Query;
	GlobalResult->Replacement = Replacement;
	GlobalResult->Options = Options;

	for each (auto Document in Controller->Model->Documents)
	{
		auto FindResult = Document->FindReplace(Operation, Query, Replacement, Options);
		if (FindResult->HasError)
			break;

		if (FindResult->TotalHits)
			GlobalResult->Add(Document, FindResult);
	}

	if (GlobalResult->TotalHitCount > 0)
	{
		// we want the newest result to show first
		CachedGlobalFindReplaceResults->Insert(0, GlobalResult);
		GlobalFindReplaceResultsListView->SetObjects(CachedGlobalFindReplaceResults, true);
		GlobalFindReplaceResultsPane->Visible = true;
		GlobalFindReplaceResultsPane->Focus();
	}
	else
		Controller->View->ShowNotification("No matches found.", nullptr, 4000);

	return -1;
}

void FindReplaceHelper::ShowFindReplacePane(IScriptEditorController^ Controller)
{
	Debug::Assert(Controller->ActiveDocument != nullptr);

	auto CaretToken = Controller->ActiveDocument->TextEditor->GetTokenAtCaretPos();

	FindReplaceFindDropdown->Text = CaretToken;
	FindReplacePane->Visible = true;
	FindReplacePane->Focus();
	FindReplaceFindDropdown->Focus();
}

InputManager::ChordData::ChordData(utilities::KeyCombo^ SecondChord, utilities::IAction^ Action, ...array<view::eViewRole>^ ActionRoles)
{
	this->SecondChord = SecondChord;
	this->Action = Action;
	this->ActionRoles = gcnew List<view::eViewRole>;

	for each (auto Role in ActionRoles)
		this->ActionRoles->Add(Role);
}

InputManager::ChordData::ChordData(utilities::IAction^ Action, ...array<view::eViewRole>^ ActionRoles)
	: ChordData(nullptr, Action, ActionRoles)
{
}

bool InputManager::ChordData::Equals(Object^ obj)
{
	if (obj == nullptr)
		return false;
	else if (obj->GetType() != ChordData::typeid)
		return false;

	auto Other = safe_cast<ChordData^>(obj);
	return this->SecondChord->Equals(Other->SecondChord) && this->Action->Equals(Other->Action);
}

void InputManager::ChordData::UpdateViewComponentShortcutTexts(utilities::KeyCombo^ Primary, view::IScriptEditorView^ View)
{
	for each (auto Role in ActionRoles)
	{
		auto Component = View->GetComponentByRole(Role)->AsButton();
		Component->ShortcutKey = Primary->ToString() + (SecondChord ?  ", " + SecondChord->ToString() : "");
	}
}

InputManager::ChordData^ InputManager::LookupDoubleKeyChordCommand(utilities::KeyCombo^ First, utilities::KeyCombo^ Second)
{
	ChordDataUnion^ FirstChordMatch = nullptr;
	if (!KeyChordCommands->TryGetValue(First, FirstChordMatch))
		return nullptr;
	else if (FirstChordMatch->Item1)
		return nullptr;

	for each (auto Itr in FirstChordMatch->Item2)
	{
		if (Itr->SecondChord->Equals(Second))
			return Itr;
	}

	return nullptr;
}

InputManager::ChordData^ InputManager::LookupSingleKeyChordCommand(utilities::KeyCombo^ First)
{
	ChordDataUnion^ FirstChordMatch = nullptr;
	if (!KeyChordCommands->TryGetValue(First, FirstChordMatch))
		return nullptr;

	return FirstChordMatch->Item1;
}

bool InputManager::HasSecondKeyOfChord(utilities::KeyCombo^ First)
{
	ChordDataUnion^ FirstChordMatch = nullptr;
	if (!KeyChordCommands->TryGetValue(First, FirstChordMatch))
		return false;

	return FirstChordMatch->Item2 != nullptr;
}

bool InputManager::IsBound(utilities::KeyCombo^ Combo)
{
	return KeyChordCommands->ContainsKey(Combo);
}

void InputManager::FocusActiveDocumentTextEditor(IScriptEditorController^ Controller)
{
	if (Controller->ActiveDocument && !Controller->ActiveDocument->TextEditor->Focused)
		Controller->ActiveDocument->TextEditor->FocusTextArea();
}

InputManager::InputManager(view::IScriptEditorView^ ParentView)
{
	this->ParentView = ParentView;
	this->TextEditorContextMenu = ParentView->GetComponentByRole(view::eViewRole::TextEditor_ContextMenu)->AsContextMenu();

	BlacklistedKeyCodes = gcnew List<Keys>;
	KeyChordCommands = gcnew Dictionary<utilities::KeyCombo^, ChordDataUnion^>;
	LastActiveFirstChord = nullptr;

	BlacklistedKeyCodes->Add(Keys::Escape);
}

InputManager::~InputManager()
{
	KeyChordCommands->Clear();
	TextEditorContextMenu = nullptr;
	ParentView = nullptr;
}

void InputManager::AddKeyChordCommand(utilities::IAction^ Action, utilities::KeyCombo^ Primary, utilities::KeyCombo^ Secondary, bool OverwriteExisting, ...array<view::eViewRole>^ ActionRoles)
{
	ChordDataUnion^ ExistingValue = nullptr;
	if (!KeyChordCommands->TryGetValue(Primary, ExistingValue))
	{
		if (Secondary == nullptr)
			ExistingValue = gcnew ChordDataUnion(gcnew ChordData(Action, ActionRoles), nullptr);
		else
		{
			ExistingValue = gcnew ChordDataUnion(nullptr, gcnew List<ChordData^>);
			ExistingValue->Item2->Add(gcnew ChordData(Secondary, Action, ActionRoles));
		}

		KeyChordCommands->Add(Primary, ExistingValue);
		return;
	}

	if (!OverwriteExisting)
	{
		if (ExistingValue->Item1 && (Secondary || ExistingValue->Item1->Action != Action))
			throw gcnew ArgumentException("Primary key chord '" + Primary->ToString() + "' has already been bound to a command without a secondary key chord");
		else if (ExistingValue->Item2)
		{
			if (Secondary == nullptr)
				throw gcnew ArgumentException("Primary key chord '" + Primary->ToString() + "' has already been bound to at least one secondary key chord");
			else if (ExistingValue->Item2->Contains(gcnew ChordData(Secondary, Action, ActionRoles)))
				throw gcnew ArgumentException("Key chord '" + Primary->ToString() + ", " + Secondary->ToString() + "' has already been bound to a different command");
		}
	}

	if (ExistingValue->Item1)
	{
		if (Secondary == nullptr)
			ExistingValue->Item1->Action = Action;
		else
		{
			auto NewData = gcnew ChordDataUnion(nullptr, gcnew List<ChordData^>);
			KeyChordCommands[Primary] = NewData;
		}
	}
	else
	{
		if (Secondary == nullptr)
			KeyChordCommands[Primary] = gcnew ChordDataUnion(gcnew ChordData(Action), nullptr);
		else
		{
			for each (auto Itr in ExistingValue->Item2)
			{
				if (Itr->SecondChord == Secondary)
				{
					Itr->Action = Action;
					return;
				}
			}

			ExistingValue->Item2->Add(gcnew ChordData(Secondary, Action, ActionRoles));
		}
	}
}

void InputManager::AddKeyChordCommand(utilities::IAction^ Action, utilities::KeyCombo^ Primary, bool OverwriteExisting, ...array<view::eViewRole>^ ActionRoles)
{
	AddKeyChordCommand(Action, Primary, nullptr, OverwriteExisting, ActionRoles);
}

void InputManager::HandleInputFocusChange(view::ViewComponentEvent^ E, IScriptEditorController^ Controller)
{
	switch (E->Component->Role)
	{
	case view::eViewRole::MainWindow:
	{
		auto EventType = safe_cast<view::components::IForm::eEvent>(E->EventType);
		if (EventType == view::components::IForm::eEvent::FocusLeave)
		{
			auto MainToolbar = Controller->View->GetComponentByRole(view::eViewRole::MainToolbar)->AsContainer();
			if (MainToolbar->Focused)
				FocusActiveDocumentTextEditor(Controller);
		}

		break;
	}
	}
}

void InputManager::HandleKeyDown(KeyEventArgs^ E, IScriptEditorController^ Controller)
{
	E->Handled = true;
	TextEditorContextMenu->Hide();

	switch (E->KeyCode)
	{
	case Keys::Escape:
		FocusActiveDocumentTextEditor(Controller);
		break;
	}

	if (LastActiveFirstChord)
	{
		// the first chord was already pressed, attempt to find the second
		switch (E->KeyCode)
		{
		case Keys::Escape:
			// cancel the active chord
			LastActiveFirstChord = nullptr;
			return;
		case Keys::LControlKey:
		case Keys::RControlKey:
		case Keys::LMenu:
		case Keys::RMenu:
		case Keys::LShiftKey:
		case Keys::RShiftKey:
			// ignore modifier key presses
			return;
		}

		auto SecondKeyOfChord = utilities::KeyCombo::FromKeyEvent(E);
		auto MatchedCommand = LookupDoubleKeyChordCommand(LastActiveFirstChord, SecondKeyOfChord);

		if (MatchedCommand)
			MatchedCommand->Action->Invoke();

		LastActiveFirstChord = nullptr;
		return;
	}

	auto FirstKeyOfChord = utilities::KeyCombo::FromKeyEvent(E);

	if (FirstKeyOfChord->Equals(SystemMenuActivateKey) || FirstKeyOfChord->Equals(SystemMenuDeactivateKey))
	{
		auto MainToolbar = Controller->View->GetComponentByRole(view::eViewRole::MainToolbar)->AsContainer();
		if (MainToolbar->Focused)
			FocusActiveDocumentTextEditor(Controller);
		else if (Controller->ActiveDocument == nullptr || Controller->ActiveDocument->TextEditor->Focused)
			MainToolbar->Focus();

		return;
	}
	else if (!IsBound(FirstKeyOfChord))
	{
		E->Handled = false;
		return;
	}

	auto SingleKeyChordCommand = LookupSingleKeyChordCommand(FirstKeyOfChord);
	if (SingleKeyChordCommand)
		SingleKeyChordCommand->Action->Invoke();
	else
		LastActiveFirstChord = FirstKeyOfChord;
}

void InputManager::HandleTextEditorMouseClick(textEditor::TextEditorMouseClickEventArgs^ E, IScriptEditorController^ Controller)
{
	TextEditorContextMenu->Hide();

	switch (E->Button)
	{
	case MouseButtons::Left:
	{

		if (Control::ModifierKeys != Keys::Control)
			break;

		auto TokenAtClickLocation = Controller->ActiveDocument->TextEditor->GetTokenAtCharIndex(E->ScriptTextOffset);
		auto AttachedScript = intellisense::IntelliSenseBackend::Get()->GetAttachedScript(TokenAtClickLocation);
		if (AttachedScript)
			Controller->ActivateOrCreateNewDocument(AttachedScript->GetIdentifier());
		break;
	}
	case MouseButtons::Right:
	{
		TextEditorContextMenu->Show(E->ScreenCoords);
		break;
	}
	}
}

bool InputManager::IsKeyBlacklisted(Keys Key)
{
	return BlacklistedKeyCodes->Contains(Key);
}

void InputManager::RefreshViewComponentShortcutTexts()
{
	for each (auto% Entry in KeyChordCommands)
	{
		if (Entry.Value->Item1)
			Entry.Value->Item1->UpdateViewComponentShortcutTexts(Entry.Key, ParentView);
		else for each (auto Itr in Entry.Value->Item2)
			Itr->UpdateViewComponentShortcutTexts(Entry.Key, ParentView);
	}
}


} // namespace components


} // namespace controllerImpl


} // namespace scriptEditor


} // namespace cse