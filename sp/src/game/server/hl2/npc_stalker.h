//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef NPC_STALKER_H
#define NPC_STALKER_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"
#include "entityoutput.h"
#include "ai_behavior.h"
#include "ai_behavior_actbusy.h"
#ifdef EZ_NPCS
#include "ez2/npc_husk_base.h"
#endif

class CBeam;
class CSprite;
class CScriptedTarget;

typedef CAI_BehaviorHost<CAI_BaseNPC> CAI_BaseStalker;

#ifdef EZ_NPCS
// Stalkers use CAI_HuskSink to seamlessly integrate with husk squads
class CNPC_Stalker : public CAI_BaseStalker, public CAI_HuskSink
#else
class CNPC_Stalker : public CAI_BaseStalker
#endif
{
	DECLARE_CLASS( CNPC_Stalker, CAI_BaseStalker );

public:
	float			m_flNextAttackSoundTime;
	float			m_flNextBreatheSoundTime;
	float			m_flNextScrambleSoundTime;
	float			m_flNextNPCThink;

	// ------------------------------
	//	Laser Beam
	// ------------------------------
	int					m_eBeamPower;
	Vector				m_vLaserDir;
	Vector				m_vLaserTargetPos;
	float				m_fBeamEndTime;
	float				m_fBeamRechargeTime;
	float				m_fNextDamageTime;
	float				m_nextSmokeTime;
	float				m_bPlayingHitWall;
	float				m_bPlayingHitFlesh;
	CBeam*				m_pBeam;
	CSprite*			m_pLightGlow;
#ifdef MAPBASE
	// This is a keyvalue now, so we have to initialize the value through somewhere that isn't Spawn()
	int					m_iPlayerAggression = 0;
	bool				m_bBleed;
#else
	int					m_iPlayerAggression;
#endif
	float				m_flNextScreamTime;

#ifdef MAPBASE
	void				UpdateOnRemove( void );
#endif

	void				KillAttackBeam(void);
	void				DrawAttackBeam(void);
	void				CalcBeamPosition(void);
	Vector				LaserStartPosition(Vector vStalkerPos);

	Vector				m_vLaserCurPos;			// Last position successfully burned
	bool				InnateWeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	
	// ------------------------------
	//	Dormancy
	// ------------------------------
	CAI_Schedule*	WakeUp(void);
	void			GoDormant(void);

public:
	void			Spawn( void );
	void			Precache( void );
	bool			CreateBehaviors();
	float			MaxYawSpeed( void );
	Class_T			Classify ( void );

	void			PrescheduleThink();

	bool			IsValidEnemy( CBaseEntity *pEnemy );
	
	void			StartTask( const Task_t *pTask );
	void			RunTask( const Task_t *pTask );
	virtual int		SelectSchedule ( void );
	virtual int		TranslateSchedule( int scheduleType );
	int				OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void			OnScheduleChange();

	void			StalkerThink(void);
	void			NotifyDeadFriend( CBaseEntity *pFriend );

	int				MeleeAttack1Conditions ( float flDot, float flDist );
	int				RangeAttack1Conditions ( float flDot, float flDist );
	void			HandleAnimEvent( animevent_t *pEvent );

	bool			FValidateHintType(CAI_Hint *pHint);
	Activity		GetHintActivity( short sHintType, Activity HintsActivity );
	float			GetHintDelay( short sHintType );

	void			IdleSound( void );
	void			DeathSound( const CTakeDamageInfo &info );
	void			PainSound( const CTakeDamageInfo &info );

	void			Event_Killed( const CTakeDamageInfo &info );
#ifdef MAPBASE
	void			TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
#endif
	void			DoSmokeEffect( const Vector &position );

	void			AddZigZagToPath(void);
	void			StartAttackBeam();
	void			UpdateAttackBeam();

#ifdef EZ_NPCS
	// Stalkers use CAI_HuskSink to seamlessly integrate with husk squads
	const char *GetBaseNPCClassname() override { return "npc_stalker"; }
	
	int GetHuskAggressionLevel() override { return m_iPlayerAggression >= 1 ? HUSK_AGGRESSION_LEVEL_ANGRY : HUSK_AGGRESSION_LEVEL_CALM; }
	int GetHuskCognitionFlags() override { return 0; }
	
	void MakeCalm( CBaseEntity *pActivator ) override { m_iPlayerAggression = 0; }
	void MakeSuspicious( CBaseEntity *pActivator ) override { }
	void MakeAngry( CBaseEntity *pActivator ) override { m_iPlayerAggression++; }
	
	bool IsSuspicious() override { return false; }
	bool IsSuspiciousOrHigher() override { return IsAngry(); }
	bool IsAngry() override { return m_iPlayerAggression >= 1; }

	bool IsPassiveTarget( CBaseEntity *pTarget ) override { return false; };
	bool IsHostileOverrideTarget( CBaseEntity *pTarget ) override { return false; };
#endif

	CNPC_Stalker(void);

	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

private:
	CAI_ActBusyBehavior		m_ActBusyBehavior;
};

#endif // NPC_STALKER_H
