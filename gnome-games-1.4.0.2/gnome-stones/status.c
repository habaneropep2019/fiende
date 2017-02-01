/* gnome-stones - status.c
 *
 * Time-stamp: <1998/10/15 20:35:32 carsten>
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
#include "status.h"



/*****************************************************************************/


GtkWidget *statusbar= NULL;
GtkWidget *app      = NULL;


/*****************************************************************************/


static GtkWidget *status_vbox= NULL;
static GtkWidget *cave_table = NULL;
static GtkWidget *game_table = NULL;

static GtkWidget *status_score_label;
static GtkWidget *status_diamonds_label;
static GtkWidget *status_cave_label;
static GtkWidget *status_lives_label;





GtkWidget *
status_widget_get (void)
{
  GtkWidget *label;

  if (cave_table)
    return cave_table;

  /* Create the status line widget.  It shows the following data:
     + Number of collected diamonds
     + Number of diamonds to collect
     + Time to go
  */
  
  /* Information about the currently played cave.  */
  
  cave_table= gtk_table_new (1, 8, FALSE);
  
  label= gtk_label_new (_("Score:"));
  gtk_table_attach (GTK_TABLE (cave_table), label, 0, 1, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (label);

  status_score_label= gtk_label_new ("0");
  gtk_table_attach (GTK_TABLE (cave_table), status_score_label, 1, 2, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (status_score_label);
  
  gtk_table_set_col_spacing (GTK_TABLE (cave_table), 2, 32);
  
  label= gtk_label_new (_("Diamonds:"));
  gtk_table_attach (GTK_TABLE (cave_table), label, 3, 4, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (label);
  
  status_diamonds_label= gtk_label_new ("0");
  gtk_table_attach (GTK_TABLE (cave_table), status_diamonds_label, 4, 5, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (status_diamonds_label);
  
  gtk_table_set_col_spacing (GTK_TABLE(cave_table), 5, 32);
  
  /* Information about the currently played game.  */
  
  game_table= gtk_table_new (1, 8, FALSE);
  
  label= gtk_label_new (_("Cave:"));
  gtk_table_attach (GTK_TABLE (game_table), label, 0, 1, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (label);

  status_cave_label= gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (game_table), status_cave_label, 1, 2, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (status_cave_label);
  
  gtk_table_set_col_spacing (GTK_TABLE (game_table), 2, 32);
  
  label= gtk_label_new (_("Lives:"));
  gtk_table_attach (GTK_TABLE (game_table), label, 3, 4, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (label);
  
  status_lives_label= gtk_label_new ("0");
  gtk_table_attach (GTK_TABLE (game_table), status_lives_label, 4, 5, 0, 1, 0, 0, 3, 3);
  gtk_widget_show (status_lives_label);
  
  gtk_table_set_col_spacing (GTK_TABLE(game_table), 5, 32);
  

  status_vbox= gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (status_vbox), cave_table, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (status_vbox), game_table, TRUE, TRUE, 0);
  
  gtk_widget_show (status_vbox);


  return status_vbox;
}


void
status_set_mode (StatusType type)
{
  if (type == STATUS_CAVE_INFO)
    {
      gtk_widget_hide (game_table);
      gtk_widget_show (cave_table);
    }
  else if (type == STATUS_GAME_INFO)
    {
      gtk_widget_hide (cave_table);
      gtk_widget_show (game_table);
    }
}


/* Setting cave informtion.  */

static gfloat maxtimer;


void
status_set_timer (guint time)
{
  gnome_appbar_set_progress (GNOME_APPBAR (statusbar), (gfloat) time/maxtimer);
}


void
status_set_maxtimer (guint time)
{
  maxtimer= time;
  
  gnome_appbar_set_progress (GNOME_APPBAR (statusbar), 0.0);
}



/*****************************************************************************/
/* Setting the information.  */

static guint status_score= 0;
static guint status_lives= 0;


void status_set_score (guint score)
{
  if (score != status_score)
    {
      gchar buffer[32];
      
      status_score= score;
      
      sprintf (buffer, "%d", status_score);
      gtk_label_set (GTK_LABEL (status_score_label), buffer);
    }
}


void
status_set_lives (guint lives)
{
  if (lives != status_lives)
    {
      gchar buffer[32];
      
      status_lives= lives;
      
      sprintf (buffer, "%d", status_lives);
      gtk_label_set (GTK_LABEL (status_lives_label), buffer);
    }
}


void
status_set_diamonds (guint diamonds)
{
  gchar buffer[32];

  sprintf (buffer, "%d", diamonds);
  gtk_label_set (GTK_LABEL (status_diamonds_label), buffer);
}


void
status_set_cave (char *cave)
{
  gtk_label_set (GTK_LABEL (status_cave_label), cave);
}


void
status_set_time (gfloat time)
{
  gnome_appbar_set_progress (GNOME_APPBAR (statusbar), time);
}

/* Setting text line information.  */

void
status_set_message (const gchar *message)
{
  gnome_appbar_set_status (GNOME_APPBAR (statusbar), message);
}


/*****************************************************************************/

void
gstone_error (const gchar *error)
{
  if (app)
    gnome_app_error (GNOME_APP (app), error);
}



/* Local Variables: */
/* mode:c */
/* eval:(load-library "time-stamp") */
/* eval:(make-local-variable 'write-file-hooks) */
/* eval:(add-hook 'write-file-hooks 'time-stamp) */
/* eval:(setq time-stamp-format '(time-stamp-yyyy/mm/dd time-stamp-hh:mm:ss user-login-name)) */
/* End: */
