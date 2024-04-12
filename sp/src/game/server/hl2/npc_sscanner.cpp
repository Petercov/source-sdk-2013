//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "npc_sscanner.h"
#include "AI_Default.h"
#include "AI_Hull.h"
#include "AI_Hint.h"
#include "AI_Node.h" // just for GetFloorZ()
#include "AI_Navigator.h"
#include "ai_moveprobe.h"
#include "AI_Squad.h"
#include "explode.h"
#include "grenade_energy.h"
#include "ndebugoverlay.h"
#include "scanner_shield.h"
#include "npc_sscanner_beam.h"
#include "gib.h"
#include "IEffects.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "movevars_shared.h"
#include "items.h"

#include "tier0/memdbgon.h"

#define	SSCANNER_MAX_SPEED				400
#define	SSCANNER_NUM_GLOWS				2
#define SSCANNER_GIB_COUNT			5   
#define SSCANNER_ATTACK_NEAR_DIST		600
#define SSCANNER_ATTACK_FAR_DIST		800

#define SSCANNER_COVER_NEAR_DIST		60
#define SSCANNER_COVER_FAR_DIST		250
#define SSCANNER_MAX_SHEILD_DIST		400

#define	SSCANNER_BANK_RATE			5
#define SSCANNER_ENERGY_WARMUP_TIME	0.5
#define SSCANNER_MIN_GROUND_DIST		150

#define SSCANNER_GIB_COUNT			5 

ConVar	sk_sscanner_health( "sk_sscanner_health","0");

extern float	GetFloorZ(const Vector &origin, float fMaxDrop);

//-----------------------------------------------------------------------------
// Private activities.
//-----------------------------------------------------------------------------
int	ACT_SSCANNER_FLINCH_BACK;
int	ACT_SSCANNER_FLINCH_FRONT;
int	ACT_SSCANNER_FLINCH_LEFT;
int	ACT_SSCANNER_FLINCH_RIGHT;
int	ACT_SSCANNER_LOOK;
int	ACT_SSCANNER_OPEN;
//int	ACT_SSCANNER_CLOSE;


BEGIN_DATADESC( CNPC_SScanner )

	DEFINE_FIELD( m_lastHurtTime,				FIELD_FLOAT ),

	DEFINE_FIELD( m_nState,					FIELD_INTEGER),
	DEFINE_FIELD( m_bShieldsDisabled,			FIELD_BOOLEAN),
	DEFINE_FIELD( m_pShield,					FIELD_EHANDLE ),
	DEFINE_FIELD( m_pShieldBeamL, FIELD_EHANDLE),
	DEFINE_FIELD( m_pShieldBeamR, FIELD_EHANDLE),
	DEFINE_FIELD( m_fNextShieldCheckTime,		FIELD_TIME),

	DEFINE_FIELD( m_vCoverPosition,			FIELD_POSITION_VECTOR),

END_DATADESC()


LINK_ENTITY_TO_CLASS( npc_sscanner, CNPC_SScanner );
IMPLEMENT_CUSTOM_AI( npc_sscanner, CNPC_SScanner );

CNPC_SScanner::CNPC_SScanner()
{
#ifdef _DEBUG
	m_vCurrentBanking.Init();
	m_vCoverPosition.Init();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Initialize the custom schedules
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_SScanner::InitCustomSchedules(void) 
{
	INIT_CUSTOM_AI(CNPC_SScanner);

	ADD_CUSTOM_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_HOVER);
	//ADD_CUSTOM_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_PATROL);
	//ADD_CUSTOM_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_CHASE_ENEMY);
	//ADD_CUSTOM_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_CHASE_TARGET);
	ADD_CUSTOM_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_OPEN);
	ADD_CUSTOM_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_CLOSE);

	//ADD_CUSTOM_CONDITION(CNPC_SScanner,	COND_SSCANNER_FLY_BLOCKED);
	//ADD_CUSTOM_CONDITION(CNPC_SScanner,	COND_SSCANNER_FLY_CLEAR);
	ADD_CUSTOM_CONDITION(CNPC_SScanner,	COND_SSCANNER_PISSED_OFF);

	ADD_CUSTOM_TASK(CNPC_SScanner,		TASK_SSCANNER_OPEN);
	ADD_CUSTOM_TASK(CNPC_SScanner,		TASK_SSCANNER_CLOSE);

	ADD_CUSTOM_ACTIVITY(CNPC_SScanner,	ACT_SSCANNER_FLINCH_BACK);
	ADD_CUSTOM_ACTIVITY(CNPC_SScanner,	ACT_SSCANNER_FLINCH_FRONT);
	ADD_CUSTOM_ACTIVITY(CNPC_SScanner,	ACT_SSCANNER_FLINCH_LEFT);
	ADD_CUSTOM_ACTIVITY(CNPC_SScanner,	ACT_SSCANNER_FLINCH_RIGHT);
	ADD_CUSTOM_ACTIVITY(CNPC_SScanner,	ACT_SSCANNER_OPEN);
	//ADD_CUSTOM_ACTIVITY(CNPC_SScanner,	ACT_SSCANNER_CLOSE);

	AI_LOAD_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_HOVER);
	//AI_LOAD_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_PATROL);
	//AI_LOAD_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_CHASE_ENEMY);
	//AI_LOAD_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_CHASE_TARGET);
	AI_LOAD_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_OPEN);
	AI_LOAD_SCHEDULE(CNPC_SScanner,	SCHED_SSCANNER_CLOSE);
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : Type - 
// Output : int - new schedule type
//-----------------------------------------------------------------------------
int CNPC_SScanner::TranslateSchedule( int scheduleType ) 
{
	switch( scheduleType )
	{
	case SCHED_FAIL_TAKE_COVER:
		return SCHED_SCANNER_PATROL;
		break;
	}
	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CNPC_SScanner::Event_Killed( const CTakeDamageInfo &info )
{
	// Copy off the takedamage info that killed me, since we're not going to call
	// up into the base class's Event_Killed() until we gib. (gibbing is ultimate death)
	m_KilledInfo = info;

	StopShield();

	// Interrupt whatever schedule I'm on
	SetCondition(COND_SCHEDULE_DONE);

	// If I have an enemy and I'm up high, do a dive bomb (unless dissolved)
	if (GetEnemy() != NULL && (info.GetDamageType() & DMG_DISSOLVE) == false)
	{
		Vector vecDelta = GetLocalOrigin() - GetEnemy()->GetLocalOrigin();
		if ((vecDelta.z > 120) && (vecDelta.Length() > 360))
		{
			// If I'm divebombing, don't take any more damage. It will make Event_Killed() be called again.
			// This is especially bad if someone machineguns the divebombing scanner. 
			AttackDivebomb();
			return;
		}
	}

	Gib();
}


//------------------------------------------------------------------------------
// Purpose : Override to split in two when attacked
// Input   :
// Output  :
//------------------------------------------------------------------------------
int CNPC_SScanner::OnTakeDamage_Alive( const CTakeDamageInfo &inputInfo )
{
	CTakeDamageInfo info = inputInfo;

	// Don't take friendly fire from combine
	if (info.GetAttacker()->Classify() == CLASS_COMBINE)
	{
		info.SetDamage( 0 );
	}
	else if (m_nState == SSCANNER_OPEN)
	{
		// Flinch in the direction of the attack
		float vAttackYaw = VecToYaw(g_vecAttackDir);

		float vAngleDiff = UTIL_AngleDiff( vAttackYaw, m_fHeadYaw );

		if (vAngleDiff > -45 && vAngleDiff < 45)
		{
			SetActivity((Activity)ACT_SSCANNER_FLINCH_BACK);
		}
		else if (vAngleDiff < -45 && vAngleDiff > -135)
		{
			SetActivity((Activity)ACT_SSCANNER_FLINCH_LEFT);
		}
		else if (vAngleDiff >  45 && vAngleDiff <  135)
		{
			SetActivity((Activity)ACT_SSCANNER_FLINCH_RIGHT);
		}
		else
		{
			SetActivity((Activity)ACT_SSCANNER_FLINCH_FRONT);
		}

		m_lastHurtTime = gpGlobals->curtime;
	}
	return (BaseClass::OnTakeDamage_Alive( info ));
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CNPC_SScanner::IsValidShieldCover( Vector &vCoverPos )
{
	if (GetEnemy() == NULL)
	{
		return true;
	}

	// Make sure I can get here
	trace_t tr;
	AI_TraceEntity( this, GetAbsOrigin(), vCoverPos, MASK_NPCSOLID, &tr );
	if (tr.fraction != 1.0)
	{
		//NDebugOverlay::Cross3D(vCoverPos,Vector(-15,-15,-15),Vector(5,5,5),255,0,0,true,1.0);
		return false;
	}

	// Make sure position is in cover
	Vector vThreatEyePos = GetEnemy()->EyePosition();
	AI_TraceLine ( vCoverPos, vThreatEyePos, MASK_OPAQUE,  this, COLLISION_GROUP_NONE, &tr );
	if (tr.fraction == 1.0)
	{
		//NDebugOverlay::Cross3D(vCoverPos,Vector(-15,-15,-15),Vector(5,5,5),0,0,255,true,1.0);
		return false;
	}
	else
	{
		//NDebugOverlay::Cross3D(vCoverPos,Vector(-15,-15,-15),Vector(5,5,5),0,255,0,true,1.0);
		return true;
	}
}

//------------------------------------------------------------------------------
// Purpose : Attempts to find a position that is in cover from the enemy
//			 but with in range to use shield.
// Input   :
// Output  : True if a cover position was set
//------------------------------------------------------------------------------
bool CNPC_SScanner::SetShieldCoverPosition( void )
{
	// Make sure I'm shielding someone and have an enemy
	if (GetTarget() == NULL	||
		GetEnemy()	 == NULL	)
	{
		m_vCoverPosition = vec3_origin;
		return false;
	}

	// If I have a current cover position check if it's valid
	if (m_vCoverPosition != vec3_origin)
	{
		// If in range of my shield target, and valid cover keep same cover position
		if ((m_vCoverPosition - GetTarget()->GetLocalOrigin()).Length() < SSCANNER_COVER_FAR_DIST &&
			IsValidShieldCover(m_vCoverPosition)								)
		{
			return true;
		}
	}

	// Otherwise try a random direction
	QAngle vAngle	= QAngle(0,random->RandomInt(-180,180),0);
	Vector vForward;
	AngleVectors( vAngle, &vForward );
	
	// Now get the position
	Vector vTestPos = GetTarget()->GetLocalOrigin() + vForward * (random->RandomInt(150,240)); 
	vTestPos.z		= GetLocalOrigin().z;

	// Is it a valid cover position
	if (IsValidShieldCover(vTestPos))
	{
		m_vCoverPosition = vTestPos;
		return true;
	}
	else
	{
		m_vCoverPosition = vec3_origin;
		return false;
	}
}

bool CNPC_SScanner::OverrideMove(float flInterval)
{
	// ----------------------------------------------
	//	If dive bombing
	// ----------------------------------------------
	if (m_nFlyMode == SCANNER_FLY_DIVE)
	{
		MoveToDivebomb(flInterval);
	}
	else
	{
		// ----------------------------------------------
		//	Select move target 
		// ----------------------------------------------
		CBaseEntity* pMoveTarget = NULL;
		float			fNearDist = 0;
		float			fFarDist = 0;
		if (GetTarget() != NULL)
		{
			pMoveTarget = GetTarget();
			fNearDist = SSCANNER_COVER_NEAR_DIST;
			fFarDist = SSCANNER_COVER_FAR_DIST;
		}
		else if (GetEnemy() != NULL)
		{
			pMoveTarget = GetEnemy();
			fNearDist = SSCANNER_ATTACK_NEAR_DIST;
			fFarDist = SSCANNER_ATTACK_FAR_DIST;
		}

		// -----------------------------------------
		//  See if we can fly there directly
		// -----------------------------------------
		if (pMoveTarget)
		{
			trace_t tr;
			Vector endPos = GetAbsOrigin() + GetCurrentVelocity() * flInterval;
			AI_TraceHull(GetAbsOrigin(), pMoveTarget->GetAbsOrigin() + Vector(0, 0, 150),
				GetHullMins(), GetHullMaxs(), MASK_NPCSOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
			if (tr.fraction != 1.0)
			{
				/*
				NDebugOverlay::Cross3D(GetLocalOrigin()+Vector(0,0,-60),Vector(-15,-15,-15),Vector(5,5,5),0,255,255,true,0.1);
				*/
				SetCondition(COND_SCANNER_FLY_BLOCKED);
			}
			else
			{
				SetCondition(COND_SCANNER_FLY_CLEAR);
			}
		}

		// -----------------------------------------------------------------
		// If I have a route, keep it updated and move toward target
		// ------------------------------------------------------------------
		if (GetNavigator()->IsGoalActive())
		{
			if (OverridePathMove(pMoveTarget, flInterval))
			{
				BlendPhyscannonLaunchSpeed();
				return true;
			}
		}
		// ----------------------------------------------
		//	Move to target directly if path is clear
		// ----------------------------------------------
		else if (pMoveTarget && HasCondition(COND_SCANNER_FLY_CLEAR))
		{
			MoveToEntity(flInterval, pMoveTarget, fNearDist, fFarDist);
		}
		// -----------------------------------------------------------------
		// Otherwise just decelerate
		// -----------------------------------------------------------------
		else
		{
			float	myDecay = 9.5;
			Decelerate(flInterval, myDecay);
			// -------------------------------------
			// If I have an enemy turn to face him
			// -------------------------------------
			if (GetEnemy())
			{
				TurnHeadToTarget(flInterval, GetEnemy()->GetLocalOrigin());
			}
		}
	}

	MoveExecute_Alive(flInterval);

	UpdateShields();
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Override base class activiites
// Input  :
// Output :
//-----------------------------------------------------------------------------
Activity CNPC_SScanner::NPC_TranslateActivity( Activity eNewActivity )
{
	if ( eNewActivity == ACT_IDLE)
	{
		if (m_nState == SSCANNER_OPEN)
		{
			return ACT_IDLE_ANGRY;
		}
		else
		{
			return ACT_IDLE;
		}
	}
	return eNewActivity;
}

//------------------------------------------------------------------------------
// Purpose : Choose which entity to shield
// Input   :
// Output  :
//------------------------------------------------------------------------------
CBaseEntity* CNPC_SScanner::PickShieldEntity(void)
{
	if (m_bShieldsDisabled)
	{
		return NULL;
	}

	//	Don't pick every frame as it gets expensive
	if (gpGlobals->curtime > m_fNextShieldCheckTime)
	{
		m_fNextShieldCheckTime = gpGlobals->curtime + 1.0;

		CBaseEntity* pBestEntity = NULL;
		float		 fBestDist	 = MAX_COORD_RANGE;
		bool		 bBestLOS	 = false;
		float		 fTestDist;
		bool		 bTestLOS	 = false;;

		AISquadIter_t iter;
		for (CAI_BaseNPC *pSquadMember = m_pSquad->GetFirstMember( &iter ); pSquadMember; pSquadMember = m_pSquad->GetNextMember( &iter ) )
		{
			CAI_BaseNPC* pTestNPC = pSquadMember;
			if (pTestNPC != NULL && 
				pTestNPC != this &&
				pTestNPC->Classify() != CLASS_SCANNER	)
			{
				bTestLOS  = pTestNPC->HasCondition( COND_HAVE_ENEMY_LOS);
				fTestDist = (GetLocalOrigin() - pTestNPC->GetLocalOrigin()).Length();

				// -----------------------------------------
				//  Test has LOS and best doesn't, pick test
				// -----------------------------------------
				if (bTestLOS && !bBestLOS)
				{
					// Skip if I can't see this entity
					if (IsValidShieldTarget(pTestNPC) )
					{
						fBestDist	= fTestDist;
						pBestEntity	= pTestNPC;
						bBestLOS	= bTestLOS;
					}
				}
				// -----------------------------------------
				//  Best has LOS and test doesn't, skip
				// -----------------------------------------
				else if (!bTestLOS && bBestLOS)
				{
					continue;
				}
				// -----------------------------------------------
				//  Otherwise pick by distance
				// -----------------------------------------------
				if (fTestDist <	fBestDist )					
				{
					// Skip if I can't see this entity
					if (IsValidShieldTarget(pTestNPC) )
					{
						fBestDist	= fTestDist;
						pBestEntity	= pTestNPC;
						bBestLOS	= bTestLOS;
					}
				}
			}
		}
		return pBestEntity;
	}
	else
	{
		return GetTarget();
	}
}

//------------------------------------------------------------------------------
// Purpose : Override to set pissed level of sscanner
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_SScanner::NPCThink(void)
{
	// --------------------------------------------------
	//  COND_SSCANNER_PISSED_OFF
	// --------------------------------------------------
	float fHurtAge		= gpGlobals->curtime - m_lastHurtTime;

	if (fHurtAge > 5.0)
	{
		ClearCondition(COND_SSCANNER_PISSED_OFF);
		m_bShieldsDisabled = false;
	}
	else
	{
		SetCondition(COND_SSCANNER_PISSED_OFF);
		m_bShieldsDisabled = true;
	}

	BaseClass::NPCThink();
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_SScanner::PowerShield(void)
{
	// -----------------------------
	//	Start the shield chase beam
	// -----------------------------
	if (!m_pShieldBeamL)
	{
		m_pShieldBeamL = CShieldBeam::ShieldBeamCreate( this, LookupAttachment("1"));
		m_pShieldBeamR = CShieldBeam::ShieldBeamCreate( this, LookupAttachment("0"));
	}

	if (!m_pShieldBeamL->IsShieldBeamOn())
	{
		m_pShieldBeamL->ShieldBeamStart(this, GetTarget(), GetCurrentVelocity(), 500 );
		m_pShieldBeamR->ShieldBeamStart(this, GetTarget(), GetCurrentVelocity(), 500 );
		m_pShieldBeamL->m_vTailOffset		= Vector(0,0,88);
		m_pShieldBeamR->m_vTailOffset		= Vector(0,0,88);
	}

	// -------------------------------------------------
	//	Start the shield when chaser reaches the shield
	// ------------------------------------------------- 
	if (m_pShieldBeamL->IsShieldBeamOn() && m_pShieldBeamL->ReachedTail())
	{

		if (!m_pShield)
		{
			m_pShield = (CScannerShield *)CreateEntityByName("scanner_shield" );
			m_pShield->Spawn();
		}
		m_pShield->SetTarget(GetTarget());
		m_pShieldBeamL->SetNoise(GetCurrentVelocity().Length());
		m_pShieldBeamR->SetNoise(GetCurrentVelocity().Length());
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_SScanner::StopShield(void)
{
	// -----------------------------
	//	Stop shield chase beam
	// -----------------------------
	if (m_pShieldBeamL)
	{
		m_pShieldBeamL->ShieldBeamStop();
		m_pShieldBeamL = NULL;
	}
	if (m_pShieldBeamR)
	{
		m_pShieldBeamR->ShieldBeamStop();
		m_pShieldBeamR = NULL;
	}

	// -----------------------------
	//	Stop the shield
	// -----------------------------
	if (m_pShield)
	{
		m_pShield->SetTarget(NULL);
	}
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_SScanner::UpdateShields(void)
{
	// -------------------------------------------------------
	//  Turn of shield and don't reset m_hTarget if in script
	// -------------------------------------------------------
	if (m_NPCState == NPC_STATE_SCRIPT)
	{
		StopShield();
		return;
	}

	// -------------------------------------------------------------------------
	//  If I'm not in a squad there's no one to shield
	// -------------------------------------------------------------------------
	if (!m_pSquad)
	{
		return;
	}

	// -------------------------------------------------------------------------
	// If I'm dead, stop the shields
	// -------------------------------------------------------------------------
	if (m_NPCState == NPC_STATE_DEAD )
	{
		SetTarget( NULL );
		StopShield();
		return;
	}

	// -------------------------------------------------------------------------
	// Pick the best entity to shield
	// -------------------------------------------------------------------------
	CBaseEntity* pShielded = PickShieldEntity();

	// -------------------------------------------------------------------------
	// If there was no best entity stop the shields 
	// -------------------------------------------------------------------------
	if (pShielded == NULL)
	{
		SetTarget( NULL );
		StopShield();
		return;
	}

	// -------------------------------------------------------------------------
	// If I'm too far to shield set best entity as target but stop the shields
	// -------------------------------------------------------------------------
	float fDist = (GetLocalOrigin() - pShielded->GetLocalOrigin()).Length();
	if (fDist > SSCANNER_MAX_SHEILD_DIST)
	{
		SetTarget( pShielded );
		StopShield();
		return;
	}

	// -------------------------------------------------------------------------
	// If this is a new target, stop the shield
	// -------------------------------------------------------------------------
	if (m_pShieldBeamL				&&
		GetTarget() != pShielded	&&
		m_pShieldBeamL->IsShieldBeamOn()	)
	{
		StopShield();
	}

	// -------------------------------------------------------------------------
	// Reset my target entity and power the shield
	// -------------------------------------------------------------------------
	SetTarget( pShielded );

	// -------------------------------------------------------------------------
	// If I'm closed, stop the shields
	// -------------------------------------------------------------------------
	if ( m_nState == SSCANNER_CLOSED )
	{
		StopShield();
	}
	else
	{
		PowerShield();
	}
	
	/* DEBUG TOOL 
	if (GetTarget())
	{
		int blue = 0;
		int red  = 0;
		if (GetTarget())
		{
			red = 255;
		}
		else
		{
			blue = 255;
		}
		NDebugOverlay::Cross3D(GetTarget()->GetLocalOrigin()+Vector(0,0,60),Vector(-15,-15,-15),Vector(5,5,5),red,0,blue,true,0.1);
		NDebugOverlay::Cross3D(GetLocalOrigin()+Vector(0,0,60),Vector(-15,-15,-15),Vector(5,5,5),red,0,blue,true,0.1);
		NDebugOverlay::Line(GetLocalOrigin(), GetTarget()->GetLocalOrigin(), red,0,blue, true, 0.1);
	}
	else
	{
		NDebugOverlay::Cross3D(GetLocalOrigin()+Vector(0,0,60),Vector(-15,-15,-15),Vector(5,5,5),0,255,0,true,0.1);
	}
	*/
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_SScanner::MoveToTarget(float flInterval, const Vector &vecMoveTarget)
{
	//const float	myAccel	 = 300.0;
	//const float	myDecay	 = 9.0;
	
	TurnHeadToTarget( flInterval, vecMoveTarget );

	// -------------------------------------
	// Move towards our target
	// -------------------------------------
	float myAccel;
	float myZAccel = 400.0f;
	float myDecay = 0.15f;

	Vector vecCurrentDir;

	// Get the relationship between my current velocity and the way I want to be going.
	vecCurrentDir = GetCurrentVelocity();
	VectorNormalize(vecCurrentDir);

	Vector targetDir = vecMoveTarget - GetAbsOrigin();
	float flDist = VectorNormalize(targetDir);

	float flDot;
	flDot = DotProduct(targetDir, vecCurrentDir);

	if (flDot > 0.25)
	{
		// If my target is in front of me, my flight model is a bit more accurate.
		myAccel = 250;
	}
	else
	{
		// Have a harder time correcting my course if I'm currently flying away from my target.
		myAccel = 128;
	}

	if (myAccel > flDist / flInterval)
	{
		myAccel = flDist / flInterval;
	}

	if (myZAccel > flDist / flInterval)
	{
		myZAccel = flDist / flInterval;
	}

#ifdef MAPBASE
	if (m_flSpeedModifier != 1.0f)
	{
		myAccel *= m_flSpeedModifier;
		//myZAccel *= m_flSpeedModifier;
	}
#endif

	MoveInDirection(flInterval, targetDir, myAccel, myZAccel, myDecay);

	// calc relative banking targets
	Vector forward, right, up;
	GetVectors(&forward, &right, &up);

	m_vCurrentBanking.x = targetDir.x;
	m_vCurrentBanking.z = 120.0f * DotProduct(right, targetDir);
	m_vCurrentBanking.y = 0;

	float speedPerc = SimpleSplineRemapVal(GetCurrentVelocity().Length(), 0.0f, GetMaxSpeed(), 0.0f, 1.0f);

	speedPerc = clamp(speedPerc, 0.0f, 1.0f);

	m_vCurrentBanking *= speedPerc;
}



//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_SScanner::MoveToEntity(float flInterval, CBaseEntity* pMoveTarget, float fMinRange, float fMaxRange)
{
	// -----------------------------------------
	//  Make sure we have a move target
	// -----------------------------------------
	if (!pMoveTarget)
	{
		return;
	}

	// -----------------------------------------
	//  Keep within range of enemy
	// -----------------------------------------
	Vector vFlyDirection = vec3_origin;
	Vector	vEnemyDir  = pMoveTarget->EyePosition() - GetAbsOrigin();
	float	fEnemyDist = VectorNormalize(vEnemyDir);
	if (fEnemyDist < fMinRange)
	{
		vFlyDirection = -vEnemyDir;
	}
	else if (fEnemyDist > fMaxRange)
	{
		vFlyDirection = vEnemyDir;
	}
	// If I'm shielding someone see if I can go to cover
	else if (SetShieldCoverPosition())
	{
		vFlyDirection = m_vCoverPosition - GetAbsOrigin();
		VectorNormalize(vFlyDirection);
	}

	TurnHeadToTarget( flInterval, pMoveTarget->GetAbsOrigin() );

	// -------------------------------------
	// Set net velocity 
	// -------------------------------------
	float	myAccel	 = 500.0;
	float	myDecay	 = 0.35; // decay to 35% in 1 second
	MoveInDirection(flInterval, vFlyDirection, myAccel, (2 * myAccel), myDecay);
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
float CNPC_SScanner::MinGroundDist(void)
{
	return SSCANNER_MIN_GROUND_DIST;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SScanner::Precache(void)
{
	//
	// Model.
	//
	PrecacheModel(DefaultOrCustomModel("models/shield_scanner.mdl"));

	PrecacheModel("models/gibs/Shield_Scanner_Gib1.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib2.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib3.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib4.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib5.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib6.mdl");

	PrecacheScriptSound("NPC_SScanner.Shoot");
	PrecacheScriptSound("NPC_SScanner.Alert");
	PrecacheScriptSound("NPC_SScanner.Die");
	PrecacheScriptSound("NPC_SScanner.Combat");
	PrecacheScriptSound("NPC_SScanner.Idle");
	PrecacheScriptSound("NPC_SScanner.Pain");
	PrecacheScriptSound("NPC_SScanner.TakePhoto");
	PrecacheScriptSound("NPC_SScanner.AttackFlash");
	PrecacheScriptSound("NPC_SScanner.DiveBombFlyby");
	PrecacheScriptSound("NPC_SScanner.DiveBomb");
	PrecacheScriptSound("NPC_SScanner.DeployMine");

	PrecacheScriptSound("NPC_SScanner.FlyLoop");

	UTIL_PrecacheOther("scanner_shield");

	engine->PrecacheModel("sprites/physbeam.vmt");	
	engine->PrecacheModel("sprites/glow01.vmt");

	engine->PrecacheModel("sprites/physring1.vmt");

	BaseClass::Precache();
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_SScanner::Gib(void)
{
	if (IsMarkedForDeletion())
		return;

	// Spawn all gibs
	{
		CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib1.mdl");
		CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib2.mdl");
		CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib3.mdl");
		CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib4.mdl");
		CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib5.mdl");
		CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib6.mdl");
	}

	// Add a random chance of spawning a battery...
	if (!HasSpawnFlags(SF_NPC_NO_WEAPON_DROP) && random->RandomFloat(0.0f, 1.0f) < 0.3f)
	{
		CItem* pBattery = (CItem*)CreateEntityByName("item_battery");
		if (pBattery)
		{
			pBattery->SetAbsOrigin(GetAbsOrigin());
			pBattery->SetAbsVelocity(GetAbsVelocity());
			pBattery->SetLocalAngularVelocity(GetLocalAngularVelocity());
			pBattery->ActivateWhenAtRest();
			pBattery->Spawn();
		}
	}

	BaseClass::Gib();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_SScanner::RunTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
		// If my enemy has moved significantly, update my path
		case TASK_WAIT_FOR_MOVEMENT:
		{
			CBaseEntity *pEnemy = GetEnemy();
			if (pEnemy && IsCurSchedule(SCHED_SCANNER_CHASE_ENEMY) && GetNavigator()->IsGoalActive())
			{
				Vector flEnemyLKP = GetEnemyLKP();
				if ((GetNavigator()->GetGoalPos() - pEnemy->EyePosition()).Length() > 40 )
				{
					GetNavigator()->UpdateGoalPos(pEnemy->EyePosition());
				}
				// If final position is enemy, exit my schedule (will go to attack hover)
				if (GetNavigator()->IsGoalActive() && 
					GetNavigator()->GetCurWaypointPos() == pEnemy->EyePosition())
				{
					TaskComplete();
					GetNavigator()->ClearGoal();		// Stop moving
					break;
				}
			}
			if (m_nState == SSCANNER_OPEN)
			{
				GetNavigator()->SetMovementActivity(ACT_IDLE_ANGRY);
			}
			else
			{
				GetNavigator()->SetMovementActivity(ACT_IDLE);
			}

			BaseClass::RunTask(pTask);
			break;
		}

		default:
		{
			BaseClass::RunTask(pTask);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_SScanner::Spawn(void)
{
	Precache();

	SetModel(DefaultOrCustomModel("models/shield_scanner.mdl"));

	m_iHealth			= sk_sscanner_health.GetFloat();

	m_nState					= SSCANNER_CLOSED;
	m_bShieldsDisabled			= false;

	m_vLastPatrolDir			= vec3_origin;

	m_fNextShieldCheckTime		= 0;

	BaseClass::Spawn();

	m_flFieldOfView = VIEW_FIELD_FULL;

	// Let this guy look far
	m_flDistTooFar	= 99999999.0;
	SetDistLook( 4000.0 );
}


void CNPC_SScanner::OnScheduleChange()
{
	BaseClass::OnScheduleChange();
}


//-----------------------------------------------------------------------------
// Purpose: Gets the appropriate next schedule based on current condition
//			bits.
//-----------------------------------------------------------------------------
int CNPC_SScanner::SelectSchedule(void)
{
	// ----------------------------------------------------
	//  If I'm dead, go into a dive bomb
	// ----------------------------------------------------
	if (m_iHealth <= 0)
	{
		m_flSpeed = SCANNER_MAX_DIVE_BOMB_SPEED;
		return SCHED_SCANNER_ATTACK_DIVEBOMB;
	}

	// -------------------------------
	// If I'm in a script sequence
	// -------------------------------
	if (m_NPCState == NPC_STATE_SCRIPT)
		return(BaseClass::SelectSchedule());

	// I'm being held by the physcannon... struggle!
	if (IsHeldByPhyscannon())
		return SCHED_SCANNER_HELD_BY_PHYSCANNON;

	switch ( m_NPCState )
	{
		// -------------------------------------------------
		//  In idle attemp to pair up with a squad 
		//  member that could use shielding
		// -------------------------------------------------
		case NPC_STATE_IDLE:
		{
			if (m_nState == SSCANNER_OPEN)
				return SCHED_SSCANNER_CLOSE;

			if (GetTarget())
			{
				// --------------------------------------------------
				//  If I'm blocked find a path to my goal entity
				// --------------------------------------------------
				if (HasCondition(COND_SCANNER_FLY_BLOCKED))
				{
					return SCHED_SCANNER_CHASE_TARGET;
				}

				return SCHED_SSCANNER_HOVER;
			}
			return SCHED_SCANNER_PATROL;
			break;
		}
		default:
		{

			// --------------------------------------------------
			//  If I'm blocked find a path to my goal entity
			// --------------------------------------------------
			if (HasCondition( COND_SCANNER_FLY_BLOCKED ))
			{  
				if ((GetTarget() != NULL) && !HasCondition( COND_SSCANNER_PISSED_OFF))
				{
					return SCHED_SCANNER_CHASE_TARGET;
				}
			}
			// --------------------------------------------------
			//  If I have a live enemy
			// --------------------------------------------------
			if (GetEnemy() != NULL && GetEnemy()->IsAlive())
			{
				// --------------------------------------------------
				//  If I'm open 
				// --------------------------------------------------
				if (m_nState == SSCANNER_OPEN)
				{	
					if (GetTarget()	== NULL)
					{
						return SCHED_SSCANNER_CLOSE;
					}
					else if (HasCondition( COND_SSCANNER_PISSED_OFF))
					{
						return SCHED_SSCANNER_CLOSE;
					}
					else
					{
						return SCHED_SSCANNER_HOVER;
					}
				}
				else if (m_nState == SSCANNER_CLOSED)
				{
					if (GetTarget()	!= NULL)
					{
						float fDist = (GetAbsOrigin() - GetTarget()->GetAbsOrigin()).Length();
						if (fDist <= SSCANNER_MAX_SHEILD_DIST)
						{
							return SCHED_SSCANNER_OPEN;
						}
						else
						{
							return SCHED_SSCANNER_HOVER;
						}
					}	
					else if (HasCondition( COND_SSCANNER_PISSED_OFF))
					{
						return SCHED_TAKE_COVER_FROM_ENEMY;
					}
					else
					{
						return SCHED_SSCANNER_HOVER;
					}
				}
				else
				{
					return SCHED_SSCANNER_HOVER;
				}
			}
			// --------------------------------------------------
			//  I have no enemy so patrol
			// --------------------------------------------------
			else if (GetTarget() == NULL)
			{
				return SCHED_SCANNER_PATROL;
			}
			// --------------------------------------------------
			//  I have no enemy so patrol
			// --------------------------------------------------
			else
			{
				return SCHED_SSCANNER_HOVER;
			}
			break;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_SScanner::StartTask( const Task_t *pTask )
{
	switch (pTask->iTask)
	{	
		case TASK_SSCANNER_OPEN:  
		{
			m_nState = SSCANNER_OPEN;
			TaskComplete();
			break;
		}
		case TASK_SSCANNER_CLOSE:  
		{
			m_nState = SSCANNER_CLOSED;
			TaskComplete();
			break;
		}
		// Override so can find hint nodes that are much further away
		case TASK_FIND_HINTNODE:
		{
			if (!GetHintNode())
			{
				SetHintNode(CAI_HintManager::FindHint(this, HINT_NONE, pTask->flTaskData, 5000));
			}
			if (GetHintNode())
			{
				TaskComplete();
			}
			else
			{
				// No hint node run from enemy
				SetSchedule( SCHED_RUN_FROM_ENEMY );
			}
			break;
		}
		default:
		{
			BaseClass::StartTask(pTask);
		}
	}
}


//------------------------------------------------------------------------------
// Purpose : 
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CNPC_SScanner::IsValidShieldTarget(CBaseEntity *pEntity)
{
	// Reject if already shielded by another squad member
	AISquadIter_t iter;
	for (CAI_BaseNPC *pSquadMember = m_pSquad->GetFirstMember( &iter ); pSquadMember; pSquadMember = m_pSquad->GetNextMember( &iter ) )
	{
		CNPC_SScanner *pScanner = dynamic_cast<CNPC_SScanner*>(pSquadMember);
		if (pScanner										&&
			pScanner			   != this					&&
			pScanner->Classify() == CLASS_SCANNER	)
		{
			if (pScanner->GetTarget() == pEntity)
			{
				// Reject unless I'm significantly closer
				float fNPCDist	= (pScanner->GetLocalOrigin() - pEntity->GetLocalOrigin()).Length();
				float fMyDist	= (GetLocalOrigin() - pEntity->GetLocalOrigin()).Length();
				if ((fNPCDist - fMyDist) > 150 )
				{
					// Steal from other scanner
					pScanner->StopShield();
					pScanner->SetTarget( NULL );
					return true;
				}
				return false;
			}
		}
	}
	return true;
}

void CNPC_SScanner::UpdateOnRemove(void)
{
	if (m_pShieldBeamL)
	{
		UTIL_Remove(m_pShieldBeamL);
	}
	if (m_pShieldBeamR)
	{
		UTIL_Remove(m_pShieldBeamR);
	}

	if (m_pShield)
	{
		UTIL_Remove(m_pShield);
	}

	// Chain at end to mimic destructor unwind order
	BaseClass::UpdateOnRemove();
}

//------------------------------------------------------------------------------
//
// Schedules
//
//------------------------------------------------------------------------------

//=========================================================
// > SCHED_SSCANNER_HOVER
//=========================================================
AI_DEFINE_SCHEDULE
(
	SCHED_SSCANNER_HOVER,

	"	Tasks"
	"		TASK_RESET_ACTIVITY		0"
	"		TASK_PLAY_SEQUENCE		ACTIVITY:ACT_IDLE"
	"	"
	"	Interrupts"
	"		COND_NEW_ENEMY"
	"		COND_SCANNER_FLY_BLOCKED"
	"		COND_LIGHT_DAMAGE"
);

//=========================================================
// > SCHED_SSCANNER_OPEN
//=========================================================
AI_DEFINE_SCHEDULE
(
	SCHED_SSCANNER_OPEN,

	"	Tasks"
	"		TASK_PLAY_SEQUENCE		ACTIVITY:ACT_SSCANNER_OPEN"
	"		TASK_SSCANNER_OPEN		0"
	"	"
	"	Interrupts"
);

//=========================================================
// > SCHED_SSCANNER_CLOSE
//=========================================================
AI_DEFINE_SCHEDULE
(
	SCHED_SSCANNER_CLOSE,

	"	Tasks"
	"		TASK_SSCANNER_CLOSE		0"
	"		TASK_PLAY_SEQUENCE		ACTIVITY:ACT_DISARM"
	"	"
	"	Interrupts"
);

#if 0
//=========================================================
// > SCHED_SSCANNER_PATROL
//=========================================================
AI_DEFINE_SCHEDULE
(
	SCHED_SSCANNER_PATROL,

	"	Tasks"
	"		TASK_SET_TOLERANCE_DISTANCE		48"
	"		TASK_SET_ROUTE_SEARCH_TIME		5"	// Spend 5 seconds trying to build a path if stuck
	"		TASK_GET_PATH_TO_RANDOM_NODE	2000"
	"		TASK_RUN_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0		"
	"	"
	"	Interrupts"
	"		COND_GIVE_WAY"
	"		COND_NEW_ENEMY"
	"		COND_SEE_ENEMY"
	"		COND_SEE_FEAR"
	"		COND_HEAR_COMBAT"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_PLAYER"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_PROVOKED"
);

//=========================================================
// > SCHED_SSCANNER_CHASE_ENEMY
//=========================================================
AI_DEFINE_SCHEDULE
(
	SCHED_SSCANNER_CHASE_ENEMY,

	"	Tasks"
	"		 TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_SSCANNER_PATROL"
	"		 TASK_SET_TOLERANCE_DISTANCE		120"
	"		 TASK_GET_PATH_TO_ENEMY				0"
	"		 TASK_RUN_PATH						0"
	"		 TASK_WAIT_FOR_MOVEMENT				0"
	"	"
	"	Interrupts"
	"		COND_ENEMY_DEAD"
	"		COND_SSCANNER_FLY_CLEAR"
);

//=========================================================
// > SCHED_SSCANNER_CHASE_TARGET
//=========================================================
AI_DEFINE_SCHEDULE
(
	SCHED_SSCANNER_CHASE_TARGET,

	"	Tasks"
	"		 TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_SSCANNER_HOVER"
	"		 TASK_SET_TOLERANCE_DISTANCE		120"
	"		 TASK_GET_PATH_TO_TARGET			0"
	"		 TASK_RUN_PATH						0"
	"		 TASK_WAIT_FOR_MOVEMENT				0"
	"	"
	"	"
	"	Interrupts"
	"		COND_SSCANNER_FLY_CLEAR"
	"		COND_SSCANNER_PISSED_OFF"
);
#endif // 0

