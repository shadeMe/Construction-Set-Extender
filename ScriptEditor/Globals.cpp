#include "Globals.h"
#include "ScriptEditor.h"
#include "ScriptEditorManager.h"
#include "Common\NativeWrapper.h"
#include "IntelliSense.h"

Globals^% Globals::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew Globals();
	}
	return Singleton;
}
Globals::Globals()
{
	ImageResources = gcnew ResourceManager("CSEScriptEditor.Images", Assembly::GetExecutingAssembly());
	AppPath = gcnew String(NativeWrapper::GetAppPath());
	INIPath = gcnew String(NativeWrapper::GetAppPath()) + gcnew String("Data\\OBSE\\Plugins\\Construction Set Extender.ini");
	MouseLocation = Point(0,0);
	Delimiters = gcnew String("., (){}[]\t\n");
	ControlChars = " \t";


	TabDelimit = Array::CreateInstance(char::typeid, 1);
	PipeDelimit = Array::CreateInstance(char::typeid, 1);
	DelimiterKeys = Array::CreateInstance(Keys::typeid, 7);

	TabDelimit->SetValue('\t', 0);
	PipeDelimit->SetValue('|', 0);
	DelimiterKeys->SetValue(Keys::OemPeriod, 0);
	DelimiterKeys->SetValue(Keys::Oemcomma, 1);
	DelimiterKeys->SetValue(Keys::Space, 2);
	DelimiterKeys->SetValue(Keys::OemOpenBrackets, 3);
	DelimiterKeys->SetValue(Keys::OemCloseBrackets, 4);
	DelimiterKeys->SetValue(Keys::Tab, 5);
	DelimiterKeys->SetValue(Keys::Enter, 6);

	PosPointerImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("IndexPointer")));
	ISEmptyImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("ISEmpty")));
	ISCommandImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("ISCommand")));
	ISLocalVarImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("ISLocalVar")));
	ISRemoteVarImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("ISRemoteVar")));
	ISUserFImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("ISUserF")));
	ISQuestImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("ISQuest")));


	SENewImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SENew")));
	SEOpenImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEOpen")));
	SEPreviousImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEPrevious")));
	SENextImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SENext")));
	SESaveImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SESave")));
	SEDeleteImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEDelete")));
	SERecompileImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SERecompile")));
	SEOptionsImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEOptions")));
	SEErrorListImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEErrorList")));
	SEFindListImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEFindList")));
	SEBookmarkListImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEBookmarkList")));
	SEConsoleImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEConsole")));
	SEDumpScriptImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEDumpScript")));
	SELoadScriptImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SELoadScript")));
	SEOffsetImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEOffset")));
	SEErrorImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEError")));
	SEWarningImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEWarning")));
	SENavBackImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SENavBack")));
	SENavForwardImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SENavForward")));
	SELineLimitImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SELineLimit")));
	SEVarIdxUpdateImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEVarIdxUpdate")));
	SEVarIdxListImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEVarIdxList")));

	SLDActiveImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SLDActive")));
	SLDDeletedImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SLDDeleted")));

	SEContextCopyImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextCopy")));
	SEContextPasteImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextPaste")));
	SEContextFindImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextFind")));
	SEContextCommentImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextComment")));
	SEContextBookmarkImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextBookmark")));
	SEContextCTBImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextCTB")));
	SEContextLookupImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextLookup")));
	SEContextDevLinkImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextDevLink")));
	SEContextJumpImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SEContextJump")));
	SENewTabImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SENewTab")));
	SESaveAllImg = gcnew Bitmap(dynamic_cast<Image^>(ImageResources->GetObject("SESaveAll")));

	ScriptEditorOptions = OptionsDialog::GetSingleton();
}

ConsoleManager^% ConsoleManager::GetSingleton()
{
	if (Singleton == nullptr) {
		Singleton = gcnew ConsoleManager();
		DebugPrint("Initialized Console");
	}
	return Singleton;
}

void ConsoleManager::Log(String^% Message, bool PrintC)
{
	MessageDump += Message->Replace("\n", "\r\n") + "\r\n";
	
	if (!PrintC || ScriptEditorManager::GetSingleton()->ActiveEditor == nullptr)		return;

	ScriptEditorManager::GetSingleton()->ActiveEditor->ConsoleBox->Text = GetDump();
	ScriptEditorManager::GetSingleton()->ActiveEditor->ConsoleBox->SelectionStart = MessageDump->Length - 1;
	ScriptEditorManager::GetSingleton()->ActiveEditor->ConsoleBox->ScrollToCaret(); 
}

void DebugPrint(String^ Message, bool PrintC, bool Achtung)	
{
	ConsoleManager::GetSingleton()->Log(Message, PrintC);
	if (Achtung)						Media::SystemSounds::Hand->Play();

	DumpToLog(Message);
}