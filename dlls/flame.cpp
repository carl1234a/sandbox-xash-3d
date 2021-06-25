#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"

LINK_ENTITY_TO_CLASS( flamethrower_flame, CFlame );

void CFlame::Explode( Vector vecSrc, Vector vecAim )
{
   TraceResult tr;
   UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ), ignore_monsters, ENT(pev), & tr);

   Explode( &tr, DMG_BURN );
}

void CFlame::Explode( TraceResult *pTrace, int bitsDamageType )
{
   float flRndSound;

   pev->model = iStringNull;
   pev->solid = SOLID_NOT;

   pev->takedamage = DAMAGE_NO;

   // Pull out of the wall a bit
   if ( pTrace->flFraction != 1.0 )
   {
     pev->origin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * 60);
   }

   int iContents = UTIL_PointContents ( pev->origin );

   MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
   WRITE_BYTE( TE_EXPLOSION );
   WRITE_COORD( pev->origin.x );
   WRITE_COORD( pev->origin.y );
   WRITE_COORD( pev->origin.z );
   WRITE_SHORT( g_sModelIndexFlame );
   WRITE_BYTE( 10 );
   WRITE_BYTE( 15 );
   WRITE_BYTE( TE_EXPLFLAG_NOSOUND );
   MESSAGE_END();

   MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
   WRITE_BYTE( TE_DLIGHT );
   WRITE_COORD( pev->origin.x );
   WRITE_COORD( pev->origin.y );
   WRITE_COORD( pev->origin.z );
   WRITE_BYTE( 50 );
   WRITE_BYTE( 220 );
   WRITE_BYTE( 150 );
   WRITE_BYTE( 0 );
   WRITE_BYTE( 10 );
   WRITE_BYTE( 10 );
   MESSAGE_END();

   CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, BIG_EXPLOSION_VOLUME, 3.0 );
   entvars_t *pevOwner;
   if ( pev->owner )
     pevOwner = VARS( pev->owner );
   else
     pevOwner = NULL;

   pev->owner = NULL;

   ::RadiusDamage( pev->origin, pev, pevOwner, pev->dmg, 175, CLASS_NONE, DMG_BURN );

   if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
   {
     UTIL_DecalTrace( pTrace, DECAL_SMALLSCORCH1 );
   }
   else
   {
     UTIL_DecalTrace( pTrace, DECAL_SMALLSCORCH3 );
   }

   flRndSound = RANDOM_FLOAT( 0 , 1 );

   EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flame_hitwall.wav", 0.55, ATTN_NORM);

   pev->effects |= EF_NODRAW;
   pev->velocity = g_vecZero;
   SetThink( &CFlame::Smoke );
   pev->nextthink = gpGlobals->time + 0.1;
}

void CFlame::Smoke( void )
{
   if (UTIL_PointContents ( pev->origin ) == CONTENTS_WATER)
   {
     UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
   }
   else
   {
     MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
     WRITE_BYTE( TE_SMOKE );
     WRITE_COORD( pev->origin.x );
     WRITE_COORD( pev->origin.y );
     WRITE_COORD( pev->origin.z );
     WRITE_SHORT( g_sModelIndexSmoke );
     WRITE_BYTE( 10 );
     WRITE_BYTE( 6 );
     MESSAGE_END();
   }
   UTIL_Remove( this );
}

void CFlame::Killed( entvars_t *pevAttacker, int iGib )
{
   Detonate( );
}


void CFlame::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
   SetThink( &CFlame::Detonate );
   pev->nextthink = gpGlobals->time;
}

void CFlame::PreDetonate( void )
{
   CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

   SetThink( &CFlame::Detonate );
   pev->nextthink = gpGlobals->time + 1;
}


void CFlame::Detonate( void )
{
   TraceResult tr;
   Vector vecSpot;

   vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
   UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ), ignore_monsters, ENT(pev), & tr);

   Explode( &tr, DMG_BURN );

}


void CFlame::ExplodeTouch( CBaseEntity *pOther )
{
   TraceResult tr;
   Vector vecSpot;

   pev->enemy = pOther->edict();

   vecSpot = pev->origin - pev->velocity.Normalize() * 32;
   UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr );

   Explode( &tr, DMG_BURN );
}

void CFlame::DangerSoundThink( void )
{
   if (!IsInWorld())
   {
     UTIL_Remove( this );
     return;
   }

   CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length( ), 0.2 );
   pev->nextthink = gpGlobals->time + 0.2;

   if (pev->waterlevel != 0)
   {
     pev->velocity = pev->velocity * 0.5;
   }
}

void CFlame::BounceTouch( CBaseEntity *pOther )
{
   if ( pOther->edict() == pev->owner )
     return;

   if (m_flNextAttack < gpGlobals->time && pev->velocity.Length() > 100)
   {
     entvars_t *pevOwner = VARS( pev->owner );
     if (pevOwner)
     {
       TraceResult tr = UTIL_GetGlobalTrace( );
       ClearMultiDamage( );
       pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB );
       ApplyMultiDamage( pev, pevOwner);
     }
     m_flNextAttack = gpGlobals->time + 1.0;
   }

   Vector vecTestVelocity;
   vecTestVelocity = pev->velocity;
   vecTestVelocity.z *= 0.45;

   if ( !m_fRegisteredSound && vecTestVelocity.Length() <= 60 )
   {
     CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4, 0.3 );
     m_fRegisteredSound = TRUE;
   }

   if (pev->flags & FL_ONGROUND)
   {
     pev->velocity = pev->velocity * 0.8;

     pev->sequence = RANDOM_LONG( 1, 1 );
   }
   else
   {
     BounceSound();
   }
   pev->framerate = pev->velocity.Length() / 200.0;
   if (pev->framerate > 1.0)
     pev->framerate = 1;
   else if (pev->framerate < 0.5)
     pev->framerate = 0;

}

void CFlame::SlideTouch( CBaseEntity *pOther )
{
   if ( pOther->edict() == pev->owner )
     return;

   if (pev->flags & FL_ONGROUND)
   {
     pev->velocity = pev->velocity * 0.95;

     if (pev->velocity.x != 0 || pev->velocity.y != 0)
     {
     }
   }
   else
   {
     BounceSound();
   }
}

void CFlame :: BounceSound( void )
{
   EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/flame_hitwall.wav", 0.25, ATTN_NORM);
}

void CFlame:: Spawn( void )
{
   pev->movetype = MOVETYPE_BOUNCE;
   pev->classname = MAKE_STRING( "flamethrower_flame" );

   pev->solid = SOLID_BBOX;

   SET_MODEL(ENT(pev), "sprites/flamesteam.spr");
   pev->rendermode = kRenderTransAdd;
   pev->rendercolor.x = 255;
   pev->rendercolor.y = 255;
   pev->rendercolor.z = 255;
   pev->renderamt = 255;
   if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
   {
     pev->scale = 1;
   }
   else
   {
     pev->scale = 2;
   }

   UTIL_SetSize(pev, Vector ( 0, 0, 0 ), Vector ( 0, 0, 0 ) );

   pev->dmg = gSkillData.plrDmgFlamethrower;
   m_fRegisteredSound = FALSE;
}

CFlame *CFlame::ShootFlame( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
   CFlame *pFlame = GetClassPtr( (CFlame *)NULL );
   pFlame->Spawn();

   pFlame->pev->gravity = 1.5;
   UTIL_SetOrigin( pFlame->pev, vecStart );
   pFlame->pev->velocity = vecVelocity;
   pFlame->pev->angles = UTIL_VecToAngles (pFlame->pev->velocity);
   pFlame->pev->owner = ENT(pevOwner);

   pFlame->SetThink( &CFlame::DangerSoundThink );
   pFlame->pev->nextthink = gpGlobals->time;

   pFlame->SetTouch( &CFlame::ExplodeTouch );

   pFlame->pev->dmg = gSkillData.plrDmgFlamethrower;

   return pFlame;
}
