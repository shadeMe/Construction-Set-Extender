#pragma once
#include "Common\Includes.h"
#include "ScriptEditor.h"

using namespace System::ComponentModel;

// TODO: ++++++++++++++++++
//		> Replace ugly switch code with a generic function



ref class IntelliSenseItem;
ref class CommandInfo;
ref class VariableInfo;
ref class UserFunction;
ref class SyntaxBox;
ref class Script;
ref class SyntaxBoxInitializer;
ref class Quest;
struct CommandTableData;


public ref struct Boxer
{
	SyntaxBox^											ISBox;						
	Script^												ScptBox;
	UserFunction^										FunctBox;

	static enum class									BoxType
														{
															e_UserFunct = 0,
															e_Script,
															e_SyntaxBox
														};
	
	BoxType												Type;

	Boxer(SyntaxBox^ Obj) : ISBox(Obj), Type(BoxType::e_SyntaxBox) {};
	Boxer(Script^ Obj) : ScptBox(Obj), Type(BoxType::e_Script) {};
	Boxer(UserFunction^ Obj) : FunctBox(Obj), Type(BoxType::e_UserFunct) {};
};

public ref class IntelliSenseDatabase								
{
	static IntelliSenseDatabase^						Singleton = nullptr;
	IntelliSenseDatabase();

	ref struct ParsedPluginData
	{
		LinkedList<UserFunction^>^						UDFList;
		LinkedList<Quest^>^								QuestList;
		LinkedList<String^>^							ActiveScriptRecords;

		ParsedPluginData() : UDFList(gcnew LinkedList<UserFunction^>()), QuestList(gcnew LinkedList<Quest^>()), ActiveScriptRecords(gcnew LinkedList<String^>()) {}
	};


	BackgroundWorker^									PluginParserThread;
	ParsedPluginData^									DoUpdateDatabase(String^ PluginName);
	void												PostUpdateDatabase(ParsedPluginData^ Data);
	void												PluginParserThread_DoWork(Object^ Sender, DoWorkEventArgs^ E);
	void												PluginParserThread_RunWorkerCompleted(Object^ Sender, RunWorkerCompletedEventArgs^ E);

	LinkedList<UserFunction^>^							UserFunctionList;
	LinkedList<Quest^>^									QuestList;
	Dictionary<String^, String^>^						URLMap;
	Dictionary<String^, Script^>^						RemoteScripts;				// key = baseEditorID
	LinkedList<String^>^								ActiveScriptRecords;		// used by the recompile all hook
public:
	LinkedList<IntelliSenseItem^>^						Enumerables;

	void												ParseCommandTable(CommandTableData* Data);

	static IntelliSenseDatabase^%						GetSingleton();


	static void											ParseScript(String^% SourceText, Boxer^ Box);
	void												AddToURLMap(String^% CmdName, String^% URL);
	void												UpdateDatabase(String^ PluginName);
	String^												GetCommandURL(String^% CmdName);
	Script^												GetRemoteScript(String^ BaseEditorID, String^ ScriptText);
	bool												IsUDF(String^% Name);
	bool												IsCommand(String^ Name);
	bool												IsActiveScriptRecord(String^% EditorID);
};

#define ISDB											IntelliSenseDatabase::GetSingleton()

public ref class IntelliSenseItem								// enumerable in the syntax box
{
	static array<String^>^								TypeIdentifier =
															{
																"Unknown Object",
																"Command",
																"Local Variable",
																"Remote Variable",
																"User Function",
																"Quest"
															};
public:
	static enum class									ItemType
															{
																e_Invalid = 0,
																e_Cmd,
																e_LocalVar,
																e_RemoteVar,
																e_UserFunct,
																e_Quest
															};

	String^%											Describe() { return Description; }

	IntelliSenseItem(String^% Desc, ItemType Type) : Description(Desc), Type(Type) {};

	virtual String^%									GetIdentifier() { return Name; }
	ItemType%											GetType() { return Type; }
	String^%											GetTypeIdentifier() { return TypeIdentifier[(int)Type]; }
protected:
	String^												Description;
	String^												Name;
	ItemType											Type;
};



public ref class CommandInfo : public IntelliSenseItem
{
	static array<String^>^								TypeIdentifier =
															{
																"Numeric",
																"Form",
																"String",
																"Array",
																"Array [Reference]",
																"Ambiguous"
															};
public:
	static enum class									CmdReturnType
															{
																e_Default = 0,
																e_Form,
																e_String,
																e_Array,
																e_ArrayIndex,
																e_Ambiguous
															};
														
	String^												Name;
	String^												Description;
	String^												Shorthand;
	UInt16												ParamCount;
	bool												RequiresParent;
	UInt16												ReturnType;


	CommandInfo(String^% Name, String^% Desc, String^% Shorthand, UInt16 NoOfParams, bool RequiresParent, UInt16 ReturnType) : 
	IntelliSenseItem(String::Format("{0}{1}\n{2} parameter(s)\nReturn Type: {3}\n\n{4}{5}", Name, (Shorthand == "None")?"":("\t[ " + Shorthand + " ]"), NoOfParams.ToString(), CommandInfo::TypeIdentifier[(int)ReturnType], Desc, (RequiresParent)?"\n\nRequires a calling reference":""), ItemType::e_Cmd),
										 Name(Name), 
										 Description(Desc), 
										 Shorthand(Shorthand), 
										 ParamCount(NoOfParams),
										 RequiresParent(RequiresParent),
										 ReturnType(ReturnType)		{};

	virtual String^%									GetIdentifier() override { return Name; }
};

public ref class VariableInfo : public IntelliSenseItem
{
public:
	static array<String^>^								TypeIdentifier =
															{
																"Integer",
																"Float",
																"Reference",
																"String Variable",
																"Array Variable"
															};

	static enum class									VariableType
															{
																e_Int = 0,
																e_Float,
																e_Ref,
																e_String,
																e_Array
															};
	String^												Name;
	VariableType										Type;
	String^												Comment;			

	VariableInfo(String^% Name, String^% Comment, VariableType Type, ItemType Scope) : 
	IntelliSenseItem(String::Format("{0} [{1}]{2}{3}", Name, VariableInfo::TypeIdentifier[(int)Type], (Comment != "")?"\n\n":"", Comment), Scope), 
										 Name(Name), 
										 Type(Type), 
										 Comment(Comment) {};											

	virtual String^%									GetIdentifier() override { return Name; }
};


public ref class Script
{
public:
	static Script^										NullScript = gcnew Script(gcnew String("scn nullscript"));
	List<VariableInfo^>^								VarList;
	String^												Name;
	String^												Description;

	Script(String^% ScriptText);

	virtual String^										Describe();
protected:
	Script();
	Script(String^% ScriptText, String^% Name);
};

public ref class Quest : public Script
{
public:
	Quest(String^% ScriptText, String^% EditorID) : Script(ScriptText, EditorID) {}
};

public ref class UserFunction : public Script
{
public:
	Array^												Parameters;			// indices of the parameters in VarList
	int													ReturnVar;			// index of the return var. -9 for ambiguous retn values

	UserFunction(String^% ScriptText);

	virtual String^										Describe() override;
	void												DumpData();
};

public ref class UserFunctionDelegate : public IntelliSenseItem	
{
public:
	UserFunction^										Parent;

	UserFunctionDelegate(UserFunction^% Parent) : Parent(Parent), IntelliSenseItem(Parent->Describe(), ItemType::e_UserFunct) {}

	virtual String^%									GetIdentifier() override { return Parent->Name; }
};

public ref class QuestDelegate : public IntelliSenseItem	
{
public:
	Quest^												Parent;

	QuestDelegate(Quest^% Parent) : Parent(Parent), IntelliSenseItem(Parent->Describe(), ItemType::e_Quest) {}

	virtual String^%									GetIdentifier() override { return Parent->Name; }
};


public ref class SyntaxBox
{
	void												Cleanup();

	void												IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
	void												IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void												IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);

	static ToolTip^										InfoTip = gcnew ToolTip();
	static ImageList^									Icons = gcnew ImageList();

	int													GetSelectedIndex();
	VariableInfo^										GetLocalVar(String^% Identifier);
public:
	static enum class									Operation
															{
																e_Default = 0,
																e_Call,
																e_Dot,
																e_Assign
															};
	static enum	class									Direction
															{
																e_Up = 0,
																e_Down
															};

	bool												CanShow;
	ScriptEditor::Workspace^							ParentEditor;
	Operation											LastOperation;
	Script^												RemoteScript;
	bool												IsObjRefr;

	List<IntelliSenseItem^>^							VarList;														// local variables
	List<IntelliSenseItem^>^							ListContents;													// handles of the list's items
	ListView^											IntelliSenseList;
	void												Initialize(SyntaxBox::Operation Op, bool Force, bool InitAll);
	void												Hide();

	SyntaxBox(ScriptEditor::Workspace^% Parent);
	void												PickIdentifier();
	bool												IsVisible()	{ return IntelliSenseList->Visible; }
	void												MoveIndex(Direction Direction);
	void												UpdateLocalVars();
	bool												QuickView(String^ TextUnderMouse);
};
