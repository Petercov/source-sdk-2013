#include "cbase.h"
#include "weapon_sin_base.h"
#include "ai_basenpc.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "gamestats.h"

acttable_t* GetSMG1Acttable();
int GetSMG1ActtableCount();

class CSinWeaponAR : public CSinWeaponBase
{
public:
	DECLARE_CLASS(CSinWeaponAR, CSinWeaponBase);
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CSinWeaponAR();

	virtual	acttable_t* ActivityList(void) { return GetSMG1Acttable(); }
	virtual	int			ActivityListCount(void) { return GetSMG1ActtableCount(); }

	virtual void	ItemPostFrame(void);
	virtual void	PrimaryAttack(void);
	virtual bool	Deploy(void);
	void	AddViewKick(void);

	int		GetMinBurst(void) { return 3; }
	int		GetMaxBurst(void) { return 8; }
	float	GetFireRate(void) { return 0.1f; }

	int				WeaponSoundRealtime(WeaponSound_t shoot_type);
	virtual void	FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir);
	void Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);

protected:

	int	m_nShotsFired;	// Number of consecutive shots fired

	float	m_flNextSoundTime;	// real-time clock of when to make next sound
};

LINK_ENTITY_TO_CLASS(weapon_assault_rifle, CSinWeaponAR);

IMPLEMENT_SERVERCLASS_ST(CSinWeaponAR, DT_SinWeaponAR)
END_SEND_TABLE();

BEGIN_DATADESC(CSinWeaponAR)

DEFINE_FIELD(m_nShotsFired, FIELD_INTEGER),
DEFINE_FIELD(m_flNextSoundTime, FIELD_TIME),

END_DATADESC();

CSinWeaponAR::CSinWeaponAR()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1400;

	m_bFiresUnderwater = false;
	m_bAltFiresUnderwater = false;
}

void CSinWeaponAR::ItemPostFrame(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Debounce the recoiling counter
	if ((pOwner->m_nButtons & IN_ATTACK) == false)
	{
		m_nShotsFired = 0;
	}

	BaseClass::ItemPostFrame();
}

void CSinWeaponAR::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	// Abort here to handle burst and auto fire modes
	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	m_nShotsFired++;

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();

	// MUST call sound before removing a round from the clip of a CHLMachineGun
	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		if (iBulletsToFire > m_iClip1)
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();
	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	FireBullets(info);

	//Factor in the view kick
	AddViewKick();

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer);

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	SendWeaponAnim(GetPrimaryAttackActivity());
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
}

bool CSinWeaponAR::Deploy(void)
{
	m_nShotsFired = 0;

	return BaseClass::Deploy();
}

void CSinWeaponAR::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	1.0f	//Degrees
#define	SLIDE_LIMIT			2.0f	//Seconds

	//Get the view kick
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	CHLMachineGun::DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

int CSinWeaponAR::WeaponSoundRealtime(WeaponSound_t shoot_type)
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

void CSinWeaponAR::FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir)
{
	int nShots = WeaponSoundRealtime(SINGLE_NPC);
	if (UsesClipsForAmmo1())
		nShots = Min(nShots, m_iClip1.Get());

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(nShots, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();

	if (UsesClipsForAmmo1())
		m_iClip1 = m_iClip1 - nShots;
}

void CSinWeaponAR::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
		{
			vecShootOrigin = pOperator->Weapon_ShootPosition();
		}

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
