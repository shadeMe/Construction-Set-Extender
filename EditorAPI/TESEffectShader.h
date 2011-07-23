#pragma once

#include "TESForm.h"

//	EditorAPI: TESEffectShader class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 134
class TESEffectShader : public TESForm
{
public:
	enum
	{
		kEffectShaderFlags_NoMembraneShader		=	/*00*/ 0x01,
		kEffectShaderFlags_NoParticleShader		=	/*03*/ 0x08,
		kEffectShaderFlags_InverseEdgeEffect	=	/*04*/ 0x10,
		kEffectShaderFlags_AffectsSkin			=	/*05*/ 0x20
	};

	enum
	{
		kBlendMode_Zero								=	0x1,
		kBlendMode_One,
		kBlendMode_SourceColor,
		kBlendMode_SourceInverseColor,
		kBlendMode_SourceAlpha,
		kBlendMode_SourceInvertedAlpha,
		kBlendMode_DestAlpha,
		kBlendMode_DestInverseAlpha,
		kBlendMode_DestColor,
		kBlendMode_DestInverseColor,
		kBlendMode_SourceAlphaSAT,
		kBlendMode__MAX
	};

	enum
	{
		kBlendOp_Add								=	0x1,
		kBlendOp_Subtract,
		kBlendOp_ReverseSubtract,
		kBlendOp_Minimum,
		kBlendOp_Maximum,
		kBlendOp__MAX
	};

	enum
	{
		kZTestFnMembraneShader_EqualTo				=	0x3,
		kZTestFnMembraneShader_GreaterThan			=	0x5,
		kZTestFnMembraneShader_GreaterThanEqualTo	=	0x7,
		kZTestFnMembraneShader__MAX
	};

	enum
	{
		kZTestFnParticleShader_Normal				=	0x4,
		kZTestFnParticleShader_AlwaysShow			=	0x8,
		kZTestFnParticleShader__MAX
	};

	// E0
	struct EffectShaderData
	{
		// 10
		struct BlendData
		{
			/*00*/ UInt32		sourceBlendMode;
			/*04*/ UInt32		blendOp;
			/*08*/ UInt32		zTestFn;
			/*0C*/ UInt32		destBlendMode;
		};

		// 1C
		struct MembraneData
		{
			/*00*/ RGBA			color;
			/*04*/ float		alphaFadeInTime;
			/*08*/ float		fullAlphaTime;
			/*0C*/ float		alphaFadeOutTime;
			/*10*/ float		persistentAlphaRatio;
			/*14*/ float		alphaPulseAmplitude;
			/*18*/ float		alphaPulseFrequency;
		};

		// 70
		struct ParticleData
		{
			/*00*/ float		birthRampUpTime;
			/*04*/ float		birthFullTime;
			/*08*/ float		birthRampDownTime;
			/*0C*/ float		birthFullRatio;
			/*10*/ float		birthPersistentRatio;
			/*14*/ float		lifeTime;
			/*18*/ float		lifeTimeDelta;
			/*1C*/ float		initialSpeedAlongNormal;
			/*20*/ float		accelerationAlongNormal;
			/*24*/ float		initialVelocity[3];
			/*30*/ float		acceleration[3];
			/*3C*/ float		scaleKeys[2];
			/*44*/ float		scaleKeyTimes[2];
			/*4C*/ RGBA			colorKeys[3];
			/*58*/ float		colorAlpha[3];
			/*64*/ float		colorKeyTimes[3];
		};

		/*00*/ UInt8			effectShaderFlags;
		/*01*/ UInt8			pad01[3];
		/*04*/ UInt32			membraneShaderSourceBlendMode;
		/*08*/ UInt32			membraneShaderBlendOp;
		/*0C*/ UInt32			membraneShaderZTestFn;
		/*10*/ MembraneData		skinMembraneShader;
		/*2C*/ float			textureAnimSpeedU;
		/*30*/ float			textureAnimSpeedV;
		/*34*/ float			fallOff;
		/*38*/ MembraneData		edgeMembraneShader;
		/*54*/ float			skinFullAlphaRatio;
		/*58*/ float			edgeFullAlphaRatio;
		/*5C*/ UInt32			membraneShaderDestBlendMode;		// not a part of the initial membraneShader chunk weirdly, unlike in the runtime def.
		/*60*/ BlendData		particleBlendData;
		/*70*/ ParticleData		particleShader;
	};

	// members
	//     /*00*/ TESForm
	/*024*/ EffectShaderData	shaderData;
	/*104*/ TESTexture			membraneShaderTexture;
	/*11C*/ TESTexture			particleShaderTexture;
};