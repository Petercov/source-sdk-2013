//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//


#ifndef NPC_ATTACKCHOPPER_H
#define NPC_ATTACKCHOPPER_H

#ifdef _WIN32
#pragma once
#endif

#include "cbasehelicopter.h"
#include "basegrenade_shared.h"
#include "ai_spotlight.h"

class CSprite;

#define SF_HELICOPTER_LOUD_ROTOR_SOUND		0x00010000
#define SF_HELICOPTER_ELECTRICAL_DRONE		0x00020000
#define SF_HELICOPTER_LIGHTS				0x00040000
#define SF_HELICOPTER_IGNORE_AVOID_FORCES	0x00080000
#define SF_HELICOPTER_AGGRESSIVE			0x00100000
#define SF_HELICOPTER_LONG_SHADOW			0x00200000
#ifdef MAPBASE
#define SF_HELICOPTER_AIM_WITH_GUN_OFF		0x00400000
#endif

enum
{
	MAX_HELICOPTER_LIGHTS = 3,
};

enum
{
	SF_GRENADE_HELICOPTER_MEGABOMB = 0x1,
};

//-----------------------------------------------------------------------------
// The bombs the attack helicopter drops 
//-----------------------------------------------------------------------------
class CGrenadeHelicopter : public CBaseGrenade
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CGrenadeHelicopter, CBaseGrenade);

	enum
	{
		SKIN_REGULAR,
		SKIN_DUD,
	};

	virtual void Precache();
	virtual void Spawn();
	virtual void UpdateOnRemove();
	virtual void OnEntityEvent(EntityEvent_t event, void* pEventData);
	virtual void PhysicsSimulate(void);
	virtual float GetShakeAmplitude(void) { return 25.0; }
	virtual float GetShakeRadius(void);
	virtual int OnTakeDamage(const CTakeDamageInfo& info);
	virtual void VPhysicsCollision(int index, gamevcollisionevent_t* pEvent);
	void		 SetExplodeOnContact(bool bExplode) { m_bExplodeOnContact = bExplode; }

	virtual QAngle PreferredCarryAngles(void) { return QAngle(-12, 98, 55); }
	virtual bool HasPreferredCarryAnglesForPlayer(CBasePlayer* pPlayer) { return true; }

	float GetBombLifetime();

#ifdef HL2_EPISODIC
	virtual void OnPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);
	virtual void OnPhysGunDrop(CBasePlayer* pPhysGunUser, PhysGunDrop_t reason);
	virtual Class_T Classify(void) { return CLASS_MISSILE; }
	void SetCollisionObject(CBaseEntity* pEntity) { m_hCollisionObject = pEntity; }
	void SendMissEvent();
	bool IsThrownByPlayer();

	virtual bool	ShouldPuntUseLaunchForces(PhysGunForce_t reason) { return (reason == PHYSGUN_FORCE_LAUNCHED); }
	virtual Vector  PhysGunLaunchVelocity(const Vector& forward, float flMass);

	void InputExplodeIn(inputdata_t& inputdata);
#endif // HL2_EPISODIC

private:
	// Pow!
	void DoExplosion(const Vector& vecOrigin, const Vector& vecVelocity);
	void ExplodeThink();
	void RampSoundThink();
	void WarningBlinkerThink();
	void StopWarningBlinker();
	void AnimateThink();
	void ExplodeConcussion(CBaseEntity* pOther);
	void BecomeActive();
	void ResolveFlyCollisionCustom(trace_t& trace, Vector& vecVelocity);

	bool m_bActivated;
	bool m_bExplodeOnContact;
	CSoundPatch* m_pWarnSound;

	EHANDLE m_hWarningSprite;
	bool m_bBlinkerAtTop;


#ifdef HL2_EPISODIC
	float m_flLifetime;
	EHANDLE	m_hCollisionObject;	// Pointer to object we re-enable collisions with when picked up
	bool m_bPickedUp;
	float m_flBlinkFastTime;
	COutputEvent m_OnPhysGunOnlyPickup;
#endif // HL2_EPISODIC
};

//-----------------------------------------------------------------------------
// The bombs the attack helicopter drops 
//-----------------------------------------------------------------------------
class CBombDropSensor : public CBaseEntity
{
	DECLARE_DATADESC();

public:
	DECLARE_CLASS(CBombDropSensor, CBaseEntity);

	void Spawn();

	// Drop a bomb at a particular location
	void	InputDropBomb(inputdata_t& inputdata);
	void	InputDropBombStraightDown(inputdata_t& inputdata);
	void	InputDropBombAtTarget(inputdata_t& inputdata);
	void	InputDropBombAtTargetAlways(inputdata_t& inputdata);
	void	InputDropBombDelay(inputdata_t& inputdata);
};

//-----------------------------------------------------------------------------
// The attack helicopter 
//-----------------------------------------------------------------------------
class CNPC_AttackHelicopter : public CBaseHelicopter
{
public:
	DECLARE_CLASS(CNPC_AttackHelicopter, CBaseHelicopter);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	CNPC_AttackHelicopter();
	~CNPC_AttackHelicopter();

	virtual void	Precache(void);
	virtual void	Spawn(void);
	virtual void	Activate(void);
	virtual bool	CreateComponents();
	virtual int		ObjectCaps();

#ifdef HL2_EPISODIC
	virtual bool	CreateVPhysics(void);
#endif // HL2_EPISODIC

	virtual void	UpdateOnRemove();
	virtual void	StopLoopingSounds();

	int		BloodColor(void) { return DONT_BLEED; }
	Class_T Classify(void) { return CLASS_COMBINE_GUNSHIP; }
	virtual int	OnTakeDamage_Alive(const CTakeDamageInfo& info);
	virtual void TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr, CDmgAccumulator* pAccumulator);
	virtual int OnTakeDamage(const CTakeDamageInfo& info);

	// Shot spread
	virtual Vector GetAttackSpread(CBaseCombatWeapon* pWeapon, CBaseEntity* pTarget);

	// More Enemy visibility check
	virtual bool FVisible(CBaseEntity* pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity** ppBlocker = NULL);

	// Think!
	virtual void PrescheduleThink(void);

	// Purpose: Set the gunship's paddles flailing!
	virtual void Event_Killed(const CTakeDamageInfo& info);

	// Drop a bomb at a particular location
	void	InputDropBomb(inputdata_t& inputdata);
	void	InputDropBombStraightDown(inputdata_t& inputdata);
	void	InputDropBombAtTarget(inputdata_t& inputdata);
	void	InputDropBombAtTargetAlways(inputdata_t& inputdata);
	void	InputDropBombAtTargetInternal(inputdata_t& inputdata, bool bCheckFairness);
	void	InputDropBombDelay(inputdata_t& inputdata);
	void	InputStartCarpetBombing(inputdata_t& inputdata);
	void	InputStopCarpetBombing(inputdata_t& inputdata);

	virtual void SetTransmit(CCheckTransmitInfo* pInfo, bool bAlways);
	virtual const char* GetTracerType(void);

	virtual void DoImpactEffect(trace_t& tr, int nDamageType);
	virtual void DoMuzzleFlash(void);

	// FIXME: Work this back into the base class
	virtual bool ShouldUseFixedPatrolLogic() { return true; }

protected:

	int m_poseWeapon_Pitch, m_poseWeapon_Yaw, m_poseRudder;
	virtual void	PopulatePoseParameters(void);

	enum GunState_t
	{
		GUN_STATE_IDLE = 0,
		GUN_STATE_CHARGING,
		GUN_STATE_FIRING,
	};

	// Gets the max speed of the helicopter
	virtual float GetMaxSpeed();
	virtual float GetMaxSpeedFiring();

	// Startup the chopper
	virtual void Startup();

	virtual void InitializeRotorSound(void);

	// Weaponry
	bool	FireGun(void);

	// Movement:	
	virtual void Flight(void);

	// Changes the main thinking method of helicopters
	virtual void Hunt(void);

	// For scripted times where it *has* to shoot
	void	InputResetIdleTime(inputdata_t& inputdata);
	void	InputSetHealthFraction(inputdata_t& inputdata);
	void	InputStartBombExplodeOnContact(inputdata_t& inputdata);
	void	InputStopBombExplodeOnContact(inputdata_t& inputdata);

	void	InputEnableAlwaysTransition(inputdata_t& inputdata);
	void	InputDisableAlwaysTransition(inputdata_t& inputdata);
	void 	InputOutsideTransition(inputdata_t& inputdata);
	void 	InputSetOutsideTransitionTarget(inputdata_t& inputdata);

	// Turns off the gun
	void	InputGunOff(inputdata_t& inputdata);

	// Vehicle attack modes
	void	InputStartBombingVehicle(inputdata_t& inputdata);
	void	InputStartTrailingVehicle(inputdata_t& inputdata);
	void	InputStartDefaultBehavior(inputdata_t& inputdata);
	void	InputStartAlwaysLeadingVehicle(inputdata_t& inputdata);

	// Deadly shooting, tex!
	void	InputEnableDeadlyShooting(inputdata_t& inputdata);
	void	InputDisableDeadlyShooting(inputdata_t& inputdata);
	void	InputStartNormalShooting(inputdata_t& inputdata);
	void	InputStartLongCycleShooting(inputdata_t& inputdata);
	void	InputStartContinuousShooting(inputdata_t& inputdata);
	void	InputStartFastShooting(inputdata_t& inputdata);

	int		GetShootingMode();
	bool	IsDeadlyShooting();

	// Bombing suppression
	void	InputEnableBombing(inputdata_t& inputdata);
	void 	InputDisableBombing(inputdata_t& inputdata);

	// Visibility tests
	void	InputDisablePathVisibilityTests(inputdata_t& inputdata);
	void 	InputEnablePathVisibilityTests(inputdata_t& inputdata);

	// Death, etc.
	void	InputSelfDestruct(inputdata_t& inputdata);

	// Enemy visibility check
	CBaseEntity* FindTrackBlocker(const Vector& vecViewPoint, const Vector& vecTargetPos);

	// Special path navigation when we explicitly want to follow a path
	void UpdateFollowPathNavigation();

	// Find interesting nearby things to shoot
	int BuildMissTargetList(int nCount, CBaseEntity** ppMissCandidates);

	// Shoot when the player's your enemy :			 
	void ShootAtPlayer(const Vector& vBasePos, const Vector& vGunDir);

	// Shoot when the player's your enemy + he's driving a vehicle
	void ShootAtVehicle(const Vector& vBasePos, const Vector& vGunDir);

	// Shoot where we're facing
	void ShootAtFacingDirection(const Vector& vBasePos, const Vector& vGunDir, bool bFirstShotAccurate);

	// Updates the facing direction
	virtual void UpdateFacingDirection(const Vector& vecActualDesiredPosition);

	// Various states of the helicopter firing...
	bool PoseGunTowardTargetDirection(const Vector& vTargetDir);

	// Compute the position to fire at (vehicle + non-vehicle case)
	void ComputeFireAtPosition(Vector* pVecActualTargetPosition);
	void ComputeVehicleFireAtPosition(Vector* pVecActualTargetPosition);

	// Various states of the helicopter firing...
	bool DoGunIdle(const Vector& vecGunDir, const Vector& vTargetDir);
	bool DoGunCharging();
	bool DoGunFiring(const Vector& vBasePos, const Vector& vGunDir, const Vector& vecFireAtPosition);
	void FireElectricityGun();

	// Chooses a point within the circle of death to fire in
	void PickDirectionToCircleOfDeath(const Vector& vBasePos, const Vector& vecFireAtPosition, Vector* pResult);

	// Gets a vehicle the enemy is in (if any)
	CBaseEntity* GetEnemyVehicle();

	// Updates the perpendicular path distance for the chopper	
	float UpdatePerpPathDistance(float flMaxPathOffset);

	// Purpose :
	void UpdateEnemyLeading(void);

	// Drop those bombs!
	void DropBombs();

	// Should we drop those bombs?
	bool ShouldDropBombs(void);

	// Returns the max firing distance
	float GetMaxFiringDistance();

	// Make sure we don't hit too many times
	void FireBullets(const FireBulletsInfo_t& info);

	// Is it "fair" to drop this bomb?
	bool IsBombDropFair(const Vector& vecBombStartPos, const Vector& vecVelocity);

	// Actually drops the bomb
	void CreateBomb(bool bCheckForFairness = true, Vector* pVecVelocity = NULL, bool bMegaBomb = false);
	CGrenadeHelicopter* SpawnBombEntity(const Vector& vecPos, const Vector& vecVelocity); // Spawns the bomb entity and sets it up

	// Deliberately aims as close as possible w/o hitting
	void AimCloseToTargetButMiss(CBaseEntity* pTarget, float flMinDist, float flMaxDist, const Vector& shootOrigin, Vector* pResult);

	// Pops a shot inside the circle of death using the burst rules
	void ShootInsideCircleOfDeath(const Vector& vBasePos, const Vector& vecFireAtPosition);

	// How easy is the target to hit?
	void UpdateTargetHittability();

	// Add a smoke trail since we've taken more damage
	void AddSmokeTrail(const Vector& vecPos);

	// Destroy all smoke trails
	void DestroySmokeTrails();

	// Creates the breakable husk of an attack chopper
	void CreateChopperHusk();

	// Pow!
	void ExplodeAndThrowChunk(const Vector& vecExplosionPos);

	// Drop a corpse!
	void DropCorpse(int nDamage);

	// Should we trigger a damage effect?
	bool ShouldTriggerDamageEffect(int nPrevHealth, int nEffectCount) const;

	// Become indestructible
	void InputBecomeIndestructible(inputdata_t& inputdata);

	// Purpose :
	float CreepTowardEnemy(float flSpeed, float flMinSpeed, float flMaxSpeed, float flMinDist, float flMaxDist);

	// Start bullrush
	void InputStartBullrushBehavior(inputdata_t& inputdata);

	void GetMaxSpeedAndAccel(float* pMaxSpeed, float* pAccelRate);
	void ComputeAngularVelocity(const Vector& vecGoalUp, const Vector& vecFacingDirection);
	void ComputeVelocity(const Vector& deltaPos, float flAdditionalHeight, float flMinDistFromSegment, float flMaxDistFromSegment, Vector* pVecAccel);
	void FlightDirectlyOverhead(void);

	// Methods related to computing leading distance
	float ComputeBombingLeadingDistance(float flSpeed, float flSpeedAlongPath, bool bEnemyInVehicle);
	float ComputeBullrushLeadingDistance(float flSpeed, float flSpeedAlongPath, bool bEnemyInVehicle);

	bool IsCarpetBombing() { return m_bIsCarpetBombing == true; }

	// Update the bullrush state
	void UpdateBullrushState(void);

	// Whether to shoot at or bomb an idle player
	bool ShouldBombIdlePlayer(void);

	// Different bomb-dropping behavior
	void BullrushBombs();

	// Switch to idle
	void SwitchToBullrushIdle(void);

	// Secondary mode
	void SetSecondaryMode(int nMode, bool bRetainTime = false);
	bool IsInSecondaryMode(int nMode);
	float SecondaryModeTime() const;

	// Should the chopper shoot the idle player?
	bool ShouldShootIdlePlayerInBullrush();

	// Shutdown shooting during bullrush
	void ShutdownGunDuringBullrush();

	// Updates the enemy
	virtual float	EnemySearchDistance();

	// Prototype zapper
	bool IsValidZapTarget(CBaseEntity* pTarget);
	void CreateZapBeam(const Vector& vecTargetPos);
	void CreateEntityZapEffect(CBaseEntity* pEnt);

	// Blink lights
	void BlinkLightsThink();

	// Spotlights
	void SpotlightThink();
	void SpotlightStartup();
	void SpotlightShutdown();

protected:

	CBaseEntity* GetCrashPoint() { return m_hCrashPoint.Get(); }

private:
	enum
	{
		ATTACK_MODE_DEFAULT = 0,
		ATTACK_MODE_BOMB_VEHICLE,
		ATTACK_MODE_TRAIL_VEHICLE,
		ATTACK_MODE_ALWAYS_LEAD_VEHICLE,
		ATTACK_MODE_BULLRUSH_VEHICLE,
	};

	enum
	{
		MAX_SMOKE_TRAILS = 5,
		MAX_EXPLOSIONS = 13,
		MAX_CORPSES = 2,
	};

	enum
	{
		BULLRUSH_MODE_WAIT_FOR_ENEMY = 0,
		BULLRUSH_MODE_GET_DISTANCE,
		BULLRUSH_MODE_DROP_BOMBS_FIXED_SPEED,
		BULLRUSH_MODE_DROP_BOMBS_FOLLOW_PLAYER,
		BULLRUSH_MODE_SHOOT_GUN,
		BULLRUSH_MODE_MEGA_BOMB,
		BULLRUSH_MODE_SHOOT_IDLE_PLAYER,
	};

	enum
	{
		SHOOT_MODE_DEFAULT = 0,
		SHOOT_MODE_LONG_CYCLE,
		SHOOT_MODE_CONTINUOUS,
		SHOOT_MODE_FAST,
	};

#ifdef HL2_EPISODIC
protected:
	virtual void InitBoneFollowers(void);
	CBoneFollowerManager	m_BoneFollowerManager;
#endif // HL2_EPISODIC

	CAI_Spotlight	m_Spotlight;
	Vector		m_angGun;
	QAngle		m_vecAngAcceleration;
	int			m_iAmmoType;
	float		m_flLastCorpseFall;

	GunState_t	m_nGunState;
	float		m_flChargeTime;
	float		m_flIdleTimeDelay;
	int			m_nRemainingBursts;
	int			m_nGrenadeCount;
	float 		m_flPathOffset;
	float 		m_flAcrossTime;
	float		m_flCurrPathOffset;
	int			m_nBurstHits;
	int			m_nMaxBurstHits;
	float		m_flCircleOfDeathRadius;
	int			m_nAttackMode;
	float		m_flInputDropBombTime;
	CHandle<CBombDropSensor>	m_hSensor;
	float		m_flAvoidMetric;
	AngularImpulse m_vecLastAngVelocity;
	CHandle<CBaseEntity>	m_hSmokeTrail[MAX_SMOKE_TRAILS];
	int			m_nSmokeTrailCount;
	bool		m_bIndestructible;
	float		m_flGracePeriod;
	bool		m_bBombsExplodeOnContact;

	bool		m_bNonCombat;

	int			m_nNearShots;
	int			m_nMaxNearShots;

	// Bomb dropping attachments
	int			m_nGunTipAttachment;
	int			m_nGunBaseAttachment;
	int			m_nBombAttachment;
	int			m_nSpotlightAttachment;
	float		m_flLastFastTime;

	// Secondary modes
	int			m_nSecondaryMode;
	float		m_flSecondaryModeStartTime;

	// Bullrush behavior
	bool		m_bRushForward;
	float		m_flBullrushAdditionalHeight;
	int			m_nBullrushBombMode;
	float		m_flNextBullrushBombTime;
	float		m_flNextMegaBombHealth;

	// Shooting method
	int			m_nShootingMode;
	bool		m_bDeadlyShooting;

	// Bombing suppression
	bool		m_bBombingSuppressed;

	// Blinking lights
	CHandle<CSprite> m_hLights[MAX_HELICOPTER_LIGHTS];
	bool		m_bShortBlink;

	// Path behavior
	bool		m_bIgnorePathVisibilityTests;

	// Teleport
	bool		m_bAlwaysTransition;
	string_t	m_iszTransitionTarget;

	// Special attacks
	bool		m_bIsCarpetBombing;

	// Fun damage effects
	float		m_flGoalRollDmg;
	float		m_flGoalYawDmg;

	// Sounds
	CSoundPatch* m_pGunFiringSound;

	// Outputs
#ifndef MAPBASE
	COutputInt	m_OnHealthChanged;
#endif
	COutputEvent m_OnShotDown;
#ifdef MAPBASE
	COutputEHANDLE m_OutBomb;
#endif

	// Crashing
	EHANDLE		m_hCrashPoint;
};


//-----------------------------------------------------------------------------
// Creates an avoidance sphere
//-----------------------------------------------------------------------------
CBaseEntity *CreateHelicopterAvoidanceSphere( CBaseEntity *pParent, int nAttachment, float flRadius, bool bAvoidBelow = false );

// Chopper gibbage
void Chopper_BecomeChunks( CBaseEntity *pChopper );
void Chopper_CreateChunk( CBaseEntity *pChopper, const Vector &vecChunkPos, const QAngle &vecChunkAngles, const char *pszChunkName, bool bSmall );
void Chopper_PrecacheChunks( CBaseEntity *pChopper );

#endif // NPC_ATTACKCHOPPER_H


