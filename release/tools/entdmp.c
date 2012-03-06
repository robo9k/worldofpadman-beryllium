//gpl etc

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define	LUMP_ENTITIES		0
#define	LUMP_PLANES			1
#define	LUMP_VERTEXES		2
#define	LUMP_VISIBILITY		3
#define	LUMP_NODES			4
#define	LUMP_TEXINFO		5
#define	LUMP_FACES			6
#define	LUMP_LIGHTING		7
#define	LUMP_LEAFS			8
#define	LUMP_LEAFFACES		9
#define	LUMP_LEAFBRUSHES	10
#define	LUMP_EDGES			11
#define	LUMP_SURFEDGES		12
#define	LUMP_MODELS			13
#define	LUMP_BRUSHES		14
#define	LUMP_BRUSHSIDES		15
#define	LUMP_POP			16
#define	LUMP_AREAS			17
#define	LUMP_AREAPORTALS	18
#define	HEADER_LUMPS		19

#define IDBSPHEADER	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
		// little-endian "IBSP"

#define BSPVERSION	46


// upper design bounds
// leaffaces, leafbrushes, planes, and verts are still bounded by
// 16 bit short limits
#define	MAX_MAP_MODELS		1024
#define	MAX_MAP_BRUSHES		8192
#define	MAX_MAP_ENTITIES	2048
#define	MAX_MAP_ENTSTRING	0x40000
#define	MAX_MAP_TEXINFO		8192

#define	MAX_MAP_AREAS		256
#define	MAX_MAP_AREAPORTALS	1024
#define	MAX_MAP_PLANES		65536
#define	MAX_MAP_NODES		65536
#define	MAX_MAP_BRUSHSIDES	65536
#define	MAX_MAP_LEAFS		65536
#define	MAX_MAP_VERTS		65536
#define	MAX_MAP_FACES		65536
#define	MAX_MAP_LEAFFACES	65536
#define	MAX_MAP_LEAFBRUSHES 65536
#define	MAX_MAP_PORTALS		65536
#define	MAX_MAP_EDGES		128000
#define	MAX_MAP_SURFEDGES	256000
#define	MAX_MAP_LIGHTING	0x200000
#define	MAX_MAP_VISIBILITY	0x100000

typedef struct
{
	int		fileofs, filelen;
} lump_t;

typedef struct
{
	int			ident;
	int			version;	
	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct texinfo_s
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			flags;			// miptex flags + overrides
	int			value;			// light emission, etc
	char		texture[32];	// texture name (textures/*.wal)
	int			nexttexinfo;	// for animations, -1 = end of chain
} texinfo_t;

typedef struct csurface_s
{
	char		name[16];
	int			flags;
	int			value;
} csurface_t;

typedef struct mapsurface_s  // used internally due to name len probs //ZOID
{
	csurface_t	c;
	char		rname[32];
} mapsurface_t;

typedef unsigned char byte;

byte	*cmod_base;

#define ERR_DROP 1


void Com_Error (int code, char *fmt, ...)
{
	va_list		argptr;
	static char		msg[1024];

	va_start (argptr,fmt);
	vsprintf (msg,fmt,argptr);
	va_end (argptr);

	fprintf (stderr, "FATAL ERROR: %s\n", msg);
	exit (1);
}

int			numtexinfo;
mapsurface_t	map_surfaces[MAX_MAP_TEXINFO];


int		LittleLong (int l) {return (l);}

char map_entitystring[MAX_MAP_ENTSTRING];

void CMod_LoadEntityString (lump_t *l)
{
	int numentitychars = l->filelen;
	if (l->filelen > MAX_MAP_ENTSTRING)
		Com_Error (ERR_DROP, "Map has too large entity lump (%d > %d)", l->filelen, MAX_MAP_ENTSTRING);

	memcpy (map_entitystring, cmod_base + l->fileofs, l->filelen);
	printf ("%s\n", map_entitystring);
}

int main(int argc, char* argv[])
{
	byte		*buf;
	int				i, len;
	dheader_t		header;
	FILE		*in;

	in = fopen(argv[1], "rb");
	if (!in) {
		fprintf (stderr, "FATAL ERROR: fopen() failed.\n");
		exit (1);
	}

	fseek (in, 0, SEEK_END);
	len = ftell (in);
	fseek (in, 0, SEEK_SET);

	buf = malloc(len);
	fread (buf, len, 1, in);

	header = *(dheader_t *)buf;
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++)
		((int *)&header)[i] = LittleLong ( ((int *)&header)[i]);

	if (header.version != BSPVERSION)
		Com_Error (ERR_DROP, "This is not a valid BSP file.");

	cmod_base = (byte *)buf;

	// load into heap
	// CMod_LoadSurfaces (&header.lumps[LUMP_TEXINFO]);
	CMod_LoadEntityString (&header.lumps[LUMP_ENTITIES]);

	free (buf);
	fclose (in);
}

