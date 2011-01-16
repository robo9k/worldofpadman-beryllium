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

#include "g_local.h"


static void BE_Svcmd_Say_f( void );
static void BE_Svcmd_Cancelvote_f( void );
static void BE_Svcmd_ShuffleTeams_f( void );
static void BE_Svcmd_RenamePlayer_f( void );
static void BE_Svcmd_DropClient_f( void );
static void BE_Svcmd_ClientCommand_f( void );
static void BE_Svcmd_Callvote_f( void );

/* FIXME: Add this to game headers? Declared in g_main.c */
int QDECL SortRanks( const void *a, const void *b );

const svcmd_t be_svcmds[] = {
	{ "stell",			BE_Svcmd_Say_f				},
	{ "ssay",			BE_Svcmd_Say_f				}, /* NOTE: Gamecode has a "say" in ConsoleCommand in g_cmds.c, engine in SV_ConSay_f in sv_ccmds.c */
	{ "ssay_team",		BE_Svcmd_Say_f				},
	{ "cancelvote",		BE_Svcmd_Cancelvote_f		},
	{ "shuffleteams",	BE_Svcmd_ShuffleTeams_f		},
	{ "rename",			BE_Svcmd_RenamePlayer_f		},
	{ "dropclient",		BE_Svcmd_DropClient_f		},
	{ "scp",			BE_Svcmd_ClientCommand_f	},
	{ "smp",			BE_Svcmd_ClientCommand_f	},
	{ "sprint",			BE_Svcmd_ClientCommand_f	},
	{ "scallvote",		BE_Svcmd_Callvote_f			}
};
const unsigned int NUM_SVCMDS = ( sizeof( be_svcmds ) / sizeof( be_svcmds[0] ) );


/* FIXME: clientStr[3] works for 2 digit clientnums with default MAX_CLIENTS 64 only */

/* TODO: Call IsANumber before atoi and IsValidClientID ? */


qboolean BE_ConCmd( const char *cmd ) {
	unsigned int i;


	assert( cmd );


	for ( i = 0; i < NUM_SVCMDS; i++ ) {
		if ( Q_stricmp( cmd, be_svcmds[i].cmdName ) == 0 ) {
			/* TODO: Since some handlers are registered for more than one command, we should pass
			         the actual command id as an argument so they don't need to Q_stricmp again.
			*/
			be_svcmds[i].cmdHandler();
			return qtrue;
		}
	}

	return qfalse;
}


/*
	Cancel a currently running vote, i.e. emulate everyone
	voted no. This doesn't work with teamvotes, as they are not
	actually used by the game.
*/
static void BE_Svcmd_Cancelvote_f( void ) {
	if ( !level.voteTime && !level.voteExecuteTime ) {
		G_Printf( "No vote in progress.\n" );
		return;
	}

	/* NOTE: Don't set voteTime to 0, because BE_CheckVote() would return too early */
	level.voteExecuteTime = 0;

	level.voteNo = level.numVotingClients;
	level.voteYes = 0;

	BE_CheckVote();

	/* TODO: Also log this? */
	SendClientCommand( CID_ALL, CCMD_PRT, S_COLOR_ITALIC"Vote was canceled.\n" );
}


/*
	Shuffle teams by splitting good players evenly across teams.
	TODO: Also keep currently better team in mind (especially with uneven player counts).
          Maybe also use kills-deaths/time instead of simply current points to get best
	      players.
*/
/* TODO: assert() any pointer dereferencing */
static void BE_Svcmd_ShuffleTeams_f( void ) {
	int i, p, team;
	int count = 0;
	int	sortedClients[MAX_CLIENTS];
	gclient_t *cl;

	if ( g_gametype.integer < GT_TEAM ) {
		G_Printf( "Not in a team gametype." );
		return;
	}

	/* TODO: Also log this? */
	SendClientCommand( CID_ALL, CCMD_PRT, S_COLOR_ITALIC"Shuffling teams ..\n" );

	/* Offset to put "best"/first player in currently inferior team */
	p = ( ( level.teamScores[TEAM_RED] > level.teamScores[TEAM_BLUE] ) ? 1 : 0 );

	for( i = 0; i < level.numConnectedClients; i++ ) {
		cl = ( level.clients + level.sortedClients[i] );

		/* If not actively playing, ignore */
		if( ( cl->sess.sessionTeam != TEAM_RED ) &&
		    ( cl->sess.sessionTeam != TEAM_BLUE ) ) {
			continue;
		}

		sortedClients[count++] = level.sortedClients[i];
	}

	/* TODO: Don't shuffle if (human)count <= 1 ? */

	qsort( sortedClients, count, sizeof( sortedClients[0] ), SortRanks );

	for( i = 0; i < count; i++ ) {
		cl = ( level.clients + sortedClients[i] );

		team = ( ( ( i + p ) % 2 ) + TEAM_RED );
		cl->sess.sessionTeam = team;

		/* Bots' team is quite different */
		if ( ( g_entities + sortedClients[i] )->r.svFlags & SVF_BOT ) {
			char ci[MAX_INFO_STRING];

			trap_GetUserinfo( sortedClients[i], ci, sizeof ( ci ) );
			if ( !Info_Validate( ci ) ) {
				G_Error( "shuffleteams: Invalid userinfo for bot %i!\n", i );
				return;
			}

			Info_SetValueForKey( ci, "team", TeamName( team ) );

			trap_SetUserinfo( sortedClients[i], ci );
		}

		ClientUserinfoChanged( sortedClients[i] );
		ClientBegin( sortedClients[i] );
	}
}


/*
	Forcefully rename a player.
	Limits for invalid characters and multiple names still apply.
*/
static void BE_Svcmd_RenamePlayer_f( void ) {
	char	clientStr[3], newname[MAX_NETNAME];
	char	userinfo[MAX_INFO_STRING];
	int		clientNum;

	if ( trap_Argc() < 3 ) {
		/* TODO: Move counting of arguments and help into BE_ConCmd() ? */
		G_Printf( "Usage: rename <cid> <newname>\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );
	clientNum = atoi( clientStr );

	if ( !ValidClientID( clientNum, qfalse ) ) {
		G_Printf( "Not a valid client number.\n" );
		return;
	}

	trap_Argv( 2, newname, sizeof( newname ) );

	/* FIXME: Renaming connecting clients is somewhat useless, since they might change
	          their name multiple times automatically during connect, which is not limited.

	          Lock names after rename? (set nameChanges to be_maxNameChanges-1)
	*/

	/* TODO: Print info that player was forcefully renamed? */

	/* Make beryllium's filter code in ClientUserinfoChanged() work like expected */
	/* NOTE: In order for rename to work always, we need to invalidate the actual value of
	         the client's maxNameChanges.
	         If we don't do this, even admin cant rename when maximum is exceeded.
	*/
	if ( level.clients[clientNum].pers.nameChanges > be_maxNameChanges.integer ) {
		level.clients[clientNum].pers.nameChanges = ( be_maxNameChanges.integer - 1 );
	}
	else {
		level.clients[clientNum].pers.nameChanges--;
	}
	level.clients[clientNum].pers.nameChangeTime = 0;

	/* TODO: Also log this? */

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	Info_SetValueForKey( userinfo, "name", newname );
	trap_SetUserinfo( clientNum, userinfo );
  	ClientUserinfoChanged( clientNum );
}


/*
	Basically the same as clientkick, but can provide a reason to the kicked client
*/
static void BE_Svcmd_DropClient_f( void ) {
	char clientStr[3], reason[MAX_STRING_CHARS];
	int clientNum;


	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage: dropclient <cid> (reason)\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );
	clientNum = atoi( clientStr );

	/* TODO: Shall we allow world here? */
	if ( !ValidClientID( clientNum, qfalse ) ) {
		G_Printf( "Not a valid client number.\n" );
		return;
	}

	if ( CON_DISCONNECTED == level.clients[clientNum].pers.connected ) {
		G_Printf( "Client not connected.\n" );
		return;
	}

	Q_strncpyz( reason, "was kicked", sizeof( reason ) );
	if ( trap_Argc() > 2 ) {
		Com_sprintf( reason, sizeof( reason ), "was kicked: %s", ConcatArgs( 2 ) );
	}

	/* TODO: Also log this? */
	
	trap_DropClient( clientNum, reason );	
}


/*
	Serverside chat functions, which look like normal chat to the client
*/
static void BE_Svcmd_Say_f( void ) {
	char arg[MAX_STRING_CHARS];


	trap_Argv( 0, arg, sizeof( arg ) );

	if ( Q_stricmp( "ssay", arg ) == 0 ) {
		if ( trap_Argc() < 2 ) {
			G_Printf( "Usage: ssay <text>\n" );
			return;
		}

		G_Say( NULL, NULL, SAY_ALL, ConcatArgs( 1 ) );
	}
	else if ( Q_stricmp( "ssay_team", arg ) == 0 ) {
		team_t team;
		int i;

		if ( trap_Argc() < 3 ) {
			G_Printf( "Usage: ssay_team <team> <text>\n" );
			return;
		}

		trap_Argv( 1, arg, sizeof( arg ) );
		team = TeamFromString( arg );

		if ( TEAM_NUM_TEAMS == team ) {
			G_Printf( "Unknown team.\n" );
			return;
		}

		/* NOTE: G_Say expects two gentity_t, so we just pick one
		         which has the desired team.
		*/
		for ( i = 0; i < level.maxclients; i++ ) {
			if ( team == level.clients[i].sess.sessionTeam ) {
				break;
			}
		}
		if ( i == level.maxclients ) {
			G_Printf( "No players in desired team.\n" );
			return;
		}

		/* NOTE: See NOTE in modified G_say! */
		G_Say( NULL, ( g_entities + i ), SAY_TEAM, ConcatArgs( 2 ) );
	}
	else if ( Q_stricmp( "stell", arg ) == 0 ) {
		int clientNum;

		if ( trap_Argc() < 3 ) {
			G_Printf( "Usage: stell <cid> <text>\n" );
			return;
		}

		trap_Argv( 1, arg, sizeof( arg ) );
		clientNum = atoi( arg );
		/* NOTE: Don't allow world here. say/print should be used instead */
		if ( !ValidClientID( clientNum, qfalse ) ) {
			G_Printf( "Not a valid client number.\n" );
			return;
		}

		/* TODO: Check for CON_CONNECTED */

		G_Say( NULL, ( g_entities + clientNum ), SAY_TELL, ConcatArgs( 2 ) );
	}
	else {
		G_Error( "Be_Svmcmd_Say_f: Unknown mode %s!\n", arg );
		return;
	}
}


/*
	A wrapper for SendClientCommand
*/
static void BE_Svcmd_ClientCommand_f( void ) {
	char arg[MAX_STRING_CHARS];
	int clientNum;
	clientCommand_t	cmd;


	trap_Argv( 0, arg, sizeof( arg ) );
	if ( Q_stricmp( "scp", arg ) == 0 ) {
		cmd = CCMD_CP;
	}
	else if ( Q_stricmp( "smp", arg ) == 0 ) {
		cmd = CCMD_MP;
	}
	else if ( Q_stricmp( "sprint", arg ) == 0 ) {
		cmd = CCMD_PRINT;
	}
	else {
		G_Error( "BE_Svcmd_ClientCommand_f: Unknown mode %s!\n", arg );
		return;
	}

	if ( trap_Argc() < 3 ) {
		G_Printf( "Usage: %s <cid> <text>\n", arg );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );
	clientNum = atoi( arg );
	if ( !ValidClientID( clientNum, qtrue ) ) {
		G_Printf( "No a valid client number.\n" );
		return;
	}

	/* See NOTE in SendClientCommand */
	if ( CCMD_PRINT == cmd ) {
		Com_sprintf( arg, sizeof( arg ), "%s\n", ConcatArgs( 2 ) );
	}
	else {
		Q_strncpyz( arg, ConcatArgs( 2 ), sizeof( arg ) );
	}


	SendClientCommand( clientNum, cmd, arg );
}


/*
	A wrapper around BE_Cmd_CallVote_f, which had to be modified to
	work without ent
*/
static void BE_Svcmd_Callvote_f( void ) {
	BE_Cmd_CallVote_f( NULL );
}

