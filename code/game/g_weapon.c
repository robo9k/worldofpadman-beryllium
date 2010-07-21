// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"

static	float	s_quadFactor;
static	vec3_t	forward, right, up;
static	vec3_t	muzzle;

/*
================
G_BounceProjectile
================
*/
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================================================================

GAUNTLET

======================================================================
*/

void Weapon_Gauntlet( gentity_t *ent ) {

}

/*
===============
CheckGauntletAttack
===============
*/
qboolean CheckGauntletAttack( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		end;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePoint ( ent, forward, right, up, muzzle );

	VectorMA (muzzle, 32, forward, end);

	trap_Trace (&tr, muzzle, NULL, NULL, end, ent->s.number, MASK_SHOT);
	if ( tr.surfaceFlags & SURF_NOIMPACT ) {
		return qfalse;
	}

	traceEnt = &g_entities[ tr.entityNum ];

	// send blood impact
	if ( traceEnt->takedamage && traceEnt->client ) {
		tent = G_TempEntity( tr.endpos, EV_MISSILE_HIT );
		tent->s.otherEntityNum = traceEnt->s.number;
		tent->s.eventParm = DirToByte( tr.plane.normal );
		tent->s.weapon = ent->s.weapon;
	}

	if ( !traceEnt->takedamage) {
		return qfalse;
	}

	if (ent->client->ps.powerups[PW_PADPOWER] ) {
		G_AddEvent( ent, EV_POWERUP_PADPOWER, 0 );
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}
	if( ent->client->ps.powerups[PW_BERSERKER] )
		s_quadFactor *= 10.0f; // one "berserker punchy" hit should kill

	damage = 50 * s_quadFactor;
	G_Damage( traceEnt, ent, ent, forward, tr.endpos,
		damage, 0, MOD_GAUNTLET );

	return qtrue;
}


/*
======================================================================

MACHINEGUN

======================================================================
*/

/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
	int		i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = (int)v[i];
		} else {
			v[i] = (int)v[i] + 1;
		}
	}
}

void weapon_nipper_fire(gentity_t *ent) {
	gentity_t	*m;

	m = fire_nipper (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}


/*
======================================================================

BFG

======================================================================
*/

void BFG_Fire ( gentity_t *ent ) {
	gentity_t	*m;

	m = fire_bfg (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;

//	VectorAdd( m->s.pos.trDelta, ent->client->ps.velocity, m->s.pos.trDelta );	// "real" physics
}


/*
======================================================================

SHOTGUN

======================================================================
*/

#define PUMPER_SPLASHDMG	50 //old: 70
#define PUMPER_SPLASHRADIUS	80
//new: g_combat.c(line ~830) 2x knockback with pumper
void weapon_supershotgun_fire (gentity_t *ent) {
	vec3_t		end;
	trace_t		trace;
	gentity_t	*tent;
	gentity_t	*traceEnt;
	int			damage;
	int			hits;
	int			passent;
	vec3_t		minPumper = {-4.0f,-4.0f,-4.0f};
	vec3_t		maxPumper = {4.0f,4.0f,4.0f};

	damage = 70 * s_quadFactor;

//	VectorMA (muzzle, 8192, forward, end);
	VectorMA (muzzle, 1024, forward, end);//vq3: lightning has 768

	// trace only against the solids, so the railgun will go through people
	hits = 0;
	traceEnt = NULL;
	passent = ent->s.number;
	trap_Trace (&trace, muzzle, minPumper, maxPumper, end, passent, MASK_SHOT );
	if( trace.startsolid ) {
		VectorCopy(muzzle,trace.endpos);
	}
	else if ( trace.entityNum < ENTITYNUM_MAX_NORMAL ) {
		traceEnt = &g_entities[ trace.entityNum ];
		if ( traceEnt->takedamage ) {
			if( LogAccuracyHit( traceEnt, ent ) ) {
				hits++;
			}
			damage*=1.0f-(trace.fraction*0.5f);
			G_Damage (traceEnt, ent, ent, forward, trace.endpos, damage, 0, MOD_SHOTGUN);
		}
	}
	// the final trace endpos will be the terminal point of the rail trail

	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( trace.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( trace.endpos, EV_RAILTRAIL );

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( trace.surfaceFlags & SURF_NOIMPACT ) {
		// don't make the explosion at the end
		tent->s.eventParm = 255;	
	} else {
		// make an explosion with radius damage
		tent->s.eventParm = DirToByte( trace.plane.normal );
		G_RadiusDamage( trace.endpos, ent, PUMPER_SPLASHDMG, PUMPER_SPLASHRADIUS, traceEnt, MOD_SHOTGUN );
	}
	tent->s.clientNum = ent->s.clientNum;
}


/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (gentity_t *ent) {
	gentity_t	*m;

	// extra vertical velocity
	forward[2] += 0.2f;
	VectorNormalize( forward );

	m = fire_grenade (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (gentity_t *ent) {
	gentity_t	*m;

	vec3_t		start;

	VectorMA( muzzle, 5, right, start );
	VectorMA( start, -5, up, start );
	m = fire_rocket (ent, start, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}


/*
======================================================================

PLASMA GUN

======================================================================
*/

void Weapon_Plasmagun_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_bubbleg (ent, muzzle, forward);// HERBY: fire bubble gum
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}

/*
======================================================================

KiLLERDUCKS

======================================================================
*/

void Weapon_KillerDucks_Fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_duck (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}

/*
======================================================================

RAILGUN

======================================================================
*/


/*
=================
weapon_railgun_fire
=================
*/
#define	MAX_RAIL_HITS	4
void weapon_railgun_fire (gentity_t *ent) {
	gentity_t	*m;

	m = fire_splasher (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}


/*
======================================================================

GRAPPLING HOOK

======================================================================
*/

void Weapon_GrapplingHook_Fire (gentity_t *ent)
{
	if (!ent->client->fireHeld && !ent->client->hook)
		fire_grapple (ent, muzzle, forward);

	ent->client->fireHeld = qtrue;
}

void Weapon_HookFree (gentity_t *ent)
{
	ent->parent->client->hook = NULL;
	ent->parent->client->ps.pm_flags &= ~PMF_GRAPPLE_PULL;
	G_FreeEntity( ent );
}

void Weapon_HookThink (gentity_t *ent)
{
	if (ent->enemy) {
		vec3_t v, oldorigin;

		VectorCopy(ent->r.currentOrigin, oldorigin);
		v[0] = ent->enemy->r.currentOrigin[0] + (ent->enemy->r.mins[0] + ent->enemy->r.maxs[0]) * 0.5;
		v[1] = ent->enemy->r.currentOrigin[1] + (ent->enemy->r.mins[1] + ent->enemy->r.maxs[1]) * 0.5;
		v[2] = ent->enemy->r.currentOrigin[2] + (ent->enemy->r.mins[2] + ent->enemy->r.maxs[2]) * 0.5;
		SnapVectorTowards( v, oldorigin );	// save net bandwidth

		G_SetOrigin( ent, v );
	}

	VectorCopy( ent->r.currentOrigin, ent->parent->client->ps.grapplePoint);
}

/*
======================================================================

LIGHTNING GUN

======================================================================
*/

void Weapon_LightningFire( gentity_t *ent ) {
	gentity_t	*m;

	m = fire_boaster (ent, muzzle, forward);
	m->damage *= s_quadFactor;
	m->splashDamage *= s_quadFactor;
}

/*
======================================================================

SPRAY PISTOL

======================================================================
*/

void check_sprayawards(gentity_t *ent)
{
	ent->client->logocounter++;

	if(ent->client->logocounter==5)
	{
		ent->client->ps.persistant[PERS_SPRAYAWARDS_COUNT]=(ent->client->ps.persistant[PERS_SPRAYAWARDS_COUNT] & 0xFF00)|((ent->client->ps.persistant[PERS_SPRAYAWARDS_COUNT]+1) & 0xFF);
		// add the sprite over the player's head
		ent->client->ps.eFlags &= REMOVE_AWARDFLAGS;
		ent->client->ps.eFlags |= EF_AWARD_SPRAYKILLER;
		ent->client->rewardTime = level.time + SPRAYREWARD_SPRITE_TIME;
		PrintMsg(NULL,"%s" S_COLOR_MAGENTA " is a SprayKiller\n",ent->client->pers.netname);

		AddScore(ent,ent->client->ps.origin,10);//old:5
		if(g_gametype.integer==GT_SPRAY)
			AddTeamScore(ent->client->ps.origin,ent->client->sess.sessionTeam,10);//old:5
	}
	else if(ent->client->logocounter==8)
	{
		ent->client->ps.persistant[PERS_SPRAYAWARDS_COUNT]+=0x100;
		// add the sprite over the player's head
		ent->client->ps.eFlags &= REMOVE_AWARDFLAGS;
		ent->client->ps.eFlags |= EF_AWARD_SPRAYGOD;
		ent->client->rewardTime = level.time + SPRAYREWARD_SPRITE_TIME;
		PrintMsg(NULL,"%s" S_COLOR_MAGENTA " is a SprayGod\n",ent->client->pers.netname);

		AddScore(ent,ent->client->ps.origin,25);//old:10
		if(g_gametype.integer==GT_SPRAY)
			AddTeamScore(ent->client->ps.origin,ent->client->sess.sessionTeam,25);//old:25

		ent->client->logocounter=0;
	}
}

void weapon_spraypistol_fire(gentity_t *ent)
{
	vec3_t		end,tmpv3;
	trace_t		tr;
	gentity_t	*tent;
	const char *team_strs[] = {"Free","Red","Blue","Spec"};

	ent->client->ps.generic1=ent->client->ps.ammo[WP_SPRAYPISTOL];

	VectorMA (muzzle, 256, forward, end);
	trap_Trace(&tr, muzzle, NULL, NULL, end, 0, MASK_SHOT );

	///AddScore ??? was bekommt der einzelne player für ein logo ... bzw was bekommt er bei der falschen wand =)
	if(g_gametype.integer==GT_SPRAY)
	{
		if(&g_entities[tr.entityNum]==level.rspraywall)
		{
			if(ent->client->sess.sessionTeam==TEAM_RED)
			{
				AddTeamScore(tr.endpos,TEAM_RED,5);
				AddScore(ent,tr.endpos,5);
				check_sprayawards(ent);
			}
			else
			{
				AddScore(ent,tr.endpos,-5);
				trap_SendServerCommand( -1, va("cdi 1 %i",(int)(random()*3.9999f)));
				
				PrintMsg(NULL,"%s" S_COLOR_MAGENTA "(Blue-Team) sprayed on the WRONG WALL!!!\n",ent->client->pers.netname);
			}
//			Com_Printf("a logo at the redwall! ... by the %s Team\n",team_strs[ent->client->sess.sessionTeam]);
			CalculateRanks();//Update Teamscore ... and do some other things ;)
		}
		else if(&g_entities[tr.entityNum]==level.bspraywall)
		{
			if(ent->client->sess.sessionTeam==TEAM_BLUE)
			{
				AddTeamScore(tr.endpos,TEAM_BLUE,5);
				AddScore(ent,tr.endpos,5);
				check_sprayawards(ent);
			}
			else
			{
				AddScore(ent,tr.endpos,-5);
				trap_SendServerCommand( -1, va("cdi 1 %i",(int)(random()*3.9999f)));

				PrintMsg(NULL,"%s" S_COLOR_MAGENTA "(Red-Team) sprayed on the WRONG WALL!!!\n",ent->client->pers.netname);
			}
//			Com_Printf("a logo at the bluewall! ... by the %s Team\n",team_strs[ent->client->sess.sessionTeam]);
			CalculateRanks();//Update Teamscore ... and do some other things ;)
		}
	}
	else //spray ffa
	{
		//all spraywalls give the same points
		if(&g_entities[tr.entityNum]==level.rspraywall || &g_entities[tr.entityNum]==level.bspraywall || &g_entities[tr.entityNum]==level.nspraywall)
		{
			AddScore(ent,tr.endpos,5);
			CalculateRanks();//Update score
			check_sprayawards(ent);
		}
	}

	//graphic-stuff ... taken from the rail-shot
	// snap the endpos to integers to save net bandwidth, but nudged towards the line
	SnapVectorTowards( tr.endpos, muzzle );

	// send railgun beam effect
	tent = G_TempEntity( tr.endpos, EV_SPRAYLOGO );

	tent->r.svFlags |= SVF_BROADCAST;

	tmpv3[0]=tr.endpos[0]-ent->s.pos.trBase[0];
	tmpv3[1]=tr.endpos[1]-ent->s.pos.trBase[1];
	tmpv3[2]=tr.endpos[2]-ent->s.pos.trBase[2];

	//vvvvvvvvvvvvvvv missbrauch :ugly: ;)
	tent->s.angles[0]=sqrt(tmpv3[0]*tmpv3[0]+tmpv3[1]*tmpv3[1]+tmpv3[2]*tmpv3[2])/200;//256

	// set player number for custom colors on the railtrail
	tent->s.clientNum = ent->s.clientNum;

	VectorCopy( muzzle, tent->s.origin2 );
	// move origin a bit to come closer to the drawn gun muzzle
	VectorMA( tent->s.origin2, 4, right, tent->s.origin2 );
	VectorMA( tent->s.origin2, -1, up, tent->s.origin2 );

	// no explosion at end if SURF_NOIMPACT, but still make the trail
	if ( tr.surfaceFlags & SURF_NOIMPACT || tr.fraction == 1.0f) {
		tent->s.eventParm = 255;	// don't make the explosion at the end
	} else {
		tent->s.eventParm = DirToByte( tr.plane.normal );
	}
	if(tr.entityNum!=ENTITYNUM_WORLD)
		tent->s.generic1=0x23;//ungebrauchte vars tut man missbrauchen ;)
}

//======================================================================


/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
CalcMuzzlePoint

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePoint ( gentity_t *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}

/*
===============
CalcMuzzlePointOrigin

set muzzle location relative to pivoting eye
===============
*/
void CalcMuzzlePointOrigin ( gentity_t *ent, vec3_t origin, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint ) {
	VectorCopy( ent->s.pos.trBase, muzzlePoint );
	muzzlePoint[2] += ent->client->ps.viewheight;
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );
	// snap to integer coordinates for more efficient network bandwidth usage
	SnapVector( muzzlePoint );
}



/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {
	if (ent->client->ps.powerups[PW_PADPOWER] ) {
		s_quadFactor = g_quadfactor.value;
	} else {
		s_quadFactor = 1;
	}

	// track shots taken for accuracy tracking.  Grapple is not a weapon and gauntet is just not tracked
	if( ent->s.weapon != WP_GRAPPLING_HOOK && ent->s.weapon != WP_PUNCHY ) {
		ent->client->accuracy_shots++;
	}

	// set aiming directions
	AngleVectors (ent->client->ps.viewangles, forward, right, up);

	CalcMuzzlePointOrigin ( ent, ent->client->oldOrigin, forward, right, up, muzzle );

	// fire the specific weapon
	switch( ent->s.weapon )
	{
	case WP_PUNCHY:
		Weapon_Gauntlet( ent );
		break;
	case WP_BOASTER:
		Weapon_LightningFire( ent );
		break;
	case WP_PUMPER:
		weapon_supershotgun_fire( ent );
		break;
	case WP_NIPPER:
		weapon_nipper_fire(ent);
		break;
	case WP_BALLOONY:
		weapon_grenadelauncher_fire( ent );
		break;
	case WP_BETTY:
		Weapon_RocketLauncher_Fire( ent );
		break;
	case WP_BUBBLEG:
		Weapon_Plasmagun_Fire( ent );
		break;
	case WP_SPLASHER:
		weapon_railgun_fire( ent );
		break;
	case WP_IMPERIUS:
		BFG_Fire( ent );
		break;

	case WP_GRAPPLING_HOOK:
		Weapon_GrapplingHook_Fire( ent );
		break;

	case WP_SPRAYPISTOL:
		weapon_spraypistol_fire(ent);
		break;

	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}


