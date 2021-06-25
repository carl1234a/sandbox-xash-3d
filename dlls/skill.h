/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
//=========================================================
// skill.h - skill level concerns
//=========================================================
#pragma once
#ifndef SKILL_H
#define SKILL_H

struct skilldata_t
{
	int iSkillLevel; // game skill level

	// Monster Health & Damage	
	float agruntHealth;
	float agruntDmgPunch;

	float apacheHealth;

	float barneyHealth;
	

	float bigmommaHealthFactor;		// Multiply each node's health by this
	float bigmommaDmgSlash;			// melee attack damage
	float bigmommaDmgBlast;			// mortar attack damage
	float bigmommaRadiusBlast;		// mortar attack radius

	float bullsquidHealth;
	float bullsquidDmgBite;
	float bullsquidDmgWhip;
	float bullsquidDmgSpit;

	float gargantuaHealth;
	float gargantuaDmgSlash;
	float gargantuaDmgFire;
	float gargantuaDmgStomp;

	float hassassinHealth;

	float headcrabHealth;
	float headcrabDmgBite;

	float hgruntHealth;
	float hgruntDmgKick;
	float hgruntShotgunPellets;
	float hgruntGrenadeSpeed;

	float houndeyeHealth;
	float houndeyeDmgBlast;

	float slaveHealth;
	float slaveDmgClaw;
	float slaveDmgClawrake;
	float slaveDmgZap;

	float ichthyosaurHealth;
	float ichthyosaurDmgShake;

	float leechHealth;
	float leechDmgBite;

	float controllerHealth;
	float controllerDmgZap;
	float controllerSpeedBall;
	float controllerDmgBall;

	float nihilanthHealth;
	float nihilanthZap;

	float scientistHealth;

	float snarkHealth;
	float snarkDmgBite;
	float snarkDmgPop;

	float zombieHealth;
	float zombieDmgOneSlash;
	float zombieDmgBothSlash;

	float turretHealth;
	float miniturretHealth;
	float sentryHealth;
	
	float zbarneyHealth;
	float zbarneyDmgOneSlash;
	float zbarneyDmgBothSlash;

	float zgruntHealth;
	float zgruntDmgOneSlash;
	float zgruntDmgBothSlash;
	
	float strooperHealth;
	float strooperDmgKick;
	float strooperGrenadeSpeed;
	float strooperMaxCharge;
	float strooperRchgSpeed;
	
	float voltigoreHealth;
	float voltigoreDmgPunch;
	float voltigoreDmgBeam;
	
	float gonomeHealth;
	float gonomeDmgOneSlash;
	float gonomeDmgGuts;
	float gonomeDmgOneBite;
	
	float babyVoltigoreHealth;
	float babyVoltigoreDmgPunch;
	
	float snakeHealth;
	float snakeDmgBite;
	
	float hgruntAllyHealth;
	float hgruntAllyDmgKick;
	float hgruntAllyShotgunPellets;
	float hgruntAllyGrenadeSpeed;
	
	float medicAllyHealth;
	float medicAllyDmgKick;
	float medicAllyGrenadeSpeed;
	float medicAllyHeal;

	float torchAllyHealth;
	float torchAllyDmgKick;
	float torchAllyGrenadeSpeed;


	// Player Weapons
	float plrDmgCrowbar;
	float plrDmg9MM;
	float plrDmg357;
	float plrDmgMP5;
	float plrDmgM203Grenade;
	float plrDmgBuckshot;
	float plrDmgCrossbowClient;
	float plrDmgCrossbowMonster;
	float plrDmgRPG;
	float plrDmgFlamethrower;
	float plrDmgGauss;
	float plrDmgEgonNarrow;
	float plrDmgEgonWide;
	float plrDmgHornet;
	float plrDmgHandGrenade;
	float plrDmgSatchel;
	float plrDmgTripmine;
	float plrDmgEagle;
	float plrDmgPWrench;
	float plrDmgSpore;
	float plrDmg762;
	float plrDmg50cal;
	float plrDmgMinigun;
	float plrDmgUSP; //CS USP WEAPON
	float plrDmgAK47; //CS Ak47 WEAPON
	float plrDmgMolotov;
	
	// weapons shared by monsters
	float monDmg9MM;
	float monDmgMP5;
	float monDmg12MM;
	float monDmgHornet;
	float monDmg762;

	// health/suit charge
	float suitchargerCapacity;
	float batteryCapacity;
	float healthchargerCapacity;
	float healthkitCapacity;
	float scientistHeal;

	// monster damage adj
	float monHead;
	float monChest;
	float monStomach;
	float monLeg;
	float monArm;

	// player damage adj
	float plrHead;
	float plrChest;
	float plrStomach;
	float plrLeg;
	float plrArm;
	
	// Monster Health & Damage
	float otisHealth;
};

extern	DLL_GLOBAL	skilldata_t	gSkillData;
float GetSkillCvar( const char *pName );

extern DLL_GLOBAL int		g_iSkillLevel;

#define SKILL_EASY		1
#define SKILL_MEDIUM	2
#define SKILL_HARD		3
#endif // SKILL_H
