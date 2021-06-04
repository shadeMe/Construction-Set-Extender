#include "MiscUtilities.h"
#include "NativeWrapper.h"

namespace cse
{
	CString::CString(System::String^ Source)
	{
		P = Marshal::StringToHGlobalAnsi(Source);
	}

	CString::~CString()
	{
		this->Free();
	}

	void ToggleFlag(UInt32* Flag, UInt32 Mask, bool State)
	{
		if (State)	*Flag |= Mask;
		else		*Flag &= ~Mask;
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

	void ImageResourceManager::SetupImageForToolStripButton(ToolStripButton^ Control)
	{
		Control->Image = CreateImage(Control->Name);
	}

	CSEControlDisposer::CSEControlDisposer( Control^ Source )
	{
		for each (Control^ Itr in Source->Controls)
		{
			try
			{
				CSEControlDisposer Disposer(Itr);
			} catch (...) {}

			delete Itr;
		}
	}

	CSEControlDisposer::CSEControlDisposer( Control::ControlCollection^ Source )
	{
		for each (Control^ Itr in Source)
		{
			try
			{
				CSEControlDisposer Disposer(Itr);
			} catch (...) {}

			delete Itr;
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
	DevComponents::DotNetBar::eTooltipColor MapRichTooltipBackgroundColorToDotNetBar(IRichTooltipContentProvider::BackgroundColor BgColor)
	{
		switch (BgColor)
		{
		case IRichTooltipContentProvider::BackgroundColor::Default:
			return DevComponents::DotNetBar::eTooltipColor::System;
		case IRichTooltipContentProvider::BackgroundColor::Blue:
			return DevComponents::DotNetBar::eTooltipColor::Blue;
		case IRichTooltipContentProvider::BackgroundColor::Yellow:
			return DevComponents::DotNetBar::eTooltipColor::Yellow;
		case IRichTooltipContentProvider::BackgroundColor::Green:
			return DevComponents::DotNetBar::eTooltipColor::Green;
		case IRichTooltipContentProvider::BackgroundColor::Red:
			return DevComponents::DotNetBar::eTooltipColor::Red;
		case IRichTooltipContentProvider::BackgroundColor::Magenta:
			return DevComponents::DotNetBar::eTooltipColor::Magenta;
		case IRichTooltipContentProvider::BackgroundColor::BlueMist:
			return DevComponents::DotNetBar::eTooltipColor::BlueMist;
		case IRichTooltipContentProvider::BackgroundColor::Lemon:
			return DevComponents::DotNetBar::eTooltipColor::Lemon;
		case IRichTooltipContentProvider::BackgroundColor::Apple:
			return DevComponents::DotNetBar::eTooltipColor::Apple;
		case IRichTooltipContentProvider::BackgroundColor::Silver:
			return DevComponents::DotNetBar::eTooltipColor::Silver;
		case IRichTooltipContentProvider::BackgroundColor::Gray:
			return DevComponents::DotNetBar::eTooltipColor::Gray;
		default:
			return DevComponents::DotNetBar::eTooltipColor::Default;
		}
	}
#endif
}