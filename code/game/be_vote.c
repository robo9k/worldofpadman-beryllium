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


/* TODO: Add an ID to every vote and return this instead of true/false for isAllowed/getVote.
         This removes all these for-loops and might even allow easy pluggable external
         help to be printed.
*/

static qboolean IsAllowedVote( const char *str ) {
	char needle[MAX_STRING_TOKENS];

	Com_sprintf( needle, sizeof( needle ), "/%s/", str );
	
	/* FIXME: Use case insensitive search? */
	return ( strstr( be_allowedVotes.string, needle ) != NULL );
}


/*
	Beryllium's replacement for Cmd_Vote_f() in g_cmds.c
*/
void BE_Cmd_Vote_f( const gentity_t *ent ) {
	char msg[8];


	if ( !level.voteTime ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"No vote in progress.\n" );
		return;
	}
	if ( ent->client->ps.eFlags & EF_VOTED ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Vote already cast.\n" );
		return;
	}
	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Not allowed to vote as spectator.\n" );
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
		/* TODO: Privacy? Cvar? Log? */
		/*
		SendClientCommand( -1, CCMD_PRT, va( "%s"S_COLOR_DEFAULT" voted yes.\n", ent->client->pers.netname ) );
		*/
	}
	else {
		level.voteNo++;
		trap_SetConfigstring( CS_VOTE_NO, va( "%i", level.voteNo ) );

		/* TODO: Privacy? Cvar? Log? */
		/*
		SendClientCommand( -1, CCMD_PRT, va( "%s"S_COLOR_DEFAULT" voted no.\n", ent->client->pers.netname ) );
		*/
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
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Voting not allowed here.\n" );
		return;
	}

	if ( level.voteTime ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"A vote is already in progress.\n" );
		return;
	}

	if ( ent->client->pers.voteCount >= MAX_VOTE_COUNT ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You have called the maximum number of votes.\n" );
		return;
	}

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Not allowed to call a vote as spectator.\n" );
		return;
	}

	if ( level.voteEnd &&
	     ( level.time - level.voteEnd ) <= ( be_votePause.integer * 1000 ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"One can not call a vote so short after the previous one.\n" );
		TimeToString( ( ( be_votePause.integer * 1000 ) - ( level.time - level.voteEnd ) ), arg1, sizeof( arg1 ) );
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( S_COLOR_ITALIC"You need to wait %s.\n", arg1 ) );
		return;
	}

	if ( ent->client->pers.voteTime &&
	     ( level.time - ent->client->pers.voteTime ) <= ( be_voteRate.integer * 1000 ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You can not call a new vote so short after your previous one.\n" );
		TimeToString( ( ( be_voteRate.integer * 1000 ) - ( level.time - ent->client->pers.voteTime ) ), arg1, sizeof( arg1 ) );
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( S_COLOR_ITALIC"You need to wait %s.\n", arg1 ) );
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
				SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Invalid vote string.\n" );
				return;
			break;
		}
	}

	if ( !IsAllowedVote( arg1 ) ) {
		/* TODO: Print allowed votes? At least at bottom of function if vote is invalid */
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Vote is not allowed.\n" );
		return;
	}

	
	/* if there is still a vote to be executed */
	if ( level.voteExecuteTime ) {
		level.voteExecuteTime = 0;
		/* TODO: Add a LogPrintf which states we are executing a vote now?
		         Votestring itself is already printed in callvote.
		*/
		trap_SendConsoleCommand( EXEC_APPEND, va( "%s\n", level.voteString ) );
	}


	for ( i = 0; i < NUM_VOTEH; i++ ) {
		if ( Q_stricmp( arg1, voteHandler[i].str ) == 0 ) {
			/* Legitimate vote
			   Any error handling, message printing etc must be done in VoteH_
			*/
			if ( voteHandler[i].cmdHandler( ent ) ) {

				/* FIXME: There is a problem clientside when displaying a votestring with double " inside.
				          voteDisplayString is also S_COLOR_ITALIC
				*/
				SendClientCommand( -1, CCMD_PRT, va( S_COLOR_ITALIC"%s"S_COLOR_ITALIC" called a vote: %s"S_COLOR_ITALIC".\n",
	                                                 ent->client->pers.netname, level.voteDisplayString ) );
				/* TODO: Create a seperate BE_Printf( type, level, text ).
				         Use voteString or voteDisplayString?
				*/
				G_LogPrintf( "callvote %i: '%s'\n", ( ent - g_entities ), level.voteString );


				/* start the voting, the caller autoamtically votes yes */
				level.voteTime = level.time;
				level.voteYes = 1;
				level.voteNo = 0;

				for ( i = 0 ; i < level.maxclients ; i++ ) {
					level.clients[i].ps.eFlags &= ~EF_VOTED;
				}
				ent->client->ps.eFlags |= EF_VOTED;
				ent->client->pers.voteCount++;

				ent->client->pers.voteTime = level.time;

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
		char validVoteString[MAX_STRING_TOKENS] = { S_COLOR_ITALIC"Valid and allowed vote commands are: " };
		for ( i = 0; i < NUM_VOTEH; i++ ) {
			if ( !IsAllowedVote( voteHandler[i].str ) ) {
				/* FIXME: Handle case when there are no allowed votes? Set g_allowVote 0? */
				continue;
			}			

			/* This is not actually "bold", but needs to differ from the "italic" hint */
			Q_strcat( validVoteString, sizeof( validVoteString ),
					  va( S_COLOR_BOLD"%s"S_COLOR_ITALIC", ", voteHandler[i].str ) );
		}
		/* Hackity, because I'm too lazy to detect last vote properly */
		Q_strncpyz( ( validVoteString + strlen( validVoteString ) - strlen( ", " ) ), ".\n" , ( strlen( ", " ) + 1 ) );


		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Not a valid vote string.\n" );
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

	/* TODO: Print numbers of yay-nay-rest? */
	if ( ( level.time - level.voteTime ) >= level.voteDuration ) {
		SendClientCommand( -1, CCMD_PRT, S_COLOR_ITALIC"Vote failed, timeout.\n" );
	}
	else {
		if ( level.voteYes > ( level.numVotingClients *  be_votePass.value ) ) {
			/* Set timeout, then execute and remove the vote at next call */
			SendClientCommand( -1, CCMD_PRT, S_COLOR_ITALIC"Vote passed.\n" );
			level.voteExecuteTime = ( level.time + VOTE_EXECUTEDELAY );
		}
		else if ( level.voteNo >= ( level.numVotingClients * ( 1.0 - be_votePass.value ) ) ) {
			/* same behavior as a timeout */
			SendClientCommand( -1, CCMD_PRT, S_COLOR_ITALIC"Vote failed.\n" );
		}
		else {
			/* still waiting for a majority */
			return;
		}
	}

	level.voteEnd = level.time;
	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );
	trap_SetConfigstring( CS_VOTE_STRING, "" );
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
		gametype_t gt;

		if ( strlen( arg2 ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You must supply a gametype keyword.\n" );
			return qfalse;
		}

		gt = StringToGametype( ConcatArgs( 2 ) );

		if ( GT_MAX_GAME_TYPE == gt ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Couldn't find a gametype with the keywords.\n" );
			return qfalse;
		}

		/* Gametype found, now "convert" into g_gametype vote */
		Q_strncpyz( arg1, "g_gametype", sizeof( arg1 ) );
		Com_sprintf( arg2, sizeof( arg2 ), "%i", gt );
	}

	
	/* Now we have a g_gametype vote */
	if ( strlen( arg2 ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You must supply a gametype number.\n" );
			return qfalse;
	}

	i = atoi( arg2 );

	if( ( GT_SINGLE_PLAYER == i ) || ( i < GT_FFA ) || ( i >= GT_MAX_GAME_TYPE ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Invalid gametype.\n" );
		return qfalse;
	}

	if ( g_gametype.integer == i ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"This is the current gametype.\n" );
		return qfalse;
	}

	Com_sprintf( level.voteString, sizeof( level.voteString ), "set g_gametype %d", i );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
	             S_COLOR_ITALIC"Gametype '%s'"S_COLOR_DEFAULT, GametypeToString( i ) );

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

		if ( strlen( arg2 ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You must supply a client name.\n" );
			return qfalse;
		}

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
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"No player found with that name. Check for typos or use 'clientkick' instead.\n" );
			return qfalse;
		}

		if ( player->pers.localClient ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You can not kick the host player.\n" );
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
	if ( strlen( arg2 ) == 0 ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You must supply a client number.\n" );
		return qfalse;
	}

	i = atoi( arg2 );

	if ( ( i < 0 ) || ( i >= MAX_CLIENTS ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Not a valid client number.\n" );
		return qfalse;
	}

	if ( CON_DISCONNECTED == level.clients[i].pers.connected ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Client not connected.\n" );
		return qfalse;
	}

	if ( ( ent - g_entities ) == i ) {
		SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You can not kick yourself.\n" );
		return qfalse;
	}

	Com_sprintf( level.voteString, sizeof( level.voteString ), "clientkick \"%i\"", i );
	Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
	             S_COLOR_ITALIC"Kick %i: '%s'"S_COLOR_DEFAULT, i, level.clients[i].pers.netname );

	/* Append additional argument, e.g. "reason" */
	if ( trap_Argc() >= 4 ) {
		char r[16];

		Q_strncpyz( r, ConcatArgs( 3 ), sizeof( r ) );
		Q_strcat( level.voteDisplayString, sizeof( level.voteDisplayString ), va( S_COLOR_ITALIC", %s"S_COLOR_DEFAULT, r ) );
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

		if ( strlen( arg2 ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You must supply a map name.\n" );
			return qfalse;
		}

		/* Does map exist at all? */
		if ( !trap_FS_FOpenFile( va( "maps/%s.bsp", arg2 ), NULL, FS_READ ) ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Map not found.\n" );
			return qfalse;
		}

		trap_Cvar_VariableStringBuffer( "mapname", s, sizeof( s ) );
		if ( Q_stricmp( arg2, s ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"This is the current map. Use 'map_restart' if you want to restart.\n" );
			return qfalse;
		}		
		
		/* Backup and re-apply current nextmap */
		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
		if ( *s ) {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s; set nextmap \"%s\"", arg2, s );
		} else {
			Com_sprintf( level.voteString, sizeof( level.voteString ), "map %s", arg2 );
		}
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
		             S_COLOR_ITALIC"Map %s"S_COLOR_DEFAULT, arg2 );

		return qtrue;
	}
	else if ( Q_stricmp( arg1, "nextmap" ) == 0 ) {
		char s[128];

		trap_Cvar_VariableStringBuffer( "nextmap", s, sizeof( s ) );
		if ( !*s ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"nextmap is not set on the server.\n" );
			return qfalse;
		}

		Q_strncpyz( level.voteString, "vstr nextmap", sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteDisplayString,
		            S_COLOR_ITALIC"Next map"S_COLOR_DEFAULT, sizeof( level.voteDisplayString ) );

		return qtrue;
	}
	else if ( Q_stricmp( arg1, "map_restart" ) == 0 ) {
		Q_strncpyz( level.voteString, "map_restart", sizeof( level.voteDisplayString ) );
		Q_strncpyz( level.voteDisplayString,
		            S_COLOR_ITALIC"Restart map"S_COLOR_DEFAULT, sizeof( level.voteDisplayString ) );

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

		if ( strlen( arg2 ) == 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( S_COLOR_NEGATIVE"You must supply a %s.\n", arg1 ) );
			return qfalse;
		}

		i = atoi( arg2 );

		if ( i < 0 ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( S_COLOR_NEGATIVE"Invalid %s.\n", arg1 ) );
			return qfalse;
		}

		if ( trap_Cvar_VariableIntegerValue( arg1 ) == i ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( S_COLOR_NEGATIVE"This is the current %s.\n", arg1 ) );
			return qfalse;
		}

		Com_sprintf( level.voteString, sizeof( level.voteString ), "%s %i", arg1, i );
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
		             S_COLOR_ITALIC"%s"S_COLOR_DEFAULT, level.voteString );

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
		Com_sprintf( level.voteDisplayString, sizeof( level.voteDisplayString ),
		             S_COLOR_ITALIC"%s"S_COLOR_DEFAULT, arg1 );

		return qtrue;
	}

}

