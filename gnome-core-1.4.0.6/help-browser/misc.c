#include <config.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <zlib.h>

#ifdef HAVE_LIBBZ2
        #include <bzlib.h>
#endif

#ifdef HAVE_LIBBZ2_1_0
/* libbz2 1.0 compatibility macros */
# define bzopen  BZ2_bzopen
# define bzclose BZ2_bzclose
# define bzdopen BZ2_bzdopen
# define bzerror BZ2_bzerror
# define bzflush BZ2_bzflush
# define bzread  BZ2_bzread
# define bzwrite BZ2_bzwrite
#endif

#include <glib.h>

#include "misc.h"

gint
getOutputFrom(gchar *argv[], gchar *writePtr, gint writeBytesLeft,
	      guchar **outbuf, gint *outbuflen)
{
	gint progPID;
	gint toProg[2];
	gint fromProg[2];
	gint status;
	void (*oldhandler)(int);
	gint bytes;
	guchar buf[8193];
	guchar *tmpoutbuf;
	gint    outpos;
	gboolean out_closed = FALSE;

	*outbuf = NULL;
	*outbuflen = 0;

	oldhandler = signal(SIGPIPE, SIG_IGN);
	
	if(pipe(toProg) < 0) {
		g_warning("couldn't make pipe");
		return -1;
	}
	if(pipe(fromProg) < 0) {
		g_warning("couldn't make pipe");
		return -1;
	}
		
	progPID = fork();
	if (progPID == 0) {
		close(toProg[1]);
		close(fromProg[0]);
		dup2(toProg[0], 0);   /* Make stdin the in pipe */
		dup2(fromProg[1], 1); /* Make stdout the out pipe */
			
		close(toProg[0]);
		close(fromProg[1]);
			
		execvp(argv[0], argv);
		g_warning("couldn't exec %s", argv[0]);
		_exit(1);
	}
	if (progPID < 0) {
	        g_warning("couldn't fork %s", argv[0]);
		return -1;
	}
		
	close(toProg[0]);
	close(fromProg[1]);
		
	/* Do not block reading or writing from/to prog. */
	fcntl(fromProg[0], F_SETFL, O_NONBLOCK);
	fcntl(toProg[1], F_SETFL, O_NONBLOCK);
		
	outpos = 0;
	tmpoutbuf = NULL;

	out_closed = FALSE;
	for(;;) {
		/* write some data to the prog */
		if (writeBytesLeft) {
			gint n, r;

			n = (writeBytesLeft > 1024) ? 1024 : writeBytesLeft;
			if ((r=write(toProg[1], writePtr, n)) < 0) {
				if (errno != EAGAIN) {
					perror("getOutputFrom()");
					exit(1);
				}
				r = 0;
			}
			writeBytesLeft -= r;
			writePtr += r;
		} else {
			out_closed = TRUE;
			close(toProg[1]);
		}

		/* Read any data from prog */
		bytes = read(fromProg[0], buf, sizeof(buf)-1);
		if(bytes > 0) {
			if (tmpoutbuf)
				tmpoutbuf=g_realloc(tmpoutbuf,outpos+bytes);
			else
				tmpoutbuf=g_malloc(bytes);

			memcpy(tmpoutbuf+outpos, buf, bytes);
			outpos += bytes;
		}

		if (waitpid(progPID, &status, WNOHANG))
			break;
	}

	bytes = read(fromProg[0], buf, sizeof(buf)-1);
	while(bytes > 0) {
		if (tmpoutbuf)
			tmpoutbuf=g_realloc(tmpoutbuf,outpos+bytes);
		else
			tmpoutbuf=g_malloc(bytes);

		memcpy(tmpoutbuf+outpos, buf, bytes);
		outpos += bytes;
		bytes = read(fromProg[0], buf, sizeof(buf)-1);
	}
		
	if( ! out_closed)
		close(toProg[1]);
	close(fromProg[0]);
	signal(SIGPIPE, oldhandler);

	if (writeBytesLeft) {
		g_warning("failed to write all data to %s", argv[0]);
		g_free(tmpoutbuf);
		return -1;
	}

/* ignore return code for now */
#if 0
	waitpid(progPID, &status, 0);
	if (!WIFEXITED(status) || WEXITSTATUS(status)) {
		g_warning("getOutputFrom(): %s failed", argv[0]);
		g_free(tmpoutbuf);
		return -1;
	}
#endif

	*outbuf    = tmpoutbuf;
	*outbuflen = outpos;

	return 0;
}

gint
loadFileToBuf( gchar *file, guchar **bufout, gint *lenout )
{
	guchar buf[8193];
	guchar *out=NULL;
	guchar *ext=NULL;
	gzFile *f=NULL;
#ifdef HAVE_LIBBZ2
	BZFILE *bf=NULL;
#endif
	gint bytes, len=0;
	gint bz = 0;

	struct stat b;

	if (stat(file, &b))
		return -1;

	if (!S_ISREG(b.st_mode))
		return -1;

	ext = strrchr (file, '.');
	if (ext) {
		if (strcmp (ext, ".bz2") == 0)
			bz = 1;
	}

	switch (bz) {
	case 1:
#ifdef HAVE_LIBBZ2 
		if ((bf=bzopen(file, "r"))==NULL)
			return -1;

		bytes=bzread(bf, buf, 8192);
		break;
#endif
	default:
		if ((f=gzopen(file, "r"))==NULL)
			return -1;

		bytes=gzread(f, buf, 8192);
		break;
	}

	while (bytes > 0) {
		if (!out)
			out = g_malloc(bytes + 1); /* 1 for zero byte */
		else 
			out = g_realloc(out, len+bytes + 1);
		
		memcpy(out+len, buf, bytes);
		len += bytes;
		out[len] = '\0';
#ifdef HAVE_LIBBZ2
		if (bz == 1)
			bytes=bzread(bf, buf, 8192);
		else
#endif
			bytes=gzread(f, buf, 8192);
	}
#ifdef HAVE_LIBBZ2
	if (bz == 1)
		bzclose(bf);
	else
#endif
		gzclose(f);

	*bufout = out;
	if (lenout)
	        *lenout = len;
	return bytes;
}

void map_spaces_to_underscores( gchar *str )
{
  gchar *p;

  g_return_if_fail(str != NULL );
  for (p=str; *p; p++)
    switch (*p)
      {
      case ' ':
      case '\n':
      case '\t':
      case '`':
      case '\'':
      case '/':
      case '\\':
      case '"':
      case '.':
      case '!':
        *p = '_';
        break;
      }
}
