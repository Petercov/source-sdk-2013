#ifndef WEAPON_SIN_BASE_H
#define WEAPON_SIN_BASE_H
#pragma once  

#include "basehlcombatweapon.h"

// Weapon base with melee and grenade attacks
class CSinWeaponBase : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CSinWeaponBase, CBaseHLCombatWeapon);
	DECLARE_DATADESC();

	CSinWeaponBase();

	virtual void	Spawn();
	virtual void	ItemPostFrame(void);
	virtual bool	CanHolster(void);
	virtual void	FireBullets(const FireBulletsInfo_t& info);

	bool	Deploy(void);
	bool	Holster(CBaseCombatWeapon* pSwitchingTo = NULL);
	bool	Reload(void);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	virtual void	FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);
	virtual void	Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary);

protected:
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition(CBasePlayer* pPlayer, const Vector& vecEye, Vector& vecSrc);
	virtual void	ThrowGrenade(CBasePlayer* pPlayer);

	//virtual	float	GetMeleeRate(void) { return	0.2f; }
	virtual float	GetMeleeHitDelay() { return 0.f; }
	virtual float	GetMeleeRange(void) { return 70.0f; }
	virtual	float	GetMeleeDamageForActivity(Activity hitActivity);

	virtual void	AddMeleeViewKick();
	virtual	void	MeleeImpactEffect(trace_t& trace);
	bool			ImpactWater(const Vector& start, const Vector& end);
	void			Swing();
	void			Hit(trace_t& traceHit, Activity nHitActivity);
	void			DelayedHit();
	Activity		ChooseIntersectionPointAndActivity(trace_t& hitTrace, const Vector& mins, const Vector& maxs, CBasePlayer* pOwner);

	float	m_flDelayedHit;
	bool	m_bHitDelayed;

	float	m_fDrawbackFinishedAt;
	bool	m_bDrawbackGrenade;

	int		m_iGrenadeAmmoType;
};

#endif // !WEAPON_SIN_BASE_H

