#include "AvalonEditComponents.h"
#include "AvalonEditTextEditor.h"
#include "Preferences.h"
#include "Globals.h"

namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


namespace avalonEdit
{


System::Windows::Media::Imaging::BitmapSource^ WPFImageResourceGenerator::CreateImageSource(String^ ResourceIdentifier)
{
	Drawing::Bitmap^ OrgResource = safe_cast<Drawing::Bitmap^>(Globals::ImageResources()->CreateImage(ResourceIdentifier));
	System::Windows::Media::Imaging::BitmapSource^ Result = nullptr;

	try
	{
		Result = System::Windows::Interop::Imaging::CreateBitmapSourceFromHBitmap(OrgResource->GetHbitmap(), IntPtr::Zero,
																				  Windows::Int32Rect::Empty,
																				  Windows::Media::Imaging::BitmapSizeOptions::FromEmptyOptions());
	}
	catch (...)
	{
		Result = nullptr;
	}
	finally
	{
		nativeWrapper::DeleteObject(OrgResource->GetHbitmap());
	}

	delete OrgResource;
	return Result;
}

void WPFFocusHelper::DispatcherCallback(Windows::UIElement^ Element)
{
	Element->Focus();
	Windows::Input::Keyboard::Focus(Element);
}

void WPFFocusHelper::ThreadCallback(Object^ Element)
{
	Windows::UIElement^ UIE = (Windows::UIElement^)Element;
	UIE->Dispatcher->Invoke(System::Windows::Threading::DispatcherPriority::ContextIdle, gcnew InvokeDelegate(&DispatcherCallback), UIE);
}

void WPFFocusHelper::Focus(Windows::UIElement^ Element)
{
	System::Threading::ThreadPool::QueueUserWorkItem(gcnew System::Threading::WaitCallback(&ThreadCallback), Element);
}


System::Windows::Point TransformToPixels(double X, double Y)
{
	// get DPI independent coords
	Windows::Point Result(X, Y);
	System::Drawing::Graphics^ g = System::Drawing::Graphics::FromHwnd(IntPtr::Zero);
	Result.X = (g->DpiX / 96) * Result.X;
	Result.Y = (g->DpiY / 96) * Result.Y;
	delete g;

	return Result;
}

System::Windows::Point TransformToPixels(System::Windows::Point In)
{
	return TransformToPixels(In.X, In.Y);
}


void LineTrackingManagerBgRenderer::RenderBackground(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, Windows::Media::Color Background, Windows::Media::Color Border, Double BorderThickness, bool ColorEntireLine)
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

void LineTrackingManagerBgRenderer::RenderSquiggle(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, Windows::Media::Color Color)
{
	Destination->EnsureVisualLines();
	TextSegment^ Segment = gcnew TextSegment();
	Segment->StartOffset = StartOffset;
	Segment->EndOffset = EndOffset;

	for each (Windows::Rect R in BackgroundGeometryBuilder::GetRectsForSegment(Destination, Segment, false))
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

void LineTrackingManagerBgRenderer::DoCurrentLineBackground(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	if (!ParentEditor->TextArea->Selection->IsEmpty)
		return;

	DocumentLine^ Line = ParentEditor->Document->GetLineByNumber(ParentEditor->TextArea->Caret->Line);
	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorCurrentLine;
	RenderBackground(textView,
						drawingContext,
						Line->Offset,
						Line->EndOffset,
						Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
						Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
						1,
						true);
}

void LineTrackingManagerBgRenderer::DoLineLimitBackground(DocumentLine^ Line, TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	if (Line->Length <= 512)
		return;

	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorCharLimit;
	RenderBackground(textView,
						drawingContext,
						Line->Offset,
						Line->EndOffset,
						Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
						Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
						1,
						true);
}

void LineTrackingManagerBgRenderer::DoSelectedStringBackground(String^ SelectionText, DocumentLine^ Line, TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	if (SelectionText->Length <= 1)
		return;

	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorSelection;
	String^ CurrentLine = ParentEditor->Document->GetText(Line);

	int Index = 0, Start = 0;
	while (Start < CurrentLine->Length && (Index = CurrentLine->IndexOf(SelectionText, Start, System::StringComparison::CurrentCultureIgnoreCase)) != -1)
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

		Start = EndIndex + 1;
	}
}

void LineTrackingManagerBgRenderer::DoErrorSquiggles(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	Color Buffer = preferences::SettingsHolder::Get()->Appearance->UnderlineColorError;
	Windows::Media::Color RenderColor = Windows::Media::Color::FromArgb(255, Buffer.R, Buffer.G, Buffer.B);

	for each (auto Itr in ErrorSquiggles)
	{
		if (Itr->Enabled)
			RenderSquiggle(textView, drawingContext, Itr->StartOffset, Itr->EndOffset, RenderColor);
	}
}

void LineTrackingManagerBgRenderer::DoFindResults(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorFindResults;
	for each (auto Itr in FindResults)
	{
		if (!Itr->Enabled)
			continue;

		RenderBackground(textView,
						drawingContext,
						Itr->StartOffset,
						Itr->EndOffset,
						Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
						Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
						1,
						false);
	}
}

void LineTrackingManagerBgRenderer::DoBraceIndicators(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	if (!OpenCloseBraces->Enabled)
		return;
	else if (OpenCloseBraces->StartOffset == -1 && OpenCloseBraces->EndOffset == -1)
		return;

	auto ValidBraceColor = Color::LightSlateGray;
	auto InvalidBraceColor = Color::MediumVioletRed;

	auto Builder = gcnew BackgroundGeometryBuilder();
	Builder->CornerRadius = 1;

	if (OpenCloseBraces->StartOffset != -1)
	{
		TextSegment^ OpenBraceSeg = gcnew TextSegment();
		OpenBraceSeg->StartOffset = OpenCloseBraces->StartOffset;
		OpenBraceSeg->Length = 1;
		Builder->AddSegment(textView, OpenBraceSeg);
		Builder->CloseFigure();						 // prevent connecting the two segments
	}

	if (OpenCloseBraces->EndOffset != -1)
	{
		TextSegment^ CloseBraceSeg = gcnew TextSegment();
		CloseBraceSeg->StartOffset = OpenCloseBraces->EndOffset;
		CloseBraceSeg->Length = 1;
		Builder->AddSegment(textView, CloseBraceSeg);
	}

	auto HighlightGeometry = Builder->CreateGeometry();
	if (HighlightGeometry == nullptr)
		return;

	auto HighlightColor = OpenCloseBraces->StartOffset == -1 || OpenCloseBraces->EndOffset == -1 ? InvalidBraceColor : ValidBraceColor;
	drawingContext->DrawGeometry(gcnew System::Windows::Media::SolidColorBrush(System::Windows::Media::Color::FromArgb(125, InvalidBraceColor.R, InvalidBraceColor.G, InvalidBraceColor.B)),
									gcnew System::Windows::Media::Pen(gcnew System::Windows::Media::SolidColorBrush(System::Windows::Media::Color::FromArgb(150, 0, 0, 0)), 0), HighlightGeometry);
}

LineTrackingManagerBgRenderer::LineTrackingManagerBgRenderer(AvalonEdit::TextEditor^ Parent)
{
	ParentEditor = Parent;

	ErrorSquiggles = gcnew List<BackgroundRenderSegment^>;
	FindResults = gcnew List<BackgroundRenderSegment^>;
	OpenCloseBraces = gcnew BackgroundRenderSegment;
}

LineTrackingManagerBgRenderer::~LineTrackingManagerBgRenderer()
{
	ParentEditor = nullptr;
}

void LineTrackingManagerBgRenderer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	textView->EnsureVisualLines();

	DoCurrentLineBackground(textView, drawingContext);
	DoErrorSquiggles(textView, drawingContext);
	DoFindResults(textView, drawingContext);
	DoBraceIndicators(textView, drawingContext);

	String^ SelectionText = "";
	if (!ParentEditor->TextArea->Selection->IsEmpty)
		SelectionText = ParentEditor->TextArea->Selection->GetText()->Replace("\t", "")->Replace(" ", "")->Replace("\n", "")->Replace("\r\n", "");

	for each (auto Line in ParentEditor->Document->Lines)
	{
		if (ParentEditor->TextArea->TextView->GetVisualLine(Line->LineNumber) == nullptr)
			continue;

		DoLineLimitBackground(Line, textView, drawingContext);
		DoSelectedStringBackground(SelectionText, Line, textView, drawingContext);
	}
}

void LineTrackingManagerBgRenderer::Redraw()
{
	ParentEditor->TextArea->TextView->InvalidateLayer(Layer);
}


AvalonEditLineAnchor::AvalonEditLineAnchor(AvalonEdit::TextEditor^ Parent, UInt32 Line, bool AllowDeletion)
{
	if (Line > Parent->LineCount)
		Line = Parent->LineCount;

	auto LineStartOffset = Parent->Document->GetLineByNumber(Line)->Offset;
	if (LineStartOffset > Parent->Document->TextLength)
		LineStartOffset = Parent->Document->TextLength;

	Anchor = Parent->Document->CreateAnchor(LineStartOffset);
	Anchor->SurviveDeletion = AllowDeletion == false;
}

void LineTrackingManager::ParentEditor_TextChanged(Object^ Sender, EventArgs^ E)
{
	// disable compiler error indicators for the changed line
	int Caret = ParentEditor->TextArea->Caret->Offset;
	int Line = ParentEditor->Document->GetLineByOffset(Caret)->LineNumber;
	bool RefreshBg = false;

	for each (auto Error in LineBgRenderer->ErrorSquiggles)
	{
		if (Error->Line == Line)
		{
			Error->Enabled = false;
			RefreshBg = true;
		}
	}

	// disable find result indicators for modified segments
	for each (auto% FindResult in TrackedFindResultSegments)
	{
		if (FindResult.Key->Start->IsDeleted || FindResult.Key->End->IsDeleted)
			continue;

		if (Caret >= FindResult.Value->StartOffset && Caret <= FindResult.Value->EndOffset)
		{
			FindResult.Value->Enabled = false;
			RefreshBg = true;
		}
	}

	if (RefreshBg)
		LineBgRenderer->Redraw();
}

void LineTrackingManager::ParentModel_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E)
{
	if (E->EventType != model::IScriptDocument::StateChangeEventArgs::eEventType::Messages)
		return;

	// update the error squiggles
	LineBgRenderer->ErrorSquiggles->Clear();
	auto ProcessedLines = gcnew List<UInt32>;
	for each (auto Itr in E->Messages)
	{
		if (!Itr->Valid)
			continue;
		else if (ProcessedLines->Contains(Itr->Line))
			continue;
		else if (Itr->Type != model::components::ScriptDiagnosticMessage::eMessageType::Error)
			continue;

		DocumentLine^ Line = ParentEditor->TextArea->Document->GetLineByNumber(Itr->Line);
		ISegment^ WhitespaceLeading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(ParentEditor->TextArea->Document, Line);
		ISegment^ WhitespaceTrailing = AvalonEdit::Document::TextUtilities::GetTrailingWhitespace(ParentEditor->TextArea->Document, Line);

		auto NewSegment = gcnew BackgroundRenderSegment;
		NewSegment->Line = Itr->Line;
		NewSegment->StartOffset = WhitespaceLeading->EndOffset;
		NewSegment->EndOffset = WhitespaceTrailing->Offset;
		LineBgRenderer->ErrorSquiggles->Add(NewSegment);
		ProcessedLines->Add(Itr->Line);
	}
}

LineTrackingManager::LineTrackingManager(AvalonEdit::TextEditor^ ParentEditor, model::IScriptDocument^ ParentScriptDocument)
{
	this->ParentEditor = ParentEditor;
	this->ParentScriptDocument = ParentScriptDocument;

	TrackedLineAnchors = gcnew List<AvalonEditLineAnchor^>;
	TrackedFindResultSegments = gcnew Dictionary<FindResultSegment^, BackgroundRenderSegment^>;
	LineBgRenderer = gcnew LineTrackingManagerBgRenderer(ParentEditor);

	ParentModelStateChangedHandler = gcnew model::IScriptDocument::StateChangeEventHandler(this, &LineTrackingManager::ParentModel_StateChanged);
	ParentEditorTextChangedHandler = gcnew EventHandler(this, &LineTrackingManager::ParentEditor_TextChanged);

	this->ParentEditor->TextArea->TextView->BackgroundRenderers->Add(LineBgRenderer);

	this->ParentEditor->TextChanged += ParentEditorTextChangedHandler;
	this->ParentScriptDocument->StateChanged += ParentModelStateChangedHandler;
}

LineTrackingManager::~LineTrackingManager()
{
	TrackedLineAnchors->Clear();
	TrackedFindResultSegments->Clear();

	ParentEditor->TextArea->TextView->BackgroundRenderers->Remove(LineBgRenderer);

	ParentEditor->TextChanged -= ParentEditorTextChangedHandler;
	ParentScriptDocument->StateChanged -= ParentModelStateChangedHandler;

	SAFEDELETE_CLR(ParentEditorTextChangedHandler);
	SAFEDELETE_CLR(ParentModelStateChangedHandler);
	SAFEDELETE_CLR(LineBgRenderer);

	ParentEditor = nullptr;
	ParentScriptDocument = nullptr;
}

TextAnchor^ LineTrackingManager::CreateAnchor(int Offset, bool AllowDeletion)
{
	if (Offset > ParentEditor->Document->TextLength)
		Offset = ParentEditor->Document->TextLength;

	TextAnchor^ New = ParentEditor->Document->CreateAnchor(Offset);
	New->SurviveDeletion = AllowDeletion == false;
	return New;
}

AvalonEditLineAnchor^ LineTrackingManager::CreateLineAnchor(UInt32 Line, bool AllowDeletion)
{
	auto NewLineAnchor = gcnew AvalonEditLineAnchor(ParentEditor, Line, AllowDeletion);;
	TrackedLineAnchors->Add(NewLineAnchor);
	return NewLineAnchor;
}

void LineTrackingManager::TrackFindResultSegment(UInt32 Start, UInt32 End)
{
	auto NewSegment = gcnew FindResultSegment;
	NewSegment->Start = CreateAnchor(Start, false);
	NewSegment->End = CreateAnchor(End, false);

	auto BgRendererSegment = gcnew BackgroundRenderSegment;
	BgRendererSegment->StartOffset = NewSegment->Start->Offset;
	BgRendererSegment->EndOffset = NewSegment->End->Offset;

	TrackedFindResultSegments->Add(NewSegment, BgRendererSegment);
	LineBgRenderer->FindResults->Add(BgRendererSegment);

	LineBgRenderer->Redraw();
}

void LineTrackingManager::ClearFindResultSegments()
{
	TrackedFindResultSegments->Clear();
	LineBgRenderer->FindResults->Clear();
	LineBgRenderer->Redraw();
}

bool LineTrackingManager::RemoveDeletedLineAnchors()
{
	auto Invalidated = gcnew List<AvalonEditLineAnchor^>;

	for each (auto Itr in TrackedLineAnchors)
	{
		if (Itr->Anchor->IsDeleted)
			Invalidated->Add(Itr);
	}

	for each (auto Itr in Invalidated)
		TrackedLineAnchors->Remove(Itr);

	if (Invalidated->Count == 0)
		return false;

	LineBgRenderer->Redraw();
	return true;
}

void ObScriptIndentStrategy::IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine)
{
	;//
}

void ObScriptIndentStrategy::IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line)
{
	UInt32 CurrentIndents = 0, PreviousIndents = 0;
	CalculateIndentsTillCurrentLine(document, line, CurrentIndents, PreviousIndents);

	AvalonEdit::Document::DocumentLine^ previousLine = line->PreviousLine;
	if (CullEmptyLines && document->GetText(previousLine)->TrimStart()->TrimEnd()->Length == 0)
		document->Replace(previousLine, "");
	else
	{
		auto LeadingWhitespace = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(document, previousLine);
		document->Replace(LeadingWhitespace, gcnew String('\t', PreviousIndents));
	}

	auto CurrentWhitespace = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(document, line);
	document->Replace(CurrentWhitespace, gcnew String('\t', CurrentIndents));
}

ObScriptIndentStrategy::~ObScriptIndentStrategy()
{
	Parent = nullptr;
}

ObScriptIndentStrategy::ObScriptIndentStrategy( AvalonEditTextEditor^ Parent, bool TrimTrailingWhitespace, bool CullEmptyLines ) :
	Parent(Parent),
	TrimTrailingWhitespace(TrimTrailingWhitespace),
	CullEmptyLines(CullEmptyLines)
{
	;//
}

void ObScriptIndentStrategy::CalculateIndentsTillCurrentLine(AvalonEdit::Document::TextDocument^ Document, AvalonEdit::Document::DocumentLine^ CurrentLine,
															UInt32% OutCurrentLineIndents, UInt32% OutPreviousLineIndents)
{
	auto Text = Document->GetText(0, CurrentLine->EndOffset)->TrimEnd();
	auto Parser = gcnew obScriptParsing::LineTokenizer();
	LineTrackingStringReader^ Reader = gcnew LineTrackingStringReader(Text);

	int RunningIndentCount = 0, CurrentLineIndents = 0, PreviousLineIndents = 0;
	for (String^ ReadLine = Reader->ReadLine(); ReadLine != nullptr; ReadLine = Reader->ReadLine())
	{
		PreviousLineIndents = CurrentLineIndents;
		CurrentLineIndents = RunningIndentCount;

		if (Parser->Tokenize(ReadLine, false))
		{
			switch (Parser->GetFirstTokenType())
			{
			case obScriptParsing::eScriptTokenType::Begin:
			case obScriptParsing::eScriptTokenType::If:
			case obScriptParsing::eScriptTokenType::ForEach:
			case obScriptParsing::eScriptTokenType::While:
				++RunningIndentCount;
				break;
			case obScriptParsing::eScriptTokenType::ElseIf:
			case obScriptParsing::eScriptTokenType::Else:
				--CurrentLineIndents;
				break;
			case obScriptParsing::eScriptTokenType::End:
			case obScriptParsing::eScriptTokenType::EndIf:
			case obScriptParsing::eScriptTokenType::Loop:
				--RunningIndentCount;
				--CurrentLineIndents;
				break;
			}
		}
	}

	PreviousLineIndents = CurrentLineIndents;
	CurrentLineIndents = RunningIndentCount;

	if (RunningIndentCount < 0)
		RunningIndentCount = 0;

	if (CurrentLineIndents < 0)
		CurrentLineIndents = 0;

	if (PreviousLineIndents < 0)
		PreviousLineIndents = 0;

	OutCurrentLineIndents = CurrentLineIndents;
	OutPreviousLineIndents = PreviousLineIndents;
}

int ObScriptCodeFoldingStrategy::FoldingSorter::Compare( AvalonEdit::Folding::NewFolding^ X, AvalonEdit::Folding::NewFolding^ Y )
{
	return X->StartOffset.CompareTo(Y->StartOffset);
}

IEnumerable<AvalonEdit::Folding::NewFolding^>^ ObScriptCodeFoldingStrategy::CreateNewFoldings( AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset )
{
	firstErrorOffset = -1;

	List<AvalonEdit::Folding::NewFolding^>^ Foldings = gcnew List<AvalonEdit::Folding::NewFolding^>();

	obScriptParsing::AnalysisData^ Data = Parent->SemanticAnalysisData;
	for each (obScriptParsing::ControlBlock^ Itr in Data->ControlBlocks)
	{
		if (Itr->IsMalformed())
			continue;
		else if (Itr->StartLine < 0 || Itr->StartLine > document->LineCount)
			continue;
		else if (Itr->EndLine - 1 < 0 || Itr->EndLine - 1 > document->LineCount)
			continue;

		auto StartLine = document->GetLineByNumber(Itr->StartLine);
		auto EndLine = document->GetLineByNumber(Itr->EndLine - 1);

		auto StartOffset = StartLine->EndOffset;
		auto EndOffset = EndLine->EndOffset;

		if (EndOffset - StartOffset < 1)
			continue;

		Foldings->Add(gcnew AvalonEdit::Folding::NewFolding(StartOffset, EndOffset));
	}

	if (Data->MalformedStructure && Data->FirstStructuralErrorLine <= document->LineCount && Data->FirstStructuralErrorLine > 0)
		firstErrorOffset = document->GetLineByNumber(Data->FirstStructuralErrorLine)->Offset;

	if (Foldings->Count)
		Foldings->Sort(Sorter);

	return Foldings;
}

ObScriptCodeFoldingStrategy::~ObScriptCodeFoldingStrategy()
{
	Parent = nullptr;
}

ObScriptCodeFoldingStrategy::ObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent) :
	Parent(Parent),
	Sorter(gcnew FoldingSorter())
{
	;//
}

StructureVisualizerRenderer::StructureVisualizerRenderer(AvalonEditTextEditor^ Parent) :
	VisualLineElementGenerator(),
	ParentEditor(Parent)
{
	InstanceCounter++;
}

int StructureVisualizerRenderer::GetFirstInterestedOffset(Int32 startOffset)
{
	DocumentLine^ CurrentLine = CurrentContext->Document->GetLineByOffset(startOffset);

	if (ParentEditor->SemanticAnalysisData->GetBlockEndingAt(CurrentLine->LineNumber))
		return startOffset + CurrentLine->Length;
	else
		return -1;
}

VisualLineElement^ StructureVisualizerRenderer::ConstructElement(Int32 offset)
{
	DocumentLine^ CurrentLine = CurrentContext->Document->GetLineByOffset(offset);
	obScriptParsing::ControlBlock^ Block = ParentEditor->SemanticAnalysisData->GetBlockEndingAt(CurrentLine->LineNumber);

	if (Block)
	{
		if (Block->StartLine < ParentEditor->FirstVisibleLine)
		{
			DocumentLine^ BlockStart = CurrentContext->Document->GetLineByNumber(Block->StartLine);
			if (BlockStart)
			{
				return gcnew InlineObjectElement(0,
													GenerateAdornment(Block->StartLine, CurrentContext->Document->GetText(BlockStart)));
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
		Data->Parent->ScrollToLine(Data->JumpLine);
}

Windows::UIElement^ StructureVisualizerRenderer::GenerateAdornment(UInt32 JumpLine, String^ ElementText)
{
	Color ForegroundColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	Font^ CustomFont = gcnew Font(preferences::SettingsHolder::Get()->Appearance->TextFont->FontFamily->Name,
									preferences::SettingsHolder::Get()->Appearance->TextFont->Size - 2,
									FontStyle::Italic);

	Windows::Media::Brush^ ForegroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(100,
													ForegroundColor.R,
													ForegroundColor.G,
													ForegroundColor.B));
	Windows::Media::Brush^ BackgroundBrush = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(0, 0, 0, 0));

	ElementText = ElementText->Replace("\t", "");
	if (ElementText->Length > 100)
		ElementText = ElementText->Substring(0, 100) + "...";

	AdornmentData^ Data = gcnew AdornmentData;
	Data->JumpLine = JumpLine;
	Data->Parent = ParentEditor;

	Windows::Controls::StackPanel^ Panel = gcnew Windows::Controls::StackPanel();
	Panel->HorizontalAlignment = Windows::HorizontalAlignment::Center;
	Panel->VerticalAlignment = Windows::VerticalAlignment::Bottom;
	Panel->Orientation = Windows::Controls::Orientation::Horizontal;
	Panel->Margin = Windows::Thickness(20, 0, 20, 0);
	Panel->Cursor = Windows::Input::Cursors::Hand;
	Panel->Tag = Data;
	Panel->PreviewMouseDown += gcnew System::Windows::Input::MouseButtonEventHandler(OnMouseClick);

	Windows::Media::Imaging::BitmapSource^ IconData = GetIconSource();
	if (IconData)
	{
		Windows::Controls::Image^ Icon = gcnew Windows::Controls::Image();
		Icon->Source = IconData;
		Icon->Width = 14;
		Icon->Height = 14;
		Icon->HorizontalAlignment = Windows::HorizontalAlignment::Center;
		Icon->VerticalAlignment = Windows::VerticalAlignment::Bottom;
		Panel->Children->Add(Icon);
	}

	Windows::Controls::Label^ AdornmentLabel = gcnew Windows::Controls::Label();
	AdornmentLabel->FontFamily = gcnew Windows::Media::FontFamily(CustomFont->FontFamily->Name);
	AdornmentLabel->FontSize = CustomFont->Size;
	AdornmentLabel->FontStyle = Windows::FontStyles::Italic;
	AdornmentLabel->Foreground = ForegroundBrush;
	AdornmentLabel->Background = BackgroundBrush;
	AdornmentLabel->Content = ElementText;
	AdornmentLabel->Padding = Windows::Thickness(0, 0, 0, 0);
	AdornmentLabel->Margin = Windows::Thickness(5, 0, 0, 0);
	AdornmentLabel->HorizontalAlignment = Windows::HorizontalAlignment::Center;
	AdornmentLabel->VerticalAlignment = Windows::VerticalAlignment::Bottom;
	Panel->Children->Add(AdornmentLabel);

	return Panel;
}

StructureVisualizerRenderer::~StructureVisualizerRenderer()
{
	InstanceCounter--;
	Debug::Assert(InstanceCounter >= 0);

	ParentEditor = nullptr;

	if (InstanceCounter == 0 && ElementIcon)
		SAFEDELETE_CLR(ElementIcon);
}

Windows::Media::Imaging::BitmapSource^ StructureVisualizerRenderer::GetIconSource()
{
	if (ElementIcon == nullptr)
		ElementIcon = WPFImageResourceGenerator::CreateImageSource("AvalonEditStructureVisualizer");

	return ElementIcon;
}

void IconMargin::OnHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	int Line = GetLineFromMousePosition(E);
	if (Line < 1)
		return;

	HandleHoverStart(Line, E);
}

void IconMargin::OnHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	HandleHoverStop();
}

void IconMargin::OnTextViewChanged(AvalonEdit::Rendering::TextView^ oldTextView, AvalonEdit::Rendering::TextView^ newTextView)
{
	if (oldTextView)
		oldTextView->VisualLinesChanged -= HandlerTextViewChanged;

	AbstractMargin::OnTextViewChanged(oldTextView, newTextView);

	if (newTextView)
		newTextView->VisualLinesChanged += HandlerTextViewChanged;

	InvalidateVisual();
}

void IconMargin::OnRedrawRequested(Object^ sender, EventArgs^ E)
{
	if (this->TextView && this->TextView->VisualLinesValid)
		InvalidateVisual();
}

Windows::Media::HitTestResult^ IconMargin::HitTestCore(Windows::Media::PointHitTestParameters^ hitTestParameters)
{
	return gcnew Windows::Media::PointHitTestResult(this, hitTestParameters->HitPoint);
}

Windows::Size IconMargin::MeasureOverride(Windows::Size availableSize)
{
	return Windows::Size(18, 0);
}

void IconMargin::OnRender(Windows::Media::DrawingContext^ drawingContext)
{
	Windows::Size renderSize = this->RenderSize;
	drawingContext->DrawRectangle(Windows::SystemColors::ControlBrush, nullptr, Windows::Rect(0, 0, renderSize.Width, renderSize.Height));
	drawingContext->DrawLine(gcnew Windows::Media::Pen(Windows::SystemColors::ControlDarkBrush, 1),
							Windows::Point(renderSize.Width - 0.5, 0),
							Windows::Point(renderSize.Width - 0.5, renderSize.Height));

	AvalonEdit::Rendering::TextView^ textView = this->TextView;

	if (textView && textView->VisualLinesValid)
	{
		Windows::Size pixelSize = PixelSnapHelpers::GetPixelSize(this);
		for each (VisualLine^ line in textView->VisualLines)
		{
			int lineNumber = line->FirstDocumentLine->LineNumber;
			Windows::Media::Imaging::BitmapSource^ icon = nullptr;
			double opacity = 1.0;
			int W = 16, H = 16;
			if (GetRenderData(lineNumber, icon, opacity, W, H))
			{
				double lineMiddle = line->GetTextLineVisualYPosition(line->TextLines[0], VisualYPosition::TextMiddle) - textView->VerticalOffset;
				Windows::Rect rect((16 - W)/2, PixelSnapHelpers::Round(lineMiddle - H/2, pixelSize.Height), W, H);
				drawingContext->PushOpacity(opacity);
				drawingContext->DrawImage(icon, rect);
				drawingContext->Pop();
			}
		}
	}
}

void IconMargin::OnMouseDown(System::Windows::Input::MouseButtonEventArgs^ e)
{
	AbstractMargin::OnMouseDown(e);
	if (e->ChangedButton == System::Windows::Input::MouseButton::Left)
		e->Handled = true;
}

void IconMargin::OnMouseMove(System::Windows::Input::MouseEventArgs^ e)
{
	AbstractMargin::OnMouseMove(e);
}

void IconMargin::OnMouseUp(System::Windows::Input::MouseButtonEventArgs^ e)
{
	AbstractMargin::OnMouseUp(e);
	int line = GetLineFromMousePosition(e);
	if (e->ChangedButton == System::Windows::Input::MouseButton::Left && TextView)
		HandleClick(line);
}

void IconMargin::OnMouseLeave(System::Windows::Input::MouseEventArgs^ e)
{
	AbstractMargin::OnMouseLeave(e);
}

int IconMargin::GetLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e)
{
	VisualLine^ vl = GetVisualLineFromMousePosition(e);
	if (vl == nullptr)
		return 0;

	return vl->FirstDocumentLine->LineNumber;
}

VisualLine^ IconMargin::GetVisualLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e)
{
	AvalonEdit::Rendering::TextView^ textView = this->TextView;
	if (textView == nullptr)
		return nullptr;

	VisualLine^ vl = textView->GetVisualLineFromVisualTop(e->GetPosition(textView).Y + textView->ScrollOffset.Y);
	return vl;
}

IconMargin::IconMargin()
{
	HoverLogic = gcnew MouseHoverLogic(this);

	HandlerHover = gcnew EventHandler<System::Windows::Input::MouseEventArgs^>(this, &IconMargin::OnHover);
	HandlerHoverStopped = gcnew EventHandler<System::Windows::Input::MouseEventArgs^>(this, &IconMargin::OnHoverStopped);
	HandlerTextViewChanged = gcnew EventHandler(this, &IconMargin::OnRedrawRequested);

	HoverLogic->MouseHover += HandlerHover;
	HoverLogic->MouseHoverStopped += HandlerHoverStopped;
}

IconMargin::~IconMargin()
{
	this->TextView = nullptr;

	HoverLogic->MouseHover -= HandlerHover;
	HoverLogic->MouseHoverStopped -= HandlerHoverStopped;

	SAFEDELETE_CLR(HoverLogic);
}

Windows::Media::Imaging::BitmapSource^ DefaultIconMargin::GetWarningIcon()
{
	if (WarningIcon == nullptr)
		WarningIcon = WPFImageResourceGenerator::CreateImageSource("AvalonEditIconMarginWarningColor");

	return WarningIcon;
}

Windows::Media::Imaging::BitmapSource^ DefaultIconMargin::GetErrorIcon()
{
	if (ErrorIcon == nullptr)
		ErrorIcon = WPFImageResourceGenerator::CreateImageSource("AvalonEditIconMarginErrorColor");

	return ErrorIcon;
}

Windows::Media::Imaging::BitmapSource^ DefaultIconMargin::GetBookmarkIcon()
{
	if (BookmarkIcon == nullptr)
		BookmarkIcon = WPFImageResourceGenerator::CreateImageSource("AvalonEditIconMarginBookmark");

	return BookmarkIcon;
}

void DefaultIconMargin::ParentModel_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E)
{
	this->InvalidateVisual();
}

void DefaultIconMargin::HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E)
{
	bool DisplayPopup = false;
	String^ PopupTitle = "";
	String^ PopupText = "";
	auto PopupBgColor = IRichTooltipContentProvider::BackgroundColor::Default;

	Windows::Point DisplayLocation = TransformToPixels(E->GetPosition(ParentEditor));
	DisplayLocation.X += System::Windows::SystemParameters::CursorWidth;
	//DisplayLocation.Y += GetVisualLineFromMousePosition(E)->Height;
	DisplayLocation = ParentEditor->PointToScreen(DisplayLocation);

	String^ kRowStart = "<div width=\"350\">", ^kRowEnd = "</div>";
	String^ kCellStart = "<span padding=\"0,0,0,5\">", ^kCellEnd = "</span>\n";

	if (ParentScriptDocument->GetErrorCount(Line))
	{
		DisplayPopup = true;
		auto Errors = ParentScriptDocument->GetMessages(Line,
														model::components::ScriptDiagnosticMessage::eMessageSource::All,
														model::components::ScriptDiagnosticMessage::eMessageType::Error);

		PopupBgColor = IRichTooltipContentProvider::BackgroundColor::Red;
		PopupTitle = Errors->Count + " error" + (Errors->Count == 1 ? "" : "s");

		PopupText += kRowStart;
		for each (auto Itr in Errors)
			PopupText += kCellStart + Itr->Text + kCellEnd;
		PopupText += kRowEnd;
	}
	else if (ParentScriptDocument->GetWarningCount(Line))
	{
		DisplayPopup = true;
		auto Warnings = ParentScriptDocument->GetMessages(Line,
														  model::components::ScriptDiagnosticMessage::eMessageSource::All,
														  model::components::ScriptDiagnosticMessage::eMessageType::Warning);

		PopupBgColor = IRichTooltipContentProvider::BackgroundColor::Yellow;
		PopupTitle = Warnings->Count + " warning" + (Warnings->Count == 1 ? "" : "s");

		PopupText += kRowStart;
		for each (auto Itr in Warnings)
			PopupText += kCellStart + Itr->Text + kCellEnd;
		PopupText += kRowEnd;
	}
	else if (ParentScriptDocument->GetBookmarkCount(Line))
	{
		DisplayPopup = true;
		auto Bookmarks = ParentScriptDocument->GetBookmarks(Line);

		PopupBgColor = IRichTooltipContentProvider::BackgroundColor::Blue;
		PopupTitle = Bookmarks->Count + " bookmark" + (Bookmarks->Count == 1 ? "" : "s");

		PopupText += kRowStart;
		for each (auto Itr in Bookmarks)
			PopupText += kCellStart + Itr->Text + kCellEnd;
		PopupText += kRowEnd;
	}

	if (DisplayPopup)
	{
		PopupTitle = "<font size=\"+2\"><b>" + PopupTitle + "</b></font>";
		PopupText = PopupText->Replace("\n", "<br/>");

		auto TooltipData = gcnew DotNetBar::SuperTooltipInfo;
		TooltipData->HeaderText = PopupTitle;
		TooltipData->BodyText = PopupText;
		TooltipData->Color = MapRichTooltipBackgroundColorToDotNetBar(PopupBgColor);

		auto Control = Control::FromHandle(PopupParent);
		Popup->SetSuperTooltip(Control, TooltipData);
		Popup->ShowTooltip(Control, Point(DisplayLocation.X, DisplayLocation.Y));
	}
}

void DefaultIconMargin::HandleHoverStop()
{
	Popup->HideTooltip();
}

void DefaultIconMargin::HandleClick(int Line)
{
	;//
}

bool DefaultIconMargin::GetRenderData(int Line, Windows::Media::Imaging::BitmapSource^% OutIcon, double% OutOpacity, int% Width, int% Height)
{
	auto ErrorCount = ParentScriptDocument->GetErrorCount(Line);
	auto WarningCount = ParentScriptDocument->GetWarningCount(Line);
	auto BookmarkCount = ParentScriptDocument->GetBookmarkCount(Line);

	if (ErrorCount == 0 && WarningCount == 0 && BookmarkCount == 0)
		return false;

	if (ErrorCount)
	{
		OutIcon = GetErrorIcon();
		Width = Height = 16;
	}
	else if (WarningCount)
	{
		OutIcon = GetWarningIcon();
		Width = Height = 16;
	}
	else if (BookmarkCount)
	{
		OutIcon = GetBookmarkIcon();
		Width = Height = 16;
	}

	OutOpacity = 1.0;
	return true;
}

DefaultIconMargin::DefaultIconMargin(AvalonEdit::TextEditor^ ParentEditor, model::IScriptDocument^ ParentScriptDocument, IntPtr ToolTipParent)
{
	++InstanceCounter;

	this->ParentEditor = ParentEditor;
	this->ParentScriptDocument = ParentScriptDocument;
	this->PopupParent = ToolTipParent;

	Popup = gcnew DotNetBar::SuperTooltip;
	Popup->DelayTooltipHideDuration = 500;
	Popup->CheckTooltipPosition = false;
	Popup->TooltipDuration = 30000;
	Popup->DefaultFont = gcnew Font(SystemFonts::DialogFont->FontFamily, 9.25);
	Popup->MinimumTooltipSize = Size(180, 25);

	ParentModelStateChangedHandler = gcnew model::IScriptDocument::StateChangeEventHandler(this, &DefaultIconMargin::ParentModel_StateChanged);
	this->ParentScriptDocument->StateChanged += ParentModelStateChangedHandler;
}

DefaultIconMargin::~DefaultIconMargin()
{
	InstanceCounter--;
	Debug::Assert(InstanceCounter >= 0);

	ParentScriptDocument->StateChanged -= ParentModelStateChangedHandler;
	SAFEDELETE_CLR(ParentModelStateChangedHandler);

	Popup->HideTooltip();
	SAFEDELETE_CLR(Popup);

	ParentEditor = nullptr;
	ParentScriptDocument = nullptr;

	if (InstanceCounter == 0)
	{
		if (WarningIcon)
			SAFEDELETE_CLR(WarningIcon);

		if (BookmarkIcon)
			SAFEDELETE_CLR(BookmarkIcon);

		if (ErrorIcon)
			SAFEDELETE_CLR(ErrorIcon);
	}
}


TagableDoubleAnimation::TagableDoubleAnimation(double fromValue, double toValue, System::Windows::Duration duration, System::Windows::Media::Animation::FillBehavior fillBehavior)
	: DoubleAnimation(fromValue, toValue, duration, fillBehavior)
{
}


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse