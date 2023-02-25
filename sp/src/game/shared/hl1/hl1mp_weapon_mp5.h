//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the MP5 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	HL1WeaponMP5_H
#define	HL1WeaponMP5_H

#ifdef CLIENT_DLL
#else
#include "hl1_basegrenade.h"
#endif
#include "hl1mp_basecombatweapon_shared.h"

#ifdef CLIENT_DLL
class CGrenadeMP5;
#else
#endif

#ifdef CLIENT_DLL
#define CHL1WeaponMP5 C_HL1WeaponMP5
#endif

class CHL1WeaponMP5 : public CBaseHL1MPCombatWeapon
{
	DECLARE_CLASS( CHL1WeaponMP5, CBaseHL1MPCombatWeapon );
public:

	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CHL1WeaponMP5();

	void	Precache( void );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DryFire( void );
	void	WeaponIdle( void );

#ifdef MAPBASE
	virtual int				CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual	acttable_t*		ActivityList(void);
	virtual	int				ActivityListCount(void);
	virtual void			NPC_PrimaryFire();

	virtual float			GetFireRate(void) { return 0.1f; }
	virtual int				GetMinBurst() { return 3; }
	virtual int				GetMaxBurst() { return 3; }
	virtual float			GetMinRestTime() { return 0.1; }
	virtual float			GetMaxRestTime() { return 0.15; }

	virtual const Vector& GetBulletSpread(void);

	int				WeaponSoundRealtime(WeaponSound_t shoot_type);

protected:
	float m_flNextSoundTime;
public:
#endif // MAPBASE


//	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};


#endif	//HL1WeaponMP5_H
