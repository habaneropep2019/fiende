/* efs.c - efs file access functions

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <sys/file.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#include "efs_internal.h"
#include "ib1_driver.h"

gint efs_errno;

static EFSDriver *efs_driver_list[] = { 
	&efs_driver_ib1, 
	&efs_driver_ib1_enc, 
	NULL 
};

static gboolean
free_hash_node (gpointer key, gpointer value, gpointer user_data)
{
	if (key) g_free (key);

	return TRUE;
}

static void
efs_typehash_clear (EFS *efs)
{

	if (efs->s2i_hash) {
		g_hash_table_foreach_remove (efs->s2i_hash, free_hash_node,
					     NULL);
		g_hash_table_destroy (efs->s2i_hash);
		efs->s2i_hash = NULL;
	}

	if (efs->i2s_hash) {
		g_hash_table_destroy (efs->i2s_hash);
		efs->i2s_hash = NULL;
	}
}

gchar*
efs_strerror(EFSResult errnum)
{
	switch (errnum) {
	case EFS_ERR_NOSEEK:       return "seek not possible";
	case EFS_ERR_PERM:         return "operation not permitted";
	case EFS_ERR_NODRIVER:     return "no driver found";
	case EFS_ERR_NOENT:        return "no such file or directory";
	case EFS_ERR_LOCKED:       return "file is locked";
	case EFS_ERR_INVPASS:      return "invalid password";
	case EFS_ERR_FORMAT:       return "wrong header format";
	case EFS_ERR_ERRNO:        return "unix system IO error";
	case EFS_ERR_NOTFILE:      return "not a file";
	case EFS_ERR_NOTDIR:       return "not a directory";
	case EFS_ERR_NOTEMPTY:     return "directory not empty";
	case EFS_ERR_EXISTS:       return "file exists"; 
	case EFS_ERR_INVAL:        return "invalid argument";
	case EFS_ERR_OK:           return "everithing is ok"; 
	case EFS_ERR_INT:          return "internal error";
	case EFS_ERR_NOTYPE:       return "no such type";
	default:                   return "unknown error";
	}
}

EFSDriver*
efs_find_driver (gchar *drivername)
{
	EFSDriver **h;
	gint i;

	h = efs_driver_list;

	i = 0; 
	while (h[i] && g_strcasecmp(drivername, h[i]->drivername)) i++;
			     
	return h[i];
}

gboolean     
efs_passwd_compare (EFSHeader *head, gchar *passwd)
{
	gchar *epw;

	epw = crypt (passwd, head->epw);
	if (strncmp(epw, head->epw, sizeof (head->epw))) return FALSE;
	return TRUE;
}

void         
efs_passwd_set (EFSHeader *head, gchar *passwd)
{
	gchar *epw;
	gint i;
	guint32 seed[2];
	gchar salt[] = "$1$........";
	const char *const seedchars = 
		"./0123456789ABCDEFGHIJKLMNOPQRST"
		"UVWXYZabcdefghijklmnopqrstuvwxyz";

	/* copied from the crypt manual */
	/* Generate a (not very) random seed. */
	seed[0] = time(NULL);
	seed[1] = getpid() ^ (seed[0] >> 14 & 0x30000);
	/* Turn it into printable characters from `seedchars'. */
	for (i = 0; i < 8; i++)
		salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];
 
	epw = crypt (passwd, salt);
	strncpy (head->epw, epw, sizeof (head->epw));
}

EFSResult
efs_open_full (EFSDir **dir, const char *path, gint flags, gint mode, 
	       EFSPassFunc pass_func, gchar *passwd)
{
	EFSResult res;
	EFSHeader *head;
	EFSDriver *driver;
	gint fd, i, j;
	struct stat sb;
	gchar buf[512], drivername[1024], lockname[1024];
	const gchar *filename;
	gchar *pw = passwd;

	g_return_val_if_fail (dir != NULL, EFS_ERR_INVAL);
	*dir = NULL;
	g_return_val_if_fail (path != NULL, EFS_ERR_INVAL);
	
	if (flags&EFS_CREATE) flags |= EFS_RDWR;
	if (flags&EFS_WRITE) flags |= EFS_READ;

	/* extract driver and filename */
	i = 0; while ((i<1024)&&path[i]&&isalnum((int)path[i])) i++;
	filename = path;
	drivername[0] = 0;

	if (path[i]==':') {
		j=i; while (path[j]==':') j++;
		strncpy (drivername, path, i);
		drivername[i] = 0;
		filename = &path[j];
	}

	strcpy (lockname, filename);
	strcpy (&lockname[strlen(filename)], ".WRITELOCK");

	driver = NULL;
	if (!(stat (filename, &sb))) {       /* exists */
		if ((flags&EFS_CREATE)&&(flags&EFS_EXCL)) 
			return EFS_ERR_EXISTS;
		if (!S_ISREG(sb.st_mode)) return EFS_ERR_NOTFILE;
	        
		/* detect driver */
			
		if (!(fd = open(filename, O_RDONLY))) return EFS_ERR_ERRNO;
		if (!((read (fd, buf, 512))==512)) { 
			close(fd);
			return EFS_ERR_ERRNO;
		}
		close(fd);

		head = (EFSHeader *)buf;
		if (strncmp(head->efs_id, EFS_FILE_ID, 4)) 
			return EFS_ERR_FORMAT;

		strncpy (drivername, head->drivername, 12);
		drivername[12] = 0;			
		if (!(driver = efs_find_driver(drivername)))
			return EFS_ERR_NODRIVER;

		if (head->protected & !pw) {
			if (!pass_func) return EFS_ERR_INVPASS;
			if (!(pw = pass_func (filename, FALSE)))
				return EFS_ERR_INVPASS;
		}

		if (head->protected && !driver->encrypted &&
		    !efs_passwd_compare(head, pw)) return EFS_ERR_INVPASS;

		if ((flags&EFS_WRITE)&& (res = efs_lock_create(lockname))) 
			return res;

		res = driver->sops->open (dir, driver, filename, flags, pw);
		
	} else { /* file does not exist */		
		if (!(flags&EFS_CREATE)) return EFS_ERR_NOENT;
		if (errno != ENOENT) return EFS_ERR_ERRNO;
 
		if (drivername[0] && 
		    (!(driver = efs_find_driver(drivername)))) {
		        return EFS_ERR_NODRIVER;
		}	

		if (!driver) driver = efs_driver_list [0];  /* default */

		if ((flags&EFS_PROT) && !pw) { 
			if (!pass_func) return EFS_ERR_INVPASS;
			if (!(pw = pass_func (filename, TRUE))) 
				return EFS_ERR_INVPASS;
		}

		if ((flags&EFS_WRITE) && (res = efs_lock_create(lockname))) 
			return res;
		
		res = driver->sops->create(dir,driver,filename,flags,mode,pw);
	}


	if (*dir) {
		if (flags&EFS_WRITE) {
			(*dir)->efs->lockname = g_strdup(lockname);
			(*dir)->efs->mode = EFS_RDWR;
		} else (*dir)->efs->mode = EFS_READ;
		(*dir)->mode |= EFS_DIR|EFS_ROOT|(flags&EFS_RDWR);
		(*dir)->efs->root = *dir;
	} else if (flags&EFS_WRITE) efs_lock_remove (lockname);

	return res;
}

/**
 * efs_open:
 * @dir: return value
 * @path: filesystem path
 * @flags: access flags
 * @mode: file permissions
 * @passwd: a password for protected files
 *
 * Description: Opens or creates a new #EFS within file @path. 
 *
 * @flags is %EFS_READ or %EFS_RDWR which request opening the #EFS read-only 
 * or read/write. flags  may  also  be  bitwise-or'd with one or more of the
 * following:
 *
 * %EFS_CREATE If the #EFS does not exist it will be created.
 *
 * %EFS_EXCL  When used with %EFS_CREATE, if the #EFS already exists it is an 
 * error and the open will fail.
 *
 * @mode  specifies  the  permissions to use if a new file is created.
 * You can specify them in octal with 0700 which gives 700 permission
 * to the file created.
 *
 * Returns: EFSResult
 */

EFSResult
efs_open (EFSDir **dir, const char *path, gint flags, gint mode, 
	  gchar *passwd)
{
	return efs_open_full (dir, path, flags, mode, NULL, passwd);
}

EFSResult
efs_open_cb (EFSDir **dir, const char *path, gint flags, gint mode, 
	     EFSPassFunc pass_func) 
{
	return efs_open_full (dir, path, flags, mode, pass_func, NULL);
}

/**
 * efs_close:
 * @root: reference to the #EFS root directory
 *
 * Description: Closes the #EFS and frees all resources.
 *
 * Returns: zero on success, or -1 if an error occurred.
 */

EFSResult
efs_close (EFSDir *root)
{
	EFSResult res;
	
	g_return_val_if_fail (root != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (root->mode&EFS_ROOT, EFS_ERR_INVAL);

	efs_typehash_clear (root->efs);

	if (root->efs->lockname) {
		efs_lock_remove (root->efs->lockname);
		g_free (root->efs->lockname);
		root->efs->lockname = NULL;
	}

	if (!(res = root->efs->driver->sops->close (root))) g_free (root);
	
	return res;
}

/**
 * efs_commit:
 * @root: reference to the #EFS root directory
 *
 * Description: Synchronize data on disk with memory.
 *
 * Returns: zero on success, or -1 if an error occurred.
 */

EFSResult
efs_commit (EFSDir *root)
{
	g_return_val_if_fail (root != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (root->mode&EFS_ROOT, EFS_ERR_INVAL);
	
	if (!(root->efs->mode&EFS_WRITE)) return EFS_ERR_PERM;

	efs_typehash_clear (root->efs);

	return root->efs->driver->sops->commit (root);
}

/**
 * efs_revert:
 * @root: reference to the #EFS root directory
 *
 * Description: undo all changes since last commit
 *
 * Returns: zero on success, or -1 if an error occurred.
 */

EFSResult
efs_revert (EFSDir *root)
{
	g_return_val_if_fail (root != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (root->mode&EFS_ROOT, EFS_ERR_INVAL);

	if (!(root->efs->mode&EFS_WRITE)) return EFS_ERR_PERM;
	
	efs_typehash_clear (root->efs);

	return root->efs->driver->sops->revert (root);
}

/**
 * efs_fsstat:
 * @root: reference to the #EFS root directory
 * @fsstat: a buffer to store the results
 *
 * Description: get information about the #EFS file system. 
 *
 * Returns: zero on success, or -1 on failure.
 */

EFSResult
efs_fsstat (EFSDir *root, EFSFSStat *fsstat)
{
	g_return_val_if_fail (root != NULL, EFS_ERR_INVAL);
	g_return_val_if_fail (root->mode&EFS_ROOT, EFS_ERR_INVAL);
	g_return_val_if_fail (fsstat != NULL, EFS_ERR_INVAL);

	return root->efs->driver->sops->fsstat (root, fsstat);
}

/**
 * efs_node_close
 * @node: file or directory descriptor
 *
 * Description: closes the file or directory descriptor.
 *
 */

EFSResult
efs_node_close (EFSNode *node)
{
	g_return_val_if_fail (node != NULL, EFS_ERR_INVAL);

	if (node->mode & EFS_FILE)
		return efs_file_close (node);
	else
		return efs_dir_close (node);
}

/**
 * efs_file_open:
 *
 * see efs_node_open()
 */

EFSResult
efs_file_open (EFSFile **file, EFSDir *dir, const char *path, gint flags)
{
	return efs_node_open (file, dir, path, flags, EFS_FILE);
}


/**
 * efs_dir_open:
 *
 * see efs_node_open()
 */

EFSResult
efs_dir_open (EFSDir **dir, EFSDir *parent, const char *path, gint flags)
{
	return efs_node_open (dir, parent, path, flags, EFS_DIR);
}

