//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Heavy machine gun
//
//=============================================================================

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "NPCevent.h"
#include "basecombatcharacter.h"
#include "AI_BaseNPC.h"

#ifdef MAPBASE
// Allows Weapon_BackupActivity() to access the SMG1's activity table.
acttable_t* GetOICWActtable();
int GetOICWActtableCount();
#endif

class CWeaponHMG1 : public CHLSelectFireMachineGun
{
public:
	DECLARE_CLASS( CWeaponHMG1, CHLSelectFireMachineGun );

	DECLARE_SERVERCLASS();

	CWeaponHMG1();
	
	void	Precache( void );
	bool	Deploy( void );
	bool	Reload( void );

	int		GetMinBurst() { return 6; }
	int		GetMaxBurst() { return 10; }

	int CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread( void )
	{
		static Vector cone = VECTOR_CONE_10DEGREES;
		return cone;
	}

	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
	{
		switch( pEvent->event )
		{
			case EVENT_WEAPON_HMG1:
			{
				Vector vecShootOrigin, vecShootDir;
				vecShootOrigin = pOperator->Weapon_ShootPosition( );

				CAI_BaseNPC *npc = pOperator->MyNPCPointer();
				Vector vecSpread;
				if (npc)
				{
					vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );
					vecSpread = VECTOR_CONE_PRECALCULATED;
				}
				else
				{
					AngleVectors( pOperator->GetLocalAngles(), &vecShootDir );
					vecSpread = GetBulletSpread();
				}
				WeaponSound(SINGLE_NPC);
				pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, vecSpread, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );
				pOperator->DoMuzzleFlash();
				m_iClip1 -= 1;
			}
			break;
			default:
				CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
				break;
		}
	}

#ifdef MAPBASE
	virtual acttable_t* GetBackupActivityList() { return GetOICWActtable(); }
	virtual int			GetBackupActivityListCount() { return GetOICWActtableCount(); }
#endif // MAPBASE

	DECLARE_ACTTABLE();
};

IMPLEMENT_SERVERCLASS_ST(CWeaponHMG1, DT_WeaponHMG1)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_hmg1, CWeaponHMG1 );

acttable_t	CWeaponHMG1::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_HMG1,			true },

#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_RELOAD,					ACT_RELOAD_HMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_HMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_HMG1,			false },

	{ ACT_WALK,						ACT_WALK_HMG1,					true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_HMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_HMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_HMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_HMG1_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_HMG1_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_HMG1,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_HMG1_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_HMG1_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_HMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_HMG1_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_HMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_HMG1_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_HMG1_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_HMG1,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_HMG1_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_HMG1_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_HMG1,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_HMG1,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_HMG1,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_HMG1,	false },
	{ ACT_COVER_LOW,				ACT_COVER_HMG1_LOW,				true },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_HMG1_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_HMG1_LOW,		false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_HMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_HMG1,		true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_HMG1_GRENADE, true },

	{ ACT_ARM,						ACT_ARM_RIFLE,					false },
	{ ACT_DISARM,					ACT_DISARM_RIFLE,				false },
#endif

#if EXPANDED_HL2_COVER_ACTIVITIES
#if EXPANDED_HL2_WEAPON_ACTIVITIES
	{ ACT_RANGE_AIM_MED,			ACT_RANGE_AIM_HMG1_MED,			false },
	{ ACT_RANGE_ATTACK1_MED,		ACT_RANGE_ATTACK_HMG1_MED,		false },
#endif
	{ ACT_COVER_WALL_R,				ACT_COVER_WALL_R_RIFLE,			false },
	{ ACT_COVER_WALL_L,				ACT_COVER_WALL_L_RIFLE,			false },
	{ ACT_COVER_WALL_LOW_R,			ACT_COVER_WALL_LOW_R_RIFLE,		false },
	{ ACT_COVER_WALL_LOW_L,			ACT_COVER_WALL_LOW_L_RIFLE,		false },
#endif

#ifdef MAPBASE
#if EXPANDED_HL2_UNUSED_WEAPON_ACTIVITIES
	// HL2:DM activities (for third-person animations in SP)
	{ ACT_HL2MP_IDLE,					ACT_HL2MP_IDLE_HMG1,                    false },
	{ ACT_HL2MP_RUN,					ACT_HL2MP_RUN_HMG1,                    false },
	{ ACT_HL2MP_IDLE_CROUCH,			ACT_HL2MP_IDLE_CROUCH_HMG1,            false },
	{ ACT_HL2MP_WALK_CROUCH,			ACT_HL2MP_WALK_CROUCH_HMG1,            false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK,	ACT_HL2MP_GESTURE_RANGE_ATTACK_HMG1,    false },
	{ ACT_HL2MP_GESTURE_RELOAD,			ACT_HL2MP_GESTURE_RELOAD_SMG1,        false },
	{ ACT_HL2MP_JUMP,					ACT_HL2MP_JUMP_HMG1,                    false },
	{ ACT_HL2MP_WALK,					ACT_HL2MP_WALK_HMG1,						false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK2,	ACT_HL2MP_GESTURE_RANGE_ATTACK2_HMG1,	false },
#endif
#endif
};

IMPLEMENT_ACTTABLE(CWeaponHMG1);

//=========================================================
CWeaponHMG1::CWeaponHMG1( )
{
}

void CWeaponHMG1::Precache( void )
{
	BaseClass::Precache();
}

bool CWeaponHMG1::Deploy( void )
{
	return BaseClass::Deploy();
}

bool CWeaponHMG1::Reload( void )
{
	bool fRet;

	fRet = BaseClass::Reload();
	if ( fRet )
	{
		WeaponSound( RELOAD );
	}

	return fRet;
}