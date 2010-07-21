// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"


void InitTrigger( gentity_t *self ) {
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	trap_SetBrushModel( self, self->model );
	self->r.contents = CONTENTS_TRIGGER;		// replaces the -1 from trap_SetBrushModel
	self->r.svFlags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
void multi_wait( gentity_t *ent ) {
	ent->nextthink = 0;
}


// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger( gentity_t *ent, gentity_t *activator ) {
	ent->activator = activator;
	if ( ent->nextthink ) {
		return;		// can't retrigger until the wait is over
	}

	if ( activator->client ) {
		if ( ( ent->spawnflags & 1 ) &&
			activator->client->sess.sessionTeam != TEAM_RED ) {
			return;
		}
		if ( ( ent->spawnflags & 2 ) &&
			activator->client->sess.sessionTeam != TEAM_BLUE ) {
			return;
		}
	}

	G_UseTargets (ent, ent->activator);

	if ( ent->wait > 0 ) {
		ent->think = multi_wait;
		ent->nextthink = level.time + ( ent->wait + ent->random * crandom() ) * 1000;
	} else {
		// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = 0;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEntity;
	}
}

void Use_Multi( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	multi_trigger( ent, activator );
}

void Touch_Multi( gentity_t *self, gentity_t *other, trace_t *trace ) {
	if( !other->client ) {
		return;
	}
	multi_trigger( self, other );
}

/*QUAKED trigger_multiple (.5 .5 .5) ?
"wait" : Seconds between triggerings, 0.5 default, -1 = one time only.
"random"	wait variance, default is 0
Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)
*/
void SP_trigger_multiple( gentity_t *ent ) {
	G_SpawnFloat( "wait", "0.5", &ent->wait );
	G_SpawnFloat( "random", "0", &ent->random );

	if ( ent->random >= ent->wait && ent->wait >= 0 ) {
		ent->random = ent->wait - FRAMETIME;
		G_Printf( "trigger_multiple has random >= wait\n" );
	}

	ent->touch = Touch_Multi;
	ent->use = Use_Multi;

	InitTrigger( ent );
	trap_LinkEntity (ent);
}



/*
==============================================================================

trigger_always

==============================================================================
*/

void trigger_always_think( gentity_t *ent ) {
	G_UseTargets(ent, ent);
	G_FreeEntity( ent );
}

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (gentity_t *ent) {
	// we must have some delay to make sure our use targets are present
	ent->nextthink = level.time + 300;
	ent->think = trigger_always_think;
}


/*
==============================================================================

trigger_push

==============================================================================
*/

void trigger_push_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {

	if ( !other->client ) {
		return;
	}

	BG_TouchJumpPad( &other->client->ps, &self->s );
}


/*
=================
AimAtTarget

Calculate origin2 so the target apogee will be hit
=================
*/
void AimAtTarget( gentity_t *self ) {
	gentity_t	*ent;
	vec3_t		origin;
	float		height, gravity, time, forward;
	float		dist;

	VectorAdd( self->r.absmin, self->r.absmax, origin );
	VectorScale ( origin, 0.5, origin );

	ent = G_PickTarget( self->target );
	if ( !ent ) {
		G_FreeEntity( self );
		return;
	}

	height = ent->s.origin[2] - origin[2];
	gravity = g_gravity.value;
	time = sqrt( height / ( .5 * gravity ) );
	if ( !time ) {
		G_FreeEntity( self );
		return;
	}

	// set s.origin2 to the push velocity
	VectorSubtract ( ent->s.origin, origin, self->s.origin2 );
	self->s.origin2[2] = 0;
	dist = VectorNormalize( self->s.origin2);

	forward = dist / time;
	VectorScale( self->s.origin2, forward, self->s.origin2 );

	self->s.origin2[2] = time * gravity;
}


/*QUAKED trigger_push (.5 .5 .5) ?
Must point at a target_position, which will be the apex of the leap.
This will be client side predicted, unlike target_push
*/
void SP_trigger_push( gentity_t *self ) {
	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	self->r.svFlags &= ~SVF_NOCLIENT;

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	self->s.eType = ET_PUSH_TRIGGER;
	self->touch = trigger_push_touch;
	self->think = AimAtTarget;
	self->nextthink = level.time + FRAMETIME;
	trap_LinkEntity (self);
}


void Use_target_push( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( !activator->client ) {
		return;
	}

	if ( activator->client->ps.pm_type != PM_NORMAL ) {
		return;
	}

	VectorCopy (self->s.origin2, activator->client->ps.velocity);

	// play fly sound every 1.5 seconds
	if ( activator->fly_sound_debounce_time < level.time ) {
		activator->fly_sound_debounce_time = level.time + 1500;
		G_Sound( activator, CHAN_AUTO, self->noise_index );
	}
}

/*QUAKED target_push (.5 .5 .5) (-8 -8 -8) (8 8 8) bouncepad
Pushes the activator in the direction.of angle, or towards a target apex.
"speed"		defaults to 1000
if "bouncepad", play bounce noise instead of windfly
*/
void SP_target_push( gentity_t *self ) {
	if (!self->speed) {
		self->speed = 1000;
	}
	G_SetMovedir (self->s.angles, self->s.origin2);
	VectorScale (self->s.origin2, self->speed, self->s.origin2);

	if ( self->spawnflags & 1 ) {
		self->noise_index = G_SoundIndex("sound/world/jumppad.wav");
	} else {
		self->noise_index = G_SoundIndex("sound/misc/windfly.wav");
	}
	if ( self->target ) {
		VectorCopy( self->s.origin, self->r.absmin );
		VectorCopy( self->s.origin, self->r.absmax );
		self->think = AimAtTarget;
		self->nextthink = level.time + FRAMETIME;
	}
	self->use = Use_target_push;
}

/*
==============================================================================

trigger_teleport

==============================================================================
*/

void trigger_teleporter_touch (gentity_t *self, gentity_t *other, trace_t *trace ) {
	gentity_t	*dest;

	if ( !other->client ) {
		return;
	}
	if ( other->client->ps.pm_type == PM_DEAD ) {
		return;
	}
	// Spectators only?
	if ( ( self->spawnflags & 1 ) && 
		(other->client->sess.sessionTeam != TEAM_SPECTATOR || (g_gametype.integer==GT_LPS && other->client->sess.livesleft<=0)) ) {
		return;
	}

	if(self->spawnflags & 0x2 && g_gametype.integer!=GT_SPRAY && g_gametype.integer!=GT_SPRAYFFA)
		return;

	if(other->client->sess.sessionTeam!=TEAM_SPECTATOR)
	{
		if(self->spawnflags & 0x2)//rein
		{
			if(!other->client->ps.ammo[WP_SPRAYPISTOL])
				return;

			other->client->logocounter=0;

			other->client->sprayroomleavetime=level.maxsprayroomtime*1000+level.time;
			other->client->sprayroomsoundflags=0;
			other->client->ps.stats[STAT_INSPRAYROOMSEKS] = level.maxsprayroomtime +1;
			if(other->client->ps.weapon!=WP_SPRAYPISTOL) other->client->last_nonspray_weapon = other->client->ps.weapon;//if...sicher is sicher ;)
			if(other->client->ps.weaponstate==WEAPON_CHARGING)
			{
				other->client->ps.weaponstate=WEAPON_READY;
				other->client->ps.weaponTime=0;
			}
			trap_SendServerCommand(other->client->ps.clientNum,va("srwc %i",WP_SPRAYPISTOL));
//			other->client->pers.cmd.weapon = WP_SPRAYPISTOL;
//			other->client->ps.weapon = WP_SPRAYPISTOL;
		}
		else if(self->spawnflags & 0x4)//raus
		{
			other->client->ps.stats[STAT_INSPRAYROOMSEKS] = 0;
			trap_SendServerCommand(other->client->ps.clientNum,va("srwc %i",other->client->last_nonspray_weapon));
//			other->client->pers.cmd.weapon = other->client->last_nonspray_weapon;
//			other->client->ps.weapon = other->client->last_nonspray_weapon;
		}
	}

	dest = 	G_PickTarget( self->target );
	if (!dest) {
		G_Printf ("Couldn't find teleporter destination\n");
		return;
	}

	TeleportPlayer( other, dest->s.origin, dest->s.angles );

}


/*QUAKED trigger_teleport (.5 .5 .5) ? SPECTATOR
Allows client side prediction of teleportation events.
Must point at a target_position, which will be the teleport destination.

If spectator is set, only spectators can use this teleport
Spectator teleporters are not normally placed in the editor, but are created
automatically near doors to allow spectators to move through them
*/
void SP_trigger_teleport( gentity_t *self ) {
	InitTrigger (self);

	// unlike other triggers, we need to send this one to the client
	// unless is a spectator trigger
	if ( self->spawnflags & 1 ) {
		self->r.svFlags |= SVF_NOCLIENT;
	} else {
		self->r.svFlags &= ~SVF_NOCLIENT;
	}

	if(self->spawnflags & 0x2)//mark the tele for cg_predict
	{
		self->s.generic1=0x23;// ;)
		self->s.origin2[0]=(self->r.absmin[0]+self->r.absmax[0])*0.5f;
		self->s.origin2[1]=(self->r.absmin[1]+self->r.absmax[1])*0.5f;
		self->s.origin2[2]=(self->r.absmin[2]+self->r.absmax[2])*0.5f;
		level.sr_tele=self;
        level.gotSrTele = qtrue;    // cyr_zMismatch
	}
//cyr{
    else if(self->spawnflags & 4){
        level.sr_teleout=self;
    }
//cyr}

	// make sure the client precaches this sound
	G_SoundIndex("sound/world/jumppad.wav");

	self->s.eType = ET_TELEPORT_TRIGGER;
	self->touch = trigger_teleporter_touch;

	trap_LinkEntity (self);
}


/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF - SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.
It does dmg points of damage each server frame
Targeting the trigger will toggle its on / off state.

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

*/
void hurt_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	if ( self->r.linked ) {
		trap_UnlinkEntity( self );
	} else {
		trap_LinkEntity( self );
	}
}

void hurt_touch( gentity_t *self, gentity_t *other, trace_t *trace ) {
	int		dflags;

	if ( !other->takedamage ) {
		return;
	}

	if ( self->timestamp > level.time ) {
		return;
	}

	if ( self->spawnflags & 16 ) {
		self->timestamp = level.time + 1000;
	} else {
		self->timestamp = level.time + FRAMETIME;
	}

	// play sound
	if ( !(self->spawnflags & 4) ) {
		G_Sound( other, CHAN_AUTO, self->noise_index );
	}

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	G_Damage (other, self, self, NULL, NULL, self->damage, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt( gentity_t *self ) {
	InitTrigger (self);

	self->noise_index = G_SoundIndex( "sound/world/electro.wav" );
	self->touch = hurt_touch;

	if ( !self->damage ) {
		self->damage = 5;
	}

	self->r.contents = CONTENTS_TRIGGER;

	if ( self->spawnflags & 2 ) {
		self->use = hurt_use;
	}

	// link in to the world if starting active
	if ( ! (self->spawnflags & 1) ) {
		trap_LinkEntity (self);
	}
}


/*
==============================================================================

timer

==============================================================================
*/


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
This should be renamed trigger_timer...
Repeatedly fires its targets.
Can be turned on or off by using.

"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0
so, the basic time between firing is a random time between
(wait - random) and (wait + random)

*/
void func_timer_think( gentity_t *self ) {
	G_UseTargets (self, self->activator);
	// set time before next firing
	self->nextthink = level.time + 1000 * ( self->wait + crandom() * self->random );
}

void func_timer_use( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	self->activator = activator;

	// if on, turn it off
	if ( self->nextthink ) {
		self->nextthink = 0;
		return;
	}

	// turn it on
	func_timer_think (self);
}

void SP_func_timer( gentity_t *self ) {
	G_SpawnFloat( "random", "1", &self->random);
	G_SpawnFloat( "wait", "1", &self->wait );

	self->use = func_timer_use;
	self->think = func_timer_think;

	if ( self->random >= self->wait ) {
		self->random = self->wait - FRAMETIME;
		G_Printf( "func_timer at %s has random >= wait\n", vtos( self->s.origin ) );
	}

	if ( self->spawnflags & 1 ) {
		self->nextthink = level.time + FRAMETIME;
		self->activator = self;
	}

	self->r.svFlags = SVF_NOCLIENT;
}


/*
==============================================================================

trigger_balloonzone

  self->target_ent->teamTime	=> delay-timer for anti-laging
  self->teamTime				=> old BB timer (time when a team started an action)

  the timevalues in teamTime[...] aren't always relative to level.time (some codeparts use it as +/- counter around 0)

==============================================================================
*/
#define MAX_BALLONS			8
#define BALLOON_THINKTIME	100
#define BALLOON_TOUCHDELAY	500
#define BALLOON_POINTTIME	3000
#define BT_RED	1
#define BT_BLUE	2

void TouchBalloonzone( gentity_t *self, gentity_t *other, trace_t *trace ) {
	// a player is standing in the balloonzone? 
	if ( other->client )
	{
		int team, timer;

		// set balloontime for the client hud
		team = other->client->sess.sessionTeam == TEAM_RED ? 0 : 1;
		timer = self->teamTime[team];
		if ( !timer ) timer = level.time;

		if ( self->target_ent->s.frame ) {
			if ( team+1 == self->target_ent->s.generic1 )
				other->client->ps.stats[STAT_BALLOONTIME] = self->target_ent->s.frame;
			else
				other->client->ps.stats[STAT_BALLOONTIME] = -0.011f * (level.time - timer) / self->wait;
		}
		else other->client->ps.stats[STAT_BALLOONTIME] = -0.011f * (level.time - timer);

		// remember touch
		if ( other->client->sess.sessionTeam == TEAM_RED ) 
			self->teamMask |= BT_RED;
		else if ( other->client->sess.sessionTeam == TEAM_BLUE ) 
			self->teamMask |= BT_BLUE;

		//#@070329: some delay ... i think there isn't always a touch-call (with laging clients)
		self->target_ent->teamTime[team] = level.time;

		other->client->lastBBTouchTriEnt= self;
		other->client->lastBBTouchTime	= level.time;
	}
}

int BalloonScore( void ) {
	int team, i;
	if ( !level.numBalloons )
		return 0;

	// test for ownage ;)
	team = level.balloonState[0];
	for ( i = 1; i < level.numBalloons; i++ )
		if ( level.balloonState[i] != team )
			return 1;

	// every balloon captured => double points
	return 2;
}

qboolean isPlayerAtTheBalloon(int clientNr, gentity_t *balloonTrigger) {
	return (level.clients[clientNr].lastBBTouchTime+BALLOON_TOUCHDELAY>level.time
			&& level.clients[clientNr].lastBBTouchTriEnt==balloonTrigger) ? qtrue : qfalse;
}

int getNumPlayersAtBalloon(gentity_t *balloonTrigger, team_t team) {
	int i;
	int	cnt=0;

	for(i=0;i<level.maxclients;++i) {
		if(level.clients[i].sess.sessionTeam==team && isPlayerAtTheBalloon(i,balloonTrigger))
			++cnt;
	}

	return cnt;
}

void giveScoreToBalloonPlayers(gentity_t *balloonTrigger, team_t team, int score) {
	int i;

	for(i=0;i<level.maxclients;++i) {
		if(level.clients[i].sess.sessionTeam==team && isPlayerAtTheBalloon(i,balloonTrigger))
			AddScore(g_entities+i,balloonTrigger->target_ent->s.origin,score);
	}
}


void ThinkBalloonzone( gentity_t *self ) {
	//"(team == 0) ? TEAM_RED : TEAM_BLUE" => ! in this code red=0, blue=1 ! (unlike TEAM_RED(1), TEAM_BLUE(2) from team_t)
	int team, op;
	int score;

	if ( self->target_ent->s.frame ) {
		// get teams
		team = self->target_ent->s.generic1 - 1;
		op = (team == 0) ? 1 : 0;

		// capturing
		if ( self->target_ent->s.frame < 11 && self->teamMask & (1 << team) && !(self->teamMask & (1 << op)) )
		{
			int numPlayers = getNumPlayersAtBalloon(self,((team==0)?TEAM_RED:TEAM_BLUE));
			if(numPlayers<=0) numPlayers=1;//... so, we must not check this later

			self->teamTime[team] += BALLOON_THINKTIME * numPlayers;
			self->target_ent->s.frame = 1 + self->teamTime[team] / (100 * self->speed);

			if ( self->target_ent->s.frame >= 11 ) {
				// captured
				self->last_move_time = 0;
				self->teamTime[team] = 0;
				level.balloonState[self->count] = '1' + team;
				trap_SetConfigstring( CS_BALLOONS, level.balloonState );

				score = BalloonScore();
				level.teamScores[(team == 0) ? TEAM_RED : TEAM_BLUE] += score;
				giveScoreToBalloonPlayers(self,((team==0)?TEAM_RED:TEAM_BLUE),1);
				CalculateRanks();
				if ( !self->message ) trap_SendServerCommand( -1, va( "mp \"Balloon captured by %s Team\"", (team == 0) ? "Red" : "Blue" ) );
				else trap_SendServerCommand( -1, va( "mp \"%s captured by %s Team\"", self->message, (team == 0) ? "Red" : "Blue" ) );
			}
		}

		// balloon is fully raised
		if ( self->target_ent->s.frame >= 11 )
		{
			// animate
			self->last_move_time += BALLOON_THINKTIME;
			if ( self->last_move_time >= 700 * self->speed )
				self->last_move_time -= 700 * self->speed;
			self->target_ent->s.frame = 11 + self->last_move_time / (100 * self->speed);

			// give points
			if(!level.intermissiontime)
			{
				self->teamTime[team] += BALLOON_THINKTIME;
				while ( self->teamTime[team] >= BALLOON_POINTTIME ) {
					self->teamTime[team] -= BALLOON_POINTTIME;
					level.teamScores[(team == 0) ? TEAM_RED : TEAM_BLUE] += BalloonScore();
					CalculateRanks();
				}
			}
		}

		// countering capture
		if ( self->teamMask & (1 << op) ) {
			int numPlayers = getNumPlayersAtBalloon(self,((op==0)?TEAM_RED:TEAM_BLUE));
			if(numPlayers<=0) numPlayers=1;//... so, we must not check this later

			if ( !self->teamTime[op] ) self->teamTime[op] = level.time;
			else if ( level.time > self->teamTime[op] + (self->wait * 1000 /numPlayers) ) { //FIXME? if some players come "later", they will also be calculated for the full time.
				// countered
				self->teamTime[0] = 0;
				self->teamTime[1] = 0;
				self->target_ent->s.frame = 0;
				level.balloonState[self->count] = '0';
				trap_SetConfigstring( CS_BALLOONS, level.balloonState );
				if ( !self->message ) trap_SendServerCommand( -1, va( "mp \"%s Team destroyed a balloon\"", (op == 0) ? "Red" : "Blue" ) );
				else trap_SendServerCommand( -1, va( "mp \"%s destroyed by %s Team\"", self->message, (op == 0) ? "Red" : "Blue" ) );
			}
		}
		else self->teamTime[op] = 0;
	}
	else {
		if ( self->teamMask & BT_RED && self->teamMask & BT_BLUE ) {
			// reset timer if both teams are trying to capture
			self->teamTime[0] = 0;
			self->teamTime[1] = 0;
		}
		else {
			for ( team = 0; team < 2; team++ ) {
				if ( self->teamMask & (1 << team) ) {
					// start capture timer or test for capture
					if ( !self->teamTime[team] ) self->teamTime[team] = level.time;
					else if ( level.time > self->teamTime[team] + 1000 ) {
						self->teamTime[team] = 0;
						self->teamTime[team^1] = level.time;
						self->target_ent->s.generic1 = team + 1;
						self->target_ent->s.frame = 1;
						level.balloonState[self->count] = 'a' + team;
						trap_SetConfigstring( CS_BALLOONS, level.balloonState );
						if ( !self->message ) trap_SendServerCommand( -1, va( "mp \"%s Team attacks a balloon\"", (team == 0) ? "Red" : "Blue" ) );
						else trap_SendServerCommand( -1, va( "mp \"%s under attack by %s Team\"", self->message, (team == 0) ? "Red" : "Blue" ) );
					}
				} 
				else self->teamTime[team] = 0;
			}
		}
	}

	// prepare next think

	//#@070329: some delay ... i think there isn't always a touch-call (with laging clients)
	// "TEAM_RED ? 0 : 1" @ teamTime[...]
	if(self->target_ent->teamTime[0]+BALLOON_TOUCHDELAY < level.time)
		self->teamMask &= ~BT_RED;
	if(self->target_ent->teamTime[1]+BALLOON_TOUCHDELAY < level.time)
		self->teamMask &= ~BT_BLUE;

	self->nextthink = level.time + BALLOON_THINKTIME;
}

/*QUAKED trigger_balloonzone (1 0 1) ?
Zone used for the BigBalloon gametype. 

"wait"  seconds until a capture is countered
"speed" seconds until the balloon is captured
*/
void SP_trigger_balloonzone( gentity_t *ent ) {
	// check gametype
	if ( g_gametype.integer != GT_BALLOON )
		return;

	// find target
	ent->target_ent = G_PickTarget( ent->target );
	if ( !ent->target_ent ) {
		Com_Printf( "trigger_balloonzone without target ignored\n" );
		return;
	}

	// add it to the list
	if ( level.numBalloons >= MAX_BALLONS ) {
		Com_Printf( "trigger_balloonzone ignored - too many balloons" );
		return;
	}
	ent->count = level.numBalloons;
	level.balloonState[level.numBalloons++] = '0';
	level.balloonState[level.numBalloons] = 0;
	trap_SetConfigstring( CS_BALLOONS, level.balloonState );

	// init
	ent->touch = TouchBalloonzone;
	ent->think = ThinkBalloonzone;
	ent->nextthink = level.time + BALLOON_THINKTIME;
	InitTrigger( ent );
	trap_LinkEntity (ent);

	// set defaults
	if ( ent->wait < 0.1 ) ent->wait = 3.0;
	if ( ent->speed < 0.1 ) ent->speed = 4.0;
	ent->target_ent->s.time = ent->speed * 1000;
}

