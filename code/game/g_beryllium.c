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


/* Functions */

/*
	Hooks into ClientCommand() in g_cmds.c
	We can catch any client command here and even override default ones,
	since our function is called before original ones.
	We already have a ent->client.
	If we return qtrue, the original function will return immediatelly.
*/
qboolean BE_ClientCommand( const gentity_t *ent, const char *cmd ) {
	/* This is just a wrapper function */
	return BE_ClCmd( ent, cmd );
}


/*
	Hooks into ConsoleCommand() in g_svcmds.c
	Basically the same as  BE_ClientCommand() for server commands, i.e. rcon.
*/
qboolean BE_ConsoleCommand( const char *cmd ) {
	/* This is just a wrapper function */
	return BE_ConCmd( cmd );
}


/*
	Beryllium's replacement for CheckVote() in g_main.c
	Determines whether a vote passed or failed and execute it
*/
/* TODO: Create be_vote.c and move function */
void BE_CheckVote( void ) {
	if ( level.voteExecuteTime && ( level.voteExecuteTime < level.time ) ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}

	if ( !level.voteTime ) {
		return;
	}

	if ( ( level.time - level.voteTime ) >= VOTE_TIME ) {
		SendClientCommand( -1, CCMD_PRT, "Vote failed, timeout.\n" );
	}
	else {
		if ( level.voteYes > ( level.numVotingClients / 2 ) ) {
			/* Set timeout, then execute and remove the vote at next call */
			SendClientCommand( -1, CCMD_PRT, "Vote passed.\n" );
			level.voteExecuteTime = ( level.time + 3000 ); /* FIXME: Magical constant; voteExecuteDelay */
		}
		else if ( level.voteNo >= ( level.numVotingClients / 2 ) ) {
			/* same behavior as a timeout */
			SendClientCommand( -1, CCMD_PRT, "Vote failed.\n" );
		}
		else {
			/* still waiting for a majority */
			return;
		}
	}

	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );
}

