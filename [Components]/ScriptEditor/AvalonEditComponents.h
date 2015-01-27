#pragma once

#include "AvalonEditDefs.h"
#include "SemanticAnalysis.h"

using namespace ICSharpCode;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		namespace AvalonEditor
		{
			ref class AvalonEditTextEditor;

			// background colorizers
			ref class AvalonEditLineBackgroundColorizer abstract : public AvalonEdit::Rendering::IBackgroundRenderer
			{
			protected:
				AvalonEdit::TextEditor^						ParentEditor;
				KnownLayer									RenderLayer;

				void RenderBackground(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, Windows::Media::Color Background, Windows::Media::Color Border, Double BorderThickness, bool ColorEntireLine);
			public:
				virtual ~AvalonEditLineBackgroundColorizer();

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) = 0;

				property KnownLayer							Layer
				{
					virtual KnownLayer get() { return RenderLayer; }
				}

				AvalonEditLineBackgroundColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer);
			};

			ref class AvalonEditCurrentLineBGColorizer : public AvalonEditLineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditCurrentLineBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer);
			};

			ref class AvalonEditScriptErrorBGColorizer : public AvalonEditLineBackgroundColorizer
			{
				List<int>^									ErrorLines;

				bool										GetLineInError(int Line);
			public:
				virtual ~AvalonEditScriptErrorBGColorizer();

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditScriptErrorBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) : AvalonEditLineBackgroundColorizer(Parent, RenderLayer), ErrorLines(gcnew List<int>()) {}

				void										AddLine(int Line);
				void										ClearLines(void);
			};

			ref class AvalonEditSelectionBGColorizer : public AvalonEditLineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditSelectionBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer);
			};

			ref class AvalonEditLineLimitBGColorizer : public AvalonEditLineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditLineLimitBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer);
			};

			ref class AvalonEditFindReplaceBGColorizer : public AvalonEditLineBackgroundColorizer
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

				AvalonEditFindReplaceBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer);

				void										AddSegment(int Offset, int Length);
				void										ClearSegments();

				virtual ~AvalonEditFindReplaceBGColorizer();
			};

			ref class AvalonEditObScriptIndentStrategy : public AvalonEdit::Indentation::IIndentationStrategy
			{
				AvalonEditTextEditor^						Parent;
				bool										TrimTrailingWhitespace;
				bool										CullEmptyLines;
			public:
				virtual ~AvalonEditObScriptIndentStrategy();

				virtual void								IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line);
				virtual void								IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine);

				AvalonEditObScriptIndentStrategy(AvalonEditTextEditor^ Parent, bool TrimTrailingWhitespace, bool CullEmptyLines);
			};

#if BUILD_AVALONEDIT_VERSION == AVALONEDIT_5_0_1
			ref class AvalonEditObScriptCodeFoldingStrategy
#else
			ref class AvalonEditObScriptCodeFoldingStrategy : public AvalonEdit::Folding::XmlFoldingStrategy
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
				virtual ~AvalonEditObScriptCodeFoldingStrategy();

#if BUILD_AVALONEDIT_VERSION == AVALONEDIT_5_0_1
				virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^			CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset);
#else
				virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^			CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset) override;
#endif
				AvalonEditObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent);
			};

			ref class TagableDoubleAnimation : public System::Windows::Media::Animation::DoubleAnimation
			{
			public:
				property Object^							Tag;

				TagableDoubleAnimation(double fromValue, double toValue, System::Windows::Duration duration, System::Windows::Media::Animation::FillBehavior fillBehavior);
			};

			ref class AvalonEditBraceHighlightingBGColorizer : public AvalonEditLineBackgroundColorizer
			{
				int											OpenBraceOffset;
				int											CloseBraceOffset;
				bool										DoHighlight;
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditBraceHighlightingBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer);

				void										SetHighlight(int OpenBraceOffset, int CloseBraceOffset);
				void										ClearHighlight(void);
			};
		}
	}
}