#include "MiscUtilities.h"
#include "NativeWrapper.h"

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
	const char* INIValue = GetINIString(CSection->String(), CKey->String(), CDefault->String());
	return gcnew String(INIValue);
}

void INIWrapper::SetINIValue(String^ Section, String^ Key, String^ Value)
{
	String^ INIPath = gcnew String(NativeWrapper::GetAppPath()) + gcnew String("Data\\OBSE\\Plugins\\Construction Set Extender.ini");
	INIWrapper::WritePrivateProfileString(Section, Key, Value, INIPath);
}