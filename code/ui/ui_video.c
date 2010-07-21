// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "ui_local.h"

void GraphicsOptions_MenuInit( void );

void LaunchCustomResoMenu(void);

/*
=======================================================================

DRIVER INFORMATION MENU

=======================================================================
*/

#define BACK0		"menu/BtnBack0"
#define BACK1		"menu/BtnBack1"

#define ID_DRIVERINFOBACK	100
#define MAX_EXTENSIONSINFOS 256
#define MAX_EXTSTRBUFFSIZE	4096

typedef struct
{
	menuframework_s	menu;
	menubitmap_s	back;

	char			stringbuff[MAX_EXTSTRBUFFSIZE];
	char*			strings[MAX_EXTENSIONSINFOS];
	int				scrollpos;
	int				minscroll;
	int				numstrings;
} driverinfo_t;

static driverinfo_t	s_driverinfo;

/*
=================
DriverInfo_Event
=================
*/
static void DriverInfo_Event( void* ptr, int event )
{
	if (event != QM_ACTIVATED)
		return;

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_DRIVERINFOBACK:
			UI_PopMenu();
			break;
	}
}

/*
=================
DriverInfo_MenuDraw
=================
*/
static void DriverInfo_MenuDraw( void )
{
	int	i;
	int	y;

	Menu_Draw( &s_driverinfo.menu );

	y=s_driverinfo.scrollpos;
	UI_DrawString( 320, y, "VENDOR", UI_CENTER|UI_SMALLFONT, color_red );
	UI_DrawString( 320, y+16, uis.glconfig.vendor_string, UI_CENTER|UI_SMALLFONT, text_color_normal );
	UI_DrawString( 320, y+32, uis.glconfig.version_string, UI_CENTER|UI_SMALLFONT, text_color_normal );
	UI_DrawString( 320, y+48, uis.glconfig.renderer_string, UI_CENTER|UI_SMALLFONT, text_color_normal );

	y+=(48+40);
	UI_DrawString( 320, y, "PIXELFORMAT", UI_CENTER|UI_SMALLFONT, color_red );
	UI_DrawString( 320, y+16, va ("color(%d-bits) Z(%d-bits) stencil(%d-bits)", uis.glconfig.colorBits, uis.glconfig.depthBits, uis.glconfig.stencilBits), UI_CENTER|UI_SMALLFONT, text_color_normal );
	y+=20;
	UI_DrawString( 320, y+16, va("maxTextureSize: %d",uis.glconfig.maxTextureSize), UI_CENTER|UI_SMALLFONT, text_color_normal );

	y+=40;
	UI_DrawString( 320, y, "EXTENSIONS", UI_CENTER|UI_SMALLFONT, color_red );


	// double column
	y+=16;
	for (i=0; i<s_driverinfo.numstrings/2; i++) {
		UI_DrawString( 320-4, y, s_driverinfo.strings[i*2], UI_RIGHT|UI_SMALLFONT, text_color_normal );
		UI_DrawString( 320+4, y, s_driverinfo.strings[i*2+1], UI_LEFT|UI_SMALLFONT, text_color_normal );
		y += SMALLCHAR_HEIGHT;
	}

	if (s_driverinfo.numstrings & 1)
		UI_DrawString( 320, y, s_driverinfo.strings[s_driverinfo.numstrings-1], UI_CENTER|UI_SMALLFONT, text_color_normal );
}

/*
=================
DriverInfo_Cache
=================
*/
void DriverInfo_Cache( void )
{
	trap_R_RegisterShaderNoMip(BACK0);
	trap_R_RegisterShaderNoMip(BACK1);
}

/*
#######################

  DriverInfo_Key

#######################
*/
static sfxHandle_t DriverInfo_Key( int key ) {
	switch ( key ) {
	case K_MWHEELDOWN:
	case K_PGDN:
		if(s_driverinfo.scrollpos>=(s_driverinfo.minscroll+16))
			s_driverinfo.scrollpos-=16;
		else
			s_driverinfo.scrollpos=s_driverinfo.minscroll;
		break;
	case K_MWHEELUP:
	case K_PGUP:
		if(s_driverinfo.scrollpos<=(10-16))
			s_driverinfo.scrollpos+=16;
		else
			s_driverinfo.scrollpos=10;
		break;
	}

	return Menu_DefaultKey( &s_driverinfo.menu, key );
}

/*
=================
UI_DriverInfo_Menu
=================
*/
static void UI_DriverInfo_Menu( void )
{
	char*	eptr;
	int		i;
	int		len;

	// zero set all our globals
	memset( &s_driverinfo, 0 ,sizeof(driverinfo_t) );

	DriverInfo_Cache();

	s_driverinfo.menu.fullscreen = qtrue;
	s_driverinfo.menu.draw       = DriverInfo_MenuDraw;

	s_driverinfo.menu.bgparts	 = BGP_SIMPLEBG;
	s_driverinfo.menu.key		 = DriverInfo_Key;

	s_driverinfo.back.generic.type		= MTYPE_BITMAP;
	s_driverinfo.back.generic.name		= BACK0;
	s_driverinfo.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_driverinfo.back.generic.callback	= DriverInfo_Event;
	s_driverinfo.back.generic.id		= ID_DRIVERINFOBACK;
    s_driverinfo.back.generic.x			= 8;
	s_driverinfo.back.generic.y			= 440;
	s_driverinfo.back.width				= 80;
	s_driverinfo.back.height			= 40;
	s_driverinfo.back.focuspic			= BACK1;
	s_driverinfo.back.focuspicinstead	= qtrue;

	Q_strncpyz(s_driverinfo.stringbuff, uis.glconfig.extensions_string, MAX_EXTSTRBUFFSIZE );

	// build null terminated extension strings
	eptr = s_driverinfo.stringbuff;
	while ( s_driverinfo.numstrings<MAX_EXTENSIONSINFOS && *eptr )
	{
		while ( *eptr && *eptr == ' ' )
			*eptr++ = '\0';

		// track start of valid string
		if (*eptr && *eptr != ' ')
			s_driverinfo.strings[s_driverinfo.numstrings++] = eptr;

		while ( *eptr && *eptr != ' ' )
			eptr++;
	}

	// safety length strings for display
	for (i=0; i<s_driverinfo.numstrings; i++) {
		len = strlen(s_driverinfo.strings[i]);
		if (len > 32) {
			s_driverinfo.strings[i][len-1] = '>';
			s_driverinfo.strings[i][len]   = '\0';
		}
	}

	s_driverinfo.scrollpos=10;
	s_driverinfo.minscroll=470-(154+((s_driverinfo.numstrings/2)+1)*16);

	Menu_AddItem( &s_driverinfo.menu, &s_driverinfo.back );

	UI_PushMenu( &s_driverinfo.menu );
}

/*
=======================================================================

GRAPHICS OPTIONS MENU

=======================================================================
*/

#define CUSTOM_RESO 11

#define BACK0		"menu/BtnBack0"
#define BACK1		"menu/BtnBack1"
#define GRAPHICS0	"menu/system/graphics0"
#define GRASHICS1	"menu/system/graphics1"
#define DISPLAY0	"menu/system/display0"
#define DISPLAY1	"menu/system/display1"
#define SOUND0		"menu/system/sound0"
#define SOUND1		"menu/system/sound1"
#define NETWORK0	"menu/system/network0"
#define NETWORK1	"menu/system/network1"
#define DRIVERINFO0	"menu/system/driverinfo0"
#define DRIVERINFO1	"menu/system/driverinfo1"
#define ACCEPT0		"menu/system/accept"
#define ACCEPT1		"menu/system/accept"

static const char *s_drivers[] =
{
	OPENGL_DRIVER_NAME,
	_3DFX_DRIVER_NAME,
	0
};

#define ID_BACK2		101
#define ID_FULLSCREEN	102
#define ID_LIST			103
#define ID_MODE			104
#define ID_DRIVERINFO	105
#define ID_GRAPHICS		106
#define ID_DISPLAY		107
#define ID_SOUND		108
#define ID_NETWORK		109
#define ID_CUSTOMRESO	110

#define MAX_CUSTOMRESOSTR 128

typedef struct {
	menuframework_s	menu;

	menubitmap_s	graphics;
	menubitmap_s	display;
	menubitmap_s	sound;
	menubitmap_s	network;

	menulist_s		list;
	menulist_s		mode;
	menulist_s		driver;
	menuslider_s	tq;
	menulist_s  	fs;
	menulist_s  	lighting;
	menulist_s  	allow_extensions;
	menulist_s  	texturebits;
	menulist_s  	colordepth;
	menulist_s  	geometry;
	menulist_s  	filter;
	menutext_s		driverinfo;
	menutext_s		customReso;
	menulist_s		flares;

	menubitmap_s	apply;
	menubitmap_s	back;

	char			customReso_str[MAX_CUSTOMRESOSTR];
	qboolean		customResoChanged;
} graphicsoptions_t;

typedef struct
{
	int mode;
	qboolean fullscreen;
	int tq;
	int lighting;
	int colordepth;
	int texturebits;
	int geometry;
	int filter;
	int driver;
	qboolean extensions;
	qboolean flares;
} InitialVideoOptions_s;

static InitialVideoOptions_s	s_ivo;
static graphicsoptions_t		s_graphicsoptions;	

static InitialVideoOptions_s s_ivo_templates[] =
{
	{
		6, qtrue, 3, 0, 2, 2, 2, 1, 0, qtrue, qtrue
	},
	{
		6, qtrue, 3, 0, 0, 0, 1, 0, 0, qtrue, qtrue
	},
	{
		3, qtrue, 2, 0, 1, 0, 0, 0, 0, qtrue, qfalse
	},
	{
		3, qtrue, 1, 1, 1, 0, 0, 0, 0, qtrue, qfalse
	},
	{
		3, qtrue, 1, 0, 0, 0, 1, 0, 0, qtrue, qfalse
	}
};

#define NUM_IVO_TEMPLATES ( sizeof( s_ivo_templates ) / sizeof( s_ivo_templates[0] ) )

/*
=================
GraphicsOptions_GetInitialVideo
=================
*/
static void GraphicsOptions_GetInitialVideo( void )
{
	s_ivo.colordepth  = s_graphicsoptions.colordepth.curvalue;
	s_ivo.driver      = s_graphicsoptions.driver.curvalue;
	s_ivo.mode        = s_graphicsoptions.mode.curvalue;
	s_ivo.fullscreen  = s_graphicsoptions.fs.curvalue;
	s_ivo.flares	  = s_graphicsoptions.flares.curvalue;
	s_ivo.extensions  = s_graphicsoptions.allow_extensions.curvalue;
	s_ivo.tq          = s_graphicsoptions.tq.curvalue;
	s_ivo.lighting    = s_graphicsoptions.lighting.curvalue;
	s_ivo.geometry    = s_graphicsoptions.geometry.curvalue;
	s_ivo.filter      = s_graphicsoptions.filter.curvalue;
	s_ivo.texturebits = s_graphicsoptions.texturebits.curvalue;
}

/*
=================
GraphicsOptions_CheckConfig
=================
*/
static void GraphicsOptions_CheckConfig( void )
{
	int i;

	for ( i = 0; i < NUM_IVO_TEMPLATES; i++ )
	{
		if ( s_ivo_templates[i].colordepth != s_graphicsoptions.colordepth.curvalue )
			continue;
		if ( s_ivo_templates[i].driver != s_graphicsoptions.driver.curvalue )
			continue;
		if ( s_ivo_templates[i].mode != s_graphicsoptions.mode.curvalue )
			continue;
		if ( s_ivo_templates[i].fullscreen != s_graphicsoptions.fs.curvalue )
			continue;
		if ( s_ivo_templates[i].tq != s_graphicsoptions.tq.curvalue )
			continue;
		if ( s_ivo_templates[i].lighting != s_graphicsoptions.lighting.curvalue )
			continue;
		if ( s_ivo_templates[i].geometry != s_graphicsoptions.geometry.curvalue )
			continue;
		if ( s_ivo_templates[i].filter != s_graphicsoptions.filter.curvalue )
			continue;
//		if ( s_ivo_templates[i].texturebits != s_graphicsoptions.texturebits.curvalue )
//			continue;
		if ( s_ivo_templates[i].flares != s_graphicsoptions.flares.curvalue )
			continue;
		s_graphicsoptions.list.curvalue = i;
		return;
	}
	s_graphicsoptions.list.curvalue = 4;
}

/*
=================
GraphicsOptions_UpdateMenuItems
=================
*/
static void GraphicsOptions_UpdateMenuItems( void )
{
	if ( s_graphicsoptions.driver.curvalue == 1 )
	{
		s_graphicsoptions.fs.curvalue = 1;
		s_graphicsoptions.fs.generic.flags |= QMF_GRAYED;
		s_graphicsoptions.colordepth.curvalue = 1;
	}
	else
	{
		s_graphicsoptions.fs.generic.flags &= ~QMF_GRAYED;
	}

	if ( s_graphicsoptions.fs.curvalue == 0 || s_graphicsoptions.driver.curvalue == 1 )
	{
		s_graphicsoptions.colordepth.curvalue = 0;
		s_graphicsoptions.colordepth.generic.flags |= QMF_GRAYED;
	}
	else
	{
		s_graphicsoptions.colordepth.generic.flags &= ~QMF_GRAYED;
	}

	if ( s_graphicsoptions.allow_extensions.curvalue == 0 )
	{
		if ( s_graphicsoptions.texturebits.curvalue == 0 )
		{
			s_graphicsoptions.texturebits.curvalue = 1;
		}
	}

	s_graphicsoptions.apply.generic.flags |= QMF_HIDDEN|QMF_INACTIVE;

	if ( s_ivo.mode != s_graphicsoptions.mode.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.fullscreen != s_graphicsoptions.fs.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.flares != s_graphicsoptions.flares.curvalue ) {
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.extensions != s_graphicsoptions.allow_extensions.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.tq != s_graphicsoptions.tq.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.lighting != s_graphicsoptions.lighting.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.colordepth != s_graphicsoptions.colordepth.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.driver != s_graphicsoptions.driver.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.texturebits != s_graphicsoptions.texturebits.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.geometry != s_graphicsoptions.geometry.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_ivo.filter != s_graphicsoptions.filter.curvalue )
	{
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	}
	if ( s_graphicsoptions.mode.curvalue==CUSTOM_RESO && s_graphicsoptions.customResoChanged )
		s_graphicsoptions.apply.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);

	GraphicsOptions_CheckConfig();
}	

/*
=================
GraphicsOptions_ApplyChanges
=================
*/
static void GraphicsOptions_ApplyChanges( void *unused, int notification )
{
	if (notification != QM_ACTIVATED)
		return;

	switch ( s_graphicsoptions.texturebits.curvalue  )
	{
	case 0:
		trap_Cvar_SetValue( "r_texturebits", 0 );
		break;
	case 1:
		trap_Cvar_SetValue( "r_texturebits", 16 );
		break;
	case 2:
		trap_Cvar_SetValue( "r_texturebits", 32 );
		break;
	}
	trap_Cvar_SetValue( "r_picmip", 3 - s_graphicsoptions.tq.curvalue );
	trap_Cvar_SetValue( "r_allowExtensions", s_graphicsoptions.allow_extensions.curvalue );
	if(s_graphicsoptions.mode.curvalue == CUSTOM_RESO)
		trap_Cvar_Set( "r_mode", "-1" );
	else
		trap_Cvar_SetValue( "r_mode", s_graphicsoptions.mode.curvalue );
	trap_Cvar_SetValue( "r_fullscreen", s_graphicsoptions.fs.curvalue );
	trap_Cvar_SetValue( "r_flares",s_graphicsoptions.flares.curvalue );
	trap_Cvar_Set( "r_glDriver", ( char * ) s_drivers[s_graphicsoptions.driver.curvalue] );
	switch ( s_graphicsoptions.colordepth.curvalue )
	{
	case 0:
		trap_Cvar_SetValue( "r_colorbits", 0 );
		trap_Cvar_SetValue( "r_depthbits", 0 );
		trap_Cvar_SetValue( "r_stencilbits", 0 );
		break;
	case 1:
		trap_Cvar_SetValue( "r_colorbits", 16 );
		trap_Cvar_SetValue( "r_depthbits", 16 );
		trap_Cvar_SetValue( "r_stencilbits", 0 );
		break;
	case 2:
		trap_Cvar_SetValue( "r_colorbits", 32 );
		trap_Cvar_SetValue( "r_depthbits", 24 );
		break;
	}
	trap_Cvar_SetValue( "r_vertexLight", s_graphicsoptions.lighting.curvalue );

	if ( s_graphicsoptions.geometry.curvalue == 2 )
	{
		trap_Cvar_SetValue( "r_lodBias", 0 );
		trap_Cvar_SetValue( "r_subdivisions", 4 );
	}
	else if ( s_graphicsoptions.geometry.curvalue == 1 )
	{
		trap_Cvar_SetValue( "r_lodBias", 1 );
		trap_Cvar_SetValue( "r_subdivisions", 12 );
	}
	else
	{
		trap_Cvar_SetValue( "r_lodBias", 1 );
		trap_Cvar_SetValue( "r_subdivisions", 20 );
	}

	if ( s_graphicsoptions.filter.curvalue )
	{
		trap_Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_LINEAR" );
	}
	else
	{
		trap_Cvar_Set( "r_textureMode", "GL_LINEAR_MIPMAP_NEAREST" );
	}

	trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}

/*
=================
GraphicsOptions_Event
=================
*/
static void GraphicsOptions_Event( void* ptr, int event ) {
	InitialVideoOptions_s *ivo;

	if( event != QM_ACTIVATED ) {
	 	return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_MODE:
		// clamp 3dfx video modes
		if ( s_graphicsoptions.driver.curvalue == 1 )
		{
			if ( s_graphicsoptions.mode.curvalue < 2 )
				s_graphicsoptions.mode.curvalue = 2;
			else if ( s_graphicsoptions.mode.curvalue > 6 )
				s_graphicsoptions.mode.curvalue = 6;
		}

		if( s_graphicsoptions.mode.curvalue==CUSTOM_RESO )
			s_graphicsoptions.customReso.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
		else
			s_graphicsoptions.customReso.generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
		break;

	case ID_LIST:
		ivo = &s_ivo_templates[s_graphicsoptions.list.curvalue];

		s_graphicsoptions.mode.curvalue        = ivo->mode;
		s_graphicsoptions.tq.curvalue          = ivo->tq;
		s_graphicsoptions.lighting.curvalue    = ivo->lighting;
		s_graphicsoptions.colordepth.curvalue  = ivo->colordepth;
		s_graphicsoptions.texturebits.curvalue = ivo->texturebits;
		s_graphicsoptions.geometry.curvalue    = ivo->geometry;
		s_graphicsoptions.filter.curvalue      = ivo->filter;
		s_graphicsoptions.fs.curvalue          = ivo->fullscreen;
		s_graphicsoptions.flares.curvalue	   = ivo->flares;
		break;

	case ID_DRIVERINFO:
		UI_DriverInfo_Menu();
		break;

	case ID_CUSTOMRESO:
		LaunchCustomResoMenu();
		break;

	case ID_BACK2:
		UI_PopMenu();
		break;

	case ID_GRAPHICS:
		break;

	case ID_DISPLAY:
		UI_PopMenu();
		UI_DisplayOptionsMenu();
		break;

	case ID_SOUND:
		UI_PopMenu();
		UI_SoundOptionsMenu();
		break;

	case ID_NETWORK:
		UI_PopMenu();
		UI_NetworkOptionsMenu();
		break;
	}
}


/*
================
GraphicsOptions_TQEvent
================
*/
static void GraphicsOptions_TQEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
	 	return;
	}
	s_graphicsoptions.tq.curvalue = (int)(s_graphicsoptions.tq.curvalue + 0.5);
}


/*
================
GraphicsOptions_MenuDraw
================
*/
void GraphicsOptions_MenuDraw (void)
{
//APSFIX - rework this
	GraphicsOptions_UpdateMenuItems();

	Menu_Draw( &s_graphicsoptions.menu );
}

/*
=================
GraphicsOptions_SetMenuItems
=================
*/
static void GraphicsOptions_SetMenuItems( void )
{
	s_graphicsoptions.mode.curvalue = trap_Cvar_VariableValue( "r_mode" );
	if( s_graphicsoptions.mode.curvalue == -1 )
		s_graphicsoptions.mode.curvalue = CUSTOM_RESO;
	else if ( s_graphicsoptions.mode.curvalue < 0 )
		s_graphicsoptions.mode.curvalue = 3;
	if( s_graphicsoptions.mode.curvalue==CUSTOM_RESO )
		s_graphicsoptions.customReso.generic.flags &= ~(QMF_HIDDEN|QMF_INACTIVE);
	else
		s_graphicsoptions.customReso.generic.flags |= (QMF_HIDDEN|QMF_INACTIVE);
	s_graphicsoptions.fs.curvalue = trap_Cvar_VariableValue("r_fullscreen");
	s_graphicsoptions.flares.curvalue = trap_Cvar_VariableValue("r_flares");
	s_graphicsoptions.allow_extensions.curvalue = trap_Cvar_VariableValue("r_allowExtensions");
	s_graphicsoptions.tq.curvalue = 3-trap_Cvar_VariableValue( "r_picmip");
	if ( s_graphicsoptions.tq.curvalue < 0 )
	{
		s_graphicsoptions.tq.curvalue = 0;
	}
	else if ( s_graphicsoptions.tq.curvalue > 3 )
	{
		s_graphicsoptions.tq.curvalue = 3;
	}

	s_graphicsoptions.lighting.curvalue = trap_Cvar_VariableValue( "r_vertexLight" ) != 0;
	switch ( ( int ) trap_Cvar_VariableValue( "r_texturebits" ) )
	{
	default:
	case 0:
		s_graphicsoptions.texturebits.curvalue = 0;
		break;
	case 16:
		s_graphicsoptions.texturebits.curvalue = 1;
		break;
	case 32:
		s_graphicsoptions.texturebits.curvalue = 2;
		break;
	}

	if ( !Q_stricmp( UI_Cvar_VariableString( "r_textureMode" ), "GL_LINEAR_MIPMAP_NEAREST" ) )
	{
		s_graphicsoptions.filter.curvalue = 0;
	}
	else
	{
		s_graphicsoptions.filter.curvalue = 1;
	}

	if ( trap_Cvar_VariableValue( "r_lodBias" ) > 0 )
	{
		if ( trap_Cvar_VariableValue( "r_subdivisions" ) >= 20 )
		{
			s_graphicsoptions.geometry.curvalue = 0;
		}
		else
		{
			s_graphicsoptions.geometry.curvalue = 1;
		}
	}
	else
	{
		s_graphicsoptions.geometry.curvalue = 2;
	}

	switch ( ( int ) trap_Cvar_VariableValue( "r_colorbits" ) )
	{
	default:
	case 0:
		s_graphicsoptions.colordepth.curvalue = 0;
		break;
	case 16:
		s_graphicsoptions.colordepth.curvalue = 1;
		break;
	case 32:
		s_graphicsoptions.colordepth.curvalue = 2;
		break;
	}

	if ( s_graphicsoptions.fs.curvalue == 0 )
	{
		s_graphicsoptions.colordepth.curvalue = 0;
	}
	if ( s_graphicsoptions.driver.curvalue == 1 )
	{
		s_graphicsoptions.colordepth.curvalue = 1;
	}
}

static void GraphicsOptions_DrawCustomReso(void* self)
{
	menutext_s *b = (menutext_s*)self;
	float	x = (b->generic.right+b->generic.left)*0.5f;
	float	y = b->generic.top;
	float	w = b->generic.right-b->generic.left;
	float	h = b->generic.bottom-b->generic.top+1;

	if (!(Menu_ItemAtCursor( b->generic.parent ) == b))
	{
		UI_FillRect(b->generic.left,y,w,h,colorBlack);// listbar_color
		UI_DrawRect(b->generic.left,y,w,h,colorWhite);
		UI_DrawStringNS(x,y+1,b->string,UI_CENTER,14.0f,text_color_normal);
	}
	else
	{
		UI_FillRect(b->generic.left,y,w,h,colorBlack);// listbar_color
		UI_DrawStringNS(x,y+1,b->string,UI_CENTER,14.0f,text_color_highlight);
		UI_DrawRect(b->generic.left,y,w,h,colorWhite);
	}

//	UI_DrawRect(b->generic.left,y,w,h,colorBlack);
}

/*
================
GraphicsOptions_MenuInit
================
*/
void GraphicsOptions_MenuInit( void )
{
	static const char *s_driver_names[] =
	{
		"Default",
		"Voodoo",
		0
	};

	static const char *tq_names[] =
	{
		"Default",
		"16 bit",
		"32 bit",
		0
	};

	static const char *s_graphics_options_names[] =
	{
		"High Quality",
		"Normal",
		"Fast",
		"Faster",
		"Custom",
		0
	};

	static const char *lighting_names[] =
	{
		"Lightmap",
		"Vertex",
		0
	};

	static const char *colordepth_names[] =
	{
		"Default",
		"16 bit",
		"32 bit",
		0
	};

	static const char *resolutions[] = 
	{
		"320x240",
		"400x300",
		"512x384",
		"640x480",
		"800x600",
		"960x720",
		"1024x768",
		"1152x864",
		"1280x1024",
		"1600x1200",
		"2048x1536",
//(original)		"856x480 wide screen",
		"custom", // 11 ... IF CHANGING THIS LIST -> change the CUSTOM_RESO define !!!!!
		0
	};
	static const char *filter_names[] =
	{
		"Bilinear",
		"Trilinear",
		0
	};
	static const char *quality_names[] =
	{
		"Low",
		"Medium",
		"High",
		0
	};
	static const char *enabled_names[] =
	{
		"Off",
		"On",
		0
	};

	int y;

	// zero set all our globals
	memset( &s_graphicsoptions, 0 ,sizeof(graphicsoptions_t) );

	GraphicsOptions_Cache();

	s_graphicsoptions.menu.wrapAround = qtrue;
	s_graphicsoptions.menu.fullscreen = qtrue;
	s_graphicsoptions.menu.draw       = GraphicsOptions_MenuDraw;
	s_graphicsoptions.menu.bgparts	= BGP_SYSTEMBG|BGP_SIMPLEBG;

	s_graphicsoptions.graphics.generic.type		= MTYPE_BITMAP;
	s_graphicsoptions.graphics.generic.name		= GRAPHICS0;
	s_graphicsoptions.graphics.generic.flags	= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_graphicsoptions.graphics.generic.callback	= GraphicsOptions_Event;
	s_graphicsoptions.graphics.generic.id		= ID_GRAPHICS;
	s_graphicsoptions.graphics.generic.x		= 26;
	s_graphicsoptions.graphics.generic.y		= 37;
	s_graphicsoptions.graphics.width			= 130;
	s_graphicsoptions.graphics.height			= 40;
	s_graphicsoptions.graphics.focuspic			= GRASHICS1;
	s_graphicsoptions.graphics.focuspicinstead	= qtrue;

	s_graphicsoptions.display.generic.type		= MTYPE_BITMAP;
	s_graphicsoptions.display.generic.name		= DISPLAY0;
	s_graphicsoptions.display.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_graphicsoptions.display.generic.callback	= GraphicsOptions_Event;
	s_graphicsoptions.display.generic.id		= ID_DISPLAY;
	s_graphicsoptions.display.generic.x			= 159;
	s_graphicsoptions.display.generic.y			= 37;
	s_graphicsoptions.display.width				= 122;
	s_graphicsoptions.display.height			= 40;
	s_graphicsoptions.display.focuspic			= DISPLAY1;
	s_graphicsoptions.display.focuspicinstead	= qtrue;

	s_graphicsoptions.sound.generic.type		= MTYPE_BITMAP;
	s_graphicsoptions.sound.generic.name		= SOUND0;
	s_graphicsoptions.sound.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_graphicsoptions.sound.generic.callback	= GraphicsOptions_Event;
	s_graphicsoptions.sound.generic.id			= ID_SOUND;
	s_graphicsoptions.sound.generic.x			= 57;
	s_graphicsoptions.sound.generic.y			= 77;
	s_graphicsoptions.sound.width				= 130;
	s_graphicsoptions.sound.height				= 40;
	s_graphicsoptions.sound.focuspic			= SOUND1;
	s_graphicsoptions.sound.focuspicinstead		= qtrue;

	s_graphicsoptions.network.generic.type		= MTYPE_BITMAP;
	s_graphicsoptions.network.generic.name		= NETWORK0;
	s_graphicsoptions.network.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_graphicsoptions.network.generic.callback	= GraphicsOptions_Event;
	s_graphicsoptions.network.generic.id		= ID_NETWORK;
	s_graphicsoptions.network.generic.x			= 194;
	s_graphicsoptions.network.generic.y			= 94;
	s_graphicsoptions.network.width				= 95;
	s_graphicsoptions.network.height			= 40;
	s_graphicsoptions.network.focuspic			= NETWORK1;
	s_graphicsoptions.network.focuspicinstead	= qtrue;

	y = 200;
#define X_OFMAINPART 160 //175

	s_graphicsoptions.list.generic.type     = MTYPE_SPINCONTROL;
	s_graphicsoptions.list.generic.name     = "Graphics Settings:";
	s_graphicsoptions.list.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.list.generic.x        = X_OFMAINPART;
	s_graphicsoptions.list.generic.y        = y;
	s_graphicsoptions.list.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.list.generic.id       = ID_LIST;
	s_graphicsoptions.list.itemnames        = s_graphics_options_names;
	y += 2 * ( BIGCHAR_HEIGHT + 2 );

	s_graphicsoptions.driver.generic.type  = MTYPE_SPINCONTROL;
	s_graphicsoptions.driver.generic.name  = "GL Driver:";
	s_graphicsoptions.driver.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.driver.generic.x     = X_OFMAINPART;
	s_graphicsoptions.driver.generic.y     = y;
	s_graphicsoptions.driver.itemnames     = s_driver_names;
	s_graphicsoptions.driver.curvalue      = (uis.glconfig.driverType == GLDRV_VOODOO);
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_allowExtensions"
	s_graphicsoptions.allow_extensions.generic.type     = MTYPE_SPINCONTROL;
	s_graphicsoptions.allow_extensions.generic.name	    = "GL Extensions:";
	s_graphicsoptions.allow_extensions.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.allow_extensions.generic.x	    = X_OFMAINPART;
	s_graphicsoptions.allow_extensions.generic.y	    = y;
	s_graphicsoptions.allow_extensions.itemnames        = enabled_names;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_mode"
	s_graphicsoptions.mode.generic.type     = MTYPE_SPINCONTROL;
	s_graphicsoptions.mode.generic.name     = "Video Mode:";
	s_graphicsoptions.mode.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.mode.generic.x        = X_OFMAINPART;
	s_graphicsoptions.mode.generic.y        = y;
	s_graphicsoptions.mode.itemnames        = resolutions;
	s_graphicsoptions.mode.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.mode.generic.id       = ID_MODE;

//customResoChanged
	Q_strncpyz(s_graphicsoptions.customReso_str,"12345x12345",MAX_CUSTOMRESOSTR);
	s_graphicsoptions.customReso.generic.type     = MTYPE_TEXTS;
	s_graphicsoptions.customReso.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.customReso.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.customReso.generic.id       = ID_CUSTOMRESO;
	s_graphicsoptions.customReso.generic.x        = X_OFMAINPART+60;
	s_graphicsoptions.customReso.generic.y        = y;
	s_graphicsoptions.customReso.string           = s_graphicsoptions.customReso_str;
	s_graphicsoptions.customReso.style            = UI_SMALLFONT;
	s_graphicsoptions.customReso.color            = color_yellow;
	s_graphicsoptions.customReso.generic.ownerdraw= GraphicsOptions_DrawCustomReso;
	y += BIGCHAR_HEIGHT+2;

	// references "r_colorbits"
	s_graphicsoptions.colordepth.generic.type     = MTYPE_SPINCONTROL;
	s_graphicsoptions.colordepth.generic.name     = "Color Depth:";
	s_graphicsoptions.colordepth.generic.flags    = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.colordepth.generic.x        = X_OFMAINPART;
	s_graphicsoptions.colordepth.generic.y        = y;
	s_graphicsoptions.colordepth.itemnames        = colordepth_names;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_fullscreen"
	s_graphicsoptions.fs.generic.type     = MTYPE_SPINCONTROL;
	s_graphicsoptions.fs.generic.name	  = "Fullscreen:";
	s_graphicsoptions.fs.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.fs.generic.x	      = X_OFMAINPART;
	s_graphicsoptions.fs.generic.y	      = y;
	s_graphicsoptions.fs.itemnames	      = enabled_names;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_vertexLight"
	s_graphicsoptions.lighting.generic.type  = MTYPE_SPINCONTROL;
	s_graphicsoptions.lighting.generic.name	 = "Lighting:";
	s_graphicsoptions.lighting.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.lighting.generic.x	 = X_OFMAINPART;
	s_graphicsoptions.lighting.generic.y	 = y;
	s_graphicsoptions.lighting.itemnames     = lighting_names;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_lodBias" & "subdivisions"
	s_graphicsoptions.geometry.generic.type  = MTYPE_SPINCONTROL;
	s_graphicsoptions.geometry.generic.name	 = "Geometric Detail:";
	s_graphicsoptions.geometry.generic.flags = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.geometry.generic.x	 = X_OFMAINPART;
	s_graphicsoptions.geometry.generic.y	 = y;
	s_graphicsoptions.geometry.itemnames     = quality_names;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_picmip"
	s_graphicsoptions.tq.generic.type	= MTYPE_SLIDER;
	s_graphicsoptions.tq.generic.name	= "Texture Detail:";
	s_graphicsoptions.tq.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.tq.generic.x		= X_OFMAINPART;
	s_graphicsoptions.tq.generic.y		= y;
	s_graphicsoptions.tq.minvalue       = 0;
	s_graphicsoptions.tq.maxvalue       = 3;
	s_graphicsoptions.tq.generic.callback = GraphicsOptions_TQEvent;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_textureBits"
	s_graphicsoptions.texturebits.generic.type  = MTYPE_SPINCONTROL;
	s_graphicsoptions.texturebits.generic.name	= "Texture Quality:";
	s_graphicsoptions.texturebits.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.texturebits.generic.x	    = X_OFMAINPART;
	s_graphicsoptions.texturebits.generic.y	    = y;
	s_graphicsoptions.texturebits.itemnames     = tq_names;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_textureMode"
	s_graphicsoptions.filter.generic.type   = MTYPE_SPINCONTROL;
	s_graphicsoptions.filter.generic.name	= "Texture Filter:";
	s_graphicsoptions.filter.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.filter.generic.x	    = X_OFMAINPART;
	s_graphicsoptions.filter.generic.y	    = y;
	s_graphicsoptions.filter.itemnames      = filter_names;
	y += BIGCHAR_HEIGHT+2;

	// references/modifies "r_flares"
	s_graphicsoptions.flares.generic.type     = MTYPE_SPINCONTROL;
	s_graphicsoptions.flares.generic.name	  = "Flares:";
	s_graphicsoptions.flares.generic.flags	  = QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.flares.generic.x	      = X_OFMAINPART;
	s_graphicsoptions.flares.generic.y	      = y;
	s_graphicsoptions.flares.itemnames	      = enabled_names;
	y += BIGCHAR_HEIGHT+2;

	s_graphicsoptions.driverinfo.generic.type     = MTYPE_TEXTS;
	s_graphicsoptions.driverinfo.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_graphicsoptions.driverinfo.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.driverinfo.generic.id       = ID_DRIVERINFO;
	s_graphicsoptions.driverinfo.generic.x        = X_OFMAINPART;
	s_graphicsoptions.driverinfo.generic.y        = y;
	s_graphicsoptions.driverinfo.string           = "Driver Info";
	s_graphicsoptions.driverinfo.style            = UI_CENTER|UI_SMALLFONT;
	s_graphicsoptions.driverinfo.color            = color_red;

	s_graphicsoptions.back.generic.type	    = MTYPE_BITMAP;
	s_graphicsoptions.back.generic.name     = BACK0;
	s_graphicsoptions.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_graphicsoptions.back.generic.callback = GraphicsOptions_Event;
	s_graphicsoptions.back.generic.id	    = ID_BACK2;
	s_graphicsoptions.back.generic.x		= 9;
	s_graphicsoptions.back.generic.y		= 440;
	s_graphicsoptions.back.width  		    = 80;
	s_graphicsoptions.back.height  		    = 40;
	s_graphicsoptions.back.focuspic         = BACK1;
	s_graphicsoptions.back.focuspicinstead	= qtrue;

	s_graphicsoptions.apply.generic.type     = MTYPE_BITMAP;
	s_graphicsoptions.apply.generic.name     = ACCEPT0;
	s_graphicsoptions.apply.generic.flags    = QMF_PULSEIFFOCUS|QMF_HIDDEN|QMF_INACTIVE;
	s_graphicsoptions.apply.generic.callback = GraphicsOptions_ApplyChanges;
	s_graphicsoptions.apply.generic.x        = 516;
	s_graphicsoptions.apply.generic.y        = 405;
	s_graphicsoptions.apply.width  		     = 102;
	s_graphicsoptions.apply.height  		 = 61;
	s_graphicsoptions.apply.focuspic         = ACCEPT1;

	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.graphics );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.display );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.sound );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.network );

	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.list );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.driver );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.allow_extensions );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.mode );

	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.customReso );

	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.colordepth );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.fs );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.lighting );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.geometry );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.tq );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.texturebits );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.filter );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.flares );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.driverinfo );

	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.back );
	Menu_AddItem( &s_graphicsoptions.menu, ( void * ) &s_graphicsoptions.apply );

	Com_sprintf(s_graphicsoptions.customReso_str,MAX_CUSTOMRESOSTR,"%dx%d",UI_GetCvarInt("r_customwidth"),UI_GetCvarInt("r_customheight"));

	GraphicsOptions_SetMenuItems();
	GraphicsOptions_GetInitialVideo();

	if ( uis.glconfig.driverType == GLDRV_ICD &&
		 uis.glconfig.hardwareType == GLHW_3DFX_2D3D )
	{
		s_graphicsoptions.driver.generic.flags |= QMF_HIDDEN|QMF_INACTIVE;
	}
}


/*
=================
GraphicsOptions_Cache
=================
*/
void GraphicsOptions_Cache( void ) {
	trap_R_RegisterShaderNoMip(BACK0);
	trap_R_RegisterShaderNoMip(BACK1);
	trap_R_RegisterShaderNoMip(GRAPHICS0);
	trap_R_RegisterShaderNoMip(GRASHICS1);
	trap_R_RegisterShaderNoMip(DISPLAY0);
	trap_R_RegisterShaderNoMip(DISPLAY1);
	trap_R_RegisterShaderNoMip(SOUND0);
	trap_R_RegisterShaderNoMip(SOUND1);
	trap_R_RegisterShaderNoMip(NETWORK0);
	trap_R_RegisterShaderNoMip(NETWORK1);
	trap_R_RegisterShaderNoMip(DRIVERINFO0);
	trap_R_RegisterShaderNoMip(DRIVERINFO1);	
	trap_R_RegisterShaderNoMip(ACCEPT0);
	trap_R_RegisterShaderNoMip(ACCEPT1);	
}


/*
=================
UI_GraphicsOptionsMenu
=================
*/
void UI_GraphicsOptionsMenu( void ) {
	GraphicsOptions_MenuInit();
	UI_PushMenu( &s_graphicsoptions.menu );
	Menu_SetCursorToItem( &s_graphicsoptions.menu, &s_graphicsoptions.graphics );
}

/*
#######################

  Custom-Resolution Menu

#######################
*/
typedef struct {
	menuframework_s	menu;

	menulist_s		templates;
	menufield_s		width;
	menufield_s		height;
	menutext_s		set; //save and exit (activate "applay"!)
	menutext_s		abort;

} customResoMenu_t;

static customResoMenu_t s_customReso;

static const char *resoTemplates_strs[] = 
{
	" 856 x  480 (VQ3 widescreen)",
	"1280 x  960 (4:3 with 1280)",
	"1280 x  800 (8:5 widescreen)",
	"1440 x  900 (8:5 widescreen)",
	"1680 x 1050 (8:5 widescreen)",
	"1920 x 1200 (8:5 widescreen)", 
	0
};

typedef struct { const char *width, *height; } intints_t;

static const intints_t resoTemplates_vals[] =
{
	{"856","480"},
	{"1280","960"},
	{"1280","800"},
	{"1440","900"},
	{"1680","1050"},
	{"1920","1200"} 
};

static int numResoTemplates = sizeof(resoTemplates_vals) / sizeof(resoTemplates_vals[0]);

#define CUSTOMRESOMENU_X 100
#define CUSTOMRESOMENU_Y 200
#define CUSTOMRESOMENU_W 320
#define CUSTOMRESOMENU_H 130

static void CustomReso_MenuDraw (void)
{
	const vec4_t colorTBlack = {0,0,0,0.9f};
	UI_FillRect(CUSTOMRESOMENU_X,CUSTOMRESOMENU_Y,CUSTOMRESOMENU_W,CUSTOMRESOMENU_H,colorTBlack);
	UI_DrawRect(CUSTOMRESOMENU_X,CUSTOMRESOMENU_Y,CUSTOMRESOMENU_W,CUSTOMRESOMENU_H,colorBlack);
	UI_DrawStringNS(CUSTOMRESOMENU_X+5,CUSTOMRESOMENU_Y+5,"enter the wanted resultion",UI_LEFT,20,color_orange);
	Menu_Draw( &s_customReso.menu );
}

static void CustomReso_Template( void *unused, int event )
{
	int i = s_customReso.templates.curvalue;
	if( event != QM_ACTIVATED ) return;

	if(i < 0 || i >= numResoTemplates)
		return;

	strcpy(s_customReso.width.field.buffer,resoTemplates_vals[i].width);
	strcpy(s_customReso.height.field.buffer,resoTemplates_vals[i].height);
}

#define ID_CRABORT	10
#define ID_CRSET	11

static void CustomReso_Buttons( void *ptr, int event )
{
	if( event != QM_ACTIVATED ) return;

	switch(((menucommon_s*)ptr)->id)
	{
	case ID_CRSET:
		trap_Cvar_Set("r_customwidth",s_customReso.width.field.buffer);
		trap_Cvar_Set("r_customheight",s_customReso.height.field.buffer);

		Com_sprintf(s_graphicsoptions.customReso_str,MAX_CUSTOMRESOSTR,"%dx%d",UI_GetCvarInt("r_customwidth"),UI_GetCvarInt("r_customheight"));
		s_graphicsoptions.customResoChanged = qtrue;

	case ID_CRABORT:
		UI_PopMenu();
		break;
	}
}

void LaunchCustomResoMenu(void)
{
	int y;
	int x;
	memset(&s_customReso,0,sizeof(s_customReso));

	s_customReso.menu.wrapAround = qtrue;
	s_customReso.menu.fullscreen = qtrue;
	s_customReso.menu.bgparts	 = BGP_LASTMENU;
	s_customReso.menu.draw	 = CustomReso_MenuDraw;

	y = CUSTOMRESOMENU_Y+5;
	// HEADER...!?!
	y += 32;
	s_customReso.templates.generic.type		= MTYPE_SPINCONTROL;
	s_customReso.templates.generic.name		= "Template:";
	s_customReso.templates.generic.flags	= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customReso.templates.generic.x		= CUSTOMRESOMENU_X+5+80;
	s_customReso.templates.generic.y		= y;
	s_customReso.templates.generic.callback	= CustomReso_Template;
	s_customReso.templates.itemnames		= resoTemplates_strs;

	y += 20;
	s_customReso.width.generic.type			= MTYPE_FIELD;
	s_customReso.width.generic.name			= "width:";
	s_customReso.width.generic.flags		= QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customReso.width.generic.x			= CUSTOMRESOMENU_X+5+8*8;
	s_customReso.width.generic.y			= y;
	s_customReso.width.field.widthInChars	= 5;
	s_customReso.width.field.maxchars		= 5;

	y += 20;
	s_customReso.height.generic.type		= MTYPE_FIELD;
	s_customReso.height.generic.name		= "height:";
	s_customReso.height.generic.flags		= QMF_NUMBERSONLY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customReso.height.generic.x			= CUSTOMRESOMENU_X+5+8*8;
	s_customReso.height.generic.y			= y;
	s_customReso.height.field.widthInChars	= 5;
	s_customReso.height.field.maxchars		= 5;

	y += 30;
	x = CUSTOMRESOMENU_X+CUSTOMRESOMENU_W-10;
	s_customReso.abort.generic.type		= MTYPE_TEXTS;
	s_customReso.abort.generic.flags	= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customReso.abort.generic.callback	= CustomReso_Buttons;
	s_customReso.abort.generic.id		= ID_CRABORT;
	s_customReso.abort.generic.x		= x;
	s_customReso.abort.generic.y		= y;
	s_customReso.abort.string			= "ABORT";
	s_customReso.abort.style			= UI_RIGHT|UI_SMALLFONT;
	s_customReso.abort.color			= color_orange;

	x -= strlen(s_customReso.abort.string)*8+10;
	s_customReso.set.generic.type		= MTYPE_TEXTS;
	s_customReso.set.generic.flags		= QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_customReso.set.generic.callback	= CustomReso_Buttons;
	s_customReso.set.generic.id			= ID_CRSET;
	s_customReso.set.generic.x			= x;
	s_customReso.set.generic.y			= y;
	s_customReso.set.string				= "SET";
	s_customReso.set.style				= UI_RIGHT|UI_SMALLFONT;
	s_customReso.set.color				= color_orange;

	Menu_AddItem( &s_customReso.menu, ( void * ) &s_customReso.templates );
	Menu_AddItem( &s_customReso.menu, ( void * ) &s_customReso.width );
	Menu_AddItem( &s_customReso.menu, ( void * ) &s_customReso.height );
	Menu_AddItem( &s_customReso.menu, ( void * ) &s_customReso.set );
	Menu_AddItem( &s_customReso.menu, ( void * ) &s_customReso.abort );

//	trap_Cvar_VariableStringBuffer("r_customwidth",s_customReso.width.field.buffer,5);
	Q_strncpyz( s_customReso.width.field.buffer, UI_Cvar_VariableString( "r_customwidth" ), sizeof( s_customReso.width.field.buffer ) );
//	trap_Cvar_VariableStringBuffer("r_customheight",s_customReso.height.field.buffer,5);
	Q_strncpyz( s_customReso.height.field.buffer, UI_Cvar_VariableString( "r_customheight" ), sizeof( s_customReso.height.field.buffer ) );

	UI_PushMenu( &s_customReso.menu );
}

