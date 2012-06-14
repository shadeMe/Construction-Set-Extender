#include "AvalonEditComponents.h"
#include "ScriptEditorPreferences.h"

namespace ConstructionSetExtender
{
	namespace TextEditors
	{
		namespace AvalonEditor
		{
			void AvalonEditSelectionColorizingTransformer::PerformColorization(VisualLineElement^ Element)
			{
				Color Buffer = PREFERENCES->LookupColorByKey("SelectionHighlightColor");
				Windows::Media::SolidColorBrush^ Brush = gcnew Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, Buffer.R, Buffer.G, Buffer.B));
				Brush->Opacity = 0.5;

				Element->TextRunProperties->SetBackgroundBrush(Brush);
			}

			void AvalonEditSelectionColorizingTransformer::ColorizeLine(DocumentLine^ line)
			{
				TextDocument^ CurrentDocument = CurrentContext->Document;
				Selection^ CurrentSelection = this->ParentEditor->TextArea->Selection;

				if (!CurrentSelection->IsEmpty)
				{
					String^ SelectionText = CurrentSelection->GetText()->Replace("\t", "")->Replace(" ", "")->Replace("\n", "")->Replace("\r\n", "");
					String^ CurrentLine = CurrentDocument->GetText(line);

					int Index = 0, Start = 0;
					if (SelectionText->Length > 4)
					{
						while ((Index = CurrentLine->IndexOf(SelectionText, Start)) != -1)
						{
							int EndIndex = Index + SelectionText->Length;
							Action<VisualLineElement^>^ Operation = gcnew Action<VisualLineElement^>(this, &AvalonEditSelectionColorizingTransformer::PerformColorization);

							ChangeLinePart(line->Offset + Index, line->Offset + EndIndex, Operation);
							Start = Index + 1;
						}
					}
				}
			}

			void AvalonEditLineLimitColorizingTransformer::PerformColorization(VisualLineElement^ Element)
			{
				Color Buffer = PREFERENCES->LookupColorByKey("CharLimitHighlightColor");
				Windows::Media::SolidColorBrush^ Brush = gcnew Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, Buffer.R, Buffer.G, Buffer.B));
				Brush->Opacity = 0.4;

				Element->TextRunProperties->SetBackgroundBrush(Brush);
			}

			void AvalonEditLineLimitColorizingTransformer::ColorizeLine(DocumentLine^ line)
			{
				TextDocument^ CurrentDocument = CurrentContext->Document;

				String^ CurrentLine = CurrentDocument->GetText(line);
				if (CurrentDocument->GetText(line)->Length > 512)
				{
					Action<VisualLineElement^>^ Operation = gcnew Action<VisualLineElement^>(this, &AvalonEditLineLimitColorizingTransformer::PerformColorization);
					ChangeLinePart(line->Offset, line->EndOffset, Operation);
				}
			}

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

				for each (Windows::Rect R in BackgroundGeometryBuilder::GetRectsForSegment(Destination, Segment, false))
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

			void AvalonEditSelectionBGColorizer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				TextDocument^ CurrentDocument = ParentEditor->Document;
				Selection^ CurrentSelection = ParentEditor->TextArea->Selection;

				if (!CurrentSelection->IsEmpty)
				{
					String^ SelectionText = CurrentSelection->GetText()->Replace("\t", "")->Replace(" ", "")->Replace("\n", "")->Replace("\r\n", "");
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
			}

			void AvalonEditFindReplaceBGColorizer::AddSegment( int Offset, int Length )
			{
				HighlightSegments->Add(Segment(Offset, Length));
			}

			void AvalonEditFindReplaceBGColorizer::ClearSegments()
			{
				HighlightSegments->Clear();
			}

			void AvalonEditObScriptIndentStrategy::IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine)
			{
				return;
			}

			void AvalonEditObScriptIndentStrategy::IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line)
			{
				int IndentCount = 0;
				bool ExdentLastLine = false;
				bool SemiExdentLastLine = false;

				IndentParser->Reset();
				IndentParser->BlockStack->Push(ScriptParser::BlockType::e_Invalid);

				for each (AvalonEdit::Document::DocumentLine^ Line in document->Lines)
				{
					if (Line->LineNumber == line->LineNumber)
						break;

					IndentParser->Tokenize(document->GetText(Line), false);

					if (!IndentParser->Valid)
						continue;

					if (!String::Compare(IndentParser->Tokens[0], "begin", true) && IndentParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						IndentParser->BlockStack->Push(ScriptParser::BlockType::e_ScriptBlock);
					else if (!String::Compare(IndentParser->Tokens[0], "if", true) && !IndentParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						IndentParser->BlockStack->Push(ScriptParser::BlockType::e_If);
					else if (!String::Compare(IndentParser->Tokens[0], "foreach", true) && !IndentParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						IndentParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
					else if (!String::Compare(IndentParser->Tokens[0], "while", true) && !IndentParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						IndentParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
					else if	(!String::Compare(IndentParser->Tokens[0], "loop", true) && IndentParser->CompareBlockStack(ScriptParser::BlockType::e_Loop))
					{
						IndentParser->BlockStack->Pop();
						if (Line->NextLine->LineNumber == line->LineNumber)
							ExdentLastLine = true;
					}
					else if (!String::Compare(IndentParser->Tokens[0], "endIf", true) && IndentParser->CompareBlockStack(ScriptParser::BlockType::e_If))
					{
						IndentParser->BlockStack->Pop();
						if (Line->NextLine->LineNumber == line->LineNumber)
							ExdentLastLine = true;
					}
					else if (!String::Compare(IndentParser->Tokens[0], "end", true) && IndentParser->CompareBlockStack(ScriptParser::BlockType::e_ScriptBlock))
					{
						IndentParser->BlockStack->Pop();
						if (Line->NextLine->LineNumber == line->LineNumber)
							ExdentLastLine = true;
					}
					else if (!String::Compare(IndentParser->Tokens[0], "elseIf", true) || !String::Compare(IndentParser->Tokens[0], "else", true))
					{
						if (IndentParser->CompareBlockStack(ScriptParser::BlockType::e_If) && Line->NextLine->LineNumber == line->LineNumber)
						{
							ExdentLastLine = true;
							SemiExdentLastLine = true;
						}
					}
				}

				IndentCount = IndentParser->BlockStack->Count - 1;

				if (CullEmptyLines)
				{
					IndentParser->Tokenize(document->GetText(line->PreviousLine), false);
					if (!IndentParser->Valid && document->GetText(line->PreviousLine)->Replace("\t", "")->Length == 0)
					{
						AvalonEdit::Document::ISegment^ Leading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(document, line->PreviousLine);
						document->Replace(Leading, "");
					}
				}

				if (ExdentLastLine)
				{
					AvalonEdit::Document::ISegment^ Leading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(document, line->PreviousLine);
					int ExdentedCount = 0;

					if (SemiExdentLastLine)
						ExdentedCount = IndentCount - 1;
					else
						ExdentedCount = IndentCount;

					String^ IndentString = "";
					for (int i = 0; i < ExdentedCount; i++)
						IndentString += "\t";

					document->Replace(Leading, IndentString);
				}

				if (TrimTrailingWhitespace)
				{
					AvalonEdit::Document::ISegment^ Trailing = AvalonEdit::Document::TextUtilities::GetTrailingWhitespace(document, line->PreviousLine);
					document->Replace(Trailing, "");
				}

				String^ IndentString = "";
				for (int i = 0; i < IndentCount; i++)
					IndentString += "\t";

				AvalonEdit::Document::ISegment^ Indentation = AvalonEdit::Document::TextUtilities::GetWhitespaceAfter(document, line->Offset);
				document->Replace(Indentation, IndentString);
				document->Replace(AvalonEdit::Document::TextUtilities::GetWhitespaceBefore(document, line->Offset), "");
			}

			int AvalonEditObScriptCodeFoldingStrategy::FoldingSorter::Compare( AvalonEdit::Folding::NewFolding^ X, AvalonEdit::Folding::NewFolding^ Y )
			{
				return X->StartOffset.CompareTo(Y->StartOffset);
			}

			IEnumerable<AvalonEdit::Folding::NewFolding^>^ AvalonEditObScriptCodeFoldingStrategy::CreateNewFoldings( AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset )
			{
				firstErrorOffset = -1;
				FoldingParser->Reset();
				FoldingParser->BlockStack->Push(ScriptParser::BlockType::e_Invalid);

				List<AvalonEdit::Folding::NewFolding^>^ Foldings = gcnew List<AvalonEdit::Folding::NewFolding^>();
				Stack<int>^ StartOffsets = gcnew Stack<int>();

				try			// yeah, ugly
				{
					for each (AvalonEdit::Document::DocumentLine^ Line in document->Lines)
					{
						FoldingParser->Tokenize(document->GetText(Line), false);

						if (!FoldingParser->Valid)
							continue;

						if (!String::Compare(FoldingParser->Tokens[0], "begin", true) && FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						{
							FoldingParser->BlockStack->Push(ScriptParser::BlockType::e_ScriptBlock);
							StartOffsets->Push(Line->EndOffset);
						}
						else if (!String::Compare(FoldingParser->Tokens[0], "if", true) && !FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						{
							FoldingParser->BlockStack->Push(ScriptParser::BlockType::e_If);
							StartOffsets->Push(Line->EndOffset);
						}
						else if (!String::Compare(FoldingParser->Tokens[0], "foreach", true) && !FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						{
							FoldingParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
							StartOffsets->Push(Line->EndOffset);
						}
						else if (!String::Compare(FoldingParser->Tokens[0], "while", true) && !FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_Invalid))
						{
							FoldingParser->BlockStack->Push(ScriptParser::BlockType::e_Loop);
							StartOffsets->Push(Line->EndOffset);
						}
						else if	(!String::Compare(FoldingParser->Tokens[0], "loop", true) && FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_Loop))
						{
							FoldingParser->BlockStack->Pop();
							Foldings->Add(gcnew AvalonEdit::Folding::NewFolding(StartOffsets->Pop(), Line->EndOffset));
						}
						else if (!String::Compare(FoldingParser->Tokens[0], "endIf", true) &&
							(FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_If) ||
							FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_IfElse)))
						{
							FoldingParser->BlockStack->Pop();
							Foldings->Add(gcnew AvalonEdit::Folding::NewFolding(StartOffsets->Pop(), Line->EndOffset));
						}
						else if (!String::Compare(FoldingParser->Tokens[0], "end", true) && FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_ScriptBlock))
						{
							FoldingParser->BlockStack->Pop();
							Foldings->Add(gcnew AvalonEdit::Folding::NewFolding(StartOffsets->Pop(), Line->EndOffset));
						}
						else if ((!String::Compare(FoldingParser->Tokens[0], "elseIf", true) || !String::Compare(FoldingParser->Tokens[0], "else", true)) &&
							(FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_If) ||
							FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_IfElse)))
						{
							if (FoldingParser->CompareBlockStack(ScriptParser::BlockType::e_IfElse))
							{
								Foldings->Add(gcnew AvalonEdit::Folding::NewFolding(StartOffsets->Pop(), Line->PreviousLine->EndOffset));
								StartOffsets->Push(Line->EndOffset);
							}
							else
							{
								FoldingParser->BlockStack->Pop();		// pop IF
								Foldings->Add(gcnew AvalonEdit::Folding::NewFolding(StartOffsets->Pop(), Line->PreviousLine->EndOffset));
								FoldingParser->BlockStack->Push(ScriptParser::BlockType::e_IfElse);
								StartOffsets->Push(Line->EndOffset);
							}
						}
					}
				}
				catch (...)
				{
					if (StartOffsets->Count)
						firstErrorOffset = StartOffsets->Pop();
				}

				if (Foldings->Count)
					Foldings->Sort(Sorter);

				return Foldings;
			}

			void AvalonEditBraceHighlightingBGColorizer::Draw( TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext )
			{
				if (DoHighlight == false || (OpenBraceOffset == -1 && CloseBraceOffset == -1))
					return;

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
		}
	}
}