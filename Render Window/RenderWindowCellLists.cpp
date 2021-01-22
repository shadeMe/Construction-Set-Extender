#include "RenderWindowCellLists.h"
#include "IconFontCppHeaders\IconsMaterialDesign.h"
#include "ToolbarOSDLayer.h"
#include "RenderWindowManager.h"

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


		RenderWindowCellLists::CellListDialogResult::CellListDialogResult()
		{
			AddBookmark = false;
			RemoveBookmark = false;
			SelectCell = false;
			Selection = nullptr;
		}

		void RenderWindowCellLists::OnSelectCell(TESObjectCELL* Cell) const
		{
			RenderWindowDeferredExecutor::DelegateT Task = [Cell]() {
				// emulating a mouse click in the cell view dialog to ensure that the right code path is selected
				// which includes our hooks
				TESWorldSpace* ParentWorldspace = Cell->GetParentWorldSpace();
				TESObjectCELL* Buffer = _TES->currentInteriorCell;
				if (ParentWorldspace == nullptr)
					_TES->currentInteriorCell = Cell;
				else
				{
					_TES->SetCurrentWorldspace(ParentWorldspace);
					_TES->currentInteriorCell = nullptr;
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
			};

			_RENDERWIN_MGR.GetDeferredExecutor()->QueueTask(Task);
		}

		void RenderWindowCellLists::AddCellToList(TESObjectCELL* Cell, UInt8 List, CellListDialogResult& Out) const
		{
			const char* EditorID = Cell->GetEditorID();
			if (EditorID == nullptr)
				EditorID = "";
			const char* Name = Cell->name.c_str();
			if (Name == nullptr)
				Name = "";

			char Location[0x100] = { 0 };
			Out.Selection = Cell;

			if (Cell->IsInterior())
				FORMAT_STR(Location, "Interior");
			else
			{
				const char* WorldspaceName = Cell->GetParentWorldSpace()->name.c_str();
				if (WorldspaceName == nullptr)
					WorldspaceName = Cell->GetParentWorldSpace()->GetEditorID();

				FORMAT_STR(Location, "%s (%d,%d)", WorldspaceName, Cell->cellData.coords->x, Cell->cellData.coords->y);
			}

			char Buffer[0x100] = { 0 };
			FORMAT_STR(Buffer, "%s %s %s", EditorID, Name, Location);
			if (FilterHelper.PassFilter(Buffer) == false)
				return;

			char Label[0x10] = { 0 };
			FORMAT_STR(Label, "%08X-%d", Cell->formID, (UInt32)List);

			ImGui::PushID(Label);
			{
				ImGui::TableNextRow(0, 20.f);
				{
					ImGui::TableNextColumn();
					{
						if (ImGui::Selectable(EditorID, false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick) &&
							ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
						{
							Out.SelectCell = true;
						}

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
					}
					ImGui::TableNextColumn();
					{
						ImGui::Selectable(Name);
					}
					ImGui::TableNextColumn();
					{
						ImGui::Selectable(Location);
					}
				}
			}
			ImGui::PopID();
		}

		void RenderWindowCellLists::RenderPopupButton()
		{
			const ImVec4 MainColor(0, 0, 0, 0);

			ImGui::PushStyleColor(ImGuiCol_Button, MainColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, MainColor);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, MainColor);

			ImGui::Button(ICON_MD_NATURE_PEOPLE "##popupbtn_cell_lists", ImVec2(0, 0));
			if (ImGui::IsItemHoveredRect())
				ImGui::SetTooltip("Recent Cells & Bookmarks");

			ImGui::PopStyleColor(3);
		}

		void RenderWindowCellLists::RenderWindowContents()
		{
			FilterHelper.Draw();

			ImGui::BeginChild("contents_child_frame", ImVec2(0, 500));
			{
				TESObjectCELL* ToSelect = nullptr;

				if (ImGui::CollapsingHeader("Bookmarks", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap))
				{
					BookmarksVisible = true;
					ImGui::SameLine(0, 85);
					if (ImGui::Button(ICON_MD_BOOKMARK " Bookmark Current Cell##bookmark_current_cell_btn") && _TES->GetCurrentCell())
						AddBookmark(_TES->GetCurrentCell());

					if (ImGui::BeginTable("##bookmark_table", 3,
						ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
						ImVec2(0, RecentsVisible ? -250 : -40)))
					{
						ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthFixed, 100);
						ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150);
						ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthFixed, 100);

						ImGui::TableSetupScrollFreeze(0, 1);
						ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 2));
						ImGui::TableHeadersRow();
						ImGui::PopStyleVar();

						TESObjectCELL* ToRemove = nullptr;
						for (auto Itr : Bookmarks)
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
							RemoveBookmark(ToRemove);

						if (ToSelect)
							OnSelectCell(ToSelect);

						ImGui::EndTable();
					}
				}
				else
					BookmarksVisible = false;

				if (ImGui::CollapsingHeader("Recently Visited", ImGuiTreeNodeFlags_DefaultOpen))
				{
					RecentsVisible = true;

					if (ImGui::BeginTable("##recents_table", 3,
						ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
						ImVec2(0, 0)))
					{
						ImGui::TableSetupColumn("Editor ID", ImGuiTableColumnFlags_WidthFixed, 100);
						ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150);
						ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthFixed, 100);

						ImGui::TableSetupScrollFreeze(0, 1);
						ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(0, 2));
						ImGui::TableHeadersRow();
						ImGui::PopStyleVar();

						TESObjectCELL* ToAdd = nullptr;
						for (auto Itr : RecentlyVisited)
						{
							CellListDialogResult Out;
							AddCellToList(Itr, kList_Recents, Out);
							if (Out.AddBookmark)
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
							AddBookmark(ToAdd);

						if (ToSelect)
							OnSelectCell(ToSelect);

						ImGui::EndTable();
					}
				}
				else
					RecentsVisible = false;
			}
			ImGui::EndChild();
		}

		void RenderWindowCellLists::SaveBookmarks(const char* PluginName, const char* DirPath) const
		{
			if (Bookmarks.size() == 0)
				return;

			std::string FilePath(DirPath); FilePath += "\\" + std::string(RenderWindowCellLists::kSaveFileName);
			try
			{
				if (std::filesystem::exists(FilePath))
					std::filesystem::remove(FilePath);

				serialization::TESForm2Text Serializer;
				std::string OutString;
				for (auto Itr : Bookmarks)
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
				if (std::filesystem::exists(FilePath) == false)
					return;

				serialization::TESForm2Text Serializer;
				std::string Line;
				std::fstream Stream(FilePath.c_str(), std::ios::in);

				while (std::getline(Stream, Line))
				{
					if (Line.length() < 2)
						continue;

					TESForm* Cell = nullptr;
					if (Serializer.Deserialize(Line, &Cell))
					{
						SME_ASSERT(Cell->formType == TESForm::kFormType_Cell);
						Bookmarks.push_back(CS_CAST(Cell, TESForm, TESObjectCELL));
					}
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
			RecentlyVisited(),
			FilterHelper()
		{
			EventSink = new GlobalEventSink(this);
			CosaveInterface = new CosaveHandler(this);

			Bookmarks.reserve(50);
			BookmarksVisible = RecentsVisible = true;
			Initialized = false;
		}

		RenderWindowCellLists::~RenderWindowCellLists()
		{
			DEBUG_ASSERT(Initialized == false);
			ClearCells();

			SAFEDELETE(EventSink);
			SAFEDELETE(CosaveInterface);
		}

		void RenderWindowCellLists::Initialize()
		{
			SME_ASSERT(Initialized == false);

			events::dialog::cellView::kSelectCell.AddSink(EventSink);
			events::plugin::kClearData.AddSink(EventSink);

			_COSAVE.Register(CosaveInterface);
			ToolbarOSDLayer::Instance.RegisterTopToolbarButton("celllists_interface",
															   std::bind(&RenderWindowCellLists::RenderPopupButton, this),
															   std::bind(&RenderWindowCellLists::RenderWindowContents, this));
			Initialized = true;
		}


		void RenderWindowCellLists::Deinitialize()
		{
			SME_ASSERT(Initialized);

			events::dialog::cellView::kSelectCell.RemoveSink(EventSink);
			events::plugin::kClearData.RemoveSink(EventSink);

			_COSAVE.Unregister(CosaveInterface);
			Initialized = false;
		}

	}
}