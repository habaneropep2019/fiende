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
#include <time.h>
#include <gnome.h>
#include "gtk/gtk.h"
#include "logview.h"

#define CALENDAR_WIDTH           180
#define CALENDAR_HEIGHT          150

#define SUNDAY                   0
#define MONDAY                   1
#define FRIDAY                   5
#define SATURDAY                 6
#define FEBRUARY                 1
#define XSEP                     3
#define CALLEFTMARGIN            5
#define BASELINESKIP             4

#define THISMONTH                1
#define OTHERMONTH               2
#define MARKEDDATE               3

/*
 *       --------
 *       Typedefs
 *       --------
 */


/*
 *    -------------------
 *    Function Prototypes
 *    -------------------
 */

void CalendarMenu (GtkWidget * widget, gpointer user_data);
void close_calendar (GtkWidget * widget, gpointer client_data);
void set_scrollbar_size (int num_lines);
void calendar_month_changed (GtkWidget *widget, gpointer data);
void read_marked_dates (CalendarData *data);
void calendar_month_changed (GtkWidget *widget, gpointer data);
void calendar_day_selected (GtkWidget *widget, gpointer unused_data);
void calendar_day_selected_double_click (GtkWidget *widget, gpointer data);
CalendarData* init_calendar_data ();
DateMark* find_prev_mark (CalendarData*);
DateMark* find_next_mark (CalendarData*);
DateMark* get_mark_from_month (CalendarData *data, gint month, gint year);
DateMark *get_mark_from_date (CalendarData *, gint, gint, gint);
GtkWidget *new_pixmap_from_data(char **xpm_data, GdkWindow *w, GdkColor *b);
void log_repaint (GtkWidget * canvas, GdkRectangle * area);



/*
 *       ----------------
 *       Global variables
 *       ----------------
 */

extern ConfigData *cfg;
extern GdkGC *gc;
extern Log *curlog, *loglist[];
extern int numlogs, curlognum;
extern char *month[12];
extern GtkWidget *main_win_scrollbar;

GtkWidget *CalendarDialog = NULL;
GtkWidget *CalendarWidget;
int calendarvisible;


/* ----------------------------------------------------------------------
   NAME:          CalendarMenu
   DESCRIPTION:   Display the calendar.
   ---------------------------------------------------------------------- */

void
CalendarMenu (GtkWidget * widget, gpointer user_data)
{
   GtkCalendar *calendar;
   GtkWidget *frame;
   GtkWidget *vbox;

   if (curlog == NULL || calendarvisible)
      return;

   if (CalendarDialog == NULL)
   {
      CalendarDialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      gtk_container_set_border_width (GTK_CONTAINER (CalendarDialog), 1);
      gtk_window_set_title (GTK_WINDOW (CalendarDialog), _("Calendar"));
      gtk_signal_connect (GTK_OBJECT (CalendarDialog), "destroy",
			  GTK_SIGNAL_FUNC (close_calendar),
			  &CalendarDialog);
      gtk_signal_connect (GTK_OBJECT (CalendarDialog), "delete_event",
			  GTK_SIGNAL_FUNC (close_calendar),
			  NULL);

      gtk_widget_set_style (CalendarDialog, cfg->main_style);
      vbox = gtk_vbox_new (FALSE, 2);
      gtk_container_set_border_width (GTK_CONTAINER (vbox), 4);
      gtk_container_add (GTK_CONTAINER (CalendarDialog), vbox);
      gtk_widget_show (vbox);

      frame = gtk_frame_new (NULL);
      gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
      gtk_container_set_border_width (GTK_CONTAINER (frame), 3);
      gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
      gtk_widget_set_style (frame, cfg->main_style);
      gtk_widget_show (frame);

      calendar = (GtkCalendar *)gtk_calendar_new();
      gtk_signal_connect (GTK_OBJECT (calendar), "month_changed",
			  GTK_SIGNAL_FUNC (calendar_month_changed),
			  NULL);
      gtk_signal_connect (GTK_OBJECT (calendar), "day_selected",
			  GTK_SIGNAL_FUNC (calendar_day_selected),
			  NULL);
      gtk_signal_connect (GTK_OBJECT (calendar), "day_selected_double_click",
			  GTK_SIGNAL_FUNC (calendar_day_selected_double_click),
			  NULL);

      gtk_container_add (GTK_CONTAINER (frame), GTK_WIDGET (calendar));
      gtk_widget_show (GTK_WIDGET (calendar));
      CalendarWidget = GTK_WIDGET (calendar);
   }
   calendarvisible = TRUE;
   init_calendar_data ();

   gtk_widget_show (CalendarDialog);

}

/* ----------------------------------------------------------------------
   NAME:	new_pixmap_from_data
   DESCRIPTION:	
   ---------------------------------------------------------------------- */

GtkWidget *
new_pixmap_from_data (char      **xpm_data,
		      GdkWindow *window,
		      GdkColor  *background)
{
  GtkWidget *wpixmap;
  GdkPixmap *pixmap;
  GdkBitmap *mask;

  pixmap = gdk_pixmap_create_from_xpm_d (window, &mask,
					 background,
					 xpm_data);
  wpixmap = gtk_pixmap_new (pixmap, mask);

  return wpixmap;
}

/* ----------------------------------------------------------------------
   NAME:          read_marked_dates
   DESCRIPTION:   Reads all the marked dates for this month and marks 
   		  them in the calendar widget.
   ---------------------------------------------------------------------- */

void
read_marked_dates (CalendarData *data)
{
  GtkCalendar *calendar;
  DateMark *mark;
  gint day, month, year;

  g_return_if_fail (data);
  calendar = GTK_CALENDAR (CalendarWidget);
  g_return_if_fail (calendar);

  gtk_calendar_clear_marks (calendar);
  gtk_calendar_get_date (calendar, &year, &month, &day);
  year -= 1900;

  /* find the current month and year (if there is one)  */
  mark = data->curmonthmark;

  while (mark)
    {
      if (mark->fulldate.tm_mon != month || 
	  mark->fulldate.tm_year != year)
	break;
      gtk_calendar_mark_day (calendar, mark->fulldate.tm_mday);
      mark = mark->next;
    }

  return;
}

        
/* ----------------------------------------------------------------------
   NAME:          init_calendar_data
   DESCRIPTION:   Sets up calendar data asociated with this log.
   ---------------------------------------------------------------------- */

CalendarData*
init_calendar_data ()
{
   CalendarData *data;

   data = curlog->caldata;
   if (data == NULL)
     data = (CalendarData*) malloc (sizeof (CalendarData));

   if (data)
     {
       data->curmonthmark = curlog->lstats.firstmark;
       curlog->caldata = data;

#if 0
       /* Move mark to first marked date in this month */
       data->curmonthmark = 
	 get_mark_from_month (data, curlog->curmark->fulldate.tm_mon,
			      curlog->curmark->fulldate.tm_year);
#endif
       
       /* signal a redraw event */
       gtk_signal_emit_by_name (GTK_OBJECT (CalendarWidget), "month_changed");
       
     }

   return data;
}

/* ----------------------------------------------------------------------
   NAME:          calendar_month_changed
   DESCRIPTION:   User changed the month in the calendar.
   ---------------------------------------------------------------------- */

void
calendar_month_changed (GtkWidget *widget, gpointer unused_data)
{
  GtkCalendar *calendar;
  CalendarData *data;
  DateMark *mark;
  gint day, month, year;

  calendar = GTK_CALENDAR (CalendarWidget);
  g_return_if_fail (calendar);

  data = curlog->caldata;
  g_return_if_fail (data);

  /* Get current date */
  gtk_calendar_get_date (calendar, &year, &month, &day);
  /* This is Y2K compatible but internally we keep track of years
     starting from 1900. This is because of Unix and not because 
     I like it!! */
  year -= 1900;
  mark = get_mark_from_month (data, month, year);
  if (mark)
    data->curmonthmark = mark;
  read_marked_dates (data);
}


/* ----------------------------------------------------------------------
   NAME:          calendar_day_selected
   DESCRIPTION:   User clicked on a calendar entry
   ---------------------------------------------------------------------- */

void
calendar_day_selected (GtkWidget *widget, gpointer unused_data)
{
  GtkCalendar *calendar;
  CalendarData *data;
  gint day, month, year;

  calendar = GTK_CALENDAR (CalendarWidget);
  gtk_calendar_get_date (calendar, &year, &month, &day);
  /* This is Y2K compatible but internally we keep track of years
     starting from 1900. This is because of Unix and not because 
     I like it!! */
  year -= 1900;
  g_return_if_fail (calendar);

  data = curlog->caldata;
  g_return_if_fail (data);
  
  /* TODO This is an ugly HACK until I add this function into
     the widget!!!!!!
  */
  /* Dates go from 0-30 in this array!!! I really need to
     add the function gtk_calendar_day_marked (day) */
  if (calendar->marked_date[day-1])
    {
      curlog->curmark = get_mark_from_date (data, day, month, year);
      MoveToMark (curlog);
      curlog->firstline = 0;
      curlog->ln = curlog->curmark->ln;
      gtk_adjustment_set_value ( GTK_RANGE(main_win_scrollbar)->adjustment,
				 curlog->ln);
      /* set_scrollbar_size (curlog->lstats.numlines); */
      log_repaint(NULL, NULL);
    }
  
}

/* ----------------------------------------------------------------------
   NAME:          calendar_day_selected
   DESCRIPTION:   User clicked on a calendar entry
   ---------------------------------------------------------------------- */

void
calendar_day_selected_double_click (GtkWidget *widget, gpointer data)
{
  calendar_day_selected(widget, data);
}

/* ----------------------------------------------------------------------
   NAME:          close_calendar
   DESCRIPTION:   Callback called when the log info window is closed.
   ---------------------------------------------------------------------- */

void
close_calendar (GtkWidget * widget, gpointer client_data)
{
   if (calendarvisible)
      gtk_widget_hide (CalendarDialog);
   CalendarDialog = NULL;
   calendarvisible = FALSE;
}


/* ----------------------------------------------------------------------
   NAME:        get_mark_from_month
   DESCRIPTION: Move curmonthmark to current month if there is a log 
                entry that month.
   ---------------------------------------------------------------------- */

DateMark *
get_mark_from_date (CalendarData *data, gint day, gint month, gint year)
{
  DateMark *mark;

  g_return_val_if_fail (data, NULL);
  mark = get_mark_from_month (data, month, year);
  
  while (mark)
    {
      if (mark->fulldate.tm_mday == day)
	return mark;
      mark = mark->next;
    }

  return mark;
}

/* ----------------------------------------------------------------------
   NAME:        get_mark_from_month
   DESCRIPTION: Move curmonthmark to current month if there is a log 
                entry that month.
   ---------------------------------------------------------------------- */

DateMark *
get_mark_from_month (CalendarData *data, gint month, gint year)
{
   DateMark *mark = data->curmonthmark;

   if (mark == NULL)
      return mark;

   /* Are we on top of the mark: return if so      */
   if (month == mark->fulldate.tm_mon &&
       year == mark->fulldate.tm_year)
      return mark;

   /* Check if we are on the next or previous mark */
   if (year > mark->fulldate.tm_year)
      mark = find_next_mark (data);
   else if (year < mark->fulldate.tm_year)
      mark = find_prev_mark (data);
   else if (year == mark->fulldate.tm_year) {
      if (month > mark->fulldate.tm_mon)
	 mark = find_next_mark (data);
      else
	 mark = find_prev_mark (data);
   }

   if (mark == NULL)
      return NULL;

   if (month == mark->fulldate.tm_mon && 
       year == mark->fulldate.tm_year)
     return mark;

   return NULL;
}

/* ----------------------------------------------------------------------
   NAME:          find_prev_mark
   DESCRIPTION:   Returns the previous mark with a different month.
   ---------------------------------------------------------------------- */

DateMark *
find_prev_mark (CalendarData *data)
{
   DateMark *mark = data->curmonthmark;

   if (mark->prev == NULL)
      return NULL;
   mark = mark->prev;
   while (TRUE)
   {
      if (mark->prev == NULL)
	 break;
      mark = mark->prev;
      if (mark->fulldate.tm_mon != mark->next->fulldate.tm_mon ||
	  mark->fulldate.tm_year != mark->next->fulldate.tm_year)
      {
	 mark = mark->next;
	 break;
      }
   }

   return mark;
}

/* ----------------------------------------------------------------------
   NAME:          find_next_mark
   DESCRIPTION:   Returns the next mark with a different month.
   ---------------------------------------------------------------------- */

DateMark *
find_next_mark (CalendarData *data)
{
   DateMark *mark = data->curmonthmark;

   while (TRUE)
   {
      if (mark->next == NULL)
	 return NULL;
      mark = mark->next;
      if (mark->fulldate.tm_mon != mark->prev->fulldate.tm_mon ||
	  mark->fulldate.tm_year != mark->prev->fulldate.tm_year)
	 break;
   }

   return mark;
}

