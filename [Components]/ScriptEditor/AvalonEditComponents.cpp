#include "AvalonEditComponents.h"
#include "AvalonEditTextEditor.h"
#include "ScriptEditorPreferences.h"

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		namespace AvalonEditor
		{
			void AvalonEditLineBackgroundColorizer::RenderBackground(TextView^ Destination,
																	System::Windows::Media::DrawingContext^ DrawingContext,
																	int StartOffset,
																	int EndOffset,
																	Windows::Media::Color Background,
																	Windows::Media::Color Border,
																	Double BorderThickness,
																	bool ColorEntireLine )
			{
				Destination->EnsureVisualLines();
				TextSegment^ Segment = gcnew TextSegment();
				Segment->StartOffset = StartOffset;
				Segment->EndOffset = EndOffset;
#if BUILD_AVALONEDIT_VERSION == AVALONEDIT_4_0_0_7070
				for each (Windows::Rect R in BackgroundGeometryBuilder::GetRectsForSegment(Destination, Segment))
#else
				for each (Windows::Rect R in BackgroundGeometryBuilder::GetRectsForSegment(Destination, Segment, false))
#endif
				{
					if (ColorEntireLine)
					{
						DrawingContext->DrawRoundedRectangle(gcnew System::Windows::Media::SolidColorBrush(Background),
							gcnew System::Windows::Media::Pen(gcnew System::Windows::Media::SolidColorBrush(Border), BorderThickness),
							Windows::Rect(R.Location, Windows::Size(Destination->ActualWidth + Destination->HorizontalOffset, R.Height)), 2, 2);
					}
					else
					{
						DrawingContext->DrawRoundedRectangle(gcnew System::Windows::Media::SolidColorBrush(Background),
							gcnew System::Windows::Media::Pen(gcnew System::Windows::Media::SolidColorBrush(Border), BorderThickness),
							Windows::Rect(R.Location, Windows::Size(R.Width, R.Height)), 2, 2);
					}
				}
			}

			AvalonEditLineBackgroundColorizer::~AvalonEditLineBackgroundColorizer()
			{
				ParentEditor = nullptr;
			}

			AvalonEditLineBackgroundColorizer::AvalonEditLineBackgroundColorizer( AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer ) :
				ParentEditor(Parent),
				RenderLayer(RenderLayer)
			{
				;//
			}

			void AvalonEditCurrentLineBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				if (ParentEditor->TextArea->Selection->IsEmpty)
				{
					DocumentLine^ Line = ParentEditor->Document->GetLineByNumber(ParentEditor->TextArea->Caret->Line);
					Color Buffer = PREFERENCES->LookupColorByKey("CurrentLineHighlightColor");
					RenderBackground(textView,
									drawingContext,
									Line->Offset,
									Line->EndOffset,
									Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
									Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
									1,
									true);
				}
			}

			AvalonEditCurrentLineBGColorizer::AvalonEditCurrentLineBGColorizer( AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer ) :
				AvalonEditLineBackgroundColorizer(Parent, RenderLayer)
			{
				;//
			}

			bool AvalonEditScriptErrorBGColorizer::GetLineInError(int Line)
			{
				for each (int Itr in ErrorLines)
				{
					if (Itr == Line)
						return true;
				}
				return false;
			}

			void AvalonEditScriptErrorBGColorizer::AddLine(int Line)
			{
				if (GetLineInError(Line) == false)
					ErrorLines->Add(Line);
			}

			void AvalonEditScriptErrorBGColorizer::ClearLines()
			{
				ErrorLines->Clear();
			}

			void AvalonEditScriptErrorBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				for (int i = 1; i <= ParentEditor->LineCount; i++)
				{
					if (GetLineInError(i))
					{
						DocumentLine^ Line = ParentEditor->Document->GetLineByNumber(i);
						Color Buffer = PREFERENCES->LookupColorByKey("ErrorHighlightColor");
						RenderBackground(textView,
										drawingContext,
										Line->Offset,
										Line->EndOffset,
										Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
										Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
										1,
										true);
					}
				}
			}

			AvalonEditScriptErrorBGColorizer::~AvalonEditScriptErrorBGColorizer()
			{
				ClearLines();
			}

			void AvalonEditSelectionBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				TextDocument^ CurrentDocument = ParentEditor->Document;
				Selection^ CurrentSelection = ParentEditor->TextArea->Selection;

				if (!CurrentSelection->IsEmpty)
				{
#if VERSION_4_0_0_7070
					String^ SelectionText = CurrentSelection->GetText(CurrentDocument)->Replace("\t", "")->Replace(" ", "")->Replace("\n", "")->Replace("\r\n", "");
#else
					String^ SelectionText = CurrentSelection->GetText()->Replace("\t", "")->Replace(" ", "")->Replace("\n", "")->Replace("\r\n", "");
#endif
					if (SelectionText->Length > 4)
					{
						Color Buffer = PREFERENCES->LookupColorByKey("SelectionHighlightColor");

						for each (DocumentLine^ Line in ParentEditor->Document->Lines)
						{
							String^ CurrentLine = ParentEditor->Document->GetText(Line);

							int Index = 0, Start = 0;
							while ((Index = CurrentLine->IndexOf(SelectionText, Start, System::StringComparison::CurrentCultureIgnoreCase)) != -1)
							{
								int EndIndex = Index + SelectionText->Length;
								RenderBackground(textView,
												drawingContext,
												Line->Offset + Index,
												Line->Offset + EndIndex,
												Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
												Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
												1,
												false);

								Start = Index + 1;
							}
						}
					}
				}
			}

			AvalonEditSelectionBGColorizer::AvalonEditSelectionBGColorizer( AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer ) :
				AvalonEditLineBackgroundColorizer(Parent, RenderLayer)
			{
				;//
			}

			void AvalonEditLineLimitBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				Color Buffer = PREFERENCES->LookupColorByKey("CharLimitHighlightColor");

				for each (DocumentLine^ Line in ParentEditor->Document->Lines)
				{
					String^ CurrentLine = ParentEditor->Document->GetText(Line);

					if (CurrentLine->Length > 512)
					{
						RenderBackground(textView,
										drawingContext,
										Line->Offset,
										Line->EndOffset,
										Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
										Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
										1,
										true);
					}
				}
			}

			AvalonEditLineLimitBGColorizer::AvalonEditLineLimitBGColorizer( AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer ) :
				AvalonEditLineBackgroundColorizer(Parent, RenderLayer)
			{
				;//
			}

			void AvalonEditFindReplaceBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				Color Buffer = PREFERENCES->LookupColorByKey("FindResultsHighlightColor");
				List<Segment>^ SegmentBuffer = gcnew List<Segment>();

				for each (Segment Itr in HighlightSegments)
				{
					try
					{
						RenderBackground(textView,
										drawingContext,
										Itr.Offset,
										Itr.Offset + Itr.Length,
										Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
										Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
										1,
										false);

						SegmentBuffer->Add(Segment(Itr));
					} catch (...) {}
				}

				HighlightSegments->Clear();

				for each (Segment Itr in SegmentBuffer)
					AddSegment(Itr.Offset, Itr.Length);

				SegmentBuffer->Clear();
			}

			void AvalonEditFindReplaceBGColorizer::AddSegment( int Offset, int Length )
			{
				HighlightSegments->Add(Segment(Offset, Length));
			}

			void AvalonEditFindReplaceBGColorizer::ClearSegments()
			{
				HighlightSegments->Clear();
			}

			AvalonEditFindReplaceBGColorizer::Segment::Segment( int Offset, int Length ) :
				Offset(Offset), Length(Length)
			{
				;//
			}

			AvalonEditFindReplaceBGColorizer::AvalonEditFindReplaceBGColorizer( AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer ) :
				AvalonEditLineBackgroundColorizer(Parent, RenderLayer),
				HighlightSegments(gcnew List<Segment>())
			{
				;//
			}

			AvalonEditFindReplaceBGColorizer::~AvalonEditFindReplaceBGColorizer()
			{
				ClearSegments();
			}

			void AvalonEditObScriptIndentStrategy::IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine)
			{
				;//
			}

			void AvalonEditObScriptIndentStrategy::IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line)
			{
				ObScriptSemanticAnalysis::AnalysisData^ Data = Parent->GetSemanticAnalysisCache(false, true);
				UInt32 CurrIndent = Data->GetLineIndentLevel(line->LineNumber);

				AvalonEdit::Document::DocumentLine^ previousLine = line->PreviousLine;
				ScriptParser^ Parser = gcnew ScriptParser();

				Parser->Tokenize(document->GetText(previousLine), false);
				if (CullEmptyLines)
				{
					if (!Parser->Valid && document->GetText(previousLine)->Replace("\t", "")->Length == 0)
					{
						AvalonEdit::Document::ISegment^ Leading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(document, previousLine);
						document->Replace(Leading, "");
					}
				}

				if (Parser->Valid)
				{
					AvalonEdit::Document::ISegment^ Leading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(document, previousLine);

					if (Parser->GetFirstTokenType() == ObScriptSemanticAnalysis::ScriptTokenType::ElseIf ||
						Parser->GetFirstTokenType() == ObScriptSemanticAnalysis::ScriptTokenType::Else)
					{
						if (CurrIndent)
							document->Replace(Leading, gcnew String('\t', CurrIndent - 1));
					}
					else if	(Parser->GetFirstTokenType() == ObScriptSemanticAnalysis::ScriptTokenType::EndIf ||
							Parser->GetFirstTokenType() == ObScriptSemanticAnalysis::ScriptTokenType::End ||
							Parser->GetFirstTokenType() == ObScriptSemanticAnalysis::ScriptTokenType::Loop)
					{
						document->Replace(Leading, gcnew String('\t', CurrIndent));
					}
				}

				AvalonEdit::Document::ISegment^ Indentation = AvalonEdit::Document::TextUtilities::GetWhitespaceAfter(document, line->Offset);
				document->Replace(Indentation, gcnew String('\t', CurrIndent));
				document->Replace(AvalonEdit::Document::TextUtilities::GetWhitespaceBefore(document, line->Offset), "");
			}

			AvalonEditObScriptIndentStrategy::~AvalonEditObScriptIndentStrategy()
			{
				;//
			}

			AvalonEditObScriptIndentStrategy::AvalonEditObScriptIndentStrategy( AvalonEditTextEditor^ Parent, bool TrimTrailingWhitespace, bool CullEmptyLines ) :
				Parent(Parent),
				TrimTrailingWhitespace(TrimTrailingWhitespace),
				CullEmptyLines(CullEmptyLines)
			{
				;//
			}

			int AvalonEditObScriptCodeFoldingStrategy::FoldingSorter::Compare( AvalonEdit::Folding::NewFolding^ X, AvalonEdit::Folding::NewFolding^ Y )
			{
				return X->StartOffset.CompareTo(Y->StartOffset);
			}

			IEnumerable<AvalonEdit::Folding::NewFolding^>^ AvalonEditObScriptCodeFoldingStrategy::CreateNewFoldings( AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset )
			{
				firstErrorOffset = -1;

				List<AvalonEdit::Folding::NewFolding^>^ Foldings = gcnew List<AvalonEdit::Folding::NewFolding^>();

				ObScriptSemanticAnalysis::AnalysisData^ Data = Parent->GetSemanticAnalysisCache(false, false);
				for each (ObScriptSemanticAnalysis::ControlBlock^ Itr in Data->ControlBlocks)
				{
					if (Itr->IsMalformed() == false &&
						Itr->StartLine <= document->LineCount && Itr->StartLine > 0 &&
						Itr->EndLine <= document->LineCount && Itr->EndLine > 0)
					{
						AvalonEdit::Document::DocumentLine^ StartLine = document->GetLineByNumber(Itr->StartLine);
						AvalonEdit::Document::DocumentLine^ EndLine = document->GetLineByNumber(Itr->EndLine);

						Foldings->Add(gcnew AvalonEdit::Folding::NewFolding(StartLine->EndOffset, EndLine->Offset - 1));
					}
				}
				if (Data->MalformedStructure && Data->FirstStructuralErrorLine <= document->LineCount && Data->FirstStructuralErrorLine > 0)
				{
					AvalonEdit::Document::DocumentLine^ ErrorLine = document->GetLineByNumber(Data->FirstStructuralErrorLine);
					firstErrorOffset = ErrorLine->Offset;
				}

				if (Foldings->Count)
					Foldings->Sort(Sorter);

				return Foldings;
			}

			AvalonEditObScriptCodeFoldingStrategy::~AvalonEditObScriptCodeFoldingStrategy()
			{
			}

			AvalonEditObScriptCodeFoldingStrategy::AvalonEditObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent) :
#if BUILD_AVALONEDIT_VERSION != AVALONEDIT_5_0_1
				AvalonEdit::Folding::AbstractFoldingStrategy(),
#endif
				Parent(Parent),
				Sorter(gcnew FoldingSorter())
			{
				;//
			}

			void AvalonEditBraceHighlightingBGColorizer::Draw( TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext )
			{
				if (DoHighlight == false || (OpenBraceOffset == -1 && CloseBraceOffset == -1))
					return;

				textView->EnsureVisualLines();

				Color ValidBraceColor = Color::LightSlateGray;
				Color InvalidBraceColor = Color::MediumVioletRed;

				BackgroundGeometryBuilder^ Builder = gcnew BackgroundGeometryBuilder();

				Builder->CornerRadius = 1;
				Builder->AlignToMiddleOfPixels = true;

				if (OpenBraceOffset != -1)
				{
					TextSegment^ OpenBraceSeg = gcnew TextSegment();
					OpenBraceSeg->StartOffset = OpenBraceOffset;
					OpenBraceSeg->Length = 1;
					Builder->AddSegment(textView, OpenBraceSeg);
					Builder->CloseFigure();						 // prevent connecting the two segments
				}

				if (CloseBraceOffset != -1)
				{
					TextSegment^ CloseBraceSeg = gcnew TextSegment();
					CloseBraceSeg->StartOffset = CloseBraceOffset;
					CloseBraceSeg->Length = 1;
					Builder->AddSegment(textView, CloseBraceSeg);
				}

				System::Windows::Media::Geometry^ HighlightGeometry = Builder->CreateGeometry();
				if (HighlightGeometry != nullptr)
				{
					if (OpenBraceOffset == -1 || CloseBraceOffset == -1)
					{
						drawingContext->DrawGeometry(gcnew System::Windows::Media::SolidColorBrush(System::Windows::Media::Color::FromArgb(125,
																									InvalidBraceColor.R,
																									InvalidBraceColor.G,
																									InvalidBraceColor.B)),
													gcnew System::Windows::Media::Pen(gcnew System::Windows::Media::SolidColorBrush(
															System::Windows::Media::Color::FromArgb(150,
																									0,
																									0,
																									0)),
																					0),
													HighlightGeometry);
					}
					else
					{
						drawingContext->DrawGeometry(gcnew System::Windows::Media::SolidColorBrush(System::Windows::Media::Color::FromArgb(125,
																									ValidBraceColor.R,
																									ValidBraceColor.G,
																									ValidBraceColor.B)),
													gcnew System::Windows::Media::Pen(gcnew System::Windows::Media::SolidColorBrush(
															System::Windows::Media::Color::FromArgb(150,
																									0,
																									0,
																									0)),
																					0),
													HighlightGeometry);
					}
				}
			}

			void AvalonEditBraceHighlightingBGColorizer::SetHighlight( int OpenBraceOffset, int CloseBraceOffset )
			{
				this->OpenBraceOffset = OpenBraceOffset;
				this->CloseBraceOffset = CloseBraceOffset;
				this->DoHighlight = true;
			}

			void AvalonEditBraceHighlightingBGColorizer::ClearHighlight( void )
			{
				this->DoHighlight = false;
			}

			AvalonEditBraceHighlightingBGColorizer::AvalonEditBraceHighlightingBGColorizer( AvalonEdit::TextEditor^% Parent, KnownLayer RenderLayer ) :
				AvalonEditLineBackgroundColorizer(Parent, RenderLayer),
				OpenBraceOffset(-1),
				CloseBraceOffset(-1),
				DoHighlight(false)
			{
				;//
			}

			TagableDoubleAnimation::TagableDoubleAnimation( double fromValue,
															double toValue,
															System::Windows::Duration duration,
															System::Windows::Media::Animation::FillBehavior fillBehavior ) :
				DoubleAnimation(fromValue, toValue, duration, fillBehavior)
			{
				;//
			}
		}
	}
}