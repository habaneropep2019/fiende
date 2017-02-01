/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 * Gnome-Mahjongg
 * (C) 1998-1999 the Free Software Foundation
 *
 *
 * Author: Francisco Bustamante
 *
 *
 * http://www.nuclecu.unam.mx/~pancho/
 * pancho@nuclecu.unam.mx
 *
 */

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <config.h>

#include <gtk/gtk.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk_imlib.h>

#include "mahjongg.h"
#include "solubility.h"

#define APPNAME "mahjongg"
#define APPNAME_LONG "Gnome Mahjongg"
/*
#define MAH_VERSION "0.99.2+"
*/
#define MAH_VERSION VERSION

#define AREA_WIDTH 600
#define AREA_HEIGHT 470
#define TILE_WIDTH 40
#define TILE_HEIGHT 56
#define HALF_WIDTH 18
#define HALF_HEIGHT 26
#define THICKNESS 5

/* #defines for the tile selection code. */
#define SELECTED_FLAG   1
#define HINT_FLAG       16

#define ELEMENTS(x) (sizeof (x) / sizeof (x [0]))

/* The number of half-cycles to blink during a hint (less 1) */
#define HINT_BLINK_NUM 5

/* Sorted such that the bottom leftest are first, and layers decrease
 * Bottom left = high y, low x ! */

tilepos easy_map [MAX_TILES] = {
 {13, 7,  4}, {12, 8,  3}, {14, 8,  3}, {12, 6,  3},
 {14, 6,  3}, {10, 10,  2}, {12, 10,  2}, {14, 10,  2},
 {16, 10,  2}, {10, 8,  2}, {12, 8,  2}, {14, 8,  2},
 {16, 8,  2}, {10, 6,  2}, {12, 6,  2}, {14, 6,  2},
 {16, 6,  2}, {10, 4,  2}, {12, 4,  2}, {14, 4,  2},
 {16, 4,  2}, {8, 12,  1}, {10, 12,  1}, {12, 12,  1},
 {14, 12,  1}, {16, 12,  1}, {18, 12,  1}, {8, 10,  1},
 {10, 10,  1}, {12, 10,  1}, {14, 10,  1}, {16, 10,  1}, 
 {18, 10,  1}, {8, 8,  1}, {10, 8,  1}, {12, 8,  1},
 {14, 8,  1}, {16, 8,  1}, {18, 8,  1}, {8, 6,  1},
 {10, 6,  1}, {12, 6,  1}, {14, 6,  1}, {16, 6,  1},
 {18, 6,  1}, {8, 4,  1}, {10, 4,  1}, {12, 4,  1},
 {14, 4,  1}, {16, 4,  1}, {18, 4,  1}, {8, 2,  1},
 {10, 2,  1}, {12, 2,  1}, {14, 2,  1}, {16, 2,  1},
 {18, 2,  1}, {2, 14,  0}, {4, 14,  0}, {6, 14,  0},
 {8, 14,  0}, {10, 14,  0}, {12, 14,  0}, {14, 14,  0}, 
 {16, 14,  0}, {18, 14,  0}, {20, 14,  0}, {22, 14,  0},
 {24, 14,  0}, {6, 12,  0}, {8, 12,  0}, {10, 12,  0},
 {12, 12,  0}, {14, 12,  0}, {16, 12,  0}, {18, 12,  0},
 {20, 12,  0}, {4, 10,  0}, {6, 10,  0}, {8, 10,  0},
 {10, 10,  0}, {12, 10,  0}, {14, 10,  0}, {16, 10,  0},
 {18, 10,  0}, {20, 10,  0}, {22, 10,  0}, {0, 7,  0},
 {2, 8,  0}, {4, 8,  0}, {6, 8,  0}, {8, 8,  0},
 {10, 8,  0}, {12, 8,  0}, {14, 8,  0}, {16, 8,  0},
 {18, 8,  0}, {20, 8,  0}, {22, 8,  0}, {24, 8,  0},
 {2, 6,  0}, {4, 6,  0}, {6, 6,  0}, {8, 6,  0},
 {10, 6,  0}, {12, 6,  0}, {14, 6,  0}, {16, 6,  0},
 {18, 6,  0}, {20, 6,  0}, {22, 6,  0}, {24, 6,  0},
 {4, 4,  0}, {6, 4,  0}, {8, 4,  0}, {10, 4,  0},
 {12, 4,  0}, {14, 4,  0}, {16, 4,  0}, {18, 4,  0},
 {20, 4,  0}, {22, 4,  0}, {6, 2,  0}, {8, 2,  0},
 {10, 2,  0}, {12, 2,  0}, {14, 2,  0}, {16, 2,  0},
 {18, 2,  0}, {20, 2,  0}, {2, 0,  0}, {4, 0,  0},
 {6, 0,  0}, {8, 0,  0}, {10, 0,  0}, {12, 0,  0},
 {14, 0,  0}, {16, 0,  0}, {18, 0,  0}, {20, 0,  0},
 {22, 0,  0}, {24, 0,  0}, {26, 7, 0}, {28, 7, 0} 
};

tilepos hard_map [MAX_TILES] = {
	{ 10, 6,  6},
	{ 9, 6,  5},
	{ 11, 6,  5},
	{ 8, 6,  4},
	{ 10, 6,  4},
	{ 12, 6,  4},
	{ 5, 6,  3},
	{ 7, 7,  3},
	{ 7, 5,  3},
	{ 9, 7,  3},
	{ 9, 5,  3},
	{ 11, 7,  3},
	{ 11, 5,  3},
	{ 13, 7,  3},
	{ 13, 5,  3},
	{ 15, 6,  3},
	{ 5, 8,  2},
	{ 7, 8,  2},
	{ 9, 8,  2},
	{ 11, 8,  2},
	{ 13, 8,  2},
	{ 15, 8,  2},
	{ 4, 6,  2},
	{ 6, 6,  2},
	{ 8, 6,  2},
	{ 10, 6,  2},
	{ 12, 6,  2},
	{ 14, 6,  2},
	{ 16, 6,  2},
	{ 5, 4,  2},
	{ 7, 4,  2},
	{ 9, 4,  2},
	{ 11, 4,  2},
	{ 13, 4,  2},
	{ 15, 4,  2},
	{ 7, 12,  1},
	{ 9, 11,  1},
	{ 11, 11,  1},
	{ 13, 12,  1},
	{ 2, 10,  1},
	{ 4, 10,  1},
	{ 6, 10,  1},
	{ 8, 9,  1},
	{ 10, 9,  1},
	{ 12, 9,  1},
	{ 14, 10,  1},
	{ 16, 10,  1},
	{ 18, 10,  1},
	{ 3, 8,  1},
	{ 3, 6,  1},
	{ 5, 8,  1},
	{ 5, 6,  1},
	{ 7, 7,  1},
	{ 9, 7,  1},
	{ 11, 7,  1},
	{ 13, 7,  1},
	{ 15, 8,  1},
	{ 17, 8,  1},
	{ 3, 4,  1},
	{ 5, 4,  1},
	{ 7, 5,  1},
	{ 9, 5,  1},
	{ 11, 5,  1},
	{ 13, 5,  1},
	{ 15, 6,  1},
	{ 17, 6,  1},
	{ 2, 2,  1},
	{ 4, 2,  1},
	{ 6, 2,  1},
	{ 8, 3,  1},
	{ 10, 3,  1},
	{ 12, 3,  1},
	{ 15, 4,  1},
	{ 17, 4,  1},
	{ 7, 0,  1},
	{ 9, 1,  1},
	{ 11, 1,  1},
	{ 14, 2,  1},
	{ 16, 2,  1},
	{ 18, 2,  1},
	{ 13, 0,  1},
	{ 6, 12,  0},
	{ 8, 12,  0},
	{ 10, 12,  0},
	{ 12, 12,  0},
	{ 14, 12,  0},
	{ 1, 11,  0},
	{ 3, 11,  0},
	{ 1, 9,  0},
	{ 0, 6,  0},
	{ 3, 9,  0},
	{ 5, 10,  0},
	{ 7, 10,  0},
	{ 9, 10,  0},
	{ 11, 10,  0},
	{ 13, 10,  0},
	{ 15, 10,  0},
	{ 17, 11,  0},
	{ 19, 11,  0},
	{ 2, 7,  0},
	{ 4, 7,  0},
	{ 6, 8,  0},
	{ 8, 8,  0},
	{ 2, 5,  0},
	{ 4, 5,  0},
	{ 6, 6,  0},
	{ 8, 6,  0},
	{ 10, 8,  0},
	{ 10, 6,  0},
	{ 12, 8,  0},
	{ 12, 6,  0},
	{ 14, 8,  0},
	{ 14, 6,  0},
	{ 17, 9,  0},
	{ 16, 7,  0},
	{ 19, 9,  0},
	{ 18, 7,  0},
	{ 1, 3,  0},
	{ 3, 3,  0},
	{ 6, 4,  0},
	{ 8, 4,  0},
	{ 10, 4,  0},
	{ 12, 4,  0},
	{ 14, 4,  0},
	{ 16, 5,  0},
	{ 18, 5,  0},
	{ 20, 6,  0},
	{ 1, 1,  0},
	{ 3, 1,  0},
	{ 5, 2,  0},
	{ 7, 2,  0},
	{ 9, 2,  0},
	{ 11, 2,  0},
	{ 13, 2,  0},
	{ 15, 2,  0},
	{ 17, 3,  0},
	{ 19, 3,  0},
	{ 17, 1,  0},
	{ 19, 1,  0},
	{ 6, 0,  0},
	{ 8, 0,  0},
	{ 10, 0,  0},
	{ 12, 0,  0},
	{ 14, 0,  0}
};

tilepos *pos = 0;
int xpos_offset;
int ypos_offset;

tile tiles[MAX_TILES];

GtkWidget *window, *pref_dialog, *appbar;
#ifdef NEED_UNUSED_CODE
GtkWidget *hint_dialog;
#endif
GtkWidget *mbox;
GtkWidget *canvas;
GtkWidget *tiles_label;
int selected_tile, visible_tiles;
int sequence_number;

static GdkImlibImage *tiles_image, *bg_image;
static gchar *tileset = 0 ;

static struct {
  char *tileset;
  int make_it_default;
} selected_tileset = {0,0};

static struct {
  char *bg;
  int make_it_default;
} selected_bg = {0,0};

static gchar *mapset = 0 ;

static struct {
  char *mapset ;
  int make_it_default;
} selected_mapset = {0,0} ;

static struct {
  GdkColor colour ;
  char *name ;
  int set;
  int make_it_default;
} backgnd = {{0,0,0,0},NULL,0,0} ;

struct _maps
{
  char *name ;
  tilepos *map ;
  int make_it_default ;
} maps[] = { { "easy",      easy_map },
	     { "difficult", hard_map } } ;


gint hint_tiles[2];
guint timer;
guint timeout_counter = HINT_BLINK_NUM + 1;

GtkWidget *moves_label;
int moves_left=0;
GtkWidget *chrono_label,*chrono_box;
GtkWidget *chrono;
gint paused=0;

typedef struct {
  int popup;
  int new;
  int set;
} popup_type;

static struct {
        popup_type warn;
        popup_type confirm;
} popup_config = {{1,0,0}, {1,0,0}};

typedef enum { NEW_GAME, RESTART_GAME, SELECT_GAME, QUIT_GAME } game_state;

enum { GAME_RUNNING, GAME_WON, GAME_LOST, GAME_DEAD } game_over;

void clear_undo_queue ();
void you_won (void);
void no_match (void);
void check_free (void);
void set_map (char *name) ;
void load_tiles (char *fname, char *bg_fname);
void new_game_reply_callback (gint reply, gpointer data);
void undo_tile_callback    (GtkWidget *widget, gpointer data);
void redo_tile_callback    (GtkWidget *widget, gpointer data);
void hint_callback         (GtkWidget *widget, gpointer data);
void properties_callback   (GtkWidget *widget, gpointer data);
void about_callback        (GtkWidget *widget, gpointer data);
void show_tb_callback      (GtkWidget *widget, gpointer data);
void sound_on_callback     (GtkWidget *widget, gpointer data);
void scores_callback       (GtkWidget *widget, gpointer data);
void exit_game_callback_query (GtkWidget *widget, gboolean *exit, gpointer data);
void exit_game_callback       (GtkWidget *widget, gpointer data);
void shuffle_tiles_callback   (GtkWidget *widget, gpointer data);
void pause_callback (void);
void new_game (void);
void restart_game (void);
void select_game (void);

GnomeUIInfo gamemenu [] = {
         GNOMEUIINFO_MENU_NEW_GAME_ITEM(exit_game_callback, NEW_GAME),

         {GNOME_APP_UI_ITEM, N_("New game with _seed..."), N_("Start a new game giving a seed number..."),
	  exit_game_callback, (gpointer)SELECT_GAME, NULL, GNOME_APP_PIXMAP_STOCK,
	  GNOME_STOCK_MENU_NEW, 0, 0, NULL},

	 GNOMEUIINFO_MENU_RESTART_GAME_ITEM(exit_game_callback, RESTART_GAME),

	 GNOMEUIINFO_SEPARATOR,

	 GNOMEUIINFO_MENU_UNDO_MOVE_ITEM(undo_tile_callback, NULL),
	 GNOMEUIINFO_MENU_REDO_MOVE_ITEM(redo_tile_callback, NULL),

	 GNOMEUIINFO_MENU_HINT_ITEM(hint_callback, NULL),

         {GNOME_APP_UI_ITEM, N_("Shu_ffle tiles"), N_("Shuffle tiles"),
	  shuffle_tiles_callback, NULL, NULL, GNOME_APP_PIXMAP_STOCK,
	  GNOME_STOCK_MENU_BLANK, 0, 0, NULL},

	 GNOMEUIINFO_SEPARATOR,

         GNOMEUIINFO_MENU_SCORES_ITEM(scores_callback, NULL),

	 GNOMEUIINFO_SEPARATOR,
         GNOMEUIINFO_MENU_EXIT_ITEM(exit_game_callback, QUIT_GAME),

	 GNOMEUIINFO_END
};

GnomeUIInfo settingsmenu [] = {
        GNOMEUIINFO_TOGGLEITEM(N_("Show _Tool Bar"),
			       N_("Toggle display of the toolbar"),
			       show_tb_callback, NULL),

	GNOMEUIINFO_SEPARATOR,

#ifdef SOUND_SUPPORT_FINISHED
        {GNOME_APP_UI_TOGGLEITEM, N_("_Sound"), NULL, NULL, NULL, NULL,
        GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
#endif

	GNOMEUIINFO_MENU_PREFERENCES_ITEM(properties_callback, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo helpmenu[] = {
        GNOMEUIINFO_HELP("mahjongg"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about_callback, NULL),
	GNOMEUIINFO_END

};

GnomeUIInfo mainmenu [] = {
	GNOMEUIINFO_MENU_GAME_TREE(gamemenu),
	GNOMEUIINFO_MENU_SETTINGS_TREE(settingsmenu),
	GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
	GNOMEUIINFO_END
};

GnomeUIInfo toolbar_uiinfo [] = {
	{GNOME_APP_UI_ITEM, N_("New"), NULL, exit_game_callback, (gpointer)NEW_GAME, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL},

/*	{GNOME_APP_UI_ITEM, N_("Seed"), NULL, exit_game_callback, (gpointer)SELECT_GAME, NULL,
        GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL},*/

        {GNOME_APP_UI_ITEM, N_("Restart"), NULL, exit_game_callback, (gpointer)RESTART_GAME, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_REFRESH, 0, 0, NULL},
        
        {GNOME_APP_UI_ITEM, N_("Hint"), NULL, hint_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_HELP, GDK_H, GDK_CONTROL_MASK, NULL},

        {GNOME_APP_UI_ITEM, N_("Undo"), NULL, undo_tile_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_UNDO, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Redo"), NULL, redo_tile_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_REDO, 0, 0, NULL},

        /* If you change the place for this button, change the index in 
           the definition of PAUSE_BUTTON below */
        {GNOME_APP_UI_TOGGLEITEM, N_("Pause"), NULL, pause_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_TIMER, 0, 0, NULL},

#ifdef SOUND_SUPPORT_FINISHED
        {GNOME_APP_UI_TOGGLEITEM, N_("Sound"), NULL, sound_on_callback, NULL, NULL,
         GNOME_APP_PIXMAP_DATA, mini_sound_xpm, 0, 0, NULL},
#endif

        {GNOME_APP_UI_ITEM, N_("Shuffle"), NULL, shuffle_tiles_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_MULTIPLE, 0, 0, NULL},

        {GNOME_APP_UI_ITEM, N_("Prefs"), NULL, properties_callback, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_PREFERENCES, 0, 0, NULL},

        {GNOME_APP_UI_SEPARATOR},

	{GNOME_APP_UI_ENDOFINFO}
};

#define PAUSE_BUTTON GTK_TOGGLE_BUTTON(toolbar_uiinfo[5].widget)

#ifdef NEED_UNUSED_CODE
static void
hint_cancel (GtkWidget *widget, void *data)
{
  int lp ;
  for (lp=0;lp<MAX_TILES;lp++)
    if (tiles[lp].selected == 17)
      {
	tiles[lp].selected = 0 ;
/* 	redraw_tile(lp) ; */
      }

  gtk_widget_destroy (hint_dialog);
  hint_dialog = 0;
}
#endif

static void
set_tile_selection (GtkWidget *widget, void *data)
{
	selected_tileset.tileset = data;
}

static void
set_bg_selection (GtkWidget *widget, void *data)
{
	selected_bg.bg = data;
}

static void
set_tile_selection_def (GtkWidget *widget, gpointer *data)
{
	selected_tileset.make_it_default = GTK_TOGGLE_BUTTON (widget)->active;
}

static void
set_bg_selection_def (GtkWidget *widget, gpointer *data)
{
        selected_bg.make_it_default = GTK_TOGGLE_BUTTON (widget)->active;
}

static void
set_map_selection (GtkWidget *widget, void *data)
{
        struct _maps *map = (struct _maps*) data;
        selected_mapset.mapset = map->name ;
}

static void
set_map_selection_def (GtkWidget *widget, gpointer *data)
{
	selected_mapset.make_it_default = GTK_TOGGLE_BUTTON (widget)->active;
}

static void
set_backgnd_selection_def (GtkWidget *widget, gpointer *data)
{
	backgnd.make_it_default = GTK_TOGGLE_BUTTON (widget)->active;
}

set_popup (popup_type *popup, gint i)
{
        popup->popup = i;
}

set_popup_def (GtkWidget *widget, popup_type *popup)
{
        popup->new = GTK_TOGGLE_BUTTON (widget)->active;
        popup->set = 1;
}

static void
free_str (GtkWidget *widget, void *data)
{
	free (data);
}

void message (gchar *message)
{
  gnome_appbar_pop (GNOME_APPBAR (appbar));
  gnome_appbar_push (GNOME_APPBAR (appbar), message);
}

void chrono_start ()
{
  gtk_clock_stop (GTK_CLOCK (chrono));
  gtk_clock_set_seconds (GTK_CLOCK (chrono), 0);
  gtk_clock_start (GTK_CLOCK (chrono));
}

int update_moves_left ()
{
        gchar tmpchar[16];

        check_free();
        sprintf(tmpchar, "%2d", moves_left);
        gtk_label_set (GTK_LABEL(moves_label), tmpchar);
        return moves_left;
}

void set_backgnd_colour (char *str)
{
  GdkColormap *colourmap ;
  GtkStyle *widget_style, *temp_style;

  if (str != backgnd.name)
    {
      if (backgnd.name)
	free (backgnd.name) ;
      backgnd.name = g_strdup (str) ;
    }
  colourmap = gtk_widget_get_colormap(canvas);
  gdk_color_parse (backgnd.name, &backgnd.colour);

  gdk_color_alloc(colourmap, &backgnd.colour);

  widget_style = gtk_widget_get_style (canvas);
  temp_style = gtk_style_copy (widget_style);
  temp_style->bg[0] = backgnd.colour;
  temp_style->bg[1] = backgnd.colour;
  temp_style->bg[2] = backgnd.colour;
  temp_style->bg[3] = backgnd.colour;
  temp_style->bg[4] = backgnd.colour;
  gtk_widget_set_style (canvas, temp_style);
  gnome_canvas_update_now (GNOME_CANVAS(canvas));
}

static void
change_tile_image (tile *tile_inf) {
	GdkImlibImage *new_image, *new_bg;
	gint orig_x, orig_y;

	orig_x = (tile_inf->image % 21) * TILE_WIDTH;
	orig_y = (tile_inf->image / 21) * TILE_HEIGHT;

	if (tile_inf->selected) {
		orig_y += 2 * TILE_HEIGHT;
	}

	gdk_imlib_destroy_image (tile_inf->current_image);
	gdk_imlib_destroy_image (tile_inf->current_bg);
	
	tile_inf->current_image = new_image = gdk_imlib_crop_and_clone_image (tiles_image, orig_x, orig_y, TILE_WIDTH, TILE_HEIGHT);
        tile_inf->current_bg = new_bg = gdk_imlib_crop_and_clone_image (bg_image, pos[tile_inf->number].layer * TILE_WIDTH,
                                                                        (tile_inf->selected != 0 ? 1 : 0) * TILE_HEIGHT,
                                                                        TILE_WIDTH, TILE_HEIGHT);
        
	gnome_canvas_item_set (tile_inf->bg_item, "image", new_bg, NULL);
	gnome_canvas_item_set (tile_inf->image_item, "image", new_image, NULL);
}

void select_tile (tile *tile_inf)
{
        tile_inf->selected |= SELECTED_FLAG;
        change_tile_image(tile_inf);
        selected_tile = tile_inf->number;
}

void unselect_tile (tile *tile_inf)
{
        selected_tile = MAX_TILES + 1;
        tile_inf->selected &= ~SELECTED_FLAG;
        change_tile_image (tile_inf);
}

static void
tile_event (GnomeCanvasItem *item, GdkEvent *event, tile *tile_inf)
{
  gchar tmpchar[16];

  if (paused) return; 

  switch(event->type) {
  case GDK_BUTTON_PRESS :
          if(tile_free(tile_inf->number)) {
                  switch (event->button.button) {
                  case 1:
                          if (tile_inf->selected & SELECTED_FLAG)
                                  unselect_tile (tile_inf);
                          else {
                                  if (selected_tile < MAX_TILES) {
                                          if ((tiles[selected_tile].type == tile_inf->type) ) {
                                                  tiles[selected_tile].visible = 0;
                                                  tile_inf->visible = 0;
                                                  tiles[selected_tile].selected &= ~SELECTED_FLAG;
                                                  change_tile_image (&tiles[selected_tile]);
                                                  gnome_canvas_item_hide (tiles[selected_tile].canvas_item);
                                                  gnome_canvas_item_hide (tile_inf->canvas_item);
                                                  clear_undo_queue ();
                                                  tiles[selected_tile].sequence = tile_inf->sequence = sequence_number;
                                                  sequence_number ++;
                                                  selected_tile = MAX_TILES + 1;
                                                  visible_tiles -= 2;
                                                  sprintf(tmpchar, "%3d", visible_tiles);
                                                  gtk_label_set (GTK_LABEL(tiles_label), tmpchar);
                                                  check_free();
                                                  sprintf(tmpchar, "%2d", moves_left);
                                                  gtk_label_set (GTK_LABEL(moves_label), tmpchar);

                                                  if (visible_tiles <= 0) {
                                                          gtk_clock_stop(GTK_CLOCK(chrono));
                                                          you_won ();
                                                  }
                                          }
                                          else
                                                  no_match ();
                                  }
                                  else 
                                          select_tile (tile_inf);
                          }
                          break;
                          
                  case 3:
                          if (selected_tile < MAX_TILES) 
                                  unselect_tile (&tiles[selected_tile]);
                          select_tile (tile_inf);
                          
                  default: 
                          break;
                  }
                  break;

          default :
                  break;
          }
  }
}

static void
pref_cancel (GtkWidget *widget, void *data)
{
	gtk_widget_destroy (pref_dialog);
	pref_dialog = 0;
}

static void
apply_preferences (void)
{
  char *buf, *buf2 = selected_tileset.tileset;
  
  int redraw = 0, sync = 0, ask_newgame = 0 ;
  if (selected_tileset.tileset)
    {
      buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/bg=bg1.png", NULL);
      load_tiles (selected_tileset.tileset, buf);
      g_free (buf);
      redraw = 1 ;
      if (selected_tileset.make_it_default)
	{
	  gnome_config_set_string ("/gmahjongg/Preferences/tileset", 
				   selected_tileset.tileset);
	  sync = 1 ;
	}
      selected_tileset.tileset = 0 ;
    }
  if (selected_bg.bg)
    {
      if (buf2) {
        load_tiles (buf2, selected_bg.bg);
      }
      else {
        buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/tileset=default.png", NULL);
        load_tiles (buf, selected_bg.bg);
	 g_free (buf);
      }
      redraw = 1 ;
      if (selected_bg.make_it_default)
	{
	  gnome_config_set_string ("/gmahjongg/Preferences/bg", 
				   selected_bg.bg);
	  sync = 1 ;
	}
      selected_bg.bg = 0 ;
    }
  if (selected_mapset.mapset)
    {
      set_map (selected_mapset.mapset) ;
      if (selected_mapset.make_it_default)
	{
	  gnome_config_set_string ("/gmahjongg/Preferences/mapset", 
				   selected_mapset.mapset);
	  sync = 1 ;
	}
      selected_mapset.mapset = 0 ;
      ask_newgame = 1 ;
    }
  if (backgnd.set)
    {
      set_backgnd_colour (backgnd.name) ;
      if (backgnd.make_it_default)
        {
	  gnome_config_set_string ("/gmahjongg/Preferences/bcolour",
				   backgnd.name) ;
	  sync = 1 ;
	}
      backgnd.set = 0 ;
    }
  if (popup_config.warn.set && (popup_config.warn.new != popup_config.warn.popup))
          {
                  set_popup (&(popup_config.warn), (popup_config.warn.popup +1) %2);
                  gnome_config_set_int ("/gmahjongg/Preferences/warn",
                                        popup_config.warn.popup);
                  sync = 1 ;
                  popup_config.warn.set = 0 ;
          }
  if (popup_config.confirm.set && (popup_config.confirm.new != popup_config.confirm.popup))
          {
                  set_popup (&(popup_config.confirm), (popup_config.confirm.popup +1) %2);
                  gnome_config_set_int ("/gmahjongg/Preferences/confirm",
                                        popup_config.confirm.popup);
                  sync = 1 ;
                  popup_config.confirm.set = 0 ;
          }
  if (sync) {
          gnome_config_sync();
/*          printf ("Synced\n") ; */
  }
  if (redraw)
    gnome_canvas_update_now(GNOME_CANVAS(canvas));
  pref_cancel (0,0);

  if (ask_newgame)
          gnome_app_question_modal (GNOME_APP (window), 
                                    "Start a new game\nwith the new mapset?", 
                                    new_game_reply_callback, NULL);
}

static void
fill_tile_menu (GtkWidget *menu, char *sdir, int is_tile)
{
	struct dirent *e;
	DIR *dir;
        int itemno = 0;
        char *dname = gnome_unconditional_pixmap_file (sdir);
        
	dir = opendir (dname);

	if (!dir)
		return;
	
	while ((e = readdir (dir)) != NULL){
		GtkWidget *item;
		char *s = g_strdup (e->d_name);

		if (!(strstr (e->d_name, ".xpm") ||
		      strstr (e->d_name, ".gif") ||
		      strstr (e->d_name, ".png") ||
		      strstr (e->d_name, ".jpg") ||
		      strstr (e->d_name, ".xbm"))){
			free (s);
			continue;
		}
			
		item = gtk_menu_item_new_with_label (s);
		gtk_widget_show (item);
		gtk_menu_append (GTK_MENU(menu), item);
                if (is_tile) {
                        gtk_signal_connect (GTK_OBJECT(item), "activate",
                                            (GtkSignalFunc)set_tile_selection, s); 
                        gtk_signal_connect (GTK_OBJECT(item), "destroy",
                                            (GtkSignalFunc) free_str, s);
                }
                else {
                        gtk_signal_connect (GTK_OBJECT(item), "activate",
                                            (GtkSignalFunc)set_bg_selection, s); 
                        gtk_signal_connect (GTK_OBJECT(item), "destroy",
                                            (GtkSignalFunc) free_str, s);
                }
                
 	        if (!strcmp(tileset, s)) 
 	        { 
 		  gtk_menu_set_active(GTK_MENU(menu), itemno); 
 		} 
	  
	        itemno++;
	}
	closedir (dir);
	g_free (dname);
}

static void
fill_map_menu (GtkWidget *menu)
{
  int lp, itemno=0 ;
  GtkWidget *item;

  for (lp=0;lp<ELEMENTS(maps);lp++)
    {
      char *str = g_strdup (_(maps[lp].name)) ;

      item = gtk_menu_item_new_with_label (str) ;
      gtk_widget_show (item);
      gtk_menu_append (GTK_MENU(menu), item);
      gtk_signal_connect (GTK_OBJECT(item), "activate",
			  (GtkSignalFunc)set_map_selection, &maps[lp]); 
      gtk_signal_connect (GTK_OBJECT(item), "destroy",
			   (GtkSignalFunc) free_str, str); 
      if (!g_strcasecmp(mapset, maps[lp].name))
	  gtk_menu_set_active(GTK_MENU(menu), itemno); 
      itemno++ ;
    }
}

#ifdef NEED_UNUSED_CODE
int find_tile_in_layer (int x, int y, int layer)
{
	int i, tile_num = MAX_TILES + 1;

	for (i = 0; i < MAX_TILES; i ++) {
	        int tx = canvas_x(i), ty = canvas_y(i);
		if ((tx < x) && ((tx + TILE_WIDTH - 1) > x)) {
			if ((ty < y) && ((ty + TILE_HEIGHT - 1) > y)) {
				if ((pos[i].layer == layer) && (tiles[i].visible == 1))
					tile_num = i;
			}
		}
	}
	return tile_num;
}
	
int find_tile (int x, int y)
{
	int i, tile_num = MAX_TILES + 1, layer = 0;

	for (i = 0; i < MAX_TILES; i++) {
	        int tx = canvas_x(i), ty = canvas_y(i);
		if ((tx < x) && ((tx + TILE_WIDTH - 1) > x) && (tiles[i].visible)) {
			if ((ty < y) && ((ty + TILE_HEIGHT - 1) > y)) {
				if ((pos[i].layer >= layer)) {
					tile_num = i;
					layer = pos[i].layer;
				}
			}
		}
	}
	return tile_num;
}
#endif
void no_match (void)
{
	GtkWidget *mb;
	if (popup_config.warn.popup) {
                mb = gnome_message_box_new (_("Tiles don't match!"),
                                            GNOME_MESSAGE_BOX_INFO,
                                            GNOME_STOCK_BUTTON_OK, NULL);
                GTK_WINDOW (mb)->position = GTK_WIN_POS_MOUSE;
                gtk_window_set_modal (&GNOME_MESSAGE_BOX(mb)->dialog.window, TRUE);
                
                gnome_dialog_set_parent (&GNOME_MESSAGE_BOX(mb)->dialog, GTK_WINDOW (window));
                gtk_widget_show (mb);
        } 
        else gnome_app_flash (GNOME_APP (window), "Tiles don't match !");
}

void check_free (void)
{
	int i;
        int tile_count[MAX_TILES];

        moves_left = 0;

        for (i=0; i<MAX_TILES; i++)
                tile_count[i] = 0;

	for (i=0;i<MAX_TILES;i++) {
                if (tile_free(i))
                        tile_count[tiles[i].type]++;
        }

        for (i=0; i<MAX_TILES; i++)
                moves_left += tile_count[i]>>1;

 	if ((moves_left == 0) && (visible_tiles>0)) { 
                GtkWidget *mb;
                if (!game_over) {
                        mb = gnome_message_box_new (_("No more moves"), 
                                                    GNOME_MESSAGE_BOX_INFO, 
                                                    GNOME_STOCK_BUTTON_OK, NULL); 
                        GTK_WINDOW (mb)->position = GTK_WIN_POS_MOUSE; 
                        gtk_window_set_modal (&GNOME_MESSAGE_BOX (mb)->dialog.window, TRUE);
                        gnome_dialog_set_parent(&GNOME_MESSAGE_BOX (mb)->dialog, GTK_WINDOW (window));
                        gtk_widget_show (mb); 
                }
                game_over = GAME_LOST;
 	} 
}

void you_won (void)
{
        gint pos;
        time_t seconds;
        gfloat score;
        gchar message[80];

        game_over = GAME_WON;
        
        seconds = GTK_CLOCK (chrono)->stopped;
        score = (seconds / 60) * 1.0 + (seconds % 60) / 100.0;
        if ( pos = gnome_score_log (score, mapset, FALSE) ) {
                gnome_scores_display (_(APPNAME_LONG), APPNAME, mapset, pos);
                sprintf (message, "Fantastic !  %.2f !\nYou have reached #%d in the Top Ten.\n\nAnother game ?", score, pos);
        }
        else
                sprintf (message, "Great !\nYou made it in %.2f.\n\nAnother game ?", score);
        gnome_app_question_modal (GNOME_APP (window), message,
                                  new_game_reply_callback, NULL);
}

void colour_changed_cb (GtkWidget *w, int r1, int g1, int b1, int a1, 
			gchar ** color)
{
  static char tmp[24] = "" ;
  guint8 r,g,b,a;

  gnome_color_picker_get_i8(GNOME_COLOR_PICKER(w), &r, &g, &b, &a);
  
  sprintf( tmp, "#%02x%02x%02x", r, g, b ) ;

  *color = tmp ;
  backgnd.set = 1 ;
}          

#ifdef OLD_PROPS
void properties_callback (GtkWidget *widget, gpointer data)
{
	GtkDialog *d;
	GtkWidget *button;
	GtkWidget *tmenu, *mmenu, *otmenu, *ommenu, *l, *hb, *cb, *f, *fv;
	GtkWidget *bcolour_gcs ;

	if (pref_dialog)
		return;

	pref_dialog = gtk_dialog_new ();
	d = GTK_DIALOG(pref_dialog);
 	gtk_signal_connect (GTK_OBJECT(pref_dialog), "close",
			    (GtkSignalFunc)pref_cancel, NULL); 

	/* The Tile sub-menu */
	otmenu = gtk_option_menu_new ();
	tmenu = gtk_menu_new ();
	fill_tile_menu (tmenu, "mahjongg", 1);
	gtk_widget_show (otmenu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(otmenu), tmenu);

	f = gtk_frame_new (_ ("Tiles"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
	gtk_widget_show (hb);
	
	l = gtk_label_new (_("Select Tiles:"));
	gtk_widget_show (l);
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), otmenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_tile_selection_def, NULL);
	gtk_widget_show (cb);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	gtk_widget_show (fv);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f);
	gtk_container_add (GTK_CONTAINER (f), fv);
	gtk_widget_show (f);

	/* The Tile Background sub-menu */
	otmenu = gtk_option_menu_new ();
	tmenu = gtk_menu_new ();
	fill_tile_menu (tmenu, "mahjongg/bg", 0);
	gtk_widget_show (otmenu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(otmenu), tmenu);

	f = gtk_frame_new (_ ("Tile Background:"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
	gtk_widget_show (hb);
	
	l = gtk_label_new (_("Select Background Tiles:"));
	gtk_widget_show (l);
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), otmenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
  	gtk_signal_connect (GTK_OBJECT(cb), "clicked", */
 			    (GtkSignalFunc)set_bg_selection_def, NULL); 
	gtk_widget_show (cb);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	gtk_widget_show (fv);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f);
	gtk_container_add (GTK_CONTAINER (f), fv);
	gtk_widget_show (f);

	/* The Map sub-menu */
	ommenu = gtk_option_menu_new ();
	mmenu = gtk_menu_new ();
	fill_map_menu (mmenu);
	gtk_widget_show (ommenu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(ommenu), mmenu);

	f = gtk_frame_new (_ ("Maps"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
       	l = gtk_label_new (_("Select Map:"));
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), ommenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_map_selection_def, NULL);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f);
	gtk_container_add (GTK_CONTAINER (f), fv);

	/* The colour */
	f = gtk_frame_new (_ ("Colours"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
        l = gtk_label_new (_("Background:")); */
      	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	{
	  int ur,ug,ub ;
	  bcolour_gcs  = gnome_color_picker_new();
	  sscanf( backgnd.name, "#%02x%02x%02x", &ur,&ug,&ub );
	  gnome_color_picker_set_i8( GNOME_COLOR_PICKER(bcolour_gcs), ur, 
				     ug, ub, 0);
	  gtk_signal_connect(GTK_OBJECT(bcolour_gcs), "color_set", 
			GTK_SIGNAL_FUNC(colour_changed_cb), &backgnd.name);
	}
	gtk_box_pack_start_defaults (GTK_BOX(hb), bcolour_gcs);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_backgnd_selection_def, NULL);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);

	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(d->vbox), f) ;
	gtk_container_add (GTK_CONTAINER (f), fv);
	
	/* Misc bottom buttons */
        button = gnome_stock_button(GNOME_STOCK_BUTTON_OK);
	 	gtk_signal_connect(GTK_OBJECT(button), "clicked", 
 			   GTK_SIGNAL_FUNC(load_callback), NULL); 
	gtk_box_pack_start(GTK_BOX(d->action_area), button, TRUE, TRUE, 5);
        button = gnome_stock_button(GNOME_STOCK_BUTTON_CANCEL);
 	gtk_signal_connect(GTK_OBJECT(button), "clicked", 
			   (GtkSignalFunc)pref_cancel,
 			   NULL);
	gtk_box_pack_start(GTK_BOX(d->action_area), button, TRUE, TRUE, 5);

        gtk_widget_show_all (pref_dialog);
}
#else
static void prefs_clicked_callback(GnomeDialog * dialog, gint button_number, 
			gpointer data)
{
        switch (button_number) {
        case 0: /* OK button */
                apply_preferences();
                break;
        };
        gnome_dialog_close(dialog);
        pref_dialog = 0;
}
         
void properties_callback (GtkWidget *widget, gpointer data)
{
	GnomeDialog *d;
	GtkWidget *button;
	GtkWidget *tmenu, *mmenu, *otmenu, *ommenu, *l, *hb, *cb, *f, *fv, *cols, *col1, *col2;
	GtkWidget *bcolour_gcs ;

	if (pref_dialog)
		return;

	pref_dialog = gnome_dialog_new (_ ("Preferences"), GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, NULL );

        d = GNOME_DIALOG(pref_dialog);
        gtk_signal_connect(GTK_OBJECT(pref_dialog), "clicked",
                           GTK_SIGNAL_FUNC(prefs_clicked_callback),
                           NULL);
                   
        gtk_signal_connect (GTK_OBJECT(pref_dialog), "close",
			    (GtkSignalFunc)pref_cancel, NULL); 

        cols = gtk_hbox_new (FALSE, FALSE);
        col1 = gtk_vbox_new (FALSE, FALSE);
        col2 = gtk_vbox_new (FALSE, FALSE);
        
	/* The Tile sub-menu */
	otmenu = gtk_option_menu_new ();
	tmenu = gtk_menu_new ();
	fill_tile_menu (tmenu, "mahjongg", 1);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(otmenu), tmenu);

	f = gtk_frame_new (_ ("Tiles"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
	
	l = gtk_label_new (_("Select Tiles:"));
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), otmenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_tile_selection_def, NULL);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(col1), f);
	gtk_container_add (GTK_CONTAINER (f), fv);

	/* The Tile Background sub-menu */
	otmenu = gtk_option_menu_new ();
	tmenu = gtk_menu_new ();
	fill_tile_menu (tmenu, "mahjongg/bg", 0);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(otmenu), tmenu);

	f = gtk_frame_new (_ ("Tile Background"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
	
	l = gtk_label_new (_("Select Tile Background:"));
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), otmenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
  	gtk_signal_connect (GTK_OBJECT(cb), "clicked", 
 			    (GtkSignalFunc)set_bg_selection_def, NULL); 

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(col1), f);
	gtk_container_add (GTK_CONTAINER (f), fv);

	/* The Map sub-menu */
	ommenu = gtk_option_menu_new ();
	mmenu = gtk_menu_new ();
	fill_map_menu (mmenu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU(ommenu), mmenu);

	f = gtk_frame_new (_ ("Maps"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
       	l = gtk_label_new (_("Select Map:"));
	    
	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	gtk_box_pack_start_defaults (GTK_BOX(hb), ommenu);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_map_selection_def, NULL);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);
	
	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(col1), f);
	gtk_container_add (GTK_CONTAINER (f), fv);

	/* The colour */
	f = gtk_frame_new (_ ("Colours"));
	gtk_container_border_width (GTK_CONTAINER (f), 5);

	hb = gtk_hbox_new (FALSE, FALSE);
	l = gtk_label_new (_("Background:"));
      	gtk_box_pack_start_defaults (GTK_BOX(hb), l);
	{
	  int ur,ug,ub ;
	  bcolour_gcs  = gnome_color_picker_new();
	  sscanf( backgnd.name, "#%02x%02x%02x", &ur,&ug,&ub );
	  gnome_color_picker_set_i8( GNOME_COLOR_PICKER(bcolour_gcs), ur, 
				     ug, ub, 0);
	  gtk_signal_connect(GTK_OBJECT(bcolour_gcs), "color_set", 
			GTK_SIGNAL_FUNC(colour_changed_cb), &backgnd.name);
	}
	gtk_box_pack_start_defaults (GTK_BOX(hb), bcolour_gcs);

	cb = gtk_check_button_new_with_label ( _("Make it the default") );
 	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_backgnd_selection_def, NULL);

	fv = gtk_vbox_new (0, 5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);

	gtk_box_pack_start_defaults (GTK_BOX(fv), hb);
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);
	gtk_box_pack_start_defaults (GTK_BOX(col2), f) ;
	gtk_container_add (GTK_CONTAINER (f), fv);
	
	/* Warning submenu */
  	f = gtk_frame_new (_ ("Warnings")); 
  	gtk_container_border_width (GTK_CONTAINER (f), 5); 

	fv = gtk_vbox_new (0,5);
	gtk_container_border_width (GTK_CONTAINER (fv), 5);

	cb = gtk_check_button_new_with_label (_("Warn when tiles don't match"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(cb), popup_config.warn.popup);
	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_popup_def, &(popup_config.warn));
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);

	cb = gtk_check_button_new_with_label (_("Confirm before quitting game"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(cb), popup_config.confirm.popup);
	gtk_signal_connect (GTK_OBJECT(cb), "clicked",
			    (GtkSignalFunc)set_popup_def, &(popup_config.confirm));
	gtk_box_pack_start_defaults (GTK_BOX(fv), cb);

	gtk_box_pack_start_defaults (GTK_BOX(col2), f) ;
	gtk_container_add (GTK_CONTAINER (f), fv);

        gtk_box_pack_start_defaults (GTK_BOX(cols), col1);
        gtk_box_pack_start_defaults (GTK_BOX(cols), col2);
        gtk_box_pack_start_defaults (GTK_BOX(d->vbox), cols);

	gnome_dialog_set_parent(d, GTK_WINDOW(window));
        gtk_widget_show_all (pref_dialog);
}
#endif


gint hint_timeout (gpointer data)
{
  timeout_counter ++;
  
  if (timeout_counter > HINT_BLINK_NUM) {
    if (selected_tile < MAX_TILES) {
      tiles[selected_tile].selected = 1;
    }
    return 0;
  }
  
  tiles[hint_tiles[0]].selected ^= HINT_FLAG;
  tiles[hint_tiles[1]].selected ^= HINT_FLAG;
  change_tile_image(&tiles[hint_tiles[0]]);
  change_tile_image(&tiles[hint_tiles[1]]);
  
  return 1;
}
  
void hint_callback (GtkWidget *widget, gpointer data)
{
        int i, j, free=0, type ;
        time_t seconds; 
        
        if (paused || game_over) 
                return; 
#ifdef NEED_UNUSED_CODE
                if (hint_dialog)
                        return;
#endif
                
                /* This prevents the flashing speeding up if the hint button is
                 * pressed multiple times. */
                if (timeout_counter <= HINT_BLINK_NUM) 
                        return;
                
                /* Snarfed from check free
                 * Tile Free is now _so_ much quicker, it is more elegant to do a
                 * British Library search, and safer. */

                /* Clear any selection */
                if (selected_tile < MAX_TILES) {
                        tiles[selected_tile].selected = 0;
                        change_tile_image (&tiles[selected_tile]);
                        selected_tile = MAX_TILES + 1;
                }
                
                for (i=0;i<MAX_TILES && !free;i++)
                        if (tile_free(i)) {
                                type = tiles[i].type ;
                                for (j=0;j<MAX_TILES && !free;j++) {
                                        free = (tiles[j].type == type && i != j && tile_free(j)) ;
                                        if (free) {
                                                tiles[i].selected ^= HINT_FLAG;
                                                tiles[j].selected ^= HINT_FLAG;
                                                change_tile_image (&tiles[i]);
                                                change_tile_image (&tiles[j]);
                                                hint_tiles[0] = i;
                                                hint_tiles[1] = j;
                                        }
                                }
                        }
                /* This is a good way to test check_free
                   for (i=0;i<MAX_TILES;i++)
                   if (tiles[i].selected == 17)
                   tiles[i].visible = 0 ;*/
                
                timeout_counter = 0;
                timer = gtk_timeout_add (250, (GtkFunction) hint_timeout, NULL);
                
                /* 30s penalty */
                gtk_clock_stop (GTK_CLOCK(chrono));
                seconds = GTK_CLOCK(chrono)->stopped;
                gtk_clock_set_seconds(GTK_CLOCK(chrono), (int) (seconds+30));
                gtk_clock_start (GTK_CLOCK(chrono));
}

void about_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *about;
	const gchar *authors [] = {
		"Code: Francisco Bustamante",
		"      Max Watson",
		"      Heinz Hempe",
		"      Michael Meeks",
                "      Philippe Chavin",
		"Tiles: Jonathan Buzzard",
		"       Max Watson",
		NULL
	};

	about = gnome_about_new (_("Gnome Mahjongg"), MAH_VERSION,
				 "(C) 1998 The Free Software Foundation",
				 (const char **)authors,
				 _("Send comments and bug reports to:\n"
				   "        pancho@nuclecu.unam.mx or\n"
				   "        mmeeks@gnu.org\n\n"
				   "Tiles under the General Public License."),
				 NULL);

       	gnome_dialog_set_parent(GNOME_DIALOG(about),GTK_WINDOW(window));
	gtk_widget_show (about);
}

void quit_game ()
{
        gnome_config_sync();
        gtk_main_quit ();
}

void pause_callback()
{
        int i;
        if(game_over) {
                gtk_toggle_button_set_state (PAUSE_BUTTON, FALSE);
                return; 
        }
        paused = !paused;
        if (paused) {
                gtk_clock_stop (GTK_CLOCK (chrono));
                for (i = 0; i < MAX_TILES; i++) 
                        if (tiles[i].visible) 
                                gnome_canvas_item_hide (tiles[i].image_item);
                message(_("... Game paused ..."));
        } 
        else {
                for (i = 0; i < MAX_TILES; i++) 
                        if (tiles[i].visible) 
                                gnome_canvas_item_show (tiles[i].image_item);
                message ("");
                gtk_clock_start (GTK_CLOCK(chrono));
        }
}

void ensure_pause_off (void)
{
        int i;

        if (paused) {
                gtk_toggle_button_set_state (PAUSE_BUTTON, FALSE);
                for (i = 0; i < MAX_TILES; i++) 
                        if (tiles[i].visible) 
                                gnome_canvas_item_show (tiles[i].image_item);
                message("");
        }
        paused = FALSE;
}

void scores_callback(GtkWidget *widget, gpointer data)
{
        gnome_scores_display (_(APPNAME_LONG), APPNAME, mapset, 0);
}

void init_game (void)
{
        gchar tmpchar[16] ;
        
        gtk_label_set(GTK_LABEL(tiles_label), MAX_TILES_STR);
        update_moves_left ();
        game_over = GAME_RUNNING;
        sequence_number = 1 ;
        visible_tiles = MAX_TILES;
        selected_tile = MAX_TILES + 1;
        gnome_canvas_update_now(GNOME_CANVAS(canvas));
        
        chrono_start();
}

void confirm_callback (gint reply, gpointer data)
{
        if (reply == GNOME_OK)
                switch ((int)data) 
                {
                case NEW_GAME: 
                        ensure_pause_off ();
                        new_game (); 
                        break;
                case RESTART_GAME: 
                        restart_game (); 
                        break;
                case SELECT_GAME: 
                        select_game (); 
                        break;
                case QUIT_GAME:
                        quit_game ();
                        break;
                default:
                        break;
                }
}

void exit_game_callback_query (GtkWidget *widget, gboolean *quit, gpointer data)
{
        const char *confirm_text;

        if ( popup_config.confirm.popup 
             && game_over != GAME_WON 
             && game_over != GAME_DEAD
             && sequence_number > 1 ) {
                switch ((game_state)data) 
                        {
                        case RESTART_GAME : 
                                confirm_text = "Really restart this game ?"; 
                                break;
                        case QUIT_GAME : 
                                confirm_text = "Really exit Gnome Mahjongg ?"; 
                                break;
                        case NEW_GAME:
                        case SELECT_GAME:
                                confirm_text = "Really start a new game ?";
                                break;
                        default: 
                                confirm_text = "Serious internal error";
                                break;
                        }
                gnome_app_ok_cancel_modal (GNOME_APP(window), confirm_text,
                                           confirm_callback, data);
        } else
                confirm_callback (GNOME_OK, data);
}

void exit_game_callback (GtkWidget *widget, gpointer data)
{
        gboolean dummy = FALSE;

        exit_game_callback_query (widget, &dummy, data);
}

void new_game_reply_callback (gint reply, gpointer data)
{
        if (reply == GNOME_YES) {
                ensure_pause_off ();
                new_game ();
        }
}

void restart_game ()
{
    int i;
    
    ensure_pause_off ();
    for (i = 0; i < MAX_TILES; i++) {
        tiles[i].visible = 1;
        tiles[i].selected = 0;
	tiles[i].sequence = 0;
	if (i == selected_tile)
	  change_tile_image (&tiles[selected_tile]);
	  
	gnome_canvas_item_show (tiles[i].canvas_item);
    }
    init_game ();
}
void redo_tile_callback (GtkWidget *widget, gpointer data)
{
        int i, change ;
        gchar tmpchar[16] ;
        
        if (paused) 
                return; 
        if (sequence_number>(MAX_TILES/2))
                return ;
        
        if (selected_tile<MAX_TILES) {
                tiles[selected_tile].selected = 0 ;
                change_tile_image (&tiles[selected_tile]);
                selected_tile = MAX_TILES + 1; 
        }
        change = 0 ;
        for (i=0; i<MAX_TILES; i++)
                if (tiles[i].sequence == sequence_number) {
                        tiles[i].selected = 0 ;
                        tiles[i].visible = 0 ;
                        gnome_canvas_item_hide (tiles[i].canvas_item);
                        visible_tiles-- ;
                        change = 1 ;
                }
        if (change) {
                if (sequence_number < MAX_TILES)
                        sequence_number++ ;
        }
        else
                  	gnome_app_flash (GNOME_APP (window), "No more redo!");
        sprintf(tmpchar,"%3d",visible_tiles) ;
        gtk_label_set(GTK_LABEL (tiles_label), tmpchar);
        
        update_moves_left ();
        gnome_canvas_update_now (GNOME_CANVAS (canvas));
}

void undo_tile_callback (GtkWidget *widget, gpointer data)
{
        int i;
        gchar tmpchar[16] ;
        
        if (paused || game_over == GAME_WON) 
                return;
        if (game_over == GAME_LOST)
                game_over = GAME_RUNNING;
        if (sequence_number>1)
                sequence_number-- ;
        else
                return ;
        
        if (selected_tile<MAX_TILES) {
                tiles[selected_tile].selected = 0 ;
                change_tile_image (&tiles[selected_tile]);
                selected_tile = MAX_TILES + 1; 
        }
        
        for (i=0; i<MAX_TILES; i++)
                if (tiles[i].sequence == sequence_number) {
                        tiles[i].selected = 0 ;
                        tiles[i].visible = 1 ;
                        visible_tiles++ ;
                        gnome_canvas_item_show (tiles[i].canvas_item);
                }
        
        sprintf (tmpchar, "%3d", visible_tiles) ;
        gtk_label_set (GTK_LABEL(tiles_label), tmpchar);
        gnome_canvas_update_now (GNOME_CANVAS (canvas));
        
        update_moves_left ();
}

void seed_dialog_clicked_cb (GnomeDialog * dialog, gint button_number, 
                             gpointer data)
{
        switch (button_number) {
        case 0: /* OK button */
                srand (atoi (gtk_entry_get_text (GTK_ENTRY (data))));
                new_game ();
                break;

        case 1: /* Cancel Button */
                break;

        default:
                break;
        };
        gnome_dialog_close(dialog);
}

void select_game ()
{
	GtkWidget *dialog, *entry, *label;

        ensure_pause_off();
	dialog = gnome_dialog_new (_("Select Game"),
				   GNOME_STOCK_BUTTON_OK,
				   GNOME_STOCK_BUTTON_CANCEL,
				   NULL);
	GTK_WINDOW (dialog)->position = GTK_WIN_POS_MOUSE;
	label = gtk_label_new (_("Game Number:"));
	gtk_box_pack_start_defaults (GTK_BOX(GNOME_DIALOG(dialog)->vbox), label);
	
	entry = gtk_entry_new ();
	gtk_box_pack_start_defaults (GTK_BOX(GNOME_DIALOG(dialog)->vbox), entry);
	gtk_signal_connect (GTK_OBJECT (dialog), "clicked",
			    GTK_SIGNAL_FUNC (seed_dialog_clicked_cb),
			    (gpointer)entry);
	gnome_dialog_set_default (GNOME_DIALOG(dialog), 0);
	gnome_dialog_editable_enters (GNOME_DIALOG(dialog),(GTK_EDITABLE(entry)));
	
       	gnome_dialog_set_parent(GNOME_DIALOG(dialog),GTK_WINDOW(window));
	gtk_widget_show_all (dialog);
}

void show_tb_callback (GtkWidget *widget, gpointer data)
{
    GnomeDockItem *gdi;
    GtkWidget *toolbar;

    gdi = gnome_app_get_dock_item_by_name (GNOME_APP (window), GNOME_APP_TOOLBAR_NAME);
    toolbar = gnome_dock_item_get_child (gdi);

    if((GTK_CHECK_MENU_ITEM(settingsmenu[0].widget))->active)
    {
        gnome_config_set_bool("gmahjongg/toolbar/show", TRUE);
        gtk_widget_show(GTK_WIDGET(gdi));
    }
    else
    {
        gnome_config_set_bool("gmahjongg/toolbar/show", FALSE);
        gtk_widget_hide(GTK_WIDGET(gdi));
	gtk_widget_queue_resize (window);
    }
}

void sound_on_callback (GtkWidget *widget, gpointer data)
{
    printf("mer\n");
}

/* You loose your re-do queue when you make a move */
void clear_undo_queue ()
{
  int lp ;
  for (lp=0;lp<MAX_TILES;lp++)
    if (tiles[lp].sequence>=sequence_number)
      tiles[lp].sequence = 0 ;
}

void set_map (char* name)
{
  if (mapset)
    g_free(mapset);
  mapset = g_strdup(name);
}

void load_map (void)
{
  char* name = mapset;
  int lp ;
  int xmax = 0, ymax = 0;
  tilepos *t;

  for (lp=0;lp<ELEMENTS(maps);lp++)
    if (g_strcasecmp (maps[lp].name, name) == 0)
      {
	pos = maps[lp].map ;

	for ( t = pos ; t < pos + MAX_TILES ; t++ ) {
	    if ( (*t).x  > xmax ) xmax = (*t).x;
	    if ( (*t).y  > ymax ) ymax = (*t).y;
	}
	xpos_offset = ( AREA_WIDTH - (HALF_WIDTH * (xmax+1)) ) / 2;
	ypos_offset = ( AREA_HEIGHT - (HALF_HEIGHT * (ymax+1) ) ) / 2;

	generate_dependancies() ;
      }
}

int canvas_x (int i)
{
     return pos[i].x * (HALF_WIDTH-0) + xpos_offset + (THICKNESS * pos[i].layer);
}
int canvas_y (int i)
{
     return pos[i].y * (HALF_HEIGHT-0) + ypos_offset - (THICKNESS * pos[i].layer);
}

void load_images (void)
{
  int i;
  
  for (i = MAX_TILES - 1; i >= 0; i --) {
    gnome_canvas_item_set (tiles[i].image_item,
			   "x", (double)canvas_x(i),
			   "y", (double)canvas_y(i),
			   NULL);
    gnome_canvas_item_set (tiles[i].bg_item,
			   "x", (double)canvas_x(i),
			   "y", (double)canvas_y(i),
			   NULL);
  }
}

void create_canvas_items (void)
{
  GdkImlibImage *image, *bg_cimage;
  gint orig_x, orig_y, i;
  
  /* It's essential that the tiles are already sorted into layer order (lowest first) */
  for (i = MAX_TILES - 1; i >= 0; i --) {
       tiles[i].canvas_item = gnome_canvas_item_new (gnome_canvas_root(GNOME_CANVAS(canvas)),
						      gnome_canvas_group_get_type (),
						      NULL);
	orig_x = (tiles[i].image % 21) * TILE_WIDTH;
	orig_y = (tiles[i].image / 21) * TILE_HEIGHT;
	
	tiles[i].number = i;
	
	tiles[i].current_image = image = gdk_imlib_crop_and_clone_image (tiles_image, orig_x, orig_y, TILE_WIDTH, TILE_HEIGHT);
	tiles[i].current_bg = bg_cimage = gdk_imlib_crop_and_clone_image (bg_image, pos[i].layer * TILE_WIDTH,
                                                                          0, TILE_WIDTH, TILE_HEIGHT);
	
        tiles[i].bg_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (tiles[i].canvas_item),
						      gnome_canvas_image_get_type(),
						      "image", bg_cimage,
						     "x", (double)canvas_x(i),
						     "y", (double)canvas_y(i),
						      "width", (double)TILE_WIDTH,
						      "height", (double)TILE_HEIGHT,
						      "anchor", GTK_ANCHOR_NW,
						      NULL);

	tiles[i].image_item = gnome_canvas_item_new (GNOME_CANVAS_GROUP (tiles[i].canvas_item),
						      gnome_canvas_image_get_type(),
						      "image", image,
						     "x", (double)canvas_x(i),
						     "y", (double)canvas_y(i),
						      "width", (double)TILE_WIDTH,
						      "height", (double)TILE_HEIGHT,
						      "anchor", GTK_ANCHOR_NW,
						      NULL);
	
	gtk_signal_connect (GTK_OBJECT (tiles[i].canvas_item), "event",
			    (GtkSignalFunc) tile_event,
			    &tiles[i]);
    }
}

void load_tiles (char *fname, char *bg_fname)
{
	char *tmp, *fn, *bg_fn;
	int i;

	tmp = g_strconcat ("mahjongg/", fname, NULL);

	fn = gnome_unconditional_pixmap_file (tmp);
	g_free (tmp);

	tmp = g_strconcat ("mahjongg/bg/", bg_fname, NULL);

	bg_fn = gnome_unconditional_pixmap_file (tmp);
	g_free (tmp);

	if (!g_file_exists (fn)) {
		char *s = g_strdup_printf (_("Could not find file %s"), fn);
		GtkWidget *box;
		
		box = gnome_message_box_new (s, GNOME_MESSAGE_BOX_ERROR, GNOME_STOCK_BUTTON_OK, NULL);
		gnome_dialog_run (GNOME_DIALOG (box));
		exit (1);
	}

	if (!g_file_exists (bg_fn)) {
		char *s = g_strdup_printf (_("Could not find file %s"), bg_fn);
		GtkWidget *box;
		
		box = gnome_message_box_new (s, GNOME_MESSAGE_BOX_ERROR, GNOME_STOCK_BUTTON_OK, NULL);
		gnome_dialog_run (GNOME_DIALOG (box));
		exit (1);
	}

	if (tileset)
		g_free (tileset);

	tileset = g_strdup(fname);
	
	if (tiles_image)
		gdk_imlib_destroy_image (tiles_image);

	if (bg_image)
		gdk_imlib_destroy_image (bg_image);

	tiles_image = gdk_imlib_load_image (fn);
	gdk_imlib_render (tiles_image, tiles_image->rgb_width, tiles_image->rgb_height);

        bg_image = gdk_imlib_load_image (bg_fn);
        gdk_imlib_render (bg_image, bg_image->rgb_width, bg_image->rgb_height);
        
	for (i = 0; i < MAX_TILES; i++) {
	  change_tile_image (&tiles[i]);
	}

	g_free (bg_fn);
	g_free (fn);
}

void create_mahjongg_board (void)
{
	gchar *buf, *buf2;
	gint ibuf;
	
	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());

	canvas = gnome_canvas_new();

	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
	
	gtk_box_pack_start_defaults (GTK_BOX (mbox), canvas);

	gtk_widget_set_usize(canvas, AREA_WIDTH, AREA_HEIGHT);
	gnome_canvas_set_pixels_per_unit (GNOME_CANVAS(canvas), 1);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(canvas), 0, 0, AREA_WIDTH, AREA_HEIGHT);
	
	gtk_widget_show (canvas);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/mapset=easy", NULL);
	set_map (buf);
	g_free (buf);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/bcolour=#003010", NULL) ;
	set_backgnd_colour (buf) ;
	g_free (buf);

	buf = gnome_config_get_string_with_default ("/gmahjongg/Preferences/tileset=default.png", NULL);
        buf2 = gnome_config_get_string_with_default ("/gmahjongg/Preferences/bg=bg1.png", NULL);
        
	ibuf = gnome_config_get_int_with_default ("/gmahjongg/Preferences/warn=1", NULL);
	set_popup (&(popup_config.warn), ibuf) ;

	ibuf = gnome_config_get_int_with_default ("/gmahjongg/Preferences/confirm=1", NULL);
	set_popup (&(popup_config.confirm), ibuf) ;

	load_map (); /* assigns pos, and calculates dependencies */
	generate_game (); /* puts in the positions of the tiles */
	create_canvas_items ();

	load_tiles (buf, buf2);
        g_free (buf2);
	g_free (buf);
	init_game ();
}

void new_game (void)
{
  int i;
  
  load_map ();
  generate_game() ;
  load_images ();

  for (i = 0; i < MAX_TILES; i++) {
    change_tile_image (&tiles[i]);
    gnome_canvas_item_show (tiles[i].canvas_item);
  }
  
  init_game ();
}

void shuffle_tiles_callback (GtkWidget *widget, gpointer data)
{
        int i, previous, first=1, num_shuffle=0;
        tile temp;
        time_t seconds;

        if (paused || game_over == GAME_DEAD || game_over == GAME_WON) return;

        do {
                num_shuffle++;

                /* We do a circular permutation (FIXME: check the english term...)*/
                for (i=0; i<MAX_TILES; i++) {
                        if (tiles[i].visible) {
                                if (first) {
                                        temp = tiles[i];
                                        first--; }
                                else {
                                        tiles[previous].type = tiles[i].type;
                                        tiles[previous].image = tiles[i].image;
                                }
                                previous = i; 
                        }
                }
                tiles[previous].type = temp.type;
                tiles[previous].image = temp.image;
        }
        while (!(update_moves_left ()) && num_shuffle < visible_tiles);
        
        if (num_shuffle >= visible_tiles) {
                GtkWidget *mb;
                game_over = GAME_DEAD;
                gtk_clock_stop (GTK_CLOCK (chrono));
                mb = gnome_message_box_new (_("Sorry, I can't find\na playable configuration."), 
                                            GNOME_MESSAGE_BOX_INFO, 
                                            GNOME_STOCK_BUTTON_OK, NULL); 
                GTK_WINDOW (mb)->position = GTK_WIN_POS_MOUSE; 
                gtk_window_set_modal (&GNOME_MESSAGE_BOX (mb)->dialog.window, TRUE);
                gnome_dialog_set_parent(&GNOME_MESSAGE_BOX (mb)->dialog, GTK_WINDOW (window));
                gtk_widget_show (mb); 
        } else {
                
                for (i=0; i<MAX_TILES; i++) {
                        tiles[i].sequence = 0;
                        if (tiles[i].visible) {
                                change_tile_image (&tiles[i]);
                                gnome_canvas_item_show (tiles[i].canvas_item);
                        }
                }
                
                game_over = GAME_RUNNING;

                /* 60s penalty */
                gtk_clock_stop (GTK_CLOCK(chrono));
                seconds = GTK_CLOCK(chrono)->stopped;
                gtk_clock_set_seconds(GTK_CLOCK(chrono), (int) (seconds+60));
                gtk_clock_start (GTK_CLOCK(chrono));
        }
}

int main (int argc, char *argv [])
{
        GnomeDockItem *gdi;
	GtkWidget *toolbar;
        gboolean show=TRUE;

        gnome_score_init (APPNAME);

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

  	gnome_init (APPNAME, MAH_VERSION, argc, argv); 
        gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-mahjongg.png");
	srand (time (NULL));
	
	window = gnome_app_new (APPNAME, _(APPNAME_LONG));
/*	gtk_window_set_policy (GTK_WINDOW (window), FALSE, FALSE, TRUE); */

	/* Statusbar for a chrono */
	chrono_box = gtk_hbox_new(0, FALSE);
	chrono_label = gtk_label_new (_("Time:"));
	gtk_box_pack_start (GTK_BOX(chrono_box), chrono_label, FALSE, FALSE, 0);
	chrono = gtk_clock_new (GTK_CLOCK_INCREASING);
	gtk_box_pack_start (GTK_BOX(chrono_box), chrono, FALSE, FALSE, 0);
	gtk_widget_show (chrono_label);
	gtk_widget_show (chrono);
	gtk_widget_show (chrono_box);

	appbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_USER);
	gtk_box_pack_end(GTK_BOX(appbar), chrono_box, FALSE, FALSE, 0);
	gnome_app_set_statusbar (GNOME_APP (window), appbar);

	gnome_app_create_menus (GNOME_APP (window), mainmenu);
	gnome_app_install_menu_hints(GNOME_APP (window), mainmenu);

        gnome_app_create_toolbar (GNOME_APP (window), toolbar_uiinfo);
	gdi = gnome_app_get_dock_item_by_name (GNOME_APP (window), GNOME_APP_TOOLBAR_NAME);
	toolbar = gnome_dock_item_get_child (gdi);
        gtk_toolbar_set_space_size(GTK_TOOLBAR (toolbar), 25);
        
        tiles_label = gtk_label_new(_("Tiles Left: "));
        gtk_widget_show(tiles_label);
	gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar), tiles_label,
				  NULL, NULL);
        tiles_label = gtk_label_new(MAX_TILES_STR);
        gtk_widget_show(tiles_label);
	gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar), tiles_label,
				  NULL, NULL);
	gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

        moves_label = gtk_label_new(_("Moves Left: "));
        gtk_widget_show(moves_label);
	gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar), moves_label,
				  NULL, NULL);
        moves_label = gtk_label_new(MAX_TILES_STR);
        gtk_widget_show(moves_label);
	gtk_toolbar_append_widget(GTK_TOOLBAR(toolbar), moves_label,
				  NULL, NULL);
  	gtk_toolbar_append_space(GTK_TOOLBAR(toolbar));

	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			    GTK_SIGNAL_FUNC (exit_game_callback_query), (gpointer)QUIT_GAME);

	mbox = gtk_vbox_new (FALSE, 0);

	gnome_app_set_contents (GNOME_APP (window), mbox);
	create_mahjongg_board ();

	gtk_widget_show (window);

        if(gnome_config_get_bool_with_default("/gmahjongg/toolbar/show",&show))
            gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(settingsmenu[0].widget), TRUE);
        else {
                gtk_check_menu_item_set_state(GTK_CHECK_MENU_ITEM(settingsmenu[0].widget), FALSE);
                gdi = gnome_app_get_dock_item_by_name (GNOME_APP (window), GNOME_APP_TOOLBAR_NAME);
                gtk_widget_hide(GTK_WIDGET(gdi)) ;
                gtk_widget_queue_resize (window);
        }

  	gnome_app_flash (GNOME_APP (window), 
  				"Welcome to Gnome Mahjongg!"); 

	gtk_main ();
	
	return 0;
}
