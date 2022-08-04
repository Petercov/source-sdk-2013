#ifndef INEWSCENECACHE_H
#define INEWSCENECACHE_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"
#include "appframework/IAppSystem.h"

class IChoreoStringPool;

class ISceneFileCacheCallback
{
public:
	virtual void FinishAsyncLoading(char const* pszScene, IChoreoStringPool* pStringPool, size_t buflen, char const* buffer) = 0;
};

class INewSceneCache : public IAppSystem
{
public:
	virtual void OutputStatus() = 0;

	// async implemenation
	virtual bool StartAsyncLoading(char const* filename) = 0;
	virtual void PollForAsyncLoading(ISceneFileCacheCallback* entity, char const* pszScene) = 0;
	virtual bool IsStillAsyncLoading(char const* filename) = 0;

	// sync
	virtual size_t		GetSceneBufferSize(char const* filename) = 0;
	virtual bool		GetSceneData(char const* filename, byte* buf, size_t bufsize) = 0;
	virtual IChoreoStringPool* GetSceneStringPool(const char* filename) = 0;

	// Physically reloads image from disk
	virtual void		Reload() = 0;
};

#define NEW_SCENE_CACHE_INTERFACE_VERSION "SceneFileCache004"

#endif // INEWSCENECACHE_H
