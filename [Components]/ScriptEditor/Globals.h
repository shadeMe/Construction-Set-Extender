#pragma once

#ifndef CSE_SEPREPROC
#include "[Common]\NativeWrapper.h"
#endif

ref class Globals
{
public:
#ifndef CSE_SEPREPROC
	static String^												AppPath = gcnew String(g_CSEInterface->CSEEditorAPI.GetAppPath());
#endif
	static String^												ScriptTextDelimiters = gcnew String("., (){}[]\t\n");
	static String^												ScriptTextControlChars = " \t";

	static array<Keys>^											ScriptTextDelimiterKeys =
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

		for each (Keys Itr in ScriptTextDelimiterKeys)
		{
			if (Itr == KeyCode) {
				Result = true;
				break;
			}
		}
		return Result;
	}
#ifndef CSE_SEPREPROC
	static array<System::Windows::Input::Key>^					ScriptTextDelimiterKeysWPF =
																{
																	System::Windows::Input::Key::OemPeriod,
																	System::Windows::Input::Key::OemComma,
																	System::Windows::Input::Key::Space,
																	System::Windows::Input::Key::OemOpenBrackets,
																	System::Windows::Input::Key::OemCloseBrackets,
																	System::Windows::Input::Key::Tab,
																	System::Windows::Input::Key::Enter
																};

	static bool GetIsDelimiterKey(System::Windows::Input::Key KeyCode)
	{
		bool Result = false;

		for each (System::Windows::Input::Key Itr in ScriptTextDelimiterKeysWPF)
		{
			if (Itr == KeyCode) {
				Result = true;
				break;
			}
		}
		return Result;
	}

	static ImageResourceManager^								ScriptEditorImageResourceManager = gcnew ImageResourceManager("CSEScriptEditor.Images");
#endif
};

#define SetupControlImage(Identifier)							Identifier##->Name = #Identifier;	\
																Identifier##->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource(#Identifier);	\
																Identifier##->ImageTransparentColor = Color::White