//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "BaseVSShader.h"
#include "vertexlitPBR_dx9_helper.h"
#include "convar.h"
#include "cpp_shader_constant_register_map.h"
#include "vertexlitPBR_vs30.inc"
#include "vertexlitPBR_ps30.inc"
#include "commandbuilder.h"

//#include "IDeferredExt.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT );
//static ConVar r_rimlight( "r_rimlight", "1", FCVAR_CHEAT );

//extern ConVar r_csm_bias;
//extern ConVar r_csm_slopescalebias;
//extern ConVar r_csm_performance;

//-----------------------------------------------------------------------------
// Initialize shader parameters
//-----------------------------------------------------------------------------
void InitParamsVertexLitPBR_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, VertexLitPBR_DX9_Vars_t &info )
{	
	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	Assert( info.m_nFlashlightTexture >= 0 );
	
	params[info.m_nBRDF]->SetStringValue("models/PBRTest/BRDF");

	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	}

	if (((info.m_nBumpmap != -1) && g_pConfig->UseBumpmapping() && params[info.m_nBumpmap]->IsDefined())
		// we don't need a tangent space if we have envmap without bumpmap
		//		|| ( info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined() ) 
		)
	{
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	}

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
	}

	// This shader can be used with hw skinning
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
}

//-----------------------------------------------------------------------------
// Initialize shader
//-----------------------------------------------------------------------------
void InitVertexLitPBR_DX9( CBaseVSShader *pShader, IMaterialVar** params, VertexLitPBR_DX9_Vars_t &info )
{
	Assert( info.m_nFlashlightTexture >= 0 );
	pShader->LoadTexture(info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB);
	
	bool bIsBaseTextureTranslucent = false;
	if ( params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
		
		if ( params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent() )
		{
			bIsBaseTextureTranslucent = true;
		}
	}

	if (info.m_nMRAOTexture != -1 && params[info.m_nMRAOTexture]->IsDefined())
	{
		pShader->LoadTexture(info.m_nMRAOTexture);
	}
	else
	{
		if (info.m_nRoughness != -1 && params[info.m_nRoughness]->IsDefined())
		{
			pShader->LoadTexture(info.m_nRoughness);
		}
		if (info.m_nMetallic != -1 && params[info.m_nMetallic]->IsDefined())
		{
			pShader->LoadTexture(info.m_nMetallic);
		}
		if (info.m_nAO != -1 && params[info.m_nAO]->IsDefined())
		{
			pShader->LoadTexture(info.m_nAO);
		}
	}
	if (info.m_nEmissive != -1 && params[info.m_nEmissive]->IsDefined())
	{
		pShader->LoadTexture(info.m_nEmissive);
	}
	if (info.m_nBRDF != -1 && params[info.m_nBRDF]->IsDefined())
	{
		pShader->LoadTexture(info.m_nBRDF);
	}
	if (info.m_nLightmap != -1 && params[info.m_nLightmap]->IsDefined())
	{
		pShader->LoadTexture(info.m_nLightmap);
	}

	if (info.m_nEnvmap != -1 && params[info.m_nEnvmap]->IsDefined())
	{
		if (!IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE))
		{
			int flags = g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0;
			flags |= TEXTUREFLAGS_ALL_MIPS;
			pShader->LoadCubeMap(info.m_nEnvmap, flags);
		}
		else
		{
			pShader->LoadTexture(info.m_nEnvmap, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0);
		}

		if (!g_pHardwareConfig->SupportsCubeMaps())
		{
			SET_FLAGS(MATERIAL_VAR_ENVMAPSPHERE);
		}
	}

	if (g_pConfig->UseBumpmapping())
	{
		if ((info.m_nBumpmap != -1) && params[info.m_nBumpmap]->IsDefined())
		{
			pShader->LoadBumpMap(info.m_nBumpmap);
			SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);
		}
	}
}

class CVertexLitPBR_DX9_Context : public CBasePerMaterialContextData
{
public:
	CCommandBufferBuilder< CFixedCommandStorageBuffer< 800 > > m_SemiStaticCmdsOut;
	bool m_bFastPath;

};

//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
static void DrawVertexLitPBR_DX9_Internal( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
	bool bHasFlashlight, VertexLitPBR_DX9_Vars_t &info, VertexCompressionType_t vertexCompression,
							CBasePerMaterialContextData **pContextDataPtr )
{
	bool bHasBaseTexture = (info.m_nBaseTexture != -1) && params[info.m_nBaseTexture]->IsTexture();
	bool bHasMRAO = (info.m_nMRAOTexture != -1) && params[info.m_nMRAOTexture]->IsTexture();
	bool bHasRoughness = !bHasMRAO && (info.m_nRoughness != -1) && params[info.m_nRoughness]->IsTexture();
	bool bHasMetallic = !bHasMRAO && (info.m_nMetallic != -1) && params[info.m_nMetallic]->IsTexture();
	bool bHasAO = !bHasMRAO && (info.m_nAO != -1) && params[info.m_nAO]->IsTexture();
	bool bHasEmissive = (info.m_nEmissive != -1) && params[info.m_nEmissive]->IsTexture();
	bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;
	bool bHasEnvmap =(info.m_nEnvmap != -1) && params[info.m_nEnvmap]->IsTexture();
	//bool bHasLegacyEnvSphereMap = bHasEnvmap && IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE);
	bool bHasBump = IsTextureSet(info.m_nBumpmap, params);
	bool bBumpAlphaSmoothness = !bHasMRAO && bHasBump && info.m_nBumpAlphaSmoothness != -1 && params[info.m_nBumpAlphaSmoothness]->GetIntValue() == 1;
	bool bUseSmoothness = !bHasMRAO && !bBumpAlphaSmoothness && info.m_nUseSmoothness != -1 && params[info.m_nUseSmoothness]->GetIntValue() == 1;
	bool bHasLightmap = (info.m_nLightmap != -1) && params[info.m_nLightmap]->IsTexture();
	bool bIsDecal = IS_FLAG_SET(MATERIAL_VAR_DECAL);

	bool bHasVertexColor = IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR);
	bool bHasVertexAlpha = IS_FLAG_SET(MATERIAL_VAR_VERTEXALPHA);

	BlendType_t nBlendType= pShader->EvaluateBlendRequirements( info.m_nBaseTexture, true );
	bool bFullyOpaque = ( nBlendType != BT_BLENDADD ) && ( nBlendType != BT_BLEND ) && !bIsAlphaTested && !bHasFlashlight;

	CVertexLitPBR_DX9_Context *pContextData = reinterpret_cast< CVertexLitPBR_DX9_Context *> ( *pContextDataPtr );
	if ( !pContextData )
	{
		pContextData = new CVertexLitPBR_DX9_Context;
		*pContextDataPtr = pContextData;
	}

	if( pShader->IsSnapshotting() )
	{
		pShaderShadow->EnableAlphaTest( bIsAlphaTested );

		if( info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f )
		{
			pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue() );
		}

		int nShadowFilterMode = 0;
		if( bHasFlashlight )
		{
			if (params[info.m_nBaseTexture]->IsTexture())
			{
				pShader->SetAdditiveBlendingShadowState( info.m_nBaseTexture, true );
			}

			if( bIsAlphaTested )
			{
				// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to 
				// be the same on both the regular pass and the flashlight pass.
				pShaderShadow->EnableAlphaTest( false );
				pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
			}
			pShaderShadow->EnableBlending( true );
			pShaderShadow->EnableDepthWrites( false );

			// Be sure not to write to dest alpha
			pShaderShadow->EnableAlphaWrites( false );

			nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
		}
		else // not flashlight pass
		{
			if (params[info.m_nBaseTexture]->IsTexture())
			{
				pShader->SetDefaultBlendingShadowState( info.m_nBaseTexture, true );
			}
		}
		
		unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
		int userDataSize = 0;

		// Always enable...will bind white if nothing specified...
		pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );		// Base (albedo) map
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, true );

		if (!bHasMRAO)
		{
			if (!bBumpAlphaSmoothness)
				pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);		// Roughness map

			pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);		// Metallic map 
		}

		if (bHasEnvmap)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER7, true);
			if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
			{
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER7, true);
			}
		}


		if (bHasVertexColor || bHasVertexAlpha)
		{
			flags |= VERTEX_COLOR;
		}

		pShaderShadow->EnableTexture( SHADER_SAMPLER4, true ); // Shadow depth map
		pShaderShadow->SetShadowDepthFiltering( SHADER_SAMPLER4 );
		pShaderShadow->EnableSRGBRead( SHADER_SAMPLER4, false );

		if( bHasFlashlight )
		{
			pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );	// Noise map
			pShaderShadow->EnableTexture( SHADER_SAMPLER6, true );	// Flashlight cookie
			pShaderShadow->EnableSRGBRead( SHADER_SAMPLER6, true );
		}

		pShaderShadow->EnableTexture(SHADER_SAMPLER8, true);	// BRDF for IBL

		pShaderShadow->EnableTexture(SHADER_SAMPLER9, true);	// Ambient Occlusion / MRAO
		pShaderShadow->EnableTexture(SHADER_SAMPLER10, true);	// Emissive map
		pShaderShadow->EnableTexture(SHADER_SAMPLER11, true);	// Lightmap

		// Always enable, since flat normal will be bound
		pShaderShadow->EnableTexture( SHADER_SAMPLER3, true );		// Normal map
		userDataSize = 4; // tangent S
		pShaderShadow->EnableTexture( SHADER_SAMPLER5, true );		// Normalizing cube map
		pShaderShadow->EnableSRGBWrite( true );
		
		// texcoord0 : base texcoord
		int pTexCoordDim[3] = { 2, 2, 3 };
		int nTexCoordCount = 1;

		// Special morphed decal information 
		if (bIsDecal && g_pHardwareConfig->HasFastVertexTextures())
		{
			nTexCoordCount = 3;
		}

		// This shader supports compressed vertices, so OR in that flag:
		flags |= VERTEX_FORMAT_COMPRESSED;

		pShaderShadow->VertexShaderVertexFormat( flags, nTexCoordCount, pTexCoordDim, userDataSize );

		const bool bFastVertexTextures = g_pHardwareConfig->HasFastVertexTextures();

		// The vertex shader uses the vertex id stream
		if (bFastVertexTextures)
			SET_FLAGS2(MATERIAL_VAR2_USES_VERTEXID);

		DECLARE_STATIC_VERTEX_SHADER_NEW( vertexlitPBR_vs30 );
		SET_STATIC_VERTEX_SHADER_COMBO_NEW(VERTEXCOLOR, bHasVertexColor || bHasVertexAlpha);
		SET_STATIC_VERTEX_SHADER_COMBO_NEW(CUBEMAP, bHasEnvmap);
		SET_STATIC_VERTEX_SHADER_COMBO_NEW(DONT_GAMMA_CONVERT_VERTEX_COLOR, bHasVertexColor);
		SET_STATIC_VERTEX_SHADER_NEW(vertexlitPBR_vs30);

		// Assume we're only going to get in here if we support 2b
		DECLARE_STATIC_PIXEL_SHADER_NEW(VertexlitPBR_ps30 );
		SET_STATIC_PIXEL_SHADER_COMBO_NEW( CUBEMAP, bHasEnvmap && !bHasFlashlight);
		//SET_STATIC_PIXEL_SHADER_COMBO( CUBEMAP_SPHERE_LEGACY, bHasLegacyEnvSphereMap);
		SET_STATIC_PIXEL_SHADER_COMBO_NEW( FLASHLIGHT, bHasFlashlight );
		SET_STATIC_PIXEL_SHADER_COMBO_NEW( CONVERT_TO_SRGB, 0 );
		SET_STATIC_PIXEL_SHADER_COMBO_NEW(SMOOTHNESS, bUseSmoothness);
		SET_STATIC_PIXEL_SHADER_COMBO_NEW(MRAOTEX, bHasMRAO);
		SET_STATIC_PIXEL_SHADER_COMBO_NEW(BUMPALPHASMOOTHNESS, bBumpAlphaSmoothness);
		SET_STATIC_PIXEL_SHADER_NEW(VertexlitPBR_ps30);

		if( bHasFlashlight )
		{
			pShader->FogToBlack();
		}
		else
		{
			pShader->DefaultFog();
		}

		// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
		pShaderShadow->EnableAlphaWrites( bFullyOpaque );
	}
	else // not snapshotting -- begin dynamic state
	{
		bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET( MATERIAL_VAR_NO_DEBUG_OVERRIDE );

		if (bLightingOnly)
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER0, TEXTURE_GREY);
		else if( bHasBaseTexture )
			pShader->BindTexture( SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame );
		else
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER0, TEXTURE_WHITE );

		if (!bHasMRAO)
		{
			if (!bBumpAlphaSmoothness)
			{
				if (bHasRoughness)
					pShader->BindTexture(SHADER_SAMPLER1, info.m_nRoughness);
				else
					pShaderAPI->BindStandardTexture(SHADER_SAMPLER1, TEXTURE_WHITE);
			}

			if (bHasMetallic)
				pShader->BindTexture(SHADER_SAMPLER2, info.m_nMetallic);
			else
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER2, TEXTURE_BLACK);
		}

		if (bHasEnvmap)
			pShader->BindTexture(SHADER_SAMPLER7, info.m_nEnvmap);

		if (bHasMRAO)
			pShader->BindTexture(SHADER_SAMPLER9, info.m_nMRAOTexture);
		else if (bHasAO)
			pShader->BindTexture(SHADER_SAMPLER9, info.m_nAO);
		else
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER9, TEXTURE_WHITE);

		if (bHasEmissive)
			pShader->BindTexture(SHADER_SAMPLER10, info.m_nEmissive);
		else
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER10, TEXTURE_BLACK);

		if (bHasLightmap)
			pShader->BindTexture(SHADER_SAMPLER11, info.m_nLightmap);
		else
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER11, TEXTURE_WHITE);

		if (!g_pConfig->m_bFastNoBump)
		{
			if (bHasBump)
			{
				pShader->BindTexture(SHADER_SAMPLER3, info.m_nBumpmap);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT);
			}
		}
		else
		{
			if (bHasBump)
			{
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER3, TEXTURE_NORMALMAP_FLAT);
			}
		}

		pShader->BindTexture(SHADER_SAMPLER8, info.m_nBRDF);
		
		LightState_t lightState = { 0, false, false };
		bool bFlashlightShadows = false;
		if( bHasFlashlight )
		{
			Assert( info.m_nFlashlightTexture >= 0 && info.m_nFlashlightTextureFrame >= 0 );
			pShader->BindTexture( SHADER_SAMPLER6, info.m_nFlashlightTexture, info.m_nFlashlightTextureFrame );
			VMatrix worldToTexture;
			ITexture *pFlashlightDepthTexture;
			FlashlightState_t state = pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );
			bFlashlightShadows = state.m_bEnableShadows && ( pFlashlightDepthTexture != NULL );

			//SetFlashLightColorFromState( state, pShaderAPI, PSREG_FLASHLIGHT_COLOR );

			if( pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows )
			{
				pShader->BindTexture( SHADER_SAMPLER4, pFlashlightDepthTexture, 0 );
				pShaderAPI->BindStandardTexture( SHADER_SAMPLER5, TEXTURE_SHADOW_NOISE_2D );
			}

			float atten[4], pos[4], tweaks[4];

			float const* pFlashlightColor = state.m_Color;
			float vPsConst[4] = { pFlashlightColor[0], pFlashlightColor[1], pFlashlightColor[2], 4.5f };
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_COLOR, vPsConst, 1);

			pShader->BindTexture(SHADER_SAMPLER6, state.m_pSpotlightTexture, state.m_nSpotlightTextureFrame);

			atten[0] = state.m_fConstantAtten;		// Set the flashlight attenuation factors
			atten[1] = state.m_fLinearAtten;
			atten[2] = state.m_fQuadraticAtten;
			atten[3] = state.m_FarZ;
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_ATTENUATION, atten, 1);

			pos[0] = state.m_vecLightOrigin[0];		// Set the flashlight origin
			pos[1] = state.m_vecLightOrigin[1];
			pos[2] = state.m_vecLightOrigin[2];
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1);

			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4);

			// Tweaks associated with a given flashlight
			tweaks[0] = state.m_flShadowMapResolution;
			tweaks[1] = ShadowAttenFromState(state);
			pShader->HashShadow2DJitter(state.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
			pShaderAPI->SetPixelShaderConstant(PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1);
		}
		else // no flashlight
		{
			pShaderAPI->GetDX9LightState( &lightState );
		}

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int fogIndex = ( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ? 1 : 0;
		int numBones = pShaderAPI->GetCurrentNumBones();

		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;
		if( bFullyOpaque ) 
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
			AssertMsg( !(bWriteDepthToAlpha && bWriteWaterFogToAlpha), "Can't write two values to alpha at the same time." );
		}

		//ITexture *pCascadedDepthTexture = (ITexture *)pShaderAPI->GetIntRenderingParameter( INT_RENDERPARM_CASCADED_DEPTHTEXTURE );
		//bool bUseCSM = false; // pCascadedDepthTexture != NULL;

		const bool bFastVertexTextures = g_pHardwareConfig->HasFastVertexTextures();

		if (bFastVertexTextures)
			pShader->SetHWMorphVertexShaderState(VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, SHADER_VERTEXTEXTURE_SAMPLER0);

		DECLARE_DYNAMIC_VERTEX_SHADER_NEW( vertexlitPBR_vs30 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO_NEW( DOWATERFOG, fogIndex );
		SET_DYNAMIC_VERTEX_SHADER_COMBO_NEW( SKINNING, numBones > 0 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO_NEW( LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING)!=0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO_NEW( COMPRESSED_VERTS, (int)vertexCompression );
		//SET_DYNAMIC_VERTEX_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
		SET_DYNAMIC_VERTEX_SHADER_COMBO_NEW(DYNAMIC_LIGHT, lightState.HasDynamicLight());
		SET_DYNAMIC_VERTEX_SHADER_COMBO_NEW(STATIC_LIGHT, lightState.m_bStaticLight ? 1 : 0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO_NEW(MORPHING, pShaderAPI->IsHWMorphingEnabled());
		SET_DYNAMIC_VERTEX_SHADER_NEW(vertexlitPBR_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER_NEW(VertexlitPBR_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW( NUM_LIGHTS, lightState.m_nNumLights );
		SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
		SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
		SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
		SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW( FLASHLIGHTSHADOWS, bFlashlightShadows );
		SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW( LIGHTMAP, bHasLightmap && !bHasFlashlight);
		SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW(LIGHT_PREVIEW,
			pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING));
		//SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW( CSM, bUseCSM && !bHasFlashlight );
		//SET_DYNAMIC_PIXEL_SHADER_COMBO_NEW(CSM_PERF, 0);// MAX(0, MIN(r_csm_performance.GetInt(), 2)) ); // i just dont know anymore
		SET_DYNAMIC_PIXEL_SHADER_NEW(VertexlitPBR_ps30);

		if (bFastVertexTextures)
		{
			bool bUnusedTexCoords[3] = { false, false, !pShaderAPI->IsHWMorphingEnabled() || !bIsDecal };
			pShaderAPI->MarkUnusedVertexFields(0, 3, bUnusedTexCoords);
		}

		pShader->SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform );
		pShader->SetModulationPixelShaderDynamicState_LinearColorSpace( 1 );
		pShader->SetAmbientCubeDynamicStateVertexShader();

		// handle mat_fullbright 2 (diffuse lighting only)
		if( bLightingOnly )
		{
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY );
		}

		pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );

		if (!bHasFlashlight)
		{
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER5, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED);
			pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);
			pShaderAPI->SetPixelShaderStateAmbientLightCube(PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight);	// Force to black if not bAmbientLight
		}

		float vEyePos_SpecExponent[4];
		pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);
		vEyePos_SpecExponent[3] = 0.0f;
		pShaderAPI->SetPixelShaderConstant(PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1);

		if( !bHasFlashlight )
		{
#if 0
			if (bUseCSM)
			{
				pShader->BindTexture(SHADER_SAMPLER4, pCascadedDepthTexture);

				VMatrix* worldToTexture0 = (VMatrix*)pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_CASCADED_MATRIX_ADDRESS_0);
				pShaderAPI->SetPixelShaderConstant(32, worldToTexture0->Base(), 4);

				lightData_Global_t csmData = GetDeferredExt()->GetLightData_Global();
				Vector csmFwd = csmData.vecLight;
				pShaderAPI->SetPixelShaderConstant(36, csmFwd.Base());

				Vector csmLight = csmData.light.AsVector3D();
				pShaderAPI->SetPixelShaderConstant(37, csmLight.Base());

				Vector csmAmbient = csmData.ambient.AsVector3D();
				pShaderAPI->SetPixelShaderConstant(38, csmAmbient.Base());

				float biasVar[2] = { r_csm_slopescalebias.GetFloat(), r_csm_bias.GetFloat() };
				pShaderAPI->SetPixelShaderConstant(39, biasVar);

				float textureSize[2] = { pCascadedDepthTexture->GetActualWidth(), pCascadedDepthTexture->GetActualHeight() };
				pShaderAPI->SetPixelShaderConstant(40, textureSize);

				pShaderAPI->SetPixelShaderConstant(41, GetDeferredExt()->GetLightData_Global().sizes.Base());
			}
#endif // 0
		}
	}
	pShader->Draw();
}


//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
void DrawVertexLitPBR_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bHasFlashlight,
				   VertexLitPBR_DX9_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr )

{
	DrawVertexLitPBR_DX9_Internal( pShader, params, pShaderAPI,	pShaderShadow, bHasFlashlight, info, vertexCompression, pContextDataPtr );
}
