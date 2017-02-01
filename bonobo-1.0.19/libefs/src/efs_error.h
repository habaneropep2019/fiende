/* efs_error.h - EFS error code definitions

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


#ifndef _EFS_ERROR_H_
#define _EFS_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif
	
typedef enum {
	EFS_ERR_INT = -1,
	EFS_ERR_OK = 0,        
	EFS_ERR_INVAL,
	EFS_ERR_EXISTS,
	EFS_ERR_NOTFILE,
	EFS_ERR_NOTDIR,
	EFS_ERR_NOTEMPTY,
	EFS_ERR_ERRNO,
	EFS_ERR_FORMAT,
	EFS_ERR_INVPASS,
	EFS_ERR_LOCKED,
	EFS_ERR_NOENT,
	EFS_ERR_NODRIVER,
	EFS_ERR_PERM,
	EFS_ERR_NOSEEK,
	EFS_ERR_NOTYPE,
	EFS_ERR_LAST        
} EFSResult;

#ifdef __cplusplus
}
#endif

#endif /* _EFS_ERROR_H_ */







