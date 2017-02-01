#include <gtk/gtk.h>
#include <gmodule.h>

/**************************************************************************
* GTK Metal Theme
*
* Version 0.9, Oct 2, 1998
*
* Copyright 1998: Randy Gordon, Integrand Systems
*                 http://www.integrand.com
*                 mailto://randy@integrand.com
*
* License: GPL (Gnu Public License)
*
*
**************************************************************************/
#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "Metal-Theme"

#define DEBUG 0

#define SCROLLBAR_WIDTH 17
#if 0
#define SCALE_WIDTH     15
#else
#define SCALE_WIDTH     31
#endif

/* Theme functions to export */
void                theme_init(GtkThemeEngine * engine);
void                theme_exit(void);

/* Exported vtable from th_draw */

extern GtkStyleClass metal_default_class;
extern GtkStyleClass metal_special_class;

/* internals */

/* external theme functions called */

static gint
gtk_range_expose_metal (GtkWidget      *widget,
                       GdkEventExpose *event);
static void
shade(GdkColor *oldcolor, GdkColor *newcolor, float mult);


/* Useful GCs for coloring (based on white_gc) */
GdkGC *metal_light_gray_gc;
GdkGC *metal_mid_gray_gc;
GdkGC *metal_dark_gray_gc;

/* Information about a single RC style
 */
typedef struct
  {
    guint refcount;
    gint  thickness;
  }
ThemeData;


enum
  {
    TOKEN_THICKNESS = G_TOKEN_LAST + 1
  };

static struct
  {
    gchar              *name;
    guint               token;
  }
theme_symbols[] =
{
  { "thickness",		TOKEN_THICKNESS  },
};

static guint        n_theme_symbols = sizeof(theme_symbols) / sizeof(theme_symbols[0]);

static void
theme_data_ref (ThemeData *theme_data)
{
  theme_data->refcount++;
}

static void
theme_data_unref (ThemeData *theme_data)
{
  theme_data->refcount--;
  if (theme_data->refcount == 0)
    {
      g_free (theme_data);
    }
}

/* external theme functions called */

static guint
theme_parse_rc_style(GScanner * scanner,
		     GtkRcStyle * rc_style)
{
  static GQuark       scope_id = 0;
  guint               old_scope;
  guint               token;
  ThemeData          *theme_data;
  gint i;

  /* Set up a new scope in this scanner. */

  if (!scope_id)
    scope_id = g_quark_from_string("theme_engine");

  /* If we bail out due to errors, we *don't* reset the scope, so the
   * error messaging code can make sense of our tokens.
   */
  old_scope = g_scanner_set_scope(scanner, scope_id);

  /* Now check if we already added our symbols to this scope
   * (in some previous call to theme_parse_rc_style for the
   * same scanner.
   */

  if (!g_scanner_lookup_symbol(scanner, theme_symbols[0].name))
    {
      g_scanner_freeze_symbol_table(scanner);
      for (i = 0; i < n_theme_symbols; i++)
	g_scanner_scope_add_symbol(scanner, scope_id,
				   theme_symbols[i].name,
				   GINT_TO_POINTER(theme_symbols[i].token));
      g_scanner_thaw_symbol_table(scanner);
    }

  /* We're ready to go, now parse the top level */

  theme_data = g_new(ThemeData, 1);
  theme_data->thickness = 2;
  theme_data->refcount = 1;

  token = g_scanner_peek_next_token(scanner);
  while (token != G_TOKEN_RIGHT_CURLY)
    {
      switch (token)
	{
	case TOKEN_THICKNESS:
	  token = g_scanner_get_next_token(scanner);

	  token = g_scanner_get_next_token(scanner);
	  if (token != G_TOKEN_EQUAL_SIGN)
	    {
	      token = G_TOKEN_EQUAL_SIGN;
	      break;
	    }

	  token = g_scanner_get_next_token(scanner);
	  if (token != G_TOKEN_INT)
	    {
	      token = G_TOKEN_INT;
	      break;
	    }

	  theme_data->thickness = scanner->value.v_int;
	  token = G_TOKEN_NONE;
	  break;

	default:
	  g_scanner_get_next_token(scanner);
	  token = G_TOKEN_RIGHT_CURLY;
	  break;
	}

      if (token != G_TOKEN_NONE)
	{
	  g_free (theme_data);
	  return token;
	}

      token = g_scanner_peek_next_token(scanner);
    }

  g_scanner_get_next_token(scanner);

  rc_style->engine_data = theme_data;
  g_scanner_set_scope(scanner, old_scope);

  return G_TOKEN_NONE;
}

static void
theme_merge_rc_style(GtkRcStyle * dest,
		     GtkRcStyle * src)
{
  ThemeData        *src_data = src->engine_data;

  if (!dest->engine_data)
    {
      if (src_data)
	{
	  theme_data_ref (src_data);
	  dest->engine_data = src_data;
	}
    }
}

static void
theme_rc_style_to_style(GtkStyle * style,
			GtkRcStyle * rc_style)
{
  ThemeData        *data = rc_style->engine_data;

  switch (data->thickness)
    {
    case 1:
      style->klass = &metal_special_class;
      break;
    case 2:
      style->klass = &metal_default_class;
      break;
    default:
      style->klass = &metal_default_class;
      g_warning ("metal theme: Invalid thickness %d in RC file\n",
		 data->thickness);
    }
}

static void
theme_duplicate_style(GtkStyle * dest,
		      GtkStyle * src)
{
  dest->klass = src->klass;
}

static void
theme_realize_style(GtkStyle * style)
{
}

static void
theme_unrealize_style(GtkStyle * style)
{
}

static void
theme_destroy_rc_style(GtkRcStyle * rc_style)
{
  theme_data_unref (rc_style->engine_data);
}

static void
theme_destroy_style(GtkStyle * style)
{
}

typedef gint (*GtkExposeEventFunc)(GtkWidget *widget,
                                  GdkEventExpose *event);
static GtkExposeEventFunc range_expose_event;
static gint range_slider_width;
static gint range_min_slider_size;
static gint range_stepper_size;
static gint range_stepper_slider_spacing;
static GtkExposeEventFunc scale_expose_event;
static gint scale_slider_length;

/******************************************************************/
void
theme_init(GtkThemeEngine * engine)
{
   GtkRangeClass *rangeclass;
   GtkScaleClass *scaleclass;
   GtkButtonClass *buttonclass;
   GtkToggleButtonClass *togglebuttonclass;
   GtkWidgetClass *widget_class;
   GtkStyle *style;
   GtkVScrollbar *scrollbar;
   GdkColor white = { 0, 0xffff, 0xffff, 0xffff };
   GdkColor gray;
   GdkGCValues values;
   GdkColormap *colormap;
   gint depth;

#if DEBUG
  printf("Metal Theme Init\n");
#endif

   engine->parse_rc_style = theme_parse_rc_style;
   engine->merge_rc_style = theme_merge_rc_style;
   engine->rc_style_to_style = theme_rc_style_to_style;
   engine->duplicate_style = theme_duplicate_style;
   engine->realize_style = theme_realize_style;
   engine->unrealize_style = theme_unrealize_style;
   engine->destroy_rc_style = theme_destroy_rc_style;
   engine->destroy_style = theme_destroy_style;
   engine->set_background = NULL;

   /* Make scrollbars wider */
   rangeclass = (GtkRangeClass *)gtk_type_class(gtk_range_get_type());
   range_slider_width = rangeclass->slider_width;
   range_min_slider_size = rangeclass->min_slider_size;
   range_stepper_size = rangeclass->stepper_size;
   range_stepper_slider_spacing = rangeclass->stepper_slider_spacing;
   rangeclass->slider_width    = SCROLLBAR_WIDTH;
   rangeclass->min_slider_size = SCROLLBAR_WIDTH;
   rangeclass->stepper_size    = SCROLLBAR_WIDTH;
   rangeclass->stepper_slider_spacing = 0;

   /* Make scale slider smaller */
   scaleclass = (GtkScaleClass *)gtk_type_class(gtk_scale_get_type());
   scale_slider_length = scaleclass->slider_length;
   scaleclass->slider_length   = SCALE_WIDTH;

   /* A bunch of hacks to workaround GTK problems */
   widget_class = (GtkWidgetClass *)rangeclass;
   range_expose_event = widget_class->expose_event;
   widget_class->expose_event = gtk_range_expose_metal;

   widget_class = (GtkWidgetClass *)scaleclass;
   scale_expose_event = widget_class->expose_event;
   widget_class->expose_event = gtk_range_expose_metal;

   /* Some useful GCs for coloring (based on shades of style->white) */
   colormap = gdk_colormap_get_system();
   depth = gdk_visual_get_system()->depth;

   /* Light Gray */
   shade(&white, &gray, 0.8);
   if (!gdk_color_alloc (colormap, &gray)) {
        g_warning ("unable to allocate color: ( %d %d %d )",
                   gray.red, gray.green, gray.blue);
   }
   values.foreground = gray;
   metal_light_gray_gc = gtk_gc_get(depth, colormap, 
                                    &values, GDK_GC_FOREGROUND);
   /* Mid Gray */
   shade(&white, &gray, 0.6);
   if (!gdk_color_alloc (colormap, &gray)) {
        g_warning ("unable to allocate color: ( %d %d %d )",
                   gray.red, gray.green, gray.blue);
   }
   values.foreground = gray;
   metal_mid_gray_gc = gtk_gc_get(depth, colormap, 
                                    &values, GDK_GC_FOREGROUND);
   /* Dark Gray */
   shade(&white, &gray, 0.5);
   if (!gdk_color_alloc (colormap, &gray)) {
        g_warning ("unable to allocate color: ( %d %d %d )",
                   gray.red, gray.green, gray.blue);
   }
   values.foreground = gray;
   metal_dark_gray_gc = gtk_gc_get(depth, colormap, 
                                    &values, GDK_GC_FOREGROUND);
}
/*****************************************************************/

static void
restore_expose_events(GtkObjectClass *klass,
                     GtkExposeEventFunc find_event,
                     GtkExposeEventFunc replace_event)
{
  GList *child_list, *child;
  g_message("Restoring expose events for %s",
           gtk_type_name(klass->type));
  if (!GTK_IS_WIDGET_CLASS(klass)) {
    g_error("Warning: restore expose events for non-widget");
  }
  else {
    GtkWidgetClass *widget_klass = GTK_WIDGET_CLASS(klass);
    if (widget_klass->expose_event == find_event)
      widget_klass->expose_event = replace_event;
  }
  child_list = gtk_type_children_types(klass->type);
  for (child = child_list; child; child = child->next) {
    gpointer child_class = gtk_type_class(GPOINTER_TO_UINT(child->data));
    restore_expose_events(GTK_OBJECT_CLASS(child_class),
                         find_event, replace_event);
  }
}


void
theme_exit(void)
{
   GtkRangeClass *rangeclass;
   GtkScaleClass *scaleclass;

   rangeclass = (GtkRangeClass *)gtk_type_class(gtk_range_get_type());
   scaleclass = (GtkScaleClass *)gtk_type_class(gtk_scale_get_type());

   rangeclass->slider_width = range_slider_width;
   rangeclass->min_slider_size = range_min_slider_size;
   rangeclass->stepper_size = range_stepper_size;
   rangeclass->stepper_slider_spacing = range_stepper_slider_spacing;

   scaleclass->slider_length = scale_slider_length;

   /* This is nasty: the expose events we installed in init() may have
      been copied into subclasses. */
   restore_expose_events(GTK_OBJECT_CLASS(rangeclass),
                        GTK_WIDGET_CLASS(rangeclass)->expose_event,
                        range_expose_event);
   restore_expose_events(GTK_OBJECT_CLASS(scaleclass),
                        GTK_WIDGET_CLASS(scaleclass)->expose_event,
                        scale_expose_event);

#if DEBUG
  printf("Metal Theme Exit\n* Need to add memory deallocation code here *\n");
#endif
}
/****************************************************************/
static gint
gtk_range_expose_metal (GtkWidget      *widget,
                       GdkEventExpose *event)
{
  GtkRange *range;

  /* The version of this method in gtkrange.c doesn't work
   * when the slider is as wide as the trough.
   */
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_RANGE (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  range = GTK_RANGE (widget);

  if (event->window == range->trough)
    {
        gtk_range_draw_trough (range);
    }
  else if (event->window == widget->window)
    {
      gtk_range_draw_background (range);
    }
  else if (event->window == range->slider)
    {
      gtk_range_draw_slider (range);
    }
  else if (event->window == range->step_forw)
    {
      gtk_range_draw_step_forw (range);
    }
  else if (event->window == range->step_back)
    {
      gtk_range_draw_step_back (range);
    }
  return FALSE;
}
/******************************************************************/
static void
shade(GdkColor *oldcolor, GdkColor *newcolor, float mult)
{
   newcolor->red   = oldcolor->red   * mult;
   newcolor->green = oldcolor->green * mult;
   newcolor->blue  = oldcolor->blue  * mult;
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
