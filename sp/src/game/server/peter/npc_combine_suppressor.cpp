#include "cbase.h"
#include "npc_combine.h"

ConVar	sk_combine_supressor_health("sk_combine_supressor_health", "250");
ConVar	sk_combine_supressor_kick("sk_combine_supressor_kick", "20");

class CNPC_CombineSuppressor : public CNPC_Combine
{
	DECLARE_CLASS(CNPC_CombineSuppressor, CNPC_Combine);
	DECLARE_DATADESC();

public:
	void		Spawn(void);
	void		Precache(void);
	void		DeathSound(const CTakeDamageInfo& info);
	virtual void		IdleSound(void);
	virtual bool		ShouldPlayIdleSound(void);

	Activity	NPC_TranslateActivity(Activity eNewActivity);
	WeaponProficiency_t CalcWeaponProficiency(CBaseCombatWeapon* pWeapon);
protected:
	WeaponClass_t m_nWeaponType;
};

LINK_ENTITY_TO_CLASS(npc_combine_suppressor, CNPC_CombineSuppressor);

BEGIN_DATADESC(CNPC_CombineSuppressor)
DEFINE_FIELD(m_nWeaponType, FIELD_CHARACTER),
END_DATADESC();

void CNPC_CombineSuppressor::Spawn(void)
{
	Precache();
	SetModel(STRING(GetModelName()));

	SetHealth(sk_combine_supressor_health.GetFloat());
	SetMaxHealth(sk_combine_supressor_health.GetFloat());
	SetKickDamage(sk_combine_supressor_kick.GetFloat());

	BaseClass::Spawn();

	CapabilitiesAdd(bits_CAP_ANIMATEDFACE);
	CapabilitiesAdd(bits_CAP_MOVE_SHOOT);
	CapabilitiesAdd(bits_CAP_DOORS_GROUP);

	CapabilitiesRemove(bits_CAP_INNATE_MELEE_ATTACK1);
}

void CNPC_CombineSuppressor::Precache(void)
{
	if (!GetModelName())
	{
		SetModelName(MAKE_STRING("models/combine_suppressor.mdl"));
	}

	PrecacheModel(STRING(GetModelName()));

	BaseClass::Precache();
}

void CNPC_CombineSuppressor::DeathSound(const CTakeDamageInfo& info)
{
	AI_CriteriaSet set;
	ModifyOrAppendDamageCriteria(set, info);
	SpeakIfAllowed(TLK_CMB_DIE, set, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}

void CNPC_CombineSuppressor::IdleSound(void)
{
	if (m_NPCState == NPC_STATE_COMBAT)
	{
		SpeakIfAllowed("TLK_COMBATIDLE", SENTENCE_PRIORITY_NORMAL, SENTENCE_CRITERIA_NORMAL);
	}
	else
		BaseClass::IdleSound();
}

bool CNPC_CombineSuppressor::ShouldPlayIdleSound(void)
{
	if (m_NPCState == NPC_STATE_COMBAT && random->RandomInt(0, 79) == 0)
	{
		return !IsSpeaking();
	}

	return BaseClass::ShouldPlayIdleSound();
}

Activity CNPC_CombineSuppressor::NPC_TranslateActivity(Activity eNewActivity)
{
	if (m_nWeaponType == WEPCLASS_HEAVY && !m_MoveAndShootOverlay.IsSuspended())
	{
		// Don't run while shooting the minigun
		switch (eNewActivity)
		{
		case ACT_RUN_AIM:
			eNewActivity = ACT_WALK_AIM;
			break;
		case ACT_RUN_AIM_AGITATED:
			eNewActivity = ACT_WALK_AIM_AGITATED;
			break;
		default:
			break;
		}
	}

	return BaseClass::NPC_TranslateActivity(eNewActivity);
}

WeaponProficiency_t CNPC_CombineSuppressor::CalcWeaponProficiency(CBaseCombatWeapon* pWeapon)
{
	m_nWeaponType = pWeapon->WeaponClassify();
	if (m_nWeaponType == WEPCLASS_HEAVY)
	{
		return WEAPON_PROFICIENCY_VERY_GOOD;
	}

	return BaseClass::CalcWeaponProficiency(pWeapon);
}
