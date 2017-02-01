/* GnomeCard - a graphical contact manager.
 *
 * dialog.h: This file is part of GnomeCard.
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

#ifndef __GNOMECARD_DIALOG
#define __GNOMECARD_DIALOG

#include <gnome.h>

#include "deladdrlist.h"
#include "phonelist.h"
#include "emaillist.h"

typedef struct
{
	/* Identity */
	GtkWidget *fn;
	GtkWidget *given, *add, *fam, *pre, *suf;
	GtkWidget *bday;

	/* Organization */
	GtkWidget *title;
	GtkWidget *orgn;
/* not used       GtkWidget *role; */
/* not used       GtkWidget *org1, *org2, *org3, *org4; */

        /* Internet Info */
        EMailList *emaillist;

        /* Address */
        GtkWidget *addrtype[6];
        GtkWidget *street1, *street2, *city, *state, *country, *zip;
        gint      curaddr;
        DelAddrList *deladdrlist;

        /* Phone numbers */
	PhoneList *phonelist;
        
	/* Geographical */
	GtkWidget *tzh, *tzm;
	GtkWidget *gplon, *gplat;
	

	/* Explanatory */
	GtkWidget *categories;
	GtkWidget *comment;

	/* Security */
	GtkWidget *key, *keypgp;
	
	GList *l;
} GnomeCardEditor;


extern void gnomecard_find_card_call(GtkWidget *widget, gpointer data);
extern void gnomecard_about(GtkWidget *widget, gpointer data);

extern void gnomecard_edit(GList *node);
extern void gnomecard_edit_card(GtkWidget *widget, gpointer data);
extern void gnomecard_append(GtkWidget *widget, gpointer data);
extern void gnomecard_open(GtkWidget *widget, gpointer data);
extern void gnomecard_open_default (GtkWidget *widget, gpointer data);
extern gboolean gnomecard_append_file(char *fname);
extern gboolean gnomecard_open_file(char *fname);
extern gboolean gnomecard_save(void);
extern void gnomecard_save_as(GtkWidget *widget, gpointer data);
extern void gnomecard_setup(GtkWidget *widget, gpointer data);

#endif
