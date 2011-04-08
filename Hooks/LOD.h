#pragma once 
#include "Common.h"

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


