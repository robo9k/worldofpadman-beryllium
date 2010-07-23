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

const svcmd_t be_svcmds[] = {
	{ "tell",		BE_Svcmd_Tell_f			},
	{ "cancelvote",	BE_Svcmd_Cancelvote_f	}
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
		G_Printf( "Usage: tell <id> <text>\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );
	clientNum = atoi( clientStr );

	/* Valid range is -1 and 0 to MAX_CLIENTS-1 */
	if ( ( clientNum < -1 ) || ( clientNum >= MAX_CLIENTS ) ) {
		G_Printf( "clientNum out of range\n" );
		return;
	}

	/* Client needs to be fully connected to see message */
	if ( ( clientNum != -1 ) &&
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
	if ( !level.voteTime ) {
		G_Printf( "No vote in progress.\n" );
		return;
	}

	level.voteNo = level.numConnectedClients;
	level.voteYes = 0;
	CheckVote();

	SendClientCommand( -1, CCMD_PRT, "Vote was canceled.\n" );
}

