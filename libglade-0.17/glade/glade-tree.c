/* -*- Mode: C; c-basic-offset: 8 -*-
 * libglade - a library for building interfaces from XML files at runtime
 * Copyright (C) 1998-2001  James Henstridge <james@daa.com.au>
 *
 * glade-tree.c: functions for managing the glade parse tree cache.
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

/*
 * a few functions to manage the cache of XML trees.  We bother with this, so
 * that multiple instantiations of a window will not need multiple parsings
 * of the associated XML file
 */
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "glade/glade-private.h"

static GHashTable *tree_hash = NULL;

GladeWidgetTree *
glade_tree_get(const char *filename)
{
	GladeWidgetTree *tree;
	gchar *orig_key;

	if (!tree_hash)
		tree_hash = g_hash_table_new(g_str_hash, g_str_equal);

	if (g_hash_table_lookup_extended(tree_hash, filename,
					 (gpointer *)&orig_key,
					 (gpointer *)&tree)) {
		struct stat statbuf;

		if (stat(filename, &statbuf) >= 0 &&
		    statbuf.st_mtime > tree->mtime) {
			GladeWidgetTree *newtree =
				glade_widget_tree_parse_file(filename);
			if (!newtree)
				g_warning("Could not parse file");
			else {
				glade_widget_tree_unref(tree);
				tree = newtree;
				g_hash_table_insert(tree_hash, orig_key, tree);
			}
		}
	} else {
		tree = glade_widget_tree_parse_file(filename);
		if (!tree) {
			g_warning("Could not parse file");
			return NULL;
		}
		g_hash_table_insert(tree_hash, g_strdup(filename), tree);
	}

	return glade_widget_tree_ref(tree);
}

