#include "cbase.h"
#include "c_te_effect_dispatch.h"
#include "particles_localspace.h"
#include "fx.h"
#include "clienteffectprecachesystem.h"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheHL1MuzzleFlash)
CLIENTEFFECT_MATERIAL("hl1/sprites/muzzleflash")
CLIENTEFFECT_MATERIAL("hl1/sprites/muzzleflash1")
CLIENTEFFECT_MATERIAL("particle/particle_smokegrenade")
CLIENTEFFECT_REGISTER_END();

void HL1MuzzleFlash(const CEffectData& data)
{
	if (!data.GetEntity())
		return;

	CSmartPtr<CLocalSpaceEmitter> pFlash = CLocalSpaceEmitter::Create("HL1MuzzleFlash", data.m_hEntity, data.m_nAttachmentIndex);

	if (data.m_nColor == 0)
	{
		SimpleParticle* pParticle = (SimpleParticle*)pFlash->AddParticle(sizeof(SimpleParticle), pFlash->GetPMaterial("hl1/sprites/muzzleflash"), vec3_origin);

		if (pParticle)
		{
			pParticle->m_vecVelocity = vec3_origin;
			pParticle->m_flDieTime = RandomFloat(.04f, .07f);
			pParticle->m_uchStartAlpha = 255;
			pParticle->m_uchStartSize = RandomFloat(1.f, 1.2f) * data.m_flScale;
			pParticle->m_uchEndSize = RandomFloat(4.f, 6.f) * data.m_flScale;
			pParticle->m_flRoll = RandomFloat(-180.f, 180.f);
			pParticle->m_flRollDelta = RandomFloat(-1.f, 1.f);
			pParticle->m_uchColor[0] = 255;
			pParticle->m_uchColor[1] = 255;
			pParticle->m_uchColor[2] = 255;
		}
	}
	else if (data.m_nColor == 1)
	{
		SimpleParticle* pParticle = (SimpleParticle*)pFlash->AddParticle(sizeof(SimpleParticle), pFlash->GetPMaterial("hl1/sprites/muzzleflash1"), vec3_origin);

		if (pParticle)
		{
			pParticle->m_vecVelocity = vec3_origin;
			pParticle->m_flDieTime = RandomFloat(.04f, .07f);
			pParticle->m_uchStartAlpha = 255;
			pParticle->m_uchStartSize = RandomFloat(.1f, 2.f) * data.m_flScale;
			pParticle->m_uchEndSize = RandomFloat(8.f, 14.f) * data.m_flScale;
			pParticle->m_flRoll = RandomFloat(-180.f, 180.f);
			pParticle->m_flRollDelta = RandomFloat(-1.f, 1.f);
			pParticle->m_uchColor[0] = 255;
			pParticle->m_uchColor[1] = 255;
			pParticle->m_uchColor[2] = 255;
		}
	}

	QAngle vAngles;
	Vector vecPos, vecForward;
	if (data.GetRenderable()->GetAttachment(data.m_nAttachmentIndex, vecPos, vAngles))
	{
		AngleVectors(vAngles, &vecForward);
		CSmartPtr<CSimpleEmitter> pSmoke = CSimpleEmitter::Create("HL1MuzzleSmoke");

		for (int i = 0; i < 4; i++)
		{
			unsigned char uchSmokeCol = RandomInt(40, 170);
			SimpleParticle* pParticle = (SimpleParticle*)pSmoke->AddParticle(sizeof(SimpleParticle), pSmoke->GetPMaterial("particle/particle_smokegrenade"), vecPos);
			if (pParticle)
			{
				pParticle->m_vecVelocity = (40 * vecForward) + Vector(0, 0, RandomFloat(50.f, 140.f));
				pParticle->m_flDieTime = RandomFloat(.1f, .5f);
				pParticle->m_uchStartAlpha = RandomInt(160, 255);
				pParticle->m_uchEndAlpha = 0;
				pParticle->m_uchStartSize = RandomFloat(0.f, 1.f) * data.m_flScale;
				pParticle->m_uchEndSize = RandomFloat(5.f, 15.f) * data.m_flScale;
				pParticle->m_flRoll = RandomFloat(-180.f, 180.f);
				pParticle->m_flRollDelta = RandomFloat(-3.f, 3.f);
				pParticle->m_uchColor[0] = uchSmokeCol;
				pParticle->m_uchColor[1] = uchSmokeCol;
				pParticle->m_uchColor[2] = uchSmokeCol;
			}
		}

		ColorRGBExp32 clr;
		clr.r = 255;
		clr.g = 140;
		clr.b = 30;
		clr.exponent = 4;
		CreateMuzzleflashLight(vecPos, clr, 75, 75, data.m_hEntity);
	}
}
DECLARE_CLIENT_EFFECT("HL1MuzzleFlash", HL1MuzzleFlash);