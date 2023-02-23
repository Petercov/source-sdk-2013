//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//


#include "cbase.h"
#include "dlight.h"
#include "tempent.h"
#include "iefx.h"
#include "c_te_legacytempents.h"
#include "basegrenade_shared.h"


class C_HL1RpgRocket : public C_BaseGrenade
{
public:
	DECLARE_CLASS( C_HL1RpgRocket, C_BaseGrenade );
	DECLARE_CLIENTCLASS();

public:
	C_HL1RpgRocket( void ) { }
	C_HL1RpgRocket( const C_HL1RpgRocket & );

public:
	void	CreateLightEffects( void );
};


IMPLEMENT_CLIENTCLASS_DT( C_HL1RpgRocket, DT_HL1RpgRocket, CHL1RpgRocket )
END_RECV_TABLE()


void C_HL1RpgRocket::CreateLightEffects( void )
{
	dlight_t *dl;
	if ( IsEffectActive(EF_DIMLIGHT) )
	{			
		dl = effects->CL_AllocDlight ( index );
		dl->origin = GetAbsOrigin();
		dl->color.r = dl->color.g = dl->color.b = 100;
		dl->radius = 200;
		dl->die = gpGlobals->curtime + 0.001;

		tempents->RocketFlare( GetAbsOrigin() );
	}
}
