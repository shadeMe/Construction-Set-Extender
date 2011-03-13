#pragma once
#include "ScriptEditor.h"

using namespace System::ComponentModel;

ref class IntelliSenseItem;
ref class CommandInfo;
ref class VariableInfo;
ref class UserFunction;
ref class IntelliSenseThingy;
ref class Script;
struct CommandTableData;


public ref struct Boxer
{
	IntelliSenseThingy^									ISBox;						
	Script^												ScptBox;
	UserFunction^										FunctBox;

	static enum class									BoxType
														{
															e_UserFunct = 0,
															e_Script,
															e_SyntaxBox
														};
	
	BoxType												Type;

	Boxer(IntelliSenseThingy^ Obj) : ISBox(Obj), Type(BoxType::e_SyntaxBox) {};
	Boxer(Script^ Obj) : ScptBox(Obj), Type(BoxType::e_Script) {};
	Boxer(UserFunction^ Obj) : FunctBox(Obj), Type(BoxType::e_UserFunct) {};
};

public ref class IntelliSenseDatabase								
{
	static IntelliSenseDatabase^						Singleton = nullptr;

	IntelliSenseDatabase();

	ref struct ParsedUpdateData
	{
		LinkedList<UserFunction^>^						UDFList;
		LinkedList<IntelliSenseItem^>^					Enumerables;
	
		ParsedUpdateData() : UDFList(gcnew LinkedList<UserFunction^>()), Enumerables(gcnew LinkedList<IntelliSenseItem^>()) {}
	};


	Timers::Timer^										DatabaseUpdateTimer;
	BackgroundWorker^									DatabaseUpdateThread;
	ParsedUpdateData^									DoUpdateDatabase();
	void												PostUpdateDatabase(ParsedUpdateData^ Data);

	void												DatabaseUpdateTimer_OnTimed(Object^ Sender, Timers::ElapsedEventArgs^ E);
	void												DatabaseUpdateThread_DoWork(Object^ Sender, DoWorkEventArgs^ E);
	void												DatabaseUpdateThread_RunWorkerCompleted(Object^ Sender, RunWorkerCompletedEventArgs^ E);

	LinkedList<UserFunction^>^							UserFunctionList;
	Dictionary<String^, String^>^						URLMap;
	Dictionary<String^, Script^>^						RemoteScripts;				// key = baseEditorID, value = scriptID

	bool												ForceUpdateFlag;
	UInt32												UpdateThreadTimerInterval;	// in minutes

	void												UpdateDatabase();
public:
	LinkedList<IntelliSenseItem^>^						Enumerables;

	void												ParseCommandTable(CommandTableData* Data);

	static IntelliSenseDatabase^%						GetSingleton();


	static void											ParseScript(String^% SourceText, Boxer^ Box);
	void												AddToURLMap(String^% CmdName, String^% URL);
	String^												GetCommandURL(String^% CmdName);
	String^												SanitizeCommandName(String^% CmdName);
	Script^												GetRemoteScript(String^ BaseEditorID, String^ ScriptText);
	bool												IsUDF(String^% Name);
	bool												IsCommand(String^% Name);
	void												ForceUpdateDatabase();
	void												InitializeDatabaseUpdateTimer();
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

	IntelliSenseItem(String^ Desc, ItemType Type) : Description(Desc), Type(Type) {};

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
public:		
	CommandInfo(String^% Name, String^% Desc, String^% Shorthand, UInt16 NoOfParams, bool RequiresParent, UInt16 ReturnType) : 
	IntelliSenseItem(String::Format("{0}{1}\n{2} parameter(s)\nReturn Type: {3}\n\n{4}{5}", Name, (Shorthand == "None")?"":("\t[ " + Shorthand + " ]"), NoOfParams.ToString(), CommandInfo::TypeIdentifier[(int)ReturnType], Desc, (RequiresParent)?"\n\nRequires a calling reference":""), ItemType::e_Cmd),
										 Name(Name), 
										 Description(Desc), 
										 Shorthand(Shorthand), 
										 ParamCount(NoOfParams),
										 RequiresParent(RequiresParent),
										 ReturnType(ReturnType)		{};

	virtual String^%									GetIdentifier() override { return Name; }
	bool												GetRequiresParent() { return RequiresParent; }
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
private:
	String^												Name;
	VariableType										Type;
	String^												Comment;	
public:
	VariableInfo(String^% Name, String^% Comment, VariableType Type, ItemType Scope) : 
	IntelliSenseItem(String::Format("{0} [{1}]{2}{3}", Name, VariableInfo::TypeIdentifier[(int)Type], (Comment != "")?"\n\n":"", Comment), Scope), 
										 Name(Name), 
										 Type(Type), 
										 Comment(Comment) {};											

	virtual String^%									GetIdentifier() override { return Name; }
	String^%											GetComment() { return Comment; }
	String^												GetTypeIdentifier() { return TypeIdentifier[(int)Type]; }
	String^%											GetName() { return Name; }
	VariableType										GetType() { return Type; }
};


public ref class Script
{
protected:
	Script();
	Script(String^% ScriptText, String^% Name);
public:
	static Script^										NullScript = gcnew Script(gcnew String("scn nullscript"));

	List<VariableInfo^>^								VarList;
	String^												Name;
	String^												Description;

	Script(String^% ScriptText);

	virtual String^										Describe();
	String^%											GetScriptName() { return Name; }
};

public ref class Quest : public IntelliSenseItem 
{
	String^												Name;
	String^												ScriptName;
public:
	Quest(String^% EditorID, String^% Desc, String^% ScrName) :
															IntelliSenseItem((gcnew String(EditorID + ((Desc != "")?"\n":"") + Desc + ((ScrName != "")?"\n\nQuest Script: ":"") + ScrName)), IntelliSenseItem::ItemType::e_Quest),
															Name(EditorID), ScriptName(ScrName) {}

	virtual String^%									GetIdentifier() override { return Name; }
};

public ref class UserFunction : public Script
{
public:
	Array^												Parameters;			// indices of the parameters in VarList
	int													ReturnVar;			// index of the return var. -9 for ambiguous retn values

	UserFunction(String^% ScriptText);

	virtual String^										Describe() override;
};

public ref class UserFunctionDelegate : public IntelliSenseItem	
{
	UserFunction^										Parent;
public:
	UserFunctionDelegate(UserFunction^% Parent) : Parent(Parent), IntelliSenseItem(Parent->Describe(), ItemType::e_UserFunct) {}

	virtual String^%									GetIdentifier() override { return Parent->Name; }
};

public ref class IntelliSenseThingy
{
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

	List<IntelliSenseItem^>^							VarList;														// local variables

	void												Initialize(IntelliSenseThingy::Operation Op, bool Force, bool InitAll);
	void												Hide();

	IntelliSenseThingy(ScriptEditor::Workspace^% Parent);
	void												PickIdentifier();
	bool												IsVisible()	{ return IntelliSenseList->Visible; }
	void												MoveIndex(Direction Direction);
	void												UpdateLocalVars();
	bool												QuickView(String^ TextUnderMouse);

	property Operation									LastOperation;
	property bool										Enabled;
private:
	void												Cleanup();

	void												IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
	void												IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E);
	void												IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);

	static ToolTip^										InfoTip = gcnew ToolTip();
	static ImageList^									Icons = gcnew ImageList();

	int													GetSelectedIndex();
	VariableInfo^										GetLocalVar(String^% Identifier);

	ScriptEditor::Workspace^							ParentEditor;
	Script^												RemoteScript;
	bool												IsObjRefr;

	List<IntelliSenseItem^>^							ListContents;													// handles of the list's items
	ListView^											IntelliSenseList;
public:
	void												HideInfoTip() { InfoTip->Hide(Control::FromHandle(IntelliSenseList->Parent->Handle)); }

	property ListView^									InternalListView
	{
		ListView^ get()
		{
			return IntelliSenseList;
		}
	}
};
