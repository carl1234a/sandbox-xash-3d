#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

enum flamethrower_e
{
   FLAMETHROWER_LONGIDLE,
   FLAMETHROWER_IDLE1,
   FLAMETHROWER_RELOAD,
   FLAMETHROWER_DEPLOY,
   FLAMETHROWER_HOLSTER,
   FLAMETHROWER_SHOOT,
};

class CFLAMETHROWER : public CBasePlayerWeapon
{
   public:
   void Spawn( void );
   void Precache( void );
   int iItemSlot( void ) { return 4; }
   int GetItemInfo(ItemInfo *p);
   int AddToPlayer( CBasePlayer *pPlayer );

   void PrimaryAttack( void );
   BOOL Deploy( void );
   void Holster( int skiplocal = 0 );
   void Reload( void );
   void WeaponIdle( void );
   float m_flNextAnimTime;
};
LINK_ENTITY_TO_CLASS( weapon_flamethrower, CFLAMETHROWER );

void CFLAMETHROWER::Spawn( )
{
   pev->classname = MAKE_STRING("weapon_flamethrower");
   Precache( );
   SET_MODEL(ENT(pev), "models/w_flamethrower.mdl");
   m_iId = WEAPON_FLAMETHROWER;

   m_iDefaultAmmo = 100;

   FallInit();
}


void CFLAMETHROWER::Precache( void )
{
   PRECACHE_MODEL("models/v_flamethrower.mdl");
   PRECACHE_MODEL("models/w_flamethrower.mdl");
   PRECACHE_MODEL("models/p_flamethrower.mdl");
   PRECACHE_MODEL("sprites/flamesteam.spr");

   PRECACHE_MODEL("models/w_flamefuel.mdl");
   PRECACHE_SOUND("items/9mmclip1.wav");

   PRECACHE_SOUND("weapons/flamethrower_reload.wav");
   PRECACHE_SOUND("weapons/flame_hitwall.wav");
   PRECACHE_SOUND("items/clipinsert1.wav");
   PRECACHE_SOUND("items/cliprelease1.wav");
   PRECACHE_SOUND ("weapons/flamethrower.wav");

   PRECACHE_SOUND ("weapons/357_cock1.wav");
   m_flNextPrimaryAttack = gpGlobals->time + 0.5;
}

int CFLAMETHROWER::GetItemInfo(ItemInfo *p)
{
   p->pszName = STRING(pev->classname);
   p->pszAmmo1 = "fuel";
   p->iMaxAmmo1 = _FUEL_MAX_CARRY;
   p->pszAmmo2 = NULL;
   p->iMaxAmmo2 = -1;
   p->iMaxClip = 100;
   p->iSlot = 6;
   p->iPosition = 2;
   p->iFlags = 0;
   p->iId = m_iId = WEAPON_FLAMETHROWER;
   p->iWeight = FLAMETHROWER_WEIGHT;

   return 1;
}

int CFLAMETHROWER::AddToPlayer( CBasePlayer *pPlayer )
{
   if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
   {
     MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
     WRITE_BYTE( m_iId );
     MESSAGE_END();
     return TRUE;
   }
   return FALSE;
}

BOOL CFLAMETHROWER::Deploy( )
{
   return DefaultDeploy( "models/v_flamethrower.mdl", "models/p_flamethrower.mdl", FLAMETHROWER_DEPLOY, "mp5" );
}

void CFLAMETHROWER::Holster( int skiplocal )
{
   SendWeaponAnim( FLAMETHROWER_HOLSTER );
}

void CFLAMETHROWER::PrimaryAttack( void )
{
   if (m_pPlayer->pev->waterlevel == 3)
   {
     PlayEmptySound( );
     m_flNextPrimaryAttack = gpGlobals->time + 0.15;
     return;
   }

   if (m_iClip <= 0)
   {
     PlayEmptySound();
     m_flNextPrimaryAttack = 0.15;
     return;
   }

   m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
   m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

   m_iClip--;

   SendWeaponAnim( FLAMETHROWER_SHOOT );

   m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

   EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/flamethrower.wav", 0.8, ATTN_NORM);

   UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

   CFlame::ShootFlame( m_pPlayer->pev,
m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16 + gpGlobals->v_right * 5 + gpGlobals->v_up * -10,
gpGlobals->v_forward * 900 );

   CFlame::ShootFlame( m_pPlayer->pev,
m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16 + gpGlobals->v_right * 10 + gpGlobals->v_up * -15,
gpGlobals->v_forward * 920 );

   CFlame::ShootFlame( m_pPlayer->pev,
m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 24 + gpGlobals->v_right * 15 + gpGlobals->v_up * -20,
gpGlobals->v_forward * 940 );

   CFlame::ShootFlame( m_pPlayer->pev,
m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 24 + gpGlobals->v_right * 5 + gpGlobals->v_up * -10,
gpGlobals->v_forward * 960 );

   CFlame::ShootFlame( m_pPlayer->pev,
m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 32 + gpGlobals->v_right * 10 + gpGlobals->v_up * -15,
gpGlobals->v_forward * 980 );

   CFlame::ShootFlame( m_pPlayer->pev,
m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 32 + gpGlobals->v_right * 15 + gpGlobals->v_up * -20,
gpGlobals->v_forward * 1000 );

   MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
   WRITE_BYTE( TE_DLIGHT );
   WRITE_COORD( pev->origin.x );
   WRITE_COORD( pev->origin.y );
   WRITE_COORD( pev->origin.z );
   WRITE_BYTE( 75 );
   WRITE_BYTE( 128 );
   WRITE_BYTE( 128 );
   WRITE_BYTE( 0 );
   WRITE_BYTE( 5 );
   WRITE_BYTE( 10 );
   MESSAGE_END();

   m_flNextPrimaryAttack = gpGlobals->time + 0.1;
   m_flTimeWeaponIdle = gpGlobals->time + 5;

   m_pPlayer->pev->punchangle.x -= 1;
}

void CFLAMETHROWER::Reload( void )
{
   int iResult;

   if ( m_iClip == 100 )
   {
     m_flNextPrimaryAttack = gpGlobals->time + 0.5;

     return;
   }

   if (m_iClip == 0)
   {
     iResult = DefaultReload( 100, FLAMETHROWER_RELOAD, 4.3 );
     EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/flamethrower_reload.wav", 1, ATTN_NORM);
   }

   if (iResult)
   {
     m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );
   }
}

void CFLAMETHROWER::WeaponIdle( void )
{
   ResetEmptySound( );

   m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

   if (m_flTimeWeaponIdle > gpGlobals->time)
     return;

   int iAnim;
   switch ( RANDOM_LONG( 0, 1 ) )
   {
     case 0:
       iAnim = FLAMETHROWER_IDLE1;
       break;

     default:
     case 1:
       iAnim = FLAMETHROWER_LONGIDLE;
       break;
   }

   SendWeaponAnim( iAnim );

   m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );
}

class CFLAMETHROWERAmmo : public CBasePlayerAmmo
{
   void Spawn( void )
   {
     Precache( );
     SET_MODEL(ENT(pev), "models/w_flamefuel.mdl");
     CBasePlayerAmmo::Spawn( );
   }
   void Precache( void )
   {
     PRECACHE_MODEL ("models/w_flamefuel.mdl");
     PRECACHE_SOUND("items/9mmclip1.wav");
   }
   BOOL AddAmmo( CBaseEntity *pOther )
   {
     int bResult = (pOther->GiveAmmo( 50, "fuel", _FUEL_MAX_CARRY ) != -1);

     if (bResult)
     {
       EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
     }
     return bResult;
   }
};
LINK_ENTITY_TO_CLASS( ammo_fuel, CFLAMETHROWERAmmo );
