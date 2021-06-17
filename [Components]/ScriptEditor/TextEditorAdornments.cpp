#include "TextEditorAdornments.h"
#include "Globals.h"
#include "Preferences.h"

using namespace DevComponents;

namespace cse
{
	namespace textEditors
	{
		Image^ ScopeBreadcrumbManager::GetScopeIcon(obScriptParsing::Structurizer::Node::NodeType Type)
		{
			if (DefaultIcon == nullptr)
				DefaultIcon = Globals::ImageResources()->CreateImage("AvalonEditStructureVisualizer");

			return DefaultIcon;
		}

		void ScopeBreadcrumbManager::Parent_LineChanged(Object^ Sender, EventArgs^ E)
		{
			Debug::Assert(Bound == true);

			UpdateToCurrentLine();
		}

		void ScopeBreadcrumbManager::Parent_BackgroundAnalysisComplete(Object^ Sender, scriptEditor::SemanticAnalysisCompleteEventArgs^ E)
		{
			Debug::Assert(Bound == true);

			GenerateCrumbs(E->Result->Clone());
		}

		void ScopeBreadcrumbManager::Parent_TextUpdated(Object^ Sender, EventArgs^ E)
		{
			Debug::Assert(Bound == true);

			ResetCrumbs();
		}

		void ScopeBreadcrumbManager::ScriptEditorPreferences_Saved(Object^ Sender, EventArgs^ E)
		{
			Root->ForeColor = Control::DefaultForeColor;

			if (Bound)
				RefreshCrumbs();
		}

		void ScopeBreadcrumbManager::GenerateCrumbs(obScriptParsing::AnalysisData^ Data)
		{
			ResetCrumbs();

			if (Visible == false)
				return;

			DataStore = gcnew obScriptParsing::Structurizer(Data, gcnew obScriptParsing::Structurizer::GetLineText(BoundParent, &ITextEditor::GetText), BoundParent->CurrentLine);
			if (DataStore->Valid)
			{
				Root->Text = Data->Name;
				Root->Tag = gcnew CrumbData(BoundParent, gcnew obScriptParsing::Structurizer::Node(obScriptParsing::Structurizer::Node::NodeType::Invalid, 1, 1, Data->Name));

				for each (auto Itr in DataStore->Output)
					CreateNewCrumb(Itr, Root, false);

				for each (auto Itr in Root->SubItems)
					EnumerateChildCrumbs((DotNetBar::CrumbBarItem^)Itr);
			}
			else
			{
				DataStore = nullptr;
			}

			UpdateToCurrentLine();
		}

		void ScopeBreadcrumbManager::EnumerateChildCrumbs(DotNetBar::CrumbBarItem^ Item)
		{
			CrumbData^ Data = (CrumbData^)Item->Tag;

			for each (auto Itr in Data->Scope->Children)
				CreateNewCrumb(Itr, Item, true);
		}

		void ScopeBreadcrumbManager::UpdateToCurrentLine()
		{
			if (DataStore == nullptr)
				return;

			int CurrentLine = BoundParent->CurrentLine;
			obScriptParsing::Structurizer::Node^ ContainingNode = DataStore->GetContainingNode(CurrentLine);
			if (ContainingNode)
			{
				for each (auto Itr in ActiveCrumbs)
				{
					if (Itr.Key == ContainingNode)
					{
						Bar->SelectedItem = Itr.Value;
						return;
					}
				}
			}
		}

		void ScopeBreadcrumbManager::CreateNewCrumb(obScriptParsing::Structurizer::Node^ Source, DevComponents::DotNetBar::CrumbBarItem^ Parent, bool EnumerateChildren)
		{
			DotNetBar::CrumbBarItem^ NewItem = gcnew DotNetBar::CrumbBarItem();
			String^ LineText = Source->Description;
			if (LineText->Length > 35)
				LineText = LineText->Substring(0, 35) + "...";

			NewItem->Text = LineText;
			NewItem->Tooltip = Source->Description;
			NewItem->Tag = gcnew CrumbData(BoundParent, Source);
			NewItem->Image = GetScopeIcon(Source->Type);

			NewItem->ForeColor = Control::DefaultForeColor;
			NewItem->HotTrackingStyle = DotNetBar::eHotTrackingStyle::None;

			ActiveCrumbs->Add(Source, NewItem);
			Parent->SubItems->Add(NewItem);

			if (EnumerateChildren)
				EnumerateChildCrumbs(NewItem);
		}

		void ScopeBreadcrumbManager::ResetCrumbs()
		{
			Root->Text = "";
			Root->Tag = nullptr;

			ActiveCrumbs->Clear();
			Root->SubItems->Clear();

			DataStore = nullptr;
			Bar->SelectedItem = Root;
		}

		ScopeBreadcrumbManager::ScopeBreadcrumbManager(DotNetBar::CrumbBar^ Bar) :
			BoundParent(nullptr),
			BackgroundAnalyzer(nullptr),
			Bar(Bar),
			DataStore(nullptr),
			ActiveCrumbs(gcnew Dictionary<obScriptParsing::Structurizer::Node^, DotNetBar::CrumbBarItem^>)
		{
			InstanceCounter++;

			Root = gcnew DotNetBar::CrumbBarItem();
			Root->ForeColor = Control::DefaultForeColor;
			Root->Image = GetScopeIcon(obScriptParsing::Structurizer::Node::NodeType::Invalid);

			ParentLineChangedHandler = gcnew EventHandler(this, &ScopeBreadcrumbManager::Parent_LineChanged);
			ParentBGAnalysisCompleteHandler = gcnew scriptEditor::SemanticAnalysisCompleteEventHandler(this, &ScopeBreadcrumbManager::Parent_BackgroundAnalysisComplete);
			ParentTextUpdatedHandler = gcnew EventHandler(this, &ScopeBreadcrumbManager::Parent_TextUpdated);
			ScriptEditorPreferencesSavedHandler = gcnew EventHandler(this, &ScopeBreadcrumbManager::ScriptEditorPreferences_Saved);

			Bar->Items->Clear();
			Bar->Items->Add(Root);
			Bar->ItemClick += gcnew EventHandler(&ScopeBreadcrumbManager::OnItemClick);

			preferences::SettingsHolder::Get()->SavedToDisk += ScriptEditorPreferencesSavedHandler;
		}

		ScopeBreadcrumbManager::~ScopeBreadcrumbManager()
		{
			Debug::Assert(Bound == false);

			Bar->Items->Clear();
			preferences::SettingsHolder::Get()->SavedToDisk -= ScriptEditorPreferencesSavedHandler;

			SAFEDELETE_CLR(Root);
			SAFEDELETE_CLR(ParentLineChangedHandler);
			SAFEDELETE_CLR(ParentBGAnalysisCompleteHandler);
			SAFEDELETE_CLR(ScriptEditorPreferencesSavedHandler);

			InstanceCounter--;
			Debug::Assert(InstanceCounter >= 0);

			if (InstanceCounter == 0)
			{
				SAFEDELETE_CLR(DefaultIcon);
			}
		}

		void ScopeBreadcrumbManager::Bind(ITextEditor ^ Parent, scriptEditor::IBackgroundSemanticAnalyzer^ BackgroundAnalyzer)
		{
			Debug::Assert(Bound == false);

			BoundParent = Parent;
			this->BackgroundAnalyzer = BackgroundAnalyzer;

			BoundParent->LineChanged += ParentLineChangedHandler;
			BackgroundAnalyzer->SemanticAnalysisComplete += ParentBGAnalysisCompleteHandler;
			BoundParent->TextUpdated += ParentTextUpdatedHandler;

			RefreshCrumbs();
		}

		void ScopeBreadcrumbManager::Unbind()
		{
			if (Bound)
			{
				ResetCrumbs();

				BoundParent->LineChanged -= ParentLineChangedHandler;
				BackgroundAnalyzer->SemanticAnalysisComplete -= ParentBGAnalysisCompleteHandler;
				BoundParent->TextUpdated -= ParentTextUpdatedHandler;

				BoundParent = nullptr;
				BackgroundAnalyzer = nullptr;
			}
		}

		void ScopeBreadcrumbManager::RefreshCrumbs()
		{
			Debug::Assert(Bound == true);

			if (Visible == false)
				return;

			obScriptParsing::AnalysisData^ ParsedData = BackgroundAnalyzer->LastAnalysisResult;
			if (ParsedData == nullptr)
				return;

			GenerateCrumbs(ParsedData);
		}

		void ScopeBreadcrumbManager::OnItemClick(Object^ Sender, EventArgs^ E)
		{
			DotNetBar::CrumbBarItem^ Selection = nullptr;
			if (Sender->GetType() == DotNetBar::CrumbBarItemView::typeid)
			{
				DotNetBar::CrumbBarItemView^ Current = (DotNetBar::CrumbBarItemView^)Sender;
				Selection = Current->AttachedItem;
			}
			else if (Sender->GetType() == DotNetBar::CrumbBarItem::typeid)
				Selection = (DotNetBar::CrumbBarItem^)Sender;

			if (Selection && Selection->Tag)
			{
				CrumbData^ Data = (CrumbData^)Selection->Tag;
				Data->Jump();
			}
		}

		ScopeBreadcrumbManager::CrumbData::CrumbData(ITextEditor ^ Parent, obScriptParsing::Structurizer::Node ^ Scope) :
			Parent(Parent),
			Scope(Scope)
		{
			;//
		}

		void ScopeBreadcrumbManager::CrumbData::Jump()
		{
			Parent->ScrollToLine(Scope->StartLine);
		}

	}
}