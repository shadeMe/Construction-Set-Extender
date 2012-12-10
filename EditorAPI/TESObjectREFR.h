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
	enum
	{
		kSpecialFlags_3DInvisible				= 1 << 31,
		kSpecialFlags_Children3DInvisible		= 1 << 30,
		kSpecialFlags_Frozen					= 1 << 29,
	};

	enum
	{
		kNiNodeSpecialFlags_DontUncull			= 1 << 15,
		kNiNodeSpecialFlags_SpecialFade			= 1 << 14,
	};

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
	NiNode*						GetNiNode(void);
	NiNode*						GenerateNiNode();
	void						UpdateNiNode();
	const Vector3*				GetPosition() const;
	const Vector3*				GetRotation() const;
	bool						SetBaseForm(TESForm* BaseForm);
	void						SetPersistent(bool Persistent);

	void						ChangeCell(TESObjectCELL* Cell);

	void						SetPosition(float X, float Y, float Z);
	void						SetRotation(float X, float Y, float Z, bool Radians = false);
	void						SetScale(float Scale);

	void						ModExtraHealth(float Health);
	void						ModExtraCharge(float Charge);
	void						ModExtraTimeLeft(float Time);
	void						ModExtraSoul(UInt8 SoulLevel);
	void						SetExtraEnableStateParentOppositeState(bool State);
	void						RemoveExtraTeleport(void);

	void						ToggleInvisiblity(void);
	void						ToggleChildrenInvisibility(void);
	void						SetFrozenState(bool State);
	void						SetAlpha(float Alpha = -1.0f);		// pass -1.0f to reset alpha

	bool						GetInvisible(void) const;
	bool						GetChildrenInvisible(void) const;
	bool						GetFrozen(void) const;
	float						GetAlpha(void);
};