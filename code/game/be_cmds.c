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
	{ "vote",		CMD_MESSAGE,	BE_Cmd_Vote_f		}
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

