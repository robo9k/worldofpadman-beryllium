/*
===========================================================================
Copyright (C) 2011 thebrain

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

enum {
	GUIDCHECK_EMPTY		= 1,
	GUIDCHECK_FORMAT	= 2
};


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


extern void G_Say( gentity_t *ent, gentity_t *target, int mode, const char *chatText );
extern char *ConcatArgs( int start ); /* FIXME: Add these to game headers? Declared in g_cmds.c */

extern void ClientCleanName( const char *in, char *out, int outSize ); /* Declared in g_client.c, formerly static */


/* "Exported" Functions */
qboolean BE_ClientCommand( gentity_t *ent, const char *cmd );
qboolean BE_ConsoleCommand( const char *cmd );

void BE_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
                vec3_t dir, vec3_t point, int *damage, int *dflags, int *mod );

void BE_ClientUserinfoChanged( int clientNum );
char *BE_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot );

void BE_ClientTimerActions( gentity_t* ent );

void BE_ClientBegan( int clientNum );

void BE_ClientKilled( gentity_t *self );

void BE_ClientDisconnect( int clientNum );

qboolean BE_CanSayTo( const gentity_t *ent, const gentity_t *other );

void IgnoreChat( gentity_t *ent, const gentity_t *other, qboolean mode );
qboolean ChatIgnored( const gentity_t *ent, const gentity_t *other );

void BE_InitBeryllium( void );

qboolean BE_CanUseTeleporter( const gentity_t *ent, gentity_t *other );
qboolean BE_CanUseMover( const gentity_t *ent, gentity_t *other );

qboolean BE_HideChat( const gentity_t *ent, const gentity_t *target, int mode, int color, const char *name, const char *message );

#endif

