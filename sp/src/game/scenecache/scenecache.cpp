
#include "dbg.h"
#include "tier1.h"
#include "ISceneFileCache.h"
#include "INewSceneCache.h"
#include "SceneImageFile.h"
#include "datacache/idatacache.h"
#include "filesystem.h"
#include "utlrbtree.h"
#include "utlbuffer.h"
#include "UtlSortVector.h"
#include "tier1/strtools.h"
#include "../game/shared/choreoscene.h"
#include "utlsymbol.h"
#include "checksum_crc.h"
#include "utlhash.h"
#include "lzmaDecoder.h"
#include "mapbase_con_groups.h"
#include "tier3/scenetokenprocessor.h"

#include "valve_minmax_off.h"
#include <unordered_map>
#include "valve_minmax_on.h"

#include "tier0/memdbgon.h"

#define SCENE_IMAGE_PATHID "GAME"
#define SCENE_FILE_PATHID "SCENES"

//-----------------------------------------------------------------------------
// Binary compiled VCDs get their strings from a pool
//-----------------------------------------------------------------------------
class CChoreoStringPool : public IChoreoStringPool
{
public:
	CChoreoStringPool(SceneImageHeader_t* pHeader) : m_pHeader(pHeader)
	{}

	short FindOrAddString(const char* pString)
	{
		// huh?, no compilation at run time, only fetches
		Assert(0);
		return -1;
	}

	bool GetString(short stringId, char* buff, int buffSize)
	{
		// fetch from compiled pool
		const char* pString = m_pHeader->String(stringId);
		if (!pString)
		{
			V_strncpy(buff, "", buffSize);
			return false;
		}
		V_strncpy(buff, pString, buffSize);
		return true;
	}

protected:
	SceneImageHeader_t* m_pHeader;
};

IFileSystem* filesystem = NULL;
IDataCache* datacache = NULL;

inline CRC32_t GetPathCRC(const char* pszScenePath)
{
	char szCleanName[MAX_PATH];
	V_strncpy(szCleanName, pszScenePath, sizeof(szCleanName));
	V_strlower(szCleanName);
	V_FixSlashes(szCleanName, '\\');

	return CRC32_ProcessSingleBuffer(szCleanName, strlen(szCleanName));
}

CUtlFilenameSymbolTable g_ScenePathTable;
CUtlSymbolTable g_SceneSoundTable(0, 16, true);

class CSoundStringPool : public IChoreoStringPool
{
public:
	CSoundStringPool()
	{}

	short FindOrAddString(const char* pString)
	{
		return g_SceneSoundTable.AddString(pString);
	}

	bool GetString(short stringId, char* buff, int buffSize)
	{
		// fetch from compiled pool
		const char* pString = g_SceneSoundTable.String(stringId);
		if (!pString)
		{
			V_strncpy(buff, "", buffSize);
			return false;
		}
		V_strncpy(buff, pString, buffSize);
		return true;
	}
} g_SoundTableStringPool;

struct SceneCacheParams_t
{
	const char* pszFileName;
	unsigned int nFileSize;
};

struct SceneFileResource_t
{
	FileNameHandle_t m_hFileName;
	unsigned int m_nFileSize;
	byte* m_pFileData;
	unsigned int	msecs;
	CUtlVector< UtlSymId_t > soundList;

	FSAsyncControl_t	m_hAsyncControl;

	bool				m_bLoadPending : 1;
	bool				m_bLoadCompleted : 1;

	SceneFileResource_t() :
		m_hFileName(0),
		m_nFileSize(0),
		m_pFileData(NULL),
		m_bLoadPending(false),
		m_bLoadCompleted(false),
		m_hAsyncControl(NULL)
	{}

	SceneFileResource_t* GetData() { return this; }
	unsigned int	Size()
	{
		return sizeof(*this) + m_nFileSize + (sizeof(UtlSymId_t) * soundList.Count());
	}

	static void AsyncLoaderCallback(const FileAsyncRequest_t& request, int numReadBytes, FSAsyncStatus_t asyncStatus)
	{
		// get our preserved data
		SceneFileResource_t* pData = (SceneFileResource_t*)request.pContext;
		Assert(pData);

		SetTokenProcessorBuffer((char *)pData->m_pFileData);
		CChoreoScene* pScene = ChoreoLoadScene(request.pszFilename, nullptr, GetTokenProcessor(), nullptr);
		SetTokenProcessorBuffer(NULL);

		// Walk all events looking for SPEAK events
		int c = pScene->GetNumEvents();
		for (int i = 0; i < c; ++i)
		{
			CChoreoEvent* pEvent = pScene->GetEvent(i);
			if (pEvent->GetType() == CChoreoEvent::SPEAK)
			{
				unsigned short stringId = g_SceneSoundTable.AddString(pEvent->GetParameters());
				if (pData->soundList.Find(stringId) == pData->soundList.InvalidIndex())
				{
					pData->soundList.AddToTail(stringId);
				}

				if (pEvent->GetCloseCaptionType() == CChoreoEvent::CC_MASTER)
				{
					char tok[CChoreoEvent::MAX_CCTOKEN_STRING];
					if (pEvent->GetPlaybackCloseCaptionToken(tok, sizeof(tok)))
					{
						stringId = g_SceneSoundTable.AddString(tok);
						if (pData->soundList.Find(stringId) == pData->soundList.InvalidIndex())
						{
							pData->soundList.AddToTail(stringId);
						}
					}
				}
			}
		}

		// Update scene duration, too
		pData->msecs = (int)(pScene->FindStopTime() * 1000.0f + 0.5f);

		delete pScene;

		// mark as completed in single atomic operation
		pData->m_bLoadCompleted = true;
	}

	void AsyncLoad()
	{
		// Already pending
		Assert(!m_hAsyncControl);

		char fileName[MAX_PATH];
		g_ScenePathTable.String(m_hFileName, fileName, MAX_PATH);

		// async load the file	
		FileAsyncRequest_t fileRequest;
		fileRequest.pContext = (void*)this;
		fileRequest.pfnCallback = AsyncLoaderCallback;
		fileRequest.pData = m_pFileData;;
		fileRequest.pszFilename = fileName;
		fileRequest.nOffset = 0;
		fileRequest.flags = FSASYNC_FLAGS_NULLTERMINATE;
		fileRequest.nBytes = m_nFileSize;
		fileRequest.priority = -1;

		// queue for async load
		MEM_ALLOC_CREDIT();
		filesystem->AsyncRead(fileRequest, &m_hAsyncControl);
		m_bLoadPending = true;
	}

	void AsyncFinish()
	{
		if (m_bLoadPending && !m_bLoadCompleted)
		{
			filesystem->AsyncFinish(m_hAsyncControl);
		}
	}

	// you must implement these static functions for the ResourceManager
	// -----------------------------------------------------------
	static SceneFileResource_t* CreateResource(const SceneCacheParams_t& params)
	{
		SceneFileResource_t* data = new SceneFileResource_t;
		data->m_nFileSize = params.nFileSize + 1;
		data->m_hFileName = g_ScenePathTable.FindOrAddFileName(params.pszFileName);
		data->m_pFileData = new byte[data->m_nFileSize];
		return data;
	}

	static unsigned int EstimatedSize(const SceneCacheParams_t& params)
	{
		// The block size is assumed to be 4K
		return (sizeof(SceneFileResource_t) + params.nFileSize + 1);
	}

	void DestroyResource()
	{
		AsyncFinish();

		delete[] m_pFileData;
		delete this;
	}

	bool GetFileName(char* pszBuf, size_t nBufSize)
	{
		return g_ScenePathTable.String(m_hFileName, pszBuf, nBufSize);
	}
};

class CSceneImageResource : public IChoreoStringPool
{
public:
	CSceneImageResource() :
		m_hFileName(0),
		m_nFileSize(0),
		m_pFileData(NULL),
		m_bLoadPending(false),
		m_bLoadCompleted(false),
		m_hAsyncControl(NULL)
	{}

	CSceneImageResource* GetData() { return this; }
	unsigned int	Size()
	{
		return sizeof(*this) + m_nFileSize;
	}

	// IChoreoStringPool
	short FindOrAddString(const char* pString)
	{
		// huh?, no compilation at run time, only fetches
		Assert(0);
		return -1;
	}

	bool GetString(short stringId, char* buff, int buffSize)
	{
		if (!m_bLoadCompleted)
		{
			V_strncpy(buff, "", buffSize);
			return false;
		}

		// fetch from compiled pool
		const char* pString = GetImage()->String(stringId);
		if (!pString)
		{
			V_strncpy(buff, "", buffSize);
			return false;
		}
		V_strncpy(buff, pString, buffSize);
		return true;
	}

	static void AsyncLoaderCallback(const FileAsyncRequest_t& request, int numReadBytes, FSAsyncStatus_t asyncStatus)
	{
		// get our preserved data
		CSceneImageResource* pData = (CSceneImageResource*)request.pContext;

		Assert(pData);

		// mark as completed in single atomic operation
		pData->m_bLoadCompleted = true;
	}

	void AsyncLoad()
	{
		// Already pending
		Assert(!m_hAsyncControl);

		char fileName[MAX_PATH];
		g_ScenePathTable.String(m_hFileName, fileName, MAX_PATH);

		// async load the file	
		FileAsyncRequest_t fileRequest;
		fileRequest.pContext = (void*)this;
		fileRequest.pfnCallback = AsyncLoaderCallback;
		fileRequest.pData = m_pFileData;;
		fileRequest.pszFilename = fileName;
		fileRequest.nOffset = 0;
		fileRequest.flags = 0;
		fileRequest.nBytes = m_nFileSize;
		fileRequest.priority = -1;

		// queue for async load
		MEM_ALLOC_CREDIT();
		filesystem->AsyncRead(fileRequest, &m_hAsyncControl);
		m_bLoadPending = true;
	}

	void AsyncFinish()
	{
		if (m_bLoadPending && !m_bLoadCompleted)
		{
			filesystem->AsyncFinish(m_hAsyncControl);
		}
	}

	bool AsyncReady() { return m_bLoadCompleted; }

	// you must implement these static functions for the ResourceManager
	// -----------------------------------------------------------
	static CSceneImageResource* CreateResource(const SceneCacheParams_t& params)
	{
		CSceneImageResource* data = new CSceneImageResource;
		data->m_nFileSize = params.nFileSize;
		data->m_hFileName = g_ScenePathTable.FindOrAddFileName(params.pszFileName);
		data->m_pFileData = new byte[data->m_nFileSize];
		return data;
	}

	static unsigned int EstimatedSize(const SceneCacheParams_t& params)
	{
		// The block size is assumed to be 4K
		return (sizeof(CSceneImageResource) + params.nFileSize);
	}

	void DestroyResource()
	{
		AsyncFinish();

		delete[] m_pFileData;
		delete this;
	}

	bool GetFileName(char* pszBuf, size_t nBufSize)
	{
		return g_ScenePathTable.String(m_hFileName, pszBuf, nBufSize);
	}

	int FindSceneInImage(CRC32_t crcFilename);
	bool GetSceneDataFromImage(int iScene, CUtlBuffer &buf);
	bool GetSceneDataFromImage(int iScene, byte* pData, size_t* pLength);
	bool GetSceneCachedData(int iScene, SceneCachedData_t* pData);
	short GetSceneCachedSound(int iScene, int iSound);

private:
	SceneImageHeader_t* GetImage() { return (SceneImageHeader_t*)m_pFileData; }

	FileNameHandle_t m_hFileName;
	unsigned int m_nFileSize;
	byte* m_pFileData;

	FSAsyncControl_t	m_hAsyncControl;

	bool				m_bLoadPending : 1;
	bool				m_bLoadCompleted : 1;
};

//-----------------------------------------------------------------------------
//  Returns -1 if not found, otherwise [0..n] index.
//-----------------------------------------------------------------------------
int CSceneImageResource::FindSceneInImage(CRC32_t crcFilename)
{
	if (!m_bLoadCompleted)
	{
		return -1;
	}
	SceneImageEntry_t* pEntries = (SceneImageEntry_t*)((byte*)GetImage() + GetImage()->nSceneEntryOffset);

	// use binary search, entries are sorted by ascending crc
	int nLowerIdx = 1;
	int nUpperIdx = GetImage()->nNumScenes;
	for (;; )
	{
		if (nUpperIdx < nLowerIdx)
		{
			return -1;
		}
		else
		{
			int nMiddleIndex = (nLowerIdx + nUpperIdx) / 2;
			CRC32_t nProbe = pEntries[nMiddleIndex - 1].crcFilename;
			if (crcFilename < nProbe)
			{
				nUpperIdx = nMiddleIndex - 1;
			}
			else
			{
				if (crcFilename > nProbe)
				{
					nLowerIdx = nMiddleIndex + 1;
				}
				else
				{
					return nMiddleIndex - 1;
				}
			}
		}
	}

	return -1;
}

bool CSceneImageResource::GetSceneDataFromImage(int iScene, CUtlBuffer& buf)
{
	buf.Clear();
	buf.SetBufferType(false, false);

	if (!m_bLoadCompleted || iScene < 0 || iScene >= GetImage()->nNumScenes)
	{
		return false;
	}

	SceneImageEntry_t* pEntries = (SceneImageEntry_t*)((byte*)GetImage() + GetImage()->nSceneEntryOffset);
	unsigned char* pData = (unsigned char*)GetImage() + pEntries[iScene].nDataOffset;
	CLZMA lzma;
	bool bIsCompressed;
	bIsCompressed = lzma.IsCompressed(pData);
	if (bIsCompressed)
	{
		int originalSize = lzma.GetActualSize(pData);

		//if (pSceneData)
		{
			if (!buf.IsExternallyAllocated() || buf.IsGrowable())
			{
				lzma.Uncompress(pData, (byte*)buf.AccessForDirectRead(originalSize));
			}
			else
			{
				unsigned char* pOutputData = (unsigned char*)malloc(originalSize);
				lzma.Uncompress(pData, pOutputData);
				buf.Put(pOutputData, originalSize);
				free(pOutputData);
			}
		}
	}
	else
	{
		buf.Put(pData, pEntries[iScene].nDataLength);
	}
	return true;
}

bool CSceneImageResource::GetSceneDataFromImage(int iScene, byte* pSceneData, size_t* pSceneLength)
{
	if (!m_bLoadCompleted || iScene < 0 || iScene >= GetImage()->nNumScenes)
	{
		return false;
	}

	SceneImageEntry_t* pEntries = (SceneImageEntry_t*)((byte*)GetImage() + GetImage()->nSceneEntryOffset);
	unsigned char* pData = (unsigned char*)GetImage() + pEntries[iScene].nDataOffset;
	CLZMA lzma;
	bool bIsCompressed;
	bIsCompressed = lzma.IsCompressed(pData);
	if (bIsCompressed)
	{
		int originalSize = lzma.GetActualSize(pData);

		if (pSceneData)
		{
			int nMaxLen = *pSceneLength;
			if (originalSize <= nMaxLen)
			{
				lzma.Uncompress(pData, pSceneData);
			}
			else
			{
				unsigned char* pOutputData = (unsigned char*)malloc(originalSize);
				lzma.Uncompress(pData, pOutputData);
				V_memcpy(pSceneData, pOutputData, nMaxLen);
				free(pOutputData);
			}
		}
		if (pSceneLength)
		{
			*pSceneLength = originalSize;
		}
	}
	else
	{
		if (pSceneData)
		{
			size_t nCountToCopy = min(*pSceneLength, (size_t)pEntries[iScene].nDataLength);
			V_memcpy(pSceneData, pData, nCountToCopy);
		}
		if (pSceneLength)
		{
			*pSceneLength = (size_t)pEntries[iScene].nDataLength;
		}
	}
	return true;
}

bool CSceneImageResource::GetSceneCachedData(int iScene, SceneCachedData_t* pData)
{
	if (!m_bLoadCompleted || iScene < 0 || iScene >= GetImage()->nNumScenes)
	{
		return false;
	}

	if (pData)
	{
		SceneImageEntry_t* pEntries = (SceneImageEntry_t*)((byte*)GetImage() + GetImage()->nSceneEntryOffset);
		void* pUnknown = (unsigned char*)GetImage() + pEntries[iScene].nSceneSummaryOffset;
		switch (GetImage()->nVersion)
		{
		case 2:
		{
			const SceneImageSummary_t* pSummary = (SceneImageSummary_t*)pUnknown;
			pData->msecs = pSummary->msecs;
			pData->numSounds = pSummary->numSounds;
		}
		break;
		case 3:
		{
			const SceneImageSummaryV3_t* pSummary = (SceneImageSummaryV3_t*)pUnknown;
			pData->msecs = pSummary->msecs;
			pData->numSounds = pSummary->numSounds;
		}
		break;
		default:
			return false;
			break;
		}
	}

	return true;
}

short CSceneImageResource::GetSceneCachedSound(int iScene, int iSound)
{
	if (!m_bLoadCompleted || iScene < 0 || iSound < 0 || iScene >= GetImage()->nNumScenes)
	{
		return -1;
	}

	SceneImageEntry_t* pEntries = (SceneImageEntry_t*)((byte*)GetImage() + GetImage()->nSceneEntryOffset);
	void* pUnknown = (unsigned char*)GetImage() + pEntries[iScene].nSceneSummaryOffset;
	switch (GetImage()->nVersion)
	{
	case 2:
	{
		const SceneImageSummary_t* pSummary = (SceneImageSummary_t*)pUnknown;
		if (iSound >= pSummary->numSounds)
			return -1;

		return pSummary->soundStrings[iSound];
	}
	break;
	case 3:
	{
		const SceneImageSummaryV3_t* pSummary = (SceneImageSummaryV3_t*)pUnknown;
		if (iSound >= pSummary->numSounds)
			return -1;

		return pSummary->soundStrings[iSound];
	}
	break;
	}

	return -1;
}

class CSceneImageCache : public CManagedDataCacheClient<CSceneImageResource, SceneCacheParams_t>
{
public:
	void OutputReport()
	{
		GetCacheSection()->OutputReport();
	}

	bool GetItemName(DataCacheClientID_t clientId, const void* pItem, char* pDest, unsigned nMaxLen)
	{
		char filePath[MAX_PATH];
		CSceneImageResource* p = (CSceneImageResource*)pItem;
		if (p->GetFileName(filePath, MAX_PATH))
		{
			V_StripFilename(filePath); // Strip scenes.image
			V_StripLastDir(filePath, MAX_PATH); // Strip scenes/
			V_StripTrailingSlash(filePath);
			V_strncpy(pDest, V_UnqualifiedFileName(filePath), nMaxLen);
			return true;
		}

		return false;
	}

	bool CacheIsPresent(DataCacheHandle_t hCache)
	{
		return GetCacheSection()->IsPresent(hCache);
	}
} g_SceneImageCache;

class CSceneFileCache : public CManagedDataCacheClient<SceneFileResource_t, SceneCacheParams_t>
{
public:
	void OutputReport()
	{
		GetCacheSection()->OutputReport();
	}

	bool GetItemName(DataCacheClientID_t clientId, const void* pItem, char* pDest, unsigned nMaxLen)
	{
		char fullFilePath[MAX_PATH];
		SceneFileResource_t* p = (SceneFileResource_t*)pItem;
		if (p->GetFileName(fullFilePath, MAX_PATH) && filesystem->FullPathToRelativePath(fullFilePath, pDest, nMaxLen))
		{
			return true;
		}

		return false;
	}

	bool CacheIsPresent(DataCacheHandle_t hCache)
	{
		return GetCacheSection()->IsPresent(hCache);
	}
} g_SceneFileCache;

struct SceneFileHash_t
{
	// Key
	CRC32_t	crcScene;

	// Value
	FileNameHandle_t hContainingFileName;
};

struct SceneFileEntry_t
{
	// Key
	CRC32_t	crcScene;

	// Value
	FileNameHandle_t hContainingFileName;
	DataCacheHandle_t hCacheHandle;
	bool bFromImage;
};

namespace SceneHash
{
	static bool Compare(SceneFileHash_t const& a, SceneFileHash_t const& b)
	{
		return a.crcScene == b.crcScene;
	}

	static unsigned int Key(SceneFileHash_t const& a)
	{
		return a.crcScene;
	}

	static bool Compare(SceneFileEntry_t const& a, SceneFileEntry_t const& b)
	{
		return a.crcScene == b.crcScene;
	}

	static unsigned int Key(SceneFileEntry_t const& a)
	{
		return a.crcScene;
	}
}

class CSceneCache : public CTier1AppSystem<INewSceneCache>
{
public:
	CSceneCache();

	virtual bool Connect(CreateInterfaceFn factory);
	virtual void Disconnect();

	virtual InitReturnVal_t Init();
	virtual void Shutdown();

	// INewSceneCache
	virtual void OutputStatus();
	virtual void Reload();

	// async implemenation
	virtual bool StartAsyncLoading(char const* filename);
	virtual void PollForAsyncLoading(ISceneFileCacheCallback* entity, char const* pszScene);
	virtual bool IsStillAsyncLoading(char const* filename);

	// persisted scene data, returns true if valid, false otherwise
	virtual bool		GetSceneCachedData(char const* pFilename, SceneCachedData_t* pData);
	virtual short		GetSceneCachedSound(int iScene, int iSound);

	// sync
	virtual size_t		GetSceneBufferSize(char const* filename);
	virtual bool		GetSceneData(char const* filename, byte* buf, size_t bufsize);

	// Blocking
	virtual IChoreoStringPool* GetSceneStringPool(const char* filename);
	virtual IChoreoStringPool* GetSceneStringPool(int iScene);

	bool GetSceneFilePath(const char* pszScene, char* pszPath, int nMaxLen);
private:
	void ScanSceneImages();
	bool FindScene(const char* pszScene, SceneFileEntry_t& entry);
	void LoadSceneImage(SceneFileEntry_t& entry);
	void LoadVCDScene(SceneFileEntry_t& entry);

	uint8 m_nConnectCount;
	uint8 m_nInitCount;

	CUtlHash<SceneFileEntry_t> m_Scenes;
	CUtlHash<SceneFileHash_t> m_SceneToImage;
	std::unordered_map<FileNameHandle_t, DataCacheHandle_t> m_ImageHandles;

	typedef CTier1AppSystem<INewSceneCache> BaseClass;
};

CSceneCache::CSceneCache() : BaseClass(true),
	m_Scenes(16, 0, 0, SceneHash::Compare, SceneHash::Key),
	m_SceneToImage(16, 0, 0, SceneHash::Compare, SceneHash::Key)
{
	m_nConnectCount = 0;
	m_nInitCount = 0;
}

void CSceneCache::OutputStatus()
{
	g_SceneImageCache.OutputReport();
	g_SceneFileCache.OutputReport();
}

void CSceneCache::Reload()
{
	m_Scenes.RemoveAll();
	m_ImageHandles.clear();
	g_SceneImageCache.CacheFlush();
	g_SceneFileCache.CacheFlush();
	g_SceneSoundTable.RemoveAll();
	ScanSceneImages();
}

bool CSceneCache::Connect(CreateInterfaceFn factory)
{
	if (m_nConnectCount == 0)
	{
		if (!BaseClass::Connect(factory))
			return false;

		if ((filesystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL)) == NULL)
			return false;

		if ((datacache = (IDataCache*)factory(DATACACHE_INTERFACE_VERSION, NULL)) == NULL)
			return false;
	}

	m_nConnectCount++;
	return true;
}

void CSceneCache::Disconnect()
{
	m_nConnectCount--;
	if (m_nConnectCount != 0)
		return;

	BaseClass::Disconnect();
}

InitReturnVal_t CSceneCache::Init()
{
	if (m_nInitCount == 0)
	{
		InitReturnVal_t nRetVal = BaseClass::Init();
		if (nRetVal != INIT_OK)
			return nRetVal;

		filesystem->MarkPathIDByRequestOnly(SCENE_FILE_PATHID, true);
		InitConsoleGroups(filesystem);

		const DataCacheLimits_t imageLimits(1024 * 1024 * 6, 16);
		g_SceneImageCache.Init(datacache, "SceneImageCache", imageLimits);

		const DataCacheLimits_t vcdLimits(1024 * 1024 * 2);
		g_SceneFileCache.Init(datacache, "SceneFileCache", vcdLimits);

		ScanSceneImages();
	}

	m_nInitCount++;
	return INIT_OK;
}

void CSceneCache::Shutdown()
{
	m_nInitCount--;
	if (m_nInitCount == 0)
	{
		m_Scenes.Purge();
		m_ImageHandles.clear();
		m_SceneToImage.Purge();
		g_SceneImageCache.Shutdown();
		g_SceneFileCache.Shutdown();
		g_SceneSoundTable.RemoveAll();
		BaseClass::Shutdown();
	}
}

void CSceneCache::ScanSceneImages()
{
	m_SceneToImage.RemoveAll();

	int iBufferSize = filesystem->GetSearchPath("GAME", true, nullptr, 0);
	char* pszSearchPaths = (char*)stackalloc(iBufferSize);
	filesystem->GetSearchPath(SCENE_IMAGE_PATHID, true, pszSearchPaths, iBufferSize);
	const char* pSceneImageName = IsX360() ? "scenes/scenes.360.image" : "scenes/scenes.image";
	char* strtokContext;
	for (char* path = V_strtok_s(pszSearchPaths, ";", &strtokContext); path; path = V_strtok_s(NULL, ";", &strtokContext))
	{
		char imagePath[MAX_PATH];
		V_ComposeFileName(path, pSceneImageName, imagePath, MAX_PATH);
		if (filesystem->FileExists(imagePath))
		{
			CUtlBuffer bufImageFile;
			if (filesystem->ReadFile(imagePath, "GAME", bufImageFile))
			{
				SceneImageHeader_t* pHeader = (SceneImageHeader_t*)bufImageFile.Base();
				if (pHeader->nId != SCENE_IMAGE_ID ||
					pHeader->nVersion < SCENE_IMAGE_MIN_VERSION ||
					pHeader->nVersion > SCENE_IMAGE_VERSION)
				{
					continue;
				}

				FileNameHandle_t hFileName = g_ScenePathTable.FindOrAddFileName(imagePath);
				SceneImageEntry_t* pEntries = (SceneImageEntry_t*)((byte*)pHeader + pHeader->nSceneEntryOffset);
				CChoreoStringPool imagePool(pHeader);

				for (int i = 0; i < pHeader->nNumScenes; i++)
				{
					SceneFileHash_t key;
					key.crcScene = pEntries[i].crcFilename;
					key.hContainingFileName = hFileName;
					auto handle = m_SceneToImage.Find(key);
					if (!m_SceneToImage.IsValidHandle(handle))
					{
						// Add it now
						m_SceneToImage.Insert(key);
					}
				}
			}
		}
	}

	CGMsg(0, CON_GROUP_SCENE_CACHE, "Discovered %i scenes from scenes.image\n", m_SceneToImage.Count());
}

bool CSceneCache::FindScene(const char* pszScene, SceneFileEntry_t& entry)
{
	entry.hCacheHandle = DC_INVALID_HANDLE;
	entry.crcScene = GetPathCRC(pszScene);
	if (filesystem->FileExists(pszScene, SCENE_FILE_PATHID))
	{
		char fullPath[MAX_PATH];
		if (filesystem->RelativePathToFullPath(pszScene, SCENE_FILE_PATHID, fullPath, sizeof(fullPath)) == NULL)
			return false;

		entry.bFromImage = false;
		entry.hContainingFileName = g_ScenePathTable.FindOrAddFileName(fullPath);
		return true;
	}

	SceneFileHash_t hashEntry;
	hashEntry.crcScene = entry.crcScene;
	auto handle = m_SceneToImage.Find(hashEntry);
	if (m_SceneToImage.IsValidHandle(handle))
	{
		hashEntry = m_SceneToImage.Element(handle);

		entry.bFromImage = true;
		entry.hContainingFileName = hashEntry.hContainingFileName;
		if (m_ImageHandles.count(entry.hContainingFileName))
			entry.hCacheHandle = m_ImageHandles.at(entry.hContainingFileName);
		
		return true;
	}

	return false;
}

void CSceneCache::LoadSceneImage(SceneFileEntry_t& entry)
{
	if (!entry.bFromImage)
		return;

	if (entry.hCacheHandle != DC_INVALID_HANDLE && g_SceneImageCache.CacheIsPresent(entry.hCacheHandle))
		return;

	char imagePath[MAX_PATH];
	g_ScenePathTable.String(entry.hContainingFileName, imagePath, MAX_PATH);

	SceneCacheParams_t params;
	params.pszFileName = imagePath;
	params.nFileSize = filesystem->Size(imagePath);
	m_ImageHandles[entry.hContainingFileName] = entry.hCacheHandle = g_SceneImageCache.CacheCreate(params);
}

void CSceneCache::LoadVCDScene(SceneFileEntry_t& entry)
{
	if (entry.bFromImage)
		return;

	if (entry.hCacheHandle != DC_INVALID_HANDLE && g_SceneFileCache.CacheIsPresent(entry.hCacheHandle))
		return;

	char imagePath[MAX_PATH];
	g_ScenePathTable.String(entry.hContainingFileName, imagePath, MAX_PATH);

	SceneCacheParams_t params;
	params.pszFileName = imagePath;
	params.nFileSize = filesystem->Size(imagePath);
	entry.hCacheHandle = g_SceneFileCache.CacheCreate(params);
}

bool CSceneCache::StartAsyncLoading(char const* filename)
{
	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(filename);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(filename, key))
			return false;

		hScene = m_Scenes.Insert(key);
	}

	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		g_SceneImageCache.CacheUnlock(entry.hCacheHandle);
	}
	else
	{
		auto pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadVCDScene(entry);
			pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		g_SceneFileCache.CacheUnlock(entry.hCacheHandle);
	}

	return true;
}

void CSceneCache::PollForAsyncLoading(ISceneFileCacheCallback* entity, char const* pszScene)
{
	Assert(entity);

	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(pszScene);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(pszScene, key))
		{
			entity->FinishAsyncLoading(pszScene, NULL, 0, NULL);
			return;
		}

		hScene = m_Scenes.Insert(key);
	}

	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		if (pCache->AsyncReady())
		{
			int iScene = pCache->FindSceneInImage(entry.crcScene);
			CUtlBuffer buf;
			pCache->GetSceneDataFromImage(iScene, buf);
			entity->FinishAsyncLoading(pszScene, pCache, buf.TellMaxPut(), (const char *)buf.Base());
		}

		g_SceneImageCache.CacheUnlock(entry.hCacheHandle);
		return;
	}
	else
	{
		auto pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadVCDScene(entry);
			pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		if (pCache->m_bLoadCompleted)
		{
			entity->FinishAsyncLoading(pszScene, NULL, pCache->m_nFileSize, (const char *)pCache->m_pFileData);
		}

		g_SceneFileCache.CacheUnlock(entry.hCacheHandle);
		return;
	}
}

bool CSceneCache::IsStillAsyncLoading(char const* filename)
{
	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(filename);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(filename, key))
			return false;

		hScene = m_Scenes.Insert(key);
	}

	bool bRet = true;
	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		bRet = !pCache->AsyncReady();
		g_SceneImageCache.CacheUnlock(entry.hCacheHandle);
	}
	else
	{
		auto pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadVCDScene(entry);
			pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		bRet = !pCache->m_bLoadCompleted;
		g_SceneFileCache.CacheUnlock(entry.hCacheHandle);
	}

	return bRet;
}

bool CSceneCache::GetSceneCachedData(char const* pFilename, SceneCachedData_t* pData)
{
	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(pFilename);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(pFilename, key))
		{
			pData->sceneId = -1;
			pData->msecs = 0;
			pData->numSounds = 0;
			return false;
		}

		hScene = m_Scenes.Insert(key);
	}

	pData->sceneId = hScene;
	bool bRet = false;
	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		int iScene = pCache->FindSceneInImage(entry.crcScene);
		bRet = pCache->GetSceneCachedData(iScene, pData);
		g_SceneImageCache.CacheUnlock(entry.hCacheHandle);
	}
	else
	{
		auto pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadVCDScene(entry);
			pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		pData->msecs = pCache->msecs;
		pData->numSounds = pCache->soundList.Count();
		bRet = true;
		g_SceneFileCache.CacheUnlock(entry.hCacheHandle);
	}

	return bRet;
}

short CSceneCache::GetSceneCachedSound(int iScene, int iSound)
{
	UtlHashHandle_t hScene = iScene;
	if (!m_Scenes.IsValidHandle(hScene))
		return -1;

	short nRet = -1;
	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		int iImageScene = pCache->FindSceneInImage(entry.crcScene);
		nRet = pCache->GetSceneCachedSound(iImageScene, iSound);
		g_SceneImageCache.CacheUnlock(entry.hCacheHandle);
	}
	else
	{
		auto pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadVCDScene(entry);
			pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		if (iSound >= 0 && iSound < pCache->soundList.Count())
			nRet = pCache->soundList[iSound];

		g_SceneFileCache.CacheUnlock(entry.hCacheHandle);
	}

	return nRet;
}

size_t CSceneCache::GetSceneBufferSize(char const* pFilename)
{
	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(pFilename);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(pFilename, key))
			return 0;

		hScene = m_Scenes.Insert(key);
	}

	size_t returnSize = 0;
	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		pCache->GetSceneDataFromImage(pCache->FindSceneInImage(entry.crcScene), NULL, &returnSize);
		g_SceneImageCache.CacheUnlock(entry.hCacheHandle);
	}
	else
	{
		auto pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadVCDScene(entry);
			pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		returnSize = pCache->m_nFileSize;
		g_SceneFileCache.CacheUnlock(entry.hCacheHandle);
	}

	return returnSize;
}

bool CSceneCache::GetSceneData(char const* pFilename, byte* buf, size_t bufsize)
{
	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(pFilename);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(pFilename, key))
			return false;

		hScene = m_Scenes.Insert(key);
	}

	bool bRet = true;
	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();

		size_t nLength = bufsize;
		bRet = pCache->GetSceneDataFromImage(pCache->FindSceneInImage(entry.crcScene), buf, &nLength);
		g_SceneImageCache.CacheUnlock(entry.hCacheHandle);
	}
	else
	{
		auto pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
		if (!pCache)
		{
			LoadVCDScene(entry);
			pCache = g_SceneFileCache.CacheLock(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		V_memcpy(buf, pCache->m_pFileData, Min(bufsize, pCache->m_nFileSize));
		g_SceneFileCache.CacheUnlock(entry.hCacheHandle);
	}

	return bRet;
}

IChoreoStringPool* CSceneCache::GetSceneStringPool(const char* pFilename)
{
	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(pFilename);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(pFilename, key))
			return nullptr;

		hScene = m_Scenes.Insert(key);
	}

	IChoreoStringPool* pRet = NULL;
	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheGet(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheGet(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		pRet = pCache;
	}
	else
	{
		pRet = &g_SoundTableStringPool;
	}

	return pRet;
}

IChoreoStringPool* CSceneCache::GetSceneStringPool(int iScene)
{
	UtlHashHandle_t hScene = iScene;
	if (!m_Scenes.IsValidHandle(hScene))
		return nullptr;

	IChoreoStringPool* pRet = NULL;
	auto& entry = m_Scenes.Element(hScene);
	if (entry.bFromImage)
	{
		auto pCache = g_SceneImageCache.CacheGet(entry.hCacheHandle);
		if (!pCache)
		{
			LoadSceneImage(entry);
			pCache = g_SceneImageCache.CacheGet(entry.hCacheHandle);
			Assert(pCache);

			pCache->AsyncLoad();
		}

		pCache->AsyncFinish();
		pRet = pCache;
	}
	else
	{
		pRet = &g_SoundTableStringPool;
	}

	return pRet;
}

bool CSceneCache::GetSceneFilePath(const char* pszScene, char* pszPath, int nMaxLen)
{
	SceneFileEntry_t key;
	key.crcScene = GetPathCRC(pszScene);
	UtlHashHandle_t hScene = m_Scenes.Find(key);
	if (!m_Scenes.IsValidHandle(hScene))
	{
		if (!FindScene(pszScene, key))
			return false;

		hScene = m_Scenes.Insert(key);
	}

	auto& entry = m_Scenes.Element(hScene);
	return g_ScenePathTable.String(entry.hContainingFileName, pszPath, nMaxLen);
}

static CSceneCache g_SceneCacheInt;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CSceneCache, INewSceneCache, NEW_SCENE_CACHE_INTERFACE_VERSION, g_SceneCacheInt);

CON_COMMAND(scenefilecache_status, "Print current scene file cache status.")
{
	g_SceneCacheInt.OutputStatus();
}

CON_COMMAND(scenefilecache_find, "Print full path to file containing the specified scene.")
{
	if (args.ArgC() != 2)
	{
		Msg("scenefilecache_find <local/path/to/scene.vcd>\n");
		return;
	}

	char fullPath[MAX_PATH];
	if (!g_SceneCacheInt.GetSceneFilePath(args[1], fullPath, MAX_PATH))
	{
		Msg("Scene not found\n");
		return;
	}

	Msg("Scene found in: %s\n", fullPath);
}