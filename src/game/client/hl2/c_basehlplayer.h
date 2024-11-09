//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if !defined( C_BASEHLPLAYER_H )
#define C_BASEHLPLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "c_baseplayer.h"
#include "c_hl2_playerlocaldata.h"

#include "COOLMOD/sp_animstate.h"

class C_BaseHLRagdoll : public C_BaseAnimatingOverlay
{
public:
	DECLARE_CLASS(C_BaseHLRagdoll, C_BaseAnimatingOverlay);
	DECLARE_CLIENTCLASS();

	C_BaseHLRagdoll();
	~C_BaseHLRagdoll();

	virtual void OnDataChanged(DataUpdateType_t type);

	int GetPlayerEntIndex() const;
	IRagdoll *GetIRagdoll() const;

	void ImpactTrace(trace_t *pTrace, int iDamageType, char *pCustomImpactName);
	void UpdateOnRemove();
	virtual void SetupWeights(const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights);

private:
	C_BaseHLRagdoll(const C_BaseHLRagdoll &) {}

	void Interp_Copy(C_BaseAnimatingOverlay *pDestinationEntity);
	void CreateHL2Ragdoll();

private:
	EHANDLE	m_hPlayer;

	CNetworkVector(m_vecRagdollVelocity);
	CNetworkVector(m_vecRagdollOrigin);
};

class C_BaseHLPlayer : public C_BasePlayer
{
public:
	DECLARE_CLASS( C_BaseHLPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

						C_BaseHLPlayer();

	virtual void		OnDataChanged( DataUpdateType_t updateType );
	virtual void		ClientThink(void);

	virtual void		AddEntity(void);

	void				SetFirstPersonBlurVar(float amount);
	void				SetIronsightBlurTime(float amount);
	void				SetBlastEffectTime();
	void				SetBlurTime();
	float				m_flBlastEffectTime;
	bool				m_bBlastEffectBlur;
	float				m_flBlurTime;
	float				m_flFPBlur;
	float				m_flIronsightBlurTime;

	int					m_iShotsFired;

	CNetworkVar( bool, m_bNightVisionOn );
	float m_flNightVisionAlpha;

	void				Weapon_DropPrimary( void );
		
	float				GetFOV();
	void				Zoom( float FOVOffset, float time );
	float				GetZoom( void );
	bool				IsZoomed( void )	{ return m_HL2Local.m_bZooming; }

	bool				IsSprinting( void ) { return m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_SPRINT; }
	bool				IsFlashlightActive( void ) { return m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_FLASHLIGHT; }
	bool				IsBreatherActive( void ) { return m_HL2Local.m_bitsActiveDevices & bits_SUIT_DEVICE_BREATHER; }

	virtual int			DrawModel( int flags );
	virtual	void		BuildTransformations( CStudioHdr *hdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed );

	LadderMove_t		*GetLadderMove() { return &m_HL2Local.m_LadderMove; }
	virtual void		ExitLadder();
	bool				IsSprinting() const { return m_fIsSprinting; }
	
	// Input handling
	virtual bool	CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	void			PerformClientSideObstacleAvoidance( float flFrameTime, CUserCmd *pCmd );
	void			PerformClientSideNPCSpeedModifiers( float flFrameTime, CUserCmd *pCmd );

	bool				IsWeaponLowered( void ) { return m_HL2Local.m_bWeaponLowered; }
	virtual void		CalcDeathCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);

public:

	C_HL2PlayerLocalData		m_HL2Local;
	EHANDLE				m_hClosestNPC;
	float				m_flSpeedModTime;
	bool				m_fIsSprinting;

private:
	C_BaseHLPlayer( const C_BaseHLPlayer & ); // not defined, not accessible
	
	bool				TestMove( const Vector &pos, float fVertDist, float radius, const Vector &objPos, const Vector &objDir );

	EHANDLE				m_hRagdoll;

	float				m_flZoomStart;
	float				m_flZoomEnd;
	float				m_flZoomRate;
	float				m_flZoomStartTime;

	bool				m_bPlayUseDenySound;		// Signaled by PlayerUse, but can be unset by HL2 ladder code...
	float				m_flSpeedMod;
	float				m_flExitSpeedMod;

	CSinglePlayerAnimState *m_pPlayerAnimState;

friend class CHL2GameMovement;
};

//SMOD: We needed a pointer directly to HL2's player (like the hl2mp player and the base player have)
inline C_BaseHLPlayer *ToHL2Player(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<C_BaseHLPlayer*>(pEntity);
}
#endif