#pragma once

#include "TESObjectACTI.h"

//	EditorAPI: TESFurniture class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
	...
*/

// 9C
class TESFurniture : public TESObjectACTI
{
public:
	enum ActiveMarkers
	{
		kActiveMarker_Marker0			= /*00*/ 0x00000001,
		kActiveMarker_Marker1			= /*01*/ 0x00000002,
		kActiveMarker_Marker2			= /*02*/ 0x00000004,
		kActiveMarker_Marker3			= /*03*/ 0x00000008,
		kActiveMarker_Marker4			= /*04*/ 0x00000010,
		kActiveMarker_Marker5			= /*05*/ 0x00000020,
		kActiveMarker_Marker6			= /*06*/ 0x00000040,
		kActiveMarker_Marker7			= /*07*/ 0x00000080,
		kActiveMarker_Marker8			= /*08*/ 0x00000100,
		kActiveMarker_Marker9			= /*09*/ 0x00000200,
		kActiveMarker_Marker10			= /*0A*/ 0x00000400,
		kActiveMarker_Marker11			= /*0B*/ 0x00000800,
		kActiveMarker_Marker12			= /*0C*/ 0x00001000,
		kActiveMarker_Marker13			= /*0D*/ 0x00002000,
		kActiveMarker_Marker14			= /*0E*/ 0x00004000,
		kActiveMarker_Marker15			= /*0F*/ 0x00008000,
		kActiveMarker_Marker16			= /*10*/ 0x00010000,
		kActiveMarker_Marker17			= /*11*/ 0x00020000,
		kActiveMarker_Marker18			= /*12*/ 0x00040000,
		kActiveMarker_Marker19			= /*13*/ 0x00080000,
		kActiveMarker_Marker20			= /*14*/ 0x00100000,
		kActiveMarker_Marker21			= /*15*/ 0x00200000,
		kActiveMarker_Marker22			= /*16*/ 0x00400000,
		kActiveMarker_Marker23			= /*17*/ 0x00800000,
		kActiveMarker_Marker24			= /*18*/ 0x01000000,
		kActiveMarker_Marker25			= /*19*/ 0x02000000,
		kActiveMarker_Marker26			= /*1A*/ 0x04000000,
		kActiveMarker_Marker27			= /*1B*/ 0x08000000,
		kActiveMarker_Marker28			= /*1C*/ 0x10000000,
		kActiveMarker_Marker29			= /*1D*/ 0x20000000,
		kActiveMarker_Marker30			= /*1E*/ 0x40000000,		// 'Sitting' marker
		kActiveMarker_Marker31			= /*1F*/ 0x80000000,		// 'Sleeping' marker
		kActiveMarker__MAX
	};

	// members
	//     /*00*/ TESObjectACTI
	/*98*/ UInt32					activeMarkerMask;
};