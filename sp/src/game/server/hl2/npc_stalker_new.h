//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef NPC_ALPHASTALKER_H
#define NPC_ALPHASTALKER_H


#include	"ai_basenpc.h"
//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	ALPHASTALKER_AE_ATTACK		1
#define	ALPHASTALKER_AE_ATTACK_BOTH		2
#define ALPHASTALKER_AE_JUMPATTACK      3


#define ALPHASTALKER_FLINCH_DELAY			2		// at most one flinch every n secs

#define ALPHASTALKER_LEAP_DELAY			random->RandomInt(1.5, 5)		// randomize leap attack cooldown

int ACT_RUN_ANGRY;



//=========================================================
//=========================================================
class CNPC_AlphaStalker : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_AlphaStalker, CAI_BaseNPC);
public:

	void Spawn(void);
	void Precache(void);
	float MaxYawSpeed(void) { return 120.0f; };
	Class_T Classify(void);
	void HandleAnimEvent(animevent_t *pEvent);
	//	int IgnoreConditions ( void );
	virtual int		TranslateSchedule(int scheduleType);
	Activity		NPC_TranslateActivity(Activity eNewActivity);

	float m_flNextFlinch;
//	bool		 PlayerFlashlightOnMyEyes(CBasePlayer *pPlayer);
	void PainSound(const CTakeDamageInfo &info);
	void DeathSound(const CTakeDamageInfo &info);
	void AlertSound(void);
	void IdleSound(void);
	//void AttackSound(void);
	void StartTask(const Task_t *pTask);
	int  SelectSchedule(void);
	void			AddZigZagToPath(void);
	// No range attacks
//	BOOL CheckRangeAttack1(float flDot, float flDist) { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) { return FALSE; }
	void LeapTouch(CBaseEntity *pOther);
	void LeapSound(void);
	void LeapDamage(CBaseEntity *pOther);
	int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	int m_iBravery;
	void RemoveIgnoredConditions(void);
	int MeleeAttack1Conditions(float flDot, float flDist);
	int RangeAttack1Conditions(float flDot, float flDist);
	bool IsJumpLegal(const Vector &startPos, const Vector &apex, const Vector &endPos) const;
	mutable float	m_flJumpDist;
	bool	m_bIsFlashlightBlind;
	float m_flLastRangeTime;
//	virtual int		TranslateSchedule(int scheduleType);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	virtual float		HearingSensitivity(void)		{ return 1.15; }
	enum
	{
		SCHED_ALPHASTALKER_TAKECOVER = BaseClass::NEXT_SCHEDULE,
		SCHED_ALPHASTALKER_INCOVER,
		SCHED_ALPHASTALKER_FLANK,
	};

	//=========================================================
	enum
	{
		TASK_STALKER_ZIGZAG = BaseClass::NEXT_TASK,
	};

};



#endif //NPC_ALPHASTALKER_H