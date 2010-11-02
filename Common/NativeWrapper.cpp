#include "NativeWrapper.h"
#include "ScriptEditor\Globals.h"

void NativeWrapper::PrintToCSStatusBar(int PanelIndex, String^ Message)
{
	CStringWrapper^ CStr = gcnew CStringWrapper(Message);
	WriteStatusBarText(PanelIndex, CStr->String());
}

String^ INIWrapper::GetINIValue(String^ Section, String^ Key, String^ Default)
{
	CStringWrapper^ CSection = gcnew CStringWrapper(Section),
					^CKey = gcnew CStringWrapper(Key),
					^CDefault = gcnew CStringWrapper(Default);
	return gcnew String(GetINIString(CSection->String(), CKey->String(), CDefault->String()));
}

void INIWrapper::SetINIValue(String^ Section, String^ Key, String^ Value)
{
	INIWrapper::WritePrivateProfileString(Section, Key, Value, Globals::INIPath);
}