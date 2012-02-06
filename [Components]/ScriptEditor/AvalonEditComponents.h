#pragma once

#include "ScriptParser.h"

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

			ref class AvalonEditLineColorizingTransformer abstract : public AvalonEdit::Rendering::DocumentColorizingTransformer
			{
			protected:
				AvalonEdit::TextEditor^						ParentEditor;
				virtual void								PerformColorization(VisualLineElement^ Element) = 0;
			public:
				virtual ~AvalonEditLineColorizingTransformer()
				{
					ParentEditor = nullptr;
				}
				AvalonEditLineColorizingTransformer(AvalonEdit::TextEditor^% Parent) : DocumentColorizingTransformer(), ParentEditor(Parent) {}
			};

			// deprecated in favor of BackgroundColorizers
			ref class AvalonEditSelectionColorizingTransformer : public AvalonEditLineColorizingTransformer
			{
			protected:
				virtual void								PerformColorization(VisualLineElement^ Element) override;
				virtual void								ColorizeLine(DocumentLine^ line) override;
			public:
				AvalonEditSelectionColorizingTransformer(AvalonEdit::TextEditor^% Parent) : AvalonEditLineColorizingTransformer(Parent) {}
			};

			ref class AvalonEditLineLimitColorizingTransformer : public AvalonEditLineColorizingTransformer
			{
			protected:
				virtual void								PerformColorization(VisualLineElement^ Element) override;
				virtual void								ColorizeLine(DocumentLine^ line) override;
			public:
				AvalonEditLineLimitColorizingTransformer(AvalonEdit::TextEditor^% Parent) : AvalonEditLineColorizingTransformer(Parent) {}
			};

			// background colorizers
			ref class AvalonEditLineBackgroundColorizer abstract : public AvalonEdit::Rendering::IBackgroundRenderer
			{
			protected:
				AvalonEdit::TextEditor^						ParentEditor;
				KnownLayer									RenderLayer;

				void RenderBackground(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, Windows::Media::Color Background, Windows::Media::Color Border, Double BorderThickness, bool ColorEntireLine);
			public:
				virtual ~AvalonEditLineBackgroundColorizer()
				{
					ParentEditor = nullptr;
				}

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) = 0;

				property KnownLayer							Layer
				{
					virtual KnownLayer get() { return RenderLayer; }
				}

				AvalonEditLineBackgroundColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) : ParentEditor(Parent), RenderLayer(RenderLayer) {}
			};

			ref class AvalonEditCurrentLineBGColorizer : public AvalonEditLineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditCurrentLineBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) : AvalonEditLineBackgroundColorizer(Parent, RenderLayer) {}
			};

			ref class AvalonEditScriptErrorBGColorizer : public AvalonEditLineBackgroundColorizer
			{
				List<int>^									ErrorLines;

				bool										GetLineInError(int Line);
			public:
				virtual ~AvalonEditScriptErrorBGColorizer()
				{
					ClearLines();
				}

				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditScriptErrorBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) : AvalonEditLineBackgroundColorizer(Parent, RenderLayer), ErrorLines(gcnew List<int>()) {}

				void										AddLine(int Line);
				void										ClearLines(void);
			};

			ref class AvalonEditSelectionBGColorizer : public AvalonEditLineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditSelectionBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) : AvalonEditLineBackgroundColorizer(Parent, RenderLayer) {}
			};

			ref class AvalonEditLineLimitBGColorizer : public AvalonEditLineBackgroundColorizer
			{
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditLineLimitBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) : AvalonEditLineBackgroundColorizer(Parent, RenderLayer) {}
			};

			ref class AvalonEditFindReplaceBGColorizer : public AvalonEditLineBackgroundColorizer
			{
				value struct Segment
				{
					int										Offset;
					int										Length;

					Segment(int Offset, int Length) : Offset(Offset), Length(Length) {}
				};

				List<Segment>^								HighlightSegments;
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditFindReplaceBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) : AvalonEditLineBackgroundColorizer(Parent, RenderLayer), HighlightSegments(gcnew List<Segment>()) {}

				void										AddSegment(int Offset, int Length);
				void										ClearSegments();

				virtual ~AvalonEditFindReplaceBGColorizer()
				{
					ClearSegments();
				}
			};

			ref class AvalonEditObScriptIndentStrategy : public AvalonEdit::Indentation::IIndentationStrategy
			{
				ScriptParser^								IndentParser;

				bool										TrimTrailingWhitespace;
				bool										CullEmptyLines;
			public:
				virtual ~AvalonEditObScriptIndentStrategy()
				{
					IndentParser->Reset();
					IndentParser = nullptr;
				}

				virtual void								IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line);
				virtual void								IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine);

				AvalonEditObScriptIndentStrategy(bool TrimTrailingWhitespace, bool CullEmptyLines) : IndentParser(gcnew ScriptParser()), TrimTrailingWhitespace(TrimTrailingWhitespace), CullEmptyLines(CullEmptyLines) {}
			};

			ref class AvalonEditObScriptCodeFoldingStrategy : public AvalonEdit::Folding::AbstractFoldingStrategy
			{
				ref class FoldingSorter : public IComparer<AvalonEdit::Folding::NewFolding^>
				{
				public:
					virtual int								Compare(AvalonEdit::Folding::NewFolding^ X, AvalonEdit::Folding::NewFolding^ Y);
				};

				ScriptParser^								FoldingParser;
				FoldingSorter^								Sorter;
			public:
				virtual ~AvalonEditObScriptCodeFoldingStrategy()
				{
					FoldingParser->Reset();
					FoldingParser = nullptr;
					Sorter = nullptr;
				}

				virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^			CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset) override;

				AvalonEditObScriptCodeFoldingStrategy() : AvalonEdit::Folding::AbstractFoldingStrategy(), FoldingParser(gcnew ScriptParser()), Sorter(gcnew FoldingSorter()) {}
			};

			ref class TagableDoubleAnimation : public System::Windows::Media::Animation::DoubleAnimation
			{
			public:
				property Object^							Tag;

				TagableDoubleAnimation(double fromValue, double toValue, System::Windows::Duration duration, System::Windows::Media::Animation::FillBehavior fillBehavior) :
				DoubleAnimation(fromValue, toValue, duration, fillBehavior) {}
			};

			ref class AvalonEditBraceHighlightingBGColorizer : public AvalonEditLineBackgroundColorizer
			{
				int											OpenBraceOffset;
				int											CloseBraceOffset;
				bool										DoHighlight;
			public:
				virtual void								Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext) override;

				AvalonEditBraceHighlightingBGColorizer(AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer) :
				AvalonEditLineBackgroundColorizer(Parent, RenderLayer), OpenBraceOffset(-1), CloseBraceOffset(-1), DoHighlight(false) {}

				void										SetHighlight(int OpenBraceOffset, int CloseBraceOffset);
				void										ClearHighlight(void);
			};
		}
	}
}