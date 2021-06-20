#pragma once

#include "Macros.h"
#include "ITextEditor.h"
#include "IIntelliSenseInterface.h"
#include "SemanticAnalysis.h"
#include "Utilities.h"

namespace cse
{


namespace scriptEditor
{


namespace model
{


namespace components
{


ref struct ScriptCompilationData
{
	String^	UnpreprocessedScriptText;
	String^	PreprocessedScriptText;
	String^	SerializedMetadata;

	bool CanCompile;
	bool HasPreprocessorDirectives;
	bool HasWarnings;

	componentDLLInterface::ScriptCompileData* CompileResult;

	ScriptCompilationData();
};


ref struct ScriptLineAnnotation : public textEditor::ILineAnchor
{
protected:
	textEditor::ILineAnchor^ Anchor_;
	String^ Text_;
public:
	ScriptLineAnnotation(textEditor::ILineAnchor^ Anchor, String^ Text);

	ImplPropertyGetOnly(UInt32, Line, Anchor_->Line);
	ImplPropertyGetOnly(bool, Valid, Anchor_->Valid);
	ImplPropertyGetOnly(String^, Text, Text_);
};


ref struct ScriptDiagnosticMessage : public ScriptLineAnnotation
{
	static enum class eMessageType
	{
		All,
		Info,
		Warning,
		Error,
	};

	static enum class eMessageSource
	{
		All,
		Compiler,
		Validator,
		Preprocessor,
	};

	static void PopulateImageListWithMessageTypeImages(ImageList^ Destination);
protected:
	static array<Image^>^ MessageTypeImages =
	{
		/*Globals::ImageResources()->CreateImage("IntelliSenseItemEmpty"),
		Globals::ImageResources()->CreateImage("MessageListInfo"),
		Globals::ImageResources()->CreateImage("MessageListWarning"),
		Globals::ImageResources()->CreateImage("MessageListError"),*/
	};

	eMessageType Type_;
	eMessageSource Source_;
public:
	ScriptDiagnosticMessage(textEditor::ILineAnchor^ Anchor, String^ Message, eMessageType Type, eMessageSource Source);

	ImplPropertyGetOnly(eMessageType, Type, Type_);
	ImplPropertyGetOnly(eMessageSource, Source, Source_);
};


ref struct ScriptBookmark : public ScriptLineAnnotation
{
public:
	ScriptBookmark(textEditor::ILineAnchor^ Anchor, String^ Description);
};

ref struct ScriptFindResult : public ScriptLineAnnotation
{
	String^ Query_;
	UInt32 Hits_;
public:
	ScriptFindResult(textEditor::ILineAnchor^ Anchor, String^ Text, UInt32 Hits, String^ Query);

	ImplPropertyGetOnly(UInt32, Hits, Hits_);
	ImplPropertyGetOnly(String^, Query, Query_);
};


ref struct ScriptTextMetadata
{
	using CaretPosition = int;

	ref struct Bookmark
	{
		property UInt32	 Line;
		property String^ Text;

		Bookmark(UInt32 Line, String^ Message);
	};

	property CaretPosition CaretPos;
	property List<Bookmark^>^ Bookmarks;
	property bool HasPreprocessorDirectives;

	ScriptTextMetadata();
};


ref class ScriptTextMetadataHelper
{
	static String^ kMetadataBlockMarker = "CSEBlock";
	static String^ kMetadataSigilCaret = "CSECaretPos";
	static String^ kMetadataSigilBookmark = "CSEBookmark";

	static void	SeparateScriptTextFromMetadataBlock(String^ RawScriptText, String^% OutScriptText, String^% OutMetadata);
public:
	static void	DeserializeRawScriptText(String^ RawScriptText, String^% OutScriptText, ScriptTextMetadata^% OutMetadata);
	static String^ SerializeMetadata(ScriptTextMetadata^ Metadata);
};


interface class IBackgroundSemanticAnalyzer
{
	ref struct AnalysisCompleteEventArgs
	{
		property obScriptParsing::AnalysisData^ Result;
		property bool BackgroundAnalysis;

		AnalysisCompleteEventArgs(obScriptParsing::AnalysisData^ Result, bool BackgroundAnalysis);
	};
	delegate void AnalysisCompleteEventHandler(Object^ Sender, AnalysisCompleteEventArgs^ E);

	property obScriptParsing::AnalysisData^ LastAnalysisResult;

	event AnalysisCompleteEventHandler^ SemanticAnalysisComplete;

	obScriptParsing::AnalysisData^ DoSynchronousAnalysis(bool RaiseCompletionEvent);
};


interface class INavigationHelper
{
	ref struct NavigationChangedEventArgs
	{
		property bool StructureChanged;
		property bool LineChanged;
		property obScriptParsing::Structurizer^ StructureData;
		property UInt32 CurrentLine;

		NavigationChangedEventArgs(obScriptParsing::Structurizer^ StructureData, UInt32 CurrentLine);
	};
	delegate void NavigationChangedEventHandler(Object^ Sender, NavigationChangedEventArgs^ E);

	event NavigationChangedEventHandler^ NavigationChanged;

	obScriptParsing::Structurizer^ GenerateStructureData(obScriptParsing::AnalysisData^ AnalysisData);
};


} // namespace components


interface class IScriptDocument
{
	static enum class eScriptType
	{
		Object = 0,
		Quest = 1,
		MagicEffect = 0x100
	};

	static enum class eSaveOperation
	{
		Default,
		AlsoSaveActiveFile,
		DontCompile,
	};

	ref struct StateChangeEventArgs
	{
		static enum class eEventType
		{
			None,
			Dirty,
			BytecodeLength,
			ScriptType,
			EditorIdAndFormId,
			Messages,
			Bookmarks,
			FindResults
		};

		eEventType EventType;

		bool Dirty;
		UInt16 BytecodeLength;		// in bytes
		eScriptType ScriptType;
		String^ EditorId;
		UInt32 FormId;
		List<components::ScriptDiagnosticMessage^>^ Messages;
		List<components::ScriptBookmark^>^ Bookmarks;
		List<components::ScriptFindResult^>^ FindResults;

		StateChangeEventArgs();
	};
	delegate void StateChangeEventHandler(Object^ Sender, StateChangeEventArgs^ E);

	event StateChangeEventHandler^ StateChanged;

	property bool Valid;
	property bool Dirty;
	property eScriptType ScriptType;
	property String^ ScriptEditorID;
	property UInt32 ScriptFormID;
	property String^ ScriptText;
	property String^ PreprocessedScriptText;
	property UInt8* ScriptBytecode;
	property UInt16 ScriptBytecodeLength;
	property void* ScriptNativeObject;

	property textEditor::ITextEditor^ TextEditor;
	property intellisense::IIntelliSenseInterfaceModel^ IntelliSenseModel;
	property components::IBackgroundSemanticAnalyzer^ BackgroundAnalyzer;
	property components::INavigationHelper^ NavigationHelper;

	// ### TODO do the housekeeping/autorecovery in the controller (since we need access to the view)
	// same with anything that involves showing a message to teh user
	void Initialize(componentDLLInterface::ScriptData* ScriptData, bool UseAutoRecoveryFile);
	bool Save(eSaveOperation SaveOperation, bool% OutHasWarning);

	List<components::ScriptDiagnosticMessage^>^ GetMessages(UInt32 Line, components::ScriptDiagnosticMessage::eMessageSource SourceFilter, components::ScriptDiagnosticMessage::eMessageType TypeFilter);
	UInt32 GetErrorCount(UInt32 Line);
	UInt32 GetWarningCount(UInt32 Line);

	void AddBookmark(UInt32 Line, String^ BookmarkText);
	void RemoveBookmark(UInt32 Line, String^ BookmarkText);
	void RemoveBookmark(components::ScriptBookmark^ Bookmark);
	List<components::ScriptBookmark^>^ GetBookmarks(UInt32 Line);
	UInt32 GetBookmarkCount(UInt32 Line);

	bool HasAutoRecoveryFile();
	DateTime GetAutoRecoveryFileLastWriteTimestamp();
};


interface class IScriptEditorModel
{
	ref struct ActiveDocumentChangedEventArgs
	{
		property IScriptDocument^ OldValue;
		property IScriptDocument^ NewValue;

		ActiveDocumentChangedEventArgs(IScriptDocument^ Old, IScriptDocument^ New);
	};
	delegate void ActiveDocumentChangedEventHandler(Object^ Sender, ActiveDocumentChangedEventArgs^ E);

	ref struct ActiveDocumentActionCollection
	{
		ref struct AddBookmarkParams
		{
			String^ BookmarkDescription;
		};

		ref struct GoToLineParams
		{
			UInt32 Line;
		};

		property BasicAction^ Copy;
		property BasicAction^ Paste;
		property BasicAction^ Comment;
		property BasicAction^ Uncomment;
		property ParameterizedAction^ AddBookmark;
		property ParameterizedAction^ GoToLine;

		ActiveDocumentActionCollection();
		~ActiveDocumentActionCollection();
	};

	property List<IScriptDocument^>^ Documents;
	property IScriptDocument^ ActiveDocument;
	property ActiveDocumentActionCollection^ ActiveDocumentActions;

	event ActiveDocumentChangedEventHandler^ ActiveDocumentChanged;

	IScriptDocument^ AllocateNewDocument();
	void AddDocument(IScriptDocument^ Document);
	void RemoveDocument(IScriptDocument^ Document);
	bool ContainsDocument(IScriptDocument^ Document);
	bool ContainsDocument(String^ EditorId);
	IScriptDocument^ LookupDocument(String^ EditorId);
};


} // namespace model


} // namespace scriptEditor


} // namespace cse