#pragma once

#include "[Common]\NativeWrapper.h"

namespace ConstructionSetExtender
{
	ref class Globals
	{
	public:
		static String^												AppPath = gcnew String(NativeWrapper::g_CSEInterfaceTable->EditorAPI.GetAppPath());
		static ImageResourceManager^								ScriptEditorImageResourceManager = gcnew ImageResourceManager("ScriptEditor.ImagesModern");
		static int													MainThreadID = -1;
		static System::Threading::Tasks::TaskScheduler^				MainThreadTaskScheduler = nullptr;
	};

#define SetupControlImage(Identifier)							Identifier##->Name = #Identifier;	\
																Identifier##->Image = Globals::ScriptEditorImageResourceManager->CreateImage(#Identifier);

#define DisposeControlImage(Identifier)							delete Identifier##->Image; \
																Identifier##->Image = nullptr
}