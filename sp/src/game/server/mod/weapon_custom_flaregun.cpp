//========= Copyright Valve Corporation, All rights reserved. ============//
// Purpose:		Flare gun (fffsssssssssss!!)
//
//				This is a custom extension of Valve's CFlaregun class.
//				Some commented-out code has been duplicated from 
//				weapon_flaregun.cpp in order to keep the mod code isolated
//				from the base game.
//
//	Author:		1upD
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "basehlcombatweapon.h"
#include "decals.h"
#include "soundenvelope.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"
#include "weapon_flaregun.h"
#include "props.h"
#include "ai_basenpc.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Custom convars for flaregun
ConVar	flaregun_primary_velocity("sv_flaregun_primary_velocity", "1500");
ConVar	flaregun_secondary_velocity("sv_flaregun_secondary_velocity", "500");
ConVar	flaregun_duration_seconds("sv_flaregun_lifetime_seconds", "30");
ConVar	flaregun_stop_velocity("sv_flaregun_stop_velocity", "128");
ConVar	flaregun_projectile_sticky("sv_flaregun_projectile_sticky", "0");
ConVar	flaregun_dynamic_lights("sv_flaregun_dynamic_lights", "1");

extern ConVar    sk_plr_dmg_flare_round;
extern ConVar    sk_npc_dmg_flare_round;

#ifdef MAPBASE
extern acttable_t* GetPistolActtable();
extern int GetPistolActtableCount();

// Allows Weapon_BackupActivity() to access the 357's activity table.
extern acttable_t* Get357Acttable();
extern int Get357ActtableCount();
#endif

// Custom derived class for flare gun projectiles
class CFlareGunProjectile : public CFlare
{
	public:
		DECLARE_CLASS(CFlareGunProjectile, CFlare);
		static CFlareGunProjectile *Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime);
		void	IgniteOtherIfAllowed(CBaseEntity *pOther);
		void	FlareGunProjectileTouch(CBaseEntity *pOther);
		void	FlareGunProjectileBurnTouch(CBaseEntity *pOther);

#ifdef MAPBASE
		DECLARE_DATADESC();
		virtual unsigned int PhysicsSolidMaskForEntity(void) const;
#endif // MAPBASE
};

class CFlaregunCustom : public CFlaregun
{
	public:
		DECLARE_CLASS(CFlaregunCustom, CFlaregun);
		virtual bool	Reload(void);

#ifdef MAPBASE
		int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

		virtual int	GetMinBurst() { return 1; }
		virtual int	GetMaxBurst() { return 1; }
		virtual float	GetMinRestTime(void) { return 1.0f; }
		virtual float	GetMaxRestTime(void) { return 2.5f; }

		virtual float GetFireRate(void)
		{
			return 1.0f;
		}

		virtual const Vector& GetBulletSpread(void)
		{
			static Vector cone = VECTOR_CONE_15DEGREES;
			if (!GetOwner() || !GetOwner()->IsNPC())
				return cone;

			static Vector AllyCone = VECTOR_CONE_2DEGREES;
			static Vector NPCCone = VECTOR_CONE_5DEGREES;

			if (GetOwner()->MyNPCPointer()->IsPlayerAlly())
			{
				// 357 allies should be cooler
				return AllyCone;
			}

			return NPCCone;
		}

		void	FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);
		void	Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary);
		void	Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);

		virtual acttable_t* ActivityList(void) { return Get357Acttable(); }
		virtual int ActivityListCount(void) { return Get357ActtableCount(); }

		virtual acttable_t* GetBackupActivityList() { return GetPistolActtable(); }
		virtual int				GetBackupActivityListCount() { return GetPistolActtableCount(); }

		virtual float			NPC_GetProjectileSpeed() { return flaregun_primary_velocity.GetFloat(); }
		virtual float			NPC_GetProjectileGravity() { return 1.f; }
		virtual unsigned int	NPC_GetProjectileSolidMask() { return MASK_SOLID; }
#endif
};

IMPLEMENT_SERVERCLASS_ST(CFlaregun, DT_Flaregun)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_flaregun, CFlaregunCustom);
PRECACHE_WEAPON_REGISTER( weapon_flaregun );

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFlaregun::Precache( void )
{
	BaseClass::Precache();

	PrecacheScriptSound( "Flare.Touch" );

	PrecacheScriptSound( "Weapon_FlareGun.Burn" );

	UTIL_PrecacheOther( "env_flare" );
}

//-----------------------------------------------------------------------------
// Purpose: Fires a flare from a given flaregun with a given velocity
//			Acts like an extension method for CFlaregun
//-----------------------------------------------------------------------------
static void AttackWithVelocity(CFlaregun * flaregun, float projectileVelocity)
{
	CBasePlayer *pOwner = ToBasePlayer(flaregun->GetOwner());

	if (pOwner == NULL)
		return;

	if (flaregun->m_iClip1 <= 0)
	{
		flaregun->SendWeaponAnim(ACT_VM_DRYFIRE);
		pOwner->m_flNextAttack = gpGlobals->curtime + flaregun->SequenceDuration();
		return;
	}

	flaregun->m_iClip1 = flaregun->m_iClip1 - 1;

	flaregun->SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pOwner->m_flNextAttack = gpGlobals->curtime + 1;

	CFlare *pFlare = CFlareGunProjectile::Create(pOwner->Weapon_ShootPosition(), pOwner->EyeAngles(), pOwner, FLARE_DURATION);

	if (pFlare == NULL)
		return;

	Vector forward;
	pOwner->EyeVectors(&forward);
	forward *= projectileVelocity;
	forward += pOwner->GetAbsVelocity(); // Add the player's velocity to the forward vector so that the flare follows the player's motion
	forward.Normalized();

	pFlare->SetAbsVelocity(forward);
	pFlare->SetGravity(1.0f);
	pFlare->SetFriction(0.85f);
	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	flaregun->WeaponSound(SINGLE);
}

//-----------------------------------------------------------------------------
// Purpose: Main attack
//-----------------------------------------------------------------------------
void CFlaregun::PrimaryAttack( void )
{
	AttackWithVelocity(this, flaregun_primary_velocity.GetFloat());
}

//-----------------------------------------------------------------------------
// Purpose: Secondary attack - launches flares closer to the player
//-----------------------------------------------------------------------------
void CFlaregun::SecondaryAttack( void )
{
	AttackWithVelocity(this, flaregun_secondary_velocity.GetFloat());
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CFlaregunCustom::Reload(void)
{
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
	}
	return fRet;
}

#ifdef MAPBASE
BEGIN_DATADESC(CFlareGunProjectile)
DEFINE_ENTITYFUNC(FlareGunProjectileTouch),
DEFINE_ENTITYFUNC(FlareGunProjectileBurnTouch),
END_DATADESC();

LINK_ENTITY_TO_CLASS(env_flare_projectile, CFlareGunProjectile);

unsigned int CFlareGunProjectile::PhysicsSolidMaskForEntity(void) const
{
	return MASK_SOLID;
}
#endif // MAPBASE

//-----------------------------------------------------------------------------
// Purpose: Create function for Flare Gun projectile
// Input  : vecOrigin - 
//			vecAngles - 
//			*pOwner - 
// Output : CFlare
//-----------------------------------------------------------------------------
CFlareGunProjectile *CFlareGunProjectile::Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime)
{
#ifndef MAPBASE
	CFlareGunProjectile* pFlare = (CFlareGunProjectile*)CreateEntityByName("env_flare");
#else
	CFlareGunProjectile* pFlare = (CFlareGunProjectile*)CreateEntityByName("env_flare_projectile");
#endif // !MAPBASE

	if (pFlare == NULL)
		return NULL;

	UTIL_SetOrigin(pFlare, vecOrigin);

	pFlare->SetLocalAngles(vecAngles);
	pFlare->Spawn();
	pFlare->SetTouch(&CFlareGunProjectile::FlareGunProjectileTouch);
	pFlare->SetThink(&CFlare::FlareThink);
	pFlare->m_bLight = flaregun_dynamic_lights.GetBool();

	//Start up the flare
	pFlare->Start(lifetime);

	//Don't start sparking immediately
	pFlare->SetNextThink(gpGlobals->curtime + 0.5f);

	//Burn out time
	pFlare->m_flTimeBurnOut = gpGlobals->curtime + lifetime;

	// Time to next burn damage
	pFlare->m_flNextDamage = gpGlobals->curtime;


	pFlare->RemoveSolidFlags(FSOLID_NOT_SOLID);
	pFlare->AddSolidFlags(FSOLID_NOT_STANDABLE);

	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	pFlare->SetOwnerEntity(pOwner);
	pFlare->m_pOwner = pOwner;

	return pFlare;
}

//-----------------------------------------------------------------------------
// Purpose: Touch function for flaregun projectiles
// Input  : *pOther - The entity that the flare has collided with
//-----------------------------------------------------------------------------
void CFlareGunProjectile::FlareGunProjectileTouch(CBaseEntity *pOther)
{
	Assert(pOther);
	if (!pOther->IsSolid())
		return;

	if ((m_nBounces < 10) && (GetWaterLevel() < 1))
	{
		// Throw some real chunks here
		g_pEffects->Sparks(GetAbsOrigin());
	}

	//If the flare hit a person or NPC, do damage here.
	if (pOther && pOther->m_takedamage)
	{
		Vector vecNewVelocity = GetAbsVelocity();
		vecNewVelocity *= 0.1f;
		SetAbsVelocity(vecNewVelocity);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
		SetGravity(1.0f);
		Die(0.5);
		IgniteOtherIfAllowed(pOther);
		m_nBounces++;
		return;
	}
	else
	{
		// hit the world, check the material type here, see if the flare should stick.
		trace_t tr;
		tr = CBaseEntity::GetTouchTrace();

		//Only do this on the first bounce if the convar is set
		if (flaregun_projectile_sticky.GetBool() && m_nBounces == 0)
		{
			const surfacedata_t *pdata = physprops->GetSurfaceData(tr.surface.surfaceProps);

			if (pdata != NULL)
			{
				//Only embed into concrete and wood (jdw: too obscure for players?)
				//if ( ( pdata->gameMaterial == 'C' ) || ( pdata->gameMaterial == 'W' ) )
				{
					Vector	impactDir = (tr.endpos - tr.startpos);
					VectorNormalize(impactDir);

					float	surfDot = tr.plane.normal.Dot(impactDir);

					//Do not stick to ceilings or on shallow impacts
					if ((tr.plane.normal.z > -0.5f) && (surfDot < -0.9f))
					{
						RemoveSolidFlags(FSOLID_NOT_SOLID);
						AddSolidFlags(FSOLID_TRIGGER);
						UTIL_SetOrigin(this, tr.endpos + (tr.plane.normal * 2.0f));
						SetAbsVelocity(vec3_origin);
						SetMoveType(MOVETYPE_NONE);

						SetTouch(&CFlareGunProjectile::FlareGunProjectileBurnTouch);

						int index = decalsystem->GetDecalIndexForName("SmallScorch");
						if (index >= 0)
						{
							CBroadcastRecipientFilter filter;
							te->Decal(filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX(tr.m_pEnt), tr.hitbox, index);
						}

						CPASAttenuationFilter filter2(this, "Flare.Touch");
						EmitSound(filter2, entindex(), "Flare.Touch");

						return;
					}
				}
			}
		}

		//Scorch decal
		if (GetAbsVelocity().LengthSqr() > (250 * 250))
		{
			int index = decalsystem->GetDecalIndexForName("FadingScorch");
			if (index >= 0)
			{
				CBroadcastRecipientFilter filter;
				te->Decal(filter, 0.0, &tr.endpos, &tr.startpos, ENTINDEX(tr.m_pEnt), tr.hitbox, index);
			}
		}

		// Change our flight characteristics
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
		SetGravity(UTIL_ScaleForGravity(640));

		m_nBounces++;

		//After the first bounce, smacking into whoever fired the flare is fair game
		SetOwnerEntity(NULL);

		// Slow down
		Vector vecNewVelocity = GetAbsVelocity();
		vecNewVelocity.x *= 0.8f;
		vecNewVelocity.y *= 0.8f;
		SetAbsVelocity(vecNewVelocity);

		//Stopped?
		if (GetAbsVelocity().Length() < flaregun_stop_velocity.GetFloat())
		{
			RemoveSolidFlags(FSOLID_NOT_SOLID);
			AddSolidFlags(FSOLID_TRIGGER);
			SetAbsVelocity(vec3_origin);
			SetMoveType(MOVETYPE_NONE);
			SetTouch(&CFlareGunProjectile::FlareGunProjectileBurnTouch);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CFlareGunProjectile::FlareGunProjectileBurnTouch(CBaseEntity *pOther)
{
	if (pOther && pOther->m_takedamage && (m_flNextDamage < gpGlobals->curtime))
	{
		m_flNextDamage = gpGlobals->curtime + 1.0f;
		IgniteOtherIfAllowed(pOther);
	}
}

void CFlareGunProjectile::IgniteOtherIfAllowed(CBaseEntity * pOther)
{
	if (!m_pOwner)
		return;

	CTakeDamageInfo info;
	if (m_pOwner->IsPlayer())
		info.Set(this, m_pOwner, sk_plr_dmg_flare_round.GetFloat(), (DMG_BULLET | DMG_BURN));
	else
		info.Set(this, m_pOwner, sk_npc_dmg_flare_round.GetFloat(), (DMG_BULLET | DMG_BURN));

	// Don't burn the player
	if (!pOther->IsPlayer())
	{
		CAI_BaseNPC* pNPC;
		pNPC = dynamic_cast<CAI_BaseNPC*>(pOther);
		if (pNPC) {
			// Don't burn boss enemies
			if (FStrEq(STRING(pNPC->m_iClassname), "npc_combinegunship")
				|| FStrEq(STRING(pNPC->m_iClassname), "npc_combinedropship")
				|| FStrEq(STRING(pNPC->m_iClassname), "npc_strider")
				|| FStrEq(STRING(pNPC->m_iClassname), "npc_helicopter")
				)
				return;

			// Don't burn friendly NPCs
			if (!pNPC->IsPlayerAlly() && pNPC->PassesDamageFilter(info))
			{
				// Burn this NPC
				pNPC->IgniteLifetime(flaregun_duration_seconds.GetFloat());
			}

		}

		// If this is a breakable prop, ignite it!
		CBreakableProp* pBreakable;
		pBreakable = dynamic_cast<CBreakableProp*>(pOther);
		if (pBreakable)
		{
			pBreakable->IgniteLifetime(flaregun_duration_seconds.GetFloat());
			// Don't do damage to props that are on fire
			if (pBreakable->IsOnFire())
				return;
		}
	}

	// Do damage
	pOther->TakeDamage(info);

}

#ifdef MAPBASE
void CFlaregunCustom::FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir)
{
	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	QAngle angles;
	VectorAngles(vecShootDir, angles);
	CFlare* pFlare = CFlareGunProjectile::Create(vecShootOrigin, angles, pOperator, FLARE_DURATION);

	if (pFlare == NULL)
		return;

	Vector forward = vecShootDir;
	forward *= flaregun_primary_velocity.GetFloat();
	forward += pOperator->GetAbsVelocity(); // Add the player's velocity to the forward vector so that the flare follows the player's motion

	pFlare->SetAbsVelocity(forward);
	pFlare->SetGravity(1.0f);
	pFlare->SetFriction(0.85f);
	pFlare->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

void CFlaregunCustom::Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}
void CFlaregunCustom::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
		// HACKHACK: Ignore the regular firing event while dual-wielding
		if (GetLeftHandGun())
			return;

		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetShootEnemyDir(vecShootOrigin);

		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}
#endif // MAPBASE
