/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *   guname: System information dialog.
 *
 *   Copyright (C) 1998 Havoc Pennington <hp@pobox.com> except marquee code.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <gnome.h>

#include "list.h"


/* Want a function not a macro, so a and b are calculated only once */
static gint max(gint a, gint b)
{
  if ( a > b )
    return a;
  else return b;
}

void fill_clist(GtkCList * list, 
                const gchar ** col1_items, const gchar ** col2_items, 
                gint numitems,  gint * width, gint * height)
{
  const gchar * row[2];
  int i;
  gint col_zero_width, col_one_width;
  GdkFont * font;
  GtkRequisition req;
  
  font = gtk_widget_get_style(GTK_WIDGET(list))->font;

  i = 0; col_zero_width = 0; col_one_width = 0;

  while ( i < numitems ) {

    /* No info is more likely than no description, but be paranoid. */
    if ( (col2_items[i] == NULL) || (col1_items[i] == NULL) ) {
      /* Don't have this information. */
      ++i;
      continue; 
    }

    row[0] = _(col1_items[i]);
    row[1] = _(col2_items[i]);
    gtk_clist_append(list, (gchar **)row);

    /* If the string is longer than any previous ones,
       increase the column width */
    
    col_zero_width = max ( gdk_string_width(font, row[0]), col_zero_width );
    col_one_width =  max ( gdk_string_width(font, row[1]), col_one_width );

    ++i;
  }

  /* The first column is a little wider than the largest string, so 
     it's not too close to the second column. */
  gtk_clist_set_column_width(list, 0, col_zero_width + 10);
  gtk_clist_set_column_width(list, 1, col_one_width);
  
  *width = col_zero_width+col_one_width + 30;
  gtk_widget_size_request(GTK_WIDGET(list), &req);
  *height = req.height + 30;
}

#ifdef HAVE_LIBGTOP_SYSINFO

void fill_clist_from_glibtop_entry (GtkCList * list,
                                    glibtop_entry * entry,
                                    gint * width, gint * height)
{
  const gchar * row[2];
  int i;
  gint col_zero_width, col_one_width;
  GdkFont * font;
  GtkRequisition req;

  font = gtk_widget_get_style(GTK_WIDGET(list))->font;

  i = 0; col_zero_width = 0; col_one_width = 0;

  while ( i < entry->labels->len ) {

    const gchar * value = NULL, * description = NULL;

    value = g_hash_table_lookup
      (entry->values, entry->labels->pdata [i]);

    if (entry->descriptions)
      description = g_hash_table_lookup
        (entry->descriptions, entry->labels->pdata [i]);

    if (value == NULL) {
      ++i;
      continue;
    }

    row[0] = entry->labels->pdata [i];
    row[1] = description ? description : value;
    gtk_clist_append(list, (gchar **)row);

    /* If the string is longer than any previous ones,
       increase the column width */
    
    col_zero_width = max ( gdk_string_width(font, row[0]), col_zero_width );
    col_one_width =  max ( gdk_string_width(font, row[1]), col_one_width );

    ++i;
  }

  /* The first column is a little wider than the largest string, so 
     it's not too close to the second column. */
  gtk_clist_set_column_width(list, 0, col_zero_width + 10);
  gtk_clist_set_column_width(list, 1, col_one_width);
  
  *width = col_zero_width+col_one_width + 30;
  gtk_widget_size_request(GTK_WIDGET(list), &req);
  *height = req.height + 30;
}

#endif

GtkWidget * create_clist(const gchar * titles[])
{
  GtkCList * list;

  list = GTK_CLIST(gtk_clist_new_with_titles(2, (gchar **)titles));

  gtk_clist_set_shadow_type(list, GTK_SHADOW_IN);

  /* Fixme, eventually you might could select an item 
     for cut and paste, or some other effect. */
  gtk_clist_set_selection_mode(list, GTK_SELECTION_BROWSE);
  gtk_clist_column_titles_passive(list);

  return GTK_WIDGET(list);
}
