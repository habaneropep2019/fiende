/* mime.c - mime type support

   Copyright (C) 2000 Maurer IT Systemlösungen KEG

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Dietmar Maurer <dietmar@maurer-it.com>

*/

#include <config.h>
#include <string.h>

#include "efs_internal.h"

#define FIRST_USER_TYPE 1000000

/* for a list od resistrated mime types see:
 * ftp://ftp.isi.edu/in-notes/iana/assignments/media-types/media-types
 */

static gchar * std_mime_types[] = {
	"application/octet-stream",
	"application/postscript",
	"application/rtf",
	"application/pdf",
	"application/zip",
	"application/sgml",
	"application/pgp-encrypted",
	"application/pgp-signature",
	"application/pgp-keys",
	"application/xml",

	"audio/basic",
	"audio/midi",
	"audio/mpeg",
	"audio/x-aiff",
	"audio/x-wav",

	"image/bmp",
	"image/cgm",
	"image/g3fax",
	"image/gif",
	"image/jpeg",
	"image/png",
	"image/tiff",

	"text/css",
	"text/directory",
	"text/enriched",
	"text/html",
	"text/plain",
	"text/richtext",
	"text/rtf",
	"text/sgml",
	"text/tab-separated-values",
	"text/uri-list",
	"text/xml",
	"text/directory",
	"text/calendar",

	"video/mpeg",
	"video/quicktime",
	"video/x-msvideo",
	"video/x-sgi-movie",

	"model/iges",
	"model/vmrl",
	"model/mesh",

	NULL
};

static GHashTable * s2i_std_hash = NULL;
static GHashTable * i2s_std_hash = NULL;

static void
efs_stdtype_init ()
{
	guint32 i, j, k, l, code;
	gchar prefix[512], *p;
	gchar tmp_prefix[512];

	if (s2i_std_hash) return;

	s2i_std_hash = g_hash_table_new (g_str_hash, g_str_equal);
	i2s_std_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	i = 0;
	code = 0;
	j = 0;
	k = 0;

	strcpy (prefix, "none");
	while (std_mime_types[i]) {
		p = strchr (std_mime_types[i],'/');
		l = p - std_mime_types[i];
		strncpy (tmp_prefix, std_mime_types[i], l);
		tmp_prefix[l] = 0;
		if (strcmp (prefix, tmp_prefix)) {
			j ++;
			k = 0;
			strcpy (prefix, tmp_prefix);
		} 
		code = j*1000+k;

		g_hash_table_insert (s2i_std_hash, std_mime_types[i], 
				     (gpointer) code);

		g_hash_table_insert (i2s_std_hash, (gpointer) code, 
				     std_mime_types[i]);

		i++;
		k++;
	}
}

static void
efs_type_init (EFS *efs)
{
	guint32 code, i;
	gint32 l, br;
	gchar buf[1024], *p;

	if (!efs->typefd) return;
	if (efs_file_seek (efs->typefd, 0, EFS_SEEK_SET, &i)) return;
	
	efs->s2i_hash = g_hash_table_new (g_str_hash, g_str_equal);
	efs->i2s_hash = g_hash_table_new (g_direct_hash, g_direct_equal);

	i = FIRST_USER_TYPE;
	while (!efs_file_read (efs->typefd, &code, 4, &br)) {
		if (br != 4) return;
		code = GUINT32_FROM_LE (code);
		if (code != i) return;
		i++;
		if (efs_file_read (efs->typefd, &l, 4, &br)) return;
		if (br != 4) return;
		l = GINT32_FROM_LE (l);
		if (l >= sizeof(buf)) return;
		if (efs_file_read (efs->typefd, buf, l, &br)) return;
		if (br != l) return;
		buf[l] = 0;
		p = g_strdup (buf);

		g_hash_table_insert (efs->s2i_hash, p, (gpointer) code);
		g_hash_table_insert (efs->i2s_hash, (gpointer) code, p);
	}
}

static EFSResult
integer_type_lookup (EFS *efs, gchar *strtype, gboolean nostd, guint32 *type)
{
	if (!strcmp (strtype, EFS_MIME_DIR) || 
	    !strcmp (strtype, EFS_MIME_DATA)) {
		*type = 0;
		return EFS_ERR_OK;
	}

	if (!s2i_std_hash) efs_stdtype_init ();
	*type = (guint32) g_hash_table_lookup (s2i_std_hash, strtype);

	if (!(*type)) {
		if (!nostd) return EFS_ERR_NOTYPE;
		if (!efs->s2i_hash) efs_type_init (efs);
		if (!efs->s2i_hash) return EFS_ERR_INT;
		*type = (guint32) g_hash_table_lookup (efs->s2i_hash, strtype);
		if (!(*type)) return EFS_ERR_NOTYPE;
	}

	return EFS_ERR_OK;
}

static EFSResult
string_type_lookup (EFSNode *node, guint32 type, gchar **strtype)
{
	EFS *efs = node->efs;

	if (type == 0) {
		if (node->mode&EFS_DIR) *strtype = EFS_MIME_DIR; 
		else *strtype = EFS_MIME_DATA;
		return EFS_ERR_OK; 
	}

	if (!i2s_std_hash) efs_stdtype_init ();
	*strtype = g_hash_table_lookup (i2s_std_hash, (gpointer)type);
	if (!(*strtype)) {
		if (!efs->i2s_hash) efs_type_init (efs);
		if (!efs->i2s_hash) return EFS_ERR_INT;
		*strtype = g_hash_table_lookup (efs->i2s_hash, (gpointer)type);
		if (!(*strtype)) return EFS_ERR_NOTYPE;
	}

	return EFS_ERR_OK;
}

static guint32
efs_create_type (EFS *efs, gchar *strtype)
{
	guint32 code, code_le, cpos;
	gint32 l, l_le;
	gchar *p;

	if (!efs->s2i_hash | !efs->i2s_hash) return 0;
	if (!efs->typefd) return 0;

	code = FIRST_USER_TYPE + efs->tc++;

	p = g_strdup (strtype);
	g_hash_table_insert (efs->s2i_hash, p, (gpointer) code);
	g_hash_table_insert (efs->i2s_hash, (gpointer) code, p);
	
	if (efs_file_seek (efs->typefd, 0, EFS_SEEK_END, &cpos)) return 0;
	code_le = GUINT32_TO_LE (code);
	if (efs_file_write (efs->typefd, &code_le, 4)) return 0;
	l = strlen(strtype);
	l_le = GINT32_TO_LE (l);
	if (efs_file_write (efs->typefd, &l_le, 4)) return 0;
	if (efs_file_write (efs->typefd, strtype, l)) return 0;

	return code;
}

EFSResult  
efs_strtype_set (EFSNode *node, gchar *strtype)
{
	guint32 type;

	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (strtype != NULL, EFS_ERR_INVAL);

	if (!integer_type_lookup (node->efs, strtype, FALSE, &type))
		return efs_type_set (node, type);

	if (!integer_type_lookup (node->efs, strtype, TRUE, &type))
		return efs_type_set (node, type);

	if (!(type = efs_create_type (node->efs, strtype))) return EFS_ERR_INT;
	
	return efs_type_set (node, type);	
}

EFSResult    
efs_strtype_get (EFSNode *node, gchar **strtype)
{
	EFSResult res;
	guint32 type;

	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (strtype != NULL, EFS_ERR_INVAL);

	if ((res = efs_type_get (node, &type))) return res;
	if ((res = string_type_lookup(node, type, strtype))) return res;

	return EFS_ERR_OK;
}

EFSResult 
efs_type_lookup (EFSNode *node, gchar *strtype, guint32 *type)
{
	EFSResult res;

	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (strtype != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (type != NULL, EFS_ERR_INVAL);

	if ((res = integer_type_lookup (node->efs, strtype, TRUE, type))) 
		return res;

	return EFS_ERR_OK;
}

EFSResult
efs_strtype_lookup (EFSNode *node, guint32 type, gchar **strtype)
{
	EFSResult res;

	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (strtype != NULL, EFS_ERR_INVAL);

	if ((res = string_type_lookup(node, type, strtype))) return res;

	return EFS_ERR_OK;
}

