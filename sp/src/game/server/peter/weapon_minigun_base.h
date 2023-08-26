#ifndef MINIGUN_BASE_H
#define MINIGUN_BASE_H
#pragma once

#include "basehlcombatweapon.h"

class CAI_BaseNPC;

typedef enum MinigunState_e : char
{
	MINI_IDLE = 0,
	MINI_SPINUP,
	MINI_READY,
} MinigunState_t;

class CBaseHLMinigun : public CHLMachineGun
{
	DECLARE_CLASS(CBaseHLMinigun, CHLMachineGun);
	DECLARE_DATADESC();
	DECLARE_ACTTABLE();
public:
	virtual void	Activate();
	virtual void	StopLoopingSounds(void);

	virtual void	ItemPostFrame(void);

	virtual void	Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
	virtual void	Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary);
	virtual void	FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);

	virtual void	SetActivity(Activity act) { return BaseClass::SetActivity(act); }
	virtual void	SetActivity(Activity act, float duration);

	virtual float	GetSpinupTime() { return 1.f; }

	CAI_BaseNPC* GetOwnerNPC() { return GetOwner() ? GetOwner()->MyNPCPointer() : nullptr; }

	WeaponClass_t			WeaponClassify() { return WEPCLASS_HEAVY; }
protected:
	void	WeaponSoundStatic(WeaponSound_t sound_type, float soundtime = 0.0f);

	void	NPC_FinishSpinup();
	void	NPC_SpinDown();

	MinigunState_t m_State;
};

#endif