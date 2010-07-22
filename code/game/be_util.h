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


typedef enum {
	CCMD_CENTERPRINT	= 1,
	CCMD_MESSAGEPRINT	= 2,
	CCMD_PRINT			= 4
};
#define CCMD_CP		CCMD_CENTERPRINT
#define CCMD_MP		CCMD_MESSAGEPRINT
#define CCMD_PRT	CCMD_PRINT


void SendClientCommand( const int clientNum, const int cmd, const char *str );

gametype_t StringToGametype( const char *str );
char* GametypeToString( const gametype_t gt );


#endif

