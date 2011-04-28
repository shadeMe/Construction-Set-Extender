#pragma once
#include "ExtenderInternals.h"

union SettingData
{
	int				i;
	UInt32			u;
	float			f;
	char*			s;
};

// 2C		### partial - look into the common settings class
class GameSetting
{
public:
	GameSetting();
	~GameSetting();

	// bases
	TESFormIDListView		listView;

	//members
	SettingData				Data;		// 24
	const char*				settingID;	// 28

	void					SetSettingID(const char* ID) { settingID = ID; }
};

struct GMSTMap_Key_Comparer
{
	bool operator()(const char* Key1, const char* Key2) const {
		return _stricmp(Key1, Key2) < 0;
	}
};

typedef std::map<const char*, GameSetting*, GMSTMap_Key_Comparer>		_DefaultGMSTMap;
extern _DefaultGMSTMap			g_DefaultGMSTMap;

void						InitializeDefaultGMSTMap();