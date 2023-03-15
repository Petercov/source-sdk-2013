#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_baseplayer.h"

#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

// memdbgon must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"

bool ShouldDrawScopeHUD()
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
	return pWeapon && pWeapon->IsWeaponZoomed() && pWeapon->GetWpnData().hudScope;
}

/**
 * Simple HUD element for displaying a sniper scope on screen
 */
class CHudScope : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudScope, vgui::Panel);

public:
	CHudScope(const char* pElementName);

	void Init();
	
	virtual void PerformLayout()
	{
		int w, h;
		GetHudSize(w, h);

		// fill the screen
		SetBounds(0, 0, w, h);

		BaseClass::PerformLayout();
	}

protected:
	virtual void ApplySchemeSettings(vgui::IScheme* scheme);
	virtual void Paint(void);

private:
	bool			m_bShow;
};

DECLARE_HUDELEMENT(CHudScope);

using namespace vgui;

/**
 * Constructor - generic HUD element initialization stuff. Make sure our 2 member variables
 * are instantiated.
 */
CHudScope::CHudScope(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudScope")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_bShow = false;

	// Scope will not show when the player is dead
	SetHiddenBits(HIDEHUD_PLAYERDEAD);
}

/**
 * Hook up our HUD message, and make sure we are not showing the scope
 */
void CHudScope::Init()
{	
	m_bShow = false;
}

/**
 * Load  in the scope material here
 */
void CHudScope::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);
}

/**
 * Simple - if we want to show the scope, draw it. Otherwise don't.
 */
void CHudScope::Paint(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	CHudTexture* pScope = 0;
	C_BaseCombatWeapon* pWeapon = pPlayer->GetActiveWeapon();
	if (pWeapon)
		pScope = pWeapon->GetWpnData().hudScope;

	if (pScope)
	{
		if (pWeapon->IsWeaponZoomed())
		{
			//Perform depth hack to prevent clips by world
		//materials->DepthRange( 0.0f, 0.1f );

			int x1 = (GetWide() / 2) - (GetTall() / 2);
			int x2 = GetWide() - (x1 * 2);
			int x3 = GetWide() - x1;

			surface()->DrawSetColor(Color(0, 0, 0, 255));
			surface()->DrawFilledRect(0, 0, x1, GetTall()); //Fill in the left side

			surface()->DrawSetColor(Color(0, 0, 0, 255));
			surface()->DrawFilledRect(x3, 0, GetWide(), GetTall()); //Fill in the right side

			pScope->DrawSelf(x1, 0, x2, GetTall(), Color(255, 255, 255, 255)); //Draw the scope as a perfect square

			//Restore depth
			//materials->DepthRange( 0.0f, 1.0f );

		// Hide the crosshair
			//pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
		}
		/*else if ((pPlayer->m_Local.m_iHideHUD & HIDEHUD_CROSSHAIR) != 0)
		{
			pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
		}*/
	}
}