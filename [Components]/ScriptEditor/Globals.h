#pragma once

#include "[Common]\NativeWrapper.h"

namespace cse
{
	ref class Globals
	{
		static ImageResourceManager^	ScriptEditorImageResourceManager = nullptr;
	public:
		static ImageResourceManager^ ImageResources()
		{
			if (ScriptEditorImageResourceManager == nullptr)
				ScriptEditorImageResourceManager = gcnew ImageResourceManager("ScriptEditor.ImagesModern");

			return ScriptEditorImageResourceManager;
		}

		static int						MainThreadID = -1;
		static System::Threading::Tasks::TaskScheduler^
										MainThreadTaskScheduler = nullptr;
	};

#define SetupControlImage(Identifier)							Identifier##->Name = #Identifier;	\
																Identifier##->Image = Globals::ImageResources()->CreateImage(#Identifier);

#define DisposeControlImage(Identifier)							delete Identifier##->Image; \
																Identifier##->Image = nullptr
}
