#pragma once
#include "obse\GameTypes.h"
#include "obse\Utilities.h"


//	EditorAPI: Magic.
//	Many class definitions are directly copied from the COEF API; Credit to JRoush for his comprehensive decoding

/* 
    'Magic' is a generic container class used to group the various global data and functions related 
    to the magic system.  As such, it has only static members and methods.  It may actually exist as 
    a class in Bethesdah's source, or a namespace, etc., but it left no trace in the assembly.

    Some of this data may actually belong as static data in a more specific class.  However, it (mostly)
    appears in contiguous memory in the assembly image, which implies it was probably very close together
    in the original source.
*/

namespace EditorAPI
{
	class   GameSetting;
	class   SpellItem;
	class   TESEffectShader;
	class   TESSound;

	class Magic
	{
	public:
		// Schools
		enum Schools
		{
			kSchool_Alteration      = 0x0,
			kSchool_Conjuration     = 0x1,
			kSchool_Destruction     = 0x2,
			kSchool_Illusion        = 0x3,
			kSchool_Mysticism       = 0x4,
			kSchool_Restoration     = 0x5,
			kSchool__MAX            = 0x6,  // used as an 'Invalid' or 'None' value
		};   

		// magic types - these correspond to (perhaps are precisely) the type codes of magic items
		enum MagicTypes
		{
			kMagicType_Spell           = 0x0,
			kMagicType_Disease         = 0x1,
			kMagicType_Power           = 0x2,
			kMagicType_LesserPower     = 0x3,
			kMagicType_Ability         = 0x4,
			kMagicType_Poison          = 0x5,   // Apparently dropped for potions w/ all hostile effects, but still included in a lot of code
			kMagicType_Enchantment     = 0x6,
			kMagicType_AlchemyItem     = 0x7,
			kMagicType_Ingredient      = 0x8,
			kMagicType__MAX            = 0x9
		};

		// casting types - these correspond to (perhaps are precisely) the four types of enchantments
		enum CastTypes
		{
			kCast_Once              = 0x0, // Scroll
			kCast_WhenUsed          = 0x1, // Staff
			kCast_WhenStrikes       = 0x2, // Weapon
			kCast_Constant          = 0x3, // Apparel
			kCast__MAX              = 0x4
		};

		// effect ranges
		enum RangeTypes
		{
			kRange_Self             = 0x0,
			kRange_Touch            = 0x1,
			kRange_Target           = 0x2,
			kRange__MAX             = 0x3  // used in some places as an 'any' or 'all' value
		};

		// projectile types
		enum ProjectileTypes
		{
			kProjType_Ball          = 0x0,
			kProjType_Bolt          = 0x1,
			kProjType_Spray         = 0x2,
			kProjType_Fog           = 0x3,
			kProjType__MAX          = 0x4   // this and all other values seem to default to 'ball' in many cases
		};
	};
}