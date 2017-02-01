/* efstest.c - some basic tests

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

#include <unistd.h>
#include <string.h>
#include <stdlib.h>


#include "efs.h"

/*#define TESTSIZE ((512/4)*(16+127+127*127+127*127*2))*/
#define TESTSIZE (1024*1)

#define SR(x) { show_result(x); g_assert_not_reached (); }

EFSResult result;

static void
show_result (const gchar *what)
{
        printf ("%s: %s (%d)\n", what, efs_strerror (result), result);
}


int 
main ()
{
	EFSDir *root,*dir1,*dir2;
	EFSFile *file;
	EFSStat filestat;
	EFSFSStat fsstat;
	gint32 i,j,size;
	guint32 code;
	gchar *p;
	struct stat fs;
	gchar buf[1024];
	gint32 br;
	guint32 pos;

	unlink ("/tmp/test.efs");

	/*******************************************************************
	 * Tests for normal files                           
	 ******************************************************************/

	printf ("basic read/write tests:\n");
	
	if ((result = efs_open (&root, "ib1enc:/tmp/test.efs", 
				EFS_CREATE|EFS_PROT, 
				0664, "test")))
		SR ("open failed");
	
	/* try to open non existent file */
	if (!(result = efs_file_open (&file, root, "test1.txt", EFS_RDWR)))
		SR ("open file succeded");
	
	/* create a new file */
	if ((result = efs_file_open (&file, root, "test5.txt",
				     EFS_CREATE|EFS_RDWR)))
		SR ("open file failed");
       
	/* create a new file */
	if ((result = efs_file_open (&file, root, "test1.txt",
				     EFS_CREATE|EFS_RDWR)))
		SR ("open file failed");

	for (i=0;i<TESTSIZE;i++) 
		if ((result = efs_file_write (file,&i,4))) SR ("write failed");

	if ((result = efs_file_close (file))) SR ("file close failed");

	/* open an existing file */
	if ((result = efs_file_open (&file, root, "test1.txt", EFS_READ)))
		SR ("open file failed");
	
	/* write to a read only file */
	if (!(result = efs_file_write (file,&i,4))) SR ("write succeded");

	/* erase the file */
	if ((result = efs_erase (root, "test1.txt"))) SR ("erase failed");

	for (i=0;i<TESTSIZE;i++) {
		gint32 data;
		if ((result=efs_file_read(file, &data, 4, &br))) 
			SR ("read failed");
		if (br != 4) SR ("read failed");
		if (data!=i) g_assert_not_reached();
	}

	if ((result = efs_file_close (file))) SR ("file close failed");

	/* try to open a deleted file */
	if (!(result = efs_file_open (&file, root, "test1.txt", EFS_RDWR)))
		SR ("open file succeded");
	
	if ((result = efs_commit (root))) SR ("commit failed");

	/* open an existing file */
	if ((result = efs_file_open (&file, root, "test5.txt", EFS_READ)))
		SR ("open file failed");	

	printf ("OK\n\n\n");

	/*******************************************************************
	 * Tests for compressed files                           
	 ******************************************************************/

	printf ("compressed file tests:\n");

	/* create a compressed file */
	if ((result = efs_file_open (&file, root, "test2.txt",
				     EFS_COMP|EFS_CREATE)))
		SR ("open failed");
		
	for (i=0;i<TESTSIZE;i++) {
		gint32 j=i/8; 
		if ((result = efs_file_write (file,&j,4))) SR ("write failed");
	}

	/* we can´t seek writable compressed files */
	if (!(result = efs_file_seek (file, 0, EFS_SEEK_SET, &pos)))
		SR ("seek succeded");

	if ((result = efs_file_close (file))) SR ("file close failed");

	/* open the file */
	if ((result = efs_file_open (&file, root, "test2.txt", EFS_READ)))
		SR ("open file failed");

	for (i=0;i<TESTSIZE;i++) {
		gint32 data; 
		if ((result=efs_file_read (file, &data, 4, &br))) 
			SR ("read failed");
		if (br != 4) SR ("read failed");
		if (data!=(i/8)) g_assert_not_reached();
	}

	/* seek to start */
	if ((result = efs_file_seek (file, 0, EFS_SEEK_SET, &pos)))
		SR ("seek failed");
	if (pos) SR ("seek failed");

	for (i=0;i<TESTSIZE;i++) {
		gint32 data; 
		if ((result=efs_file_read (file, &data, 4, &br))) 
			SR ("read failed");
		if (br != 4) SR ("read failed");
		if (data!=(i/8)) g_assert_not_reached();
	}
	
	if ((result = efs_file_close (file))) SR ("file close failed");

	if ((result = efs_stat (root, "test2.txt", &filestat))) 
		SR ("stat failed");

	if ((result = efs_revert (root))) SR ("revert failed");

	if ((result = efs_fsstat (root, &fsstat))) SR ("fsstat failed");

	printf("Unused Space: %d\n",fsstat.free);

	if ((result = efs_close (root))) SR ("close failed");

	printf ("OK\n\n\n");

	/*******************************************************************
	 * Some directory tests  
	 ******************************************************************/

	printf ("basic directory tests:\n");
	
	unlink ("/tmp/testdir.efs");

	if ((result = efs_open (&root, "/tmp/testdir.efs", EFS_CREATE, 
				0664, NULL)))
		SR ("open failed");

	if (!(result = efs_dir_open (&dir2, root, "testdir", EFS_READ)))
		SR ("open dir succeded");	

	if (!(result = efs_dir_open (&dir2, root, 
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     "0123456789012345678901234567890123456789"
				     , EFS_READ)))
		SR ("open dir succeded");	

	if ((result = efs_dir_open (&dir1, root, "testdir", EFS_CREATE)))
		SR ("open dir failed");	

	if ((result = efs_dir_close (dir1))) SR ("close dir failed");
	

	if (!(result = efs_dir_open (&dir2, root, "testdir/a", EFS_READ)))
		SR ("open dir succeded");	

	for (i = 0; i < 200; i++) {
		sprintf(buf,"testdir/testdir%d", i);
		if ((result = efs_dir_open (&dir2, root, buf, EFS_CREATE)))
			SR ("open dir failed");
		if ((result = efs_dir_close (dir2))) SR ("close dir failed");
	}

	for (i = 0; i < 100; i++) {
		sprintf(buf,"testdir/testdir%d", i);
		if ((result = efs_erase (root, buf))) SR ("rmdir failed");
	}
	
	if (!(result = efs_erase (root, "testdir"))) SR ("rmdir succeded");

	if ((result = efs_rename (root, "testdir", "testdir2"))) 
		SR ("rename failed");

	for (i = 0; i < 100; i++) {
		sprintf(buf,"testdir2/testdir%d", i);
		if (!(result = efs_erase (root, buf))) SR ("rmdir succeded");
	}

	for (i = 100; i < 200; i++) {
		sprintf(buf,"testdir2/testdir%d", i);
		if ((result = efs_erase (root, buf))) SR ("rmdir failed");
	}

	if ((result = efs_erase (root, "testdir2"))) SR ("rmdir failed");
	
	if ((result = efs_commit (root))) SR ("commit failed");

	if ((result = efs_close (root))) SR ("close failed");

	printf ("OK\n\n\n");

	/*******************************************************************
	 * Testing mime type system
	 ******************************************************************/

	printf ("Testing mime types:\n");

	if ((result = efs_open (&root, "/tmp/test.efs", EFS_WRITE, 
			       0664, "test"))) 
		SR ("open failed");

	if ((result = efs_strtype_set (root, "text/plain"))) 
		SR ("strtype failed");

	if ((result = efs_type_lookup (root, "text/plain", &code)))
		SR ("type lookup failed");
	printf("Type = %d\n",code);
	if (code != 4004) g_assert_not_reached();

	if ((result = efs_strtype_get (root, &p))) 
		SR ("strtype get failed");
	printf("StrType = %s\n",p);
	if (strcmp ("text/plain",p)) g_assert_not_reached();

	if ((result = efs_file_open (&file, root, "mime.test", EFS_CREATE)))
		SR ("file open failed");		

	if (!(result = efs_type_lookup (file, "text/xxxx", &code)))
		SR ("type lookup succeded");

	if ((result = efs_strtype_set (file, "text/xxxx"))) 
		SR ("strtype failed");

	if ((result = efs_type_lookup (file, "text/xxxx", &code)))
		SR ("type lookup failed");
	printf("Type = %d\n",code);
	if (code != 1000000) g_assert_not_reached();

	if ((result = efs_strtype_lookup (file, code, &p)))
		SR ("strtype lookup failed");
	printf("StrType = %s\n",p);
	if (strcmp ("text/xxxx",p)) g_assert_not_reached();

	if ((result = efs_file_close (file))) SR ("file close failed");

	if ((result = efs_commit (root))) SR ("commit failed");

	if ((result = efs_close (root))) SR ("close failed");

	printf("reopen efs file\n");

	if ((result = efs_open (&root, "/tmp/test.efs", EFS_WRITE, 
			       0664, "test"))) 
		SR ("open failed");

	if ((result = efs_file_open (&file, root, "mime.test", EFS_READ)))
		SR ("file open failed");		

	if ((result = efs_strtype_get (file, &p))) 
		SR ("strtype get failed");
	printf("StrType = %s\n",p);
	if (strcmp ("text/xxxx",p)) g_assert_not_reached();

	if ((result = efs_close (root))) SR ("close failed");
	
	printf ("OK\n\n\n");
	
	/*******************************************************************
	 * Testing space efficiency
	 ******************************************************************/

	printf ("Test space efficiency:\n");

	size = 1;

	while (size <= 1024) {
		gchar buf[1024];
		unlink ("/tmp/test1.efs");
		if ((result = efs_open (&root, "ib1:/tmp/test1.efs", 
					EFS_CREATE, 0664, NULL))) {
			SR ("open failed");
		}

		for (j=0;j<4;j++) {

			sprintf(buf,"test%d.txt",j);
			if ((result = efs_file_open (&file, root, buf,
						     EFS_CREATE|EFS_RDWR))) {
				SR ("file open failed");		
			}
		     
			for (i=0;i<size;i++) {
				if (efs_file_write (file,buf,1024)) 
					g_assert_not_reached();
			}
			

			if (efs_file_close (file)) g_assert_not_reached();
		}

		if ((result = efs_commit (root))) SR ("commit failed");

		if ((result = efs_close (root))) SR ("close failed");

		if (stat ("/tmp/test1.efs", &fs)) g_assert_not_reached();

		printf ("Space efficiency: filesize %5dKB = %.2f%%\n",
			size,
			((fs.st_size-512*3)/(1024.0*size*j)-1)*100);
		
		size *= 2;
	}

	printf ("OK\n\n\n");

	exit(0);
}
