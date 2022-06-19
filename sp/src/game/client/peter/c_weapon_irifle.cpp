#include "cbase.h"

#include "model_types.h"
#include "clienteffectprecachesystem.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"
#include "particle_property.h"
#include "clientsideeffects.h"
#include "fx_quad.h"

class C_PlasmaBolt : public C_BaseCombatCharacter
{
	DECLARE_CLASS(C_PlasmaBolt, C_BaseCombatCharacter);
	DECLARE_CLIENTCLASS();
public:

	C_PlasmaBolt(void);

	virtual RenderGroup_t GetRenderGroup(void)
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TWOPASS;
	}

	virtual void	OnDataChanged(DataUpdateType_t updateType);

private:

	C_PlasmaBolt(const C_PlasmaBolt&); // not defined, not accessible
};

IMPLEMENT_CLIENTCLASS_DT(C_PlasmaBolt, DT_PlasmaBolt, CPlasmaBolt)
END_RECV_TABLE();

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_PlasmaBolt::C_PlasmaBolt(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_PlasmaBolt::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		ParticleProp()->Create("irifle_smoke", PATTACH_ABSORIGIN_FOLLOW);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void PlasmaBoltImpactCallback(const CEffectData& data)
{
	QAngle angles;
	matrix3x4_t matRotation;
	Vector left, forward;
	VectorVectors(data.m_vNormal, left, forward);
	MatrixSetColumn(forward, 0, matRotation);
	MatrixSetColumn(left, 1, matRotation);
	MatrixSetColumn(data.m_vNormal, 2, matRotation);
	MatrixAngles(matRotation, angles);

	DispatchParticleEffect("irifle_sparks", data.m_vOrigin, angles);

	FX_AddQuad(data.m_vOrigin,
		data.m_vNormal,
		24,
		16,
		0,
		1.0f,
		0.0f,
		0.4f,
		random->RandomInt(0, 360),
		0,
		Vector(1.0f, 1.0f, 1.0f),
		0.5f,
		"effects/redglow",
		(FXQUAD_BIAS_ALPHA));
}

DECLARE_CLIENT_EFFECT("IrifleBoltImpact", PlasmaBoltImpactCallback);