#pragma once

#include "TESForm.h"

//	EditorAPI: TESClimate class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

class	TESWeather;

// AC
class TESClimate : public TESFormIDListView, public TESModel
{
public:
	// 08
	struct WeatherEntry
	{
		/*00*/ TESWeather*				weather;
		/*04*/ UInt32					chance;
	};

	// Actually derived from BSSimpleList<WeatherEntry*>
	// 24
	class TESWeatherList
	{
	public:
		typedef tList<WeatherEntry> WeatherListT;

		enum
		{
			kWeatherListFlags_Default						= 0,
			kWeatherListFlags_IgnoreChanceDiscrepancies,
			kWeatherListFlags_RevalidateChances
		};

		// members
		//     /*00*/ void**			vtbl;
		/*04*/ WeatherListT				weatherList;
		/*0C*/ UInt32					listViewID;		// listview that displays the entries
		/*10*/ UInt32					comboBoxID;		// combobox that lists valid weathers
		/*14*/ UInt32					buttonID;		// button used to add new entires
		/*18*/ UInt32					popupID;		// popup menu used to delete existing entries
		/*1C*/ UInt32					unk1C;
		/*20*/ UInt32					flags;

		virtual void					VFn00();
	};

	enum
	{
		kMoonFlags_PhaseLengthMask	=		   0x3F,

		kMoonFlags_Secunda			=	/*06*/ 0x40,
		kMoonFlags_Masser			=	/*07*/ 0x80,
	};

	// members
	//     /*00*/ TESFormIDListView
	//     /*24*/ TESModel
	/*48*/ TESWeatherList				weatherData;
	/*6C*/ TESTexture					sun;
	/*84*/ TESTexture					sunGlare;
	/*9C*/ UInt8						sunriseBegin;	// these are the number of 10 minute increments past midnight
	/*9D*/ UInt8						sunriseEnd;
	/*9E*/ UInt8						sunsetBegin;	// 6 increments per hour
	/*9F*/ UInt8						sunsetEnd;
	/*A0*/ UInt8						volatility;
	/*A1*/ UInt8						moonInfo;
	/*A2*/ UInt8						padA2[2];
	/*A4*/ void*						unkA4;
	/*A8*/ UInt32						unkA8;
};