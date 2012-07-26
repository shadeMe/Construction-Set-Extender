#pragma once

#include "TESForm.h"
#include "Setting.h"
#include "BSTCaseInsensitiveStringMap.h"
#include "[Common]\HandShakeStructs.h"

//	EditorAPI: GameSetting class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    SettingCollection is an abstract interface for managing settings from a common source (file, registery, etc.)
    INISettingCollection and it's descendents manage settings from the assorted INI files
    RegSettingCollection managed windows registry values.
    GameSettingCollection manages the configurable game settings.

    NOTE:
    SettingCollection and it's abstract descendents are actually templated to accept arbitrary data types.  The only template
    parameter used is 'Setting', so functionally the classes defined here are identical to those defined in the game/CS.
    However, as with many of the template classes reproduced (rather than imported) in COEF, dynamic casting may not work
    as expected with these types.
*/

class   TESFile;

// 10C
class SettingCollection // actually SettingCollection<Setting*>
{
public:
    // members
    //     /*000*/ void**           vtbl;
    /*004*/ char            filename[0x104];	// full path of ini file for ini settings, ignored for other collections
    /*108*/ void*           fileObject;			// actual type depends on class, but must be nonzero when file is open
												// TESFile* for gmst, HKEY for RegSetting, this* pointer for INISetting
	virtual void			VFn00() = 0;
};

// 114
class SettingCollectionList : public SettingCollection  // actually SettingCollectionList<Setting*>
{
public:
    typedef tList<Setting> SettingListT;

    // members
    //     /*000*/ SettingCollection
    /*10C*/ SettingListT	settingList;

	// methods
	Setting*				LookupByName(const char* Name);
};

// 114
class INISettingCollection : public SettingCollectionList
{
public:
    // fileName holds the ini file name
    // fileObject holds the this* pointer when file is "open", and is zero otherwise
    // file is not actually opened; all i/o uses Get/WritePrivateProfileString()

	// no additional members

	static INISettingCollection*		Instance;
};

// 114
class LipSynchroSettingCollection : public INISettingCollection
{
public:
    // no additional members
};

// 114
class REGINISettingCollection : public INISettingCollection
{
public:
    // no additional members
};

// 114
class RegSettingCollection : public SettingCollectionList
{
public:
    // fileName holds the registry key name, from HKEY_LOCAL_MACHINE
    // fileObject holds an HKEY to the target registery key.

    // no additional members
};

// 120
class SettingCollectionMap : public SettingCollection   // actually SettingCollectionMap<Setting*>
{
public:
	typedef ConstructionSetExtender_OverriddenClasses::BSTCaseInsensitiveStringMap<Setting*>	SettingMapT;

    // members
    //     /*000*/ SettingCollection
    /*10C*/ SettingMapT      settingMap;
};

// 120
class GameSettingCollection : public SettingCollectionMap
{
public:
    // fileName seems to be ignored
    // fileObject holds a TESFile* for loading/saving the setting

    // no additional members

	// methods
	void					CreateDefaultCopy();	// creates a copy of the collection
	void					ResetCollection();		// resets the collection with data from the default copy
	UInt32					GetGMSTCount();
	void					SerializeGMSTDataForHandShake(ComponentDLLInterface::GMSTData* HandShakeData);

	static GameSettingCollection*		Instance;
};

extern const char*				g_CSINIPath;