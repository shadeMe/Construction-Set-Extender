#pragma once

#include "SemanticAnalysis.h"

namespace cse
{


namespace scriptEditor
{


namespace intellisense
{


ref class IntelliSenseItem;
interface class IIntelliSenseInterfaceView;

ref struct IntelliSenseInputEventArgs
{
	static enum class Event
	{
		KeyDown,
		KeyUp,
		MouseDown,
		MouseUp
	};

	property Event				Type;

	property KeyEventArgs^		KeyEvent;
	property MouseEventArgs^	MouseEvent;
	property bool				Handled;

	IntelliSenseInputEventArgs(Event Type, KeyEventArgs^ Source)
	{
		this->Type = Type;
		this->KeyEvent = Source;
		this->MouseEvent = nullptr;
		this->Handled = false;
	}

	IntelliSenseInputEventArgs(Event Type, MouseEventArgs^ Source)
	{
		this->Type = Type;
		this->KeyEvent = nullptr;
		this->MouseEvent = Source;
		this->Handled = false;
	}
};

ref struct IntelliSenseContextChangeEventArgs
{
	static enum class Event
	{
		Reset,

		TextChanged,
		CaretPosChanged,
		ScrollOffsetChanged,

		SemanticAnalysisCompleted
	};

	property Event		Type;

	property int		CaretPos;
	property UInt32		CurrentLineNumber;
	property int		CurrentLineStartPos;
	property bool		CurrentLineInsideViewport;

	property String^	ClippedLineText;				// clipped to the caret pos
	property obScriptParsing::AnalysisData^
						SemanticAnalysisData;

	property Point		DisplayScreenCoords;

	IntelliSenseContextChangeEventArgs(Event Type)
	{
		this->Type = Type;

		CaretPos = -1;
		CurrentLineNumber = 0;
		CurrentLineStartPos = -1;
		CurrentLineInsideViewport = true;

		ClippedLineText = String::Empty;
		SemanticAnalysisData = nullptr;

		DisplayScreenCoords = Point(0, 0);
	}
};

ref struct IntelliSenseInsightHoverEventArgs
{
	static enum class Event
	{
		HoverStart,
		HoverStop
	};

	property Event			Type;

	property UInt32			Line;
	property String^		HoveredToken;
	property String^		PreviousToken;		// relative to the hovered token
	property bool			DotOperatorInUse;
	property bool			HoveringOverComment;

	property List<String^>^	ErrorMessagesForHoveredLine;
	property Point			DisplayScreenCoords;

	IntelliSenseInsightHoverEventArgs(Event Type)
	{
		this->Type = Type;

		Line = 0;
		HoveredToken = String::Empty;
		PreviousToken = String::Empty;
		DotOperatorInUse = false;
		HoveringOverComment = false;
		ErrorMessagesForHoveredLine = gcnew List<String^>;
		DisplayScreenCoords = Point(0, 0);
	}
};

delegate void IntelliSenseInputEventHandler(Object^ Sender, IntelliSenseInputEventArgs^ E);
delegate void IntelliSenseInsightHoverEventHandler(Object^ Sender, IntelliSenseInsightHoverEventArgs^ E);
delegate void IntelliSenseContextChangeEventHandler(Object^ Sender, IntelliSenseContextChangeEventArgs^ E);

interface class IIntelliSenseInterfaceConsumer
{
	event IntelliSenseInputEventHandler^			IntelliSenseInput;
	event IntelliSenseInsightHoverEventHandler^		IntelliSenseInsightHover;
	event IntelliSenseContextChangeEventHandler^	IntelliSenseContextChange;
};


interface class IIntelliSenseInterfaceModel
{
public:
	property List<IntelliSenseItem^>^	DataStore;

	void	Bind(IIntelliSenseInterfaceView^ To);
	void	Unbind();
	bool	IsLocalVariable(String^ Identifier);
};

ref class IntelliSenseShowInsightToolTipArgs : public IRichTooltipContentProvider
{
	String^		TooltipHeaderText_;
	String^		TooltipBodyText_;
	Image^		TooltipBodyImage_;
	String^		TooltipFooterText_;
	Image^		TooltipFooterImage_;
	IRichTooltipContentProvider::eBackgroundColor
				TooltipBgColor_;
public:
	virtual property String^ TooltipHeaderText
	{
		String^ get() { return TooltipHeaderText_; }
		void set(String^ set) { TooltipHeaderText_ = set; }
	}
	virtual property String^ TooltipBodyText
	{
		String^ get() { return TooltipBodyText_; }
		void set(String^ set) { TooltipBodyText_ = set; }
	}
	virtual property Image^	TooltipBodyImage
	{
		Image^ get() { return TooltipBodyImage_; }
		void set(Image^ set) { TooltipBodyImage_ = set; }
	}
	virtual property String^ TooltipFooterText
	{
		String^ get() { return TooltipFooterText_; }
		void set(String^ set) { TooltipFooterText_ = set; }
	}
	virtual property Image^	TooltipFooterImage
	{
		Image^ get() { return TooltipFooterImage_; }
		void set(Image^ set) { TooltipFooterImage_ = set; }
	}
	virtual property IRichTooltipContentProvider::eBackgroundColor TooltipBgColor
	{
		IRichTooltipContentProvider::eBackgroundColor get() { return TooltipBgColor_; }
		void set(IRichTooltipContentProvider::eBackgroundColor set) { TooltipBgColor_ = set; }
	}
	property Point DisplayScreenCoords;
	property IntPtr	ParentWindowHandle;

	IntelliSenseShowInsightToolTipArgs()
	{
		TooltipHeaderText_ = String::Empty;
		TooltipBodyText_ = String::Empty;
		TooltipFooterText_ = String::Empty;
		TooltipBodyImage_ = TooltipFooterImage_ = nullptr;
		TooltipBgColor_ = IRichTooltipContentProvider::eBackgroundColor::Default;

		DisplayScreenCoords = Point(0, 0);
		ParentWindowHandle = IntPtr::Zero;
	}
};

interface class IIntelliSenseInterfaceView
{
public:
	static enum	class MoveDirection
	{
		Up,
		Down,
	};

	event EventHandler^					ItemSelected;
	event EventHandler^					Dismissed;

	property bool						Visible;
	property IntelliSenseItem^			Selection;

	void				Bind(IIntelliSenseInterfaceModel^ To);
	void				Unbind();

	void				ChangeSelection(MoveDirection Direction);
	void				DimOpacity();
	void				ResetOpacity();

	void				ShowInsightToolTip(IntelliSenseShowInsightToolTipArgs^ Args);
	void				HideInsightToolTip();

	void				Update();			// refreshes the item list
	void				Show(Drawing::Point Location, IntPtr Parent);
	void				Hide();
};


} // namespace intelliSense


} // namespace scriptEditor


} // namespace cse