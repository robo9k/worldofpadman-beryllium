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
	Hooks midway into G_Damage() in g_combat.c.
	TODO: Create separate functions which handle knockback and radius damage
*/
void BE_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int *damage, int *dflags, int *mod ) {

	assert( targ );
	assert( attacker );
	assert( mod );
	assert( damage );


	/* We are only interested in clients' damage */
	if ( !( targ->client ) || !( attacker->client ) ) {
		return;
	}

	/* Respawn protection */
	/* TODO: Visualise this by making weapons non-functional? Or simply centerprint countdown?
	         Remove protection if weapon fired!
	*/
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


	assert( ( 0 <= clientNum ) && ( MAX_CLIENTS > clientNum ) ); /* FIXME: ValidClientID()? */
	assert( ent->client );


	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );


	/* Fix wrong player skins */
	/* NOTE: Clients who have cg_forceModel set might still see wrong models, since we
	         can not change their clientside setting (needs clientside fixing - should use userinfo as well ).
	         It seems like team_model and team_headmodel are transmitted as userinfo, but the server never uses
	         them for player configstrings.
	*/
	if ( !validPlayermodel( Info_ValueForKey( userinfo, "model" ), Info_ValueForKey( userinfo, "headmodel" ) ) ) {
		Info_SetValueForKey( userinfo, "model", DEFAULT_PLAYERMODEL_S );
		Info_SetValueForKey( userinfo, "headmodel", DEFAULT_PLAYERMODEL_S );
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
	/* Hectic dislikes renamed bots.. */
	if ( !( ent->r.svFlags & SVF_BOT ) ) {
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
					if ( CON_CONNECTED == ent->client->pers.connected ) {
						SendClientCommand( CID_ALL, CCMD_PRT, va( S_COLOR_ITALIC"%s"S_COLOR_ITALIC" was automatically renamed to %s"S_COLOR_ITALIC".\n", oldname, newname ) );
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
	Hooks early into ClientConnect() in g_client.c.
	If we return NULL, ClientConnect() continues, otherwise client will be
	rejected with our return value.
*/
/* FIXME: Call trap_DropClient() rather than having them trying to connect continously? */
char *BE_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char userinfo[MAX_INFO_STRING];
	char *value;
	char ip[NET_ADDRSTRMAXLEN], guid[GUIDSTRMAXLEN];


	assert( ( 0 <= clientNum ) && ( MAX_CLIENTS > clientNum ) ); /* FIXME: ValidClientID()? */


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
  
	if( !*ip ) {
    	return "No IP in userinfo.";
	}
	/* "127.0.0.1" or the like */
	/* NOTE: Other possible values "bot", "localhost". But these don't occur, since
	         we check for isBot and "localhost" luckily passes. We might also want to
	         check number of dots?
	*/
	if ( strlen( ip ) < 7 ) {
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
			return "Maximum simultaneos connections per IP exceeded.";
		}
	}


	if ( be_checkGUIDs.integer ) {
		/* "Its value is a 32 character string made up of [a-f] and [0-9] characters." */
		value = Info_ValueForKey( userinfo, "cl_guid" );
  		Q_strncpyz( guid, value, sizeof( guid ) );

		/* TODO: Add GUIDCHECK_MULTIPLE? */

		if ( *guid && ( be_checkGUIDs.integer & GUIDCHECK_FORMAT ) ) {
			int count = 0;
			qboolean valid = qtrue;

			while ( guid[count] != '\0' && valid ) {
				if( ( ( guid[count] < '0' ) || ( guid[count] > '9' ) ) &&
				    ( ( guid[count] < 'A' ) || ( guid[count] > 'F' ) ) ) {
					valid = qfalse;
					break;
	   			}
      			count++;
		    }

			if ( !valid || ( count != 32 ) )  {
				return "Invalid GUID in userinfo.";
			}
		}
		else if ( be_checkGUIDs.integer & GUIDCHECK_EMPTY ) {
			/* NOTE: This happens with v1.1 engine clients in World of Padman.
			         So maybe we should return a more helpfull "error message"?
			*/
			return "No GUID in userinfo.";
		}
	}


	/* NOTE: We can not set any data for the client, since original ClientConnect()
	         will erase the whole client struct with memset() after we return.
	*/


	return NULL;
}

