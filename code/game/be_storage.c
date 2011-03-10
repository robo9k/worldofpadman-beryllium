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


/*
	Called on clients' first connect
*/
void BE_InitClientStorageData( gclient_t *client ) {
	clientStorage_t *stor;


	assert( client );

	stor = &client->storage;

	/* TODO: Implement */

	BE_WriteClientStorageData( client );
}


/*
	Called on game shutdown
*/
void BE_WriteClientStorageData( const gclient_t *client ) {
	char	*var, *value;
	int		clientNum;


	assert( client );

	clientNum = ( client - level.clients );
	var = va( STORAGE_CVARNAME"%ld", clientNum );

	/* TODO: Implement */

	trap_Cvar_Set( var, value );
}


/*
	Called on clients' reconnect
*/
void BE_ReadClientStorageData( gclient_t *client ) {
	char	data[MAX_INFO_STRING];
	int		clientNum;
	char	*var;


	assert( client );

	clientNum = ( client - level.clients );
	var = va( STORAGE_CVARNAME"%ld", clientNum );
	
	trap_Cvar_VariableStringBuffer( var, data, sizeof( data ) );

	/* TODO: Implement */
}


/*
	Called on game startup
*/
void BE_InitWorldStorage( void ) {
	return;
}


/*
	Called on game shutdown
*/
void BE_WriteStorageData( void ) {
	int i;


	for ( i = 0 ; i < level.maxclients ; i++ ) {
		if ( CON_CONNECTED == level.clients[i].pers.connected ) {
			BE_WriteClientStorageData( &level.clients[i] );
		}
	}
}

