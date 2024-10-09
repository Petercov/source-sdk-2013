#include "cbase.h"
#include "isaverestore.h"
#include "convar_saverestore.h"

static short CONVAR_SAVE_RESTORE_VERSION = 1;

//-----------------------------------------------------------------------------

class CConvarSaveRestoreBlockHandler : public CDefSaveRestoreBlockHandler
{
public:
	const char* GetBlockName()
	{
		return "ConVar";
	}

	//---------------------------------

	void Save(ISave* pSave)
	{
		if (!g_pCVar)
			return;

		pSave->StartBlock("ConVars");

		if (m_SaveConVars.IsEmpty())
			FindConVars();

		// Write # of saved convars
		short nSaveCount = m_SaveConVars.Count();
		pSave->WriteShort(&nSaveCount);
		// Write out each convar
		for (ConVarRef& var : m_SaveConVars)
		{
			pSave->WriteString(var.GetName());

			const bool bNoString = var.IsFlagSet(FCVAR_NEVER_AS_STRING);
			pSave->WriteBool(&bNoString);
			if (bNoString)
			{
				const float flValue = var.GetFloat();
				pSave->WriteFloat(&flValue);
			}
			else
			{
				const char* pszValue = var.GetString();
				const short nValueLen = V_strlen(pszValue);
				pSave->WriteShort(&nValueLen);
				pSave->WriteString(pszValue);
			}
		}
		pSave->EndBlock();
	}

	//---------------------------------

	void WriteSaveHeaders(ISave* pSave)
	{
		pSave->WriteShort(&CONVAR_SAVE_RESTORE_VERSION);
	}

	//---------------------------------

	void ReadRestoreHeaders(IRestore* pRestore)
	{
		// No reason why any future version shouldn't try to retain backward compatability. The default here is to not do so.
		short version;
		pRestore->ReadShort(&version);
		// only load if version matches and if we are loading a game, not a transition
		m_fDoLoad = ((version == CONVAR_SAVE_RESTORE_VERSION)
#ifndef CLIENT_DLL
			&& ((MapLoad_LoadGame == gpGlobals->eLoadType) || (MapLoad_NewGame == gpGlobals->eLoadType))
#endif // !CLIENT_DLL
			);
	}

	//---------------------------------

	void Restore(IRestore* pRestore, bool createPlayers)
	{
		if (!g_pCVar)
			return;

		if (m_fDoLoad)
		{
			pRestore->StartBlock();
			// read # of convars
			int nSavedConvars = pRestore->ReadShort();

			CUtlString strValue;
			while (nSavedConvars--)
			{
				char szName[64];
				pRestore->ReadString(szName, ARRAYSIZE(szName), 0);
				ConVarRef var(szName);

				bool bNoString;
				pRestore->ReadBool(&bNoString);
				if (bNoString)
				{
					float flValue;
					pRestore->ReadFloat(&flValue);
					var.SetValue(flValue);
				}
				else
				{
					short nValueLen = pRestore->ReadShort();
					strValue.SetLength(nValueLen);
					pRestore->ReadString(strValue.Get(), nValueLen, 0);
					var.SetValue(strValue);
				}
			}
			pRestore->EndBlock();
		}
	}

private:
	void FindConVars()
	{
		m_SaveConVars.RemoveAll();
		ICvar::Iterator iter(g_pCVar);
		for (iter.SetFirst(); iter.IsValid(); iter.Next())
		{
			ConCommandBase* pBase = iter.Get();
			if (pBase->IsCommand())
				continue;

			if (!pBase->IsFlagSet(FCVAR_GAME_SAVE))
				continue;
#ifdef CLIENT_DLL
			if (!pBase->IsFlagSet(FCVAR_CLIENTDLL) || pBase->IsFlagSet(FCVAR_REPLICATED))
				continue;
#else
			if (!pBase->IsFlagSet(FCVAR_GAMEDLL) && !pBase->IsFlagSet(FCVAR_REPLICATED))
				continue;
#endif // CLIENT_DLL

			ConVar* pVar = static_cast<ConVar*>(pBase);
			m_SaveConVars.AddToTail(pVar);
		}
	}

	bool m_fDoLoad;
	CUtlVector<ConVarRef> m_SaveConVars;
};

//-----------------------------------------------------------------------------

CConvarSaveRestoreBlockHandler g_ConvarSaveRestoreBlockHandler;
ISaveRestoreBlockHandler* GetConvarSaveRestoreBlockHandler()
{
	return &g_ConvarSaveRestoreBlockHandler;
}
