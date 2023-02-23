//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Base combat character with no AI
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#ifndef HL1_AI_BASENPC_H
#define HL1_AI_BASENPC_H

#pragma warning(push)
#include <set>
#pragma warning(pop)

#ifdef _WIN32
#pragma once
#endif


#include "ai_basenpc.h"
#include "AI_Motor.h"
//=============================================================================
// >> CHL1NPCTalker
//=============================================================================

class CHL1BaseNPC : public CAI_BaseNPC
{
	DECLARE_CLASS( CHL1BaseNPC, CAI_BaseNPC );

public:
	CHL1BaseNPC( void )
	{

	}

	void	TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr, CDmgAccumulator *pAccumulator );
	bool	ShouldGib( const CTakeDamageInfo &info );
	virtual bool	CorpseGib( const CTakeDamageInfo &info );

	bool	HasAlienGibs( void );
	bool	HasHumanGibs( void );

	void	Precache( void );

	int		IRelationPriority( CBaseEntity *pTarget );
	bool	NoFriendlyFire( void );

	void	EjectShell( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int iType );

	virtual int		SelectDeadSchedule();

	virtual const char* GetBurningParticle() { return "burning_gib_01"; }
	virtual	bool		AllowedToIgnite(void) { return true; }

protected:
	virtual void		PopulatePoseParameters(void);
};

#endif //HL1_AI_BASENPC_H