/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/


#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
#include "decals.h"
#include "sporegrenade.h"
#include "gunmod.h"

LINK_ENTITY_TO_CLASS( spore, CSpore );

CSpore *CSpore::CreateSporeGrenade( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, bool m_bIsAI )
{
	CSpore *pSpore = GetClassPtr( (CSpore *)NULL );
	UTIL_SetOrigin( pSpore->pev, vecOrigin );
	pSpore->pev->angles = vecAngles;
	pSpore->m_iPrimaryMode = FALSE;
	pSpore->pev->angles = vecAngles;
	pSpore->pev->owner = pOwner->edict();
	pSpore->pev->classname = MAKE_STRING("spore");
	pSpore->m_bIsAI = m_bIsAI;
	pSpore->Spawn();

	return pSpore;
}

//=========================================================
//=========================================================
CSpore *CSpore::CreateSporeRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner )
{
	CSpore *pSpore = GetClassPtr( (CSpore *)NULL );
	UTIL_SetOrigin( pSpore->pev, vecOrigin );
	pSpore->pev->angles = vecAngles;
	pSpore->m_iPrimaryMode = TRUE;
	pSpore->pev->angles = vecAngles;
	pSpore->pev->owner = pOwner->edict();
	pSpore->pev->classname = MAKE_STRING("spore");
	pSpore->Spawn();
 	pSpore->pev->angles = vecAngles;

	return pSpore;
}
//=========================================================
//=========================================================

void CSpore :: Spawn( void )
{
	Precache( );

	if (m_iPrimaryMode)
		pev->movetype = MOVETYPE_FLY;
	else
		pev->movetype = MOVETYPE_BOUNCE;

	pev->solid = SOLID_BBOX;

	SET_MODEL(ENT(pev), "models/spore.mdl");
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );
	UTIL_MakeVectors( pev->angles );
	if( m_bIsAI )
	{
		pev->gravity = 0.5;
		pev->friction = 0.7;
	}
	Glow();

	if (m_iPrimaryMode)
		SetTouch( &CSpore::RocketTouch );
	else
		SetTouch( &CSpore::BounceTouch );

	SetThink( &CSpore::FlyThink );
	float flDelay = m_bIsAI ? 4.0 : 2.0;
	pev->dmgtime = gpGlobals->time + flDelay;
	pev->nextthink = gpGlobals->time + 0.1;

	pev->dmg = gSkillData.plrDmgSpore;
}
//=========================================================

void CSpore :: Precache( void )
{
	PRECACHE_MODEL("models/spore.mdl");
	m_iDrips = PRECACHE_MODEL("sprites/tinyspit.spr");
	m_iGlow = PRECACHE_MODEL("sprites/glow01.spr");
	m_iExplode = PRECACHE_MODEL ("sprites/spore_exp_01.spr");
	m_iExplodeC = PRECACHE_MODEL ("sprites/spore_exp_c_01.spr");
	PRECACHE_SOUND ("weapons/splauncher_impact.wav");
	PRECACHE_SOUND ("weapons/spore_hit1.wav");//Bounce grenade
	PRECACHE_SOUND ("weapons/spore_hit2.wav");//Bounce grenade
	PRECACHE_SOUND ("weapons/spore_hit3.wav");//Bounce grenade
}
//=========================================================

void CSpore :: Glow( void )
{
	m_pSprite = CSprite::SpriteCreate( "sprites/glow01.spr", pev->origin, FALSE );
	m_pSprite->SetAttachment( edict(), 0 );
	m_pSprite->pev->scale = 0.8;
	m_pSprite->SetTransparency( kRenderTransAdd, 180, 180, 40, 100, kRenderFxDistort );
}
//=========================================================

void CSpore :: FlyThink( void  )
{
	Vector vecEnd = pev->origin.Normalize();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE_SPRAY );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( vecEnd.x );
		WRITE_COORD( vecEnd.y );
		WRITE_COORD( vecEnd.z );
		WRITE_SHORT( m_iDrips );
		WRITE_BYTE( 2 ); // count
		WRITE_BYTE( 20 ); // speed
		WRITE_BYTE( 80 );
	MESSAGE_END();

	if (!m_iPrimaryMode)
	{
		if (pev->dmgtime <= gpGlobals->time)
			Explode();
	}
	pev->nextthink = gpGlobals->time + 0.03;
}

void CSpore::RocketTouch( CBaseEntity *pOther )
{
	if(pOther->pev->takedamage)
	{
		entvars_t *pevOwner;
		pevOwner = VARS( pev->owner );

		pOther->TakeDamage( pev, pevOwner, pev->dmg, DMG_GENERIC );
	}

	Explode();
}

void CSpore::BounceTouch( CBaseEntity *pOther )
{
	if ( !pOther->pev->takedamage )
	{
		if( pev->flags & FL_SWIM )
		{
			pev->velocity = pev->velocity * 0.5;
			pev->sequence = 1;
		}
		else
			BounceSound(); // play bounce sound
	}
	else
	{
		entvars_t *pevOwner;
		pevOwner = VARS( pev->owner );

		pOther->TakeDamage( pev, pevOwner, pev->dmg, DMG_GENERIC );

		Explode();
	}

	pev->framerate = pev->velocity.Length() / 200.0;
	if (pev->framerate > 1.0)
		pev->framerate = 1;
	else if (pev->framerate < 0.5)
		pev->framerate = 0;

}

void CSpore :: BounceSound( void )
{
	switch (RANDOM_LONG (0, 2))
	{
		case 0: 
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/spore_hit1.wav", 0.25, ATTN_NORM); 
			break;
		case 1: 
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/spore_hit2.wav", 0.25, ATTN_NORM); 
			break;
		case 2: 
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/spore_hit3.wav", 0.25, ATTN_NORM); 
			break;
	}
}

void CSpore::Explode( void )
{
	SetTouch( NULL );
	SetThink( NULL );
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/splauncher_impact.wav", 1, ATTN_NORM);

	TraceResult tr;
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		switch ( RANDOM_LONG( 0, 1 ) )
		{
			case 0:	
				WRITE_SHORT( m_iExplode );
				break;
			default:
			case 1:
				WRITE_SHORT( m_iExplodeC );
				break;
		}
		WRITE_BYTE( 20 ); // scale * 10
		WRITE_BYTE( 128 ); // framerate
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE_SPRAY );		// This makes a dynamic light and the explosion sprites/sound
		WRITE_COORD( pev->origin.x );	// Send to PAS because of the sound
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( tr.vecPlaneNormal.x );
		WRITE_COORD( tr.vecPlaneNormal.y );
		WRITE_COORD( tr.vecPlaneNormal.z );
		WRITE_SHORT( m_iDrips );
		WRITE_BYTE( 2  ); // count
		WRITE_BYTE( 40 ); // speed
		WRITE_BYTE( 80 );
	MESSAGE_END();

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD( pev->origin.x );	// X
		WRITE_COORD( pev->origin.y );	// Y
		WRITE_COORD( pev->origin.z );	// Z
		WRITE_BYTE( 10 );		// radius * 0.1
		WRITE_BYTE( 15 );		// r
		WRITE_BYTE( 220 );		// g
		WRITE_BYTE( 40 );		// b
		WRITE_BYTE( 5 );		// time * 10
		WRITE_BYTE( 10 );		// decay * 0.1
	MESSAGE_END( );

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_SPRITE_SPRAY );
		WRITE_COORD( pev->origin.x );
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_COORD( RANDOM_FLOAT( -1, 1 ) );
		WRITE_COORD( 1 );
		WRITE_COORD( RANDOM_FLOAT( -1, 1 ) );
		WRITE_SHORT( m_iDrips );
		WRITE_BYTE( 2 );
		WRITE_BYTE( 20 );
		WRITE_BYTE( 80 );
	MESSAGE_END();

    	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	::RadiusDamage ( pev->origin, pev, pevOwner, pev->dmg, 200, CLASS_PLAYER_BIOWEAPON, DMG_GENERIC );

	if (m_iPrimaryMode)
	{
		TraceResult tr;
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT( pev ), &tr );
	}

	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.3;
	UTIL_Remove ( this );
	UTIL_Remove( m_pSprite );
	m_pSprite = NULL;
}
