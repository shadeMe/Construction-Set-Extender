#pragma once
#include "Hooks-Common.h"

namespace cse
{
	namespace hooks
	{
		// hooks that modify LOD related code
		void PatchLODHooks(void);

		_DeclareNopHdlr(LODLandTextureMipMapLevelA, "patches the LOD texture generator to generate the full mip chain for diffuse maps");
		_DeclareMemHdlr(LODLandTextureMipMapLevelB, "patches the LOD texture generator to generate the full mip chain for normal maps");
		_DeclareMemHdlr(LODLandTextureAllocation, "increases the resolution of lod landscape diffuse maps");
		_DeclareMemHdlr(LODLandTextureDestruction, "");
		_DeclareMemHdlr(LODLandD3DTextureSelection, "");
		_DeclareMemHdlr(LODLandBSTextureSelection, "");
		_DeclareMemHdlr(GenerateLODPartialTexture, "");
		_DeclareMemHdlr(GenerateLODFullTexture, "");
		_DeclareMemHdlr(GenerateLODFullTextureFileName, "corrects the filename used to save generated textures under");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardA, "prevents the GenerateLODDiffuseMaps() function from being called recursively");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardB, "ugly kludge. I repeat - fugly kludges, these are");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardC, "");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardD, "");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardE, "");
		_DeclareMemHdlr(LODTextureGenNotificationPrologA, "adds a progress dialog for the generation process");
		_DeclareMemHdlr(LODTextureGenNotificationPrologB, "");
		_DeclareMemHdlr(LODTextureGenNotificationEpilog, "");
		_DeclareMemHdlr(LODTextureGenNotificationUpdate, "");
		_DeclareMemHdlr(LODTextureGenBlackTextureFix, "fixes the bug that caused half of the texture to not be rendered");
		_DeclareMemHdlr(LODTextureGenBlackTexturePartialFix, "for the partial textures, this");
		_DeclareMemHdlr(NiRenderedTextureCreateSourceTextureA, "adds NULL checks all over the place");
		_DeclareMemHdlr(NiRenderedTextureCreateSourceTextureB, "");
		_DeclareMemHdlr(LODTextureGenSavePartialToDisk, "");
		_DeclareMemHdlr(GeneratePartialLODFilePath, "temp hook to add support for exterior cell snapshots");
	}
}
