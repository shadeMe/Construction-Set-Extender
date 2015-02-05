#include "AvalonEditComponents.h"
#include "AvalonEditTextEditor.h"
#include "ScriptEditorPreferences.h"
#include "Globals.h"

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
				Color Buffer = PREFERENCES->LookupColorByKey("ErrorHighlightColor");
				ScriptParser^ Parser = gcnew ScriptParser();

				for (int i = 1; i <= ParentEditor->LineCount; i++)
				{
					if (GetLineInError(i))
					{
						DocumentLine^ Line = ParentEditor->Document->GetLineByNumber(i);
						if (Parser->Tokenize(ParentEditor->Document->GetText(Line), false))
						{
							RenderSquiggly(textView, drawingContext,
										   Line->Offset + Parser->Indices[0], Line->EndOffset,
										   Windows::Media::Color::FromArgb(255, Buffer.R, Buffer.G, Buffer.B));
						}
					}
				}
			}

			ScriptErrorBGColorizer::~ScriptErrorBGColorizer()
			{
				ClearLines();
			}

			void ScriptErrorBGColorizer::RenderSquiggly(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, Windows::Media::Color Color)
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
					Windows::Point StartPoint = R.BottomLeft;
					Windows::Point EndPoint = R.BottomRight;
					Windows::Media::SolidColorBrush^ Brush = gcnew Windows::Media::SolidColorBrush(Color);
					Brush->Freeze();

					double Offset = 2.5;
					int Count = Math::Max((int)((EndPoint.X - StartPoint.X) / Offset) + 1, 4);
					Windows::Media::StreamGeometry^ Geometry = gcnew Windows::Media::StreamGeometry;
					Windows::Media::StreamGeometryContext^ Context = Geometry->Open();
					List<Windows::Point>^ Points = gcnew List < Windows::Point >;

					for (int i = 0; i < Count; i++)
						Points->Add(Windows::Point(StartPoint.X + i * Offset, StartPoint.Y - ((i + 1) % 2 == 0 ? Offset : 0)));

					Context->BeginFigure(StartPoint, false, false);
					Context->PolyLineTo(Points, true, false);
					delete Context;

					Geometry->Freeze();

					Windows::Media::Pen^ Pen = gcnew System::Windows::Media::Pen(Brush, 1);
					Pen->Freeze();
					DrawingContext->DrawGeometry(Windows::Media::Brushes::Transparent, Pen, Geometry);
				}
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
				ParentEditor(Parent),
				IconSource(nullptr)
			{
				Drawing::Bitmap^ OrgResource = (Drawing::Bitmap^)Globals::ScriptEditorImageResourceManager->CreateImageFromResource("AvalonEditStructureVisualizer");

				try
				{
					IconSource = System::Windows::Interop::Imaging::CreateBitmapSourceFromHBitmap(OrgResource->GetHbitmap(),
																								  IntPtr::Zero,
																								  Windows::Int32Rect::Empty,
																								  Windows::Media::Imaging::BitmapSizeOptions::FromEmptyOptions());
				}
				catch (...)
				{
					IconSource = nullptr;
				}
				finally
				{
					NativeWrapper::DeleteObject(OrgResource->GetHbitmap());
				}

				delete OrgResource;
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
							return gcnew InlineObjectElement(0, GenerateAdornment(Block->StartLine, CurrentContext->Document->GetText(BlockStart)));
						}
					}
				}

				return nullptr;
			}

			void StructureVisualizerRenderer::OnMouseClick(Object^ Sender, Windows::Input::MouseButtonEventArgs^ E)
			{
				E->Handled = true;
				AdornmentData^ Data = (AdornmentData^)((Windows::Controls::StackPanel^)Sender)->Tag;
				if (Data)
				{
					Data->Parent->ScrollToLine(Data->JumpLine);
				}
			}

			Windows::UIElement^ StructureVisualizerRenderer::GenerateAdornment(UInt32 JumpLine, String^ ElementText)
			{
				Color ForegroundColor = PREFERENCES->LookupColorByKey("ForegroundColor");
				Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"),
											  PREFERENCES->FetchSettingAsInt("FontSize", "Appearance"),
											  (FontStyle)PREFERENCES->FetchSettingAsInt("FontStyle", "Appearance"));

				Windows::Media::Brush^ ForegroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(100, ForegroundColor.R, ForegroundColor.G, ForegroundColor.B));
				Windows::Media::Brush^ BackgroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(0, 0, 0, 0));

				ElementText = ElementText->Replace("\t", "");

				AdornmentData^ Data = gcnew AdornmentData;
				Data->JumpLine = JumpLine;
				Data->Parent = ParentEditor;

				Windows::Controls::StackPanel^ Panel = gcnew Windows::Controls::StackPanel();
				Panel->HorizontalAlignment = Windows::HorizontalAlignment::Center;
				Panel->Orientation = Windows::Controls::Orientation::Horizontal;
				Panel->Margin = Windows::Thickness(15, 0, 15, 0);
				Panel->Cursor = Windows::Input::Cursors::Hand;
				Panel->Tag = Data;
				Panel->PreviewMouseDown += gcnew System::Windows::Input::MouseButtonEventHandler(OnMouseClick);

				if (IconSource)
				{
					Windows::Controls::Image^ Icon = gcnew Windows::Controls::Image();
					Icon->Source = IconSource;
					Icon->Width = 12;
					Icon->Height = 12;
					Icon->HorizontalAlignment = Windows::HorizontalAlignment::Center;
					Icon->VerticalAlignment = Windows::VerticalAlignment::Bottom;
					Panel->Children->Add(Icon);
				}

				Windows::Controls::Label^ AdornmentLabel = gcnew Windows::Controls::Label();
				AdornmentLabel->FontFamily = gcnew Windows::Media::FontFamily(CustomFont->FontFamily->Name);
				AdornmentLabel->FontSize = CustomFont->Size;
				AdornmentLabel->Foreground = ForegroundBrush;
				AdornmentLabel->Background = BackgroundBrush;
				AdornmentLabel->Content = ElementText;
				AdornmentLabel->Padding = Windows::Thickness(0, 0, 0, 0);
				AdornmentLabel->Margin = Windows::Thickness(4, 0, 0, 0);
				AdornmentLabel->HorizontalAlignment = Windows::HorizontalAlignment::Center;
				AdornmentLabel->VerticalAlignment = Windows::VerticalAlignment::Bottom;
				Panel->Children->Add(AdornmentLabel);

				return Panel;
			}
		}
	}
}