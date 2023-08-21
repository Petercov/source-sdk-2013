#include "cbase.h"
#include "weapon_minigun_base.h"

class CWeaponSuppressor : public CBaseHLMinigun
{
	DECLARE_CLASS(CWeaponSuppressor, CBaseHLMinigun);
	DECLARE_SERVERCLASS();
public:
	CWeaponSuppressor();

	const char* GetTracerType(void) { return "HelicopterTracer"; }
	void	DoImpactEffect(trace_t& tr, int nDamageType);

	virtual float	GetSpinupTime() { return 1.6f; }
	virtual float	GetFireRate(void) { return 0.1f; }

	int		GetMinBurst() { return GetMaxBurst(); }
	int		GetMaxBurst() { return GetMaxClip1(); }

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone = VECTOR_CONE_3DEGREES;
		return cone;
	}

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
};

LINK_ENTITY_TO_CLASS(weapon_suppressor_lmg, CWeaponSuppressor);

IMPLEMENT_SERVERCLASS_ST(CWeaponSuppressor, DT_WeaponSuppressor)
END_SEND_TABLE();

CWeaponSuppressor::CWeaponSuppressor()
{
}

void CWeaponSuppressor::DoImpactEffect(trace_t& tr, int nDamageType)
{
	UTIL_ImpactTrace(&tr, nDamageType, "AR2Impact");
}
