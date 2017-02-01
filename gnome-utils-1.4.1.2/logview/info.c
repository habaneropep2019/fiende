/*  ----------------------------------------------------------------------

    Copyright (C) 1998  Cesar Miquel  (miquel@df.uba.ar)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    ---------------------------------------------------------------------- */


#include <config.h>
#include <gnome.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include "logview.h"
#if 0
#include "close.xpm"
#endif

#define INFO_WIDTH            350
#define INFO_HEIGHT           140


void LogInfo (GtkWidget * widget, gpointer user_data);
void CloseLogInfo (GtkWidget * widget, GtkWindow ** window);
int RepaintLogInfo (GtkWidget * widget, GdkEventExpose * event);



/*
 *  --------------------------
 *  Local and global variables
 *  --------------------------
 */

int loginfovisible;
GtkWidget *InfoDialog;
GtkWidget *info_canvas;

extern ConfigData *cfg;
extern GdkGC *gc;
extern Log *curlog;


/* ----------------------------------------------------------------------
   NAME:          LogInfo
   DESCRIPTION:   Display the statistics of the log.
   ---------------------------------------------------------------------- */

void
LogInfo (GtkWidget * widget, gpointer user_data)
{
   GtkWidget *frame;
   GtkWidget *vbox;
   int h1, h2;

   if (curlog == NULL || loginfovisible)
      return;

   InfoDialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
   gtk_signal_connect (GTK_OBJECT (InfoDialog), "destroy",
		       (GtkSignalFunc) CloseLogInfo,
		       &InfoDialog);
   gtk_signal_connect (GTK_OBJECT (InfoDialog), "delete_event",
		       (GtkSignalFunc) CloseLogInfo,
		       &InfoDialog);
   gtk_window_set_title (GTK_WINDOW (InfoDialog), _("Log stats"));
   gtk_container_set_border_width (GTK_CONTAINER (InfoDialog), 0);
   gtk_widget_set_style (InfoDialog, cfg->main_style);
   gtk_widget_realize (InfoDialog);

   vbox = gtk_vbox_new (FALSE, 2);
   gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
   gtk_container_add (GTK_CONTAINER (InfoDialog), vbox);
   gtk_widget_show (vbox);

   frame = gtk_frame_new (NULL);
   gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
   gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
   gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
   gtk_widget_set_style (frame, cfg->main_style);
   gtk_widget_show (frame);

   info_canvas = gtk_drawing_area_new ();
   gtk_signal_connect (GTK_OBJECT (info_canvas), "expose_event",
		       (GtkSignalFunc) RepaintLogInfo, NULL);

   h1 = cfg->headingb->descent + cfg->headingb->ascent;
   h2 = cfg->fixedb->descent + cfg->fixedb->ascent+2;
   gtk_drawing_area_size (GTK_DRAWING_AREA (info_canvas), INFO_WIDTH, h1*2+8*h2);
   gtk_widget_set_events (info_canvas, GDK_EXPOSURE_MASK);
   gtk_widget_set_style (info_canvas, cfg->white_bg_style);
   gtk_container_add (GTK_CONTAINER (frame), info_canvas);
   gtk_widget_show (info_canvas);
   gtk_widget_realize (info_canvas);

   gtk_widget_show (InfoDialog);

   loginfovisible = TRUE;
}

/* ----------------------------------------------------------------------
   NAME:          RepaintLogInfo
   DESCRIPTION:   Repaint the log info window.
   ---------------------------------------------------------------------- */

int
RepaintLogInfo (GtkWidget * widget, GdkEventExpose * event)
{
   static GdkDrawable *canvas;
   char buffer[1024];
   int x, y, h, w;
   int win_width, win_height;

   canvas = info_canvas->window;
   win_width = info_canvas->allocation.width;
   win_height = info_canvas->allocation.height;

   /* Draw title */
   h = cfg->headingb->descent + cfg->headingb->ascent;
   w = gdk_string_measure (cfg->fixedb, "X");
   x = 5;
   y = cfg->headingb->ascent + 6;
   gdk_gc_set_foreground (gc, &cfg->blue);
   gdk_draw_rectangle (canvas, gc, TRUE, 3, 3, win_width - 6, h+6);
   gdk_gc_set_foreground (gc, &cfg->white);
   gdk_draw_string (canvas, cfg->headingb, gc, x+3, y, _("Log information"));
   y += 9 + cfg->headingb->descent + cfg->fixedb->ascent;


   /* Draw rectangle */
   h = cfg->fixedb->descent + cfg->fixedb->ascent+2;
   gdk_gc_set_foreground (gc, &cfg->blue3);
   gdk_draw_rectangle (canvas, gc, TRUE, 15*w+6, 
		       y - cfg->fixed->ascent-3, 
		       win_width - 9 - 15*w, 
		       win_height - (y - cfg->fixed->ascent));
   gdk_gc_set_foreground (gc, &cfg->gray75);
   gdk_draw_rectangle (canvas, gc, TRUE, 3, y-cfg->fixed->ascent-3, 
		       15*w, win_height - (y - cfg->fixed->ascent));

   /* Draw Info */
   gdk_gc_set_foreground (gc, &cfg->black);
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Log:"));
   y += h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Size:"));
   y += h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Modified:"));
   y += h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Start date:"));
   y += h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Last date:"));
   y += h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Num. lines:"));

   gdk_gc_set_font (gc, cfg->fixed);

   /* Check that there is at least one log */
   if (curlog == NULL)
      return -1;

   h = cfg->headingb->descent + cfg->headingb->ascent;
   y = cfg->headingb->ascent + 6;
   y += 9 + cfg->headingb->descent + cfg->fixedb->ascent;
   x = 15*w + 6 + 2;
   g_snprintf (buffer, sizeof (buffer), "%s", curlog->name);
   h = cfg->fixedb->descent + cfg->fixedb->ascent+2;
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);
   y += h;
   g_snprintf (buffer, sizeof (buffer),
	      _("%ld bytes"), (long) curlog->lstats.size);
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);
   y += h;
   g_snprintf (buffer, sizeof (buffer), "%s ", ctime (&curlog->lstats.mtime));
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);
   y += h;
   g_snprintf (buffer, sizeof (buffer), "%s ",
	      ctime (&curlog->lstats.startdate));
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);
   y += h;
   g_snprintf (buffer, sizeof (buffer), "%s ", ctime (&curlog->lstats.enddate));
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);
   y += h;
   g_snprintf (buffer, sizeof (buffer), "%ld ", curlog->lstats.numlines);
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);

   return TRUE;
}

/* ----------------------------------------------------------------------
   NAME:          CloseLogInfo
   DESCRIPTION:   Callback called when the log info window is closed.
   ---------------------------------------------------------------------- */

void
CloseLogInfo (GtkWidget * widget, GtkWindow ** window)
{
   if (loginfovisible)
      gtk_widget_hide (InfoDialog);
   InfoDialog = NULL;
   loginfovisible = FALSE;
}

