#include "ListViewUtilities.h"

ListViewItem^ GetListViewSelectedItem(ListView^% Source)
{
	ListViewItem^ Result = nullptr;
	if (Source->SelectedItems->Count)
		Result = Source->SelectedItems[0];
	return Result;
}


int CSEListViewStringSorter::Compare(Object^ X, Object^ Y)
{
	int Result = -1;
	Result = String::Compare(((ListViewItem^)X)->SubItems[_Column]->Text, ((ListViewItem^)Y)->SubItems[_Column]->Text, true);
	if (_Order == SortOrder::Descending)	Result *= -1;
	return Result;
}

int CSEListViewIntSorter::Compare(Object^ X, Object^ Y)
{
	int Result = -1;
	try
	{
		if (Hex)
			Result = Int32::Parse(	((ListViewItem^)X)->SubItems[_Column]->Text, 
									Globalization::NumberStyles::HexNumber) - 
					Int32::Parse(	((ListViewItem^)Y)->SubItems[_Column]->Text, 
									Globalization::NumberStyles::HexNumber);
		else
			Result = Int32::Parse(((ListViewItem^)X)->SubItems[_Column]->Text) - 
					Int32::Parse(((ListViewItem^)Y)->SubItems[_Column]->Text);
	}
	catch (...) {}

	if (_Order == SortOrder::Descending)	Result *= -1;
	return Result;
}

int CSEListViewImgSorter::Compare(Object^ X, Object^ Y)
{
	int Result = -1;
	Result = ((ListViewItem^)X)->ImageIndex - ((ListViewItem^)Y)->ImageIndex;

	if (_Order == SortOrder::Descending)	Result *= -1;
	return Result;
}