//fixed
/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// bullsquid - big, spotty tentacle-mouthed meanie.
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"nodes.h"
#include	"effects.h"
#include	"decals.h"
#include	"soundent.h"
#include	"game.h"

#define		FRIENDLY_SPRINT_DIST	256 // how close the squid has to get before starting to sprint and refusing to swerve
//iFriendlySpitSprite & iSquidSpitSprite; CFriendlySpit & CSquidSpit
int iFriendlySpitSprite;
	
//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_FRIENDLY_HURTHOP = LAST_COMMON_SCHEDULE + 1,
	SCHED_FRIENDLY_SMELLFOOD,
	SCHED_FRIENDLY_SEECRAB,
	SCHED_FRIENDLY_EAT,
	SCHED_FRIENDLY_SNIFF_AND_EAT,
	SCHED_FRIENDLY_WALLOW
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_FRIENDLY_HOPTURN = LAST_COMMON_TASK + 1
};

//=========================================================
// Bullsquid's spit projectile
//=========================================================
class CFriendlySpit : public CBaseEntity
{
public:
	void Spawn( void );

	static void Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	void Touch( CBaseEntity *pOther );
	void EXPORT Animate( void );

	virtual int Save( CSave &save );
	virtual int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	int m_maxFrame;
};

LINK_ENTITY_TO_CLASS( friendlyspit, CFriendlySpit )

TYPEDESCRIPTION	CFriendlySpit::m_SaveData[] =
{
	DEFINE_FIELD( CFriendlySpit, m_maxFrame, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CFriendlySpit, CBaseEntity )

void CFriendlySpit::Spawn( void )
{
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING( "friendlyspit" );

	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;

	SET_MODEL( ENT( pev ), "sprites/bigspit.spr" );
	pev->frame = 0;
	pev->scale = 0.5;

	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	m_maxFrame = (float)MODEL_FRAMES( pev->modelindex ) - 1;
}

void CFriendlySpit::Animate( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	if( pev->frame++ )
	{
		if( pev->frame > m_maxFrame )
		{
			pev->frame = 0;
		}
	}
}

void CFriendlySpit::Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CFriendlySpit *pSpit = GetClassPtr( (CFriendlySpit *)NULL );
	pSpit->Spawn();

	UTIL_SetOrigin( pSpit->pev, vecStart );
	pSpit->pev->velocity = vecVelocity;
	pSpit->pev->owner = ENT( pevOwner );

	pSpit->SetThink( &CFriendlySpit::Animate );
	pSpit->pev->nextthink = gpGlobals->time + 0.1;
}

void CFriendlySpit::Touch( CBaseEntity *pOther )
{
	TraceResult tr;
	int iPitch;

	// splat sound
	iPitch = RANDOM_FLOAT( 90, 110 );

	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch );

	switch( RANDOM_LONG( 0, 1 ) )
	{
	case 0:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	case 1:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	}

	if( !pOther->pev->takedamage )
	{
		// make a splat on the wall
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT( pev ), &tr );
		UTIL_DecalTrace( &tr, DECAL_SPIT1 + RANDOM_LONG( 0, 1 ) );

		// make some flecks
		MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, tr.vecEndPos );
			WRITE_BYTE( TE_SPRITE_SPRAY );
			WRITE_COORD( tr.vecEndPos.x );	// pos
			WRITE_COORD( tr.vecEndPos.y );
			WRITE_COORD( tr.vecEndPos.z );
			WRITE_COORD( tr.vecPlaneNormal.x );	// dir
			WRITE_COORD( tr.vecPlaneNormal.y );
			WRITE_COORD( tr.vecPlaneNormal.z );
			WRITE_SHORT( iFriendlySpitSprite );	// model
			WRITE_BYTE( 5 );			// count
			WRITE_BYTE( 30 );			// speed
			WRITE_BYTE( 80 );			// noise ( client will divide by 100 )
		MESSAGE_END();
	}
	else
	{
		pOther->TakeDamage( pev, pev, gSkillData.bullsquidDmgSpit, DMG_GENERIC );
	}

	SetThink( &CBaseEntity::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		BFRIENDLY_AE_SPIT		( 1 )
#define		BFRIENDLY_AE_BITE		( 2 )
#define		BFRIENDLY_AE_BLINK		( 3 )
#define		BFRIENDLY_AE_TAILWHIP	( 4 )
#define		BFRIENDLY_AE_HOP		( 5 )
#define		BFRIENDLY_AE_THROW		( 6 )

class CFriendly : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int ISoundMask( void );
	int Classify( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void IdleSound( void );
	void PainSound( void );
	void DeathSound( void );
	void AlertSound( void );
	void AttackSound( void );
	void StartTask( Task_t *pTask );
	void RunTask( Task_t *pTask );
	BOOL CheckMeleeAttack1( float flDot, float flDist );
	BOOL CheckMeleeAttack2( float flDot, float flDist );
	BOOL CheckRangeAttack1( float flDot, float flDist );
	void RunAI( void );
	BOOL FValidateHintType( short sHint );
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType( int Type );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	int IRelationship( CBaseEntity *pTarget );
	int IgnoreConditions( void );
	MONSTERSTATE GetIdealState( void );

	int Save( CSave &save ); 
	int Restore( CRestore &restore );

	CUSTOM_SCHEDULES
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fCanThreatDisplay;// this is so the squid only does the "I see a headcrab!" dance one time. 

	float m_flLastHurtTime;// we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.
	float m_flNextSpitTime;// last time the bullsquid used the spit attack.
};

LINK_ENTITY_TO_CLASS( monster_friendly, CFriendly )

TYPEDESCRIPTION	CFriendly::m_SaveData[] =
{
	DEFINE_FIELD( CFriendly, m_fCanThreatDisplay, FIELD_BOOLEAN ),
	DEFINE_FIELD( CFriendly, m_flLastHurtTime, FIELD_TIME ),
	DEFINE_FIELD( CFriendly, m_flNextSpitTime, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CFriendly, CBaseMonster )

//=========================================================
// IgnoreConditions 
//=========================================================
int CFriendly::IgnoreConditions( void )
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if( gpGlobals->time - m_flLastHurtTime <= 20 )
	{
		// haven't been hurt in 20 seconds, so let the squid care about stink. 
		iIgnore = bits_COND_SMELL | bits_COND_SMELL_FOOD;
	}

	if( m_hEnemy != NULL )
	{
		if( FClassnameIs( m_hEnemy->pev, "monster_headcrab" ) )
		{
			// (Unless after a tasty headcrab)
			iIgnore = bits_COND_SMELL | bits_COND_SMELL_FOOD;
		}
	}

	return iIgnore;
}

//=========================================================
// IRelationship - overridden for bullsquid so that it can
// be made to ignore its love of headcrabs for a while.
//=========================================================
int CFriendly::IRelationship( CBaseEntity *pTarget )
{
	if( gpGlobals->time - m_flLastHurtTime < 5 && FClassnameIs( pTarget->pev, "monster_headcrab" ) )
	{
		// if squid has been hurt in the last 5 seconds, and is getting relationship for a headcrab, 
		// tell squid to disregard crab. 
		return R_NO;
	}

	return CBaseMonster::IRelationship( pTarget );
}

//=========================================================
// TakeDamage - overridden for bullsquid so we can keep track
// of how much time has passed since it was last injured
//=========================================================
int CFriendly::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	float flDist;
	Vector vecApex;

	// if the squid is running, has an enemy, was hurt by the enemy, hasn't been hurt in the last 3 seconds, and isn't too close to the enemy,
	// it will swerve. (whew).
	if( m_hEnemy != NULL && IsMoving() && pevAttacker == m_hEnemy->pev && gpGlobals->time - m_flLastHurtTime > 3 )
	{
		flDist = ( pev->origin - m_hEnemy->pev->origin ).Length2D();

		if( flDist > FRIENDLY_SPRINT_DIST )
		{
			flDist = ( pev->origin - m_Route[m_iRouteIndex].vecLocation ).Length2D();// reusing flDist.

			if( FTriangulate( pev->origin, m_Route[m_iRouteIndex].vecLocation, flDist * 0.5, m_hEnemy, &vecApex ) )
			{
				InsertWaypoint( vecApex, bits_MF_TO_DETOUR | bits_MF_DONT_SIMPLIFY );
			}
		}
	}

	if( !FClassnameIs( pevAttacker, "monster_headcrab" ) )
	{
		// don't forget about headcrabs if it was a headcrab that hurt the squid.
		m_flLastHurtTime = gpGlobals->time;
	}

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CFriendly::CheckRangeAttack1( float flDot, float flDist )
{
	if( IsMoving() && flDist >= 512 )
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return FALSE;
	}

	if( flDist > 64 && flDist <= 784 && flDot >= 0.5 && gpGlobals->time >= m_flNextSpitTime )
	{
		if( m_hEnemy != NULL )
		{
			if( fabs( pev->origin.z - m_hEnemy->pev->origin.z ) > 256 )
			{
				// don't try to spit at someone up really high or down really low.
				return FALSE;
			}
		}

		if( IsMoving() )
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextSpitTime = gpGlobals->time + 5;
		}
		else
		{
			// not moving, so spit again pretty soon.
			m_flNextSpitTime = gpGlobals->time + 0.5;
		}

		return TRUE;
	}

	return FALSE;
}

//=========================================================
// CheckMeleeAttack1 - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the tailwhip attack
//=========================================================
BOOL CFriendly::CheckMeleeAttack1( float flDot, float flDist )
{
	if( m_hEnemy->pev->health <= gSkillData.bullsquidDmgWhip && flDist <= 85 && flDot >= 0.7 )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckMeleeAttack2 - bullsquid is a big guy, so has a longer
// melee range than most monsters. This is the bite attack.
// this attack will not be performed if the tailwhip attack
// is valid.
//=========================================================
BOOL CFriendly::CheckMeleeAttack2( float flDot, float flDist )
{
	if( flDist <= 85 && flDot >= 0.7 && !HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )		// The player & bullsquid can be as much as their bboxes 
	{										// apart (48 * sqrt(3)) and he can still attack (85 is a little more than 48*sqrt(3))
		return TRUE;
	}
	return FALSE;
}

//=========================================================
//  FValidateHintType 
//=========================================================
BOOL CFriendly::FValidateHintType( short sHint )
{
	int i;

	static short sFriendlyHints[] =
	{
		HINT_WORLD_HUMAN_BLOOD,
	};

	for( i = 0; i < ARRAYSIZE( sFriendlyHints ); i++ )
	{
		if( sFriendlyHints[i] == sHint )
		{
			return TRUE;
		}
	}

	ALERT( at_aiconsole, "Couldn't validate hint type" );
	return FALSE;
}

//=========================================================
// ISoundMask - returns a bit mask indicating which types
// of sounds this monster regards. In the base class implementation,
// monsters care about all sounds, but no scents.
//=========================================================
int CFriendly::ISoundMask( void )
{
	return	bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE |
		bits_SOUND_PLAYER;
}

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CFriendly::Classify( void )
{
	return CLASS_ALIEN_PREDATOR;
}

//=========================================================
// IdleSound 
//=========================================================
#define FRIENDLY_ATTN_IDLE	(float)1.5
void CFriendly::IdleSound( void )
{
	switch( RANDOM_LONG( 0, 4 ) )
	{
	case 0:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_idle1.wav", 1, FRIENDLY_ATTN_IDLE );
		break;
	case 1:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_idle2.wav", 1, FRIENDLY_ATTN_IDLE );
		break;
	case 2:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_idle3.wav", 1, FRIENDLY_ATTN_IDLE );
		break;
	case 3:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_idle4.wav", 1, FRIENDLY_ATTN_IDLE );
		break;
	case 4:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_idle5.wav", 1, FRIENDLY_ATTN_IDLE );
		break;
	}
}

//=========================================================
// PainSound 
//=========================================================
void CFriendly::PainSound( void )
{
	int iPitch = RANDOM_LONG( 85, 120 );

	switch( RANDOM_LONG( 0, 3 ) )
	{
	case 0:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "friendly/friendly_pain1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	case 1:	
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "friendly/friendly_pain1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	case 2:	
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "friendly/friendly_pain1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	case 3:	
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "friendly/friendly_pain1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	}
}

//=========================================================
// AlertSound
//=========================================================
void CFriendly::AlertSound( void )
{
	int iPitch = RANDOM_LONG( 140, 160 );

	switch( RANDOM_LONG( 0, 1 ) )
	{
	case 0:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "friendly/friendly_idle1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	case 1:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "friendly/friendly_idle2.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	}
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CFriendly::SetYawSpeed( void )
{
	int ys;

	ys = 0;

	switch ( m_Activity )
	{
	case ACT_WALK:
		ys = 90;
		break;
	case ACT_RUN:
		ys = 90;
		break;
	case ACT_IDLE:
		ys = 90;
		break;
	case ACT_RANGE_ATTACK1:
		ys = 90;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CFriendly::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case BFRIENDLY_AE_SPIT:
			{
				Vector vecSpitOffset;
				Vector vecSpitDir;

				UTIL_MakeVectors( pev->angles );

				// !!!HACKHACK - the spot at which the spit originates (in front of the mouth) was measured in 3ds and hardcoded here.
				// we should be able to read the position of bones at runtime for this info.
				vecSpitOffset = ( gpGlobals->v_right * 8 + gpGlobals->v_forward * 37 + gpGlobals->v_up * 23 );
				vecSpitOffset = ( pev->origin + vecSpitOffset );
				vecSpitDir = ( ( m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs ) - vecSpitOffset ).Normalize();

				vecSpitDir.x += RANDOM_FLOAT( -0.05, 0.05 );
				vecSpitDir.y += RANDOM_FLOAT( -0.05, 0.05 );
				vecSpitDir.z += RANDOM_FLOAT( -0.05, 0 );

				// do stuff for this event.
				AttackSound();

				// spew the spittle temporary ents.
				MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecSpitOffset );
					WRITE_BYTE( TE_SPRITE_SPRAY );
					WRITE_COORD( vecSpitOffset.x );	// pos
					WRITE_COORD( vecSpitOffset.y );	
					WRITE_COORD( vecSpitOffset.z );	
					WRITE_COORD( vecSpitDir.x );	// dir
					WRITE_COORD( vecSpitDir.y );	
					WRITE_COORD( vecSpitDir.z );	
					WRITE_SHORT( iFriendlySpitSprite );	// model
					WRITE_BYTE( 15 );			// count
					WRITE_BYTE( 130 );			// speed
					WRITE_BYTE( 25 );			// noise ( client will divide by 100 )
				MESSAGE_END();

				CFriendlySpit::Shoot( pev, vecSpitOffset, vecSpitDir * 900 );
			}
			break;
		case BFRIENDLY_AE_BITE:
			{
				// SOUND HERE!
				CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.bullsquidDmgBite, DMG_SLASH );

				if( pHurt )
				{
					//pHurt->pev->punchangle.z = -15;
					//pHurt->pev->punchangle.x = -45;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * 100;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 100;
				}
			}
			break;
		case BFRIENDLY_AE_TAILWHIP:
			{
				CBaseEntity *pHurt = CheckTraceHullAttack( 70, gSkillData.bullsquidDmgWhip, DMG_CLUB | DMG_ALWAYSGIB );

				if( pHurt ) 
				{
					pHurt->pev->punchangle.z = -20;
					pHurt->pev->punchangle.x = 20;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 200;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_up * 100;
				}
			}
			break;
		case BFRIENDLY_AE_BLINK:
			{
				// close eye. 
				pev->skin = 1;
			}
			break;
		case BFRIENDLY_AE_HOP:
			{
				float flGravity = g_psv_gravity->value;

				// throw the squid up into the air on this frame.
				if( FBitSet( pev->flags, FL_ONGROUND ) )
				{
					pev->flags -= FL_ONGROUND;
				}

				// jump into air for 0.8 (24/30) seconds
				//pev->velocity.z += ( 0.875 * flGravity ) * 0.5;
				pev->velocity.z += ( 0.625 * flGravity ) * 0.5;
			}
			break;
		case BFRIENDLY_AE_THROW:
			{
				int iPitch;

				// squid throws its prey IF the prey is a client. 
				CBaseEntity *pHurt = CheckTraceHullAttack( 70, 0, 0 );

				if( pHurt )
				{
					// croonchy bite sound
					iPitch = RANDOM_FLOAT( 90, 110 );
					switch( RANDOM_LONG( 0, 1 ) )
					{
					case 0:
						EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "friendly/friendly_attack1.wav", 1, ATTN_NORM, 0, iPitch );
						break;
					case 1:
						EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "friendly/friendly_attack2.wav", 1, ATTN_NORM, 0, iPitch );
						break;
					}

					//pHurt->pev->punchangle.x = RANDOM_LONG( 0, 34 ) - 5;
					//pHurt->pev->punchangle.z = RANDOM_LONG( 0, 49 ) - 25;
					//pHurt->pev->punchangle.y = RANDOM_LONG( 0, 89 ) - 45;
		
					// screeshake transforms the viewmodel as well as the viewangle. No problems with seeing the ends of the viewmodels.
					UTIL_ScreenShake( pHurt->pev->origin, 25.0, 1.5, 0.7, 2 );

					if( pHurt->IsPlayer() )
					{
						UTIL_MakeVectors( pev->angles );
						pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 300 + gpGlobals->v_up * 300;
					}
				}
			}
			break;
		default:
			CBaseMonster::HandleAnimEvent( pEvent );
	}
}

//=========================================================
// Spawn
//=========================================================
void CFriendly::Spawn()
{
	Precache();

	SET_MODEL( ENT( pev ), "models/friendly.mdl" );
	UTIL_SetSize( pev, Vector( -32, -32, 0 ), Vector( 32, 32, 64 ) );

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	pev->health = 200;
	m_flFieldOfView = 0.2;
	m_MonsterState = MONSTERSTATE_NONE;

	m_fCanThreatDisplay = TRUE;
	m_flNextSpitTime = gpGlobals->time;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CFriendly::Precache()
{
	PRECACHE_MODEL( "models/friendly.mdl" );

	PRECACHE_MODEL( "sprites/bigspit.spr" );// spit projectile.

	iFriendlySpitSprite = PRECACHE_MODEL( "sprites/tinyspit.spr" );// client side spittle.

	PRECACHE_SOUND( "zombie/claw_miss2.wav" );// because we use the basemonster SWIPE animation event

	PRECACHE_SOUND( "friendly/bc_attack2.wav" );

	PRECACHE_SOUND( "friendly/bc_die1.wav" );
	PRECACHE_SOUND( "friendly/bc_die2.wav" );
	PRECACHE_SOUND( "friendly/bc_die3.wav" );

	PRECACHE_SOUND( "friendly/friendly_idle1.wav" );
	PRECACHE_SOUND( "friendly/friendly_idle2.wav" );
	PRECACHE_SOUND( "friendly/friendly_idle3.wav" );
	PRECACHE_SOUND( "friendly/friendly_idle4.wav" );

	PRECACHE_SOUND( "friendly/friendly_pain1.wav" );
	PRECACHE_SOUND( "friendly/friendly_pain1.wav" );
	PRECACHE_SOUND( "friendly/friendly_pain1.wav" );
	PRECACHE_SOUND( "friendly/friendly_pain1.wav" );
	
	PRECACHE_SOUND( "friendly/friendly_attack1.wav" );
	PRECACHE_SOUND( "friendly/friendly_attack2.wav" );
	PRECACHE_SOUND( "friendly/friendly_attack1.wav" );


	PRECACHE_SOUND( "bullchicken/bc_acid1.wav" );

	PRECACHE_SOUND( "bullchicken/friendly_attack1.wav" );

	PRECACHE_SOUND( "bullchicken/bc_spithit1.wav" );
}

//=========================================================
// DeathSound
//=========================================================
void CFriendly::DeathSound( void )
{
	switch( RANDOM_LONG( 0, 2 ) )
	{
	case 0:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_die1.wav", 1, ATTN_NORM );
		break;
	case 1:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_die1.wav", 1, ATTN_NORM );
		break;
	case 2:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_die1.wav", 1, ATTN_NORM );
		break;
	}
}

//=========================================================
// AttackSound
//=========================================================
void CFriendly::AttackSound( void )
{
	switch( RANDOM_LONG( 0, 1 ) )
	{
	case 0:
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "friendly/friendly_attack1.wav", 1, ATTN_NORM );
		break;
	case 1:
		EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "friendly/friendly_attack2.wav", 1, ATTN_NORM );
		break;
	}
}

//========================================================
// RunAI - overridden for bullsquid because there are things
// that need to be checked every think.
//========================================================
void CFriendly::RunAI( void )
{
	// first, do base class stuff
	CBaseMonster::RunAI();

	if( pev->skin != 0 )
	{
		// close eye if it was open.
		pev->skin = 0; 
	}

	if( RANDOM_LONG( 0, 39 ) == 0 )
	{
		pev->skin = 1;
	}

	if( m_hEnemy != NULL && m_Activity == ACT_RUN )
	{
		// chasing enemy. Sprint for last bit
		if( ( pev->origin - m_hEnemy->pev->origin).Length2D() < FRIENDLY_SPRINT_DIST )
		{
			pev->framerate = 1.25;
		}
	}
}

//========================================================
// AI Schedules Specific to this monster
//=========================================================

// primary range attack
Task_t tlFriendlyRangeAttack1[] =
{
	{ TASK_STOP_MOVING, 0 },
	{ TASK_FACE_IDEAL, (float)0 },
	{ TASK_RANGE_ATTACK1, (float)0 },
	{ TASK_SET_ACTIVITY, (float)ACT_IDLE },
};
//Testbag
Schedule_t slFriendlyRangeAttack1[] =
{
	{
		tlFriendlyRangeAttack1,
		ARRAYSIZE( tlFriendlyRangeAttack1 ),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
		0,
		"Friendly Range Attack1"
	},
};

// Chase enemy schedule
Task_t tlFriendlyChaseEnemy1[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_RANGE_ATTACK1 },// !!!OEM - this will stop nasty squid oscillation.
	{ TASK_GET_PATH_TO_ENEMY, (float)0 },
	{ TASK_RUN_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
};

Schedule_t slFriendlyChaseEnemy[] =
{
	{
		tlFriendlyChaseEnemy1,
		ARRAYSIZE( tlFriendlyChaseEnemy1 ),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_SMELL_FOOD |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_TASK_FAILED |
		bits_COND_HEAR_SOUND,
		bits_SOUND_DANGER |
		bits_SOUND_MEAT,
		"Friendly Chase Enemy"
	},
};

Task_t tlFriendlyHurtHop[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_SOUND_WAKE, (float)0 },
	{ TASK_FRIENDLY_HOPTURN, (float)0 },
	{ TASK_FACE_ENEMY, (float)0 },// in case squid didn't turn all the way in the air.
};

Schedule_t slFriendlyHurtHop[] =
{
	{
		tlFriendlyHurtHop,
		ARRAYSIZE( tlFriendlyHurtHop ),
		0,
		0,
		"FriendlyHurtHop"
	}
};

Task_t tlFriendlySeeCrab[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_SOUND_WAKE, (float)0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_EXCITED },
	{ TASK_FACE_ENEMY, (float)0 },
};

Schedule_t slFriendlySeeCrab[] =
{
	{
		tlFriendlySeeCrab,
		ARRAYSIZE( tlFriendlySeeCrab ),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"FriendlySeeCrab"
	}
};

// squid walks to something tasty and eats it.
Task_t tlFriendlyEat[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_EAT, (float)10 },// this is in case the squid can't get to the food
	{ TASK_STORE_LASTPOSITION, (float)0 },
	{ TASK_GET_PATH_TO_BESTSCENT, (float)0 },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_EAT },
	{ TASK_PLAY_SEQUENCE, (float)ACT_EAT },
	{ TASK_PLAY_SEQUENCE, (float)ACT_EAT },
	{ TASK_EAT, (float)50 },
	{ TASK_GET_PATH_TO_LASTPOSITION, (float)0 },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_CLEAR_LASTPOSITION, (float)0 },
};

Schedule_t slFriendlyEat[] =
{
	{
		tlFriendlyEat,
		ARRAYSIZE( tlFriendlyEat ),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_NEW_ENEMY,
		// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
		// here or the monster won't detect these sounds at ALL while running this schedule.
		bits_SOUND_MEAT |
		bits_SOUND_CARCASS,
		"FriendlyEat"
	}
};

// this is a bit different than just Eat. We use this schedule when the food is far away, occluded, or behind
// the squid. This schedule plays a sniff animation before going to the source of food.
Task_t tlFriendlySniffAndEat[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_EAT, (float)10 },// this is in case the squid can't get to the food
	{ TASK_PLAY_SEQUENCE, (float)ACT_DETECT_SCENT },
	{ TASK_STORE_LASTPOSITION, (float)0 },
	{ TASK_GET_PATH_TO_BESTSCENT, (float)0 },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_EAT },
	{ TASK_PLAY_SEQUENCE, (float)ACT_EAT },
	{ TASK_PLAY_SEQUENCE, (float)ACT_EAT },
	{ TASK_EAT, (float)50 },
	{ TASK_GET_PATH_TO_LASTPOSITION, (float)0 },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_CLEAR_LASTPOSITION, (float)0 },
};

Schedule_t slFriendlySniffAndEat[] =
{
	{
		tlFriendlySniffAndEat,
		ARRAYSIZE( tlFriendlySniffAndEat ),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_NEW_ENEMY,
		// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
		// here or the monster won't detect these sounds at ALL while running this schedule.
		bits_SOUND_MEAT |
		bits_SOUND_CARCASS,
		"FriendlySniffAndEat"
	}
};

// squid does this to stinky things. 
Task_t tlFriendlyWallow[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_EAT, (float)10 },// this is in case the squid can't get to the stinkiness
	{ TASK_STORE_LASTPOSITION, (float)0 },
	{ TASK_GET_PATH_TO_BESTSCENT, (float)0 },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_INSPECT_FLOOR },
	{ TASK_EAT, (float)50 },// keeps squid from eating or sniffing anything else for a while.
	{ TASK_GET_PATH_TO_LASTPOSITION, (float)0 },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_CLEAR_LASTPOSITION, (float)0 },
};

Schedule_t slFriendlyWallow[] =
{
	{
		tlFriendlyWallow,
		ARRAYSIZE( tlFriendlyWallow ),
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_NEW_ENEMY,
		// even though HEAR_SOUND/SMELL FOOD doesn't break this schedule, we need this mask
		// here or the monster won't detect these sounds at ALL while running this schedule.
		bits_SOUND_GARBAGE,
		"FriendlyWallow"
	}
};

DEFINE_CUSTOM_SCHEDULES( CFriendly ) 
{
	slFriendlyRangeAttack1,
	slFriendlyChaseEnemy,
	slFriendlyHurtHop,
	slFriendlySeeCrab,
	slFriendlyEat,
	slFriendlySniffAndEat,
	slFriendlyWallow
};

IMPLEMENT_CUSTOM_SCHEDULES( CFriendly, CBaseMonster )

//=========================================================
// GetSchedule 
//=========================================================
Schedule_t *CFriendly::GetSchedule( void )
{
	switch( m_MonsterState )
	{
	case MONSTERSTATE_ALERT:
		{
			if( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
			{
				return GetScheduleOfType( SCHED_FRIENDLY_HURTHOP );
			}

			if( HasConditions( bits_COND_SMELL_FOOD ) )
			{
				CSound *pSound;

				pSound = PBestScent();
				
				if( pSound && ( !FInViewCone( &pSound->m_vecOrigin ) || !FVisible( pSound->m_vecOrigin ) ) )
				{
					// scent is behind or occluded
					return GetScheduleOfType( SCHED_FRIENDLY_SNIFF_AND_EAT );
				}

				// food is right out in the open. Just go get it.
				return GetScheduleOfType( SCHED_FRIENDLY_EAT );
			}

			if( HasConditions( bits_COND_SMELL ) )
			{
				// there's something stinky. 
				CSound *pSound;

				pSound = PBestScent();
				if( pSound )
					return GetScheduleOfType( SCHED_FRIENDLY_WALLOW );
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		{
			// dead enemy
			if( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster::GetSchedule();
			}

			if( HasConditions( bits_COND_NEW_ENEMY ) )
			{
				if( m_fCanThreatDisplay && IRelationship( m_hEnemy ) == R_HT )
				{
					// this means squid sees a headcrab!
					m_fCanThreatDisplay = FALSE;// only do the headcrab dance once per lifetime.
					return GetScheduleOfType( SCHED_FRIENDLY_SEECRAB );
				}
				else
				{
					return GetScheduleOfType( SCHED_WAKE_ANGRY );
				}
			}

			if( HasConditions( bits_COND_SMELL_FOOD ) )
			{
				CSound *pSound;

				pSound = PBestScent();

				if( pSound && ( !FInViewCone( &pSound->m_vecOrigin ) || !FVisible( pSound->m_vecOrigin ) ) )
				{
					// scent is behind or occluded
					return GetScheduleOfType( SCHED_FRIENDLY_SNIFF_AND_EAT );
				}

				// food is right out in the open. Just go get it.
				return GetScheduleOfType( SCHED_FRIENDLY_EAT );
			}

			if( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}

			if( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
			}

			if( HasConditions( bits_COND_CAN_MELEE_ATTACK2 ) )
			{
				return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
			}

			return GetScheduleOfType( SCHED_CHASE_ENEMY );
			break;
		}
	default:
			break;
	}

	return CBaseMonster::GetSchedule();
}

//=========================================================
// GetScheduleOfType
//=========================================================
Schedule_t *CFriendly::GetScheduleOfType( int Type ) 
{
	switch( Type )
	{
	case SCHED_RANGE_ATTACK1:
		return &slFriendlyRangeAttack1[0];
		break;
	case SCHED_FRIENDLY_HURTHOP:
		return &slFriendlyHurtHop[0];
		break;
	case SCHED_FRIENDLY_SEECRAB:
		return &slFriendlySeeCrab[0];
		break;
	case SCHED_FRIENDLY_EAT:
		return &slFriendlyEat[0];
		break;
	case SCHED_FRIENDLY_SNIFF_AND_EAT:
		return &slFriendlySniffAndEat[0];
		break;
	case SCHED_FRIENDLY_WALLOW:
		return &slFriendlyWallow[0];
		break;
	case SCHED_CHASE_ENEMY:
		return &slFriendlyChaseEnemy[0];
		break;
	}

	return CBaseMonster::GetScheduleOfType( Type );
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.  OVERRIDDEN for bullsquid because it needs to
// know explicitly when the last attempt to chase the enemy
// failed, since that impacts its attack choices.
//=========================================================
void CFriendly::StartTask( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch( pTask->iTask )
	{
	case TASK_MELEE_ATTACK2:
		{
			switch( RANDOM_LONG( 0, 2 ) )
			{
			case 0:
				EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_attack1.wav", 1, ATTN_NORM );
				break;
			case 1:
				EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_attack2.wav", 1, ATTN_NORM );
				break;
			case 2:
				EMIT_SOUND( ENT( pev ), CHAN_VOICE, "friendly/friendly_attack1.wav", 1, ATTN_NORM );
				break;
			}

			CBaseMonster::StartTask( pTask );
			break;
		}
	case TASK_FRIENDLY_HOPTURN:
		{
			SetActivity( ACT_HOP );
			MakeIdealYaw( m_vecEnemyLKP );
			break;
		}
	case TASK_GET_PATH_TO_ENEMY:
		{
			if( BuildRoute( m_hEnemy->pev->origin, bits_MF_TO_ENEMY, m_hEnemy ) )
			{
				m_iTaskStatus = TASKSTATUS_COMPLETE;
			}
			else
			{
				ALERT( at_aiconsole, "GetPathToEnemy failed!!\n" );
				TaskFail();
			}
			break;
		}
	default:
		{
			CBaseMonster::StartTask( pTask );
			break;
		}
	}
}

//=========================================================
// RunTask
//=========================================================
void CFriendly::RunTask( Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_FRIENDLY_HOPTURN:
		{
			MakeIdealYaw( m_vecEnemyLKP );
			ChangeYaw( pev->yaw_speed );

			if( m_fSequenceFinished )
			{
				m_iTaskStatus = TASKSTATUS_COMPLETE;
			}
			break;
		}
	default:
		{
			CBaseMonster::RunTask( pTask );
			break;
		}
	}
}

//=========================================================
// GetIdealState - Overridden for Bullsquid to deal with
// the feature that makes it lose interest in headcrabs for 
// a while if something injures it. 
//=========================================================
MONSTERSTATE CFriendly::GetIdealState( void )
{
	int iConditions;

	iConditions = IScheduleFlags();

	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		/*
		COMBAT goes to ALERT upon death of enemy
		*/
		{
			if( m_hEnemy != NULL && ( iConditions & bits_COND_LIGHT_DAMAGE || iConditions & bits_COND_HEAVY_DAMAGE ) && FClassnameIs( m_hEnemy->pev, "monster_headcrab" ) )
			{
				// if the squid has a headcrab enemy and something hurts it, it's going to forget about the crab for a while.
				m_hEnemy = NULL;
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			break;
		}
	default:
			break;
	}

	m_IdealMonsterState = CBaseMonster::GetIdealState();

	return m_IdealMonsterState;
}
