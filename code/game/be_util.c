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


/*
	Send a command to one or all clients. Basically a wrapper around trap_SendServerCommand()
	See CG_ServerCommand() in cg_servercmds.c
*/
void SendClientCommand( const int clientNum, const int cmd, const char *str ) {
	// Valid range is -1 and 0 to MAX_CLIENTS-1
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
	Converts a string into a gametype using partial match.
	Might return invalid gametype if no match
*/
gametype_t StringToGametype( const char *str ) {
	return GT_MAX_GAME_TYPE;
}

