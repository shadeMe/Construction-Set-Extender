#pragma once
#include "IncludesCLR.h"

#pragma warning(disable: 4677)

namespace cse
{


ListViewItem^ GetListViewSelectedItem(ListView^ Source);

ref class ListViewGenericSorter abstract
{
protected:
	int	_Column;
	SortOrder _Order;
public:
	ListViewGenericSorter() : _Column(0), _Order(SortOrder::Ascending) {}
	ListViewGenericSorter(int Index, SortOrder Order) : _Column(Index), _Order(Order) {}
};


ref class ListViewIntSorter : public ListViewGenericSorter, public System::Collections::IComparer
{
	bool Hex;
public:
	ListViewIntSorter() : ListViewGenericSorter(), Hex(false) {}
	ListViewIntSorter(int Index, SortOrder Order, bool Hex) : ListViewGenericSorter(Index, Order), Hex(Hex) {}

	virtual int Compare(Object^ X, Object^ Y);
};

ref class ListViewStringSorter : public ListViewGenericSorter, public System::Collections::IComparer
{
public:
	ListViewStringSorter() : ListViewGenericSorter() {}
	ListViewStringSorter(int Index, SortOrder Order) : ListViewGenericSorter(Index, Order) {}

	virtual int Compare(Object^ X, Object^ Y);
};

ref class CString
{
	IntPtr P;

	void Free() { Marshal::FreeHGlobal(P); }
public:
	CString(String^ Source);
	~CString();

	const char* c_str() { return static_cast<char*>(P.ToPointer()); }
};

ref class ImageResourceManager
{
	ResourceManager^ Manager;
public:
	ImageResourceManager(String^ BaseName);

	Image^ CreateImage(String^ ResourceIdentifier);
};

ref class WindowHandleWrapper : public IWin32Window
{
	IntPtr _hwnd;
public:
	WindowHandleWrapper(IntPtr Handle) : _hwnd(Handle) {}

	property IntPtr Handle
	{
		virtual IntPtr get()
		{
			return _hwnd;
		}
	};
};

ref class LineTrackingStringReader : public System::IO::StringReader
{
	UInt32 CurrentLine;
public:
	LineTrackingStringReader(String^% In) : StringReader(In), CurrentLine(0) {}

	virtual String^ ReadLine() override
	{
		String^ Out = StringReader::ReadLine();
		if (Out != nullptr)
			CurrentLine++;

		return Out;
	}

	property UInt32 LineNumber			// line no of the last read line
	{
		virtual UInt32 get() { return CurrentLine; }
	}
};

void CopyStringToCharBuffer(String^% Source, char* Buffer, UInt32 Size);

// Fix for the white bottom border when using the System toolstrip renderer
// https://stackoverflow.com/questions/1918247/how-to-disable-the-line-under-tool-strip-in-winform-c
ref class CustomToolStripSystemRenderer : public ToolStripSystemRenderer
{
protected:
	virtual void OnRenderToolStripBorder(ToolStripRenderEventArgs^ e) override {}
};

// For use with DotNetBar::SuperToolTip
// Text can include limited HTML-markup
// c.f https://www.devcomponents.com/kb2/?p=515
interface class IRichTooltipContentProvider
{
	static enum class eBackgroundColor
	{
		Default,
		Blue,
		Yellow,
		Green,
		Red,
		Magenta,
		BlueMist,
		Lemon,
		Apple,
		Silver,
		Gray
	};

	property String^ TooltipHeaderText;
	property String^ TooltipBodyText;
	property Image^ TooltipBodyImage;
	property String^ TooltipFooterText;
	property Image^ TooltipFooterImage;
	property eBackgroundColor TooltipBgColor;
};

#ifdef CSE_SE
DevComponents::DotNetBar::eTooltipColor MapRichTooltipBackgroundColorToDotNetBar(IRichTooltipContentProvider::eBackgroundColor BgColor);
#endif


} // namespace cse