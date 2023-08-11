#include "cbase.h"
#include "weapon_sin_base.h"
#include "npcevent.h"
#include "ai_basenpc.h"

extern acttable_t* GetPistolActtable();
extern int GetPistolActtableCount();

class CSinWeaponMagnum : public CSinWeaponBase
{
public:
	DECLARE_CLASS(CSinWeaponMagnum, CSinWeaponBase);
	DECLARE_SERVERCLASS();

	virtual	acttable_t* ActivityList(void) { return GetPistolActtable(); }
	virtual	int			ActivityListCount(void) { return GetPistolActtableCount(); }

	virtual int	GetMinBurst()
	{
		return 1;
	}

	virtual int	GetMaxBurst()
	{
		return 3;
	}

	virtual float GetFireRate(void) { return RPM_TO_RATE(300); }
	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}

	virtual void	Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
};

LINK_ENTITY_TO_CLASS(weapon_magnum, CSinWeaponMagnum);

IMPLEMENT_SERVERCLASS_ST(CSinWeaponMagnum, DT_SinWeaponMagnum)
END_SEND_TABLE();

void CSinWeaponMagnum::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_PISTOL_FIRE:
	{
#ifdef MAPBASE
		// HACKHACK: Ignore the regular firing event while dual-wielding
		if (GetLeftHandGun())
			return;
#endif

		Vector vecShootOrigin, vecShootDir;
		vecShootOrigin = pOperator->Weapon_ShootPosition();

		CAI_BaseNPC* npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);

		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
	}
	break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}