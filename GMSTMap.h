#pragma once
#include "ExtenderInternals.h"

struct VariableData;
typedef VariableData GMSTData;

struct GMSTMap_Key_Comparer
{
	bool operator()(const char* Key1, const char* Key2) const {
		return _stricmp(Key1, Key2) < 0;
	}
};

typedef std::map<const char*, GameSetting*, GMSTMap_Key_Comparer>		_DefaultGMSTMap;
extern _DefaultGMSTMap			g_DefaultGMSTMap;

void						InitializeDefaultGMSTMap();
UInt32						CountGMSTForms();
void						InitializeHandShakeGMSTData(GMSTData* HandShakeData);