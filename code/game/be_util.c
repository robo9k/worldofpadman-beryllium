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


typedef struct {
	char		*str;
	gametype_t	type;
} mapGametypeString_t;


const mapGametypeString_t gametypeRemap[] = {
		{ "Free For All",			GT_FFA				},
		{ "Tournament",				GT_TOURNAMENT		},
		{ "Single Player",			GT_SINGLE_PLAYER	},
		{ "Spray your Color FFA",	GT_SPRAYFFA			},
		{ "Last Pad Standing",		GT_LPS				},
		{ "Team Deathmatch",		GT_TEAM				},
		{ "Capure the Flag",		GT_CTF				},
		{ "Spray your Color TP",	GT_SPRAY			},
		{ "Big Balloon",			GT_BALLOON			}
	};
const NUM_GTSTRS = ( sizeof( gametypeRemap ) / sizeof( gametypeRemap[0] ) );


/*
	Send a command to one or all clients. Basically a wrapper around trap_SendServerCommand()
	See CG_ServerCommand() in cg_servercmds.c
*/
void SendClientCommand( const int clientNum, const int cmd, const char *str ) {
	/* Valid range is -1 and 0 to MAX_CLIENTS-1 */
	if ( ( clientNum < -1 ) || ( clientNum >= MAX_CLIENTS ) ) {
		G_Error( "SendClientCommand: clientNum %i out of range\n", clientNum );
	}
	/* TODO: Check wheter clientNum is connected */

	if ( cmd & CCMD_CENTERPRINT ) {
		trap_SendServerCommand( clientNum, va( "cp \"%s\"", str ) );
	}
	if ( cmd & CCMD_MESSAGEPRINT ) {
		trap_SendServerCommand( clientNum, va( "mp \"%s\"", str ) );
	}
	if ( cmd & CCMD_PRINT ) {
		trap_SendServerCommand( clientNum, va( "print \"%s\"", str ) );
	}
}


/*
	Converts a string into a gametype. List is from ui_callvote.c
	Might return invalid gametype if no match
*/
/* FIXME: Use partial match?
*/
gametype_t StringToGametype( const char *str ) {
	int i;	

	for ( i = 0; i < NUM_GTSTRS; i++ ) {
		if ( Q_stricmp( gametypeRemap[i].str, str ) == 0 ) {
			return ( gametypeRemap[i].type );
		}
	}

	return GT_MAX_GAME_TYPE;
}


/*
	Converts a gametype into a string.
	Might return NULL if no match
*/
char* GametypeToString( const gametype_t gt ) {
	int i;

	for ( i = 0; i < NUM_GTSTRS; i++ ) {
		if ( gametypeRemap[i].type == gt ) {
			return ( gametypeRemap[i].str );
		}
	}

	return NULL;
}


/*
	Converts a time into a short string.
	Assumes time is given in ms
*/
char* TimeToString( const int time, char *str, const size_t size ) {
	int min, tens, sec;

	if ( ( str == NULL ) || ( size <= 0 ) ) {
		return NULL;
	}

	sec = ( time / 1000 );

	min   = ( sec / 60 );
	sec  -= ( min * 60 );
	tens  = ( sec / 10 );
	sec  -= ( tens * 10 );

	Com_sprintf( str, size, "%i:%i%i", min, tens, sec );

	return str;
}

