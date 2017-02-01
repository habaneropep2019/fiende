/*
 *  Gnome Character Map
 *  menus.c - Menus for the main window
 *
 *  Copyright (C) Hongli Lai
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
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

#ifndef _MENUS_C_
#define _MENUS_C_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "menus.h"
#include "callbacks.h"
#include "gcharmap-map-24.xpm"
#include "gcharmap-map-16.xpm"


GnomeUIInfo file_menu[] =
{
    GNOMEUIINFO_MENU_EXIT_ITEM (cb_exit_click, NULL),
    GNOMEUIINFO_END
};

GnomeUIInfo edit_menu[] =
{
    GNOMEUIINFO_MENU_CUT_ITEM (cb_cut_click, NULL),
    GNOMEUIINFO_MENU_COPY_ITEM (cb_copy_click, NULL),
    GNOMEUIINFO_MENU_PASTE_ITEM (cb_paste_click, NULL),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_CLEAR_ITEM (cb_clear_click, NULL),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_SELECT_ALL_ITEM (cb_select_all_click, NULL),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_DATA(N_("Insert Character..."),
      N_("Insert character(s) by choosing character codes."), cb_insert_char_click,
      NULL, gcharmap_map_16_xpm),
    GNOMEUIINFO_END
};

GnomeUIInfo view_menu[] =
{
    GNOMEUIINFO_TOGGLEITEM(N_("_Action Toolbar"), N_("View or hide the action toolbar"),
      cb_toggle_actionbar, NULL),
    GNOMEUIINFO_TOGGLEITEM(N_("_Text Toolbar"), N_("View or hide the text toolbar"),
      cb_toggle_textbar, NULL),
    GNOMEUIINFO_TOGGLEITEM(N_("_Statusbar"), N_("View or hide the statusbar"),
      cb_toggle_statusbar, NULL),
    GNOMEUIINFO_END
};

GnomeUIInfo settings_menu[] =
{
    GNOMEUIINFO_ITEM_STOCK(N_("Character Table's Font..."), N_("Set the character "
      "table's font."), cb_set_chartable_font, GNOME_STOCK_MENU_FONT),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_TOGGLEITEM(N_("_Insert at end"), N_("Insert every selected character "
      "at the end of the text entry"), cb_set_insert_at_end, NULL),
    GNOMEUIINFO_TOGGLEITEM(N_("_Focusable Buttons"), N_("Set the buttons to be "
      "focusable or not"), cb_set_button_focusable, NULL),
    GNOMEUIINFO_END
};

GnomeUIInfo help_menu[] =
{
    GNOMEUIINFO_HELP ("gcharmap"),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_MENU_ABOUT_ITEM (cb_about_click, NULL),
    GNOMEUIINFO_END
};

GnomeUIInfo menubar[] =
{
    GNOMEUIINFO_MENU_FILE_TREE(file_menu),
    GNOMEUIINFO_MENU_EDIT_TREE(edit_menu),
    GNOMEUIINFO_MENU_VIEW_TREE(view_menu),
    GNOMEUIINFO_MENU_SETTINGS_TREE(settings_menu),
    GNOMEUIINFO_MENU_HELP_TREE(help_menu),
    GNOMEUIINFO_END
};

GnomeUIInfo toolbar[] = {
    GNOMEUIINFO_ITEM(N_("Insert"), N_("Insert character(s) by choosing character codes"),
      cb_insert_char_click, &gcharmap_map_24_xpm),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK(N_("Cut"), N_("Cut the selection"),
      cb_cut_click, GNOME_STOCK_PIXMAP_CUT),
    GNOMEUIINFO_ITEM_STOCK(N_("Copy"), N_("Copy the selection"),
      cb_copy_click, GNOME_STOCK_PIXMAP_COPY),
    GNOMEUIINFO_ITEM_STOCK(N_("Paste"), N_("Paste the clipboard"),
      cb_paste_click, GNOME_STOCK_PIXMAP_PASTE),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK(N_("Help"), N_("Show Gnome Character Map's manual"),
      cb_help_click, GNOME_STOCK_PIXMAP_HELP),
    GNOMEUIINFO_ITEM_STOCK(N_("About"), N_("About this application"),
      cb_about_click, GNOME_STOCK_PIXMAP_ABOUT),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK(N_("Exit"), N_("Exit the program"),
      cb_exit_click, GNOME_STOCK_PIXMAP_EXIT),
    GNOMEUIINFO_END
};


#endif /* _MENUS_C_ */
