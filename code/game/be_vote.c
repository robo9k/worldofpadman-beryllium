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

extern char	*ConcatArgs( int start ); /* FIXME: Add this to game headers? Declared in g_cmds.c */


/* Internal functions */
static qboolean VoteH_Map( const gentity_t *ent );
static qboolean VoteH_Kick( const gentity_t *ent );
static qboolean VoteH_Gametype( const gentity_t *ent );
static qboolean VoteH_Misc( const gentity_t *ent );

static qboolean IsAllowedVote( const char *str );


const voteHandler_t voteHandler[] = {
	{ "nextmap",			VoteH_Map		},
	{ "map",				VoteH_Map		},
	{ "map_restart",		VoteH_Map		},
	{ "kick",				VoteH_Kick		},
	{ "clientkick",			VoteH_Kick		},
	{ "timelimit",			VoteH_Misc		},
	{ "pointlimit",			VoteH_Misc		},
	{ "g_gametype",			VoteH_Gametype	},
	{ "setgametype",		VoteH_Gametype	},
/*	{ "g_dowarmup",			VoteH_Misc		},*/ /* This is just too stupid to be votable */
	{ "fastgamespeed",		VoteH_Misc		},
	{ "normalgamespeed",	VoteH_Misc 		}
};
const unsigned int NUM_VOTEH = ( sizeof( voteHandler ) / sizeof( voteHandler[0] ) );


static qboolean IsAllowedVote( const char *str ) {
	return qtrue;
}


/*
	Beryllium's replacement for Cmd_Vote_f() in g_cmds.c
*/
void BE_Cmd_Vote_f( const gentity_t *ent ) {
	char msg[8];

	if ( !level.voteTime ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "No vote in progress.\n" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Vote already cast.\n" );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Not allowed to vote as spectator.\n" );
		return;
	}

	SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Vote cast, thank you.\n" );

	ent->client->ps.eFlags |= EF_VOTED;

	trap_Argv( 1, msg, sizeof( msg ) );

	if ( ( msg[0] == 'y' ) || ( msg[0] == 'Y' ) || ( msg[0] == '1' ) ) {
		level.voteYes++;
		trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );

		/* TODO: Don't print to self? Need SendClientCommandExcept()
	             Use Loglevels?
		*/
		SendClientCommand( -1, CCMD_PRT, va( "%s"S_COLOR_WHITE" voted "S_COLOR_GREEN"yes"S_COLOR_WHITE".\n", ent->client->pers.netname ) );
	}
	else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );

		SendClientCommand( -1, CCMD_PRT, va( "%s"S_COLOR_WHITE" voted "S_COLOR_RED"no"S_COLOR_WHITE".\n", ent->client->pers.netname ) );
	}

	/* "a majority will be determined in CheckVote, which will also account
	   for players entering or leaving"
	*/
}


/*
	Beryllium's replacement for the original Cmd_CallVote_f() in g_cmds.c
*/
void BE_Cmd_CallVote_f( const gentity_t *ent ) {
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

	/* FIXME: One can gain new votes by changing team. If we use a fixed limit, it should be persistant
	          for each map.
	*/
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

	
	/* if there is still a vote to be executed */
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}


	for ( i = 0; i < NUM_VOTEH; i++ ) {
		if ( Q_stricmp( arg1, voteHandler[i].str ) == 0 ) {
			if ( !IsAllowedVote( arg1 ) ) {
				/* TODO: Print allowed votes? At least at bottom of function if vote is invalid */
				SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Vote is not allowed.\n" );
				return;
			}


			/* Legitimate vote
			   Any error handling, message printing etc must be done in VoteH_
			*/
			if ( voteHandler[i].cmdHandler( ent ) ) {

				/* FIXME: There is a problem clientside when displaying a votestring with double " inside */
				SendClientCommand( -1, CCMD_PRT, va( "%s"S_COLOR_WHITE" called a vote: "S_COLOR_YELLOW"%s"S_COLOR_WHITE".\n",
	                                                 ent->client->pers.netname, level.voteDisplayString ) );
				/* TODO: Create a seperate BE_Logf() with loglevels and such. Use voteString or voteDisplayString? */
				G_LogPrintf( "%i called vote '%s'\n", ( ent - g_entities ), level.voteString );


				/* start the voting, the caller autoamtically votes yes */
				level.voteTime = level.time;
				level.voteYes = 1;
				level.voteNo = 0;

				for ( i = 0 ; i < level.maxclients ; i++ ) {
					level.clients[i].ps.eFlags &= ~EF_VOTED;
				}
				ent->client->ps.eFlags |= EF_VOTED;
				ent->client->pers.voteCount++;

				/* A littly hackity to get votes with variable time.
				   The client has a hardcoded duration of VOTE_TIME, which
				   is 30s. So we need to send a modified level.voteTime to the client
				   to get its time being displayed correctly, while still having
				   the correct voteTime in game.
				*/
				/* voteDuration is needed so chaning the cvar doesn't fuck up current vote
				   Cvar is in seconds.
				*/
				level.voteDuration = ( be_voteDuration.integer * 1000 );
				i = ( level.voteTime + ( level.voteDuration - VOTE_TIME ) );	

				trap_SetConfigstring( CS_VOTE_TIME, va( "%i", i ) );
				trap_SetConfigstring( CS_VOTE_STRING, level.voteDisplayString );	
				trap_SetConfigstring( CS_VOTE_YES, va( "%i", level.voteYes ) );
				trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );

				return;
			}
			else {
				/* Error inside vote handler. Error output is done in handler itself, so nothing to do */
				return;
			}
		}
	}

	if ( NUM_VOTEH == i ) {
		char validVoteString[MAX_STRING_TOKENS] = { "Valid and allowed vote commands are: " };
		for ( i = 0; i < NUM_VOTEH; i++ ) {
			if ( !IsAllowedVote( voteHandler[i].str ) ) {
				/* FIXME: Handle case when there are no allowed votes? Set g_allowVote 0? */
				continue;
			}			

			Q_strcat( validVoteString, sizeof( validVoteString ),
					  ( i < ( NUM_VOTEH - 1 ) ) ? va( S_COLOR_YELLOW"%s"S_COLOR_WHITE", ", voteHandler[i].str )
                                                : va( S_COLOR_YELLOW"%s"S_COLOR_WHITE".\n", voteHandler[i].str ) );
		}

		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Invalid vote string.\n" );
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, validVoteString );
		return;
	}

}


/*
	Beryllium's replacement for CheckVote() in g_main.c
	Determines whether a vote passed or failed and execute it
*/
void BE_CheckVote( void ) {
	if ( level.voteExecuteTime && ( level.voteExecuteTime < level.time ) ) {
		level.voteExecuteTime = 0;
		trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}

	if ( !level.voteTime ) {
		return;
	}

	if ( ( level.time - level.voteTime ) >= level.voteDuration ) {
		SendClientCommand( -1, CCMD_PRT, "Vote failed, timeout.\n" );
	}
	else {
		if ( level.voteYes > ( level.numVotingClients / 2 ) ) {
			/* Set timeout, then execute and remove the vote at next call */
			SendClientCommand( -1, CCMD_PRT, "Vote passed.\n" );
			level.voteExecuteTime = ( level.time + VOTE_EXECUTEDELAY );
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


/*
	Handler for g_gametype and setgametype vote
*/
static qboolean VoteH_Gametype( const gentity_t *ent ) {
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	int		i;


	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );


	/* Special: A variant of g_gametype vote */
	if ( Q_stricmp( arg1, "setgametype" ) == 0 ) {
		gametype_t gt = StringToGametype( ConcatArgs( 2 ) );

		if ( GT_MAX_GAME_TYPE == gt ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Couldn't find a gametype with the keywords.\n" );
			return qfalse;
		}

		/* Gametype found, now "convert" into g_gametype vote */
		Q_strncpyz( arg1, "g_gametype", sizeof( arg1 ) );
		Com_sprintf( arg2, sizeof( arg2 ), "%i", gt );
	}

	
	/* Now we have a g_gametype vote */
	i = atoi( arg2 );

	if( ( GT_SINGLE_PLAYER == i ) || ( i < GT_FFA ) || ( i >= GT_MAX_GAME_TYPE ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Invalid gametype.\n" );
		return qfalse;
	}

	if ( g_gametype.integer == i ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "This is the current gametype.\n" );
		return qfalse;
	}

	Com_sprintf( level.voteString, sizeof( level.voteString ), "set g_gametype %d", i );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), S_COLOR_YELLOW"Gametype '%s'", GametypeToString( i ) );

	return qtrue;
}


/*
	Handler for clientkick and kick vote
*/
static qboolean VoteH_Kick( const gentity_t *ent ) {
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	int		i;


	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );


	/* Special: A variant of clientkick vote */
	if ( Q_stricmp( arg1, "kick" ) == 0 ) {
		gclient_t	*player;
		char		cleanName[64];
		int			id = -1;

		/* try to circumvent renaming exploit and use id, which can not change */
		for ( i = 0; i < level.maxclients; i++ ) {
			player = &level.clients[i];

			if ( CON_DISCONNECTED == player->pers.connected ) {
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

		if ( -1 == id ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "No player found with that name. Check for typos or use 'clientkick' instead.\n" );
			return qfalse;
		}

		if ( player->pers.localClient ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "You can not kick the host player.\n" );
			return qfalse;
		}

		/* We've got one, convert into clientkick vote
		   TODO: Check for multiple matches
		   TODO: Display old and current name in votestring?
		*/
		Q_strncpyz( arg1, "clientkick", sizeof( arg1 ) );
		Com_sprintf( arg2, sizeof( arg2 ), "%i", id );
	}


	/* Now we have a clientkick vote */
	i = atoi( arg2 );

	if ( ( i < 0 ) || ( i >= MAX_CLIENTS ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Not a valid client number.\n" );
		return qfalse;
	}

	if ( CON_DISCONNECTED == level.clients[i].pers.connected ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Client not connected.\n" );
		return qfalse;
	}

	Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick \"%i\"", i );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), S_COLOR_YELLOW"Kick %i: '%s'", i, level.clients[i].pers.netname );

	/* Append additional argument, e.g. "reason" */
	if ( trap_Argc() >= 4 ) {
		char r[16];

		Q_strncpyz( r, ConcatArgs( 3 ), sizeof( r ) );
		Q_strcat( level.voteDisplayString, sizeof( level.voteDisplayString ), va( S_COLOR_WHITE", %s", r ) );
	}

	return qtrue;
}


/*
	Handler for map, nextmap and map_restart vote.
*/
static qboolean VoteH_Map( const gentity_t *ent ) {
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];


	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );


	if ( Q_stricmp( arg1, "map" ) == 0 ) {
		char s[128];
		
		/* Does map exist at all? */
		if ( trap_FS_FOpenFile( va( "maps/%s.bsp", arg2 ), NULL, FS_READ ) == -1 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "Map not found.\n" );
			return qfalse;
		}

		trap_Cvar_VariableStringBuffer( "mapname", s, sizeof( s ) );
		if ( Q_stricmp( arg2, s ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "This is the current map. Use 'map_restart' if you want to restart.\n" );
			return qfalse;
		}		
		
		/* Backup and re-apply current nextmap */
		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
		if ( *s ) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s; set nextmap \"%s\"", arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s", arg2 );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), S_COLOR_YELLOW"Map %s", arg2 );

		return qtrue;
	}
	else if ( Q_stricmp( arg1, "nextmap" ) == 0 ) {
		char s[128];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
		if ( !*s ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, "nextmap is not set on the server.\n" );
			return qfalse;
		}

		Q_strncpyz( level.voteString, "vstr nextmap", sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteDisplayString, S_COLOR_YELLOW"Next map", sizeof( level.voteDisplayString ) );

		return qtrue;
	}
	else if ( Q_stricmp( arg1, "map_restart" ) == 0 ) {
		Q_strncpyz( level.voteString, "map_restart", sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteDisplayString, S_COLOR_YELLOW"Restart map", sizeof( level.voteDisplayString ) );

		return qtrue;
	}

	/* Uh? */
	return qfalse;
}


/*
	Handler for any other vote
*/
static qboolean VoteH_Misc( const gentity_t *ent ) {
	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];
	int		i;


	trap_Argv( 1, arg1, sizeof( arg1 ) );
	trap_Argv( 2, arg2, sizeof( arg2 ) );


	if ( ( Q_stricmp( arg1, "pointlimit" ) == 0 ) ||
	     ( Q_stricmp( arg1, "timelimit" ) == 0 ) ) {

		i = atoi( arg2 );

		if ( i < 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( "Invalid %s.\n", arg1 ) );
			return qfalse;
		}

		if ( trap_Cvar_VariableIntegerValue( arg1 ) == i ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( "This is the current %s.\n", arg1 ) );
			return qfalse;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), S_COLOR_YELLOW"%s", level.voteString );

		return qtrue;
	}
	else {
		/* FIXME: This is ugly. I'd prefer some sort of array. Also re-enable g_doWarmup? */
		char *cmd;
		if ( Q_stricmp( arg1, "fastgamespeed" ) == 0 ) {
			cmd = "set g_speed 320";
		}
		else if ( Q_stricmp( arg1, "normalgamespeed" ) == 0 ) {
			cmd = ( "set g_speed "DEFAULT_GSPEED_STR );
		}
		else {
			G_Error( "VoteH_Misc: Unhandled vote %s\n", arg1 );
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s", cmd );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ), S_COLOR_YELLOW"%s", arg1 );

		return qtrue;
	}

}

