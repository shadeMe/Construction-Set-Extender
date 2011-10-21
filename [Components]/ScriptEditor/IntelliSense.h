#pragma once
#include "[Common]\HandShakeStructs.h"
#include "[Common]\AuxiliaryWindowsForm.h"

namespace IntelliSense
{
	ref class IntelliSenseItem;
	ref class CommandInfo;
	ref class VariableInfo;
	ref class UserFunction;
	ref class IntelliSenseInterface;
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

		ParsedUpdateData^									InitializeDatabaseUpdate();
		void												FinalizeDatabaseUpdate(ParsedUpdateData^ Data);

		void												DatabaseUpdateTimer_OnTimed(Object^ Sender, Timers::ElapsedEventArgs^ E);
		void												DatabaseUpdateThread_DoWork(Object^ Sender, DoWorkEventArgs^ E);
		void												DatabaseUpdateThread_RunWorkerCompleted(Object^ Sender, RunWorkerCompletedEventArgs^ E);

		Timers::Timer^										DatabaseUpdateTimer;
		BackgroundWorker^									DatabaseUpdateThread;

		LinkedList<UserFunction^>^							UserFunctionList;
		Dictionary<String^, String^>^						DeveloperURLMap;
		Dictionary<String^, Script^>^						RemoteScripts;				// key = baseEditorID

		bool												ForceUpdateFlag;
		UInt32												UpdateThreadTimerInterval;	// in minutes

		void												UpdateDatabase();
	public:
		LinkedList<IntelliSenseItem^>^						Enumerables;

		void												InitializeCommandTableDatabase(ComponentDLLInterface::CommandTableData* Data);
		void												InitializeGMSTDatabase(ComponentDLLInterface::IntelliSenseUpdateData* GMSTCollection);

		static IntelliSenseDatabase^%						GetSingleton();
		static void											ParseScript(String^% SourceText, Boxer^ Box);

		void												RegisterDeveloperURL(String^% CmdName, String^% URL);
		String^												LookupDeveloperURLByCommand(String^% CmdName);

		String^												SanitizeCommandIdentifier(String^% CmdName);

		Script^												CacheRemoteScript(String^ BaseEditorID, String^ ScriptText);	// returns the cached script
		IntelliSenseItem^									LookupRemoteScriptVariable(String^ BaseEditorID, String^ Variable);

		bool												GetIsIdentifierUserFunction(String^% Name);
		bool												GetIsIdentifierScriptCommand(String^% Name);

		void												ForceUpdateDatabase();
		void												InitializeDatabaseUpdateThread();
	};

	#define ISDB											IntelliSenseDatabase::GetSingleton()

	public ref class IntelliSenseItem						// enumerable in the intellisense interface
	{
		static array<String^>^								TypeIdentifier =
																{
																	"Unknown Object",
																	"Command",
																	"Local Variable",
																	"Remote Variable",
																	"User Defined Function",
																	"Quest",
																	"Global Variable",
																	"Game Setting",
																	"Object"
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
																	e_GMST,
																	e_Form
																};

		String^												Describe() { return Description; }

		IntelliSenseItem(String^ Desc, ItemType Type) : Description(Desc), Type(Type) {};
		IntelliSenseItem() : Description(""), Type(ItemType::e_Invalid) {}

		virtual String^										GetIdentifier() = 0;
		ItemType											GetType() { return Type; }
		String^												GetTypeIdentifier() { return TypeIdentifier[(int)Type]; }
	protected:
		String^												Description;
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
		String^												CmdDescription;
		String^												Shorthand;
		UInt16												ParamCount;
		bool												RequiresParent;
		UInt16												ReturnType;
		SourceType											Source;
	public:
		CommandInfo(String^% Name, String^% Desc, String^% Shorthand, UInt16 NoOfParams, bool RequiresParent, UInt16 ReturnType, SourceType Source) :
		  IntelliSenseItem(String::Format("{0}{1}\n{2} parameter(s)\nReturn Type: {3}\n\n{4}{5}", Name, (Shorthand == "None")?"":("\t[ " + Shorthand + " ]"), NoOfParams.ToString(), CommandInfo::TypeIdentifier[(int)ReturnType], Desc, (RequiresParent)?"\n\nRequires a calling reference":""), ItemType::e_Cmd),
			  Name(Name),
			  CmdDescription(Desc),
			  Shorthand(Shorthand),
			  ParamCount(NoOfParams),
			  RequiresParent(RequiresParent),
			  ReturnType(ReturnType),
			  Source(Source)	{};

		  virtual String^									GetIdentifier() override { return Name; }
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
		VariableType										VarType;
		String^												Comment;
	public:
		VariableInfo(String^% Name, String^% Comment, VariableType Type, ItemType Scope) :
		  IntelliSenseItem(String::Format("{0} [{1}]{2}{3}", Name, VariableInfo::TypeIdentifier[(int)Type], (Comment != "")?"\n\n":"", Comment), Scope),
			  Name(Name),
			  VarType(Type),
			  Comment(Comment) {};

		  virtual String^									GetIdentifier() override { return Name; }
		  String^											GetComment() { return Comment; }
		  String^											GetTypeIdentifier() { return TypeIdentifier[(int)Type]; }
		  String^											GetName() { return Name; }
		  VariableType										GetVariableType() { return VarType; }
	};

	public ref class Script
	{
	public:
		typedef List<VariableInfo^>							VarListT;
	protected:
		Script();
		Script(String^% ScriptText, String^% Name);

		VarListT^											VarList;
		String^												Name;
		String^												Description;
	public:
		static Script^										NullScript = gcnew Script(gcnew String("scn nullscript"));

		Script(String^% ScriptText);

		virtual String^										Describe();
		String^												GetIdentifier() { return Name; }

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

		  virtual String^									GetIdentifier() override { return Name; }
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
		UserFunctionDelegate(UserFunction^% Parent) : IntelliSenseItem(Parent->Describe(), ItemType::e_UserFunct), Parent(Parent) {}

		virtual String^										GetIdentifier() override { return Parent->GetIdentifier(); }
	};

	public ref class EditorIDForm : public IntelliSenseItem
	{
	protected:
		static enum class									FormFlags
																{
																	e_FromMaster           = /*00*/ 0x00000001,
																	e_FromActiveFile       = /*01*/ 0x00000002,
																	e_Deleted              = /*05*/ 0x00000020,
																	e_TurnOffFire          = /*07*/ 0x00000080,
																	e_QuestItem            = /*0A*/ 0x00000400,
																	e_Disabled             = /*0B*/ 0x00000800,
																	e_Ignored              = /*0C*/ 0x00001000,
																	e_Temporary            = /*0E*/ 0x00004000,
																	e_VisibleWhenDistant   = /*0F*/ 0x00008000,
																};

		String^												Name;
		UInt32												FormType;
		UInt32												FormID;
		UInt32												Flags;

		String^												GetFormTypeIdentifier();
	public:
		EditorIDForm(ComponentDLLInterface::FormData* Data);

		virtual String^										GetIdentifier() override { return Name; }
	};

	public ref class IntelliSenseInterface
	{
	private:
		void												Destroy();
		void												CleanupInterface();

		void												IntelliSenseList_SelectedIndexChanged(Object^ Sender, EventArgs^ E);
		void												IntelliSenseList_KeyDown(Object^ Sender, KeyEventArgs^ E);
		void												IntelliSenseList_MouseDoubleClick(Object^ Sender, MouseEventArgs^ E);
		void												IntelliSenseBox_Cancel(Object^ Sender, CancelEventArgs^ E);

		static ImageList^									IntelliSenseItemIcons = gcnew ImageList();
		static ToolTip^										InfoToolTip = gcnew ToolTip();

		VariableInfo^										LookupLocalVariableByIdentifier(String^% Identifier);
		bool												ShowQuickInfoTip(String^ MainToken, String^ ParentToken, Point Location);

		void												DisplayInfoToolTip(String^ Title, String^ Message, Point Location, IntPtr ParentHandle, UInt32 Duration);

		Object^												ParentEditor;				// ScriptEditor::Workspace^
		bool												Destroying;
		bool												CallingObjectIsRef;
		Script^												RemoteScript;

		List<IntelliSenseItem^>^							CurrentListContents;
		List<IntelliSenseItem^>^							LocalVariableDatabase;

		ListView^											IntelliSenseList;
		NonActivatingImmovableAnimatedForm^					IntelliSenseBox;
	public:
		IntelliSenseInterface(Object^% ParentWorkspace);
		~IntelliSenseInterface()
		{
			Destroy();
		}

		static enum class									Operation
															{
																e_Default = 0,
																e_Call,
																e_Dot,
																e_Assign
															};

		static enum	class									MoveDirection
															{
																e_Up = 0,
																e_Down
															};

		property Operation									LastOperation;
		property bool										Enabled;
		property bool										Visible
		{
			virtual bool get() { return IntelliSenseBox->Visible; }
		}
		property UInt32										MaximumVisibleItemCount;

		void												ShowInterface(IntelliSenseInterface::Operation DisplayOperation, bool ForceDisplay, bool ShowAllItems);
		void												HideInterface();

		void												PickSelection();
		void												ChangeCurrentSelection(MoveDirection Direction);
		void												UpdateLocalVariableDatabase();
		bool												ShowQuickViewTooltip(String^ MainToken, String^ ParentToken);
		bool												ShowQuickViewTooltip(String^ MainToken, String^ ParentToken, Point MouseLocation);

		void												HideInfoToolTip();

		void												AddLocalVariableToDatabase(VariableInfo^ Variable) { LocalVariableDatabase->Add(Variable); }
		void												ClearLocalVariableDatabase() { LocalVariableDatabase->Clear(); }
	};

	public ref struct Boxer
	{
		IntelliSenseInterface^								SourceIntelliSenseInterface;
		Script^												SourceScript;

		static enum class									BoxType
															{
																e_UserFunction = 0,
																e_Script,
																e_IntelliSenseInterface
															};

		BoxType												Type;

		Boxer(IntelliSenseInterface^ Obj) : SourceIntelliSenseInterface(Obj), Type(BoxType::e_IntelliSenseInterface) {};
		Boxer(Script^ Obj) : SourceScript(Obj), Type(BoxType::e_Script) {};
		Boxer(UserFunction^ Obj) : SourceScript(Obj), Type(BoxType::e_UserFunction) {};
	};
}