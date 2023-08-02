#include "cbase.h"
#include "weapon_sin_base.h"
#include "ai_basenpc.h"
#include "gamestats.h"
#include "npcevent.h"

extern ConVar sk_plr_num_shotgun_pellets;
extern ConVar sk_npc_num_shotgun_pellets;

extern acttable_t* GetShotgunActtable();
extern int GetShotgunActtableCount();

class CSinWeaponShotgun : public CSinWeaponBase
{
public:
	DECLARE_CLASS(CSinWeaponShotgun, CSinWeaponBase);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CSinWeaponShotgun();

	virtual	acttable_t* ActivityList(void) { return GetShotgunActtable(); }
	virtual	int			ActivityListCount(void) { return GetShotgunActtableCount(); }

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector vitalAllyCone = VECTOR_CONE_3DEGREES;
		static Vector cone = VECTOR_CONE_10DEGREES;

		if (GetOwner() && (GetOwner()->Classify() == CLASS_PLAYER_ALLY_VITAL))
		{
			// Give Alyx's shotgun blasts more a more directed punch. She needs
			// to be at least as deadly as she would be with her pistol to stay interesting (sjb)
			return vitalAllyCone;
		}

		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 3; }

	virtual float			GetMinRestTime() { return 1.2f; }
	virtual float			GetMaxRestTime() { return 1.5f; }

	virtual float			GetFireRate(void) { return 0.8f; }

	virtual void	FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);
	void Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);

	void ItemPostFrame(void);
	void PrimaryAttack(void);
	void Pump(void);

protected:
	bool	m_bNeedPump;		// When emptied completely
};

LINK_ENTITY_TO_CLASS(weapon_scattergun, CSinWeaponShotgun);

BEGIN_DATADESC(CSinWeaponShotgun)
DEFINE_FIELD(m_bNeedPump, FIELD_BOOLEAN),
END_DATADESC();

IMPLEMENT_SERVERCLASS_ST(CSinWeaponShotgun, DT_SinWeaponShotgun)
END_SEND_TABLE();

CSinWeaponShotgun::CSinWeaponShotgun()
{
	m_bNeedPump = false;
}

void CSinWeaponShotgun::FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir)
{
	//CAI_BaseNPC* npc = pOperator->MyNPCPointer();
	Assert(pOperator);
	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

#ifdef MAPBASE
	pOperator->FireBullets(sk_npc_num_shotgun_pellets.GetInt(), vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);
#else
	pOperator->FireBullets(8, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);
#endif
}

void CSinWeaponShotgun::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
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

void CSinWeaponShotgun::ItemPostFrame(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	if ((m_bNeedPump) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		Pump();
		return;
	}

	BaseClass::ItemPostFrame();
}

void CSinWeaponShotgun::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	// MUST call sound before removing a round from the clip of a CMachineGun
	WeaponSound(SINGLE);

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);

	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Don't fire again until fire animation has completed
#ifdef MAPBASE
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
#else
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
#endif
	m_iClip1 -= 1;

	Vector	vecSrc = pPlayer->Weapon_ShootPosition();
	Vector	vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 1.0);

	// Fire the bullets, and force the first shot to be perfectly accuracy
	pPlayer->FireBullets(sk_plr_num_shotgun_pellets.GetInt(), vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0, -1, -1, 0, NULL, true, true);

#ifdef EZ
	pPlayer->ViewPunch(QAngle(random->RandomFloat(-4, -2), random->RandomFloat(-4, 4), 0)); // Breadman - values doubled
#else
	pPlayer->ViewPunch(QAngle(random->RandomFloat(-2, -1), random->RandomFloat(-2, 2), 0));
#endif

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner());

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	if (m_iClip1)
	{
		// pump so long as some rounds are left.
		m_bNeedPump = true;
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
}

void CSinWeaponShotgun::Pump(void)
{
	CBaseCombatCharacter* pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	m_bNeedPump = false;

	WeaponSound(SPECIAL1);

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_PUMP);

	pOwner->m_flNextAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
}
