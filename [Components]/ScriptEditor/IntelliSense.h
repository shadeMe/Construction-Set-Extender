#pragma once

using namespace System::ComponentModel;

struct CommandTableData;
struct IntelliSenseUpdateData;

namespace IntelliSense
{
	ref class IntelliSenseItem;
	ref class CommandInfo;
	ref class VariableInfo;
	ref class UserFunction;
	ref class IntelliSenseThingy;
	ref class Script;
	ref struct Boxer;

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
		void ParseGMSTCollection(IntelliSenseUpdateData* GMSTCollection);

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
																	"Quest",
																	"Global Variable",
																	"Game Setting"
																};
	public:
		static enum class									ItemType
																{
																	e_Invalid = 0,
																	e_Cmd,
																	e_LocalVar,
																	e_RemoteVar,
																	e_UserFunct,
																	e_Quest,
																	e_GlobalVar,
																	e_GMST
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
	public:
		static enum class									SourceType
																{
																	e_Vanilla = 0,
																	e_OBSE
																};
	private:
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
		SourceType											Source;
	public:
		CommandInfo(String^% Name, String^% Desc, String^% Shorthand, UInt16 NoOfParams, bool RequiresParent, UInt16 ReturnType, SourceType Source) :
		  IntelliSenseItem(String::Format("{0}{1}\n{2} parameter(s)\nReturn Type: {3}\n\n{4}{5}", Name, (Shorthand == "None")?"":("\t[ " + Shorthand + " ]"), NoOfParams.ToString(), CommandInfo::TypeIdentifier[(int)ReturnType], Desc, (RequiresParent)?"\n\nRequires a calling reference":""), ItemType::e_Cmd),
			  Name(Name),
			  Description(Desc),
			  Shorthand(Shorthand),
			  ParamCount(NoOfParams),
			  RequiresParent(RequiresParent),
			  ReturnType(ReturnType),
			  Source(Source)	{};

		  virtual String^%									GetIdentifier() override { return Name; }
		  bool												GetRequiresParent() { return RequiresParent; }
		  SourceType										GetSource() { return Source; }
	};

	public ref class VariableInfo : public IntelliSenseItem
	{
	public:
		static array<String^>^								TypeIdentifier =
																{
																	"Integer",
																	"Float",
																	"Reference",
																	"String",
																	"Array"
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
		  String^											GetTypeIdentifier() { return TypeIdentifier[(int)Type]; }
		  String^%											GetName() { return Name; }
		  VariableType										GetVariableType() { return Type; }
	};

	public ref class Script
	{
	public:
		typedef List<VariableInfo^>							_VarList;
	protected:
		Script();
		Script(String^% ScriptText, String^% Name);

		_VarList^											VarList;
		String^												Name;
		String^												Description;
	public:
		static Script^										NullScript = gcnew Script(gcnew String("scn nullscript"));

		Script(String^% ScriptText);

		virtual String^										Describe();
		String^%											GetIdentifier() { return Name; }

		void												SetName(String^ Name) { this->Name = Name; }
		void												SetDescription(String^ Description) { this->Description = Description; }

		void												AddVariable(VariableInfo^ Variable) { VarList->Add(Variable); }
		void												ClearVariableList() { VarList->Clear(); }
		List<VariableInfo^>::Enumerator^					GetVariableListEnumerator() { return VarList->GetEnumerator(); }
	};

	public ref class Quest : public IntelliSenseItem
	{
	protected:
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
	protected:
		Array^												Parameters;			// indices of the parameters in VarList
		int													ReturnVar;			// index of the return var. -9 for ambiguous retn values
	public:
		UserFunction(String^% ScriptText);

		virtual String^										Describe() override;

		void												AddParameter(int VariableIndex, int ParameterIndex) { Parameters->SetValue(VariableIndex, ParameterIndex); }
		void												SetReturnVariable(int VariableIndex) { ReturnVar = VariableIndex; }
	};

	public ref class UserFunctionDelegate : public IntelliSenseItem
	{
		UserFunction^										Parent;
	public:
		UserFunctionDelegate(UserFunction^% Parent) : Parent(Parent), IntelliSenseItem(Parent->Describe(), ItemType::e_UserFunct) {}

		virtual String^%									GetIdentifier() override { return Parent->GetIdentifier(); }
	};

	public ref class NonActivatingImmovableForm : public Form
	{
	protected:
		property bool										ShowWithoutActivation
		{
			virtual bool									get() override { return true; }
		}

		virtual void										WndProc(Message% m) override;

		bool												AllowMove;
	public:
		void												ShowAtLocation(Drawing::Point Position, IntPtr ParentHandle);
		void												SetSize(Drawing::Size WindowSize);

		NonActivatingImmovableForm() : AllowMove(false), Form() {}
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

		void												Initialize(IntelliSenseThingy::Operation Op, bool Force, bool InitAll);
		void												Hide();

		IntelliSenseThingy(Object^% Parent);
		void												PickIdentifier();
		bool												IsVisible()	{ return IntelliSenseList->Visible; }
		void												MoveIndex(Direction Direction);
		void												UpdateLocalVars();
		bool												QuickView(String^ TextUnderMouse);
		bool												QuickView(String^ TextUnderMouse, Point MouseLocation);
		void												Destroy() { Destroying= true; IntelliSenseBox->Close(); }			

		property Operation									LastOperation;
		property bool										Enabled;
	private:
		void												Cleanup();

		void												IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
		void												IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E);
		void												IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);
		void												IntelliSenseBox_Cancel(Object^ Sender, CancelEventArgs^ E);

		static ToolTip^										InfoTip = gcnew ToolTip();
		static ImageList^									Icons = gcnew ImageList();

		int													GetSelectedIndex();
		VariableInfo^										GetLocalVar(String^% Identifier);
		bool												ShowQuickInfoTip(String^ TextUnderMouse, Point TipLoc);

		Object^												ParentEditor;		// declared as an Object^ to work around a cyclic dependency
		Script^												RemoteScript;
		bool												IsObjRefr;
		bool												Destroying;

		List<IntelliSenseItem^>^							ListContents;		// handles of the list's items
		ListView^											IntelliSenseList;
		NonActivatingImmovableForm^							IntelliSenseBox;
		List<IntelliSenseItem^>^							LocalVarList;													
	public:
		void												HideInfoTip() { InfoTip->Hide(Control::FromHandle(IntelliSenseList->Parent->Handle)); }

		void												AddLocalVariable(VariableInfo^ Variable) { LocalVarList->Add(Variable); }
		void												ClearLocalVariableList() { LocalVarList->Clear(); }		
	};

	public ref struct Boxer
	{
		IntelliSenseThingy^									SourceIntelliSenseThingy;
		Script^												SourceScript;

		static enum class									BoxType
		{
			e_UserFunction = 0,
			e_Script,
			e_IntelliSenseThingy
		};

		BoxType												Type;

		Boxer(IntelliSenseThingy^ Obj) : SourceIntelliSenseThingy(Obj), Type(BoxType::e_IntelliSenseThingy) {};
		Boxer(Script^ Obj) : SourceScript(Obj), Type(BoxType::e_Script) {};
		Boxer(UserFunction^ Obj) : SourceScript(Obj), Type(BoxType::e_UserFunction) {};
	};
}