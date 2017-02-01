/* GnomeCard - a graphical contact manager.
 *
 * columnhdrs.h: This file is part of GnomeCard.
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

#ifndef GNOMECARD_COLEDIT_H
#define GNOMECARD_COLEDIT_H

#include "card.h"

typedef enum {
    COLTYPE_END = -1,
    COLTYPE_FULLNAME = 0,
    COLTYPE_CARDNAME,
    COLTYPE_FIRSTNAME,
    COLTYPE_MIDDLENAME,
    COLTYPE_LASTNAME,
    COLTYPE_PREFIX,
    COLTYPE_SUFFIX,
    COLTYPE_ORG,
    COLTYPE_TITLE,
    COLTYPE_EMAIL,
    COLTYPE_URL,
    COLTYPE_PHONE
} ColumnType;

typedef struct {
    gchar      *colname;
    gchar      *coltypename;
    ColumnType  coltype;
} ColumnHeader;


gchar *getColumnNameFromType(ColumnType type);
gchar *getColumnTypeNameFromType(ColumnType type);
ColumnType getColumnTypeFromName(gchar *name);
ColumnType getColumnTypeFromTypeName(gchar *typename);
ColumnHeader *getColumnHdrFromType(ColumnType type);
gint numColumnHeaders(GList *cols);
GList *buildColumnHeaders(ColumnType *cols);
gchar *getValFromColumnHdr(Card *crd, ColumnHeader *hdr);
GList *getAllColumnHdrs(void);

#endif
