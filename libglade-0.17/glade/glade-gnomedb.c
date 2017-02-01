/* -*- Mode: C; c-basic-offset: 8 -*-
 * libglade - a library for building interfaces from XML files at runtime
 * Copyright (C) 1998-2001  James Henstridge <james@daa.com.au>
 *
 * glade-gnomedb.c: support for Gnome-db widgets.
 * Copyright (C) 2001 Advanced Software Production Line, S.L. <aspl@wanadoo.es>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <glade/glade-build.h>
#include <glade/glade-private.h>
#include <gnome.h>
#include <bonobo.h>
#include <gnome-db.h>

#ifndef ENABLE_NLS
/* a slight optimisation when gettext is off */
#define glade_xml_gettext(xml, msgid) (msgid)
#endif
#undef _
#define _(msgid) (glade_xml_gettext(xml, msgid))

static GtkWidget *
dblogin_new(GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
     
	wid = gnome_db_login_new (NULL, NULL, NULL);
     
	return wid;
}

static GtkWidget *
dblogindlg_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid = NULL;
	GtkWidget *wid2 = NULL;
     
	wid2 = gnome_db_login_new (NULL,NULL,NULL);
          
	wid = gnome_db_logindlg_new (GNOME_DB_LOGIN (wid2), NULL);
     
	return wid;
}

static GtkWidget *
dbdsnconfig_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid = NULL;
     
	wid = gnome_db_dsn_config_new (NULL);
     
	return (wid);
}


static GtkWidget *
dbbrowser_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid = NULL;
     
	wid = gnome_db_browser_new (NULL);
        
               
	return wid;
}

static GtkWidget *
dbgrid_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_grid_new (NULL);
     
               
	return wid;
}

static GtkWidget *
dbdataset_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_dataset_new (NULL);
     
	return wid;
}

static GtkWidget *
dbcombo_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_combo_new (NULL,0);
     
	return wid;
}

static GtkWidget *
dblist_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_list_new (NULL,0);
     
	return wid;
}

static GtkWidget *
dbreport_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_report_new ();
     
	return wid;
}

static GtkWidget *
dbiconlist_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_icon_list_new (NULL, 0);
     
	return (wid);
}

static GtkWidget *
dberror_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_error_new (NULL);
     
	return (wid);
}

static GtkWidget *
dberrordlg_new (GladeXML *xml, GladeWidgetInfo *info)
{
	GtkWidget *wid;
       
	wid = gnome_db_errordlg_new (NULL, NULL);
     
	return (wid);
}

static const GladeWidgetBuildData widget_data [] = {
	{ "GnomeDbLogin",       dblogin_new,        NULL},
	{ "GnomeDbLoginDlg",    dblogindlg_new,     NULL},
	{ "GnomeDbDsnConfig",   dbdsnconfig_new,    NULL},
	{ "GnomeDbBrowser",     dbbrowser_new,      NULL},
	{ "GnomeDbGrid",        dbgrid_new,         NULL},
	{ "GnomeDbDataset",     dbdataset_new,      NULL},
	{ "GnomeDbCombo",       dbcombo_new,        NULL},
	{ "GnomeDbList",        dblist_new,         NULL},
	{ "GnomeDbReport",      dbreport_new,       NULL},
	{ "GnomeDbIconList",    dbiconlist_new,     NULL},
	{ "GnomeDbError",       dberror_new,        NULL},
	{ "GnomeDbErrorDlg",    dberrordlg_new,     NULL},
	{ NULL, NULL, NULL }
};

void
glade_init_gnome_db_widgets(void)
{
	glade_register_widgets(widget_data);
}

/**
 * glade_gnome_db_init
 *
 * This function performs initialisation of glade, similar to what glade_init
 * does (in fact it calls glade_init for you).  The difference is that it
 * also initialises the GNOME widget building routines.
 *
 * As well as calling this initialisation function, GNOME programs should
 * also link with the libglade-gnome library, which contains all the
 * GNOME libglade stuff.
 */
void
glade_gnome_db_init(void)
{
	static gboolean initialised = FALSE;

	if (initialised) return;
	initialised = TRUE;
	glade_gnome_init();
	glade_init_gnome_db_widgets();
}
