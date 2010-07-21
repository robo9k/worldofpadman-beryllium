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


/* Internal functions */
static void BE_Cmd_CallVote_f( const gentity_t *ent );


/* Variables */
const commands_t be_ccmds[] = {
	{ "callvote",	CMD_MESSAGE,	BE_Cmd_CallVote_f },
	{ "cv",			CMD_MESSAGE,	BE_Cmd_CallVote_f }
};
const unsigned int NUM_CMDS = ( sizeof( be_ccmds ) / sizeof( be_ccmds[0] ) );

const char* validVotes[] = {
	"nextmap",
	"map",
	"map_restart",
	"kick",
	"clientkick",
	"timelimit",
	"pointlimit",
	"g_gametype",
	"setgametype"
};
const unsigned int NUM_VOTES = ( sizeof( validVotes ) / sizeof( validVotes[0] ) );


/* Functions */

/*
	This function just loops through our registered client commands,
	checks conditions in flags are met and then calls the function.
*/
qboolean BE_ClCmd( const gentity_t *ent, const char *cmd ) {
	unsigned int i;

	for ( i = 0; i < NUM_CMDS; i++ ) {
		if ( Q_stricmp( cmd, be_ccmds[i].cmdName ) == 0 ) {
			// do tests here to reduce the amount of repeated code
			// We provide this command, but the conditions to actually execute it are not given

			if ( !( be_ccmds[i].cmdFlags & CMD_INTERMISSION ) && level.intermissiontime ) {
				return qtrue;
			}

			if ( ( be_ccmds[i].cmdFlags & CMD_CHEAT ) && !g_cheats.integer ) {
				return qtrue;
			}

			if ( ( be_ccmds[i].cmdFlags & CMD_LIVING ) && ( ent->client->ps.pm_type == PM_DEAD ) ) {
				return qtrue;
			}

			/* TODO: Test for CMD_MESSAGE. This is only useful with muting implemented */


			/* Conditions are met, execute! */
			be_ccmds[i].cmdHandler( ent );
			return qtrue;
		}
	}

	/* This is none of our business */
	return qfalse;
}

/*
	Beryllium's replacement for the original Cmd_CallVote_f() in g_cmds.c
*/
static void BE_Cmd_CallVote_f( const gentity_t *ent ) {
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	*c;
	int		i;

	if ( !g_allowVote.integer ) {
		SendClientCommand( ( ent - g_entities ), ( CCMD_PRT | CCMD_CP ), "Voting not allowed here.\n" );
		return;
	}

	if ( level.voteTime ) {
		SendClientCommand( ( ent - g_entities ), ( CCMD_PRT | CCMD_CP ), "A vote is already in progress.\n" );
		return;
	}

	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		SendClientCommand( ( ent - g_entities ), ( CCMD_PRT | CCMD_CP ), "You have called the maximum number of votes.\n" );
		return;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SendClientCommand( ( ent - g_entities ), ( CCMD_PRT | CCMD_CP ), "Not allowed to call a vote as spectator.\n" );
		return;
	}

	/* make sure it is a valid command to vote on */
	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );

	/* NOTE: http://bugzilla.icculus.org/show_bug.cgi?id=3593 */
	/* check for command separators in arg2 */
	for ( c = arg2; *c; ++c ) {
		switch ( *c ) {
			case '\n':
			case '\r':
			case ';':
				SendClientCommand( ( ent - g_entities ), ( CCMD_PRT | CCMD_CP ), "Invalid vote string.\n" );
				return;
			break;
		}
	}

	for ( i = 0; i < NUM_VOTES; i++ ) {
		if ( Q_stricmp( arg1, validVotes[i] ) == 0 ) {
			break;
		}
	}
	if ( i == NUM_VOTES ) {
		char validVoteString[MAX_STRING_TOKENS] = { "Valid vote commands are: " };
		for ( i = 0; i < NUM_VOTES; i++ ) {
			Q_strcat( validVoteString, sizeof( validVoteString ),
					  ( i < ( NUM_VOTES - 1 ) ) ? va( "%s, ", validVotes[i] ) : va( "%s.\n", validVotes[i] ) );
		}

		SendClientCommand( ( ent - g_entities ), ( CCMD_PRT | CCMD_CP ), "Invalid vote string\n" );
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, validVoteString );
		return;
	}

	/* if there is still a vote to be executed */
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}

	/* TODO: Declare a voteType enum? Otherwise we'd need to check strings again */
}

