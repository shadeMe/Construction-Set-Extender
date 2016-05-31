#include "ListViewUtilities.h"

namespace cse
{
	ListViewItem^ GetListViewSelectedItem(ListView^ Source)
	{
		ListViewItem^ Result = nullptr;
		if (Source->VirtualMode == false && Source->SelectedItems->Count)
			Result = Source->SelectedItems[0];
		return Result;
	}

	int GetListViewSelectedItemIndex(ListView^ Source)
	{
		int Result = -1;

		if (Source->SelectedIndices->Count)
			Result = Source->SelectedIndices[0];

		return Result;
	}

	ListViewItem^ FindItemWithText(ListView^ Source, String^ Substring, bool SearchInSubItems, bool CaseInsensitive)
	{
		for each (ListViewItem^ Item in Source->Items)
		{
			if (Item->Text->IndexOf(Substring, 0, (CaseInsensitive ? StringComparison::CurrentCultureIgnoreCase : StringComparison::CurrentCulture)) != -1)
				return Item;
			else if (SearchInSubItems)
			{
				for each (ListViewItem::ListViewSubItem^ SubItem in Item->SubItems)
				{
					if (SubItem->Text->IndexOf(Substring, 0, (CaseInsensitive ? StringComparison::CurrentCultureIgnoreCase : StringComparison::CurrentCulture)) != -1)
						return Item;
				}
			}
		}

		return nullptr;
	}

	int ListViewStringSorter::Compare(Object^ X, Object^ Y)
	{
		int Result = -1;
		Result = String::Compare(((ListViewItem^)X)->SubItems[_Column]->Text, ((ListViewItem^)Y)->SubItems[_Column]->Text, true);
		if (_Order == SortOrder::Descending)	Result *= -1;
		return Result;
	}

	int ListViewIntSorter::Compare(Object^ X, Object^ Y)
	{
		int Result = -1;
		try
		{
			if (Hex)
				Result = Int32::Parse(((ListViewItem^)X)->SubItems[_Column]->Text,
				Globalization::NumberStyles::HexNumber) -
				Int32::Parse(((ListViewItem^)Y)->SubItems[_Column]->Text,
				Globalization::NumberStyles::HexNumber);
			else
				Result = Int32::Parse(((ListViewItem^)X)->SubItems[_Column]->Text) -
				Int32::Parse(((ListViewItem^)Y)->SubItems[_Column]->Text);
		}
		catch (...) {}

		if (_Order == SortOrder::Descending)	Result *= -1;
		return Result;
	}

	int ListViewImgSorter::Compare(Object^ X, Object^ Y)
	{
		int Result = -1;
		Result = ((ListViewItem^)X)->ImageIndex - ((ListViewItem^)Y)->ImageIndex;

		if (_Order == SortOrder::Descending)
			Result *= -1;
		return Result;
	}
}