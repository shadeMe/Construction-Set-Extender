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