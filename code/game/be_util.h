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

#ifndef _BE_UTIL_H
#define _BE_UTIL_H


enum {
	CCMD_CENTERPRINT	= 1,
	CCMD_MESSAGEPRINT	= 2,
	CCMD_PRINT			= 4,

	CCMD_CP				= CCMD_CENTERPRINT,
	CCMD_MP				= CCMD_MESSAGEPRINT,
	CCMD_PRT			= CCMD_PRINT
};

/* NOTE: Anything from 0 to MAX_CLIENTS-1 is a valid client id
         (but not neccessarily a valid/connected client).
         Below are special (return) values.
         Gamecode and engine use -1 to indicate "every client".
*/
enum {
	CID_NONE 		= MAX_CLIENTS,
	CID_WORLD		= -1,
	CID_ALL			= CID_WORLD,
	CID_MULTIPLE	= -2		/* NOTE: See comment for ClientnumFromString() */
};

/* NOTE: With current "MAX_CLIENTS 64" this'd fit into char instead of int,
         but engine/gamecode use int everywhere.
*/
typedef signed int clientNum_t;


void SendClientCommand( const clientNum_t clientNum, const int cmd, const char *str );

gametype_t StringToGametype( const char *str );
char* GametypeToString( const gametype_t gt );


char* TimeToString( const int time, char *str, const size_t size );


const char	*Q_stristr( const char *s, const char *find);

qboolean ValidClientID( const int clientNum, const qboolean allowWorld );

clientNum_t ClientnumFromString( const char *name );

#endif

