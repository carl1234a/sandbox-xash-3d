/***
Created by Solexid
*
****/



#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "monsters.h"
#include "soundent.h"
#include "weapons.h"
#include "nodes.h"
#include "effects.h"
#include "customentity.h"
#include "gamerules.h"
#include "customweapons.h"
#include "unpredictedweapon.h"

#define	GRAV_BEAM_SPRITE_PRIMARY_VOLUME		30
#define GRAV_BEAM_SPRITE		"sprites/xbeam3.spr"
#define GRAV_FLARE_SPRITE		"sprites/hotglow.spr"
#define GRAV_SOUND_OFF			"buttons/latchunlocked1.wav"
#define GRAV_SOUND_RUN			"weapons/mine_activate.wav"
#define GRAV_SOUND_FAILRUN		"houndeye/he_die3.wav"
#define GRAV_SOUND_STARTUP		"weapons/gauss2.wav"
#define EGON_SWITCH_NARROW_TIME			0.75			// Time it takes to switch fire modes

enum gauss_e {
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};


class CGravGun : public CBasePlayerWeaponU
{
public:


	float m_flLastCmd;
	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 4; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer(CBasePlayer *pPlayer);
	int m_iStage;
	BOOL Deploy(void);
	void Holster(int skiplocal = 0);
	int m_iGrabFailures;
	EHANDLE m_hAimentEntity;
	void UpdateEffect(const Vector &startPoint, const Vector &endPoint, float timeBlend);
	CBaseEntity *	TraceForward(CBaseEntity *pMe, float radius);
	void CreateEffect(void);
	void DestroyEffect(void);
	void EndAttack(void);
	void Attack(void);
	void Attack2(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void WeaponIdle(void);
	void Pull(CBaseEntity* ent);
	void GravAnim(int iAnim, int skiplocal, int body);
	Vector PredictTarget( float length );
	CBaseEntity *GetCrossEnt( Vector gunpos, Vector aim, float radius );
	float m_flNextGravgunAttack;
	float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.


	void EXPORT GrabThink( void );
	void EXPORT PullThink( void );
	float Fire(const Vector &vecOrigSrc, const Vector &vecDir);

	BOOL HasAmmo(void);


	enum GRAV_FIREMODE { FIRE_NARROW, FIRE_WIDE };

	CBeam				*m_pBeam;
	CBeam				*m_pNoise;
	CSprite				*m_pSprite;

private:
	float				m_shootTime;
	GRAV_FIREMODE		m_fireMode;
	float				m_shakeTime;
	BOOL				m_deployed;
	float				m_fPushSpeed;
};

LINK_ENTITY_TO_CLASS(weapon_gravgun, CGravGun);

void CGravGun::Spawn()
{
	pev->classname = MAKE_STRING("weapon_gravgun");
	Precache();
	m_iId = WEAPON_GRAVGUN;
	SET_MODEL(ENT(pev), "models/w_gravcannon.mdl");
	m_iClip = -1;
	m_iDefaultAmmo = -1;

	FallInit();// get ready to fall down.
}
#define CONST_MULT 10
#define CMD_MULT 100
Vector CGravGun::PredictTarget(float length)
{
	Vector predicted = m_pPlayer->pev->origin;
	float cmdtime = gpGlobals->time - m_flLastCmd;
	//ALERT( at_notice, "PredictTarget %f\n", cmdtime );

	// button-based prediction
	if( m_pPlayer->pev->button & IN_FORWARD )
		predicted = predicted + gpGlobals->v_forward * CONST_MULT + gpGlobals->v_forward * cmdtime * CMD_MULT;
	// do not predict back? it may hurt player
	//if( m_pPlayer->pev->button & IN_BACK )
		//predicted = predicted + gpGlobals->v_forward * -CONST_MULT / 2 + gpGlobals->v_forward * cmdtime * -CMD_MULT / 2;
	if( m_pPlayer->pev->button & IN_MOVELEFT )
		predicted = predicted + gpGlobals->v_right * -CONST_MULT + gpGlobals->v_right * cmdtime * -CMD_MULT;
	if( m_pPlayer->pev->button & IN_MOVERIGHT )
		predicted = predicted + gpGlobals->v_right * CONST_MULT + gpGlobals->v_right * cmdtime * CMD_MULT;

	// distance
	predicted = predicted + gpGlobals->v_forward * length;

	// velocity animation
	predicted = predicted + m_pPlayer->pev->velocity * cmdtime;
	return predicted;
}

void CGravGun::Precache(void)
{
	PRECACHE_MODEL("models/w_gravcannon.mdl");
	PRECACHE_MODEL("models/v_gravcannon.mdl");
	PRECACHE_MODEL("models/p_gravcannon.mdl");

	PRECACHE_MODEL("models/w_9mmclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");
	
	PRECACHE_SOUND(GRAV_SOUND_OFF);
	PRECACHE_SOUND(GRAV_SOUND_RUN); 
	PRECACHE_SOUND(GRAV_SOUND_FAILRUN);
	PRECACHE_SOUND(GRAV_SOUND_STARTUP);

	PRECACHE_MODEL(GRAV_BEAM_SPRITE);
	PRECACHE_MODEL("sprites/hotglow.spr");

	PRECACHE_SOUND("weapons/357_cock1.wav");
	PRECACHE_GENERIC("sprites/weapon_gravgun.txt");

}


BOOL CGravGun::Deploy(void)
{
	m_deployed = FALSE;
	m_fireState = FIRE_OFF;
	SetThink( NULL );
	return DefaultDeploy("models/v_gravcannon.mdl", "models/p_gravcannon.mdl", GAUSS_DRAW, "gauss");
}

int CGravGun::AddToPlayer(CBasePlayer *pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}



void CGravGun::Holster(int skiplocal /* = 0 */)
{
	
	SetThink(NULL);
	if (m_hAimentEntity) { m_hAimentEntity->pev->velocity = Vector(0, 0, 0); }
	m_hAimentEntity = NULL;
	EndAttack();
	m_iStage = 0;
	m_flNextGravgunAttack = gpGlobals->time + 0.5;
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	GravAnim(GAUSS_HOLSTER,0,0);
	SetThink(NULL);
}

int CGravGun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_GRAVGUN;
	p->iFlags = 0;
	p->iWeight = 20;

	return 1;
}

BOOL CGravGun::HasAmmo(void)
{
	
	return TRUE;
}



void CGravGun::Attack(void)
{
	pev->nextthink = gpGlobals->time + 1.1;
	m_flNextGravgunAttack - gpGlobals->time + 0.5;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecAiming = gpGlobals->v_forward;
	Vector vecSrc = m_pPlayer->GetGunPosition();

	switch (m_fireState)
	{
	case FIRE_OFF:
	{
		m_fireMode = FIRE_WIDE;


		GravAnim(GAUSS_FIRE2, 1, 0);
		m_pPlayer->m_iWeaponVolume = 20;
		m_flTimeWeaponIdle = gpGlobals->time + 0.04;
		pev->fuser1 = gpGlobals->time + 0.1;
		
		m_iStage = 0;
		m_fireState = FIRE_CHARGE;
	}
	break;

	case FIRE_CHARGE:
	{
		float dist = Fire(vecSrc, vecAiming);
		m_pPlayer->m_iWeaponVolume = 20;

		if (pev->fuser1 <= gpGlobals->time)
		{
			GravAnim(GAUSS_SPIN, 1, 0);
			pev->fuser1 = 1000;
			SetThink(NULL);
		}
		

		//CBaseEntity* crosent = TraceForward(m_pPlayer, 1000);
		CBaseEntity* crossent = m_hAimentEntity;
		if( m_hAimentEntity )
			m_hAimentEntity->m_fireState = 0;
		m_hAimentEntity = NULL;
		if( !crossent )
			crossent = GetCrossEnt(vecSrc, gpGlobals->v_forward, dist + 30);
		if( !crossent || !( m_fPushSpeed = crossent->TouchGravGun(m_pPlayer,3) ) )
			crossent = TraceForward(m_pPlayer, 1000);
		if(crossent && (m_fPushSpeed = crossent->TouchGravGun(m_pPlayer,3) ) )
		{
			m_flNextGravgunAttack = gpGlobals->time + 0.8;
			DestroyEffect();
			m_fireMode = FIRE_WIDE;
			Vector origin = crossent->pev->origin;
			if(crossent->IsBSPModel())
				origin = VecBModelOrigin( crossent->pev );
			UpdateEffect( vecSrc, origin, 1 );


			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, GRAV_SOUND_STARTUP, 0.1, ATTN_NORM, 0, 70 + RANDOM_LONG(0, 34));

			//if (crosent->pev->flags& FL_ONGROUND) { pev->velocity = pev->velocity * 0.95; };

			//crossent->TouchGravGun(m_pPlayer,3);
			Vector pusher = vecAiming;
			pusher.x = pusher.x * m_fPushSpeed;
			pusher.y = pusher.y * m_fPushSpeed;
			pusher.z = pusher.z * m_fPushSpeed * 0.7;
			crossent->pev->velocity = pusher+m_pPlayer->pev->velocity;
		}
		else
		{
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "gravgun/dry.wav", 0.6, ATTN_NORM);
			crossent = NULL;
		}
		if (gpGlobals->time >= m_flNextGravgunAttack)
		{
			m_flNextGravgunAttack = gpGlobals->time + 0.7;
			EndAttack();
		}

	}
	
	m_flNextGravgunAttack = gpGlobals->time + 0.5;
	pev->nextthink = gpGlobals->time + 0.2;
	SetThink( &CGravGun::DestroyEffect );
	
	break;
	}

}
void CGravGun::GravAnim(int iAnim, int skiplocal, int body)
{

	m_pPlayer->pev->weaponanim = iAnim;



	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev);
	WRITE_BYTE(iAnim); // sequence number
	WRITE_BYTE(pev->body); // weaponmodel bodygroup.
	MESSAGE_END();
}

void CGravGun::Attack2(void)
{
	//if (temp) { temp = NULL; }
	//if(temp) return;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecAiming = gpGlobals->v_forward;
	//Vector vecAiming = UTIL_GetAimVector(m_pPlayer->edict(), 1000);
	Vector vecSrc = m_pPlayer->GetGunPosition();

	switch (m_fireState)
	{
		case FIRE_OFF:
		{
			GravAnim(GAUSS_FIRE, 1, 0);
			m_pPlayer->m_iWeaponVolume = 20;

			m_fireState = FIRE_CHARGE;
			m_fireMode = FIRE_WIDE;

		}
		break;

		case FIRE_CHARGE:
		{
			float dist = Fire(vecSrc, vecAiming) + 30;
			m_flNextGravgunAttack = gpGlobals->time + 0.1;
			//ALERT( at_console, "dist: %f\n", dist );
			m_pPlayer->m_iWeaponVolume = 100;

			if (pev->fuser1 <= gpGlobals->time)
			{

				pev->fuser1 = 1000;
			}
			//CBaseEntity* crossent = TraceForward(m_pPlayer,500);
			CBaseEntity* crossent = GetCrossEnt(vecSrc, gpGlobals->v_forward, dist );
			if( !crossent || !(m_fPushSpeed = crossent->TouchGravGun(m_pPlayer,0)) )
			{
				crossent = TraceForward(m_pPlayer, 1000);
				if( !crossent )
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, "gravgun/dry.wav", 0.3, ATTN_NORM);
				else if( !(m_fPushSpeed = crossent->TouchGravGun(m_pPlayer,0)) )
				{
					EMIT_SOUND(ENT(pev), CHAN_WEAPON, GRAV_SOUND_FAILRUN, 0.3, ATTN_NORM);
					crossent = NULL;
				}
			}
			if ( crossent ){
				if(m_fireMode != FIRE_NARROW)
					DestroyEffect();
				m_fireMode = FIRE_NARROW;
				Vector origin = crossent->pev->origin;
				if(crossent->IsBSPModel())
					origin = VecBModelOrigin( crossent->pev );
				UpdateEffect( vecSrc, origin, 1 );
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "gravgun/pickup.wav", 0.6, ATTN_NORM, 0, 70 + RANDOM_LONG(0, 34));
				if(crossent->TouchGravGun(m_pPlayer, 0))
				{
					m_hAimentEntity = crossent;
					GravAnim(GAUSS_SPIN, 0, 0);
					SetThink( &CGravGun::PullThink );
					pev->nextthink = gpGlobals->time + 0.001;
				}

			}
			else
			{
				if( m_fireMode == FIRE_NARROW )
					DestroyEffect();

				m_fireMode = FIRE_WIDE;
			}
		}
		break;
	}

}
CBaseEntity *CGravGun::GetCrossEnt( Vector gunpos, Vector aim, float radius )
{
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	edict_t		*pClosest = NULL;
	Vector		vecLOS;
	float flMaxDot = 0.9;
	float flDot;

	if ( !pEdict )
		return NULL;

	edict_t *player = m_pPlayer->edict();

	// uncomment this for profiling
	// int tracecount = 0;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;

		if( pEdict->v.solid == SOLID_BSP || pEdict->v.movetype == MOVETYPE_PUSHSTEP )
			continue; //bsp models will be found by trace later

		Vector origin = VecBModelOrigin(&pEdict->v);

		vecLOS = origin - gunpos;

		// too far, ignore it now
		if( vecLOS.Length() > radius )
			continue;

		// ignore player
		if( pEdict == player )
			continue;

		//ALERT( at_console, "len: %f\n", vecLOS.Length() );
		vecLOS = UTIL_ClampVectorToBox(vecLOS, pEdict->v.size * 0.5);

		flDot = DotProduct(vecLOS, aim);
		if (flDot <= flMaxDot)
			continue;
		//tracecount++;
		TraceResult tr;
		UTIL_TraceLine(gunpos, origin, missile, player, &tr);
		if( ( tr.vecEndPos - gunpos ).Length() + 30 < (origin - gunpos).Length())
			continue;
		pClosest = pEdict;
		flMaxDot = flDot;
	}

	//ALERT( at_console, "tracecount: %d\n", tracecount );

	return CBaseEntity::Instance(pClosest);

}

CBaseEntity*  CGravGun::TraceForward(CBaseEntity *pMe,float radius)
{
	TraceResult tr;
	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs, pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * radius, missile, pMe->edict(), &tr);
	if( tr.flFraction != 1.0 && !FNullEnt(tr.pHit) )
		return CBaseEntity::Instance(tr.pHit);
	return NULL;
}

//Used for prop grab and 
void CGravGun::GrabThink()
{
	if (( m_iGrabFailures < 50 )&& m_hAimentEntity )
	{
			Vector origin = m_hAimentEntity->pev->origin;
			if( m_hAimentEntity->IsBSPModel() )
				origin = VecBModelOrigin(m_hAimentEntity->pev );
			if( ( origin - m_pPlayer->pev->origin).Length() > 150 )
				m_iGrabFailures++;
			else
				m_iGrabFailures = 0;

			UpdateEffect(pev->origin, origin, 1);

			Pull(m_hAimentEntity);
			pev->nextthink = gpGlobals->time + 0.001;
	}
	else{
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, GRAV_SOUND_OFF, 1, ATTN_NORM, 0, 70 + RANDOM_LONG(0, 34));
		m_iGrabFailures = 0;
		SetThink( NULL );

		if(m_hAimentEntity)
		{
			m_hAimentEntity->m_fireState = 0;
			m_hAimentEntity->pev->velocity = Vector(0,0,0);
			m_hAimentEntity = NULL;
		}
		EndAttack();
		m_iStage = 0;
	}
}

void CGravGun::PullThink()
{
	if( m_hAimentEntity )
	{
		Pull( m_hAimentEntity );
		pev->nextthink = gpGlobals->time + 0.05;
	}
	else SetThink( NULL );
}

void CGravGun::Pull(CBaseEntity* ent)
{
	Vector origin = ent->pev->origin;
	if( ent->IsBSPModel())
		origin = VecBModelOrigin(ent->pev);
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector target = PredictTarget( 75 );
	target.z += 32;
	if ((target - origin).Length() > 60){
		target = PredictTarget(110);
		Vector diff = target - origin;

		target.z += 60;
		

		//ALERT(at_console, "%s 1 %d : %f\n", STRING(ent->pev->classname), m_iStage, ((target - VecBModelOrigin(ent->pev)).Length()));

	
		if( !m_iStage )
		{
			ent->pev->velocity = diff.Normalize()*(55000.0* 1.0/diff.Length());
			if(ent->pev->velocity.Length() > 500 )
				ent->pev->velocity = diff.Normalize() * 500;

			ent->pev->velocity.z += 20;
			if( diff.Length() < 150 )
			{
				m_iStage = 1;

				SetThink( &CGravGun::GrabThink );
				pev->nextthink = gpGlobals->time + 0.001;
				ent->TouchGravGun(m_pPlayer, 1);
			}
		}
		else
		{
			ent->pev->velocity = diff.Normalize()*(55000.0* 1.0/diff.Length());
			if(ent->pev->velocity.Length() > 900 )
				ent->pev->velocity = diff.Normalize() * 900;
			ent->pev->velocity.z += 15;
			//ALERT( at_notice, "vel %f\n", ent->pev->velocity.Length() );
		}
		//ent->pev->velocity = ent->pev->velocity + m_pPlayer->pev->velocity;
			/////
#ifdef BEAMS
		CBeam* m_pBeam1 = CBeam::BeamCreate(GRAV_BEAM_SPRITE, 40);
		m_pBeam1->SetFlags(BEAM_FSHADEOUT);
		m_pBeam1->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
		m_pBeam1->pev->flags |= FL_SKIPLOCALHOST;
		m_pBeam1->pev->owner = m_pPlayer->edict();
		m_pBeam1->SetStartPos(target);
		m_pBeam1->SetEndPos(VecBModelOrigin(ent->pev));
		m_pBeam1->SetWidth(40 - (1 * 20));
		m_pBeam1->SetBrightness(130);
#endif // BEAMS

		
		/////
		//ALERT(at_console, "%s 2: %f\n", STRING(ent->pev->classname), m_iStage, ent->pev->velocity.Length());
	}
	else if( ent->TouchGravGun(m_pPlayer, 2) )
	{	
		ent->pev->velocity = (target - origin)* 80;
		ent->m_fireState = ENTINDEX( m_pPlayer->edict() );
		if(ent->pev->velocity.Length()>900)
			ent->pev->velocity = (target - origin).Normalize() * 900;
		ent->pev->velocity = ent->pev->velocity + m_pPlayer->pev->velocity;
		m_iStage = 2;
		SetThink( &CGravGun::GrabThink );
		pev->nextthink = gpGlobals->time + 0.001;
	}
	else
	{
		SetThink(NULL);
		if( m_hAimentEntity )
		{
			m_hAimentEntity->m_fireState = 0;
			m_hAimentEntity = NULL;
		}
		EndAttack();
		m_iStage = 0;
	}
}





void CGravGun::PrimaryAttack(void)
{
	m_flLastCmd = gpGlobals->time;
	if (m_flNextGravgunAttack < gpGlobals->time)
	{
	
	
	SetThink(NULL);
	Attack();

	}

}


void CGravGun::SecondaryAttack(void)
{
	m_flLastCmd = gpGlobals->time;
	if (m_flNextGravgunAttack < gpGlobals->time)
	{
		if (m_iStage)
		{
			if( m_fireState != FIRE_OFF )
			{
				return;
			}
			//m_fireMode = FIRE_WIDE;
			EndAttack();
			SetThink(NULL);
			m_flNextGravgunAttack = gpGlobals->time + 0.6;
			//m_flTimeWeaponIdle = gpGlobals->time + 0.1;

			m_iStage = 0;
			if( m_hAimentEntity )
			{
				m_hAimentEntity->pev->velocity = Vector(0,0,0);
				m_hAimentEntity->m_fireState = 0;
				EMIT_SOUND( ENT( m_hAimentEntity->pev ), CHAN_VOICE, "weapons/357_cock1.wav", 0.8, ATTN_NORM );
				m_hAimentEntity = NULL;
			}
		}
		else {
			Attack2();
		}

	}
}
float CGravGun::Fire(const Vector &vecOrigSrc, const Vector &vecDir)
{
	Vector vecDest = vecOrigSrc + vecDir * 2048;
	edict_t		*pentIgnore;
	TraceResult tr;

	pentIgnore = m_pPlayer->edict();
	Vector tmpSrc = vecOrigSrc + gpGlobals->v_up * -8 + gpGlobals->v_right * 3;
	UTIL_TraceLine(vecOrigSrc, vecDest, dont_ignore_monsters, pentIgnore, &tr);

	if (tr.fAllSolid)
		return (tr.vecEndPos - tmpSrc).Length();

	UpdateEffect(tmpSrc, tr.vecEndPos, 1);
	return (tr.vecEndPos - tmpSrc).Length();
}


void CGravGun::UpdateEffect(const Vector &startPoint, const Vector &endPoint, float timeBlend)
{
#ifndef CLIENT_DLL
	if (!m_pBeam)
	{
		CreateEffect();
	}

	m_pBeam->SetStartPos(endPoint);
	m_pBeam->SetBrightness(255 - (timeBlend * 180));
	m_pBeam->SetWidth(40 - (timeBlend * 20));

	if (m_fireMode == FIRE_WIDE)
		m_pBeam->SetColor(100 + (25 * timeBlend),  104 + 80 * fabs(sin(gpGlobals->time * 10)),10 );
	else
		m_pBeam->SetColor(90 + (25 * timeBlend), 100 + 80 * fabs(sin(gpGlobals->time * 10)), 10+(30 * timeBlend));
	Vector& lel=pev->origin;
	lel[0] = 30;
	lel[1] = 30;
	lel[2] = 30;

	//Vector& ar= m_pPlayer->pev-> origin+ m_pPlayer->pev->view_ofs;
	//Vector& al = pev->angles;

	
//	UTIL_SetOrigin(m_pSprite->pev, ar);
//	m_pSprite->pev->frame += 8 * gpGlobals->frametime;
	//if (m_pSprite->pev->frame > m_pSprite->Frames())
	//	m_pSprite->pev->frame = 0;

	m_pNoise->SetStartPos(endPoint);

#endif

}

void CGravGun::CreateEffect(void)
{

#ifndef CLIENT_DLL
	DestroyEffect();
	
	m_pBeam = CBeam::BeamCreate(GRAV_BEAM_SPRITE, 40);
	m_pBeam->PointEntInit(pev->origin, m_pPlayer->entindex());
	m_pBeam->SetFlags(BEAM_FSINE);
	m_pBeam->SetEndAttachment(1);
	m_pBeam->pev->spawnflags |= SF_BEAM_TEMPORARY;	// Flag these to be destroyed on save/restore or level transition
	//m_pBeam->pev->flags |= FL_SKIPLOCALHOST;
	m_pBeam->pev->owner = m_pPlayer->edict();

	m_pNoise = CBeam::BeamCreate(GRAV_BEAM_SPRITE, 55);
	m_pNoise->PointEntInit(pev->origin, m_pPlayer->entindex());
	m_pNoise->SetScrollRate(3);
	m_pNoise->SetBrightness(100);
	m_pNoise->SetEndAttachment(1);
	m_pNoise->pev->spawnflags |= SF_BEAM_TEMPORARY;
	//m_pNoise->pev->flags |= FL_SKIPLOCALHOST;
	m_pNoise->pev->owner = m_pPlayer->edict();

	/*m_pSprite = CSprite::SpriteCreate(GRAV_FLARE_SPRITE, m_pPlayer->GetGunPosition(), TRUE);
	m_pSprite->pev->scale = 1.0;

	m_pSprite->SetTransparency(kRenderGlow, 255, 140, 0, 255, kRenderFxPulseFast);
	m_pSprite->pev->spawnflags |= SF_SPRITE_TEMPORARY;
	m_pSprite->pev->flags |= FL_SKIPLOCALHOST;
	m_pSprite->pev->owner = m_pPlayer->edict();*/

	if (m_fireMode == FIRE_WIDE)
	{
		m_pBeam->SetScrollRate(300);
		m_pBeam->SetNoise(20);
		m_pNoise->SetColor(200, 120, 30);
		m_pNoise->SetNoise(8);
	}
	else
	{
		m_pBeam->SetScrollRate(200);
		m_pBeam->SetNoise(5);
		m_pNoise->SetColor(0, 255, 0);
		m_pNoise->SetNoise(2);
		EMIT_SOUND_DYN(ENT(m_pBeam->pev), CHAN_VOICE, "gravgun/hold.wav", 0.2, ATTN_NORM, 0, 70 + RANDOM_LONG(0, 34));
	}
#endif

}


void CGravGun::DestroyEffect(void)
{

#ifndef CLIENT_DLL
	if (m_pBeam)
	{
		if( m_fireMode == FIRE_NARROW )
			EMIT_SOUND( ENT(m_pBeam->pev), CHAN_VOICE, "gravgun/drop.wav", 0.2, ATTN_NORM );
		UTIL_Remove(m_pBeam);
		m_pBeam = NULL;
	}
	if (m_pNoise)
	{
		UTIL_Remove(m_pNoise);
		m_pNoise = NULL;
	}
	if (m_pSprite)
	{
		if (m_fireMode == FIRE_WIDE)
			m_pSprite->Expand(10, 500);
		else
			UTIL_Remove(m_pSprite);
		m_pSprite = NULL;
	}
#endif

}



void CGravGun::WeaponIdle(void)
{
	m_flLastCmd = gpGlobals->time;
	ResetEmptySound();

	if (m_flTimeWeaponIdle > gpGlobals->time)
		return;

	if (m_fireState != FIRE_OFF)
		EndAttack();

	GravAnim(GAUSS_IDLE, 0, 0);
	m_deployed = TRUE;
}



void CGravGun::EndAttack(void)
{
	bool bMakeNoise = false;
   // if (m_AimentEntity&&m_AimentEntity->pev->velocity.Length() > 100&& (m_AimentEntity->pev->origin-m_pPlayer->pev->origin).Length()<100) { m_AimentEntity->pev->velocity = m_AimentEntity->pev->velocity / 10; }
	//ALERT( at_console, "EndAttack()\n");
	if (m_fireState != FIRE_OFF) //Checking the button just in case!.
		bMakeNoise = true;
	m_flNextGravgunAttack = gpGlobals->time + 0.1;
	m_flTimeWeaponIdle = gpGlobals->time + 0.2;

	m_fireState = FIRE_OFF;

	DestroyEffect();
	if( m_pfnThink == &CGravGun::PullThink )
		SetThink( NULL );
}




