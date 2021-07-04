#pragma once

#include "ITextEditor.h"
#include "IScriptEditorModel.h"


namespace cse
{


namespace scriptEditor
{


namespace textEditor
{


namespace avalonEdit
{


using namespace DevComponents;
using namespace ICSharpCode;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;
using namespace ICSharpCode::AvalonEdit::Utils;


ref struct WPFImageResourceGenerator
{
public:
	static System::Windows::Media::Imaging::BitmapSource^ CreateImageSource(String^ ResourceIdentifier);
};

// lovely hack, this - http://apocryph.org/archives/275
ref class WPFFocusHelper
{
	static void DispatcherCallback(Windows::UIElement^ Element);
	static void ThreadCallback(Object^ Element);

	delegate void InvokeDelegate(Windows::UIElement^ Element);
public:
	static void Focus(Windows::UIElement^ Element);
};

System::Windows::Point TransformToPixels(double X, double Y);		// device-independent to pixels
System::Windows::Point TransformToPixels(System::Windows::Point In);

ref class AvalonEditTextEditor;
ref class LineTrackingManager;

ref class BracketSearchData
{
	Char Symbol;
	int StartOffset;
public:
	property int EndOffset;
	property bool Mismatching;

	static String^ ValidOpeningBrackets = "([{";
	static String^ ValidClosingBrackets = ")]}";

	static enum class eBracketType
	{
		Invalid = 0,
		Curved,
		Square,
		Squiggly
	};
	static enum class eBracketState
	{
		Invalid = 0,
		Opening,
		Closing
	};

	BracketSearchData(Char Symbol, int StartOffset);

	eBracketType GetType();
	eBracketState GetKind();
	int GetStartOffset();
};

ref struct LineAnchor : public ILineAnchor
{
	TextAnchor^ Anchor_;
public:
	LineAnchor(AvalonEdit::TextEditor^ Parent, UInt32 Line, bool AllowDeletion);

	virtual property UInt32 Line
	{
		UInt32 get() { return Anchor_->Line; }
		void set(UInt32 s) {}
	}
	virtual property bool Valid
	{
		bool get() { return !Anchor_->IsDeleted; }
		void set(bool s) {}
	}
	property TextAnchor^ Anchor
	{
		TextAnchor^ get() { return Anchor_; }
	}
};

ref struct StaticDocumentSegment
{
	property UInt32 Line;
	property int StartOffset;
	property int EndOffset;
	property bool Enabled;

	StaticDocumentSegment();
};

ref class AnchoredDocumentSegment
{
	TextAnchor^ StartOffsetAnchor;
	TextAnchor^ EndOffsetAnchor;
public:
	AnchoredDocumentSegment(TextAnchor^ Start, TextAnchor^ End);

	property UInt32 Line
	{
		UInt32 get() { return StartOffsetAnchor->Line; }
	}
	property int StartOffset
	{
		int get() { return StartOffsetAnchor->Offset; }
	}
	property int EndOffset
	{
		int get() { return EndOffsetAnchor->Offset; }
	}
	property bool Enabled;
	property bool Valid
	{
		bool get() { return !StartOffsetAnchor->IsDeleted && !EndOffsetAnchor->IsDeleted; }
	}

	property TextAnchor^ StartAnchor
	{
		TextAnchor^ get() { return StartOffsetAnchor; }
	}
	property TextAnchor^ EndAnchor
	{
		TextAnchor^ get() { return EndOffsetAnchor; }
	}
};

ref class LineTrackingManagerBgRenderer : public AvalonEdit::Rendering::IBackgroundRenderer
{
	AvalonEdit::TextEditor^ ParentEditor;


	void RenderBackground(TextView^ Destination,
						  System::Windows::Media::DrawingContext^ DrawingContext,
						  int StartOffset, int EndOffset,
						  Windows::Media::Color Background,
						  Windows::Media::Color Border,
						  Double BorderThickness,
						  bool ColorEntireLine);
	void RenderSquiggle(TextView^ Destination,
					 	System::Windows::Media::DrawingContext^ DrawingContext,
					 	int StartOffset, int EndOffset,
					 	Windows::Media::Color Color);

	void DoCurrentLineBackground(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
	void DoLineLimitBackground(DocumentLine^ Line, TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
	void DoSelectedStringBackground(String^ SelectionText, DocumentLine^ Line, TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
	void DoErrorSquiggles(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
	void DoFindResults(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
	void DoBraceIndicators(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
public:
	LineTrackingManagerBgRenderer(AvalonEdit::TextEditor^ Parent);
	~LineTrackingManagerBgRenderer();

	property List<AnchoredDocumentSegment^>^ ErrorSquiggles;
	property List<AnchoredDocumentSegment^>^ FindResults;
	property StaticDocumentSegment^ OpenCloseBraces;
	property KnownLayer Layer
	{
		virtual KnownLayer get() { return KnownLayer::Background; }
	}

	virtual void Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
	void Redraw();
};


ref class LineTrackingManager
{
	AvalonEdit::TextEditor^ ParentEditor;
	model::IScriptDocument^ ParentScriptDocument;
	Dictionary<TextAnchor^, LineAnchor^>^ TrackedLineAnchors;
	Dictionary<TextAnchor^, AnchoredDocumentSegment^>^ StartAnchorToFindResultSegments;
	Dictionary<TextAnchor^, AnchoredDocumentSegment^>^ EndAnchorToFindResultSegments;
	List<TextAnchor^>^ AnchorDeletionAccumulator;
	LineTrackingManagerBgRenderer^ LineBgRenderer;
	Timer^ DeferredAnchorDeletionTimer;

	model::IScriptDocument::StateChangeEventHandler^ ParentModelStateChangedHandler;
	EventHandler^ ParentEditorTextChangedHandler;
	EventHandler^ TextAnchorDeletedHandler;
	EventHandler^ DeferredAnchorDeletionTimerTickHandler;

	void ParentEditor_TextChanged(Object^ Sender, EventArgs^ E);
	void ParentModel_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E);
	void TextAnchor_Deleted(Object^ Sender, EventArgs^ E);
	void DeferredAnchorDeletionTimer_Tick(Object^ Sender, EventArgs^ E);

	TextAnchor^ CreateAnchor(int Offset, bool AllowDeletion);
	AnchoredDocumentSegment^ CreateAnchoredSegment(int StartOffset, int EndOffset, bool AllowDeletion);
public:
	LineTrackingManager(AvalonEdit::TextEditor^ ParentEditor, model::IScriptDocument^ ParentScriptDocument);
	~LineTrackingManager();

	event EventHandler^ LineAnchorInvalidated;

	property LineTrackingManagerBgRenderer^ LineBackgroundRenderer
	{
		LineTrackingManagerBgRenderer^ get() { return LineBgRenderer; }
	}

	LineAnchor^ CreateLineAnchor(UInt32 Line, bool AllowDeletion);
	void TrackFindResultSegment(UInt32 Start, UInt32 End);
	void ClearFindResultSegments();
};


ref class ObScriptIndentStrategy : public AvalonEdit::Indentation::IIndentationStrategy
{
	AvalonEditTextEditor^ Parent;
	bool TrimTrailingWhitespace;
	bool CullEmptyLines;

	void CalculateIndentsTillCurrentLine(AvalonEdit::Document::TextDocument^ Document, AvalonEdit::Document::DocumentLine^ CurrentLine,
											UInt32% OutCurrentLineIndents, UInt32% OutPreviousLineIndents);
public:
	ObScriptIndentStrategy(AvalonEditTextEditor^ Parent, bool TrimTrailingWhitespace, bool CullEmptyLines);
	virtual ~ObScriptIndentStrategy();

	virtual void IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line);
	virtual void IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine);

};

ref class ObScriptCodeFoldingStrategy
{
	ref class FoldingSorter : public IComparer<AvalonEdit::Folding::NewFolding^>
	{
	public:
		virtual int Compare(AvalonEdit::Folding::NewFolding^ X, AvalonEdit::Folding::NewFolding^ Y);
	};

	AvalonEditTextEditor^ Parent;
	FoldingSorter^ Sorter;
public:
	ObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent);
	virtual ~ObScriptCodeFoldingStrategy();

	virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^ CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset);
};

ref struct TagableDoubleAnimation : public System::Windows::Media::Animation::DoubleAnimation
{
	TagableDoubleAnimation(double fromValue, double toValue, System::Windows::Duration duration, System::Windows::Media::Animation::FillBehavior fillBehavior);

	property Object^ Tag;
};

ref class StructureVisualizerRenderer : public VisualLineElementGenerator
{
protected:
	static void	OnMouseClick(Object^ Sender, Windows::Input::MouseButtonEventArgs^ E);

	static Windows::Media::Imaging::BitmapSource^ ElementIcon = nullptr;
	static int InstanceCounter = 0;

	static Windows::Media::Imaging::BitmapSource^ GetIconSource();

	ref class AdornmentData
	{
	public:
		UInt32 JumpLine;
		AvalonEditTextEditor^ Parent;
	};

	AvalonEditTextEditor^ ParentEditor;

	Windows::UIElement^	GenerateAdornment(UInt32 JumpLine, String^ ElementText);
public:
	StructureVisualizerRenderer(AvalonEditTextEditor^ Parent);
	virtual ~StructureVisualizerRenderer();

	virtual int	GetFirstInterestedOffset(Int32 startOffset) override;
	virtual VisualLineElement^ ConstructElement(Int32 offset) override;
};

// derived from ICSharpCode.AvalonEdit.AddIn.IconBarMargin
ref class IconMargin : public AbstractMargin
{
protected:
	MouseHoverLogic^ HoverLogic;

	EventHandler<System::Windows::Input::MouseEventArgs^>^ HandlerHover;
	EventHandler<System::Windows::Input::MouseEventArgs^>^ HandlerHoverStopped;
	EventHandler^ HandlerTextViewChanged;

	void OnHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void OnHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
	void OnRedrawRequested(Object^ sender, EventArgs^ E);

	virtual void OnTextViewChanged(AvalonEdit::Rendering::TextView^ oldTextView, AvalonEdit::Rendering::TextView^ newTextView) override;
	virtual Windows::Media::HitTestResult^ HitTestCore(Windows::Media::PointHitTestParameters^ hitTestParameters) override;
	virtual Windows::Size MeasureOverride(Windows::Size availableSize) override;
	virtual void OnRender(Windows::Media::DrawingContext^ drawingContext) override;
	virtual void OnMouseDown(System::Windows::Input::MouseButtonEventArgs^ e) override;
	virtual void OnMouseMove(System::Windows::Input::MouseEventArgs^ e) override;
	virtual void OnMouseUp(System::Windows::Input::MouseButtonEventArgs^ e) override;
	virtual void OnMouseLeave(System::Windows::Input::MouseEventArgs^ e) override;

	int			 GetLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e);
	VisualLine^	 GetVisualLineFromMousePosition(System::Windows::Input::MouseEventArgs^ e);

	virtual void HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E) abstract;
	virtual void HandleHoverStop() abstract;
	virtual void HandleClick(int Line) abstract;
	virtual bool GetRenderData(int Line,
							   Windows::Media::Imaging::BitmapSource^% OutIcon,
							   double% OutOpacity,
							   int% Width,
							   int% Height) abstract;		// return false to skip rendering the line
public:
	IconMargin();
	virtual ~IconMargin();
};

ref class DefaultIconMargin : public IconMargin
{
	static int InstanceCounter = 0;

	static Windows::Media::Imaging::BitmapSource^ WarningIcon = nullptr;
	static Windows::Media::Imaging::BitmapSource^ ErrorIcon = nullptr;
	static Windows::Media::Imaging::BitmapSource^ BookmarkIcon = nullptr;

	static Windows::Media::Imaging::BitmapSource^ GetWarningIcon();
	static Windows::Media::Imaging::BitmapSource^ GetErrorIcon();
	static Windows::Media::Imaging::BitmapSource^ GetBookmarkIcon();
protected:
	AvalonEdit::TextEditor^ ParentEditor;
	model::IScriptDocument^ ParentScriptDocument;
	DotNetBar::SuperTooltip^ Popup;
	IntPtr PopupParent;
	model::IScriptDocument::StateChangeEventHandler^ ParentModelStateChangedHandler;

	void ParentModel_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E);

	virtual void HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E) override;
	virtual void HandleHoverStop() override;
	virtual void HandleClick(int Line) override;
	virtual bool GetRenderData(int Line,
							   Windows::Media::Imaging::BitmapSource^% OutIcon,
							   double% OutOpacity,
							   int% Width,
							   int% Height) override;
public:
	DefaultIconMargin(AvalonEdit::TextEditor^ ParentEditor, model::IScriptDocument^ ParentScriptDocument, IntPtr ToolTipParent);
	virtual ~DefaultIconMargin();
};

ref class ScriptBytecodeOffsetMargin : public ICSharpCode::AvalonEdit::Editing::LineNumberMargin
{
protected:
	model::IScriptDocument^ ParentScriptDocument;
	model::IScriptDocument::StateChangeEventHandler^ ParentModelStateChangedHandler;

	void ParentModel_StateChanged(Object^ Sender, model::IScriptDocument::StateChangeEventArgs^ E);

	virtual Windows::Size MeasureOverride(Windows::Size availableSize) override;
	virtual void OnRender(Windows::Media::DrawingContext^ drawingContext) override;
public:
	ScriptBytecodeOffsetMargin(model::IScriptDocument^ ParentScriptDocument);
	virtual ~ScriptBytecodeOffsetMargin();

	static void AddToTextArea(AvalonEdit::TextEditor^ Field, ScriptBytecodeOffsetMargin^ Margin);
	static void RemoveFromTextArea(AvalonEdit::TextEditor^ Field, ScriptBytecodeOffsetMargin^ Margin);
};


} // namespace avalonEdit


} // namespace textEditor


} // namespace scriptEditor


} // namespace cse