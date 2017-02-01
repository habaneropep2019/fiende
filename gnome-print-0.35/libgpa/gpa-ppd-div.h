/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 2000 Jose M Celorio
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
 *
 * Authors :
 *  Chema Celorio <chema@celorio.com>
 *
 */
#ifndef __GNOME_PRINT_ADMIN_PPD_DIV_H__
#define __GNOME_PRINT_ADMIN_PPD_DIV_H__

BEGIN_GNOME_DECLS

extern gboolean debug_turned_on;

typedef struct _GpaPpdDiv       GpaPpdDiv;
typedef struct _GpaPpdSimpleDiv GpaPpdSimpleDiv;

GpaPpdSimpleDiv * gpa_ppd_simple_div_new   (gint size,
					    gchar **labels);
void              gpa_ppd_simple_div_add   (GpaPpdSimpleDiv *ppd_div,
					    gchar **tags,
					    const gchar *code);
gboolean          gpa_ppd_simple_div_solve (GpaPpdSimpleDiv *ppd_div,
					    const gchar *start_tag,
					    const gchar *end_tag,
					    gchar ** cleaned_code);

GpaPpdDiv * gpa_ppd_div_new (void);
void        gpa_ppd_div_add (GpaPpdDiv *ppd_div, const gchar *code, GHashTable *hash);
gboolean    gpa_ppd_div_solve (GpaPpdDiv *ppd_div, gchar **set_code_);


END_GNOME_DECLS

#endif /* __GNOME_PRINT_ADMIN_PPD_DIV_H__ */
