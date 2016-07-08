#include "AvalonEditComponents.h"
#include "AvalonEditTextEditor.h"
#include "ScriptEditorPreferences.h"
#include "Globals.h"

namespace cse
{
	System::Windows::Media::Imaging::BitmapSource^ WPFImageResourceGenerator::CreateImageSource(String^ ResourceIdentifier)
	{
		Drawing::Bitmap^ OrgResource = (Drawing::Bitmap^)Globals::ScriptEditorImageResourceManager->CreateImage(ResourceIdentifier);
		System::Windows::Media::Imaging::BitmapSource^ Result = nullptr;

		try
		{
			Result = System::Windows::Interop::Imaging::CreateBitmapSourceFromHBitmap(OrgResource->GetHbitmap(),
																					  IntPtr::Zero,
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
		UIE->Dispatcher->Invoke(System::Windows::Threading::DispatcherPriority::Normal, gcnew InvokeDelegate(&DispatcherCallback), UIE);
	}

	void WPFFocusHelper::Focus(Windows::UIElement^ Element)
	{
		System::Threading::ThreadPool::QueueUserWorkItem(gcnew System::Threading::WaitCallback(&ThreadCallback), Element);
	}

	namespace textEditors
	{
		namespace avalonEditor
		{
			void ILineBackgroundColorizer::RenderBackground(TextView^ Destination,
															System::Windows::Media::DrawingContext^ DrawingContext,
															int StartOffset,
															int EndOffset,
															Windows::Media::Color Background,
															Windows::Media::Color Border,
															Double BorderThickness,
															bool ColorEntireLine)
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

			ILineBackgroundColorizer::ILineBackgroundColorizer(AvalonEdit::TextEditor^ Parent, KnownLayer RenderLayer) :
				ParentEditor(Parent),
				RenderLayer(RenderLayer)
			{
				;//
			}

			ScriptMessage::ScriptMessage(LineTrackingManager^ Parent, TextAnchor^ Location,
										 IScriptTextEditor::ScriptMessageType Type,
										 IScriptTextEditor::ScriptMessageSource Source,
										 String^ Text)
			{
				Manager = Parent;
				Anchor = Location;
				MessageType = Type;
				MessageSource = Source;
				MessageString = Text;
				IndicatorDisabled = false;
			}

			int ScriptMessage::Line()
			{
				return Anchor->Line;
			}

			String^ ScriptMessage::Message()
			{
				return MessageString;
			}

			int ScriptMessage::ImageIndex()
			{
				return (int)MessageType;
			}

			void ScriptMessage::Jump()
			{
				Manager->Jump(this);
			}

			ScriptMessage::~ScriptMessage()
			{
				Manager = nullptr;
			}

			IScriptTextEditor::ScriptMessageSource ScriptMessage::Source()
			{
				return MessageSource;
			}

			bool ScriptMessage::Deleted()
			{
				return Anchor->IsDeleted;
			}

			IScriptTextEditor::ScriptMessageType ScriptMessage::Type()
			{
				return MessageType;
			}

			ScriptBookmark::ScriptBookmark(LineTrackingManager^ Parent, TextAnchor^ Location, String^ Text)
			{
				Manager = Parent;
				Anchor = Location;
				Description = Text;
			}

			int ScriptBookmark::Line()
			{
				if (Anchor->IsDeleted)
					return 0;
				else
					return Anchor->Line;
			}

			String^ ScriptBookmark::Message()
			{
				return Description;
			}

			void ScriptBookmark::Jump()
			{
				Manager->Jump(this);
			}

			ScriptBookmark::~ScriptBookmark()
			{
				Manager = nullptr;
			}

			bool ScriptBookmark::Deleted()
			{
				return Anchor->IsDeleted;
			}

			ScriptFindResult::ScriptFindResult(LineTrackingManager^ Parent, TextAnchor^ Start, TextAnchor^ End, String^ Text)
			{
				Manager = Parent;
				AnchorStart = Start;
				AnchorEnd = End;
				Description = Text;
				IndicatorDisabled = false;
			}

			int ScriptFindResult::Line()
			{
				return AnchorStart->Line;
			}

			String^ ScriptFindResult::Message()
			{
				return Description;
			}

			void ScriptFindResult::Jump()
			{
				Manager->Jump(this);
			}

			ScriptFindResult::~ScriptFindResult()
			{
				Manager = nullptr;
			}

			bool ScriptFindResult::Deleted()
			{
				return (AnchorStart->IsDeleted || AnchorEnd->IsDeleted);
			}

			int ScriptFindResult::StartOffset()
			{
				return AnchorStart->Offset;
			}

			int ScriptFindResult::EndOffset()
			{
				return AnchorEnd->Offset;
			}

			int TrackingMessageSorter::Compare(TrackingMessage^ X, TrackingMessage^ Y)
			{
				switch (CompareField)
				{
				case cse::textEditors::avalonEditor::TrackingMessageSorter::ComparisonField::Line:
					return X->Line() < Y->Line();
				case cse::textEditors::avalonEditor::TrackingMessageSorter::ComparisonField::Message:
					return String::Compare(X->Message(), Y->Message(), true);
				case cse::textEditors::avalonEditor::TrackingMessageSorter::ComparisonField::ImageIndex:
					{
						TrackingImageMessage^ XImage = (TrackingImageMessage^)X;
						TrackingImageMessage^ YImage = (TrackingImageMessage^)Y;

						if (XImage == nullptr || YImage == nullptr)
							throw gcnew ArgumentException("Object is not a TrackingImageMessage");

						return XImage->ImageIndex() < YImage->ImageIndex();
					}
				default:
					return 0;
				}
			}

			int TrackingMessageListViewSorter::Compare(Object^ X, Object^ Y)
			{
				int Result = -1;

				TrackingMessage^ First = (TrackingMessage^)((ListViewItem^)X)->Tag;
				TrackingMessage^ Second = (TrackingMessage^)((ListViewItem^)Y)->Tag;

				switch (_Column)
				{
				case 0:		// line
					Result = First->Line() - Second->Line();
					break;
				case 1:		// message
					Result = String::Compare(First->Message(), Second->Message(), true);
					break;
				}

				if (_Order == SortOrder::Descending)
					Result *= -1;

				return Result;
			}

			int TrackingImageMessageListViewSorter::Compare(Object^ X, Object^ Y)
			{
				int Result = -1;

				TrackingImageMessage^ First = (TrackingImageMessage^)((ListViewItem^)X)->Tag;
				TrackingImageMessage^ Second = (TrackingImageMessage^)((ListViewItem^)Y)->Tag;

				switch (_Column)
				{
				case 0:		// image/dummy column
					Result = First->ImageIndex() - Second->ImageIndex();
					break;
				case 1:		// line
					Result = First->Line() - Second->Line();
					break;
				case 2:		// message
					Result = String::Compare(First->Message(), Second->Message(), true);
					break;
				}

				if (_Order == SortOrder::Descending)
					Result *= -1;

				return Result;
			}

			void ScriptBookmarkBinder::InitializeListView(ListView^ Control)
			{
				ColumnHeader^ Line = gcnew ColumnHeader();
				Line->Text = "Line";
				Line->Width = 50;
				ColumnHeader^ Message = gcnew ColumnHeader();
				Message->Text = "Description";
				Message->Width = 720;

				Control->Columns->Add(Line);
				Control->Columns->Add(Message);
			}

			System::Collections::IComparer^ ScriptBookmarkBinder::GetSorter(int Column, SortOrder Order)
			{
				return gcnew TrackingMessageListViewSorter(Column, Order);
			}

			int ScriptBookmarkBinder::GetImageIndex(ScriptBookmark^ Item)
			{
				return -1;
			}

			void ScriptBookmarkBinder::ActivateItem(ScriptBookmark^ Item)
			{
				Item->Jump();
			}

			void ScriptBookmarkBinder::KeyPress(KeyEventArgs^ E)
			{
				if (E->KeyCode == Keys::Delete)
				{
					ListViewItem^ Selection = GetListViewSelectedItem(Sink);
					if (Selection)
					{
						ScriptBookmark^ Data = (ScriptBookmark^)Selection->Tag;
						Source->Remove(Data);
					}
				}
			}

			UInt32 ScriptBookmarkBinder::GetColumnCount()
			{
				return 2;
			}

			String^ ScriptBookmarkBinder::GetSubItemText(ScriptBookmark^ Item, int Column)
			{
				switch (Column)
				{
				case 0:
					return Item->Line().ToString();
				case 1:
					return Item->Message();
				default:
					return "<unknown>";
				}
			}

			UInt32 ScriptBookmarkBinder::GetDefaultSortColumn()
			{
				return 0;
			}

			System::Windows::Forms::SortOrder ScriptBookmarkBinder::GetDefaultSortOrder()
			{
				return SortOrder::Ascending;
			}

			void ScriptMessageBinder::InitializeListView(ListView^ Control)
			{
				ColumnHeader^ Dummy = gcnew ColumnHeader();
				Dummy->Text = " ";
				Dummy->Width = 25;
				ColumnHeader^ Line = gcnew ColumnHeader();
				Line->Text = "Line";
				Line->Width = 50;
				ColumnHeader^ Message = gcnew ColumnHeader();
				Message->Text = "Message";
				Message->Width = 720;

				Control->Columns->Add(Dummy);
				Control->Columns->Add(Line);
				Control->Columns->Add(Message);

				Control->SmallImageList = gcnew ImageList();
				Control->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("MessageListWarning"));
				Control->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImage("MessageListError"));
			}

			System::Collections::IComparer^ ScriptMessageBinder::GetSorter(int Column, SortOrder Order)
			{
				return gcnew TrackingImageMessageListViewSorter(Column, Order);
			}

			int ScriptMessageBinder::GetImageIndex(ScriptMessage^ Item)
			{
				return Item->ImageIndex();
			}

			void ScriptMessageBinder::ActivateItem(ScriptMessage^ Item)
			{
				Item->Jump();
			}

			void ScriptMessageBinder::KeyPress(KeyEventArgs^ E)
			{
				;//
			}

			UInt32 ScriptMessageBinder::GetColumnCount()
			{
				return 3;
			}

			String^ ScriptMessageBinder::GetSubItemText(ScriptMessage^ Item, int Column)
			{
				switch (Column)
				{
				case 0:
					return "";
				case 1:
					return Item->Line().ToString();
				case 2:
					return Item->Message();
				default:
					return "<unknown>";
				}
			}

			UInt32 ScriptMessageBinder::GetDefaultSortColumn()
			{
				return 0;
			}

			System::Windows::Forms::SortOrder ScriptMessageBinder::GetDefaultSortOrder()
			{
				// errors first
				return SortOrder::Descending;
			}

			bool ScriptFindResultBinder::HasLine(ScriptFindResult^ Check)
			{
				for each (ListViewItem^ Itr in Sink->Items)
				{
					ScriptFindResult^ Data = (ScriptFindResult^)Itr->Tag;
					if (Data->Line() == Check->Line())
						return true;
				}

				return false;
			}

			ListViewItem^ ScriptFindResultBinder::Create(ScriptFindResult^ Data)
			{
				if (HasLine(Data) == false)
					return SimpleListViewBinder::Create(Data);
				else
					return nullptr;
			}

			void ScriptFindResultBinder::InitializeListView(ListView^ Control)
			{
				ColumnHeader^ Line = gcnew ColumnHeader();
				Line->Text = "Line";
				Line->Width = 50;
				ColumnHeader^ Message = gcnew ColumnHeader();
				Message->Text = "Result";
				Message->Width = 720;

				Control->Columns->Add(Line);
				Control->Columns->Add(Message);
			}

			System::Collections::IComparer^ ScriptFindResultBinder::GetSorter(int Column, SortOrder Order)
			{
				return gcnew TrackingMessageListViewSorter(Column, Order);
			}

			int ScriptFindResultBinder::GetImageIndex(ScriptFindResult^ Item)
			{
				return -1;
			}

			void ScriptFindResultBinder::ActivateItem(ScriptFindResult^ Item)
			{
				Item->Jump();
			}

			void ScriptFindResultBinder::KeyPress(KeyEventArgs^ E)
			{
				;//
			}

			UInt32 ScriptFindResultBinder::GetColumnCount()
			{
				return 2;
			}

			String^ ScriptFindResultBinder::GetSubItemText(ScriptFindResult^ Item, int Column)
			{
				switch (Column)
				{
				case 0:
					return Item->Line().ToString();
				case 1:
					return Item->Message();
				default:
					return "<unknown>";
				}
			}

			UInt32 ScriptFindResultBinder::GetDefaultSortColumn()
			{
				return 0;
			}

			System::Windows::Forms::SortOrder ScriptFindResultBinder::GetDefaultSortOrder()
			{
				return SortOrder::Ascending;
			}

			void ScriptErrorIndicator::RenderSquiggly(TextView^ Destination,
													  System::Windows::Media::DrawingContext^ DrawingContext,
													  int StartOffset, int EndOffset,
													  Windows::Media::Color Color)
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

			ScriptErrorIndicator::ScriptErrorIndicator(GetColorizerSegments^ Getter)
			{
				Delegate = Getter;
			}

			ScriptErrorIndicator::~ScriptErrorIndicator()
			{
				SAFEDELETE_CLR(Delegate);
			}

			void ScriptErrorIndicator::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				Color Buffer = PREFERENCES->LookupColorByKey("ErrorHighlightColor");
				Windows::Media::Color RenderColor = Windows::Media::Color::FromArgb(255, Buffer.R, Buffer.G, Buffer.B);

				for each (ColorizerSegment^ Itr in Delegate())
					RenderSquiggly(textView, drawingContext, Itr->Start, Itr->End, RenderColor);
			}

			ScriptFindResultIndicator::ScriptFindResultIndicator(GetColorizerSegments^ Getter) :
				ILineBackgroundColorizer(nullptr, KnownLayer::Background)
			{
				Delegate = Getter;
			}

			ScriptFindResultIndicator::~ScriptFindResultIndicator()
			{
				SAFEDELETE_CLR(Delegate);
			}

			void ScriptFindResultIndicator::Draw(TextView^ textView, System::Windows::Media::DrawingContext^ drawingContext)
			{
				Color Buffer = PREFERENCES->LookupColorByKey("FindResultsHighlightColor");

				for each (ColorizerSegment^ Itr in Delegate())
				{
					try
					{
						RenderBackground(textView,
										 drawingContext,
										 Itr->Start,
										 Itr->End,
										 Windows::Media::Color::FromArgb(100, Buffer.R, Buffer.G, Buffer.B),
										 Windows::Media::Color::FromArgb(150, Buffer.R, Buffer.G, Buffer.B),
										 1,
										 false);
					}
					catch (...) {}
				}
			}

			void LineTrackingManager::Parent_TextChanged(Object^ Sender, EventArgs^ E)
			{
				// disable compiler error indicators for the changed line
				int Caret = Parent->TextArea->Caret->Offset;
				int Line = Parent->Document->GetLineByOffset(Caret)->LineNumber;

				List<ScriptMessage^>^ CompilerErrors = gcnew List < ScriptMessage^ > ;

				if (GetMessages(Line,
								textEditors::IScriptTextEditor::ScriptMessageSource::Compiler,
								textEditors::IScriptTextEditor::ScriptMessageType::Error,
								CompilerErrors))
				{
					for each (auto Itr in CompilerErrors)
						Itr->IndicatorDisabled = true;
				}

				// disable find result indicators for modified segments
				List<ScriptFindResult^>^ CurrentResults = gcnew List < ScriptFindResult^ > ;
				if (GetFindResults(Line, CurrentResults))
				{
					for each (auto Itr in CurrentResults)
					{
						if (Caret >= Itr->StartOffset() && Caret <= Itr->EndOffset())
							Itr->IndicatorDisabled = true;
					}
				}
			}

			TextAnchor^ LineTrackingManager::CreateAnchor(UInt32 Offset, bool AllowDeletion)
			{
				if (Offset >= Parent->Document->TextLength)
					Offset = 0;

				TextAnchor^ New = Parent->Document->CreateAnchor(Offset);
				New->SurviveDeletion = AllowDeletion == false;
				return New;
			}

			void LineTrackingManager::RefreshBackgroundRenderers(bool IgnoreBatchUpdate)
			{
				if (IgnoreBatchUpdate || CurrentBatchUpdate == UpdateSource::None)
					Parent->TextArea->TextView->InvalidateLayer(KnownLayer::Background);
			}

			UInt32 LineTrackingManager::GetFindResults(UInt32 At, List<ScriptFindResult^>^% Out)
			{
				int Count = 0;
				for each (ScriptFindResult^ Itr in FindResults)
				{
					if (Itr->Line() == At)
					{
						Out->Add(Itr);
						Count++;
					}
				}
				return Count;
			}

			List<ColorizerSegment^>^ LineTrackingManager::GetErrorColorizerSegments()
			{
				List<ColorizerSegment^>^ Result = gcnew List < ColorizerSegment^ > ;
				List<int>^ ParsedLines = gcnew List < int > ;

				for each (ScriptMessage^ Itr in Messages)
				{
					if (Itr->Deleted() == false &&
						Itr->IndicatorDisabled == false &&
						Itr->Type() == IScriptTextEditor::ScriptMessageType::Error &&
						ParsedLines->Contains(Itr->Line()) == false)
					{
						DocumentLine^ Line = Parent->TextArea->Document->GetLineByNumber(Itr->Line());
						ISegment^ WhitespaceLeading = AvalonEdit::Document::TextUtilities::GetLeadingWhitespace(Parent->TextArea->Document, Line);
						ISegment^ WhitespaceTrailing = AvalonEdit::Document::TextUtilities::GetTrailingWhitespace(Parent->TextArea->Document, Line);

						ColorizerSegment^ Segment = gcnew ColorizerSegment;
						Segment->Start = WhitespaceLeading->EndOffset;
						Segment->End = WhitespaceTrailing->Offset;
						Result->Add(Segment);
						ParsedLines->Add(Itr->Line());
					}
				}

				return Result;
			}

			List<ColorizerSegment^>^ LineTrackingManager::GetFindResultColorizerSegments()
			{
				List<ColorizerSegment^>^ Result = gcnew List < ColorizerSegment^ >;

				for each (ScriptFindResult^ Itr in FindResults)
				{
					if (Itr->Deleted() == false && Itr->IndicatorDisabled == false)
					{
						ColorizerSegment^ Segment = gcnew ColorizerSegment;
						Segment->Start = Itr->StartOffset();
						Segment->End = Itr->EndOffset();
						Result->Add(Segment);
					}
				}

				return Result;
			}

			void LineTrackingManager::OnTrackedDataUpdated()
			{
				if (CurrentUpdateCounter == 0)
					TrackedDataUpdated(this, EventArgs::Empty);
			}

			LineTrackingManager::LineTrackingManager(AvalonEdit::TextEditor^ ParentEditor) :
				Parent(ParentEditor)
			{
				Messages = gcnew SimpleBindingList < ScriptMessage^ > ;
				Bookmarks = gcnew SimpleBindingList < ScriptBookmark^ > ;
				FindResults = gcnew SimpleBindingList < ScriptFindResult^ > ;

				BinderMessages = gcnew ScriptMessageBinder;
				BinderBookmarks = gcnew ScriptBookmarkBinder;
				BinderFindResults = gcnew ScriptFindResultBinder;

				CurrentBatchUpdate = UpdateSource::None;
				CurrentUpdateCounter = 0;

				ErrorColorizer = gcnew ScriptErrorIndicator(gcnew GetColorizerSegments(this, &LineTrackingManager::GetErrorColorizerSegments));
				FindResultColorizer = gcnew ScriptFindResultIndicator(gcnew GetColorizerSegments(this, &LineTrackingManager::GetFindResultColorizerSegments));

				Parent->TextArea->TextView->BackgroundRenderers->Add(ErrorColorizer);
				Parent->TextArea->TextView->BackgroundRenderers->Add(FindResultColorizer);

				ParentTextChangedHandler = gcnew EventHandler(this, &LineTrackingManager::Parent_TextChanged);
				Parent->TextChanged += ParentTextChangedHandler;
			}

			LineTrackingManager::~LineTrackingManager()
			{
				Debug::Assert(CurrentBatchUpdate == UpdateSource::None && CurrentUpdateCounter == 0);

				Unbind();

				ClearMessages(IScriptTextEditor::ScriptMessageSource::None, IScriptTextEditor::ScriptMessageType::None);
				ClearBookmarks();
				ClearFindResults(false);

				Parent->TextArea->TextView->BackgroundRenderers->Remove(ErrorColorizer);
				Parent->TextArea->TextView->BackgroundRenderers->Remove(FindResultColorizer);

				Parent->TextChanged -= ParentTextChangedHandler;

				SAFEDELETE_CLR(FindResultColorizer);
				SAFEDELETE_CLR(ErrorColorizer);

				SAFEDELETE_CLR(Messages);
				SAFEDELETE_CLR(FindResults);
				SAFEDELETE_CLR(Bookmarks);

				SAFEDELETE_CLR(BinderMessages);
				SAFEDELETE_CLR(BinderBookmarks);
				SAFEDELETE_CLR(BinderFindResults);

				SAFEDELETE_CLR(ParentTextChangedHandler);

				Parent = nullptr;
			}

			void LineTrackingManager::Bind(ListView^ MessageList, ListView^ BookmarkList, ListView^ FindResultList)
			{
				BinderMessages->Bind(MessageList, Messages);
				BinderBookmarks->Bind(BookmarkList, Bookmarks);
				BinderFindResults->Bind(FindResultList, FindResults);
			}

			void LineTrackingManager::Unbind()
			{
				BinderMessages->Unbind();
				BinderBookmarks->Unbind();
				BinderFindResults->Unbind();
			}

			void LineTrackingManager::BeginUpdate(UpdateSource Source)
			{
				Debug::Assert(CurrentBatchUpdate == UpdateSource::None || CurrentBatchUpdate == Source);
				Debug::Assert(Source != UpdateSource::None);

				if (CurrentUpdateCounter == 0)
				{
					switch (Source)
					{
					case cse::textEditors::avalonEditor::LineTrackingManager::UpdateSource::Messages:
						Messages->BeginUpdate();
						break;
					case cse::textEditors::avalonEditor::LineTrackingManager::UpdateSource::Bookmarks:
						Bookmarks->BeginUpdate();
						break;
					case cse::textEditors::avalonEditor::LineTrackingManager::UpdateSource::FindResults:
						FindResults->BeginUpdate();
						break;
					}
				}

				CurrentUpdateCounter++;
				CurrentBatchUpdate = Source;
			}

			void LineTrackingManager::EndUpdate(bool Sort)
			{
				Debug::Assert(CurrentBatchUpdate != UpdateSource::None);

				CurrentUpdateCounter--;
				Debug::Assert(CurrentUpdateCounter >= 0);

				if (CurrentUpdateCounter == 0)
				{
					switch (CurrentBatchUpdate)
					{
					case cse::textEditors::avalonEditor::LineTrackingManager::UpdateSource::Messages:
						Messages->EndUpdate(Sort, gcnew ScriptMessageSorter(TrackingMessageSorter::ComparisonField::ImageIndex));
						break;
					case cse::textEditors::avalonEditor::LineTrackingManager::UpdateSource::Bookmarks:
						Bookmarks->EndUpdate(Sort, gcnew ScriptBookmarkSorter(TrackingMessageSorter::ComparisonField::Line));
						break;
					case cse::textEditors::avalonEditor::LineTrackingManager::UpdateSource::FindResults:
						FindResults->EndUpdate(Sort, gcnew ScriptFindResultSorter(TrackingMessageSorter::ComparisonField::Line));
						break;
					}

					CurrentBatchUpdate = UpdateSource::None;
					RefreshBackgroundRenderers(false);
					OnTrackedDataUpdated();
				}
			}

			void LineTrackingManager::TrackMessage(UInt32 Line,
												   IScriptTextEditor::ScriptMessageType Type,
												   IScriptTextEditor::ScriptMessageSource Source,
												   String^ Message)
			{
				Debug::Assert(Type != IScriptTextEditor::ScriptMessageType::None);
				Debug::Assert(Source != IScriptTextEditor::ScriptMessageSource::None);
				Debug::Assert(Line > 0);

				if (Line > Parent->LineCount)
					Line = Parent->LineCount;

				ScriptMessage^ New = gcnew ScriptMessage(this, CreateAnchor(Parent->Document->GetLineByNumber(Line)->Offset, false),
														 Type, Source, Message->Replace("\t", "")->Replace("\r", "")->Replace("\n", ""));
				Messages->Add(New);
				RefreshBackgroundRenderers(false);
				OnTrackedDataUpdated();
			}

			void LineTrackingManager::ClearMessages(IScriptTextEditor::ScriptMessageSource SourceFilter,
													IScriptTextEditor::ScriptMessageType TypeFilter)
			{
				List<ScriptMessage^>^ Buffer = gcnew List < ScriptMessage^ > ;

				BeginUpdate(LineTrackingManager::UpdateSource::Messages);
				for each (ScriptMessage^ Itr in Messages)
				{
					if ((SourceFilter == IScriptTextEditor::ScriptMessageSource::None || Itr->Source() == SourceFilter) &&
						(TypeFilter == IScriptTextEditor::ScriptMessageType::None || Itr->Type() == TypeFilter))
					{
						Buffer->Add(Itr);
					}
				}

				for each (ScriptMessage^ Itr in Buffer)
					Messages->Remove(Itr);

				EndUpdate(false);
			}

			bool LineTrackingManager::GetMessages(UInt32 Line,
												  IScriptTextEditor::ScriptMessageSource SourceFilter,
												  IScriptTextEditor::ScriptMessageType TypeFilter,
												  List<ScriptMessage^>^% OutMessages)
			{
				bool Result = false;
				for each (ScriptMessage^ Itr in Messages)
				{
					if (Itr->Line() == Line)
					{
						if ((SourceFilter == IScriptTextEditor::ScriptMessageSource::None || Itr->Source() == SourceFilter) &&
							(TypeFilter == IScriptTextEditor::ScriptMessageType::None || Itr->Type() == TypeFilter))
						{
							OutMessages->Add(Itr);
							Result = true;
						}
					}
				}

				return Result;
			}

			UInt32 LineTrackingManager::GetMessageCount(UInt32 Line,
														IScriptTextEditor::ScriptMessageSource SourceFilter,
														IScriptTextEditor::ScriptMessageType TypeFilter)
			{
				int Count = 0;
				for each (ScriptMessage^ Itr in Messages)
				{
					if (Line == 0 || Itr->Line() == Line)
					{
						if ((SourceFilter == IScriptTextEditor::ScriptMessageSource::None || Itr->Source() == SourceFilter) &&
							(TypeFilter == IScriptTextEditor::ScriptMessageType::None || Itr->Type() == TypeFilter))
						{
							Count++;
						}
					}
				}

				return Count;
			}

			void LineTrackingManager::AddBookmark(UInt32 Line, String^ Description)
			{
				Debug::Assert(Line > 0);
				if (Line > Parent->LineCount)
					Line = Parent->LineCount;

				Description->Replace("\t", " ");
				ScriptBookmark^ New = gcnew ScriptBookmark(this, CreateAnchor(Parent->Document->GetLineByNumber(Line)->Offset, true), Description);

				Bookmarks->Add(New);
				RefreshBackgroundRenderers(false);
				OnTrackedDataUpdated();
			}

			UInt32 LineTrackingManager::GetBookmarks(UInt32 Line, List<ScriptBookmark^>^% Out)
			{
				int Count = 0;
				for each (ScriptBookmark^ Itr in Bookmarks)
				{
					if (Itr->Deleted() == false && Itr->Line() == Line)
					{
						Out->Add(Itr);
						Count++;
					}
				}
				return Count;
			}

			void LineTrackingManager::ClearBookmarks()
			{
				Bookmarks->Clear();
				RefreshBackgroundRenderers(false);
				OnTrackedDataUpdated();
			}

			String^ LineTrackingManager::SerializeBookmarks()
			{
				String^ Out = "";

				for each (ScriptBookmark^ Itr in Bookmarks)
					Out += ";<" + kMetadataSigilBookmark + ">\t" + Itr->Line() + "\t" + Itr->Message() + "\t</" + kMetadataSigilBookmark + ">\n";

				return Out;
			}

			void LineTrackingManager::DeserializeBookmarks(String^ Serialized, bool ClearExisting)
			{
				BeginUpdate(LineTrackingManager::UpdateSource::Bookmarks);

				if (ClearExisting)
					ClearBookmarks();

				ScriptParser^ TextParser = gcnew ScriptParser();
				StringReader^ StringParser = gcnew StringReader(Serialized);
				String^ ReadLine = StringParser->ReadLine();
				int LineNo = 0;

				while (ReadLine != nullptr)
				{
					if (TextParser->Tokenize(ReadLine, false) == false)
					{
						ReadLine = StringParser->ReadLine();
						continue;
					}

					if (TextParser->GetTokenIndex(";<" + kMetadataSigilBookmark + ">") == 0)
					{
						array<String^>^ Splits = ReadLine->Substring(TextParser->Indices[0])->Split((String("\t")).ToCharArray());
						try	{
							LineNo = int::Parse(Splits[1]);
						} catch (...) {
							LineNo = 1;
						}

						if (LineNo > 0 && LineNo <= Parent->Document->LineCount)
							AddBookmark(LineNo, Splits[2]);
					}

					ReadLine = StringParser->ReadLine();
				}

				EndUpdate(true);
			}

			void LineTrackingManager::TrackFindResult(UInt32 Start, UInt32 End, String^ Text)
			{
				ScriptFindResult^ New = gcnew ScriptFindResult(this, CreateAnchor(Start, false), CreateAnchor(End, false), Text);

				FindResults->Add(New);
				RefreshBackgroundRenderers(false);
				OnTrackedDataUpdated();
			}

			void LineTrackingManager::ClearFindResults(bool IndicatorOnly)
			{
				if (IndicatorOnly)
				{
					for each (ScriptFindResult^ Itr in FindResults)
						Itr->IndicatorDisabled = true;
				}
				else
					FindResults->Clear();

				RefreshBackgroundRenderers(false);
				OnTrackedDataUpdated();
			}

			void LineTrackingManager::Cleanup()
			{
				List<ScriptMessage^>^ RemovedMessages = gcnew List < ScriptMessage^ > ;
				List<ScriptBookmark^>^ RemovedBookmarks = gcnew List < ScriptBookmark^ > ;
				List<ScriptFindResult^>^ RemovedFindResults = gcnew List < ScriptFindResult^ > ;

				for each (ScriptMessage^ Itr in Messages)
				{
					if (Itr->Deleted())
						RemovedMessages->Add(Itr);
				}

				for each (ScriptBookmark^ Itr in Bookmarks)
				{
					if (Itr->Deleted())
						RemovedBookmarks->Add(Itr);
				}

				for each (ScriptFindResult^ Itr in FindResults)
				{
					if (Itr->Deleted())
						RemovedFindResults->Add(Itr);
				}

				static const int BatchUpdateThreshold = 5;
				{
					if (RemovedMessages->Count > BatchUpdateThreshold)
						BeginUpdate(LineTrackingManager::UpdateSource::Messages);

					for each (ScriptMessage^ Itr in RemovedMessages)
						Messages->Remove(Itr);

					if (RemovedMessages->Count > BatchUpdateThreshold)
						EndUpdate(false);
				}

				{
					if (RemovedBookmarks->Count > BatchUpdateThreshold)
						BeginUpdate(LineTrackingManager::UpdateSource::Bookmarks);

					for each (ScriptBookmark^ Itr in RemovedBookmarks)
						Bookmarks->Remove(Itr);

					if (RemovedBookmarks->Count > BatchUpdateThreshold)
						EndUpdate(false);
				}

				{
					if (RemovedFindResults->Count > BatchUpdateThreshold)
						BeginUpdate(LineTrackingManager::UpdateSource::FindResults);

					for each (ScriptFindResult^ Itr in RemovedFindResults)
						FindResults->Remove(Itr);

					if (RemovedFindResults->Count > BatchUpdateThreshold)
						EndUpdate(false);
				}

				RemovedMessages->Clear();
				RemovedBookmarks->Clear();
				RemovedFindResults->Clear();

				RefreshBackgroundRenderers(false);
				OnTrackedDataUpdated();
			}

			void LineTrackingManager::Jump(TrackingMessage^ To)
			{
				int Line = To->Line();

				if (Line > 0 && Line <= Parent->LineCount)
				{
					Parent->TextArea->Caret->Line = Line;
					Parent->TextArea->Caret->Column = 0;
					Parent->TextArea->Caret->BringCaretToView();
					Parent->Focus();
				}
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

			void ObScriptIndentStrategy::IndentLines(AvalonEdit::Document::TextDocument^ document, Int32 beginLine, Int32 endLine)
			{
				;//
			}

			void ObScriptIndentStrategy::IndentLine(AvalonEdit::Document::TextDocument^ document, AvalonEdit::Document::DocumentLine^ line)
			{
				obScriptParsing::AnalysisData^ Data = Parent->GetSemanticAnalysisCache(false, true);
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

					if (Parser->GetFirstTokenType() == obScriptParsing::ScriptTokenType::ElseIf ||
						Parser->GetFirstTokenType() == obScriptParsing::ScriptTokenType::Else)
					{
						if (CurrIndent)
							document->Replace(Leading, gcnew String('\t', CurrIndent - 1));
					}
					else if	(Parser->GetFirstTokenType() == obScriptParsing::ScriptTokenType::EndIf ||
							Parser->GetFirstTokenType() == obScriptParsing::ScriptTokenType::End ||
							Parser->GetFirstTokenType() == obScriptParsing::ScriptTokenType::Loop)
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
				Parent = nullptr;
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

				obScriptParsing::AnalysisData^ Data = Parent->GetSemanticAnalysisCache(false, false);
				for each (obScriptParsing::ControlBlock^ Itr in Data->ControlBlocks)
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
				Parent = nullptr;
			}

			ObScriptCodeFoldingStrategy::ObScriptCodeFoldingStrategy(AvalonEditTextEditor^ Parent) :
#if BUILD_AVALONEDIT_VERSION < AVALONEDIT_5_0_1
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
															System::Windows::Media::Color::FromArgb(150, 0, 0, 0)), 0),
													HighlightGeometry);
					}
					else
					{
						drawingContext->DrawGeometry(gcnew System::Windows::Media::SolidColorBrush(System::Windows::Media::Color::FromArgb(125,
																									ValidBraceColor.R,
																									ValidBraceColor.G,
																									ValidBraceColor.B)),
													gcnew System::Windows::Media::Pen(gcnew System::Windows::Media::SolidColorBrush(
															System::Windows::Media::Color::FromArgb(150, 0, 0, 0)), 0),
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
				InstanceCounter++;
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
				obScriptParsing::ControlBlock^ Block = ParentEditor->GetSemanticAnalysisCache(false, false)->GetBlockEndingAt(CurrentLine->LineNumber);

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
				Color ForegroundColor = PREFERENCES->LookupColorByKey("ForegroundColor");
				Font^ CustomFont = gcnew Font(PREFERENCES->FetchSettingAsString("Font", "Appearance"),
											  PREFERENCES->FetchSettingAsInt("FontSize", "Appearance") - 2,
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
				{
					Color Current = SystemColors::Control;
					if (Current.R < 100 && Current.B < 100 && Current.G < 100)
						WarningIcon = WPFImageResourceGenerator::CreateImageSource("AvalonEditIconMarginWarningColor");
					else
						WarningIcon = WPFImageResourceGenerator::CreateImageSource("AvalonEditIconMarginWarningDark");
				}

				return WarningIcon;
			}

			Windows::Media::Imaging::BitmapSource^ DefaultIconMargin::GetBookmarkIcon()
			{
				if (BookmarkIcon == nullptr)
					BookmarkIcon = WPFImageResourceGenerator::CreateImageSource("AvalonEditIconMarginBookmark");

				return BookmarkIcon;
			}

			void DefaultIconMargin::HandleHoverStart(int Line, System::Windows::Input::MouseEventArgs^ E)
			{
				bool DisplayPopup = false;
				ToolTipIcon PopupIcon = ToolTipIcon::None;
				String^ PopupTitle = "";
				String^ PopupText = "";
				Windows::Point DisplayLocation = E->GetPosition(Parent);
				DisplayLocation.Y += GetVisualLineFromMousePosition(E)->Height;

				List<ScriptMessage^>^ Warnings = gcnew List < ScriptMessage^ >;
				List<ScriptBookmark^>^ Bookmarks = gcnew List < ScriptBookmark^ >;
				if (LineTracker->GetMessages(Line,
										 IScriptTextEditor::ScriptMessageSource::None,
										 IScriptTextEditor::ScriptMessageType::Warning,
										 Warnings))
				{
					DisplayPopup = true;
					PopupIcon = ToolTipIcon::Warning;
					PopupTitle = Warnings->Count + " Warning" + (Warnings->Count == 1 ? "" : "s");

					for each (auto Itr in Warnings)
						PopupText += Itr->Message() + "\n";

					if (PopupText->Length)
						PopupText->Remove(PopupText->Length - 1);
				}
				else if (LineTracker->GetBookmarks(Line,Bookmarks))
				{
					DisplayPopup = true;
					PopupIcon = ToolTipIcon::Info;
					PopupTitle = Bookmarks->Count + " Bookmark" + (Bookmarks->Count == 1 ? "" : "s");

					for each (auto Itr in Bookmarks)
						PopupText += Itr->Message() + "\n";

					if (PopupText->Length)
						PopupText->Remove(PopupText->Length - 1);
				}

				if (DisplayPopup)
					ShowPopup(PopupTitle, PopupText, PopupIcon, Point(DisplayLocation.X, DisplayLocation.Y));
			}

			void DefaultIconMargin::HandleHoverStop()
			{
				HidePopup();
			}

			void DefaultIconMargin::HandleClick(int Line)
			{
				;//
			}

			bool DefaultIconMargin::GetRenderData(int Line, Windows::Media::Imaging::BitmapSource^% OutIcon, double% OutOpacity, int% Width, int% Height)
			{
				// warnings override bookmarks
				List<ScriptMessage^>^ Warnings = gcnew List < ScriptMessage^ >;
				List<ScriptBookmark^>^ Bookmarks = gcnew List < ScriptBookmark^ >;
				LineTracker->GetMessages(Line, IScriptTextEditor::ScriptMessageSource::None, IScriptTextEditor::ScriptMessageType::Warning, Warnings);
				LineTracker->GetBookmarks(Line, Bookmarks);

				if (Warnings->Count == 0 && Bookmarks->Count == 0)
					return false;

				if (Warnings->Count)
				{
					OutIcon = GetWarningIcon();
					Width = Height = 16;
				}
				else
				{
					OutIcon = GetBookmarkIcon();
					Width = Height = 16;
				}

				OutOpacity = 1.0;
				return true;
			}

			DefaultIconMargin::DefaultIconMargin(AvalonEdit::TextEditor^ ParentEditor, LineTrackingManager^ ParentLineTracker, IntPtr ToolTipParent)
			{
				InstanceCounter++;

				Debug::Assert(ParentEditor != nullptr);
				Debug::Assert(ParentLineTracker != nullptr);

				Parent = ParentEditor;
				LineTracker = ParentLineTracker;
				PopupParentHandle = ToolTipParent;

				Popup = gcnew Windows::Forms::ToolTip;
				Popup->AutoPopDelay = 500;
				Popup->InitialDelay = 500;
				Popup->ReshowDelay = 0;
				Popup->ToolTipIcon = ToolTipIcon::None;

				LineTracker->TrackedDataUpdated += HandlerTextViewChanged;
			}

			DefaultIconMargin::~DefaultIconMargin()
			{
				InstanceCounter--;
				Debug::Assert(InstanceCounter >= 0);

				LineTracker->TrackedDataUpdated -= HandlerTextViewChanged;

				HidePopup();
				SAFEDELETE_CLR(Popup);

				Parent = nullptr;
				LineTracker = nullptr;

				if (InstanceCounter == 0)
				{
					if (WarningIcon)
						SAFEDELETE_CLR(WarningIcon);

					if (BookmarkIcon)
						SAFEDELETE_CLR(BookmarkIcon);
				}
			}

			void DefaultIconMargin::ShowPopup(String^ Title, String^ Message, ToolTipIcon Icon, Drawing::Point Location)
			{
				Popup->ToolTipIcon = Icon;
				Popup->ToolTipTitle = Title;
				Popup->Show(Message, Control::FromHandle(PopupParentHandle), Location, 6000);
			}

			void DefaultIconMargin::HidePopup()
			{
				Popup->Hide(Control::FromHandle(PopupParentHandle));
			}
		}
	}
}