#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "fx_line.h"
#include "clienteffectprecachesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern Vector GetTracerOrigin(const CEffectData& data);
extern void FX_TracerSound(const Vector& start, const Vector& end, int iTracerType);

CLIENTEFFECT_REGISTER_BEGIN(PrecacheHL1Tracers)
CLIENTEFFECT_MATERIAL("hl1/tracer_middle")
CLIENTEFFECT_REGISTER_END()

void HL1TracerCallback(const CEffectData& data)
{
	C_BasePlayer* player = C_BasePlayer::GetLocalPlayer();

	if (player == NULL)
		return;

	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	int iEntIndex = data.entindex();

	if (iEntIndex && iEntIndex == player->index)
	{
		vecStart = data.m_vStart;
		QAngle	vangles;
		Vector	vforward, vright, vup;

		engine->GetViewAngles(vangles);
		AngleVectors(vangles, &vforward, &vright, &vup);

		VectorMA(data.m_vStart, 4, vright, vecStart);
		vecStart[2] -= 0.5f;
	}

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 10000;
	}

	//Don't make small tracers
	float dist;
	Vector dir;

	VectorSubtract(data.m_vOrigin, vecStart, dir);
	dist = VectorNormalize(dir);

	// Don't make short tracers.
	if (dist >= 128)
	{
		float length = random->RandomFloat(64.0f, 128.0f);
		float life = (dist + length) / flVelocity;	//NOTENOTE: We want the tail to finish its run as well

		//Add it
		FX_AddDiscreetLine(vecStart, dir, flVelocity, length, dist, random->RandomFloat(0.8f, 1.2f), life, "hl1/tracer_middle");
	}

	if (bWhiz)
	{
		FX_TracerSound(vecStart, data.m_vOrigin, TRACER_TYPE_DEFAULT);
	}
}
DECLARE_CLIENT_EFFECT("HL1Tracer", HL1TracerCallback);

void HL1TracerLargeCallback(const CEffectData& data)
{
	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 10000;
	}

	//Don't make small tracers
	float dist;
	Vector dir;

	VectorSubtract(data.m_vOrigin, vecStart, dir);
	dist = VectorNormalize(dir);

	// Don't make short tracers.
	if (dist >= 256)
	{
		float length = random->RandomFloat(256.0f, 384.0f);
		float life = (dist + length) / flVelocity;	//NOTENOTE: We want the tail to finish its run as well

		//Add it
		FX_AddDiscreetLine(vecStart, dir, flVelocity, length, dist, 5.f, life, "hl1/tracer_middle");
	}

	if (bWhiz)
	{
		FX_TracerSound(vecStart, data.m_vOrigin, TRACER_TYPE_GUNSHIP);
	}
}
DECLARE_CLIENT_EFFECT("HL1TracerLarge", HL1TracerLargeCallback);