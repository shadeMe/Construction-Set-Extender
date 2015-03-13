#pragma once

#include "IncludesCLR.h"

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
protected:
	virtual property bool DoubleBuffered
	{
		bool get() override { return true; }
	}
};

generic<typename T>
ref class SimpleBindingList : public System::Collections::IEnumerable
{
public:
	ref struct AddRemoveEventArgs
	{
		T		Item;
		bool	UpdateInProgress;

		AddRemoveEventArgs(T Item, bool Updating) : Item(Item), UpdateInProgress(Updating) {}
	};

	ref struct ClearEventArgs
	{
		bool	UpdateInProgress;

		ClearEventArgs(bool Updating) : UpdateInProgress(Updating) {}
	};

	ref struct UpdateEventArgs
	{
		UInt32	Count;		// number of items in the list

		UpdateEventArgs(UInt32 Count) : Count(Count) {}
	};

	delegate void	AddRemoveEventHandler(Object^ Sender, AddRemoveEventArgs^ E);
	delegate void	ClearEventHandler(Object^ Sender, ClearEventArgs^ E);
	delegate void	UpdateEventHandler(Object^ Sender, UpdateEventArgs^ E);
protected:
	List<T>^	DataStore;
	bool		Updating;

	void		OnAdd(T Item) { Added(this, gcnew AddRemoveEventArgs(Item, Updating)); }
	void		OnRemove(T Item) { Removed(this, gcnew AddRemoveEventArgs(Item, Updating)); }
	void		OnClear() { Cleared(this, gcnew ClearEventArgs(Updating)); }
	void		OnBeginUpdate(UInt32 Count) { UpdateStarted(this, gcnew UpdateEventArgs(Count)); }
	void		OnEndUpdate(UInt32 Count) { UpdateStopped(this, gcnew UpdateEventArgs(Count)); }
public:
	SimpleBindingList() : DataStore(gcnew List<T>), Updating(false) {}
	~SimpleBindingList()
	{
		Clear();
	}

	event AddRemoveEventHandler^	Added;
	event AddRemoveEventHandler^	Removed;
	event ClearEventHandler^		Cleared;
	event UpdateEventHandler^		UpdateStarted;
	event UpdateEventHandler^		UpdateStopped;

	void Add(T Item)
	{
		DataStore->Add(Item);
		OnAdd(Item);
	}

	void Remove(T Item)
	{
		DataStore->Remove(Item);
		OnRemove(Item);
	}

	void Clear()
	{
		DataStore->Clear();
		OnClear();
	}

	void BeginUpdate()
	{
		if (Updating)
			throw gcnew System::InvalidOperationException("An update is already in progress");

		Updating = true;
		OnBeginUpdate(DataStore->Count);
	}

	void EndUpdate()
	{
		if (Updating == false)
			throw gcnew System::InvalidOperationException("No update in progress");

		Updating = false;
		OnEndUpdate(DataStore->Count);
	}

	bool Contains(T Item)
	{
		return DataStore->Contains(Item);
	}

	property UInt32 Count
	{
		virtual UInt32 get() { return DataStore->Count; }
		virtual void set(UInt32 e) { throw gcnew System::InvalidOperationException; }
	}

	virtual System::Collections::IEnumerator^ GetEnumerator()
	{
		return DataStore->GetEnumerator();
	}
};

generic<typename T>
ref class SimpleListViewBinder
{
protected:
	SimpleBindingList<T>^	Source;
	ListView^				Sink;
	int						SortColumn;

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
		{
			if (SortColumn != -1)
				Sink->Sort();
		}
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
		PopulateFromSource(true);
		Sink->EndUpdate();
	}

	void SinkHandlerDrawSubItem(Object^ Sender, DrawListViewSubItemEventArgs^ E)
	{
		DrawItem(E);
	}

	void SinkHandlerColumnClick(Object^ Sender, ColumnClickEventArgs^ E)
	{
		if (E->Column != SortColumn)
		{
			SortColumn = E->Column;
			Sink->Sorting = SortOrder::Ascending;
		}
		else
		{
			if (Sink->Sorting == SortOrder::Ascending)
				Sink->Sorting = SortOrder::Descending;
			else
				Sink->Sorting = SortOrder::Ascending;
		}

		Sink->Sort();
		System::Collections::IComparer^ Sorter = GetSorter(E->Column, Sink->Sorting);
		Sink->ListViewItemSorter = Sorter;
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

	DrawListViewSubItemEventHandler^					SinkDrawSubItem;
	ColumnClickEventHandler^							SinkColumnClick;
	EventHandler^										SinkItemActivate;
	KeyEventHandler^									SinkKeyUp;

	virtual void	InitializeListView(ListView^ Control) abstract;							// create headers, image lists, etc
	virtual System::Collections::IComparer^	GetSorter(int Column, SortOrder Order) abstract;

	virtual int		GetImageIndex(T Item) abstract;		// returns -1 for no image
	virtual void	DrawItem(DrawListViewSubItemEventArgs^ E) abstract;		// render subitems
	virtual void	ActivateItem(T Item) abstract;
	virtual void	KeyPress(KeyEventArgs^ E) abstract;
public:
	SimpleListViewBinder() : Source(nullptr), Sink(nullptr), SortColumn(-1)
	{
		SourceAdd = gcnew SimpleBindingList<T>::AddRemoveEventHandler(this, &SimpleListViewBinder::SourceHandlerAdded);
		SourceRemove = gcnew SimpleBindingList<T>::AddRemoveEventHandler(this, &SimpleListViewBinder::SourceHandlerRemoved);
		SourceClear = gcnew SimpleBindingList<T>::ClearEventHandler(this, &SimpleListViewBinder::SourceHandlerCleared);
		SourceBeginUpdate = gcnew SimpleBindingList<T>::UpdateEventHandler(this, &SimpleListViewBinder::SourceHandlerUpdateStarted);
		SourceEndUpdate = gcnew SimpleBindingList<T>::UpdateEventHandler(this, &SimpleListViewBinder::SourceHandlerUpdateStopped);

		SinkDrawSubItem = gcnew DrawListViewSubItemEventHandler(this, &SimpleListViewBinder::SinkHandlerDrawSubItem);
		SinkColumnClick = gcnew ColumnClickEventHandler(this, &SimpleListViewBinder::SinkHandlerColumnClick);
		SinkItemActivate = gcnew EventHandler(this, &SimpleListViewBinder::SinkHandlerItemActivate);
		SinkKeyUp = gcnew KeyEventHandler(this, &SimpleListViewBinder::SinkHandlerKeyUp);
	}

	~SimpleListViewBinder()
	{
		Unbind();
	}

	void Bind(ListView^ To, SimpleBindingList<T>^ With)
	{
		Source = With;
		Sink = To;

		Sink->OwnerDraw = true;
		Sink->View = View::Details;
		Sink->Tag = this;

		Sink->DrawSubItem += SinkDrawSubItem;
		Sink->ColumnClick += SinkColumnClick;
		Sink->ItemActivate += SinkItemActivate;
		Sink->KeyUp += SinkKeyUp;

		Source->Added += SourceAdd;
		Source->Removed += SourceRemove;
		Source->Cleared += SourceClear;
		Source->UpdateStarted += SourceBeginUpdate;
		Source->UpdateStopped += SourceEndUpdate;

		SortColumn = -1;

		Sink->BeginUpdate();
		Sink->Columns->Clear();
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

			Sink->Items->Clear();
			Sink->Columns->Clear();
			Sink->SmallImageList->Images->Clear();
			Sink->Tag = nullptr;

			Sink->DrawSubItem -= SinkDrawSubItem;
			Sink->ColumnClick -= SinkColumnClick;
			Sink->ItemActivate -= SinkItemActivate;
			Sink->KeyUp -= SinkKeyUp;

			Source = nullptr;
			Sink = nullptr;

			SortColumn = -1;
		}
	}

	property bool	Bound
	{
		virtual bool get() { return Sink != nullptr; }
	}
};