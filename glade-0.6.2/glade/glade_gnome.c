
/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998-1999  Damon Chaplin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * This file contains general Gnome-related decalarations & code.
 */

#include <config.h>

#ifdef USE_GNOME

#include <gnome.h>

#include "glade_gnome.h"
#include "glade_project.h"
#include "source.h"
#include "utils.h"


static void glade_gnome_write_menu_item_standard_source (GtkMenuItem * widget,
							 GbWidgetWriteSourceData * data);
static void glade_gnome_write_radio_menu_item_source (GtkMenuItem * widget,
						      GbWidgetWriteSourceData * data,
						      GString *source_buffer,
						      gchar *label,
						      gchar *tooltip,
						      gchar *handler,
						      gchar *accel_key_buffer,
						      gchar *accel_mods);
static void glade_gnome_finish_radio_menu_item_group_source (GtkMenuShell * menu_shell,
							     GbWidgetWriteSourceData *data);
static gboolean glade_gnome_is_first_radio_menu_item_in_group (GtkMenuItem *menuitem);



/*
 * I intend to tidy up all the code for choice properties at some point,
 * so we don't duplicate them and we use simple convenience functions for them.
 */

/* Choices for the GnomeDockItem placement. */
const gint GladePlacementValues[] =
{
  GNOME_DOCK_TOP,
  GNOME_DOCK_RIGHT,
  GNOME_DOCK_BOTTOM,
  GNOME_DOCK_LEFT,
  GNOME_DOCK_FLOATING
};
const gint GladePlacementSize = sizeof (GladePlacementValues) / sizeof (gint);
const gchar *GladePlacementSymbols[] =
{
  "GNOME_DOCK_TOP",
  "GNOME_DOCK_RIGHT",
  "GNOME_DOCK_BOTTOM",
  "GNOME_DOCK_LEFT",
  "GNOME_DOCK_FLOATING"
};


/* Choices for the GnomeDockItem Orientation. */
const gchar *GladeOrientationChoices[] =
{
  "Horizontal",
  "Vertical",
  NULL
};
const gint GladeOrientationValues[] =
{
  GTK_ORIENTATION_HORIZONTAL,
  GTK_ORIENTATION_VERTICAL
};
const gint GladeOrientationSize = sizeof (GladeOrientationValues) / sizeof (GladeOrientationValues[0]);
const gchar *GladeOrientationSymbols[] =
{
  "GTK_ORIENTATION_HORIZONTAL",
  "GTK_ORIENTATION_VERTICAL"
};


/*************************************************************************
 * Stock Gnome Menu Items.
 *************************************************************************/

/* These are indices of a few special items, so if you change the
   GladeStockMenuItemValues array be sure to update this. */
const gint GladeStockMenuItemNew = 3;
const gint GladeStockMenuHelpTree = 37;

/* These are copied from gnome-libs/libgnomeui/gnome-app-helper.h
   The first item is 'None' to indicate no stock item is being used. */
GnomeUIInfo GladeStockMenuItemValues[] =
{
  /* 0 */
  {
    GNOME_APP_UI_ITEM, N_("None"), NULL, NULL, NULL, NULL,
    GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL
  },
  /* 1 */
  GNOMEUIINFO_SEPARATOR,

  /* 2 */
  GNOMEUIINFO_MENU_FILE_TREE (NULL),
  /* 3 - see GladeStockMenuItemNew above. */
  GNOMEUIINFO_MENU_NEW_ITEM (N_("_New"), NULL, NULL, NULL),
  GNOMEUIINFO_MENU_OPEN_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_SAVE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_SAVE_AS_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_REVERT_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_PRINT_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_PRINT_SETUP_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_CLOSE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_EXIT_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,

  /* 13 */
  GNOMEUIINFO_MENU_EDIT_TREE (NULL),
  GNOMEUIINFO_MENU_CUT_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_COPY_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_PASTE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_SELECT_ALL_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_CLEAR_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_UNDO_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_REDO_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_FIND_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_FIND_AGAIN_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_REPLACE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_PROPERTIES_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,

  /* 26 */
  GNOMEUIINFO_MENU_VIEW_TREE (NULL),
  GNOMEUIINFO_SEPARATOR,

  /* 28 */
  GNOMEUIINFO_MENU_SETTINGS_TREE (NULL),
  GNOMEUIINFO_MENU_PREFERENCES_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,

  /* 31 */
  GNOMEUIINFO_MENU_FILES_TREE (NULL),
  GNOMEUIINFO_SEPARATOR,

  /* 33 */
  GNOMEUIINFO_MENU_WINDOWS_TREE (NULL),
  GNOMEUIINFO_MENU_NEW_WINDOW_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_CLOSE_WINDOW_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,

  /* 37 */
  GNOMEUIINFO_MENU_HELP_TREE (NULL),
  GNOMEUIINFO_MENU_ABOUT_ITEM (NULL, NULL),
  GNOMEUIINFO_SEPARATOR,

  /* 40 */
  GNOMEUIINFO_MENU_GAME_TREE (NULL),
  GNOMEUIINFO_MENU_NEW_GAME_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_PAUSE_GAME_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_RESTART_GAME_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_UNDO_MOVE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_REDO_MOVE_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_HINT_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_SCORES_ITEM (NULL, NULL),
  GNOMEUIINFO_MENU_END_GAME_ITEM (NULL, NULL),
  GNOMEUIINFO_END
};


/* These were created by a perl script using the above. */
const gchar* GladeStockMenuItemSymbols[] =
{
  NULL,
  NULL,
  "GNOMEUIINFO_MENU_FILE_TREE",
  "GNOMEUIINFO_MENU_NEW_ITEM",
  "GNOMEUIINFO_MENU_OPEN_ITEM",
  "GNOMEUIINFO_MENU_SAVE_ITEM",
  "GNOMEUIINFO_MENU_SAVE_AS_ITEM",
  "GNOMEUIINFO_MENU_REVERT_ITEM",
  "GNOMEUIINFO_MENU_PRINT_ITEM",
  "GNOMEUIINFO_MENU_PRINT_SETUP_ITEM",
  "GNOMEUIINFO_MENU_CLOSE_ITEM",
  "GNOMEUIINFO_MENU_EXIT_ITEM",
  NULL,
  "GNOMEUIINFO_MENU_EDIT_TREE",
  "GNOMEUIINFO_MENU_CUT_ITEM",
  "GNOMEUIINFO_MENU_COPY_ITEM",
  "GNOMEUIINFO_MENU_PASTE_ITEM",
  "GNOMEUIINFO_MENU_SELECT_ALL_ITEM",
  "GNOMEUIINFO_MENU_CLEAR_ITEM",
  "GNOMEUIINFO_MENU_UNDO_ITEM",
  "GNOMEUIINFO_MENU_REDO_ITEM",
  "GNOMEUIINFO_MENU_FIND_ITEM",
  "GNOMEUIINFO_MENU_FIND_AGAIN_ITEM",
  "GNOMEUIINFO_MENU_REPLACE_ITEM",
  "GNOMEUIINFO_MENU_PROPERTIES_ITEM",
  NULL,
  "GNOMEUIINFO_MENU_VIEW_TREE",
  NULL,
  "GNOMEUIINFO_MENU_SETTINGS_TREE",
  "GNOMEUIINFO_MENU_PREFERENCES_ITEM",
  NULL,
  "GNOMEUIINFO_MENU_FILES_TREE",
  NULL,
  "GNOMEUIINFO_MENU_WINDOWS_TREE",
  "GNOMEUIINFO_MENU_NEW_WINDOW_ITEM",
  "GNOMEUIINFO_MENU_CLOSE_WINDOW_ITEM",
  NULL,
  "GNOMEUIINFO_MENU_HELP_TREE",
  "GNOMEUIINFO_MENU_ABOUT_ITEM",
  NULL,
  "GNOMEUIINFO_MENU_GAME_TREE",
  "GNOMEUIINFO_MENU_NEW_GAME_ITEM",
  "GNOMEUIINFO_MENU_PAUSE_GAME_ITEM",
  "GNOMEUIINFO_MENU_RESTART_GAME_ITEM",
  "GNOMEUIINFO_MENU_UNDO_MOVE_ITEM",
  "GNOMEUIINFO_MENU_REDO_MOVE_ITEM",
  "GNOMEUIINFO_MENU_HINT_ITEM",
  "GNOMEUIINFO_MENU_SCORES_ITEM",
  "GNOMEUIINFO_MENU_END_GAME_ITEM",
  NULL
};

const gint GladeStockMenuItemSize = sizeof (GladeStockMenuItemSymbols) / sizeof (GladeStockMenuItemSymbols[0]);


/* Returns the index of the given stock menu pixmap name in the
   GladeStockMenuPixmapChoices[] and related arrays, or -1 if the icon name
   isn't found. The icon name should be the gettext-translated name. */
gint
glade_gnome_get_stock_menu_pixmap_index (const gchar *icon_name)
{
  static GHashTable *stock_menu_icon_hash = NULL;

  gint stock_index, i;

  /* If this is the first call to this function, we create the hash. */
  if (stock_menu_icon_hash == NULL)
    {
      stock_menu_icon_hash = g_hash_table_new (g_str_hash, g_str_equal);

      for (i = 0; GladeStockMenuPixmapChoices[i]; i++)
	{
	  /* We add 1 to the index to distinguish an index of 0 from an
	     unknown icon name. */
	  g_hash_table_insert (stock_menu_icon_hash,
			       _(GladeStockMenuPixmapChoices[i]),
			       GINT_TO_POINTER (i + 1));
	}
    }

  /* Now lookup the icon name in the hash. */
  stock_index = GPOINTER_TO_INT (g_hash_table_lookup (stock_menu_icon_hash,
						      icon_name));

  /* Remember to subtract 1 to get the real index. This also means that
     if the icon_name wasn't found we return -1. */
  return stock_index - 1;
}


/*************************************************************************
 * Stock Gnome Menu Pixmaps.
 *************************************************************************/

/* These were generated from gnome-stock.h with a Perl script, using
   gnome-libs 1.0.16. */
const gchar *GladeStockMenuPixmapChoices[] =
{
  N_("Blank"),
  N_("New"),
  N_("Save"),
  N_("Save As"),
  N_("Revert"),
  N_("Open"),
  N_("Close"),
  N_("Quit"),
  N_("Cut"),
  N_("Copy"),
  N_("Paste"),
  N_("Properties"),
  N_("Preferences"),
  N_("About"),
  N_("Scores"),
  N_("Undo"),
  N_("Redo"),
  N_("Print"),
  N_("Search"),
  N_("Search/Replace"),
  N_("Back"),
  N_("Forward"),
  N_("First"),
  N_("Last"),
  N_("Home"),
  N_("Stop"),
  N_("Refresh"),
  N_("Mail"),
  N_("Receive Mail"),
  N_("Send Mail"),
  N_("Reply to Mail"),
  N_("Forward Mail"),
  N_("New Mail"),
  N_("Trash"),
  N_("Trash Full"),
  N_("Undelete"),
  N_("Timer"),
  N_("Timer Stopped"),
  N_("Spellchecker"),
  N_("Microphone"),
  N_("Line In"),
  N_("Cdrom"),
  N_("Volume"),
  N_("Midi"),
  N_("Book Red"),
  N_("Book Green"),
  N_("Book Blue"),
  N_("Book Yellow"),
  N_("Book Open"),
  N_("Convert"),
  N_("Jump To"),
  N_("Up"),
  N_("Down"),
  N_("Top"),
  N_("Bottom"),
  N_("Attach"),
  N_("Index"),
  N_("Font"),
  N_("Exec"),
  N_("Left"),
  N_("Right"),
  N_("Center"),
  N_("Justify"),
  N_("Bold"),
  N_("Italic"),
  N_("Underline"),
  N_("Strikeout"),

  NULL
};

const gchar *GladeStockMenuPixmapValues[] =
{
  GNOME_STOCK_MENU_BLANK,
  GNOME_STOCK_MENU_NEW,
  GNOME_STOCK_MENU_SAVE,
  GNOME_STOCK_MENU_SAVE_AS,
  GNOME_STOCK_MENU_REVERT,
  GNOME_STOCK_MENU_OPEN,
  GNOME_STOCK_MENU_CLOSE,
  GNOME_STOCK_MENU_EXIT,
  GNOME_STOCK_MENU_CUT,
  GNOME_STOCK_MENU_COPY,
  GNOME_STOCK_MENU_PASTE,
  GNOME_STOCK_MENU_PROP,
  GNOME_STOCK_MENU_PREF,
  GNOME_STOCK_MENU_ABOUT,
  GNOME_STOCK_MENU_SCORES,
  GNOME_STOCK_MENU_UNDO,
  GNOME_STOCK_MENU_REDO,
  GNOME_STOCK_MENU_PRINT,
  GNOME_STOCK_MENU_SEARCH,
  GNOME_STOCK_MENU_SRCHRPL,
  GNOME_STOCK_MENU_BACK,
  GNOME_STOCK_MENU_FORWARD,
  GNOME_STOCK_MENU_FIRST,
  GNOME_STOCK_MENU_LAST,
  GNOME_STOCK_MENU_HOME,
  GNOME_STOCK_MENU_STOP,
  GNOME_STOCK_MENU_REFRESH,
  GNOME_STOCK_MENU_MAIL,
  GNOME_STOCK_MENU_MAIL_RCV,
  GNOME_STOCK_MENU_MAIL_SND,
  GNOME_STOCK_MENU_MAIL_RPL,
  GNOME_STOCK_MENU_MAIL_FWD,
  GNOME_STOCK_MENU_MAIL_NEW,
  GNOME_STOCK_MENU_TRASH,
  GNOME_STOCK_MENU_TRASH_FULL,
  GNOME_STOCK_MENU_UNDELETE,
  GNOME_STOCK_MENU_TIMER,
  GNOME_STOCK_MENU_TIMER_STOP,
  GNOME_STOCK_MENU_SPELLCHECK,
  GNOME_STOCK_MENU_MIC,
  GNOME_STOCK_MENU_LINE_IN,
  GNOME_STOCK_MENU_CDROM,
  GNOME_STOCK_MENU_VOLUME,
  GNOME_STOCK_MENU_MIDI,
  GNOME_STOCK_MENU_BOOK_RED,
  GNOME_STOCK_MENU_BOOK_GREEN,
  GNOME_STOCK_MENU_BOOK_BLUE,
  GNOME_STOCK_MENU_BOOK_YELLOW,
  GNOME_STOCK_MENU_BOOK_OPEN,
  GNOME_STOCK_MENU_CONVERT,
  GNOME_STOCK_MENU_JUMP_TO,
  GNOME_STOCK_MENU_UP,
  GNOME_STOCK_MENU_DOWN,
  GNOME_STOCK_MENU_TOP,
  GNOME_STOCK_MENU_BOTTOM,
  GNOME_STOCK_MENU_ATTACH,
  GNOME_STOCK_MENU_INDEX,
  GNOME_STOCK_MENU_FONT,
  GNOME_STOCK_MENU_EXEC,
  GNOME_STOCK_MENU_ALIGN_LEFT,
  GNOME_STOCK_MENU_ALIGN_RIGHT,
  GNOME_STOCK_MENU_ALIGN_CENTER,
  GNOME_STOCK_MENU_ALIGN_JUSTIFY,
  GNOME_STOCK_MENU_TEXT_BOLD,
  GNOME_STOCK_MENU_TEXT_ITALIC,
  GNOME_STOCK_MENU_TEXT_UNDERLINE,
  GNOME_STOCK_MENU_TEXT_STRIKEOUT,

  NULL
};

const gchar *GladeStockMenuPixmapSymbols[] =
{
  "GNOME_STOCK_MENU_BLANK",
  "GNOME_STOCK_MENU_NEW",
  "GNOME_STOCK_MENU_SAVE",
  "GNOME_STOCK_MENU_SAVE_AS",
  "GNOME_STOCK_MENU_REVERT",
  "GNOME_STOCK_MENU_OPEN",
  "GNOME_STOCK_MENU_CLOSE",
  "GNOME_STOCK_MENU_EXIT",
  "GNOME_STOCK_MENU_CUT",
  "GNOME_STOCK_MENU_COPY",
  "GNOME_STOCK_MENU_PASTE",
  "GNOME_STOCK_MENU_PROP",
  "GNOME_STOCK_MENU_PREF",
  "GNOME_STOCK_MENU_ABOUT",
  "GNOME_STOCK_MENU_SCORES",
  "GNOME_STOCK_MENU_UNDO",
  "GNOME_STOCK_MENU_REDO",
  "GNOME_STOCK_MENU_PRINT",
  "GNOME_STOCK_MENU_SEARCH",
  "GNOME_STOCK_MENU_SRCHRPL",
  "GNOME_STOCK_MENU_BACK",
  "GNOME_STOCK_MENU_FORWARD",
  "GNOME_STOCK_MENU_FIRST",
  "GNOME_STOCK_MENU_LAST",
  "GNOME_STOCK_MENU_HOME",
  "GNOME_STOCK_MENU_STOP",
  "GNOME_STOCK_MENU_REFRESH",
  "GNOME_STOCK_MENU_MAIL",
  "GNOME_STOCK_MENU_MAIL_RCV",
  "GNOME_STOCK_MENU_MAIL_SND",
  "GNOME_STOCK_MENU_MAIL_RPL",
  "GNOME_STOCK_MENU_MAIL_FWD",
  "GNOME_STOCK_MENU_MAIL_NEW",
  "GNOME_STOCK_MENU_TRASH",
  "GNOME_STOCK_MENU_TRASH_FULL",
  "GNOME_STOCK_MENU_UNDELETE",
  "GNOME_STOCK_MENU_TIMER",
  "GNOME_STOCK_MENU_TIMER_STOP",
  "GNOME_STOCK_MENU_SPELLCHECK",
  "GNOME_STOCK_MENU_MIC",
  "GNOME_STOCK_MENU_LINE_IN",
  "GNOME_STOCK_MENU_CDROM",
  "GNOME_STOCK_MENU_VOLUME",
  "GNOME_STOCK_MENU_MIDI",
  "GNOME_STOCK_MENU_BOOK_RED",
  "GNOME_STOCK_MENU_BOOK_GREEN",
  "GNOME_STOCK_MENU_BOOK_BLUE",
  "GNOME_STOCK_MENU_BOOK_YELLOW",
  "GNOME_STOCK_MENU_BOOK_OPEN",
  "GNOME_STOCK_MENU_CONVERT",
  "GNOME_STOCK_MENU_JUMP_TO",
  "GNOME_STOCK_MENU_UP",
  "GNOME_STOCK_MENU_DOWN",
  "GNOME_STOCK_MENU_TOP",
  "GNOME_STOCK_MENU_BOTTOM",
  "GNOME_STOCK_MENU_ATTACH",
  "GNOME_STOCK_MENU_INDEX",
  "GNOME_STOCK_MENU_FONT",
  "GNOME_STOCK_MENU_EXEC",
  "GNOME_STOCK_MENU_ALIGN_LEFT",
  "GNOME_STOCK_MENU_ALIGN_RIGHT",
  "GNOME_STOCK_MENU_ALIGN_CENTER",
  "GNOME_STOCK_MENU_ALIGN_JUSTIFY",
  "GNOME_STOCK_MENU_TEXT_BOLD",
  "GNOME_STOCK_MENU_TEXT_ITALIC",
  "GNOME_STOCK_MENU_TEXT_UNDERLINE",
  "GNOME_STOCK_MENU_TEXT_STRIKEOUT",

  NULL
};

const gint GladeStockMenuPixmapSize = sizeof (GladeStockMenuPixmapSymbols) / sizeof (GladeStockMenuPixmapSymbols[0]);


/* Returns the index of the given stock pixmap name in the
   GladeStockPixmapChoices[] and related arrays, or -1 if the icon name
   isn't found. The icon name should be the gettext-translated name. */
gint
glade_gnome_get_stock_pixmap_index (const gchar *icon_name)
{
  static GHashTable *stock_pixmap_hash = NULL;

  gint stock_index, i;

  /* If this is the first call to this function, we create the hash. */
  if (stock_pixmap_hash == NULL)
    {
      stock_pixmap_hash = g_hash_table_new (g_str_hash, g_str_equal);

      for (i = 0; GladeStockPixmapChoices[i]; i++)
	{
	  /* We add 1 to the index to distinguish an index of 0 from an
	     unknown icon name. */
	  g_hash_table_insert (stock_pixmap_hash,
			       _(GladeStockPixmapChoices[i]),
			       GINT_TO_POINTER (i + 1));
	}
    }

  /* Now lookup the icon name in the hash. */
  stock_index = GPOINTER_TO_INT (g_hash_table_lookup (stock_pixmap_hash,
						      icon_name));

  /* Remember to subtract 1 to get the real index. This also means that
     if the icon_name wasn't found we return -1. */
  return stock_index - 1;
}


/*************************************************************************
 * Stock Gnome Pixmaps.
 *************************************************************************/

/* These were generated from gnome-stock.h with a Perl script, using
   gnome-libs 1.0.16.
   Note that ToolBarIndices in gnome/gnomeapp.c uses indexes into this array
   so if this is changed make sure they are updated. */
const gchar *GladeStockPixmapChoices[] =
{
  N_("None"),
  N_("New"),
  N_("Open"),
  N_("Close"),
  N_("Revert"),
  N_("Save"),
  N_("Save As"),
  N_("Cut"),
  N_("Copy"),
  N_("Paste"),
  N_("Clear"),
  N_("Properties"),
  N_("Preferences"),
  N_("Help"),
  N_("Scores"),
  N_("Print"),
  N_("Search"),
  N_("Search/Replace"),
  N_("Back"),
  N_("Forward"),
  N_("First"),
  N_("Last"),
  N_("Home"),
  N_("Stop"),
  N_("Refresh"),
  N_("Undo"),
  N_("Redo"),
  N_("Timer"),
  N_("Timer Stopped"),
  N_("Mail"),
  N_("Receive Mail"),
  N_("Send Mail"),
  N_("Reply to Mail"),
  N_("Forward Mail"),
  N_("New Mail"),
  N_("Trash"),
  N_("Trash Full"),
  N_("Undelete"),
  N_("Spellchecker"),
  N_("Microphone"),
  N_("Line In"),
  N_("Cdrom"),
  N_("Volume"),
  N_("Midi"),
  N_("Book Red"),
  N_("Book Green"),
  N_("Book Blue"),
  N_("Book Yellow"),
  N_("Book Open"),
  N_("About"),
  N_("Quit"),
  N_("Multiple"),
  N_("Not"),
  N_("Convert"),
  N_("Jump To"),
  N_("Up"),
  N_("Down"),
  N_("Top"),
  N_("Bottom"),
  N_("Attach"),
  N_("Index"),
  N_("Font"),
  N_("Exec"),
  N_("Left"),
  N_("Right"),
  N_("Center"),
  N_("Justify"),
  N_("Bold"),
  N_("Italic"),
  N_("Underline"),
  N_("Strikeout"),
  N_("Text Indent"),
  N_("Text Unindent"),
  N_("Color Select"),
  N_("Add"),
  N_("Remove"),
  N_("Table Borders"),
  N_("Table Fill"),
  N_("Text Bulleted List"),
  N_("Text Numbered List"),

  NULL
};

const gchar *GladeStockPixmapValues[] =
{
  NULL,
  GNOME_STOCK_PIXMAP_NEW,
  GNOME_STOCK_PIXMAP_OPEN,
  GNOME_STOCK_PIXMAP_CLOSE,
  GNOME_STOCK_PIXMAP_REVERT,
  GNOME_STOCK_PIXMAP_SAVE,
  GNOME_STOCK_PIXMAP_SAVE_AS,
  GNOME_STOCK_PIXMAP_CUT,
  GNOME_STOCK_PIXMAP_COPY,
  GNOME_STOCK_PIXMAP_PASTE,
  GNOME_STOCK_PIXMAP_CLEAR,
  GNOME_STOCK_PIXMAP_PROPERTIES,
  GNOME_STOCK_PIXMAP_PREFERENCES,
  GNOME_STOCK_PIXMAP_HELP,
  GNOME_STOCK_PIXMAP_SCORES,
  GNOME_STOCK_PIXMAP_PRINT,
  GNOME_STOCK_PIXMAP_SEARCH,
  GNOME_STOCK_PIXMAP_SRCHRPL,
  GNOME_STOCK_PIXMAP_BACK,
  GNOME_STOCK_PIXMAP_FORWARD,
  GNOME_STOCK_PIXMAP_FIRST,
  GNOME_STOCK_PIXMAP_LAST,
  GNOME_STOCK_PIXMAP_HOME,
  GNOME_STOCK_PIXMAP_STOP,
  GNOME_STOCK_PIXMAP_REFRESH,
  GNOME_STOCK_PIXMAP_UNDO,
  GNOME_STOCK_PIXMAP_REDO,
  GNOME_STOCK_PIXMAP_TIMER,
  GNOME_STOCK_PIXMAP_TIMER_STOP,
  GNOME_STOCK_PIXMAP_MAIL,
  GNOME_STOCK_PIXMAP_MAIL_RCV,
  GNOME_STOCK_PIXMAP_MAIL_SND,
  GNOME_STOCK_PIXMAP_MAIL_RPL,
  GNOME_STOCK_PIXMAP_MAIL_FWD,
  GNOME_STOCK_PIXMAP_MAIL_NEW,
  GNOME_STOCK_PIXMAP_TRASH,
  GNOME_STOCK_PIXMAP_TRASH_FULL,
  GNOME_STOCK_PIXMAP_UNDELETE,
  GNOME_STOCK_PIXMAP_SPELLCHECK,
  GNOME_STOCK_PIXMAP_MIC,
  GNOME_STOCK_PIXMAP_LINE_IN,
  GNOME_STOCK_PIXMAP_CDROM,
  GNOME_STOCK_PIXMAP_VOLUME,
  GNOME_STOCK_PIXMAP_MIDI,
  GNOME_STOCK_PIXMAP_BOOK_RED,
  GNOME_STOCK_PIXMAP_BOOK_GREEN,
  GNOME_STOCK_PIXMAP_BOOK_BLUE,
  GNOME_STOCK_PIXMAP_BOOK_YELLOW,
  GNOME_STOCK_PIXMAP_BOOK_OPEN,
  GNOME_STOCK_PIXMAP_ABOUT,
  GNOME_STOCK_PIXMAP_EXIT,
  GNOME_STOCK_PIXMAP_MULTIPLE,
  GNOME_STOCK_PIXMAP_NOT,
  GNOME_STOCK_PIXMAP_CONVERT,
  GNOME_STOCK_PIXMAP_JUMP_TO,
  GNOME_STOCK_PIXMAP_UP,
  GNOME_STOCK_PIXMAP_DOWN,
  GNOME_STOCK_PIXMAP_TOP,
  GNOME_STOCK_PIXMAP_BOTTOM,
  GNOME_STOCK_PIXMAP_ATTACH,
  GNOME_STOCK_PIXMAP_INDEX,
  GNOME_STOCK_PIXMAP_FONT,
  GNOME_STOCK_PIXMAP_EXEC,
  GNOME_STOCK_PIXMAP_ALIGN_LEFT,
  GNOME_STOCK_PIXMAP_ALIGN_RIGHT,
  GNOME_STOCK_PIXMAP_ALIGN_CENTER,
  GNOME_STOCK_PIXMAP_ALIGN_JUSTIFY,
  GNOME_STOCK_PIXMAP_TEXT_BOLD,
  GNOME_STOCK_PIXMAP_TEXT_ITALIC,
  GNOME_STOCK_PIXMAP_TEXT_UNDERLINE,
  GNOME_STOCK_PIXMAP_TEXT_STRIKEOUT,
  GNOME_STOCK_PIXMAP_TEXT_INDENT,
  GNOME_STOCK_PIXMAP_TEXT_UNINDENT,
  GNOME_STOCK_PIXMAP_COLORSELECTOR,
  GNOME_STOCK_PIXMAP_ADD,
  GNOME_STOCK_PIXMAP_REMOVE,
  GNOME_STOCK_PIXMAP_TABLE_BORDERS,
  GNOME_STOCK_PIXMAP_TABLE_FILL,
  GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST,
  GNOME_STOCK_PIXMAP_TEXT_NUMBERED_LIST,

  NULL
};

const gchar *GladeStockPixmapSymbols[] =
{
  NULL,
  "GNOME_STOCK_PIXMAP_NEW",
  "GNOME_STOCK_PIXMAP_OPEN",
  "GNOME_STOCK_PIXMAP_CLOSE",
  "GNOME_STOCK_PIXMAP_REVERT",
  "GNOME_STOCK_PIXMAP_SAVE",
  "GNOME_STOCK_PIXMAP_SAVE_AS",
  "GNOME_STOCK_PIXMAP_CUT",
  "GNOME_STOCK_PIXMAP_COPY",
  "GNOME_STOCK_PIXMAP_PASTE",
  "GNOME_STOCK_PIXMAP_CLEAR",
  "GNOME_STOCK_PIXMAP_PROPERTIES",
  "GNOME_STOCK_PIXMAP_PREFERENCES",
  "GNOME_STOCK_PIXMAP_HELP",
  "GNOME_STOCK_PIXMAP_SCORES",
  "GNOME_STOCK_PIXMAP_PRINT",
  "GNOME_STOCK_PIXMAP_SEARCH",
  "GNOME_STOCK_PIXMAP_SRCHRPL",
  "GNOME_STOCK_PIXMAP_BACK",
  "GNOME_STOCK_PIXMAP_FORWARD",
  "GNOME_STOCK_PIXMAP_FIRST",
  "GNOME_STOCK_PIXMAP_LAST",
  "GNOME_STOCK_PIXMAP_HOME",
  "GNOME_STOCK_PIXMAP_STOP",
  "GNOME_STOCK_PIXMAP_REFRESH",
  "GNOME_STOCK_PIXMAP_UNDO",
  "GNOME_STOCK_PIXMAP_REDO",
  "GNOME_STOCK_PIXMAP_TIMER",
  "GNOME_STOCK_PIXMAP_TIMER_STOP",
  "GNOME_STOCK_PIXMAP_MAIL",
  "GNOME_STOCK_PIXMAP_MAIL_RCV",
  "GNOME_STOCK_PIXMAP_MAIL_SND",
  "GNOME_STOCK_PIXMAP_MAIL_RPL",
  "GNOME_STOCK_PIXMAP_MAIL_FWD",
  "GNOME_STOCK_PIXMAP_MAIL_NEW",
  "GNOME_STOCK_PIXMAP_TRASH",
  "GNOME_STOCK_PIXMAP_TRASH_FULL",
  "GNOME_STOCK_PIXMAP_UNDELETE",
  "GNOME_STOCK_PIXMAP_SPELLCHECK",
  "GNOME_STOCK_PIXMAP_MIC",
  "GNOME_STOCK_PIXMAP_LINE_IN",
  "GNOME_STOCK_PIXMAP_CDROM",
  "GNOME_STOCK_PIXMAP_VOLUME",
  "GNOME_STOCK_PIXMAP_MIDI",
  "GNOME_STOCK_PIXMAP_BOOK_RED",
  "GNOME_STOCK_PIXMAP_BOOK_GREEN",
  "GNOME_STOCK_PIXMAP_BOOK_BLUE",
  "GNOME_STOCK_PIXMAP_BOOK_YELLOW",
  "GNOME_STOCK_PIXMAP_BOOK_OPEN",
  "GNOME_STOCK_PIXMAP_ABOUT",
  "GNOME_STOCK_PIXMAP_EXIT",
  "GNOME_STOCK_PIXMAP_MULTIPLE",
  "GNOME_STOCK_PIXMAP_NOT",
  "GNOME_STOCK_PIXMAP_CONVERT",
  "GNOME_STOCK_PIXMAP_JUMP_TO",
  "GNOME_STOCK_PIXMAP_UP",
  "GNOME_STOCK_PIXMAP_DOWN",
  "GNOME_STOCK_PIXMAP_TOP",
  "GNOME_STOCK_PIXMAP_BOTTOM",
  "GNOME_STOCK_PIXMAP_ATTACH",
  "GNOME_STOCK_PIXMAP_INDEX",
  "GNOME_STOCK_PIXMAP_FONT",
  "GNOME_STOCK_PIXMAP_EXEC",
  "GNOME_STOCK_PIXMAP_ALIGN_LEFT",
  "GNOME_STOCK_PIXMAP_ALIGN_RIGHT",
  "GNOME_STOCK_PIXMAP_ALIGN_CENTER",
  "GNOME_STOCK_PIXMAP_ALIGN_JUSTIFY",
  "GNOME_STOCK_PIXMAP_TEXT_BOLD",
  "GNOME_STOCK_PIXMAP_TEXT_ITALIC",
  "GNOME_STOCK_PIXMAP_TEXT_UNDERLINE",
  "GNOME_STOCK_PIXMAP_TEXT_STRIKEOUT",
  "GNOME_STOCK_PIXMAP_TEXT_INDENT",
  "GNOME_STOCK_PIXMAP_TEXT_UNINDENT",
  "GNOME_STOCK_PIXMAP_COLORSELECTOR",
  "GNOME_STOCK_PIXMAP_ADD",
  "GNOME_STOCK_PIXMAP_REMOVE",
  "GNOME_STOCK_PIXMAP_TABLE_BORDERS",
  "GNOME_STOCK_PIXMAP_TABLE_FILL",
  "GNOME_STOCK_PIXMAP_TEXT_BULLETED_LIST",
  "GNOME_STOCK_PIXMAP_TEXT_NUMBERED_LIST",

  NULL
};

const gint GladeStockPixmapSize = sizeof (GladeStockPixmapSymbols) / sizeof (GladeStockPixmapSymbols[0]);

/*************************************************************************
 * Stock Gnome Buttons.
 *************************************************************************/

/* These must be the index of the corresponding item in the arrays below.
   They are used when creating GnomeDialog & GnomeMessageBox widgets. */
const gint GladeStockButtonOK = 1;
const gint GladeStockButtonApply = 6;
const gint GladeStockButtonCancel = 2;

const gchar *GladeStockButtonChoices[] =
{
  "None",
  "OK",
  "Cancel",
  "Yes",
  "No",
  "Close",
  "Apply",
  "Help",
  "Next",
  "Prev",
  "Up",
  "Down",
  "Font",
  NULL
};
/* Note these are strings rather than gints as we normally use for choices. */
const gchar* GladeStockButtonValues[] =
{
  NULL,
  GNOME_STOCK_BUTTON_OK,
  GNOME_STOCK_BUTTON_CANCEL,
  GNOME_STOCK_BUTTON_YES,
  GNOME_STOCK_BUTTON_NO,
  GNOME_STOCK_BUTTON_CLOSE,
  GNOME_STOCK_BUTTON_APPLY,
  GNOME_STOCK_BUTTON_HELP,
  GNOME_STOCK_BUTTON_NEXT,
  GNOME_STOCK_BUTTON_PREV,
  GNOME_STOCK_BUTTON_UP,
  GNOME_STOCK_BUTTON_DOWN,
  GNOME_STOCK_BUTTON_FONT
};
const gchar *GladeStockButtonSymbols[] =
{
  NULL,
  "GNOME_STOCK_BUTTON_OK",
  "GNOME_STOCK_BUTTON_CANCEL",
  "GNOME_STOCK_BUTTON_YES",
  "GNOME_STOCK_BUTTON_NO",
  "GNOME_STOCK_BUTTON_CLOSE",
  "GNOME_STOCK_BUTTON_APPLY",
  "GNOME_STOCK_BUTTON_HELP",
  "GNOME_STOCK_BUTTON_NEXT",
  "GNOME_STOCK_BUTTON_PREV",
  "GNOME_STOCK_BUTTON_UP",
  "GNOME_STOCK_BUTTON_DOWN",
  "GNOME_STOCK_BUTTON_FONT"
};

const gint GladeStockButtonSize = sizeof (GladeStockButtonSymbols) / sizeof (GladeStockButtonSymbols[0]);


/*************************************************************************
 * Functions to output GnomeUIInfo structs in the source code.
 *************************************************************************/

/* This creates the start of the GnomeUIInfo structs, and is called by the
   write_source functions of menus and menubars if we are building a Gnome
   application. The menuitems call glade_gnome_write_menu_item_source() to
   add to this, and the entire structure is added to one of the main source
   buffers at the end of gb_widget_write_source (), by calling
   glade_gnome_finish_menu_source(). */
void
glade_gnome_start_menu_source (GtkMenuShell * menu_shell,
			       GbWidgetWriteSourceData * data)
{
  GString *source_buffer;
  GtkWidget *parent_item;
  gint uiinfo_index = 0, stock_item_index;

  source_buffer = g_string_sized_new (1024);
  g_string_sprintf (source_buffer, "static GnomeUIInfo %s_uiinfo[] =\n{\n",
		    data->real_wname);
  gtk_object_set_data (GTK_OBJECT (menu_shell), "glade-uiinfo", source_buffer);
  gtk_object_set_data (GTK_OBJECT (menu_shell), "glade-uiinfo-name",
		       g_strdup (data->real_wname));

  /* See if this is the start of the 'Help' menu, and if it is output the
     GNOMEUIINFO_HELP item which inserts all the topics from topic.dat. */
  if (glade_project_get_gnome_help_support (data->project)
      && GTK_IS_MENU (menu_shell))
    {
      parent_item = gtk_menu_get_attach_widget (GTK_MENU (menu_shell));

      if (parent_item)
	{
	  stock_item_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (parent_item), GladeMenuItemStockIndexKey));
	  if (stock_item_index == GladeStockMenuHelpTree)
	    {
	      g_string_sprintfa (source_buffer, "  GNOMEUIINFO_HELP (%s),\n",
				 source_make_string (data->program_name,
						     FALSE));
	      uiinfo_index++;
	    }
	}
    }

  gtk_object_set_data (GTK_OBJECT (menu_shell), "glade-uiinfo-index",
		       GINT_TO_POINTER (uiinfo_index));
}


/* This is called after a menu or menubar has been output. It finishes off
   the GnomeUIInfo struct and adds it to the GLADE_UIINFO source buffer. */
void
glade_gnome_finish_menu_source (GtkMenuShell * menu_shell,
				GbWidgetWriteSourceData * data)
{
  GString *source_buffer;
  gchar *uiinfo_name;

  /* Finish off any radio group. */
  glade_gnome_finish_radio_menu_item_group_source (menu_shell, data);

  source_buffer = gtk_object_get_data (GTK_OBJECT (menu_shell),
				       "glade-uiinfo");
  g_return_if_fail (source_buffer != NULL);

  uiinfo_name = gtk_object_get_data (GTK_OBJECT (menu_shell),
				     "glade-uiinfo-name");
  g_return_if_fail (uiinfo_name != NULL);

  g_string_append (source_buffer, "  GNOMEUIINFO_END\n};\n\n");
  g_string_append (data->source_buffers[GLADE_UIINFO], source_buffer->str);
  g_string_free (source_buffer, TRUE);
  gtk_object_set_data (GTK_OBJECT (menu_shell), "glade-uiinfo", NULL);

  g_free (uiinfo_name);
  gtk_object_set_data (GTK_OBJECT (menu_shell), "glade-uiinfo-name", NULL);
}


/* This is called by the write_source functions of all menu items when
   outputting Gnome source code. It adds to the GnomeUIInfo structs stored
   in the parent menu/menubar. */
void
glade_gnome_write_menu_item_source (GtkMenuItem * widget,
				    GbWidgetWriteSourceData * data)
{
  GString *source_buffer;
  gint stock_item_index;
  gboolean has_child_menu = FALSE;
  GList *tmp_list;
  GladeSignal *activate_signal = NULL;
  GladeAccelerator *activate_accel = NULL;
  gchar *handler = NULL;

  source_buffer = gtk_object_get_data (GTK_OBJECT (GTK_WIDGET (widget)->parent), "glade-uiinfo");
  g_return_if_fail (source_buffer != NULL);

  if (widget->submenu)
    has_child_menu = TRUE;

  /* First check if it is a separator, since they are simple. */
  if (GTK_BIN (widget)->child == NULL)
    {
      g_string_append (source_buffer, "  GNOMEUIINFO_SEPARATOR,\n");
      glade_gnome_write_menu_item_standard_source (widget, data);
      return;
    }


  /* Find 'activate' handler in widget data. */
  tmp_list = data->widget_data->signals;
  while (tmp_list)
    {
      GladeSignal *signal = (GladeSignal *) tmp_list->data;
      if (!strcmp (signal->name, "activate"))
	{
	  activate_signal = signal;
	  handler = source_create_valid_identifier (signal->handler);
	  break;
	}
      tmp_list = tmp_list->next;
    }

  /* Find 'activate' accelerator in widget data. */
  tmp_list = data->widget_data->accelerators;
  while (tmp_list)
    {
      GladeAccelerator *accel = (GladeAccelerator *) tmp_list->data;
      if (!strcmp (accel->signal, "activate"))
	{
	  activate_accel = accel;
	  break;
	}
      tmp_list = tmp_list->next;
    }

  /* See if it is a stock item. */
  stock_item_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget),
							   GladeMenuItemStockIndexKey));
  if (stock_item_index > 0)
    {
      GnomeUIInfo *uiinfo;

      uiinfo = &GladeStockMenuItemValues[stock_item_index];

      /* Special code for the New item. If it has children it is
	 GNOMEUIINFO_MENU_NEW_SUBTREE, else it is GNOMEUIINFO_MENU_NEW_ITEM
	 and we must also output the label and tooltip. */
      if (stock_item_index == GladeStockMenuItemNew)
	{
	  if (has_child_menu)
	    {
	      g_string_sprintfa (source_buffer,
				 "  GNOMEUIINFO_MENU_NEW_SUBTREE (%s_menu_uiinfo),\n",
				 data->real_wname);
	    }
	  else
	    {
	      gchar *label, *tooltip;

	      label = glade_util_get_label_text (GTK_BIN (widget)->child);
	      tooltip = data->widget_data->tooltip ? g_strdup (source_make_static_string (data->widget_data->tooltip, data->use_gettext)) : NULL;

	      g_string_sprintfa (source_buffer,
				 "  GNOMEUIINFO_MENU_NEW_ITEM (%s, %s, %s, NULL),\n",
				 source_make_static_string (label, data->use_gettext),
				 tooltip ? tooltip : "NULL",
				 handler ? handler : "NULL");
	      g_free (tooltip);
	      g_free (label);
	    }
	}
      /* See if it is a subtree item. */
      else if (uiinfo->type == GNOME_APP_UI_SUBTREE_STOCK)
	{
	  g_string_sprintfa (source_buffer, "  %s (%s_menu_uiinfo),\n",
			     GladeStockMenuItemSymbols[stock_item_index],
			     data->real_wname);

	  /* The stock subtree item should really have children, but if it
	     doesn't, we just output an empty GnomeUIInfo struct here. */
	  if (!has_child_menu)
	    {
	      g_string_sprintfa (data->source_buffers[GLADE_UIINFO],
				 "static GnomeUIInfo %s_menu_uiinfo[] =\n{\n"
				 "  GNOMEUIINFO_END\n};\n\n",
				 data->real_wname);
	    }
	}
      /* It must be a normal stock item. */
      else
	{
	  g_string_sprintfa (source_buffer, "  %s (%s, NULL),\n",
			     GladeStockMenuItemSymbols[stock_item_index],
			     handler ? handler : "NULL");
	}
    }
  else
    /* It is an ordinary menu item, so we need to output its type, label,
       tooltip, accelerator, and icon (stock or ordinary), and also check
       if it has a child menu. */
    {
      gchar *type, *label, *tooltip, *icon_name, *accel_mods;
      gchar *pixmap_type, *pixmap_info, *pixmap_filename;
      gchar accel_key_buffer[128];
      gboolean free_pixmap_info;
      
      label = glade_util_get_label_text (GTK_BIN (widget)->child);
      tooltip = data->widget_data->tooltip ? g_strdup (source_make_static_string (data->widget_data->tooltip, data->use_gettext)) : NULL;

      /* Determine if the item has an icon and if it is a stock icon or an
	 xpm file. */
      free_pixmap_info = FALSE;
      icon_name = gtk_object_get_data (GTK_OBJECT (widget),
				       GladeMenuItemIconKey);
      if (icon_name && icon_name[0])
	{
	  gint stock_icon_index;

	  stock_icon_index = glade_gnome_get_stock_menu_pixmap_index (icon_name);
	  if (stock_icon_index >= 0)
	    {
	      pixmap_type = "GNOME_APP_PIXMAP_STOCK";
	      pixmap_info = (gchar*)GladeStockMenuPixmapSymbols[stock_icon_index];
	    }
	  else
	    {
	      pixmap_type = "GNOME_APP_PIXMAP_FILENAME";
	      pixmap_filename = g_strdup_printf ("%s%c%s", data->program_name,
						 G_DIR_SEPARATOR,
						 g_basename (icon_name));
	      pixmap_info = g_strdup (source_make_string (pixmap_filename,
							  FALSE));
	      g_free (pixmap_filename);
	      free_pixmap_info = TRUE;
	    }
	}
      else
	{
	  pixmap_type = "GNOME_APP_PIXMAP_NONE";
	  pixmap_info = "NULL";
	}

      strcpy (accel_key_buffer, "0");
      accel_mods = "0";
      if (activate_accel)
	{
	  /* Make sure we don't overflow the buffer. */
	  if (strlen (activate_accel->key) < 100)
	    {
	      strcpy (accel_key_buffer, "GDK_");
	      strcat (accel_key_buffer, activate_accel->key);
	    }
	  else
	    g_warning ("Overflow of accel_key_buffer");

	  accel_mods = glade_util_create_modifiers_string (activate_accel->modifiers);
	}

      if (has_child_menu)
	{
	  type = "GNOME_APP_UI_SUBTREE";
	  g_string_sprintfa (source_buffer,
			     "  {\n"
			     "    %s, %s,\n"
			     "    %s,\n"
			     "    %s_menu_uiinfo, NULL, NULL,\n"
			     "    %s, %s,\n"
			     "    %s, (GdkModifierType) %s, NULL\n"
			     "  },\n",
			     type,
			     source_make_static_string (label,
							data->use_gettext),
			     tooltip ? tooltip : "NULL",
			     data->real_wname,
			     pixmap_type, pixmap_info,
			     accel_key_buffer, accel_mods);
	  
	}
      else
	{
	  if (GTK_IS_RADIO_MENU_ITEM (widget))
	    {
	      glade_gnome_write_radio_menu_item_source (widget, data,
							source_buffer, label,
							tooltip, handler,
							accel_key_buffer,
							accel_mods);
	    }
	  else
	    {
	      if (GTK_IS_CHECK_MENU_ITEM (widget))
		{
		  type = "GNOME_APP_UI_TOGGLEITEM";
		  pixmap_type = "GNOME_APP_PIXMAP_NONE";
		  if (free_pixmap_info)
		    g_free (pixmap_info);
		  pixmap_info = "NULL";
		  free_pixmap_info = FALSE;
		}
	      else
		type = "GNOME_APP_UI_ITEM";

	      g_string_sprintfa (source_buffer,
				 "  {\n"
				 "    %s, %s,\n"
				 "    %s,\n"
				 "    (gpointer) %s, NULL, NULL,\n"
				 "    %s, %s,\n"
				 "    %s, (GdkModifierType) %s, NULL\n"
				 "  },\n",
				 type,
				 source_make_static_string (label,
							    data->use_gettext),
				 tooltip ? tooltip : "NULL",
				 handler ? handler : "NULL",
				 pixmap_type, pixmap_info,
				 accel_key_buffer, accel_mods);
	    }
	}

      if (free_pixmap_info)
	g_free (pixmap_info);
      g_free (tooltip);
      g_free (label);
    }

  glade_gnome_write_menu_item_standard_source (widget, data);


  /* If there is an "activate" signal handler, we may need to output the empty
     handler function and declaration, depending on its last modification
     time. */
  if (activate_signal)
    {
      /* Check if we need to output it. */
      if (data->creating_callback_files
	  || (activate_signal->last_modification_time > data->last_write_time))
	{
	  gb_widget_write_signal_handler_source (GTK_WIDGET (widget), data,
						 "activate", handler);
	}
    }

  g_free (handler);
}


/* This outputs code to save the pointer to the widget and possibly set its
   name, using the index into the GnomeUIInfo struct. */
static void
glade_gnome_write_menu_item_standard_source (GtkMenuItem * widget,
					     GbWidgetWriteSourceData * data)
{
  GladeWidgetData *wdata = data->widget_data;
  gchar *uiinfo_name, *uiinfo_name_key, *uiinfo_index_key;
  gint uiinfo_index;

  if (GTK_IS_RADIO_MENU_ITEM (widget))
    {
      uiinfo_name_key = "glade-radio-group-uiinfo-name";
      uiinfo_index_key = "glade-radio-group-uiinfo-index";
    }
  else
    {
      uiinfo_name_key = "glade-uiinfo-name";
      uiinfo_index_key = "glade-uiinfo-index";
    }

  uiinfo_name = gtk_object_get_data (GTK_OBJECT (GTK_WIDGET (widget)->parent),
				     uiinfo_name_key);
  g_return_if_fail (uiinfo_name != NULL);

  uiinfo_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (GTK_WIDGET (widget)->parent), uiinfo_index_key));

  if (data->set_widget_names)
    source_add (data,
		"  gtk_widget_set_name (%s_uiinfo[%i].widget, \"%s\");\n",
		uiinfo_name, uiinfo_index, data->real_wname);

  if (!data->use_component_struct)
    {
      source_add (data,
		  "  gtk_widget_ref (%s_uiinfo[%i].widget);\n"
		  "  gtk_object_set_data_full (GTK_OBJECT (%s), %s,\n"
		  "                            %s_uiinfo[%i].widget,\n"
		  "                            (GtkDestroyNotify) gtk_widget_unref);\n",
		  uiinfo_name, uiinfo_index,
		  data->component_name,
		  source_make_string (data->real_wname, FALSE),
		  uiinfo_name, uiinfo_index);
    }

  if (GTK_IS_CHECK_MENU_ITEM (widget) && wdata->flags & GLADE_ACTIVE)
    source_add (data,
		"  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (%s_uiinfo[%i].widget), TRUE);\n",
		uiinfo_name, uiinfo_index);

  if (!(wdata->flags & GLADE_SENSITIVE))
    source_add (data,
		"  gtk_widget_set_sensitive (%s_uiinfo[%i].widget, FALSE);\n",
		uiinfo_name, uiinfo_index);

  gtk_object_set_data (GTK_OBJECT (GTK_WIDGET (widget)->parent),
		       uiinfo_index_key, GINT_TO_POINTER (uiinfo_index + 1));
}


/* This outputs the source for a radio menu item. The group of radio items
   is placed in a separate GnomeUIInfo struct, and a pointer to that is added
   in the menus GnomeUIInfo.
   First we check if the radio item is the start of a new group.
   If it is, we finish off any previous group and start a new one. */
static void
glade_gnome_write_radio_menu_item_source (GtkMenuItem * widget,
					  GbWidgetWriteSourceData * data,
					  GString *source_buffer,
					  gchar *label,
					  gchar *tooltip,
					  gchar *handler,
					  gchar *accel_key_buffer,
					  gchar *accel_mods)
{
  GString *group_source_buffer;
  gint uiinfo_index;

  if (glade_gnome_is_first_radio_menu_item_in_group (widget))
    {
      /* Finish off any existing group. */
      glade_gnome_finish_radio_menu_item_group_source (GTK_MENU_SHELL (GTK_WIDGET (widget)->parent), data);

      /* Start the new group. */
      group_source_buffer = g_string_sized_new (1024);
      g_string_sprintf (group_source_buffer,
			"static GnomeUIInfo %s_uiinfo[] =\n{\n",
			data->real_wname);
      gtk_object_set_data (GTK_OBJECT (GTK_WIDGET (widget)->parent),
			   "glade-radio-group-uiinfo", group_source_buffer);
      gtk_object_set_data (GTK_OBJECT (GTK_WIDGET (widget)->parent),
			   "glade-radio-group-uiinfo-name",
			   g_strdup (data->real_wname));
      gtk_object_set_data (GTK_OBJECT (GTK_WIDGET (widget)->parent),
			   "glade-radio-group-uiinfo-index",
			   GINT_TO_POINTER (0));

      /* Insert the radio group into the menu's GnomeUIInfo struct. */
      g_string_sprintfa (source_buffer,
			 "  {\n"
			 "    GNOME_APP_UI_RADIOITEMS, NULL, NULL, %s_uiinfo,\n"
			 "    NULL, NULL, GNOME_APP_PIXMAP_NONE, NULL, 0,\n"
			 "    (GdkModifierType) 0, NULL\n"
			 "  },\n",
			 data->real_wname);

      /* We need to increment the index of the parent uiinfo, so that the
	 GNOME_APP_UI_RADIOITEMS element is skipped. */
      uiinfo_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (GTK_WIDGET (widget)->parent), "glade-uiinfo-index"));
      gtk_object_set_data (GTK_OBJECT (GTK_WIDGET (widget)->parent),
			   "glade-uiinfo-index",
			   GINT_TO_POINTER (uiinfo_index + 1));
    }
  else
    {
      group_source_buffer = gtk_object_get_data (GTK_OBJECT (GTK_WIDGET (widget)->parent),
						 "glade-radio-group-uiinfo");
      g_return_if_fail (group_source_buffer != NULL);
    }

  g_string_sprintfa (group_source_buffer,
		     "  {\n"
		     "    GNOME_APP_UI_ITEM, %s,\n"
		     "    %s,\n"
		     "    (gpointer) %s, NULL, NULL,\n"
		     "    GNOME_APP_PIXMAP_NONE, NULL,\n"
		     "    %s, (GdkModifierType) %s, NULL\n"
		     "  },\n",
		     source_make_static_string (label,
						data->use_gettext),
		     tooltip ? tooltip : "NULL",
		     handler ? handler : "NULL",
		     accel_key_buffer, accel_mods);
}


/* This checks if the menu has any unfinished radio group GnomeUIInfo struct
   attached. If it does it is added to the final source buffer and freed. */
static void
glade_gnome_finish_radio_menu_item_group_source (GtkMenuShell * menu_shell,
						 GbWidgetWriteSourceData *data)
{
  GString *group_source_buffer;
  gchar *group_uiinfo_name;

  group_source_buffer = gtk_object_get_data (GTK_OBJECT (menu_shell),
					     "glade-radio-group-uiinfo");
  if (!group_source_buffer)
    return;

  group_uiinfo_name = gtk_object_get_data (GTK_OBJECT (menu_shell),
					   "glade-radio-group-uiinfo-name");
  g_return_if_fail (group_uiinfo_name != NULL);

  /* Finish the struct and add it onto the final source buffer. */
  g_string_append (group_source_buffer, "  GNOMEUIINFO_END\n};\n\n");
  g_string_append (data->source_buffers[GLADE_UIINFO],
		   group_source_buffer->str);

  /* Now free it and reset the pointer. */
  g_string_free (group_source_buffer, TRUE);
  gtk_object_set_data (GTK_OBJECT (menu_shell), "glade-radio-group-uiinfo",
		       NULL);

  g_free (group_uiinfo_name);
  gtk_object_set_data (GTK_OBJECT (menu_shell),
		       "glade-radio-group-uiinfo-name", NULL);
}


/* This returns TRUE if the given radio menu item is the first item in a
   radio group. */
static gboolean
glade_gnome_is_first_radio_menu_item_in_group (GtkMenuItem *menuitem)
{
  GList *children, *elem;
  GtkWidget *prev_item;
  gchar *group, *prev_group;

  children = GTK_MENU_SHELL (GTK_WIDGET (menuitem)->parent)->children;

  elem = g_list_find (children, menuitem);
  g_return_val_if_fail (elem != NULL, FALSE);

  /* If the item is the first item, it must start a new group. */
  if (elem == children)
    return TRUE;

  /* If the previous item is not a radio item, it must be in a new group. */
  prev_item = GTK_WIDGET (elem->prev->data);
  if (!GTK_IS_RADIO_MENU_ITEM (prev_item))
    return TRUE;

  /* If the item's group isn't the same as the previous item's, it must start
     a new group. */
  group = gtk_object_get_data (GTK_OBJECT (menuitem),
			       "GtkRadioMenuItem::group");
  if (group && group[0] == '\0')
    group = NULL;

  prev_group = gtk_object_get_data (GTK_OBJECT (prev_item),
				    "GtkRadioMenuItem::group");
  if (prev_group && prev_group[0] == '\0')
    prev_group = NULL;

  if (group)
    {
      if (!prev_group || strcmp (group, prev_group))
	return TRUE;
    }
  else
    {
      if (prev_group)
	return TRUE;
    }

  return FALSE;
}


/*************************************************************************
 * Utility Functions.
 *************************************************************************/

gboolean
glade_gnome_is_gnome_dialog_button (GtkWidget *widget)
{
  gchar *child_name;

  if (widget->parent)
    {
      child_name = gb_widget_get_child_name (widget->parent);
      if (child_name && !strcmp (child_name, "GnomeDialog:action_area"))
	return TRUE;
    }
  return FALSE;
}


/* If the given widget is a dock item inside a GnomeApp it returns the
   GnomeApp, otherwise it returns NULL. */
GnomeApp*
glade_gnome_is_app_dock_item (GtkWidget *widget)
{
  GtkWidget *dock, *app;

  if (!GNOME_IS_DOCK_ITEM (widget))
    return NULL;

  if (!widget->parent)
    return NULL;

  /* Floating items are direct children of a GnomeDock.
     Other items are children of a GnomeDockBand. */
  if (GNOME_IS_DOCK (widget->parent))
    {
      dock = widget->parent;
    }
  else
    {
      if (!GNOME_IS_DOCK_BAND (widget->parent))
	return NULL;
      dock = widget->parent->parent;
      if (!dock || !GNOME_IS_DOCK (dock))
	return NULL;
    }

  if (!dock->parent || !GTK_IS_VBOX (dock->parent))
    return NULL;

  app = dock->parent->parent;
  if (!app || !GNOME_IS_APP (app))
    return NULL;

  return GNOME_APP (app);
}


/* Tries to translate the text in Glade's domain, and if there is no
   translation try the gnome-libs domain. */
gchar*
glade_gnome_gettext		     (const gchar	      *text)
{
#ifdef ENABLE_NLS
  char *s;

  s = gettext (text);
  if (s == text)
    s = dgettext ("gnome-libs", text);

  return s;
#else
  return text;
#endif
}


#endif /* USE_GNOME */
