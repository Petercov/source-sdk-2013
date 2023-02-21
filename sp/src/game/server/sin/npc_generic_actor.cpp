#include "cbase.h"
#include "ai_baseactor.h"

#include "tier0/memdbgon.h"

#define SF_ACTOR_IGNORE_SEMAPHORE		( 1 << 16 ) // 65536
#define SF_ACTOR_INVINCIBLE				( 1 << 17 ) // 131072

class CNPC_SinActor : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_SinActor, CAI_BaseActor);
	DECLARE_DATADESC();

	void Precache();
	void Spawn();
	bool UseSemaphore(void);
	int	GetSoundInterests(void) { return NULL; }
	Class_T Classify() { return CLASS_NONE; }

private:
	int m_nInitialHealth;
	int m_nSinBloodColor;
	bool m_bDontHammerBBox;
};

BEGIN_DATADESC(CNPC_SinActor)
DEFINE_KEYFIELD(m_nInitialHealth, FIELD_INTEGER, "initialHealth"),
DEFINE_KEYFIELD(m_nSinBloodColor, FIELD_INTEGER, "bloodColor"),
DEFINE_KEYFIELD(m_bDontHammerBBox, FIELD_BOOLEAN, "dontHammerBBox"),
END_DATADESC();

LINK_ENTITY_TO_CLASS(npc_generic_actor, CNPC_SinActor);

void CNPC_SinActor::Precache()
{
	PrecacheModel(STRING(GetModelName()));

	BaseClass::Precache();
}

void CNPC_SinActor::Spawn()
{
	Precache();

	SetModel(STRING(GetModelName()));

	SetHullType(HULL_HUMAN);
	if (!m_bDontHammerBBox)
		SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	m_flFieldOfView = 0.02;
	m_NPCState = NPC_STATE_NONE;
	m_iHealth = m_nInitialHealth;

	switch (m_nSinBloodColor)
	{
	case -1:
		SetBloodColor(DONT_BLEED);
		break;
	case 195:
		SetBloodColor(BLOOD_COLOR_YELLOW);
		break;
	case 247:
	default:
		SetBloodColor(BLOOD_COLOR_RED);
		break;
	}

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_SQUAD);

	//if (!HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		CapabilitiesAdd(bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
		CapabilitiesAdd(bits_CAP_USE_WEAPONS | bits_CAP_AIM_GUN | bits_CAP_MOVE_SHOOT);
		CapabilitiesAdd(bits_CAP_DUCK | bits_CAP_DOORS_GROUP);
		CapabilitiesAdd(bits_CAP_USE_SHOT_REGULATOR);
	}
	CapabilitiesAdd(bits_CAP_NO_HIT_PLAYER | bits_CAP_NO_HIT_SQUADMATES | bits_CAP_FRIENDLY_DMG_IMMUNE);
	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	SetMoveType(MOVETYPE_STEP);

	m_HackedGunPos = Vector(0, 0, 55);

	BaseClass::Spawn();

	NPCInit();

	if (HasSpawnFlags(SF_ACTOR_INVINCIBLE))
		m_takedamage = DAMAGE_EVENTS_ONLY;
}

bool CNPC_SinActor::UseSemaphore(void)
{
	if (HasSpawnFlags(SF_ACTOR_IGNORE_SEMAPHORE))
		return false;

	return BaseClass::UseSemaphore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_SinCount : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_SinCount, CAI_BaseActor);

	void	Spawn(void);
	void	Precache(void);
	Class_T Classify(void);
	int		GetSoundInterests(void);
	bool	UseSemaphore(void);
};

LINK_ENTITY_TO_CLASS(npc_count, CNPC_SinCount);

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_SinCount::Classify(void)
{
	return	CLASS_NONE;
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_SinCount::GetSoundInterests(void)
{
	return	NULL;
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_SinCount::Spawn()
{
	// Breen is allowed to use multiple models, because he has a torso version for monitors.
	// He defaults to his normal model.
	char* szModel = (char*)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/characters/count/count.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	SetModel(szModel);

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_RED);
	m_iHealth = 8;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);
	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);

	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_SinCount::Precache()
{
	PrecacheModel(STRING(GetModelName()));
	BaseClass::Precache();
}

bool CNPC_SinCount::UseSemaphore(void)
{
	if (HasSpawnFlags(SF_ACTOR_IGNORE_SEMAPHORE))
		return false;

	return BaseClass::UseSemaphore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_SinJC : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_SinJC, CAI_BaseActor);

	void	Spawn(void);
	void	Precache(void);
	Class_T Classify(void);
	int		GetSoundInterests(void);
	bool	UseSemaphore(void);
};

LINK_ENTITY_TO_CLASS(npc_jc, CNPC_SinJC);

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_SinJC::Classify(void)
{
	return	CLASS_PLAYER_ALLY_VITAL;
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_SinJC::GetSoundInterests(void)
{
	return	NULL;
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_SinJC::Spawn()
{
	// Allow custom model usage (mostly for monitors)
	char* szModel = (char*)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/sirgibs/ragdolls/sinepisode/jc.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	SetModel(szModel);

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_RED);
	m_iHealth = 8;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);

	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_SinJC::Precache()
{
	PrecacheModel(STRING(GetModelName()));

	BaseClass::Precache();
}

bool CNPC_SinJC::UseSemaphore(void)
{
	if (HasSpawnFlags(SF_ACTOR_IGNORE_SEMAPHORE))
		return false;

	return BaseClass::UseSemaphore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_SinHologram : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_SinHologram, CAI_BaseActor);
	DECLARE_DATADESC();

	void	Spawn(void);
	void	Precache(void);
	Class_T Classify(void);
	int		GetSoundInterests(void);
	bool	UseSemaphore(void);

	void	SetInterference(bool bEnable);
};

BEGIN_DATADESC(CNPC_SinHologram)
END_DATADESC();

LINK_ENTITY_TO_CLASS(npc_hologram, CNPC_SinHologram);

class CHologramTrigger : public CBaseEntity
{
public:
	DECLARE_CLASS(CHologramTrigger, CBaseEntity);
	DECLARE_DATADESC();

	// 
	// Creates a trigger with the specified bounds

	static CHologramTrigger* Create(const Vector& vecOrigin, const Vector& vecMins, const Vector& vecMaxs, CNPC_SinHologram* pOwner)
	{
		CHologramTrigger* pTrigger = (CHologramTrigger*)CreateEntityByName("trigger_sin_hologram");
		if (pTrigger == NULL)
			return NULL;

		UTIL_SetOrigin(pTrigger, vecOrigin);
		UTIL_SetSize(pTrigger, vecMins, vecMaxs);
		pTrigger->SetOwnerEntity(pOwner);
		pTrigger->SetParent(pOwner);

		pTrigger->Spawn();

		return pTrigger;
	}

	//
	// Setup the entity

	void Spawn(void)
	{
		BaseClass::Spawn();

		SetSolid(SOLID_BBOX);
		SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	}

	void Activate()
	{
		BaseClass::Activate();
		SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID); // Fixes up old savegames
	}

	virtual void StartTouch(CBaseEntity* pOther) {
		if (CanTouch(pOther))
		{
			if (m_nTouchCount == 0)
				GetHologram()->SetInterference(true);

			m_nTouchCount++;
		}
	}
	virtual void EndTouch(CBaseEntity* pOther) {
		if (CanTouch(pOther))
		{
			m_nTouchCount--;

			if (m_nTouchCount == 0)
				GetHologram()->SetInterference(false);
		}
	}

private:
	inline CNPC_SinHologram* GetHologram() { return static_cast<CNPC_SinHologram*> (GetOwnerEntity()); }
	bool CanTouch(CBaseEntity* pOther);

	int		m_nTouchCount;
};

BEGIN_DATADESC(CHologramTrigger)
DEFINE_FIELD(m_nTouchCount, FIELD_INTEGER),
END_DATADESC();

LINK_ENTITY_TO_CLASS(trigger_sin_hologram, CHologramTrigger);

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_SinHologram::Classify(void)
{
	return CLASS_NONE;
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_SinHologram::GetSoundInterests(void)
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_SinHologram::Spawn()
{
	Precache();
	SetModel(STRING(GetModelName()));

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE|FSOLID_NOT_SOLID);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(DONT_BLEED);
	m_iHealth = 8;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;

	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION | EFL_NO_DAMAGE_FORCES | EFL_DONTBLOCKLOS);

	NPCInit();

	m_takedamage = DAMAGE_NO;

	CHologramTrigger::Create(GetAbsOrigin(), Vector(-15, -15, 0), Vector(15, 15, 72), this);
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_SinHologram::Precache()
{
	PrecacheModel(STRING(GetModelName()));

	BaseClass::Precache();
}

bool CNPC_SinHologram::UseSemaphore(void)
{
	if (HasSpawnFlags(SF_ACTOR_IGNORE_SEMAPHORE))
		return false;

	return BaseClass::UseSemaphore();
}

bool CHologramTrigger::CanTouch(CBaseEntity* pOther)
{
	if (!pOther || pOther == GetOwnerEntity() || pOther->GetMoveType() == MOVETYPE_NONE || !pOther->IsSolid())
		return false;

	if (pOther->GetFlags() & (FL_NPC | FL_CLIENT))
		return true;

	if (pOther->GetMoveType() == MOVETYPE_VPHYSICS)
		return true;

	return false;
}

void CNPC_SinHologram::SetInterference(bool bEnable)
{
	if (bEnable)
	{
		m_nRenderFX = kRenderFxHologram;
		m_nRenderMode = kRenderTransTexture;
		SetRenderColorA(200);
		m_nSkin = 1;
	}
	else
	{
		m_nRenderFX = kRenderFxNone;
		m_nRenderMode = kRenderNormal;
		SetRenderColorA(255);
		m_nSkin = 0;
	}
}
