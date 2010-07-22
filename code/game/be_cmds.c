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
extern char	*ConcatArgs( int start ); /* FIXME: Add this to game headers? Declared in g_cmds.c */
static void BE_Cmd_CallVote_f( const gentity_t *ent ) {
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	char	*c;
	int		i;

	if ( !g_allowVote.integer ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Voting not allowed here.\n" );
		return;
	}

	if ( level.voteTime ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "A vote is already in progress.\n" );
		return;
	}

	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "You have called the maximum number of votes.\n" );
		return;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Not allowed to call a vote as spectator.\n" );
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
				SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Invalid vote string.\n" );
				return;
			break;
		}
	}

	for ( i = 0; i < NUM_VOTES; i++ ) {
		if ( Q_stricmp( arg1, validVotes[i] ) == 0 ) {
			break;
		}
	}
	if ( NUM_VOTES == i ) {
		char validVoteString[MAX_STRING_TOKENS] = { "Valid vote commands are: " };
		for ( i = 0; i < NUM_VOTES; i++ ) {
			Q_strcat( validVoteString, sizeof( validVoteString ),
					  ( i < ( NUM_VOTES - 1 ) ) ? va( "%s, ", validVotes[i] ) : va( "%s.\n", validVotes[i] ) );
		}

		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Invalid vote string.\n" );
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, validVoteString );
		return;
	}

	/* if there is still a vote to be executed */
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}

	/* TODO: Declare a voteType enum? Otherwise we'd need to check strings again */
	/* TODO: Put all these checks into external functions? */

	/* Special: A variant of g_gametype vote */
	if ( Q_stricmp( arg1, "setgametype" ) == 0 ) {
		gametype_t gt = StringToGametype( ConcatArgs( 2 ) );

		if ( GT_MAX_GAME_TYPE == gt ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Couldn't find a gametype with the keywords.\n" );
			return;
		}

		/* Gametype found, now "convert" into g_gametype vote */
		Q_strncpyz( arg1, "g_gametype", sizeof( arg1 ) );
		Com_sprintf( arg2, sizeof( arg2 ), "%i", gt );
	}
	/* Special: A variant of clientkick vote */
	else if ( Q_stricmp( arg1, "kick" ) == 0 ) {
		gclient_t	*player;
		char		cleanName[64];
		int			id = -1;

		/* this is nasty, no need to have it voteable. Admins shall use rcon */
		/* kick allbots might be legitimate though */
		if ( Q_stricmp( arg2, "all" ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Only admins are allowed to do this. If a player is named 'all', use 'clientkick' instead.\n" );
			return;
		}

		/* try to circumvent renaming exploit and use id, which can not change */
		for ( i = 0; i < level.maxclients; i++ ) {
			player = &level.clients[i];

			if ( player->pers.connected == CON_DISCONNECTED ) {
				continue;
			}

			/* exact match */
			if ( Q_stricmp( arg2, player->pers.netname ) == 0 ) {
				id = i;
				break;
			}

			/* match without colorcodes */
			Q_strncpyz( cleanName, player->pers.netname, sizeof( cleanName ) );
			Q_CleanStr( cleanName );
			if ( Q_stricmp( arg2, cleanName ) == 0 ) {
				id = i;
				break;
			}

			/* TODO: Partial match with strstr (dangerous?) */
		}

		if ( id == -1 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "No player found with that name. Check for typos or use 'clientkick' instead.\n" );
			return;
		}

		/* FIXME: Handle LAN servers */
		/*
		if ( player->pers.localClient ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "You can not kick the host player.\n" );
			return;
		}
		*/

		/* We've got one, convert into clientkick vote
		   TODO: Check for multiple matches
		   TODO: Display old and current name in votestring?
		*/
		Q_strncpyz( arg1, "clientkick", sizeof( arg1 ) );
		Com_sprintf( arg2, sizeof( arg2 ), "%i", id );
	}


	if ( Q_stricmp( arg1, "g_gametype" ) == 0 ) {
		i = atoi( arg2 );

		if( ( GT_SINGLE_PLAYER == i ) || ( i < GT_FFA ) || ( i >= GT_MAX_GAME_TYPE ) ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Invalid gametype.\n" );
			return;
		}

		if ( g_gametype.integer == i ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "This is the current gametype.\n" );
			return;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %d", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Gametype %s", GametypeToString( i ) );
	}
	else if ( Q_stricmp( arg1, "map" ) == 0 ) {
		char s[MAX_STRING_CHARS];
		fileHandle_t f;
		
		/* Does map exist at all? */
		f = trap_FS_FOpenFile( va( "maps/%s.bsp", arg2 ), &f, FS_READ );
		if ( -1 == f ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Map not found.\n" );
			return;
		}
		
		/* Backup and re-apply current nextmap */
		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
		if ( *s ) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s; set nextmap \"%s\"", arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s", arg2 );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Map %s", arg2 );
	}
	else if ( Q_stricmp( arg1, "nextmap" ) == 0 ) {
		char s[MAX_STRING_CHARS];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
		if ( !*s ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "nextmap is not set on the server.\n" );
			return;
		}

		Q_strncpyz( level.voteString, "vstr nextmap", sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteDisplayString, "Next map", sizeof( level.voteDisplayString ) );
	}
	else if ( Q_stricmp( arg1, "clientkick" ) == 0 ) {
		i = atoi( arg2 );

		if ( ( i < 0 ) || ( i >= MAX_CLIENTS ) ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Not a valid clientID.\n" );
			return;
		}

		if ( level.clients[i].pers.connected == CON_DISCONNECTED ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Client not connected.\n" );
			return;
		}

		/* TODO: Display clientid as well? */
		Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick \"%i\"", i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "Kick %s", level.clients[i].pers.netname );
	}
	else if ( Q_stricmp( arg1, "map_restart" ) == 0 ) {
		Q_strncpyz( level.voteString, "map_restart", sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteDisplayString, "Restart map", sizeof( level.voteDisplayString ) );
	}
	/* Any other vote is not explicitly handled */
	/* These are currently: pointlimit, timelimit */
	else {
		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), "%s", level.voteString );
	}

	/* Phew, we finally got a valid and formatted vote */
	SendClientCommand( -1, CCMD_PRT, va( "%s called a vote: \"%s\".\n", ent->client->pers.netname, level.voteDisplayString ) );
	/* TODO: Create a seperate BE_Logf() with loglevels and such */
	G_LogPrintf( "%i called a vote \"%s\"\n", ( ent - g_entities ), level.voteString );

	/* start the voting, the caller autoamtically votes yes */
	level.voteTime = level.time;
	level.voteYes = 1;
	level.voteNo = 0;

	for ( i = 0 ; i < level.maxclients ; i++ ) {
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}
	ent->client->ps.eFlags |= EF_VOTED;
	ent->client->pers.voteCount++;

	trap_SetConfigstring( CS_VOTE_TIME, va( "%i", level.voteTime ) );
	trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
	trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
	trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );
}

