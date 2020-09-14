#pragma once

#include "SemanticAnalysis.h"

namespace cse
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

			property String^		CurrentToken;
			property String^		PreviousToken;
			property bool			DotOperatorInUse;

			property bool			UseOverrideParams;
			property String^		OverrideTitle;
			property String^		OverrideText;
			property ToolTipIcon	OverrideIcon;

			property Point			DisplayScreenCoords;

			IntelliSenseInsightHoverEventArgs(Event Type)
			{
				this->Type = Type;

				CurrentToken = String::Empty;
				PreviousToken = String::Empty;
				DotOperatorInUse = false;

				UseOverrideParams = false;
				OverrideTitle = String::Empty;
				OverrideText = String::Empty;
				OverrideIcon = ToolTipIcon::None;

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
			property List<IntelliSenseItem^>^		DataStore;

			void									Bind(IIntelliSenseInterfaceView^ To);
			void									Unbind();
		};

		ref struct IntelliSenseShowInsightToolTipArgs
		{
			property String^		Title;
			property String^		Text;
			property ToolTipIcon	Icon;

			property Point			DisplayScreenCoords;
			property IntPtr			ParentWindowHandle;

			IntelliSenseShowInsightToolTipArgs()
			{
				Title = String::Empty;
				Text = String::Empty;
				Icon = ToolTipIcon::None;

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
			void				Show(Point Location, IntPtr Parent);
			void				Hide();
		};
	}
}