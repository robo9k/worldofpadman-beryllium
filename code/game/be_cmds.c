/*
===========================================================================
Copyright (C) 2011 thebrain

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


/* Variables */
const ccmd_t BE_CCMDS[] = {
	{ "callvote",	CMD_MESSAGE,	BE_Cmd_CallVote_f	},
	{ "cv",			CMD_MESSAGE,	BE_Cmd_CallVote_f	},
	{ "vote",		CMD_MESSAGE,	BE_Cmd_Vote_f		},
	{ "ignore",		0,				BE_Cmd_Ignore_f		}
};
const unsigned int NUM_CCMDS = ARRAY_LEN( BE_CCMDS );


/* Functions */

/*
	This function just loops through our registered client commands,
	checks conditions in flags are met and then calls the function.
*/
qboolean BE_ClCmd( const gentity_t *ent, const char *cmd ) {
	unsigned int i;


	assert( ent );
	assert( ent->client );
	assert( cmd );


	for ( i = 0; i < NUM_CCMDS; i++ ) {
		if ( Q_stricmp( cmd, BE_CCMDS[i].cmdName ) == 0 ) {
			/* do tests here to reduce the amount of repeated code */
			/* We provide this command, but the conditions to actually execute it are not given */

			if ( !( BE_CCMDS[i].cmdFlags & CMD_INTERMISSION ) && level.intermissiontime ) {
				return qtrue;
			}

			if ( ( BE_CCMDS[i].cmdFlags & CMD_CHEAT ) && !g_cheats.integer ) {
				return qtrue;
			}

			if ( ( BE_CCMDS[i].cmdFlags & CMD_LIVING ) && ( PM_DEAD == ent->client->ps.pm_type ) ) {
				return qtrue;
			}

			/* TODO: Test for CMD_MESSAGE. This is only useful with muting implemented */


			/* Conditions are met, execute! */
			BE_CCMDS[i].cmdHandler( ent );
			return qtrue;
		}
	}

	/* This is none of our business */
	return qfalse;
}


/*
	Allows to ignore and unignore chats and voice from other players
*/
void BE_Cmd_Ignore_f( const gentity_t *ent ) {
	/* Print current list */
	if ( trap_Argc() == 1 ) {
		char buff[MAX_STRING_TOKENS] = { "" };
		char message[MAX_STRING_TOKENS];
		int i, c = 0;
		gentity_t *other;
		
		for ( i = 0; i < level.maxclients; i++ ) {
			other = ( g_entities + i );
			
			if ( BE_Ignored( ent, other ) ) {
				Q_strcat( buff, sizeof( buff ), va( "%s%s"S_COLOR_DEFAULT, ( ( c > 0 ) ? ", " : "" ), other->client->pers.netname ) );
				c++;
			}
		}

		if ( c > 0 ) {
			Q_strncpyz( message, "You are currently ignoring: ", sizeof( message ) );
			Q_strcat( message, sizeof( message ), buff );
		}
		else {
			Q_strncpyz( message, "You are currently not ignoring anyone.", sizeof( message ) );
		}

		SendClientCommand( ( ent - g_entities ), CCMD_PRT, va( "%s\n", message ) );
	}
	/* (Un)ignore a client number */
	else if ( trap_Argc() == 2 ) {
		char		arg1[MAX_STRING_TOKENS];
		int			clientNum;
		qboolean	ignore;
		const gentity_t	*other;
		char		*message;

		trap_Argv( 1, arg1, sizeof( arg1 ) );

		if ( !IsANumber( arg1 ) ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You must supply a client number.\n" );
			return;
		}

		clientNum = atoi( arg1 );

		if ( !ValidClientID( clientNum, qfalse ) ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Not a valid client number.\n" );
			return;
		}

		other = ( g_entities + clientNum );

		if ( CON_CONNECTED != other->client->pers.connected ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"Client not connected.\n" );
			return;			
		}
		else if ( other == ent  ) {
			SendClientCommand( ( ent - g_entities ), CCMD_PRT, S_COLOR_NEGATIVE"You can not ignore yourself.\n" );
			return;		
		}

		ignore = BE_Ignored( ent, other );
		ignore = !ignore;

		/* FIXME: Const correctness */
		BE_Ignore( (gentity_t*)ent, other, ignore );

		/* TODO: Ignore VoIP */

		if ( ignore ) {
			message = va( "You are now ignoring %s"S_COLOR_DEFAULT".\n", other->client->pers.netname );
		}
		else {
			message = va( "You are no longer ignoring %s"S_COLOR_DEFAULT".\n", other->client->pers.netname );
		}

		SendClientCommand( ( ent - g_entities ), CCMD_PRT, message );	
	}
}

