// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// ( $FLASHLIGHT == 0 ) && ( $FLASHLIGHTSHADOWS == 1 )
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class flowmapped_water_vs20_Static_Index
{
	unsigned int m_nBASETEXTURE : 2;
	unsigned int m_nFLASHLIGHT : 2;
	unsigned int m_nLIGHTMAPWATERFOG : 2;
#ifdef _DEBUG
	bool m_bBASETEXTURE : 1;
	bool m_bFLASHLIGHT : 1;
	bool m_bLIGHTMAPWATERFOG : 1;
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

	flowmapped_water_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nBASETEXTURE = 0;
		m_nFLASHLIGHT = 0;
		m_nLIGHTMAPWATERFOG = 0;
#ifdef _DEBUG
		m_bBASETEXTURE = false;
		m_bFLASHLIGHT = false;
		m_bLIGHTMAPWATERFOG = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bBASETEXTURE && m_bFLASHLIGHT && m_bLIGHTMAPWATERFOG );
		return ( 1 * m_nBASETEXTURE ) + ( 2 * m_nFLASHLIGHT ) + ( 4 * m_nLIGHTMAPWATERFOG ) + 0;
	}
};

#define shaderStaticTest_flowmapped_water_vs20 vsh_forgot_to_set_static_BASETEXTURE + vsh_forgot_to_set_static_FLASHLIGHT + vsh_forgot_to_set_static_LIGHTMAPWATERFOG


class flowmapped_water_vs20_Dynamic_Index
{
public:
	flowmapped_water_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_flowmapped_water_vs20 1

