/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#include <config.h>
#include <ctype.h>
#include <sys/stat.h>

#include "ggvutils.h"

#include "stock/stock-zoom-in.xpm"
#include "stock/stock-zoom-out.xpm"

#include "stock/toggleall.xpm"
#include "stock/clearall.xpm"
#include "stock/toggleeven.xpm"
#include "stock/toggleodd.xpm"

#include <math.h>

/* Quote filename for system call */
gchar *ggv_quote_filename (const gchar *str)
{
        gchar c, *s, *t, *buf;

        s = (gchar *) str;
        t = buf = g_new (gchar, 2 * strlen (str) + 1);

        while ((c = *s++) != '\0') {
                switch (c) {
                case ' ':
                case '\t':
                case '\n': /* white space */
                case '\'':
                case '"':
                case '\\': /* quoting chars */
                case '|':
                case '&':
                case ';':  /* metacharacters */
                case '(':
                case ')':
                case '<':
                case '>':
                case '!':
                case '{':
                case '}':  /* reserved characters */
                case '*':
                case '[':
                case '?':
                case ']':  /* globbing chars */
                case '^':
                case '$':
                case '`':  /* expansion characters */
                        *t++ = '\\';
                        *t++ = c;
                        break;
                case '#':  /* comment char */
                        if (s == str)
                                *t++ = '\\';
                        *t++ = c;
                        break;
                default:
                        *t++ = c;
                        break;
                }
        }
                *t++ = '\0';
        return buf;
}

/* If file exists and is a regular file then return its length, else -1 */
gint ggv_file_length (const gchar *filename)
{
        struct stat stat_rec;

	if (filename && (stat (filename, &stat_rec) == 0)
	    && S_ISREG (stat_rec.st_mode))
		return stat_rec.st_size;
	else
                return -1;
}

/* Test if file exists, is a regular file and its length is > 0 */
gboolean ggv_file_readable (const char *filename)
{
        return (ggv_file_length (filename) > 0);
}

/* Set a tooltip for a widget */
void ggv_set_tooltip(GtkWidget* w, const gchar* tip)
{
	GtkTooltips* t = gtk_tooltips_new();

	gtk_tooltips_set_tip(t, w, tip, NULL);
}


void ggv_stock_init (void)
{
	static GnomeStockPixmapEntry entries[6];

	entries[0].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[0].data.width = 24;
	entries[0].data.height = 24;
	entries[0].data.xpm_data = stock_zoom_in_xpm;

	entries[1].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[1].data.width = 24;
	entries[1].data.height = 24;
	entries[1].data.xpm_data = stock_zoom_out_xpm;

	entries[2].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[2].data.width = 16;
	entries[2].data.height = 24;
	entries[2].data.xpm_data = toggleall_xpm;

	entries[3].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[3].data.width = 16;
	entries[3].data.height = 24;
	entries[3].data.xpm_data = toggleeven_xpm;

	entries[4].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[4].data.width = 16;
	entries[4].data.height = 24;
	entries[4].data.xpm_data = toggleodd_xpm;

	entries[5].data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	entries[5].data.width = 16;
	entries[5].data.height = 24;
	entries[5].data.xpm_data = clearall_xpm;

	gnome_stock_pixmap_register (STOCK_ZOOM_IN, GNOME_STOCK_PIXMAP_REGULAR, &entries[0]);
	gnome_stock_pixmap_register (STOCK_ZOOM_OUT, GNOME_STOCK_PIXMAP_REGULAR, &entries[1]);
	gnome_stock_pixmap_register (STOCK_TOGGLE_ALL, GNOME_STOCK_PIXMAP_REGULAR, &entries[2]);
	gnome_stock_pixmap_register (STOCK_TOGGLE_EVEN, GNOME_STOCK_PIXMAP_REGULAR, &entries[3]);
	gnome_stock_pixmap_register (STOCK_TOGGLE_ODD, GNOME_STOCK_PIXMAP_REGULAR, &entries[4]);
	gnome_stock_pixmap_register (STOCK_CLEAR_ALL, GNOME_STOCK_PIXMAP_REGULAR, &entries[5]);
}

gfloat ggv_compute_zoom(gint zoom_spec)
{
	return (gfloat)pow(1.2, zoom_spec); /* The Knuth magstep formula rules */
}
