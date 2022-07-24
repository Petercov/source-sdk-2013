//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef BONE_ACCESSOR_H
#define BONE_ACCESSOR_H
#ifdef _WIN32
#pragma once
#endif


#include "studio.h"


class C_BaseAnimating;


class CBoneAccessor
{
public:
#ifdef CLIENT_DLL
	typedef matrix3x4a_t bonematrix_t;
#else
	typedef matrix3x4_t bonematrix_t;
#endif
	
	CBoneAccessor();
	CBoneAccessor(bonematrix_t *pBones ); // This can be used to allow access to all bones.
	
	// Initialize.
#if defined( CLIENT_DLL )
	void Init( const C_BaseAnimating *pAnimating, bonematrix_t *pBones );
#endif
	
	int GetReadableBones();
	void SetReadableBones( int flags );

	int GetWritableBones();
	void SetWritableBones( int flags );

	// Get bones for read or write access.
	const bonematrix_t&	GetBone( int iBone ) const;
	const bonematrix_t&	operator[]( int iBone ) const;
	bonematrix_t&		GetBoneForWrite( int iBone );

	bonematrix_t*		GetBoneArrayForWrite( ) const;

private:

#if defined( CLIENT_DLL ) && defined( _DEBUG )
	void SanityCheckBone( int iBone, bool bReadable ) const;
#endif

	// Only used in the client DLL for debug verification.
	const C_BaseAnimating *m_pAnimating;

	bonematrix_t* m_pBones;

	int m_ReadableBones;		// Which bones can be read.
	int m_WritableBones;		// Which bones can be written.
};


inline CBoneAccessor::CBoneAccessor()
{
	m_pAnimating = NULL;
	m_pBones = NULL;
	m_ReadableBones = m_WritableBones = 0;
}

inline CBoneAccessor::CBoneAccessor(bonematrix_t* pBones )
{
	m_pAnimating = NULL;
	m_pBones = pBones;
}

#if defined( CLIENT_DLL )
	inline void CBoneAccessor::Init( const C_BaseAnimating *pAnimating, bonematrix_t *pBones )
	{
		m_pAnimating = pAnimating;
		m_pBones = pBones;
	}
#endif

inline int CBoneAccessor::GetReadableBones()
{
	return m_ReadableBones;
}

inline void CBoneAccessor::SetReadableBones( int flags )
{
	m_ReadableBones = flags;
}

inline int CBoneAccessor::GetWritableBones()
{
	return m_WritableBones;
}

inline void CBoneAccessor::SetWritableBones( int flags )
{
	m_WritableBones = flags;
}

inline const CBoneAccessor::bonematrix_t& CBoneAccessor::GetBone( int iBone ) const
{
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	SanityCheckBone( iBone, true );
#endif
	return m_pBones[iBone];
}

inline const CBoneAccessor::bonematrix_t& CBoneAccessor::operator[]( int iBone ) const
{
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	SanityCheckBone( iBone, true );
#endif
	return m_pBones[iBone];
}

inline CBoneAccessor::bonematrix_t& CBoneAccessor::GetBoneForWrite( int iBone )
{
#if defined( CLIENT_DLL ) && defined( _DEBUG )
	SanityCheckBone( iBone, false );
#endif
	return m_pBones[iBone];
}

inline CBoneAccessor::bonematrix_t*CBoneAccessor::GetBoneArrayForWrite( void ) const
{
	return m_pBones;
}

#endif // BONE_ACCESSOR_H
