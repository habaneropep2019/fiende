/* GnomeCard - a graphical contact manager.
 *
 * canvas.c: This file is part of GnomeCard.
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <gnome.h>

#include "card.h"
#include "dialog.h"
#include "deladdrlist.h"
#include "phonelist.h"
#include "gnomecard.h"
#include "canvas.h"
#include "dnd.h"
#include "my.h"

#define CARDNAME_FONTSET "-adobe-helvetica-medium-r-normal-*-18-*-*-*-p-*-*-*,-cronyx-helvetica-medium-r-normal-*-20-*-*-*-p-*-koi8-r,-*-*-medium-r-normal-*-18-*-*-*-*-*-ksc5601.1987-0,*"
#define TITLE_FONTSET "-b&h-lucidatypewriter-medium-r-normal-*-*-140-*-*-m-*-iso8859-1,-cronyx-helvetica-medium-r-normal-*-17-*-*-*-p-*-koi8-r,-*-*-medium-r-no-*-*-14-*-*-*-*-*-ksc5601.1987-0,*"
#define COMMENT_FONTSET "-b&h-lucidatypewriter-medium-r-normal-*-*-100-*-*-m-*-iso8859-1,-cronyx-helvetica-medium-r-normal-*-11-*-*-*-p-*-koi8-r,-*-*-medium-r-*-*-10-*-*-*-*-*-ksc5601.1987-0,*"
#define CANVAS_FONTSET "-b&h-lucidatypewriter-medium-r-normal-*-*-120-*-*-m-*-iso8859-1,-cronyx-helvetica-medium-r-normal-*-14-*-*-*-p-*-koi8-r,-*-*-medium-r-*-*-12-*-*-*-*-*-ksc5601.1987-0,*"
#define ADDR_FONTSET "-b&h-lucidatypewriter-medium-r-normal-*-*-100-*-*-m-*-iso8859-1,-cronyx-helvetica-medium-r-normal-*-11-*-*-*-p-*-koi8-r,-*-*-medium-r-*-*-10-*-*-*-*-*-ksc5601.1987-0,*"
#define CANVAS_WIDTH 225
#define CANVAS_HEIGHT 350
#define LIST_SPACING 15.0
#define ADDR_SPACING 11.0

#define HEADER_BOX_COLOR  "khaki"
#define SUBHEADER_BOX_COLOR ""
#define LABEL_BOX_COLOR "dark khaki"

#define HEADER_TEXT_COLOR "black"
#define LABEL_TEXT_COLOR   "black"
#define BODY_TEXT_COLOR    "black"

GtkWidget *gnomecard_canvas;
static GnomeCanvasItem *cardname, *canvastext;
static GnomeCanvasItem *cardname_box, *bg_box;
static GnomeCanvasGroup *root;

static void gnomecard_canvas_text_item_set(GnomeCanvasItem *p, gchar *text);

GtkWidget *
gnomecard_canvas_new(void)
{
	GtkWidget *canvas;
	gdouble x1, x2, y1, y2;
        GdkFont *cardname_font, *title_font, *canvas_font, *addr_font, *comment_font; 

	canvas = gnomecard_canvas = gnome_canvas_new();
	configure_source_card_dnd (canvas);
	gtk_widget_pop_visual();
	gtk_widget_pop_colormap();

	gtk_widget_set_usize (GTK_WIDGET(canvas), 
			      CANVAS_WIDTH, CANVAS_HEIGHT);

	root = GNOME_CANVAS_GROUP (gnome_canvas_root(GNOME_CANVAS(canvas)));

        cardname_font = gdk_fontset_load (CARDNAME_FONTSET);
	title_font = gdk_fontset_load (TITLE_FONTSET);
	canvas_font = gdk_fontset_load (CANVAS_FONTSET);
	addr_font = gdk_fontset_load (ADDR_FONTSET);
	comment_font = gdk_fontset_load (COMMENT_FONTSET);

	bg_box = gnome_canvas_item_new (root, gnome_canvas_rect_get_type (),
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", (double) (CANVAS_WIDTH - 1),
			       "y2", (double) (CANVAS_HEIGHT - 1),
			       "fill_color", "white",
			       "outline_color", "black",
			       "width_pixels", 1,
			       NULL);

	/* label at top of canvas, contains current cardname */
	cardname_box = gnome_canvas_item_new (root, gnome_canvas_rect_get_type (),
			       "x1", 5.0,
			       "y1", 5.0,
			       "x2", (double) (CANVAS_WIDTH - 10),
			       "y2", (double) 35,
			       "fill_color", HEADER_BOX_COLOR,
			       "outline_color", HEADER_BOX_COLOR,
			       "width_pixels", 0,
			       NULL);

	cardname = gnome_canvas_item_new (root, gnome_canvas_text_get_type (),
				      "text", "",
				      "x", 6.0, /*CANVAS_WIDTH / 2.0,*/
				      "y", 12.0,
				      "fontset", CARDNAME_FONTSET,
				      "anchor", GTK_ANCHOR_NORTH_WEST,
				      "fill_color", HEADER_TEXT_COLOR,
				      NULL);

	canvastext = gnome_canvas_item_new (root, gnome_canvas_text_get_type (),
				      "text", "",
				      "x", 5.0,
				      "y", 45.0,
				      "fontset", CANVAS_FONTSET,
				      "anchor", GTK_ANCHOR_NORTH_WEST,
				      "fill_color", BODY_TEXT_COLOR,
				      NULL);
	/* set scrolling region */
	gnome_canvas_item_get_bounds(GNOME_CANVAS_ITEM(root), &x1,&y1,&x2,&y2);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas), x1, y1, x2, y2);

	/* all done */
	return canvas;
}

static void
gnomecard_canvas_text_item_set(GnomeCanvasItem *p, gchar *text)
{
	gnome_canvas_item_set (p, "text", text, NULL);
}

void gnomecard_clear_canvas(void)
{
    gnomecard_canvas_text_item_set(cardname, "");
		gnomecard_canvas_text_item_set(canvastext, "");
}

void
gnomecard_update_canvas(Card *crd) 
{
	gchar *text, *text2;
	gdouble x1, x2, y1, y2;
	gdouble width, height;
	
	g_assert(crd != NULL);
	
	text = card_to_string (crd);
	text2 = strchr (text, '\n');
	if (! text2)
	  text2 = text;
	else
	  text2++;
	gnomecard_canvas_text_item_set(canvastext, text2);
	
	/* card name */
	gnome_canvas_item_get_bounds(GNOME_CANVAS_ITEM(canvastext), 
				     NULL, &y1, &x2, &y2);
	{
		GdkFont *font;
		char *str;
		
		str = (crd->fname.str)? crd->fname.str : _("No Card Name.");
		
		font = gdk_fontset_load(CARDNAME_FONTSET);
		if (!font) {
			font = gdk_font_load ("fixed");
			g_assert (font != NULL);
		}

		width = gdk_string_width(font, str);
		gdk_font_unref(font);
		
		width = MAX (CANVAS_WIDTH - 5.0, width);
		width = MAX (x2, width);
		gnomecard_canvas_text_item_set(cardname, str);
		gnome_canvas_item_set(cardname_box, "x2", width, NULL);
	}
	
	width = width + 4.0;
	height = MAX (CANVAS_HEIGHT - 1, y2 - y1 + 45);
	gnome_canvas_item_set(bg_box, 
			      "x2", width, 
			      "y2", height, 
			      NULL);
	
	
	/* set scrolling region */
	gnome_canvas_item_get_bounds(GNOME_CANVAS_ITEM(root), &x1,&y1,&x2,&y2);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(gnomecard_canvas),
				       x1, y1, x2, y2);
}
