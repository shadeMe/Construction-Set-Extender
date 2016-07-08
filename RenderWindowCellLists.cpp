#include "RenderWindowCellLists.h"

namespace cse
{
	namespace renderWindow
	{



		RenderWindowCellLists::CosaveHandler::CosaveHandler(RenderWindowCellLists* Parent) :
			serialization::PluginCosaveManager::IEventHandler(),
			Parent(Parent)
		{
			SME_ASSERT(Parent);
		}

		void RenderWindowCellLists::CosaveHandler::HandleLoad(const char* PluginName, const char* CosaveDirectory)
		{
			Parent->LoadBookmarks(PluginName, CosaveDirectory);
		}

		void RenderWindowCellLists::CosaveHandler::HandleSave(const char* PluginName, const char* CosaveDirectory)
		{
			Parent->SaveBookmarks(PluginName, CosaveDirectory);
		}

		void RenderWindowCellLists::CosaveHandler::HandleShutdown(const char* PluginName, const char* CosaveDirectory)
		{
			;//
		}

		RenderWindowCellLists::OSDLayer::CellListDialogResult::CellListDialogResult()
		{
			AddBookmark = false;
			RemoveBookmark = false;
			SelectCell = false;
			Selection = NULL;
		}

		void RenderWindowCellLists::OSDLayer::OnSelectCell(TESObjectCELL* Cell) const
		{
			// emulating a mouse click in the cell view dialog to ensure that the right code path is selected
			// which includes our hooks
			TESWorldSpace* ParentWorldspace = Cell->GetParentWorldSpace();
			TESObjectCELL* Buffer = _TES->currentInteriorCell;
			if (ParentWorldspace == NULL)
				_TES->currentInteriorCell = Cell;
			else
			{
				_TES->SetCurrentWorldspace(ParentWorldspace);
				_TES->currentInteriorCell = NULL;
			}

			TESCellViewWindow::UpdateCurrentWorldspace();
			TESCellViewWindow::RefreshCellList(false);
			TESCellViewWindow::SetCellSelection(Cell);
			_TES->currentInteriorCell = Buffer;

			// the cell is selected in the list at this point, so we emulate a double click
			NMITEMACTIVATE Data = { 0 };
			Data.hdr.code = -3;		// ### what notification is this?
			Data.hdr.hwndFrom = *TESCellViewWindow::CellListHandle;
			Data.hdr.idFrom = TESCellViewWindow::kCellListView;
			Data.iItem = TESListView::GetItemByData(*TESCellViewWindow::CellListHandle, Cell);
			SME_ASSERT(Data.iItem != -1);
			Data.lParam = (LPARAM)Cell;
			SendMessage(*TESCellViewWindow::WindowHandle, WM_NOTIFY, Data.hdr.idFrom, (LPARAM)&Data);

			SetActiveWindow(*TESRenderWindow::WindowHandle);
		}

		void RenderWindowCellLists::OSDLayer::AddCellToList(TESObjectCELL* Cell, UInt8 List, CellListDialogResult& Out) const
		{
			const char* EditorID = Cell->GetEditorID();
			if (EditorID == NULL)
				EditorID = "";
			const char* Name = Cell->name.c_str();
			if (Name == NULL)
				Name = "";

			char Location[0x100] = { 0 };
			Out.Selection = Cell;

			if (Cell->GetIsInterior())
				FORMAT_STR(Location, "Interior");
			else
				FORMAT_STR(Location, "%s (%d,%d)", Cell->GetParentWorldSpace()->name.c_str(), Cell->cellData.coords->x, Cell->cellData.coords->y);

			char Buffer[0x100] = {0};
			FORMAT_STR(Buffer, "%s %s %s", EditorID, Name, Location);
			if (FilterHelper.PassFilter(Buffer) == false)
				return;

			char Label[0x10] = {0};
			FORMAT_STR(Label, "%08X-%d", Cell->formID, (UInt32)List);

			ImGui::PushID(Label);
			if (ImGui::Selectable(EditorID, false, ImGuiSelectableFlags_SpanAllColumns))
				Out.SelectCell = true;
			ImGui::PopID();

			if (List == kList_Bookmark && ImGui::BeginPopupContextItem("BookmarkPopup"))
			{
				if (ImGui::Selectable("Remove Bookmark"))
					Out.RemoveBookmark = true;
				ImGui::EndPopup();
			}
			else if (List == kList_Recents && ImGui::BeginPopupContextItem("RecentsPopup"))
			{
				if (ImGui::Selectable("Add Bookmark"))
					Out.AddBookmark = true;
				ImGui::EndPopup();
			}

			ImGui::NextColumn();
			ImGui::Selectable(Name);
			ImGui::NextColumn();
			ImGui::Selectable(Location);
			ImGui::NextColumn();
		}

		RenderWindowCellLists::OSDLayer::OSDLayer(RenderWindowCellLists* Parent) :
			IRenderWindowOSDLayer(IRenderWindowOSDLayer::kPriority_CellLists),
			Parent(Parent),
			FilterHelper()
		{
			SME_ASSERT(Parent);
		}

		void RenderWindowCellLists::OSDLayer::Draw(RenderWindowOSD* OSD, ImGuiDX9* GUI)
		{
			ImGui::SetNextWindowPos(ImVec2(10, 300), ImGuiSetCond_FirstUseEver);
			if (!ImGui::Begin("Cell Lists", NULL, ImGuiWindowFlags_NoFocusOnAppearing))
			{
				ImGui::End();
				return;
			}

			FilterHelper.Draw();

			TESObjectCELL* ToSelect = NULL;
			if (ImGui::CollapsingHeader("Bookmarks"))
			{
				if (_TES->GetCurrentCell() && ImGui::Button("Bookmark Current Cell"))
					Parent->AddBookmark(_TES->GetCurrentCell());

				if (Parent->Bookmarks.size())
				{
					ImGui::Columns(3, "BookmarkList");
					ImGui::Separator();
					ImGui::Text("EditorID"); ImGui::NextColumn();
					ImGui::Text("Name"); ImGui::NextColumn();
					ImGui::Text("Location"); ImGui::NextColumn();
					ImGui::Separator();

					TESObjectCELL* ToRemove = NULL;
					for each (auto Itr in Parent->Bookmarks)
					{
						CellListDialogResult Out;
						AddCellToList(Itr, kList_Bookmark, Out);
						if (Out.RemoveBookmark)
						{
							ToRemove = Out.Selection;
							break;
						}
						else if (Out.SelectCell)
						{
							ToSelect = Out.Selection;
							break;
						}
					}

					if (ToRemove)
						Parent->RemoveBookmark(ToRemove);

					if (ToSelect)
					{
						ImGui::End();
						OnSelectCell(ToSelect);
						return;
					}

					ImGui::Columns(1);
					ImGui::Separator();
				}
			}

			if (Parent->RecentlyVisited.size() && ImGui::CollapsingHeader("Recently Visited"))
			{
				ImGui::Columns(3, "RecentsList");
				ImGui::Separator();
				ImGui::Text("EditorID"); ImGui::NextColumn();
				ImGui::Text("Name"); ImGui::NextColumn();
				ImGui::Text("Location"); ImGui::NextColumn();
				ImGui::Separator();

				TESObjectCELL* ToAdd = NULL;
				for each (auto Itr in Parent->RecentlyVisited)
				{
					CellListDialogResult Out;
					AddCellToList(Itr, kList_Recents, Out);
					if (Out.RemoveBookmark)
					{
						ToAdd = Out.Selection;
						break;
					}
					else if (Out.SelectCell)
					{
						ToSelect = Out.Selection;
						break;
					}
				}

				if (ToAdd)
					Parent->AddBookmark(ToAdd);

				if (ToSelect)
				{
					ImGui::End();
					OnSelectCell(ToSelect);
					return;
				}

				ImGui::Columns(1);
				ImGui::Separator();
			}

			ImGui::End();
		}

		bool RenderWindowCellLists::OSDLayer::NeedsBackgroundUpdate()
		{
			return false;
		}

		RenderWindowCellLists::GlobalEventSink::GlobalEventSink(RenderWindowCellLists* Parent) :
			SME::MiscGunk::IEventSink(),
			Parent(Parent)
		{
			SME_ASSERT(Parent);
		}

		void RenderWindowCellLists::GlobalEventSink::Handle(SME::MiscGunk::IEventData* Data)
		{
			const events::TypedEventSource* Source = dynamic_cast<const events::TypedEventSource*>(Data->Source);
			SME_ASSERT(Source);

			switch (Source->GetTypeID())
			{
			case events::TypedEventSource::kType_CellView_SelectCell:
				{
					events::dialog::cellView::CellViewDialogEventData* Args = dynamic_cast<events::dialog::cellView::CellViewDialogEventData*>(Data);
					SME_ASSERT(Args);
					Parent->VisitCell(Args->Cell);
				}

				break;
			case events::TypedEventSource::kType_Plugin_ClearData:
				Parent->ClearCells();
				break;
			}
		}

		const UInt32				RenderWindowCellLists::kVisitedListCapacity = 30;
		const char*					RenderWindowCellLists::kSaveFileName = "RenderWindowCellBookmarks";

		void RenderWindowCellLists::SaveBookmarks(const char* PluginName, const char* DirPath) const
		{
			if (Bookmarks.size() == 0)
				return;

			std::string FilePath(DirPath); FilePath += "\\" + std::string(RenderWindowCellLists::kSaveFileName);
			try
			{
				if (bfs::exists(FilePath))
					bfs::remove(FilePath);

				serialization::TESForm2Text Serializer;
				std::string OutString;
				for each (auto Itr in Bookmarks)
				{
					std::string Temp;
					if (Serializer.Serialize(Itr, Temp))
						OutString += Temp + "\n";
				}

				std::fstream Stream(FilePath.c_str(), std::ios::out);
				Stream << OutString;
				Stream.close();
			}
			catch (std::exception& e)
			{
				BGSEECONSOLE_MESSAGE("Couldn't save render window cell bookmarks. Error - %s", e.what());
			}
		}

		void RenderWindowCellLists::LoadBookmarks(const char* PluginName, const char* DirPath)
		{
			std::string FilePath(DirPath); FilePath += "\\" + std::string(RenderWindowCellLists::kSaveFileName);
			try
			{
				Bookmarks.clear();
				if (bfs::exists(FilePath) == false)
					return;

				serialization::TESForm2Text Serializer;
				std::string Line;
				std::fstream Stream(FilePath.c_str(), std::ios::in);

				while (std::getline(Stream, Line))
				{
					if (Line.length() < 2)
						return;

					TESForm* Cell = NULL;
					if (Serializer.Deserialize(Line, &Cell))
					{
						SME_ASSERT(Cell->formType == TESForm::kFormType_Cell);
						Bookmarks.push_back(CS_CAST(Cell, TESForm, TESObjectCELL));
					}
					else
						throw std::exception("Invalid bookmark entry");
				}

				Stream.close();
			}
			catch (std::exception& e)
			{
				BGSEECONSOLE_MESSAGE("Couldn't load render window cell bookmarks. Error - %s", e.what());
			}
		}

		void RenderWindowCellLists::AddBookmark(TESObjectCELL* Cell)
		{
			SME_ASSERT(Cell);

			if (std::find(Bookmarks.begin(), Bookmarks.end(), Cell) == Bookmarks.end())
				Bookmarks.push_back(Cell);
		}

		void RenderWindowCellLists::RemoveBookmark(TESObjectCELL* Cell)
		{
			SME_ASSERT(Cell);

			CellArrayT::iterator Match = std::find(Bookmarks.begin(), Bookmarks.end(), Cell);

			if (Match != Bookmarks.end())
				Bookmarks.erase(Match);
		}

		void RenderWindowCellLists::VisitCell(TESObjectCELL* Cell)
		{
			SME_ASSERT(Cell);

			CellListT::iterator Match = std::find(RecentlyVisited.begin(), RecentlyVisited.end(), Cell);
			if (Match == RecentlyVisited.end())
			{
				if (RecentlyVisited.size() == kVisitedListCapacity)
					RecentlyVisited.pop_back();
			}
			else
				RecentlyVisited.erase(Match);

			RecentlyVisited.push_front(Cell);
		}

		void RenderWindowCellLists::ClearCells()
		{
			Bookmarks.clear();
			RecentlyVisited.clear();
		}

		RenderWindowCellLists::RenderWindowCellLists() :
			Bookmarks(),
			RecentlyVisited()
		{
			EventSink = new GlobalEventSink(this);
			CosaveInterface = new CosaveHandler(this);
			OSDRenderer = new OSDLayer(this);

			Bookmarks.reserve(50);
			Initialized = false;
		}

		RenderWindowCellLists::~RenderWindowCellLists()
		{
			DEBUG_ASSERT(Initialized == false);
			ClearCells();

			SAFEDELETE(EventSink);
			SAFEDELETE(CosaveInterface);
			SAFEDELETE(OSDRenderer);
		}

		void RenderWindowCellLists::Initialize(RenderWindowOSD* OSD)
		{
			SME_ASSERT(Initialized == false);

			OSD->AttachLayer(OSDRenderer);

			events::dialog::cellView::kSelectCell.AddSink(EventSink);
			events::plugin::kClearData.AddSink(EventSink);

			_COSAVE.Register(CosaveInterface);
			Initialized = true;
		}


		void RenderWindowCellLists::Deinitialize(RenderWindowOSD* OSD)
		{
			SME_ASSERT(Initialized);

			OSD->DetachLayer(OSDRenderer);

			events::dialog::cellView::kSelectCell.RemoveSink(EventSink);
			events::plugin::kClearData.RemoveSink(EventSink);

			_COSAVE.Unregister(CosaveInterface);
			Initialized = false;
		}

	}
}