#pragma once

#include "AvalonEditDefs.h"
#include "SemanticAnalysis.h"

using namespace ICSharpCode;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;

// http://danielgrunwald.de/coding/AvalonEdit/rendering.php

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		namespace AvalonEditor
		{
			ref class AvalonEditTextEditor;
/*

			ref class AnchorSegmentTracker
			{
				typedef Dictionary<ScriptErrorMessage^, AnchorSegment^> ErrorMessageTableT;

				List<AnchorSegment^>^					FindReplaceResults;
				ErrorMessageTableT^						CompileTimeErrors;
				List<AnchorSegment^>^					ValidatorErrors;		// updated from the semantic analysis cache

				AvalonEdit::Document::TextDocument^		Parent;
			public:
				AnchorSegmentTracker(AvalonEdit::Document::TextDocument^ Source);
				~AnchorSegmentTracker();

				void							TrackFindReplaceResult(int StartOffset, int EndOffset);
				void							ClearFindReplaceResults(void);

				void							TrackCompileTimeError(UInt32 Line);
				void							ClearCompileTimeErrors(void);
				void
			};*/

			ref class ILineBackgroundColorizer abstract : public AvalonEdit::Rendering::IBackgroundRenderer
			{
			protected:
				AvalonEdit::TextEditor^						ParentEditor;
				KnownLayer									RenderLayer;

				void RenderBackground(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, Windows::Media::Color Background, Windows::Media::Color Border, Double BorderThickness, bool ColorEntireLine);
			public:
				virtual ~ILineBackgroundColorizer();

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) = 0;

				property KnownLayer							Layer
				{
					virtual KnownLayer get() { return RenderLayer; }
				}

				ILineBackgroundColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);
			};

			ref class CurrentLineBGColorizer : public ILineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				CurrentLineBGColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);
			};

			ref class ScriptErrorBGColorizer : public ILineBackgroundColorizer
			{
				List<int>^									ErrorLines;

				bool										GetLineInError(int Line);
				void										RenderSquiggly(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, Windows::Media::Color Color);
			public:
				virtual ~ScriptErrorBGColorizer();

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				ScriptErrorBGColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer) : ILineBackgroundColorizer(Parent, RenderLayer), ErrorLines(gcnew List<int>()) {}

				void										AddLine(int Line);
				void										ClearLines(void);
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

			ref class FindReplaceBGColorizer : public ILineBackgroundColorizer
			{
				value struct Segment
				{
					int										Offset;
					int										Length;

					Segment(int Offset, int Length);
				};

				List<Segment>^								HighlightSegments;
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				FindReplaceBGColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer);

				void										AddSegment(int Offset, int Length);
				void										ClearSegments();

				virtual ~FindReplaceBGColorizer();
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
			};
		}
	}
}