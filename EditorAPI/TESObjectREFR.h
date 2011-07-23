#pragma once

#include "obse\NiNodes.h"

#include "TESForm.h"
#include "TESObjectCELL.h"
#include "ExtraDataList.h"

//	EditorAPI: TESObjectREFR class.
//	A number of class definitions are directly derived from the COEF API; Credit to JRoush for his comprehensive decoding

/*
    TESObjectREFR is the parent for all 'instances' of base forms in the game world
    While each ref is a distinct form, it also points back to the 'base' form that it instantiates
    Refs store local data like position, size, flags, etc.
*/

// 60
class TESObjectREFR : public TESForm, public TESChildCell, public TESMemContextForm
{
public:
	// members
	//     /*00*/ TESForm
	//     /*24*/ TESChildCell
	//     /*28*/ TESMemContextForm - empty, no members
	/*28*/ TESForm*				baseForm;
	/*2C*/ Vector3				rotation;
	/*38*/ Vector3				position;
	/*44*/ float				scale;
	/*48*/ TESObjectCELL*		parentCell;
	/*4C*/ ExtraDataList		extraData;

	// methods
	void						Update3D();								// refreshes the render window 3D data
	Vector3*					GetPosition() {	return &position; }
	bool						SetBaseForm(TESForm* BaseForm);
	void						SetPersistent(bool Persistent);

	void						ModExtraHealth(float Health);
	void						ModExtraCharge(float Charge);
	void						ModExtraTimeLeft(float Time);
	void						ModExtraSoul(UInt8 SoulLevel);
	void						SetExtraEnableStateParentOppositeState(bool State);
	NiNode*						GetExtraRef3DData(void);
	void						RemoveExtraTeleport(void);
};