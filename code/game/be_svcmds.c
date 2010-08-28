/*
===========================================================================
Copyright (C) 2010 thebrain

This program is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "g_local.h"


static void BE_Svcmd_Tell_f( void );
static void BE_Svcmd_Cancelvote_f( void );
static void BE_Svcmd_ShuffleTeams_f( void );

/* FIXME: Add this to game headers? Declared in g_main.c */
int QDECL SortRanks( const void *a, const void *b );

const svcmd_t be_svcmds[] = {
	{ "tell",			BE_Svcmd_Tell_f			},
	{ "cancelvote",		BE_Svcmd_Cancelvote_f	},
	{ "shuffleteams",	BE_Svcmd_ShuffleTeams_f	}
};
const unsigned int NUM_SVCMDS = ( sizeof( be_svcmds ) / sizeof( be_svcmds[0] ) );


qboolean BE_ConCmd( const char *cmd ) {
	unsigned int i;

	for ( i = 0; i < NUM_SVCMDS; i++ ) {
		if ( Q_stricmp( cmd, be_svcmds[i].cmdName ) == 0 ) {
			be_svcmds[i].cmdHandler();
			return qtrue;
		}
	}

	return qfalse;
}


/*
	Print text to one or all clients. Text is displayed in
	the client's log as it is passed to the function
	(except for some weird double quotes with " )
*/
static void BE_Svcmd_Tell_f( void ) {
	char clientStr[MAX_TOKEN_CHARS];
	int clientNum;

	if ( trap_Argc() < 3 ) {
		/* TODO: Move counting of arguments and help into BE_ConCmd() ? */
		G_Printf( "Usage: tell <cid> <text>\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );
	clientNum = atoi( clientStr );

	if ( !ValidClientID( clientNum, qtrue ) ) {
		G_Printf( "Cid out of range\n" );
		return;
	}

	/* Client needs to be fully connected to see message */
	if ( ( clientNum != CID_ALL ) &&
	     level.clients[clientNum].pers.connected != CON_CONNECTED ) {
			G_Printf( "Client not connected.\n" );
			return;
	}

	trap_SendServerCommand( clientNum, va( "print \"%s\n\"", ConcatArgs( 2 ) ) );
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
		if ( (g_entities + sortedClients[i])->r.svFlags & SVF_BOT ) {
			char ci[MAX_INFO_STRING];

			trap_GetUserinfo( sortedClients[i], ci, sizeof ( ci ) );
			if ( !Info_Validate( ci ) ) {
				G_Error( "shuffleteams: Invalid userinfo for bot!\n" );
			}

			Info_SetValueForKey( ci, "team", TeamName( team ) );

			trap_SetUserinfo( sortedClients[i], ci );
		}

		ClientUserinfoChanged( sortedClients[i] );
		ClientBegin( sortedClients[i] );
	}
}

