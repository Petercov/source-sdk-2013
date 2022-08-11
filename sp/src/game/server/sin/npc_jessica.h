#ifndef SIN_JESSICA_H
#define SIN_JESSICA_H
#ifdef WIN32
#pragma once
#endif // WIN32

#include "npc_playercompanion.h"

#define SF_JESSICA_INVINCIBLE		( 1 << 16 ) // 65536

class CNPC_SinJessica : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS(CNPC_SinJessica, CNPC_PlayerCompanion);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	enum 
	{
		SCHED_JESS_THROW_ITEM = BaseClass::NEXT_SCHEDULE,

		COND_JESS_THROW_ITEM = BaseClass::NEXT_CONDITION,
	};

	void	Spawn(void);
	void	SelectModel();
	Class_T Classify(void);
	void	UseFunc(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	InputThrowItemAtPlayer(inputdata_t& inputdata);
	void 	HandleAnimEvent(animevent_t* pEvent);

	void	GatherConditions();
	int		SelectSchedulePriorityAction();
	void	BuildScheduleTestBits();

private:
	int m_iThrowAmmoPrimary;
	int m_iThrowAmmoSecondary;
	string_t m_iszThrowItem;
};
#endif // !SIN_JESSICA_H
