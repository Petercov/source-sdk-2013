// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ($CUBEMAP || FLASHLIGHT ) && $LIGHT_PREVIEW
// ($PIXELFOGTYPE == 0) && ($WRITEWATERFOGTODESTALPHA != 0)
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// ( $CUBEMAP == 0 ) && ( $OLDIBL == 0 ) && ( $OLDIBL == 1 )
// ( $FLASHLIGHT == 1 ) && ( $OLDIBL == 0 ) && ( $OLDIBL == 1 )
// ( $CUBEMAP == 1 ) && ( $FLASHLIGHT == 1 )
// ( $FLASHLIGHT == 1) && ( $CSM == 1)
// $CUBEMAP_SPHERE_LEGACY && ($CUBEMAP == 0)
// ($CSM == 0) && ($CSM_PERF != 0)
// $PARALLAXCORRECT && !$CUBEMAP
// ($SMOOTHNESS == 1) && ($MRAOTEX == 1)
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
class lightmappedPBR_ps30_Static_Index
{
	unsigned int m_nCONVERT_TO_SRGB : 1;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nCUBEMAP : 2;
	unsigned int m_nSMOOTHNESS : 2;
	unsigned int m_nSEAMLESS : 2;
	unsigned int m_nBUMPMAP : 2;
	unsigned int m_nPARALLAXCORRECT : 2;
	unsigned int m_nMRAOTEX : 2;
	unsigned int m_nBUMPALPHASMOOTHNESS : 2;
#ifdef _DEBUG
	bool m_bCONVERT_TO_SRGB : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bCUBEMAP : 1;
	bool m_bSMOOTHNESS : 1;
	bool m_bSEAMLESS : 1;
	bool m_bBUMPMAP : 1;
	bool m_bPARALLAXCORRECT : 1;
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

	void SetSEAMLESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS = i;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif	// _DEBUG
	}

	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif	// _DEBUG
	}

	void SetPARALLAXCORRECT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPARALLAXCORRECT = i;
#ifdef _DEBUG
		m_bPARALLAXCORRECT = true;
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

	lightmappedPBR_ps30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nCONVERT_TO_SRGB = 0;
		m_nFLASHLIGHT = 0;
		m_nCUBEMAP = 0;
		m_nSMOOTHNESS = 0;
		m_nSEAMLESS = 0;
		m_nBUMPMAP = 0;
		m_nPARALLAXCORRECT = 0;
		m_nMRAOTEX = 0;
		m_nBUMPALPHASMOOTHNESS = 0;
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = false;
		m_bFLASHLIGHT = false;
		m_bCUBEMAP = false;
		m_bSMOOTHNESS = false;
		m_bSEAMLESS = false;
		m_bBUMPMAP = false;
		m_bPARALLAXCORRECT = false;
		m_bMRAOTEX = false;
		m_bBUMPALPHASMOOTHNESS = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCONVERT_TO_SRGB && m_bFLASHLIGHT && m_bCUBEMAP && m_bSMOOTHNESS && m_bSEAMLESS && m_bBUMPMAP && m_bPARALLAXCORRECT && m_bMRAOTEX && m_bBUMPALPHASMOOTHNESS );
		AssertMsg( !( ( m_nCUBEMAP == 1 ) && ( m_nFLASHLIGHT == 1 ) ), "Invalid combo combination ( ( CUBEMAP == 1 ) && ( FLASHLIGHT == 1 ) )" );
		AssertMsg( !( m_nPARALLAXCORRECT && !m_nCUBEMAP ), "Invalid combo combination ( PARALLAXCORRECT && !CUBEMAP )" );
		AssertMsg( !( ( m_nSMOOTHNESS == 1 ) && ( m_nMRAOTEX == 1 ) ), "Invalid combo combination ( ( SMOOTHNESS == 1 ) && ( MRAOTEX == 1 ) )" );
		AssertMsg( !( ( m_nBUMPALPHASMOOTHNESS == 1 ) && ( m_nMRAOTEX == 1 ) ), "Invalid combo combination ( ( BUMPALPHASMOOTHNESS == 1 ) && ( MRAOTEX == 1 ) )" );
		AssertMsg( !( ( m_nBUMPALPHASMOOTHNESS == 1 ) && ( m_nSMOOTHNESS == 1 ) ), "Invalid combo combination ( ( BUMPALPHASMOOTHNESS == 1 ) && ( SMOOTHNESS == 1 ) )" );
		return ( 48 * m_nCONVERT_TO_SRGB ) + ( 48 * m_nFLASHLIGHT ) + ( 96 * m_nCUBEMAP ) + ( 192 * m_nSMOOTHNESS ) + ( 384 * m_nSEAMLESS ) + ( 768 * m_nBUMPMAP ) + ( 1536 * m_nPARALLAXCORRECT ) + ( 3072 * m_nMRAOTEX ) + ( 6144 * m_nBUMPALPHASMOOTHNESS ) + 0;
	}
};

#define shaderStaticTest_lightmappedPBR_ps30 psh_forgot_to_set_static_CONVERT_TO_SRGB + psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_SMOOTHNESS + psh_forgot_to_set_static_SEAMLESS + psh_forgot_to_set_static_BUMPMAP + psh_forgot_to_set_static_PARALLAXCORRECT + psh_forgot_to_set_static_MRAOTEX + psh_forgot_to_set_static_BUMPALPHASMOOTHNESS


class lightmappedPBR_ps30_Dynamic_Index
{
	unsigned int m_nWRITEWATERFOGTODESTALPHA : 2;
	unsigned int m_nPIXELFOGTYPE : 2;
	unsigned int m_nWRITE_DEPTH_TO_DESTALPHA : 2;
	unsigned int m_nFLASHLIGHTSHADOWS : 2;
	unsigned int m_nCSM : 1;
	unsigned int m_nCSM_PERF : 1;
	unsigned int m_nLIGHT_PREVIEW : 2;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA : 1;
	bool m_bPIXELFOGTYPE : 1;
	bool m_bWRITE_DEPTH_TO_DESTALPHA : 1;
	bool m_bFLASHLIGHTSHADOWS : 1;
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

	lightmappedPBR_ps30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nWRITEWATERFOGTODESTALPHA = 0;
		m_nPIXELFOGTYPE = 0;
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
		m_nFLASHLIGHTSHADOWS = 0;
		m_nCSM = 0;
		m_nCSM_PERF = 0;
		m_nLIGHT_PREVIEW = 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
		m_bPIXELFOGTYPE = false;
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
		m_bFLASHLIGHTSHADOWS = false;
		m_bLIGHT_PREVIEW = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE && m_bWRITE_DEPTH_TO_DESTALPHA && m_bFLASHLIGHTSHADOWS && m_bLIGHT_PREVIEW );
		AssertMsg( !( ( m_nPIXELFOGTYPE == 0 ) && ( m_nWRITEWATERFOGTODESTALPHA != 0 ) ), "Invalid combo combination ( ( PIXELFOGTYPE == 0 ) && ( WRITEWATERFOGTODESTALPHA != 0 ) )" );
		AssertMsg( !( ( m_nCSM == 0 ) && ( m_nCSM_PERF != 0 ) ), "Invalid combo combination ( ( CSM == 0 ) && ( CSM_PERF != 0 ) )" );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		AssertMsg( !( 1 && ( 1 && ( ( m_nPIXELFOGTYPE != 1 ) && m_nWRITEWATERFOGTODESTALPHA ) ) ), "Invalid combo combination ( 1 && ( 1 && ( ( PIXELFOGTYPE != 1 ) && WRITEWATERFOGTODESTALPHA ) ) )" );
		return ( 1 * m_nWRITEWATERFOGTODESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + ( 4 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 8 * m_nFLASHLIGHTSHADOWS ) + ( 16 * m_nCSM ) + ( 16 * m_nCSM_PERF ) + ( 16 * m_nLIGHT_PREVIEW ) + 0;
	}
};

#define shaderDynamicTest_lightmappedPBR_ps30 psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_LIGHT_PREVIEW

