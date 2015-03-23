#pragma once

#include "NativeWrapper.h"
#include "MiscUtilities.h"

namespace ConstructionSetExtender
{
	ListViewItem^											GetListViewSelectedItem(ListView^ Source);
	int														GetListViewSelectedItemIndex(ListView^ Source);
	ListViewItem^											FindItemWithText(ListView^ Source, String^ Substring, bool SearchInSubItems, bool CaseInsensitive);

	ref class ListViewGenericSorter abstract
	{
	protected:
		int													_Column;
		SortOrder											_Order;
	public:
		ListViewGenericSorter() : _Column(0), _Order(SortOrder::Ascending) {}
		ListViewGenericSorter(int Index, SortOrder Order) : _Column(Index), _Order(Order) {}
	};

	ref class ListViewStringSorter : public ListViewGenericSorter, public System::Collections::IComparer
	{
	public:
		ListViewStringSorter() : ListViewGenericSorter() {}
		ListViewStringSorter(int Index, SortOrder Order) : ListViewGenericSorter(Index, Order) {}

		virtual int											Compare(Object^ X, Object^ Y);
	};

	ref class ListViewIntSorter : public ListViewGenericSorter, public System::Collections::IComparer
	{
		bool												Hex;
	public:
		ListViewIntSorter() : ListViewGenericSorter(), Hex(false) {}
		ListViewIntSorter(int Index, SortOrder Order, bool Hex) : ListViewGenericSorter(Index, Order), Hex(Hex) {}

		virtual int											Compare(Object^ X, Object^ Y);
	};

	ref class ListViewImgSorter : public ListViewGenericSorter, public System::Collections::IComparer
	{
	public:
		ListViewImgSorter() : ListViewGenericSorter() {}
		ListViewImgSorter(int Index, SortOrder Order) : ListViewGenericSorter(Index, Order) {}

		virtual int											Compare(Object^ X, Object^ Y);
	};

	ref class DoubleBufferedListView : public ListView
	{
		bool		PaintInProgress;
	protected:
		virtual property bool DoubleBuffered
		{
			bool get() override { return true; }
		}

		[StructLayout(System::Runtime::InteropServices::LayoutKind::Sequential)]
		value struct NMHDR
		{
			IntPtr hwndFrom;
			IntPtr idFrom;
			int code;
		};

		void WndProc(Message% m) override
		{
			switch (m.Msg)
			{
			case 0x0F: // WM_PAINT
		//		if (NativeWrapper::GetForegroundWindow() == m.HWnd ||
		//			NativeWrapper::GetActiveWindow() == m.HWnd)
				{
					PaintInProgress = true;
					ListView::WndProc(m);
					PaintInProgress = false;
				}

				break;
			case 0x204E: // WM_REFLECT_NOTIFY
				{
					NMHDR nmhdr = (NMHDR)m.GetLParam(NMHDR::typeid);

					if (nmhdr.code == -12)
					{ // NM_CUSTOMDRAW
						if (PaintInProgress)
							ListView::WndProc(m);
					}
					else
						ListView::WndProc(m);
				}

				break;
			default:
				ListView::WndProc(m);
				break;
			}
		}
	};

	generic<typename T>
	ref class SimpleListViewBinder
	{
	protected:
		SimpleBindingList<T>^	Source;
		ListView^				Sink;
		int						LastSortColumn;
		SortOrder				LastSortOrder;

		ListViewItem^ Find(T Data)
		{
			for each (ListViewItem^ Itr in Sink->Items)
			{
				if (Itr->Tag == Data)
					return Itr;
			}

			return nullptr;
		}

		virtual ListViewItem^ Create(T Data)
		{
			ListViewItem^ New = gcnew ListViewItem;
			int ImageIndex = GetImageIndex(Data);

			if (ImageIndex != -1)
				New->ImageIndex = ImageIndex;

			for (int i = 1; i < GetColumnCount(); i++)
				New->SubItems->Add("");

			New->Tag = Data;
			return New;
		}

		void PopulateFromSource(bool Sort)
		{
			for each (T Itr in Source)
			{
				ListViewItem^ New = Create(Itr);
				if (New)
					Sink->Items->Add(New);
			}

			if (Sort)
				SortSink(GetDefaultSortColumn(), GetDefaultSortOrder());
		}

		void SortSink(int Column, SortOrder Order)
		{
			LastSortColumn = Column;
			LastSortOrder = Order;

			System::Collections::IComparer^ Sorter = GetSorter(LastSortColumn, LastSortOrder);
			Sink->ListViewItemSorter = Sorter;
			Sink->Sorting = LastSortOrder;
			Sink->Sort();
		}

		void SourceHandlerAdded(Object^ Sender, SimpleBindingList<T>::AddRemoveEventArgs^ E)
		{
			if (E->UpdateInProgress == false)
			{
				ListViewItem^ New = Create(E->Item);
				if (New)
					Sink->Items->Add(New);
			}
		}

		void SourceHandlerRemoved(Object^ Sender, SimpleBindingList<T>::AddRemoveEventArgs^ E)
		{
			if (E->UpdateInProgress == false)
			{
				ListViewItem^ Old = Find(E->Item);
				if (Old)
					Sink->Items->Remove(Old);
			}
		}

		void SourceHandlerCleared(Object^ Sender, SimpleBindingList<T>::ClearEventArgs^ E)
		{
			if (E->UpdateInProgress == false)
				Sink->Items->Clear();
		}

		void SourceHandlerUpdateStarted(Object^ Sender, SimpleBindingList<T>::UpdateEventArgs^ E)
		{
			Sink->BeginUpdate();
			Sink->Items->Clear();
		}

		void SourceHandlerUpdateStopped(Object^ Sender, SimpleBindingList<T>::UpdateEventArgs^ E)
		{
			PopulateFromSource(false);
			Sink->EndUpdate();
			SortSink(LastSortColumn, LastSortOrder);
		}

		void SourceHandlerSorted(Object^ Sender, SimpleBindingList<T>::SortEventArgs^ E)
		{
			// we don't care about how the objects were sorted in the binding list
			SortSink(GetDefaultSortColumn(), GetDefaultSortOrder());
		}

		void SinkHandlerDrawItem(Object^ Sender, DrawListViewItemEventArgs^ E)
		{
			;//
		}

		void SinkHandlerDrawSubItem(Object^ Sender, DrawListViewSubItemEventArgs^ E)
		{
			T Data = (T)E->Item->Tag;

			if (E->ItemState.HasFlag(ListViewItemStates::Selected) == false)
				E->DrawBackground();

			int ImageIndex = GetImageIndex(Data);
			if (ImageIndex != -1 && E->ColumnIndex == 0)
			{
				E->Graphics->SmoothingMode = Drawing2D::SmoothingMode::AntiAlias;
				E->Graphics->DrawImage(E->Item->ImageList->Images[ImageIndex], E->SubItem->Bounds.Location);
			}

			SolidBrush^ TextBrush = gcnew SolidBrush(E->SubItem->ForeColor);
			StringFormat^ Format = gcnew StringFormat;
			RectangleF Layout(E->Bounds.X + E->Bounds.Height, E->Bounds.Y, E->Bounds.Width - E->Bounds.Height, E->Bounds.Height);

			E->Graphics->DrawString(GetSubItemText(Data, E->ColumnIndex), E->SubItem->Font, TextBrush, Layout, Format);

			delete Format;
		}

		void SinkHandlerDrawColumnHeader(Object^ Sender, DrawListViewColumnHeaderEventArgs^ E)
		{
			E->DrawDefault = true;
		}

		void SinkHandlerColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
		{
			if (E->Column != LastSortColumn)
			{
				LastSortColumn = E->Column;
				Sink->Sorting = SortOrder::Ascending;
			}
			else
			{
				if (Sink->Sorting == SortOrder::Ascending)
					Sink->Sorting = SortOrder::Descending;
				else
					Sink->Sorting = SortOrder::Ascending;
			}

			SortSink(E->Column, Sink->Sorting);
		}

		void SinkHandlerItemActivate(Object^ Sender, EventArgs^ E)
		{
			ListViewItem^ Selection = GetListViewSelectedItem(Sink);
			if (Selection)
				ActivateItem((T)Selection->Tag);
		}

		void SinkHandlerKeyUp(Object^ Sender, KeyEventArgs^ E)
		{
			KeyPress(E);
		}

		SimpleBindingList<T>::AddRemoveEventHandler^		SourceAdd;
		SimpleBindingList<T>::AddRemoveEventHandler^		SourceRemove;
		SimpleBindingList<T>::ClearEventHandler^			SourceClear;
		SimpleBindingList<T>::UpdateEventHandler^			SourceBeginUpdate;
		SimpleBindingList<T>::UpdateEventHandler^			SourceEndUpdate;
		SimpleBindingList<T>::SortEventHandler^				SourceSort;

		DrawListViewItemEventHandler^						SinkDrawItem;
		DrawListViewSubItemEventHandler^					SinkDrawSubItem;
		DrawListViewColumnHeaderEventHandler^				SinkDrawColumnHeader;
		ColumnClickEventHandler^							SinkColumnClick;
		EventHandler^										SinkItemActivate;
		KeyEventHandler^									SinkKeyUp;

		virtual void										InitializeListView(ListView^ Control) abstract;	// create headers, image lists, etc
		virtual System::Collections::IComparer^				GetSorter(int Column, SortOrder Order) abstract;
		virtual int											GetImageIndex(T Item) abstract;		// returns -1 for no image
		virtual String^										GetSubItemText(T Item, int Column) abstract;
		virtual UInt32										GetColumnCount() abstract;
		virtual void										ActivateItem(T Item) abstract;
		virtual void										KeyPress(KeyEventArgs^ E) abstract;
		virtual UInt32										GetDefaultSortColumn() abstract;
		virtual SortOrder									GetDefaultSortOrder() abstract;
	public:
		SimpleListViewBinder() : Source(nullptr), Sink(nullptr), LastSortColumn(-1), LastSortOrder(SortOrder::Ascending)
		{
			SourceAdd = gcnew SimpleBindingList<T>::AddRemoveEventHandler(this, &SimpleListViewBinder::SourceHandlerAdded);
			SourceRemove = gcnew SimpleBindingList<T>::AddRemoveEventHandler(this, &SimpleListViewBinder::SourceHandlerRemoved);
			SourceClear = gcnew SimpleBindingList<T>::ClearEventHandler(this, &SimpleListViewBinder::SourceHandlerCleared);
			SourceBeginUpdate = gcnew SimpleBindingList<T>::UpdateEventHandler(this, &SimpleListViewBinder::SourceHandlerUpdateStarted);
			SourceEndUpdate = gcnew SimpleBindingList<T>::UpdateEventHandler(this, &SimpleListViewBinder::SourceHandlerUpdateStopped);
			SourceSort = gcnew SimpleBindingList<T>::SortEventHandler(this, &SimpleListViewBinder::SourceHandlerSorted);

			SinkDrawItem = gcnew DrawListViewItemEventHandler(this, &SimpleListViewBinder::SinkHandlerDrawItem);
			SinkDrawSubItem = gcnew DrawListViewSubItemEventHandler(this, &SimpleListViewBinder::SinkHandlerDrawSubItem);
			SinkDrawColumnHeader = gcnew DrawListViewColumnHeaderEventHandler(this, &SimpleListViewBinder::SinkHandlerDrawColumnHeader);
			SinkColumnClick = gcnew ColumnClickEventHandler(this, &SimpleListViewBinder::SinkHandlerColumnClick);
			SinkItemActivate = gcnew EventHandler(this, &SimpleListViewBinder::SinkHandlerItemActivate);
			SinkKeyUp = gcnew KeyEventHandler(this, &SimpleListViewBinder::SinkHandlerKeyUp);
		}

		~SimpleListViewBinder()
		{
			Unbind();

			SAFEDELETE_CLR(SourceAdd);
			SAFEDELETE_CLR(SourceRemove);
			SAFEDELETE_CLR(SourceClear);
			SAFEDELETE_CLR(SourceBeginUpdate);
			SAFEDELETE_CLR(SourceEndUpdate);
			SAFEDELETE_CLR(SourceSort);

			SAFEDELETE_CLR(SinkDrawItem);
			SAFEDELETE_CLR(SinkDrawSubItem);
			SAFEDELETE_CLR(SinkDrawColumnHeader);
			SAFEDELETE_CLR(SinkColumnClick);
			SAFEDELETE_CLR(SinkItemActivate);
			SAFEDELETE_CLR(SinkKeyUp);
		}

		void Bind(ListView^ To, SimpleBindingList<T>^ With)
		{
			Source = With;
			Sink = To;

			Sink->OwnerDraw = true;
			Sink->View = View::Details;
			Sink->Tag = this;

			Sink->DrawItem += SinkDrawItem;
			Sink->DrawSubItem += SinkDrawSubItem;
			Sink->DrawColumnHeader += SinkDrawColumnHeader;
			Sink->ColumnClick += SinkColumnClick;
			Sink->ItemActivate += SinkItemActivate;
			Sink->KeyUp += SinkKeyUp;

			Source->Added += SourceAdd;
			Source->Removed += SourceRemove;
			Source->Cleared += SourceClear;
			Source->UpdateStarted += SourceBeginUpdate;
			Source->UpdateStopped += SourceEndUpdate;
			Source->Sorted += SourceSort;

			LastSortColumn = -1;
			LastSortOrder = SortOrder::Ascending;

			Sink->BeginUpdate();
			Sink->Columns->Clear();
			if (Sink->SmallImageList)
				Sink->SmallImageList->Images->Clear();
			InitializeListView(Sink);
			PopulateFromSource(false);
			Sink->EndUpdate();
		}

		void Unbind()
		{
			if (Bound)
			{
				Source->Added -= SourceAdd;
				Source->Removed -= SourceRemove;
				Source->Cleared -= SourceClear;
				Source->UpdateStarted -= SourceBeginUpdate;
				Source->UpdateStopped -= SourceEndUpdate;
				Source->Sorted -= SourceSort;

				Sink->Items->Clear();
				Sink->Columns->Clear();
				if (Sink->SmallImageList)
					Sink->SmallImageList->Images->Clear();
				Sink->Tag = nullptr;

				Sink->DrawItem -= SinkDrawItem;
				Sink->DrawSubItem -= SinkDrawSubItem;
				Sink->DrawColumnHeader -= SinkDrawColumnHeader;
				Sink->ColumnClick -= SinkColumnClick;
				Sink->ItemActivate -= SinkItemActivate;
				Sink->KeyUp -= SinkKeyUp;

				Source = nullptr;
				Sink = nullptr;

				LastSortColumn = -1;
			}
		}

		property bool Bound
		{
			virtual bool get() { return Sink != nullptr; }
		}
	};
}
