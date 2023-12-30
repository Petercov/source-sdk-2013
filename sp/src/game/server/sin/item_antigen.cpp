#include "cbase.h"
#include "gamerules.h"
#include "player.h"
#include "vguiscreen.h"
#include "props.h"
#include "in_buttons.h"
#include "eventqueue.h"

#include "tier0/memdbgon.h"

class CAntigenDispenserTrigger;

#define ANTIGEN_DISPENSER_MODEL "models/items/antigen/antigen_dispenser.mdl"
#define ANTIGEN_TANK_MODEL "models/items/antigen/antigentank.mdl"
#define ANTIGEN_FULL_POSEPARAM "percentFull"

class CAntigenTank : public CPhysicsProp
{
	DECLARE_CLASS(CAntigenTank, CPhysicsProp);
public:
	DECLARE_DATADESC();

	CAntigenTank()
	{
		m_iJuice = 100;
	}

	void Precache();
	void Spawn();

	void InputSetCharge(inputdata_t& inputdata);

	int m_iJuice;
};

BEGIN_DATADESC(CAntigenTank)

DEFINE_KEYFIELD(m_iJuice, FIELD_INTEGER, "antigen"),
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetCharge", InputSetCharge),

END_DATADESC();

LINK_ENTITY_TO_CLASS(prop_antigen_explosive_barrel, CAntigenTank);
LINK_ENTITY_TO_CLASS(prop_antigen_canister, CAntigenTank);

void CAntigenTank::Precache()
{
	BaseClass::Precache();
}

void CAntigenTank::Spawn()
{
	SetModelName(AllocPooledString_StaticConstantStringPointer(ANTIGEN_TANK_MODEL));
	BaseClass::Spawn();
	SetPoseParameter(ANTIGEN_FULL_POSEPARAM, m_iJuice * 0.01f);
}

void CAntigenTank::InputSetCharge(inputdata_t& inputdata)
{
	m_iJuice = Clamp(inputdata.value.Int(), 0, 100);
	SetPoseParameter(ANTIGEN_FULL_POSEPARAM, m_iJuice * 0.01f);
}

class CAntigenDispenser : public CBaseAnimating
{
	DECLARE_CLASS(CAntigenDispenser, CBaseAnimating);
public:
	DECLARE_DATADESC();

	void Spawn();
	void Precache(void);
	int  DrawDebugTextOverlays(void);
	bool CreateVPhysics(void);
	void Off(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return BaseClass::ObjectCaps() | m_iCaps; }

	void SetCanister(bool bInserted);
	void UpdateJuice(int newJuice);
	float MaxJuice() const { return 100; }

#ifdef MAPBASE
	void InputRecharge( inputdata_t &inputdata );
	void InputSetCharge(inputdata_t& inputdata);
	int		m_iIncrementValue;
#endif // MAPBASE

	// Handle a potentially complex command from a client.
	// Returns true if the command was handled successfully.
	virtual bool	HandleEntityCommand(CBasePlayer* pClient, KeyValues* pKeyValues);
	void	SpawnControlPanel();
	bool	EjectCanister(CBasePlayer* pPlayer);
	bool	InsertCanister(CAntigenTank* pTank);

	float m_flNextCharge;
	int		m_iJuice;
	int		m_iOn;			// 0 = off, 1 = startup, 2 = going
	float   m_flSoundTime;

	int		m_nState;
	int		m_iCaps;
	bool	m_bCanister;

	COutputFloat m_OutRemainingHealth;
#ifdef MAPBASE
	COutputEvent m_OnHalfEmpty;
	COutputEvent m_OnEmpty;
	COutputEvent m_OnFull;
#endif
	COutputEvent m_OnPlayerUse;

	COutputEvent m_OnCanisterInserted;
	COutputEHANDLE m_OnCanisterEjected;

	void StudioFrameAdvance(void);

	float m_flJuice;

	CHandle< CAntigenDispenserTrigger > m_hTrigger;
};

BEGIN_DATADESC(CAntigenDispenser)

DEFINE_FIELD(m_flNextCharge, FIELD_TIME),
DEFINE_KEYFIELD(m_iJuice, FIELD_INTEGER, "antigen"),
DEFINE_FIELD(m_iOn, FIELD_INTEGER),
DEFINE_FIELD(m_flSoundTime, FIELD_TIME),
DEFINE_FIELD(m_nState, FIELD_INTEGER),
DEFINE_FIELD(m_iCaps, FIELD_INTEGER),
DEFINE_FIELD(m_flJuice, FIELD_FLOAT),
#ifdef MAPBASE
DEFINE_INPUT(m_iIncrementValue, FIELD_INTEGER, "SetIncrementValue"),
#endif
DEFINE_FIELD(m_bCanister, FIELD_BOOLEAN),
DEFINE_FIELD(m_hTrigger, FIELD_EHANDLE),

// Function Pointers
DEFINE_THINKFUNC(Off),

DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
DEFINE_OUTPUT(m_OutRemainingHealth, "OutRemainingHealth"),
DEFINE_OUTPUT(m_OnCanisterInserted, "OnCanisterInserted"),
DEFINE_OUTPUT(m_OnCanisterEjected, "OnCanisterEjected"),
#ifdef MAPBASE
DEFINE_OUTPUT(m_OnHalfEmpty, "OnHalfEmpty"),
DEFINE_OUTPUT(m_OnEmpty, "OnEmpty"),
DEFINE_OUTPUT(m_OnFull, "OnFull"),

DEFINE_INPUTFUNC(FIELD_VOID, "Recharge", InputRecharge),
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetCharge", InputSetCharge),
#endif

END_DATADESC();

LINK_ENTITY_TO_CLASS(item_antigen_dispenser, CAntigenDispenser);

class CAntigenDispenserTrigger : public CBaseEntity
{
	DECLARE_CLASS(CAntigenDispenserTrigger, CBaseEntity);

public:

	// 
	// Creates a trigger with the specified bounds

	static CAntigenDispenserTrigger* Create(const Vector& vecOrigin, const Vector& vecMins, const Vector& vecMaxs, CBaseEntity* pOwner)
	{
		CAntigenDispenserTrigger* pTrigger = (CAntigenDispenserTrigger*)CreateEntityByName("trigger_antigen_dispenser");
		if (pTrigger == NULL)
			return NULL;

		UTIL_SetOrigin(pTrigger, vecOrigin);
		UTIL_SetSize(pTrigger, vecMins, vecMaxs);
		pTrigger->SetOwnerEntity(pOwner);
		pTrigger->SetParent(pOwner);

		pTrigger->Spawn();

		return pTrigger;
	}

	//
	// Handles the trigger touching its intended quarry

	void CargoTouch(CBaseEntity* pOther)
	{
		// Cannot be ignoring touches
		if ((m_hIgnoreEntity == pOther) || (m_flIgnoreDuration >= gpGlobals->curtime))
			return;

		// Make sure this object is being held by the player
		if (pOther->VPhysicsGetObject() == NULL || (pOther->VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_PLAYER_HELD) == false)
			return;

		AddCargo(pOther);
	}

	bool AddCargo(CBaseEntity* pOther)
	{
		// For now, only bother with strider busters
		if ((FClassnameIs(pOther, "prop_antigen_canister") == false) &&
			(FClassnameIs(pOther, "prop_antigen_explosive_barrel") == false)
			)
			return false;

		// Must be a physics prop
		CAntigenTank* pProp = dynamic_cast<CAntigenTank*>(pOther);
		if (pOther == NULL)
			return false;

		CAntigenDispenser* pDisp = dynamic_cast<CAntigenDispenser*>(GetOwnerEntity());
		if (pDisp == NULL)
			return false;

		// Make the player release the item
		Pickup_ForcePlayerToDropThisObject(pOther);

		if (pDisp->InsertCanister(pProp))
		{
			// Stop touching this item
			Disable();

			return true;
		}
		else
			return false;
	}

	//
	// Setup the entity

	void Spawn(void)
	{
		BaseClass::Spawn();

		SetSolid(SOLID_BBOX);
		SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);

		SetTouch(&CAntigenDispenserTrigger::CargoTouch);
	}

	void Activate()
	{
		BaseClass::Activate();
		SetSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID); // Fixes up old savegames
	}

	//
	// When we've stopped touching this entity, we ignore it

	void EndTouch(CBaseEntity* pOther)
	{
		if (pOther == m_hIgnoreEntity)
		{
			m_hIgnoreEntity = NULL;
		}

		BaseClass::EndTouch(pOther);
	}

	//
	// Disables the trigger for a set duration

	void IgnoreTouches(CBaseEntity* pIgnoreEntity)
	{
		m_hIgnoreEntity = pIgnoreEntity;
		m_flIgnoreDuration = gpGlobals->curtime + 0.5f;
	}

	void Disable(void)
	{
		SetTouch(NULL);
	}

	void Enable(void)
	{
		SetTouch(&CAntigenDispenserTrigger::CargoTouch);
	}

protected:

	float					m_flIgnoreDuration;
	CHandle	<CBaseEntity>	m_hIgnoreEntity;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(trigger_antigen_dispenser, CAntigenDispenserTrigger);

BEGIN_DATADESC(CAntigenDispenserTrigger)
DEFINE_FIELD(m_flIgnoreDuration, FIELD_TIME),
DEFINE_FIELD(m_hIgnoreEntity, FIELD_EHANDLE),
DEFINE_ENTITYFUNC(CargoTouch),
END_DATADESC();

void CAntigenDispenser::Spawn()
{
	Precache();

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);

	SetModel(ANTIGEN_DISPENSER_MODEL);
	AddEffects(EF_NOSHADOW);
	CreateVPhysics();

	Vector vecForward;
	GetVectors(&vecForward, nullptr, nullptr);

	// Approx size of the hold
	Vector vecMins(-6.0, -6.0, -4.0);
	Vector vecMaxs(6.0, 6.0, 4.0);
	m_hTrigger = CAntigenDispenserTrigger::Create(GetAbsOrigin() + vecForward * 5, vecMins, vecMaxs, this);

	ResetSequence(LookupSequence("idle"));

#ifdef MAPBASE
	if (m_iIncrementValue == 0)
		m_iIncrementValue = 1;
#endif
	SetCanister(m_iJuice > 0);
	UpdateJuice(m_iJuice);

	m_nState = 0;

	m_flJuice = m_iJuice;
	StudioFrameAdvance();

	SpawnControlPanel();
}

void CAntigenDispenser::Precache(void)
{
	PrecacheModel(ANTIGEN_DISPENSER_MODEL);

	PrecacheScriptSound("dispenser.empty");
	PrecacheScriptSound("dispenser.load");
	PrecacheScriptSound("dispenser.eject");
	PrecacheScriptSound("dispenser.use_loop");
	PrecacheScriptSound("dispenser.stop");
}

int CAntigenDispenser::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];
		if (m_bCanister)
			Q_snprintf(tempstr, sizeof(tempstr), "Charge left: %i", m_iJuice);
		else
			V_strcpy_safe(tempstr, "No canister");
		EntityText(text_offset, tempstr, 0);
		text_offset++;
	}
	return text_offset;
}

bool CAntigenDispenser::CreateVPhysics(void)
{
	return VPhysicsInitStatic();
}

void CAntigenDispenser::Off(void)
{
	// Stop looping sound.
	if (m_iOn > 1)
	{
		StopSound("dispenser.use_loop");
		EmitSound("dispenser.stop");
	}

	m_iOn = 0;
	m_flJuice = m_iJuice;

	StudioFrameAdvance();
	SetThink(NULL);
}

#define CHARGE_RATE 0.25f
#define CHARGES_PER_SECOND 1.0f / CHARGE_RATE
#define CALLS_PER_SECOND 7.0f * CHARGES_PER_SECOND

#ifdef MAPBASE
#define CUSTOM_CHARGES_PER_SECOND(inc) inc / CHARGE_RATE
#endif

void CAntigenDispenser::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Make sure that we have a caller
	if (!pActivator)
		return;

	// if it's not a player, ignore
	if (!pActivator->IsPlayer())
		return;
	CBasePlayer* pPlayer = dynamic_cast<CBasePlayer*>(pActivator);

	// Reset to a state of continuous use.
	m_iCaps = FCAP_CONTINUOUS_USE;

	if (m_iOn)
	{
		float flCharges = CHARGES_PER_SECOND;
		float flCalls = CALLS_PER_SECOND;

#ifdef MAPBASE
		if (m_iIncrementValue != 0)
			flCharges = CUSTOM_CHARGES_PER_SECOND(m_iIncrementValue);
#endif

		m_flJuice -= flCharges / flCalls;
		StudioFrameAdvance();
	}

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		m_nState = 1;
		Off();
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise.
	// disabled HEV suit dependency for now.
	//if ((m_iJuice <= 0) || (!(pActivator->m_bWearingSuit)))
	if (m_iJuice <= 0)
	{
		if (m_flSoundTime <= gpGlobals->curtime)
		{
			m_flSoundTime = gpGlobals->curtime + 0.62;
			EmitSound("dispenser.empty");
		}
		return;
	}

	if (pActivator->GetHealth() >= pActivator->GetMaxHealth())
	{
		if (pPlayer)
		{
			pPlayer->m_afButtonPressed &= ~IN_USE;
		}

		// Make the user re-use me to get started drawing health.
		m_iCaps = FCAP_IMPULSE_USE;

		EmitSound("dispenser.empty");
		return;
	}

	SetNextThink(gpGlobals->curtime + CHARGE_RATE);
	SetThink(&CAntigenDispenser::Off);

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->curtime)
		return;

	// Play the on sound or the looping charging sound
	if (!m_iOn)
	{
		m_iOn++;
		EmitSound("dispenser.use_loop");
		m_flSoundTime = 0.56 + gpGlobals->curtime;

		m_OnPlayerUse.FireOutput(pActivator, this);
	}
	/*if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->curtime))
	{
		m_iOn++;
		CPASAttenuationFilter filter(this, "WallHealth.LoopingContinueCharge");
		filter.MakeReliable();
		EmitSound(filter, entindex(), "WallHealth.LoopingContinueCharge");
	}*/

	// charge the player
#ifdef MAPBASE
	if (pActivator->TakeHealth(m_iIncrementValue, DMG_GENERIC))
	{
		UpdateJuice(m_iJuice - m_iIncrementValue);
	}
#else
	if (pActivator->TakeHealth(1, DMG_GENERIC))
	{
		m_iJuice--;
	}
#endif

	// Send the output.
#ifdef MAPBASE
	float flRemaining = m_iJuice / MaxJuice();
#else
	float flRemaining = m_iJuice / 100;
#endif
	m_OutRemainingHealth.Set(flRemaining, pActivator, this);

	// govern the rate of charge
	m_flNextCharge = gpGlobals->curtime + 0.1;
}

void CAntigenDispenser::UpdateJuice(int newJuice)
{
#ifdef MAPBASE
	bool reduced = newJuice < m_iJuice;
	if (reduced)
	{
		// Fire 1/2 way output and/or empyt output
		int oneHalfJuice = (int)(MaxJuice() * 0.5f);
		if (newJuice <= oneHalfJuice && m_iJuice > oneHalfJuice)
		{
			m_OnHalfEmpty.FireOutput(this, this);
		}

		if (newJuice <= 0)
		{
			m_OnEmpty.FireOutput(this, this);
		}
	}
	else if (newJuice != MaxJuice() &&
		newJuice == (int)100)
	{
		m_OnFull.FireOutput(this, this);
	}
#endif // MAPBASE

	m_iJuice = newJuice;
}

void CAntigenDispenser::InputRecharge(inputdata_t& inputdata)
{
	SetCanister(true);
	UpdateJuice(MaxJuice());
	m_flJuice = m_iJuice;
	StudioFrameAdvance();
}

void CAntigenDispenser::InputSetCharge(inputdata_t& inputdata)
{
	int iJuice = Clamp(inputdata.value.Int(), 0, 100);

	SetCanister(true);
	UpdateJuice(iJuice);
	m_flJuice = m_iJuice;
	StudioFrameAdvance();
}

bool CAntigenDispenser::HandleEntityCommand(CBasePlayer* pClient, KeyValues* pKeyValues)
{
	if (FStrEq("eject", pKeyValues->GetString()))
	{
		EjectCanister(pClient);
		return true;
	}

	return false;
}

void CAntigenDispenser::SpawnControlPanel()
{
	// FIXME: Deal with dynamically resizing control panels?

	// If we're attached to an entity, spawn control panels on it instead of use
	CBaseAnimating* pEntityToSpawnOn = this;
	char* pOrgLL = "controlpanel0_ll";
	char* pOrgUR = "controlpanel0_ur";

	Assert(pEntityToSpawnOn);

	// Lookup the attachment point...
	int nLLAttachmentIndex = pEntityToSpawnOn->LookupAttachment(pOrgLL);

	if (nLLAttachmentIndex <= 0)
	{
		return;
	}

	int nURAttachmentIndex = pEntityToSpawnOn->LookupAttachment(pOrgUR);
	if (nURAttachmentIndex <= 0)
	{
		return;
	}

	const char* pScreenName = "vgui_dispenser_panel";
	const char* pScreenClassname = "vgui_screen";

	// Compute the screen size from the attachment points...
	matrix3x4_t	panelToWorld;
	pEntityToSpawnOn->GetAttachment(nLLAttachmentIndex, panelToWorld);

	matrix3x4_t	worldToPanel;
	MatrixInvert(panelToWorld, worldToPanel);

	// Now get the lower right position + transform into panel space
	Vector lr, lrlocal;
	pEntityToSpawnOn->GetAttachment(nURAttachmentIndex, panelToWorld);
	MatrixGetColumn(panelToWorld, 3, lr);
	VectorTransform(lr, worldToPanel, lrlocal);

	float flWidth = lrlocal.x;
	float flHeight = lrlocal.y;

	CVGuiScreen* pScreen = CreateVGuiScreen(pScreenClassname, pScreenName, pEntityToSpawnOn, this, nLLAttachmentIndex);
	pScreen->SetActualSize(flWidth, flHeight);
	pScreen->SetActive(true);
	pScreen->SetTransparency(true);
}

bool CAntigenDispenser::EjectCanister(CBasePlayer* pPlayer)
{
	if (!m_bCanister)
		return false;

	CAntigenTank* pTank = (CAntigenTank*)CreateNoSpawn("prop_antigen_canister", GetAbsOrigin(), GetAbsAngles(), this);
	if (!pTank)
		return false;
	pTank->m_iJuice = m_iJuice;
	if (DispatchSpawn(pTank) < 0)
		return false;

	SetCanister(false);
	m_flJuice = m_iJuice = 0;
	StudioFrameAdvance();

	CBaseEntity* pActivator = this;
	if (pPlayer)
	{
		pActivator = pPlayer;
		pPlayer->ForceDropOfCarriedPhysObjects();

		variant_t value;
		g_EventQueue.AddEvent(pTank, "Use", value, 0.01f, pPlayer, this);
	}

	m_hTrigger->IgnoreTouches(pTank);
	m_hTrigger->Enable();

	EmitSound("dispenser.eject");
	m_OnCanisterEjected.Set(pTank, pActivator, this);
	return true;
}

bool CAntigenDispenser::InsertCanister(CAntigenTank* pTank)
{
	if (m_bCanister || !pTank)
		return false;

	SetCanister(true);
	UpdateJuice(pTank->m_iJuice);
	m_flJuice = m_iJuice;
	StudioFrameAdvance();

	/*CBaseEntity* pActivator = pTank->HasPhysicsAttacker(0.5f);
	if (!pActivator)
		pActivator = this;*/

	EmitSound("dispenser.load");
	m_OnCanisterInserted.FireOutput(this, this);
	UTIL_Remove(pTank);
	return true;
}

void CAntigenDispenser::StudioFrameAdvance(void)
{
	m_flPlaybackRate = 0;

	const float flMaxJuice = MaxJuice();
	SetPoseParameter(ANTIGEN_FULL_POSEPARAM, (float)(m_flJuice / flMaxJuice));
	//	Msg( "Cycle: %f - Juice: %d - m_flJuice :%f - Interval: %f\n", (float)GetCycle(), (int)m_iJuice, (float)m_flJuice, GetAnimTimeInterval() );

	if (!m_flPrevAnimTime)
	{
		m_flPrevAnimTime = gpGlobals->curtime;
	}

	// Latch prev
	m_flPrevAnimTime = m_flAnimTime;
	// Set current
	m_flAnimTime = gpGlobals->curtime;
}

void CAntigenDispenser::SetCanister(bool bInserted)
{
	m_bCanister = bInserted;

	int nBody = FindBodygroupByName("withTank");
	if (bInserted)
	{
		m_hTrigger->Disable();
		SetBodygroup(nBody, 1);
		m_iCaps = FCAP_CONTINUOUS_USE;
	}
	else
	{
		SetBodygroup(nBody, 0);
		m_iCaps = 0;
		m_hTrigger->Enable();
	}
}
