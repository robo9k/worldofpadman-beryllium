// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

GAME OPTIONS MENU

=======================================================================
*/


#include "ui_local.h"

#define ART_BACK0				"menu/BtnBack0"
#define ART_BACK1				"menu/BtnBack1"

#define PREFERENCES_X_POS		540

#define ID_CROSSHAIR			127
#define ID_SIMPLEITEMS			128
#define ID_HIGHQUALITYSKY		129
#define ID_EJECTINGBRASS		130
#define ID_WALLMARKS			131
#define ID_DYNAMICLIGHTS		132
#define ID_IDENTIFYTARGET		133
#define ID_SYNCEVERYFRAME		134
#define ID_FORCEMODEL			135
#define ID_DRAWTEAMOVERLAY		136
#define ID_ALLOWDOWNLOAD		137
#define ID_BACK					138

#define	ID_FFAHUD				139
#define ID_CONNOTIFY			140
#define	ID_TIMELEFT				141

#define	ID_TIMER				142
#define	ID_FPS					143
#define	ID_UPS					144
#define	ID_REALTIME				145

#define	NUM_CROSSHAIRS			10


typedef struct {
	menuframework_s		menu;

	menulist_s			crosshair;
	menuradiobutton_s	simpleitems;
	menuradiobutton_s	brass;
	menuradiobutton_s	wallmarks;
	menuradiobutton_s	dynamiclights;
	menuradiobutton_s	identifytarget;
	menuradiobutton_s	highqualitysky;
	menuradiobutton_s	synceveryframe;
	menuradiobutton_s	forcemodel;
	menulist_s			drawteamoverlay;

	menulist_s			ffahud;
	menulist_s			con_notifytime;
	menuradiobutton_s	timeleft;

	menuradiobutton_s	timer;
	menuradiobutton_s	fps;
	menuradiobutton_s	ups;
	menuradiobutton_s	realtime;

	menuradiobutton_s	allowdownload;
	menubitmap_s		back;

	qhandle_t			crosshairShader[NUM_CROSSHAIRS];
} preferences_t;

static preferences_t s_preferences;

static const char *teamoverlay_names[] =
{
	"off",
	"upper right",
	"lower right",
	"lower left",
	0
};

static const char *ffahud_names[] =
{
	"black",
	"red",
	"blue",
	"green",
	"chrome",
	"whitemetal",
	"rust",
	"flower",
	"wood",
	"airforce",
	0
};

static const char *con_notifytime_strs[] =
{
	"only text(normal)",
	"only text(long)",
	"+icons(normal)",
	"+icons(long)",
	0
};

static void Preferences_SetMenuItems( void )
{
	int notify;

	notify=UI_GetCvarInt("con_notifytime");
	if(notify>0 && notify<=3)
		s_preferences.con_notifytime.curvalue = 0;
	else if(notify>3)
		s_preferences.con_notifytime.curvalue = 1;
	else if(notify<0 && notify>=-5)
		s_preferences.con_notifytime.curvalue = 2;
	else if(notify<-5)
		s_preferences.con_notifytime.curvalue = 3;

	s_preferences.ffahud.curvalue			= Com_Clamp( 0, 9, trap_Cvar_VariableValue( "cg_wopFFAhud" ) );
	s_preferences.timeleft.curvalue			= Com_Clamp( 0, 1, trap_Cvar_VariableValue("cg_drawTimeLeft") );

	s_preferences.timer.curvalue			= Com_Clamp( 0, 1, trap_Cvar_VariableValue("cg_drawTimer") );
	s_preferences.fps.curvalue				= Com_Clamp( 0, 1, trap_Cvar_VariableValue("cg_drawFPS") );
	s_preferences.ups.curvalue				= Com_Clamp( 0, 1, trap_Cvar_VariableValue("cg_drawups") );
	s_preferences.realtime.curvalue			= Com_Clamp( 0, 1, trap_Cvar_VariableValue("cg_drawRealTime") );

	s_preferences.crosshair.curvalue		= (int)trap_Cvar_VariableValue( "cg_drawCrosshair" ) % NUM_CROSSHAIRS;
	s_preferences.simpleitems.curvalue		= trap_Cvar_VariableValue( "cg_simpleItems" ) != 0;
	s_preferences.brass.curvalue			= trap_Cvar_VariableValue( "cg_brassTime" ) != 0;
	s_preferences.wallmarks.curvalue		= trap_Cvar_VariableValue( "cg_marks" ) != 0;
	s_preferences.identifytarget.curvalue	= trap_Cvar_VariableValue( "cg_drawCrosshairNames" ) != 0;
	s_preferences.dynamiclights.curvalue	= trap_Cvar_VariableValue( "r_dynamiclight" ) != 0;
	s_preferences.highqualitysky.curvalue	= trap_Cvar_VariableValue ( "r_fastsky" ) == 0;
	s_preferences.synceveryframe.curvalue	= trap_Cvar_VariableValue( "r_finish" ) != 0;
	s_preferences.forcemodel.curvalue		= trap_Cvar_VariableValue( "cg_forcemodel" ) != 0;
	s_preferences.drawteamoverlay.curvalue	= Com_Clamp( 0, 3, trap_Cvar_VariableValue( "cg_drawTeamOverlay" ) );
	s_preferences.allowdownload.curvalue	= trap_Cvar_VariableValue( "cl_allowDownload" ) != 0;
}


static void Preferences_Event( void* ptr, int notification ) {
	if( notification != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_CROSSHAIR:
		s_preferences.crosshair.curvalue++;
		if( s_preferences.crosshair.curvalue == NUM_CROSSHAIRS ) {
			s_preferences.crosshair.curvalue = 0;
		}
		trap_Cvar_SetValue( "cg_drawCrosshair", s_preferences.crosshair.curvalue );
		break;

	case ID_SIMPLEITEMS:
		trap_Cvar_SetValue( "cg_simpleItems", s_preferences.simpleitems.curvalue );
		break;

	case ID_HIGHQUALITYSKY:
		trap_Cvar_SetValue( "r_fastsky", !s_preferences.highqualitysky.curvalue );
		break;

	case ID_EJECTINGBRASS:
		if ( s_preferences.brass.curvalue )
			trap_Cvar_Reset( "cg_brassTime" );
		else
			trap_Cvar_SetValue( "cg_brassTime", 0 );
		break;

	case ID_WALLMARKS:
		trap_Cvar_SetValue( "cg_marks", s_preferences.wallmarks.curvalue );
		break;

	case ID_DYNAMICLIGHTS:
		trap_Cvar_SetValue( "r_dynamiclight", s_preferences.dynamiclights.curvalue );
		break;		

	case ID_IDENTIFYTARGET:
		trap_Cvar_SetValue( "cg_drawCrosshairNames", s_preferences.identifytarget.curvalue );
		break;

	case ID_SYNCEVERYFRAME:
		trap_Cvar_SetValue( "r_finish", s_preferences.synceveryframe.curvalue );
		break;

	case ID_FORCEMODEL:
		trap_Cvar_SetValue( "cg_forcemodel", s_preferences.forcemodel.curvalue );
		break;

	case ID_DRAWTEAMOVERLAY:
		trap_Cvar_SetValue( "cg_drawTeamOverlay", s_preferences.drawteamoverlay.curvalue );
		break;

	case ID_ALLOWDOWNLOAD:
		trap_Cvar_SetValue( "cl_allowDownload", s_preferences.allowdownload.curvalue );
		trap_Cvar_SetValue( "sv_allowDownload", s_preferences.allowdownload.curvalue );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;

	case ID_FFAHUD:
		trap_Cvar_SetValue( "cg_wopFFAhud", s_preferences.ffahud.curvalue );
		break;

	case ID_TIMELEFT:
		trap_Cvar_SetValue( "cg_drawTimeLeft", s_preferences.timeleft.curvalue );
		break;

	case ID_TIMER:
		trap_Cvar_SetValue( "cg_drawTimer", s_preferences.timer.curvalue );
		break;
	case ID_FPS:
		trap_Cvar_SetValue( "cg_drawFPS", s_preferences.fps.curvalue );
		break;
	case ID_UPS:
		trap_Cvar_SetValue( "cg_drawUPS", s_preferences.ups.curvalue );
		break;
	case ID_REALTIME:
		trap_Cvar_SetValue( "cg_drawRealTime", s_preferences.realtime.curvalue );
		break;

	case ID_CONNOTIFY:
		switch(s_preferences.con_notifytime.curvalue)
		{
		case 0:
			trap_Cvar_Set( "con_notifytime", "3" );
			break;
		case 1:
			trap_Cvar_Set( "con_notifytime", "8" );
			break;
		case 2:
			trap_Cvar_Set( "con_notifytime", "-5" );
			break;
		case 3:
			trap_Cvar_Set( "con_notifytime", "-10" );
			break;
		}
		break;
	}
}


/*
=================
Crosshair_Draw
=================
*/
static void Crosshair_Draw( void *self ) {
	menulist_s	*s;
	float		*color;
	int			x, y;
	int			style;
	qboolean	focus;

	s = (menulist_s *)self;
	x = s->generic.x;
	y =	s->generic.y;

	style = UI_SMALLFONT;
	focus = (s->generic.parent->cursor == s->generic.menuPosition);

	if ( s->generic.flags & QMF_GRAYED )
		color = text_color_disabled;
	else if ( focus )
	{
		color = text_color_highlight;
		style |= UI_PULSE;
	}
	else if ( s->generic.flags & QMF_BLINK )
	{
		color = text_color_highlight;
		style |= UI_BLINK;
	}
	else
		color = text_color_normal;

	if ( focus )
	{
		// draw cursor
		UI_FillRect( s->generic.left, s->generic.top, s->generic.right-s->generic.left+1, s->generic.bottom-s->generic.top+1, listbar_color ); 
		UI_DrawChar( x, y, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	}

	UI_DrawString( x - SMALLCHAR_WIDTH, y, s->generic.name, style|UI_RIGHT, color );
	if( !s->curvalue ) {
		return;
	}
	UI_DrawHandlePic( x + SMALLCHAR_WIDTH, y - 4, 24, 24, s_preferences.crosshairShader[s->curvalue] );
}


static void Preferences_MenuInit( void ) {
	int				y;

	memset( &s_preferences, 0 ,sizeof(preferences_t) );

	Preferences_Cache();

	s_preferences.menu.wrapAround = qtrue;
	s_preferences.menu.fullscreen = qtrue;
	s_preferences.menu.bgparts		= BGP_GAMEOPTIONS|BGP_SIMPLEBG;

	y = 110;
	s_preferences.crosshair.generic.type		= MTYPE_TEXT;
	s_preferences.crosshair.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NODEFAULTINIT|QMF_OWNERDRAW;
	s_preferences.crosshair.generic.x			= PREFERENCES_X_POS;
	s_preferences.crosshair.generic.y			= y;
	s_preferences.crosshair.generic.name		= "Crosshair:";
	s_preferences.crosshair.generic.callback	= Preferences_Event;
	s_preferences.crosshair.generic.ownerdraw	= Crosshair_Draw;
	s_preferences.crosshair.generic.id			= ID_CROSSHAIR;
	s_preferences.crosshair.generic.top			= y - 4;
	s_preferences.crosshair.generic.bottom		= y + 20;
	s_preferences.crosshair.generic.left		= PREFERENCES_X_POS - ( ( strlen(s_preferences.crosshair.generic.name) + 1 ) * SMALLCHAR_WIDTH );
	s_preferences.crosshair.generic.right		= PREFERENCES_X_POS + 48;

	y += BIGCHAR_HEIGHT+2+4;
	s_preferences.simpleitems.generic.type        = MTYPE_RADIOBUTTON;
	s_preferences.simpleitems.generic.name	      = "Simple Items:";
	s_preferences.simpleitems.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.simpleitems.generic.callback    = Preferences_Event;
	s_preferences.simpleitems.generic.id          = ID_SIMPLEITEMS;
	s_preferences.simpleitems.generic.x	          = PREFERENCES_X_POS;
	s_preferences.simpleitems.generic.y	          = y;

	y += BIGCHAR_HEIGHT;
	s_preferences.wallmarks.generic.type          = MTYPE_RADIOBUTTON;
	s_preferences.wallmarks.generic.name	      = "Marks on Walls:";
	s_preferences.wallmarks.generic.flags	      = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.wallmarks.generic.callback      = Preferences_Event;
	s_preferences.wallmarks.generic.id            = ID_WALLMARKS;
	s_preferences.wallmarks.generic.x	          = PREFERENCES_X_POS;
	s_preferences.wallmarks.generic.y	          = y;

/* ... not used in wop
	s_preferences.brass.generic.type              = MTYPE_RADIOBUTTON;
	s_preferences.brass.generic.name	          = "Ejecting Brass:";
	s_preferences.brass.generic.flags	          = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.brass.generic.callback          = Preferences_Event;
	s_preferences.brass.generic.id                = ID_EJECTINGBRASS;
	s_preferences.brass.generic.x	              = PREFERENCES_X_POS;
	s_preferences.brass.generic.y	              = y;
*/

	y += BIGCHAR_HEIGHT+2;
	s_preferences.dynamiclights.generic.type      = MTYPE_RADIOBUTTON;
	s_preferences.dynamiclights.generic.name	  = "Dynamic Lights:";
	s_preferences.dynamiclights.generic.flags     = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.dynamiclights.generic.callback  = Preferences_Event;
	s_preferences.dynamiclights.generic.id        = ID_DYNAMICLIGHTS;
	s_preferences.dynamiclights.generic.x	      = PREFERENCES_X_POS;
	s_preferences.dynamiclights.generic.y	      = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.identifytarget.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.identifytarget.generic.name	  = "Identify Target:";
	s_preferences.identifytarget.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.identifytarget.generic.callback = Preferences_Event;
	s_preferences.identifytarget.generic.id       = ID_IDENTIFYTARGET;
	s_preferences.identifytarget.generic.x	      = PREFERENCES_X_POS;
	s_preferences.identifytarget.generic.y	      = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.highqualitysky.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.highqualitysky.generic.name	  = "High Quality Sky:";
	s_preferences.highqualitysky.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.highqualitysky.generic.callback = Preferences_Event;
	s_preferences.highqualitysky.generic.id       = ID_HIGHQUALITYSKY;
	s_preferences.highqualitysky.generic.x	      = PREFERENCES_X_POS;
	s_preferences.highqualitysky.generic.y	      = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.synceveryframe.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.synceveryframe.generic.name	  = "Sync Every Frame:";
	s_preferences.synceveryframe.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.synceveryframe.generic.callback = Preferences_Event;
	s_preferences.synceveryframe.generic.id       = ID_SYNCEVERYFRAME;
	s_preferences.synceveryframe.generic.x	      = PREFERENCES_X_POS;
	s_preferences.synceveryframe.generic.y	      = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.forcemodel.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.forcemodel.generic.name	  = "Force Player Models:";
	s_preferences.forcemodel.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.forcemodel.generic.callback = Preferences_Event;
	s_preferences.forcemodel.generic.id       = ID_FORCEMODEL;
	s_preferences.forcemodel.generic.x	      = PREFERENCES_X_POS;
	s_preferences.forcemodel.generic.y	      = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.drawteamoverlay.generic.type     = MTYPE_SPINCONTROL;
	s_preferences.drawteamoverlay.generic.name	   = "Draw Team Overlay:";
	s_preferences.drawteamoverlay.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.drawteamoverlay.generic.callback = Preferences_Event;
	s_preferences.drawteamoverlay.generic.id       = ID_DRAWTEAMOVERLAY;
	s_preferences.drawteamoverlay.generic.x	       = PREFERENCES_X_POS;
	s_preferences.drawteamoverlay.generic.y	       = y;
	s_preferences.drawteamoverlay.itemnames			= teamoverlay_names;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.ffahud.generic.type		= MTYPE_SPINCONTROL;
	s_preferences.ffahud.generic.name		= "FFA Hud:";
	s_preferences.ffahud.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.ffahud.generic.callback	= Preferences_Event;
	s_preferences.ffahud.generic.id			= ID_FFAHUD;
	s_preferences.ffahud.generic.x			= PREFERENCES_X_POS;
	s_preferences.ffahud.generic.y			= y;
	s_preferences.ffahud.itemnames			= ffahud_names;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.con_notifytime.generic.type		= MTYPE_SPINCONTROL;
	s_preferences.con_notifytime.generic.name		= "Chat:";//"Chat(con_notifytime):";
	s_preferences.con_notifytime.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.con_notifytime.generic.callback	= Preferences_Event;
	s_preferences.con_notifytime.generic.id			= ID_CONNOTIFY;
	s_preferences.con_notifytime.generic.x			= PREFERENCES_X_POS;
	s_preferences.con_notifytime.generic.y			= y;
	s_preferences.con_notifytime.itemnames			= con_notifytime_strs;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.timer.generic.type		= MTYPE_RADIOBUTTON;
	s_preferences.timer.generic.name		= "display Timer:";
	s_preferences.timer.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.timer.generic.callback	= Preferences_Event;
	s_preferences.timer.generic.id			= ID_TIMER;
	s_preferences.timer.generic.x			= PREFERENCES_X_POS;
	s_preferences.timer.generic.y			= y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.timeleft.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.timeleft.generic.name	    = "display Timeleft:";
	s_preferences.timeleft.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.timeleft.generic.callback = Preferences_Event;
	s_preferences.timeleft.generic.id       = ID_TIMELEFT;
	s_preferences.timeleft.generic.x	    = PREFERENCES_X_POS;
	s_preferences.timeleft.generic.y	    = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.realtime.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.realtime.generic.name	    = "display RealTime:";
	s_preferences.realtime.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.realtime.generic.callback = Preferences_Event;
	s_preferences.realtime.generic.id       = ID_REALTIME;
	s_preferences.realtime.generic.x	    = PREFERENCES_X_POS;
	s_preferences.realtime.generic.y	    = y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.ups.generic.type			= MTYPE_RADIOBUTTON;
	s_preferences.ups.generic.name			= "display run-speed:";
	s_preferences.ups.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.ups.generic.callback		= Preferences_Event;
	s_preferences.ups.generic.id			= ID_UPS;
	s_preferences.ups.generic.x				= PREFERENCES_X_POS;
	s_preferences.ups.generic.y				= y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.fps.generic.type			= MTYPE_RADIOBUTTON;
	s_preferences.fps.generic.name			= "display frames/second:";
	s_preferences.fps.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.fps.generic.callback		= Preferences_Event;
	s_preferences.fps.generic.id			= ID_FPS;
	s_preferences.fps.generic.x				= PREFERENCES_X_POS;
	s_preferences.fps.generic.y				= y;

	y += BIGCHAR_HEIGHT+2;
	s_preferences.allowdownload.generic.type     = MTYPE_RADIOBUTTON;
	s_preferences.allowdownload.generic.name	   = "Automatic Downloading:";
	s_preferences.allowdownload.generic.flags	   = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_preferences.allowdownload.generic.callback = Preferences_Event;
	s_preferences.allowdownload.generic.id       = ID_ALLOWDOWNLOAD;
	s_preferences.allowdownload.generic.x	       = PREFERENCES_X_POS;
	s_preferences.allowdownload.generic.y	       = y;

	s_preferences.back.generic.type		= MTYPE_BITMAP;
	s_preferences.back.generic.name		= ART_BACK0;
	s_preferences.back.generic.flags	= QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_preferences.back.generic.x		= 552;//8;
	s_preferences.back.generic.y		= 440;
	s_preferences.back.generic.id		= ID_BACK;
	s_preferences.back.generic.callback	= Preferences_Event;
    s_preferences.back.width			= 80;
	s_preferences.back.height			= 40;
	s_preferences.back.focuspic			= ART_BACK1;
	s_preferences.back.focuspicinstead	= qtrue;


	Menu_AddItem( &s_preferences.menu, &s_preferences.crosshair );
	Menu_AddItem( &s_preferences.menu, &s_preferences.simpleitems );
	Menu_AddItem( &s_preferences.menu, &s_preferences.wallmarks );

//	Menu_AddItem( &s_preferences.menu, &s_preferences.brass );

	Menu_AddItem( &s_preferences.menu, &s_preferences.dynamiclights );
	Menu_AddItem( &s_preferences.menu, &s_preferences.identifytarget );
	Menu_AddItem( &s_preferences.menu, &s_preferences.highqualitysky );
	Menu_AddItem( &s_preferences.menu, &s_preferences.synceveryframe );
	Menu_AddItem( &s_preferences.menu, &s_preferences.forcemodel );
	Menu_AddItem( &s_preferences.menu, &s_preferences.drawteamoverlay );

	Menu_AddItem( &s_preferences.menu, &s_preferences.ffahud );
	Menu_AddItem( &s_preferences.menu, &s_preferences.con_notifytime );
	Menu_AddItem( &s_preferences.menu, &s_preferences.timer );
	Menu_AddItem( &s_preferences.menu, &s_preferences.timeleft );
	Menu_AddItem( &s_preferences.menu, &s_preferences.realtime );
	Menu_AddItem( &s_preferences.menu, &s_preferences.ups );
	Menu_AddItem( &s_preferences.menu, &s_preferences.fps );

	Menu_AddItem( &s_preferences.menu, &s_preferences.allowdownload );

	Menu_AddItem( &s_preferences.menu, &s_preferences.back );

	Preferences_SetMenuItems();
}


/*
===============
Preferences_Cache
===============
*/
void Preferences_Cache( void ) {
	int		n;

	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	for( n = 0; n < NUM_CROSSHAIRS; n++ ) {
		s_preferences.crosshairShader[n] = trap_R_RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + n ) );
	}
}


/*
===============
UI_PreferencesMenu
===============
*/
void UI_PreferencesMenu( void ) {
	Preferences_MenuInit();
	UI_PushMenu( &s_preferences.menu );
}
