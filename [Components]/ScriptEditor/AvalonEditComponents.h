#pragma once

#include "AvalonEditDefs.h"
#include "[Common]/ListViewUtilities.h"
#include "ScriptTextEditorInterface.h"

using namespace ICSharpCode;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;

/* http://danielgrunwald.de/coding/AvalonEdit/rendering.php */

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		namespace AvalonEditor
		{
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

				property KnownLayer							Layer
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

				virtual int									Line() override;
				virtual String^								Message() override;
				virtual int									ImageIndex() override;
				virtual void								Jump() override;
				virtual bool								Deleted() override;

				IScriptTextEditor::ScriptMessageSource		Source();
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

				virtual int									Line() override;
				virtual String^								Message() override;
				virtual void								Jump() override;
				virtual bool								Deleted() override;

				int											StartOffset();
				int											EndOffset();
			};

			ref class ScriptBookmarkBinder : public SimpleListViewBinder < ScriptBookmark^ >
			{
			protected:
				virtual void	InitializeListView(ListView^ Control) override;
				virtual System::Collections::IComparer^	GetSorter(int Column, SortOrder Order) override;

				virtual int		GetImageIndex(ScriptBookmark^ Item) override;
				virtual void	DrawItem(DrawListViewSubItemEventArgs^ E) override;
				virtual void	ActivateItem(ScriptBookmark^ Item) override;
				virtual void	KeyPress(KeyEventArgs^ E) override;
			};

			ref class ScriptMessageBinder : public SimpleListViewBinder < ScriptMessage^ >
			{
			protected:
				virtual void	InitializeListView(ListView^ Control) override;
				virtual System::Collections::IComparer^	GetSorter(int Column, SortOrder Order) override;

				virtual int		GetImageIndex(ScriptMessage^ Item) override;
				virtual void	DrawItem(DrawListViewSubItemEventArgs^ E) override;
				virtual void	ActivateItem(ScriptMessage^ Item) override;
				virtual void	KeyPress(KeyEventArgs^ E) override;
			};

			ref class ScriptFindResultBinder : public SimpleListViewBinder < ScriptFindResult^ >
			{
			protected:
				bool			HasLine(ScriptFindResult^ Check);

				virtual ListViewItem^	Create(ScriptFindResult^ Data) override;

				virtual void	InitializeListView(ListView^ Control) override;
				virtual System::Collections::IComparer^	GetSorter(int Column, SortOrder Order) override;

				virtual int		GetImageIndex(ScriptFindResult^ Item) override;
				virtual void	DrawItem(DrawListViewSubItemEventArgs^ E) override;
				virtual void	ActivateItem(ScriptFindResult^ Item) override;
				virtual void	KeyPress(KeyEventArgs^ E) override;
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
				static String^								kMetadataSigilBookmark = "CSEBookmark";

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

				TextAnchor^									CreateAnchor(UInt32 Offset);
				void										RefreshBackgroundRenderers(bool IgnoreBatchUpdate);
				void										GetBookmarks(UInt32 At, List<ScriptBookmark^>^% Out);

				List<ColorizerSegment^>^					GetErrorColorizerSegments();
				List<ColorizerSegment^>^					GetFindResultColorizerSegments();
			public:
				LineTrackingManager(AvalonEdit::TextEditor^ ParentEditor);
				~LineTrackingManager();

				void										Bind(ListView^ MessageList, ListView^ BookmarkList, ListView^ FindResultList);
				void										Unbind();

				void										BeginUpdate(UpdateSource Source);
				void										EndUpdate();

				void										TrackMessage(UInt32 Line,
																		 IScriptTextEditor::ScriptMessageType Type,
																		 IScriptTextEditor::ScriptMessageSource Source,
																		 String^ Message);
				void										ClearMessages(IScriptTextEditor::ScriptMessageSource Filter);		// pass None to clear all

				void										AddBookmark(UInt32 Line, String^ Description);
				void										ClearBookmarks();
				String^										SerializeBookmarks();
				void										DeserializeBookmarks(String^ Serialized, bool ClearExisting);

				void										TrackFindResult(UInt32 Start, UInt32 End, String^ Text);
				void										ClearFindResults();

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

#if BUILD_AVALONEDIT_VERSION == AVALONEDIT_5_0_1
			ref class ObScriptCodeFoldingStrategy
#else
			ref class ObScriptCodeFoldingStrategy : public AvalonEdit::Folding::XmlFoldingStrategy
#endif
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

#if BUILD_AVALONEDIT_VERSION == AVALONEDIT_5_0_1
				virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^			CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset);
#else
				virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^			CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset) override;
#endif
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
				static void									OnMouseClick(Object^ Sender, Windows::Input::MouseButtonEventArgs^ E);

				ref class AdornmentData
				{
				public:
					UInt32						JumpLine;
					AvalonEditTextEditor^		Parent;
				};

				AvalonEditTextEditor^						ParentEditor;
				Windows::Media::Imaging::BitmapSource^		IconSource;

				Windows::UIElement^							GenerateAdornment(UInt32 JumpLine, String^ ElementText);
			public:
				virtual int									GetFirstInterestedOffset(Int32 startOffset) override;
				virtual VisualLineElement^					ConstructElement(Int32 offset) override;

				StructureVisualizerRenderer(AvalonEditTextEditor^ Parent);
				~StructureVisualizerRenderer();
			};
		}
	}
}