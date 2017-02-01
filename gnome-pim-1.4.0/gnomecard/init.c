/* GnomeCard - a graphical contact manager.
 *
 * init.c: This file is part of GnomeCard.
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <gnome.h>

#include "gnomecard.h"
#include "images.h"
#include "init.h"
#include "pix.h"

static GnomeStockPixmapEntry *gnomecard_pentry_new(gchar **xpm_data, gint size)
{
	GnomeStockPixmapEntry *pentry;
	
	pentry = g_malloc(sizeof(GnomeStockPixmapEntry));
	pentry->data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	pentry->data.width = size;
	pentry->data.height = size;
	pentry->data.label = NULL;
	pentry->data.xpm_data = xpm_data;
	
	return pentry;
}

extern void gnomecard_init_stock(void)
{
	GnomeStockPixmapEntry *pentry;
	gchar **xpms[] = { cardnew_xpm, cardedit_xpm, first_xpm, last_xpm, cardfind_xpm,
		                 addr_xpm, phone_xpm, email_xpm, NULL };
	gchar *names[] = { "New", "Edit", "First", "Last", "Find", "Addr", "Phone", "EMail" };
	gchar stockname[22];
	int i;
	
	for (i = 0; xpms[i]; i++) {
		snprintf(stockname, 22, "GnomeCard%s", names[i]);
		pentry = gnomecard_pentry_new(xpms[i], 24);
		gnome_stock_pixmap_register(stockname, GNOME_STOCK_PIXMAP_REGULAR, pentry);

		snprintf(stockname, 22, "GnomeCard%sMenu", names[i]);
		pentry = gnomecard_pentry_new(xpms[i], 16);
		gnome_stock_pixmap_register(stockname, GNOME_STOCK_PIXMAP_REGULAR, pentry);
	}
}

extern void gnomecard_init_pixes(void)
{
	null_pix = g_malloc(sizeof(pix));
	null_pix->width = 0;
	null_pix->height = 0;
	null_pix->pixmap = NULL;
	null_pix->mask = NULL;
	
	crd_pix = pix_new(cardnew_xpm);
	ident_pix = pix_new(ident_xpm);
	geo_pix = pix_new(geo_xpm);
	sec_pix = pix_new(sec_xpm);
	phone_pix = pix_new(phone_xpm);
	email_pix = pix_new(email_xpm);
	addr_pix = pix_new(addr_xpm);
	expl_pix = pix_new(expl_xpm);
	org_pix = pix_new(org_xpm);
}

extern void gnomecard_init_defaults(void)
{
	char *fname;
	
	gnomecard_find_sens = gnome_config_get_bool("/GnomeCard/find/sens=False");
	gnomecard_find_back = gnome_config_get_bool("/GnomeCard/find/back=False");
	gnomecard_find_str = gnome_config_get_string("/GnomeCard/find/str=");
	gnomecard_fname = gnome_config_get_string("/GnomeCard/file/open=");
}

