/* -*- Mode: C; tab-width: 5; indent-tabs-mode: t; c-basic-offset: 5 -*- */
/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
 *           Miguel de Icaza (miguel@gnu.org)
 *
 *  A system print preview window.  Based on print-preview.c
 *  from gnumeric.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * gnome-print-master.c: Master interface for printing
 * Based on print.c and print-preview.c from gnumeric.
 *
 * Authors:
 *    Miguel de Icaza (miguel@gnu.org)
 *    Michael Zucchi <zucchi#helixcode.com>
 *
 * Given the large memory usage of an entire sheet on
 * a canvas, we have now taken a new approach: we keep in
 * a GNOME Print Metafile each page.  And we render this
 * metafile into the printing context on page switch.
 */
/* Must include these two first */
#include "config.h"
#include <libgnomeprint/gnome-print-i18n.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtkaccelgroup.h>
#include <gtk/gtkmenushell.h>
#include <libgnomeui/gnome-app.h>
#include <libgnomeui/gnome-app-helper.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-canvas.h>
#include <libgnomeui/gnome-canvas-rect-ellipse.h>
#include <libgnomeui/gnome-stock.h>

#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-printer-dialog.h>

#include <libgnomeprint/gnome-print-master-private.h>
#include "gnome-print-master-preview.h"
#include "gnome-print-preview-icons.h"

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#define TOOLBAR_BUTTON_BASE 5
#define MOVE_INDEX          5

typedef enum {
	MODE_MOVE,
	MODE_MOVE_DRAGGING,
	MODE_ZOOM_IN,
	MODE_ZOOM_OUT
} PreviewMode;


struct _GnomePrintMasterPreviewPrivate {
	GnomePrintMaster *master; /* what drives us */

	/*
	 * Preview canvas
	 */
	GtkWidget            *scrolled_window;
	GnomeCanvas	     *canvas;

	/*
	 * status display
	 */
	GtkWidget	    *page_entry;
	GtkWidget	    *last;

	int                  current_page;
	int                  pagecount;

	/* the output print context, a gnomeprintpreview */
	GnomePrintPreview *output;

	int		     first_page_rendered;

	GnomeUIInfo          *toolbar;

	PreviewMode          mode;

	/*
	 * Used for dragging the sheet
	 */
	int                  drag_anchor_x, drag_anchor_y;
	int                  drag_ofs_x, drag_ofs_y;

	gboolean             landscape;
	/* Dimensions of the paper.  In landscape mode, width is probably
	   bigger than height since landscape is taken into account.  */
	gdouble              width, height;
};

#define Private GnomePrintMasterPreviewPrivate

/*
 * Padding in points around the simulated page
 */
#define PAGE_PAD 4

static void
render_page (GnomePrintMasterPreview *pmp, int page)
{
	const GnomePaper *paper;
	const char *paper_name;
	Private *pp = pmp->priv;

	gtk_object_unref (GTK_OBJECT (pp->output));
	pp->output = NULL;

	/*
	 * Create the preview printing context
	 */
	paper = pp->master->paper;
	paper_name = gnome_paper_name (paper);
	pp->output = (GnomePrintPreview *) gnome_print_preview_new (pp->canvas, paper_name ? paper_name : "A4");

	/*
	 * Reset scrolling region always
	 */
	gnome_canvas_set_scroll_region (
		pp->canvas,
		0 - PAGE_PAD,
		0 - PAGE_PAD,
		pp->width + PAGE_PAD,
		pp->height + PAGE_PAD);

	if (pp->landscape) {
		double affine [6];

		art_affine_translate (affine, 0, pp->width);
		gnome_print_concat (GNOME_PRINT_CONTEXT (pp->output), affine);

		art_affine_rotate (affine, -90.0);
		gnome_print_concat (GNOME_PRINT_CONTEXT (pp->output), affine);
	}

	gnome_print_meta_render_from_object_page (
		GNOME_PRINT_CONTEXT(pp->output),
		GNOME_PRINT_META (gnome_print_master_get_context(pp->master)),
		page);
}

static void
goto_page (GnomePrintMasterPreview *pmp, int page)
{
	char text [4 * sizeof (int)];
	Private *pp = pmp->priv;

	sprintf (text, "%d", page + 1);
	gtk_entry_set_text (GTK_ENTRY (pp->page_entry), text);
	
	if (page == pp->current_page)
		return;

	pp->current_page = page;

	render_page (pmp, page);
}

static void
change_page_cmd (GtkEntry *entry, GnomePrintMasterPreview *pmp)
{
	Private *pp = pmp->priv;
	char *text = gtk_entry_get_text (entry);
	int p;

	p = atoi (text);
	p--;
	if (p < 0){
		goto_page (pmp, 0);
		return;
	}
	if (p > pp->pagecount - 1){
		goto_page (pmp, pp->pagecount-1);
		return;
	}
	goto_page (pmp, p);
}

static void
do_zoom (GnomePrintMasterPreview *pmp, int factor)
{
	double zoom;
	Private *pp = pmp->priv;
	
	if (factor > 0) {
		zoom = pp->canvas->pixels_per_unit * M_SQRT2;
	} else if (factor < 0) {
		zoom = pp->canvas->pixels_per_unit / M_SQRT2;
	} else {
		zoom = 1.0;
	}

	zoom = CLAMP (zoom, 0.0625, 16.0);

	gnome_canvas_set_pixels_per_unit (pp->canvas, zoom);
}

/* Button press handler for the print preview canvas */
static gint
preview_canvas_button_press (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GnomePrintMasterPreview *pmp = data;
	Private *pp = pmp->priv;
	int retval;

	retval = FALSE;

	switch (pp->mode) {
	case MODE_MOVE:
		if (event->button != 1)
			break;

		pp->mode = MODE_MOVE_DRAGGING;
		pp->drag_anchor_x = event->x;
		pp->drag_anchor_y = event->y;
		gnome_canvas_get_scroll_offsets (GNOME_CANVAS (widget),
						 &pp->drag_ofs_x,
						 &pp->drag_ofs_y);

		gdk_pointer_grab (widget->window, FALSE,
				  (GDK_POINTER_MOTION_MASK
				   | GDK_POINTER_MOTION_HINT_MASK
				   | GDK_BUTTON_RELEASE_MASK),
				  NULL,
				  NULL, /*cursor_get (GNUMERIC_CURSOR_HAND_CLOSED),*/
				  event->time);

		retval = TRUE;
		break;

	default:
		break;
	}

	return retval;
}

/* Drags the print preview canvas to the specified position */
static void
drag_to (GnomePrintMasterPreview *pmp, int x, int y)
{
	Private *pp = pmp->priv;
	int dx, dy;

	dx = pp->drag_anchor_x - x;
	dy = pp->drag_anchor_y - y;

	/* Right now this will suck for diagonal movement.  GtkLayout does not
	 * have a way to scroll itself diagonally, i.e. you have to change the
	 * vertical and horizontal adjustments independently, leading to ugly
	 * visual results.  The canvas freezes and thaws the layout in case of
	 * diagonal movement, forcing it to repaint everything.
	 *
	 * This will be resolved in the canvas.
	 */
	gnome_canvas_scroll_to (pp->canvas, pp->drag_ofs_x + dx, pp->drag_ofs_y + dy);
}

/* Button release handler for the print preview canvas */
static gint
preview_canvas_button_release (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GnomePrintMasterPreview *pmp = data;
	Private *pp = pmp->priv;
	int retval;

	retval = FALSE;

	switch (pp->mode) {
	case MODE_MOVE_DRAGGING:
		if (event->button != 1)
			break;

		drag_to (pmp, event->x, event->y);
		pp->mode = MODE_MOVE;

		gdk_pointer_ungrab (event->time);
		retval = TRUE;
		break;

	default:
		break;
	}

	return retval;
}

/* Motion notify handler for the print preview canvas */
static gint
preview_canvas_motion (GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	GnomePrintMasterPreview *pmp = data;
	Private *pp = pmp->priv;
	int retval;
	gint x, y;
	GdkModifierType mods;

	retval = FALSE;

	switch (pp->mode) {
	case MODE_MOVE_DRAGGING:
		if (event->is_hint)
			gdk_window_get_pointer (widget->window, &x, &y, &mods);
		else {
			x = event->x;
			y = event->y;
		}

		drag_to (pmp, x, y);
		retval = TRUE;
		break;

	default:
		break;
	}

	return retval;
}


static void
preview_close_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	gtk_object_destroy (GTK_OBJECT (pmp));
}

static void
preview_file_print_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	Private *pp = pmp->priv;
	
	gnome_print_master_print (pp->master);
	/* should we clean ourselves up now? */
}

static void
preview_first_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	goto_page (pmp, 0);
}

static void
preview_next_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	Private *pp = pmp->priv;
	int current_page = pp->current_page;

	if (current_page+2 > pp->pagecount)
		return;
	goto_page (pmp, current_page+1);
}

static void
preview_prev_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	Private *pp = pmp->priv;
	int current_page = pp->current_page;

	if (current_page < 1)
		return;
	goto_page (pmp, current_page-1);
}

static void
preview_last_page_cmd (void *unused, GnomePrintMasterPreview *pmp)
{
	Private *pp = pmp->priv;
	
	goto_page (pmp, pp->pagecount-1);
}

static void
preview_zoom_in_cmd (GtkToggleButton *t, GnomePrintMasterPreview *pmp)
{
	do_zoom (pmp, 1);
}

static void
preview_zoom_out_cmd (GtkToggleButton *t, GnomePrintMasterPreview *pmp)
{
	do_zoom (pmp, -1);
}

static void
preview_zoom_fit_cmd (GtkToggleButton *t, GnomePrintMasterPreview *pmp)
{
	double zoomx, zoomy;
	Private *pp = pmp->priv;
	int width = GTK_WIDGET (pp->canvas)->allocation.width;
	int height = GTK_WIDGET (pp->canvas)->allocation.height;
	
	zoomx = width / (pp->width + 5.0 + PAGE_PAD);
	zoomy = height / (pp->height + 5.0 + PAGE_PAD);
	
	if (zoomy < zoomx)
		zoomx = zoomy;
	gnome_canvas_set_pixels_per_unit (pp->canvas, zoomx);
}

static void
preview_zoom_fit_wide_cmd (GtkToggleButton *t, GnomePrintMasterPreview *pmp)
{
	double zoom;
	Private *pp = pmp->priv;
	int width = GTK_WIDGET (pp->canvas)->allocation.width;

	/* include paper decoration */
	zoom = width / (pp->width + 5.0 + PAGE_PAD);
	gnome_canvas_set_pixels_per_unit (pp->canvas, zoom);
}

static void
preview_zoom_fit_tall_cmd (GtkToggleButton *t, GnomePrintMasterPreview *pmp)
{
	double zoom;
	Private *pp = pmp->priv;
	int height = GTK_WIDGET (pp->canvas)->allocation.height;
	
	/* include paper decoration */
	zoom = height / (pp->height + 5.0 + PAGE_PAD);
	gnome_canvas_set_pixels_per_unit (pp->canvas, zoom);
}

static gint
preview_canvas_key (GtkWidget *widget, GdkEventKey *event, void *data)
{
	GnomePrintMasterPreview *pmp = data;
	Private *pp = pmp->priv;
	int x,y;
	int height, width;
	int domove=0;

	gnome_canvas_get_scroll_offsets (pp->canvas, &x, &y);
	height = GTK_WIDGET (pp->canvas)->allocation.height;
	width = GTK_WIDGET (pp->canvas)->allocation.width;

	switch (event->keyval) {
	case '1':
		preview_zoom_fit_cmd (0, pmp);
		break;
	case '2':		/* as good a key as any? */
		preview_zoom_fit_wide_cmd (0, pmp);
		break;
	case '3':
		preview_zoom_fit_tall_cmd (0, pmp);
		break;
	case '+':
	case '=':
	case GDK_KP_Add:
		do_zoom (pmp, 1);
		break;
	case '-':
	case '_':
	case GDK_KP_Subtract:
		do_zoom (pmp, -1);
		break;
	case GDK_KP_Right:
	case GDK_Right:
		if (event->state & GDK_SHIFT_MASK)
			x+=width;
		else
			x+=10;
		domove=1;
		break;
	case GDK_KP_Left:
	case GDK_Left:
		if (event->state & GDK_SHIFT_MASK)
			x-=width;
		else
			x-=10;
		domove=1;
		break;
	case GDK_KP_Up:
	case GDK_Up:
		if (event->state & GDK_SHIFT_MASK)
			goto page_up;
		y-=10;
		domove=1;
		break;
	case GDK_KP_Down:
	case GDK_Down:
		if (event->state & GDK_SHIFT_MASK)
			goto page_down;
		y+=10;
		domove=1;
		break;
	case GDK_KP_Page_Up:
	case GDK_Page_Up:
	case GDK_Delete:
	case GDK_KP_Delete:
	case GDK_BackSpace:
	page_up:
		if (y<=0) {
			if (pp->current_page>0) {
				goto_page (pmp, pp->current_page-1);
				y = GTK_LAYOUT (pp->canvas)->height-height;
			}
		} else {
			y-=height;
		}
		domove=1;
		break;
	case GDK_KP_Page_Down:
	case GDK_Page_Down:
	case ' ':
	page_down:
		if (y>=GTK_LAYOUT (pp->canvas)->height-height) {
			if (pp->current_page<pp->pagecount-1) {
				goto_page (pmp, pp->current_page+1);
				y=0;
			}
		} else {
			y+=height;
		}
		domove=1;
		break;
	case GDK_KP_Home:
	case GDK_Home:
		goto_page (pmp, 0);
		y=0;
		domove=1;
		break;
	case GDK_KP_End:
	case GDK_End:
		goto_page (pmp, pp->pagecount-1);
		y=0;
		domove=1;
		break;
	case GDK_Escape:
		gtk_object_destroy (GTK_OBJECT (pmp));
		return TRUE;
	default:
		return FALSE;
	}

	if (domove)
		gnome_canvas_scroll_to (pp->canvas, x, y);

	gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");
	return TRUE;
}

static void
create_preview_canvas (GnomePrintMasterPreview *pmp)
{
	GtkWidget *box, *status;
	Private *pp = pmp->priv;
	const GnomePaper *paper;
	const char *paper_name;
	GnomeCanvasItem *i;

	gtk_widget_push_colormap (gdk_rgb_get_cmap ());
	gtk_widget_push_visual (gdk_rgb_get_visual ());

	pp->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	pp->canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
	gnome_canvas_set_pixels_per_unit (pp->canvas, 1.0);

	gtk_signal_connect (GTK_OBJECT (pp->canvas), "button_press_event",
			    GTK_SIGNAL_FUNC (preview_canvas_button_press),
			    pmp);
	gtk_signal_connect (GTK_OBJECT (pp->canvas), "button_release_event",
			    GTK_SIGNAL_FUNC (preview_canvas_button_release),
			    pmp);
	gtk_signal_connect (GTK_OBJECT (pp->canvas), "motion_notify_event",
			    GTK_SIGNAL_FUNC (preview_canvas_motion),
			    pmp);
	gtk_signal_connect (GTK_OBJECT (pp->canvas), "key_press_event",
			    GTK_SIGNAL_FUNC (preview_canvas_key),
			    pmp);

	gtk_container_add (GTK_CONTAINER (pp->scrolled_window), GTK_WIDGET (pp->canvas));

	/*
	 * Create the preview printing context
	 */
	paper = pp->master->paper;
	paper_name = gnome_paper_name (paper);
	pp->output = (GnomePrintPreview *) gnome_print_preview_new (pp->canvas, paper_name ? paper_name : "A4");

	/*
	 * Now add some padding above and below and put a simulated
	 * page on the background
	 */
	i = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (gnome_canvas_root (pp->canvas)),
		gnome_canvas_rect_get_type (),
		"x1",   	 0.0,
		"y1",   	 0.0,
		"x2",   	 (double) pp->width,
		"y2",   	 (double) pp->height,
		"fill_color",    "white",
		"outline_color", "black",
		"width_pixels",  1,
		NULL);
	gnome_canvas_item_lower_to_bottom (i);
	i = gnome_canvas_item_new (
		GNOME_CANVAS_GROUP (gnome_canvas_root (pp->canvas)),
		gnome_canvas_rect_get_type (),
		"x1",   	 3.0,
		"y1",   	 3.0,
		"x2",   	 (double) pp->width + 3,
		"y2",   	 (double) pp->height + 3,
		"fill_color",    "black",
		NULL);
	gnome_canvas_item_lower_to_bottom (i);
	gnome_canvas_set_scroll_region (
		pp->canvas,
		0 - PAGE_PAD,
		0 - PAGE_PAD,
		pp->width + PAGE_PAD,
		pp->height + PAGE_PAD);

	box = gtk_vbox_new (FALSE, 0);
	status = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (status), gtk_label_new (_ ("Page: ")), FALSE, FALSE, 0);
	pp->page_entry = gtk_entry_new ();
	gtk_widget_set_usize (pp->page_entry, 40, 0);
	gtk_signal_connect (GTK_OBJECT (pp->page_entry), "activate", change_page_cmd, pmp);
	gtk_box_pack_start (GTK_BOX (status), pp->page_entry, FALSE, FALSE, 0);
	pp->last = gtk_label_new ("");
	gtk_box_pack_start (GTK_BOX (status), pp->last, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (box), status, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (box), pp->scrolled_window, TRUE, TRUE, 0);
	gnome_app_set_contents (GNOME_APP (pmp), box);
	gtk_widget_show_all (box);

	gtk_widget_grab_focus (GTK_WIDGET (pp->canvas));

	return;
}


static GnomeUIInfo preview_file_menu [] = {
	GNOMEUIINFO_MENU_PRINT_ITEM (preview_file_print_cmd, NULL),
	GNOMEUIINFO_MENU_CLOSE_ITEM (preview_close_cmd, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo preview_view_menu [] = {
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("_First page"), N_ ("Shows the first page"),
		preview_first_page_cmd, GNOME_STOCK_PIXMAP_FIRST),
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("_Previous page"), N_ ("Shows the previous page"),
		preview_prev_page_cmd, GNOME_STOCK_PIXMAP_BACK),
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("_Next page"), N_ ("Shows the next page"),
		preview_next_page_cmd, GNOME_STOCK_PIXMAP_FORWARD),
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("_Last page"), N_ ("Shows the last page"),
		preview_last_page_cmd, GNOME_STOCK_PIXMAP_LAST),

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, N_ ("Zoom _in"), N_ ("Zooms in"), preview_zoom_in_cmd },
	{ GNOME_APP_UI_ITEM, N_ ("Zoom _out"), N_ ("Zooms out"), preview_zoom_out_cmd },

	GNOMEUIINFO_END
};

static GnomeUIInfo top_menu [] = {
	GNOMEUIINFO_MENU_FILE_TREE (preview_file_menu),
	{ GNOME_APP_UI_SUBTREE, N_ ("_View"), NULL, preview_view_menu },
	GNOMEUIINFO_END
};

static GnomeUIInfo toolbar [] = {
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("Print"), N_ ("Prints the current file"),
		preview_file_print_cmd, GNOME_STOCK_PIXMAP_PRINT),
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("First"), N_ ("Shows the first page"),
		preview_first_page_cmd, GNOME_STOCK_PIXMAP_FIRST),
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("Back"), N_ ("Shows the previous page"),
		preview_prev_page_cmd, GNOME_STOCK_PIXMAP_BACK),
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("Next"), N_ ("Shows the next page"),
		preview_next_page_cmd, GNOME_STOCK_PIXMAP_FORWARD),
	GNOMEUIINFO_ITEM_STOCK (
		N_ ("Last"), N_ ("Shows the last page"),
		preview_last_page_cmd, GNOME_STOCK_PIXMAP_LAST),

	GNOMEUIINFO_ITEM (N_ ("Zoom in"), N_ ("Zooms the page in"), preview_zoom_in_cmd, stock_zoom_in_xpm),
	GNOMEUIINFO_ITEM (N_ ("Zoom out"), N_ ("Zooms the page out"), preview_zoom_out_cmd, stock_zoom_out_xpm),
	GNOMEUIINFO_ITEM (N_ ("Fit"), N_ ("Zooms to fit the whole page"), preview_zoom_fit_cmd, stock_zoom_fit_xpm),
	GNOMEUIINFO_ITEM (N_ ("Fit Wide"), N_ ("Zooms to fit the width of the page"), preview_zoom_fit_wide_cmd, stock_zoom_fit_wide_xpm),
	GNOMEUIINFO_ITEM (N_ ("Fit Tall"), N_ ("Zooms to fit the height of the page"), preview_zoom_fit_tall_cmd, stock_zoom_fit_tall_xpm),
	GNOMEUIINFO_END
};

#define CM2PT(v) ((v) * 72 / 2.54)

static void
create_toplevel (GnomePrintMasterPreview *pmp)
{
	char *old_msg_locale;
	gint width, height;
	const GnomePaper *paper;
	Private *pp;

	g_return_if_fail (pmp != NULL);

	pp = pmp->priv;

	paper  = pp->master->paper;
	if (pp->landscape) {
		pp->height = paper ? gnome_paper_pswidth (paper) : CM2PT (21);
		pp->width  = paper ? gnome_paper_psheight (paper) : CM2PT (29.7);
	} else {
		pp->width  = paper ? gnome_paper_pswidth (paper) : CM2PT (21);
		pp->height = paper ? gnome_paper_psheight (paper) : CM2PT (29.7);
	}

	width  = pp->width + PAGE_PAD * 3;
	height = pp->height + PAGE_PAD * 3;

	if (width > gdk_screen_width () - 40)
		width = gdk_screen_width () - 40;

	if (height > gdk_screen_height () - 100)
		height = gdk_screen_height () - 100;

	gtk_widget_set_usize (GTK_WIDGET (pmp), width, height);
	gtk_window_set_policy (GTK_WINDOW (pmp), TRUE, TRUE, FALSE);

	old_msg_locale = g_strdup (textdomain (NULL));
	textdomain (PACKAGE);
	gnome_app_create_menus_with_data (GNOME_APP (pmp), top_menu, pmp);

	pp->toolbar = g_malloc (sizeof (toolbar));
	memcpy (pp->toolbar, toolbar, sizeof (toolbar));

	gnome_app_create_toolbar_with_data (GNOME_APP (pmp), pp->toolbar, pmp);
	textdomain (old_msg_locale);
	g_free (old_msg_locale);
}

/*
  The GnomePrintMasterPreview object
*/

static void gnome_print_master_preview_class_init (GnomePrintMasterPreviewClass *class);
static void gnome_print_master_preview_init       (GnomePrintMasterPreview      *gspaper);

static GnomeAppClass *parent_class;

guint
gnome_print_master_preview_get_type (void)
{
	static guint print_master_preview_type = 0;
	
	if (!print_master_preview_type) {
		GtkTypeInfo print_master_preview_info = {
			"GnomePrintMasterPreview",
			sizeof (GnomePrintMasterPreview),
			sizeof (GnomePrintMasterPreviewClass),
			 (GtkClassInitFunc) gnome_print_master_preview_class_init,
			 (GtkObjectInitFunc) gnome_print_master_preview_init,
			 (GtkArgSetFunc) NULL,
			 (GtkArgGetFunc) NULL
		};
		
		print_master_preview_type = gtk_type_unique (gnome_app_get_type (), &print_master_preview_info);
	}
	
	return print_master_preview_type;
}

static void
gnome_print_master_preview_finalize (GtkObject *object)
{
	GnomePrintMasterPreview *pmp = GNOME_PRINT_MASTER_PREVIEW (object);
	Private *pp = pmp->priv;

	if (pp->output != NULL)
		gtk_object_unref (GTK_OBJECT (pp->output));
	g_free (pp->toolbar);

	if (pp->master != NULL)
		gtk_object_unref (GTK_OBJECT (pp->master));

	g_free (pp);
	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnome_print_master_preview_class_init (GnomePrintMasterPreviewClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) class;
	parent_class = gtk_type_class (gnome_app_get_type ());

	object_class->finalize = gnome_print_master_preview_finalize;
}

static void
gnome_print_master_preview_init (GnomePrintMasterPreview *pmp)
{
	Private *pp;

	pp = pmp->priv = g_malloc0 (sizeof (*pp));
	pp->current_page = -1;
}

/**
 * gnome_print_master_preview_new:
 * @gpm: A GnomePrintMaster which has been printed to and completed.
 * @title: Window title for the preview window.
 * 
 * Create a new preview window widget.  The preview has inbuilt
 * buttons for zooming and panning, and menu's to print.
 * 
 * Return value: A newly created GnomePrintMasterPreview widget.
 **/
GnomePrintMasterPreview *
gnome_print_master_preview_new (GnomePrintMaster *gpm,
				const char *title)
{
	return gnome_print_master_preview_new_with_orientation
		 (gpm, title, FALSE);
}

/**
 * gnome_print_master_preview_new_with_orientation:
 * @gpm: A GnomePrintMaster which has been printed to and completed.
 * @title: Window title for the preview window.
 * @landscape: TRUE to rotate paper.
 * 
 * Create a new preview window widget.  The preview has inbuilt
 * buttons for zooming and panning, and menu's to print.
 * 
 * Return value: A newly created GnomePrintMasterPreview widget.
 **/
GnomePrintMasterPreview *
gnome_print_master_preview_new_with_orientation (GnomePrintMaster *gpm,
						 const char *title,
						 gboolean landscape)
{
	GnomePrintMasterPreview *pmp;
	Private *pp;
	char text[4 * sizeof (int)];

	pmp = GNOME_PRINT_MASTER_PREVIEW (gtk_type_new (gnome_print_master_preview_get_type ()));

	gnome_app_construct (GNOME_APP (pmp), "preview_window", title);

	pp = pmp->priv;
	pp->landscape = landscape;

	pp->master = gpm;
	gtk_object_ref (GTK_OBJECT (gpm));

	create_toplevel (pmp);
	create_preview_canvas (pmp);

	/* this zooms to fit, once we know how big the window actually is */
	gtk_signal_connect (GTK_OBJECT (pp->canvas), "realize",
			   GTK_SIGNAL_FUNC (preview_zoom_fit_cmd), pmp);

	pp->pagecount = gnome_print_master_get_pages (gpm);

	goto_page (pmp, 0);
	
	sprintf (text, "/%d", pp->pagecount);
	gtk_label_set_text (GTK_LABEL (pp->last), text);

	return pmp;
}

