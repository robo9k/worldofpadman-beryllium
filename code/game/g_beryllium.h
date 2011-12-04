/*
===========================================================================
Copyright (C) 2010, 2011 the-brain

This program is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://gnu.org/licenses/>.
===========================================================================
*/

#ifndef _G_BERYYLIUM_H
#define _G_BERYLLIUM_H


#ifndef BERYLLIUM_VERSION
	#error "BERYLLIUM_VERSION not defined, Makefile corrupt?"
#endif


#define S_COLOR_DEFAULT		S_COLOR_WHITE	/* Used to "reset" colored output after other colored output */
#define S_COLOR_POSITIVE	S_COLOR_GREEN
#define S_COLOR_NEGATIVE	S_COLOR_RED
#define S_COLOR_ITALIC		S_COLOR_YELLOW	/* Important output */
#define S_COLOR_BOLD		S_COLOR_CYAN	/* Even more important output */

#define BE_LOG_PREFIX		"beryllium: "


#define DEFAULT_PLAYERMODEL_S	"padman"
#define DEFAULT_PLAYERSKIN_S	"default"
#define PLAYERMODEL_PATH_S		"models/wop_players/"

#define INVALID_PLAYERNAME_DEFAULT_S	"UnnamedPlayer"
#define RENAMED_PLAYERNAME_S			"RenamedPlayer"

#define CHAT_SERVER_NAME	"server"

#define MAX_CAMPTIME		20

#define MAX_TARGETNAME		64
#define MAX_SECRETS			64

#define MAX_GUIDBANS		128

#define EC					"\x19"

#define BERYLLIUM_ASCII_S	" _____             _ _ _           \n" \
							"| __  |___ ___ _ _| | |_|_ _ _____ \n" \
							"| __ -| -_|  _| | | | | | | |     |\n" \
							"|_____|___|_| |_  |_|_|_|___|_|_|_|\n" \
							"              |___|                \n"

#define G_VERSION_S			"wop 1.5_SVN1898M"

typedef struct {
	char guid[GUIDSTRMAXLEN];
} guidBan_t;

enum {
	GUIDCHECK_EMPTY		= 1,
	GUIDCHECK_FORMAT	= 2
};

enum {
	CHAT_SWAP			= 1,
	CHAT_SPECTATOR_TEAM	= 2
};

enum {
	BE_DF_BACKGROUNDRELOAD	= 1,
	BE_DF_GRAPPLE			= 2,
	BE_DF_STARTHEALTH		= 4,
	BE_DF_NOPOWERUPS		= 8,
	BE_DF_NOHOLDABLES		= 16,
	BE_DF_NOARMOR			= 32,
	BE_DF_NOHEALTH			= 64,
	BE_DF_NOAMMO			= 128,
	BE_DF_NOWEAPONS			= 256,
	BE_DF_CLEVERDUCKS		= 512,
	BE_DF_AIRKILL			= 1024
};

extern int			numGUIDBans;
extern guidBan_t	guidBans[MAX_GUIDBANS];


/* Cvars. See g_main.c */
extern vmCvar_t	be_version;

extern vmCvar_t	be_voteDuration;
extern vmCvar_t	be_allowedVotes;
extern vmCvar_t	be_votePause;
extern vmCvar_t be_voteRate;
extern vmCvar_t be_votePass;
extern vmCvar_t be_maxVotes;

extern vmCvar_t	be_respawnProtect;

extern vmCvar_t	be_switchTeamTime;

extern vmCvar_t	be_maxNameChanges;

extern vmCvar_t	be_checkGUIDs;

extern vmCvar_t	be_maxConnections;

extern vmCvar_t	g_version;

extern vmCvar_t	be_campDistance;

extern vmCvar_t	be_checkPings;

extern vmCvar_t	be_oneUp;

extern vmCvar_t	be_noSecrets;
extern vmCvar_t	be_debugSecrets;

extern vmCvar_t	be_hideChat;

extern vmCvar_t	be_banFile;

/* Parts of unlagged gamecode here as not to pollute the vanilla code */
extern vmCvar_t g_truePing;

extern vmCvar_t	be_selfDamage;

extern vmCvar_t	be_chatFlags;

extern vmCvar_t	be_overrideEntities;

extern vmCvar_t	be_dmFlags;

extern vmCvar_t	be_startWeapons;


/* FIXME: Add these to game headers? Declared in g_cmds.c, partially static */
void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText );
char *ConcatArgs( int start );
void G_SayTo( gentity_t *ent, gentity_t *other, int mode, int color, const char *name, const char *message );

/* Declared in g_client.c, formerly static */
extern void ClientCleanName( const char *in, char *out, int outSize );


/* "Exported" Functions */
qboolean BE_ClientCommand( gentity_t *ent, const char *cmd );
qboolean BE_ConsoleCommand( const char *cmd );

void BE_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
                vec3_t dir, vec3_t point, int *damage, int *dflags, int *mod );

void BE_ClientUserinfoChanged( int clientNum );
char *BE_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );

void BE_ClientTimerActions( gentity_t* ent );

void BE_ClientBegan( int clientNum );

void BE_ClientSpawn( gentity_t *ent );

void BE_ClientKilled( gentity_t *self );

void BE_ClientDisconnect( int clientNum );

qboolean BE_CanSayTo( const gentity_t *ent, const gentity_t *other );

void IgnoreChat( gentity_t *ent, const gentity_t *other, qboolean mode );
qboolean ChatIgnored( const gentity_t *ent, const gentity_t *other );

void BE_InitBeryllium( void );

qboolean BE_CanUseTeleporter( const gentity_t *ent, gentity_t *other );
qboolean BE_CanUseMover( const gentity_t *ent, gentity_t *other );

qboolean BE_HideChat( gentity_t *ent, gentity_t *target, int mode, int color, const char *name, const char *message );

void BE_LoadBans( void );
void BE_WriteBans( void );
qboolean AddBan( guidBan_t ban );
qboolean DeleteBan( unsigned int index );

/* See NOTE in implementation */
qboolean G_SetTeam( gentity_t *ent, char *s, qboolean force );

qboolean BE_GetEntityToken( char *buffer, int bufferSize );
void BE_PreSpawnEntities( void );
void BE_PostSpawnEntities( void );

qboolean BE_CallSpawn( const char *classname );
qboolean BE_ItemDisabled( gitem_t *item );

#endif

