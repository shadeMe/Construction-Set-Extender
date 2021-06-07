#pragma once
#include "IncludesCLR.h"

#pragma warning(disable: 4677)

namespace cse
{
	ref class CString
	{
		IntPtr	P;

		void		Free() { Marshal::FreeHGlobal(P); }
	public:
		CString(String^ Source);
		~CString();

		const char* c_str() { return static_cast<char*>(P.ToPointer()); }
	};

	void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State);		// state = 1 [ON], 0 [OFF]

	ref class CSEGeneralException : public System::InvalidOperationException
	{
	public:
		CSEGeneralException(String^ Message) : System::InvalidOperationException(Message) {};
	};

	ref class ImageResourceManager
	{
		ResourceManager^	Manager;
	public:
		ImageResourceManager(String^ BaseName);

		Image^				CreateImage(String^ ResourceIdentifier);
		void				SetupImageForToolStripButton(ToolStripButton^ Control);
	};

	ref class CSEControlDisposer
	{
	public:
		CSEControlDisposer(Control^ Source);
		CSEControlDisposer(Control::ControlCollection^ Source);
	};

	ref class WindowHandleWrapper : public IWin32Window
	{
		IntPtr					_hwnd;
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
		UInt32					CurrentLine;
	public:
		LineTrackingStringReader(String^% In) : StringReader(In), CurrentLine(0) {}

		virtual String^			ReadLine() override
		{
			String^ Out = StringReader::ReadLine();
			if (Out != nullptr)
				CurrentLine++;

			return Out;
		}

		property UInt32			LineNumber			// line no of the last read line
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
		static enum class BackgroundColor
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

		property String^	TooltipHeaderText;
		property String^	TooltipBodyText;
		property Image^		TooltipBodyImage;
		property String^	TooltipFooterText;
		property Image^		TooltipFooterImage;
		property BackgroundColor
							TooltipBgColor;
	};

#ifdef CSE_SE
	DevComponents::DotNetBar::eTooltipColor MapRichTooltipBackgroundColorToDotNetBar(IRichTooltipContentProvider::BackgroundColor BgColor);
#endif
}