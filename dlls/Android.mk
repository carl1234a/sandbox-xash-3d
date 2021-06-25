#HLSDK server Android port
#Copyright (c) nicknekit

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := server

include $(XASH3D_CONFIG)

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a-hard)
LOCAL_MODULE_FILENAME = libserver_hardfp
endif

LOCAL_CFLAGS += -D_LINUX -DCLIENT_WEAPONS -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -D_snprintf=snprintf \
	-fno-exceptions -DNO_VOICEGAMEMGR -Wno-conversion-null -Wno-write-strings -std=gnu++98 -Wno-invalid-offsetof


LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)

LOCAL_C_INCLUDES := $(SDL_PATH)/include \
		    $(LOCAL_PATH)/. \
		    $(LOCAL_PATH)/wpn_shared \
		    $(LOCAL_PATH)/../common \
		    $(LOCAL_PATH)/../engine/common \
		    $(LOCAL_PATH)/../engine \
		    $(LOCAL_PATH)/../public \
		    $(LOCAL_PATH)/../pm_shared \
		    $(LOCAL_PATH)/../game_shared

LOCAL_SRC_FILES := agrunt.cpp airtank.cpp \
           aflock.cpp \
           animating.cpp \
           animation.cpp \
           apache.cpp \
           barnacle.cpp \
           barney.cpp \
           bigmomma.cpp \
           bloater.cpp \
           bmodels.cpp \
           bullsquid.cpp \
           buttons.cpp \
           cbase.cpp \
           client.cpp \
           combat.cpp \
           controller.cpp \
           crossbow.cpp \
           crowbar.cpp \
           defaultai.cpp \
           doors.cpp \
           effects.cpp \
           egon.cpp \
           explode.cpp \
		   enttools.cpp \
           flyingmonster.cpp \
           func_break.cpp \
           func_tank.cpp \
           game.cpp \
           gamerules.cpp \
           gargantua.cpp \
           gauss.cpp \
           genericmonster.cpp \
           ggrenade.cpp \
           globals.cpp \
           glock.cpp \
           gman.cpp \
           h_ai.cpp \
           h_battery.cpp \
           h_cycler.cpp \
           h_cine.cpp \
           h_export.cpp \
           handgrenade.cpp \
           hassassin.cpp \
           headcrab.cpp \
           healthkit.cpp \
           hgrunt.cpp \
           hornet.cpp \
           hornetgun.cpp \
           houndeye.cpp \
           ichthyosaur.cpp \
           islave.cpp \
           items.cpp \
           leech.cpp \
           lights.cpp \
           maprules.cpp \
           monstermaker.cpp \
           monsters.cpp \
           monsterstate.cpp \
           mortar.cpp \
           mp5.cpp \
           multiplay_gamerules.cpp \
           nihilanth.cpp \
           nodes.cpp \
	   observer.cpp \
           osprey.cpp \
           pathcorner.cpp \
           plane.cpp \
           plats.cpp \
           player.cpp \
	   playermonster.cpp \
           python.cpp \
           rat.cpp \
           roach.cpp \
           rpg.cpp \
	   satchel.cpp \
           schedule.cpp \
           scientist.cpp \
           scripted.cpp \
           shotgun.cpp \
           singleplay_gamerules.cpp \
           skill.cpp \
           sound.cpp \
           soundent.cpp \
           spectator.cpp \
           squadmonster.cpp \
           squeakgrenade.cpp \
           subs.cpp \
           talkmonster.cpp \
           teamplay_gamerules.cpp \
           tentacle.cpp \
           tempmonster.cpp \
           triggers.cpp \
           tripmine.cpp \
           turret.cpp \
           util.cpp \
           weapons.cpp \
           world.cpp \
           xen.cpp \
           zombie.cpp \
           prop.cpp \
           gravgun.cpp \
           ar2.cpp \
           big_cock.cpp \
           unpredictedweapon.cpp \
	   ../pm_shared/pm_debug.c \
	   ../pm_shared/pm_math.c \
	   ../pm_shared/pm_shared.c \
	   coop.cpp \
	   gateofbabylon.cpp \
	   gravgunmod.cpp \
	   shock.cpp \
	   shockrifle.cpp \
	   displacer.cpp \
	   m249.cpp \
	   knife.cpp \
	   gunmod.cpp \
	   BMOD_flyingcrowbar.cpp \
	   bottle.cpp \
	   eagle.cpp \
	   pipewrench.cpp \
	   penguin.cpp \
	   otis.cpp \
	   zombie_barney.cpp \
	   zombie_soldier.cpp \
	   fast_zombie.cpp \
	   black_headcrab.cpp \
	   hl2headcrab.cpp \
	   hl2_zombie.cpp \
	   gonome.cpp \
	   strooper.cpp \
	   massn.cpp \
	   voltigore.cpp \
	   sporegrenade.cpp \
	   pink_panther.cpp \
	   shrek.cpp \
	   sniperrifle.cpp \
	   sporelauncher.cpp \
	   spore_ammo.cpp \
	   nukelauncher.cpp \
	   blkop_osprey.cpp \
	   cleansuit_scientist.cpp \
	   fgrunt_medic.cpp \
	   fgrunt_torch.cpp \
	   chaingun.cpp \
	   rcallymonster.cpp \
	   fgrunt.cpp \
	   wolf.cpp \
	   Snake.cpp \
	   Cow.cpp \
	   Chicken.cpp \
	   Trex.cpp \
	   Spider.cpp \
	   uzi.cpp \
	   usp.cpp \
	   hammer.cpp \
	   bonewheel.cpp \
	   bigsmoke.cpp \
	   mariozombie.cpp \
	   skrillex.cpp \
	   zombozo.cpp \
	   ak47.cpp \
	   xm1014.cpp \
	   skeleton.cpp \
	   barrel_explosive.cpp \
	   molotov.cpp \
	   monster_molotov.cpp \
	   evilsci.cpp \
	   superchav.cpp \
	   blackscary.cpp \
	   physgun.cpp \
	   flamethrower.cpp \
	   flame.cpp \
	   grapple.cpp
#	   ../game_shared/voice_gamemgr.cpp

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)
