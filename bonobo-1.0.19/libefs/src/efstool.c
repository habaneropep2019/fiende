/* efstool.c - efs manipulation tool

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

#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

#include "efs_internal.h"

EFSDir *root;
gchar path[EFS_MAXPATHLEN];

static gchar*
get_passwd (const gchar *filename, gboolean new)
{
	static gchar passwd[1024];

	
	if (new) printf("This File (%s) is password protected.\n", filename);
	printf("Please enter the password: ");
	       
	passwd[0] = 0;
	fgets (passwd, sizeof(passwd)-2, stdin);
	passwd[sizeof(passwd)] = 0;
	if (passwd[0] && (passwd[strlen(passwd)-1] == '\n'))
		passwd[strlen(passwd)-1] = 0;
	return passwd;
}

static void
prompt (void)
{
	fprintf (stdout, "efs:%s# ",path);
	fflush (stdout);
}

static const char *
full_path (const char *name)
{
	static gchar buf[EFS_MAXPATHLEN];

	if (name[0]==EFS_FDEL) return name;

	strcpy (buf, path);
	if (path[strlen(buf)-1]!=EFS_FDEL) strcat (buf, "/");
	strcat (buf, name);
	return buf;
}

static void
stat_command ()
{
	EFSFSStat stat;

	if (!efs_fsstat(root, &stat)) {

		printf("EFS Filesystem Summary:\n");
		printf("Driver:      %s\n",stat.drivername);
		printf("Size:        %d\n",stat.size);
		printf("Name length: %d\n",stat.namelen);
		printf("Version:     %d\n",stat.version);
		printf("Free:        %d\n",stat.free);
	} else {
		printf("stat command failed\n");
	}
}

static void
ls_command ()
{
	EFSDir *dir;
	EFSNode *node;
	EFSStat fs;
	EFSDirEntry de;
	const char *name = strtok (NULL, " \t\n");
	gchar *ctype;
	guint32 type;

	if (name) name = strdup(full_path(name));
	else name = path;

	if (efs_dir_open (&dir, root, name, 0)) {
		fprintf (stderr, "ls failed!\n");
		return;
	}

	while (!efs_dir_read(dir, &de)) {
		if (de.type==EFS_DIR) printf("D ");
		else printf("  ");
		printf("%s",de.name);

		if (!efs_node_open (&node, dir, de.name, EFS_READ, 0)) {

			efs_node_stat (node, &fs);
			efs_type_get (node, &type);
			efs_strtype_get (node, &ctype);

			printf (" (%d, %d, %s)",fs.size, type, ctype);
		}
		printf("\n");
	}

	efs_dir_close (dir);
}

static void
mv_command ()
{
	const char* from = strtok (NULL, " \t\n");
	const char* to = strtok (NULL, " \t\n");
	
	if (!to || !from) {
		fprintf (stderr, "usage: mv TO FROM\n");
		return;
	}

	from = strdup(full_path(from));
	to = strdup(full_path(to));

	if (efs_rename(root, from, to)) {
		fprintf (stderr, "move failed!\n");
		return;	
	}
}

static void
rm_command ()
{
	const char *name;

	name = strtok (NULL, " \t\n");

	if (!name) {
		fprintf (stderr, "usage: rm filename\n");
		return;
	}

	name = strdup(full_path(name));

	if (efs_erase(root, name))
		fprintf(stderr,"rm failed (%s)\n", name);

}

static void
get_command ()
{
	const char *name;
	const char *lname;
	FILE    *fdest;
	EFSFile *fsrc;

	name = strtok (NULL, " \t\n");

	if (!name) {
		fprintf (stderr, "usage: get filename\n");
		return;
	}

	name = strdup (full_path (name));

	if (efs_file_open (&fsrc, root, name, EFS_READ)) {
		fprintf (stderr,"efs open failed (%s)\n", name);
		return;
	}
	
	lname = strrchr (name, '/');
	if (!lname)
		lname = name;
	else
		lname++;
	if (!(fdest = fopen (lname, "wb+"))) {
		fprintf (stderr,"open failed (%s)\n", lname);
		efs_file_close (fsrc);
		return;
	}

	while (1) {
		char   buffer [1024];
		gint32 br;

		if (efs_file_read (fsrc, buffer, 1024, &br)) break;

		fwrite (buffer, br, 1, fdest);
	}

	efs_file_close (fsrc);
	fclose (fdest);
}

static void
mkdir_command ()
{
	const char *name;
	EFSDir *dir;

	name = strtok (NULL, " \t\n");

	if (!name) {
		fprintf (stderr, "usage: mkdir dirname\n");
		return;
	}

	name = strdup(full_path(name));

	if (efs_dir_open(&dir, root, name, EFS_CREATE|EFS_EXCL)) {
		fprintf(stderr,"mkdir failed (%s)\n", name);
	} else efs_dir_close (dir);
}

static void
cd_command ()
{
	EFSDir *dir;
	const char* to;
	static char *op = NULL;

	if (op) g_free(op);
	op = strdup(path);

	to = strtok (NULL, " \t\n");

	if (!to) {
		fprintf (stderr, "usage: cd DIR\n");
		return;
	}

	if (!strcmp(to,"..")) {
		gint i;
	  
		i = strlen(path);
		while ((i>0) && (path[i-1]==EFS_FDEL)) i--;
		while ((i>0) && (path[i-1]!=EFS_FDEL)) i--;
		while ((i>0) && (path[i-1]==EFS_FDEL)) i--;
		
		if (!i) path[1]=0;
		else path[i]=0;
	} else if (to[0]==EFS_FDEL) {
		strcpy (path, to);
	} else {
		if (path[strlen(path)-1]!=EFS_FDEL) strcat (path, "/");
		strcat (path, to);
	}

	if (efs_dir_open(&dir, root, path, 0)) {
		fprintf(stderr,"No such directory\n");
		strcpy (path, op);
		return;
	}
	efs_dir_close (dir);
}

static void
help_command ()
{
	printf("Available commands:\n");
	printf("ls             list directory contents\n");
	printf("cd dir         change directory\n");
	printf("get file       extract a file\n");
	printf("mv old new     move file or directory\n");
	printf("rm name        delete a file or directory\n");
	printf("mkdir name     create a directory\n");
	printf("commit         commit changes\n");
	printf("stat           print filesystem status\n");
	printf("exit           quit efstool\n");
	printf("help           print this screen\n");
}

static void
add_file_or_dir (gchar *name) 
{
	struct stat sb;
	EFSFile *file;
	EFSDir *dir;
	gint fd, i, j;
	gchar buf[512],*nn;
	DIR *d;
	struct dirent *de;

	if (lstat(name,&sb) == -1) {
		printf("add %s failed!\n", name);
		return;
	}

	if (S_ISDIR(sb.st_mode)) {
		if (!(d = opendir (name))) return;
		if (efs_dir_open(&dir, root, name, EFS_CREATE|EFS_EXCL)) 
			return;
		efs_dir_close (dir);

		while ((de = readdir (d))) {
			if (!strcmp(de->d_name,".")) continue;
			if (!strcmp(de->d_name,"..")) continue;
			nn = g_strconcat (name,"/", de->d_name, NULL);
			add_file_or_dir (nn);
			g_free(nn);
		}
		closedir(d);
		return;
	}
	if (S_ISREG(sb.st_mode)) {
		if (efs_file_open (&file, root, name, EFS_CREATE|EFS_EXCL)) 
			return;
		if ((fd = open (name, O_RDONLY)) == -1) return;

		while ((i=read (fd, buf, 512))) {
			j = efs_file_write(file, buf, i);
		}

		efs_file_close(file);
		close (fd);
		
		return;
	}

	printf("add %s failed (not a file)!\n", name);
}

static void
list_contents (EFSDir *dir)
{
	EFSDir *d;
	EFSDirEntry de;

	efs_dir_seek (dir, 0);
	while (efs_dir_read (dir, &de) > 0) {
		printf("%s\n", de.name);
	}

	efs_dir_seek (dir, 0);
	while (efs_dir_read (dir, &de) > 0) {
		if (de.type == EFS_DIR) {
			if (!efs_dir_open (&d, dir, de.name, EFS_READ)) {
				printf("\n%s:\n",de.name);
				list_contents(d);
				efs_dir_close(d);
			}
		}
	}
}

static void 
print_usage()
{
	fprintf (stderr, "usage: efstool efs-file\n");
	fprintf (stderr, "       efstool c efs-file {files}\n");
	fprintf (stderr, "       efstool t efs-file\n");
}

int
main(int argc, char *argv[])
{
	char read_buf[1024];
	gint i;

	if (argc < 2) {
		print_usage();
		exit (1);
	}

	if (argc == 2) { /* interactive mode */
		if (efs_open_cb(&root,argv[1],EFS_RDWR,0644,get_passwd)) {
			fprintf (stderr, "efs_open failed\n");
			exit (1);
		}

		strcpy (path, "/");

		prompt ();

		while (fgets (read_buf, 1024, stdin) != NULL) {
			const char* cmd = strtok (read_buf, " \t\n");

			if (cmd == NULL) {
				/* nothing */
			} else if (strcmp (cmd, "ls") == 0) {
				ls_command ();
			} else if (strcmp (cmd, "mv") == 0) {
				mv_command ();
			} else if (strcmp (cmd, "cd") == 0) {
				cd_command ();
			} else if (strcmp (cmd, "get") == 0) {
				get_command ();
			} else if (strcmp (cmd, "rm") == 0) {
				rm_command ();
			} else if (strcmp (cmd, "mkdir") == 0) {
				mkdir_command ();
			} else if (strcmp (cmd, "commit") == 0) {
				efs_commit (root);
			} else if (strcmp (cmd, "stat") == 0) {
				stat_command ();
			} else if (strcmp (cmd, "help") == 0) {
				help_command ();
			} else if (strcmp (cmd, "exit") == 0) {
				break;
			} else if (strcmp (cmd, "quit") == 0) {
				break;
			} else {
				fprintf (stderr, "unrecognized command: %s\n",
					 cmd);
			}
			prompt ();
		}
		
		efs_close (root);

	} else {

		if (!strcmp(argv[1], "c")&&(argc>=4)) {

			if (efs_open(&root, argv[2], EFS_CREATE|EFS_EXCL, 
				     0644, NULL)) {
				fprintf (stderr, "efs_open failed\n");
				exit (1);
			}
		
			for (i = 3; i < argc; i++) {
				add_file_or_dir (argv[i]);
			}

			efs_commit (root);
			efs_close (root);
			exit (0);
		}
		if (!strcmp (argv[1], "t")&&(argc==3)) {

			if (efs_open(&root, argv[2], EFS_READ, 0, NULL)) {
				fprintf (stderr, "efs_open failed\n");
				exit (1);
			}

			list_contents (root);

			efs_close (root);
			exit(0);
		}

		print_usage();
	}

	return 0;
}

