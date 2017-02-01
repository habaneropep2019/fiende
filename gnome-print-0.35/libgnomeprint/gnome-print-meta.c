/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Metafile printer driver.
 *
 * Authors:
 *    Miguel de Icaza (miguel@gnu.org)
 *    Michael Zucchi <notzed@helixcode.com>
 *    Morten Welinder (terra@diku.dk)
 *
 * TODO:
 *    If the GnomeFont is not found during the replay of a metafile
 *    we should load any font at the specified size.
 *
 *    We could keep a cache of all the fonts referenced during a
 *    playback (instead of using gnome_font_new always) and unref
 *    all the fonts when the render is done.
 *
 *    Handle GNOME_META_TEXTLINE
 *
 *    Because each page is played back in isolation, if there are any
 *    options that persist across showpage's, then they need to be
 *    tracked, and stored in the page header for each page.  But I
 *    dont think there are...
 */
#include <config.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>

#include <libgnomeprint/gnome-glyphlist-private.h>
#include <libgnomeprint/gnome-print-private.h>
#include <libgnomeprint/gnome-print-meta.h>

#undef META_DEBUG

typedef struct _GnomePrintMetaPrivate GnomePrintMetaPrivate;

struct _GnomePrintMeta {
	GnomePrintContext pc;

	char *buffer;
	int   buffer_size;
	int   current;
	int   pages;

	GnomePrintMetaPrivate *priv;
};

struct _GnomePrintMetaClass {
	GnomePrintContextClass parent_class;
};

static GnomePrintContextClass *parent_class = NULL;

/* Note: this must continue to be the same length  as "GNOME_METAFILE-0.0" */
#define GNOME_METAFILE_SIGNATURE "GNOME_METAFILE-1.1"
#define GNOME_METAFILE_SIGNATURE_SIZE 18
#define BLOCKSIZE 4096

typedef struct {
	char signature [GNOME_METAFILE_SIGNATURE_SIZE];
	gint32  size;
} GnomeMetaFileHeader;

/* this is used to quickly scan pages through a metafile, it is
   stored at the start of the file, and after every showpage */
#define PAGE_SIGNATURE "PAGE"
typedef struct {
	char signature[4];	/* magic token for a page header */
	gint32 size;
} GnomeMetaPageHeader;

/* ENCODED sizes maybe != structure sizes (alignment) */
#define FILEHEADER_SIZE (GNOME_METAFILE_SIGNATURE_SIZE+4)
#define PAGEHEADER_SIZE (4+4)

struct _GnomePrintMetaPrivate {
	int last_page;		/* offset into the buffer for the last page header */
};

static const char *decode_header (const char *data, GnomeMetaFileHeader *mh);

typedef enum {
	GNOME_META_NEWPATH,
	GNOME_META_MOVETO,
	GNOME_META_LINETO,
	GNOME_META_CURVETO,
	GNOME_META_CLOSEPATH,
	GNOME_META_SETRGBCOLOR,
	GNOME_META_FILL,
	GNOME_META_SETLINEWIDTH,
	GNOME_META_SETMITERLIMIT,
	GNOME_META_SETLINEJOIN,
	GNOME_META_SETLINECAP,
	GNOME_META_SETDASH,
	GNOME_META_STROKEPATH,
	GNOME_META_STROKE,
	GNOME_META_SETFONT,
	GNOME_META_SHOW_SIZED,
	GNOME_META_CONCAT,
	GNOME_META_GSAVE,
	GNOME_META_GRESTORE,
	GNOME_META_CLIP,
	GNOME_META_GRAYIMAGE,
	GNOME_META_RGBIMAGE,
	GNOME_META_TEXTLINE,
	GNOME_META_BEGINPAGE,
	GNOME_META_SHOWPAGE,
	GNOME_META_CLOSE,
	GNOME_META_SETOPACITY,
	GNOME_META_RGBAIMAGE,
	GNOME_META_GLYPHLIST
} GnomeMetaType;

#ifdef META_DEBUG
static char *meta_type_names[] = {
	"NEWPATH",
	"MOVETO",
	"LINETO",
	"CURVETO",
	"CLOSEPATH",
	"SETRGBCOLOR",
	"FILL",
	"SETLINEWIDTH",
	"SETMITERLIMIT",
	"SETLINEJOIN",
	"SETLINECAP",
	"SETDASH",
	"STROKEPATH",
	"STROKE",
	"SETFONT",
	"SHOW_SIZED",
	"CONCAT",
	"GSAVE",
	"GRESTORE",
	"CLIP",
	"GRAYIMAGE",
	"RGBIMAGE",
	"TEXTLINE",
	"BEGINPAGE",
	"SHOWPAGE",
	"CLOSE",
	"SETOPACITY",
	"RGBAIMAGE",
	"GLYPHLIST"
};
#endif

typedef enum {
	GNOME_META_DOUBLE_INT,        /* An integer.  */
	GNOME_META_DOUBLE_INT1000,    /* An integer to be divided by 1000.  */
	GNOME_META_DOUBLE_I386        /* IEEE-xxxx little endian.  */
} GnomeMetaDoubleType;

static void
gnome_print_meta_finalize (GtkObject *object)
{
	GnomePrintMeta *meta;

	meta = GNOME_PRINT_META (object);

	g_free (meta->buffer);
	g_free (meta->priv);

	(*GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}

/* returns true if there is room (grows it if there is not) */
static gboolean
check_room (GnomePrintMeta *meta, int size)
{
	int bytes_left;

	bytes_left = meta->buffer_size - meta->current;

	if (bytes_left < size){
		int min_size = MAX (BLOCKSIZE, size-bytes_left);
		void *newp;

		newp = g_realloc (meta->buffer, meta->buffer_size + min_size);
		if (!newp)
			return FALSE;

		meta->buffer = newp;
		meta->buffer_size = meta->buffer_size + min_size;
	}
	return TRUE;
}

static void
encode_block (GnomePrintMeta *meta, int size, void const *data)
{
	if (!check_room (meta, size))
		return;

	memcpy (meta->buffer + meta->current, data, size);
	meta->current += size;
}

static void
encode_int_header (GnomePrintMeta *pc, gint32 value)
{
	gint32 new = g_htonl (value);

	encode_block (GNOME_PRINT_META (pc), sizeof (gint32), &new);
}

static void
encode_int (GnomePrintContext *pc, gint32 value)
{
	char *out, *out0;
	guint32 vabs, mask;
	int bits;

	GnomePrintMeta *meta = GNOME_PRINT_META (pc);
	if (!check_room (meta, sizeof (value) * 8 / 7 + 1))
		return;

	/*
	 * We encode an integer as a sequence of bytes where all but the
	 * last have the high bit set to zero.  The final byte does have
	 * that bit set.
	 *
	 * Bit 6 of the first byte contains the sign bit.
	 *
	 * The remaining 6, 7, ..., 7 bits contain the absolute value,
	 * starting with the six least significant bits in the first
	 * byte.
	 */
	out0 = out = meta->buffer + meta->current;
	vabs = (value >= 0) ? value : -value;
	bits = 6;
	mask = 0x3f;

	do {
		*out++ = (vabs & mask);
		vabs >>= bits;
		bits = 7;
		mask = 0x7f;
	} while (vabs);

	out[-1] |= 0x80;
	if (value < 0) out0[0] |= 0x40;
	meta->current = out - meta->buffer;
}

static void
encode_double (GnomePrintContext *pc, double d)
{
	int sd = sizeof (d);

	if (d == (gint32)d) {
		encode_int (pc, GNOME_META_DOUBLE_INT);
		encode_int (pc, (int)d);
	} else {
		double d1000 = d * 1000;
		if (d1000 == (gint32)d1000) {
			encode_int (pc, GNOME_META_DOUBLE_INT1000);
			encode_int (pc, (int)d1000);
		} else {
			encode_int (pc, GNOME_META_DOUBLE_I386);
#if G_BYTE_ORDER == G_BIG_ENDIAN
			{
				int     i;
				guint8 *t  = (guint8 *)&d;
				guint8 block[sizeof (d)];

				for (i = 0; i < sd; i++)
					block[sd - 1 - i] = t[i];
				encode_block (GNOME_PRINT_META (pc), sd, &block);
			}
#elif G_BYTE_ORDER == G_LITTLE_ENDIAN
			encode_block (GNOME_PRINT_META (pc), sd, &d);
#else
#error encode_double_needs_attention
#endif
		}
	}
}

static void
encode_six_doubles (GnomePrintContext *pc, double x1, double y1, double x2, double y2, double x3, double y3)
{
	encode_double (pc, x1);
	encode_double (pc, y1);
	encode_double (pc, x2);
	encode_double (pc, y2);
	encode_double (pc, x3);
	encode_double (pc, y3);
}

static void
encode_string_sized (GnomePrintContext *pc, const char *str, int bytes)
{
	encode_int (pc, bytes);
	encode_block (GNOME_PRINT_META (pc), bytes, str);
}

static void
encode_string (GnomePrintContext *pc, const char *str)
{
	encode_string_sized (pc, str, strlen (str));
}

static void
encode_header (GnomePrintMeta *pc, GnomeMetaFileHeader *mh)
{
	encode_block (pc, sizeof (mh->signature), mh->signature);
	encode_int_header (pc, mh->size);
}

/* sets the size of the last page */
static void
close_page_header (GnomePrintMeta *pc)
{
	if (pc->priv->last_page != -1) {
		gint32 l;
		l = g_htonl (pc->current - pc->priv->last_page - PAGEHEADER_SIZE);
		/* we cannot map the page header back into memory, so poke it back in */
		/* Note: unaligned access -- MW.  */
		memcpy ((pc->buffer + pc->priv->last_page) + 4, &l, sizeof (l));
	}
}

static void
encode_page_header (GnomePrintMeta *pc, GnomeMetaPageHeader *mh)
{
	close_page_header (pc);
	pc->priv->last_page = pc->current;
	encode_block (pc, sizeof (mh->signature), mh->signature);
	encode_int_header (pc, mh->size);
#ifdef META_DEBUG
	g_warning ("Page header ends at 0x%x", pc->current);
#endif
}

static int
meta_newpath (GnomePrintContext *pc)
{
#ifdef META_DEBUG
	g_warning ("Newpath starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_NEWPATH);
	return 0;
}

static int
meta_moveto (GnomePrintContext *pc, double x, double y)
{
#ifdef META_DEBUG
	g_warning ("MoveTo starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_MOVETO);
	encode_double (pc, x);
	encode_double (pc, y);
	return 0;
}

static int
meta_lineto (GnomePrintContext *pc, double x, double y)
{
#ifdef META_DEBUG
	g_warning ("LineTo starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_LINETO);
	encode_double (pc, x);
	encode_double (pc, y);
	return 0;
}

static int
meta_curveto (GnomePrintContext *pc, double x1, double y1, double x2, double y2, double x3, double y3)
{
	encode_int (pc, GNOME_META_CURVETO);
	encode_six_doubles (pc, x1, y1, x2, y2, x3, y3);
	return 0;
}

static int
meta_closepath (GnomePrintContext *pc)
{
	encode_int (pc, GNOME_META_CLOSEPATH);
	return 0;
}

static int
meta_setrgbcolor (GnomePrintContext *pc, double r, double g, double b)
{
#ifdef META_DEBUG
	g_warning ("SetRGBColor starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_SETRGBCOLOR);
	encode_double (pc, r);
	encode_double (pc, g);
	encode_double (pc, b);
	return 0;
}

static int
meta_fill (GnomePrintContext *pc, ArtWindRule rule)
{
	encode_int (pc, GNOME_META_FILL);
	encode_int (pc, rule);
	return 0;
}

static int
meta_setlinewidth (GnomePrintContext *pc, double width)
{
#ifdef META_DEBUG
	g_warning ("SetLineWidth starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_SETLINEWIDTH);
	encode_double (pc, width);
	return 0;
}

static int
meta_setmiterlimit (GnomePrintContext *pc, double limit)
{
	encode_int (pc, GNOME_META_SETMITERLIMIT);
	encode_double (pc, limit);
	return 0;
}

static int
meta_setlinejoin (GnomePrintContext *pc, int jointype)
{
	encode_int (pc, GNOME_META_SETLINEJOIN);
	encode_int (pc, jointype);
	return 0;
}

static int
meta_setlinecap (GnomePrintContext *pc, int captype)
{
	encode_int (pc, GNOME_META_SETLINECAP);
	encode_int (pc, captype);
	return 0;
}

static int
meta_setdash (GnomePrintContext *pc, int n_values, const double *values, double offset)
{
	int i;

	encode_int (pc, GNOME_META_SETDASH);
	encode_int (pc, n_values);

	for (i = 0; i < n_values; i++)
		encode_double (pc, values [i]);
	encode_double (pc, offset);
	return 0;
}

static int
meta_strokepath (GnomePrintContext *pc)
{
	encode_int (pc, GNOME_META_STROKEPATH);
	return 0;
}

static int
meta_stroke (GnomePrintContext *pc)
{
	encode_int (pc, GNOME_META_STROKE);
	return 0;
}

static int
meta_setfont (GnomePrintContext *pc, GnomeFont *font)
{
	const char *fontname;

	fontname = gnome_font_get_name (font);

#ifdef META_DEBUG
	g_warning ("SetFont starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_SETFONT);
	encode_double (pc, gnome_font_get_size (font));
	encode_string (pc, fontname);
	return 0;
}


static int
meta_show_sized (GnomePrintContext *pc, const char *text, int bytes)
{
#ifdef META_DEBUG
	g_warning ("Show_sized starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_SHOW_SIZED);
	encode_string_sized (pc, text, bytes);

	return 0;
}

static int
meta_concat (GnomePrintContext *pc, const double matrix [6])
{
	encode_int (pc, GNOME_META_CONCAT);
	encode_six_doubles (pc, matrix [0], matrix [1], matrix [2], matrix [3], matrix [4], matrix [5]);
	return 0;
}

static int
meta_gsave (GnomePrintContext *pc)
{
	encode_int (pc, GNOME_META_GSAVE);
	return 0;
}

static int
meta_grestore (GnomePrintContext *pc)
{
	encode_int (pc, GNOME_META_GRESTORE);
	return 0;
}

static int
meta_clip (GnomePrintContext *pc, ArtWindRule rule)
{
	encode_int (pc, GNOME_META_CLIP);
	encode_int (pc, rule);
	return 0;
}

static void
encode_image (GnomePrintContext *pc, const char *data, int width, int height, int rowstride, int bytes_per_pixel)
{
	int ix, y;

	encode_int (pc, height);
	encode_int (pc, width);

	ix = 0;
	for (y = 0; y < height; y++){
		encode_block (GNOME_PRINT_META (pc), width * bytes_per_pixel, &data [ix]);
		ix += rowstride;
	}
}

static int
meta_grayimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	encode_int (pc, GNOME_META_GRAYIMAGE);
	encode_image (pc, data, width, height, rowstride, 1);
	return 0;
}

static int
meta_rgbimage (GnomePrintContext *pc, const char *data, int width, int height, int rowstride)
{
	encode_int (pc, GNOME_META_RGBIMAGE);
	encode_image (pc, data, width, height, rowstride, 3);
	return 0;
}

static int
meta_textline (GnomePrintContext *pc, GnomeTextLine *line)
{
	encode_int (pc, GNOME_META_TEXTLINE);
	g_warning ("meta_textline: not implemented yet");
	return 0;
}

static int
meta_showpage (GnomePrintContext *pc)
{
	GnomeMetaPageHeader page_header = {
		PAGE_SIGNATURE,
		-1
	};
	GnomePrintMeta *meta = GNOME_PRINT_META (pc);

#ifdef META_DEBUG
	g_warning ("ShowPage starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_SHOWPAGE);

	/* assume another page is coming */
	encode_page_header (meta, &page_header);

	meta->pages++;
	return 0;
}

static int
meta_beginpage (GnomePrintContext *pc, const char *name_of_this_page)
{
#ifdef META_DEBUG
	g_warning ("BeginPage starts at 0x%x",
		   GNOME_PRINT_META (pc)->current);
#endif
	encode_int (pc, GNOME_META_BEGINPAGE);
	encode_string (pc, name_of_this_page);
	return 0;
}

static int
meta_close (GnomePrintContext *pc)
{
	gint32 l;
	GnomePrintMeta *meta = GNOME_PRINT_META (pc);

	l = g_htonl (meta->buffer_size);
	/* we cannot just map the header to memory, so poke it manually */
	/* Note: unaligned access -- MW.  */
	memcpy (meta->buffer + GNOME_METAFILE_SIGNATURE_SIZE, &l, sizeof (l));

	return 0;
}

static int
meta_setopacity (GnomePrintContext *pc, double opacity)
{
	encode_int (pc, GNOME_META_SETOPACITY);
	encode_double (pc, opacity);
	return 0;
}

static int
meta_rgbaimage (GnomePrintContext *pc, const char *data,
		int width, int height, int rowstride)
{
	encode_int (pc, GNOME_META_RGBAIMAGE);
	encode_image (pc, data, width, height, rowstride, 4);
	return 0;
}

static int
meta_glyphlist (GnomePrintContext *pc, GnomeGlyphList *gl)
{
	gint i;

	encode_int (pc, GNOME_META_GLYPHLIST);
	encode_int (pc, gl->g_length);
	for (i = 0; i < gl->g_length; i++) {
		encode_int (pc, gl->glyphs[i]);
	}
	encode_int (pc, gl->r_length);
	for (i = 0; i < gl->r_length; i++) {
		encode_int (pc, gl->rules[i].code);
		switch (gl->rules[i].code) {
		case GGL_POSITION:
		case GGL_ADVANCE:
		case GGL_COLOR:
			encode_int (pc, gl->rules[i].value.ival);
			break;
		case GGL_MOVETOX:
		case GGL_MOVETOY:
		case GGL_RMOVETOX:
		case GGL_RMOVETOY:
		case GGL_LETTERSPACE:
		case GGL_KERNING:
			encode_double (pc, gl->rules[i].value.dval);
			break;
		case GGL_FONT:
			encode_double (pc, gnome_font_get_size (gl->rules[i].value.font));
			encode_string (pc, gnome_font_get_name (gl->rules[i].value.font));
			break;
		case GGL_PUSHCP:
		case GGL_POPCP:
		default:
			break;
		}
	}

	return 0;
}

static void
gnome_print_meta_class_init (GnomePrintMetaClass *class)
{
	GtkObjectClass *object_class;
	GnomePrintContextClass *pc_class;

	object_class = (GtkObjectClass *)class;
	pc_class = (GnomePrintContextClass *)class;

	parent_class = gtk_type_class (gnome_print_context_get_type ());

	object_class->finalize = gnome_print_meta_finalize;

	/* initialization code, autogenned */
	pc_class->newpath = meta_newpath;
	pc_class->moveto = meta_moveto;
	pc_class->lineto = meta_lineto;
	pc_class->curveto = meta_curveto;
	pc_class->closepath = meta_closepath;
	pc_class->setrgbcolor = meta_setrgbcolor;
	pc_class->fill = meta_fill;
	pc_class->setlinewidth = meta_setlinewidth;
	pc_class->setmiterlimit = meta_setmiterlimit;
	pc_class->setlinejoin = meta_setlinejoin;
	pc_class->setlinecap = meta_setlinecap;
	pc_class->setdash = meta_setdash;
	pc_class->strokepath = meta_strokepath;
	pc_class->stroke = meta_stroke;
	pc_class->setfont = meta_setfont;
	pc_class->show_sized = meta_show_sized;
	pc_class->concat = meta_concat;
	pc_class->gsave = meta_gsave;
	pc_class->grestore = meta_grestore;
	pc_class->clip = meta_clip;
	pc_class->grayimage = meta_grayimage;
	pc_class->rgbimage = meta_rgbimage;
	pc_class->textline = meta_textline;
	pc_class->beginpage = meta_beginpage;
	pc_class->showpage = meta_showpage;
	pc_class->close = meta_close;
	pc_class->setopacity = meta_setopacity;
	pc_class->rgbaimage = meta_rgbaimage;
	pc_class->glyphlist = meta_glyphlist;
}

static void
gnome_print_meta_init (GnomePrintMeta *meta)
{
	GnomeMetaFileHeader header = {
		GNOME_METAFILE_SIGNATURE,
		-1
	};
	GnomeMetaPageHeader page_header = {
		PAGE_SIGNATURE,
		-1
	};

	meta->buffer_size = BLOCKSIZE;
	meta->buffer = g_malloc (meta->buffer_size);
	meta->priv = g_malloc0 (sizeof (GnomePrintMetaPrivate));
	meta->priv->last_page = -1;
	encode_header (meta, &header);
	encode_page_header (meta, &page_header);
}

/**
 * gnome_print_meta_get_type:
 *
 * Returns the GtkType for the GnomePrintMeta object
 */
GtkType
gnome_print_meta_get_type (void)
{
	static GtkType meta_type = 0;

	if (!meta_type){
		GtkTypeInfo meta_info =
		{
			"GnomePrintMeta",
			sizeof (GnomePrintMeta),
			sizeof (GnomePrintMetaClass),
			(GtkClassInitFunc) gnome_print_meta_class_init,
			(GtkObjectInitFunc) gnome_print_meta_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL,
		};

		meta_type = gtk_type_unique (gnome_print_context_get_type (), &meta_info);
	}

	return meta_type;
}

/**
 * gnome_print_meta_new:
 *
 * Creates a new Metafile context GnomePrint object.
 *
 * Returns: An empty %GnomePrint context that represents a
 * metafile priting context.
 */
GnomePrintMeta *
gnome_print_meta_new (void)
{
	GnomePrintMeta *meta;

	meta = gtk_type_new (gnome_print_meta_get_type ());

	return meta;
}

/**
 * gnome_print_meta_new_from:
 * @data: Pointer to a gnome_print_meta metadata.
 *
 * Creates a new Metafile context GnomePrint object.
 *
 * Initializes the contents from a buffer that contains a
 * GNOME_METAFILE stream.
 *
 * Returns: A new %GnomePrint context that represented the Metafile
 * printing context.
 */
GnomePrintMeta *
gnome_print_meta_new_from (const void *data)
{
	GnomePrintMeta *meta;
	GnomeMetaFileHeader header;
	int len;

	g_return_val_if_fail (data != NULL, NULL);

	decode_header (data, &header);
	if (strncmp (header.signature, GNOME_METAFILE_SIGNATURE, GNOME_METAFILE_SIGNATURE_SIZE) != 0)
		return NULL;

	len = header.size;
	meta = gtk_type_new (gnome_print_meta_get_type ());

	if (meta->buffer_size < len){
		g_free (meta->buffer);
		meta->buffer = g_malloc (len);

		if (!meta->buffer){
			gtk_object_destroy (GTK_OBJECT (meta));
			return NULL;
		}
	}
	memcpy (meta->buffer, data, len);
	meta->current = len;
	return meta;
}

/**
 * gnome_print_meta_access_buffer:
 * @meta:
 * @buffer: points to a void * that will be modified to
 *          point to the new buffer
 * @buflen: pointer to an integer that will be set to the lenght of buffer.
 *
 * Makes buffer point to the internal GnomePrintMeta information.
 *
 * Returns 0 on a failure, any other value on success
 */
int
gnome_print_meta_access_buffer (GnomePrintMeta *meta, void **buffer, int *buflen)
{
	char *p;
	gint32 l;

	g_return_val_if_fail (meta != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_META (meta), 0);

	l = g_htonl (meta->current);

	p = *buffer = meta->buffer;
	/* we cannot just map the header to memory, so poke it manually */
	/* Note: unaligned access -- MW.  */
	memcpy (p + GNOME_METAFILE_SIGNATURE_SIZE, &l, sizeof (l));
	*buflen = meta->current;

	return 1;
}

/**
 * gnome_print_meta_get_copy:
 * @meta:
 * @buffer: points to a void * that will be modified to
 *          point to the new buffer
 * @buflen: pointer to an integer that will be set to the lenght of buffer.
 *
 * Duplicates the internal buffer and stores a pointer to the block with
 * a copy of this in *@buffer.  The size of the buffer is stored in @buflen.
 *
 * Returns 0 on a failure, any other value on success
 */
int
gnome_print_meta_get_copy (GnomePrintMeta *meta, void **buffer, int *buflen)
{
	char *p;
	gint32 l;

	g_return_val_if_fail (meta != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_META (meta), 0);

	p = *buffer = g_malloc (meta->current);
	if (*buffer == NULL)
		return 0;

	l = g_htonl (meta->current);

	memcpy (p, meta->buffer, meta->buffer_size);
	/* we cannot just map the header to memory, so poke it manually */
	/* Note: unaligned access -- MW.  */
	memcpy (p + GNOME_METAFILE_SIGNATURE_SIZE, &l, sizeof (l));
	*buflen = meta->current;

	return 1;
}

/* less compact, but gauranteed size (just makes things easier to encode
   fixed size data like headers) */
static const char *
decode_int_header (const char *data, gint32 *dest)
{
	gint32 nint;
	memcpy (&nint, data, sizeof (gint32));
	*dest = g_ntohl (nint);
	return data + sizeof (gint32);
}

static const char *
decode_int (const char *data, gint32 *dest)
{
	guint32 vabs = 0, mask = 0x3f;
	int bits = 6, shift = 0;
	int neg;
	char c;

	neg = (*data & 0x40);
	do {
		vabs |= ((c = *data++) & mask) << shift;
		shift += bits;
		bits = 7;
		mask = 0x7f;
	} while ((c & 0x80) == 0);

	*dest = neg ? -vabs : vabs;
	return data;
}

static const char *
decode_double (const char *data, double *dest)
{
	int i;
	int sd = sizeof (double);
	data = decode_int (data, &i);

	switch ((GnomeMetaDoubleType)i) {
	case GNOME_META_DOUBLE_INT:
		data = decode_int (data, &i);
		*dest = i;
		break;
	case GNOME_META_DOUBLE_INT1000:
		data = decode_int (data, &i);
		*dest = i / 1000.0;
		break;
	case GNOME_META_DOUBLE_I386:
#if G_BYTE_ORDER == G_BIG_ENDIAN
		for (i = 0; i < sd; i++)
			((guint8 *)dest)[sd - 1 - i] = data[i];
		data += sd;
#elif G_BYTE_ORDER == G_LITTLE_ENDIAN
		memcpy (dest, data, sd);
		data += sd;
#else
#error decode_double_needs_attention
#endif
		break;
	default:
		*dest = 0;   /* ??? */
	}

	return data;
}

static const char *
decode_string (const char *data, char **dest)
{
	int len;
	data = decode_int (data, &len);
	*dest = g_malloc (len + 1);
	memcpy (*dest, data, len);
	(*dest)[len] = 0;
	return data + len;
}

static const char *
decode_image (const char *data, GnomePrintContext *dest, int bytes_per_pixel)
{
	void *buffer;
	gint32 height, width;
	int size;

	data = decode_int (data, &height);
	data = decode_int (data, &width);

	size = height * width * bytes_per_pixel;
	buffer = g_malloc (size);
	memcpy (buffer, data, size);

	if (bytes_per_pixel == 1)
		gnome_print_grayimage (dest, buffer, width, height, width);
	else if (bytes_per_pixel == 3)
		gnome_print_rgbimage (dest, buffer, width, height, width * bytes_per_pixel);
	else
		gnome_print_rgbaimage (dest, buffer, width, height, width * bytes_per_pixel);

	g_free (buffer);
	return data + size;
}

static const char *
decode_block (const char *data, void *out, int len)
{
	memcpy (out, data, len);
	return data+len;
}

static const char *
decode_header (const char *data, GnomeMetaFileHeader *mh)
{
	data = decode_block (data, mh->signature, sizeof (mh->signature));
	data = decode_int_header (data, &mh->size);
	return data;
}

static const char *
decode_page_header (const char *data, GnomeMetaPageHeader *mh)
{
	data = decode_block (data, mh->signature, sizeof (mh->signature));
	data = decode_int_header (data, &mh->size);
	return data;
}

static const char *
locate_page_header (const char *meta_stream, int page)
{
	const char *data = meta_stream + FILEHEADER_SIZE, *next;
	GnomeMetaPageHeader page_header;

	next = decode_page_header (data, &page_header);
	while (page>0 && page_header.size != -1) {
		data = next + page_header.size;
		next = decode_page_header (data, &page_header);
		page--;
	}
	if (page_header.size == -1)
		return NULL;
	return data;
}

/*
  from the current point in the data stream, print pages pages, -1
  means print all
*/
static gboolean
do_render (GnomePrintContext *dest, const char *data, int size, int pages)
{
	const char *end = data + size;
	double matrix [6];
	double x1, y1, x2, y2, x3, y3;
	double x, y, o;
	double r, g, b;
	GnomeMetaPageHeader page_header;

	/*
	 * Output if all pages were requested, or the first page was
	 *
	 * Note that this assumes each page is completely described
	 * by its own data block.
	 */
	while (data < end){
		gint32 opcode, i;
#ifdef META_DEBUG
		int start_offset;
		start_offset = size - (end - data);
#endif

		data = decode_int (data, &opcode);
#ifdef META_DEBUG
		if (opcode >= 0 &&
		    opcode < sizeof (meta_type_names)/sizeof (char *))
			g_warning ("Meta op: '%s' starts at offset 0x%x",
				   meta_type_names [opcode],
				   start_offset);
		else
			g_warning ("Meta op: OOB %d starts at offset 0x%x",
				   opcode, start_offset);
#endif

		switch ((GnomeMetaType) opcode){
		case GNOME_META_NEWPATH:
			gnome_print_newpath (dest);
			break;

		case GNOME_META_MOVETO:
			data = decode_double (data, &x);
			data = decode_double (data, &y);
			gnome_print_moveto (dest, x, y);
			break;

		case GNOME_META_LINETO:
			data = decode_double (data, &x);
			data = decode_double (data, &y);
			gnome_print_lineto (dest, x, y);
			break;

		case GNOME_META_CURVETO:
			data = decode_double (data, &x1);
			data = decode_double (data, &y1);
			data = decode_double (data, &x2);
			data = decode_double (data, &y2);
			data = decode_double (data, &x3);
			data = decode_double (data, &y3);

			gnome_print_curveto (dest, x1, y1, x2, y2, x3, y3);
			break;

		case GNOME_META_CLOSEPATH:
			gnome_print_closepath (dest);
			break;

		case GNOME_META_SETRGBCOLOR:
			data = decode_double (data, &r);
			data = decode_double (data, &g);
			data = decode_double (data, &b);
			gnome_print_setrgbcolor (dest, r, g, b);
			break;

		case GNOME_META_FILL:
			data = decode_int (data, &i);
			if (i == ART_WIND_RULE_NONZERO) {
				gnome_print_fill (dest);
			} else {
				gnome_print_eofill (dest);
			}
			break;

		case GNOME_META_SETLINEWIDTH:
			data = decode_double (data, &x);
			gnome_print_setlinewidth (dest, x);
			break;

		case GNOME_META_SETMITERLIMIT:
			data = decode_double (data, &x);
			gnome_print_setmiterlimit (dest, x);
			break;

		case GNOME_META_SETLINEJOIN:
			data = decode_int (data, &i);
			gnome_print_setlinejoin (dest, i);
			break;

		case GNOME_META_SETLINECAP:
			data = decode_int (data, &i);
			gnome_print_setlinecap (dest, i);
			break;

		case GNOME_META_SETDASH: {
			int n;
			double *values, offset;

			data = decode_int (data, &n);
			values = g_new (double, n);
			for (i = 0; i < n; i++)
				data = decode_double (data, &values [i]);
			data = decode_double (data, &offset);
			gnome_print_setdash (dest, n, values, offset);
			g_free (values);
			break;
		}

		case GNOME_META_STROKEPATH:
			gnome_print_strokepath (dest);
			break;

		case GNOME_META_STROKE:
			gnome_print_stroke (dest);
			break;

		case GNOME_META_SETFONT: {
			GnomeFont *font;
			char *name;

			data = decode_double (data, &x);
			data = decode_string (data, &name);
			font = gnome_font_new (name, x);
			g_free (name);

			if (font) {
				gnome_print_setfont (dest, font);
				gtk_object_unref (GTK_OBJECT (font));
			}
			break;
		}

		case GNOME_META_SHOW_SIZED: {
			gint length;

			data = decode_int (data, &length);

			if (length > 0) {
				char *text = g_malloc (length);

				data = decode_block (data, text, length);
				gnome_print_show_sized (dest, text, length);
				g_free (text);
			}

			break;
		}

		case GNOME_META_CONCAT:
			data = decode_double (data, &matrix [0]);
			data = decode_double (data, &matrix [1]);
			data = decode_double (data, &matrix [2]);
			data = decode_double (data, &matrix [3]);
			data = decode_double (data, &matrix [4]);
			data = decode_double (data, &matrix [5]);
			gnome_print_concat (dest, matrix);
			break;

		case GNOME_META_GSAVE:
			gnome_print_gsave (dest);
			break;

		case GNOME_META_GRESTORE:
			gnome_print_grestore (dest);
			break;

		case GNOME_META_CLIP:
			data = decode_int (data, &i);
			if (i == ART_WIND_RULE_NONZERO) {
				gnome_print_clip (dest);
			} else {
				gnome_print_eoclip (dest);
			}
			break;

		case GNOME_META_GRAYIMAGE:
			data = decode_image (data, dest, 1);
			break;

		case GNOME_META_RGBIMAGE:
			data = decode_image (data, dest, 3);
			break;

		case GNOME_META_SHOWPAGE:
			gnome_print_showpage (dest);

			/* this is unused here ... */
			data = decode_page_header (data, &page_header);

			if (pages!=-1) {
				pages--;
				if (pages<=0)
					return TRUE;
			}
			break;

		case GNOME_META_BEGINPAGE: {
			char *name;

			data = decode_string (data, &name);
			gnome_print_beginpage (dest, name);
			g_free (name);
			break;
		}

		case GNOME_META_TEXTLINE:
			g_warning ("FIXME: Text line missing");
			break;

		case GNOME_META_CLOSE:
			g_warning ("CLOSE encountered in metafile - possible version conflict");
			break;

		case GNOME_META_SETOPACITY:
			data = decode_double (data, &o);
			gnome_print_setopacity (dest, o);
			break;

		case GNOME_META_RGBAIMAGE:
			data = decode_image (data, dest, 4);
			break;
		case GNOME_META_GLYPHLIST: {
			GnomeGlyphList *gl;
			int len, code, ival, i;
			double dval;

			gl = gtk_type_new (GNOME_TYPE_GLYPHLIST);
			data = decode_int (data, &len);
			if (len > 0) {
				gl->glyphs = g_new (int, len);
				gl->g_length = len;
				gl->g_size = len;
				for (i = 0; i < len; i++) {
					data = decode_int (data, &ival);
					gl->glyphs[i] = ival;
				}
			}
			data = decode_int (data, &len);
			if (len > 0) {
				gl->rules = g_new (GGLRule, len);
				gl->r_length = len;
				gl->r_size = len;
				for (i = 0; i < len; i++) {
					data = decode_int (data, &code);
					gl->rules[i].code = code;
					switch (code) {
					case GGL_POSITION:
					case GGL_ADVANCE:
					case GGL_COLOR:
						data = decode_int (data, &ival);
						gl->rules[i].value.ival = ival;
						break;
					case GGL_MOVETOX:
					case GGL_MOVETOY:
					case GGL_RMOVETOX:
					case GGL_RMOVETOY:
					case GGL_LETTERSPACE:
					case GGL_KERNING:
						data = decode_double (data, &dval);
						gl->rules[i].value.dval = dval;
						break;
					case GGL_FONT: {
						GnomeFont *font;
						char *name;
						data = decode_double (data, &dval);
						data = decode_string (data, &name);
						font = gnome_font_new (name, dval);
						if (font == NULL) g_print ("Cannot find font: %s\n", name);
						g_free (name);
						gl->rules[i].value.font = font;
						break;
					}
					case GGL_PUSHCP:
					case GGL_POPCP:
					default:
						break;
					}
				}
			}
			gnome_print_glyphlist (dest, gl);
			gnome_glyphlist_unref (gl);
			break;
		}
		default:
			g_warning ("Serious print meta data corruption %d",
				   opcode);
			break;
		}
	}
	return TRUE;
}

/**
 * gnome_print_meta_render:
 * @destination: Destination printer context.
 * @meta_stream: a metadata stream to replay
 *
 * Plays the @meta_steam metadata stream into the @destination printer.
 *
 * Returns TRUE on success.
 */
gboolean
gnome_print_meta_render (GnomePrintContext *destination, const void *meta_stream)
{
	const char *data;
	GnomeMetaFileHeader header;

	g_return_val_if_fail (destination != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (destination), FALSE);
	g_return_val_if_fail (meta_stream != NULL, FALSE);

	data = meta_stream;
	data = decode_header (data, &header);
	if (strncmp (header.signature, GNOME_METAFILE_SIGNATURE, GNOME_METAFILE_SIGNATURE_SIZE) != 0)
		return FALSE;

	if (header.size == -1){
		g_warning ("This stream was not closed");
		return FALSE;
	}
	return do_render (destination, data + PAGEHEADER_SIZE,
			  header.size, -1);
}

/**
 * gnome_print_meta_render_from_object:
 * @destination: Destination printer context.
 * @source: an existing GnomePrintMeta printer context.
 *
 * Plays the contents of the GnomePrintMeta @source metadata stream
 * into the @destination printer.
 *
 * Returns: TRUE on success.
 */
gboolean
gnome_print_meta_render_from_object (GnomePrintContext *destination, const GnomePrintMeta *source)
{
#if 1
	const char * data;
	GnomeMetaFileHeader header;
#endif
	g_return_val_if_fail (destination != NULL, FALSE);
	g_return_val_if_fail (source != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (destination), FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_META (source), FALSE);

#if 0
	return gnome_print_meta_render (destination, source->buffer);
#else
	/* Render even unclosed metafiles */

	data = source->buffer;
	data = decode_header (data, &header);

	if (strncmp (header.signature, GNOME_METAFILE_SIGNATURE, GNOME_METAFILE_SIGNATURE_SIZE) != 0)
		return FALSE;

	return do_render (destination, data + PAGEHEADER_SIZE,
			  source->current - FILEHEADER_SIZE - PAGEHEADER_SIZE, -1);
#endif
}

/**
 * gnome_print_meta_render_page:
 * @destination: Destination printer context.
 * @meta_stream: a metadata stream to replay
 * @page: Page to be rendered.
 *
 * Plays the @meta_steam metadata stream into the @destination printer.
 * Output commands will only take place for page @page.
 *
 * Returns TRUE on success.
 */
gboolean
gnome_print_meta_render_page (GnomePrintContext *destination, const void *meta_stream, int page)
{
	const char *data;
	GnomeMetaFileHeader header;
	const char *page_data;

	g_return_val_if_fail (destination != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (destination), FALSE);
	g_return_val_if_fail (meta_stream != NULL, FALSE);

	data = meta_stream;
	data = decode_header (data, &header);
	if (strncmp (header.signature, GNOME_METAFILE_SIGNATURE, GNOME_METAFILE_SIGNATURE_SIZE) != 0)
		return FALSE;

	if (header.size == -1){
		g_warning ("This printing context has not been closed");
		return FALSE;
	}

	page_data = locate_page_header (meta_stream, page);
	if (page_data == NULL) {
		g_warning ("Trying to print a non-existant page");
		return FALSE;
	}
	return do_render (destination, page_data + PAGEHEADER_SIZE,
			  header.size, 1);
}

/**
 * gnome_print_meta_render_from_object_page:
 * @destination: Destination printer context.
 * @source: an existing GnomePrintMeta printer context.
 * @page: Page to be rendered.
 *
 * Plays the contents of the GnomePrintMeta @source metadata stream
 * into the @destination printer.  Output commands will only take place for page @page.
 *
 * Returns: TRUE on success.
 */
gboolean
gnome_print_meta_render_from_object_page (GnomePrintContext *destination,
					  const GnomePrintMeta *source,
					  int page)
{
	g_return_val_if_fail (destination != NULL, FALSE);
	g_return_val_if_fail (source != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_CONTEXT (destination), FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_META (source), FALSE);

	return gnome_print_meta_render_page (destination, source->buffer, page);
}

int
gnome_print_meta_pages (const GnomePrintMeta *meta)
{
	g_return_val_if_fail (meta != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_META (meta), 0);

	return meta->pages;
}
