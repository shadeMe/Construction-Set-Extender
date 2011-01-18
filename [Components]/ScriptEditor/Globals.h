#pragma once

#include "[Common]\NativeWrapper.h"

public ref class Globals
{
public:
	static String^												AppPath = gcnew String(NativeWrapper::GetAppPath());
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

	static bool GetIsDelimiterKey(Keys KeyCode)
	{
		bool Result = false;

		for each (Keys Itr in DelimiterKeys)
		{
			if (Itr == KeyCode) {
				Result = true;
				break;
			}
		}
		return Result;
	}

	static ImageResourceManager^								ScriptEditorImageResourceManager = gcnew ImageResourceManager("CSEScriptEditor.Images");
};

#define SetupControlImage(Identifier)							Identifier##->Name = #Identifier;	\
																Identifier##->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource(#Identifier);	\
																Identifier##->ImageTransparentColor = Color::White