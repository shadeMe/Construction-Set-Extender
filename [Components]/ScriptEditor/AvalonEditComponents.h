#pragma once

#include "[Common]/ListViewUtilities.h"
#include "ScriptTextEditorInterface.h"

using namespace ICSharpCode;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;
using namespace ICSharpCode::AvalonEdit::Utils;

/* http://danielgrunwald.de/coding/AvalonEdit/rendering.php */

namespace cse
{
	ref class WPFImageResourceGenerator
	{
	public:
		static System::Windows::Media::Imaging::BitmapSource^		CreateImageSource(String^ ResourceIdentifier);
	};

	// lovely hack, this - http://apocryph.org/archives/275
	ref class WPFFocusHelper
	{
		static void		DispatcherCallback(Windows::UIElement^ Element);
		static void		ThreadCallback(Object^ Element);

		delegate void	InvokeDelegate(Windows::UIElement^ Element);
	public:
		static void		Focus(Windows::UIElement^ Element);
	};

	namespace textEditors
	{
		namespace avalonEditor
		{

			System::Windows::Point TransformToPixels(double X, double Y);		// device-independent to pixels
			System::Windows::Point TransformToPixels(System::Windows::Point In);

			ref class AvalonEditTextEditor;
			ref class LineTrackingManager;

			ref class ILineBackgroundColorizer abstract : public AvalonEdit::Rendering::IBackgroundRenderer
			{
			protected:
				AvalonEdit::TextEditor^						ParentEditor;
				KnownLayer									RenderLayer;

				void										RenderBackground(TextView^ Destination,
																			 System::Windows::Media::DrawingContext^ DrawingContext,
																			 int StartOffset, int EndOffset,
																			 Windows::Media::Color Background,
																			 Windows::Media::Color Border,
																			 Double BorderThickness,
																			 bool ColorEntireLine);
			public:
				virtual ~ILineBackgroundColorizer();

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) abstract;

				property KnownLayer Layer
				{
					virtual KnownLayer get() { return RenderLayer; }
				}

				ILineBackgroundColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);
			};

			ref class TrackingMessage abstract
			{
			public:
				virtual int					Line() abstract;
				virtual String^				Message() abstract;

				virtual void				Jump() abstract;
				virtual bool				Deleted() abstract;
			};

			ref class TrackingImageMessage abstract : public TrackingMessage
			{
			public:
				virtual int					ImageIndex() abstract;
			};

			ref struct TrackingMessageSorter abstract
			{
				static enum class ComparisonField
				{
					Line,
					Message,
					ImageIndex
				};
			protected:
				ComparisonField				CompareField;
			public:
				TrackingMessageSorter(ComparisonField Field) : CompareField(Field) {}

				int							Compare(TrackingMessage^ X, TrackingMessage^ Y);
			};

			ref class TrackingMessageListViewSorter : public ListViewGenericSorter, public System::Collections::IComparer
			{
			public:
				TrackingMessageListViewSorter(int Index, SortOrder Order) : ListViewGenericSorter(Index, Order) {}

				virtual int					Compare(Object^ X, Object^ Y);
			};

			ref class TrackingImageMessageListViewSorter : public ListViewGenericSorter, public System::Collections::IComparer
			{
			public:
				TrackingImageMessageListViewSorter(int Index, SortOrder Order) : ListViewGenericSorter(Index, Order) {}

				virtual int					Compare(Object^ X, Object^ Y);
			};

			ref class ScriptMessage : public TrackingImageMessage
			{
				LineTrackingManager^						Manager;
			protected:
				TextAnchor^									Anchor;
				IScriptTextEditor::ScriptMessageType		MessageType;
				IScriptTextEditor::ScriptMessageSource		MessageSource;
				String^										MessageString;
			public:
				ScriptMessage(LineTrackingManager^ Parent, TextAnchor^ Location,
							  IScriptTextEditor::ScriptMessageType Type,
							  IScriptTextEditor::ScriptMessageSource Source,
							  String^ Text);
				~ScriptMessage();

				property bool								IndicatorDisabled;

				virtual int									Line() override;
				virtual String^								Message() override;
				virtual int									ImageIndex() override;
				virtual void								Jump() override;
				virtual bool								Deleted() override;

				IScriptTextEditor::ScriptMessageSource		Source();
				IScriptTextEditor::ScriptMessageType		Type();
			};

			ref struct ScriptMessageSorter : public System::Collections::Generic::IComparer < ScriptMessage^ >, TrackingMessageSorter
			{
				ScriptMessageSorter(ComparisonField Field) : TrackingMessageSorter(Field) {}

				virtual int Compare(ScriptMessage^ X, ScriptMessage^ Y)
				{
					return TrackingMessageSorter::Compare(X, Y);
				}
			};

			ref class ScriptBookmark : public TrackingMessage
			{
				LineTrackingManager^						Manager;
			protected:
				TextAnchor^									Anchor;
				String^										Description;
			public:
				ScriptBookmark(LineTrackingManager^ Parent, TextAnchor^ Location, String^ Text);
				~ScriptBookmark();

				virtual int									Line() override;
				virtual String^								Message() override;
				virtual void								Jump() override;
				virtual bool								Deleted() override;
			};

			ref struct ScriptBookmarkSorter : public System::Collections::Generic::IComparer < ScriptBookmark^ >, TrackingMessageSorter
			{
				ScriptBookmarkSorter(ComparisonField Field) : TrackingMessageSorter(Field) {}

				virtual int Compare(ScriptBookmark^ X, ScriptBookmark^ Y)
				{
					return TrackingMessageSorter::Compare(X, Y);
				}
			};

			ref class ScriptFindResult : public TrackingMessage
			{
				LineTrackingManager^						Manager;
			protected:
				TextAnchor^									AnchorStart;
				TextAnchor^									AnchorEnd;
				String^										Description;
			public:
				ScriptFindResult(LineTrackingManager^ Parent, TextAnchor^ Start, TextAnchor^ End, String^ Text);
				~ScriptFindResult();

				property bool								IndicatorDisabled;

				virtual int									Line() override;
				virtual String^								Message() override;
				virtual void								Jump() override;
				virtual bool								Deleted() override;

				int											StartOffset();
				int											EndOffset();
			};

			ref struct ScriptFindResultSorter : public System::Collections::Generic::IComparer < ScriptFindResult^ >, TrackingMessageSorter
			{
				ScriptFindResultSorter(ComparisonField Field) : TrackingMessageSorter(Field) {}

				virtual int Compare(ScriptFindResult^ X, ScriptFindResult^ Y)
				{
					return TrackingMessageSorter::Compare(X, Y);
				}
			};

			ref class ScriptBookmarkBinder : public SimpleListViewBinder < ScriptBookmark^ >
			{
			protected:
				virtual void	InitializeListView(ListView^ Control) override;
				virtual System::Collections::IComparer^	GetSorter(int Column, SortOrder Order) override;

				virtual int		GetImageIndex(ScriptBookmark^ Item) override;
				virtual String^	GetSubItemText(ScriptBookmark^ Item, int Column) override;
				virtual void	ActivateItem(ScriptBookmark^ Item) override;
				virtual void	KeyPress(KeyEventArgs^ E) override;
				virtual UInt32	GetColumnCount() override;
				virtual UInt32	GetDefaultSortColumn() override;
				virtual SortOrder GetDefaultSortOrder() override;
			};

			ref class ScriptMessageBinder : public SimpleListViewBinder < ScriptMessage^ >
			{
			protected:
				virtual void	InitializeListView(ListView^ Control) override;
				virtual System::Collections::IComparer^	GetSorter(int Column, SortOrder Order) override;

				virtual int		GetImageIndex(ScriptMessage^ Item) override;
				virtual String^	GetSubItemText(ScriptMessage^ Item, int Column) override;
				virtual void	ActivateItem(ScriptMessage^ Item) override;
				virtual void	KeyPress(KeyEventArgs^ E) override;
				virtual UInt32	GetColumnCount() override;
				virtual UInt32	GetDefaultSortColumn() override;
				virtual SortOrder GetDefaultSortOrder() override;
			};

			ref class ScriptFindResultBinder : public SimpleListViewBinder < ScriptFindResult^ >
			{
			protected:
				bool			HasLine(ScriptFindResult^ Check);

				virtual ListViewItem^	Create(ScriptFindResult^ Data) override;

				virtual void	InitializeListView(ListView^ Control) override;
				virtual System::Collections::IComparer^	GetSorter(int Column, SortOrder Order) override;

				virtual int		GetImageIndex(ScriptFindResult^ Item) override;
				virtual String^	GetSubItemText(ScriptFindResult^ Item, int Column) override;
				virtual void	ActivateItem(ScriptFindResult^ Item) override;
				virtual void	KeyPress(KeyEventArgs^ E) override;
				virtual UInt32	GetColumnCount() override;
				virtual UInt32	GetDefaultSortColumn() override;
				virtual SortOrder GetDefaultSortOrder() override;
			};

			ref struct ColorizerSegment
			{
				int Start;
				int End;
			};

			delegate List<ColorizerSegment^>^				GetColorizerSegments();

			ref class ScriptErrorIndicator : public AvalonEdit::Rendering::IBackgroundRenderer
			{
			protected:
				GetColorizerSegments^						Delegate;

				void										RenderSquiggly(TextView^ Destination,
																		   System::Windows::Media::DrawingContext^ DrawingContext,
																		   int StartOffset, int EndOffset,
																		   Windows::Media::Color Color);
			public:
				ScriptErrorIndicator(GetColorizerSegments^ Getter);
				~ScriptErrorIndicator();

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);

				property KnownLayer							Layer
				{
					virtual KnownLayer get() { return KnownLayer::Background; }
				}
			};

			ref class ScriptFindResultIndicator : public ILineBackgroundColorizer
			{
			protected:
				GetColorizerSegments^						Delegate;
			public:
				ScriptFindResultIndicator(GetColorizerSegments^ Getter);
				~ScriptFindResultIndicator();

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;
			};

			ref class LineTrackingManager
			{
			public:
				static enum class UpdateSource
				{
					None,
					Messages,
					Bookmarks,
					FindResults,
				};
			private:
				AvalonEdit::TextEditor^						Parent;

				SimpleBindingList<ScriptMessage^>^			Messages;
				SimpleBindingList<ScriptBookmark^>^			Bookmarks;
				SimpleBindingList<ScriptFindResult^>^		FindResults;

				ScriptMessageBinder^						BinderMessages;
				ScriptBookmarkBinder^						BinderBookmarks;
				ScriptFindResultBinder^						BinderFindResults;

				UpdateSource								CurrentBatchUpdate;
				int											CurrentUpdateCounter;

				ScriptErrorIndicator^						ErrorColorizer;
				ScriptFindResultIndicator^					FindResultColorizer;

				EventHandler^								ParentTextChangedHandler;

				void										Parent_TextChanged(Object^ Sender, EventArgs^ E);

				TextAnchor^									CreateAnchor(UInt32 Offset, bool AllowDeletion);
				void										RefreshBackgroundRenderers(bool IgnoreBatchUpdate);
				UInt32										GetFindResults(UInt32 At, List<ScriptFindResult^>^% Out);

				List<ColorizerSegment^>^					GetErrorColorizerSegments();
				List<ColorizerSegment^>^					GetFindResultColorizerSegments();

				void										OnTrackedDataUpdated();
			public:
				LineTrackingManager(AvalonEdit::TextEditor^ ParentEditor);
				~LineTrackingManager();

				event EventHandler^							TrackedDataUpdated;

				void										Bind(ListView^ MessageList, ListView^ BookmarkList, ListView^ FindResultList);
				void										Unbind();

				void										BeginUpdate(UpdateSource Source);
				void										EndUpdate(bool Sort);

				void										TrackMessage(UInt32 Line,
																		 IScriptTextEditor::ScriptMessageType Type,
																		 IScriptTextEditor::ScriptMessageSource Source,
																		 String^ Message);
				void										ClearMessages(IScriptTextEditor::ScriptMessageSource SourceFilter,
																		  IScriptTextEditor::ScriptMessageType TypeFilter);		// pass None to clear all
				bool										GetMessages(UInt32 Line,
																		IScriptTextEditor::ScriptMessageSource SourceFilter,
																		IScriptTextEditor::ScriptMessageType TypeFilter,
																		List<ScriptMessage^>^% OutMessages);					// returns false when there are no messages
				UInt32										GetMessageCount(UInt32 Line,
																			IScriptTextEditor::ScriptMessageSource SourceFilter,
																			IScriptTextEditor::ScriptMessageType TypeFilter);	// pass zero as line to count all lines

				void										AddBookmark(UInt32 Line, String^ Description);
				UInt32										GetBookmarks(UInt32 Line, List<ScriptBookmark^>^% Out);
				List<ScriptTextMetadata::Bookmark^>^		GetAllBookmarks();
				void										ClearBookmarks();

				void										TrackFindResult(UInt32 Start, UInt32 End, String^ Text);
				void										ClearFindResults(bool IndicatorOnly);

				void										Cleanup();						// removes deleted anchors
				void										Jump(TrackingMessage^ To);
			};

			ref class CurrentLineBGColorizer : public ILineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				CurrentLineBGColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);
			};

			ref class SelectionBGColorizer : public ILineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				SelectionBGColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);
			};

			ref class LineLimitBGColorizer : public ILineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				LineLimitBGColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);
			};

			ref class ObScriptIndentStrategy : public AvalonEdit::Indentation::IIndentationStrategy
			{
				AvalonEditTextEditor^						Parent;
				bool										TrimTrailingWhitespace;
				bool										CullEmptyLines;
			public:
				virtual ~ObScriptIndentStrategy();

				virtual void								IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line);
				virtual void								IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine);

				ObScriptIndentStrategy(AvalonEditTextEditor^ Parent, bool TrimTrailingWhitespace, bool CullEmptyLines);
			};

			ref class ObScriptCodeFoldingStrategy
			{
				ref class FoldingSorter : public IComparer<AvalonEdit::Folding::NewFolding^>
				{
				public:
					virtual int								Compare(AvalonEdit::Folding::NewFolding^ X, AvalonEdit::Folding::NewFolding^ Y);
				};

				AvalonEditTextEditor^						Parent;
				FoldingSorter^								Sorter;
			public:
				virtual ~ObScriptCodeFoldingStrategy();

				virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^			CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset);
				ObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent);
			};

			ref class TagableDoubleAnimation : public System::Windows::Media::Animation::DoubleAnimation
			{
			public:
				property Object^							Tag;

				TagableDoubleAnimation(double fromValue, double toValue, System::Windows::Duration duration, System::Windows::Media::Animation::FillBehavior fillBehavior);
			};

			ref class BraceHighlightingBGColorizer : public ILineBackgroundColorizer
			{
				int											OpenBraceOffset;
				int											CloseBraceOffset;
				bool										DoHighlight;
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				BraceHighlightingBGColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);

				void										SetHighlight(int OpenBraceOffset, int CloseBraceOffset);
				void										ClearHighlight(void);
			};

			ref class StructureVisualizerRenderer : public VisualLineElementGenerator
			{
			protected:
				static void											OnMouseClick(Object^ Sender, Windows::Input::MouseButtonEventArgs^ E);

				static Windows::Media::Imaging::BitmapSource^		ElementIcon = nullptr;
				static int											InstanceCounter = 0;

				static Windows::Media::Imaging::BitmapSource^		GetIconSource();

				ref class AdornmentData
				{
				public:
					UInt32						JumpLine;
					AvalonEditTextEditor^		Parent;
				};

				AvalonEditTextEditor^						ParentEditor;

				Windows::UIElement^							GenerateAdornment(UInt32 JumpLine, String^ ElementText);
			public:
				virtual int									GetFirstInterestedOffset(Int32 startOffset) override;
				virtual VisualLineElement^					ConstructElement(Int32 offset) override;

				StructureVisualizerRenderer(AvalonEditTextEditor^ Parent);
				~StructureVisualizerRenderer();
			};

			// derived from ICSharpCode.AvalonEdit.AddIn.IconBarMargin
			ref class IconMargin : public AbstractMargin
			{
			protected:
				MouseHoverLogic^	HoverLogic;

				EventHandler<System::Windows::Input::MouseEventArgs^>^		HandlerHover;
				EventHandler<System::Windows::Input::MouseEventArgs^>^		HandlerHoverStopped;
				EventHandler^												HandlerTextViewChanged;

				void	OnHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
				void	OnHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
				void	OnRedrawRequested(Object^ sender, EventArgs^ E);

				virtual void								OnTextViewChanged(AvalonEdit::Rendering::TextView^ oldTextView,
																			  AvalonEdit::Rendering::TextView^ newTextView) override;
				virtual Windows::Media::HitTestResult^		HitTestCore(Windows::Media::PointHitTestParameters^ hitTestParameters) override;
				virtual Windows::Size						MeasureOverride(Windows::Size availableSize) override;
				virtual void								OnRender(Windows::Media::DrawingContext^ drawingContext) override;
				virtual void								OnMouseDown(System::Windows::Input::MouseButtonEventArgs^ e) override;
				virtual void								OnMouseMove(System::Windows::Input::MouseEventArgs^ e) override;
				virtual void								OnMouseUp(System::Windows::Input::MouseButtonEventArgs^ e) override;
				virtual void								OnMouseLeave(System::Windows::Input::MouseEventArgs^ e) override;

				int											GetLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e);
				VisualLine^									GetVisualLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e);

				virtual void								HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E) abstract;
				virtual void								HandleHoverStop() abstract;
				virtual void								HandleClick(int Line) abstract;
				virtual bool								GetRenderData(int Line,
																		  Windows::Media::Imaging::BitmapSource^% OutIcon,
																		  double% OutOpacity,
																		  int% Width,
																		  int% Height) abstract;		// return false to skip rendering the line
			public:
				IconMargin();
				~IconMargin();
			};

			ref class DefaultIconMargin : public IconMargin
			{
				static int											InstanceCounter = 0;
				static Windows::Media::Imaging::BitmapSource^		WarningIcon = nullptr;
				static Windows::Media::Imaging::BitmapSource^		ErrorIcon = nullptr;
				static Windows::Media::Imaging::BitmapSource^		BookmarkIcon = nullptr;

				static Windows::Media::Imaging::BitmapSource^		GetWarningIcon();
				static Windows::Media::Imaging::BitmapSource^		GetErrorIcon();
				static Windows::Media::Imaging::BitmapSource^		GetBookmarkIcon();
			protected:
				AvalonEdit::TextEditor^		Parent;
				LineTrackingManager^		LineTracker;
				Windows::Forms::ToolTip^	Popup;
				IntPtr						PopupParentHandle;

				virtual void				HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E) override;
				virtual void				HandleHoverStop() override;
				virtual void				HandleClick(int Line) override;
				virtual bool				GetRenderData(int Line,
														Windows::Media::Imaging::BitmapSource^% OutIcon,
														double% OutOpacity,
														int% Width,
														int% Height) override;

				void						ShowPopup(String^ Title, String^ Message, ToolTipIcon Icon, Drawing::Point Location);
				void						HidePopup();
			public:
				DefaultIconMargin(AvalonEdit::TextEditor^ ParentEditor, LineTrackingManager^ ParentLineTracker, IntPtr ToolTipParent);
				~DefaultIconMargin();
			};
		}
	}
}