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

#include "g_local.h"


static void BE_Svcmd_Say_f( void );
static void BE_Svcmd_Cancelvote_f( void );
static void BE_Svcmd_ShuffleTeams_f( void );
static void BE_Svcmd_RenamePlayer_f( void );
static void BE_Svcmd_DropClient_f( void );
static void BE_Svcmd_ClientCommand_f( void );
static void BE_Svcmd_Callvote_f( void );
static void BE_Svcmd_LogPrint_f( void );
static void BE_Svcmd_RehashGUIDs_f( void );
static void BE_Svcmd_ListGUIDs_f( void );
static void BE_Svcmd_BanGUID_f( void );
static void BE_Svcmd_DelGUID_f( void );
static void BE_Svcmd_FlushGUIDs_f( void );
static void BE_Svcmd_SetHandicap_f( void );
static void BE_Svcmd_SetTeam_f( void );
static void BE_Svcmd_LockTeam_f( void );
static void BE_Svcmd_RunAs_f( void );
static void BE_Svcmd_PlaySound_f( void );
static void BE_Svcmd_Memory_f( void );
static void BE_Svcmd_Beryllium_f( void );
static void BE_Svcmd_QueryCvar_f( void );


/* FIXME: Add this to game headers? Declared in g_main.c */
int QDECL SortRanks( const void *a, const void *b );

const svcmd_t BE_SVCMDS[] = {
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
	{ "scallvote",		BE_Svcmd_Callvote_f			},
	{ "logprint",		BE_Svcmd_LogPrint_f			},
	{ "rehashguids",	BE_Svcmd_RehashGUIDs_f		},
	{ "listguids",		BE_Svcmd_ListGUIDs_f		},
	{ "banguid",		BE_Svcmd_BanGUID_f			},
	{ "delguid",		BE_Svcmd_DelGUID_f			},
	{ "flushguids",		BE_Svcmd_FlushGUIDs_f		},
	{ "handicap",		BE_Svcmd_SetHandicap_f		},
	{ "forceteam",		BE_Svcmd_SetTeam_f			},	/* NOTE: Override existing implementation */
	{ "lockteam",		BE_Svcmd_LockTeam_f			},
	{ "runas",			BE_Svcmd_RunAs_f			},
	{ "sound",			BE_Svcmd_PlaySound_f		},
	{ "memory",			BE_Svcmd_Memory_f			},
	{ "beryllium",		BE_Svcmd_Beryllium_f		},
	{ "querycvar",		BE_Svcmd_QueryCvar_f		}
};
const unsigned int NUM_SVCMDS = ARRAY_LEN( BE_SVCMDS );


/* FIXME: char clientStr[3] works for 2 digit clientnums with default MAX_CLIENTS 64 only */


qboolean BE_ConCmd( const char *cmd ) {
	unsigned int i;


	G_assert( cmd );


	for ( i = 0; i < NUM_SVCMDS; i++ ) {
		if ( Q_stricmp( cmd, BE_SVCMDS[i].cmdName ) == 0 ) {
			/* TODO: Since some handlers are registered for more than one command, we should pass
			         the actual command id as an argument so they don't need to Q_stricmp again.
			*/
			/* FIXME: assert( cmdHandler ) */
			BE_SVCMDS[i].cmdHandler();
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
/* TODO: G_assert() any pointer dereferencing */
static void BE_Svcmd_ShuffleTeams_f( void ) {
	int i, p, team;
	int count = 0;
	int	sortedClients[MAX_CLIENTS];
	gclient_t *cl;
	gentity_t *ent;


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
		ent = ( g_entities + sortedClients[i] );

		team = ( ( ( i + p ) % 2 ) + TEAM_RED );
		cl->sess.sessionTeam = team;

		/* NOTE: Needed to return team items such as lollies, as this will call Team_ReturnFlag() etc.  */
		/* FIXME: We should really use SetTeam() instead. Be aware of limits within the function! */
		cl->ps.stats[STAT_HEALTH] = ent->health = -999;
		player_die( ent, ent, ent, 100000, MOD_SUICIDE );

		ClientUserinfoChanged( sortedClients[i] );
		/* NOTE/FIXME: There are some clientside problems with base weapon after ClientBegin(),
		               see "srwc" commands in ClientSpawn(). To reproduce go into sprayroom and shuffleteams.
		               It works without the instant respawn due to ClientBegin(), since we are forcefully setting health.
		*/
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

	if ( !Q_isanumber( clientStr ) ) {
		G_Printf( "You must supply a client number.\n" );
		return;
	}

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
		G_Printf( "Usage: dropclient <cid> [reason]\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );

	if ( !Q_isanumber( clientStr ) ) {
		G_Printf( "You must supply a client number.\n" );
		return;
	}

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

		if ( ( g_gametype.integer < GT_TEAM ) && ( ( TEAM_RED == team ) || ( TEAM_BLUE == team ) ) ) {
			G_Printf( "Not a valid team in non-team gametype.\n" );
			return;		
		}
		else if ( TEAM_FREE == team ) {
			G_Printf( "Not a valid team in team gametype.\n" );
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

		if ( !Q_isanumber( arg ) ) {
			G_Printf( "You must supply a client number.\n" );
			return;
		}


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

	if ( !Q_isanumber( arg ) ) {
		G_Printf( "You must supply a client number.\n" );
		return;
	}

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


/*
	A way for external tools to add entries to the logfile
*/
static void BE_Svcmd_LogPrint_f( void ) {
	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage: logprint <text>\n" );
		return;
	}

	G_LogPrintf( "%s\n", ConcatArgs( 1 ) );
}


/*
	Reloads guid bans from disk, dropping current ones in memory
*/
static void BE_Svcmd_RehashGUIDs_f( void ) {
	BE_LoadBans();
}


/*
	Lists all guid bans
*/
static void BE_Svcmd_ListGUIDs_f( void ) {
	int index;

	for ( index = 0; index < numGUIDBans; index++ ) {
		G_Printf( "Ban #%d: %s\n", index, guidBans[ index ].guid );
	}
	G_Printf( "Total %d GUID bans.\n", index );
}


/*
	Adds a guid to the ban list
*/
static void BE_Svcmd_BanGUID_f( void ) {
	char		arg[MAX_STRING_CHARS];
	int			clientNum;
	guidBan_t	ban;

	if ( trap_Argc() != 2 ) {
		G_Printf( "Usage: banguid <cid|guid>\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if ( strlen( arg ) == ( GUIDSTRMAXLEN - 1 ) ) {
		if ( !validGUID( arg ) ) {
			G_Printf( "Not a valid GUID!\n" );
			return;
		}

		Q_strncpyz( ban.guid, arg, sizeof( ban.guid ) );
	}
	else {
		gentity_t *ent;

		if ( !Q_isanumber( arg ) ) {
			G_Printf( "You must supply a client number!\n" );
			return;
		}

		clientNum = atoi( arg );

		if ( !ValidClientID( clientNum, qfalse ) ) {
			G_Printf( "No a valid client number!\n" );
			return;
		}

		if ( level.clients[ clientNum ].pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client not connected!\n" );
			return;
		}

		ent = &( g_entities[ clientNum ] );
		if ( ent->r.svFlags & SVF_BOT ) {
			G_Printf( "Can not ban bots!\n" );
			return;
		}

		Q_strncpyz( ban.guid, level.clients[ clientNum ].pers.guid, sizeof( ban.guid ) );
	}

	if ( !AddBan( ban ) ) {
		G_Printf( "Error adding guid ban!\n" );
		return;
	}

	// TODO: Print info if already present?
	G_Printf( "Added guid ban.\n" );

	// TODO: Kick client(s) if connected?

	BE_WriteBans();
}


/*
	Deletes a guid from the ban list
*/
static void BE_Svcmd_DelGUID_f( void ) {
	char arg[MAX_STRING_CHARS];
	int banNum;
	int index;

	if ( trap_Argc() != 2 ) {
		G_Printf( "Usage: delguid <id|guid>\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if ( strlen( arg ) == ( GUIDSTRMAXLEN - 1 ) ) {
		index = 0;
		banNum = 0;
		while ( index < numGUIDBans ) {
			if ( Q_stricmp( guidBans[ index ].guid, arg ) == 0 ) {
				if ( !DeleteBan( index ) ) {
					G_Printf( "Could not delete guid ban %d!\n", banNum );
					// FIXME: G_Error()?
					return;
				}
				else {
					banNum++;
				}
			}
			else {
				index++;
			}
		}
		if ( banNum > 0 ) {
			G_Printf( "Deleted guid ban.\n" );
		}
		else {
			G_Printf( "GUID not in list.\n" );
			return;
		}
	}
	else {
		if ( !Q_isanumber( arg ) ) {
			G_Printf( "You must supply a guid ban number!\n" );
			return;
		}

		banNum = atoi( arg );

		if ( ( banNum < 0 ) || ( banNum >= numGUIDBans ) ) {
			G_Printf( "Invalid guid ban id %d!\n", banNum );
			return;
		}

		if ( !DeleteBan( banNum ) ) {
			G_Printf( "Could not delete guid ban %d!\n", banNum );
			// FIXME: Continue, G_Error(), ..?
			return;
		}
		
		G_Printf( "Deleted guid ban %d.\n", banNum );
	}

	BE_WriteBans();
}


/*
	Deletes all guid bans, also on disk
*/
static void BE_Svcmd_FlushGUIDs_f( void ) {
	numGUIDBans = 0;
	BE_WriteBans();

	G_Printf( "All GUID bans have been deleted.\n" );
}


/*
	Sets the handicap value of a player.
	TODO: Once forced by admin, disallow changes by player.
*/
static void BE_Svcmd_SetHandicap_f( void ) {
	char	clientStr[3], userinfo[MAX_INFO_STRING];
	int		clientNum, handicap;


	if ( trap_Argc() < 3 ) {
		G_Printf( "Usage: handicap <cid> <handicap>\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );

	if ( !Q_isanumber( clientStr ) ) {
		G_Printf( "You must supply a client number.\n" );
		return;
	}

	clientNum = atoi( clientStr );
	if ( !ValidClientID( clientNum, qfalse ) ) {
		G_Printf( "Not a valid client number.\n" );
		return;
	}

	if ( CON_DISCONNECTED == level.clients[clientNum].pers.connected ) {
		G_Printf( "Client not connected.\n" );
		return;
	}


	/* Abuse so far unused array */
	trap_Argv( 2, userinfo, sizeof( userinfo ) );

	if ( !Q_isanumber( userinfo ) ) {
		G_Printf( "You must supply a handicap number.\n" );
		return;
	}

	handicap = atoi( userinfo );
	if ( ( handicap < 1 ) || ( handicap > 100 ) ) {
		G_Printf( "Handicap must be in range 1-100.\n");
		return;
	}

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	Info_SetValueForKey( userinfo, "handicap", va( "%d", handicap ) );
	trap_SetUserinfo( clientNum, userinfo );

	/* NOTE: We could delay calling UserinfoChanged() and set maxHealth and
	         STAT_MAX_HEALTH ourselves, but we want the clients to display correct
	         handicap in scoreboard.
	*/
	ClientUserinfoChanged( clientNum );

	/* TODO: Print message to player! */
}


/*
	Forces the player into the given team, regardless of any limits.
*/
static void BE_Svcmd_SetTeam_f( void ) {
	char	clientStr[3], teamStr[16];
	int		clientNum;
	team_t	team;


	if ( trap_Argc() < 3 ) {
		G_Printf( "Usage: forceteam <cid> <team>\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );

	if ( !Q_isanumber( clientStr ) ) {
		G_Printf( "You must supply a client number.\n" );
		return;
	}

	clientNum = atoi( clientStr );
	if ( !ValidClientID( clientNum, qfalse ) ) {
		G_Printf( "Not a valid client number.\n" );
		return;
	}

	if ( CON_DISCONNECTED == level.clients[clientNum].pers.connected ) {
		G_Printf( "Client not connected.\n" );
		return;
	}


	trap_Argv( 2, teamStr, sizeof( teamStr ) );

	team = TeamFromString( teamStr );
	if ( TEAM_NUM_TEAMS == team ) {
		G_Printf( "Not a valid team.\n" );
		return;
	}

	if ( ( g_gametype.integer < GT_TEAM ) && ( ( TEAM_RED == team ) || ( TEAM_BLUE == team ) ) ) {
		G_Printf( "Not a valid team.\n" );
		return;		
	} 

	G_SetTeam( &g_entities[clientNum], (char*)TeamName( team ), qtrue );
}


/*
	Toggles whether team is locked.
*/
static void BE_Svcmd_LockTeam_f( void ) {
	char		teamStr[16];
	team_t		team;
	qboolean	lock;


	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage: lockteam <team>\n" );
		return;
	}

	trap_Argv( 1, teamStr, sizeof( teamStr ) );

	team = TeamFromString( teamStr );
	if ( TEAM_NUM_TEAMS == team ) {
		G_Printf( "Not a valid team.\n" );
		return;
	}

	if ( g_gametype.integer < GT_TEAM ) {
		if ( ( TEAM_RED == team ) || ( TEAM_BLUE == team ) ) {
			G_Printf( "Not a valid team in non-team gametype.\n" );
			return;
		}
	}
	else {
		if ( ( TEAM_FREE == team ) ) {
			G_Printf( "Not a valid team in team gametype.\n" );
			return;
		}
	}

	/* TODO: Does it make sense to lock spectators? */

	lock = !level.teamLocked[team];
	if ( lock ) {
		G_Printf( "Locking team.\n" );
	}
	else {
		G_Printf( "Unlocking team.\n" );
	}

	level.teamLocked[team] = lock;
}


/*
	Creates a command to be run by the specified client.
*/
static void BE_Svcmd_RunAs_f( void ) {
	char	clientStr[3], arg[MAX_STRING_CHARS];
	int		clientNum;


	if ( trap_Argc() < 3 ) {
		G_Printf( "Usage: runas <cid> <command>\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );

	if ( !Q_isanumber( clientStr ) ) {
		G_Printf( "You must supply a client number.\n" );
		return;
	}

	clientNum = atoi( clientStr );
	if ( !ValidClientID( clientNum, qfalse ) ) {
		G_Printf( "Not a valid client number.\n" );
		return;
	}

	if ( CON_DISCONNECTED == level.clients[clientNum].pers.connected ) {
		G_Printf( "Client not connected.\n" );
		return;
	}

	/* NOTE: We can not validate the command in any way.
	         It'd be possible to filter /disconnect and such, but well..
	*/
	/* Create a local copy */
	Com_sprintf( arg, sizeof( arg ), "%s", ConcatArgs( 2 ) );

	ExecuteClientCommand( clientNum, arg );	
}


/*
	Plays a sound to all players
*/
static void BE_Svcmd_PlaySound_f( void ) {
	char arg[MAX_STRING_CHARS];
	int soundIndex;

	if ( trap_Argc() < 2 ) {
		G_Printf( "Usage: sound <filename>\n" );
		return;
	}

	trap_Argv( 1, arg, sizeof( arg ) );

	if ( !fileExists( arg ) ) {
		G_Printf( "File not found.\n" );
		return;
	}

	soundIndex = G_SoundIndex( arg );
	PlayGlobalSound( soundIndex );
}


/*
	Prints information about beryllium's internal memory usage
*/
static void BE_Svcmd_Memory_f( void ) {
	BE_MemoryInfo();
}


/*
	Prints version information about beryllium.
*/
static void BE_Svcmd_Beryllium_f( void ) {
	char buf[MAX_STRING_CHARS] = { "" };

	Q_strcat(buf, sizeof( buf ), va( "%s\n", BERYLLIUM_ASCII_S ) );
	Q_strcat(buf, sizeof( buf ), va( S_COLOR_BLUE"version"S_COLOR_DEFAULT": "S_COLOR_CYAN"%s\n", BERYLLIUM_VERSION_S )  );
	Q_strcat(buf, sizeof( buf ), va( S_COLOR_BLUE"built"S_COLOR_DEFAULT":   "S_COLOR_CYAN"%s, %s\n", __DATE__, __TIME__ ) );
	Q_strcat(buf, sizeof( buf ), va( S_COLOR_BLUE"base"S_COLOR_DEFAULT":    "S_COLOR_CYAN"%s\n", G_VERSION_S ) );

	if ( !G_ColoredOutput() ) {
		Q_DecolorStr( buf, buf, sizeof( buf ) );
	}
	G_Printf( "%s", buf );
}


/*
	Sends a cvar query to a client
*/
static void BE_Svcmd_QueryCvar_f( void ) {
	char	clientStr[3], cmd[MAX_STRING_CHARS];
	int		clientNum;

	if ( trap_Argc() < 3 ) {
		G_Printf( "Usage: querycvar <cid> <cvar>\n" );
		return;
	}

	trap_Argv( 1, clientStr, sizeof( clientStr ) );

	if ( !Q_isanumber( clientStr ) ) {
		G_Printf( "You must supply a client number.\n" );
		return;
	}

	clientNum = atoi( clientStr );
	if ( !ValidClientID( clientNum, qfalse ) ) {
		G_Printf( "Not a valid client number.\n" );
		return;
	}

	if ( CON_DISCONNECTED == level.clients[clientNum].pers.connected ) {
		G_Printf( "Client not connected.\n" );
		return;
	}

	Com_sprintf( cmd, sizeof( cmd ), "qc %s", ConcatArgs( 2 ) );

	trap_SendServerCommand( clientNum, cmd );
}

