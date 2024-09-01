// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ($CUBEMAP || $FLASHLIGHT) && $LIGHT_PREVIEW
// ($PIXELFOGTYPE == 0) && ($WRITEWATERFOGTODESTALPHA != 0)
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ( $CUBEMAP == 1 ) && ( $FLASHLIGHT == 1 )
// ( $FLASHLIGHT == 1) && ( $CSM == 1)
// $CUBEMAP_SPHERE_LEGACY && ($CUBEMAP == 0)
// ($LIGHTMAP && $FLASHLIGHT)
// ($CSM == 0) && ($CSM_PERF != 0)
// $SMOOTHNESS && $MRAOTEX
// ($BUMPALPHASMOOTHNESS == 1) && ($MRAOTEX == 1)
// ($BUMPALPHASMOOTHNESS == 1) && ($SMOOTHNESS == 1)
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
class VertexlitPBR_ps30_Static_Index
{
	unsigned int m_nCONVERT_TO_SRGB : 1;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nCUBEMAP : 2;
	unsigned int m_nSMOOTHNESS : 2;
	unsigned int m_nMRAOTEX : 2;
	unsigned int m_nBUMPALPHASMOOTHNESS : 2;
#ifdef _DEBUG
	bool m_bCONVERT_TO_SRGB : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bCUBEMAP : 1;
	bool m_bSMOOTHNESS : 1;
	bool m_bMRAOTEX : 1;
	bool m_bBUMPALPHASMOOTHNESS : 1;
#endif	// _DEBUG
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nCONVERT_TO_SRGB = i;
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = true;
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

	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif	// _DEBUG
	}

	void SetSMOOTHNESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSMOOTHNESS = i;
#ifdef _DEBUG
		m_bSMOOTHNESS = true;
#endif	// _DEBUG
	}

	void SetMRAOTEX( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMRAOTEX = i;
#ifdef _DEBUG
		m_bMRAOTEX = true;
#endif	// _DEBUG
	}

	void SetBUMPALPHASMOOTHNESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPALPHASMOOTHNESS = i;
#ifdef _DEBUG
		m_bBUMPALPHASMOOTHNESS = true;
#endif	// _DEBUG
	}

	VertexlitPBR_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = 0;
		m_nFLASHLIGHT = 0;
		m_nCUBEMAP = 0;
		m_nSMOOTHNESS = 0;
		m_nMRAOTEX = 0;
		m_nBUMPALPHASMOOTHNESS = 0;
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = false;
		m_bFLASHLIGHT = false;
		m_bCUBEMAP = false;
		m_bSMOOTHNESS = false;
		m_bMRAOTEX = false;
		m_bBUMPALPHASMOOTHNESS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCONVERT_TO_SRGB && m_bFLASHLIGHT && m_bCUBEMAP && m_bSMOOTHNESS && m_bMRAOTEX && m_bBUMPALPHASMOOTHNESS );
		AssertMsg( !( ( m_nCUBEMAP == 1 ) && ( m_nFLASHLIGHT == 1 ) ), "Invalid combo combination ( ( CUBEMAP == 1 ) && ( FLASHLIGHT == 1 ) )" );
		AssertMsg( !( m_nSMOOTHNESS && m_nMRAOTEX ), "Invalid combo combination ( SMOOTHNESS && MRAOTEX )" );
		AssertMsg( !( ( m_nBUMPALPHASMOOTHNESS == 1 ) && ( m_nMRAOTEX == 1 ) ), "Invalid combo combination ( ( BUMPALPHASMOOTHNESS == 1 ) && ( MRAOTEX == 1 ) )" );
		AssertMsg( !( ( m_nBUMPALPHASMOOTHNESS == 1 ) && ( m_nSMOOTHNESS == 1 ) ), "Invalid combo combination ( ( BUMPALPHASMOOTHNESS == 1 ) && ( SMOOTHNESS == 1 ) )" );
		return ( 480 * m_nCONVERT_TO_SRGB ) + ( 480 * m_nFLASHLIGHT ) + ( 960 * m_nCUBEMAP ) + ( 1920 * m_nSMOOTHNESS ) + ( 3840 * m_nMRAOTEX ) + ( 7680 * m_nBUMPALPHASMOOTHNESS ) + 0;
	}
};

#define shaderStaticTest_VertexlitPBR_ps30 psh_forgot_to_set_static_CONVERT_TO_SRGB + psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_SMOOTHNESS + psh_forgot_to_set_static_MRAOTEX + psh_forgot_to_set_static_BUMPALPHASMOOTHNESS


class VertexlitPBR_ps30_Dynamic_Index
{
	unsigned int m_nWRITEWATERFOGTODESTALPHA : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nNUM_LIGHTS : 3;
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
	unsigned int m_nLIGHTMAP : 2;
	unsigned int m_nCSM : 1;
	unsigned int m_nCSM_PERF : 1;
	unsigned int m_nLIGHT_PREVIEW : 2;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
	bool m_bPIXELFOGTYPE : 1;
	bool m_bNUM_LIGHTS : 1;
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
	bool m_bFLASHLIGHTSHADOWS : 1;
	bool m_bLIGHTMAP : 1;
	bool m_bLIGHT_PREVIEW : 1;
#endif	// _DEBUG
public:
	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif	// _DEBUG
	}

	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif	// _DEBUG
	}

	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
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

	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif	// _DEBUG
	}

	void SetLIGHTMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTMAP = i;
#ifdef _DEBUG
		m_bLIGHTMAP = true;
#endif	// _DEBUG
	}

	void SetCSM( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nCSM = i;
	}

	void SetCSM_PERF( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nCSM_PERF = i;
	}

	void SetLIGHT_PREVIEW( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nLIGHT_PREVIEW = i;
#ifdef _DEBUG
		m_bLIGHT_PREVIEW = true;
#endif	// _DEBUG
	}

	VertexlitPBR_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nPIXELFOGTYPE = 0;
		m_nNUM_LIGHTS = 0;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
		m_nFLASHLIGHTSHADOWS = 0;
		m_nLIGHTMAP = 0;
		m_nCSM = 0;
		m_nCSM_PERF = 0;
		m_nLIGHT_PREVIEW = 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
		m_bPIXELFOGTYPE = false;
		m_bNUM_LIGHTS = false;
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
		m_bFLASHLIGHTSHADOWS = false;
		m_bLIGHTMAP = false;
		m_bLIGHT_PREVIEW = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE && m_bNUM_LIGHTS && m_bWRITE_DEPTH_TO_DESTALPHA && m_bFLASHLIGHTSHADOWS && m_bLIGHTMAP && m_bLIGHT_PREVIEW );
		AssertMsg( !( ( m_nPIXELFOGTYPE == 0 ) && ( m_nWRITEWATERFOGTODESTALPHA != 0 ) ), "Invalid combo combination ( ( PIXELFOGTYPE == 0 ) && ( WRITEWATERFOGTODESTALPHA != 0 ) )" );
		AssertMsg( !( ( m_nCSM == 0 ) && ( m_nCSM_PERF != 0 ) ), "Invalid combo combination ( ( CSM == 0 ) && ( CSM_PERF != 0 ) )" );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		return ( 1 * m_nWRITEWATERFOGTODESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + ( 4 * m_nNUM_LIGHTS ) + ( 20 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 40 * m_nFLASHLIGHTSHADOWS ) + ( 80 * m_nLIGHTMAP ) + ( 160 * m_nCSM ) + ( 160 * m_nCSM_PERF ) + ( 160 * m_nLIGHT_PREVIEW ) + 0;
	}
};

#define shaderDynamicTest_VertexlitPBR_ps30 psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_LIGHTMAP + psh_forgot_to_set_dynamic_LIGHT_PREVIEW

