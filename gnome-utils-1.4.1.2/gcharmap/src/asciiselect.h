/*
 *  Gnome Character Map
 *  asciiselect.h - The ASCII character selector dialog
 *
 *  Copyright (c) 2000 Hongli Lai
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

#ifndef _ASCII_SELECT_H_
#define _ASCII_SELECT_H_

#include <gtk/gtk.h>


#define ASCII_SELECT_TYPE             (ascii_select_get_type ())
#define ASCII_SELECT(obj)             GTK_CHECK_CAST (obj, ASCII_SELECT_TYPE, AsciiSelect)
#define ASCII_SELECT_CLASS(klass)     GTK_CHECK_CLASS_CAST ((klass), ASCII_SELECT_TYPE, AsciiSelectClass)
#define ASCII_IS_SELECT(obj)          GTK_CHECK_TYPE (obj, ASCII_SELECT_TYPE)
#define ASCII_IS_SELECT_CLASS(klass)  GTK_CHECK_TYPE ((klass), ASCII_SELECT_TYPE)

typedef struct _AsciiSelect
{
    GtkObject parent_struct;
    GtkWidget *window;
} AsciiSelect;

typedef struct _AsciiSelectClass
{
    GtkObjectClass parent_klass;
} AsciiSelectClass;


guint ascii_select_get_type (void);
AsciiSelect *ascii_select_new (void);
void ascii_select_destroy (AsciiSelect *obj);


#endif /* _ASCII_SELECT_H_ */

