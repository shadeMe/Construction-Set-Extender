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


}