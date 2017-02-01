/*
 * file locking support functions
 *
 * original code from liblockfile
 *
 * Much of the code is copied from liblockfile, which is written by
 * Miquel van Smoorenburg <miquels@cistron.nl>.
 *
 * This is only a quick hack to support file locking for libefs.
 * Changes made by Dietmar Maurer <dietmar@maurer-it.com>
 *
 *
 */

#include "efs_internal.h"

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>


static GList *efs_lockfiles = NULL;
static gint atexit_registered = 0;

/**
 * efs_remove_all_lockfiles:
 *
 * Description: This function is called at program exit and removes
 * all open lockfiles. 
 *
 */

static void
efs_remove_all_lockfiles (void)
{
	GList *l;

	l = efs_lockfiles;
	while (l) {
		if (l->data) { unlink (l->data); g_free (l->data); }
		l = l->next;
	}

	g_list_free (efs_lockfiles);
	efs_lockfiles = NULL;
}

/**
 * efs_lock_create:
 * @lockfile: the lockfile name (filesystem path)
 * 
 * Description: Create a lockfile in a NFS save way.
 *
 * Returns: zero on success, or -1 on failure.
 */

EFSResult
efs_lock_create (const char *lockfile)
{
	struct stat	st, st1;
	char		tmplock[1024];
	char		sysname[256];
	char		buf[32];
	char		*p;
	int		sleeptime = 5;
	int		statfailed = 0;
	int		fd;
	int		i, len, res;
	int             retries = 2;

	if (!atexit_registered) {
		atexit_registered = 1;
		g_atexit(efs_remove_all_lockfiles);
	}

	/*
	 *	Safety measure.
	 */

	if (strlen(lockfile) + 32 > 1024) return -1;

	/*
	 *	Create a temp lockfile (hopefully unique) and write
	 *	our pid in it.
	 */

	if (gethostname(sysname, sizeof(sysname)) < 0) return -1;
	if ((p = strchr(sysname, '.')) != NULL)
		*p = 0;
	strcpy(tmplock, lockfile);
	if ((p = strrchr(tmplock, '/')) == NULL) p = tmplock;
	else p++;

	sprintf(p,".lk%05d%x%s",(int)getpid(), (int)time(NULL) & 15, sysname);
	
	i = umask(022);
	fd = open(tmplock, O_WRONLY|O_CREAT|O_EXCL, 0644);
	umask(i);
	if (fd < 0) return -1;
       
	sprintf(buf, "%d\n%s\n", (int)getpid(),sysname);
	p = buf;
	len = strlen(buf);

	i = write(fd, p, len);
	if (close(fd) != 0) i = -1;
       
	if (i != len) {
		unlink(tmplock);
		return -1;
	}

	/*
	 *	Now try to link the temporary lock to the lock.
	 */

	for (i = 0; i < retries && retries > 0; i++) {

		sleeptime = i > 12 ? 60 : 5 * i;
		if (sleeptime > 0) sleep(sleeptime);

		if ((res = efs_lock_check(lockfile)) == 1) return 0;
		if (res == -1) unlink(lockfile);

		link (tmplock, lockfile);

		if (lstat(tmplock, &st1) < 0) return -1;
		if (lstat(lockfile, &st) < 0) {
			if (statfailed++ > 5) {
				unlink(tmplock);
				return -1;
			}
			continue;
		}

		/*
		 *	See if we got the lock.
		 */
		if (st.st_rdev == st1.st_rdev && st.st_ino  == st1.st_ino) {
			unlink(tmplock);
			efs_lockfiles = g_list_prepend (efs_lockfiles, 
							g_strdup(lockfile));

			return 0;
		}
		statfailed = 0;
	}

	unlink(tmplock);
	return -1;
}

/**
 * efs_lock_check:
 * @lockfile: the lockfile name (filesystem path)
 * 
 * Description: See if another process has locked the file.
 *
 * Returns: Returns 0 if so, -1 if not, or 1 if this
 * process has locked the file by itself.
 */

EFSResult
efs_lock_check (const char *lockfile)
{
	struct stat	st;
	pid_t		pid;
	int		fd, len, r;
	char		buf[512], sn[512], sysname[256];
	
	if (stat(lockfile, &st) < 0) return -1;

	if (gethostname(sysname, sizeof(sysname)) < 0) return -1;

	pid = 0;
	if ((fd = open(lockfile, O_RDONLY)) >= 0) {
		len = read(fd, buf, sizeof(buf));
		close(fd);
		if (len > 0) {
			buf[len] = 0;
			sn[0] = 0;
			sscanf (buf, "%d\n%255s\n", &pid, sn);
			sn[sizeof(sn)-1] = 0;
		}
	}
	
	if (pid && !strcmp(sysname, sn)) {
		if (getpid() == pid) return 1;
		r = kill(pid, 0);
		if (r == 0 || errno == EPERM)
			return 0;
		if (r < 0 && errno == ESRCH)
			return -1;
		
	}

	return 0;
}

/**
 * efs_lock_remove:
 * @lockfile: the lockfile name (filesystem path)
 * 
 * Description: Remove a lock
 *
 * Returns: zero on success, or -1 on failure.
 */

EFSResult
efs_lock_remove (const char *lockfile)
{
	GList *l;

	if (!lockfile) return 0;

	l = efs_lockfiles;
	while (l) {
		if (!strcmp(l->data, lockfile)) {
			g_free (l->data);
			if (l->prev)
				l->prev->next = l->next;
			if (l->next)
				l->next->prev = l->prev;
	  
			if (efs_lockfiles == l)
				efs_lockfiles = l->next;
	  
			g_list_free_1 (l);

			break;
		} else l = l->next;
	}

	return unlink (lockfile);
}
