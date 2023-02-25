//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
//#include "basecombatweapon.h"
#include "hl1mp_basecombatweapon_shared.h"
#include "npcevent.h"
//#include "basecombatcharacter.h"
//#include "AI_BaseNPC.h"
#ifdef CLIENT_DLL
#include "c_baseplayer.h"
#else
#include "player.h"
#endif
#include "hl1mp_weapon_mp5.h"
//#include "hl1_weapon_mp5.h"
#ifdef CLIENT_DLL
#else
#include "hl1_grenade_mp5.h"
#endif
#include "gamerules.h"
#ifdef CLIENT_DLL
#else
#include "soundent.h"
#include "game.h"
#ifdef MAPBASE
#include "ai_basenpc.h"
#include "te_effect_dispatch.h"
#endif // MAPBASE
#endif
#include "in_buttons.h"
#include "engine/IEngineSound.h"

#ifndef MAPBASE
extern ConVar    sk_plr_dmg_mp5_grenade;
extern ConVar    sk_max_mp5_grenade;
extern ConVar	 sk_mp5_grenade_radius;
#else
extern ConVar hl1_sk_plr_dmg_mp5_grenade;
extern ConVar hl1_sk_max_mp5_grenade;
extern ConVar hl1_sk_mp5_grenade_radius;
extern ConVar sk_hgrunt_gspeed;

#define sk_plr_dmg_mp5_grenade hl1_sk_plr_dmg_mp5_grenade
#define sk_plr_max_mp5_grenade hl1_sk_max_mp5_grenade
#define sk_mp5_grenade_radius hl1_sk_mp5_grenade_radius
#endif // !MAPBASE


//=========================================================
//=========================================================

IMPLEMENT_NETWORKCLASS_ALIASED( HL1WeaponMP5, DT_HL1WeaponMP5 );

BEGIN_NETWORK_TABLE(CHL1WeaponMP5, DT_HL1WeaponMP5)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CHL1WeaponMP5)
END_PREDICTION_DATA()

#ifdef HL1_DLL
LINK_ENTITY_TO_CLASS(weapon_mp5, CHL1WeaponMP5);

PRECACHE_WEAPON_REGISTER(weapon_mp5);
#else
LINK_ENTITY_TO_CLASS(weapon_hl1_mp5, CHL1WeaponMP5);
#endif // HL1_DLL


//IMPLEMENT_SERVERCLASS_ST( CHL1WeaponMP5, DT_HL1WeaponMP5 )
//END_SEND_TABLE()

BEGIN_DATADESC(CHL1WeaponMP5)
#if defined(MAPBASE) && defined(GAME_DLL)
DEFINE_FIELD(m_flNextSoundTime, FIELD_TIME),
#endif
END_DATADESC();

CHL1WeaponMP5::CHL1WeaponMP5( )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
}

#if defined(MAPBASE) && !defined(CLIENT_DLL)
extern acttable_t* GetSMG1Acttable();
extern int GetSMG1ActtableCount();

acttable_t* CHL1WeaponMP5::ActivityList(void)
{
	return GetSMG1Acttable();
}

int CHL1WeaponMP5::ActivityListCount(void)
{
	return GetSMG1ActtableCount();
}

//-----------------------------------------------------------------------------
// Purpose: Make enough sound events to fill the estimated think interval
// returns: number of shots needed
//-----------------------------------------------------------------------------
int CHL1WeaponMP5::WeaponSoundRealtime(WeaponSound_t shoot_type)
{
	int numBullets = 0;

	// ran out of time, clamp to current
	if (m_flNextSoundTime < gpGlobals->curtime)
	{
		m_flNextSoundTime = gpGlobals->curtime;
	}

	// make enough sound events to fill up the next estimated think interval
	float dt = clamp(m_flAnimTime - m_flPrevAnimTime, 0, 0.2);
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound(SINGLE_NPC, m_flNextSoundTime);
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}
	if (m_flNextSoundTime < gpGlobals->curtime + dt)
	{
		WeaponSound(SINGLE_NPC, m_flNextSoundTime);
		m_flNextSoundTime += GetFireRate();
		numBullets++;
	}

	return numBullets;
}

float GetCurrentGravity(void);
void CHL1WeaponMP5::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2_ALTFIRE:
		{
			WeaponSound(DOUBLE_NPC);

			CAI_BaseNPC* npc = pOperator->MyNPCPointer();
			if (!npc)
				return;

			Vector vecShootOrigin;
			vecShootOrigin = pOperator->Weapon_ShootPosition();

			Vector vecTarget = npc->GetAltFireTarget();
			Vector vecThrow;
			if (vecTarget == vec3_origin || sk_hgrunt_gspeed.GetFloat() <= 0)
				AngleVectors(npc->EyeAngles(), &vecThrow); // Not much else to do, unfortunately
			else
			{
				// Because this is happening right now, we can't "VecCheckThrow" and can only "VecDoThrow", you know what I mean?
				// ...Anyway, this borrows from that so we'll never return vec3_origin.
				//vecThrow = VecCheckThrow( this, vecShootOrigin, vecTarget, 600.0, 0.5 );

				vecThrow = (vecTarget - vecShootOrigin);

				// throw at a constant time
				float time = vecThrow.Length() / sk_hgrunt_gspeed.GetFloat();
				vecThrow = vecThrow * (1.0 / time);

				// adjust upward toss to compensate for gravity loss
				vecThrow.z += (GetCurrentGravity() * 0.5) * time * 0.5;
			}

			CGrenadeMP5* pGrenade = (CGrenadeMP5*)Create("grenade_mp5", vecShootOrigin, npc->EyeAngles(), npc);
			pGrenade->SetAbsVelocity(vecThrow);
			pGrenade->SetLocalAngularVelocity(QAngle(random->RandomFloat(-100, -500), 0, 0));
			pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY);
			pGrenade->SetThrower(npc);
			pGrenade->SetDamage(sk_plr_dmg_mp5_grenade.GetFloat());

			variant_t var;
			var.SetEntity(pGrenade);
			npc->FireNamedOutput("OnThrowGrenade", var, pGrenade, npc);
		}
		break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

void CHL1WeaponMP5::NPC_PrimaryFire()
{
	CAI_BaseNPC* pAI = GetOwner()->MyNPCPointer();
	if (!pAI)
		return;

	Vector vecShootOrigin = pAI->Weapon_ShootPosition();
	Vector vecShootDir = pAI->GetActualShootTrajectory(vecShootOrigin);

	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pAI->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pAI, SOUNDENT_CHANNEL_WEAPON, pAI->GetEnemy());
	pAI->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	
	CEffectData data;
	data.m_vOrigin = vecShootOrigin;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = 1; // LookupAttachment("muzzle");
	data.m_flScale = 1.f;
	data.m_nColor = 1;
	DispatchEffect("HL1MuzzleFlash", data);

	m_iClip1 = m_iClip1 - 1;

}
const Vector& CHL1WeaponMP5::GetBulletSpread(void)
{
	static Vector cone = VECTOR_CONE_4DEGREES;
	return cone;
}
#endif // MAPBASE

void CHL1WeaponMP5::Precache( void )
{
	BaseClass::Precache();

#ifndef CLIENT_DLL
	UTIL_PrecacheOther( "grenade_mp5" );
#endif
}


void CHL1WeaponMP5::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )	
	{
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		DryFire();
		return;
	}

	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash();

	m_iClip1--;

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack	= gpGlobals->curtime + 0.1;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );	

	if ( !g_pGameRules->IsMultiplayer() )
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
//		pPlayer->FireBullets( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
		FireBulletsInfo_t info( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
		info.m_pAttacker = pPlayer;
		info.m_iTracerFreq = 2;

		pPlayer->FireBullets( info );
	}
	else
	{
		// single player spread
//		pPlayer->FireBullets( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
		FireBulletsInfo_t info( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
		info.m_pAttacker = pPlayer;
		info.m_iTracerFreq = 2;

		pPlayer->FireBullets( info );
	}

	EjectShell( pPlayer, 0 );

	pPlayer->ViewPunch( QAngle( random->RandomFloat( -2, 2 ), 0, 0 ) );
#ifdef CLIENT_DLL
	pPlayer->DoMuzzleFlash();
#else
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
#endif

#ifdef CLIENT_DLL
#else
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2 );
#endif

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + random->RandomFloat( 10, 15 ) );
}


void CHL1WeaponMP5::SecondaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

	if ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 )
	{
		DryFire( );
		return;
	}

	WeaponSound(WPN_DOUBLE);

	pPlayer->DoMuzzleFlash();

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecThrow = pPlayer->GetAutoaimVector( 0 ) * 800;
	QAngle angGrenAngle;

	VectorAngles( vecThrow, angGrenAngle );

#ifdef CLIENT_DLL
#else
	CGrenadeMP5 * m_pMyGrenade = (CGrenadeMP5*)Create( "grenade_mp5", vecSrc, angGrenAngle, GetOwner() );
	m_pMyGrenade->SetAbsVelocity( vecThrow );
	m_pMyGrenade->SetLocalAngularVelocity( QAngle( random->RandomFloat( -100, -500 ), 0, 0 ) );
	m_pMyGrenade->SetMoveType( MOVETYPE_FLYGRAVITY ); 
	m_pMyGrenade->SetThrower( GetOwner() );
	m_pMyGrenade->SetDamage( sk_plr_dmg_mp5_grenade.GetFloat() * g_pGameRules->GetDamageMultiplier() );
#endif

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( QAngle( -10, 0, 0 ) );

	// Register a muzzleflash for the AI.
#if !defined(CLIENT_DLL)
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
#endif

#ifdef CLIENT_DLL
#else
	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2 );
#endif

	// Decrease ammo
	pPlayer->RemoveAmmo( 1, m_iSecondaryAmmoType );
	if ( pPlayer->GetAmmoCount( m_iSecondaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.0;
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0;
	SetWeaponIdleTime( gpGlobals->curtime + 5.0 );
}


void CHL1WeaponMP5::DryFire( void )
{
	WeaponSound( EMPTY );
	m_flNextPrimaryAttack	= gpGlobals->curtime + 0.15;
	m_flNextSecondaryAttack	= gpGlobals->curtime + 0.15;
}


void CHL1WeaponMP5::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer )
	{
		pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	}

	bool bElapsed = HasWeaponIdleTimeElapsed();

	BaseClass::WeaponIdle();
	
	if( bElapsed )
		SetWeaponIdleTime( gpGlobals->curtime + random->RandomInt( 3, 5 ) );
}
