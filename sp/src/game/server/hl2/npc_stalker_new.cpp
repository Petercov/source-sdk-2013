//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: A slow-moving, once-human headcrab victim with only melee attacks.
//
// UNDONE: Make head take 100% damage, body take 30% damage.
// UNDONE: Don't flinch every time you get hit.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "game.h"
#include "AI_Default.h"
#include "ai_interactions.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "ai_squadslot.h"
#include "ai_squad.h"
#include "ai_route.h"
#include "ai_tacticalservices.h"
#include	"ai_hull.h"
#include	"ai_node.h"
#include	"ai_network.h"
#include	"ai_hint.h"
#include	"ai_schedule.h"
#include "ai_moveprobe.h"
#include "ai_task.h"
#include "NPCEvent.h"
#include "npc_stalker_new.h"
#include "basecombatcharacter.h"
#include "gib.h"
#include "soundent.h"
#include "engine/IEngineSound.h"
#include "AI_Interactions.h"
#include "ndebugoverlay.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "ai_senses.h"
#include "ai_memory.h"
#define	STALKER_SENTENCE_VOLUME		(float)0.35
ConVar	sk_alphastalker_health("sk_alphastalker_health", "125");
ConVar  sk_alphastalker_melee_dmg("sk_alphastalker_melee_dmg", "30");
ConVar  sk_alphastalker_dmg_both_slash("sk_alphastalker_dmg_both_slash", "60");
ConVar sk_alphastalker_leap_range("sk_alphastalker_leap_range", "500");
extern ConVar sv_gravity;


LINK_ENTITY_TO_CLASS(npc_alphastalker, CNPC_AlphaStalker);
IMPLEMENT_CUSTOM_AI(npc_alphastalker, CNPC_AlphaStalker);
BEGIN_DATADESC(CNPC_AlphaStalker)
DEFINE_FIELD(m_flLastRangeTime, FIELD_TIME)
END_DATADESC()

//=========================================================
// Spawn
//=========================================================
void CNPC_AlphaStalker::Spawn()
{
	Precache();

	//	SetModel( "models/zombie.mdl" );

	SetModel(STRING(GetModelName()));

	SetRenderColor(255, 255, 255, 255);

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	m_flLastRangeTime = gpGlobals->curtime;
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	m_bloodColor = BLOOD_COLOR_ZOMBIE;
	m_iHealth = sk_alphastalker_health.GetFloat();
	SetViewOffset(Vector(0, 0, 100));// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.8;
	m_NPCState = NPC_STATE_NONE;
	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_INNATE_MELEE_ATTACK1 | bits_CAP_DOORS_GROUP | bits_CAP_MOVE_JUMP | bits_CAP_MOVE_CLIMB | bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_DUCK);
	m_iBravery = 0;
	NPCInit();
}

bool CNPC_AlphaStalker::IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const
{
	const float MAX_JUMP_RISE = 512.0f;
	const float MAX_JUMP_DISTANCE = 512.0f;
	const float MAX_JUMP_DROP = 512.0f;

	if (BaseClass::IsJumpLegal(startPos, apex, endPos, MAX_JUMP_RISE, MAX_JUMP_DROP, MAX_JUMP_DISTANCE))
	{
		// Hang onto the jump distance. The AI is going to want it.
		m_flJumpDist = (startPos - endPos).Length();

		return true;
	}
	return false;
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CNPC_AlphaStalker::Precache()
{

	if (!GetModelName())
	{
		SetModelName(MAKE_STRING("models/alphastalker.mdl"));
	}
	PrecacheModel("models/alphastalker.mdl");

	PrecacheModel(STRING(GetModelName()));

	PrecacheScriptSound("AlphaStalker.AttackHit");
	PrecacheScriptSound("AlphaStalker.AttackMiss");
	PrecacheScriptSound("AlphaStalker.Pain");
	PrecacheScriptSound("AlphaStalker.Die");
	PrecacheScriptSound("AlphaStalker.Idle");
	PrecacheScriptSound("AlphaStalker.Alert");
	PrecacheScriptSound("AlphaStalker.Attack");
	PrecacheScriptSound("AlphaStalker.Scramble");
	PrecacheScriptSound("AlphaStalker.Breathing");
	PrecacheScriptSound("d1_town.Slicer");
	PrecacheScriptSound("Zombie.AttackMiss");
	PrecacheScriptSound("AlphaStalker.Leap");
	PrecacheScriptSound("NPC_Stalker.Hit");
	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: Returns this monster's place in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_AlphaStalker::Classify(void)
{
	return CLASS_STALKER;
}


void CNPC_AlphaStalker::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_ANNOUNCE_ATTACK:
	{
								 // Stop waiting if enemy facing me or lost enemy
								 CBaseCombatCharacter* pBCC = GetEnemyCombatCharacterPointer();
								 if (!pBCC || pBCC->FInViewCone(this))
								 {
									 TaskComplete();
								 }

								 if (gpGlobals->curtime >= m_flWaitFinished)
								 {
									 TaskComplete();
								 }
								 break;
	}

		case TASK_STALKER_ZIGZAG:
		{

		if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
		{
		TaskComplete();
		GetNavigator()->ClearGoal();		// Stop moving
		}
		else if (!GetNavigator()->IsGoalActive())
		{
		SetIdealActivity(GetStoppedActivity());
		}
		else if (ValidateNavGoal())
		{
		SetIdealActivity(GetNavigator()->GetMovementActivity());
		AddZigZagToPath();
		}
		break;
		}
	case TASK_RANGE_ATTACK1: //leap at target when in range
	{
								 SetIdealActivity(ACT_RANGE_ATTACK1);
								 SetTouch(&CNPC_AlphaStalker::LeapTouch);
								 EmitSound("AlphaStalker.Leap");
								 break;
	}

	case TASK_MELEE_ATTACK1:
	{
							   SetIdealActivity(ACT_MELEE_ATTACK1);
							   // AttackSound();
							   EmitSound("AlphaStalker.Attack");
							   break;

	}

	case TASK_FACE_ENEMY:
	{
							if (GetEnemy() != NULL)
							{
								BaseClass::StartTask(pTask);
								return;
							}

							if (FacingIdeal())
							{
								TaskComplete();
							}
							break;
	}
	default:
	{
			   BaseClass::StartTask(pTask);
			   break;
	}
	}
}

void CNPC_AlphaStalker::InitCustomSchedules(void)
{
	INIT_CUSTOM_AI(CNPC_AlphaStalker);
	ADD_CUSTOM_SCHEDULE(CNPC_AlphaStalker, SCHED_ALPHASTALKER_TAKECOVER);
	ADD_CUSTOM_SCHEDULE(CNPC_AlphaStalker, SCHED_ALPHASTALKER_INCOVER);
	ADD_CUSTOM_SCHEDULE(CNPC_AlphaStalker, SCHED_ALPHASTALKER_FLANK);
	AI_LOAD_SCHEDULE(CNPC_AlphaStalker, SCHED_ALPHASTALKER_TAKECOVER);
	AI_LOAD_SCHEDULE(CNPC_AlphaStalker, SCHED_ALPHASTALKER_INCOVER);
	AI_LOAD_SCHEDULE(CNPC_AlphaStalker, SCHED_ALPHASTALKER_FLANK);
	ADD_CUSTOM_TASK(CNPC_AlphaStalker, TASK_STALKER_ZIGZAG);
}

#define ZIG_ZAG_SIZE 3600

void CNPC_AlphaStalker::AddZigZagToPath(void)
{
	// If already on a detour don't add a zigzag
	if (GetNavigator()->GetCurWaypointFlags() & bits_WP_TO_DETOUR)
	{
		return;
	}

	// If enemy isn't facing me or occluded, don't add a zigzag
	if (HasCondition(COND_ENEMY_OCCLUDED) || !HasCondition(COND_ENEMY_FACING_ME))
	{
		return;
	}

	Vector waypointPos = GetNavigator()->GetCurWaypointPos();
	Vector waypointDir = (waypointPos - GetAbsOrigin());

	// If the distance to the next node is greater than ZIG_ZAG_SIZE
	// then add a random zig/zag to the path
	if (waypointDir.LengthSqr() > ZIG_ZAG_SIZE)
	{
		// Pick a random distance for the zigzag (less that sqrt(ZIG_ZAG_SIZE)
		float distance = random->RandomFloat(30, 60);

		// Get me a vector orthogonal to the direction of motion
		VectorNormalize(waypointDir);
		Vector vDirUp(0, 0, 1);
		Vector vDir;
		CrossProduct(waypointDir, vDirUp, vDir);

		// Pick a random direction (left/right) for the zigzag
		if (random->RandomInt(0, 1))
		{
			vDir = -1 * vDir;
		}

		// Get zigzag position in direction of target waypoint
		Vector zigZagPos = GetAbsOrigin() + waypointDir * 60;

		// Now offset 
		zigZagPos = zigZagPos + (vDir * distance);

		// Now make sure that we can still get to the zigzag position and the waypoint
		AIMoveTrace_t moveTrace1, moveTrace2;
		GetMoveProbe()->MoveLimit(NAV_GROUND, GetAbsOrigin(), zigZagPos, MASK_NPCSOLID, NULL, &moveTrace1);
		GetMoveProbe()->MoveLimit(NAV_GROUND, zigZagPos, waypointPos, MASK_NPCSOLID, NULL, &moveTrace2);
		if (!IsMoveBlocked(moveTrace1) && !IsMoveBlocked(moveTrace2))
		{
			GetNavigator()->PrependWaypoint(zigZagPos, NAV_GROUND, bits_WP_TO_DETOUR);
		}
	}
}

int CNPC_AlphaStalker::SelectSchedule(void)
{


	if (HasCondition(COND_ENEMY_DEAD))// && IsOkToSpeak() )
	{
		EmitSound("AlphaStalker.Announce");
	}

	switch (m_NPCState)
	{

	case NPC_STATE_ALERT:
	{
							if (HasCondition(COND_HEAR_DANGER) || HasCondition(COND_HEAR_COMBAT))
							{
							//	if (HasCondition(COND_HEAR_DANGER))
									return SCHED_TAKE_COVER_FROM_BEST_SOUND;

							//	else
							//		return SCHED_INVESTIGATE_SOUND;
							}
	}
		break;



	case NPC_STATE_COMBAT:
	{
							 // dead enemy
							 if (HasCondition(COND_ENEMY_DEAD))
								 return BaseClass::SelectSchedule(); // call base class, all code to handle dead enemies is centralized there.

							 // always act surprized with a new enemy
							 if (HasCondition(COND_NEW_ENEMY)) {
								 if (RandomInt(1, 10) < 3) {
									 EmitSound("AlphaStalker.Alert");
								 }
								 return SCHED_ALPHASTALKER_TAKECOVER;
								 Msg("Alpha Stalker: If you can read this, my custom AI Works!\n");
							 }

							 // if (HasCondition(COND_HEAVY_DAMAGE || COND_LIGHT_DAMAGE || COND_REPEATED_DAMAGE))
							 // return SCHED_SMALL_FLINCH;

							 if (HasCondition(COND_BEHIND_ENEMY))
							 {
								 // are we behind our enemy?
								 // Stalk him
								 return SCHED_CHASE_ENEMY;
							 }

							 if (HasCondition(COND_ENEMY_OCCLUDED))
							 {
								 return SCHED_ALPHASTALKER_TAKECOVER;
							 }

							 if (HasCondition(COND_TOO_FAR_TO_ATTACK))
							 {
								 return SCHED_ALPHASTALKER_TAKECOVER;
							 }
							 if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE) || HasCondition(COND_REPEATED_DAMAGE) || HasCondition(COND_NOT_FACING_ATTACK))
							 {
								 return SCHED_ALPHASTALKER_TAKECOVER;
							 }

							 if (HasCondition(COND_CAN_RANGE_ATTACK1))
							 {
								 //Leap Attack!
								 return SCHED_RANGE_ATTACK1;
							 }
							 if (HasCondition(COND_CAN_MELEE_ATTACK1))
							 {
								 return SCHED_MELEE_ATTACK1;
							 }
							 /*		 if (HasCondition(COND_ENEMY_FACING_ME) && HasCondition(COND_TOO_CLOSE_TO_ATTACK)) //forget about retreating if enemy is within pursuing range
							 {
							 return SCHED_CHASE_ENEMY;
							 }*/
							 /* if (m_iHealth < 20 || m_iBravery < 0)
							 {
							 if (!HasCondition(COND_CAN_MELEE_ATTACK1))
							 {
							 SetDefaultFailSchedule(SCHED_CHASE_ENEMY);
							 if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
							 return SCHED_TAKE_COVER_FROM_ENEMY;
							 if (HasCondition(COND_SEE_ENEMY) && HasCondition(COND_ENEMY_FACING_ME))
							 return SCHED_TAKE_COVER_FROM_ENEMY;
							 }
							 }*/


	}
		break;

	case NPC_STATE_IDLE:
		if (HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return SCHED_SMALL_FLINCH;
		}

		if (GetEnemy() == NULL) //&& GetFollowTarget() )
		{
			{

				return SCHED_IDLE_WANDER;
			}
		}

		// try to say something about smells
		break;
	}

	return BaseClass::SelectSchedule();
}

int CNPC_AlphaStalker::TranslateSchedule(int scheduleType)
{
	int baseType;
	baseType = BaseClass::TranslateSchedule(scheduleType);
	switch (scheduleType)
	{
	case SCHED_BACK_AWAY_FROM_ENEMY:
	{
									   return SCHED_CHASE_ENEMY;
	}
		break;
	}
	return BaseClass::TranslateSchedule(scheduleType);
}

Activity CNPC_AlphaStalker::NPC_TranslateActivity(Activity eNewActivity)
{
	if (eNewActivity == ACT_IDLE && m_NPCState != NPC_STATE_IDLE)
	{
		return (Activity)ACT_IDLE_ANGRY;
	}

	if (HasCondition(COND_SEE_ENEMY) && eNewActivity == ACT_RUN)
	{
		return (Activity)ACT_RUN_AGITATED;
	}
	return eNewActivity;
}


int CNPC_AlphaStalker::RangeAttack1Conditions(float flDot, float flDist)
{
	if (m_flLastRangeTime > gpGlobals->curtime)
	{
		return(COND_NONE);
	}

	if (!(GetFlags() & FL_ONGROUND))
	{
		return(COND_NONE);
	}
	if (flDist < 128)
	{
		return(COND_TOO_CLOSE_TO_ATTACK);
	}
	if (flDist > sk_alphastalker_leap_range.GetFloat())
	{
		return(COND_TOO_FAR_TO_ATTACK);
	}
	else if (flDot < 0.9)
	{
		return(COND_NOT_FACING_ATTACK);
	}

	return(COND_CAN_RANGE_ATTACK1);
}


void CNPC_AlphaStalker::LeapTouch(CBaseEntity *pOther)
{
	if (pOther->Classify() == Classify())
	{
		return;
	}

	// Don't hit if back on ground
	if (!(GetFlags() & FL_ONGROUND) && (pOther->IsNPC() || pOther->IsPlayer()))
	{
		LeapDamage(pOther);
	}

	SetTouch(NULL);
}

void CNPC_AlphaStalker::LeapDamage(CBaseEntity *pOther)
{
	CTakeDamageInfo info(this, this, sk_alphastalker_dmg_both_slash.GetFloat(), DMG_SLASH);
	CalculateMeleeDamageForce(&info, GetAbsVelocity(), GetAbsOrigin());
	pOther->TakeDamage(info);
	EmitSound("d1_town.Slicer");
}

/*bool CNPC_AlphaStalker::PlayerFlashlightOnMyEyes(CBasePlayer *pPlayer)
{
if (pPlayer->FlashlightIsOn() != 0) {
Vector vecEyes, vecPlayerForward;
vecEyes = EyePosition();
pPlayer->EyeVectors(&vecPlayerForward);

Vector vecToEyes = (vecEyes - pPlayer->EyePosition());
float flDist = VectorNormalize(vecToEyes);


float flDot = DotProduct(vecPlayerForward, vecToEyes);
if (flDot < 0.98)
return false;

// Check facing to ensure we're in front of her
Vector los = (pPlayer->EyePosition() - vecEyes);
los.z = 0;
VectorNormalize(los);
Vector facingDir = EyeDirection2D();
flDot = DotProduct(los, facingDir);
return (flDot > 0.3);
}
}*/



//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CNPC_AlphaStalker::HandleAnimEvent(animevent_t *pEvent)
{
	Vector v_forward, v_right;
	switch (pEvent->event)
	{
	case ALPHASTALKER_AE_ATTACK:
	{
								   // do stuff for this event.
								   CBaseEntity *pHurt;



								   pHurt = CheckTraceHullAttack(32, Vector(-16, -16, -16), Vector(16, 16, 16), sk_alphastalker_melee_dmg.GetFloat(), DMG_SLASH);

								   if (pHurt)
								   {
									   if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
									   {
										   pHurt->ViewPunch(QAngle(5, 0, random->RandomInt(-10, 10)));
									   }

									   // Spawn some extra blood if we hit a BCC
									   CBaseCombatCharacter* pBCC = ToBaseCombatCharacter(pHurt);
									   if (pBCC)
									   {
										   SpawnBlood(pBCC->EyePosition(), pBCC->GetAbsOrigin(), pBCC->BloodColor(), sk_alphastalker_melee_dmg.GetFloat());
									   }

									   // Play a attack hit sound
									   EmitSound("NPC_Stalker.Hit");
									   EmitSound("d1_town.Slicer");
								   }
								   else
								   {
									   EmitSound("Zombie.AttackMiss");
								   }

								   if (random->RandomInt(0, 5))
									   EmitSound("AlphaStalker.Attack");
	}
		break;

	case ALPHASTALKER_AE_ATTACK_BOTH:
	{
										// do stuff for this event.
										CBaseEntity *pHurt;



										pHurt = CheckTraceHullAttack(32, Vector(-16, -16, -16), Vector(16, 16, 16), sk_alphastalker_dmg_both_slash.GetFloat(), DMG_SLASH);

										if (pHurt)
										{
											if (pHurt->GetFlags() & (FL_NPC | FL_CLIENT))
											{
												pHurt->ViewPunch(QAngle(5, 0, random->RandomInt(-10, 10)));
											}

											// Spawn some extra blood if we hit a BCC
											CBaseCombatCharacter* pBCC = ToBaseCombatCharacter(pHurt);
											if (pBCC)
											{
												SpawnBlood(pBCC->EyePosition(), pBCC->GetAbsOrigin(), pBCC->BloodColor(), sk_alphastalker_dmg_both_slash.GetFloat());
											}

											// Play a attack hit sound
											EmitSound("NPC_Stalker.Hit");
											EmitSound("d1_town.Slicer");
											EmitSound("d1_town.Slicer");
										}
										else
										{
											EmitSound("Zombie.AttackMiss");
											EmitSound("Zombie.AttackMiss");
										}

										if (random->RandomInt(0, 5))
											EmitSound("AlphaStalker.Attack");
	}
		break;

	case ALPHASTALKER_AE_JUMPATTACK:
	{
									   m_flLastRangeTime = gpGlobals->curtime + ALPHASTALKER_LEAP_DELAY;
									   SetGroundEntity(NULL);

									   //
									   // Take him off ground so engine doesn't instantly reset FL_ONGROUND.
									   //
									   UTIL_SetOrigin(this, GetAbsOrigin() + Vector(0, 0, 1));

									   Vector vecJumpDir;
									   CBaseEntity *pEnemy = GetEnemy();
									   if (pEnemy)
									   {
										   Vector vecEnemyEyePos = pEnemy->GetAbsOrigin(); //eyeposition

										   float gravity = sv_gravity.GetFloat();
										   if (gravity <= 1)
										   {
											   gravity = 1;
										   }

										   //
										   // How fast does the stalker need to travel to reach my enemy's eyes given gravity?
										   //
										   float height = (vecEnemyEyePos.z - GetAbsOrigin().z);
										   if (height < 32) //original value is 16
										   {
											   height = 32;
										   }
										   else if (height > 120) //120
										   {
											   height = 120;
										   }
										   float speed = sqrt(2 * gravity * height);
										   float time = speed / gravity;

										   //
										   // Scale the sideways velocity to get there at the right time
										   //
										   vecJumpDir = vecEnemyEyePos - GetAbsOrigin();
										   vecJumpDir = vecJumpDir / time;

										   //
										   // Speed to offset gravity at the desired height.
										   //
										   vecJumpDir.z = speed;

										   //
										   // Don't jump too far/fast.
										   //
										   float distance = vecJumpDir.Length();
										   if (distance > 850) //650
										   {
											   vecJumpDir = vecJumpDir * (850.0 / distance);
										   }
									   }
									   else
									   {
										   //
										   // Jump hop, don't care where.
										   //
										   Vector forward, up;
										   AngleVectors(GetAbsAngles(), &forward, NULL, &up);
										   vecJumpDir = Vector(forward.x, forward.y, up.z) * 350;
									   }

									   int iSound = random->RandomInt(0, 2);
									   if (iSound != 0)
									   {
										   EmitSound("AlphaStalker.Leap");; //play go_alert sounds here when ready
									   }

									   SetAbsVelocity(vecJumpDir);
									   m_flNextAttack = gpGlobals->curtime + 2;
	}
		break;
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}


static float DamageForce(const Vector &size, float damage)
{
	float force = damage * ((32 * 32 * 72.0) / (size.x * size.y * size.z)) * 5;

	if (force > 1000.0)
	{
		force = 1000.0;
	}

	return force;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
// Output : int
//-----------------------------------------------------------------------------
int CNPC_AlphaStalker::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo info = inputInfo;

	// Take 30% damage from bullets
	if (info.GetDamageType() == DMG_BURN)
	{
		Vector vecDir = GetAbsOrigin() - info.GetInflictor()->WorldSpaceCenter();
		VectorNormalize(vecDir);
		float flForce = DamageForce(WorldAlignSize(), info.GetDamage());
		SetAbsVelocity(GetAbsVelocity() + vecDir * flForce);
		info.ScaleDamage(1.5f);
	}

	// HACK HACK -- until we fix this.
	if (IsAlive())
		PainSound(info);

	return BaseClass::OnTakeDamage_Alive(info);
}

void CNPC_AlphaStalker::PainSound(const CTakeDamageInfo &info)
{
	if (random->RandomInt(0, 5) < 2)
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "AlphaStalker.Pain");
	}
}

void CNPC_AlphaStalker::DeathSound(const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlphaStalker.Die");
}


void CNPC_AlphaStalker::AlertSound(void)
{
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlphaStalker.Alert");
}

void CNPC_AlphaStalker::IdleSound(void)
{
	// Play a random idle sound
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlphaStalker.Idle");
}

/*void CNPC_AlphaStalker::AttackSound(void)
{
// Play a random attack sound
CPASAttenuationFilter filter(this);
EmitSound(filter, entindex(), "AlphaStalker.Attack");
}*/

void CNPC_AlphaStalker::LeapSound(void)
{
	// Play a random attack sound
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), "AlphaStalker.Leap");
}

int CNPC_AlphaStalker::MeleeAttack1Conditions(float flDot, float flDist)
{
	if (flDist > 54)
	{
		return COND_ENEMY_TOO_FAR;
	}
	else if (flDot < 0.8)
	{
		return (COND_NONE);
	}
	else if (GetEnemy() == NULL)
	{
		return (COND_NONE);
	}

	return COND_CAN_MELEE_ATTACK1;
}

void CNPC_AlphaStalker::RemoveIgnoredConditions(void)
{
	if (GetActivity() == ACT_MELEE_ATTACK1)
	{
		// Nothing stops an attacking zombie.
		ClearCondition(COND_LIGHT_DAMAGE);
		ClearCondition(COND_HEAVY_DAMAGE);
		ClearCondition(COND_ENEMY_FACING_ME);
	}

	if (GetActivity() == ACT_RANGE_ATTACK1)
	{
		// Nothing stops an attacking zombie.
		ClearCondition(COND_LIGHT_DAMAGE);
		ClearCondition(COND_HEAVY_DAMAGE);
		ClearCondition(COND_ENEMY_FACING_ME);
	}

	if (GetActivity() == ACT_COVER_LOW) {
		ClearCondition(COND_NOT_FACING_ATTACK);
		SetHullType(HULL_TINY);

	}

	else
	{
		SetHullType(HULL_HUMAN);
	}

	if (GetActivity() == ACT_RUN || GetActivity() == ACT_WALK)
	{
		ClearCondition(COND_ENEMY_FACING_ME);
		ClearCondition(COND_BEHIND_ENEMY);
		ClearCondition(COND_LIGHT_DAMAGE);
		ClearCondition(COND_HEAVY_DAMAGE);
		ClearCondition(COND_TOO_CLOSE_TO_ATTACK);
	}

	if ((GetActivity() == ACT_SMALL_FLINCH) || (GetActivity() == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->curtime)
			m_flNextFlinch = gpGlobals->curtime + ALPHASTALKER_FLINCH_DELAY;
	}

	BaseClass::RemoveIgnoredConditions();
}


AI_DEFINE_SCHEDULE
(
SCHED_ALPHASTALKER_TAKECOVER,

"   Tasks"
"   TASK_STOP_MOVING    0"
"   TASK_GET_PATH_TO_ENEMY_LOS  0"
"	TASK_RUN_PATH				0"
"	TASK_WAIT_FOR_MOVEMENT		0"
"	TASK_FIND_NEAR_NODE_COVER_FROM_ENEMY 1024"
"	TASK_SET_TOLERANCE_DISTANCE 32"
"	TASK_RUN_PATH				0"
"	TASK_WAIT_FOR_MOVEMENT		0"
"	TASK_SET_SCHEDULE			SCHEDULE:SCHED_ALPHASTALKER_INCOVER"
""
"   Interrupts"
"       COND_WEAPON_BLOCKED_BY_FRIEND"
"      COND_ENEMY_DEAD"
//"       COND_LIGHT_DAMAGE"
//"       COND_HEAVY_DAMAGE"
//"		COND_REPEATED_DAMAGE"
"       COND_NO_PRIMARY_AMMO"
"      COND_HEAR_DANGER"
"	   COND_TOO_CLOSE_TO_ATTACK"
"      COND_HEAR_MOVE_AWAY"
"	   COND_CAN_RANGE_ATTACK1"
"	   COND_CAN_MELEE_ATTACK1"
"	   COND_BEHIND_ENEMY"
);

AI_DEFINE_SCHEDULE
(
SCHED_ALPHASTALKER_INCOVER,

"   Tasks"
"	TASK_WAIT_FOR_MOVEMENT		0"
"	TASK_PLAY_SEQUENCE_FACE_ENEMY	ACTIVITY:ACT_COVER_LOW"
"   TASK_WAIT_FACE_ENEMY		999"
"	TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_CHASE_ENEMY"
""
"   Interrupts"
"       COND_WEAPON_BLOCKED_BY_FRIEND"
"      COND_ENEMY_DEAD"
"       COND_LIGHT_DAMAGE"
"       COND_HEAVY_DAMAGE"
"		COND_REPEATED_DAMAGE"
"       COND_NO_PRIMARY_AMMO"
"      COND_HEAR_DANGER"
"	   COND_SEE_ENEMY"
"	   COND_NEW_ENEMY"
"	   COND_TOO_CLOSE_TO_ATTACK"
"      COND_HEAR_MOVE_AWAY"
"	   COND_CAN_RANGE_ATTACK1"
"	   COND_CAN_MELEE_ATTACK1"
"	   COND_BEHIND_ENEMY"
);


/*(SCHED_ALPHASTALKER_FLANK)
"   TASK_SET_TOLERANCE_DISTANCE		24"
"   TASK_GET_FLANK_RADIUS_PATH_TO_ENEMY_LOS 500"
"   TASK_RUN_PATH				0"
"	TASK_WAIT_FOR_MOVEMENT		0"
*/

AI_DEFINE_SCHEDULE
(
SCHED_ALPHASTALKER_FLANK,

"   Tasks"
"   TASK_SET_TOLERANCE_DISTANCE		24"
"	TASK_STORE_ENEMY_POSITION_IN_SAVEPOSITION 0"
"   TASK_GET_FLANK_RADIUS_PATH_TO_ENEMY_LOS 500"
"   TASK_RUN_PATH				0"
"	TASK_WAIT_FOR_MOVEMENT		0"
"	TASK_SET_FAIL_SCHEDULE		SCHEDULE:SCHED_ALPHASTALKER_INCOVER"
""
"   Interrupts"
"       COND_WEAPON_BLOCKED_BY_FRIEND"
"      COND_ENEMY_DEAD"
"       COND_LIGHT_DAMAGE"
"       COND_HEAVY_DAMAGE"
"		COND_REPEATED_DAMAGE"
"       COND_NO_PRIMARY_AMMO"
"      COND_HEAR_DANGER"
"	   COND_TOO_CLOSE_TO_ATTACK"
"      COND_HEAR_MOVE_AWAY"
"	   COND_CAN_RANGE_ATTACK1"
"	   COND_CAN_MELEE_ATTACK1"
"	   COND_BEHIND_ENEMY"
);
