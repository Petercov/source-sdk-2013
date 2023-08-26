#include "cbase.h"
#include "weapon_minigun_base.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "in_buttons.h"

const char* MiniSoundContext = "NPCSoundThink";

BEGIN_DATADESC(CBaseHLMinigun)

DEFINE_FIELD(m_State, FIELD_CHARACTER),

DEFINE_THINKFUNC(NPC_FinishSpinup),
DEFINE_THINKFUNC(NPC_SpinDown),

END_DATADESC();

void CBaseHLMinigun::Activate()
{
	BaseClass::Activate();

	if (GetOwner())
	{
		switch (m_State)
		{
		case MINI_IDLE:
		default:
			break;
		case MINI_SPINUP:
			WeaponSoundStatic(SPECIAL1, GetNextThink() - GetSpinupTime());
			break;
		case MINI_READY:
			WeaponSoundStatic(SPECIAL1, gpGlobals->curtime - GetSpinupTime());
			break;
		}
	}
}

void CBaseHLMinigun::StopLoopingSounds(void)
{
	const char* pszSpinSound = GetShootSound(SPECIAL1);
	if (pszSpinSound && pszSpinSound[0])
		StopSound(pszSpinSound);
}

void CBaseHLMinigun::ItemPostFrame(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Debounce the recoiling counter
	if ((pOwner->m_nButtons & IN_ATTACK) == false)
	{
		m_nShotsFired = 0;
	}

	UpdateAutoFire();

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	if (UsesClipsForAmmo1())
	{
		CheckReload();
	}

	bool bFired = false;

	// Secondary attack has priority
	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
#ifdef MAPBASE
		if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
		{
			// Don't do anything, just cancel the whole function
			return;
		}
		else
#endif
			if (UsesSecondaryAmmo() && pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
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
				// FIXME: This isn't necessarily true if the weapon doesn't have a secondary fire!
				// For instance, the crossbow doesn't have a 'real' secondary fire, but it still 
				// stops the crossbow from firing on the 360 if the player chooses to hold down their
				// zoom button. (sjb) Orange Box 7/25/2007
#if !defined(CLIENT_DLL)
				if (!IsX360() || !ClassMatches("weapon_crossbow"))
#endif
				{
					bFired = ShouldBlockPrimaryFire();
				}

				SecondaryAttack();

				// Secondary ammo doesn't have a reload animation
				if (UsesClipsForAmmo2())
				{
					// reload clip2 if empty
					if (m_iClip2 < 1)
					{
						pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);
						m_iClip2 = m_iClip2 + 1;
					}
				}
			}
	}

	if (!bFired && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
#ifdef MAPBASE
		if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
		{
			// Don't do anything, just cancel the whole function
			return;
		}
		else
#endif
			// Clip empty? Or out of ammo on a no-clip weapon?
			if (!IsMeleeWeapon() &&
				((UsesClipsForAmmo1() && m_iClip1 <= 0) || (!UsesClipsForAmmo1() && pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)))
			{
				HandleFireOnEmpty();
			}
			else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
			{
				// This weapon doesn't fire underwater
				WeaponSound(EMPTY);
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
				return;
			}
			else
			{
				//NOTENOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
				//			on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
				//			However, because the player can also be doing a secondary attack, the edge trigger may be missed.
				//			We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
				//			first shot.  Right now that's too much of an architecture change -- jdw

				// If the firing button was just pressed, or the alt-fire just released, reset the firing time
				if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
				{
					m_flNextPrimaryAttack = gpGlobals->curtime;
				}

				PrimaryAttack();

				if (AutoFiresFullClip())
				{
					m_bFiringWholeClip = true;
				}

#ifdef CLIENT_DLL
				pOwner->SetFiredWeapon(true);
#endif
			}
	}

	// -----------------------
	//  Reload pressed / Clip Empty
	// -----------------------
	if ((pOwner->m_nButtons & IN_RELOAD) && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
		m_fFireDuration = 0.0f;
	}

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (CanReload() && pOwner->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}
}

void CBaseHLMinigun::Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator)
{
	if ((pEvent->type & AE_TYPE_NEWEVENTSYSTEM) == 0)
	{
		switch (pEvent->event)
		{
		case EVENT_WEAPON_SMG1:
		case EVENT_WEAPON_HMG1:
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
		return;
		default:
			break;
		}
	}

	BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
}

void CBaseHLMinigun::Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;
	m_State = MINI_READY;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

void CBaseHLMinigun::FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir)
{
	int nShots = WeaponSoundRealtime(SINGLE_NPC);
	if (UsesClipsForAmmo1())
		nShots = Min(nShots, m_iClip1.Get());

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(nShots, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();

	SetContextThink(&ThisClass::NPC_SpinDown, gpGlobals->curtime + Max(0.1f, GetFireRate()) + 0.05f, MiniSoundContext);

	if (UsesClipsForAmmo1())
	{
		m_iClip1 = m_iClip1 - nShots;
		if (m_iClip1 <= 0)
		{
			SetNextThink(gpGlobals->curtime, MiniSoundContext);
		}
	}
}

void CBaseHLMinigun::SetActivity(Activity act, float duration)
{
	if (GetOwnerNPC() && GetSpinupTime() > 0.f && act == ActivityOverride(ACT_RANGE_ATTACK1, 0))
	{
		switch (m_State)
		{
		default:
		case MINI_IDLE:
		{
			CAI_ShotRegulator* pRegulator = GetOwnerNPC()->GetShotRegulator();
			if (!pRegulator)
				break;

			float flShotTime = gpGlobals->curtime + GetSpinupTime();
			pRegulator->NoteWeaponUnfired();
			pRegulator->FireNoEarlierThan(flShotTime);
			SetContextThink(&ThisClass::NPC_FinishSpinup, flShotTime - gpGlobals->interval_per_tick, MiniSoundContext);
			WeaponSoundStatic(SPECIAL1);
			m_State = MINI_SPINUP;
		}
		case MINI_SPINUP:
			return;
		case MINI_READY:
			break;
		}
	}

	BaseClass::SetActivity(act, duration);
}

void CBaseHLMinigun::WeaponSoundStatic(WeaponSound_t sound_type, float soundtime)
{
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char* shootsound = GetShootSound(sound_type);
	if (!shootsound || !shootsound[0])
		return;

	CSoundParameters params;

	if (!GetParametersForSound(shootsound, params, NULL))
		return;

	if (params.play_to_owner_only)
	{
		// Am I only to play to my owner?
		if (GetOwner() && GetOwner()->IsPlayer())
		{
			CSingleUserRecipientFilter filter(ToBasePlayer(GetOwner()));
			if (IsPredicted() && CBaseEntity::GetPredictionPlayer())
			{
				filter.UsePredictionRules();
			}
			EmitSound(filter, entindex(), shootsound, NULL, soundtime);
		}
	}
	else
	{
		CPASAttenuationFilter filter(GetOwner() ? GetOwnerEntity() : this, params.soundlevel);
		if (IsPredicted() && CBaseEntity::GetPredictionPlayer())
		{
			filter.UsePredictionRules();
		}
		EmitSound(filter, entindex(), shootsound, NULL, soundtime);
	}
}

void CBaseHLMinigun::NPC_FinishSpinup()
{
	m_State = MINI_READY;

	SetContextThink(&ThisClass::NPC_SpinDown, gpGlobals->curtime + Max(0.1f, GetFireRate()) + 0.05f, MiniSoundContext);
}

void CBaseHLMinigun::NPC_SpinDown()
{
	if (GetOwnerNPC())
	{
		if ((GetOwnerNPC()->HasCondition(COND_SEE_ENEMY) ||
			GetOwnerNPC()->HasCondition(COND_SEE_HATE) ||
			GetOwnerNPC()->HasCondition(COND_SEE_DISLIKE) ||
			//GetOwnerNPC()->HasCondition(COND_SEE_FEAR) ||
			GetOwnerNPC()->HasCondition(COND_SEE_NEMESIS)
			) && (m_iClip1 > 0 || !UsesClipsForAmmo1()))
		{
			SetNextThink(gpGlobals->curtime + 0.1f, MiniSoundContext);
			return;
		}
	}

	const char* pszSpinSound = GetShootSound(SPECIAL1);
	if (pszSpinSound && pszSpinSound[0])
		StopSound(pszSpinSound);
	
	WeaponSoundStatic(SPECIAL2);
	if (UsesClipsForAmmo1() && m_iClip1 <= 0)
		WeaponSoundStatic(SPECIAL3);
	
	m_State = MINI_IDLE;
	SetContextThink(NULL, 0, MiniSoundContext);
}

acttable_t	CBaseHLMinigun::m_acttable[] = {
	{ ACT_RANGE_ATTACK1,			ACT_DOD_PRIMARYATTACK_DEPLOYED_MG,			true },
	{ ACT_RELOAD,					ACT_DOD_RELOAD_FG42,				true },
	{ ACT_IDLE,						ACT_DOD_STAND_IDLE,					true },
	{ ACT_IDLE_ANGRY,				ACT_DOD_STAND_AIM_MG,			true },

	{ ACT_WALK,						ACT_DOD_WALK_IDLE_MG,					true },
	{ ACT_WALK_AIM,					ACT_DOD_WALK_AIM_MG,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_DOD_RUN_IDLE_MG,					true },
	{ ACT_RUN_AIM,					ACT_DOD_RUN_AIM_MG,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_DOD_PRIMARYATTACK_MG,		true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_DOD_PRIMARYATTACK_PRONE_MG,		true },
	{ ACT_COVER_LOW,				ACT_DOD_CROUCH_AIM_MG,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_DOD_CROUCH_AIM_MG,			false },
	{ ACT_RELOAD_LOW,				ACT_DOD_RELOAD_PRONE_DEPLOYED_MG34,				false },
	{ ACT_GESTURE_RELOAD,			ACT_DOD_RELOAD_DEPLOYED_MG,		true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_DOD_STAND_IDLE,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_DOD_STAND_IDLE,		false },
	{ ACT_IDLE_AGITATED,			ACT_DOD_STAND_AIM_MG,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_DOD_WALK_IDLE_MG,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_DOD_WALK_IDLE_MG,		false },
	{ ACT_WALK_AGITATED,			ACT_DOD_WALK_AIM_MG,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_DOD_RUN_IDLE_MG,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_DOD_RUN_IDLE_MG,		false },
	{ ACT_RUN_AGITATED,				ACT_DOD_RUN_AIM_MG,				false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_DOD_STAND_IDLE,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_DOD_STAND_AIM_MG,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_DOD_STAND_AIM_MG,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_DOD_WALK_IDLE_MG,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_DOD_WALK_AIM_MG,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_DOD_WALK_AIM_MG,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_DOD_RUN_IDLE_MG,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_DOD_RUN_AIM_MG,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_DOD_RUN_AIM_MG,				false },//always aims
	//End readiness activities

#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_ARM,						ACT_ARM_RIFLE,					false },
	{ ACT_DISARM,					ACT_DISARM_RIFLE,				false },
#endif

#if EXPANDED_HL2_COVER_ACTIVITIES
	{ ACT_RANGE_AIM_MED,			ACT_DOD_CROUCH_AIM_MG,			false },
	{ ACT_RANGE_ATTACK1_MED,		ACT_DOD_PRIMARYATTACK_PRONE_MG,		false },

	{ ACT_COVER_WALL_R,				ACT_COVER_WALL_R_RIFLE,			false },
	{ ACT_COVER_WALL_L,				ACT_COVER_WALL_L_RIFLE,			false },
	{ ACT_COVER_WALL_LOW_R,			ACT_COVER_WALL_LOW_R_RIFLE,		false },
	{ ACT_COVER_WALL_LOW_L,			ACT_COVER_WALL_LOW_L_RIFLE,		false },
#endif

#ifdef MAPBASE
	// HL2:DM activities (for third-person animations in SP)
	{ ACT_HL2MP_IDLE,                    ACT_HL2MP_IDLE_SMG1,                    false },
	{ ACT_HL2MP_RUN,                    ACT_HL2MP_RUN_SMG1,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,            ACT_HL2MP_IDLE_CROUCH_SMG1,            false },
	{ ACT_HL2MP_WALK_CROUCH,            ACT_HL2MP_WALK_CROUCH_SMG1,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,    ACT_HL2MP_GESTURE_RANGE_ATTACK_SMG1,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,            ACT_HL2MP_GESTURE_RELOAD_SMG1,        false },
	{ ACT_HL2MP_JUMP,                    ACT_HL2MP_JUMP_SMG1,                    false },
#if EXPANDED_HL2DM_ACTIVITIES
	{ ACT_HL2MP_WALK,					ACT_HL2MP_WALK_SMG1,					false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK2,	ACT_HL2MP_GESTURE_RANGE_ATTACK2_SMG1,	false },
#endif
#endif
};

IMPLEMENT_ACTTABLE(CBaseHLMinigun);