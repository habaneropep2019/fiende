/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   guname: System information dialog.
 *
 *   Copyright (C) 1998 Havoc Pennington <hp@pobox.com> except marquee code.
 *   Control Center applet support by Nat Friedman <nat@gnome-support.com>.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <gnome.h>
#include <libgnome/gnome-util.h>
#include <libgnomeui/gnome-window-icon.h>
#include <errno.h>

#include <sys/utsname.h>
#include <unistd.h>

#ifdef GUNAME_CAPPLET
#include <capplet-widget.h>
#endif

#include "list.h"
#if HAVE_LIBGTOP
#include "moreinfo.h"
#endif
#include "info.h"

#define APPNAME "guname"
#define COPYRIGHT_NOTICE _("Copyright 1998, under the GNU General Public License.")

#ifndef VERSION
#define VERSION "0.0.0"
#endif

#ifdef GUNAME_CAPPLET
static gint ignore_me;
static struct poptOption guname_options[] = {
  {"capplet", '\0', POPT_ARG_NONE, &ignore_me, 0,
   N_("Make guname act as a control center applet"), NULL},
  {NULL, '\0', 0, NULL, 0}
};
#endif /* GUNAME_CAPPLET */

/****************************
  Function prototypes
  ******************************/

#ifdef GUNAME_CAPPLET
static void create_guname_capplet_window (void);
#endif /* GUNAME_CAPPLET */

static void popup_main_dialog();
static void save_callback(GtkWidget * menuitem, gpointer data);
static void mail_callback(GtkWidget * menuitem, gpointer data);
static void detailed_callback(GtkWidget * menuitem, gpointer data);
static gint list_clicked_cb(GtkCList * list, GdkEventButton * e);

static void write_to_filestream(FILE * f);
static gchar* write_to_string();

static void do_logo_box(GtkWidget * box);
static void do_list_box(GtkWidget * box);
static void do_popup(GtkWidget * clist);
static void do_marquee(GtkWidget * box);
static int marquee_timer (gpointer data);

/****************************
  Globals
  ************************/

GtkWidget * popup = NULL;

/*******************************
  Main
  *******************************/

int
main ( int argc, char ** argv )
{
  gboolean capplet_mode = FALSE;
  int i;

  /* Initialize the i18n stuff (the call to 'libgtop' catalog is to translate
   * the strings that are imported from libtop libs
   */
  bindtextdomain ("libgtop", GNOMELOCALEDIR);
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

#ifdef GUNAME_CAPPLET
  for (i = 1; i < argc; i ++)
    if (! strcmp (argv [i], "--capplet")) {
      capplet_mode = TRUE;
      break;
    }

  if (capplet_mode) {
    int init_results;
    GnomeClient *client;
    GnomeClientFlags flags;

    init_results = gnome_capplet_init ("guname-capplet", VERSION,
                                       argc, argv, guname_options, 0, NULL);
    if (init_results < 0)
        g_error (_("an initialization error occurred while "
                 "starting 'guname-capplet'."));

    client = gnome_master_client ();

    if (client)
        flags = gnome_client_get_flags (client);
    else
        flags = 0;

    if (flags & GNOME_CLIENT_IS_CONNECTED) {
        gnome_client_set_restart_style (client, GNOME_RESTART_NEVER);
            
        gnome_client_flush (client);
    }
  } else {
    gnome_init_with_popt_table (APPNAME, VERSION, argc, argv, guname_options, 0, NULL);
  }
#else /* ! GUNAME_CAPPLET */
  gnome_init (APPNAME, VERSION, argc, argv);
#endif /* ! GUNAME_CAPPLET */

  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-info.png");

  load_system_info ();

#if HAVE_LIBGTOP
  load_moreinfo ();
#endif

#ifdef GUNAME_CAPPLET
  if (capplet_mode) {
    create_guname_capplet_window ();

    capplet_gtk_main ();

    return EXIT_SUCCESS;
  }
#endif /* GUNAME_CAPPLET */

  popup_main_dialog();

  gtk_main();

  return EXIT_SUCCESS;
}

#ifdef GUNAME_CAPPLET
static void
create_guname_capplet_window (void)
{
    GtkWidget *capplet;
    GtkWidget *vbox;
    GtkWidget *logo_box;
    GtkWidget *list_box;

    capplet = capplet_widget_new ();

    vbox = gtk_vbox_new (FALSE, 0);

    logo_box = gtk_vbox_new (FALSE, 0);
    do_logo_box (logo_box);

    list_box = gtk_vbox_new (TRUE, 0);
    do_list_box (list_box);

    gtk_box_pack_start (GTK_BOX (vbox), logo_box, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), list_box, FALSE, FALSE, 0);

    gtk_container_add (GTK_CONTAINER (capplet), vbox);

    gtk_widget_show_all (capplet);
}

#endif /* GUNAME_CAPPLET */

/****************************
 Little sparkles                            
 ****************************/
#ifdef WITH_SPARKLES

#define NUM_FRAMES 8

struct _Sparkle {
  GnomeCanvasItem* hline;
  GnomeCanvasItem* vline;
  GnomeCanvasItem* hsmall;
  GnomeCanvasItem* vsmall;
 
  GnomeCanvasPoints* hpoints[NUM_FRAMES];
  GnomeCanvasPoints* vpoints[NUM_FRAMES];
  GnomeCanvasPoints* hspoints[NUM_FRAMES];
  GnomeCanvasPoints* vspoints[NUM_FRAMES];
  /* -1 is magic for "stop" */
  gint count;
  /* growing or shrinking */
  gboolean up;
};

typedef struct _Sparkle Sparkle;

static void
sparkle_destroy(Sparkle* sparkle)
{
  int i;
  g_return_if_fail(sparkle != NULL);

  gtk_object_destroy(GTK_OBJECT(sparkle->hline));
  gtk_object_destroy(GTK_OBJECT(sparkle->vline));
  gtk_object_destroy(GTK_OBJECT(sparkle->hsmall));
  gtk_object_destroy(GTK_OBJECT(sparkle->vsmall));

  i = 0;
  while (i < NUM_FRAMES) {
    gnome_canvas_points_free(sparkle->hpoints[i]);
    gnome_canvas_points_free(sparkle->vpoints[i]);
    gnome_canvas_points_free(sparkle->hspoints[i]);
    gnome_canvas_points_free(sparkle->vspoints[i]);
    ++i;
  }
  g_free(sparkle);
}

static gint
sparkle_timeout(Sparkle* sparkle)
{
  g_return_val_if_fail(sparkle != 0, FALSE);

  if (sparkle->count == -1) {
    sparkle_destroy(sparkle);
    return FALSE;
  }

  gnome_canvas_item_set(sparkle->hline, "points", 
                        sparkle->hpoints[sparkle->count], NULL);

  gnome_canvas_item_set(sparkle->vline, "points", 
                        sparkle->vpoints[sparkle->count], NULL);

  gnome_canvas_item_set(sparkle->hsmall, "points", 
                        sparkle->hspoints[sparkle->count], NULL);

  gnome_canvas_item_set(sparkle->vsmall, "points", 
                        sparkle->vspoints[sparkle->count], NULL);

  
  if (sparkle->count == NUM_FRAMES-1) sparkle->up = FALSE;

  if (sparkle->up) ++(sparkle->count);
  else --(sparkle->count);
  
  return TRUE;
}

static void 
fill_points(GnomeCanvasPoints* points, double x, double y, double delta, 
            gboolean horizontal, gboolean square)
{
  if (horizontal) {
    if (square) {
      points->coords[0] = x - delta;
      points->coords[1] = y;
      points->coords[2] = x + delta;
      points->coords[3] = y;
    }
    else {
      points->coords[0] = x - delta;
      points->coords[1] = y - delta;
      points->coords[2] = x + delta;
      points->coords[3] = y + delta;
    }
  }
  else {
    if (square) {
      points->coords[0] = x;
      points->coords[1] = y - delta;
      points->coords[2] = x;
      points->coords[3] = y + delta;
    }
    else {
      points->coords[0] = x + delta;
      points->coords[1] = y - delta;
      points->coords[2] = x - delta;
      points->coords[3] = y + delta;
    }
  }  
}

#define DELTA 0.4

static void 
sparkle_new(GnomeCanvas* canvas, double x, double y)
{
  int i; double delta;
  Sparkle* sparkle = g_new(Sparkle,1);

  GnomeCanvasPoints* points = gnome_canvas_points_new(2);

  fill_points(points, x, y, 0.1, TRUE, TRUE);
  sparkle->hsmall = gnome_canvas_item_new(GNOME_CANVAS_GROUP(canvas->root),
                                         gnome_canvas_line_get_type(),
                                          "points", points,
                                          "fill_color", "light gray",
                                          "width_units", 1.0,
                                          NULL);

  gnome_canvas_item_raise_to_top(sparkle->hsmall);

  fill_points(points, x, y, 0.1, FALSE, TRUE);
  sparkle->vsmall = gnome_canvas_item_new(GNOME_CANVAS_GROUP(canvas->root),
                                          gnome_canvas_line_get_type(),
                                          "points", points,
                                          "fill_color", "light gray",
                                          "width_units", 1.0,
                                          NULL);

  gnome_canvas_item_raise_to_top(sparkle->vsmall);

  fill_points(points, x, y, DELTA, TRUE, TRUE);
  sparkle->hline = gnome_canvas_item_new(GNOME_CANVAS_GROUP(canvas->root),
                                         gnome_canvas_line_get_type(),
                                         "points", points,
                                         "fill_color", "white",
                                         "width_units", 1.0,
                                         NULL);

  fill_points(points, x, y, DELTA, FALSE, TRUE);
  sparkle->vline = gnome_canvas_item_new(GNOME_CANVAS_GROUP(canvas->root),
                                         gnome_canvas_line_get_type(),
                                         "points", points,
                                         "fill_color", "white",
                                         "width_units", 1.0,
                                         NULL);

  gnome_canvas_points_free(points);
  
  i = 0;
  delta = 0.0;
  while ( i < NUM_FRAMES ) {
    sparkle->hpoints[i] = gnome_canvas_points_new(2);
    sparkle->vpoints[i] = gnome_canvas_points_new(2);
    sparkle->hspoints[i] = gnome_canvas_points_new(2);
    sparkle->vspoints[i] = gnome_canvas_points_new(2);
    

    fill_points(sparkle->hspoints[i], x, y, delta, TRUE, FALSE);    
    fill_points(sparkle->vspoints[i], x, y, delta, FALSE, FALSE);    

    delta += DELTA;
    fill_points(sparkle->hpoints[i], x, y, delta, TRUE, TRUE);
    fill_points(sparkle->vpoints[i], x, y, delta + delta*.70, FALSE, TRUE);
    ++i;
  }

  sparkle->count = 0;
  sparkle->up = TRUE;
  
  gtk_timeout_add(75,(GtkFunction)sparkle_timeout, sparkle);
}


/**************************************
 Set up the GUI
  ******************************/

static gint new_sparkles_timeout(GnomeCanvas* canvas)
{
  /* These numbers are all dependent on the particular logo
     pixmap we're using. */
  static gint which_sparkle = 0;
  switch (which_sparkle) {
  case 0:
    sparkle_new(canvas,50.0,70.0);
    break;
  case 1: 
    sparkle_new(canvas,70.0,130.0);
    break;
  case 2:
    sparkle_new(canvas,100.0,37.0);
    break;
  case 3: 
    sparkle_new(canvas,120.0,110.0);
    break;
  case 4: 
    sparkle_new(canvas,140.0,120.0);
    break;
  case 5: 
    sparkle_new(canvas,110.0,160.0);
    break;
  default:
    which_sparkle = -1;
    break;
  };

  ++which_sparkle;
  return TRUE;
}

static void
free_imlib_image (GtkObject *object, gpointer data)
{
	gdk_imlib_destroy_image (data);
}

#endif

static gboolean
close_cb (GtkWidget * widget, gpointer user_data)
{
        gtk_main_quit ();
	return FALSE;
}

static void
help_cb (GtkWidget * widget, gpointer user_data)
{
    GnomeHelpMenuEntry ref = {"guname", "index.html"};
                        gnome_help_display (NULL, &ref);
}


static void popup_main_dialog()
{
  GtkWidget * d;
  GtkWidget * logo_box;
  GtkWidget * list_box;

  d = gnome_dialog_new( _("System Information"), 
                        GNOME_STOCK_BUTTON_OK,
                        GNOME_STOCK_BUTTON_HELP, 
                        NULL );
  /* we have loads of timeouts which would crash if the
   * window is ever destroyed */
  gnome_dialog_close_hides (GNOME_DIALOG (d), TRUE);
 
  logo_box = gtk_vbox_new(FALSE, GNOME_PAD_SMALL/2);
  do_logo_box(logo_box);

  list_box = gtk_vbox_new(TRUE, GNOME_PAD_SMALL/2);
  do_list_box(list_box);

  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(d)->vbox), logo_box,
                     FALSE, FALSE, GNOME_PAD_SMALL/2);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(d)->vbox), list_box,
                     TRUE, TRUE, GNOME_PAD_SMALL/2);

  gtk_signal_connect (GTK_OBJECT(d), "close", 
                       GTK_SIGNAL_FUNC(close_cb), &d); 

  gnome_dialog_button_connect (GNOME_DIALOG (d), 0,
                               GTK_SIGNAL_FUNC (close_cb), &d);
  gnome_dialog_button_connect (GNOME_DIALOG (d), 1,
                                     GTK_SIGNAL_FUNC (help_cb), &d);
  gnome_dialog_set_default (GNOME_DIALOG (d), 0);

  gtk_widget_show_all(d);
}

static void do_logo_box(GtkWidget * box)
{
  GtkWidget * hbox;
  GtkWidget * buttonbox;
  GtkWidget * button;
  GtkWidget * label, *contrib_label;
  GtkWidget * pixmap = NULL;
  GtkStyle * style;
  GdkFont * font;
  gchar * s;
  GtkWidget * marquee_frame, * align;
#ifdef WITH_SPARKLES
  GdkImlibImage *im;
  GtkWidget *image;
#endif

  hbox = gtk_hbox_new(FALSE,GNOME_PAD_SMALL);
  buttonbox = gtk_vbox_new(FALSE,GNOME_PAD_SMALL);
  gtk_container_border_width(GTK_CONTAINER(buttonbox), GNOME_PAD);

  s = gnome_pixmap_file ("gnome-logo-large.png");

  if (s == NULL) {
    /* Try this one. */
    s = gnome_pixmap_file ("gnome-default.png");
  }

#ifndef WITH_SPARKLES
  if (s) {
    pixmap = gnome_pixmap_new_from_file(s);
  }
#else  
  if (s) {
    pixmap = gnome_canvas_new();
    
	im = gdk_imlib_load_image (s);

    gtk_widget_set_usize (pixmap, im->rgb_width, im->rgb_height);
    gnome_canvas_set_scroll_region (GNOME_CANVAS (pixmap), 0, 0, 
                                    im->rgb_width, im->rgb_height);

	image = gnome_canvas_item_new (GNOME_CANVAS_GROUP(GNOME_CANVAS(pixmap)->root),
                                   gnome_canvas_image_get_type (),
                                   "image", im,
                                   "x", 0.0,
                                   "y", 0.0,
                                   "width", (double) im->rgb_width,
                                   "height", (double) im->rgb_height,
                                   "anchor", GTK_ANCHOR_NW,
                                   NULL);

	gtk_signal_connect (GTK_OBJECT (image), "destroy",
                        (GtkSignalFunc) free_imlib_image,
                        im);

    g_free(s);
    
    gtk_timeout_add(1300,(GtkFunction)new_sparkles_timeout,pixmap);
  }
#endif

  /* Also covers case where gnome_pixmap_new fails? */
  if ( pixmap == NULL ) {
    pixmap = gtk_label_new(_("Logo file not found"));
  }

  /* Up here so it uses the default font */
  contrib_label = gtk_label_new(_("GNOME contributors:"));

  style = gtk_style_new ();
  /* fontset used by guname; adapt it if your language requires it */
  font = gdk_fontset_load (_("-Adobe-Helvetica-Medium-R-Normal--*-180-*-*-*-*-*-*"));

  if (font) {
    gdk_font_unref (style->font);
    style->font = font;
  }

  if (style->font) {
    gtk_widget_push_style (style);
  }

  label = gtk_label_new(_("GNOME: The GNU Network Object Model Environment"));

  gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, GNOME_PAD_SMALL/2);
  gtk_box_pack_start(GTK_BOX(box), hbox, TRUE, TRUE, GNOME_PAD_SMALL/2);
  gtk_box_pack_start(GTK_BOX(hbox), pixmap, TRUE, TRUE, GNOME_PAD_SMALL/2);
  gtk_box_pack_end(GTK_BOX(hbox), buttonbox, TRUE, TRUE, GNOME_PAD_SMALL/2);

  if (style->font) {
    gtk_widget_pop_style();
  }
  else gtk_style_unref(style);

  marquee_frame = gtk_frame_new(NULL);
  gtk_frame_set_shadow_type(GTK_FRAME(marquee_frame), GTK_SHADOW_IN);
  do_marquee(marquee_frame);
  align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
  gtk_container_add(GTK_CONTAINER(align), marquee_frame);

  gtk_box_pack_start(GTK_BOX(buttonbox), contrib_label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(buttonbox), align, FALSE, FALSE, 0);

  /* FIXME this is a lazy cut-and-paste fest should be a function */
  button = gtk_button_new();
  pixmap = gnome_stock_pixmap_widget(button,GNOME_STOCK_PIXMAP_MAIL_SND);
  label = gtk_label_new(_("Email Information..."));

  hbox  = gtk_hbox_new(FALSE, GNOME_PAD_SMALL/2);

  gtk_box_pack_start(GTK_BOX(hbox),pixmap,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_container_add(GTK_CONTAINER(button),hbox);
  gtk_box_pack_end(GTK_BOX(buttonbox), button, FALSE, FALSE, GNOME_PAD);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(mail_callback), NULL);

  button = gtk_button_new();
  pixmap = gnome_stock_pixmap_widget(button,GNOME_STOCK_PIXMAP_SAVE_AS);
  label = gtk_label_new(_("Save Information to File..."));
  hbox  = gtk_hbox_new(FALSE, GNOME_PAD_SMALL / 2);
  gtk_box_pack_start(GTK_BOX(hbox),pixmap,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_container_add(GTK_CONTAINER(button),hbox);
  gtk_box_pack_end(GTK_BOX(buttonbox), button, FALSE, FALSE, GNOME_PAD);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(save_callback), NULL);

#if HAVE_LIBGTOP
  button = gtk_button_new();
  pixmap = gnome_stock_pixmap_widget(button,GNOME_STOCK_PIXMAP_BOOK_OPEN);
  label = gtk_label_new(_("Detailed Information..."));
  hbox  = gtk_hbox_new(FALSE, GNOME_PAD_SMALL/2);
  gtk_box_pack_start(GTK_BOX(hbox),pixmap,FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(hbox),label,TRUE,TRUE,0);
  gtk_container_add(GTK_CONTAINER(button),hbox);
  gtk_box_pack_end(GTK_BOX(buttonbox), button, FALSE, FALSE, GNOME_PAD);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
                     GTK_SIGNAL_FUNC(detailed_callback), NULL);
#endif
}

static void do_list_box(GtkWidget * box)
{
  GtkCList * list;
  GtkWidget *sw;
  const gchar * titles[] = { N_("Category"), N_("Your System") };
  gint width, height;

  titles[0]=_(titles[0]);
  titles[1]=_(titles[1]);

  list = GTK_CLIST(create_clist(titles));
  sw = gtk_scrolled_window_new (NULL, NULL);
  
  gtk_clist_freeze(list); /* does this matter if we haven't shown yet? */

  gtk_clist_set_shadow_type(list, GTK_SHADOW_IN);
  /* Fixme, eventually you might could select an item 
     for cut and paste, or some other effect. */
  gtk_clist_set_selection_mode(list, GTK_SELECTION_BROWSE);

  gtk_container_add (GTK_CONTAINER (sw), GTK_WIDGET (list));
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_widget_show (GTK_WIDGET (list));
  gtk_clist_column_titles_passive(list);

  do_popup(GTK_WIDGET(sw));
  gtk_widget_set_events(GTK_WIDGET(list), GDK_BUTTON_PRESS_MASK);
  gtk_signal_connect(GTK_OBJECT(list), "button_press_event",
                     GTK_SIGNAL_FUNC(list_clicked_cb), NULL);

  fill_clist(list, descriptions, info, end_system_info, &width, &height);

  gtk_clist_thaw(list);

  gtk_widget_set_usize(GTK_WIDGET(sw), width, height);

  gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(sw));
}

static void do_popup(GtkWidget * clist)
{
  GtkWidget * mi;

  popup = gtk_menu_new();
    
  mi = gnome_stock_menu_item(GNOME_STOCK_MENU_SAVE_AS,_("Save As..."));
  gtk_signal_connect(GTK_OBJECT(mi), "activate",
                     GTK_SIGNAL_FUNC(save_callback), NULL);
  gtk_menu_append(GTK_MENU(popup), mi);
  gtk_widget_show(mi);

  mi = gnome_stock_menu_item(GNOME_STOCK_MENU_MAIL_SND,_("Mail To..."));
  gtk_signal_connect(GTK_OBJECT(mi), "activate",
                     GTK_SIGNAL_FUNC(mail_callback), NULL);
  gtk_menu_append(GTK_MENU(popup), mi);
  gtk_widget_show(mi);  

#if HAVE_LIBGTOP
  mi = gnome_stock_menu_item(GNOME_STOCK_MENU_BLANK,
                             _("Detailed Information..."));
  gtk_signal_connect(GTK_OBJECT(mi), "activate",
                     GTK_SIGNAL_FUNC(detailed_callback), NULL);
  gtk_menu_append(GTK_MENU(popup), mi);
  gtk_widget_show(mi);  
#endif
}

/**********************************
  Callbacks
  *******************************/
static gint list_clicked_cb(GtkCList * list, GdkEventButton * e)
{
  if (e->button == 1) {
    /* Ignore button 1 */
    return FALSE; 
  }

  /* don't change the CList selection. */
  gtk_signal_emit_stop_by_name (GTK_OBJECT (list), "button_press_event");

  gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL,
                 NULL, e->button, time(NULL));
  return TRUE; 
}

static void file_selection_cb(GtkWidget * button, gpointer fs)
{
  gchar * fn;
  FILE * f;

  fn = gtk_file_selection_get_filename(GTK_FILE_SELECTION(fs));

  if (fn) {
    f = fopen(fn, "w");
    if (f) {
      gtk_widget_hide(GTK_WIDGET(fs));
      write_to_filestream(f);
      if ( fclose(f) != 0 ) {
        gchar * t = 
          g_strdup_printf(_("Error closing file `%s': \n"
                         "%s\n"
                         "Some or all data may not have been written."),
                         fn,
                         g_unix_error_string(errno));
        gnome_error_dialog(t);
        g_free(t);
      }
      gtk_widget_destroy(GTK_WIDGET(fs));
      return;
    }
    else {
      gchar * s = g_strdup_printf(_("Couldn't open file `%s': %s"),
                                 fn,
                                 g_unix_error_string(errno));
      gnome_error_dialog(s);
      g_free(s);
    }
  }
  /* I think this frees fn */
  gtk_widget_destroy(GTK_WIDGET(fs));
}

static void save_callback(GtkWidget * menuitem, gpointer data)
{
  static GtkWidget * fs = NULL;

  if (fs != NULL)
  {
  	gdk_window_show(fs->window);
	gdk_window_raise(fs->window);
	return;
  }

  fs = gtk_file_selection_new(_("Save System Information As..."));
  gnome_window_icon_set_from_default (GTK_WINDOW (fs));
  gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(fs)->ok_button), "clicked",
                     GTK_SIGNAL_FUNC(file_selection_cb), fs);

  gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(fs)->cancel_button), "clicked",
                            GTK_SIGNAL_FUNC(gtk_widget_destroy), GTK_OBJECT(fs));
  /* Mark the dialog as destroyed when it is destroyed so we don't try to show it again */
  gtk_signal_connect(GTK_OBJECT(fs), "destroy",
		     GTK_SIGNAL_FUNC(gtk_widget_destroyed), &fs);
  
  gtk_widget_show(fs);
}

struct MailData {
  gchar* to;
  gchar* body;
};

static gint mail_closed_callback(GtkWidget* dialog,
                                  struct MailData* data)
{
  g_free(data->to);
  g_free(data->body);
  g_free(data);
  return FALSE;
}

static void mail_clicked_callback(GtkWidget* dialog, gint button,
                                  struct MailData* data)
{
  if (button == GNOME_YES) {
    FILE * p;
    gchar * command;
    gint failure;  
    
    /* This isn't translated; maybe good, since most tech support email should be
       in English? don't know. */
    if (gnome_is_program_in_path("mailx"))

        command = g_strconcat("mailx -s \"System Information for host ", 
                             info[si_host] ? info[si_host] : "Unknown",
                             "\" ", data->to, NULL);
    
    else if (gnome_is_program_in_path("mail"))
    
        command = g_strconcat("mail -s \"System Information for host ", 
                             info[si_host] ? info[si_host] : "Unknown",
                             "\" ", data->to, NULL);
    
    p = popen(command, "w");
    
    if (p) {
      write_to_filestream(p);
      failure = pclose(p);
      if (failure) {
        /* I don't think the error_string() will reliably mean anything. */
        gchar * s = g_strdup_printf(_("Command failed ` %s ': %s"),
                                   command, 
                                   g_unix_error_string(errno));
        gnome_error_dialog(s);
        g_free(s);
      }
    }
    else {
      gchar * t = g_strdup_printf(_("Couldn't run command ` %s ': %s"), 
                                 command,
                                 g_unix_error_string(errno));
      gnome_error_dialog(t);
      g_free(t);
    }
    g_free(command);
  }
  else if (button == GNOME_NO) {
    ;
  }
  else {
    g_assert_not_reached();
  }
  /* Data is freed in the close callback */
}

static void confirm_mail(struct MailData* data)
{
  GtkWidget* dialog;
  GtkWidget* less;
  GtkWidget* label;
  gchar* question;

  question = g_strdup_printf(_("The following mail will be sent to %s.\n"
                            " Are you sure you want to mail this information?"),
                            data->to);

  label = gtk_label_new(question);
  less = gnome_less_new();
  /* Stupid hack, set_usize. gnome_less needs fixing */
  gtk_widget_set_usize(less,400,-1);
  gnome_less_set_fixed_font(GNOME_LESS(less),TRUE);
  gnome_less_show_string(GNOME_LESS(less),data->body);

  dialog = gnome_dialog_new(_("Are you sure?"), GNOME_STOCK_BUTTON_YES,
                            GNOME_STOCK_BUTTON_NO, NULL);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), label,
                     FALSE,FALSE,GNOME_PAD_SMALL);
  gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), less,
                     TRUE,TRUE,GNOME_PAD_SMALL);
  gnome_dialog_set_close(GNOME_DIALOG(dialog),TRUE);
  gtk_window_set_policy(GTK_WINDOW(dialog),TRUE,TRUE,FALSE);

  gtk_signal_connect(GTK_OBJECT(dialog),"clicked",
                     GTK_SIGNAL_FUNC(mail_clicked_callback),
                     data);
  gtk_signal_connect(GTK_OBJECT(dialog), "close",
                     GTK_SIGNAL_FUNC(mail_closed_callback),
                     data);

  gtk_widget_show_all(dialog);
}

static void mailx_callback(gchar * string, gpointer data)
{
  struct MailData* md;

  if (string == NULL) return;

  md = g_new(struct MailData, 1);
  md->to = string;
  md->body = write_to_string();
  confirm_mail(md);
}

static void mail_callback(GtkWidget * menuitem, gpointer data)
{
  static GtkWidget *mx = NULL;

  if (mx != NULL)
  {
  	gdk_window_show(mx->window);
	gdk_window_raise(mx->window);
	return;
  }
  mx = gnome_request_dialog(FALSE,
                       _("Address to mail to:"),
                       NULL,
                       256,
                       mailx_callback,
                       NULL,
                       NULL);
  gtk_signal_connect(GTK_OBJECT(mx), "destroy",
		     GTK_SIGNAL_FUNC(gtk_widget_destroyed), &mx);
}

static void detailed_callback(GtkWidget * menuitem, gpointer data)
{
#if HAVE_LIBGTOP
  display_moreinfo();
#endif
}

/*********************************************
  Non-GUI
  ***********************/

static void write_to_filestream(FILE * f)
{
  gint i = 0;
  while ( i < end_system_info ) {
    if (info[i] == NULL) {
      /* No information on this. */
      ;
    }
    else {
      fprintf (f, "%-30s %s\n", _(descriptions[i]), info[i]);
    }
    ++i;
  }  
}

static gchar* write_to_string()
{
  gchar* final = NULL;
  gchar* tmp  = NULL;
  gint i = 0;
  while ( i < end_system_info ) {
    if (info[i] == NULL) {
      /* No information on this. */
      ;
    }
    else {
      gchar buf[200];
      g_snprintf (buf, 200, "%-30s %s\n", _(descriptions[i]), info[i]);
      tmp = g_strconcat(final ? final : "", buf, NULL);
      g_free(final);
      final = tmp;
    }
    ++i;
  }  
  return final;
}

/***************************** 
  Marquee thingy - probably there should be a widget to do this.  Code
  taken from the GIMP, about_dialog.c, Copyright Spencer Kimball and
  Peter Mattis.
  ***********************************/

static const gchar * scroll_text[] = {
#include "AUTHORS.h"
  "Wanda the Benevolent Fish"
};

static int nscroll_texts = sizeof (scroll_text) / sizeof (scroll_text[0]);
/* Add 7 to the size for Gnome 1.0 luck! */
static int scroll_text_widths[sizeof(scroll_text)/sizeof(scroll_text[0])+7] = { 0 };
static int cur_scroll_text = 0;
static int cur_scroll_index = 0;

static int shuffle_array[ sizeof(scroll_text) / sizeof(scroll_text[0]) ];

static GtkWidget *scroll_area = NULL;
static GdkPixmap *scroll_pixmap = NULL;
static int do_scrolling = TRUE;
static int scroll_state = 0;
static int offset = 0;
static int timer = 0;

static gint marquee_expose (GtkWidget * widget, GdkEventExpose * event, gpointer data)
{
  if (!timer) {
    marquee_timer(NULL);
    timer = gtk_timeout_add (75, marquee_timer, NULL);
  }
  return FALSE;
}

static void do_marquee(GtkWidget * box)
{
  int i; 
  int max_width;

  max_width = 0;
  for (i = 0; i < nscroll_texts; i++) {
    scroll_text_widths[i] = gdk_string_width (box->style->font, scroll_text[i]);
    max_width = MAX (max_width, scroll_text_widths[i]);
  }
  
  scroll_area = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (scroll_area),
                         max_width + 10,
                         box->style->font->ascent +
                         box->style->font->descent + 5 ); /* 5 is for border. */
  gtk_container_add (GTK_CONTAINER (box), scroll_area);

  for (i = 0; i < nscroll_texts; i++) {
    shuffle_array[i] = i;
  }
  
  for (i = 0; i < nscroll_texts; i++) {
    int j, k;
    j = rand() % nscroll_texts;
    k = rand() % nscroll_texts;
    if (j != k) 
      {
        int t;
        t = shuffle_array[j];
        shuffle_array[j] = shuffle_array[k];
        shuffle_array[k] = t;
      }
  }

  gtk_widget_set_events(scroll_area, GDK_EXPOSURE_MASK);
  gtk_signal_connect(GTK_OBJECT(scroll_area), "expose_event",
                     GTK_SIGNAL_FUNC(marquee_expose), NULL);
}

static int marquee_timer (gpointer data)
{
  gint return_val;

  return_val = TRUE;

  if (do_scrolling)
    {
      if (!scroll_pixmap)
        scroll_pixmap = gdk_pixmap_new (scroll_area->window,
                                        scroll_area->allocation.width,
                                        scroll_area->allocation.height,
                                        -1);
  
      switch (scroll_state)
        {
        case 1:
          scroll_state = 2;
          timer = gtk_timeout_add (400, marquee_timer, NULL);
          return_val = FALSE;
          break;
        case 2:
          scroll_state = 3;
          timer = gtk_timeout_add (75, marquee_timer, NULL);
          return_val = FALSE;
          break;
        }

      if (offset > (scroll_text_widths[cur_scroll_text] + scroll_area->allocation.width))
        {
          scroll_state = 0;
          cur_scroll_index += 1;
          if (cur_scroll_index == nscroll_texts)
            cur_scroll_index = 0;
	  
          cur_scroll_text = shuffle_array[cur_scroll_index];

          offset = 0;
        }

      gdk_draw_rectangle (scroll_pixmap,
                          scroll_area->style->white_gc,
                          TRUE, 0, 0,
                          scroll_area->allocation.width,
                          scroll_area->allocation.height);
      gdk_draw_string (scroll_pixmap,
                       scroll_area->style->font,
                       scroll_area->style->black_gc,
                       scroll_area->allocation.width - offset,
                       scroll_area->style->font->ascent + 2, /* 2 from top */
                       scroll_text[cur_scroll_text]);
      gdk_draw_pixmap (scroll_area->window,
                       scroll_area->style->black_gc,
                       scroll_pixmap, 0, 0, 0, 0,
                       scroll_area->allocation.width,
                       scroll_area->allocation.height);

      offset += 15;
      if (scroll_state == 0)
        {
          if (offset > ((scroll_area->allocation.width + scroll_text_widths[cur_scroll_text]) / 2))
            {
              scroll_state = 1;
              offset = (scroll_area->allocation.width + scroll_text_widths[cur_scroll_text]) / 2;
            }
        }
    }

  return return_val;
}
