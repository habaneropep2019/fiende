#ifndef GNOMELOCALEDIR
#define GNOMELOCALEDIR "/usr/share/locale"
#endif
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
#include <libgnomeui/gnome-window-icon.h>

/*
 *    -------------------
 *    Function Prototypes
 *    -------------------
 */

void repaint (GtkWidget * canvas, GdkRectangle * area);
void CreateMainWin (void);
gboolean log_repaint (GtkWidget * canvas, GdkRectangle * area);
gboolean PointerMoved (GtkWidget * canvas, GdkEventMotion * event);
gboolean HandleLogKeyboard (GtkWidget * win, GdkEventKey * event_key);
gboolean handle_log_mouse_button (GtkWidget * win, GdkEventButton *event);
void ExitProg (GtkWidget * widget, gpointer user_data);
void LoadLogMenu (GtkWidget * widget, gpointer user_data);
void CloseLogMenu (GtkWidget * widget, gpointer user_data);
void change_log_menu (GtkWidget * widget, gpointer user_data);
void CalendarMenu (GtkWidget * widget, gpointer user_data);
void MonitorMenu (GtkWidget* widget, gpointer user_data); 
void create_zoom_view (GtkWidget * widget, gpointer user_data);
void UserPrefsDialog(GtkWidget * widget, gpointer user_data);
void AboutShowWindow (GtkWidget* widget, gpointer user_data);
void CloseApp (void);
void CloseLog (Log *);
void FileSelectCancel (GtkWidget * w, GtkFileSelection * fs);
void FileSelectOk (GtkWidget * w, GtkFileSelection * fs);
void MainWinScrolled (GtkAdjustment *adjustment, GtkRange *);
void CanvasResized (GtkWidget *widget, GtkAllocation *allocation);
gboolean ScrollWin (GtkRange *range, gpointer event);
void LogInfo (GtkWidget * widget, gpointer user_data);
void UpdateStatusArea (void);
void set_scrollbar_size (int);
void change_log (int dir);
void open_databases (void);
void destroy (void);
void InitApp (void);
int InitPages (void);
int RepaintLogInfo (GtkWidget * widget, GdkEventExpose * event);
int read_regexp_db (char *filename, GList **db);
int read_actions_db (char *filename, GList **db);
void print_db (GList *gb);
Log *OpenLogFile (char *);
GtkWidget *new_pixmap_from_data (char  **, GdkWindow *, GdkColor *);
GtkWidget *create_menu (char *item[], int n);

/*
 *    ,-------.
 *    | Menus |
 *    `-------'
 */


GnomeUIInfo file_menu[] = {
        {GNOME_APP_UI_ITEM, N_("Open log..."), 
	 N_("Open log"), LoadLogMenu, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN, 0, 0, NULL},
        {GNOME_APP_UI_ITEM, N_("Export log..."), 
	 N_("Export log"), StubCall, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE_AS, 0, 0, NULL},
        {GNOME_APP_UI_ITEM, N_("Close log"), 
	 N_("Close log"), CloseLogMenu, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
        {GNOME_APP_UI_ITEM, N_("Switch log"), 
	 N_("Switch log"), change_log_menu, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
        {GNOME_APP_UI_ITEM, N_("Monitor..."), 
	 N_("Monitor log"), MonitorMenu, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
        {GNOME_APP_UI_ITEM, N_("Exit"), 
	 N_("Exit program"), ExitProg, NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'E', GDK_CONTROL_MASK, NULL},
        {GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};

GnomeUIInfo view_menu[] = {
        {GNOME_APP_UI_ITEM, N_("Calendar"), 
	 N_("Show calendar log"), CalendarMenu, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 'C', GDK_CONTROL_MASK, NULL},
        {GNOME_APP_UI_ITEM, N_("Log stats"), 
	 N_("Show log stats"), LogInfo, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 'I', GDK_CONTROL_MASK, NULL},
        {GNOME_APP_UI_ITEM, N_("Zoom"), 
	 N_("Show line info"), create_zoom_view, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 'Z', GDK_CONTROL_MASK, NULL},
        {GNOME_APP_UI_ITEM, N_("Preferences..."), 
	 N_("Show user preferences"), UserPrefsDialog, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 'P', GDK_CONTROL_MASK, NULL},
        {GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};

#if 0
GnomeUIInfo filter_menu[] = {
        {GNOME_APP_UI_ITEM, N_("Select...               "), 
	 N_("Select log events"), StubCall, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
        {GNOME_APP_UI_ITEM, N_("Filter..                "), 
	 N_("Filter log events"), StubCall, NULL, NULL,
         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL},
        {GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};
#endif

GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_HELP("logview"),
        {GNOME_APP_UI_ITEM, N_("About..."), 
	 N_("Info about logview"), AboutShowWindow,
         NULL, NULL,
         GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL},
        {GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};

GnomeUIInfo main_menu[] = {
	GNOMEUIINFO_MENU_FILE_TREE(file_menu),
	GNOMEUIINFO_MENU_VIEW_TREE(view_menu),
/*        {GNOME_APP_UI_SUBTREE, N_("F_ilter"), NULL,  filter_menu, NULL, NULL, */
/*         GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL}, */
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
        {GNOME_APP_UI_ENDOFINFO, NULL, NULL, NULL}
};
                 

/*
 *       ----------------
 *       Global variables
 *       ----------------
 */


GtkWidget *app = NULL;
GtkWidget *main_win_scrollbar = NULL;
GtkLabel *filename_label = NULL, *date_label = NULL;

GList *regexp_db = NULL, *descript_db = NULL, *actions_db = NULL;
UserPrefsStruct *user_prefs = NULL;
UserPrefsStruct user_prefs_struct = {0};
ConfigData *cfg = NULL;
GtkWidget *open_file_dialog = NULL;

extern GdkGC *gc;
extern Log *curlog, *loglist[];
extern int numlogs, curlognum;
extern int loginfovisible, calendarvisible;
extern int cursor_visible;

/* ----------------------------------------------------------------------
   NAME:          destroy
   DESCRIPTION:   Exit program.
   ---------------------------------------------------------------------- */

void
destroy (void)
{
   CloseApp ();
}

/* ----------------------------------------------------------------------
   NAME:          main
   DESCRIPTION:   Program entry point.
   ---------------------------------------------------------------------- */

int
main (int argc, char *argv[])
{
  bindtextdomain (PACKAGE, GNOMELOCALEDIR);
  textdomain (PACKAGE);

  QueueErrMessages (TRUE);

  /*  Initialize gnome & gtk */
  gnome_init ("logview", VERSION, argc, argv);

  gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-log.png");
  
  /*  Load graphics config */
  cfg = CreateConfig();
  
  /*  Show about window */
  /* AboutShowWindow (NULL, NULL); */

  InitApp ();

  QueueErrMessages (FALSE);
  ShowQueuedErrMessages ();
   
  /*  Loop application */
  gtk_main ();

  return 0;
}

/* ----------------------------------------------------------------------
   NAME:        InitApp
   DESCRIPTION: Main initialization routine.
   ---------------------------------------------------------------------- */

void
InitApp ()
{
  /*  Initialize variables */
  loginfovisible = FALSE;
  regexp_db = NULL;
  user_prefs = &user_prefs_struct;
  SetDefaultUserPrefs(user_prefs);

  /* Read databases */
  open_databases ();

  /*  Read files and init data. */
  if (InitPages () < 0)
    ShowErrMessage (_("No log files to open"));

  /*  Display main window */
  CreateMainWin ();
}

/* ----------------------------------------------------------------------
   NAME:        CreateMainWin
   DESCRIPTION: Creates the main window.
   ---------------------------------------------------------------------- */

void
CreateMainWin ()
{
   GtkWidget *canvas;
   GtkWidget *w, *box, *table, *hbox2;
   GtkWidget *padding;
   GtkLabel *label;
   GtkObject *adj;
   GtkAllocation req_size;

   /* Create App */

   app = gnome_app_new ("Logview", _("System Log Viewer"));

   gtk_container_set_border_width ( GTK_CONTAINER (app), 0);
   gtk_window_set_default_size ( GTK_WINDOW (app), LOG_CANVAS_W, LOG_CANVAS_H);
   req_size.x = req_size.y = 0;
   req_size.width = 400;
   req_size.height = 400;
   gtk_widget_size_allocate ( GTK_WIDGET (app), &req_size );
   gtk_signal_connect (GTK_OBJECT (app), "destroy",
		       GTK_SIGNAL_FUNC (destroy), NULL);
   gtk_signal_connect (GTK_OBJECT (app), "delete_event",
		       GTK_SIGNAL_FUNC (destroy), NULL);

   /* Create menus */
   gnome_app_create_menus (GNOME_APP (app), main_menu);

   box = gtk_vbox_new (FALSE, 0);
   gnome_app_set_contents (GNOME_APP (app), box);

   /* Deactivate unfinished items */
   gtk_widget_set_state (file_menu[1].widget, GTK_STATE_INSENSITIVE);
   if (numlogs < 2)
     gtk_widget_set_state (file_menu[3].widget, GTK_STATE_INSENSITIVE);
#if 0
   gtk_widget_set_state (filter_menu[0].widget, GTK_STATE_INSENSITIVE);
   gtk_widget_set_state (filter_menu[1].widget, GTK_STATE_INSENSITIVE);
#endif

   /* Create main canvas and scroll bars */
   table = gtk_table_new (2, 2, FALSE);
   gtk_widget_show (table);

   w = gtk_viewport_new (NULL, NULL);
   gtk_widget_set_usize (w, LOG_CANVAS_W, 0); 
   gtk_widget_show (w);
               
   canvas = gtk_drawing_area_new ();
   /*gtk_drawing_area_size (GTK_DRAWING_AREA (canvas), 2*LOG_CANVAS_W, LOG_CANVAS_H); */
   gtk_drawing_area_size (GTK_DRAWING_AREA (canvas), 2*LOG_CANVAS_W,
			  LOG_CANVAS_H); 
   /*gtk_widget_set_usize ( GTK_WIDGET (canvas), LOG_CANVAS_W, LOG_CANVAS_H);*/
   gtk_container_add (GTK_CONTAINER (w), canvas);
   gtk_table_attach (GTK_TABLE (table), w, 0, 1, 0, 1,
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

   adj = (GtkObject *)gtk_viewport_get_hadjustment (GTK_VIEWPORT (w));
   w = gtk_hscrollbar_new (GTK_ADJUSTMENT (adj));
   gtk_widget_show (w);
   gtk_table_attach (GTK_TABLE (table), w, 0, 1, 1, 2,
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		     (GtkAttachOptions) (0), 0, 0);

   if (curlog != NULL)
     adj = gtk_adjustment_new ( curlog->ln, 0.0,
				curlog->lstats.numlines,
				1.0, 10.0, (float) LINES_P_PAGE);
   else
     adj = gtk_adjustment_new (100.0, 0.0, 101.0, 1, 10, 101.0);

   main_win_scrollbar = (GtkWidget *)gtk_vscrollbar_new (GTK_ADJUSTMENT(adj));
   gtk_range_set_update_policy (GTK_RANGE (main_win_scrollbar), GTK_UPDATE_CONTINUOUS);

   gtk_table_attach (GTK_TABLE (table), main_win_scrollbar, 1, 2, 0, 1,
		     (GtkAttachOptions) (0),
		     (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
   gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		       (GtkSignalFunc) MainWinScrolled,
		       (gpointer) main_win_scrollbar);       
   gtk_signal_connect (GTK_OBJECT (main_win_scrollbar), "button_release_event", 
 		       (GtkSignalFunc) ScrollWin, 
 		       (gpointer) main_win_scrollbar);        
   gtk_widget_show (main_win_scrollbar);  


   gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 0);
   
   /*  Install event handlers */
   gtk_signal_connect (GTK_OBJECT (canvas), "expose_event",
		       GTK_SIGNAL_FUNC (log_repaint), NULL);
   gtk_signal_connect (GTK_OBJECT (canvas), "motion_notify_event",
		       GTK_SIGNAL_FUNC (PointerMoved), NULL);
   gtk_signal_connect (GTK_OBJECT (app), "key_press_event",
		       GTK_SIGNAL_FUNC (HandleLogKeyboard), NULL);
   gtk_signal_connect (GTK_OBJECT (app), "button_press_event",
		       GTK_SIGNAL_FUNC (handle_log_mouse_button), NULL);
   gtk_signal_connect (GTK_OBJECT (canvas), "size_allocate",
		       GTK_SIGNAL_FUNC (CanvasResized), NULL);
   gtk_widget_set_events (canvas, GDK_EXPOSURE_MASK |
			  GDK_BUTTON_PRESS_MASK |
			  GDK_POINTER_MOTION_MASK );

   gtk_widget_set_events (app, GDK_KEY_PRESS_MASK);


   /*gtk_widget_set_style (canvas, cfg->black_bg_style); */
   gtk_widget_set_style (canvas, cfg->white_bg_style);
   gtk_widget_show (canvas);


   /* Create status area at bottom */
   hbox2 = gtk_hbox_new (FALSE, 2);
   gtk_container_set_border_width ( GTK_CONTAINER (hbox2), 3);

   label = (GtkLabel *)gtk_label_new (_("Filename: "));
   gtk_label_set_justify (label, GTK_JUSTIFY_LEFT);
   gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (label), FALSE, FALSE, 0);
   gtk_widget_show (GTK_WIDGET (label));  

   filename_label = (GtkLabel *)gtk_label_new ("");
   gtk_widget_show ( GTK_WIDGET (filename_label));  
   gtk_label_set_justify (label, GTK_JUSTIFY_LEFT);
   gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (filename_label), 
		       FALSE, FALSE, 0);
   
   /* Add padding to right justify */
   padding = gtk_label_new (" ");
   gtk_widget_show (padding);
   gtk_box_pack_start (GTK_BOX (hbox2), padding, TRUE, TRUE, 0);

   label = (GtkLabel *)gtk_label_new (_("Date: "));
   gtk_label_set_justify (label, GTK_JUSTIFY_RIGHT);
   gtk_widget_set_usize (GTK_WIDGET(label), 40, -1);
   gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (label), FALSE, FALSE, 0);
   gtk_widget_show (GTK_WIDGET (label));  

   date_label = (GtkLabel *)gtk_label_new ("");
   gtk_widget_show (GTK_WIDGET (date_label));  
   gtk_widget_set_usize (GTK_WIDGET (label), 60, -1);
   gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (date_label), FALSE, FALSE, 0);

   gtk_widget_show (hbox2);

   gtk_box_pack_start (GTK_BOX (box), hbox2, FALSE, FALSE, 0);

   gtk_widget_show (box);
   gtk_widget_show (app);

}

/* ----------------------------------------------------------------------
   NAME:          MainScreenResized
   DESCRIPTION:   The main screen was resized.
   ---------------------------------------------------------------------- */

void
CanvasResized (GtkWidget *widget, GtkAllocation *allocation)
{
  DB (
      if (allocation)
      	printf ("Main screen resized!\n New size = (%d,%d)\n", 
		allocation->width, allocation->height);
      else
        printf ("Main screen resized!\nNo allocation :(\n");
	);
}

/* ----------------------------------------------------------------------
   NAME:          ScrollWin
   DESCRIPTION:   When the mouse button is released we scroll the window.
   ---------------------------------------------------------------------- */

gboolean
ScrollWin (GtkRange *range, gpointer event)
{
  int newln;
  
  newln = (int) range->adjustment->value;
  if (curlog == NULL ||
      newln >= curlog->lstats.numlines ||
      newln < 0)
    return FALSE;

  /* evil, yes */
  if (newln == 0)
	  newln = 1;
  
  /* Goto mark */
  MoveToMark (curlog);
  curlog->firstline = 0;
  
  ScrollDown (newln - curlog->curmark->ln);
  
  /* Repaint screen */
  log_repaint(NULL, NULL);

  return FALSE;
}

/* ----------------------------------------------------------------------
   NAME:          MainWinScrolled
   DESCRIPTION:   main window scrolled
   ---------------------------------------------------------------------- */

void
MainWinScrolled (GtkAdjustment *adjustment, GtkRange *range)
{
  int newln, howmuch;
  DateMark *mark;

  newln = (int) range->adjustment->value;

 if (newln < 0 ||
     curlog == NULL)
   return;

 /* evil, yes */
 if (newln == 0)
	 newln = 1;

 if (newln >= curlog->lstats.numlines)
   newln = curlog->lstats.numlines - 1;

  /* Find mark which has this line */
  mark = curlog->curmark;
  if (mark == NULL)
    return;

  if (newln >= mark->ln)
    {
      while( mark->next != NULL)
	{
	  if (newln <= mark->next->ln)
	    break;
	  mark = mark->next;
	}
    }
  else
    {
      while( mark->prev != NULL)
	{
	  if (newln >= mark->ln)
	    break;
	  mark = mark->prev;
	}
    }

  /* Now lets make it the current mark */
  cursor_visible = FALSE;
  howmuch = newln - curlog->ln;
  curlog->ln = newln;
  if (mark != curlog->curmark)
    {
      curlog->curmark = mark;
      MoveToMark (curlog);
      curlog->firstline = 0;
      howmuch = newln - mark->ln;
    }

  /* Update status area */
  UpdateStatusArea ();

  /* Only scroll when the scrollbar is released */
  if (howmuch > 0)
      ScrollDown (howmuch);
  else
      ScrollUp (-1*howmuch);

  if (howmuch != 0)
    log_repaint(NULL, NULL);
}


/* ----------------------------------------------------------------------
   NAME:          set_scrollbar_size
   DESCRIPTION:   Set size of scrollbar acording to file.
   ---------------------------------------------------------------------- */

void set_scrollbar_size (int num_lines)
{
  GtkObject *adj;

  /*adj = gtk_adjustment_new ( curlog->ln, 0.0, num_lines+LINES_P_PAGE, 1.0, 10.0, (float) LINES_P_PAGE);*/
  adj = gtk_adjustment_new (-1, 0.0, num_lines,
			    1.0, 10.0, (float) LINES_P_PAGE);
  gtk_range_set_adjustment (GTK_RANGE (main_win_scrollbar),
			    GTK_ADJUSTMENT (adj));
  gtk_signal_connect (GTK_OBJECT (adj), "value_changed",
		      (GtkSignalFunc) MainWinScrolled,
		      (gpointer) main_win_scrollbar);       
  gtk_adjustment_set_value (GTK_ADJUSTMENT(adj), curlog->ln);
  gtk_widget_realize (main_win_scrollbar);

  gtk_widget_queue_resize (main_win_scrollbar);
}

/* ----------------------------------------------------------------------
   NAME:          CloseLogMenu
   DESCRIPTION:   Close the current log.
   ---------------------------------------------------------------------- */

void
CloseLogMenu (GtkWidget * widget, gpointer user_data)
{
   int i;

   if (numlogs == 0)
      return;

   CloseLog (curlog);
   numlogs--;
   if (numlogs == 0)
   {
      curlog = NULL;
      loglist[0] = NULL;
      curlognum = 0;
      log_repaint (NULL, NULL);
      if (loginfovisible)
	 RepaintLogInfo (NULL, NULL);
      return;
   }
   for (i = curlognum; i < numlogs; i++)
      loglist[i] = loglist[i + 1];
   loglist[i] = NULL;

   if (curlognum > 0)
      curlognum--;
   curlog = loglist[curlognum];
   log_repaint (NULL, NULL);

   if (loginfovisible)
      RepaintLogInfo (NULL, NULL);

   /* Change menu entry if there is only one log */
   if (numlogs < 2)
     gtk_widget_set_state (file_menu[3].widget, GTK_STATE_INSENSITIVE);

   set_scrollbar_size (curlog->lstats.numlines);
}

/* ----------------------------------------------------------------------
   NAME:          change_log_menu
   DESCRIPTION:   Switch log
   ---------------------------------------------------------------------- */

void
change_log_menu (GtkWidget * widget, gpointer user_data)
{
  change_log (1);
}

/* ----------------------------------------------------------------------
   NAME:          FileSelectOk
   DESCRIPTION:   User selected a file.
   ---------------------------------------------------------------------- */

void
FileSelectOk (GtkWidget * w, GtkFileSelection * fs)
{
   char *f;
   Log *tl;

   /* Check that we haven't opened all logfiles allowed    */
   if (numlogs >= MAX_NUM_LOGS)
     {
       ShowErrMessage (_("Too many open logs. Close one and try again"));
       return;
     }

   f = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)));
   gtk_widget_destroy (GTK_WIDGET (fs));

   if (f != NULL) {
      if ((tl = OpenLogFile (f)) != NULL)
      {
	 curlog = tl;
	 loglist[numlogs] = tl;
	 numlogs++;
	 curlognum = numlogs - 1;

	 /* Clear window */
	 log_repaint (NULL, NULL);
	 if (loginfovisible)
	   RepaintLogInfo (NULL, NULL);
	 if (calendarvisible)
	   init_calendar_data();
	 UpdateStatusArea();

	 /* Set main scrollbar */
	 set_scrollbar_size (curlog->lstats.numlines);

	 if (numlogs >= 2)
	   gtk_widget_set_sensitive (file_menu[3].widget, TRUE);
      }
   }
   g_free (f);

}

/* ----------------------------------------------------------------------
   NAME:          LoadLogMenu
   DESCRIPTION:   Open a new log defined by the user.
   ---------------------------------------------------------------------- */

void
LoadLogMenu (GtkWidget * widget, gpointer user_data)
{
   GtkWidget *filesel = NULL;

   /*  Cannot open more than MAX_NUM_LOGS */
   if (numlogs == MAX_NUM_LOGS)
      return;
   
   /*  Cannot have more than one fileselect window */
   /*  at one time. */
   if (open_file_dialog != NULL) {
	   gtk_widget_show_now (open_file_dialog);
	   gdk_window_raise (open_file_dialog->window);
	   return;
   }


   filesel = gtk_file_selection_new (_("Open new logfile"));
   gtk_window_set_transient_for (GTK_WINDOW (filesel), GTK_WINDOW (app));
   gnome_window_icon_set_from_default (GTK_WINDOW (filesel));

   /* Make window modal */
   gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);

   gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), 
   				    PATH_MESSAGES);
   gtk_window_set_position (GTK_WINDOW (filesel), GTK_WIN_POS_MOUSE);
   gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
		       "clicked", (GtkSignalFunc) FileSelectOk,
		       filesel);
   gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->cancel_button),
			      "clicked", (GtkSignalFunc) gtk_widget_destroy,
			      GTK_OBJECT (filesel));

   gtk_signal_connect (GTK_OBJECT (filesel),
		       "destroy", (GtkSignalFunc) gtk_widget_destroyed,
		       &open_file_dialog);

   gtk_widget_show (filesel);

   open_file_dialog = filesel;
}





/* ----------------------------------------------------------------------
   NAME:          ExitProg
   DESCRIPTION:   Callback to call when program exits.
   ---------------------------------------------------------------------- */

void 
ExitProg (GtkWidget * widget, gpointer user_data)
{
   CloseApp ();
}

/* ----------------------------------------------------------------------
   NAME:          CloseApp
   DESCRIPTION:   Close everything and exit.
   ---------------------------------------------------------------------- */

void 
CloseApp (void)
{
   int i;

   for (i = 0; i < numlogs; i++)
      CloseLog (loglist[i]);

   gtk_exit (0);
}

/* ----------------------------------------------------------------------
   NAME:          open_databases
   DESCRIPTION:   Try to locate regexp and descript databases and load
   	          them.
   ---------------------------------------------------------------------- */

void
open_databases (void)
{
	char full_name[1024];
	gboolean found;

	/* Find regexp DB -----------------------------------------------------  */
	found = FALSE;
	if (cfg->regexp_db_path != NULL) {
		g_snprintf (full_name, sizeof (full_name),
			    "%s/logview-regexp.db", cfg->regexp_db_path);
		DB (fprintf (stderr, "Looking for database in [%s]\n", cfg->regexp_db_path));
		if (access (full_name, R_OK) == 0)  {
			found = TRUE;
			read_regexp_db (full_name, &regexp_db);
		}
	}

	if ( ! found) {
		g_snprintf (full_name, sizeof (full_name),
			    "%s/share/logview/logview-regexp.db", LOGVIEWINSTALLPREFIX);
		if (access (full_name, R_OK) == 0) {
			found = TRUE;
			g_free (cfg->regexp_db_path);
			cfg->regexp_db_path = g_strdup (full_name);
			read_regexp_db (full_name, &regexp_db);
		}
	}

	/* Find description DB ------------------------------------------------  */
	found = FALSE;
	if (cfg->descript_db_path != NULL) {
		g_snprintf (full_name, sizeof (full_name),
			    "%s/logview-descript.db", cfg->descript_db_path);
		DB (fprintf (stderr, "Looking for database in [%s]\n", cfg->descript_db_path));
		if (access (full_name, R_OK) == 0) {
			read_descript_db (full_name, &descript_db);
			found = TRUE;
		}
	}

	if ( ! found) {
		g_snprintf (full_name, sizeof (full_name),
			    "%s/share/logview/logview-descript.db", LOGVIEWINSTALLPREFIX);
		if (access (full_name, R_OK) == 0) {
			found = TRUE;
			g_free (cfg->descript_db_path);
			cfg->descript_db_path = g_strdup (full_name);
			read_descript_db (full_name, &descript_db);
		}
	}


	/* Find action DB ------------------------------------------------  */
	found = FALSE;
	g_snprintf (full_name, sizeof (full_name),
		    "%s/.gnome/logview-actions.db", g_get_home_dir ());
	DB (fprintf (stderr, "Looking for database in [%s/.gnome]\n",
		     g_get_home_dir ()));
	if (access (full_name, R_OK) == 0) {
		found = TRUE;
		read_actions_db (full_name, &actions_db);
	}

	if ( ! found && cfg->action_db_path != NULL) {
		g_snprintf (full_name, sizeof (full_name),
			    "%s/logview-actions.db", cfg->action_db_path);
		DB (fprintf (stderr, "Looking for database in [%s]\n", cfg->action_db_path));
		if (access (full_name, R_OK) == 0) {
			found = TRUE;
			read_actions_db (full_name, &actions_db);
		}
	}


	if ( ! found) {
		g_snprintf (full_name, sizeof (full_name),
			    "%s/share/logview/logview-actions.db", LOGVIEWINSTALLPREFIX);
		if (access (full_name, R_OK) == 0) {
			found = TRUE;
			g_free (cfg->action_db_path);
			cfg->action_db_path = g_strdup (full_name);
			read_actions_db (full_name, &actions_db);
		}
	}

	/* If debugging then print DB */
	DB (print_db (regexp_db));
}

/* ----------------------------------------------------------------------
   NAME:          IsLeapYear
   DESCRIPTION:   Return TRUE if year is a leap year.
   ---------------------------------------------------------------------- */
int
IsLeapYear (int year)
{
   if ((1900 + year) % 4 == 0)
      return TRUE;
   else
      return FALSE;
}

void SetDefaultUserPrefs(UserPrefsStruct *prefs)
{
	/* Make defaults configurable later */
	/* Will have to save prefs. eventually too*/

	prefs->process_column_width = 12;
	prefs->hostname_column_width = 15;
	prefs->message_column_width = 24;
}
