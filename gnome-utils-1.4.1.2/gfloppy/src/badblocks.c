/*
 * badblocks.c		- Bad blocks checker
 *
 * Copyright (C) 1992, 1993, 1994  Remy Card <card@masi.ibp.fr>
 *                                 Laboratoire MASI, Institut Blaise Pascal
 *                                 Universite Pierre et Marie Curie (Paris VI)
 *
 * Copyright 1995, 1996, 1997 by Theodore Ts'o
 *
 * This file is based on the minix file system programs fsck and mkfs
 * written and copyrighted by Linus Torvalds <Linus.Torvalds@cs.helsinki.fi>
 * 
 * %Begin-Header%
 * This file may be redistributed under the terms of the GNU Public
 * License.
 * %End-Header%
 *
 */

/*
 * History:
 * 93/05/26	- Creation from e2fsck
 * 94/02/27	- Made a separate bad blocks checker
 * 99/09/30     - modified by Red Hat, Inc. (jrb@redhat.com) for use in gfloppy.
 */

#include "config.h"

#include <gnome.h>

#include <errno.h>
#include <fcntl.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#ifdef HAVE_LINUX_FD_H
#include <linux/fd.h>
#include <linux/fs.h>
#endif

#include <et/com_err.h>
#include <ext2fs/ext2_io.h>

#include "gfloppy.h"



int w_flag = 0;			/* do r/w test */
static unsigned long currently_testing = 0;
static unsigned long num_blocks = 0;
static GFloppy *internal_floppy = NULL;


/* keep in sync with fd_print in fd_format */
static void
fd_print (GFloppy *floppy, gchar *string)
{
	write (floppy->message[1], string, strlen (string));
	write (floppy->message[1], "\000", 1);
}

static void
print_status (void)
{
	gchar *msg = NULL;
	gint val = 0;

	val = CLAMP ( (gint) ((0.67 +  0.33*((gfloat)currently_testing)/num_blocks) * 100), 67, 100);
	msg = g_strdup_printf ("P%3d", val);
	fd_print (internal_floppy, msg);
	g_free (msg);
}

static void
alarm_intr (int alnum)
{
	signal (SIGALRM, alarm_intr);
	alarm(1);
	if (!num_blocks)
		return;
	print_status ();
}

/*
 * Perform a test of a block; return the number of blocks readable/writeable.
 */
static long do_test (int dev, char * buffer, int try, unsigned long block_size,
		     unsigned long current_block)
{
	long got;

	/* Seek to the correct loc. */
	if (ext2fs_llseek (dev, (ext2_loff_t) current_block * block_size,
			   SEEK_SET) != (ext2_loff_t) current_block * block_size)
		fd_print (internal_floppy, _("MInternal Error: Unable to seek to the correct location."));

	/* Try the read */
	got = read (dev, buffer, try * block_size);

	if (got < 0)
		got = 0;	
	if (got & 511) {
		gchar *msg;
		msg = g_strdup_printf (_("MInternal Error: Weird value (%ld) in do_test\n"), got);
		fd_print (internal_floppy, msg);
		g_free (msg);
	}
	got /= block_size;
	return got;
}

static void flush_bufs (int dev, int sync)
{
	if (sync && fsync (dev) == -1)
		;
		/*com_err (program_name, errno, "during fsync");*/

#ifdef BLKLSBUF
	ioctl (dev, BLKFLSBUF, 0);	/* In case this is a HD */
#endif
#ifdef FDFLUSH
	ioctl (dev, FDFLUSH, 0);	/* In case this is floppy */
#endif
}

static gint
test_ro (int dev, unsigned long blocks_count,
	 unsigned long block_size, 
	 unsigned long from_count)
{
#define TEST_BUFFER_BLOCKS 16
	char * blkbuf;
	int try;
	long got;

	blkbuf = g_malloc (TEST_BUFFER_BLOCKS * block_size);
	flush_bufs (dev, 0);
	try = TEST_BUFFER_BLOCKS;
	currently_testing = from_count;
	num_blocks = blocks_count;
	while (currently_testing < blocks_count)
	{
		gchar *msg;
		print_status ();
		if (currently_testing + try > blocks_count)
			try = blocks_count - currently_testing;
		got = do_test (dev, blkbuf, try, block_size, currently_testing);
		currently_testing += got;

		if (got == try) {
			try = TEST_BUFFER_BLOCKS;
			continue;
		} else {
			try = 1;
		}

		if (got == 0) {
			currently_testing++;
			msg = g_strdup_printf (_("EBad block at block %lu"), currently_testing);
			fd_print (internal_floppy, msg);
			g_free (msg);
			
			/*fprintf (out, "%lu\n", currently_testing++);*/
		}
		
	}
	num_blocks = 0;
/*	alarm(0); */
	fflush (stderr);
	g_free (blkbuf);
}

#if 0
static void test_rw (GFloppy *floppy, int dev,
		     unsigned long blocks_count,
		     unsigned long block_size,
		     unsigned long from_count)
{
	int i;
	char * buffer;
	unsigned char pattern[] = {0xaa, 0x55, 0xff, 0x00};

	buffer = malloc (2 * block_size);
	if (!buffer)
	{
		com_err (program_name, ENOMEM, "while allocating buffers");
		exit (1);
	}

	flush_bufs (dev, 0);

	for (i = 0; i < sizeof (pattern); i++) {
		memset (buffer, pattern[i], block_size);
		num_blocks = blocks_count;
		currently_testing = from_count;
		for (;
		     currently_testing < blocks_count;
		     currently_testing++)
		{
			if (ext2fs_llseek (dev, (ext2_loff_t) currently_testing *
					   block_size, SEEK_SET) !=
			    (ext2_loff_t) currently_testing * block_size)
				com_err (program_name, errno,
					 "during seek on block %d",
					 currently_testing);
			write (dev, buffer, block_size);
		}
		num_blocks = 0;
		alarm (0);

		flush_bufs (dev, 1);

		num_blocks = blocks_count;
		currently_testing = from_count;

		for (;
		     currently_testing < blocks_count;
		     currently_testing++)
		{
			if (ext2fs_llseek (dev, (ext2_loff_t) currently_testing *
					   block_size, SEEK_SET) !=
			    (ext2_loff_t) currently_testing * block_size)
				com_err (program_name, errno,
					 "during seek on block %d",
					 currently_testing);

			if (read (dev, buffer + block_size, block_size) < block_size)
				fprintf (out, "%ld\n", currently_testing);
			else if (memcmp (buffer, buffer + block_size, block_size))
				fprintf (out, "%ld\n", currently_testing);
		}
		num_blocks = 0;
		alarm (0);

		flush_bufs (dev, 0);
	}
}
#endif

gint
check_badblocks (GFloppy *floppy)
{
	unsigned long block_size;
	unsigned long blocks_count;
	unsigned long from_count;
	int dev;
	gchar *msg;

	gint i = 0;

/*	while (!i)
	;*/
	fd_print (floppy, _("MChecking for bad blocks..."));
	internal_floppy = floppy;
	/* assume read-only test for now */
	w_flag = 0;

	/* setup defaults based on floppy */
	block_size = 512;
	blocks_count = floppy->nblocks;
	from_count = 0;

	dev = open (floppy->device, w_flag ? O_RDWR : O_RDONLY);
	if (dev < 0) {
		msg = g_strdup_printf (_("MFailed to open device %s for badblock checking\n"),
			floppy->device);
		fd_print (floppy, msg);
		g_free (msg);
		return -1;
	}


#if 0
	/* We don't even want to think about rw anymore */
	if (w_flag)
		test_rw (floppy, dev, blocks_count, block_size, from_count);
	else
		test_ro (floppy, dev, blocks_count, block_size, from_count);
#endif

	test_ro (dev, blocks_count, block_size, from_count);

	close (dev);
	fd_print (floppy, _("MChecking for bad blocks... Done"));
	return 0;
}
