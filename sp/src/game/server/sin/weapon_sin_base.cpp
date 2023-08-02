#include "cbase.h"
#include "weapon_sin_base.h"
#include "in_buttons.h"
#include "player.h"
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "soundent.h"
#include "grenade_frag.h"
#include "npcevent.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_TIMER	3.0f //Seconds

#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f // inches

#define BLUDGEON_HULL_DIM		16

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

ConVar sk_sin_plr_dmg_melee("sk_sin_plr_dmg_melee", "");

BEGIN_DATADESC(CSinWeaponBase)
DEFINE_FIELD(m_fDrawbackFinishedAt, FIELD_TIME),
DEFINE_FIELD(m_flDelayedHit, FIELD_TIME),
DEFINE_FIELD(m_bDrawbackGrenade, FIELD_BOOLEAN),
DEFINE_FIELD(m_bHitDelayed, FIELD_BOOLEAN),
DEFINE_FIELD(m_iGrenadeAmmoType, FIELD_INTEGER),
END_DATADESC();

CSinWeaponBase::CSinWeaponBase()
{
	m_bDrawbackGrenade = false;
	m_fDrawbackFinishedAt = 0.f;

	m_bHitDelayed = false;
	m_flDelayedHit = 0.f;

	m_iGrenadeAmmoType = -1;
}

void CSinWeaponBase::Spawn()
{
	BaseClass::Spawn();

	m_iGrenadeAmmoType = GetAmmoDef()->Index("grenade");
}

void CSinWeaponBase::ItemPostFrame(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	if (m_bHitDelayed)
	{
		if (gpGlobals->curtime >= m_flDelayedHit)
			DelayedHit();

		return;
	}

	if (m_bDrawbackGrenade)
	{
		if (gpGlobals->curtime >= m_fDrawbackFinishedAt)
		{
			if (!(pOwner->m_nButtons & IN_GRENADE1))
			{
				SendWeaponAnim(ACT_VM_THROW);
				ThrowGrenade(pOwner);

				pOwner->RemoveAmmo(1, m_iGrenadeAmmoType);

				//Update our times
				m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
				m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();
				m_flTimeWeaponIdle = gpGlobals->curtime + GetViewModelSequenceDuration();

				m_bDrawbackGrenade = false;
			}
		}
		return;
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
			//if (!IsX360() || !ClassMatches("weapon_crossbow"))
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

	if (!bFired && (pOwner->m_nButtons & IN_GRENADE1) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
#ifdef MAPBASE
		if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
		{
			// Don't do anything, just cancel the whole function
			return;
		}
		else
#endif
		if (m_iGrenadeAmmoType >= 0 && pOwner->GetAmmoCount(m_iGrenadeAmmoType) > 0)
		{
			// FIXME: This isn't necessarily true if the weapon doesn't have a secondary fire!
			// For instance, the crossbow doesn't have a 'real' secondary fire, but it still 
			// stops the crossbow from firing on the 360 if the player chooses to hold down their
			// zoom button. (sjb) Orange Box 7/25/2007
#if !defined(CLIENT_DLL)
		//if (!IsX360() || !ClassMatches("weapon_crossbow"))
#endif
			{
				bFired = ShouldBlockPrimaryFire();
			}

			m_bDrawbackGrenade = true;
			SendWeaponAnim(ACT_VM_PULLBACK);
			m_fDrawbackFinishedAt = gpGlobals->curtime + GetViewModelSequenceDuration();

			// Put both of these off indefinitely. We do not know how long
			// the player will hold the grenade.
			m_flTimeWeaponIdle = FLT_MAX;
			m_flNextPrimaryAttack = FLT_MAX;
			m_flNextSecondaryAttack = FLT_MAX;
		}
	}

	if (!bFired && (pOwner->m_nButtons & IN_ATTACK3) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
#ifdef MAPBASE
		if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
		{
			// Don't do anything, just cancel the whole function
			return;
		}
		else
#endif
		{
			bFired = ShouldBlockPrimaryFire();

			Swing();
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
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2) || (pOwner->m_nButtons & IN_ATTACK3) || (CanReload() && pOwner->m_nButtons & IN_RELOAD)))
	{
		// no fire buttons down or reloading
		if (!ReloadOrSwitchWeapons() && (m_bInReload == false))
		{
			WeaponIdle();
		}
	}
}

bool CSinWeaponBase::CanHolster(void)
{
	if (m_bHitDelayed)
		return false;

	return BaseClass::CanHolster();
}

void CSinWeaponBase::FireBullets(const FireBulletsInfo_t& info)
{
	FireBulletsInfo_t infoCopy(info);
	infoCopy.m_pAttacker = GetOwner();
	BaseClass::FireBullets(infoCopy);
}

bool CSinWeaponBase::Deploy(void)
{
	m_bDrawbackGrenade = false;

	return BaseClass::Deploy();
}

bool CSinWeaponBase::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	m_bDrawbackGrenade = false;

	return BaseClass::Holster(pSwitchingTo);
}

bool CSinWeaponBase::Reload(void)
{
	bool fRet;

	fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		WeaponSound(RELOAD);
	}

	return fRet;
}

void CSinWeaponBase::FireNPCPrimaryAttack(CBaseCombatCharacter* pOperator, Vector& vecShootOrigin, Vector& vecShootDir)
{
	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_PISTOL, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	WeaponSound(SINGLE_NPC);
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

void CSinWeaponBase::Operator_ForceNPCFire(CBaseCombatCharacter* pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

void CSinWeaponBase::CheckThrowPosition(CBasePlayer* pPlayer, const Vector& vecEye, Vector& vecSrc)
{
	trace_t tr;

	UTIL_TraceHull(vecEye, vecSrc, -Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2), Vector(GRENADE_RADIUS + 2, GRENADE_RADIUS + 2, GRENADE_RADIUS + 2),
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr);

	if (tr.DidHit())
	{
		vecSrc = tr.endpos;
	}
}

void CSinWeaponBase::ThrowGrenade(CBasePlayer* pPlayer)
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors(&vForward, &vRight, NULL);
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * -8.0f;
	CheckThrowPosition(pPlayer, vecEye, vecSrc);
	//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity(&vecThrow, NULL);
	vecThrow += vForward * 1200;
	Fraggrenade_Create(vecSrc, vec3_angle, vecThrow, AngularImpulse(600, random->RandomInt(-1200, 1200), 0), pPlayer, GRENADE_TIMER, false);

	WeaponSound(SPECIAL3);

#ifdef MAPBASE
	pPlayer->SetAnimation(PLAYER_ATTACK3);
#endif
}

float CSinWeaponBase::GetMeleeDamageForActivity(Activity hitActivity)
{
	return sk_sin_plr_dmg_melee.GetFloat();
}

void CSinWeaponBase::AddMeleeViewKick()
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}

void CSinWeaponBase::MeleeImpactEffect(trace_t& trace)
{
	// See if we hit water (we don't do the other impact effects in this case)
	if (ImpactWater(trace.startpos, trace.endpos))
		return;

	//FIXME: need new decals
	UTIL_ImpactTrace(&trace, DMG_CLUB);
}

bool CSinWeaponBase::ImpactWater(const Vector& start, const Vector& end)
{
	return false;
}

void CSinWeaponBase::Swing()
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetMeleeRange());

	Vector swingEnd = swingStart + forward * GetMeleeRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = ACT_VM_SWINGHIT;

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetMeleeDamageForActivity(nHitActivity), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = ACT_VM_SWINGMISS;

#ifndef MAPBASE
		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
#endif // !MAPBASE
	}
#ifndef MAPBASE
	else
	{
		Hit(traceHit, nHitActivity, bIsSecondary ? true : false);
	}
#endif

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration(); //GetMeleeRate();
	m_flNextSecondaryAttack = gpGlobals->curtime + GetViewModelSequenceDuration();

#ifndef MAPBASE
	//Play swing sound
	WeaponSound(SINGLE);
#endif

#ifdef MAPBASE
	pOwner->SetAnimation(PLAYER_ATTACK2);

	if (GetMeleeHitDelay() > 0.f)
	{
		//Play swing sound
		WeaponSound(MELEE_MISS);

		m_flDelayedHit = gpGlobals->curtime + GetMeleeHitDelay();
		m_bHitDelayed = true;
	}
	else
	{
		if (traceHit.fraction == 1.0f)
		{
			// We want to test the first swing again
			Vector testEnd = swingStart + forward * GetMeleeRange();

			//Play swing sound
			WeaponSound(MELEE_MISS);

			// See if we happened to hit water
			ImpactWater(swingStart, testEnd);
		}
		else
		{
			// Other melee sounds
			if (traceHit.m_pEnt && traceHit.m_pEnt->IsWorld())
				WeaponSound(MELEE_HIT_WORLD);
			else if (traceHit.m_pEnt && !traceHit.m_pEnt->PassesDamageFilter(triggerInfo))
				WeaponSound(MELEE_MISS);
			else
				WeaponSound(MELEE_HIT);

			Hit(traceHit, nHitActivity);
		}
	}
#endif
}

void CSinWeaponBase::Hit(trace_t& traceHit, Activity nHitActivity)
{
	CBasePlayer* pPlayer = ToBasePlayer(GetOwner());

	//Do view kick
	AddMeleeViewKick();

	//Make sound for the AI
	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

	CBaseEntity* pHitEntity = traceHit.m_pEnt;

	//Apply damage to a hit target
	if (pHitEntity != NULL)
	{
		Vector hitDirection;
		pPlayer->EyeVectors(&hitDirection, NULL, NULL);
		VectorNormalize(hitDirection);
		CTakeDamageInfo info(GetOwner(), GetOwner(), GetMeleeDamageForActivity(nHitActivity), DMG_CLUB);


		if (pPlayer && pHitEntity->IsNPC())
		{
			// If bonking an NPC, adjust damage.
			info.AdjustPlayerDamageInflictedForSkillLevel();
		}

		CalculateMeleeDamageForce(&info, hitDirection, traceHit.endpos);

		pHitEntity->DispatchTraceAttack(info, hitDirection, &traceHit);
		ApplyMultiDamage();

		// Now hit all triggers along the ray that... 
		TraceAttackToTriggers(info, traceHit.startpos, traceHit.endpos, hitDirection);
	}

	// Apply an impact effect
	MeleeImpactEffect(traceHit);
}

void CSinWeaponBase::DelayedHit()
{
	m_bHitDelayed = false;

	trace_t traceHit;

	// Try a ray
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	//pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetMeleeRange());

	Vector swingEnd = swingStart + forward * GetMeleeRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.732f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	if (traceHit.fraction == 1.0f)
	{
		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetMeleeRange();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{
		CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetMeleeDamageForActivity(GetActivity()), GetDamageType());
		triggerInfo.SetDamagePosition(traceHit.startpos);
		triggerInfo.SetDamageForce(forward);

		// Other melee sounds
		if (traceHit.m_pEnt && traceHit.m_pEnt->IsWorld())
			WeaponSound(MELEE_HIT_WORLD);
		else if (traceHit.m_pEnt && traceHit.m_pEnt->PassesDamageFilter(triggerInfo))
			WeaponSound(MELEE_HIT);

		Hit(traceHit, GetActivity());
	}
}

Activity CSinWeaponBase::ChooseIntersectionPointAndActivity(trace_t& hitTrace, const Vector& mins, const Vector& maxs, CBasePlayer* pOwner)
{
	int			i, j, k;
	float		distance;
	const float* minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0)
	{
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 2; k++)
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0)
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance)
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}


	return ACT_VM_SWINGHIT;
}
