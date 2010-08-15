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

#ifndef _G_BERYYLIUM_H
#define _G_BERYLLIUM_H


#ifndef BERYLLIUM_VERSION
	#error "BERYLLIUM_VERSION not defined, Makefile corrupt?"
#endif


#define S_COLOR_DEFAULT		S_COLOR_WHITE	/* Used to "reset" colored output after other colored output */
#define S_COLOR_POSITIVE	S_COLOR_GREEN
#define S_COLOR_NEGATIVE	S_COLOR_RED
#define S_COLOR_ITALIC		S_COLOR_YELLOW	/* Important output */
#define S_COLOR_BOLD		S_COLOR_CYAN	/* Even more important output */


/* Cvars. See g_main.c */
extern vmCvar_t	be_version;

extern vmCvar_t	be_voteDuration;
extern vmCvar_t	be_allowedVotes;
extern vmCvar_t	be_votePause;
extern vmCvar_t be_voteRate;
extern vmCvar_t be_votePass;

extern vmCvar_t	be_respawnProtect;


extern char	*ConcatArgs( int start ); /* FIXME: Add this to game headers? Declared in g_cmds.c */


/* "Exported" Functions */
qboolean BE_ClientCommand( const gentity_t *ent, const char *cmd );
qboolean BE_ConsoleCommand( const char *cmd );

void BE_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int *damage, int *dflags, int *mod );


#endif

