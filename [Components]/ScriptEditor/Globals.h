#pragma once

#include "[Common]\NativeWrapper.h"

namespace ConstructionSetExtender
{
	ref class Globals
	{
	public:
		static String^												AppPath = gcnew String(NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetAppPath());
		static ImageResourceManager^								ScriptEditorImageResourceManager = gcnew ImageResourceManager("CSEScriptEditor.Images");
	};

#define SetupControlImage(Identifier)							Identifier##->Name = #Identifier;	\
																Identifier##->Image = Globals::ScriptEditorImageResourceManager->CreateImageFromResource(#Identifier);	\
																Identifier##->ImageTransparentColor = Color::White
#define DisposeControlImage(Identifier)						delete Identifier##->Image
}