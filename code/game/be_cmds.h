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

#ifndef _BE_CMDS_H
#define _BE_CMDS_H

#include "g_local.h"

/* Structs */

typedef enum {
	CMD_CHEAT			= 1,
	CMD_MESSAGE			= 2,
	CMD_LIVING			= 4,
	CMD_INTERMISSION	= 8
} cmdFlag_t;

typedef struct {
    char		*cmdName;
    cmdFlag_t	cmdFlags;
    void		( *cmdHandler )( gentity_t *ent );
} commands_t;


/* Externals */

/* Functions */
qboolean BE_ClCmd( gentity_t *ent, char *cmd );


#endif

