#include "AvalonEditComponents.h"
#include "AvalonEditTextEditor.h"
#include "IScriptEditorView.h"
#include "Preferences.h"
#include "Utilities.h"

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
	Drawing::Bitmap^ OrgResource = safe_cast<Drawing::Bitmap^>(view::components::CommonIcons::Get()->ResourceManager->CreateImage(ResourceIdentifier));
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

BracketSearchData::BracketSearchData(Char Symbol, int StartOffset) :
	Symbol(Symbol),
	StartOffset(StartOffset)
{
	EndOffset = -1;
	Mismatching = false;
}

BracketSearchData::eBracketType BracketSearchData::GetType()
{
	switch (Symbol)
	{
	case '(':
	case ')':
		return eBracketType::Curved;
	case '[':
	case ']':
		return eBracketType::Square;
	case '{':
	case '}':
		return eBracketType::Squiggly;
	default:
		return eBracketType::Invalid;
	}
}

BracketSearchData::eBracketState BracketSearchData::GetKind()
{
	switch (Symbol)
	{
	case '(':
	case '[':
	case '{':
		return eBracketState::Opening;
	case ')':
	case ']':
	case '}':
		return eBracketState::Closing;
	default:
		return eBracketState::Invalid;
	}
}

int BracketSearchData::GetStartOffset()
{
	return StartOffset;
}

LineAnchor::LineAnchor(AvalonEdit::TextEditor^ Parent, UInt32 Line, bool AllowDeletion)
{
	if (Line > Parent->LineCount)
		Line = Parent->LineCount;

	auto LineStartOffset = Parent->Document->GetLineByNumber(Line)->Offset;
	if (LineStartOffset > Parent->Document->TextLength)
		LineStartOffset = Parent->Document->TextLength;

	Anchor_ = Parent->Document->CreateAnchor(LineStartOffset);
	Anchor_->SurviveDeletion = AllowDeletion == false;
}


StaticDocumentSegment::StaticDocumentSegment()
{
	Line = 0;
	StartOffset = 0;
	EndOffset = 0;
	Enabled = true;
}

AnchoredDocumentSegment::AnchoredDocumentSegment(TextAnchor^ Start, TextAnchor^ End)
	: StartOffsetAnchor(Start), EndOffsetAnchor(End)
{
	Enabled = true;
}

CustomBrushes::BgRendererBrush::BgRendererBrush(Color BackColor, int Alpha)
{
	auto BgColor = Windows::Media::Color::FromArgb(Alpha, BackColor.R, BackColor.G, BackColor.B);
	Background = gcnew System::Windows::Media::SolidColorBrush(BgColor);
	Background->Freeze();

	auto BorderColor = Windows::Media::Color::FromArgb(Math::Max(255, Alpha + 50), BackColor.R, BackColor.G, BackColor.B);
	auto BorderBrush = gcnew System::Windows::Media::SolidColorBrush(BorderColor);
	BorderBrush->Freeze();

	Border = gcnew System::Windows::Media::Pen(BorderBrush, 1);
	Border->Freeze();
}

void CustomBrushes::RecreateAll()
{
	Color ForegroundColor = preferences::SettingsHolder::Get()->Appearance->ForeColor;
	Color BackgroundColor = preferences::SettingsHolder::Get()->Appearance->BackColor;
	ForeColor = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, ForegroundColor.R, ForegroundColor.G, ForegroundColor.B));
	ForeColorPartlyTransparent = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(150, ForegroundColor.R, ForegroundColor.G, ForegroundColor.B));
	BackColor = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(255, BackgroundColor.R, BackgroundColor.G, BackgroundColor.B));

	ForeColor->Freeze();
	ForeColorPartlyTransparent->Freeze();
	BackColor->Freeze();

	Transparent = gcnew System::Windows::Media::SolidColorBrush(Windows::Media::Color::FromArgb(0, 0, 0, 0));
	Transparent->Freeze();

	SelectionMatch = gcnew BgRendererBrush(preferences::SettingsHolder::Get()->Appearance->BackColorSelection, 120);
	CurrentLine = gcnew BgRendererBrush(preferences::SettingsHolder::Get()->Appearance->BackColorCurrentLine, 120);
	FindResults = gcnew BgRendererBrush(preferences::SettingsHolder::Get()->Appearance->BackColorFindResults, 120);

	auto ErrorSquiglyColor = Windows::Media::Color::FromArgb(255,
		preferences::SettingsHolder::Get()->Appearance->UnderlineColorError.R,
		preferences::SettingsHolder::Get()->Appearance->UnderlineColorError.G,
		preferences::SettingsHolder::Get()->Appearance->UnderlineColorError.B);

	auto ErrorSquiglyBrush = gcnew Windows::Media::SolidColorBrush(ErrorSquiglyColor);
	ErrorSquiglyBrush->Freeze();

	ErrorSquigly = gcnew System::Windows::Media::Pen(ErrorSquiglyBrush, 1);
	ErrorSquigly->Freeze();

	ValidBrace = gcnew BgRendererBrush(Color::LightSlateGray, 120);
	InvalidBrace = gcnew BgRendererBrush(Color::MediumVioletRed, 120);
}

void CustomBrushes::Preferences_Changed(Object^ Sender, EventArgs^ E)
{
	RecreateAll();
}

CustomBrushes::CustomBrushes()
{
	preferences::SettingsHolder::Get()->PreferencesChanged += gcnew System::EventHandler(this, &CustomBrushes::Preferences_Changed);

	RecreateAll();
}

CustomBrushes^ CustomBrushes::Get()
{
	if (Singleton == nullptr)
		Singleton = gcnew CustomBrushes;

	return Singleton;
}

void LineTrackingManagerBgRenderer::RenderBackground(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, CustomBrushes::BgRendererBrush^ Brush, bool ColorEntireLine)
{
	TextSegment^ Segment = gcnew TextSegment();
	Segment->StartOffset = StartOffset;
	Segment->EndOffset = EndOffset;

	for each (auto% R in BackgroundGeometryBuilder::GetRectsForSegment(Destination, Segment, false))
	{
		if (ColorEntireLine)
		{
			Windows::Rect Bounds(R.Location, Windows::Size(Destination->ActualWidth + Destination->HorizontalOffset, R.Height));
			DrawingContext->DrawRoundedRectangle(Brush->Background, Brush->Border, Bounds, 2, 2);
		}
		else
		{
			Windows::Rect Bounds(R.Location, Windows::Size(R.Width, R.Height));
			DrawingContext->DrawRoundedRectangle(Brush->Background, Brush->Border, Bounds, 2, 2);
		}
	}
}

void LineTrackingManagerBgRenderer::RenderSquiggle(TextView^ Destination, System::Windows::Media::DrawingContext^ DrawingContext, int StartOffset, int EndOffset, System::Windows::Media::Pen^ Pen)
{
	TextSegment^ Segment = gcnew TextSegment();
	Segment->StartOffset = StartOffset;
	Segment->EndOffset = EndOffset;

	for each (Windows::Rect R in BackgroundGeometryBuilder::GetRectsForSegment(Destination, Segment, false))
	{
		Windows::Point StartPoint = R.BottomLeft;
		Windows::Point EndPoint = R.BottomRight;

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
		DrawingContext->DrawGeometry(Windows::Media::Brushes::Transparent, Pen, Geometry);
	}
}

void LineTrackingManagerBgRenderer::DoCurrentLineBackground(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext, int FirstVisibleLine, int LastVisibleLine)
{
	if (!ParentEditor->TextArea->Selection->IsEmpty)
		return;

	DocumentLine^ Line = ParentEditor->Document->GetLineByNumber(ParentEditor->TextArea->Caret->Line);
	if (Line->LineNumber < FirstVisibleLine || Line->LineNumber > LastVisibleLine)
		return;

	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorCurrentLine;
	RenderBackground(textView, drawingContext, Line->Offset, Line->EndOffset, CustomBrushes::Get()->CurrentLine, true);
}

void LineTrackingManagerBgRenderer::DoSelectedStringBackground(String^ SelectionText, DocumentLine^ Line, TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	if (SelectionText->Length <= 1)
		return;

	String^ CurrentLine = ParentEditor->Document->GetText(Line);

	int Index = 0, Start = 0;
	while (Start < CurrentLine->Length && (Index = CurrentLine->IndexOf(SelectionText, Start, System::StringComparison::CurrentCultureIgnoreCase)) != -1)
	{
		int EndIndex = Index + SelectionText->Length;
		RenderBackground(textView, drawingContext,
						 Line->Offset + Index,
						 Line->Offset + EndIndex,
						 CustomBrushes::Get()->SelectionMatch, false);

		Start = EndIndex + 1;
	}
}

void LineTrackingManagerBgRenderer::DoErrorSquiggles(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext, int FirstVisibleLine, int LastVisibleLine)
{
	Color Buffer = preferences::SettingsHolder::Get()->Appearance->UnderlineColorError;

	auto RenderColor = Windows::Media::Color::FromArgb(255, Buffer.R, Buffer.G, Buffer.B);

	for each (auto Itr in ErrorSquiggles)
	{
		if (!Itr->Enabled || !Itr->Valid)
			continue;
		else if (Itr->Line < FirstVisibleLine || Itr->Line > LastVisibleLine)
			continue;

		RenderSquiggle(textView, drawingContext, Itr->StartOffset, Itr->EndOffset, CustomBrushes::Get()->ErrorSquigly);
	}
}

void LineTrackingManagerBgRenderer::DoFindResults(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext, int FirstVisibleLine, int LastVisibleLine)
{
	Color Buffer = preferences::SettingsHolder::Get()->Appearance->BackColorFindResults;

	for each (auto Itr in FindResults)
	{
		if (!Itr->Enabled || !Itr->Valid)
			continue;
		else if (Itr->Line < FirstVisibleLine || Itr->Line > LastVisibleLine)
			continue;

		RenderBackground(textView, drawingContext,
						 Itr->StartOffset,
						 Itr->EndOffset,
						 CustomBrushes::Get()->FindResults, false);
	}
}

void LineTrackingManagerBgRenderer::DoBraceIndicators(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext, int FirstVisibleLine, int LastVisibleLine)
{
	if (!OpenCloseBraces->Enabled)
		return;
	else if (OpenCloseBraces->StartOffset == -1 && OpenCloseBraces->EndOffset == -1)
		return;
	else if (OpenCloseBraces->Line < FirstVisibleLine || OpenCloseBraces->Line > LastVisibleLine)
		return;

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

	delete Builder;
	HighlightGeometry->Freeze();

	auto HighlightColor = OpenCloseBraces->StartOffset == -1 || OpenCloseBraces->EndOffset == -1 ? CustomBrushes::Get()->InvalidBrace : CustomBrushes::Get()->ValidBrace;
	drawingContext->DrawGeometry(HighlightColor->Background, HighlightColor->Border, HighlightGeometry);
}

LineTrackingManagerBgRenderer::LineTrackingManagerBgRenderer(AvalonEdit::TextEditor^ Parent)
{
	ParentEditor = Parent;

	ErrorSquiggles = gcnew List<AnchoredDocumentSegment^>;
	FindResults = gcnew List<AnchoredDocumentSegment^>;
	OpenCloseBraces = gcnew StaticDocumentSegment;
}

LineTrackingManagerBgRenderer::~LineTrackingManagerBgRenderer()
{
	ParentEditor = nullptr;
}

void LineTrackingManagerBgRenderer::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
{
	textView->EnsureVisualLines();
	if (!textView->VisualLinesValid)
		return;

	auto FirstVisibleLine = textView->VisualLines[0]->FirstDocumentLine->LineNumber;
	auto LastVisibleLine = textView->VisualLines[textView->VisualLines->Count - 1]->FirstDocumentLine->LineNumber;

	DoCurrentLineBackground(textView, drawingContext, FirstVisibleLine, LastVisibleLine);
	DoErrorSquiggles(textView, drawingContext, FirstVisibleLine, LastVisibleLine);
	DoFindResults(textView, drawingContext, FirstVisibleLine, LastVisibleLine);
	DoBraceIndicators(textView, drawingContext, FirstVisibleLine, LastVisibleLine);

	String^ SelectionText = "";
	if (!ParentEditor->TextArea->Selection->IsEmpty)
		SelectionText = ParentEditor->TextArea->Selection->GetText()->Replace("\t", "")->Replace(" ", "")->Replace("\n", "")->Replace("\r\n", "");

	for (int i = FirstVisibleLine; i <= LastVisibleLine; ++i)
	{
		auto Line = ParentEditor->Document->GetLineByNumber(i);
		DoSelectedStringBackground(SelectionText, Line, textView, drawingContext);
	}
}

void LineTrackingManagerBgRenderer::Redraw()
{
	ParentEditor->TextArea->TextView->InvalidateLayer(Layer);
}

void LineTrackingManager::ParentEditor_TextChanged(Object^ Sender, EventArgs^ E)
{
	// disable compiler error indicators for the changed line
	int Caret = ParentEditor->TextArea->Caret->Offset;
	int Line = ParentEditor->Document->GetLineByOffset(Caret)->LineNumber;
	bool RefreshBg = false;

	for each (auto Error in LineBgRenderer->ErrorSquiggles)
	{
		if (!Error->Valid)
			continue;

		if (Error->Line == Line)
		{
			Error->Enabled = false;
			RefreshBg = true;
		}
	}

	// disable find result indicators for modified segments
	for each (auto% Segment in StartAnchorToFindResultSegments)
	{
		if (!Segment.Value->Valid)
			continue;

		if (Caret >= Segment.Value->StartOffset && Caret <= Segment.Value->EndOffset)
		{
			Segment.Value->Enabled = false;
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

		auto NewSegment = CreateAnchoredSegment(WhitespaceLeading->EndOffset, WhitespaceTrailing->Offset, true);
		ProcessedLines->Add(Itr->Line);
		LineBgRenderer->ErrorSquiggles->Add(NewSegment);
	}

	LineBgRenderer->Redraw();
}

void LineTrackingManager::TextAnchor_Deleted(Object^ Sender, EventArgs^ E)
{
	auto Anchor = safe_cast<TextAnchor^>(Sender);

	AnchorDeletionAccumulator->Add(Anchor);

	if (!DeferredAnchorDeletionTimer->Enabled)
		DeferredAnchorDeletionTimer->Start();
}

void LineTrackingManager::DeferredAnchorDeletionTimer_Tick(Object^ Sender, EventArgs^ E)
{
	DeferredAnchorDeletionTimer->Stop();

	if (AnchorDeletionAccumulator->Count == 0)
		return;

	int DeletedLineAnchors = 0;
	for each (auto Anchor in AnchorDeletionAccumulator)
	{
		Anchor->Deleted -= TextAnchorDeletedHandler;

		if (TrackedLineAnchors->ContainsKey(Anchor))
		{
			TrackedLineAnchors->Remove(Anchor);
			++DeletedLineAnchors;
		}

		AnchoredDocumentSegment^ FindSegment = nullptr;
		if (StartAnchorToFindResultSegments->TryGetValue(Anchor, FindSegment) || EndAnchorToFindResultSegments->TryGetValue(Anchor, FindSegment))
		{
			StartAnchorToFindResultSegments->Remove(FindSegment->StartAnchor);
			EndAnchorToFindResultSegments->Remove(FindSegment->EndAnchor);
		}
	}

	AnchorDeletionAccumulator->Clear();
	if (DeletedLineAnchors > 0)
		LineAnchorInvalidated(this, EventArgs::Empty);
}

LineTrackingManager::LineTrackingManager(AvalonEdit::TextEditor^ ParentEditor, model::IScriptDocument^ ParentScriptDocument)
{
	this->ParentEditor = ParentEditor;
	this->ParentScriptDocument = ParentScriptDocument;

	TrackedLineAnchors = gcnew Dictionary<TextAnchor^, LineAnchor^>;
	StartAnchorToFindResultSegments = gcnew Dictionary<TextAnchor^, AnchoredDocumentSegment^>;
	EndAnchorToFindResultSegments = gcnew Dictionary<TextAnchor^, AnchoredDocumentSegment^>;
	AnchorDeletionAccumulator = gcnew List<TextAnchor^>;
	LineBgRenderer = gcnew LineTrackingManagerBgRenderer(ParentEditor);
	DeferredAnchorDeletionTimer = gcnew Timer;

	DeferredAnchorDeletionTimer->Interval = 1000;
	DeferredAnchorDeletionTimer->Enabled = false;

	ParentModelStateChangedHandler = gcnew model::IScriptDocument::StateChangeEventHandler(this, &LineTrackingManager::ParentModel_StateChanged);
	ParentEditorTextChangedHandler = gcnew EventHandler(this, &LineTrackingManager::ParentEditor_TextChanged);
	TextAnchorDeletedHandler = gcnew EventHandler(this, &LineTrackingManager::TextAnchor_Deleted);
	DeferredAnchorDeletionTimerTickHandler = gcnew EventHandler(this, &LineTrackingManager::DeferredAnchorDeletionTimer_Tick);

	ParentEditor->TextArea->TextView->BackgroundRenderers->Add(LineBgRenderer);

	ParentEditor->TextChanged += ParentEditorTextChangedHandler;
	ParentScriptDocument->StateChanged += ParentModelStateChangedHandler;
	DeferredAnchorDeletionTimer->Tick += DeferredAnchorDeletionTimerTickHandler;
}

LineTrackingManager::~LineTrackingManager()
{
	DeferredAnchorDeletionTimer->Enabled = false;

	for each (auto Anchor in TrackedLineAnchors->Keys)
		Anchor->Deleted -= DeferredAnchorDeletionTimerTickHandler;

	for each (auto Anchor in StartAnchorToFindResultSegments->Keys)
		Anchor->Deleted -= DeferredAnchorDeletionTimerTickHandler;

	for each (auto Anchor in EndAnchorToFindResultSegments->Keys)
		Anchor->Deleted -= DeferredAnchorDeletionTimerTickHandler;

	TrackedLineAnchors->Clear();
	StartAnchorToFindResultSegments->Clear();
	EndAnchorToFindResultSegments->Clear();
	AnchorDeletionAccumulator->Clear();

	ParentEditor->TextArea->TextView->BackgroundRenderers->Remove(LineBgRenderer);

	ParentEditor->TextChanged -= ParentEditorTextChangedHandler;
	ParentScriptDocument->StateChanged -= ParentModelStateChangedHandler;
	DeferredAnchorDeletionTimer->Tick -= DeferredAnchorDeletionTimerTickHandler;

	SAFEDELETE_CLR(TextAnchorDeletedHandler);
	SAFEDELETE_CLR(ParentModelStateChangedHandler);
	SAFEDELETE_CLR(DeferredAnchorDeletionTimerTickHandler);
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

AnchoredDocumentSegment^ LineTrackingManager::CreateAnchoredSegment(int StartOffset, int EndOffset, bool AllowDeletion)
{
	auto StartAnchor = CreateAnchor(StartOffset, AllowDeletion);
	StartAnchor->Deleted += TextAnchorDeletedHandler;

	auto EndAnchor = CreateAnchor(EndOffset, AllowDeletion);
	EndAnchor->Deleted += TextAnchorDeletedHandler;

	auto NewSegment = gcnew AnchoredDocumentSegment(StartAnchor, EndAnchor);
	return NewSegment;
}

LineAnchor^ LineTrackingManager::CreateLineAnchor(UInt32 Line, bool AllowDeletion)
{
	auto NewLineAnchor = gcnew LineAnchor(ParentEditor, Line, AllowDeletion);
	NewLineAnchor->Anchor->Deleted += TextAnchorDeletedHandler;

	TrackedLineAnchors->Add(NewLineAnchor->Anchor, NewLineAnchor);
	return NewLineAnchor;
}

void LineTrackingManager::TrackFindResultSegment(UInt32 Start, UInt32 End)
{
	auto NewSegment = CreateAnchoredSegment(Start, End, true);

	StartAnchorToFindResultSegments->Add(NewSegment->StartAnchor, NewSegment);
	EndAnchorToFindResultSegments->Add(NewSegment->EndAnchor, NewSegment);

	LineBgRenderer->FindResults->Add(NewSegment);
	LineBgRenderer->Redraw();
}

void LineTrackingManager::ClearFindResultSegments()
{
	for each (auto % Segment in EndAnchorToFindResultSegments)
	{
		Segment.Value->StartAnchor->Deleted -= TextAnchorDeletedHandler;
		Segment.Value->EndAnchor->Deleted -= TextAnchorDeletedHandler;
	}

	StartAnchorToFindResultSegments->Clear();
	EndAnchorToFindResultSegments->Clear();

	LineBgRenderer->FindResults->Clear();
	LineBgRenderer->Redraw();
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
				return gcnew InlineObjectElement(0, GenerateAdornment(Block->StartLine, CurrentContext->Document->GetText(BlockStart)));
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
	static const int kMaxTextLength = 50;
	Font^ CustomFont = gcnew Font(preferences::SettingsHolder::Get()->Appearance->TextFont->FontFamily->Name,
								  preferences::SettingsHolder::Get()->Appearance->TextFont->Size - 1);

	ElementText = ElementText->Replace("\t", "");
	String^ TruncatedElementText = ElementText;
	if (TruncatedElementText->Length > kMaxTextLength)
		TruncatedElementText = TruncatedElementText->Substring(0, kMaxTextLength) + "...";

	AdornmentData^ Data = gcnew AdornmentData;
	Data->JumpLine = JumpLine;
	Data->Parent = ParentEditor;

	auto Panel = gcnew Windows::Controls::StackPanel();
	Panel->HorizontalAlignment = Windows::HorizontalAlignment::Center;
	Panel->VerticalAlignment = Windows::VerticalAlignment::Bottom;
	Panel->Orientation = Windows::Controls::Orientation::Horizontal;
	Panel->Margin = Windows::Thickness(20, 0, 20, 0);
	Panel->Cursor = Windows::Input::Cursors::Hand;
	Panel->Tag = Data;
	Panel->PreviewMouseDown += gcnew System::Windows::Input::MouseButtonEventHandler(OnMouseClick);

	auto AdornmentLabel = gcnew Windows::Controls::Label();
	AdornmentLabel->FontFamily = gcnew Windows::Media::FontFamily(CustomFont->FontFamily->Name);
	AdornmentLabel->FontSize = CustomFont->SizeInPoints * 96.f / 72.f;
	AdornmentLabel->FontStyle = Windows::FontStyles::Italic;
	AdornmentLabel->Foreground = CustomBrushes::Get()->ForeColorPartlyTransparent;
	AdornmentLabel->Background = CustomBrushes::Get()->Transparent;
	AdornmentLabel->Content = TruncatedElementText;
	AdornmentLabel->Padding = Windows::Thickness(0, 0, 0, 0);
	AdornmentLabel->Margin = Windows::Thickness(5, 0, 0, 0);
	AdornmentLabel->HorizontalAlignment = Windows::HorizontalAlignment::Center;
	AdornmentLabel->VerticalAlignment = Windows::VerticalAlignment::Bottom;
	auto AdornmentTooltip = gcnew Windows::Controls::ToolTip;
	AdornmentTooltip->FontFamily = gcnew Windows::Media::FontFamily(CustomFont->FontFamily->Name);
	AdornmentTooltip->FontSize = CustomFont->SizeInPoints * 96.f / 72.f;
	AdornmentTooltip->Foreground = CustomBrushes::Get()->ForeColor;
	AdornmentTooltip->Background = CustomBrushes::Get()->BackColor;
	AdornmentTooltip->BorderBrush = CustomBrushes::Get()->BackColor;
	AdornmentTooltip->Content = ElementText;
	Panel->Children->Add(AdornmentLabel);
	Panel->ToolTip = AdornmentTooltip;

	return Panel;
}

StructureVisualizerRenderer::~StructureVisualizerRenderer()
{
	ParentEditor = nullptr;
}

void IconMarginBase::OnHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	int Line = GetLineFromMousePosition(E);
	if (Line < 1)
		return;

	HandleHoverStart(Line, E);
}

void IconMarginBase::OnHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E)
{
	HandleHoverStop();
}

void IconMarginBase::OnTextViewChanged(AvalonEdit::Rendering::TextView^ oldTextView, AvalonEdit::Rendering::TextView^ newTextView)
{
	if (oldTextView)
		oldTextView->VisualLinesChanged -= HandlerTextViewChanged;

	AbstractMargin::OnTextViewChanged(oldTextView, newTextView);

	if (newTextView)
		newTextView->VisualLinesChanged += HandlerTextViewChanged;

	InvalidateVisual();
}

void IconMarginBase::OnRedrawRequested(Object^ sender, EventArgs^ E)
{
	if (this->TextView && this->TextView->VisualLinesValid)
		InvalidateVisual();
}

Windows::Media::HitTestResult^ IconMarginBase::HitTestCore(Windows::Media::PointHitTestParameters^ hitTestParameters)
{
	return gcnew Windows::Media::PointHitTestResult(this, hitTestParameters->HitPoint);
}

Windows::Size IconMarginBase::MeasureOverride(Windows::Size availableSize)
{
	return Windows::Size(24, 0);
}

void IconMarginBase::OnMouseDown(System::Windows::Input::MouseButtonEventArgs^ e)
{
	AbstractMargin::OnMouseDown(e);
	if (e->ChangedButton == System::Windows::Input::MouseButton::Left)
		e->Handled = true;
}

void IconMarginBase::OnMouseMove(System::Windows::Input::MouseEventArgs^ e)
{
	AbstractMargin::OnMouseMove(e);
}

void IconMarginBase::OnMouseUp(System::Windows::Input::MouseButtonEventArgs^ e)
{
	AbstractMargin::OnMouseUp(e);
	int line = GetLineFromMousePosition(e);
	if (e->ChangedButton == System::Windows::Input::MouseButton::Left && TextView)
		HandleClick(line);
}

void IconMarginBase::OnMouseLeave(System::Windows::Input::MouseEventArgs^ e)
{
	AbstractMargin::OnMouseLeave(e);
}

int IconMarginBase::GetLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e)
{
	VisualLine^ vl = GetVisualLineFromMousePosition(e);
	if (vl == nullptr)
		return 0;

	return vl->FirstDocumentLine->LineNumber;
}

VisualLine^ IconMarginBase::GetVisualLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e)
{
	AvalonEdit::Rendering::TextView^ textView = this->TextView;
	if (textView == nullptr)
		return nullptr;

	VisualLine^ vl = textView->GetVisualLineFromVisualTop(e->GetPosition(textView).Y + textView->ScrollOffset.Y);
	return vl;
}

IconMarginBase::IconMarginBase()
{
	HoverLogic = gcnew MouseHoverLogic(this);

	HandlerHover = gcnew EventHandler<System::Windows::Input::MouseEventArgs^>(this, &IconMarginBase::OnHover);
	HandlerHoverStopped = gcnew EventHandler<System::Windows::Input::MouseEventArgs^>(this, &IconMarginBase::OnHoverStopped);
	HandlerTextViewChanged = gcnew EventHandler(this, &IconMarginBase::OnRedrawRequested);

	HoverLogic->MouseHover += HandlerHover;
	HoverLogic->MouseHoverStopped += HandlerHoverStopped;
}

IconMarginBase::~IconMarginBase()
{
	this->TextView = nullptr;

	HoverLogic->MouseHover -= HandlerHover;
	HoverLogic->MouseHoverStopped -= HandlerHoverStopped;

	SAFEDELETE_CLR(HoverLogic);
}

Windows::Media::Imaging::BitmapSource^ DefaultIconMargin::GetWarningIcon()
{
	if (WarningIcon == nullptr)
		WarningIcon = WPFImageResourceGenerator::CreateImageSource("Warning");

	return WarningIcon;
}

Windows::Media::Imaging::BitmapSource^ DefaultIconMargin::GetErrorIcon()
{
	if (ErrorIcon == nullptr)
		ErrorIcon = WPFImageResourceGenerator::CreateImageSource("Error");

	return ErrorIcon;
}

Windows::Media::Imaging::BitmapSource^ DefaultIconMargin::GetBookmarkIcon()
{
	if (BookmarkIcon == nullptr)
		BookmarkIcon = WPFImageResourceGenerator::CreateImageSource("Bookmark");

	return BookmarkIcon;
}

void DefaultIconMargin::ParentModel_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E)
{
	switch (E->EventType)
	{
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Dirty:
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Messages:
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Bookmarks:
	case model::IScriptDocument::StateChangeEventArgs::eEventType::DisplayingPreprocessorOutput:
		this->InvalidateVisual();
		break;
	}
}

void DefaultIconMargin::OnRender(Windows::Media::DrawingContext^ drawingContext)
{
	auto BackgroundBrush = CustomBrushes::Get()->BackColor;

	drawingContext->DrawRectangle(BackgroundBrush, nullptr, Windows::Rect(0, 0, RenderSize.Width, RenderSize.Height));

	if (TextView == nullptr || !TextView->VisualLinesValid)
		return;

	auto PixelSize = PixelSnapHelpers::GetPixelSize(this);
	auto FirstVisibleLine = TextView->VisualLines[0]->FirstDocumentLine->LineNumber;
	auto LastVisibleLine = TextView->VisualLines[TextView->VisualLines->Count - 1]->FirstDocumentLine->LineNumber;
	auto LineAnnotationCounts = ParentScriptDocument->CountAnnotationsForLineRange(FirstVisibleLine, LastVisibleLine);

	for each (auto VisualLine in TextView->VisualLines)
	{
		auto VisualLineNumber = VisualLine->FirstDocumentLine->LineNumber;
		model::IScriptDocument::PerLineAnnotationCounts^ LineCounts = nullptr;
		if (LineAnnotationCounts->TryGetValue(VisualLineNumber, LineCounts))
		{
			Windows::Media::Imaging::BitmapSource^ Icon = nullptr;
			if (LineCounts->ErrorCount)
				Icon = GetErrorIcon();
			else if (LineCounts->WarningCount)
				Icon = GetWarningIcon();
			else if (LineCounts->BookmarkCount)
				Icon = GetBookmarkIcon();

			if (Icon == nullptr)
				continue;

			auto Width = 12;
			auto Height = 12;
			auto Opacity = 1.0;

			double LineMiddle = VisualLine->GetTextLineVisualYPosition(VisualLine->TextLines[0], VisualYPosition::TextMiddle) - TextView->VerticalOffset;
			Windows::Rect Rect((16 - Width) / 2, PixelSnapHelpers::Round(LineMiddle - Height/2, PixelSize.Height), Width, Height);
			drawingContext->PushOpacity(Opacity);
			drawingContext->DrawImage(Icon, Rect);
			drawingContext->Pop();
		}
	}

	AbstractMargin::OnRender(drawingContext);
}

void DefaultIconMargin::HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E)
{
	bool DisplayPopup = false;
	String^ PopupTitle = "";
	String^ PopupText = "";
	auto PopupBgColor = Color::Empty, PopupTextColor = Color::Empty;
	Image^ BodyImage = nullptr;

	Windows::Point DisplayLocation = TransformToPixels(E->GetPosition(ParentEditor));
	DisplayLocation.X += System::Windows::SystemParameters::CursorWidth;
	//DisplayLocation.Y += GetVisualLineFromMousePosition(E)->Height;
	DisplayLocation = ParentEditor->PointToScreen(DisplayLocation);

	auto Mb = gcnew utilities::TextMarkupBuilder;
	Mb->Table(1, 350);
	{
		if (ParentScriptDocument->GetMessageCountErrors(Line))
		{
			DisplayPopup = true;
			auto Errors = ParentScriptDocument->GetMessages(Line,
															model::components::ScriptDiagnosticMessage::eMessageSource::All,
															model::components::ScriptDiagnosticMessage::eMessageType::Error);

			PopupBgColor = preferences::SettingsHolder::Get()->Appearance->TooltipBackColorError;
			PopupTextColor = preferences::SettingsHolder::Get()->Appearance->TooltipForeColorError;
			BodyImage = view::components::CommonIcons::Get()->ErrorLarge;

			PopupTitle = Errors->Count + " Error" + (Errors->Count == 1 ? "" : "s");

			for each (auto Itr in Errors)
				Mb->TableNextRow()->TableNextColumn()->Font(1, true)->Text("» ")->Text(Itr->Text)->PopTag()->TableNextColumn();
		}
		else if (ParentScriptDocument->GetMessageCountWarnings(Line))
		{
			DisplayPopup = true;
			auto Warnings = ParentScriptDocument->GetMessages(Line,
															  model::components::ScriptDiagnosticMessage::eMessageSource::All,
															  model::components::ScriptDiagnosticMessage::eMessageType::Warning);

			PopupBgColor = preferences::SettingsHolder::Get()->Appearance->TooltipBackColorWarning;
			PopupTextColor = preferences::SettingsHolder::Get()->Appearance->TooltipForeColorWarning;
			BodyImage = view::components::CommonIcons::Get()->WarningLarge;

			PopupTitle = Warnings->Count + " Warning" + (Warnings->Count == 1 ? "" : "s");

			for each (auto Itr in Warnings)
				Mb->TableNextRow()->TableNextColumn()->Font(1, true)->Text("» ")->Text(Itr->Text)->PopTag()->TableNextColumn();
		}
		else if (ParentScriptDocument->GetBookmarkCount(Line))
		{
			DisplayPopup = true;
			auto Bookmarks = ParentScriptDocument->GetBookmarks(Line);

			PopupBgColor = preferences::SettingsHolder::Get()->Appearance->TooltipBackColorBookmark;
			PopupTextColor = preferences::SettingsHolder::Get()->Appearance->TooltipForeColorBookmark;
			BodyImage = nullptr;

			PopupTitle = Bookmarks->Count + " Bookmark" + (Bookmarks->Count == 1 ? "" : "s");

			for each (auto Itr in Bookmarks)
				Mb->TableNextRow()->TableNextColumn()->Font(1, true)->Text("» ")->Text(Itr->Text)->PopTag()->TableNextColumn();
		}
	}
	Mb->PopTable();

	if (DisplayPopup)
	{
		PopupText = Mb->ToMarkup();
		PopupTitle =  Mb->Reset()->Font(3, true)->Bold()->Text(PopupTitle)->PopTag(2)->ToMarkup();

		auto TooltipData = gcnew DotNetBar::SuperTooltipInfo;
		TooltipData->HeaderText = PopupTitle;
		TooltipData->BodyText = PopupText;
		TooltipData->BodyImage = BodyImage;
		TooltipData->Color = DevComponents::DotNetBar::eTooltipColor::System;

		auto TooltipColorSwapper = gcnew utilities::SuperTooltipColorSwapper(PopupTextColor, PopupBgColor);
		auto Control = Control::FromHandle(PopupParent);
		Popup->SetSuperTooltip(Control, TooltipData);
		TooltipColorSwapper->ShowTooltip(Popup, Control, Point(DisplayLocation.X, DisplayLocation.Y));
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


void ScriptBytecodeOffsetMargin::ParentModel_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E)
{
	switch (E->EventType)
	{
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Dirty:
	case model::IScriptDocument::StateChangeEventArgs::eEventType::Bytecode:
	case model::IScriptDocument::StateChangeEventArgs::eEventType::DisplayingPreprocessorOutput:
		this->InvalidateVisual();
		break;
	}
}

Windows::Size ScriptBytecodeOffsetMargin::MeasureOverride(Windows::Size availableSize)
{
	LineNumberMargin::MeasureOverride(availableSize);
	auto ForegroundBrush = safe_cast<System::Windows::Media::Brush^>(GetValue(Windows::Controls::Control::ForegroundProperty));
	auto FormattedText = gcnew Windows::Media::FormattedText("OOOO",
											 				 Globalization::CultureInfo::CurrentCulture, Windows::FlowDirection::LeftToRight,
															 typeface, emSize, ForegroundBrush,
															 nullptr, Windows::Media::TextOptions::GetTextFormattingMode(this));
	return Windows::Size(FormattedText->Width, 0);
}

void ScriptBytecodeOffsetMargin::OnRender(Windows::Media::DrawingContext^ drawingContext)
{
	if (TextView == nullptr || !TextView->VisualLinesValid)
		return;

	auto ForegroundBrush = safe_cast<System::Windows::Media::Brush^>(GetValue(Windows::Controls::Control::ForegroundProperty));
	for each (auto Line in TextView->VisualLines)
	{
		auto LineNo = Line->FirstDocumentLine->LineNumber;
		UInt16 LineOffset;
		auto LineOffsetText = gcnew String(' ', 4);
		if (ParentScriptDocument->GetBytecodeOffsetForScriptLine(LineNo, LineOffset))
		{
			if (LineOffset != 0xFFFF)
				LineOffsetText = LineOffset.ToString("X4");
		}
		else
			LineOffsetText = "ERR!";

		auto FormattedText = gcnew Windows::Media::FormattedText(LineOffsetText,
											 					 Globalization::CultureInfo::CurrentCulture, Windows::FlowDirection::LeftToRight,
																 typeface, emSize, ForegroundBrush,
																 nullptr, Windows::Media::TextOptions::GetTextFormattingMode(this));
		auto YPos = Line->GetTextLineVisualYPosition(Line->TextLines[0], ICSharpCode::AvalonEdit::Rendering::VisualYPosition::TextTop);
		drawingContext->DrawText(FormattedText, Windows::Point(RenderSize.Width - FormattedText->Width, YPos - TextView->VerticalOffset));
	}
}

ScriptBytecodeOffsetMargin::ScriptBytecodeOffsetMargin(model::IScriptDocument^ ParentScriptDocument)
{
	this->ParentScriptDocument = ParentScriptDocument;

	ParentModelStateChangedHandler = gcnew model::IScriptDocument::StateChangeEventHandler(this, &ScriptBytecodeOffsetMargin::ParentModel_StateChanged);
	this->ParentScriptDocument->StateChanged += ParentModelStateChangedHandler;
}

ScriptBytecodeOffsetMargin::~ScriptBytecodeOffsetMargin()
{
	ParentScriptDocument->StateChanged -= ParentModelStateChangedHandler;
	SAFEDELETE_CLR(ParentModelStateChangedHandler);

	ParentScriptDocument = nullptr;
}


void ScriptBytecodeOffsetMargin::AddToTextArea(AvalonEdit::TextEditor^ Field, ScriptBytecodeOffsetMargin^ Margin)
{
	auto FirstSeparator = safe_cast<Windows::Shapes::Line^>(DottedLineMargin::Create());
	auto SecondSeparator = safe_cast<Windows::Shapes::Line^>(DottedLineMargin::Create());

	int LineNumberMarginIdx = -1;
	for (int i = 0; i < Field->TextArea->LeftMargins->Count; ++i)
	{
		if (Field->TextArea->LeftMargins[i]->GetType() == LineNumberMargin::typeid)
		{
			LineNumberMarginIdx = i;
			break;
		}
	}

	Debug::Assert(LineNumberMarginIdx != -1);

	int InsertAtIdx = LineNumberMarginIdx;
	Field->TextArea->LeftMargins->Insert(InsertAtIdx, FirstSeparator);
	Field->TextArea->LeftMargins->Insert(InsertAtIdx + 1, Margin);
	Field->TextArea->LeftMargins->Insert(InsertAtIdx + 2, SecondSeparator);

	auto ForegroundBrush = CustomBrushes::Get()->ForeColor;
	auto ForegroundBinding = gcnew System::Windows::Data::Binding("LineNumbersForeground");
	ForegroundBinding->Source = Field;

	FirstSeparator->SetBinding(Windows::Shapes::Line::StrokeProperty, ForegroundBinding);
	SecondSeparator->SetBinding(Windows::Shapes::Line::StrokeProperty, ForegroundBinding);
	FirstSeparator->SetValue(Windows::Shapes::Line::StrokeProperty, ForegroundBrush);
	SecondSeparator->SetValue(Windows::Shapes::Line::StrokeProperty, ForegroundBrush);
}

void ScriptBytecodeOffsetMargin::RemoveFromTextArea(AvalonEdit::TextEditor^ Field, ScriptBytecodeOffsetMargin^ Margin)
{
	auto MarginIdx = Field->TextArea->LeftMargins->IndexOf(Margin);
	Debug::Assert(MarginIdx != -1);

	// 2 separators + 1 margin
	Field->TextArea->LeftMargins->RemoveAt(MarginIdx - 1);
	Field->TextArea->LeftMargins->RemoveAt(MarginIdx - 1);
	Field->TextArea->LeftMargins->RemoveAt(MarginIdx - 1);
}


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse