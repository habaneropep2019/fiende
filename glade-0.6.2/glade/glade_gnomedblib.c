/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998-1999  Damon Chaplin
 *  Copyright (C) 2001  Carlos Perelló Marín <carlos@gnome-db.org>
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

#include "gladeconfig.h"

#ifdef USE_GNOME_DB

#include "glade.h"
#include "gbwidget.h"

/* I've commented this out to avoid warnings. */
/*gchar *libname = "GNOME 1.0";*/

GbWidget *gb_gnome_db_browser_init ();
GbWidget *gb_gnome_db_combo_init ();
GbWidget *gb_gnome_db_error_init ();
GbWidget *gb_gnome_db_errordlg_init ();
GbWidget *gb_gnome_db_grid_init ();
GbWidget *gb_gnome_db_list_init ();
GbWidget *gb_gnome_db_login_init ();
GbWidget *gb_gnome_db_logindlg_init ();
GbWidget *gb_gnome_db_report_init ();
GbWidget *gb_gnome_db_iconlist_init ();
GbWidget *gb_gnome_db_dsnconfig_init ();
GbWidget *gb_gnome_db_dataset_init ();

static GladeWidgetInitData gnome_db[] = {
	{ "GnomeDbLogin",       gb_gnome_db_login_init },
        { "GnomeDbLoginDlg",    gb_gnome_db_logindlg_init },
        { "GnomeDbDsnConfig",   gb_gnome_db_dsnconfig_init },
        { "GnomeDbBrowser",     gb_gnome_db_browser_init },
        { "GnomeDbGrid",        gb_gnome_db_grid_init },
        { "GnomeDbDataset",     gb_gnome_db_dataset_init },
        { "GnomeDbCombo",       gb_gnome_db_combo_init },
        { "GnomeDbList",        gb_gnome_db_list_init },
        { "GnomeDbReport",      gb_gnome_db_report_init },
        { "GnomeDbIconList",    gb_gnome_db_iconlist_init },
        { "GnomeDbError",       gb_gnome_db_error_init },
        { "GnomeDbErrorDlg",    gb_gnome_db_errordlg_init },  
	{ NULL, NULL }  
};


static GladePaletteSectionData sections[] =
{
  { "Gnome DB", gnome_db },
  { NULL, NULL }
};


GladePaletteSectionData *get_gnome_db_widgets()
{
	return sections;
}

#endif
