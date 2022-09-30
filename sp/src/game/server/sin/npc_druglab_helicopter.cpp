#include "cbase.h"
#include "gib.h"
#include "soundenvelope.h"
#include "hl2\npc_attackchopper.h"

class CNPC_SinHelicopter : public CNPC_AttackHelicopter
{
public:
	DECLARE_CLASS(CNPC_SinHelicopter, CNPC_AttackHelicopter);

	CNPC_SinHelicopter();

	virtual void	Precache(void);
	virtual void	Spawn(void);

protected:
	virtual void	InitializeRotorSound(void);
	virtual void	ChargeGunSound();

	// Drop a corpse!
	virtual void	DropCorpse(int nDamage);
};

LINK_ENTITY_TO_CLASS(npc_druglab_helicopter, CNPC_SinHelicopter);

CNPC_SinHelicopter::CNPC_SinHelicopter()
{
}

void CNPC_SinHelicopter::Precache(void)
{
	if (GetModelName() == NULL_STRING)
	{
		SetModelName(AllocPooledString("models/vehicles/sintek_helicopter/sintek_helicopter.mdl"));
	}

	BaseClass::Precache();

	PrecacheScriptSound("NPC_SinHelicopter.ChargeGun");
	if (HasSpawnFlags(SF_HELICOPTER_LOUD_ROTOR_SOUND))
	{
		PrecacheScriptSound("NPC_SinHelicopter.RotorsLoud");
	}
	else
	{
		PrecacheScriptSound("NPC_SinHelicopter.Rotors");
	}
	//PrecacheScriptSound("NPC_SinHelicopter.DropMine");
	//PrecacheScriptSound("NPC_SinHelicopter.BadlyDamagedAlert");
	//PrecacheScriptSound("NPC_SinHelicopter.CrashingAlarm1");
	//PrecacheScriptSound("NPC_SinHelicopter.MegabombAlert");

	PrecacheScriptSound("NPC_SinHelicopter.RotorBlast");
	//PrecacheScriptSound("NPC_SinHelicopter.EngineFailure");
	PrecacheScriptSound("NPC_SinHelicopter.FireGun");
	PrecacheScriptSound("NPC_SinHelicopter.Crash");

	// If we're never going to engage in combat, we don't need to load these assets!
	if (m_bNonCombat == false)
	{
		PrecacheModel("models/characters/MobGrunt/mobgrunt.mdl");
	}
}

void CNPC_SinHelicopter::Spawn(void)
{
	BaseClass::Spawn();
}

void CNPC_SinHelicopter::InitializeRotorSound(void)
{
	if (!m_pRotorSound)
	{
		CSoundEnvelopeController& controller = CSoundEnvelopeController::GetController();
		CPASAttenuationFilter filter(this);

		if (HasSpawnFlags(SF_HELICOPTER_LOUD_ROTOR_SOUND))
		{
			m_pRotorSound = controller.SoundCreate(filter, entindex(), "NPC_SinHelicopter.RotorsLoud");
		}
		else
		{
			m_pRotorSound = controller.SoundCreate(filter, entindex(), "NPC_SinHelicopter.Rotors");
		}

		m_pRotorBlast = controller.SoundCreate(filter, entindex(), "NPC_SinHelicopter.RotorBlast");
		m_pGunFiringSound = controller.SoundCreate(filter, entindex(), "NPC_SinHelicopter.FireGun");
		controller.Play(m_pGunFiringSound, 0.0, 100);
	}
	else
	{
		Assert(m_pRotorSound);
		Assert(m_pRotorBlast);
		Assert(m_pGunFiringSound);
	}


	CBaseHelicopter::InitializeRotorSound();
}

void CNPC_SinHelicopter::ChargeGunSound()
{
	EmitSound("NPC_SinHelicopter.ChargeGun");
}

void CNPC_SinHelicopter::DropCorpse(int nDamage)
{
	// Don't drop another corpse if the next guy's not out on the gun yet
	if (m_flLastCorpseFall > gpGlobals->curtime)
		return;

	// Clamp damage to prevent ridiculous ragdoll velocity
	if (nDamage > 250.0f)
		nDamage = 250.0f;

	m_flLastCorpseFall = gpGlobals->curtime + 3.0;

	// Spawn a ragdoll combine guard
	float forceScale = nDamage * 75 * 4;
	Vector vecForceVector = RandomVector(-1, 1);
	vecForceVector.z = 0.5;
	vecForceVector *= forceScale;

	CBaseEntity* pGib = CreateRagGib("models/characters/MobGrunt/mobgrunt.mdl", GetAbsOrigin(), GetAbsAngles(), vecForceVector);
	if (pGib)
	{
		pGib->SetOwnerEntity(this);
	}
}
