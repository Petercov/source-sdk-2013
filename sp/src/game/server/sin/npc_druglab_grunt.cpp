#include "cbase.h"
#include "npc_combine.h"

int ACT_PISTOL_MELEE_ATTACK;

#define SF_MOBGRUNT_START_SITTING ( 1 << 17 )
#define SF_MOBGRUNT_TRIGGER_DNA_SCAN ( 1 << 18 )
#define SF_MOBGRUNT_ONE_HIT_KILL ( 1 << 19 )
#define SF_MOBGRUNT_NO_DROP_2ND_AMMO ( 1 << 20 )
#define SF_MOBGRUNT_JETPACK_RELATIVE_PATHNODE_FOLLOW ( 1 << 21 )
#define SF_MOBGRUNT_JETPACK_IGNORE_LOS_ON_RPF ( 1 << 22 )
#define SF_MOBGRUNT_JETPACK_DOUBLE_SPEED ( 1 << 23 )
#define SF_MOBGRUNT_NO_SECONDARY_FIRE ( 1 << 24 )

ConVar sk_elite_grunt_health("sk_elite_grunt_health", "0");
ConVar sk_elite_grunt_kick("sk_elite_grunt_kick", "0");
ConVar sk_elite_grunt_head_mult("sk_elite_grunt_head_mult", "1");

ConVar sk_grunt_health("sk_grunt_health", "0");
ConVar sk_grunt_kick("sk_grunt_kick", "0");

ConVar sk_grunt_jetpack_health("sk_grunt_jetpack_health", "0");
ConVar sk_grunt_pistol_health("sk_grunt_pistol_health", "0");
ConVar sk_grunt_scattergun_health("sk_grunt_scattergun_health", "0");
ConVar sk_grunt_assault_rifle_health("sk_grunt_assault_rifle_health", "0");

class CNPC_DruglabGrunt : public CNPC_Combine
{
public:
	DECLARE_CLASS(CNPC_DruglabGrunt, CNPC_Combine);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	void		Spawn(void);
	void		Precache(void);
	void		DeathSound(const CTakeDamageInfo& info);

#ifdef SIN_NPCS
	Class_T			Classify(void) { return CLASS_SINTEK; }
#endif // SIN_NPCS


	virtual bool	IsUsingTacticalVariant(int variant);

	bool			IsAltFireCapable();
	bool			IsGrenadeCapable() { return true; }

	virtual Activity	Weapon_TranslateActivity(Activity baseAct, bool* pRequired = NULL);
protected:
	float	m_flGrenadeHeight;
};

LINK_ENTITY_TO_CLASS(npc_druglab_grunt_pistol, CNPC_DruglabGrunt);
LINK_ENTITY_TO_CLASS(npc_druglab_grunt_scattergun, CNPC_DruglabGrunt);
LINK_ENTITY_TO_CLASS(npc_druglab_grunt_assault_rifle, CNPC_DruglabGrunt);
LINK_ENTITY_TO_CLASS(npc_druglab_grunt_elite, CNPC_DruglabGrunt);

BEGIN_DATADESC(CNPC_DruglabGrunt)
DEFINE_KEYFIELD(m_flGrenadeHeight, FIELD_FLOAT, "GrenadeHeight"),
END_DATADESC();

void CNPC_DruglabGrunt::Spawn(void)
{
	Precache();
	SetModel(STRING(GetModelName()));

	if (IsElite())
	{
		// Stronger, tougher.
		SetHealth(sk_elite_grunt_health.GetFloat());
		SetMaxHealth(sk_elite_grunt_health.GetFloat());
		SetKickDamage(sk_elite_grunt_kick.GetFloat());
	}
	else
	{
		SetHealth(sk_grunt_health.GetFloat());
		SetMaxHealth(sk_grunt_health.GetFloat());
		SetKickDamage(sk_grunt_kick.GetFloat());
	}

	CapabilitiesAdd(bits_CAP_ANIMATEDFACE);
	CapabilitiesAdd(bits_CAP_MOVE_SHOOT);
	CapabilitiesAdd(bits_CAP_DOORS_GROUP);

	BaseClass::Spawn();

	if (!IsElite() && GetActiveWeapon())
	{
		switch (GetActiveWeapon()->WeaponClassify())
		{
		case WEPCLASS_HANDGUN:
			SetHealth(sk_grunt_pistol_health.GetFloat());
			SetMaxHealth(sk_grunt_pistol_health.GetFloat());
			break;
		case WEPCLASS_SHOTGUN:
			SetHealth(sk_grunt_scattergun_health.GetFloat());
			SetMaxHealth(sk_grunt_scattergun_health.GetFloat());
			m_nSkin = 1;
			break;
		case WEPCLASS_RIFLE:
			SetHealth(sk_grunt_assault_rifle_health.GetFloat());
			SetMaxHealth(sk_grunt_assault_rifle_health.GetFloat());
			m_nSkin = 2;
			break;
		default:
			break;
		}
	}
}

void CNPC_DruglabGrunt::Precache(void)
{
	m_fIsElite = FClassnameIs(this, "npc_druglab_grunt_elite");
	if (m_spawnEquipment == AllocPooledString_StaticConstantStringPointer("weapon_magnum"))
	{
		m_spawnEquipment = AllocPooledString_StaticConstantStringPointer("weapon_npc_pistol");
	}

	if (!GetModelName())
	{
		if (m_fIsElite)
			SetModelName(MAKE_STRING("models/characters/SintekGrunt/SintekGrunt.mdl"));
		else
			SetModelName(MAKE_STRING("models/characters/MobGrunt/mobgrunt.mdl"));
	}

	PrecacheModel(STRING(GetModelName()));

	UTIL_PrecacheOther("item_healthvial");

	BaseClass::Precache();
}

void CNPC_DruglabGrunt::DeathSound(const CTakeDamageInfo& info)
{
	AI_CriteriaSet set;
	ModifyOrAppendDamageCriteria(set, info);
	SpeakIfAllowed(TLK_CMB_DIE, set, SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}

bool CNPC_DruglabGrunt::IsUsingTacticalVariant(int variant)
{
	if (variant == TACTICAL_VARIANT_GRENADE_HAPPY && GetEnemy() && GetEnemy()->IsPlayer())
	{
		if (GetAbsOrigin().z - GetEnemyLKP().z > m_flGrenadeHeight)
			return true;
	}

	return BaseClass::IsUsingTacticalVariant(variant);
}

bool CNPC_DruglabGrunt::IsAltFireCapable()
{
	if (HasSpawnFlags(SF_MOBGRUNT_NO_SECONDARY_FIRE))
		return false;

	return BaseClass::BaseClass::IsAltFireCapable();
}

Activity CNPC_DruglabGrunt::Weapon_TranslateActivity(Activity baseAct, bool* pRequired)
{
	if (GetActiveWeapon() && GetActiveWeapon()->WeaponClassify() == WEPCLASS_HANDGUN)
	{
		if (baseAct == ACT_MELEE_ATTACK1)
			return (Activity)ACT_PISTOL_MELEE_ATTACK;
	}

	return BaseClass::Weapon_TranslateActivity(baseAct, pRequired);
}

AI_BEGIN_CUSTOM_NPC(npc_druglab_grunt, CNPC_DruglabGrunt)
DECLARE_ACTIVITY(ACT_PISTOL_MELEE_ATTACK)
AI_END_CUSTOM_NPC();