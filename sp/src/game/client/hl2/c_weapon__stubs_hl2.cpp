//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS( cycler_weapon, WeaponCycler, C_BaseCombatWeapon );

STUB_WEAPON_CLASS( weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_flaregun, Flaregun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon );
STUB_WEAPON_CLASS( weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage );

STUB_WEAPON_CLASS(weapon_suppressor_lmg, WeaponSuppressor, C_HLMachineGun);

#ifndef HL2MP
STUB_WEAPON_CLASS( weapon_ar2, WeaponAR2, C_HLMachineGun );
STUB_WEAPON_CLASS( weapon_frag, WeaponFrag, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun );
STUB_WEAPON_CLASS( weapon_357, Weapon357, C_BaseHLCombatWeapon );

#ifndef MAPBASE
STUB_WEAPON_CLASS(weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon);
#else
STUB_WEAPON_CLASS(weapon_crossbow, WeaponCrossbow, C_BaseHLZoomableWeapon);
#endif // !MAPBASE

#ifndef MAPBASE
STUB_WEAPON_CLASS( weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon );
#endif

STUB_WEAPON_CLASS( weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon );

#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS( weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon );
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
#endif

#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS( weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage );
#endif

#ifdef HL2BETA_WEAPONS
STUB_WEAPON_CLASS(weapon_oicw, WeaponOICW, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_hmg1, WeaponHMG1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_smg2, WeaponSMG2, C_HLSelectFireMachineGun);
#ifndef MAPBASE
STUB_WEAPON_CLASS(weapon_sniperrifle, WeaponSniperRifle, C_BaseHLCombatWeapon);
#else
STUB_WEAPON_CLASS(weapon_sniperrifle, WeaponSniperRifle, C_BaseHLZoomableWeapon);
#endif // !MAPBASE
STUB_WEAPON_CLASS(weapon_irifle, WeaponIRifle, C_BaseHLCombatWeapon);
#endif // HL2BETA_WEAPONS

#ifdef EZ2_WEAPONS
//STUB_WEAPON_CLASS( weapon_pulsepistol, WeaponPulsePistol, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_displacer_pistol, DisplacerPistol, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_ar2_proto, WeaponAR2Proto, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_endgame, WeaponEndGame, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS( weapon_arbeit_clipboard, WeaponArbeitClipboard, C_WeaponCitizenPackage );

STUB_WEAPON_CLASS( weapon_flechette_shotgun, WeaponFlechetteShotgun, C_BaseHLCombatWeapon );
#endif

#ifdef GOREAGULATION_WEAPONS
STUB_WEAPON_CLASS(weapon_gore_smg1, WeaponGoreGun, C_WeaponSMG1);
STUB_WEAPON_CLASS(weapon_gore_shotgun, WeaponGorePumper, C_WeaponShotgun);
STUB_WEAPON_CLASS(weapon_gore_357, WeaponGoreSpinner, C_Weapon357);
STUB_WEAPON_CLASS(weapon_gore_crowbar, WeaponGoreClub, C_WeaponCrowbar);
STUB_WEAPON_CLASS(weapon_gore_crossbow, WeaponGoreBow, C_WeaponCrossbow);
#endif // GOREAGULATION_WEAPONS

#endif
