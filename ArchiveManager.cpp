#include "ExtenderInternals.h"
#include "ArchiveManager.h"

void InitializeArchives()
{
	if (*g_LoadedArchives == 0)		return;

	for (IDirectoryIterator Itr((std::string(g_AppPath + "Data\\")).c_str(), "*.bsa"); !Itr.Done(); Itr.Next())
	{
		std::string FileName(Itr.Get()->cFileName);
		FileName = FileName.substr(FileName.find_last_of("\\") + 1);

		bool IsLoaded = false;
		for (GenericNode<Archive>* Itr = (*g_LoadedArchives); Itr; Itr = Itr->next)
		{
			if (Itr->data)
			{
				std::string LoadedFileName(Itr->data->bsfile.m_path);
				LoadedFileName = LoadedFileName.substr(LoadedFileName.find_last_of("\\") + 1);

				if (!_stricmp(LoadedFileName.c_str(), FileName.c_str()))
				{
					IsLoaded = true;
					break;
				}
			}
		}

		if (IsLoaded == false)
		{
			CreateArchive(FileName.c_str(), 0, 0);
			DebugPrint("BSA Archive %s loaded", FileName.c_str());
		}
	}
}