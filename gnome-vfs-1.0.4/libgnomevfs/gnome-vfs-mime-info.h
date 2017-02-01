/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* gnome-vfs-mime-info.h
 *
 * Copyright (C) 1998 Miguel de Icaza
 * Copyright (C) 2000 Eazel, Inc
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA. 
 */

#ifndef GNOME_VFS_MIME_INFO_H
#define GNOME_VFS_MIME_INFO_H

#include <libgnomevfs/gnome-vfs-result.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
	
	/* functions to freeze/thaw the internal hash tables to 
	   avoid writing them back to disk everytime you modify 
	   them through the _set_ functions */
void             gnome_vfs_mime_freeze                          (void);
void             gnome_vfs_mime_thaw                            (void);

	/* forces a reload of the config files */
void       	 gnome_vfs_mime_info_reload   	  	 	(void);


gboolean	 gnome_vfs_mime_type_is_known			(const char *mime_type);

	/* functions which access to the .keys files */
const char 	*gnome_vfs_mime_get_value        		(const char *mime_type,
								 const char *key);
GnomeVFSResult   gnome_vfs_mime_set_value                       (const char *mime_type, 
								 const char *key, 
								 const char *value);
GList      	*gnome_vfs_mime_get_key_list      		(const char *mime_type);
void             gnome_vfs_mime_keys_list_free                  (GList *mime_ype_list);

	/* functions to access the .mime files */
GList 	   	*gnome_vfs_mime_get_extensions_list 		(const char *mime_type);
void	   	 gnome_vfs_mime_extensions_list_free 		(GList      *list);
char 	   	*gnome_vfs_mime_get_extensions_string 	 	(const char *mime_type);
char 	   	*gnome_vfs_mime_get_extensions_pretty_string    (const char *mime_type);
GList 	        *gnome_vfs_get_registered_mime_types 	 	(void);
void	         gnome_vfs_mime_registered_mime_type_list_free 	(GList      *list);
GnomeVFSResult	 gnome_vfs_mime_set_registered_type_key 	(const char *mime_type, 
							  	 const char *key, 
							  	 const char *data);
GnomeVFSResult   gnome_vfs_mime_set_extensions_list             (const char *mime_type, 
								 const char *extension_list);
void             gnome_vfs_mime_registered_mime_type_delete     (const char *mime_type);
void             gnome_vfs_mime_reset                           (void);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* GNOME_VFS_MIME_INFO_H */














