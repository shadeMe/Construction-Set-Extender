#include "MiscUtilities.h"
#include "NativeWrapper.h"

namespace cse
{
ListViewItem^ GetListViewSelectedItem(ListView^ Source)
{
	ListViewItem^ Result = nullptr;
	if (Source->VirtualMode == false && Source->SelectedItems->Count)
		Result = Source->SelectedItems[0];
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

int ListViewStringSorter::Compare(Object^ X, Object^ Y)
{
	int Result = -1;
	Result = String::Compare(((ListViewItem^)X)->SubItems[_Column]->Text, ((ListViewItem^)Y)->SubItems[_Column]->Text, true);
	if (_Order == SortOrder::Descending)	Result *= -1;
	return Result;
}

CString::CString(System::String^ Source)
{
	P = Marshal::StringToHGlobalAnsi(Source);
}

CString::~CString()
{
	this->Free();
}


ImageResourceManager::ImageResourceManager(String^ BaseName)
{
	Manager = gcnew ResourceManager(BaseName, Assembly::GetExecutingAssembly());
}

Image^ ImageResourceManager::CreateImage(String^ ResourceIdentifier)
{
	try {
		return dynamic_cast<Image^>(Manager->GetObject(ResourceIdentifier));
	} catch (...) {
		return nullptr;
	}
}

void CopyStringToCharBuffer( String^% Source, char* Buffer, UInt32 Size )
{
	int i = 0;
	for (i=0; i < Source->Length && i < Size; i++)
		*(Buffer + i) = Source[i];

	if (i < Size)
		*(Buffer + i) = '\0';
	else
		Buffer[Size - 1] = '\0';
}

#ifdef CSE_SE
DevComponents::DotNetBar::eTooltipColor MapRichTooltipBackgroundColorToDotNetBar(IRichTooltipContentProvider::eBackgroundColor BgColor)
{
	switch (BgColor)
	{
	case IRichTooltipContentProvider::eBackgroundColor::Default:
		return DevComponents::DotNetBar::eTooltipColor::System;
	case IRichTooltipContentProvider::eBackgroundColor::Blue:
		return DevComponents::DotNetBar::eTooltipColor::Blue;
	case IRichTooltipContentProvider::eBackgroundColor::Yellow:
		return DevComponents::DotNetBar::eTooltipColor::Yellow;
	case IRichTooltipContentProvider::eBackgroundColor::Green:
		return DevComponents::DotNetBar::eTooltipColor::Green;
	case IRichTooltipContentProvider::eBackgroundColor::Red:
		return DevComponents::DotNetBar::eTooltipColor::Red;
	case IRichTooltipContentProvider::eBackgroundColor::Magenta:
		return DevComponents::DotNetBar::eTooltipColor::Magenta;
	case IRichTooltipContentProvider::eBackgroundColor::BlueMist:
		return DevComponents::DotNetBar::eTooltipColor::BlueMist;
	case IRichTooltipContentProvider::eBackgroundColor::Lemon:
		return DevComponents::DotNetBar::eTooltipColor::Lemon;
	case IRichTooltipContentProvider::eBackgroundColor::Apple:
		return DevComponents::DotNetBar::eTooltipColor::Apple;
	case IRichTooltipContentProvider::eBackgroundColor::Silver:
		return DevComponents::DotNetBar::eTooltipColor::Silver;
	case IRichTooltipContentProvider::eBackgroundColor::Gray:
		return DevComponents::DotNetBar::eTooltipColor::Gray;
	default:
		return DevComponents::DotNetBar::eTooltipColor::Default;
	}
}
#endif
}