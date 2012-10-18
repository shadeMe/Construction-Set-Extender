#pragma once
#include "Hooks-Common.h"

namespace ConstructionSetExtender
{
	namespace INISettings
	{
		enum
		{
			kLOD_PartialTextureResolution = 0,
			kLOD_DeletePartialsAfterGeneration,

			kLOD__MAX
		};
		extern const BGSEditorExtender::BGSEEINIManagerSettingFactory::SettingData		kLODINISettings[kLOD__MAX];
		BGSEditorExtender::BGSEEINIManagerSettingFactory*								GetLOD(void);
	}

	namespace Hooks
	{
		// hooks that modify LOD related code
		void PatchLODHooks(void);

		extern UInt8		g_LODDiffuseMapGeneratorState;
		enum
		{
			kLODDiffuseMapGeneratorState_NotInUse = 0,
			kLODDiffuseMapGeneratorState_Partials,
			kLODDiffuseMapGeneratorState_FullMap,
		};

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
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardB, "ugly kludge. I repeat - fucking ugly kludges, these are");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardC, "");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardD, "");
		_DeclareMemHdlr(GenerateLODDiffuseMapsReentryGuardE, "");
		_DeclareMemHdlr(LODTextureGenNotificationPrologA, "adds a progress dialog for the generation process");
		_DeclareMemHdlr(LODTextureGenNotificationPrologB, "");
		_DeclareMemHdlr(LODTextureGenNotificationEpilog, "");
		_DeclareMemHdlr(LODTextureGenNotificationUpdate, "");
		_DeclareMemHdlr(LODTextureGenBlackTextureFix, "fixes the bug that caused half of the texture to not be rendered");
		_DeclareMemHdlr(LODTextureGenBlackTexturePartialFix, "for the partial textures, this");
	}
}
