#include "cbase.h"
#include "npc_jessica.h"

int AE_JESSICA_THROW_ITEM;
int ACT_THROW_ITEM;

BEGIN_DATADESC(CNPC_SinJessica)

DEFINE_USEFUNC(UseFunc),

DEFINE_FIELD(m_iszThrowItem, FIELD_STRING),
DEFINE_INPUT(m_iThrowAmmoPrimary, FIELD_INTEGER, "SetPrimaryAmmoCountToThrow"),
DEFINE_INPUT(m_iThrowAmmoSecondary, FIELD_INTEGER, "SetSecondaryAmmoCountToThrow"),
DEFINE_INPUTFUNC(FIELD_STRING, "ThrowItemAtPlayer", InputThrowItemAtPlayer),

END_DATADESC();

LINK_ENTITY_TO_CLASS(npc_jessica, CNPC_SinJessica);

void CNPC_SinJessica::SelectModel()
{
	SetModelName(AllocPooledString("models/sirgibs/ragdolls/sinepisode/jessica.mdl"));
}

Class_T CNPC_SinJessica::Classify(void)
{
	return CLASS_PLAYER_ALLY_VITAL;
}

void CNPC_SinJessica::Spawn(void)
{
	m_iHealth = 80;

	BaseClass::Spawn();

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);

	NPCInit();

	SetUse(&CNPC_SinJessica::UseFunc);
	
	if (HasSpawnFlags(SF_JESSICA_INVINCIBLE))
		m_takedamage = DAMAGE_EVENTS_ONLY;
}

void CNPC_SinJessica::UseFunc(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed(TLK_USE, 0, true);
	m_bDontUseSemaphore = false;
}

void CNPC_SinJessica::InputThrowItemAtPlayer(inputdata_t& inputdata)
{
	m_iszThrowItem = inputdata.value.StringID();
	if (IsCustomInterruptConditionSet(COND_JESS_THROW_ITEM))
		SetSchedule(SCHED_JESS_THROW_ITEM);
}

void CNPC_SinJessica::GatherConditions()
{
	BaseClass::GatherConditions();

	if (m_iszThrowItem != NULL_STRING)
		SetCondition(COND_JESS_THROW_ITEM);
	else
		ClearCondition(COND_JESS_THROW_ITEM);
}

int CNPC_SinJessica::SelectSchedulePriorityAction()
{
	if (HasCondition(COND_JESS_THROW_ITEM) && !HasCondition(COND_TARGET_OCCLUDED))
		return SCHED_JESS_THROW_ITEM;

	return BaseClass::SelectSchedulePriorityAction();
}

void CNPC_SinJessica::BuildScheduleTestBits()
{
	BaseClass::BuildScheduleTestBits();

	if (ConditionInterruptsCurSchedule(COND_IDLE_INTERRUPT) || IsCurSchedule(SCHED_SCENE_GENERIC))
	{
		SetCustomInterruptCondition(COND_JESS_THROW_ITEM);
	}
}

void CNPC_SinJessica::HandleAnimEvent(animevent_t* pEvent)
{
	if (pEvent->type & AE_TYPE_NEWEVENTSYSTEM && pEvent->event == AE_JESSICA_THROW_ITEM)
	{
		Vector vecStart;
		QAngle angStart;
		if (!GetAttachment("anim_attachment_RH", vecStart, angStart))
		{
			vecStart = Weapon_ShootPosition();
			angStart = GetAbsAngles();
		}

		CBaseEntity* pItem = Create(STRING(m_iszThrowItem), vecStart, angStart, this);
		m_iszThrowItem = NULL_STRING;

		if (pItem)
		{
			if (pItem->IsBaseCombatWeapon())
			{
				CBaseCombatWeapon* pWeapon = pItem->MyCombatWeaponPointer();
				pWeapon->m_iClip1 = m_iThrowAmmoPrimary;
				pWeapon->m_iClip2 = m_iThrowAmmoSecondary;
			}

			Vector vecThrow;
			if (GetTarget())
			{
				// I've been told to throw it somewhere specific.
				CTraceFilterSkipTwoEntities filter(pItem, GetTarget(), pItem->GetCollisionGroup());
				vecThrow = VecCheckToss(this, &filter, vecStart, GetTarget()->WorldSpaceCenter(), 0.2, 1.0, false);
			}
			
			IPhysicsObject* pObj = pItem->VPhysicsGetObject();
			if (pObj != NULL && pItem->GetMoveType() == MOVETYPE_VPHYSICS)
			{
				AngularImpulse	angImp(200, 200, 200);
				pObj->AddVelocity(&vecThrow, &angImp);
			}
			else
			{
				pItem->SetAbsVelocity(vecThrow);
			}
		}
	}
	else
		BaseClass::HandleAnimEvent(pEvent);
}

AI_BEGIN_CUSTOM_NPC(npc_jessica, CNPC_SinJessica)

DECLARE_ANIMEVENT(AE_JESSICA_THROW_ITEM)
DECLARE_ACTIVITY(ACT_THROW_ITEM)
DECLARE_CONDITION(COND_JESS_THROW_ITEM)

DEFINE_SCHEDULE(
	SCHED_JESS_THROW_ITEM,
	
	"	Tasks"
	"		TASK_TARGET_PLAYER					0"
	"		TASK_WEAPON_HOLSTER					0"
	"		TASK_FACE_TARGET					0"
	"		TASK_PLAY_SEQUENCE_FACE_TARGET		ACTIVITY:ACT_THROW_ITEM"
	"	"
	"	Interrupts"
	"	COND_TARGET_OCCLUDED"
)

AI_END_CUSTOM_NPC();