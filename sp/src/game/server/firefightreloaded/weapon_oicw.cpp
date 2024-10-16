﻿//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "basecombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "weapon_oicw.h"
#include "grenade_ar2.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "ai_memory.h"
#include "soundent.h"
#include "hl2_player.h"
#include "EntityFlame.h"
#include "weapon_flaregun.h"
#include "te_effect_dispatch.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"

extern ConVar    sk_plr_dmg_oicw_grenade;
extern ConVar    sk_npc_dmg_oicw_grenade;

float GetCurrentGravity(void);

#define OICW_ZOOM_RATE	0.5f	// Interval between zoom levels in seconds.
#define	OICW_FASTEST_REFIRE_TIME		0.1f

//=========================================================
//=========================================================

BEGIN_DATADESC(CWeaponOICW)

DEFINE_FIELD(m_nShotsFired, FIELD_INTEGER),
#ifndef MAPBASE
DEFINE_FIELD(m_bZoomed, FIELD_BOOLEAN),
#endif // !MAPBASE
DEFINE_FIELD(m_flSoonestPrimaryAttack, FIELD_TIME),
DEFINE_FIELD(m_iFireMode, FIELD_INTEGER),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponOICW, DT_WeaponOICW)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_oicw, CWeaponOICW);
PRECACHE_WEAPON_REGISTER(weapon_oicw);

acttable_t	CWeaponOICW::m_acttable[] =
{
#if AR2_ACTIVITY_FIX == 1
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_AR2,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_AR2,			false },

	{ ACT_WALK,						ACT_WALK_AR2,					true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_AR2_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_AR2_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_AR2,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_AR2_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_AR2_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_AR2,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_AR2_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_AR2_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_AR2_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_AR2_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_AR2,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_AR2_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_AR2_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_AR2,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_AR2_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_AR2_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_AR2,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_AR2,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_AR2,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_AR2_LOW,				true },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_AR2_LOW,		false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
#else
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },		// FIXME: hook to AR2 unique

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
#endif

#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_ARM,						ACT_ARM_RIFLE,					false },
	{ ACT_DISARM,					ACT_DISARM_RIFLE,				false },
#endif

#if EXPANDED_HL2_COVER_ACTIVITIES
	{ ACT_RANGE_AIM_MED,			ACT_RANGE_AIM_AR2_MED,			false },
	{ ACT_RANGE_ATTACK1_MED,		ACT_RANGE_ATTACK_AR2_MED,		false },

	{ ACT_COVER_WALL_R,				ACT_COVER_WALL_R_RIFLE,			false },
	{ ACT_COVER_WALL_L,				ACT_COVER_WALL_L_RIFLE,			false },
	{ ACT_COVER_WALL_LOW_R,			ACT_COVER_WALL_LOW_R_RIFLE,		false },
	{ ACT_COVER_WALL_LOW_L,			ACT_COVER_WALL_LOW_L_RIFLE,		false },
#endif

#ifdef MAPBASE
	// HL2:DM activities (for third-person animations in SP)
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_AR2,                    false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_AR2,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_AR2,            false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_AR2,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_SMG1,        false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_AR2,                    false },
#if EXPANDED_HL2DM_ACTIVITIES
	{ ACT_HL2MP_WALK,					ACT_HL2MP_WALK_AR2,						false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK2,	ACT_HL2MP_GESTURE_RANGE_ATTACK2_AR2,	false },
#endif
#endif
};

IMPLEMENT_ACTTABLE(CWeaponOICW);

#ifdef MAPBASE
// Allows Weapon_BackupActivity() to access the SMG1's activity table.
acttable_t* GetOICWActtable()
{
	return CWeaponOICW::m_acttable;
}

int GetOICWActtableCount()
{
	return ARRAYSIZE(CWeaponOICW::m_acttable);
}

#define m_bZoomed m_bInZoom
#endif

CWeaponOICW::CWeaponOICW()
{
	m_fMinRange1 = 65;
	m_fMaxRange1 = 2048;

	m_fMinRange2 = 256;
	m_fMaxRange2 = 1024;

	m_nShotsFired = 0;
	m_iFireMode = 0;

	m_bAltFiresUnderwater = false;

	m_flSoonestPrimaryAttack = gpGlobals->curtime;
}

void CWeaponOICW::Precache(void)
{
	UTIL_PrecacheOther("grenade_ar2");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Offset the autoreload
//-----------------------------------------------------------------------------
bool CWeaponOICW::Deploy(void)
{
	m_nShotsFired = 0;
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();

	return BaseClass::Deploy();
}

void CWeaponOICW::Equip(CBaseCombatCharacter *pOwner)
{
	m_flNextSecondaryAttack = gpGlobals->curtime;

	return BaseClass::Equip(pOwner);
}

//-----------------------------------------------------------------------------
// Purpose: Handle grenade detonate in-air (even when no ammo is left)
//-----------------------------------------------------------------------------
void CWeaponOICW::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if ((pOwner->m_nButtons & IN_ATTACK) == false)
	{
		m_nShotsFired = 0;
	}

	//Zoom in
	if (pOwner->m_afButtonPressed & IN_ATTACK2)
	{
		if (m_iFireMode == 0)
		{
			Zoom();
		}
		else if (m_iFireMode == 1)
		{
			if (m_flNextSecondaryAttack <= gpGlobals->curtime)
			{
				if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
				{
					if (m_flNextEmptySoundTime < gpGlobals->curtime)
					{
						WeaponSound(EMPTY);
						m_flNextSecondaryAttack = m_flNextEmptySoundTime = gpGlobals->curtime + 0.5;
					}
				}
				else if (pOwner->GetWaterLevel() == 3 && m_bAltFiresUnderwater == false)
				{
					// This weapon doesn't fire underwater
					WeaponSound(EMPTY);
					m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
					return;
				}
				else
				{
					GrenadeAttack();
				}
			}
		}
	}

	//Throw a grenade.
	if (pOwner->m_afButtonPressed & IN_ATTACK3)
	{
		if (m_iFireMode == 0)
		{
			if (m_bZoomed)
			{
				Zoom();
			}
			//CFmtStr hint;
			//hint.sprintf("#Valve_OICW_Grenades");
			//pOwner->HintMessage("#Valve_OICW_Grenades");
			m_iFireMode = 1;
			WeaponSound(SPECIAL3);
		}
		else if (m_iFireMode == 1)
		{
			//CFmtStr hint;
			//hint.sprintf("#Valve_OICW_Scope");
			//pOwner->HintMessage("#Valve_OICW_Scope");
			m_iFireMode = 0;
			WeaponSound(SPECIAL3);
		}
		return;
	}

	//Allow a refire as fast as the player can click
	if (m_bZoomed && ((pOwner->m_nButtons & IN_ATTACK) == false) && (m_flSoonestPrimaryAttack < gpGlobals->curtime))
	{
		m_flNextPrimaryAttack = gpGlobals->curtime - 0.1f;
	}

	//Don't kick the same when we're zoomed in
	if (m_bZoomed)
	{
		m_fFireDuration = 0.05f;
	}

	if (pOwner->m_nButtons & IN_ATTACK2)
		return;

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponOICW::GetPrimaryAttackActivity(void)
{
	if (m_nShotsFired < 2)
		return ACT_VM_PRIMARYATTACK;

	if (m_nShotsFired < 3)
		return ACT_VM_RECOIL1;

	if (m_nShotsFired < 4)
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CWeaponOICW::PrimaryAttack(void)
{
	m_nShotsFired++;

	m_flSoonestPrimaryAttack = gpGlobals->curtime + OICW_FASTEST_REFIRE_TIME;

	BaseClass::PrimaryAttack();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOICW::GrenadeAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	//Must have ammo
	if ((pPlayer->GetAmmoCount(m_iSecondaryAmmoType) <= 0) || (pPlayer->GetWaterLevel() == 3))
	{
		SendWeaponAnim(ACT_VM_DRYFIRE);
		BaseClass::WeaponSound(EMPTY);
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	if (m_bInReload)
		m_bInReload = false;

	// MUST call sound before removing a round from the clip of a CMachineGun
	BaseClass::WeaponSound(WPN_DOUBLE);

	pPlayer->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecThrow;
	// Don't autoaim on grenade tosses
	AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
	VectorScale(vecThrow, 1000.0f, vecThrow);

	//Create the grenade
	QAngle angles;
	VectorAngles(vecThrow, angles);
	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecSrc, angles, pPlayer);
	pGrenade->SetAbsVelocity(vecThrow);

	pGrenade->SetLocalAngularVelocity(RandomAngle(-400, 400));
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	pGrenade->SetThrower(GetOwner());
	pGrenade->SetDamage(sk_plr_dmg_oicw_grenade.GetFloat());

	SendWeaponAnim(ACT_VM_SECONDARYATTACK);

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Decrease ammo
	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;

	// Register a muzzleflash for the AI.
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, false, GetClassname());
}

void CWeaponOICW::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	{
		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
		WeaponSound(SINGLE_NPC);
		pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
		pOperator->DoMuzzleFlash();
		m_iClip1 = m_iClip1 - 1;
	}
	break;
	case EVENT_WEAPON_AR2_ALTFIRE:
	{
		WeaponSound(DOUBLE_NPC);

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
		if (!npc)
			return;

		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetShootEnemyDir(vecShootOrigin);

		Vector vecTarget = npc->GetAltFireTarget();
		Vector vecThrow;
		if (vecTarget == vec3_origin)
			AngleVectors(npc->EyeAngles(), &vecThrow); // Not much else to do, unfortunately
		else
		{
			// Because this is happening right now, we can't "VecCheckThrow" and can only "VecDoThrow", you know what I mean?
			// ...Anyway, this borrows from that so we'll never return vec3_origin.
			//vecThrow = VecCheckThrow( this, vecShootOrigin, vecTarget, 600.0, 0.5 );

			vecThrow = (vecTarget - vecShootOrigin);

			// throw at a constant time
			float time = vecThrow.Length() / 600.0;
			vecThrow = vecThrow * (1.0 / time);

			// adjust upward toss to compensate for gravity loss
			vecThrow.z += (GetCurrentGravity() * 0.5) * time * 0.5;
		}

		CGrenadeAR2* pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecShootOrigin, vec3_angle, npc);
		pGrenade->SetAbsVelocity(vecThrow);
		pGrenade->SetLocalAngularVelocity(QAngle(0, 400, 0));
		pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);

		pGrenade->SetThrower(npc);

		pGrenade->SetGravity(0.5); // lower gravity since grenade is aerodynamic and engine doesn't know it.

		pGrenade->SetDamage(sk_npc_dmg_oicw_grenade.GetFloat());

		variant_t var;
		var.SetEntity(pGrenade);
		npc->FireNamedOutput("OnThrowGrenade", var, pGrenade, npc);
	}
	break;
	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

/*
==================================================
AddViewKick
==================================================
*/

void CWeaponOICW::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	2.5f	//Degrees
#define	SLIDE_LIMIT			2.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

void CWeaponOICW::Zoom(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	//color32 lightGreen = { 50, 255, 170, 32 };

	if (m_bZoomed)
	{
		if (pPlayer->SetFOV(this, 0, 0.1f))
		{
			pPlayer->ShowViewModel(true);

			// Zoom out to the default zoom level
			WeaponSound(SPECIAL2);

			m_bZoomed = false;

			//UTIL_ScreenFade(pPlayer, lightGreen, 0.2f, 0, (FFADE_IN | FFADE_PURGE));
		}
	}
	else
	{
		if (pPlayer->SetFOV(this, 35, 0.1f))
		{
			pPlayer->ShowViewModel(false);

			WeaponSound(SPECIAL1);

			m_bZoomed = true;

			//UTIL_ScreenFade(pPlayer, lightGreen, 0.2f, 0, (FFADE_OUT | FFADE_PURGE | FFADE_STAYOUT));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CWeaponOICW::GetFireRate(void)
{
	if (m_bZoomed)
		return 0.3f;

	return 0.1f;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : NULL - 
//-----------------------------------------------------------------------------
bool CWeaponOICW::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	if (m_bZoomed)
	{
		Zoom();
	}

	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponOICW::Reload(void)
{
	if (m_bZoomed)
	{
		Zoom();
	}

	bool fRet;
	float fCacheTime = m_flNextSecondaryAttack;

	fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		// Undo whatever the reload process has done to our secondary
		// attack timer. We allow you to interrupt reloading to fire
		// a grenade.
		m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;

		WeaponSound(RELOAD);
	}

	return fRet;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponOICW::Drop(const Vector &velocity)
{
	if (m_bZoomed)
	{
		Zoom();
	}

	BaseClass::Drop(velocity);
}