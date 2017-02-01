/* $Id: gdict-pref.c,v 1.9 2000/04/06 00:46:16 hovinen Exp $ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *  Mike Hughes <mfh@psilord.com>
 *  Bradford Hovinen <hovinen@udel.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  GDict preferences
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>

#include "dict.h"
#include "gdict-pref.h"
#include "gdict-app.h"
#include "gdict-applet.h"

GDictPref gdict_pref = { 
    NULL, 0, TRUE, NULL, NULL,
    TRUE,
    { { NULL, NULL, NULL },
      { NULL, NULL, NULL },
      { NULL, NULL, NULL },
      { NULL, NULL, NULL },
      { NULL, NULL, NULL },
      { NULL, NULL, NULL },
      { NULL, NULL, NULL },
      { NULL, NULL, NULL } }
};

static GdkFont *
config_get_font (gchar *path, gchar *dflt) {
    gchar *key = g_strconcat(path, "=", dflt, NULL);
    gchar *fn_name = gnome_config_get_string(key);
    GdkFont *font = gdk_font_load(fn_name);
    
    g_free(key);
    g_free(fn_name);
    return font;
}

static gchar *
config_get_font_name (gchar *path, gchar *dflt) {
    gchar *key = g_strconcat(path, "=", dflt, NULL);
    gchar *fn_name = gnome_config_get_string(key);
    
    g_free(key);
    return fn_name;
}

static GdkColor *
config_get_color (gchar *path, gint red_dflt, gint green_dflt, gint blue_dflt) {
    gchar *red_key = g_strdup_printf("%s_red=%d", path, red_dflt);
    gchar *green_key = g_strdup_printf("%s_green=%d", path, green_dflt);
    gchar *blue_key = g_strdup_printf("%s_blue=%d", path, blue_dflt);
    gint red = gnome_config_get_int(red_key);
    gint green = gnome_config_get_int(green_key);
    gint blue = gnome_config_get_int(blue_key);
    GdkColor *color = g_malloc(sizeof(GdkColor));
    
    color->red = red;
    color->green = green;
    color->blue = blue;
    gdk_color_alloc (gtk_widget_get_colormap(GTK_WIDGET (defbox)), color);
    g_free(red_key);
    g_free(green_key);
    g_free(blue_key);
    return color;
}

typedef struct _TypefaceDefault {
    gchar *font_key;
    gchar *color_key;
    gchar *font;
    GdkColor color;
} TypefaceDefault;

TypefaceDefault defaults[NUM_TYPEFACES] = {
    { "fheadword", "cheadword", "-adobe-helvetica-bold-r-normal-*-*-120-*-*-p-*-iso8859-1", { 45568, 8704, 8704 } },
    { "fnumber", "cnumber", "-adobe-courier-bold-r-normal-*-*-130-*-*-m-*-iso8859-1", { 0, 35584, 35584 } },
    { "fpronunciation", "cpronunciation", "-adobe-times-medium-i-normal-*-*-120-*-*-p-*-iso8859-1", { 7168, 34304, 60928 } },
    { "fetymology", "cetymology", "-adobe-times-medium-i-normal-*-*-120-*-*-p-*-iso8859-1", { 15360, 45824, 28928 } },
    { "fpart", "cpart", "-adobe-times-bold-r-normal-*-*-120-*-*-p-*-iso8859-1", { 30464, 34816, 39168 } },
    { "fexample", "cexample", "-adobe-helvetica-medium-o-normal-*-*-120-*-*-p-*-iso8859-1", { 27392, 36352, 8960 } },
    { "fxref", "cxref", "-adobe-helvetica-medium-r-normal-*-*-120-*-*-p-*-iso8859-1", { 20992, 35584, 35584 } },
    { "fbody", "cbody", "-adobe-times-medium-r-normal-*-*-120-*-*-p-*-iso8859-1", { 0, 0, 0 } }
};

/* gdict_pref_load
 *
 * Loads configuration from config file
 */

void 
gdict_pref_load (void) {
    gchar *prefix, *server_key, *port_key;
    gint i;

    /* g_return_if_fail(gnome_app_id != NULL); */

    /* prefix = g_strconcat ("/", gnome_app_id, "/Preferences/", NULL); */
    prefix = g_strdup ("/gdict/Preferences/");
    server_key = g_strconcat("server=", DICT_DEFAULT_SERVER, NULL);
    port_key = g_strdup_printf("port=%d", DICT_DEFAULT_PORT);

    if (gdict_pref.server != NULL)
        g_free(gdict_pref.server);
    
    gnome_config_push_prefix(prefix);
    gdict_pref.server = gnome_config_get_string(server_key);
    gdict_pref.port = gnome_config_get_int(port_key);
    gdict_pref.smart = gnome_config_get_bool("smart=TRUE");
    gdict_pref.database = gnome_config_get_string ("database=!");
    gdict_pref.dfl_strat = gnome_config_get_string ("strategy=lev");

    if (gdict_applet_toggle)
        gdict_pref.applet_handle = gnome_config_get_bool("applet_handle=TRUE");

    for (i = 0; i < NUM_TYPEFACES; i++) {
        g_free(gdict_pref.typefaces[i].font_name);
        g_free(gdict_pref.typefaces[i].color);
        
        gdict_pref.typefaces[i].font_name = 
            config_get_font_name (defaults[i].font_key, defaults[i].font);
        gdict_pref.typefaces[i].font = 
            config_get_font (defaults[i].font_key, defaults[i].font);
        gdict_pref.typefaces[i].color = 
            config_get_color (defaults[i].color_key, defaults[i].color.red,
                              defaults[i].color.green, defaults[i].color.blue);
    }
    
    gnome_config_pop_prefix();
    g_free(prefix);
    g_free(server_key);
    g_free(port_key);
}

static void config_set_font_name (gchar *path, gchar *name)
{
    gnome_config_set_string(path, name);
}

static void 
config_set_color (gchar *path, GdkColor *color) {
    gchar *red_key = g_strconcat(path, "_red", NULL);
    gchar *green_key = g_strconcat(path, "_green", NULL);
    gchar *blue_key = g_strconcat(path, "_blue", NULL);
    gnome_config_set_int(red_key, color->red);
    gnome_config_set_int(green_key, color->green);
    gnome_config_set_int(blue_key, color->blue);
    g_free(red_key);
    g_free(green_key);
    g_free(blue_key);
}

void 
gdict_pref_save (void) {
    gchar *prefix;
    gint i;
    
    /* g_return_if_fail(gnome_app_id != NULL); */

    /* prefix = g_strconcat ("/", gnome_app_id, "/Preferences/", NULL); */
    prefix = g_strdup ("/gdict/Preferences/");
    gnome_config_push_prefix(prefix);

    gnome_config_set_string("server", gdict_pref.server);
    gnome_config_set_int("port", gdict_pref.port);
    gnome_config_set_bool("smart", gdict_pref.smart);
    gnome_config_set_string ("database", gdict_pref.database);
    gnome_config_set_string ("strategy", gdict_pref.dfl_strat);

    if (gdict_applet_toggle)
        gnome_config_set_bool("applet_handle", gdict_pref.applet_handle);

    for (i = 0; i < NUM_TYPEFACES; i++) {
        config_set_font_name(defaults[i].font_key, 
                             gdict_pref.typefaces[i].font_name);
        config_set_color(defaults[i].color_key, 
                         gdict_pref.typefaces[i].color);
    }

    gnome_config_pop_prefix();
    g_free(prefix);

    gnome_config_sync();
}
