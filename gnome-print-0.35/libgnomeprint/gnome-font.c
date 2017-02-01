#include <math.h>

#include <stdlib.h>
#include <gtk/gtkobject.h>
#include <gtk/gtkwidget.h>
#include <libart_lgpl/art_affine.h>

#include <string.h>
#include <libgnomeprint/gp-unicode.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-font-private.h>

static void gnome_font_class_init (GnomeFontClass *klass);

static void gnome_font_init (GnomeFont *font);

static void gnome_font_finalize (GtkObject *object);
static void gnome_font_set_arg (GtkObject *o, GtkArg *arg, guint arg_id);
static void gnome_font_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static guint font_hash (gconstpointer key);
static gboolean font_equal (gconstpointer key1, gconstpointer key2);

static GHashTable * fonts = NULL;

static GtkObjectClass *parent_class = NULL;

/* The arguments we take */
enum {
	ARG_0,
	ARG_ASCENDER,
	ARG_DESCENDER,
	ARG_UNDERLINE_POSITION,
	ARG_UNDERLINE_THICKNESS
};

GtkType
gnome_font_get_type (void)
{
	static GtkType font_type = 0;
	if (!font_type) {
		GtkTypeInfo font_info = {
			"GnomeFont",
			sizeof (GnomeFont),
			sizeof (GnomeFontClass),
			(GtkClassInitFunc) gnome_font_class_init,
			(GtkObjectInitFunc) gnome_font_init,
			NULL, NULL, NULL
		};
		font_type = gtk_type_unique (gtk_object_get_type (), &font_info);
	}
	return font_type;
}

/* Return the amount of kerning for the two glyphs. */

gdouble
gnome_font_get_glyph_kerning (const GnomeFont *font, int glyph1, int glyph2)
{
	gdouble kern;

	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);

	kern = gnome_font_face_get_glyph_kerning (font->face, glyph1, glyph2);

	return kern * 0.001 * font->size;
}

/* Return the amount of kerning for the two glyphs. */

gdouble
gnome_font_face_get_glyph_kerning (const GnomeFontUnsized *face, int glyph1, int glyph2)
{
	g_return_val_if_fail (face != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), 0.0);
#if 0
  int ktabsize;
  const GnomeFontKernPair *ktab;
  int j;

  g_return_val_if_fail (font != NULL, 0);
    
  ktabsize = font->private->num_kerns;
  ktab = font->private->kerns;
  for (j = KERN_PAIR_HASH (glyph1, glyph2) & (ktabsize - 1);
       ktab[j].glyph1 != -1;
       j = (j + 1) & (ktabsize - 1))
    if (ktab[j].glyph1 == glyph1 && ktab[j].glyph2 == glyph2)
      return ktab[j].x_amt;
  return 0;
#else
  /* fixme: (Lauris) */
  return 0.0;
#endif
}

static void
gnome_font_class_init (GnomeFontClass *class)
{
  GtkObjectClass *object_class;

  object_class = (GtkObjectClass*) class;

  parent_class = gtk_type_class (gtk_object_get_type ());

  
  gtk_object_add_arg_type ("GnomeFont::ascender", GTK_TYPE_DOUBLE, 
			   GTK_ARG_READABLE, ARG_ASCENDER);
  gtk_object_add_arg_type ("GnomeFont::descender", GTK_TYPE_DOUBLE, 
			   GTK_ARG_READABLE, ARG_DESCENDER);
  gtk_object_add_arg_type ("GnomeFont::underline_position", GTK_TYPE_DOUBLE, 
			   GTK_ARG_READABLE, ARG_UNDERLINE_POSITION);
  gtk_object_add_arg_type ("GnomeFont::underline_thickness", GTK_TYPE_DOUBLE, 
			   GTK_ARG_READABLE, ARG_UNDERLINE_THICKNESS);

  object_class->finalize = gnome_font_finalize;
  object_class->set_arg = gnome_font_set_arg;
  object_class->get_arg = gnome_font_get_arg;
}

static void
gnome_font_init (GnomeFont *font)
{
	font->size = 0.0;
	font->outlines = g_hash_table_new (NULL, NULL);
}

static guint
font_hash (gconstpointer key)
{
	return (guint) ((GnomeFont *) key)->face + (guint) ((GnomeFont *) key)->size;
}

static gboolean
font_equal (gconstpointer key1, gconstpointer key2)
{
	GnomeFontPrivate * priv1, * priv2;

	priv1 = (GnomeFontPrivate *) key1;
	priv2 = (GnomeFontPrivate *) key2;

	if (((GnomeFont *) key1)->face != ((GnomeFont *) key2)->face) return FALSE;

	return (fabs (((GnomeFont *) key1)->size - ((GnomeFont *) key2)->size) < 0.05);
}

GnomeFont *
gnome_font_face_get_font_full (GnomeFontFace * face, gdouble size, gdouble * affine)
{
	GnomeFont * font;
	GnomeFont search;

	g_return_val_if_fail (face != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_FACE (face), NULL);

	if (!fonts) fonts = g_hash_table_new (font_hash, font_equal);

	search.face = face;
	search.size = size;

	font = g_hash_table_lookup (fonts, &search);

	if (font) {
		gnome_font_ref (font);
		return font;
	}

	font = gtk_type_new (gnome_font_get_type ());

	font->size = size;
	font->face = face;

	gnome_font_face_ref (face);

	g_hash_table_insert (fonts, font, font);

	return font;
}

GnomeFont *
gnome_font_new (const char * name, double size)
{
	GnomeFontFace *face;
	GnomeFont *font;

	face = gnome_font_face_new (name);

	if (face) {
		font = gnome_font_face_get_font_full (face, size, NULL);
		gnome_font_face_unref (face);
	} else {
		font = NULL;
	}

	return font;
}

GnomeFont *
gnome_font_new_from_full_name (const char *name)
{
  char *copy;
  char *str_size;
  double size;
  GnomeFont *font;

  g_return_val_if_fail(name != NULL, NULL);

  copy = g_strdup(name);
  str_size = strrchr(copy, ' ');
  if ( str_size ) 
    {
      *str_size = 0;
      str_size ++;
    size = atof (str_size);
    } 
  else 
    size = 10;

  font = gnome_font_new( copy, size );
  g_free(copy);
  return font;
}

/* Find the closest weight matching the family name, weight, and italic
   specs. */

GnomeFont *
gnome_font_new_closest (const char * family_name,
			GnomeFontWeight weight,
			gboolean italic,
			double size)
{
	GnomeFontFace * face;
	GnomeFont * font;

	face = gnome_font_unsized_closest (family_name, weight, italic);

	if (face) {
		font = gnome_font_face_get_font_full (face, size, NULL);
		gnome_font_face_unref (face);
	} else {
		font = NULL;
	}

	return font;
}

static void
gnome_font_set_arg (GtkObject *o, GtkArg *arg, guint arg_id)
{
	GnomeFont *gnome_font;

	gnome_font = GNOME_FONT (o);
	
	switch (arg_id){
	default:
	  break;
	}
}

static void
gnome_font_get_arg (GtkObject *object, GtkArg *arg, guint arg_id)
{
	GnomeFont *gnome_font;

	gnome_font = GNOME_FONT (object);

	switch (arg_id) {
	case ARG_ASCENDER:
	  GTK_VALUE_DOUBLE (*arg) = gnome_font_get_ascender(gnome_font);
	  break;
	case ARG_DESCENDER:
	  GTK_VALUE_DOUBLE (*arg) = gnome_font_get_descender(gnome_font);
	  break;
	case ARG_UNDERLINE_POSITION:
	  GTK_VALUE_DOUBLE (*arg) = gnome_font_get_underline_position(gnome_font);
	  break;
	case ARG_UNDERLINE_THICKNESS:
	  GTK_VALUE_DOUBLE (*arg) = gnome_font_get_underline_thickness(gnome_font);
	  break;
	default:
	  arg->type = GTK_TYPE_INVALID;
	  break;
	}
}

ArtPoint *
gnome_font_get_glyph_stdadvance (const GnomeFont * font, gint glyph, ArtPoint * advance)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (advance != NULL, NULL);

	g_return_val_if_fail (gnome_font_face_get_glyph_stdadvance (font->face, glyph, advance), NULL);

	advance->x *= 0.001 * font->size;
	advance->y *= 0.001 * font->size;

	return advance;
}

/* fixme: */

ArtDRect *
gnome_font_get_glyph_stdbbox (const GnomeFont * font, gint glyph, ArtDRect * bbox)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
	g_return_val_if_fail (bbox != NULL, NULL);

	g_return_val_if_fail (gnome_font_face_get_glyph_stdbbox (font->face, glyph, bbox), NULL);

	bbox->x0 *= 0.001 * font->size;
	bbox->y0 *= 0.001 * font->size;
	bbox->x1 *= 0.001 * font->size;
	bbox->y1 *= 0.001 * font->size;

	return bbox;
}

/* fixme: implement */

const ArtBpath *
gnome_font_get_glyph_stdoutline (const GnomeFont * font, gint glyph)
{
	const ArtBpath *outline, *faceoutline;

	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	outline = g_hash_table_lookup (font->outlines, GINT_TO_POINTER (glyph));

	if (!outline) {
		gdouble affine[6];

		faceoutline = gnome_font_face_get_glyph_stdoutline (font->face, glyph);

		if (!faceoutline) return NULL;

		art_affine_scale (affine, 0.001 * font->size, 0.001 * font->size);
		outline = art_bpath_affine_transform (faceoutline, affine);

		g_hash_table_insert (font->outlines, GINT_TO_POINTER (glyph), (gpointer) outline);
	}

	return outline;
}

/**
 * gnome_font_get_ascender:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The ascender of the font.
 */
double
gnome_font_get_ascender (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_ascender (font->face) * 0.001 * font->size;
}

/**
 * gnome_font_get_descender:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The descender of the font.
 */
double
gnome_font_get_descender (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_descender (font->face) * 0.001 * font->size;
}

/**
 * gnome_font_get_underline_position:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The underline position of the font.
 */
double
gnome_font_get_underline_position (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_underline_position (font->face) * 0.001 * font->size;
}

/**
 * gnome_font_get_underline_thickness:
 * @font: the GnomeFont to operate on
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: The underline thickness of the font.
 */
double
gnome_font_get_underline_thickness (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0); 

	return gnome_font_face_get_underline_thickness (font->face) * 0.001 * font->size;
}

/* return a pointer to the (PostScript) name of the font */
const gchar *
gnome_font_get_name (const GnomeFont *font)
{
  g_return_val_if_fail (font != NULL, NULL);
  g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

  return gnome_font_face_get_name (font->face);
}

const gchar *
gnome_font_get_ps_name (const GnomeFont *font)
{
  g_return_val_if_fail (font != NULL, NULL);
  g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

  return gnome_font_face_get_ps_name (font->face);
}

char *
gnome_font_get_full_name (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return g_strdup_printf ("%s %f", gnome_font_get_name (font), font->size);
}

char *
gnome_font_get_pfa (const GnomeFont *font)
{
  g_return_val_if_fail (font != NULL, NULL);
  g_return_val_if_fail (GNOME_IS_FONT (font), NULL);
  return gnome_font_face_get_pfa (font->face);
}

const gchar *
gnome_font_get_family_name (const GnomeFont * font)
{
	g_return_val_if_fail (font != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

	return gnome_font_face_get_family_name (font->face);
}

/* fixme: implement */

const gchar *
gnome_font_get_species_name (const GnomeFont * font)
{
  g_return_val_if_fail (font != NULL, NULL);
  g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

  return gnome_font_face_get_species_name (font->face);
}

GnomeFontWeight
gnome_font_get_weight_code (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0);

	return gnome_font_face_get_weight_code (font->face);
}

gboolean
gnome_font_is_italic (const GnomeFont *font)
{
	g_return_val_if_fail (font != NULL, 0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0);

	return gnome_font_face_is_italic (font->face);
}

gdouble
gnome_font_get_size (const GnomeFont * font)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);

	return font->size;
}

const GnomeFontFace *
gnome_font_get_face (const GnomeFont * font)
{
  g_return_val_if_fail (font != NULL, NULL);
  g_return_val_if_fail (GNOME_IS_FONT (font), NULL);

  return font->face;
}

gdouble
gnome_font_get_glyph_width (const GnomeFont *font, gint ch)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);

	return gnome_font_face_get_glyph_width (font->face, ch) * 0.001 * font->size;
}

double
gnome_font_get_width_utf8 (const GnomeFont *font, const char *s)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);
	g_return_val_if_fail (s != NULL, 0.0);

	return gnome_font_get_width_utf8_sized (font, s, strlen (s));
}

double
gnome_font_get_width_utf8_sized (const GnomeFont *font, const char *text, int n)
{
	double width;
	const gchar *p;

	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);
	g_return_val_if_fail (text != NULL, 0.0);

	width = 0.0;

	for (p = text; p && p < (text + n); p = g_utf8_next_char (p)) {
		gint unival, glyph;
		unival = g_utf8_get_char (p);
		glyph = gnome_font_lookup_default (font, unival);
		width += gnome_font_face_get_glyph_width (font->face, glyph);
	}

	return width * 0.001 * font->size;
}

/**
 * gnome_font_get_width_string:
 * @font: the GnomeFont to operate on
 * @s: the string to measure
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: the length of the string @s using the @font.
 */
double
gnome_font_get_width_string (const GnomeFont *font, const char *s)
{
	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);
	g_return_val_if_fail (s != NULL, 0.0);

	return gnome_font_get_width_string_n (font, s, strlen (s));
}

/**
 * gnome_font_get_width_string_n:
 * @font: the GnomeFont to operate on
 * @s: the string to measure
 * @n: number of characters to measure
 *
 * This works with the standard Adobe encoding and without kerning or
 * ligatures. When the text libs get written, this function will be
 * deprecated.
 *
 * Returns: the length of the @n characters of the string @s using
 * the @font.
 */
double
gnome_font_get_width_string_n (const GnomeFont *font, const char *s, int n)
{
	double width;
	gint i;

	g_return_val_if_fail (font != NULL, 0.0);
	g_return_val_if_fail (GNOME_IS_FONT (font), 0.0);
	g_return_val_if_fail (s != NULL, 0.0);

	width = 0.0;

	for (i = 0; i < n; i++) {
		gint glyph;
		glyph = gnome_font_face_lookup_default (font->face, ((guchar *) s)[i]);
		width += gnome_font_face_get_glyph_width (font->face, glyph);
	}

	return width * 0.001 * font->size;
}

/* Get the glyph number corresponding to a given unicode, or -1 if it
   is not mapped. */
gint
gnome_font_lookup_default (const GnomeFont *font, gint unicode)
{
	g_return_val_if_fail (font != NULL, -1);
	g_return_val_if_fail (GNOME_IS_FONT (font), -1);

	return gnome_font_face_lookup_default (font->face, unicode);
}

static gboolean
free_outline (gpointer a, gpointer b, gpointer data)
{
	g_free (b);

	return TRUE;
}

static void
gnome_font_finalize (GtkObject *object)
{
	GnomeFont *font;

	font = GNOME_FONT (object);

	if (font->face) {
		g_hash_table_remove (fonts, font);
		gnome_font_face_unref (font->face);
		font->face = NULL;
	}

	if (font->outlines) {
		g_hash_table_foreach_remove (font->outlines, free_outline, NULL);
		g_hash_table_destroy (font->outlines);
		font->outlines = NULL;
	}

	(* GTK_OBJECT_CLASS (parent_class)->finalize) (object);
}




