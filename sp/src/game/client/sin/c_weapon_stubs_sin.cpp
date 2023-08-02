#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS(weapon_assault_rifle, SinWeaponAR, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_scattergun, SinWeaponShotgun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_magnum, SinWeaponMagnum, C_BaseHLCombatWeapon);

// NPC weapons
STUB_WEAPON_CLASS(weapon_chaingun, SinWeaponChaingun, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_npc_pistol, SinWeaponPistol, C_BaseHLCombatWeapon);
//STUB_WEAPON_CLASS(weapon_smg, SinWeaponSMG, C_HLMachineGun);