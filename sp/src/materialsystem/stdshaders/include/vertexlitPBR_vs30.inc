// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#pragma once
#include "shaderlib/cshader.h"
class vertexlitPBR_vs30_Static_Index
{
	unsigned int m_nVERTEXCOLOR : 2;
	unsigned int m_nCUBEMAP : 2;
	unsigned int m_nDONT_GAMMA_CONVERT_VERTEX_COLOR : 2;
#ifdef _DEBUG
	bool m_bVERTEXCOLOR : 1;
	bool m_bCUBEMAP : 1;
	bool m_bDONT_GAMMA_CONVERT_VERTEX_COLOR : 1;
#endif	// _DEBUG
public:
	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
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

	void SetDONT_GAMMA_CONVERT_VERTEX_COLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDONT_GAMMA_CONVERT_VERTEX_COLOR = i;
#ifdef _DEBUG
		m_bDONT_GAMMA_CONVERT_VERTEX_COLOR = true;
#endif	// _DEBUG
	}

	vertexlitPBR_vs30_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nVERTEXCOLOR = 0;
		m_nCUBEMAP = 0;
		m_nDONT_GAMMA_CONVERT_VERTEX_COLOR = 0;
#ifdef _DEBUG
		m_bVERTEXCOLOR = false;
		m_bCUBEMAP = false;
		m_bDONT_GAMMA_CONVERT_VERTEX_COLOR = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bVERTEXCOLOR && m_bCUBEMAP && m_bDONT_GAMMA_CONVERT_VERTEX_COLOR );
		return ( 128 * m_nVERTEXCOLOR ) + ( 256 * m_nCUBEMAP ) + ( 512 * m_nDONT_GAMMA_CONVERT_VERTEX_COLOR ) + 0;
	}
};

#define shaderStaticTest_vertexlitPBR_vs30 vsh_forgot_to_set_static_VERTEXCOLOR + vsh_forgot_to_set_static_CUBEMAP + vsh_forgot_to_set_static_DONT_GAMMA_CONVERT_VERTEX_COLOR


class vertexlitPBR_vs30_Dynamic_Index
{
	unsigned int m_nCOMPRESSED_VERTS : 2;
	unsigned int m_nDOWATERFOG : 2;
	unsigned int m_nSKINNING : 2;
	unsigned int m_nLIGHTING_PREVIEW : 2;
	unsigned int m_nDYNAMIC_LIGHT : 2;
	unsigned int m_nSTATIC_LIGHT : 2;
	unsigned int m_nMORPHING : 2;
#ifdef _DEBUG
	bool m_bCOMPRESSED_VERTS : 1;
	bool m_bDOWATERFOG : 1;
	bool m_bSKINNING : 1;
	bool m_bLIGHTING_PREVIEW : 1;
	bool m_bDYNAMIC_LIGHT : 1;
	bool m_bSTATIC_LIGHT : 1;
	bool m_bMORPHING : 1;
#endif	// _DEBUG
public:
	void SetCOMPRESSED_VERTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCOMPRESSED_VERTS = i;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = true;
#endif	// _DEBUG
	}

	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif	// _DEBUG
	}

	void SetSKINNING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSKINNING = i;
#ifdef _DEBUG
		m_bSKINNING = true;
#endif	// _DEBUG
	}

	void SetLIGHTING_PREVIEW( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nLIGHTING_PREVIEW = i;
#ifdef _DEBUG
		m_bLIGHTING_PREVIEW = true;
#endif	// _DEBUG
	}

	void SetDYNAMIC_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDYNAMIC_LIGHT = i;
#ifdef _DEBUG
		m_bDYNAMIC_LIGHT = true;
#endif	// _DEBUG
	}

	void SetSTATIC_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSTATIC_LIGHT = i;
#ifdef _DEBUG
		m_bSTATIC_LIGHT = true;
#endif	// _DEBUG
	}

	void SetMORPHING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMORPHING = i;
#ifdef _DEBUG
		m_bMORPHING = true;
#endif	// _DEBUG
	}

	vertexlitPBR_vs30_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nCOMPRESSED_VERTS = 0;
		m_nDOWATERFOG = 0;
		m_nSKINNING = 0;
		m_nLIGHTING_PREVIEW = 0;
		m_nDYNAMIC_LIGHT = 0;
		m_nSTATIC_LIGHT = 0;
		m_nMORPHING = 0;
#ifdef _DEBUG
		m_bCOMPRESSED_VERTS = false;
		m_bDOWATERFOG = false;
		m_bSKINNING = false;
		m_bLIGHTING_PREVIEW = false;
		m_bDYNAMIC_LIGHT = false;
		m_bSTATIC_LIGHT = false;
		m_bMORPHING = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bCOMPRESSED_VERTS && m_bDOWATERFOG && m_bSKINNING && m_bLIGHTING_PREVIEW && m_bDYNAMIC_LIGHT && m_bSTATIC_LIGHT && m_bMORPHING );
		return ( 1 * m_nCOMPRESSED_VERTS ) + ( 2 * m_nDOWATERFOG ) + ( 4 * m_nSKINNING ) + ( 8 * m_nLIGHTING_PREVIEW ) + ( 16 * m_nDYNAMIC_LIGHT ) + ( 32 * m_nSTATIC_LIGHT ) + ( 64 * m_nMORPHING ) + 0;
	}
};

#define shaderDynamicTest_vertexlitPBR_vs30 vsh_forgot_to_set_dynamic_COMPRESSED_VERTS + vsh_forgot_to_set_dynamic_DOWATERFOG + vsh_forgot_to_set_dynamic_SKINNING + vsh_forgot_to_set_dynamic_LIGHTING_PREVIEW + vsh_forgot_to_set_dynamic_DYNAMIC_LIGHT + vsh_forgot_to_set_dynamic_STATIC_LIGHT + vsh_forgot_to_set_dynamic_MORPHING

