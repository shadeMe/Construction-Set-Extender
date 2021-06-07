#pragma once

#include "[Common]/ListViewUtilities.h"
#include "ScriptTextEditorInterface.h"
#include "WorkspaceModelInterface.h"

using namespace ICSharpCode;
using namespace ICSharpCode::AvalonEdit::Rendering;
using namespace ICSharpCode::AvalonEdit::Document;
using namespace ICSharpCode::AvalonEdit::Editing;
using namespace ICSharpCode::AvalonEdit::Utils;

/* http://danielgrunwald.de/coding/AvalonEdit/rendering.php */

namespace cse
{
	ref class WPFImageResourceGenerator
	{
	public:
		static System::Windows::Media::Imaging::BitmapSource^		CreateImageSource(String^ ResourceIdentifier);
	};

	// lovely hack, this - http://apocryph.org/archives/275
	ref class WPFFocusHelper
	{
		static void		DispatcherCallback(Windows::UIElement^ Element);
		static void		ThreadCallback(Object^ Element);

		delegate void	InvokeDelegate(Windows::UIElement^ Element);
	public:
		static void		Focus(Windows::UIElement^ Element);
	};

	namespace textEditors
	{
		namespace avalonEditor
		{
			using namespace DevComponents;

			System::Windows::Point TransformToPixels(double X, double Y);		// device-independent to pixels
			System::Windows::Point TransformToPixels(System::Windows::Point In);

			ref class AvalonEditTextEditor;
			ref class LineTrackingManager;

			ref struct BackgroundRenderSegment
			{
				property UInt32 Line;
				property int StartOffset;
				property int EndOffset;
				property bool Enabled;

				BackgroundRenderSegment()
				{
					Line = 0;
					StartOffset = 0;
					EndOffset = 0;
					Enabled = true;
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


				property List<BackgroundRenderSegment^>^ ErrorSquiggles;
				property List<BackgroundRenderSegment^>^ FindResults;
				property BackgroundRenderSegment^		 OpenCloseBraces;
				property KnownLayer Layer
				{
					virtual KnownLayer get() { return KnownLayer::Background; }
				}

				virtual void Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext);
				void Redraw();
			};


			ref struct AvalonEditLineAnchor : public ILineAnchor
			{
				TextAnchor^ Anchor;
			public:
				AvalonEditLineAnchor(AvalonEdit::TextEditor^ Parent, UInt32 Line, bool AllowDeletion);

				virtual property UInt32 Line
				{
					UInt32 get() { return Anchor->Line; }
					void set(UInt32 s) {}
				}
				virtual property bool Valid
				{
					bool get() { return !Anchor->IsDeleted; }
					void set(bool s) {}
				}
			};

			ref class LineTrackingManager
			{
				ref struct FindResultSegment
				{
					TextAnchor^ Start;
					TextAnchor^ End;
				};

				AvalonEdit::TextEditor^		ParentEditor;
				scriptEditor::IWorkspaceModel^
											ParentModel;
				List<AvalonEditLineAnchor^>^
											TrackedLineAnchors;
				Dictionary<FindResultSegment^, BackgroundRenderSegment^>^
											TrackedFindResultSegments;
				LineTrackingManagerBgRenderer^
											LineBgRenderer;
				scriptEditor::IWorkspaceModel::StateChangeEventHandler^
											ParentModelStateChangedHandler;
				EventHandler^				ParentEditorTextChangedHandler;


				void		ParentEditor_TextChanged(Object^ Sender, EventArgs^ E);
				void		ParentModel_StateChanged(scriptEditor::IWorkspaceModel^ Sender, scriptEditor::IWorkspaceModel::StateChangeEventArgs^ E);

				TextAnchor^ CreateAnchor(int Offset, bool AllowDeletion);
			public:
				LineTrackingManager(AvalonEdit::TextEditor^ ParentEditor, scriptEditor::IWorkspaceModel^ ParentModel);
				~LineTrackingManager();

				property LineTrackingManagerBgRenderer^ LineBackgroundRenderer
				{
					LineTrackingManagerBgRenderer^ get() { return LineBgRenderer; }
				}

				AvalonEditLineAnchor^	CreateLineAnchor(UInt32 Line, bool AllowDeletion);
				void					TrackFindResultSegment(UInt32 Start, UInt32 End);
				void					ClearFindResultSegments();
				bool					RemoveDeletedLineAnchors(); // returns true if any line anchors were removed
			};


			ref class ObScriptIndentStrategy : public AvalonEdit::Indentation::IIndentationStrategy
			{
				AvalonEditTextEditor^
						Parent;
				bool	TrimTrailingWhitespace;
				bool	CullEmptyLines;

				void	CalculateIndentsTillCurrentLine(AvalonEdit::Document::TextDocument^ Document, AvalonEdit::Document::DocumentLine^ CurrentLine,
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
				FoldingSorter^		  Sorter;
			public:
				ObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent);
				virtual ~ObScriptCodeFoldingStrategy();

				virtual IEnumerable<AvalonEdit::Folding::NewFolding^>^ CreateNewFoldings(AvalonEdit::Document::TextDocument^ document, int% firstErrorOffset);
			};

			ref struct TagableDoubleAnimation : public System::Windows::Media::Animation::DoubleAnimation
			{
				TagableDoubleAnimation(double fromValue, double toValue, System::Windows::Duration duration, System::Windows::Media::Animation::FillBehavior fillBehavior)
					: DoubleAnimation(fromValue, toValue, duration, fillBehavior) {}

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

				EventHandler<System::Windows::Input::MouseEventArgs^>^		HandlerHover;
				EventHandler<System::Windows::Input::MouseEventArgs^>^		HandlerHoverStopped;
				EventHandler^												HandlerTextViewChanged;

				void OnHover(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
				void OnHoverStopped(Object^ Sender, System::Windows::Input::MouseEventArgs^ E);
				void OnRedrawRequested(Object^ sender, EventArgs^ E);

				virtual void OnTextViewChanged(AvalonEdit::Rendering::TextView^ oldTextView,
																			  AvalonEdit::Rendering::TextView^ newTextView) override;
				virtual Windows::Media::HitTestResult^
							HitTestCore(Windows::Media::PointHitTestParameters^ hitTestParameters) override;
				virtual Windows::Size
							MeasureOverride(Windows::Size availableSize) override;
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
				AvalonEdit::TextEditor^		ParentEditor;
				scriptEditor::IWorkspaceModel^
											ParentModel;
				DotNetBar::SuperTooltip^	Popup;
				IntPtr						PopupParent;
				scriptEditor::IWorkspaceModel::StateChangeEventHandler^
											ParentModelStateChangedHandler;

				void ParentModel_StateChanged(scriptEditor::IWorkspaceModel^ Sender, scriptEditor::IWorkspaceModel::StateChangeEventArgs^ E);

				virtual void HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E) override;
				virtual void HandleHoverStop() override;
				virtual void HandleClick(int Line) override;
				virtual bool GetRenderData(int Line,
							 			Windows::Media::Imaging::BitmapSource^% OutIcon,
							 			double% OutOpacity,
							 			int% Width,
							 			int% Height) override;
			public:
				DefaultIconMargin(AvalonEdit::TextEditor^ ParentEditor, scriptEditor::IWorkspaceModel^ ParentModel, IntPtr ToolTipParent);
				virtual ~DefaultIconMargin();
			};
		}
	}
}