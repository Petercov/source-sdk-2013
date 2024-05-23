//====== Copyright © 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef IMATERIALPROXYDICT_H
#define IMATERIALPROXYDICT_H
#ifdef _WIN32
#pragma once
#endif

#ifdef MAPBASE_MATPROXYFACTORY
#include "interface.h"

class IMaterialProxy;

typedef IMaterialProxy* MaterialProxyFactory_t();

class IMaterialProxyDict
{
public:
	// This is used to instance a proxy.
	virtual IMaterialProxy* CreateProxy(const char* proxyName) = 0;
	// virtual destructor
	virtual	~IMaterialProxyDict() {}
	// Used by EXPOSE_MATERIAL_PROXY to insert all proxies into the dictionary.
	virtual void Add(const char* pMaterialProxyName, MaterialProxyFactory_t* pMaterialProxyFactory) = 0;
};

extern IMaterialProxyDict& GetMaterialProxyDict();

class MaterialProxyReg : public InterfaceReg
{
public:
	MaterialProxyReg(MaterialProxyFactory_t fn, const char* pName, const char* pIFaceName) : InterfaceReg((InstantiateInterfaceFn)fn, pIFaceName)
	{
		GetMaterialProxyDict().Add(pName, fn);
	}
};

#define EXPOSE_MATERIAL_PROXY( className, proxyName )						\
	static IMaterialProxy *__Create##className##Factory( void )				\
	{																		\
		return static_cast< IMaterialProxy * >( new className );			\
	};																		\
	static MaterialProxyReg __g_Create##className##_reg(__Create##className##Factory, proxyName, proxyName IMATERIAL_PROXY_INTERFACE_VERSION);  
#endif // MAPBASE_MATPROXYFACTORY

#endif // IMATERIALPROXYDICT_H
