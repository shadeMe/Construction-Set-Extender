#pragma once

//	EditorAPI: Setting class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	Setting is a common base for game settings, ini settings, etc.  Basically just name+value pairs.
*/

// 08
class Setting
{
public:

	enum SettingTypes
	{
		kSetting_Bool           = 0x0,  // 'b' size 1
		kSetting_Char           = 0x1,  // 'c' size 1
		kSetting_UnsignedChar   = 0x2,  // 'h' size 1
		kSetting_SignedInt      = 0x3,  // 'i' size 4
		kSetting_UnsignedInt    = 0x4,  // 'u' size 4
		kSetting_Float          = 0x5,  // 'f' size 4
		kSetting_String         = 0x6,  // 'S'/'s' size indet. See note below^^
		kSetting_RGB            = 0x7,  // 'r' size 4. alpha byte set to 255 (?)
		kSetting_RGBA           = 0x8,  // 'a' size 4
		kSetting__MAX           = 0x9
	};

	// union for various value types
	union SettingValue
	{
		bool        b;
		char        c;
		UInt8       h;
		SInt32      i;
		UInt32      u;
		float       f;
		const char* s;
		struct
		{
			// alpha is least sig. byte, *opposite* standard windows order
			UInt8   alpha;
			UInt8   blue;
			UInt8   green;
			UInt8   red;
		}			rgba;
	};

	// members
	/*00*/ SettingValue			value;
	/*04*/ const char*			name;   // must begin with one of the type characters

	// methods
	void						SetStringValue(const char* Value);
	UInt8						GetValueType(void);
};
STATIC_ASSERT(sizeof(Setting) == 0x8);