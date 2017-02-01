#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"


/* Complicated dialog boxes can be slow to create, so we create them once and
   keep pointers to them in static variables so we can reuse them. */
static GtkWidget *open_filesel = NULL;
static GtkWidget *save_filesel = NULL;
static GtkWidget *fontsel = NULL;

/* This is the filename of the file currently being edited. */
static gchar *current_filename = NULL;

/* This flag is set to TRUE if the file has been changed. We don't actually
   use it here, but the program could be extended to prompt to save changes. */
static gboolean file_changed = FALSE;

/* A key used to store pointers to the main window. */
static const gchar *MainWindowKey = "MainWindowKey";

/* The size of chunks to read in when opening files. */
#define BUFFER_SIZE 8192


/* Static functions. */
static void new_file (GtkWidget *main_window);
static void open_file (GtkWidget *main_window);
static void save_as (GtkWidget *main_window);

static void real_open_file (GtkWidget *main_window,
			    gchar *filename);
static void real_save_file (GtkWidget *main_window,
			    gchar *filename);


/***************************************************************************
 * File Menu Commands.
 ***************************************************************************/
void
on_New_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  /* We use the Glade utility function lookup_widget() to get a pointer to the
     main window. The first argument is any widget in the window/dialog and
     the second argument is the name of the widget you want to get. */
  main_window = lookup_widget (GTK_WIDGET (menuitem), "main_window");
  new_file (main_window);
}


void
on_Open_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  main_window = lookup_widget (GTK_WIDGET (menuitem), "main_window");
  open_file (main_window);
}


void
on_Save_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  main_window = lookup_widget (GTK_WIDGET (menuitem), "main_window");

  /* If the current document doesn't have a filename yet, we call save_as
     to show the file selection dialog. */
  if (current_filename == NULL)
    save_as (main_window);
  else
    real_save_file (main_window, current_filename);
}


void
on_Save_As_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  main_window = lookup_widget (GTK_WIDGET (menuitem), "main_window");
  save_as (main_window);
}


void
on_Exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_exit (0);
}


/***************************************************************************
 * Edit Menu Commands.
 * The GtkText superclass GtkEditable makes this very easy for us by
 * providing clipboard functions.
 ***************************************************************************/
void
on_Cut_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *text, *statusbar;

  text = lookup_widget (GTK_WIDGET (menuitem), "text1");
  gtk_editable_cut_clipboard (GTK_EDITABLE (text));

  statusbar = lookup_widget (GTK_WIDGET (menuitem), "statusbar1");
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Text cut to clipboard.");
}


void
on_Copy_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *text, *statusbar;

  text = lookup_widget (GTK_WIDGET (menuitem), "text1");
  gtk_editable_copy_clipboard (GTK_EDITABLE (text));

  statusbar = lookup_widget (GTK_WIDGET (menuitem), "statusbar1");
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Text copied.");
}


void
on_Paste_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *text, *statusbar;

  text = lookup_widget (GTK_WIDGET (menuitem), "text1");
  gtk_editable_paste_clipboard (GTK_EDITABLE (text));

  statusbar = lookup_widget (GTK_WIDGET (menuitem), "statusbar1");
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Text pasted.");
}


void
on_Clear_activate                     (GtkMenuItem     *menuitem,
				       gpointer         user_data)
{
  GtkWidget *text, *statusbar;

  text = lookup_widget (GTK_WIDGET (menuitem), "text1");
  gtk_editable_delete_selection (GTK_EDITABLE (text));

  statusbar = lookup_widget (GTK_WIDGET (menuitem), "statusbar1");
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Text deleted.");
}


/***************************************************************************
 * Toolbar Buttons.
 ***************************************************************************/
void
on_new_button_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  main_window = lookup_widget (GTK_WIDGET (button), "main_window");
  new_file (main_window);
}


void
on_open_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  main_window = lookup_widget (GTK_WIDGET (button), "main_window");
  open_file (main_window);
}


void
on_save_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  main_window = lookup_widget (GTK_WIDGET (button), "main_window");

  if (current_filename == NULL)
    save_as (main_window);
  else
    real_save_file (main_window, current_filename);
}


/***************************************************************************
 * Main Window Signals.
 ***************************************************************************/

/* This is called when the main window gets a "delete_event" signal, which
   typically happens when the user clicks on the close icon on the window's
   title bar. If we didn't handle this, the window would be destroyed but
   our application would not exit. */
gboolean
on_main_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_exit (0);

  /* Shouldn't reach here, but we add it to stop compiler warnings. */
  return FALSE;
}


/***************************************************************************
 * Font Selection Dialog.
 ***************************************************************************/

/* This is called when the 'Change Font' command is used. We just need to
   show the font selection dialog. */
void
on_Font_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *main_window;

  main_window = lookup_widget (GTK_WIDGET (menuitem), "main_window");
  if (!fontsel)
    {
      fontsel = create_font_selection ();
      gtk_object_set_data (GTK_OBJECT (fontsel), MainWindowKey, main_window);
    }
  gtk_widget_show (fontsel);

  /* We raise the window in case it is already created and shown, but is
     hidden behind another window. */
  gdk_window_raise (fontsel->window);
}


void
on_fontsel_apply_button_clicked        (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *fontsel, *main_window, *text, *statusbar;
  GtkStyle *style;
  GdkFont *font;

  fontsel = gtk_widget_get_toplevel (GTK_WIDGET (button));
  main_window = gtk_object_get_data (GTK_OBJECT (fontsel), MainWindowKey);
  text = lookup_widget (main_window, "text1");
  statusbar = lookup_widget (main_window, "statusbar1");
  font = gtk_font_selection_dialog_get_font (GTK_FONT_SELECTION_DIALOG (fontsel));

  /* If no font is selected, or the font couldn't be loaded, just return. */
  if (!font)
    return;

  /* We copy the current style, and update the font. */
  style = gtk_style_copy (text->style);
  gdk_font_unref (style->font);
  style->font = font;
  gdk_font_ref (font);
  gtk_widget_set_style (text, style);

  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Font updated.");
}


void
on_fontsel_ok_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
  GtkWidget *fontsel;

  /* We do the same thing as apply, but we close the dialog after. */
  on_fontsel_apply_button_clicked (button, NULL);
  fontsel = gtk_widget_get_toplevel (GTK_WIDGET (button));
  gtk_widget_hide (fontsel);
}


void
on_fontsel_cancel_button_clicked       (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_hide (gtk_widget_get_toplevel (GTK_WIDGET (button)));
}


gboolean
on_fontsel_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_hide (gtk_widget_get_toplevel (widget));
  return TRUE;
}


/***************************************************************************
 * Text Widget signals.
 ***************************************************************************/

/*
 * Text changed callback. This signal is emitted whenever the text in the
 * GtkText changes. We don't use this at present, but it could be used to
 * prompt to save changes before opening new files or quitting.
 */
void
on_text_changed                        (GtkEditable     *editable,
                                        gpointer         user_data)
{
  file_changed = TRUE;
}


/***************************************************************************
 * Open File Selection Dialog.
 ***************************************************************************/
void
on_open_filesel_ok_button_clicked           (GtkButton       *button,
					     gpointer         user_data)
{
  GtkWidget *filesel, *main_window;
  gchar *filename;

  filesel = gtk_widget_get_toplevel (GTK_WIDGET (button));
  main_window = gtk_object_get_data (GTK_OBJECT (filesel), MainWindowKey);
  gtk_widget_hide (filesel);
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));
  real_open_file (main_window, filename);
}


void
on_open_filesel_cancel_button_clicked       (GtkButton       *button,
					     gpointer         user_data)
{
  gtk_widget_hide (gtk_widget_get_toplevel (GTK_WIDGET (button)));
}


gboolean
on_open_filesel_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_hide (gtk_widget_get_toplevel (widget));
  return TRUE;
}


/***************************************************************************
 * Save File Selection Dialog.
 ***************************************************************************/
void
on_save_filesel_ok_button_clicked           (GtkButton       *button,
					     gpointer         user_data)
{
  GtkWidget *filesel, *main_window;
  gchar *filename;

  filesel = gtk_widget_get_toplevel (GTK_WIDGET (button));
  main_window = gtk_object_get_data (GTK_OBJECT (filesel), MainWindowKey);
  gtk_widget_hide (filesel);
  filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (filesel));
  real_save_file (main_window, filename);
}


void
on_save_filesel_cancel_button_clicked       (GtkButton       *button,
					     gpointer         user_data)
{
  gtk_widget_hide (gtk_widget_get_toplevel (GTK_WIDGET (button)));
}


gboolean
on_save_filesel_delete_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
  gtk_widget_hide (gtk_widget_get_toplevel (widget));
  return TRUE;
}


/***************************************************************************
 * About Dialog.
 ***************************************************************************/
void
on_About_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  GtkWidget *about;

  about = create_about_dialog ();
  gtk_widget_show (about);
}


void
on_about_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
  gtk_widget_destroy (gtk_widget_get_toplevel (GTK_WIDGET (button)));
}



/***************************************************************************
 * Private functions.
 ***************************************************************************/

/* This sets the window title according to the current filename. */
void
set_window_title (GtkWidget *main_window)
{
  gchar *title;

  title = g_strdup_printf ("Editor: %s\n",
			   current_filename ? g_basename (current_filename)
			                    : "untitled");

  gtk_window_set_title (GTK_WINDOW (main_window), title);
  g_free (title);
}


/* This creates a new document, by clearing the text widget and setting the
   current filename to NULL. */
static void
new_file (GtkWidget *main_window)
{
  GtkWidget *text, *statusbar;

  text = lookup_widget (GTK_WIDGET (main_window), "text1");
  gtk_editable_delete_text (GTK_EDITABLE (text), 0, -1);
  g_free (current_filename);
  current_filename = NULL;
  file_changed = FALSE;
  set_window_title (main_window);

  statusbar = lookup_widget (main_window, "statusbar1");
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "New file.");
}


/* This shows the file selection dialog to open a file. */
static void
open_file (GtkWidget *main_window)
{
  /* We use the same file selection widget each time, so first
     of all we create it if it hasn't already been created. */
  if (open_filesel == NULL)
    open_filesel = create_open_file_selection ();

  /* We save a pointer to the main window inside the file selection's
     data list, so we can get it easily in the callbacks. */
  gtk_object_set_data (GTK_OBJECT (open_filesel), MainWindowKey, main_window);

  gtk_widget_show (open_filesel);
  gdk_window_raise (open_filesel->window);
}


/* This shows the file selection dialog to save a file. */
static void
save_as (GtkWidget *main_window)
{
  if (save_filesel == NULL)
    save_filesel = create_save_file_selection ();

  gtk_object_set_data (GTK_OBJECT (save_filesel), MainWindowKey, main_window);

  /* If the current document has a filename we use that as the default. */
  if (current_filename)
    gtk_file_selection_set_filename (GTK_FILE_SELECTION (save_filesel),
				     current_filename);

  gtk_widget_show (save_filesel);
  gdk_window_raise (save_filesel->window);
}


/* This loads the given file. */
static void
real_open_file (GtkWidget *main_window, gchar *filename)
{
  GtkWidget *text, *statusbar;
  FILE *fp;
  gchar buffer[BUFFER_SIZE];
  gint bytes_read;

  text = lookup_widget (main_window, "text1");
  statusbar = lookup_widget (main_window, "statusbar1");
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_text_freeze (GTK_TEXT (text));
  gtk_editable_delete_text (GTK_EDITABLE (text), 0, -1);
  g_free (current_filename);
  current_filename = NULL;
  file_changed = FALSE;

  fp = fopen (filename, "r");
  if (fp == NULL)
    {
      gtk_text_thaw (GTK_TEXT (text));
      gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Couldn't open file.");
      return;
    }

  for (;;)
    {
      bytes_read = fread (buffer, sizeof (gchar), BUFFER_SIZE, fp);

      if (bytes_read > 0)
	gtk_text_insert (GTK_TEXT (text), NULL, NULL, NULL, buffer,
			 bytes_read);

      if (bytes_read != BUFFER_SIZE && (feof (fp) || ferror (fp)))
	break;
    }

  /* If there is an error while loading, we reset everything to a good state.
   */
  if (ferror (fp))
    {
      fclose (fp);
      gtk_editable_delete_text (GTK_EDITABLE (text), 0, -1);
      gtk_text_thaw (GTK_TEXT (text));
      set_window_title (main_window);
      gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Error loading file.");
      return;
    }

  fclose (fp);
  gtk_text_thaw (GTK_TEXT (text));

  current_filename = g_strdup (filename);
  set_window_title (main_window);
  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "File opened.");
}


static void
real_save_file (GtkWidget *main_window, gchar *filename)
{
  GtkWidget *text, *statusbar;
  gchar *data;
  FILE *fp;
  gint bytes_written, len;

  if (current_filename == NULL || strcmp (current_filename, filename))
    {
      g_free (current_filename);
      current_filename = g_strdup (filename);
      set_window_title (main_window);
    }

  text = lookup_widget (GTK_WIDGET (main_window), "text1");
  statusbar = lookup_widget (main_window, "statusbar1");
  data = gtk_editable_get_chars (GTK_EDITABLE (text), 0, -1);

  fp = fopen (filename, "w");
  if (fp == NULL)
    {
      g_free (data);
      return;
    }

  len = strlen (data);
  bytes_written = fwrite (data, sizeof (gchar), len, fp);
  fclose (fp);

  g_free (data);

  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), 1);
  if (len != bytes_written)
    {
      gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "Error saving file.");
      return;
    }

  file_changed = FALSE;
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), 1, "File saved.");
}

