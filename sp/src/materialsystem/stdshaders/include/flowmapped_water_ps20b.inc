// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW
// defined $HDRTYPE && defined $HDRENABLED && !$HDRTYPE && $HDRENABLED
// defined $PIXELFOGTYPE && defined $WRITEWATERFOGTODESTALPHA && ( $PIXELFOGTYPE != 1 ) && $WRITEWATERFOGTODESTALPHA
// defined $LIGHTING_PREVIEW && defined $HDRTYPE && $LIGHTING_PREVIEW && $HDRTYPE != 0
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPTINT && $LIGHTING_PREVIEW && $FASTPATHENVMAPTINT
// defined $LIGHTING_PREVIEW && defined $FASTPATHENVMAPCONTRAST && $LIGHTING_PREVIEW && $FASTPATHENVMAPCONTRAST
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH
// ($FLASHLIGHT || $FLASHLIGHTSHADOWS) && $LIGHTING_PREVIEW

#pragma once
#include "shaderlib/cshader.h"
class flowmapped_water_ps20b_Static_Index
{
	unsigned int m_nBASETEXTURE : 2;
	unsigned int m_nENVMAP : 2;
	unsigned int m_nREFRACT : 2;
	unsigned int m_nABOVEWATER : 2;
	unsigned int m_nFLOW_DEBUG : 2;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nLIGHTMAPWATERFOG : 2;
	unsigned int m_nFORCEFRESNEL : 2;
#ifdef _DEBUG
	bool m_bBASETEXTURE : 1;
	bool m_bENVMAP : 1;
	bool m_bREFRACT : 1;
	bool m_bABOVEWATER : 1;
	bool m_bFLOW_DEBUG : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bLIGHTMAPWATERFOG : 1;
	bool m_bFORCEFRESNEL : 1;
#endif	// _DEBUG
public:
	void SetBASETEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBASETEXTURE = i;
#ifdef _DEBUG
		m_bBASETEXTURE = true;
#endif	// _DEBUG
	}

	void SetENVMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAP = i;
#ifdef _DEBUG
		m_bENVMAP = true;
#endif	// _DEBUG
	}

	void SetREFRACT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nREFRACT = i;
#ifdef _DEBUG
		m_bREFRACT = true;
#endif	// _DEBUG
	}

	void SetABOVEWATER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nABOVEWATER = i;
#ifdef _DEBUG
		m_bABOVEWATER = true;
#endif	// _DEBUG
	}

	void SetFLOW_DEBUG( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nFLOW_DEBUG = i;
#ifdef _DEBUG
		m_bFLOW_DEBUG = true;
#endif	// _DEBUG
	}

	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif	// _DEBUG
	}

	void SetLIGHTMAPWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTMAPWATERFOG = i;
#ifdef _DEBUG
		m_bLIGHTMAPWATERFOG = true;
#endif	// _DEBUG
	}

	void SetFORCEFRESNEL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFORCEFRESNEL = i;
#ifdef _DEBUG
		m_bFORCEFRESNEL = true;
#endif	// _DEBUG
	}

	flowmapped_water_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBASETEXTURE = 0;
		m_nENVMAP = 0;
		m_nREFRACT = 0;
		m_nABOVEWATER = 0;
		m_nFLOW_DEBUG = 0;
		m_nFLASHLIGHT = 0;
		m_nLIGHTMAPWATERFOG = 0;
		m_nFORCEFRESNEL = 0;
#ifdef _DEBUG
		m_bBASETEXTURE = false;
		m_bENVMAP = false;
		m_bREFRACT = false;
		m_bABOVEWATER = false;
		m_bFLOW_DEBUG = false;
		m_bFLASHLIGHT = false;
		m_bLIGHTMAPWATERFOG = false;
		m_bFORCEFRESNEL = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBASETEXTURE && m_bENVMAP && m_bREFRACT && m_bABOVEWATER && m_bFLOW_DEBUG && m_bFLASHLIGHT && m_bLIGHTMAPWATERFOG && m_bFORCEFRESNEL );
		return ( 8 * m_nBASETEXTURE ) + ( 16 * m_nENVMAP ) + ( 32 * m_nREFRACT ) + ( 64 * m_nABOVEWATER ) + ( 128 * m_nFLOW_DEBUG ) + ( 384 * m_nFLASHLIGHT ) + ( 768 * m_nLIGHTMAPWATERFOG ) + ( 1536 * m_nFORCEFRESNEL ) + 0;
	}
};

#define shaderStaticTest_flowmapped_water_ps20b psh_forgot_to_set_static_BASETEXTURE + psh_forgot_to_set_static_ENVMAP + psh_forgot_to_set_static_REFRACT + psh_forgot_to_set_static_ABOVEWATER + psh_forgot_to_set_static_FLOW_DEBUG + psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_LIGHTMAPWATERFOG + psh_forgot_to_set_static_FORCEFRESNEL


class flowmapped_water_ps20b_Dynamic_Index
{
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
#ifdef _DEBUG
	bool m_bFLASHLIGHTSHADOWS : 1;
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
#endif	// _DEBUG
public:
	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif	// _DEBUG
	}

	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
	}

	flowmapped_water_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nFLASHLIGHTSHADOWS = 0;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
		m_nPIXELFOGTYPE = ( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = false;
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bFLASHLIGHTSHADOWS && m_bWRITE_DEPTH_TO_DESTALPHA );
		return ( 1 * m_nFLASHLIGHTSHADOWS ) + ( 2 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 4 * m_nPIXELFOGTYPE ) + 0;
	}
};

#define shaderDynamicTest_flowmapped_water_ps20b psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA

