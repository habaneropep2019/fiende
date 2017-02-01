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
#include "gnome.h"

#define ZOOM_WIDTH            500
#define ZOOM_HEIGHT           300


/*
 *  --------------------------
 *  Local and global variables
 *  --------------------------
 */

int zoom_visible;
GtkWidget *zoom_dialog;
GtkWidget *zoom_canvas;

extern ConfigData *cfg;
extern GdkGC *gc;
extern Log *curlog;
extern char *month[12];
extern GList *regexp_db, *descript_db;

void close_zoom_view (GtkWidget * widget, GtkWindow ** window);
void create_zoom_view (GtkWidget * widget, gpointer user_data);
int match_line_in_db (LogLine *line, GList *db);
void draw_parbox (GdkDrawable *win, GdkFont *font, GdkGC *gc,  \
	     int x, int y, int width, char *text, int max_num_lines);

/* ----------------------------------------------------------------------
   NAME:          create_zoom_view
   DESCRIPTION:   Display the statistics of the log.
   ---------------------------------------------------------------------- */

void
create_zoom_view (GtkWidget * widget, gpointer user_data)
{
   GtkWidget *frame;
   GtkWidget *vbox;
   int h1, h2, height;

   if (curlog == NULL || zoom_visible)
      return;

   zoom_dialog  = gtk_window_new (GTK_WINDOW_TOPLEVEL);
   gtk_signal_connect (GTK_OBJECT (zoom_dialog), "destroy",
		       (GtkSignalFunc) close_zoom_view,
		       &zoom_dialog);
   gtk_signal_connect (GTK_OBJECT (zoom_dialog), "delete_event",
		       (GtkSignalFunc) close_zoom_view,
		       &zoom_dialog);
   gtk_window_set_title (GTK_WINDOW (zoom_dialog), _("Zoom view"));
   gtk_container_set_border_width (GTK_CONTAINER (zoom_dialog), 0);
   gtk_widget_set_style (zoom_dialog, cfg->main_style);
   gtk_widget_realize (zoom_dialog);

   vbox = gtk_vbox_new (FALSE, 2);
   gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
   gtk_container_add (GTK_CONTAINER (zoom_dialog), vbox);
   gtk_widget_show (vbox);

   /* Create frame for main view */
   frame = gtk_frame_new (NULL);
   gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
   gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
   gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
   gtk_widget_set_style (frame, cfg->main_style);
   gtk_widget_show (frame);


   /* Create drawing area where data will be drawn */
   zoom_canvas = gtk_drawing_area_new ();
   gtk_signal_connect (GTK_OBJECT (zoom_canvas), "expose_event",
		       (GtkSignalFunc) repaint_zoom, zoom_canvas);

   h1 = cfg->headingb->descent + cfg->headingb->ascent;
   h2 = cfg->fixedb->descent + cfg->fixedb->ascent+2;
   height = h1*2+14*h2;
   gtk_drawing_area_size (GTK_DRAWING_AREA (zoom_canvas), ZOOM_WIDTH, height);
   gtk_widget_set_events (zoom_canvas, GDK_EXPOSURE_MASK);
   /* gtk_widget_set_style (zoom_canvas, cfg->black_bg_style); */
   gtk_widget_set_style (zoom_canvas, cfg->white_bg_style);
   gtk_container_add (GTK_CONTAINER (frame), zoom_canvas);
   gtk_widget_show (zoom_canvas);
   gtk_widget_realize (zoom_canvas);

   gtk_widget_show (zoom_dialog);

   zoom_visible = TRUE;
}

/* ----------------------------------------------------------------------
   NAME:          repaint_zoom
   DESCRIPTION:   Repaint the zoom window.
   ---------------------------------------------------------------------- */

int
repaint_zoom (GtkWidget * widget, GdkEventExpose * event)
{
   static GdkDrawable *canvas;
   char buffer[1024];
   int x, y, h, w, win_width;
   int win_height;
   GdkRectangle *area;
   LogLine *line;
   struct tm date = {0};

   canvas = zoom_canvas->window;
   win_width = zoom_canvas->allocation.width;
   win_height = zoom_canvas->allocation.height;


   /* Get area that was exposed */
   area = NULL;
   if (event != NULL)
       area = &event->area;

   /* Draw title */
   h = cfg->headingb->descent + cfg->headingb->ascent;
   w = gdk_string_measure (cfg->fixedb, "X");
   x = 5;
   y = cfg->headingb->ascent + 6;
   gdk_gc_set_foreground (gc, &cfg->blue);
   gdk_draw_rectangle (canvas, gc, TRUE, 3, 3, win_width - 6, h+6);
   gdk_gc_set_foreground (gc, &cfg->white);
   if (curlog != NULL)
     g_snprintf (buffer, sizeof (buffer),
		 _("Log line detail for %s"), curlog->name);
   else
     g_snprintf (buffer, sizeof (buffer),
		 _("Log line detail for <No log loaded>"));
   gdk_draw_string (canvas, cfg->headingb, gc, x, y, buffer);
   y += 9 + cfg->headingb->descent + cfg->fixedb->ascent;


   /* Draw Info */
   gdk_gc_set_foreground (gc, &cfg->black);
   /* gdk_gc_set_foreground (gc, &cfg->white); */

   h = cfg->fixedb->descent + cfg->fixedb->ascent+2;

   /* Draw bg. rectangles */
   gdk_gc_set_foreground (gc, &cfg->blue3);
   gdk_draw_rectangle (canvas, gc, TRUE, 15*w+6, 
		       y - cfg->fixed->ascent-3, 
		       win_width - 9 - 15*w, 
		       win_height - (y - cfg->fixed->ascent) );
   gdk_gc_set_foreground (gc, &cfg->gray75);
   gdk_draw_rectangle (canvas, gc, TRUE, 3, 
		       y-cfg->fixed->ascent-3, 
		       15*w, win_height - (y - cfg->fixed->ascent) );

   gdk_gc_set_foreground (gc, &cfg->black);
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Date:"));
   y += h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Process:"));
   y += h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Message:"));
   y += 4*h;
   gdk_draw_string (canvas, cfg->fixedb, gc, x, y, _("Description:"));

   gdk_gc_set_font (gc, cfg->fixed);

   /* Check that there is at least one log */
   if (curlog == NULL)
      return -1;

   /* Draw line info */
   h = cfg->headingb->descent + cfg->headingb->ascent;
   y = cfg->headingb->ascent + 6;
   y += 9 + cfg->headingb->descent + cfg->fixedb->ascent;
   x = 15*w + 6 + 2;
   h = cfg->fixedb->descent + cfg->fixedb->ascent+2;

   line = &(curlog->pointerpg->line[curlog->pointerln]);

   date.tm_mon = line->month;
   date.tm_year = 70 /* bogus */;
   date.tm_mday = line->date;
   date.tm_hour = line->hour;
   date.tm_min = line->min;
   date.tm_sec = line->sec;
   date.tm_isdst = 0;

   /* Translators: do not include year, it would be bogus */
   if (strftime (buffer, sizeof (buffer), _("%B %e %X"), &date) <= 0) {
	   /* as a backup print in US style */
	   g_snprintf (buffer, sizeof (buffer),
		       "%s %d %02d:%02d:%02d",
		       _(month[(int)line->month]), 
		       (int) line->date,
		       (int) line->hour,
		       (int) line->min,
		       (int) line->sec);
   }
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);
   y += h;
   g_snprintf (buffer, sizeof (buffer), "%s ", line->process);
   gdk_draw_string (canvas, cfg->fixed, gc, x, y, buffer);
   y += h;
   draw_parbox (canvas, cfg->fixed, gc, x, y, 380, line->message, 4);
   y += 4*h;
   if (match_line_in_db (line, regexp_db))
     {
       if (find_tag_in_db (line, descript_db))
	 draw_parbox (canvas, cfg->fixed, gc, x, y, 380,
			  line->description->description, 8);
     }

   return TRUE;
}


/* ----------------------------------------------------------------------
   NAME:          draw_parbox
   DESCRIPTION:   Draw the given text in a box of a given width breaking
                  words. The width is given in pixels.
   ---------------------------------------------------------------------- */

void
draw_parbox (GdkDrawable *win, GdkFont *font, GdkGC *gc, 
	     int x, int y, int width, char *text, int max_num_lines)
{
   char *p1, *p2, *p3, c;
   char *buffer;
   int  ypos, done;

   ypos = y;

   /* Copy string and modify our buffer string */
   buffer = g_strdup(text);

   done = FALSE;
   p1 = p2 = buffer;
   while (!done)
     {
       c = *p1; *p1 = 0;
       while ( gdk_string_measure (font, p2) < width && c != 0)
         {
	   *p1 = c;
	   p1++;
	   c = *p1;
	   *p1 = 0;
         }
       switch (c)
         {
         case ' ':
	   p1++;
	   gdk_draw_string (win, font, gc, x, ypos, p2);
	   break;
         case '\0':
	   done = TRUE;
	   gdk_draw_string (win, font, gc, x, ypos, p2);
	   break;
         default:
	   p3 = p1;
	   while (*p1 != ' ' && p1 > p2)
	     p1--;
	   if (p1 == p2)
	     {
               gdk_draw_string (win, font, gc, x, ypos, p2);
               *p3 = c;
               p1 = p3;
               break;
	     }
	   else
	     {
               *p3 = c;
               *p1 = 0;
               p1++;
               gdk_draw_string (win, font, gc, x, ypos, p2);
	     }
	   break;
         }
       ypos += font->descent + font->ascent;
       p2 = p1;
     }
   
   g_free (buffer);

}


/* ----------------------------------------------------------------------
   NAME:          CloseLogInfo
   DESCRIPTION:   Callback called when the log info window is closed.
   ---------------------------------------------------------------------- */

void
close_zoom_view (GtkWidget * widget, GtkWindow ** window)
{
   if (zoom_visible)
      gtk_widget_hide (zoom_dialog);
   zoom_dialog = NULL;
   zoom_visible = FALSE;
}

