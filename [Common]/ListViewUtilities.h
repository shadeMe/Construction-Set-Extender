#pragma once

#include "Includes.h"

ListViewItem^											GetListViewSelectedItem(ListView^% Source);

ref class												CSEListViewSorter
{
protected:
	int													_Column;
	SortOrder											_Order;
public:
	CSEListViewSorter() : _Column(0), _Order(SortOrder::Ascending) {}
	CSEListViewSorter(int Index, SortOrder Order) : _Column(Index), _Order(Order) {}	
};

ref class CSEListViewStringSorter : public CSEListViewSorter, public System::Collections::IComparer
{
public:
	CSEListViewStringSorter() : CSEListViewSorter() {}
	CSEListViewStringSorter(int Index, SortOrder Order) : CSEListViewSorter(Index, Order) {}	

	virtual int											Compare(Object^ X, Object^ Y);
};

ref class CSEListViewIntSorter : public CSEListViewSorter, public System::Collections::IComparer
{
	bool												Hex;
public:
	CSEListViewIntSorter() : CSEListViewSorter(), Hex(false) {}
	CSEListViewIntSorter(int Index, SortOrder Order, bool Hex) : CSEListViewSorter(Index, Order), Hex(Hex) {}	

	virtual int											Compare(Object^ X, Object^ Y);
};

ref class CSEListViewImgSorter : public CSEListViewSorter, public System::Collections::IComparer
{
public:
	CSEListViewImgSorter() : CSEListViewSorter() {}
	CSEListViewImgSorter(int Index, SortOrder Order) : CSEListViewSorter(Index, Order) {}	

	virtual int											Compare(Object^ X, Object^ Y);
};
