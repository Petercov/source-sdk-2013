//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Dr. Kleiner, a suave ladies man leading the fight against the evil 
//			combine while constantly having to help his idiot assistant Gordon
//
//=============================================================================//


//-----------------------------------------------------------------------------
// Generic NPC - purely for scripted sequence work.
//-----------------------------------------------------------------------------
#include "cbase.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "ai_hull.h"
#include "npc_playercompanion.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_odell_health("sk_odell_health", "40");

//-----------------------------------------------------------------------------
// NPC's Anim Events Go Here
//-----------------------------------------------------------------------------

int AE_ODELL_WELDER_ATTACHMENT;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_Odell : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS(CNPC_Odell, CNPC_PlayerCompanion);
	DECLARE_DATADESC();

	void	Spawn(void);
	void	Precache(void);
	Class_T Classify(void);
	void	HandleAnimEvent(animevent_t *pEvent);
	virtual void	SelectModel();

	DEFINE_CUSTOM_AI;

private:
	void CreateWelder(void);
	CHandle<CBaseAnimating>	m_hWelder;
	bool m_bShouldHaveWelder;
};

LINK_ENTITY_TO_CLASS(npc_odell, CNPC_Odell);
//LINK_ENTITY_TO_CLASS(npc_human_scientist_kleiner, CNPC_Odell);

BEGIN_DATADESC(CNPC_Odell)
DEFINE_FIELD(m_hWelder, FIELD_EHANDLE),
DEFINE_KEYFIELD(m_bShouldHaveWelder, FIELD_BOOLEAN, "ShouldHaveWelder"),
END_DATADESC();

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Odell::Classify(void)
{
	return	CLASS_PLAYER_ALLY_VITAL;
}


//-----------------------------------------------------------------------------
// Purpose: Create and initialized Alyx's EMP tool
//-----------------------------------------------------------------------------

void CNPC_Odell::CreateWelder(void)
{
	if (!m_bShouldHaveWelder || m_hWelder)
		return;

	m_hWelder = (CBaseAnimating*)CreateEntityByName("prop_dynamic");
	if (m_hWelder)
	{
		m_hWelder->SetModel("models/props_scart/welding_torch.mdl");
		m_hWelder->SetName(AllocPooledString("odell_weldertool"));
		int iAttachment = LookupAttachment("Welder_Holster");
		m_hWelder->SetParent(this, iAttachment);
		m_hWelder->SetOwnerEntity(this);
		m_hWelder->SetSolid(SOLID_NONE);
		m_hWelder->SetLocalOrigin(Vector(0, 0, 0));
		m_hWelder->SetLocalAngles(QAngle(0, 0, 0));
		m_hWelder->AddSpawnFlags(SF_DYNAMICPROP_NO_VPHYSICS);
		m_hWelder->AddEffects(EF_PARENT_ANIMATES);
		m_hWelder->Spawn();
	}
}

//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_Odell::HandleAnimEvent(animevent_t *pEvent)
{
	if ((pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && pEvent->event == AE_ODELL_WELDER_ATTACHMENT)
	{
		if (!m_hWelder)
		{
			// Old savegame?
			CreateWelder();
			if (!m_hWelder)
				return;
		}

		int iAttachment = LookupAttachment(pEvent->options);
		m_hWelder->SetParent(this, iAttachment);
		

		if (!stricmp(pEvent->options, "Welder_Holster"))
		{
			m_hWelder->SetLocalOrigin(Vector(0, 0, 0));
			m_hWelder->SetLocalAngles(QAngle(0, 0, 0));
		}
		else
		{
			m_hWelder->SetLocalOrigin(Vector(0, 0, 0));
			m_hWelder->SetLocalAngles(QAngle(90, 0, 0));
		}

		return;
	}

	BaseClass::HandleAnimEvent(pEvent);
}

void CNPC_Odell::SelectModel()
{
	// Allow custom model usage (mostly for monitors)
	char* szModel = (char*)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/odell.mdl";
		SetModelName(AllocPooledString(szModel));
	}
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Odell::Spawn()
{
	BaseClass::Spawn();

	m_iHealth = sk_odell_health.GetInt();

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);

	CreateWelder();

	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_Odell::Precache()
{
	PrecacheModel("models/props_scart/welding_torch.mdl");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(npc_odell, CNPC_Odell)

DECLARE_ANIMEVENT(AE_ODELL_WELDER_ATTACHMENT);

AI_END_CUSTOM_NPC();