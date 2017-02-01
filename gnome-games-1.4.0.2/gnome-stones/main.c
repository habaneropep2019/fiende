/* gnome-stones - main.c
 *
 * Time-stamp: <2000/05/09 18:20:21 martin>
 *
 * Copyright (C) 1998 Carsten Schaar
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
#include <libgnomeui/gnome-window-icon.h>
#include <stdlib.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include "game.h"
#include "cave.h"
#include "object.h"
#include "status.h"
#include "player.h"
#include "preferences.h"
#include "io.h"
#include "view.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>

/* You should leave 'USE_GNOME_CANVAS' undefined, because this game
   currently doesn't support all features with gnome_canvas stuff
   enabled.  */
#undef USE_GNOME_CANVAS

#undef USE_KEY_RELEASE


#define APP_NAME "gnome-stones"


/* Definitions */

#define START_DELAY 3000
#define END_DELAY 3000
#define CURTAIN_DELAY 50
#define COUNTDOWN_DELAY 20

#define GAME_COLS  20
#define GAME_ROWS 12
#define GAME_SCROLL_MAX 3
#define GAME_SCROLL_MIN 6


#ifdef USE_KEY_RELEASE
#define GAME_EVENTS (GDK_KEY_PRESS_MASK             |\
		     GDK_KEY_RELEASE_MASK)
;
#else
#define GAME_EVENTS (GDK_KEY_PRESS_MASK)
#endif


/*****************************************************************************/
/* Used widgets. */

static GtkWidget     *gstones_view;
static GdkPixmap     *title_image;
static GdkPixmap     *title_template;

static gint           player_x_direction= 0;
static gint           player_y_direction= 0;
static gboolean       player_push       = FALSE;


/****************************************************************************/
/* Some constants.  */

/* This message will be printed in the statusline, if no message was
   specified in a game file.  */

static char *default_message= N_("Gnome-Stones (c) 1998 Carsten Schaar");


/****************************************************************************/
/* Commandline options.  */

/* Command-line arguments understood by this module.  */
static const struct poptOption options[] = {
  {"game", 'g', POPT_ARG_STRING, &default_game, 0,
   N_("Game to play"), N_("FILENAME")},
  {NULL, '\0', 0, NULL, 0}
};



/****************************************************************************/

static void
curtain_start (GStonesCave *cave);


static void
game_start_cb (GtkWidget *widget, gpointer data);



/****************************************************************************/
/* Image stuff  */

static void
my_exit (GtkWidget *widget, gpointer client_data)
{
  exit (1);
}

static GdkImlibImage *
load_image_from_path (const char *relative_name)
{
  gchar         *filename;
  GdkImlibImage *image= NULL;

  filename= gnome_pixmap_file (relative_name);
  if (filename)
    image= gdk_imlib_load_image (filename);

  g_free (filename);
  
  if (!image)
    {
      GtkWidget *widget;
      char       buffer[1024];
      g_snprintf (buffer, sizeof(buffer), 
		  _("An error occured while loading the image file \"%s\".\n"
		    "Please make sure, that Gnome-Stones is "
		    "correctly installed!"), relative_name);
					    
      widget= gnome_error_dialog (buffer);
      gtk_signal_connect (GTK_OBJECT (widget), "close",
			  GTK_SIGNAL_FUNC (my_exit), NULL);

      gtk_main ();
    }
  
  return image;
}



static void
title_image_load (void)
{
  GdkImlibImage *image= load_image_from_path ("gnome-stones/title.png");

  gdk_imlib_render (image, image->rgb_width, image->rgb_height);
  title_template= gdk_imlib_copy_image (image);
  title_image   = gdk_imlib_move_image (image);  

  /* Redraw the game widget.  */
  gtk_widget_draw (gstones_view, NULL);
}



/****************************************************************************/

/* Game widget handling.  */

#ifdef USE_GNOME_CANVAS
static GnomeCanvasItem *game_items[GAME_COLS][GAME_ROWS];
static GdkImlibImage   *game_current_images[GAME_COLS][GAME_ROWS];


static void
game_update_image (GtkWidget *widget)
{
  GdkImlibImage *image;
  int x1, y1, x2, y2, x, y;

  x1 = x_offset;
  y1 = y_offset;
  x2 = GAME_COLS+x_offset-1;
  y2 = GAME_ROWS+y_offset-1;
  
  for (x = x1; x <= x2; x++)
    for (y = y1; y <= y2; y++)
      {
	
	gboolean cave_mode;
	
	if ((curtain_mode == GAME_CURTAIN_CLOSING && 
	     (x+y > curtain+x_offset+y_offset)) ||
	    (curtain_mode == GAME_CURTAIN_OPENING && 
	     (x+y < curtain+x_offset+y_offset)))
	  {
	    cave_mode= TRUE;
	    image= curtain_imlib_image;
	  }
	else if (display_mode == GAME_DISPLAY_IMAGE)
	  {
	    cave_mode= FALSE;
	  }
	else
	  {
	    cave_mode= TRUE;

	    image= cave_get_imlib_image (cave, x+1, y+1);
	  }
	
	if (image != game_current_images[x-x_offset][y-y_offset])
	  {
	    gnome_canvas_item_set (game_items[x-x_offset][y-y_offset],
				   "image", image,
				   NULL);
	    game_current_images[x-x_offset][y-y_offset]= image;
	  }
      }

  gnome_canvas_update_now (GNOME_CANVAS (widget));
}
#endif



static void
game_update_title (void)
{
  /* Replace image with template.  */
  gdk_draw_pixmap (title_image,
		   gstones_view->style->black_gc, title_template,
		   0, 0,
		   0, 0, GAME_COLS*STONE_SIZE, GAME_ROWS*STONE_SIZE);

  if (game)
    {
      static char *gametitle= N_("Game title:");
      static char *cavename = N_("Start cave:");

      guint    height;
      GdkFont *font;
      GList   *tmp= g_list_nth (game->start_caves, start_cave);

      font = gdk_font_ref (gstones_view->style->font);
      if (!font)
	font= gdk_font_load ("fixed");

      height= font->ascent+font->descent;

      gdk_draw_text   (title_image,
		       font,
		       gstones_view->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30-height*3-height/2,
		       _(gametitle),
		       strlen (_(gametitle)));

      gdk_draw_text   (title_image,
		       font,
		       gstones_view->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30-height*2-height/2,
		       game->title,
		       strlen (_(game->title)));
      
      gdk_draw_text   (title_image,
		       font,
		       gstones_view->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30-height,
		       _(cavename),
		       strlen (_(cavename)));

      gdk_draw_text   (title_image,
		       font,
		       gstones_view->style->black_gc,
		       30, GAME_ROWS*STONE_SIZE-30,
		       tmp->data,
		       strlen (tmp->data));

      gdk_font_unref (font);
    }  

  /* Redraw the game widget.  */
  gtk_widget_draw (gstones_view, NULL);
}


static guint last_keyval= GDK_VoidSymbol;

static gint
game_widget_key_press_callback (GtkWidget   *widget,
				GdkEventKey *event,
				gpointer     client_data)
{
  player_push= event->state & GDK_SHIFT_MASK;
  
  switch (event->keyval)
    {
    case GDK_KP_8:
    case GDK_KP_Up:
    case GDK_Up:
      if (state == STATE_RUNNING)
	{
	  /* Move player.  */
	  player_x_direction=  0;
	  player_y_direction= -1;
	  last_keyval      = event->keyval;
	  return TRUE;
	}
      else if ((state == STATE_TITLE) && game)
	{
	  if (start_cave+1 < g_list_length (game->start_caves))
	    {
	      start_cave++;
	      game_update_title ();
	    }
	  return TRUE;
	}
      return FALSE;
    case GDK_KP_4:
    case GDK_KP_Left:
    case GDK_Left:
      player_x_direction= -1;
      player_y_direction=  0;
      last_keyval      = event->keyval;
      return TRUE;
    case GDK_KP_6:
    case GDK_KP_Right:
    case GDK_Right:
      player_x_direction=  1;
      player_y_direction=  0;
      last_keyval      = event->keyval;
      return TRUE;
    case GDK_KP_2:
    case GDK_KP_Down:
    case GDK_Down:
      if (state == STATE_RUNNING)
	{
	  /* Move player.  */
	  player_x_direction=  0;
	  player_y_direction=  1;
	  last_keyval      = event->keyval;
	  return TRUE;
	}
      else if ((state == STATE_TITLE) && game)
	{
	  if (start_cave > 0)
	    {
	      start_cave--;
	      game_update_title ();
	    }
	  return TRUE;
	}
      return FALSE;
    case GDK_Escape:
      if ((state == STATE_RUNNING) && cave)
	{
	  if (player_push)
	    curtain_start (NULL);
	  else
	    cave_player_die (cave);
	}
      return TRUE;
    case GDK_P:
    case GDK_p:
      if ((state == STATE_RUNNING) && cave)
	{
	  cave_toggle_pause_mode (cave);
	  
	  if (cave->flags & CAVE_PAUSING)
	    gnome_appbar_set_status (GNOME_APPBAR (statusbar), _("Pause"));
	  else
	    gnome_appbar_refresh (GNOME_APPBAR (statusbar));
	}
      return FALSE;
    case GDK_KP_Enter:
    case GDK_Return:
      if (state == STATE_TITLE)
	{
	  game_start_cb (NULL, NULL);
	  return TRUE; 
	}
      return FALSE;
    }

  return FALSE;
};


static gint
game_widget_key_release_callback (GtkWidget   *widget,
				  GdkEventKey *event,
				  gpointer     client_data)
{
  if (last_keyval == event->keyval)
    {
      player_x_direction=  0;
      player_y_direction=  0;
      last_keyval       =  GDK_VoidSymbol;
      return TRUE;
    }

  return FALSE;
}


#ifdef USE_GNOME_CANVAS
/* We need this variable to be global, because we can only fill this
   canvas after loading the needed images.  */
GtkWidget *canvas;

/* This function need 'canvas' and 'curtain_imlib_image' to have well
   defined values.  */

static gboolean
game_widget_fill (void)
{
  guint x;
  guint y;
  
  for (x= 0; x < GAME_COLS ; x+= 1)
    for (y= 0; y < GAME_COLS ;  y+= 1)
      {
	game_items[x][y]=
	  gnome_canvas_item_new (GNOME_CANVAS_GROUP (gnome_canvas_root 
						     (GNOME_CANVAS 
						      (canvas))),
				 gnome_canvas_image_get_type (),
				 "image", curtain_imlib_image,
				 "x", (double) x*STONE_SIZE,
				 "y", (double) y*STONE_SIZE,
				 "width", (double) curtain_imlib_image->rgb_width,
				 "height", (double) curtain_imlib_image->rgb_height,
				 "anchor", GTK_ANCHOR_NW,
				 NULL);
	
      }
  
  return TRUE;
}
#endif /* USE_GNOME_CANVAS */



/****************************************************************************/
/* Timeout stuff

   In the following, we list all needed timeout handles.  */

static guint iteration_timeout= 0;
static guint countdown_timeout= 0;

/****************************************************************************/

GStonesCave *curtain_cave;

static gint
start_cave_delay_timeout (gpointer data);

static void
iteration_start (GStonesCave *cave);

void
curtain_ready (ViewCurtainMode mode)
{
  switch (mode)
    {
    case VIEW_CURTAIN_OPEN:  
      if (curtain_cave)
	{
	  state= STATE_WAITING_TO_START;
	  gtk_timeout_add (START_DELAY, start_cave_delay_timeout, curtain_cave);
	}
      else
	state= STATE_TITLE;
      break;
      

    case VIEW_CURTAIN_CLOSED:
      /* If the iteration has been running, we stop it now.  */
      gtk_timeout_remove (iteration_timeout);
      
      /* An existing cave must be deleted.  */
      if (cave)
	{
	  cave_free (cave);
	  cave= NULL;
	}
      
      if (curtain_cave)
	{
	  view_display_cave (GSTONES_VIEW (gstones_view), curtain_cave);
	  status_set_mode (STATUS_GAME_INFO);
	  
	  iteration_start (curtain_cave);
	  
	  /* Print message to screen.  */
	  if (curtain_cave->message)
	    gnome_appbar_set_default (GNOME_APPBAR (statusbar), curtain_cave->message);
	  else
	    gnome_appbar_set_default (GNOME_APPBAR (statusbar), _(default_message));
	}
      else
	view_display_image (GSTONES_VIEW (gstones_view), title_image);
      
      break;
    }
}


/* Start a curtain sequence.  

   'cave' determines, what happens after the curtain sequence.
   Setting 'cave' to NULL means, that the title image will be shown
   after the sequence.  Setting 'cave' to a non NULL value, lets the
   program load a new cave and start it.  */

void
curtain_start (GStonesCave *cave)
{
  curtain_cave= cave;
  state       = STATE_CURTAIN;

  view_set_curtain_mode (GSTONES_VIEW (gstones_view), 
			 VIEW_CURTAIN_ANIMATE, curtain_ready);
}



/****************************************************************************/


static gint
start_cave_delay_timeout (gpointer data)
{
  GStonesCave *cave= (GStonesCave *) data;
  
  if (state == STATE_WAITING_TO_START)
    { 
      status_set_mode (STATUS_CAVE_INFO);
      cave_start (cave);
      
      state= STATE_RUNNING;
    }

  return FALSE;
}



/*****************************************************************************/
/* Countdown stuff

   The following functions are needed to implement the timer countdown,
   when the player has finished a level.  */

static gint
countdown_timeout_function (gpointer data)
{
  GStonesCave *cave   = (GStonesCave *)data;
  GStonesCave *newcave= NULL;
  
  if (state == STATE_COUNTDOWN)
    {
      if (cave->timer > 0)
	{
	  cave->timer-= 1000;
	  if (cave->timer < 0) cave->timer= 0;

	  player_set_time (player, cave->timer, FALSE);
	  
	  /* FIXME: The added value should be configureable on a per
             game basis.  */
	  player_inc_score (player, 1);
	  
	  return TRUE;
	}
      
      if (cave->next)
	newcave= gstones_cave_load (game, cave->next);
	
      curtain_start (newcave);

      if (!newcave)
	{
	  gint pos;

	  pos= gnome_score_log (player->score, NULL, TRUE);

	  gnome_scores_display ("Gnome-Stones", APP_NAME, NULL, pos);
	  gnome_app_flash (GNOME_APP (app), _("Congratulations, you win!"));
	}
    }
  return FALSE;
}

/* Start a countdown.  

   'cavename' determines, what happens after the countdown.
   Setting 'cavename' to NULL means, the title image will be shown
   after the countdown.  Setting 'cavename' to a non NULL value, lets
   the program load a new cave and start it.  */

void
countdown_start (GStonesCave *cave)
{
  /* When showing the countdown, there is no iteration in the
     background anymore.  */
  gtk_timeout_remove (iteration_timeout);

  state    = STATE_COUNTDOWN;
  
  countdown_timeout= 
    gtk_timeout_add (COUNTDOWN_DELAY, 
		     (GtkFunction) countdown_timeout_function, 
		     (gpointer) cave);
}



/*****************************************************************************/

/* This widget will be used when querying the joystick device.  */
static GtkWidget *joystick_widget   = NULL;

/* The joystick should only be queried, if 'joystick_widget' has
   focus.  This variable is used to store the focus information.  */
static gboolean   joystick_has_focus= FALSE;


void
joystick_set_properties (guint32 deviceid, gfloat switch_level)
{
  joystick_deviceid= deviceid;
  if (joystick_deviceid != GDK_CORE_POINTER)
    gdk_input_set_mode (joystick_deviceid, GDK_MODE_SCREEN);

  joystick_switch_level= switch_level;
}


static void
joystick_get_information (gint *x_direction, gint *y_direction)
{
  /* FIXME: This function should only return a joystick movement, if
     the game_widget has focus.  */
  if (joystick_deviceid != GDK_CORE_POINTER && joystick_has_focus)
    {
      gdouble x;
      gdouble y;
      
      gint xi;
      gint yi;
      
      gint xdir= 0;
      gint ydir= 0;
      
      GdkModifierType mask;
      
      gdk_input_window_get_pointer (GTK_WIDGET (joystick_widget)->window, 
				    joystick_deviceid, 
				    &x, &y, NULL, NULL, NULL, &mask);
      gdk_window_get_origin (GTK_WIDGET (joystick_widget)->window, &xi, &yi);
      
      /* Unfortunatly there is now way, to get the joystick values
         directly.  So we have to revert the calculation that GDK does
         here.  Propably one should extend GDK with an input mode like
         GDK_MODE_RAW.  */

      x= ((gdouble) xi+x)/gdk_screen_width ()*100.0;
      y= ((gdouble) yi+y)/gdk_screen_height ()*100.0;
      
      if (x > joystick_switch_level)
	xdir= 1;
      else if (x < -joystick_switch_level)
	xdir= -1;
      else if (y > joystick_switch_level)
	ydir= 1;
      else if (y < -joystick_switch_level)
	ydir= -1;
      
      if (x_direction) *x_direction= xdir;
      if (y_direction) *y_direction= ydir;
    }
}


static gint
joystick_focus_change_event (GtkWidget     *widget,
			     GdkEventFocus *event,
			     gpointer       client_data)
{
  joystick_has_focus= (event->in != 0);
  
  return TRUE;
}

static void
joystick_set_widget (GtkWidget *widget)
{
  g_return_if_fail (widget);

  gtk_widget_set_extension_events (widget, GDK_EXTENSION_EVENTS_ALL);
  
  gtk_widget_set_events (widget, gtk_widget_get_events (widget) | 
			 GDK_FOCUS_CHANGE);
  gtk_signal_connect (GTK_OBJECT (widget), "focus_in_event",
		      (GtkSignalFunc) joystick_focus_change_event, 0);
  gtk_signal_connect (GTK_OBJECT (widget), "focus_out_event",
		      (GtkSignalFunc) joystick_focus_change_event, 0);

  joystick_widget= widget;
}



/*****************************************************************************/


CaveFlags flags= 0;

static gint
iteration_timeout_function (gpointer data)
{
  GStonesCave *cave  = (GStonesCave *)data;
  gint         retval= TRUE;

  if (state == STATE_TITLE)
    return TRUE;

  joystick_get_information (&player_x_direction, &player_y_direction);
  
  cave_iterate (cave, player_x_direction, player_y_direction, player_push);

#ifndef USE_KEY_RELEASE
  player_x_direction= 0;
  player_y_direction= 0;
#endif 
  
  if (!(flags & CAVE_FINISHED) && (cave->flags & CAVE_FINISHED))
    {
      if (!(cave->flags & CAVE_PLAYER_EXISTS) && !cave->is_intermission)
	{
	  player_inc_lives (player, -1);
	  
	  if (player->lives != 0)
	    { 
	      /* Restart cave.  */
	      GStonesCave *newcave= gstones_cave_load (game, cave->name);
	      
	      curtain_start (newcave);
	    }
	  else
	    {
	      /* That's it.  No lives anymore.  */
	      gint pos= gnome_score_log (player->score, NULL, TRUE);

	      /* FIXME: somewhere is a bug, that makes this needed.  */
	      if (pos > 10) pos= 0;
	      
	      gnome_scores_display ("Gnome-Stones", APP_NAME, NULL, pos);
	      gnome_app_flash (GNOME_APP (app), _("Game over!"));
	      curtain_start (NULL);
	    }
	}
      else
	countdown_start (cave);
      
      retval= TRUE;
      
    }

  /* Manage scrolling.  */
  view_calculate_offset (GSTONES_VIEW (gstones_view), cave);

#ifdef USE_GNOME_CANVAS  
  game_update_image (gstones_view);
#else
  gtk_widget_draw (gstones_view, NULL);
#endif
  flags= cave->flags;

  return retval;
}


static void
iteration_start (GStonesCave *newcave)
{
  /* FIXME: bad hack.  */
  cave = newcave;
  flags= 0;

  cave_set_player (cave, player);
  status_set_cave (cave->name);
  
  iteration_timeout= 
    gtk_timeout_add (game->frame_rate, iteration_timeout_function, cave);
}



/****************************************************************************/
/* Additional game stuff.  */


void
load_game (const gchar *filename, guint _start_cave)
{
  char         buffer[256];
  GStonesGame *newgame;
  
  g_return_if_fail (filename);
  
  /* We don't need to load a game twice.  */
  if (game && (strcmp (game->filename, filename) == 0))
    {
      /* Give feedback in statusbar.  */
      sprintf (buffer, "Game '%s' already loaded.", filename);
      gnome_app_flash (GNOME_APP (app), buffer);
      return;
    }
  
  /* Load game.  */
  newgame= gstones_game_load (filename);

  if (newgame)
    {
      /* Remove all running timeouts.  */
      gtk_timeout_remove (iteration_timeout);
      gtk_timeout_remove (countdown_timeout);

      /* Set display into title mode.  */
      view_display_image (GSTONES_VIEW (gstones_view), title_image);
      view_set_curtain_mode (GSTONES_VIEW (gstones_view), 
			     VIEW_CURTAIN_OPEN, NULL);

      /* Set program state into title state.  */
      state= STATE_TITLE;

      /* Remove any existing game and cave.  */
      if (cave) cave_free (cave);
      if (game) game_free (game);

      /* Set the global variables, concerning the game to some default
         values.  We don't need to remove 'player'.  This will be done
         automatically, when starting a game.  */
      cave      = NULL;
      game      = newgame;
      start_cave= _start_cave;

      /* Update the title image.  */
      game_update_title ();
      
      /* Give feedback in statusbar.  */
      sprintf (buffer, "Game '%s' loaded.", filename);
      gnome_app_flash (GNOME_APP (app), buffer);
    }
}



/*****************************************************************************/
/* Menu callback functions */


static void
game_start_cb (GtkWidget *widget, gpointer data)
{
  GList       *tmp = NULL;
  GStonesCave *cave= NULL;
  
  if (!game) return;
  
  /* Initialize the player information.  */
  if (player) player_free (player);
  player= player_new (game);
  if (!player) return;

  tmp = g_list_nth (game->start_caves, start_cave);
  cave= gstones_cave_load (game, tmp->data);
  
  if (cave)
    curtain_start (cave);
}


static void
game_pause_cb (GtkWidget *widget, gpointer data)
{
  if ((state == STATE_RUNNING) && cave)
    {
      cave_toggle_pause_mode (cave);
      
      if (cave->flags & CAVE_PAUSING)
	    gnome_appbar_set_status (GNOME_APPBAR (statusbar), _("Pause"));
      else
	gnome_appbar_refresh (GNOME_APPBAR (statusbar));
    }
}


static void
preferences_cb (GtkWidget *widget, gpointer data)
{
  preferences_dialog_show ();
}


static void
quit_cb (GtkWidget *widget, gpointer data)
{
  preferences_save_global ();

  exit (0);
}


static void
show_scores_cb (GtkWidget *widget, gpointer data)
{
  gnome_scores_display ("Gnome-Stones", APP_NAME, NULL, GPOINTER_TO_INT (data));
}


static void
about_cb (GtkWidget *widget, gpointer data)
{
  GtkWidget *about= NULL;
  
  const gchar *authors[]= {
    "Carsten Schaar <nhadcasc@fs-maphy.uni-hannover.de>",
    NULL
  };
  
  about= gnome_about_new (_("Gnome-Stones"), VERSION,
			  "(C) 1998 Carsten Schaar",
			  authors,
			  _("A game."),
			  NULL);
  gtk_widget_show (about);
}



/*****************************************************************************/
/* Menu definitions */


static GnomeUIInfo game_menu[]= {
  GNOMEUIINFO_MENU_NEW_GAME_ITEM(game_start_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_PAUSE_GAME_ITEM (game_pause_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_SCORES_ITEM(show_scores_cb, NULL),
  GNOMEUIINFO_SEPARATOR,
  GNOMEUIINFO_MENU_EXIT_ITEM(quit_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[]= {
  GNOMEUIINFO_HELP(APP_NAME),
  GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu[]= {
  GNOMEUIINFO_MENU_PREFERENCES_ITEM(preferences_cb, NULL),
  GNOMEUIINFO_END
};

static GnomeUIInfo main_menu[]= 
{
  GNOMEUIINFO_MENU_GAME_TREE(game_menu),
  GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
  GNOMEUIINFO_MENU_HELP_TREE(help_menu),
  GNOMEUIINFO_END
};



/*****************************************************************************/
/* Startup sequence.  */


static gboolean
scan_public_game_directory (void)
{
  game_directory_scan (CAVESDIR);

  return TRUE;
}


static gboolean
scan_private_game_directory (void)
{
  char *dir= gnome_util_home_file ("gnomes-stones/games");
  
  game_directory_scan (dir);

  g_free (dir);
  return TRUE;
}


static gboolean
scan_public_plugin_directory (void)
{
  char *dir= gnome_unconditional_libdir_file ("gnome-stones/objects/");

  plugin_load_plugins_in_dir (dir);

  g_free (dir);
  return TRUE;
}


static gboolean
scan_private_plugin_directory (void)
{
  char *dir= gnome_util_home_file ("gnomes-stones/objects");
  
  plugin_load_plugins_in_dir (dir);

  g_free (dir);
  return TRUE;
}


/*****************************************************************************/
/* Startup sequence.  */


typedef struct
{
  gboolean (*function)(void);
  char     *description;
} StartupSequence;


StartupSequence startup_sequence[]=
{
  {
    scan_private_plugin_directory,
    N_("Scanning private object directory...")
  },
  {
    scan_public_plugin_directory,
    N_("Scanning public object directory...")
  },
#ifdef USE_GNOME_CANVAS
  {
    game_widget_fill,
    N_("Filling game widget...")
  },
#endif /* USE_GNOME_CANVAS */
  {
    scan_private_game_directory,
    N_("Scanning private game directory...")
  },
  {
    scan_public_game_directory,
    N_("Scanning public game directory...")
  },
  {
    preferences_restore,
    N_("Restoring preferences...")
  }
};


static gint startup_function (gpointer data)
{
  static guint index= 0;
  static guint max  = 0;
  
  if (max == 0)
    max= sizeof (startup_sequence)/sizeof (startup_sequence[0]);

  gnome_appbar_set_status (GNOME_APPBAR (statusbar), 
			   _(startup_sequence[index].description));
  
  startup_sequence[index].function ();
  
  gnome_appbar_set_progress (GNOME_APPBAR (statusbar),
			     (gfloat) (index+1)/(gfloat) max);
  index++;

  if (index == max)
    {
      state= STATE_TITLE;
      gnome_appbar_refresh (GNOME_APPBAR (statusbar));
    }
  
  return (index != max);
}



/*****************************************************************************/
/* Main function.  */


int
main (int argc, char *argv[])
{
  GtkWidget *frame= NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *table= NULL;
  
  gnome_score_init (APP_NAME);

  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  gnome_init_with_popt_table (APP_NAME, VERSION, argc, argv, options, 0, NULL);
  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-stones.png");
  /* That's what a gnome application needs:  */
  app= gnome_app_new ("gnome-stones", _("Gnome-Stones"));
  gtk_window_set_policy  (GTK_WINDOW (app), FALSE, FALSE, TRUE);

  /* ... a menu line, ... */
  gnome_app_create_menus   (GNOME_APP (app), main_menu);

  /* ... a toolbar, ... */
  /*  gnome_app_create_toolbar (GNOME_APP (app), toolbar); */

  /* ... a statusline ... */
  statusbar= gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_USER);
  gnome_app_set_statusbar (GNOME_APP (app), statusbar);

  gnome_appbar_set_default (GNOME_APPBAR (statusbar), _(default_message));

  gnome_app_install_menu_hints(GNOME_APP (app), main_menu);

  /* and, last but not least the game display.  */
  vbox = gtk_vbox_new (FALSE, 0);

  gstones_view= view_new (load_image_from_path ("gnome-stones/curtain.png"));

  frame= gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_widget_show (frame);
  gtk_container_add (GTK_CONTAINER (frame), gstones_view);

  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);

  table= status_widget_get ();
  gtk_widget_show (table);
  gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
  
  gnome_app_set_contents (GNOME_APP (app), vbox);
  
  gtk_widget_set_events (app, gtk_widget_get_events (app) | GAME_EVENTS);

  gtk_signal_connect (GTK_OBJECT (app), "key_press_event",
		      GTK_SIGNAL_FUNC (game_widget_key_press_callback), 0);
  gtk_signal_connect (GTK_OBJECT (app), "key_release_event",
		      GTK_SIGNAL_FUNC (game_widget_key_release_callback), 0);
  gtk_signal_connect (GTK_OBJECT (app), "delete_event",
		      GTK_SIGNAL_FUNC (gtk_widget_destroy), 0);
  gtk_signal_connect (GTK_OBJECT (app), "destroy",
		      GTK_SIGNAL_FUNC (gtk_main_quit), 0);

  joystick_set_widget (app);

  gtk_widget_realize (app);

  gtk_widget_show (app);

  title_image_load ();
  gdk_pixmap_ref (title_image);
  view_display_image (GSTONES_VIEW (gstones_view), title_image);

  session_management_init ();

  gtk_idle_add (startup_function, NULL);

  gtk_main ();

  return 0;
}


/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
