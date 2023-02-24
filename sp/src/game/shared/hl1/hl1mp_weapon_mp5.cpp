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

BEGIN_DATADESC( CHL1WeaponMP5 )
END_DATADESC()

CHL1WeaponMP5::CHL1WeaponMP5( )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
}

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
