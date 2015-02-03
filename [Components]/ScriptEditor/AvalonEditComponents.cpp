#include "AvalonEditComponents.h"
#include "AvalonEditTextEditor.h"
#include "ScriptEditorPreferences.h"

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		namespace AvalonEditor
		{
			void ILineBackgroundColorizer::RenderBackground(TextView^ Destination,
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

			ILineBackgroundColorizer::~ILineBackgroundColorizer()
			{
				ParentEditor = nullptr;
			}

			ILineBackgroundColorizer::ILineBackgroundColorizer( AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer ) :
				ParentEditor(Parent),
				RenderLayer(RenderLayer)
			{
				;//
			}

			void CurrentLineBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
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

			CurrentLineBGColorizer::CurrentLineBGColorizer( AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer ) :
				ILineBackgroundColorizer(Parent, RenderLayer)
			{
				;//
			}

			bool ScriptErrorBGColorizer::GetLineInError(int Line)
			{
				for each (int Itr in ErrorLines)
				{
					if (Itr == Line)
						return true;
				}
				return false;
			}

			void ScriptErrorBGColorizer::AddLine(int Line)
			{
				if (GetLineInError(Line) == false)
					ErrorLines->Add(Line);
			}

			void ScriptErrorBGColorizer::ClearLines()
			{
				ErrorLines->Clear();
			}

			void ScriptErrorBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
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

			ScriptErrorBGColorizer::~ScriptErrorBGColorizer()
			{
				ClearLines();
			}

			void SelectionBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
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

			SelectionBGColorizer::SelectionBGColorizer( AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer ) :
				ILineBackgroundColorizer(Parent, RenderLayer)
			{
				;//
			}

			void LineLimitBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
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

			LineLimitBGColorizer::LineLimitBGColorizer( AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer ) :
				ILineBackgroundColorizer(Parent, RenderLayer)
			{
				;//
			}

			void FindReplaceBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
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

			void FindReplaceBGColorizer::AddSegment( int Offset, int Length )
			{
				HighlightSegments->Add(Segment(Offset, Length));
			}

			void FindReplaceBGColorizer::ClearSegments()
			{
				HighlightSegments->Clear();
			}

			FindReplaceBGColorizer::Segment::Segment( int Offset, int Length ) :
				Offset(Offset), Length(Length)
			{
				;//
			}

			FindReplaceBGColorizer::FindReplaceBGColorizer( AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer ) :
				ILineBackgroundColorizer(Parent, RenderLayer),
				HighlightSegments(gcnew List<Segment>())
			{
				;//
			}

			FindReplaceBGColorizer::~FindReplaceBGColorizer()
			{
				ClearSegments();
			}

			void ObScriptIndentStrategy::IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine)
			{
				;//
			}

			void ObScriptIndentStrategy::IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line)
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

			ObScriptIndentStrategy::~ObScriptIndentStrategy()
			{
				;//
			}

			ObScriptIndentStrategy::ObScriptIndentStrategy( AvalonEditTextEditor^ Parent, bool TrimTrailingWhitespace, bool CullEmptyLines ) :
				Parent(Parent),
				TrimTrailingWhitespace(TrimTrailingWhitespace),
				CullEmptyLines(CullEmptyLines)
			{
				;//
			}

			int ObScriptCodeFoldingStrategy::FoldingSorter::Compare( AvalonEdit::Folding::NewFolding^ X, AvalonEdit::Folding::NewFolding^ Y )
			{
				return X->StartOffset.CompareTo(Y->StartOffset);
			}

			IEnumerable<AvalonEdit::Folding::NewFolding^>^ ObScriptCodeFoldingStrategy::CreateNewFoldings( AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset )
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

			ObScriptCodeFoldingStrategy::~ObScriptCodeFoldingStrategy()
			{
			}

			ObScriptCodeFoldingStrategy::ObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent) :
#if BUILD_AVALONEDIT_VERSION != AVALONEDIT_5_0_1
				AvalonEdit::Folding::AbstractFoldingStrategy(),
#endif
				Parent(Parent),
				Sorter(gcnew FoldingSorter())
			{
				;//
			}

			void BraceHighlightingBGColorizer::Draw( TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext )
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

			void BraceHighlightingBGColorizer::SetHighlight( int OpenBraceOffset, int CloseBraceOffset )
			{
				this->OpenBraceOffset = OpenBraceOffset;
				this->CloseBraceOffset = CloseBraceOffset;
				this->DoHighlight = true;
			}

			void BraceHighlightingBGColorizer::ClearHighlight( void )
			{
				this->DoHighlight = false;
			}

			BraceHighlightingBGColorizer::BraceHighlightingBGColorizer( AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer ) :
				ILineBackgroundColorizer(Parent, RenderLayer),
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

			StructureVisualizerRenderer::StructureVisualizerRenderer(AvalonEditTextEditor^ Parent) :
				VisualLineElementGenerator(),
				ParentEditor(Parent)
			{
				;//
			}

			int StructureVisualizerRenderer::GetFirstInterestedOffset(Int32 startOffset)
			{
				DocumentLine^ CurrentLine = CurrentContext->Document->GetLineByOffset(startOffset);

				if (ParentEditor->GetSemanticAnalysisCache(false, false)->GetBlockEndingAt(CurrentLine->LineNumber))
					return startOffset + CurrentLine->Length;
				else
					return -1;
			}

			VisualLineElement^ StructureVisualizerRenderer::ConstructElement(Int32 offset)
			{
				DocumentLine^ CurrentLine = CurrentContext->Document->GetLineByOffset(offset);
				ObScriptSemanticAnalysis::ControlBlock^ Block = ParentEditor->GetSemanticAnalysisCache(false, false)->GetBlockEndingAt(CurrentLine->LineNumber);

				if (Block && Block->BasicBlock)
				{
				//	if (ParentEditor->GetLineVisible(Block->StartLine))
				//	if (ParentEditor->GetCurrentLineNumber() != CurrentLine->LineNumber && ParentEditor->GetInSelection(offset) == false)
					{
						DocumentLine^ BlockStart = CurrentContext->Document->GetLineByNumber(Block->StartLine);
						if (BlockStart)
						{
							Color ForegroundColor = PREFERENCES->LookupColorByKey("ForegroundColor");
							Color BackgroundColor = PREFERENCES->LookupColorByKey("BackgroundColor");
							Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"),
														  PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"),
														  (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));
							Windows::Media::Brush^ ForegroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(100, ForegroundColor.R, ForegroundColor.G, ForegroundColor.B));
							Windows::Media::Brush^ BackgroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B));

							String^ StartText = CurrentContext->Document->GetText(BlockStart)->Replace("\t", "");

							Windows::Controls::Label^ UIElement = gcnew Windows::Controls::Label();
							UIElement->FontFamily = gcnew Windows::Media::FontFamily(CustomFont->FontFamily->Name);
							UIElement->FontSize = CustomFont->Size;
							UIElement->Foreground = ForegroundBrush;
							UIElement->Background = BackgroundBrush;
							UIElement->Content = StartText;
							UIElement->Padding = Windows::Thickness(0, 0, 0, 0);
							UIElement->Margin = Windows::Thickness(10, 0, 0, 0);
							UIElement->HorizontalAlignment = Windows::HorizontalAlignment::Left;
							UIElement->VerticalAlignment = Windows::VerticalAlignment::Top;

							return gcnew InlineObjectElement(0, UIElement);
						}
					}
				}

				return nullptr;
			}
		}
	}
}