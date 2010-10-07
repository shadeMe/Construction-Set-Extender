#pragma once
#include "Common\Includes.h"
#include "Common\MiscUtilities.h"
#include "OptionsDialog.h"

// TODO: ++++++++++++++++++
//		Why is this a singleton ?



public ref class Globals
{
	static Globals^										Singleton = nullptr;
public:
	ResourceManager^									ImageResources;
	String^												AppPath;
	String^												INIPath;
	Point												MouseLocation;
	String^												Delimiters;
	String^												ControlChars;

	Bitmap^												PosPointerImg;
	Bitmap^												ISEmptyImg;
	Bitmap^												ISCommandImg;
	Bitmap^												ISLocalVarImg;
	Bitmap^												ISRemoteVarImg;
	Bitmap^												ISUserFImg;
	Bitmap^												ISQuestImg;

	Bitmap^												SENewImg;
	Bitmap^												SEOpenImg;
	Bitmap^												SEPreviousImg;
	Bitmap^												SENextImg;
	Bitmap^												SESaveImg;
	Bitmap^												SEDeleteImg;
	Bitmap^												SERecompileImg;
	Bitmap^												SEOptionsImg;
	Bitmap^												SEErrorListImg;
	Bitmap^												SEFindListImg;
	Bitmap^												SEBookmarkListImg;
	Bitmap^												SEConsoleImg;
	Bitmap^												SEDumpScriptImg;
	Bitmap^												SELoadScriptImg;
	Bitmap^												SEOffsetImg;
	Bitmap^												SEErrorImg;
	Bitmap^												SEWarningImg;
	Bitmap^												SENavBackImg;
	Bitmap^												SENavForwardImg;
	Bitmap^												SELineLimitImg;
	Bitmap^												SEVarIdxUpdateImg;
	Bitmap^												SEVarIdxListImg;

	Bitmap^												SLDActiveImg;
	Bitmap^												SLDDeletedImg;

	Bitmap^												SEContextCopyImg;
	Bitmap^												SEContextPasteImg;
	Bitmap^												SEContextFindImg;
	Bitmap^												SEContextCommentImg;
	Bitmap^												SEContextBookmarkImg;
	Bitmap^												SEContextCTBImg;
	Bitmap^												SEContextLookupImg;
	Bitmap^												SEContextDevLinkImg;
	Bitmap^												SEContextJumpImg;
	Bitmap^												SENewTabImg;
	Bitmap^												SESaveAllImg;

	OptionsDialog^										ScriptEditorOptions;

	Array^												TabDelimit;
	Array^												PipeDelimit;
	Array^												DelimiterKeys;


	static Globals^%									GetSingleton();

	Globals();
};

#define GLOB											Globals::GetSingleton()