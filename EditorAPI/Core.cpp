#include "Core.h"

TESFile* TESDataHandler::LookupModByName(const char* PluginName)
{
	for (tList<TESFile>::Iterator Itr = (*g_TESDataHandler)->fileList.Begin(); !Itr.End(); ++Itr)
	{
		TESFile* Plugin = Itr.Get();
		if (!Plugin)
			break;
		
		if (!_stricmp(Plugin->fileName, PluginName))
			return Plugin;
	}

	return NULL;
}