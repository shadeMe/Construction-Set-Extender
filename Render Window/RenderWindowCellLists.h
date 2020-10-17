#pragma once

#include "RenderWindowOSD.h"

namespace cse
{
	namespace renderWindow
	{
		class RenderWindowCellLists
		{
			class CosaveHandler : public serialization::PluginCosaveManager::IEventHandler
			{
				RenderWindowCellLists*		Parent;
			public:
				CosaveHandler(RenderWindowCellLists* Parent);

				virtual void					HandleLoad(const char* PluginName, const char* CosaveDirectory);
				virtual void					HandleSave(const char* PluginName, const char* CosaveDirectory);
				virtual void					HandleShutdown(const char* PluginName, const char* CosaveDirectory);
			};

			class GlobalEventSink : public SME::MiscGunk::IEventSink
			{
				RenderWindowCellLists*			Parent;
			public:
				GlobalEventSink(RenderWindowCellLists* Parent);

				virtual void					Handle(SME::MiscGunk::IEventData* Data);
			};

			friend class CosaveHandler;
			friend class OSDLayer;
			friend class GlobalEventSink;

			static const UInt32						kVisitedListCapacity;
			static const char*						kSaveFileName;

			typedef std::list<TESObjectCELL*>		CellListT;
			typedef std::vector<TESObjectCELL*>		CellArrayT;

			enum
			{
				kList_Bookmark = 0,
				kList_Recents
			};

			struct CellListDialogResult
			{
				bool				AddBookmark;
				bool				RemoveBookmark;
				bool				SelectCell;
				TESObjectCELL*		Selection;

				CellListDialogResult();
			};

			CellArrayT				Bookmarks;
			CellListT				RecentlyVisited;
			GlobalEventSink*		EventSink;
			CosaveHandler*			CosaveInterface;
			ImGuiTextFilter			FilterHelper;
			bool					BookmarksVisible;
			bool					RecentsVisible;
			bool					Initialized;

			void					RenderPopupButton();
			void					RenderWindowContents();

			void					OnSelectCell(TESObjectCELL* Cell) const;
			void					AddCellToList(TESObjectCELL* Cell, UInt8 List, CellListDialogResult& Out) const;	// returns true if the context menu item was selected

			void					SaveBookmarks(const char* PluginName, const char* DirPath) const;
			void					LoadBookmarks(const char* PluginName, const char* DirPath);

			void					AddBookmark(TESObjectCELL* Cell);
			void					RemoveBookmark(TESObjectCELL* Cell);
			void					VisitCell(TESObjectCELL* Cell);
			void					ClearCells();
		public:
			RenderWindowCellLists();
			~RenderWindowCellLists();

			void					Initialize();
			void					Deinitialize();
		};
	}
}