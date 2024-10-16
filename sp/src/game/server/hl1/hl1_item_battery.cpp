//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the suit batteries.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"
#include "hl1_items.h"


#define BATTERY_MODEL "models/w_battery.mdl"

ConVar	sk_batteryhl1( "sk_battery_hl1","0" );			

class CItemBatteryHL1 : public CHL1Item
{
public:
	DECLARE_CLASS(CItemBatteryHL1, CHL1Item );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( BATTERY_MODEL );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel( BATTERY_MODEL );

		PrecacheScriptSound( "Item.Pickup" );
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ((pPlayer->ArmorValue() < MAX_NORMAL_BATTERY) && pPlayer->IsSuitEquipped())
		{
			int pct;
			char szcharge[64];

			pPlayer->IncrementArmorValue( sk_batteryhl1.GetFloat(), MAX_NORMAL_BATTERY );

			CPASAttenuationFilter filter( pPlayer, "Item.Pickup" );
			EmitSound( filter, pPlayer->entindex(), "Item.Pickup" );

			CSingleUserRecipientFilter user( pPlayer );
			user.MakeReliable();

			UserMessageBegin( user, "ItemPickup" );
				WRITE_STRING( GetClassname() );
			MessageEnd();

			
			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)( (float)(pPlayer->ArmorValue() * 100.0) * (1.0/MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;
		
			Q_snprintf( szcharge,sizeof(szcharge),"!HEV_%1dP", pct );
			
			//UTIL_EmitSoundSuit(edict(), szcharge);
			pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return true;		
		}
		return false;
	}

#ifdef MAPBASE
	// Used to access item_healthkit values, etc. from outside of the class
	virtual float GetItemAmount() { return sk_batteryhl1.GetFloat(); }
#endif // MAPBASE
};

#ifdef HL1_DLL
LINK_ENTITY_TO_CLASS(item_battery, CItemBatteryHL1);
PRECACHE_REGISTER(item_battery);
#else
LINK_ENTITY_TO_CLASS(item_battery_hl1, CItemBatteryHL1);
#endif // HL1_DLL


