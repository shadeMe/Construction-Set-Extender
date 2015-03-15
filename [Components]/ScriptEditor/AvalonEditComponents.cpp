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

			ScriptBookmark::ScriptBookmark(LineTrackingManager^ Parent, TextAnchor^ Location, String^ Text)
			{
				Manager = Parent;
				Anchor = Location;
				Description = Text;
			}

			int ScriptBookmark::Line()
			{
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
				Message->Width = 300;

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
				Message->Width = 300;

				Control->Columns->Add(Dummy);
				Control->Columns->Add(Line);
				Control->Columns->Add(Message);

				Control->SmallImageList = gcnew ImageList();
				Control->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListWarning"));
				Control->SmallImageList->Images->Add(Globals::ScriptEditorImageResourceManager->CreateImageFromResource("MessageListError"));
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
				Message->Width = 300;

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
				delete Delegate;
				Delegate = nullptr;
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
				delete Delegate;
				Delegate = nullptr;
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

			TextAnchor^ LineTrackingManager::CreateAnchor(UInt32 Offset)
			{
				if (Offset >= Parent->Document->TextLength)
					Offset = 0;

				TextAnchor^ New = Parent->Document->CreateAnchor(Offset);
				New->SurviveDeletion = true;
				return New;
			}

			void LineTrackingManager::RefreshBackgroundRenderers(bool IgnoreBatchUpdate)
			{
				if (IgnoreBatchUpdate || CurrentBatchUpdate == UpdateSource::None)
					Parent->TextArea->TextView->InvalidateLayer(KnownLayer::Background);
			}

			void LineTrackingManager::GetBookmarks(UInt32 At, List<ScriptBookmark^>^% Out)
			{
				for each (ScriptBookmark^ Itr in Bookmarks)
				{
					if (Itr->Line() == At)
						Out->Add(Itr);
				}
			}

			List<ColorizerSegment^>^ LineTrackingManager::GetErrorColorizerSegments()
			{
				List<ColorizerSegment^>^ Result = gcnew List < ColorizerSegment^ > ;
				List<int>^ ParsedLines = gcnew List < int > ;

				for each (ScriptMessage^ Itr in Messages)
				{
					if (Itr->Deleted() == false && ParsedLines->Contains(Itr->Line()) == false)
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
					if (Itr->Deleted() == false)
					{
						ColorizerSegment^ Segment = gcnew ColorizerSegment;
						Segment->Start = Itr->StartOffset();
						Segment->End = Itr->EndOffset();
						Result->Add(Segment);
					}
				}

				return Result;
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
			}

			LineTrackingManager::~LineTrackingManager()
			{
				Debug::Assert(CurrentBatchUpdate == UpdateSource::None && CurrentUpdateCounter == 0);

				Unbind();

				ClearMessages(IScriptTextEditor::ScriptMessageSource::None);
				ClearBookmarks();
				ClearFindResults();

				Parent->TextArea->TextView->BackgroundRenderers->Remove(ErrorColorizer);
				Parent->TextArea->TextView->BackgroundRenderers->Remove(FindResultColorizer);

				delete ErrorColorizer;
				delete FindResultColorizer;
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
					case ConstructionSetExtender::TextEditors::AvalonEditor::LineTrackingManager::UpdateSource::Messages:
						Messages->BeginUpdate();
						break;
					case ConstructionSetExtender::TextEditors::AvalonEditor::LineTrackingManager::UpdateSource::Bookmarks:
						Bookmarks->BeginUpdate();
						break;
					case ConstructionSetExtender::TextEditors::AvalonEditor::LineTrackingManager::UpdateSource::FindResults:
						FindResults->BeginUpdate();
						break;
					}
				}

				CurrentUpdateCounter++;
				CurrentBatchUpdate = Source;
			}

			void LineTrackingManager::EndUpdate()
			{
				Debug::Assert(CurrentBatchUpdate != UpdateSource::None);

				CurrentUpdateCounter--;
				Debug::Assert(CurrentUpdateCounter >= 0);

				if (CurrentUpdateCounter == 0)
				{
					switch (CurrentBatchUpdate)
					{
					case ConstructionSetExtender::TextEditors::AvalonEditor::LineTrackingManager::UpdateSource::Messages:
						Messages->EndUpdate();
						break;
					case ConstructionSetExtender::TextEditors::AvalonEditor::LineTrackingManager::UpdateSource::Bookmarks:
						Bookmarks->EndUpdate();
						break;
					case ConstructionSetExtender::TextEditors::AvalonEditor::LineTrackingManager::UpdateSource::FindResults:
						FindResults->EndUpdate();
						break;
					}

					CurrentBatchUpdate = UpdateSource::None;
					RefreshBackgroundRenderers(false);
				}
			}

			void LineTrackingManager::TrackMessage(UInt32 Line,
												   IScriptTextEditor::ScriptMessageType Type,
												   IScriptTextEditor::ScriptMessageSource Source,
												   String^ Message)
			{
				Debug::Assert(Source != IScriptTextEditor::ScriptMessageSource::None);
				Debug::Assert(Line > 0);
				if (Line > Parent->LineCount)
					Line = Parent->LineCount;

				ScriptMessage^ New = gcnew ScriptMessage(this, CreateAnchor(Parent->Document->GetLineByNumber(Line)->Offset),
														 Type, Source, Message);
				Messages->Add(New);
				RefreshBackgroundRenderers(false);
			}

			void LineTrackingManager::ClearMessages(IScriptTextEditor::ScriptMessageSource Filter)
			{
				if (Filter == IScriptTextEditor::ScriptMessageSource::None)
				{
					// remove all
					Messages->Clear();
					RefreshBackgroundRenderers(false);
				}
				else
				{
					List<ScriptMessage^>^ Buffer = gcnew List < ScriptMessage^ > ;

					BeginUpdate(LineTrackingManager::UpdateSource::Messages);
					for each (ScriptMessage^ Itr in Messages)
					{
						if (Itr->Source() == Filter)
							Buffer->Add(Itr);
					}

					for each (ScriptMessage^ Itr in Buffer)
						Messages->Remove(Itr);

					EndUpdate();
				}
			}

			bool LineTrackingManager::GetMessages(UInt32 Line, IScriptTextEditor::ScriptMessageSource Filter, List<ScriptMessage^>^% OutMessages)
			{
				bool Result = false;
				for each (ScriptMessage^ Itr in Messages)
				{
					if (Itr->Line() == Line &&
						(Filter == IScriptTextEditor::ScriptMessageSource::None || Itr->Source() == Filter))
					{
						OutMessages->Add(Itr);
						Result = true;
					}
				}

				return Result;
			}

			void LineTrackingManager::AddBookmark(UInt32 Line, String^ Description)
			{
				TODO("how do we handle deleted bookmark anchors?");
				Debug::Assert(Line > 0);
				if (Line > Parent->LineCount)
					Line = Parent->LineCount;

				Description->Replace("\t", " ");
				ScriptBookmark^ New = gcnew ScriptBookmark(this, CreateAnchor(Parent->Document->GetLineByNumber(Line)->Offset), Description);

				Bookmarks->Add(New);
				RefreshBackgroundRenderers(false);
			}

			void LineTrackingManager::ClearBookmarks()
			{
				Bookmarks->Clear();
				RefreshBackgroundRenderers(false);
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
						} catch (...){
							LineNo = 1;
						}

						if (LineNo > 0 && LineNo <= Parent->Document->LineCount)
							AddBookmark(LineNo, Splits[2]);
					}

					ReadLine = StringParser->ReadLine();
				}

				EndUpdate();
			}

			void LineTrackingManager::TrackFindResult(UInt32 Start, UInt32 End, String^ Text)
			{
				ScriptFindResult^ New = gcnew ScriptFindResult(this, CreateAnchor(Start), CreateAnchor(End), Text);

				FindResults->Add(New);
				RefreshBackgroundRenderers(false);
			}

			void LineTrackingManager::ClearFindResults()
			{
				FindResults->Clear();
				RefreshBackgroundRenderers(false);
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
						EndUpdate();
				}

				{
					if (RemovedBookmarks->Count > BatchUpdateThreshold)
						BeginUpdate(LineTrackingManager::UpdateSource::Bookmarks);

					for each (ScriptBookmark^ Itr in RemovedBookmarks)
						Bookmarks->Remove(Itr);

					if (RemovedBookmarks->Count > BatchUpdateThreshold)
						EndUpdate();
				}

				{
					if (RemovedFindResults->Count > BatchUpdateThreshold)
						BeginUpdate(LineTrackingManager::UpdateSource::FindResults);

					for each (ScriptFindResult^ Itr in RemovedFindResults)
						FindResults->Remove(Itr);

					if (RemovedFindResults->Count > BatchUpdateThreshold)
						EndUpdate();
				}

				RemovedMessages->Clear();
				RemovedBookmarks->Clear();
				RemovedFindResults->Clear();
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

			StructureVisualizerRenderer::~StructureVisualizerRenderer()
			{
				delete IconSource;

				IconSource = nullptr;
				ParentEditor = nullptr;
			}
		}
	}
}