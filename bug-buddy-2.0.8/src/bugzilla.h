/* bug-buddy bug submitting program
 *
 * Copyright (C) 2001 Jacob Berkman
 * Copyright 2001 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@bug-buddy.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __BUGZILLA_H__
#define __BUGZILLA_H__

#include <libgnomevfs/gnome-vfs-types.h>
#include <libxml/parser.h>

typedef struct _BugzillaBTS BugzillaBTS;

typedef void (*XMLFunc) (BugzillaBTS *bts, xmlDoc *doc);

typedef enum {
	BUGZILLA_SUBMIT_DEBIAN,
	BUGZILLA_SUBMIT_FREITAG
} BugzillaSubmitType;

typedef struct {
	char *system_path;
	char *cache_path;

	GnomeVFSURI *source_uri;
	GnomeVFSURI *dest_uri;

	XMLFunc xml_func;

	gboolean read_from_cache;
	gboolean download;
	gboolean done;
} BugzillaXMLFile;

struct _BugzillaBTS {
	char      *name;
	char      *subdir;
	char      *email;

	BugzillaXMLFile *products_xml;
	BugzillaXMLFile *config_xml;

	BugzillaSubmitType submit_type;
	char *icon;

	GdkPixmap *pixmap;
	GdkBitmap *mask;

	char *severity_node;
	char *severity_item;
	char *severity_header;
	
	/* products.xml */
	GSList    *products;

	/* config.xml */
	GSList    *severities;
	GSList    *opsys;
};

typedef struct {
	BugzillaBTS *bts;
	char        *name;
	char        *description;
	GSList      *components;
} BugzillaProduct;

typedef struct {
	BugzillaProduct *product;
	char            *name;
	char            *description;
} BugzillaComponent;

void load_bugzillas (void);
void load_bugzilla_xml (void);

void bugzilla_bts_add_products_to_clist (BugzillaBTS *bts);
void bugzilla_product_add_components_to_clist (BugzillaProduct *product);

char *generate_email_text (void);

#endif /* __BUGZILLA_H__ */
