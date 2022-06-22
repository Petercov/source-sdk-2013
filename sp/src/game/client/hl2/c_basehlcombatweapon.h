//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "basehlcombatweapon_shared.h"

#ifndef C_BASEHLCOMBATWEAPON_H
#define C_BASEHLCOMBATWEAPON_H
#ifdef _WIN32
#pragma once
#endif

#ifndef MAPBASE
class C_HLMachineGun : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS(C_HLMachineGun, C_BaseHLCombatWeapon);
#else
class C_HLMachineGun : public C_BaseHLZoomableWeapon
{
public:
	DECLARE_CLASS(C_HLMachineGun, C_BaseHLZoomableWeapon);
#endif // !MAPBASE

	DECLARE_CLIENTCLASS();
};

class C_HLSelectFireMachineGun : public C_HLMachineGun
{
public:
	DECLARE_CLASS( C_HLSelectFireMachineGun, C_HLMachineGun );
	DECLARE_CLIENTCLASS();
};

class C_BaseHLBludgeonWeapon : public C_BaseHLCombatWeapon
{
public:
	DECLARE_CLASS( C_BaseHLBludgeonWeapon, C_BaseHLCombatWeapon );
	DECLARE_CLIENTCLASS();
};

#endif // C_BASEHLCOMBATWEAPON_H
