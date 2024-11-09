//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basecombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "weapon_ar2.h"
#include "grenade_ar2.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "ai_memory.h"
#include "soundent.h"
#include "hl2_player.h"
#include "EntityFlame.h"
#include "weapon_flaregun.h"
#include "te_effect_dispatch.h"
#include "prop_combine_ball.h"
#include "beam_shared.h"
#include "npc_combine.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "bullet_ar2.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_weapon_ar2_alt_fire_radius( "sk_weapon_ar2_alt_fire_radius", "10" );
ConVar sk_weapon_ar2_alt_fire_duration( "sk_weapon_ar2_alt_fire_duration", "2" );
ConVar sk_weapon_ar2_alt_fire_mass( "sk_weapon_ar2_alt_fire_mass", "150" );

extern ConVar bullettimesim;

extern ConVar disable_bullettime;

//=========================================================
//=========================================================

BEGIN_DATADESC( CWeaponAR2 )

	DEFINE_FIELD( m_flDelayedFire,	FIELD_TIME ),
	DEFINE_FIELD( m_bShotDelayed,	FIELD_BOOLEAN ),
	//DEFINE_FIELD( m_nVentPose, FIELD_INTEGER ),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponAR2, DT_WeaponAR2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_ar2, CWeaponAR2 );
PRECACHE_WEAPON_REGISTER(weapon_ar2);

acttable_t	CWeaponAR2::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },		// FIXME: hook to AR2 unique

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },

// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },

	{ ACT_HL2MP_IDLE, ACT_HL2MP_IDLE_AR2, false },
	{ ACT_HL2MP_RUN, ACT_HL2MP_RUN_AR2, false },
	{ ACT_HL2MP_IDLE_CROUCH, ACT_HL2MP_IDLE_CROUCH_AR2, false },
	{ ACT_HL2MP_WALK_CROUCH, ACT_HL2MP_WALK_CROUCH_AR2, false },
	{ ACT_HL2MP_GESTURE_RANGE_ATTACK, ACT_HL2MP_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_HL2MP_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, false },
	{ ACT_HL2MP_JUMP, ACT_HL2MP_JUMP_AR2, false },
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_AR2, false },
};

IMPLEMENT_ACTTABLE(CWeaponAR2);

CWeaponAR2::CWeaponAR2( )
{
	m_fMinRange1	= 65;
	m_fMaxRange1	= 2048;

	m_fMinRange2	= 256;
	m_fMaxRange2	= 1024;

	m_nShotsFired	= 0;
	m_nVentPose		= -1;

	m_bAltFiresUnderwater = false;
}

void CWeaponAR2::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther( "prop_combine_ball" );
	UTIL_PrecacheOther( "env_entity_dissolver" );
	UTIL_PrecacheOther( "bullet_ar2" );
}

//-----------------------------------------------------------------------------
// Purpose: Handle grenade detonate in-air (even when no ammo is left)
//-----------------------------------------------------------------------------
void CWeaponAR2::ItemPostFrame( void )
{
	// See if we need to fire off our secondary round
	if ( m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire )
	{
		DelayedAttack();
	}

	// Update our pose parameter for the vents
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner )
	{
		CBaseViewModel *pVM = pOwner->GetViewModel();

		if ( pVM )
		{
			if ( m_nVentPose == -1 )
			{
				m_nVentPose = pVM->LookupPoseParameter( "VentPoses" );
			}
			
			float flVentPose = RemapValClamped( m_nShotsFired, 0, 5, 0.0f, 1.0f );
			pVM->SetPoseParameter( m_nVentPose, flVentPose );
		}
	}

	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponAR2::GetPrimaryAttackActivity( void )
{
	if ( m_nShotsFired < 2 )
		return ACT_VM_PRIMARYATTACK;

	if ( m_nShotsFired < 3 )
		return ACT_VM_RECOIL1;
	
	if ( m_nShotsFired < 4 )
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

void CWeaponAR2::FireNPC9MMBullet( void )
{
	// m_vecEnemyLKP should be center of enemy body
	Vector vecArmPos;
	QAngle angArmDir;
	Vector vecDirToEnemy;
	QAngle angDir;

	if ( GetEnemy() )
	{
		Vector vecEnemyLKP = GetEnemy()->GetAbsOrigin();

		vecDirToEnemy = ( ( vecEnemyLKP ) - GetAbsOrigin() );
		VectorAngles( vecDirToEnemy, angDir );
		VectorNormalize( vecDirToEnemy );
	}
	else
	{
		angDir = GetAbsAngles();
		angDir.x = -angDir.x;

		Vector vForward;
		AngleVectors( angDir, &vForward );
		vecDirToEnemy = vForward;
	}

	DoMuzzleFlash();

	// make angles +-180
	if (angDir.x > 180)
	{
		angDir.x = angDir.x - 360;
	}

	VectorAngles( vecDirToEnemy, angDir );

	float RandomAngle = (rand() % 55960);
	float RandMagnitudeX = ((rand() % 70375) / 3800.0);
	float RandMagnitudeY = ((rand() % 70375) / 3800.0);
	angDir.x += (RandMagnitudeX)*cos(RandomAngle);
	angDir.y += (RandMagnitudeY)*sin(RandomAngle);

	AngleVectors(angDir, &vecDirToEnemy);

	GetAttachment( "muzzle", vecArmPos, angArmDir );

	vecArmPos = vecArmPos + vecDirToEnemy * 32;

	CBaseEntity *pBullet = CBaseEntity::Create( "bullet_9mm", vecArmPos, QAngle( 0, 0, 0 ), this );

	Vector vForward;
	AngleVectors( angDir, &vForward );
	
	pBullet->SetAbsVelocity( vForward * 475 );
	pBullet->SetOwnerEntity( this );
			
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, this, SOUNDENT_CHANNEL_WEAPON, GetEnemy() );

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponAR2::PrimaryAttack( void )
{
	CHL2_Player *pPlayer = dynamic_cast < CHL2_Player* >(UTIL_PlayerByIndex(1));

	if (!pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
	{
		// Only the player fires this way so we can cast
		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (!pPlayer)
			return;

		// Abort here to handle burst and auto fire modes
		if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
			return;

		m_nShotsFired++;

		pPlayer->DoMuzzleFlash();

		// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
		// especially if the weapon we're firing has a really fast rate of fire.
		int iBulletsToFire = 0;
		float fireRate = GetFireRate();

		// MUST call sound before removing a round from the clip of a CHLMachineGun
		while (m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			WeaponSound(SINGLE, m_flNextPrimaryAttack);
			m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
			iBulletsToFire++;
		}

		// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
		if (UsesClipsForAmmo1())
		{
			if (iBulletsToFire > m_iClip1)
				iBulletsToFire = m_iClip1;
			m_iClip1 -= iBulletsToFire;
		}

		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

		// Fire the bullets
		FireBulletsInfo_t info;
		info.m_iShots = iBulletsToFire;
		info.m_vecSrc = pPlayer->Weapon_ShootPosition();
		info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
		info.m_vecSpread = pPlayer->GetAttackSpread(this);
		info.m_flDistance = MAX_TRACE_LENGTH;
		info.m_iAmmoType = m_iPrimaryAmmoType;
		info.m_iTracerFreq = 2;
		FireBullets(info);

		//Factor in the view kick
		AddViewKick();

		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer);

		if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		{
			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
		}

		SendWeaponAnim(GetPrimaryAttackActivity());
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		// Register a muzzleflash for the AI
		pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
	}
	else if (pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
	{
		Fire9MMBullet();
	}

	if (bullettimesim.GetInt() == 1 && disable_bullettime.GetInt() == 1)
	{
		Fire9MMBullet();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponAR2::Fire9MMBullet( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if (!pPlayer)
		return;
	
	// Abort here to handle burst and auto fire modes
	if ( (UsesClipsForAmmo1() && m_iClip1 == 0) || ( !UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType) ) )
		return;

	m_nShotsFired++;

	pPlayer->DoMuzzleFlash();

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	int iBulletsToFire = 0;
	float fireRate = GetFireRate();

	// MUST call sound before removing a round from the clip of a CHLMachineGun
	while ( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if ( UsesClipsForAmmo1() )
	{
		if ( iBulletsToFire > m_iClip1 )
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

	Vector vecAiming;
	vecAiming = pPlayer->GetAutoaimVector(0);
	Vector vecSrc = pPlayer->Weapon_ShootPosition();

	QAngle angAiming;
	VectorAngles(vecAiming, angAiming);

	CBulletAR2 *pBullet = CBulletAR2::BulletCreate(vecSrc, angAiming, pPlayer);

	if (pPlayer->GetWaterLevel() == 3)
	{
		pBullet->SetAbsVelocity(vecAiming * 1100);
	}
	else
	{
		pBullet->SetAbsVelocity(vecAiming * 1400);
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + fireRate;

	/*
	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition( );
	info.m_vecDirShooting = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	info.m_vecSpread = pPlayer->GetAttackSpread( this );
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	FireBullets( info );
	*/

	//Factor in the view kick
	AddViewKick();

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer );
	
	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	}

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeaponAR2::DoImpactEffect( trace_t &tr, int nDamageType )
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + ( tr.plane.normal * 1.0f );
	data.m_vNormal = tr.plane.normal;

	DispatchEffect( "AR2Impact", data );

	BaseClass::DoImpactEffect( tr, nDamageType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::DelayedAttack( void )
{
	m_bShotDelayed = false;
	
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner == NULL )
		return;

	// Deplete the clip completely
	SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	m_flNextSecondaryAttack = pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration();

	// Register a muzzleflash for the AI
	pOwner->DoMuzzleFlash();
	pOwner->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );
	
	WeaponSound( WPN_DOUBLE );

	pOwner->RumbleEffect(RUMBLE_SHOTGUN_DOUBLE, 0, RUMBLE_FLAG_RESTART );

	// Fire the bullets
	Vector vecSrc	 = pOwner->Weapon_ShootPosition( );
	Vector vecAiming = pOwner->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );
	Vector impactPoint = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(	vecSrc, 
						vecVelocity, 
						sk_weapon_ar2_alt_fire_radius.GetFloat(), 
						sk_weapon_ar2_alt_fire_mass.GetFloat(),
						sk_weapon_ar2_alt_fire_duration.GetFloat(),
						pOwner );

	// View effects
	color32 white = {255, 255, 255, 64};
	UTIL_ScreenFade( pOwner, white, 0.1, 0, FFADE_IN  );
	
	//Disorient the player
	QAngle angles = pOwner->GetLocalAngles();

	angles.x += random->RandomInt( -4, 4 );
	angles.y += random->RandomInt( -4, 4 );
	angles.z = 0;

	pOwner->SnapEyeAngles( angles );
	
	pOwner->ViewPunch( QAngle( random->RandomInt( -8, -12 ), random->RandomInt( 1, 2 ), 0 ) );

	// Decrease ammo
	pOwner->RemoveAmmo( 1, m_iSecondaryAmmoType );

	// Can shoot again immediately
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::SecondaryAttack( void )
{
	if ( m_bShotDelayed )
		return;

	// Cannot fire underwater
	if ( GetOwner() && GetOwner()->GetWaterLevel() == 3 )
	{
		SendWeaponAnim( ACT_VM_DRYFIRE );
		BaseClass::WeaponSound( EMPTY );
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
		return;
	}

	m_bShotDelayed = true;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.5f;

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	if( pPlayer )
	{
		pPlayer->RumbleEffect(RUMBLE_AR2_ALT_FIRE, 0, RUMBLE_FLAG_RESTART );
	}

	SendWeaponAnim( ACT_VM_FIDGET );
	WeaponSound( SPECIAL1 );

	m_iSecondaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, false, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponAR2::CanHolster( void )
{
	if ( m_bShotDelayed )
		return false;

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
//-----------------------------------------------------------------------------
bool CWeaponAR2::Reload( void )
{
	if ( m_bShotDelayed )
		return false;

	return BaseClass::Reload();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	Vector vecShootOrigin, vecShootDir;

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT( npc != NULL );

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
		AngleVectors( angShootDir, &vecShootDir );
	}
	else 
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );
	}

	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );

	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2 );

	// NOTENOTE: This is overriden on the client-side
	// pOperator->DoMuzzleFlash();

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCSecondaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles )
{
	WeaponSound( WPN_DOUBLE );

	if ( !GetOwner() )
		return;
		
	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if ( !pNPC )
		return;
	
	// Fire!
	Vector vecSrc;
	Vector vecAiming;

	if ( bUseWeaponAngles )
	{
		QAngle	angShootDir;
		GetAttachment( LookupAttachment( "muzzle" ), vecSrc, angShootDir );
		AngleVectors( angShootDir, &vecAiming );
	}
	else 
	{
		vecSrc = pNPC->Weapon_ShootPosition( );
		
		Vector vecTarget;

		CNPC_Combine *pSoldier = dynamic_cast<CNPC_Combine *>( pNPC );
		if ( pSoldier )
		{
			// In the distant misty past, elite soldiers tried to use bank shots.
			// Therefore, we must ask them specifically what direction they are shooting.
			vecTarget = pSoldier->GetAltFireTarget();
		}
		else
		{
			// All other users of the AR2 alt-fire shoot directly at their enemy.
			if ( !pNPC->GetEnemy() )
				return;
				
			vecTarget = pNPC->GetEnemy()->BodyTarget( vecSrc );
		}

		vecAiming = vecTarget - vecSrc;
		VectorNormalize( vecAiming );
	}

	Vector impactPoint = vecSrc + ( vecAiming * MAX_TRACE_LENGTH );

	float flAmmoRatio = 1.0f;
	float flDuration = RemapValClamped( flAmmoRatio, 0.0f, 1.0f, 0.5f, sk_weapon_ar2_alt_fire_duration.GetFloat() );
	float flRadius = RemapValClamped( flAmmoRatio, 0.0f, 1.0f, 4.0f, sk_weapon_ar2_alt_fire_radius.GetFloat() );

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(	vecSrc, 
		vecVelocity, 
		flRadius, 
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		flDuration,
		pNPC );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	if ( bSecondary )
	{
		FireNPCSecondaryAttack( pOperator, true );
	}
	else
	{
		// Ensure we have enough rounds in the clip
		m_iClip1++;

		FireNPCPrimaryAttack( pOperator, true );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CHL2_Player *pPlayer = dynamic_cast < CHL2_Player* >(UTIL_PlayerByIndex(1));

	switch( pEvent->event )
	{ 
		case EVENT_WEAPON_AR2:
			{
				if (!pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
				{
					FireNPCPrimaryAttack(pOperator, false);
			    }
				else if (pPlayer->m_HL2Local.m_bInSlowMo && disable_bullettime.GetInt() == 0)
				{
					FireNPC9MMBullet();
				}
				if (bullettimesim.GetInt() == 1 && disable_bullettime.GetInt() == 1)
				{
					FireNPC9MMBullet();
				}
			}
			break;

		case EVENT_WEAPON_AR2_ALTFIRE:
			{
				FireNPCSecondaryAttack( pOperator, false );
			}
			break;

		default:
			CBaseCombatWeapon::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	8.0f	//Degrees
	#define	SLIDE_LIMIT			5.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
		return;

	float flDuration = m_fFireDuration;

	if( g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE )
	{
		// On the 360 (or in any configuration using the 360 aiming scheme), don't let the
		// AR2 progressive into the late, highly inaccurate stages of its kick. Just
		// spoof the time to make it look (to the kicking code) like we haven't been
		// firing for very long.
		flDuration = MIN( flDuration, 0.75f );
	}

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, flDuration, SLIDE_LIMIT );
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponAR2::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 3.0,		0.85	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}