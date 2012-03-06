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


int		numSecrets;
char	secretNames[MAX_SECRETS][MAX_TARGETNAME];

int			numGUIDBans;
guidBan_t	guidBans[MAX_GUIDBANS];

static char	*pWorldspawn, *pWorldspawnParsePoint;


/* Functions */

/*
	Hooks into ClientCommand() in g_cmds.c
	We can catch any client command here and even override default ones,
	since our function is called before original one.
	We already have a ent->client.
	If we return qtrue, the original function will return immediatelly.
*/
qboolean BE_ClientCommand( gentity_t *ent, const char *cmd ) {
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
	Hooks midway into G_Damage() in g_combat.c.
	TODO: Create separate functions which handle knockback and radius damage
*/
void BE_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int *damage, int *dflags, int *mod ) {
	G_assert( targ != NULL );
	G_assert( attacker != NULL );
	G_assert( mod  != NULL );
	G_assert( damage != NULL );


	/* We are only interested in clients' damage */
	if ( !( targ->client ) || !( attacker->client ) ) {
		return;
	}

	
	G_assert( mod != NULL );
	G_assert( damage != NULL );


	/* Respawn protection */
	/* TODO: Visualise this by making weapons non-functional? Or simply centerprint countdown?
	         Remove protection if weapon fired!
	*/
	/* TODO: Add MOD_WATER, MOD_SLIME, MOD_LAVA? */
	if ( ( MOD_TRIGGER_HURT != *mod ) && ( targ != attacker ) ) {
		if ( ( level.time - attacker->client->respawnTime ) <= ( be_respawnProtect.integer * 1000 ) ) {	
			*damage = 0;
		}
		if ( ( level.time - targ->client->respawnTime ) <= ( be_respawnProtect.integer * 1000 ) ) {
			*damage = 0;
		}
	}

}


/*
	Hooks into ClientUserinfoChanged() in g_client.c
	FIXME: ClientUserinfoChanged() might get called excessivly and each time
           it will print the full userinfo to log. This not quite wanted behaviour.
           Maybe cache userinfo for each client and strcmp(), then return from
           original function if no changes?
*/
void BE_ClientUserinfoChanged( int clientNum ) {
	char userinfo[MAX_INFO_STRING];
	qboolean changed = qfalse;
	gentity_t *ent = ( g_entities + clientNum );
	int i;
	char name[MAX_NETNAME], oldname[MAX_NETNAME];
	char model[MAX_QPATH], headmodel[MAX_QPATH], *modelInfo, *headmodelInfo;


	G_assert( ( 0 <= clientNum ) && ( MAX_CLIENTS > clientNum ) ); /* FIXME: ValidClientID()? */


	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );


	/* Fix wrong player skins */
	/* NOTE: Clients who have cg_forceModel set might still see wrong models, since we
	         can not change their clientside setting (needs clientside fixing - should use userinfo as well ).
	*/
	if ( g_gametype.integer >= GT_TEAM ) {
		modelInfo		= "team_model";
		headmodelInfo	= "team_headmodel";
	}
	else {
		modelInfo		= "model";
		headmodelInfo	= "headmodel";
	}

	Q_strncpyz( model, Info_ValueForKey( userinfo, modelInfo ), sizeof( model ) );
	Q_strncpyz( headmodel, Info_ValueForKey( userinfo, headmodelInfo ), sizeof( headmodel ) );

	if ( !validPlayermodel( model, headmodel ) ) {
		/* TODO: Add a default model for teamgames? */
		Info_SetValueForKey( userinfo, modelInfo, DEFAULT_PLAYERMODEL_S );
		Info_SetValueForKey( userinfo, headmodelInfo, DEFAULT_PLAYERMODEL_S );
		changed = qtrue;
	}


	/* Used twice below */
	ClientCleanName( Info_ValueForKey( userinfo, "name" ), name, sizeof( name ) );
	Q_strncpyz( oldname, ent->client->pers.netname, sizeof( oldname ) );


	/* Limit renaming */
	if ( strcmp( name, oldname ) != 0 ) {
		/* FIXME: Needs some better value for default infinite behaviour, at least some #define */
		if ( ( be_maxNameChanges.integer != -1 ) &&
		     ( ent->client->pers.nameChanges >= be_maxNameChanges.integer ) ) {
			/* FIXME: Only print (and revert) when connected?
			          Also don't print when from BE_Svcmd_RenamePlayer_f() but directly print there.
			*/
			SendClientCommand( clientNum, CCMD_PRT, va( S_COLOR_NEGATIVE"You have reached the maximum number of %i renames.\n", be_maxNameChanges.integer ) );
			/* NOTE: We only set our internal copies, thus multiple name check has to be after rename limit
			         to actually revert name by copying into userinfo and netname.
			*/
			Q_strncpyz( name, oldname, sizeof( name ) );
			Info_SetValueForKey( userinfo, "name", oldname );
			changed = qtrue;
		}

		if ( CON_CONNECTED == ent->client->pers.connected ) {
			/* TODO: nameChangeTime is unused at the moment. Use in conjunction with floodlimit? */
			ent->client->pers.nameChangeTime = level.time;
        	ent->client->pers.nameChanges++;
		}
	}


	/* Fix multiple names */
	/* NOTE: Since we only mess with userinfo, the clientside idea of its name remains the same. */	
	/* FIXME: This is hax! ClientnumFromString() depends on netname, but it will only be set
	          after the original ClientUserinfoChanged() returns. So we need to set and reset
	          to the new name in userinfo.
	*/
	if ( !( ent->r.svFlags & SVF_BOT ) && ( be_settings.integer & BE_UNIQUENAMES ) ) {
		Q_strncpyz( ent->client->pers.netname, name, sizeof( ent->client->pers.netname ) );
		if ( ClientnumFromString( name ) == CID_MULTIPLE ) {
			char newname[MAX_NETNAME];
			for ( i = 0; i < level.maxclients; i++ ) {
				Com_sprintf( newname, sizeof( newname ), RENAMED_PLAYERNAME_S" %i", ( i + 1 ) );
				if ( ClientnumFromString( newname ) == CID_NONE ) {
					Info_SetValueForKey( userinfo, "name", newname );
					changed = qtrue;
					Q_strncpyz( ent->client->pers.netname, newname, sizeof( ent->client->pers.netname ) );
					/* NOTE: This happens a lot while client connects, only print when intentionally renaming in-game.
					         Also note that original ClientUserinfoChanged() will not detect this renaming, so we "need" to print some info anyways.
					*/
					if ( ( CON_CONNECTED == ent->client->pers.connected ) &&
					     Q_stricmp( oldname, newname ) ) {
						SendClientCommand( CID_ALL, CCMD_PRT,
						                   va( S_COLOR_ITALIC"%s"S_COLOR_ITALIC" was automatically renamed to %s"S_COLOR_ITALIC".\n",
						                       oldname, newname ) );
					}
					break;
				}
			}
		}
		else {
			/* Revert hax above to comply with original ClientUserinfoChanged() rename behavior */
			Q_strncpyz( ent->client->pers.netname, oldname, sizeof( ent->client->pers.netname ) );
		}

	}


	/* TODO: Check whether client wants to change ip or guid, then drop him? */


	if ( changed ) {
		trap_SetUserinfo( clientNum , userinfo );
	}
}


/*
	Returns whether the given guid is banned due to ban list
*/
static qboolean GUIDBanned( const char *guid ) {
	int index;


	if ( !guid ) {
		// If no guid is supplied, let them pass. GUIDCHECK_EMPTY is supposed to check this
		return qfalse;
	}

	for ( index = 0; index < numGUIDBans; index++ ) {
		if ( Q_stricmp( guidBans[ index ].guid, guid ) == 0 ) {
			return qtrue;
		}
	}

	return qfalse;
}


/*
	Hooks early into ClientConnect() in g_client.c.
	If we return NULL, ClientConnect() continues, otherwise client will be
	rejected with our return value.
*/
/* FIXME: Call trap_DropClient() rather than having them trying to connect continously? */
char *BE_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char userinfo[MAX_INFO_STRING];
	char *value;
	char ip[NET_ADDRSTRMAXLEN], guid[GUIDSTRMAXLEN];


	G_assert( ( 0 <= clientNum ) && ( MAX_CLIENTS > clientNum ) ); /* FIXME: ValidClientID()? */


	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	/* check for malformed or illegal info strings */
	if ( !Info_Validate( userinfo ) ) {
		return "Invalid userinfo.";
	}

	/* All other checks are only reasonable against "humans" */	
	if ( isBot ) {
		return NULL;
	}


	value = Info_ValueForKey( userinfo, "ip" );
	Q_strncpyz( ip, value, sizeof( ip ) );
	/* NOTE: Do not strip port anymore, see NOTE about consistency below */
  

	if ( !*ip ) {
    	return "No IP in userinfo.";
	}
	/* "127.0.0.1", "::1" or the like */
	/* NOTE: Other possible values "bot", "localhost". But these don't occur, since
	         we check for isBot and "localhost" luckily passes. We might also want to
	         check number of dots (beware of IPv6)?
	*/
	if ( strlen( ip ) < 3 ) {
		return "Invalid IP in userinfo.";
	}


	if ( be_maxConnections.integer ) {
		gclient_t *other;
		int i, count = 0;

		for ( i = 0 ; i < level.maxclients; i++ ) {
			other = &level.clients[i];
			/* NOTE: CON_CONNECTING is only set in original ClientConnect() lateron */
			if ( other && ( CON_DISCONNECTED != other->pers.connected ) ) {
				/* NOTE: For proper counting below ensure that ip is saved consistently,
				         i.e. without :port!
				         IPv6 addresses include more than one ":". Also note that
				         recent versions of ioquake do not include the port in ip
				         userinfo field anymore.
				*/
				if ( strcmp( ip, other->pers.ip ) == 0 ) {
					count++;
				}
			}
		}

		if ( count >= be_maxConnections.integer ) {
			return "Maximum simultanous connections per IP exceeded.";
		}
	}

	value = Info_ValueForKey( userinfo, "cl_guid" );
  	Q_strncpyz( guid, value, sizeof( guid ) );

	if ( be_checkGUIDs.integer ) {
		/* TODO: Add GUIDCHECK_MULTIPLE? */

		if ( *guid && ( be_checkGUIDs.integer & GUIDCHECK_FORMAT ) ) {
			if ( !validGUID( guid ) ) {
				return "Invalid GUID in userinfo.";
			}
		}
		else if ( be_checkGUIDs.integer & GUIDCHECK_EMPTY ) {
			/* NOTE: This happens with bad installations (no rw on wopkey).
			         So maybe we should return a more helpfull "error message"?
			*/
			return "No GUID in userinfo.";
		}
	}

	// Check against banlist now that guid was most likely validated
	if ( GUIDBanned( guid ) ) {
		return "You are banned from this server.";
	}


	/* NOTE: We can not set any data for the client, since original ClientConnect()
	         will erase the whole client struct with memset() after we return.
	*/

	return NULL;
}


/*
	Called once a second for each alive client
*/
void BE_ClientTimerActions( gentity_t* ent ) {
	vec3_t	position;
	int counter, remaining;


	G_assert( ent != NULL );


	if ( CON_CONNECTED != ent->client->pers.connected ) {
		return;
	}
	if ( TEAM_SPECTATOR == ent->client->sess.sessionTeam ) {
		return;
	}


	/* Anti-Camping, check how far we moved */
	VectorCopy( ent->client->ps.origin, position );
	if ( DistanceSquared( ent->client->pers.campPosition, position ) < Square( be_campDistance.value ) ) {
		ent->client->pers.campCounter++;
	}
	else {
		ent->client->pers.campCounter = 0;
	}
	VectorCopy( position, ent->client->pers.campPosition );

	counter = ent->client->pers.campCounter;
	remaining = ( MAX_CAMPTIME - counter );
	/* TODO: Make the countdown configurable via cvar? */
	if ( ( remaining <= 10 ) && ( remaining > 5 ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_CP, S_COLOR_ITALIC"Camping will get you killed.." );
	}
	else if ( ( remaining <= 5 ) && ( remaining >= 1 ) ) {
		SendClientCommand( ( ent - g_entities ), CCMD_CP, S_COLOR_BOLD"Get moving!" );
	}

	if ( remaining <= 0 ) {
		/* Let them commit suicide! */
		G_Damage( ent, NULL, ent, NULL, NULL, 3000, 0, MOD_UNKNOWN );
	}


	/* Check ping and connection, then drop client when too bad */
	/* FIXME: Write some sort of wrapper for DropClient()? */
	/* FIXME: Can ping ever be < 0? Need to add test whether sv_minPing is set */
	counter = ( ( ent->client->pers.realPing < 999 ) ? ent->client->pers.realPing : 999 );
	remaining = trap_Cvar_VariableIntegerValue( "sv_maxping" );
	if ( remaining && ( counter > remaining ) ) {
		ent->client->pers.connectionCounter++;
	}
	else if ( counter < trap_Cvar_VariableIntegerValue( "sv_minping" ) ) {
		ent->client->pers.connectionCounter++;
	}
	else if ( ent->s.eFlags & EF_CONNECTION ) {
		ent->client->pers.connectionCounter++;
	}
	else {
		ent->client->pers.connectionCounter = 0;
	}

	/* TODO: Print a warning */
	if ( be_checkPings.integer && ( ent->client->pers.connectionCounter > be_checkPings.integer ) ) {
		trap_DropClient( ( ent - g_entities ), "Bad connection" );
	}
}


/*
	Called at the end of ClientBegin() but before ranks are calculated
*/
void BE_ClientBegan( int clientNum ) {
	gentity_t *ent;
	char userinfo[MAX_INFO_STRING];
	char *value;
	int  smoothClients;


	G_assert( ( 0 <= clientNum ) && ( MAX_CLIENTS > clientNum ) ); /* FIXME: ValidClientID()? */


	ent = &g_entities[clientNum];

	ent->client->pers.lifeShards = 0;

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	if ( !ent->client->storage.sawGreeting ) {
		ent->client->storage.sawGreeting = qtrue;

		/* Do some silent self-advertisement */
		if ( ent->client->storage.firstTime ) {
			SendClientCommand( clientNum, CCMD_PRT,
			                   va( SKIPNOTIFY_S"This server is running "S_COLOR_BLUE"beryllium"S_COLOR_CYAN" "BERYLLIUM_VERSION S_COLOR_DEFAULT".\n" ) );
		}
	}

	/* Warn if cg_smoothClients is set, since unlagged does not use it */
	value = Info_ValueForKey( userinfo, "cg_smoothClients" );
	smoothClients = atoi( value );	
	if ( smoothClients ) {
		G_Say( NULL, ent, SAY_TELL,
		       S_COLOR_NEGATIVE"Please disable "S_COLOR_BOLD"cg_smoothClients" S_COLOR_NEGATIVE", as it may cause bugs with unlagged!" );
	}
}


/*
	Called after client has been killed but before tossing items
*/
void BE_ClientKilled( gentity_t *self ) {
	G_assert( self != NULL );


	if ( ( GT_LPS == g_gametype.integer ) &&
	     be_oneUp.integer && !level.warmupTime ) {
		gentity_t	*killer_ent;
		gclient_t	*killer;
		int			numShards, neededShards;

		killer_ent = &g_entities[ self->client->lasthurt_client ];
		
		if ( killer_ent->client && ( killer_ent != self ) ) {
			killer = killer_ent->client;

			killer->pers.lifeShards++;

			numShards		= killer->pers.lifeShards;
			neededShards	= be_oneUp.integer;
			if ( numShards >= neededShards ) {
				/* NOTE: AWARD_IMPRESSIVE has no clientside code */
				SetAward( killer , AWARD_EXCELLENT );

				killer->sess.livesleft++;
				CalculateRanks();
				SendScoreboardMessageToAllClients();
				killer->pers.lifeShards = 0;

				SendClientCommand( CID_ALL, CCMD_PRT, va( "%s"S_COLOR_DEFAULT" gained an extra life!\n", killer->pers.netname ) );
			}
			else {
				SendClientCommand( ( killer_ent - g_entities ), CCMD_PRT, va( "You gained a life shard (%i of %i).\n", numShards, neededShards ) );
			}
		}
	}
}


/*
	Called at the very end of ClientDisconnect()
*/
void BE_ClientDisconnect( int clientNum ) {
	gentity_t *ent, *player;
	int i;


	G_assert( ( 0 <= clientNum ) && ( MAX_CLIENTS > clientNum ) ); /* FIXME: ValidClientID()? */


	ent = ( g_entities + clientNum );
	for ( i = 0; i < level.maxclients; i++ ) {
		player = ( g_entities + i );
		
		/* Make sure new connections do not get ignored */
		IgnoreChat( player, ent, qfalse );
	}
}


/*
	Sets whether ent is ignoring other
*/
void IgnoreChat( gentity_t *ent, const gentity_t *other, qboolean mode ) {
	int clientNum;
	char cmd[MAX_STRING_TOKENS];


	G_assert( ent != NULL );
	G_assert( other != NULL );


	clientNum = ( other - g_entities );

	ent->client->storage.ignoreList[clientNum] = mode;

	switch ( mode ) {
		case qtrue:
			Com_sprintf( cmd, sizeof( cmd ), "voip ignore %d\n", clientNum );
			break;
		default:
			Com_sprintf( cmd, sizeof( cmd ), "voip unignore %d\n", clientNum );
			break;
	}

	/* NOTE: *_voip is meant as a version number, so do not check for == 1 */
	if ( trap_Cvar_VariableIntegerValue( "sv_voip" ) ) {
		/* TODO: Check whether client has voip at all? See userinfo "cl_voip". */
		/* TODO: Test whether client is bot, is connected etc. */
		ExecuteClientCommand( ( ent - g_entities ), cmd );
	}
}


/*
	Returns whether ent is ignoring other
*/
qboolean ChatIgnored( const gentity_t *ent, const gentity_t *other ) {
	int clientNum;


	G_assert( ent != NULL );
	G_assert( other != NULL );


	clientNum = ( other - g_entities );

	return ( ent->client->storage.ignoreList[clientNum] );
}


/*
	Determines whether two players can chat with each other.
	This is called after all the original checks in G_SayTo()
*/
qboolean BE_CanSayTo( const gentity_t *ent, const gentity_t *other ) {
	/* NOTE: ent can be NULL for server messages */
	G_assert( other != NULL );


	if ( ent ) {
		return ( ChatIgnored( other, ent ) == qfalse );
	}

	return qtrue;
}


/*
	Parses the given buffer for secrets :)
*/
static void ParseSecrets( char *buff ) {
	const char	*token;
	char		*target;

	while ( qtrue ) {
		token = COM_Parse( &buff );
		if ( !*token ) {
			break;
		}

		if ( Q_stricmp( token, "secret" ) == 0 ) {
			if ( numSecrets >= MAX_SECRETS ) {
				G_Printf( BE_LOG_PREFIX"Secrets limit of %d exceeded.\n", MAX_SECRETS );
				break;
			}

			/* TODO: Use G_Alloc or another way of "dynamic" memory */

			target = secretNames[ numSecrets++ ];

			token = COM_Parse( &buff );
			if ( !*token ) {
				COM_ParseWarning( "Missing target name for secret." );
				break;
			}

			/* FIXME: Use sizeof */
			Q_strncpyz( target, token, MAX_TARGETNAME );
		}		
	}

	/* TODO: Validate each target name by parsing actual entities in map? */

	G_Printf( BE_LOG_PREFIX"Parsed %d secret%s.\n", numSecrets, ( numSecrets != 1 ? "s" : "" ) );
}


/*
	Loads secrets for be_noSecrets from a file specific for current map
*/
static void BE_LoadSecrets( void ) {
	int				len;
	fileHandle_t	f;
	char			*buff, *buffPtr;
	char			filename[MAX_QPATH];


	/* Don't even bother parsing if disabled. This is CVAR_LATCH, don't worry */
	if ( !be_noSecrets.integer ) {
		return;
	}

	Com_sprintf( filename, sizeof( filename ), "maps/%s.cfg", level.mapname );
	
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		G_Printf( BE_LOG_PREFIX"Could not open \"%s\".\n", filename );
		return;
	}

	buff = buffPtr = BE_Alloc( len );
	G_assert( buff );

	trap_FS_Read( buff, len, f );
	buff[len] = '\0';
	trap_FS_FCloseFile( f );

	COM_BeginParseSession( filename );
	ParseSecrets( buffPtr );

	BE_Free( buff );
}


/*
	Adds a new ban to memory list
*/
qboolean AddBan( guidBan_t ban ) {
	int index;

	if ( numGUIDBans > ARRAY_LEN( guidBans ) ) {
		return qfalse;
	}

	// check whether it's already in list
	for ( index = 0; index < numGUIDBans; index++ ) {
		if ( Q_stricmp( guidBans[ index ].guid, ban.guid ) == 0 ) {
			return qtrue;
		}
	}

	Q_strncpyz( guidBans[ numGUIDBans ].guid, ban.guid, sizeof( guidBans[ numGUIDBans ].guid ) );
	numGUIDBans++;

	return qtrue;
}


/*
	Deletes the given guid ban from memory
*/
qboolean DeleteBan( unsigned int index ) {
	if ( index == ( numGUIDBans - 1 ) ) {
		numGUIDBans--;
		return qtrue;
	}
	else if ( index < ( ARRAY_LEN( guidBans ) - 1 ) ) {
		memmove( ( guidBans + index ), ( guidBans + index + 1 ), ( ( numGUIDBans - index - 1 ) * sizeof( *guidBans ) ) );
		numGUIDBans--;
		return qtrue;
	}

	// Not present or invalid index
	return qfalse;
}


/*
	Writes all guid bans from memory to disk
*/
void BE_WriteBans( void ) {
	int				index, len;
	fileHandle_t	f;
	char			*filename;
	char			buff[GUIDSTRMAXLEN+1];

	filename = be_banFile.string;
	if ( !filename[0] ) {
		/* FIXME: Hardcoded, cvarTable_t.cvarName */
		G_Printf( "be_banFile not set, will not save to disk.\n" );
		return;
	}

	len = trap_FS_FOpenFile( filename, &f, FS_WRITE );
	if ( !f ) {
		G_Printf( BE_LOG_PREFIX"Could not open \"%s\".\n", filename );
		return;	
	}

	for ( index = 0; index < numGUIDBans; index++ ) {
		Com_sprintf( buff, sizeof( buff ), "%s\n", guidBans[ index ].guid );
		len = strlen( buff );
		trap_FS_Write( buff, len, f );		
	}
	trap_FS_FCloseFile( f );
}

/*
	Parses the given buffer for guid bans
*/
static void ParseBans( char *buff ) {
	const char	*token;
	guidBan_t	ban;


	numGUIDBans = 0;

	while ( qtrue ) {
		token = COM_Parse( &buff );
		if ( !token[0] ) {
			break;
		}

		if ( !validGUID( token ) ) {
			COM_ParseWarning( "Not a valid GUID %s", token );
			continue;
		}

		if ( numGUIDBans >= MAX_GUIDBANS ) {
			G_Printf( BE_LOG_PREFIX"GUID bans limit of %d exceeded.\n", MAX_GUIDBANS );
			break;
		}

		Q_strncpyz( ban.guid, token, sizeof( ban.guid ) );
		// TODO: Check return value
		AddBan( ban );
	}

	G_Printf( BE_LOG_PREFIX"Parsed %d GUID ban%s.\n", numGUIDBans, ( numGUIDBans != 1 ? "s" : "" ) );
}


/*
	Loads GUID bans from disk
*/
void BE_LoadBans( void ) {
	int				len;
	fileHandle_t	f;
	char			*buff, *buffPtr;
	char			*filename;

	filename = be_banFile.string;
	if ( !filename[0] ) {
		/* FIXME: Hardcoded, cvarTable_t.cvarName */
		G_Printf( BE_LOG_PREFIX"be_banFile not set, will not read banlist from disk.\n" );
		return;
	}

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		G_Printf( BE_LOG_PREFIX"Could not open \"%s\".\n", filename );
		return;	
	}

	buff = buffPtr = BE_Alloc( len );
	G_assert( buff != NULL );

	trap_FS_Read( buff, len, f );
	buff[len] = '\0';
	trap_FS_FCloseFile( f );

	COM_BeginParseSession( filename );
	ParseBans( buffPtr );

	BE_Free( buff );
}


/*
	Called at end of G_InitGame().
	Initialize structures specific to beryllium here
*/
void BE_InitBeryllium( void ) {
	char serverinfo[MAX_INFO_STRING];


	trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );

	/* NOTE: This is used in several places in original code, but strage enough never cached */
	Q_strncpyz( level.mapname, Info_ValueForKey( serverinfo, "mapname" ), sizeof( level.mapname ) );

	/* NOTE: Function decides whether to actually load anything */
	BE_LoadSecrets();
	BE_LoadBans();
}


/*
	Determines whether the given teleporter can be used.
	if not, will spawn the player at a random position instead of original target.
*/
qboolean BE_CanUseTeleporter( const gentity_t *ent, gentity_t *other ) {
	if ( be_noSecrets.integer ) {
		int i;
		vec3_t	origin, angles;
		qboolean allowed = qtrue;


		G_assert( ent != NULL);


		for ( i = 0; i < numSecrets; i++ ) {
			if ( Q_stricmp( ent->target, secretNames[i] ) == 0 ) {
				allowed = qfalse;
				break;
			}
		}

		if ( !allowed ) {
			G_assert( other != NULL );


			/* TODO: Print some info to the player? */
			SelectSpawnPoint( other->r.currentOrigin, origin, angles, qfalse );
			TeleportPlayer( other, origin, angles );
		}
		
		return allowed;
	}

	return qtrue;
}


/*
	Returns whether the mover can be used/shot
*/
qboolean BE_CanUseMover( const gentity_t *ent, gentity_t *other ) {
	if ( be_noSecrets.integer ) {
		int i;


		G_assert( ent != NULL );


		for ( i = 0; i < numSecrets; i++ ) {
			if ( Q_stricmp( ent->target, secretNames[i] ) == 0 ) {
				return qfalse;
			}
		}
	}

	return qtrue;
}


/*
	Decides whether to "hide" the given chat.
	The chatline has already been written to gamelog.
*/
qboolean BE_HideChat( gentity_t *ent, gentity_t *target, int mode, int color, const char *name, const char *message ) {
	int len;


	G_assert( message != NULL );


	len = strlen( be_hideChat.string );
	/* Only filter player messages, not those from server chat */
	if ( ( len > 0 ) && ent ) {
		if ( Q_strncmp( be_hideChat.string, message, len ) == 0 ) {
			/* Echo back to author, so it does not seem "lost" */
			G_SayTo( ent, ent, mode, /*color*/COLOR_WHITE, name, message );

			return qtrue;
		}		
	}

	return qfalse;
}


/*
	TODO: Should go into g_utils.c?
	Returns the number of msecs of one server frame, see SV_FrameMsec()
*/
int G_FrameMsec( void ) {
	int fps = trap_Cvar_VariableIntegerValue( "sv_fps" );

	if ( fps ) {
		int frameMsec = ( 1000.0 / fps );

		return frameMsec;
	}
	else {
		return 1;
	}
}


static void BE_LoadEntities( const char *mapname ) {
	int				len;
	fileHandle_t	f;
	char			filename[MAX_QPATH];


	G_assert( mapname );

	pWorldspawn = NULL;
	if ( !be_overrideEntities.integer ) {
		/* Make sure we've NULLed pWorldspawn, since it's used in all other checks */
		return;
	}

	Com_sprintf( filename, sizeof( filename ), "maps/%s.dat", mapname );

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		G_Printf( BE_LOG_PREFIX"Could not open \"%s\", will fallback to .bsp entities.\n", filename );
		return;	
	}

	/* We need a second pointer because COM_Parse() will overwrite and thus
	   leak our buffer pointer otherwise.
	*/
	pWorldspawn = pWorldspawnParsePoint = BE_Alloc( len );
	G_assert( pWorldspawn != NULL );

	trap_FS_Read( pWorldspawn, len, f );
	pWorldspawn[len] = '\0';
	trap_FS_FCloseFile( f );

	G_Printf( BE_LOG_PREFIX"Loaded entities from \"%s\".\n", filename );
}


/*
	Called once we're ready to parse entities
*/
void BE_PreSpawnEntities( void ) {
	char	serverinfo[MAX_INFO_STRING];
	char	mapname[MAX_INFO_VALUE];
	

	trap_GetServerinfo( serverinfo, sizeof( serverinfo ) );
	Q_strncpyz( mapname, Info_ValueForKey( serverinfo, "mapname" ), sizeof( mapname ) );

	/* FIXME: Copy pasta. BE_InitBeryllium() is called to late. */

	BE_LoadEntities( mapname );
}


static void BE_FreeEntities( void ) {
	if ( pWorldspawn != NULL ) {
		BE_Free( pWorldspawn );
		pWorldspawn = NULL;
	}
}


/*
	Called after entities have been parsed
*/
void BE_PostSpawnEntities( void ) {
	BE_FreeEntities();
}


/*
	See GET_ENTITY_TOKEN in SV_GameSystemCalls()
*/
qboolean BE_GetEntityToken( char *buffer, int bufferSize ) {
	const char	*s;

	if ( NULL == pWorldspawn ) {
		return trap_GetEntityToken( buffer, bufferSize );
	}
	else {
		s = COM_Parse( &pWorldspawnParsePoint );
		Q_strncpyz( buffer, s, bufferSize );
		if ( !pWorldspawnParsePoint && !s[0] ) {
			return qfalse;
		}
		else {
			return qtrue;
		}
	}
}

