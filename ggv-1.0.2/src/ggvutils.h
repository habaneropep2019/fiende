/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#ifndef __GGV_MISC_H_H__
#define __GGV_MISC_H__

#include <gnome.h>
#include <math.h>

/* Quote filename for system call */
gchar *ggv_quote_filename (const gchar *str);

/* If file exists and is a regular file then return its length, else -1 */
gint ggv_file_length (const gchar *filename);

/* Test if file exists, is a regular file and its length is > 0 */
gboolean ggv_file_readable (const char *filename);

/* Set a tooltip for a widget */
void ggv_set_tooltip(GtkWidget* w, const gchar* tip);

#define STOCK_ZOOM_IN	"GGV_stock_zoom_in"
#define STOCK_ZOOM_OUT	"GGV_stock_zoom_out"
#define STOCK_TOGGLE_ALL "GGV_toggle_all"
#define STOCK_TOGGLE_EVEN "GGV_toggle_even"
#define STOCK_TOGGLE_ODD "GGV_toggle_odd"
#define STOCK_CLEAR_ALL "GGV_clear_all"

void ggv_stock_init (void);

gfloat ggv_compute_zoom(gint zoom_spec);

#endif
