//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Glock - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "hl1mp_basecombatweapon_shared.h"
//#include "basecombatcharacter.h"
//#include "AI_BaseNPC.h"
#ifdef CLIENT_DLL
#include "c_baseplayer.h"
#else
#include "player.h"
#endif
#include "gamerules.h"
#include "in_buttons.h"
#ifdef CLIENT_DLL
#else
#include "soundent.h"
#include "game.h"
#ifdef MAPBASE
#include "te_effect_dispatch.h"
#include "ai_basenpc.h"
#endif // MAPBASE
#endif
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"

#ifdef CLIENT_DLL
#define CHL1WeaponGlock C_HL1WeaponGlock
#endif

class CHL1WeaponGlock : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS( CHL1WeaponGlock, CBaseHL1MPCombatWeapon );

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

public:

	CHL1WeaponGlock(void);

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	bool	Reload( void );
	void	WeaponIdle( void );
	void	DryFire( void );

#if defined(MAPBASE) && !defined(CLIENT_DLL)
	virtual int				CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual	acttable_t*		ActivityList(void);
	virtual	int				ActivityListCount(void);
	virtual void			NPC_PrimaryFire();

	virtual float			GetFireRate(void) { return 0.3f; }
	//virtual int				GetMinBurst() { return 3; }
	//virtual int				GetMaxBurst() { return 3; }
	//virtual float			GetMinRestTime() { return 0.15; }
	//virtual float			GetMaxRestTime() { return 0.2; }

	virtual const Vector&	GetBulletSpread(void);
#endif

private:
	void	GlockFire( float flSpread , float flCycleTime, bool fUseAutoAim );
};

IMPLEMENT_NETWORKCLASS_ALIASED( HL1WeaponGlock, DT_HL1WeaponGlock );

BEGIN_NETWORK_TABLE(CHL1WeaponGlock, DT_HL1WeaponGlock)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CHL1WeaponGlock)
END_PREDICTION_DATA()

#ifdef HL1_DLL
LINK_ENTITY_TO_CLASS(weapon_glock, CHL1WeaponGlock);

PRECACHE_WEAPON_REGISTER(weapon_glock);
#else
LINK_ENTITY_TO_CLASS(weapon_hl1_glock, CHL1WeaponGlock);
#endif // HL1_DLL


CHL1WeaponGlock::CHL1WeaponGlock( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= true;
}


void CHL1WeaponGlock::Precache( void )
{

	BaseClass::Precache();
}


void CHL1WeaponGlock::DryFire( void )
{
	WeaponSound( EMPTY );
	SendWeaponAnim( ACT_VM_DRYFIRE );
		
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
}


void CHL1WeaponGlock::PrimaryAttack( void )
{
	GlockFire( 0.01, 0.3, TRUE );
}


void CHL1WeaponGlock::SecondaryAttack( void )
{
	GlockFire( 0.1, 0.2, FALSE );
}


void CHL1WeaponGlock::GlockFire( float flSpread , float flCycleTime, bool fUseAutoAim )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			DryFire();
		}

		return;
	}

	WeaponSound( SINGLE );

	pPlayer->DoMuzzleFlash();

	m_iClip1--;

	if ( m_iClip1 == 0 )
		SendWeaponAnim( ACT_GLOCK_SHOOTEMPTY );
	else
		SendWeaponAnim( ACT_VM_PRIMARYATTACK );

	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack	= gpGlobals->curtime + flCycleTime;
	m_flNextSecondaryAttack	= gpGlobals->curtime + flCycleTime;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming;
	
	if ( fUseAutoAim )
	{
		vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );	
	}
	else
	{
		vecAiming = pPlayer->GetAutoaimVector( 0 );	
	}

//	pPlayer->FireBullets( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0 );
	FireBulletsInfo_t info( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), MAX_TRACE_LENGTH, m_iPrimaryAmmoType );
	info.m_pAttacker = pPlayer;
	pPlayer->FireBullets( info );

	EjectShell( pPlayer, 0 );

	pPlayer->ViewPunch( QAngle( -2, 0, 0 ) );
#if !defined(CLIENT_DLL)
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 400, 0.2 );
#endif

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	SetWeaponIdleTime( gpGlobals->curtime + random->RandomFloat( 10, 15 ) );
}


bool CHL1WeaponGlock::Reload( void )
{
	bool iResult;

	if ( m_iClip1 == 0 )
		iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_GLOCK_SHOOT_RELOAD );
	else
		iResult = DefaultReload( GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD );

	return iResult;
}



void CHL1WeaponGlock::WeaponIdle( void )
{
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if ( pPlayer )
	{
		pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	}

	// only idle if the slid isn't back
	if ( m_iClip1 != 0 )
	{
		BaseClass::WeaponIdle();
	}
}

#if defined(MAPBASE) && !defined(CLIENT_DLL)
extern acttable_t* GetPistolActtable();
extern int GetPistolActtableCount();

acttable_t* CHL1WeaponGlock::ActivityList(void)
{
	return GetPistolActtable();
}

int CHL1WeaponGlock::ActivityListCount(void)
{
	return GetPistolActtableCount();
}

void CHL1WeaponGlock::NPC_PrimaryFire()
{
	if (m_iClip1 <= 0)
	{
		WeaponSound(EMPTY);
		return;
	}

	CAI_BaseNPC* pAI = GetOwner()->MyNPCPointer();
	if (!pAI)
		return;

	Vector vecShootOrigin = pAI->Weapon_ShootPosition();
	Vector vecShootDir = pAI->GetActualShootTrajectory(vecShootOrigin);

	WeaponSound(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pAI->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pAI, SOUNDENT_CHANNEL_WEAPON, pAI->GetEnemy());
	pAI->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	CEffectData data;
	data.m_vOrigin = vecShootOrigin;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = 1; // LookupAttachment("muzzle");
	data.m_flScale = 1.f;
	data.m_nColor = 0;
	DispatchEffect("HL1MuzzleFlash", data);

	m_iClip1 = m_iClip1 - 1;

}
const Vector& CHL1WeaponGlock::GetBulletSpread(void)
{
	static Vector cone = VECTOR_CONE_2DEGREES;
	return cone;
}
#endif // MAPBASE