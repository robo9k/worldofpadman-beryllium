// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

DEMOS MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK0			"menu/BtnBack0"
#define ART_BACK1			"menu/BtnBack1"	
#define ART_GO0				"menu/demo/play0"
#define ART_GO1				"menu/demo/play1"
#define ART_ARROWUP0		"menu/demo/arrowup0.tga"
#define ART_ARROWUP1		"menu/demo/arrowup1.tga"
#define ART_ARROWDOWN0		"menu/demo/arrowdown0.tga"
#define ART_ARROWDOWN1		"menu/demo/arrowdown1.tga"

#define MAX_DEMOS			128
#define NAMEBUFSIZE			( MAX_DEMOS * 16 )

#define ID_BACK				10
#define ID_GO				11
#define ID_LIST				12
#define ID_RIGHT			13
#define ID_LEFT				14

#define ARROWS_WIDTH		128
#define ARROWS_HEIGHT		48


typedef struct {
	menuframework_s	menu;

	menulist_s		list;

	menubitmap1024s_s	left;
	menubitmap1024s_s	right;
	menubitmap_s		back;
	menubitmap1024s_s	go;

	int				numDemos;
	char			names[NAMEBUFSIZE];
	char			*demolist[MAX_DEMOS];
} demos_t;

static demos_t	s_demos;


/*
===============
Demos_MenuEvent
===============
*/
static void Demos_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_GO:
		UI_ForceMenuOff ();
		trap_Cmd_ExecuteText( EXEC_APPEND, va( "demo %s\n",
								s_demos.list.itemnames[s_demos.list.curvalue]) );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;

	case ID_LEFT:
//		ScrollList_Key( &s_demos.list, K_LEFTARROW );	// only !=1 columns
		ScrollList_Key( &s_demos.list, K_PGUP );		// only <=1 columns
		break;

	case ID_RIGHT:
//		ScrollList_Key( &s_demos.list, K_RIGHTARROW );	// only !=1 columns
		ScrollList_Key( &s_demos.list, K_PGDN );		// only <=1 columns
		break;
	}
}


/*
=================
UI_DemosMenu_Key
=================
*/
static sfxHandle_t UI_DemosMenu_Key( int key ) {
	menucommon_s	*item;

	item = Menu_ItemAtCursor( &s_demos.menu );

	return Menu_DefaultKey( &s_demos.menu, key );
}


/*
===============
Demos_MenuInit
===============
*/
static void Demos_MenuInit( void ) {
	int		i;
	int		len;
	char	*demoname, extension[32];

	memset( &s_demos, 0 ,sizeof(demos_t) );
	s_demos.menu.key = UI_DemosMenu_Key;

	Demos_Cache();

	s_demos.menu.fullscreen = qtrue;
	s_demos.menu.wrapAround = qtrue;
	s_demos.menu.bgparts	= BGP_DEMOBG|BGP_SIMPLEBG;

	s_demos.left.generic.type		= MTYPE_BITMAP1024S;
	s_demos.left.x					= 96;//367;
	s_demos.left.y					= 204;//524;
	s_demos.left.w					= 38;//99;
	s_demos.left.h					= 98;//38;
	s_demos.left.shader				= trap_R_RegisterShaderNoMip(ART_ARROWUP0);
	s_demos.left.mouseovershader	= trap_R_RegisterShaderNoMip(ART_ARROWUP1);
	s_demos.left.generic.callback	= Demos_MenuEvent;
	s_demos.left.generic.id			= ID_LEFT;

	s_demos.right.generic.type		= MTYPE_BITMAP1024S;
	s_demos.right.x					= 96;//561;
	s_demos.right.y					= 458;//524;
	s_demos.right.w					= 38;//98;
	s_demos.right.h					= 98;//38;
	s_demos.right.shader			= trap_R_RegisterShaderNoMip(ART_ARROWDOWN0);
	s_demos.right.mouseovershader	= trap_R_RegisterShaderNoMip(ART_ARROWDOWN1);
	s_demos.right.generic.callback	= Demos_MenuEvent;
	s_demos.right.generic.id		= ID_RIGHT;

	s_demos.back.generic.type		= MTYPE_BITMAP;
	s_demos.back.generic.name		= ART_BACK0;
	s_demos.back.generic.flags		= QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS;
    s_demos.back.generic.x			= 8;
	s_demos.back.generic.y			= 440;
	s_demos.back.generic.id			= ID_BACK;
	s_demos.back.generic.callback	= Demos_MenuEvent;
    s_demos.back.width				= 80;
	s_demos.back.height				= 40;
	s_demos.back.focuspic			= ART_BACK1;
	s_demos.back.focuspicinstead	= qtrue;

	s_demos.go.generic.type		= MTYPE_BITMAP1024S;
	s_demos.go.x				= 84;//815;
	s_demos.go.y				= 350;//633;
	s_demos.go.w				= 63;//181;
	s_demos.go.h				= 63;//110;
	s_demos.go.shader			= trap_R_RegisterShaderNoMip(ART_GO0);
	s_demos.go.mouseovershader	= trap_R_RegisterShaderNoMip(ART_GO1);
	s_demos.go.generic.callback	= Demos_MenuEvent;
	s_demos.go.generic.id		= ID_GO;

	s_demos.list.generic.type		= MTYPE_SCROLLLIST;
	s_demos.list.generic.flags		= QMF_PULSEIFFOCUS;
	s_demos.list.generic.callback	= Demos_MenuEvent;
	s_demos.list.generic.id			= ID_LIST;
	s_demos.list.generic.x			= 100;
	s_demos.list.generic.y			= 60;
	s_demos.list.width				= 28;
	s_demos.list.height				= 20;
	s_demos.list.columns			= 1;

	Com_sprintf(extension, sizeof(extension), "dm_%d", (int)trap_Cvar_VariableValue( "protocol" ) );
	s_demos.list.numitems			= trap_FS_GetFileList( "demos", extension, s_demos.names, NAMEBUFSIZE );
	s_demos.list.itemnames			= (const char **)s_demos.demolist;

	if (!s_demos.list.numitems) {
		strcpy( s_demos.names, "No Demos Found." );
		s_demos.list.numitems = 1;

		//degenerate case, not selectable
		s_demos.go.generic.flags |= (QMF_INACTIVE|QMF_HIDDEN);
	}
	else if (s_demos.list.numitems > MAX_DEMOS)
		s_demos.list.numitems = MAX_DEMOS;

	demoname = s_demos.names;
	for ( i = 0; i < s_demos.list.numitems; i++ ) {
		s_demos.list.itemnames[i] = demoname;
		
		// strip extension
		len = strlen( demoname );
		if (!Q_stricmp(demoname +  len - 4,".dm3"))
			demoname[len-4] = '\0';

		Q_strupr(demoname);

		demoname += len + 1;
	}

	Menu_AddItem( &s_demos.menu, &s_demos.list );
	Menu_AddItem( &s_demos.menu, &s_demos.left );
	Menu_AddItem( &s_demos.menu, &s_demos.right );
	Menu_AddItem( &s_demos.menu, &s_demos.back );
	Menu_AddItem( &s_demos.menu, &s_demos.go );
}

/*
=================
Demos_Cache
=================
*/
void Demos_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK1 );
	trap_R_RegisterShaderNoMip( ART_GO0 );
	trap_R_RegisterShaderNoMip( ART_GO1 );
	trap_R_RegisterShaderNoMip( ART_ARROWUP0 );
	trap_R_RegisterShaderNoMip( ART_ARROWUP1 );
	trap_R_RegisterShaderNoMip( ART_ARROWDOWN0 );
	trap_R_RegisterShaderNoMip( ART_ARROWDOWN1 );
}

/*
===============
UI_DemosMenu
===============
*/
void UI_DemosMenu( void ) {
	Demos_MenuInit();
	UI_PushMenu( &s_demos.menu );
}
