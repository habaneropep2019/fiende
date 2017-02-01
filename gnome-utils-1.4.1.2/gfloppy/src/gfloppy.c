/* gfloppy.c
 *
 * Copyright (C) 1999 Red Hat, Inc.
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

#include "config.h"

#include <gnome.h>

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/major.h>

#ifdef HAVE_LINUX_FD_H
#include <linux/fd.h>
#include <linux/fs.h>
#endif

#include "gfloppy.h"
#include "fdformat.h"
#include "badblocks.h"

/* keep in sync with fd_print in badblocks */
static void
fd_print (GFloppy *floppy, gchar *string)
{
	write (floppy->message[1], string, strlen (string));
	write (floppy->message[1], "\000", 1);
}

static gint
floppy_block_size (GFloppySize size)
{
	gint rc;

	switch (size) {
	case 0:
		/* 1.44M */
		rc = 2880;
		break;
	case 1:
		/* 1.2M */
		rc = 2160;
		break;
	case 2:
		/* 720k */
		rc = 1440;		break;
	case 3:
		/* 360k */
		rc = 720;
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	return rc;
}

static gchar *
make_mformat_cmd (GFloppy *floppy)
{
	gchar *retval = NULL;

	switch (floppy->size) {
	case 0:
		/* 1.44 m */
		retval = g_strconcat (floppy->mformat_cmd, " -c 80 -h 2 -s 18 ", floppy->mdevice, NULL);
		break;
	case 1:
		/* 1.2 m */
		retval = g_strconcat (floppy->mformat_cmd, " -c 80 -h 2 -s 15 ", floppy->mdevice, NULL);
		break;
	case 2:
		/* 720 k */
		retval = g_strconcat (floppy->mformat_cmd, " -c 80 -h 2 -s 9 ", floppy->mdevice, NULL);
		break;
	case 3:
		/* 360 k */
		retval = g_strconcat (floppy->mformat_cmd, " -c 40 -h 2 -s 9 ", floppy->mdevice, NULL);
		break;
	default:
		g_assert_not_reached ();
	}
	return retval;
}

static gchar *
make_mke2fs_cmd (GFloppy *floppy)
{
	gchar *retval = NULL;

	return g_strconcat (floppy->mke2fs_cmd, " ", floppy->device, NULL);

#if 0
	gchar *bad_block_flag;

	if (floppy->quick_format)
		bad_block_flag = "";
	else
		bad_block_flag = "-c ";

	switch (floppy->size) {
	case 0:
		/* 1.44 m */
		retval = g_strconcat (floppy->mke2fs_cmd, " -b 2880 ", bad_block_flag, floppy->device, NULL);
		break;
	case 1:
		/* 1.2 m */
		retval = g_strconcat (floppy->mke2fs_cmd, " -b 2160 ", bad_block_flag, floppy->device, NULL);
		break;
	case 2:
		/* 720 k */
		retval = g_strconcat (floppy->mke2fs_cmd, " -b 1440 ", bad_block_flag, floppy->device, NULL);
		break;
	case 3:
		/* 360 k */
		retval = g_strconcat (floppy->mke2fs_cmd, " -b 720 ", bad_block_flag, floppy->device, NULL);
		break;
	default:
		g_assert_not_reached ();
	}
	return retval;
#endif
}

static int
format_ext2fs (GFloppy *floppy)
{
	gchar *cmd;
	gint rc = 0, i;
	fd_print (floppy, "P000");

	/* low-level format */
	if (!floppy->quick_format) {
		rc = fdformat_disk (floppy);
	}
	if (rc != 0)
		return -1;

	/* make the filesystem */
	cmd = make_mke2fs_cmd (floppy);

	fd_print (floppy, _("MMaking filesystem on disk..."));
	rc = system (cmd);
	if (rc > 3) {
		fd_print (floppy, _("EUnable to create filesystem correctly."));
		return -1;
	}

	fd_print (floppy, _("MMaking filesystem on disk... Done"));
	if (floppy->quick_format)
		fd_print (floppy, "P100");
	
	/* Check for bad_blocks */
	if (!floppy->quick_format) {
		rc = check_badblocks (floppy);
	}
	return rc;
}

static int
format_fat (GFloppy *floppy)
{
	gchar *cmd;
	gint rc = 0;

	/* low-level format */
	if (!floppy->quick_format) {
		rc = fdformat_disk (floppy);
	}
	if (rc != 0)
		return -1;

	/* make the filesystem */
	cmd = make_mformat_cmd (floppy);
	fd_print (floppy, _("MMaking filesystem on disk..."));
	rc = system (cmd);
	if (rc != 0) {
		fd_print (floppy, _("EUnable to create filesystem."));
		return -1;
	}
	fd_print (floppy, _("MMaking filesystem on disk... Done"));
	if (floppy->quick_format)
		fd_print (floppy, "P100");

	/* check for bad_blocks */
	if (!floppy->quick_format) {
		fd_print (floppy, _("MChecking for bad blocks... (this might take a while)")); 
		rc = system (g_strconcat (floppy->badblocks_cmd, " ", floppy->mdevice, NULL));
		if (rc == 0) {
			fd_print (floppy, "P100");
			fd_print (floppy, _("MChecking for bad blocks... Done"));
		} else {
			fd_print (floppy, _("EUnable to create filesystem."));
			return -1;
		}
	}
	return rc;
}

void
format_floppy (GFloppy *floppy)
{
	gint rc = 0;

	/* setup size field based on user request */
	floppy->nblocks = floppy_block_size (floppy->size);

	if (floppy->type == GFLOPPY_E2FS)
		rc = format_ext2fs (floppy);
	else
		rc = format_fat (floppy);

	_exit (rc);
}

GFloppyStatus
test_floppy_device (gchar *device)
{
	struct stat s;
        struct floppy_drive_struct ds;
        char name[32];
	gint fd;
	GFloppyStatus retval;

	/* sanity check */
	if (device == NULL || *device == '\000')
		return GFLOPPY_NO_DEVICE;

	if (stat (device, &s) < 0)
		return GFLOPPY_NO_DEVICE;
	if (!S_ISBLK(s.st_mode) || MAJOR(s.st_rdev) != FLOPPY_MAJOR)
		return GFLOPPY_NO_DEVICE;

	if (access (device, R_OK|W_OK) != 0)
		return GFLOPPY_INVALID_PERMISSIONS;

	fd = open (device, O_RDONLY|O_NONBLOCK);
	if (fd < 0)
		return GFLOPPY_NO_DEVICE;

	ioctl (fd, FDRESET, NULL);
	if (ioctl (fd, FDGETDRVTYP, name) == 0) {
		if (name && strcmp(name,"(null)")) {
			if (ioctl(fd, FDPOLLDRVSTAT, &ds) == 0 && ds.track >= 0)
				retval = GFLOPPY_DEVICE_OK;
			else
				retval = GFLOPPY_DEVICE_DISCONNECTED;
		} else {
			retval = GFLOPPY_NO_DEVICE;
		}
	}
	close(fd);

	return retval;
}
