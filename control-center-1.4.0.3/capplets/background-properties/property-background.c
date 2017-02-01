/* -*- mode: C; c-file-style: "linux" -*- */

/*
 * Background display property module.
 * (C) 1997 the Free Software Foundation
 * (C) 2000 Helix Code, Inc.
 * (C) 1999, 2000 Red Hat, Inc.
 *
 * Authors: Miguel de Icaza.
 *          Federico Mena.
 *          Radek Doulik
 *          Michael Fulbright
 *          Justin Maurer
 *          Owen Taylor
 */

#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include <gdk/gdkx.h>

#include "capplet-widget.h"
#include <gnome.h>

#include "imlib-misc.h"
#include "render-background.h"

#define MONITOR_CONTENTS_X 20
#define MONITOR_CONTENTS_Y 10
#define MONITOR_CONTENTS_WIDTH 157
#define MONITOR_CONTENTS_HEIGHT 111

static GtkWidget *capplet=NULL;

static GtkWidget *monitor;
static GtkWidget *menuitem1, *menuitem2, *menuitem3;

static GtkWidget *fileSel = NULL;
static GtkWidget *wpMenu;
static GtkWidget *wpOMenu;
static gchar *wpFileSelName = NULL;
static gint wpNum;
static gint fillPreview = TRUE;
static gint ignoreChanges = TRUE;

enum {
	TARGET_URI_LIST
};

BGState origState, curState;

/* The pointers to the color selectors */
static GtkWidget *cp1, *cp2, *cl2;

void background_init(void);
void background_properties_init(void);
void background_read(BGState *state);
void background_setup(BGState *state);
static void background_write (BGState *state);
void printState(BGState *state);
void copyState(BGState *dest, BGState *src);
void background_ok(GtkWidget *widget, BGState *state);

void
printState( BGState *state )
{
	printf("\n-------------------------------\n");
	printf("Background is : %s\n", (state->enabled) ? "Enabled" : "Disabled");
	printf("Color 1: #%04x%04x%04x\n",
	       state->bgColor1.red >> 8,
	       state->bgColor1.green >> 8,
	       state->bgColor1.blue >> 8);
	printf("Color 2: #%04x%04x%04x\n",
	       state->bgColor2.red >> 8,
	       state->bgColor2.green >> 8,
	       state->bgColor2.blue >> 8);
	printf("Mode   : %s\n", (state->grad) ? "gradient" : "solid");
	printf("Direct : %s\n", (state->vertical) ? "vertical" : "horizontal");
	printf("bgType : %s\n", (state->bgType == BACKGROUND_SIMPLE) ? "none" : state->wpFileName);
	printf("wpType : %d\n", state->wpType);
	printf("\n-------------------------------\n");
}

void
copyState(BGState *dest, BGState *src)
{
	memcpy(dest, src, sizeof(BGState));
	if (src->wpFileName)
		dest->wpFileName = g_strdup(src->wpFileName);
	if (src->wpFileSelName)
		dest->wpFileSelName = g_strdup(src->wpFileSelName);
}
    

static void
fill_monitor (int prop_changed, BGState *state)
{
	GdkCursor *cursor;
	gint output_width, output_height;
	GdkPixbuf *bg_pixbuf;

	if (prop_changed)
		capplet_widget_state_changed(CAPPLET_WIDGET(capplet), TRUE);
    
	if (!state->enabled) {
		/* set bg black, put in text "Disabled" */
		if (fillPreview) {
			int cx, cy, cw, ch;
			GdkColor color;
			GdkGC *gc;
			GdkFont  *font;
			GdkPixmap *screen;
			int     height, width;
			const char *disabled_string = _("Disabled");

			screen = GTK_PIXMAP (GTK_BIN (monitor)->child)->pixmap;
			cw = MONITOR_CONTENTS_WIDTH;
			ch = MONITOR_CONTENTS_HEIGHT;
			cx = MONITOR_CONTENTS_X;
			cy = MONITOR_CONTENTS_Y;
			if (!GTK_WIDGET_REALIZED (monitor))
				gtk_widget_realize (monitor);
			gc = gdk_gc_new (monitor->window);

			gdk_color_black(gtk_widget_get_colormap(GTK_BIN(monitor)->child),
					&color);
			gdk_gc_set_foreground(gc, &color);
			gdk_draw_rectangle (screen, gc, TRUE, cx, cy, cw, ch);

			font = GTK_BIN(monitor)->child->style->font;
			width = gdk_string_width ( font, disabled_string);
			height = gdk_string_height ( font, disabled_string);

			gdk_color_white(gtk_widget_get_colormap(GTK_BIN(monitor)->child),
					&color);
			gdk_gc_set_foreground(gc, &color);
			gdk_draw_string (screen, font, gc, 
					 cx + (cw-width)/2, cy + (ch-height)/2 + height/2,
					 disabled_string);

			gdk_gc_unref (gc);
			gtk_widget_queue_draw (monitor);
		}

		return;
	}

	if (fillPreview) {
		output_width = MONITOR_CONTENTS_WIDTH;
		output_height = MONITOR_CONTENTS_HEIGHT;
	} else {
		output_width = gdk_screen_width ();
		output_height = gdk_screen_height ();
		
		if (!state->grad && state->bgType == BACKGROUND_SIMPLE) {
			GdkColor tmp;

			set_root_pixmap (NULL);

			tmp.pixel = xpixel_from_color (&state->bgColor1);
			
			gdk_window_set_background (GDK_ROOT_PARENT(), &tmp);
			gdk_window_clear (GDK_ROOT_PARENT());

			return;
		}
	}

	
	/* Set the cursor to a nice watch.  The main_window may not exist
	 * if we are called from the session manager, though
	 */
    
	if (capplet && capplet->window) {
		cursor = gdk_cursor_new (GDK_WATCH);
		gdk_window_set_cursor (capplet->window, cursor);
		gdk_cursor_destroy (cursor);
	}

	/* The last argument, can_tile, needs to be false if there is no image.
	 * This was never a proplem previously because the solid-color/no-image
	 * case was handled specially (and incorrectly) above.
	 */
	bg_pixbuf = make_background (state, output_width, output_height, !fillPreview && state->bgType != BACKGROUND_SIMPLE);


	if (!bg_pixbuf) {
		/* Loading the image failed */
		
		state->bgType = BACKGROUND_SIMPLE;
		gtk_option_menu_set_history (GTK_OPTION_MENU (wpOMenu), 0);

		fill_monitor (FALSE, state);
	} else {
		if (fillPreview) {
			GtkWidget *pixmap_widget = GTK_BIN(monitor)->child;
			
			render_to_drawable (bg_pixbuf, GTK_PIXMAP (pixmap_widget)->pixmap,
					    pixmap_widget->style->white_gc,
					    0, 0,
					    MONITOR_CONTENTS_X, MONITOR_CONTENTS_Y,
					    MONITOR_CONTENTS_WIDTH, MONITOR_CONTENTS_HEIGHT,
					    GDK_RGB_DITHER_MAX, 0, 0);
			gtk_widget_queue_draw (monitor);
			
		} else {

			GdkPixmap *bg_pixmap = make_root_pixmap (gdk_pixbuf_get_width (bg_pixbuf),
								 gdk_pixbuf_get_height (bg_pixbuf));
			GdkGC *gc = gdk_gc_new (bg_pixmap);

			render_to_drawable (bg_pixbuf, bg_pixmap, gc, 0, 0, 0, 0, 
					    gdk_pixbuf_get_width (bg_pixbuf), gdk_pixbuf_get_height (bg_pixbuf),
					    GDK_RGB_DITHER_MAX, 0, 0);
			set_root_pixmap (bg_pixmap);
			dispose_root_pixmap (bg_pixmap);
		}
    
		gdk_pixbuf_unref (bg_pixbuf);
	}

	/* Reset the cursor to normal */
    
	if (capplet && capplet->window)
		gdk_window_set_cursor (capplet->window, NULL);
}

static void
color_sel_set (GnomeColorPicker *gcp, gpointer data)
{
	gushort short_r, short_g, short_b, short_a;

	if (!ignoreChanges) {
		gnome_color_picker_get_i16(GNOME_COLOR_PICKER(cp1), 
					   &short_r, &short_g, &short_b, &short_a);
		
		curState.bgColor1.red = short_r;
		curState.bgColor1.green = short_g;
		curState.bgColor1.blue = short_b;
		
		gnome_color_picker_get_i16(GNOME_COLOR_PICKER(cp2), 
					   &short_r, &short_g, &short_b, &short_a);
		
		curState.bgColor2.red = short_r;
		curState.bgColor2.green = short_g;
		curState.bgColor2.blue = short_b;
		
		fill_monitor (TRUE, &curState);
	}
}

static gint
idle_fill_monitor (gpointer data)
{
	fill_monitor (FALSE, &curState);
	return FALSE;
}

static GtkWidget *radion;
static GtkWidget *tiledButton, *scaledButton, *embossedButton;
static GtkWidget *wallp, *fill;
static GtkWidget *scaledkeepButton, *centeredButton;

static void
set_enabled (GtkWidget *widget, gpointer data)
{
	gint a = GTK_TOGGLE_BUTTON (widget)->active;
        gtk_widget_set_sensitive(wallp, a);
        gtk_widget_set_sensitive(fill, a);
	curState.enabled = a;
	fill_monitor (!ignoreChanges, &curState); 
}
    
static void
set_wallpaper_type (GtkWidget *widget, gpointer data)
{
	if (GTK_TOGGLE_BUTTON (widget)->active) {
		curState.wpType = (gint) data;
	
		fill_monitor (!ignoreChanges, &curState);
	}
	if (!ignoreChanges)
		capplet_widget_state_changed(CAPPLET_WIDGET(capplet), TRUE);
}

static void
set_solid (GtkWidget *widget, gpointer data)
{
	gtk_widget_set_sensitive(cp1, TRUE);
	gtk_widget_set_sensitive(cp2, FALSE);
	gtk_widget_set_sensitive(cl2, FALSE);
	curState.enabled = TRUE;
	curState.grad = FALSE;
	fill_monitor (!ignoreChanges, &curState);
}

static void
set_grad (GtkWidget *widget, gpointer data)
{
	gtk_widget_set_sensitive(cp1, TRUE);
	gtk_widget_set_sensitive(cp2, TRUE);
	gtk_widget_set_sensitive(cl2, TRUE);
	curState.enabled = TRUE;
	curState.grad = TRUE;
	if ((gboolean)data == TRUE)
		curState.vertical = FALSE;
	else
		curState.vertical = TRUE;
	fill_monitor (!ignoreChanges, &curState);
}

static GtkWidget *
color_setup (BGState *state)
{
	GtkWidget *frame;
	GtkWidget *table;
	GtkWidget *menu, *optionmenu;
    
	frame = gtk_frame_new (_("Color"));
    
	table = gtk_table_new (3, 2, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER(table), GNOME_PAD);
	gtk_table_set_col_spacings (GTK_TABLE(table), GNOME_PAD);
	gtk_container_add (GTK_CONTAINER(frame), table);
    
	menu = gtk_menu_new ();

	menuitem1 = gtk_menu_item_new_with_label(_("Solid"));
	gtk_signal_connect(GTK_OBJECT(menuitem1), "activate", GTK_SIGNAL_FUNC(set_solid), 
			   NULL);
	gtk_menu_append(GTK_MENU(menu), menuitem1); 
	gtk_widget_show(menuitem1);

	menuitem2 = gtk_menu_item_new_with_label(_("Horizontal Gradient"));
	gtk_signal_connect(GTK_OBJECT(menuitem2), "activate", GTK_SIGNAL_FUNC(set_grad), 
			   GINT_TO_POINTER (TRUE));
	gtk_menu_append(GTK_MENU(menu), menuitem2);
	gtk_widget_show(menuitem2);

	menuitem3 = gtk_menu_item_new_with_label(_("Vertical Gradient"));
	gtk_signal_connect(GTK_OBJECT(menuitem3), "activate", GTK_SIGNAL_FUNC(set_grad),
			   FALSE);
	gtk_menu_append(GTK_MENU(menu), menuitem3);
	gtk_widget_show(menuitem3);    

	optionmenu = gtk_option_menu_new();
	gtk_option_menu_set_menu(GTK_OPTION_MENU(optionmenu), menu);

	if (curState.grad) {
		if (curState.vertical) {
			gtk_option_menu_set_history (GTK_OPTION_MENU(optionmenu), 2);
		} else {
			gtk_option_menu_set_history (GTK_OPTION_MENU(optionmenu), 1);
		}
	} else {
		gtk_option_menu_set_history (GTK_OPTION_MENU(optionmenu), 0);
	}

	gtk_table_attach (GTK_TABLE(table), optionmenu, 0, 2, 0, 1,
			  GTK_EXPAND | GTK_SHRINK | GTK_FILL,
			  GTK_EXPAND | GTK_SHRINK, 0, 0); 

	cp1 = gnome_color_picker_new ();
	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(cp1), 
				    state->bgColor1.red, state->bgColor1.green,
				    state->bgColor1.blue, 0xff);
	gtk_signal_connect(GTK_OBJECT(cp1), "color_set", 
			   GTK_SIGNAL_FUNC(color_sel_set), NULL);
	
	cp2 = gnome_color_picker_new ();
	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(cp2), 
				    state->bgColor2.red, state->bgColor2.green,
				    state->bgColor2.blue, 0xff);
	gtk_signal_connect(GTK_OBJECT(cp2), "color_set", 
			   GTK_SIGNAL_FUNC(color_sel_set), NULL);
	gtk_table_attach_defaults (GTK_TABLE(table), gtk_label_new (_("Primary Color")), 0, 1, 1, 2);
	gtk_table_attach (GTK_TABLE(table), cp1, 1, 2, 1, 2, 0, 0, 0, 0);
	cl2 = gtk_label_new (_("Secondary Color"));
	gtk_table_attach_defaults (GTK_TABLE(table), cl2, 0, 1, 2, 3);
	gtk_table_attach (GTK_TABLE(table), cp2, 1, 2, 2, 3, 0, 0, 0, 0);

	if (!state->grad) {
		gtk_widget_set_sensitive (cp2, FALSE);
		gtk_widget_set_sensitive (cl2, FALSE);
	}

	gtk_idle_add ((GtkFunction) idle_fill_monitor, NULL);
	gtk_widget_show_all (frame);
	return frame;
}

static gint
delete_browse (GtkWidget *w, GdkEvent *e, GtkWidget **f)
{	
	if (wpFileSelName)
		g_free (wpFileSelName);
	wpFileSelName = NULL;
	
	fileSel = NULL;
	return FALSE;
}

static void
browse_activated (GtkWidget *w, gchar *s)
{
	if (curState.wpFileName)
		g_free(curState.wpFileName);
	curState.wpFileName = g_strdup(s);
	curState.bgType = (s) ? BACKGROUND_WALLPAPER : BACKGROUND_SIMPLE;
	fill_monitor (TRUE, &curState);
}

static void
wp_selection_cancel (GtkWidget *w, GtkWidget **f)
{
	gtk_widget_destroy (fileSel);
	fileSel = NULL;

#if 0
	GtkWidget *cf = *f;
	delete_browse (w, NULL, f);
#endif
}

static void
set_monitor_filename (gchar *str)
{
	GString *gs;
	gchar num[32];
	gint found = -1, i=1;
	GList *child = GTK_MENU_SHELL (wpMenu)->children;
	GtkWidget *cf;
    
/*    printf("searching for %s\n",str); */
	while (child) {
		if (child->data)
			if (GTK_BIN (child->data)->child) {
/*		printf ("Searching %s\n", GTK_LABEL (GTK_BIN (child->data)->child)->label); */
				if (!strcmp (GTK_LABEL (GTK_BIN (child->data)->child)->label, str)) {
					found = i;
/*		    printf ("found: %d\n", i);  */
				}
			}
		i++;
		child = child->next;
	}
    
	/* hack */
	if (!ignoreChanges && found < 0) {
	
		cf = gtk_menu_item_new_with_label (str);
		gtk_signal_connect (GTK_OBJECT (cf),
				    "activate",
				    (GtkSignalFunc) browse_activated, str);
		gtk_menu_append (GTK_MENU (wpMenu), cf);
		gtk_widget_show (cf);
		wpNum++;
	
		gs = g_string_new ("/Background/Default/wallpaper");
		snprintf (num, sizeof(num), "%d", wpNum);
		g_string_append (gs, num);
		gnome_config_set_string (gs->str, str);
		g_string_free (gs, TRUE);
	
		gnome_config_set_int ("/Background/Default/wallpapers", wpNum);
		gnome_config_set_string ("/Background/Default/wallpapers_dir",
					 str);
	
		found = wpNum;
		gnome_config_sync ();
	}
    
	if (curState.wpFileName)
		g_free(curState.wpFileName);
	curState.wpFileName = g_strdup(str);
	curState.bgType = BACKGROUND_WALLPAPER;
    
	gtk_option_menu_set_history (GTK_OPTION_MENU (wpOMenu), found);
    
	fill_monitor (!ignoreChanges, &curState);

	if (!ignoreChanges)
		capplet_widget_state_changed(CAPPLET_WIDGET(capplet), TRUE);
}

static void
wp_selection_ok (GtkWidget *w, GtkWidget **f)
{
	if (w) {
		if (wpFileSelName)
			g_free (wpFileSelName);
		wpFileSelName = g_strdup (gtk_file_selection_get_filename
					  (GTK_FILE_SELECTION (*f)));

		if (!g_file_test(wpFileSelName, G_FILE_TEST_ISFILE)) {
			g_free(wpFileSelName);
			wpFileSelName = NULL;
		}

	}

	gtk_widget_destroy(fileSel);
        fileSel = NULL;

	if (wpFileSelName)
		set_monitor_filename (wpFileSelName);

}

static void
setup_preview(GtkWidget *widget)
{
	char *p;
	GList *l;
	GtkWidget *pp = NULL;
	GdkImlibImage *im;
	int w,h;
	GtkWidget *frame;
	GtkFileSelection *fs;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_WIDGET (widget));

	frame = gtk_object_get_data(GTK_OBJECT(widget),"frame");
	fs = gtk_object_get_data(GTK_OBJECT(frame),"fs");

	if((l = gtk_container_children(GTK_CONTAINER(frame))) != NULL) {
		pp = l->data;
		g_list_free(l);
	}

	if(pp)
		gtk_widget_destroy(pp);
	
	p = gtk_file_selection_get_filename(fs);
	if(!p || !g_file_test (p,G_FILE_TEST_ISLINK|G_FILE_TEST_ISFILE) ||
	   !(im = gdk_imlib_load_image (p)))
		return;

	w = im->rgb_width;
	h = im->rgb_height;
	if(w>h) {
		if(w>100) {
			h = h*(100.0/w);
			w = 100;
		}
	} else {
		if(h>100) {
			w = w*(100.0/h);
			h = 100;
		}
	}
	pp = gnome_pixmap_new_from_imlib_at_size (im, w, h);
	gtk_widget_show(pp);
	gtk_container_add(GTK_CONTAINER(frame),pp);

	gdk_imlib_destroy_image(im);
}

static void
browse_wallpapers (GtkWidget *w, gpointer p)
{
	GtkWidget *hbox, *widg;
	if (!fileSel) {
    
		fileSel = gtk_file_selection_new (_("Wallpaper Selection"));
		hbox = GTK_FILE_SELECTION (fileSel)->file_list;
		do {
			hbox = hbox->parent;
			if(!hbox) {
				g_warning(_("Can't find an hbox, using a normal file "
					    "selection"));
				return;
			}
		} while(!GTK_IS_HBOX(hbox));
		widg = gtk_frame_new(_("Preview"));
		gtk_widget_show(widg);
		gtk_box_pack_end(GTK_BOX(hbox),widg,FALSE,FALSE,0);
		gtk_widget_set_usize(widg,110,110);
		gtk_object_set_data(GTK_OBJECT(widg),"fs",fileSel);
		gtk_object_set_data(GTK_OBJECT(GTK_FILE_SELECTION (fileSel)->file_list),
				    "frame",widg);
		gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION (fileSel)->file_list),"select_row",
				   GTK_SIGNAL_FUNC(setup_preview),NULL);
		gtk_object_set_data(GTK_OBJECT(GTK_FILE_SELECTION (fileSel)->selection_entry),"frame",widg);
		gtk_signal_connect_while_alive(GTK_OBJECT(GTK_FILE_SELECTION (fileSel)->selection_entry),
					       "changed",
					       GTK_SIGNAL_FUNC(setup_preview),NULL,
					       GTK_OBJECT(fileSel));


		if (wpFileSelName)
			gtk_file_selection_set_filename (GTK_FILE_SELECTION (fileSel),
							 wpFileSelName);
		else if (origState.wpFileName)
			gtk_file_selection_set_filename (GTK_FILE_SELECTION (fileSel),
							 origState.wpFileName);
	
		gtk_signal_connect (GTK_OBJECT (fileSel), "delete_event",
				    (GtkSignalFunc) delete_browse,
				    &fileSel);
	
		gtk_signal_connect (GTK_OBJECT
				    (GTK_FILE_SELECTION (fileSel)->ok_button),
				    "clicked", (GtkSignalFunc) wp_selection_ok,
				    &fileSel);
	
		gtk_signal_connect (GTK_OBJECT
				    (GTK_FILE_SELECTION (fileSel)->cancel_button),
				    "clicked",
				    (GtkSignalFunc) wp_selection_cancel,
				    &fileSel);
	
		gtk_widget_show (fileSel);
	}
}

static GtkWidget *
wallpaper_setup (BGState *state)
{
	GtkWidget *wallp;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *none;
	GtkWidget *but;
	GtkWidget *rbut;
	gint i;
	gint selectedWp = 0;
	GString *wpName;
	gchar num[32];
	gchar *wpName1;
    
	wallp = gtk_frame_new (_("Wallpaper"));
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, GNOME_PAD);
	but = gtk_button_new_with_label (_(" Browse... "));
	gtk_signal_connect (GTK_OBJECT (but), "clicked",
			    (GtkSignalFunc) browse_wallpapers, NULL);
    
	wpMenu = gtk_menu_new ();
	none = gtk_menu_item_new_with_label (_("none"));
	gtk_menu_append (GTK_MENU (wpMenu), none);
	gtk_widget_show (none);
	gtk_signal_connect (GTK_OBJECT (none),
			    "activate",
			    (GtkSignalFunc) browse_activated, NULL);
    
	wpNum = gnome_config_get_int ("/Background/Default/wallpapers=0");
    
	for (i = 0; i<wpNum; i++) {
	
		/* printf ("wallpaper%d", i); */
		wpName = g_string_new ("/Background/Default/wallpaper");
		snprintf (num, sizeof(num),"%d", i+1);
		g_string_append (wpName, num);
		g_string_append (wpName, "=???");
	
		wpName1 = gnome_config_get_string (wpName->str);
		/* printf (": %s\n", wpName1); */
		if (wpName1) {
			if (state->wpFileName)
				if (!strcmp (wpName1, state->wpFileName))
					selectedWp = i + 1;
	    
			none = gtk_menu_item_new_with_label (wpName1);
			gtk_menu_append (GTK_MENU (wpMenu), none);
			gtk_signal_connect (GTK_OBJECT (none),
					    "activate",
					    (GtkSignalFunc) browse_activated, wpName1);
			gtk_widget_show (none);
		}
	
		g_string_free (wpName, TRUE);
	}
    
	wpOMenu = gtk_option_menu_new ();
	gtk_option_menu_set_menu (GTK_OPTION_MENU (wpOMenu), wpMenu);	
	gtk_option_menu_set_history (GTK_OPTION_MENU (wpOMenu), selectedWp);
	gtk_widget_set_usize (wpOMenu, 120, -1);
    
	gtk_box_pack_start (GTK_BOX (hbox), wpOMenu, TRUE, TRUE, 0);
	gtk_box_pack_end (GTK_BOX (hbox), but, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    
	rbut = gtk_radio_button_new_with_label (NULL, 
						_("Embossed Logo"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rbut),
				      (state->wpType == WALLPAPER_EMBOSSED));
	gtk_signal_connect (GTK_OBJECT (rbut), "toggled",
			    (GtkSignalFunc) set_wallpaper_type,
			    (gpointer) WALLPAPER_EMBOSSED);
	gtk_box_pack_end (GTK_BOX (vbox), rbut, FALSE, FALSE, 0);
	gtk_widget_show (rbut);
	embossedButton = rbut;

	rbut = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (rbut)),
						_("Scaled"));
	gtk_signal_connect (GTK_OBJECT (rbut), "toggled",
			    (GtkSignalFunc) set_wallpaper_type,
			    (gpointer)WALLPAPER_SCALED);
	gtk_box_pack_end (GTK_BOX (vbox), rbut, FALSE, FALSE, 0);
	gtk_widget_show (rbut);
	scaledButton = rbut;

	rbut = gtk_radio_button_new_with_label
		(gtk_radio_button_group (GTK_RADIO_BUTTON (rbut)),
		 _("Scaled (keep aspect)"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rbut),
				      (state->wpType == WALLPAPER_SCALED_KEEP));
	gtk_signal_connect (GTK_OBJECT (rbut), "toggled",
			    (GtkSignalFunc) set_wallpaper_type,
			    (gpointer) WALLPAPER_SCALED_KEEP);
	gtk_box_pack_end (GTK_BOX (vbox), rbut, FALSE, FALSE, 0);
	gtk_widget_show (rbut);
	scaledkeepButton = rbut;

	rbut = gtk_radio_button_new_with_label
		(gtk_radio_button_group (GTK_RADIO_BUTTON (rbut)),
		 _("Centered"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rbut),
				      (state->wpType == WALLPAPER_CENTERED));
	gtk_signal_connect (GTK_OBJECT (rbut), "toggled",
			    (GtkSignalFunc) set_wallpaper_type,
			    (gpointer) WALLPAPER_CENTERED);
	gtk_box_pack_end (GTK_BOX (vbox), rbut, FALSE, FALSE, 0);
	gtk_widget_show (rbut);
	centeredButton = rbut;

	rbut = gtk_radio_button_new_with_label
		(gtk_radio_button_group (GTK_RADIO_BUTTON (rbut)),
		 _("Tiled"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (rbut),
				      (state->wpType == WALLPAPER_TILED));
	gtk_signal_connect (GTK_OBJECT (rbut), "toggled",
			    (GtkSignalFunc) set_wallpaper_type,
			    (gpointer) WALLPAPER_TILED);
	gtk_box_pack_end (GTK_BOX (vbox), rbut, FALSE, FALSE, 0);
	gtk_widget_show (rbut);
	tiledButton = rbut;

	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD);
	gtk_container_add (GTK_CONTAINER (wallp), vbox);

	gtk_widget_show (wpOMenu);
	gtk_widget_show (but);
	gtk_widget_show (hbox);
	gtk_widget_show (vbox);
	gtk_widget_show (wallp);

	return wallp;
}

static void
background_apply (BGState *state)
{
	GtkWidget *choice;

	/* need to move all this stuff eventually */
	ignoreChanges = TRUE;


	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(cp1), 
				    state->bgColor1.red, state->bgColor1.green,
				    state->bgColor1.blue, 0xff);
	gnome_color_picker_set_i16 (GNOME_COLOR_PICKER(cp2), 
				    state->bgColor2.red, state->bgColor2.green,
				    state->bgColor2.blue, 0xff);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radion), state->enabled);

	if (state->enabled) {
		gtk_widget_set_sensitive(cp2, state->grad);
		gtk_widget_set_sensitive(cl2, state->grad);
	} else {
		gtk_widget_set_sensitive(cp1, FALSE);
		gtk_widget_set_sensitive(cp2, FALSE);
		gtk_widget_set_sensitive(cl2, FALSE);
	}
	switch (state->wpType) {
	case WALLPAPER_SCALED:
		choice = scaledButton;
		break;
	case WALLPAPER_SCALED_KEEP:
		choice = scaledkeepButton;
		break;
	case WALLPAPER_CENTERED:
		choice = centeredButton;
		break;
	case WALLPAPER_TILED:
		choice = tiledButton;
		break;
	case WALLPAPER_EMBOSSED:
		choice = embossedButton;
		break;
	default:
		choice = tiledButton;
		break;
	}
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (choice), TRUE);

	if (state->bgType!= BACKGROUND_SIMPLE) {
		gchar *tmp;
		/* ok this is horrible i should die */
		tmp = g_strdup(state->wpFileName);
		set_monitor_filename(tmp);
		g_free(tmp);
	} else {
		gtk_option_menu_set_history(GTK_OPTION_MENU(wpOMenu), 0);
	}

	fillPreview = FALSE;
	fill_monitor (FALSE, state);
	fillPreview = TRUE;
	ignoreChanges=FALSE;
}

static void
background_help(GtkWidget *widget, BGState *state)
{
    GnomeHelpMenuEntry help_entry= {"control-center",
    "desktop-intro.html#GCCBACK"};
    gnome_help_display (NULL, &help_entry);
}

static void
background_try(GtkWidget *widget, BGState *state)
{
	/* printState(state); */
	
	/* Go ahead and write out the state
	 * - this allows nautilus to see the changes so it can update
	 * - revert or cancel will write back the original state if necessary
	 */
	 
	/* Write before applying so nautilus can see the changes when it gets the x-event */
	background_write(state);
	background_apply(state);
}

static void
background_cancel ()
{
	/* Write before applying so nautilus can see the changes when it gets the x-event */
	background_write(&origState);
	background_apply(&origState);
}

static void
background_revert ()
{
	ignoreChanges = TRUE;


	/* Write before applying so nautilus can see the changes when it gets the x-event */
	background_write(&origState);
	background_apply(&origState);
	fillPreview = TRUE;
	fill_monitor (FALSE, &origState);


	copyState(&curState, &origState);
	ignoreChanges = FALSE;
}

static void
background_write (BGState *state)
{
	char buffer [60];


	snprintf (buffer, sizeof(buffer), "#%02x%02x%02x",
		  state->bgColor1.red >> 8,
		  state->bgColor1.green >> 8,
		  state->bgColor1.blue >> 8);
	gnome_config_set_string ("/Background/Default/color1", buffer);
	snprintf (buffer, sizeof(buffer), "#%02x%02x%02x",
		  state->bgColor2.red >> 8,
		  state->bgColor2.green >> 8,
		  state->bgColor2.blue >> 8);
	gnome_config_set_string ("/Background/Default/color2", buffer);

	gnome_config_set_bool ("/Background/Default/Enabled", state->enabled);
	gnome_config_set_string ("/Background/Default/simple",
				 (state->grad) ? "gradient" : "solid");
	gnome_config_set_string ("/Background/Default/gradient",
				 (state->vertical) ? "vertical" : "horizontal");
    
	gnome_config_set_string ("/Background/Default/wallpaper",
				 (state->bgType == BACKGROUND_SIMPLE) ? "none" : state->wpFileName);
	gnome_config_set_int ("/Background/Default/wallpaperAlign", state->wpType);
    
	gnome_config_sync ();
#if 0    
	background_apply(state);
#endif
}

void
background_read ( BGState *state )
{
	gint r, g, b;

	gdk_color_parse
		(gnome_config_get_string ("/Background/Default/color1=#005060"),
		 &state->bgColor1);
	r = state->bgColor1.red >> 8;
	g = state->bgColor1.green >> 8;
	b = state->bgColor1.blue >> 8;
	state->bgColor1.pixel = gdk_imlib_best_color_match(&r, &g, &b);
 
	gdk_color_parse
		(gnome_config_get_string ("/Background/Default/color2=#0000ff"),
		 &state->bgColor2);

	r = state->bgColor2.red >> 8;
	g = state->bgColor2.green >> 8;
	b = state->bgColor2.blue >> 8;
	state->bgColor2.pixel = gdk_imlib_best_color_match(&r, &g, &b);
  
	state->enabled = gnome_config_get_bool
		("/Background/Default/Enabled=True");
	state->bgType = (strcasecmp
		  (gnome_config_get_string
		   ("/Background/Default/type=simple"),
		   "simple"));
	state->grad = (strcasecmp
		(gnome_config_get_string
		 ("/Background/Default/simple=solid"),
		 "solid"));
	state->vertical = !(strcasecmp
		     (gnome_config_get_string
		      ("/Background/Default/gradient=vertical"),
		      "vertical"));
	state->wpType = gnome_config_get_int ("/Background/Default/wallpaperAlign=0");

	state->wpFileName = gnome_config_get_string ("/Background/Default/wallpaper=none");
	state->wpFileSelName = gnome_config_get_string 
		("/Background/Default/wallpapers_dir=./");

	if (!strcasecmp (state->wpFileName, "none")) {
		g_free(state->wpFileName);
		state->wpFileName = NULL;
		state->bgType = BACKGROUND_SIMPLE;
	} else
		state->bgType = BACKGROUND_WALLPAPER;
}

void
background_ok(GtkWidget *widget, BGState *state)
{
	/* Write before applying so nautilus can see the changes when it gets the x-event */
	background_write(state);
	background_apply(state);
}

void
background_init() {
	background_read(&origState);
	copyState(&curState, &origState);
	ignoreChanges = TRUE;
	background_setup(&origState);
	ignoreChanges = FALSE;

}

void
background_properties_init() {
	background_read(&origState);
	copyState(&curState, &origState);
	ignoreChanges = TRUE;
	fillPreview = FALSE;
	fill_monitor (FALSE, &origState);
	fillPreview = TRUE;
	ignoreChanges = FALSE;

}

/*
 * Invoked when a filename is dropped on the monitor widget
 */
static void
img_dnd_drop (GtkWidget *widget, GdkDragContext *context, gint x, gint y,
	      GtkSelectionData *selection_data, guint info,
	      guint time, gpointer data)
{
        GList *names;
  
	switch (info) {
	case TARGET_URI_LIST:
		names = gnome_uri_list_extract_filenames (selection_data->data);
		if (names) {
			set_monitor_filename ((gchar *)names->data);
			gnome_uri_list_free_strings (names);
		}
		break;
	default:
		break;
	}
}

/* Returns an GtkEventBox with a GtkPixmap in it.  The pixmap is
 * inside an eventbox because we need it to have an X window.  It will
 * the use the visual and colormap that the global imlib_data carry
 * (i.e. the default visual and colormap).
 */

static GtkWidget *
get_monitor_preview_widget (void)
{
	char *f;
	GdkPixmap *pixmap, *mask;
	GdkGC *gc;
	GdkPixbuf *pixbuf;
	GtkWidget *eb, *p;

	f = gnome_pixmap_file ("monitor.png");
	pixbuf = gdk_pixbuf_new_from_file (f);
	g_free (f);

	if (pixbuf) {
		pixmap = gdk_pixmap_new (GDK_ROOT_PARENT (),
					 gdk_pixbuf_get_width (pixbuf),
					 gdk_pixbuf_get_height (pixbuf),
					 -1);

		gc = gdk_gc_new (pixmap);
		render_to_drawable (pixbuf, pixmap, gc, 0, 0, 0, 0,
				    gdk_pixbuf_get_width (pixbuf),
				    gdk_pixbuf_get_height (pixbuf),
				    GDK_RGB_DITHER_NORMAL,
				    0, 0);
		gdk_gc_unref (gc);
		
		gdk_pixbuf_render_pixmap_and_mask (pixbuf, NULL, &mask, 128);
		
	} else {
		pixmap = NULL;
		mask = NULL;
	}

	eb = gtk_event_box_new ();
	p = gtk_pixmap_new (pixmap, mask);
	gtk_container_add (GTK_CONTAINER (eb), p);
	gtk_widget_show (p);
	gtk_widget_show (eb);

	return eb;
}

/*
 * background_setup: creates the dialog box for configuring the
 * display background and registers it with the display property
 * configurator
 */
void
background_setup (BGState *state)
{
	static GtkTargetEntry drop_types [] = { 
		{ "text/uri-list", 0, TARGET_URI_LIST },
	};
	static gint n_drop_types = sizeof (drop_types) / sizeof(drop_types[0]);

	GtkWidget *frame;
	GtkWidget *vbox, *vbox2;
	GtkWidget *hbox;
    
	capplet = capplet_widget_new();
	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	vbox2 = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);


	wallp  = wallpaper_setup (state);
	fill = color_setup (state);
	monitor = get_monitor_preview_widget ();
	frame = gtk_frame_new (_("Preview") );
	gtk_container_add (GTK_CONTAINER(frame), monitor);
	gtk_drag_dest_set (GTK_WIDGET (monitor),
			   GTK_DEST_DEFAULT_MOTION |
			   GTK_DEST_DEFAULT_HIGHLIGHT |
			   GTK_DEST_DEFAULT_DROP,
			   drop_types, n_drop_types,
			   GDK_ACTION_COPY);
	gtk_signal_connect (GTK_OBJECT (monitor), "drag_data_received",
			    GTK_SIGNAL_FUNC (img_dnd_drop), NULL);
	gdk_null_window_warnings = 0;

	radion = gtk_check_button_new_with_label ( _("Use GNOME to set background") );

	gtk_box_pack_start (GTK_BOX (vbox2), wallp, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox2), fill, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), radion, FALSE, FALSE, 0);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radion), state->enabled);
	gtk_widget_set_sensitive (fill, state->enabled);
	gtk_widget_set_sensitive (wallp, state->enabled);

	gtk_signal_connect (GTK_OBJECT(radion), "toggled",
			    (GtkSignalFunc) set_enabled,
			    NULL);

	gtk_widget_show_all (vbox);

	gtk_signal_connect (GTK_OBJECT (capplet), "help",
			    GTK_SIGNAL_FUNC (background_help), &curState);
	gtk_signal_connect (GTK_OBJECT (capplet), "try",
			    GTK_SIGNAL_FUNC (background_try), &curState);
	gtk_signal_connect (GTK_OBJECT (capplet), "revert",
			    GTK_SIGNAL_FUNC (background_revert), NULL);
	gtk_signal_connect (GTK_OBJECT (capplet), "cancel",
			    GTK_SIGNAL_FUNC (background_cancel), NULL);
	gtk_signal_connect (GTK_OBJECT (capplet), "ok",
			    GTK_SIGNAL_FUNC (background_ok), &curState);

	gtk_container_add(GTK_CONTAINER(capplet), vbox);
	gtk_widget_show(capplet);
}


#if 0
enum {
	INIT_KEY      = -1,
	WALLPAPER_KEY = -2,
	COLOR_KEY     = -3,
	ENDCOLOR_KEY  = -4,
	ORIENT_KEY    = -5,
	SOLID_KEY     = -6,
	GRADIENT_KEY  = -7,
	ALIGN_KEY     = -8
};

/* Options used by background properties.  */
static struct argp_option arguments[] =
{
	{ "init",                    -1,  NULL,        0, N_("Set parameters from saved state and exit"), 1 },
	{ "setwallpaper",  WALLPAPER_KEY, N_("IMAGE"), 0, N_("Sets the wallpaper to the value specified"), 1 },
	{ "color",         COLOR_KEY,     N_("COLOR"), 0, N_("Specifies the background color"), 1 },
	{ "endcolor",      ENDCOLOR_KEY,  N_("COLOR"), 0, N_("Specifies end background color for gradient"), 1 },
	{ "orient",        ORIENT_KEY,    N_("ORIENT"),0, N_("Gradient orientation: vertical or horizontal"), 1 },
	{ "solid",         SOLID_KEY,     NULL,        0, N_("Use a solid fill for the background"), 1 },
	{ "gradient",      GRADIENT_KEY,  NULL,        0, N_("Use a gradient fill for the background"), 1 },
	{ "wallpapermode", ALIGN_KEY,     N_("MODE"),  0, N_("Display wallpaper: tiled, centered, scaled or ratio"), 1 },
	{ NULL, 0, NULL, 0, NULL, 0 }
};

/* Forward decl of our parsing function.  */
static error_t parse_func (int key, char *arg, struct argp_state *state);

/* The parser used by this program.  */
struct argp parser = {
	arguments,
	parse_func,
	NULL,
	NULL,
	NULL,
	NULL
};

static error_t
parse_func (int key, char *arg, struct argp_state *state)
{
	char *align_keys [] = { "tiled", "centered", "scaled", "ratio" };
	int i;
	
	switch (key){
	case ARGP_KEY_ARG:
		argp_usage (state);
		return 0;

	case INIT_KEY:
		init = 1;
		return 0;

	case WALLPAPER_KEY:
		gnome_config_set_string ("/Background/Default/wallpaper", arg);
		init = need_sync = 1;
		return 0;

	case COLOR_KEY:
		gnome_config_set_string ("/Background/Default/color1", arg);
		init = need_sync = 1;
		return 0;

	case ENDCOLOR_KEY:
		gnome_config_set_string ("/Background/Default/color2", arg);
		gnome_config_set_string ("/Background/Default/simple", "gradient");
		init = need_sync = 1;
		return 0;

	case ORIENT_KEY:
		if (strcasecmp (arg, "vertical") == 0 || strcasecmp (arg, "horizontal") == 0){
			gnome_config_set_string ("/Background/Default/gradient", arg);
			init = need_sync = 1;
			return 0;
		} else
			return ARGP_ERR_UNKNOWN;
	case SOLID_KEY:
		gnome_config_set_string ("/Background/Default/simple", "solid");
		init = need_sync = 1;
		return 0;

	case GRADIENT_KEY:
		gnome_config_set_string ("/Background/Default/simple", "gradient");
		init = need_sync = 1;
		return 0;

	case ALIGN_KEY:
		for (i = 0; i < 4; i++)
			if (strcasecmp (align_keys [i], arg) == 0){
				gnome_config_set_int ("/Background/Default/wallpaperAlign", i);
				init = need_sync = 1;
				return 0;
			}
		return ARGP_ERR_UNKNOWN;

	default:
		return ARGP_ERR_UNKNOWN;
		break;
	}
}

#endif

