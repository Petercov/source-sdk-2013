//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//========= Copyright © 1996-2003, Valve LLC, All rights reserved. ============
//
// Purpose: This is the incendiary rifle.
//
//=============================================================================


#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "soundent.h"
#include "player.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "weapon_flaregun.h"
#include "gamestats.h"
#include "decals.h"
#include "te_effect_dispatch.h"
#include "ai_basenpc.h"
#include "rumble_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef MAPBASE
#define BOLT_AIR_VELOCITY	2500
#define BOLT_WATER_VELOCITY	1500

#define IRIFLE_DAMAGE_TYPE DMG_ENERGYBEAM

extern acttable_t* GetAR2Acttable();
extern int GetAR2ActtableCount();

extern ConVar weapon_crossbow_new_hit_locations;

extern ConVar	sk_plr_dmg_irifle;
extern ConVar	sk_npc_dmg_irifle;

//-----------------------------------------------------------------------------
// Plasma Bolt
//-----------------------------------------------------------------------------
class CPlasmaBolt : public CBaseCombatCharacter
{
	DECLARE_CLASS(CPlasmaBolt, CBaseCombatCharacter);

public:
	CPlasmaBolt();
	~CPlasmaBolt();

	Class_T Classify(void) { return CLASS_NONE; }

public:
	void Spawn(void);
	void Precache(void);
	void BubbleThink(void);
	void BoltTouch(CBaseEntity* pOther);
	bool CreateVPhysics(void);
	unsigned int PhysicsSolidMaskForEntity() const;
	static CPlasmaBolt* BoltCreate(const Vector& vecOrigin, const QAngle& angAngles, CBaseCombatCharacter* pentOwner = NULL);

	void InputSetDamage(inputdata_t& inputdata);
	float m_flDamage;

	virtual void SetDamage(float flDamage) { m_flDamage = flDamage; }

protected:

	bool	CreateSprites(void);

	//CHandle<CSprite>		m_pGlowSprite;
	//CHandle<CSpriteTrail>	m_pGlowTrail;

	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};
LINK_ENTITY_TO_CLASS(plasma_bolt, CPlasmaBolt);

BEGIN_DATADESC(CPlasmaBolt)
// Function Pointers
DEFINE_FUNCTION(BubbleThink),
DEFINE_FUNCTION(BoltTouch),

// These are recreated on reload, they don't need storage
//DEFINE_FIELD(m_pGlowSprite, FIELD_EHANDLE),
//DEFINE_FIELD( m_pGlowTrail, FIELD_EHANDLE ),

DEFINE_KEYFIELD(m_flDamage, FIELD_FLOAT, "Damage"),

// Inputs
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetDamage", InputSetDamage),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST(CPlasmaBolt, DT_PlasmaBolt)
END_SEND_TABLE();

CPlasmaBolt* CPlasmaBolt::BoltCreate(const Vector& vecOrigin, const QAngle& angAngles, CBaseCombatCharacter* pentOwner)
{
	// Create a new entity with CPlasmaBolt private data
	CPlasmaBolt* pBolt = (CPlasmaBolt*)CreateEntityByName("plasma_bolt");
	UTIL_SetOrigin(pBolt, vecOrigin);
	pBolt->SetAbsAngles(angAngles);
	pBolt->Spawn();
	pBolt->SetOwnerEntity(pentOwner);
	if (pentOwner && pentOwner->IsNPC())
		pBolt->m_flDamage = sk_npc_dmg_irifle.GetFloat();
	//else
	//	pBolt->m_flDamage = sk_plr_dmg_irifle.GetFloat();

	return pBolt;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPlasmaBolt::CPlasmaBolt(void)
{
	// Independent bolts without m_flDamage set need damage
	m_flDamage = sk_plr_dmg_irifle.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPlasmaBolt::~CPlasmaBolt(void)
{
	
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPlasmaBolt::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
unsigned int CPlasmaBolt::PhysicsSolidMaskForEntity() const
{
	return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX) & ~CONTENTS_GRATE;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPlasmaBolt::CreateSprites(void)
{
	

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlasmaBolt::Spawn(void)
{
	Precache();

	SetModel("models/crossbow_bolt.mdl");
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	UTIL_SetSize(this, -Vector(0.3f, 0.3f, 0.3f), Vector(0.3f, 0.3f, 0.3f));
	SetSolid(SOLID_BBOX);
	SetGravity(0.05f);

	SetRenderMode(kRenderNone);

	// Make sure we're updated if we're underwater
	UpdateWaterState();

	SetTouch(&CPlasmaBolt::BoltTouch);

	SetThink(&CPlasmaBolt::BubbleThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	CreateSprites();
}


void CPlasmaBolt::Precache(void)
{
	// This is used by C_TEStickyBolt, despte being different from above!!!
	PrecacheModel("models/crossbow_bolt.mdl");

	PrecacheParticleSystem("irifle_smoke");
	PrecacheParticleSystem("irifle_sparks");

	PrecacheScriptSound("Weapon_IRifle.PlasmaHitBody");
	PrecacheScriptSound("Weapon_IRifle.PlasmaHitWorld");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlasmaBolt::InputSetDamage(inputdata_t& inputdata)
{
	m_flDamage = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CPlasmaBolt::BoltTouch(CBaseEntity* pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		// 
		// But some physics objects that are also triggers (like weapons) shouldn't go through this check.
		// 
		// Note: rpg_missile has the same code, except it properly accounts for weapons in a different way.
		// This was discovered after I implemented this and both work fine, but if this ever causes problems,
		// use rpg_missile's implementation:
		// 
		// if ( pOther->IsSolidFlagSet(FSOLID_TRIGGER|FSOLID_VOLUME_CONTENTS) && pOther->GetCollisionGroup() != COLLISION_GROUP_WEAPON )
		// 
		if (pOther->GetMoveType() == MOVETYPE_NONE && ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY)))
			return;
	}

	if (pOther->m_takedamage != DAMAGE_NO)
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize(vecNormalizedVel);

#if defined(HL2_EPISODIC)
		//!!!HACKHACK - specific hack for ep2_outland_10 to allow crossbow bolts to pass through her bounding box when she's crouched in front of the player
		// (the player thinks they have clear line of sight because Alyx is crouching, but her BBOx is still full-height and blocks crossbow bolts.
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->Classify() == CLASS_PLAYER_ALLY_VITAL && FStrEq(STRING(gpGlobals->mapname), "ep2_outland_10"))
		{
			// Change the owner to stop further collisions with Alyx. We do this by making her the owner.
			// The player won't get credit for this kill but at least the bolt won't magically disappear!
			SetOwnerEntity(pOther);
			return;
		}
#endif//HL2_EPISODIC

		CBaseAnimating* pOtherAnimating = pOther->GetBaseAnimating();
		if (weapon_crossbow_new_hit_locations.GetInt() > 0)
		{
			// A very experimental and weird way of getting a crossbow bolt to deal accurate knockback.
			if (pOtherAnimating && pOtherAnimating->GetModelPtr() && pOtherAnimating->GetModelPtr()->numbones() > 1)
			{
				int iClosestBone = -1;
				float flCurDistSqr = Square(128.0f);
				matrix3x4_t bonetoworld;
				Vector vecBonePos;
				for (int i = 0; i < pOtherAnimating->GetModelPtr()->numbones(); i++)
				{
					pOtherAnimating->GetBoneTransform(i, bonetoworld);
					MatrixPosition(bonetoworld, vecBonePos);

					float flDist = vecBonePos.DistToSqr(GetLocalOrigin());
					if (flDist < flCurDistSqr)
					{
						iClosestBone = i;
						flCurDistSqr = flDist;
					}
				}

				if (iClosestBone != -1)
				{
					tr.physicsbone = pOtherAnimating->GetPhysicsBone(iClosestBone);
				}
			}
		}

		CTakeDamageInfo	dmgInfo(this, GetOwnerEntity(), m_flDamage, IRIFLE_DAMAGE_TYPE);
		if (pOtherAnimating && pOtherAnimating->PassesDamageFilter(dmgInfo))
		{
			pOtherAnimating->Ignite(30.0f);
		}

		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer() && pOther->IsNPC())
		{
			dmgInfo.AdjustPlayerDamageInflictedForSkillLevel();
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);

			CBasePlayer* pPlayer = ToBasePlayer(GetOwnerEntity());
			if (pPlayer)
			{
				gamestats->Event_WeaponHit(pPlayer, true, "weapon_irifle", dmgInfo);
			}

		}
		else
		{
			CalculateMeleeDamageForce(&dmgInfo, vecNormalizedVel, tr.endpos, 0.7f);
			dmgInfo.SetDamagePosition(tr.endpos);
			pOther->DispatchTraceAttack(dmgInfo, vecNormalizedVel, &tr);
		}

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if (pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS)
			return;

		if (!pOther->IsAlive())
		{
			// We killed it! 
			const surfacedata_t* pdata = physprops->GetSurfaceData(tr.surface.surfaceProps);
			if (pdata->game.material == CHAR_TEX_GLASS)
			{
				return;
			}
		}

		SetAbsVelocity(Vector(0, 0, 0));

		// play body "thwack" sound
		EmitSound("Weapon_IRifle.PlasmaHitBody");

		UTIL_DecalTrace(&tr, "FadingScorch");
		/*
		Vector vForward;

		AngleVectors(GetAbsAngles(), &vForward);
		VectorNormalize(vForward);

		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_BLOCKLOS, pOther, COLLISION_GROUP_NONE, &tr2);

		if (tr2.fraction != 1.0f)
		{
			//			NDebugOverlay::Box( tr2.endpos, Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 255, 0, 0, 10 );
			//			NDebugOverlay::Box( GetAbsOrigin(), Vector( -16, -16, -16 ), Vector( 16, 16, 16 ), 0, 0, 255, 0, 10 );

			if (tr2.m_pEnt == NULL || (tr2.m_pEnt && tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE))
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.fraction != 1.0f;

				DispatchEffect("BoltImpact", data);
			}
		}
		*/
		SetTouch(NULL);
		SetThink(NULL);

		//if (!g_pGameRules->IsMultiplayer())
		{
			UTIL_Remove(this);
		}
	}
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		if (pOther->GetMoveType() == MOVETYPE_NONE && !(tr.surface.flags & SURF_SKY))
		{
			EmitSound("Weapon_IRifle.PlasmaHitWorld");

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize(vecDir);

			// See if we should reflect off this surface
			float hitDot = DotProduct(tr.plane.normal, -vecDir);

			if ((hitDot < 0.5f) && (speed > 100))
			{
				Vector vReflection = 2.0f * tr.plane.normal * hitDot + vecDir;

				QAngle reflectAngles;

				VectorAngles(vReflection, reflectAngles);

				SetLocalAngles(reflectAngles);

				SetAbsVelocity(vReflection * speed * 0.75f);

				// Start to sink faster
				SetGravity(1.0f);
			}
			else
			{
				//FIXME: We actually want to stick (with hierarchy) to what we've hit
				SetMoveType(MOVETYPE_NONE);

				AddEffects(EF_NODRAW);
				SetTouch(NULL);
				SetThink(&CPlasmaBolt::SUB_Remove);
				SetNextThink(gpGlobals->curtime + 2.0f);
			}

			CEffectData	data;
			data.m_vOrigin = tr.endpos;
			data.m_vNormal = tr.plane.normal;
			data.m_nEntIndex = 0;

			DispatchEffect("IrifleBoltImpact", data);
			UTIL_DecalTrace(&tr, "FadingScorch");

			// Shoot some sparks
			if (UTIL_PointContents(GetAbsOrigin()) != CONTENTS_WATER)
			{
				g_pEffects->Sparks(GetAbsOrigin());
			}
		}
		else
		{
			// Put a mark unless we've hit the sky
			if ((tr.surface.flags & SURF_SKY) == false)
			{
				UTIL_DecalTrace(&tr, "FadingScorch");
			}

			UTIL_Remove(this);
		}
	}

	//if (g_pGameRules->IsMultiplayer())
	{
		//		SetThink( &CPlasmaBolt::ExplodeThink );
		//		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlasmaBolt::BubbleThink(void)
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.1f);

	// Make danger sounds out in front of me, to scare snipers back into their hole
	CSoundEnt::InsertSound(SOUND_DANGER_SNIPERONLY, GetAbsOrigin() + GetAbsVelocity() * 0.2, 120.0f, 0.5f, this, SOUNDENT_CHANNEL_REPEATED_DANGER);

	if (GetWaterLevel() == 0)
		return;

	UTIL_BubbleTrail(GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5);
}
#endif

//###########################################################################
//	>> CWeaponIRifle
//###########################################################################

class CWeaponIRifle : public CBaseHLCombatWeapon
{
public:

	CWeaponIRifle();

	DECLARE_SERVERCLASS();
	DECLARE_CLASS( CWeaponIRifle, CBaseHLCombatWeapon );

	void	Precache( void );
	bool	Deploy( void );

	void	PrimaryAttack( void );
	virtual float GetFireRate( void ) { return 1; };

	int CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}


#ifdef MAPBASE
	virtual int	GetMinBurst() { return 1; }
	virtual int	GetMaxBurst() { return 1; }

	virtual float	GetMinRestTime(void) { return 3.0f; } // 1.5f
	virtual float	GetMaxRestTime(void) { return 3.0f; } // 2.0f

	virtual void	Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary);
	virtual void	Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);

	void	FireBolt(void);
	void	FireNPCBolt(CAI_BaseNPC* pOwner, Vector& vecShootOrigin, Vector& vecShootDir);
#endif

#ifndef MAPBASE
	DECLARE_ACTTABLE();
#else
	acttable_t* ActivityList(void) { return GetAR2Acttable(); }
	int ActivityListCount(void) { return GetAR2ActtableCount(); }
#endif // !MAPBASE

};

IMPLEMENT_SERVERCLASS_ST(CWeaponIRifle, DT_WeaponIRifle)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_irifle, CWeaponIRifle);
PRECACHE_WEAPON_REGISTER(weapon_irifle);

//---------------------------------------------------------
// Activity table
//---------------------------------------------------------
#ifndef MAPBASE
acttable_t	CWeaponIRifle::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_ML, true },
};
IMPLEMENT_ACTTABLE(CWeaponIRifle);
#endif // !MAPBASE


//---------------------------------------------------------
// Constructor
//---------------------------------------------------------
CWeaponIRifle::CWeaponIRifle()
{
	m_bReloadsSingly = true;

	m_fMinRange1		= 65;
	m_fMinRange2		= 65;
#ifndef MAPBASE
	m_fMaxRange1		= 200;
	m_fMaxRange2		= 200;
#else
	m_fMaxRange1 = 4096;
	m_fMaxRange2 = 4096;
#endif
}

//---------------------------------------------------------
//---------------------------------------------------------
void CWeaponIRifle::Precache( void )
{
	BaseClass::Precache();

#ifdef MAPBASE
	PrecacheParticleSystem("irifle_smoke");
	PrecacheParticleSystem("irifle_sparks");

	PrecacheScriptSound("Weapon_IRifle.PlasmaHitBody");
	PrecacheScriptSound("Weapon_IRifle.PlasmaHitWorld");
#endif // MAPBASE
}

//---------------------------------------------------------
//---------------------------------------------------------
void CWeaponIRifle::PrimaryAttack( void )
{
#ifndef MAPBASE
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	m_iClip1 = m_iClip1 - 1;

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pOwner->m_flNextAttack = gpGlobals->curtime + 1;

	CFlare* pFlare = CFlare::Create(pOwner->Weapon_ShootPosition(), pOwner->EyeAngles(), pOwner, FLARE_DURATION);

	if (pFlare == NULL)
		return;

	Vector forward;
	pOwner->EyeVectors(&forward);

	pFlare->SetAbsVelocity(forward * 1500);

	WeaponSound(SINGLE);
#else
	FireBolt();

	SetWeaponIdleTime(gpGlobals->curtime + SequenceDuration(ACT_VM_PRIMARYATTACK));

	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

		pPlayer->SetAnimation(PLAYER_ATTACK1);
	}
#endif // !MAPBASE
}

//---------------------------------------------------------
// BUGBUG - don't give ammo here.
//---------------------------------------------------------
bool CWeaponIRifle::Deploy( void )
{
#ifndef MAPBASE
	CBaseCombatCharacter* pOwner = GetOwner();
	if (pOwner)
	{
		pOwner->GiveAmmo(90, m_iPrimaryAmmoType);
	}
#endif // !MAPBASE

	return BaseClass::Deploy();
}

#ifdef MAPBASE
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponIRifle::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	{
		CAI_BaseNPC* pNPC = pOperator->MyNPCPointer();
		Assert(pNPC);

		Vector vecSrc = pNPC->Weapon_ShootPosition();
		Vector vecAiming = pNPC->GetActualShootTrajectory(vecSrc);

		FireNPCBolt(pNPC, vecSrc, vecAiming);
		//m_bMustReload = true;
	}
	break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponIRifle::Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCBolt(pOperator->MyNPCPointer(), vecShootOrigin, vecShootDir);

	//m_bMustReload = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponIRifle::FireNPCBolt(CAI_BaseNPC* pOwner, Vector& vecShootOrigin, Vector& vecShootDir)
{
	Assert(pOwner);

	QAngle angAiming;
	VectorAngles(vecShootDir, angAiming);

	CPlasmaBolt* pBolt = CPlasmaBolt::BoltCreate(vecShootOrigin, angAiming, pOwner);

	if (pOwner->GetWaterLevel() == 3)
	{
		pBolt->SetAbsVelocity(vecShootDir * BOLT_WATER_VELOCITY);
	}
	else
	{
		pBolt->SetAbsVelocity(vecShootDir * BOLT_AIR_VELOCITY);
	}

	m_iClip1--;

	WeaponSound(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 200, 0.2);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 2.5f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponIRifle::FireBolt(void)
{
	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	pOwner->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAG_RESTART);

	Vector vecAiming = pOwner->GetAutoaimVector(0);
	Vector vecSrc = pOwner->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);

#if defined(HL2_EPISODIC)
	// !!!HACK - the other piece of the Alyx crossbow bolt hack for Outland_10 (see ::BoltTouch() for more detail)
	if (FStrEq(STRING(gpGlobals->mapname), "ep2_outland_10"))
	{
		trace_t tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 24.0f, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

		if (tr.m_pEnt != NULL && tr.m_pEnt->Classify() == CLASS_PLAYER_ALLY_VITAL)
		{
			// If Alyx is right in front of the player, make sure the bolt starts outside of the player's BBOX, or the bolt
			// will instantly collide with the player after the owner of the bolt is switched to Alyx in ::BoltTouch(). We 
			// avoid this altogether by making it impossible for the bolt to collide with the player.
			vecSrc += vecAiming * 24.0f;
		}
	}
#endif

	CPlasmaBolt* pBolt = CPlasmaBolt::BoltCreate(vecSrc, angAiming, pOwner);

	if (pOwner->GetWaterLevel() == 3)
	{
		pBolt->SetAbsVelocity(vecAiming * BOLT_WATER_VELOCITY);
	}
	else
	{
		pBolt->SetAbsVelocity(vecAiming * BOLT_AIR_VELOCITY);
	}

	m_iClip1--;

	pOwner->ViewPunch(QAngle(-2, 0, 0));

	WeaponSound(SINGLE);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 200, 0.2);

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	if (!m_iClip1 && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pOwner->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.75;
}
#endif // MAPBASE
