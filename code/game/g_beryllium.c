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

	/* We are only interested in clients' damage */
	if ( !( targ->client ) || !( attacker->client ) ) {
		return;
	}

	/* Respawn protection */
	/* TODO: Visualise this by making weapons non-functional?
	         Remove protection if weapon fired.
	         Maybe also apply a certain shader while being protected?
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

