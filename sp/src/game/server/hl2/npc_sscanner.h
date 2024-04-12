//=========== (C) Copyright 1999 Valve, L.L.C. All rights reserved. ===========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose:		Base combat character with no AI
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================

#ifndef	NPC_SSCANNER_H
#define	NPC_SSCANNER_H

#include "npc_basescanner.h"

//-----------------------------------------------------------------------------
// Attachment points.
//-----------------------------------------------------------------------------

class CSprite;
class CBeam;
class CShieldBeam;
class CScannerShield;

enum SScannerState_t
{
	SSCANNER_OPEN,	
	SSCANNER_CLOSED,
};

//-----------------------------------------------------------------------------
// Sheild Scanner 
//-----------------------------------------------------------------------------
class CNPC_SScanner : public CNPC_BaseScanner
{
	DECLARE_CLASS( CNPC_SScanner, CNPC_BaseScanner);

public:
	CNPC_SScanner();

	void			Event_Killed( const CTakeDamageInfo &info );
	int				OnTakeDamage_Alive( const CTakeDamageInfo &info );
	void			Gib(void);
	void			NPCThink( void );

	virtual char* GetScannerSoundPrefix(void) { return "NPC_SScanner"; }
 	virtual int		SelectSchedule(void);
	virtual int		TranslateSchedule( int scheduleType );
	Activity		NPC_TranslateActivity( Activity eNewActivity );
	void			OnScheduleChange();

	bool			OverrideMove(float flInterval);
	void			MoveToTarget(float flInterval, const Vector &MoveTarget);
	void			MoveToEntity(float flInterval, CBaseEntity* pMoveTarget, float fMinRange, float fMaxRange);
	float			MinGroundDist(void);

	void			Precache(void);
	void			RunTask( const Task_t *pTask );
	void			Spawn(void);
	void			StartTask( const Task_t *pTask );
	void			UpdateOnRemove(void);

	DEFINE_CUSTOM_AI;

	DECLARE_DATADESC();

	enum
	{
		//-----------------------------------------------------------------------------
		// SScanner schedules.
		//-----------------------------------------------------------------------------
		SCHED_SSCANNER_HOVER = BaseClass::NEXT_SCHEDULE,
		//SCHED_SSCANNER_PATROL,
		//SCHED_SSCANNER_CHASE_ENEMY,
		//SCHED_SSCANNER_CHASE_TARGET,
		SCHED_SSCANNER_OPEN,
		SCHED_SSCANNER_CLOSE,

		//-----------------------------------------------------------------------------
		// Custom tasks.
		//-----------------------------------------------------------------------------
		TASK_SSCANNER_OPEN = BaseClass::NEXT_TASK,
		TASK_SSCANNER_CLOSE,

		//-----------------------------------------------------------------------------
		// Custom Conditions
		//-----------------------------------------------------------------------------
		COND_SSCANNER_PISSED_OFF = BaseClass::NEXT_CONDITION,
	};

protected:
	float			m_lastHurtTime;

	// ==================
	// Shield
	// ==================
	SScannerState_t m_nState;
	bool			m_bShieldsDisabled;
	CHandle<CScannerShield>	m_pShield;
	CShieldBeam*	m_pShieldBeamL;		
	CShieldBeam*	m_pShieldBeamR;		
	float			m_fNextShieldCheckTime;
	CBaseEntity*	PickShieldEntity(void);
	void			PowerShield(void);
	void			StopShield(void);
	void			UpdateShields(void);
	bool			IsValidShieldTarget(CBaseEntity *pEntity);
	bool			IsValidShieldCover( Vector &vCoverPos );
	bool			SetShieldCoverPosition( void );

	// ==================
	// Movement variables.
	// ==================
	Vector			m_vCoverPosition;		// Position to go for cover
};

#endif	//NPC_SSCANNER_H
