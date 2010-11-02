#pragma once

#include "Common\NativeWrapper.h"
#include "Common\Includes.h"
#include "Common\MiscUtilities.h"

public ref class Globals
{
public:
	static ResourceManager^										ImageResources = gcnew ResourceManager("CSEScriptEditor.Images", Assembly::GetExecutingAssembly());
	static String^												AppPath = gcnew String(NativeWrapper::GetAppPath());
	static String^												INIPath = gcnew String(NativeWrapper::GetAppPath()) + gcnew String("Data\\OBSE\\Plugins\\Construction Set Extender.ini");
	static Point												MouseLocation = Point(0,0);
	static String^												Delimiters = gcnew String("., (){}[]\t\n");
	static String^												ControlChars = " \t";

	static array<Char>^											TabDelimit = { '\t' };
	static array<Char>^											PipeDelimit = { '|' };
	static array<Keys>^											DelimiterKeys = 
																{
																	Keys::OemPeriod,
																	Keys::Oemcomma,
																	Keys::Space,
																	Keys::OemOpenBrackets,
																	Keys::OemCloseBrackets,
																	Keys::Tab,
																	Keys::Enter
																};
};