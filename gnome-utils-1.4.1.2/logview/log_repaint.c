
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
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <gnome.h>
#include "logview.h"
#include "logrtns.h"

/*
 * -----------------
 * Local definitions
 * ------------------
 */

#define LOG_COL1                 7
#define LOG_COL2                 70
#define LOG_COL3                 170
#define LOG_COL4                 200
#define LOG_COL5                 300
#define LOG_COL6                 340
#define LOG_COL7                 400
#define LOG_TITLEY               10

#define MAX_NUM_DPY_LINES        300
#define MAX_DPY_NUM_CHARS        200
#define DEF_NUM_LOGS             1

#define FRMT_HEADING             1
#define FRMT_NORMAL              2


/* Global variables */

extern int loginfovisible, calendarvisible;
extern int zoom_visible;
extern ConfigData *cfg;
extern GtkWidget *main_win_scrollbar;
extern GtkLabel *filename_label, *date_label;
extern UserPrefsStruct *user_prefs;


/*
 * -------------------
 * Module variables 
 * -------------------
 */


GdkGC *gc;
GdkDrawable *canvas;

Log *loglist[MAX_NUM_LOGS];
Log *curlog;
int numlogs, curlognum;
int log_line_sep;
int cursor_visible;
int canvas_width, canvas_height;
char *deflognames[] =
{PATH_MESSAGES};

char *month[12] =
{N_("January"), N_("February"), N_("March"), N_("April"), N_("May"),
 N_("June"), N_("July"), N_("August"), N_("September"), N_("October"),
 N_("November"), N_("December")};
extern GtkWidget *main_win_scrollbar;

/*
 * -------------------
 * Function prototypes
 * -------------------
 */

int InitPages (void);
int HandleLogEvents (void);
int GetLineAtCursor (int y);
int NumTextLines (int l);
int RepaintCalendar (GtkWidget * widget, GdkEventExpose * event);
int RepaintLogInfo (GtkWidget *, GdkEventExpose *);
int rapaint_zoom (GtkWidget * widget, GdkEventExpose * event);
gboolean HandleLogKeyboard (GtkWidget * win, GdkEventKey * event_key);
gboolean PointerMoved (GtkWidget * cv, GdkEventMotion * event);
gboolean log_repaint (GtkWidget * cv, GdkEventExpose * event);
void log_redrawcursor (int ol, int nl, Page * np);
void log_redrawdetail (void);
void DrawMonthHeader (LogLine * line, int y);
void DrawLogLine (LogLine *line, int y);
void DrawLogCursor (int y);
void EraseLogCursor (int y);
void Draw3DBox (GdkDrawable *win, GdkGC *gc, int x, int y, int w, int h, GdkColor color[3]);
void CloseApp (void);
void UpdateStatusArea (void);
void change_log (int direction);
void create_zoom_view (GtkWidget * widget, gpointer user_data);
void close_zoom_view (GtkWidget * widget, GtkWindow ** window);
gboolean handle_log_mouse_button (GtkWidget * win, GdkEventButton * event_key);
Page *GetPageAtCursor (int y);

/* ----------------------------------------------------------------------
   NAME:        PointerMoved
   DESCRIPTION: The pointer moved inside the window.
   ---------------------------------------------------------------------- */

gboolean
PointerMoved (GtkWidget * cv, GdkEventMotion * event)
{
   int cursory, nl;
   Page *np;

   /* Check that there is at least one log */
   if (curlog == NULL)
      return FALSE;

   cursory = event->y;
   cursor_visible = TRUE;
   if ((nl = GetLineAtCursor (cursory)) != -1 && nl != curlog->pointerln)
   {
      np = GetPageAtCursor (cursory);
      log_redrawcursor (curlog->pointerln, nl, np);
      curlog->pointerln = nl;
      curlog->pointerpg = np;
      log_redrawdetail ();
   }

   return FALSE;
}


/* ----------------------------------------------------------------------
   NAME:        NumTxtLines
   DESCRIPTION: Returns the number of log-lines to read to ocupy l
   screen lines.
   ---------------------------------------------------------------------- */

int
NumTextLines (int l)
{
   int cd, cm, ln, count;
   Page *pg;
   LogLine *line;

   cd = cm = -1;
   ln = curlog->firstline;
   pg = curlog->currentpg;
   count = 0;

   while (count < l)
   {
      line = &pg->line[ln];
      if ((cd != line->date && line->date >= 0) ||
	  (cm != line->month && line->month >= 0))
      {
	 count += 2;
	 cd = line->date;
	 cm = line->month;
      }
      ln++;
      count++;
   }

   return ((ln - curlog->firstline));
}

/* ----------------------------------------------------------------------
   NAME:        handle_log_mouse_button
   DESCRIPTION: User clicked on main window: open zoom view. 
   ---------------------------------------------------------------------- */

gboolean
handle_log_mouse_button (GtkWidget * win, GdkEventButton *event)
{
  static guint32 lasttime;
  static int clicked_before = FALSE;

  if (event->type == GDK_BUTTON_PRESS && !clicked_before)
    {
      lasttime = event->time;
      clicked_before = TRUE;
      return FALSE;
    }
  
  clicked_before = FALSE;
  if (event->time - lasttime < 100 ||
      event->time - lasttime > 200)
    return FALSE;

  /* If zoom is already visible ignore */
  if (!zoom_visible)
    create_zoom_view (NULL, NULL);

  return FALSE;
}

/* ----------------------------------------------------------------------
   NAME:        HandleLogKeyboard
   DESCRIPTION: Handle all posible keyboard actions.
   ---------------------------------------------------------------------- */

gboolean
HandleLogKeyboard (GtkWidget * win, GdkEventKey * event_key)
{
  GtkAdjustment *adj;
  guint key;

  /* Check that there is at least one log */
  if (curlog == NULL)
    return FALSE;
  
  adj = GTK_ADJUSTMENT (GTK_RANGE (main_win_scrollbar)->adjustment);
  key = event_key->keyval;
  cursor_visible = FALSE;
  switch (key)
    {
    case GDK_Q:
    case GDK_q:
      CloseApp ();
      break;
      
    case GDK_plus:
      change_log(1);
      break;
      
    case GDK_minus:
      change_log(-1);
      break;
      
     case GDK_Page_Up: 
     case GDK_KP_Page_Up: 
     case GDK_R9: 
       cursor_visible = FALSE;
       gtk_adjustment_set_value (adj, (adj->value - (float) (LINES_P_PAGE >> 1)));
      break;

    case GDK_Up:
      cursor_visible = FALSE;
      gtk_adjustment_set_value (adj, (adj->value - 1.0));
      break;
      
    case GDK_Page_Down:
    case GDK_KP_Page_Down:
    case GDK_R15:
      cursor_visible = FALSE;
      gtk_adjustment_set_value (adj, (adj->value + (float) (LINES_P_PAGE >> 1)));
      break;
    case GDK_Down:
      cursor_visible = FALSE;
      gtk_adjustment_set_value (adj, (adj->value + 1.0));
      break;
      
    default:
      return FALSE;
      break;
    };
  
   return TRUE;
}

/* ----------------------------------------------------------------------
   NAME:        change_log
   DESCRIPTION: 
   ---------------------------------------------------------------------- */

void
change_log (int direction)
{
  if (numlogs == 1)
    return;
  if (direction > 0)
    {
      if (curlognum == numlogs - 1)
	curlognum = 0;
      else
	curlognum++;
    }
  else      
    {
      if (curlognum == 0)
	curlognum = numlogs - 1;
      else
	curlognum--;
    }

  curlog = loglist[curlognum];
  set_scrollbar_size (curlog->lstats.numlines);
  gtk_adjustment_set_value ( GTK_RANGE(main_win_scrollbar)->adjustment, 
			     curlog->ln);
  log_repaint (NULL, NULL);
  if (loginfovisible)
    RepaintLogInfo (NULL, NULL);
  if (calendarvisible)
    init_calendar_data ();
  UpdateStatusArea();
}

/* ----------------------------------------------------------------------
   NAME:        ScrollDown
   DESCRIPTION: Scroll the main window down.
   ---------------------------------------------------------------------- */

void
ScrollDown (int howmuch)
{
  int ln;
  Page *next;

  ln = curlog->firstline;
  ln += howmuch;

  while (ln >= LINES_P_PAGE && !curlog->currentpg->islastpage )
    {
      ln -= LINES_P_PAGE;
      next = curlog->currentpg->next;
      if (next == curlog->lastpg)
	ReadNPagesDown (curlog, next, (NUM_PAGES>>1));
      curlog->currentpg = curlog->currentpg->next;
    }

  if (ln > curlog->currentpg->ll && curlog->currentpg->islastpage)
    ln = curlog->currentpg->ll;

  curlog->firstline = ln;
}

/* ----------------------------------------------------------------------
   NAME:        ScrollUp
   DESCRIPTION: Scroll the main window up.
   ---------------------------------------------------------------------- */

void
ScrollUp (int howmuch)
{
  int ln;
  Page *next;

  ln = curlog->firstline;
  ln -= howmuch;

  if (curlog->currentpg->isfirstpage == TRUE && !curlog->currentpg->isfirstpage)
    {
      if (ln < 0)
	ln = 0;
      curlog->firstline = ln < curlog->currentpg->fl ? curlog->currentpg->fl : ln;
      return;
    }

  while (ln < 0)
    {
      ln += LINES_P_PAGE;
      next = curlog->currentpg->prev;
      if (next == curlog->firstpg)
	ReadNPagesUp (curlog, next, (NUM_PAGES>>1));
      if (curlog->currentpg->isfirstpage == FALSE)
	curlog->currentpg = curlog->currentpg->prev;
    }
  if (curlog->currentpg->isfirstpage == TRUE)
    curlog->firstline = MAX (ln, curlog->currentpg->fl);
  else
    curlog->firstline = ln;


  return;
}

/* ----------------------------------------------------------------------
   NAME:        log_repaint
   DESCRIPTION: Redraw screen.
   ---------------------------------------------------------------------- */

gboolean
log_repaint (GtkWidget * win, GdkEventExpose * event)
{
   static int firsttime = TRUE;
   int ln, y, cm, cd, offset;
   Page *pg;
   LogLine *line;
   GdkRectangle *area;

   if (firsttime)
   {
      if (win == NULL)
	 return FALSE;
      firsttime = FALSE;
      canvas = win->window;
      gc = gdk_gc_new (canvas);
      log_line_sep = cfg->fixedb->ascent + 3;
   }
   offset = LOG_TITLEY;

   area = NULL;
   if (event != NULL)
       area = &event->area;

   /* Erase page */
   if (area != NULL)
     gdk_window_clear_area (canvas, area->x, area->y, 
			    area->width, area->height);
   else
     gdk_window_clear (canvas); /* <-- This causes a LOT of flickering! */

   /* Check that there is at least one log */
   if (curlog == NULL)
      return FALSE;

   pg = curlog->currentpg;
   ln = curlog->firstline;
   cm = cd = -1;
   y = offset;

   log_redrawdetail ();

   gdk_window_get_size (canvas, &canvas_width, &canvas_height);
   while ((y - offset) < canvas_height)
   {
      line = &pg->line[ln];
      if ((line->month != cm && line->month > 0) ||
	  (line->date != cd && line->date > 0))
      {
	 DrawMonthHeader (line, y);
	 cm = line->month;
	 cd = line->date;
	 y += 2 * log_line_sep;
      }
      if (ln == curlog->pointerln && pg == curlog->pointerpg && cursor_visible)
	 DrawLogCursor (y);

      DrawLogLine (line, y);
      y += log_line_sep;
      ln++;
      if (ln > pg->ll && pg->islastpage == TRUE)
	 break;
      if (ln > pg->ll)
      {
	 if (pg->islastpage == FALSE)
	    pg = pg->next;
	 else
	    break;
	 ln = 0;
      }
   }

  /* Repaint status bar */
   UpdateStatusArea ();

   return TRUE;
}

void
UpdateStatusArea ()
{
  struct tm *tdm;
  char status_text[255];
  char *buffer;
  /* Translators: Date only format, %x should well do really */
  const char *time_fmt = _("%x"); /* an evil way to avoid warning */

  if (curlog == NULL)
    return;

  if (curlog->name != NULL)
    {
      strncpy (status_text, curlog->name, 30);
      gtk_label_get ( filename_label, (char **)&buffer);
      if (strcmp (status_text, buffer) != 0)
	gtk_label_set_text (filename_label, status_text);
    }

  tdm = &curlog->curmark->fulldate;

  if (strftime (status_text, sizeof (status_text), time_fmt, tdm) <= 0) {
	  /* as a backup print in US style */
	  g_snprintf (status_text, sizeof (status_text), "%02d/%02d/%02d", 
		      tdm->tm_mday, tdm->tm_mon, tdm->tm_year % 100);
  }
  gtk_label_get ( date_label, (char **)&buffer);
  if (strcmp (status_text, buffer) != 0)
    gtk_label_set_text (date_label, status_text);
}

/* ----------------------------------------------------------------------
   NAME:        log_redrawcursor
   DESCRIPTION: Redraw screen.
   ---------------------------------------------------------------------- */

void
log_redrawcursor (int ol, int nl, Page * np)
{
   int ln, y, cm, cd, offset;
   Page *pg, *op;
   LogLine *line;

   offset = LOG_TITLEY;

   pg = curlog->currentpg;
   op = curlog->pointerpg;
   ln = curlog->firstline;
   cm = cd = -1;
   y = offset;

   gdk_window_get_size (canvas, &canvas_width, &canvas_height);
   while ((y - offset) < canvas_height)
   {
      line = &pg->line[ln];
      if ((line->month != cm && line->month >= 0) ||
	  (line->date != cd && line->date >= 0))
      {
	 cm = line->month;
	 cd = line->date;
	 y += 2 * log_line_sep;
      }
      if ((ln == ol && pg == op) || (ln == nl && pg == np))
      {
	 if (ln == nl && cursor_visible)
	    DrawLogCursor (y);
	 else
	    EraseLogCursor (y);

	 DrawLogLine (line, y);
      }
      y += log_line_sep;
      ln++;
      if (ln > pg->ll && pg->islastpage == TRUE)
	 break;
      if (ln >= LINES_P_PAGE)
      {
	 if (pg->islastpage == FALSE)
	    pg = pg->next;
	 else
	    break;
	 ln = 0;
      }
   }

}

/* ----------------------------------------------------------------------
   NAME:        DrawLogLine
   DESCRIPTION: Displays a single log line
   ---------------------------------------------------------------------- */

void
DrawLogLine (LogLine *line, int y)
{
  char tmp[1024];
  int num_chars, max_num_chars;
  int col_pos = 0;
  int char_width;
  
  char_width = gdk_char_width (cfg->fixed, 'A');
  
  /*gdk_gc_set_foreground (gc, &cfg->white); */
  gdk_gc_set_foreground (gc, &cfg->black);

  if (line->hour >= 0 && line->min >= 0 && line->sec >= 0) {
	  struct tm date = {0};
	  date.tm_mon = line->month;
	  date.tm_year = 70 /* bogus */;
	  date.tm_mday = line->date;
	  date.tm_hour = line->hour;
	  date.tm_min = line->min;
	  date.tm_sec = line->sec;
	  date.tm_isdst = 0;

	  /* Translators: should be only the time, date could be bogus */
	  if (strftime (tmp, sizeof (tmp), _("%X"), &date) <= 0) {
		  /* as a backup print in 24 hours style */
		  g_snprintf (tmp, sizeof (tmp), "%02d:%02d:%02d", line->hour, line->min, line->sec);
	  }

  } else {
	  strcpy (tmp, " ");
  }
  gdk_draw_string (canvas, cfg->fixedb, gc, LOG_COL1, y, tmp);

  /* Print four spaces */
  col_pos = LOG_COL1 + strlen(tmp)*char_width;
  strcpy (tmp, "    ");
  gdk_draw_string (canvas, cfg->fixedb, gc, col_pos, y, tmp);
  col_pos = col_pos + 4*char_width;

  strcpy (tmp, " ");
  if(user_prefs->hostname_column_width > 0)
  {
  	g_snprintf (tmp, sizeof (tmp), "%s", line->hostname);
  	if (strlen (tmp) > user_prefs->hostname_column_width)
    		tmp[user_prefs->hostname_column_width+1] = '\0';
  }
  gdk_draw_string (canvas, cfg->fixed, gc, col_pos, y, tmp);

  /* Print four spaces */
  col_pos = col_pos + user_prefs->hostname_column_width*char_width;
  strcpy (tmp, "    ");
  gdk_draw_string (canvas, cfg->fixedb, gc, col_pos, y, tmp);
  col_pos = col_pos + 4*char_width;

  strcpy (tmp, " ");
  if(user_prefs->process_column_width > 0)
  {
  	g_snprintf (tmp, sizeof (tmp), "%s", line->process);
  	if (strlen (tmp) > user_prefs->process_column_width)
    		tmp[user_prefs->process_column_width+1] = '\0';
  }
  gdk_draw_string (canvas, cfg->fixed, gc, col_pos, y, tmp);

  /* Print four spaces */
  col_pos = col_pos + user_prefs->process_column_width*char_width;
  strcpy (tmp, "    ");
  gdk_draw_string (canvas, cfg->fixedb, gc, col_pos, y, tmp);
  col_pos = col_pos + 4*char_width;

  /* For now max string length is ignored */
  num_chars = MAX (strlen (line->message), 1023);
  tmp[1023] = '\0';
  strncpy (tmp, line->message, 1023);
  max_num_chars = (canvas_width - 10 - col_pos) / gdk_char_width (cfg->fixed, 'A');
  if (max_num_chars < num_chars)
    max_num_chars = num_chars;
  tmp[max_num_chars] = '\0';
  gdk_draw_string (canvas, cfg->fixed, gc, col_pos, y, tmp);
}


/* ----------------------------------------------------------------------
   NAME:        DrawMonthHeader
   DESCRIPTION: Draw the header for the current month.
   ---------------------------------------------------------------------- */

void
DrawMonthHeader (LogLine * line, int y)
{
   char buf[100];
   int  h, centery, skip;
   GdkColor color[3];

   color[0] = cfg->blue1;
   color[1] = cfg->blue;
   color[2] = cfg->blue3;

   h = cfg->headingb->ascent - cfg->headingb->descent;
   skip = (log_line_sep - cfg->fixed->ascent + cfg->fixed->descent);
   centery = y + log_line_sep - ((2 * log_line_sep - skip - h) >> 1);
   Draw3DBox (canvas, gc, 5, y - log_line_sep + skip , canvas_width - 10, 2*log_line_sep-skip, color);

   gdk_gc_set_foreground (gc, &cfg->black);
   if (line->month >= 0 && line->month < 12) {
	   GDate *date = g_date_new_dmy (line->date, line->month+1, 2000 /* bogus */);
	   /* Translators: Make sure this is only Month and Day format, year
	    * will be bogus here */
	   if (g_date_strftime (buf, sizeof (buf), _("%B %e"), date) <= 0) {
		   /* If we fail just use the US format */
		   g_snprintf (buf, sizeof (buf), "%s %d", _(month[(int) line->month]), line->date);
	   }
	   g_date_free (date);
   } else {
	   g_snprintf (buf, sizeof (buf), "?%d? %d", (int) line->month, line->date);
   }
   gdk_draw_string (canvas, cfg->headingb, gc, LOG_COL1 - 1, centery + 1, buf);
   gdk_gc_set_foreground (gc, &cfg->white);
   gdk_draw_string (canvas, cfg->headingb, gc, LOG_COL1, centery, buf);

}

/* ----------------------------------------------------------------------
   NAME:        DrawLogCursor
   DESCRIPTION: Draw the cursor under the log line.
   ---------------------------------------------------------------------- */

void
DrawLogCursor (int y)
{
   GdkColor color[3];

   color[0] = cfg->gray25;
   color[1] = cfg->gray50;
   color[2] = cfg->gray75;

   Draw3DBox (canvas, gc, 5, y - log_line_sep + 3, canvas_width - 10, log_line_sep, color);
}

/* ----------------------------------------------------------------------
   NAME:        Draw3DBox
   DESCRIPTION: Draw a "3d" box using the three colors povided. 
   color[0]      bottom and right colors
   color[1]      background of box
   color[2]      top and left colors
   ---------------------------------------------------------------------- */

void
Draw3DBox (GdkDrawable *win, GdkGC *gc, int x, int y, int w, int h, GdkColor color[3])
{
   gdk_gc_set_foreground (gc, &color[1]);
   gdk_draw_rectangle (win, gc, TRUE, x, y, w, h);
   h--;
   w--;
   gdk_gc_set_foreground (gc, &color[2]);
   gdk_draw_line (win, gc, x, y + h, x, y);
   gdk_draw_line (win, gc, x, y, x + w, y);
   gdk_gc_set_foreground (gc, &color[0]);
   gdk_draw_line (win, gc, x, y + h, x + w, y + h);
   gdk_draw_line (win, gc, x + w, y + h, x + w, y);
}


/* ----------------------------------------------------------------------
   NAME:                                EraseLogCursor
   DESCRIPTION: Erase the cursor under the log line.
   ---------------------------------------------------------------------- */

void
EraseLogCursor (int y)
{
  gdk_window_clear_area (canvas, 5, y - log_line_sep + 3, canvas_width - 10, log_line_sep);
}


/* ----------------------------------------------------------------------
   NAME:        log_redrawdetail
   DESCRIPTION: Redraw area were the detailed information is drawn.
   ---------------------------------------------------------------------- */

void
log_redrawdetail ()
{
  if (zoom_visible)
    repaint_zoom (NULL, NULL);
}

/* ----------------------------------------------------------------------
   NAME:        InitPages
   DESCRIPTION: Returns -1 if there was a error otherwise TRUE;
   ---------------------------------------------------------------------- */

int
InitPages ()
{
   int i;

   /* Open default logs.            */
   numlogs = 0;
   for (i = 0; i < DEF_NUM_LOGS; i++)
     {
       curlog = OpenLogFile (deflognames[i]);
       if (curlog == NULL)
	 continue;
       loglist[numlogs] = curlog;
       numlogs++;
     }

   if (numlogs == 0)
      return -1;

   curlognum = numlogs - 1;
   curlog = loglist[curlognum];

   return TRUE;
}



/* ----------------------------------------------------------------------
   NAME:          GetLineAtCursor
   DESCRIPTION:   Given the y-coordinate of the pointer inside the frame
   calculate the line number of the log line under it.
   It returns -1 if the cursor is not over a log line.
   ---------------------------------------------------------------------- */

int
GetLineAtCursor (int y)
{
   Page *pg;
   LogLine *line;
   int sln, ln, cm, cd, i;
   int maxnumlines;

   sln = (y + 13 - LOG_TITLEY) / log_line_sep - 2;
   maxnumlines = canvas_height / log_line_sep;

   /* Check that we are at least bellow the header, */
   if (sln < 0)
      return -1;		/* guess not. */

   ln = curlog->firstline;
   pg = curlog->currentpg;
   line = &pg->line[ln];
   cm = line->month;
   cd = line->date;
   i = 0;
   /* while (i != sln && i < LINES_P_PAGE + SPARE_LINES) */
   while (i != sln && i < maxnumlines)
   {
      i++;
      ln++;
      if (ln > pg->ll && pg->islastpage == TRUE)
	 return -1;
      if (ln >= LINES_P_PAGE)
      {
	 if (pg->islastpage == FALSE)
	    pg = pg->next;
	 else
	    return -1;
	 ln = 0;
      }
      line = &pg->line[ln];
      if ((line->month != cm && line->month >= 0) ||
	  (line->date != cd && line->date >= 0))
      {
	 i += 2;
	 if (sln < i && sln >= i - 2)
	 {
	    return -1;
	 }
	 cm = line->month;
	 cd = line->date;
      }
   }

   return (ln);

}

/* ----------------------------------------------------------------------
   NAME:          GetPageAtCursor
   DESCRIPTION:   Given the y-coordinate of the pointer inside the frame
   calculate the page of the log line under it.
   ---------------------------------------------------------------------- */
Page *
GetPageAtCursor (int y)
{
   LogLine *line;
   Page *pg;
   int sln, ln, cm, cd, i;
   int maxnumlines;

   sln = (y + 13 - LOG_TITLEY) / log_line_sep - 2;
   maxnumlines = canvas_height / log_line_sep;

   ln = curlog->firstline;
   pg = curlog->currentpg;
   line = &pg->line[ln];
   cm = line->month;
   cd = line->date;
   i = 0;

   /* Check that we are at least bellow the header, */
   if (sln < 0)
      return pg;		/* guess not. */

   /* while (i != sln && i < LINES_P_PAGE + SPARE_LINES) */
   while (i != sln && i < maxnumlines)
   {
      i++;
      ln++;
      if (ln > pg->ll && pg->islastpage == TRUE)
	 return pg;
      if (ln >= LINES_P_PAGE)
      {
	 if (pg->islastpage == FALSE)
	    pg = pg->next;
	 else
	    return pg;
	 ln = 0;
      }
      line = &pg->line[ln];
      if ((line->month != cm && line->month >= 0) ||
	  (line->date != cd && line->date >= 0))
      {
	 i += 2;
	 if (sln < i && sln >= i - 2)
	    return pg;
	 cm = line->month;
	 cd = line->date;
      }
   }

   return (pg);

}

