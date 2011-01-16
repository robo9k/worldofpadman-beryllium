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


typedef struct {
	char		*str;
	gametype_t	type;
} mapGametypeString_t;


const mapGametypeString_t gametypeRemap[] = {
		{ GAMETYPE_NAME( GT_FFA ),			GT_FFA				},
		{ GAMETYPE_NAME( GT_TOURNAMENT ),	GT_TOURNAMENT		},
		{ GAMETYPE_NAME( GT_SINGLE_PLAYER ),GT_SINGLE_PLAYER	},
		{ GAMETYPE_NAME( GT_SPRAYFFA ),		GT_SPRAYFFA			},
		{ GAMETYPE_NAME( GT_LPS ),			GT_LPS				},
		{ GAMETYPE_NAME( GT_TEAM ),			GT_TEAM				},
		{ GAMETYPE_NAME( GT_CTF ),			GT_CTF				},
		{ GAMETYPE_NAME( GT_SPRAY ),		GT_SPRAY			},
		{ GAMETYPE_NAME( GT_BALLOON ),		GT_BALLOON			}
	};
const NUM_GTSTRS = ( sizeof( gametypeRemap ) / sizeof( gametypeRemap[0] ) );


/*
	Send a command to one or all clients. Basically a wrapper around trap_SendServerCommand()
	See CG_ServerCommand() in cg_servercmds.c
*/
void SendClientCommand( clientNum_t clientNum, clientCommand_t cmd, const char *str ) {
	assert( str );


	if ( !ValidClientID( clientNum, qtrue ) ) {
		G_Error( "SendClientCommand: clientNum %i out of range\n", clientNum );
		return;
	}
	/* TODO: Check wheter clientNum is connected */

	switch ( cmd ) {
		case CCMD_CENTERPRINT:
			trap_SendServerCommand( clientNum, va( "cp \"%s\"", str ) );
			break;
		case CCMD_MESSAGEPRINT:
			trap_SendServerCommand( clientNum, va( "mp \"%s\"", str ) );
			break;
		case CCMD_PRINT:
			/* NOTE: World of Padman v1.2 is flawed with con_notifytime > 0
			         It'll always remove the last character in CG_Printf in cg_main.c
			*/
			trap_SendServerCommand( clientNum, va( "print \"%s\"", str ) );
			break;

		default:
			G_Error( "SendClientCommand: cmd %i out of clientCommand_t range!\n", cmd );
			return;
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


	assert( str );


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
char* GametypeToString( gametype_t gt ) {
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
char* TimeToString( int time, char *str, size_t size ) {
	int min, tens, sec;


	/* FIXME: Replace check below with assert()? */
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


/*
	Returns whether given clientNum integer is in valid
	range from 0 to MAX_CLIENTS-1.
	Does not do any other sanity checks!
*/
qboolean ValidClientID( int clientNum, qboolean allowWorld ) {
	if ( ( clientNum < 0 ) || ( clientNum >= MAX_CLIENTS ) ) {
		if ( allowWorld && ( CID_WORLD == clientNum ) ) {
			return qtrue;
		}

		return qfalse;
	}
	
	return qtrue;
}


/*
	Returns a clientNum_t for a given string/name.
	If there are multiple matches, it won't return the first
	match but CID_MULTIPLE (sort of an error).
*/
/* TODO: Rewrite to better handle multiple matches OR
         add ClientnumsFromString() (note the plural!).
   NOTE: There's always a tradeoff between accuracy and comfort
*/
clientNum_t ClientnumFromString( const char *name ) {
	gclient_t	*player;
	char		cleanName[MAX_NETNAME];
	int			id = CID_NONE, matches = 0, i;


	assert( name );

	/* FIXME: Return as soon as matches > 1? */

	for ( i = 0; i < level.maxclients; i++ ) {
		player = &level.clients[i];

		if ( CON_DISCONNECTED == player->pers.connected ) {
			continue;
		}

		/* exact case-insensitive match */
		if ( Q_stricmp( name, player->pers.netname ) == 0 ) {
			id = i;
			matches++;
			continue;
		}

		/* match without colorcodes */
		/* FIXME: Does this violate the multiple matches approach? */
		Q_strncpyz( cleanName, player->pers.netname, sizeof( cleanName ) );
		Q_CleanStr( cleanName );
		if ( Q_stricmp( name, cleanName ) == 0 ) {
			id = i;
			matches++;
			continue;
		}

		/* TODO: Partial match with strstr (dangerous?) */
	}

	if ( matches > 1 ) {
		return CID_MULTIPLE;
	}
	else {
		return id;
	}
}


/*
	Helper function to check whether a file exists serverside
*/
qboolean fileExists( const char *path ) {
	assert( path );


	return ( trap_FS_FOpenFile( path, NULL, FS_READ ) );
}


/*
	Helper function to fix models like "padman/doesntexist"
	which basically use a non existing skin.
*/
qboolean validPlayermodel( const char *model, const char *headModel ) {
	char base[MAX_QPATH];
	char *skin;


	/* FIXME: Replace check below with assert()? */
	if ( ( model == NULL ) || ( headModel == NULL ) ) {
		return qfalse;
	}
	
	Q_strncpyz( base, model, sizeof( base ) );
	skin = strchr( base, '/' );
	if ( skin ) {
		*skin++ = '\0';
	}
	else {
		skin = DEFAULT_PLAYERSKIN_S;
	}
	if ( !fileExists( va( PLAYERMODEL_PATH_S"%s/upper_%s.skin", base, skin ) ) ) {
		return qfalse;
	}

	Q_strncpyz( base, headModel, sizeof( base ) );
	skin = strchr( base, '/' );
	if ( skin ) {
		*skin++ = '\0';
	}
	else {
		skin = DEFAULT_PLAYERSKIN_S;
	}
	if ( !fileExists( va( PLAYERMODEL_PATH_S"%s/head_%s.skin", base, skin ) ) ) {
		return qfalse;
	}


	return qtrue;	
}


/*
	Returns a team_t for the given string.
	SetTeam() has some implementation of this..
*/
team_t TeamFromString( const char *s ) {
	team_t team = TEAM_NUM_TEAMS;


	assert( s );


	if ( ( Q_stricmp( "spectator", s ) == 0 ) || ( Q_stricmp( "s", s ) == 0 ) )  {
		team = TEAM_SPECTATOR;
	}
	else if (  ( Q_stricmp( "red", s ) == 0 ) || ( Q_stricmp( "r", s ) == 0 ) ) {
		team = TEAM_RED;
	}
	else if ( ( Q_stricmp( "blue", s ) == 0 ) || ( Q_stricmp( "b", s ) == 0 ) ) {
		team = TEAM_BLUE;
	}
	else if ( ( Q_stricmp( "free", s ) == 0 ) || ( Q_stricmp( "f", s ) == 0 ) ) {
		team = TEAM_FREE;
	}

	return team;
}


/*
	Returns whether given char is a number
*/
qboolean Q_isnumeric( char c ) {
	if ( ( c >= '0' ) && ( c <= '9' ) ) {
		return qtrue;
	}
	else {
		return qfalse;
	}
}


/*
	Returns whether given string is a simple number.
	Does work with 0362, NOT +.5e3-2f
	FIXME: Does not work for negative numbers
*/
qboolean IsANumber( const char *str ) {
	int i, l;


	/* FIXME: Replace check below with assert()? */
	if ( !str ) {
		return qfalse;
	}

	l = strlen( str );

	for ( i = 0; i < l; i++ ) {
		if ( !Q_isnumeric( str[i] ) ) {
			return qfalse;
		}
	}

	return qtrue;
}


/*
	If ent is given, will use CCMD_PRINT, otherwise G_Printf.
	This is usefull for functions that can be used by both clients
	and rcon.

	TODO: Use fmt and .. once va functions are buffer safe?
*/
void PrintMessage( const gentity_t *ent, const char *msg ) {
	/* FIXME: Replace check below with assert()? */
	if ( !msg ) {
		return;
	}

	if ( NULL == ent ) {
		/* TODO: Strip colors with Q_CleanStr (violate const / buffer)? */
		G_Printf( msg );
	}
	else {
		SendClientCommand( ( ent - g_entities ), CCMD_PRINT, msg );
	}
}


/*
	Returns whether given string is in "/string/string2/" list
	Used in be_allowedVotes cvar
*/
qboolean InList( const char *haystack, const char *needle ) {
	char pattern[MAX_STRING_TOKENS];


	/* Should no needle but haystack return qtrue? :D */
	/* FIXME: Replace check below with assert()? */
	if ( !needle || !haystack ) {
		return qfalse;
	}

	Com_sprintf( pattern, sizeof( pattern ), "/%s/", needle );

	return ( Q_stristr( haystack, pattern ) != NULL );
}


/*
	Used to spit out some debug information, even in release builds.
*/
/* NOTE: This is an almost exact copy of G_Printf */
void QDECL G_DPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		text[1024];

	/* FIXME: Rather add a new g_debug/g_developer cvar and loglevels for it? */
	if ( !trap_Cvar_VariableIntegerValue( "developer" ) ) {
		return;
	}

	va_start( argptr, fmt );
	vsprintf( text, fmt, argptr );
	va_end( argptr );

	trap_Printf( text );
}
