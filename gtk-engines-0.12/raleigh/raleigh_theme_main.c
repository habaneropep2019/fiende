#include <gmodule.h>
#include "raleigh.h"

/* Theme functions to export */
void                theme_init(GtkThemeEngine * engine);
void                theme_exit(void);


/* internals */

static void
interpolate_color (GdkColor *a,
		   GdkColor *b,
		   GdkColor *c,
		   gdouble   k)
{
  c->red = a->red * (1. - k) + b->red * k;
  c->green = a->green * (1. - k) + b->green * k;
  c->blue = a->blue * (1. - k) + b->blue * k;
}

static guint
theme_parse_rc_style (GScanner   *scanner,
		      GtkRcStyle *rc_style)
{
  guint               token;

  token = g_scanner_peek_next_token(scanner);
  while (token != G_TOKEN_RIGHT_CURLY)
    {
      switch (token)
	{
	default:
	  g_scanner_get_next_token(scanner);
	  token = G_TOKEN_RIGHT_CURLY;
	  break;
	}

      if (token != G_TOKEN_NONE)
	return token;

      token = g_scanner_peek_next_token(scanner);
    }

  g_scanner_get_next_token(scanner);

  rc_style->engine_data = NULL;

  return G_TOKEN_NONE;
}

static void
theme_merge_rc_style(GtkRcStyle * dest,
		     GtkRcStyle * src)
{
}

static void
set_props (GtkStyle *style)
{
  gtk_style_set_prop_experimental (style, "GtkButton::default_spacing", 6);
  gtk_style_set_prop_experimental (style, "GtkCheckButton::indicator_size", 13);
  gtk_style_set_prop_experimental (style, "GtkOptionMenu::indicator_width", OPTION_INDICATOR_WIDTH);
  gtk_style_set_prop_experimental (style, "GtkOptionMenu::indicator_left_spacing", OPTION_INDICATOR_LEFT_SPACING);
  gtk_style_set_prop_experimental (style, "GtkOptionMenu::indicator_right_spacing", OPTION_INDICATOR_RIGHT_SPACING);
  gtk_style_set_prop_experimental (style, "GtkPaned::handle_full_size", 1);
  gtk_style_set_prop_experimental (style, "GtkRange::trough_border", 1);
  gtk_style_set_prop_experimental (style, "GtkRange::slider_width", 14);
  gtk_style_set_prop_experimental (style, "GtkRange::stepper_size", 14);
  gtk_style_set_prop_experimental (style, "GtkRange::stepper_spacing", 0);
  gtk_style_set_prop_experimental (style, "GtkSpinButton::shadow_type", GTK_SHADOW_IN);
}

static void
theme_rc_style_to_style(GtkStyle   *style,
			GtkRcStyle *rc_style)
{
  static GtkStyleClass *class = NULL;
  RaleighEngineData *engine_data;
  gint i;

  if (!class)
    {
      GtkStyle *tmp_style = gtk_style_new ();
      class = g_new (GtkStyleClass, 1);

      raleigh_initialize_style (class, style->klass);
      gtk_style_unref (tmp_style);
    }
  
  style->klass = class;
  engine_data = style->engine_data = g_new0 (RaleighEngineData, 1);

  for (i = 0; i < 5; i++)
    interpolate_color (&style->text[i], &style->base[i], &engine_data->aa[i], 0.5);
  set_props (style);
}

static void
theme_duplicate_style(GtkStyle *dest,
		      GtkStyle *src)
{
  dest->engine_data = g_memdup (src->engine_data, sizeof (RaleighEngineData));
  
  set_props (dest);
}

static void
theme_realize_style(GtkStyle * style)
{
  gint i;
  RaleighEngineData *engine_data = style->engine_data;

  for (i = 0; i < 5; i++)
    {
      GdkGCValues gc_values;

      gdk_colormap_alloc_color (style->colormap, &engine_data->aa[i],
				FALSE, TRUE);
      
      gc_values.foreground = engine_data->aa[i];

      engine_data->aa_gc[i] = gtk_gc_get (style->depth, style->colormap,
					  &gc_values, GDK_GC_FOREGROUND);
    }
}

static void
theme_unrealize_style(GtkStyle * style)
{
  gint i;
  RaleighEngineData *engine_data = style->engine_data;

  for (i = 0; i < 5; i++)
    {
      gtk_gc_release (engine_data->aa_gc[i]);

      /* We don't free the colors, because we don't know if
       * gtk_gc_release() actually freed the GC. FIXME - need
       * a way of ref'ing colors explicitely so GtkGC can
       * handle things properly.
       */
    }
}

static void
theme_destroy_rc_style(GtkRcStyle * rc_style)
{
}

static void
theme_destroy_style(GtkStyle * style)
{
  g_free (style->engine_data);
}

void
theme_init(GtkThemeEngine * engine)
{
  engine->parse_rc_style = theme_parse_rc_style;
  engine->merge_rc_style = theme_merge_rc_style;
  engine->rc_style_to_style = theme_rc_style_to_style;
  engine->duplicate_style = theme_duplicate_style;
  engine->realize_style = theme_realize_style;
  engine->unrealize_style = theme_unrealize_style;
  engine->destroy_rc_style = theme_destroy_rc_style;
  engine->destroy_style = theme_destroy_style;
  engine->set_background = NULL;

}

void
theme_exit(void)
{
}

/* The following function will be called by GTK+ when the module
 * is loaded and checks to see if we are compatible with the
 * version of GTK+ that loads us.
 */
G_MODULE_EXPORT const gchar* g_module_check_init (GModule *module);
const gchar*
g_module_check_init (GModule *module)
{
  return gtk_check_version (GTK_MAJOR_VERSION,
			    GTK_MINOR_VERSION,
			    GTK_MICRO_VERSION - GTK_INTERFACE_AGE);
}

