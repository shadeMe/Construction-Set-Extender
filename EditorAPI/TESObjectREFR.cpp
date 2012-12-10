#include "TESObjectREFR.h"
#include "..\Hooks\Hooks-Renderer.h"

using namespace ConstructionSetExtender;

void TESObjectREFR::UpdateNiNode()
{
	SetPosition(position.x, position.y, position.z);
	SetRotation(rotation.x, rotation.y, rotation.z, true);
	SetScale(scale);
}

bool TESObjectREFR::SetBaseForm(TESForm* BaseForm)
{
	return thisCall<bool>(0x005415A0, this, BaseForm);
}

void TESObjectREFR::SetPersistent(bool Persistent)
{
	thisCall<UInt32>(0x0053F0D0, this, Persistent);
}

void TESObjectREFR::ModExtraHealth( float Health )
{
	thisCall<UInt32>(0x0053F4E0, this, Health);
}

void TESObjectREFR::ModExtraCharge( float Charge )
{
	thisCall<UInt32>(0x0053F3C0, this, Charge);
}

void TESObjectREFR::ModExtraTimeLeft( float Time )
{
	thisCall<UInt32>(0x0053F620, this, Time);
}

void TESObjectREFR::ModExtraSoul( UInt8 SoulLevel )
{
	thisCall<UInt32>(0x0053F710, this, SoulLevel);
}

void TESObjectREFR::SetExtraEnableStateParentOppositeState( bool State )
{
	thisCall<UInt32>(0x0053FA80, this, State);
}

NiNode* TESObjectREFR::GetNiNode( void )
{
	return thisCall<NiNode*>(0x00542950, this);
}

void TESObjectREFR::RemoveExtraTeleport( void )
{
	thisCall<UInt32>(0x0053F7A0, this);
}

NiNode* TESObjectREFR::GenerateNiNode()
{
	return thisVirtualCall<NiNode*>(0x178, this);
}

void TESObjectREFR::SetScale( float Scale )
{
	thisCall<void>(0x00542420, this, Scale);
}

void TESObjectREFR::SetPosition( float X, float Y, float Z )
{
	TESObjectCELL* Unk00 = thisCall<TESObjectCELL*>(0x00544380, this);
	TESObjectCELL* ExteriorAtCoordsProlog = _DATAHANDLER->GetExteriorCell(position.x, position.y, position.z, NULL, NULL);

	thisCall<void>(0x00544250, this, X, Y, Z);									// TESObjectREFR::SetPosition
	thisCall<void>(0x0053FD10, this, position.x, position.y, position.z);		// TESObjectREFR::SetExtraEditorMoveDataPosition

	TESObjectCELL* ExteriorAtCoordsEpilog = _DATAHANDLER->GetExteriorCell(position.x, position.y, position.z, NULL, NULL);
	if (ExteriorAtCoordsProlog != ExteriorAtCoordsEpilog)
		_DATAHANDLER->MoveReference(ExteriorAtCoordsEpilog, this);

	NiNode* Node3D = GetNiNode();
	if (Node3D)
	{
		Node3D->m_localTranslate.x = position.x;
		Node3D->m_localTranslate.y = position.y;
		Node3D->m_localTranslate.z = position.z;

		cdeclCall<void>(0x00609F60, Node3D, true);		// NiNode::UpdateCollision?
		TESRender::UpdateAVObject(Node3D);

		ExtraLight::ExtraLightData* xLight = thisCall<ExtraLight::ExtraLightData*>(0x00540110, this);	// TESObjectREFR::GetExtraLight
		if (xLight && xLight->light && baseForm && baseForm->formType == kFormType_Light)
		{
			NiNode* SceneNode = cdeclCall<NiNode*>(0x007662E0, 0);										// TESRender::GetSceneNode
			thisCall<void>(0x007713A0, SceneNode, xLight->light);
		}

		TESRenderWindow::Redraw();
	}
}

void TESObjectREFR::SetRotation( float X, float Y, float Z, bool Radians /*= false*/ )
{
	if (baseForm && baseForm->formType == kFormType_Tree)
		return;

	if (Radians == false)
	{
		X *= 0.01745329238474369;
		Y *= 0.01745329238474369;
		Z *= 0.01745329238474369;
	}

	rotation.x = X;
	rotation.y = Y;
	rotation.z = Z;

	thisCall<void>(0x0053FC70, this, rotation.x, rotation.y, rotation.z);		// TESObjectREFR::SetExtraEditorMoveDataRotation

	NiNode* Node3D = GetNiNode();
	if (Node3D)
	{
		NiMatrix33 RotationMatrix = {0};
		thisCall<NiMatrix33*>(0x00542AC0, this, &RotationMatrix);				// TESObjectREFR::GetLocalRotationMatrix

		memcpy(&Node3D->m_localRotate, &RotationMatrix, sizeof(NiMatrix33));
		cdeclCall<void>(0x00609F60, Node3D, true);								// NiNode::UpdateCollision?
		TESRender::UpdateAVObject(Node3D);

		TESRenderWindow::Redraw();
	}
}

const Vector3* TESObjectREFR::GetPosition() const
{
	return &position;
}

const Vector3* TESObjectREFR::GetRotation() const
{
	return &rotation;
}

void TESObjectREFR::ToggleInvisiblity( void )
{
	if (GetInvisible())
		SME::MiscGunk::ToggleFlag(&formFlags, kSpecialFlags_3DInvisible, false);
	else
		SME::MiscGunk::ToggleFlag(&formFlags, kSpecialFlags_3DInvisible, true);
}

void TESObjectREFR::ToggleChildrenInvisibility( void )
{
	if (GetChildrenInvisible())
		SME::MiscGunk::ToggleFlag(&formFlags, kSpecialFlags_Children3DInvisible, false);
	else
		SME::MiscGunk::ToggleFlag(&formFlags, kSpecialFlags_Children3DInvisible, true);
}

void TESObjectREFR::SetFrozenState( bool State )
{
	SME::MiscGunk::ToggleFlag(&formFlags, kSpecialFlags_Frozen, State);
}

void TESObjectREFR::SetAlpha( float Alpha /*= -1.0f*/ )
{
	// doesn't blend correctly with other objects, or itself

	BSFadeNode* FadeNode = (BSFadeNode*)GetNiNode();
	if (FadeNode == NULL)
		return;

	if (Alpha == -1.0f)
	{
		SME::MiscGunk::ToggleFlag(&FadeNode->m_flags, kNiNodeSpecialFlags_SpecialFade, false);
		FadeNode->fCurrentAlpha = 1.0f;
	}
	else
	{
		SME::MiscGunk::ToggleFlag(&FadeNode->m_flags, kNiNodeSpecialFlags_SpecialFade, true);
		FadeNode->fCurrentAlpha = Alpha;
	}
}

bool TESObjectREFR::GetInvisible( void ) const
{
	return (formFlags & kSpecialFlags_3DInvisible);
}

bool TESObjectREFR::GetChildrenInvisible( void ) const
{
	return (formFlags & kSpecialFlags_Children3DInvisible);
}

bool TESObjectREFR::GetFrozen( void ) const
{
	return (formFlags & kSpecialFlags_Frozen);
}

float TESObjectREFR::GetAlpha( void )
{
	BSFadeNode* FadeNode = (BSFadeNode*)GetNiNode();

	if (FadeNode && (FadeNode->m_flags & kNiNodeSpecialFlags_SpecialFade))
		return FadeNode->fCurrentAlpha;
	else
		return 1.0f;
}

void TESObjectREFR::ChangeCell( TESObjectCELL* Cell )
{
	thisVirtualCall<void>(0x1AC, this, Cell);
}
