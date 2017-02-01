#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <X11/Xatom.h>
#include "gnorba.h"

CORBA_ORB _gnorba_gnome_orbit_orb;
static CORBA_Principal _gnorba_request_cookie = { 0, 0, NULL, CORBA_FALSE };

static char *_gnorba_get_cookie_reliably(const char *setme);
void         _gnome_gnorba_cookie_setup(Display *disp, Window rootwin);
extern void goad_register_arguments(void);

int _gnorba_corba_prio;

#ifndef ORBIT_USES_GLIB_MAIN_LOOP

static gboolean
orb_handle_connection(GIOChannel *source, GIOCondition cond,
		      GIOPConnection *cnx)
{

	/* The best way to know about an fd exception is if select()/poll()
	 * tells you about it, so we just relay that information on to ORBit
	 * if possible
	 */
	
	if(cond & (G_IO_HUP|G_IO_NVAL|G_IO_ERR))
		giop_main_handle_connection_exception(cnx);
	else
		giop_main_handle_connection(cnx);
	
	return TRUE;
}

static void
orb_add_connection(GIOPConnection *cnx)
{
	int tag;
	GIOChannel *channel;
	
	channel = g_io_channel_unix_new(GIOP_CONNECTION_GET_FD(cnx));
	tag = g_io_add_watch_full   (channel, _gnorba_corba_prio,
				     G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL, 
				     (GIOFunc)orb_handle_connection,
				     cnx, NULL);
	g_io_channel_unref (channel);
	
	cnx->user_data = GUINT_TO_POINTER (tag);
}

static void
orb_remove_connection(GIOPConnection *cnx)
{
	g_source_remove(GPOINTER_TO_UINT (cnx->user_data));
	cnx->user_data = GINT_TO_POINTER (-1);
}

#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */

static ORBit_MessageValidationResult
gnome_ORBit_request_validate(CORBA_unsigned_long request_id,
			     CORBA_Principal *principal,
			     CORBA_char *operation)
{
	if (principal->_length == _gnorba_request_cookie._length
	    && !(principal->_buffer[principal->_length - 1])
	    && !strcmp(principal->_buffer, _gnorba_request_cookie._buffer))
		return ORBIT_MESSAGE_ALLOW_ALL;
	else
		return ORBIT_MESSAGE_BAD;
}

/*
 * I bet these will require porting sooner or later
 */
static void
get_exclusive_lock (int fd)
{
	/* flock (fd, LOCK_EX); */
	struct flock lbuf;

	lbuf.l_type = F_WRLCK;
	lbuf.l_whence = SEEK_SET;
	lbuf.l_start = lbuf.l_len = 0L; /* Lock the whole file.  */
	fcntl (fd, F_SETLKW, &lbuf);
}

static void
release_lock (int fd)
{
	/* flock (fd, LOCK_UN); */
	struct flock lbuf;

	lbuf.l_type = F_UNLCK;
	lbuf.l_whence = SEEK_SET;
	lbuf.l_start = lbuf.l_len = 0L; /* Unlock the whole file.  */
	fcntl (fd, F_SETLKW, &lbuf);
}

/*
 * We assume that if we could get this far, then /tmp/orbit-$username is
 * secured (because of CORBA_ORB_init).
 */
static char *
_gnorba_get_cookie_reliably (const char *setme)
{
	struct passwd *pwent;
	char buf[64];
	char *random_string = NULL;
	char *name;
	int fd = -1;
	
	pwent = getpwuid(getuid());
	if(!pwent) {
		fprintf(stderr,_("Failed to get user details for UID %d (%s)\n"), getuid(),strerror(errno));
		goto out;
	}
	
	name = g_strconcat ("/tmp/orbit-", pwent->pw_name, "/cookie", NULL);
	
	if(setme) {
		
		/* Just write it into the file for reference purposes */
		fd = open (name, O_CREAT|O_WRONLY, S_IRUSR | S_IWUSR);
		
		if (fd < 0) {
                        fprintf(stderr, _("Failed to open CORBA cookie file (%s) for writing: %s\n"), name, strerror(errno));
			goto out;
                }
                        
		get_exclusive_lock(fd);

		if (write(fd, setme, strlen(setme)) < 0) {
                        fprintf(stderr, _("Failed to write CORBA cookie file (%s): %s\n"), name, strerror(errno));
                        goto out;
                }
                
		release_lock(fd);
		random_string = g_strdup(setme);
		
	} else {
		
		buf [sizeof(buf)-1] = '\0';
		
		/*
		 * Create the file exclusively with permissions rw for the
		 * user.  if this fails, it means the file already existed
		 */
		fd = open (name, O_CREAT|O_EXCL|O_WRONLY, S_IRUSR | S_IWUSR);
		
		if(fd >= 0) {
			unsigned int i;
			
			get_exclusive_lock (fd);
			srand (time (NULL));
			for (i = 0; i < sizeof (buf)-1; i++)
				buf [i] = (rand () % (126-33)) + 33;
			
			if(write(fd, buf, sizeof(buf)-1) < (sizeof(buf)-1)) {
                                fprintf(stderr, _("Failed to write CORBA cookie to `%s': %s\n"), name, strerror(errno));
				goto out;
                        }
			
			release_lock(fd);
		} else if(fd < 0) {
			int i;
			fd = open(name, O_RDONLY);

                        if (fd < 0) {
                                fprintf(stderr, _("Failed to open CORBA cookie file `%s': %s\n"), name, strerror(errno));
				goto out;
                        }
                        
			i = read(fd, buf, sizeof(buf)-1);
			if(i < 0) {
                                fprintf(stderr, _("Failed to read CORBA cookie file `%s': %s\n"), name, strerror(errno));
				goto out;
                        }
			buf[i] = '\0';
		}
		
		random_string = g_strdup(buf);
	}
	
 out:
	if(fd >= 0)
		close(fd);
	g_free(name);
	
	return random_string;
}

static const char *
_gnorba_cookie_setup(const char *setme)
{
  g_return_val_if_fail(!_gnorba_request_cookie._buffer, _gnorba_request_cookie._buffer);

  _gnorba_request_cookie._buffer = _gnorba_get_cookie_reliably (setme);
		
  if (_gnorba_request_cookie._buffer == NULL ||
      *_gnorba_request_cookie._buffer == '\0') {
          fprintf(stderr, _("Failed to obtain CORBA authentication cookie, exiting\n"));
          exit(1);
  }
		
  _gnorba_request_cookie._length = strlen(_gnorba_request_cookie._buffer) + 1;

  ORBit_set_request_validation_handler(&gnome_ORBit_request_validate);
  ORBit_set_default_principal(&_gnorba_request_cookie);

  return _gnorba_request_cookie._buffer;
}

/* GUI version of the above function.
 * description:
 * Grabs the X server to guarantee mutual exclusion
 *
 * Tries to retrieve the cookie from the display.
 *
 * If the cookie is already there, we ungrab as we won't be needing to touch
 * the display any more WRT cookies.
 *
 * Do the cookie getting/setting stuff with the local file.
 *
 * If a cookie wasn't already set on $DISPLAY, then set it from the value
 * that is now in the local file and ungrab the server.
 * 
 * note: Please be extremely careful when playing around with the grab
 * behaviour.  It is not simplified in order to minimize the amount of
 * time the display is grabbed.
 */
void
_gnome_gnorba_cookie_setup(Display *disp, Window rootwin)
{
  int ret_fmt;
  Atom prop, ret_type;
  unsigned long ret_nitems, ret_bytes_after;
  unsigned char *ret_prop;
  const char *setval;

  prop = XInternAtom(disp, "GNOME_SESSION_CORBA_COOKIE", False);
  XGrabServer(disp);
  XGetWindowProperty(disp, rootwin, prop, 0, 9999, False, XA_STRING,
		     &ret_type, &ret_fmt, &ret_nitems, &ret_bytes_after,
		     &ret_prop);

  if(ret_type == None)
    ret_prop = NULL;
  else
    {
      XUngrabServer(disp);
      XFlush(disp);
    }

  setval = _gnorba_cookie_setup(ret_prop);

  if(ret_prop == NULL) {
    XChangeProperty(disp, rootwin, prop, XA_STRING, 8, PropModeReplace,
		    setval, strlen(setval));
    XUngrabServer(disp);
    XFlush(disp);
  } else {
    XFree(ret_prop); /* XFree barfs on NULL ptrs */
  }
}

/**
 * gnorba_CORBA_init:
 * @argc: argc pointer from the application
 * @argv: argv from the application
 * @flags: GnorbaInitFlags that control the way we startup
 * @ev: A CORBA_Environment for catching CORBA errors
 *
 * Sets up the ORBit connection add/remove function pointers
 * to our routines, which inform the gtk main loop about
 * the CORBA connection fd's.
 * 
 * Calls gnome_init and CORBA_ORB_init with the specified params.
 *
 * Sets up a cookie for requests.
 *
 * Returns the CORBA_ORB initialized by the GNORBA libraries.
 */
CORBA_ORB
gnorba_CORBA_init(int *argc, char **argv,
		  GnorbaInitFlags flags,
		  CORBA_Environment *ev)
{
	CORBA_ORB retval;

#ifndef ORBIT_USES_GLIB_MAIN_LOOP
	IIOPAddConnectionHandler = orb_add_connection;
	IIOPRemoveConnectionHandler = orb_remove_connection;
#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */

	if (flags & GNORBA_INIT_CORBA_PRIO_HIGH)
		_gnorba_corba_prio = G_PRIORITY_DEFAULT;
	else
		_gnorba_corba_prio = G_PRIORITY_LOW;
	
	_gnorba_gnome_orbit_orb = retval = CORBA_ORB_init(argc, argv, "orbit-local-orb", ev);
	
	if(!(flags & GNORBA_INIT_DISABLE_COOKIES))
		_gnorba_cookie_setup(NULL);
	
	return retval;
}

/**
 * gnome_CORBA_ORB:
 *
 * Returns the CORBA_ORB initialized by the GNORBA libraries.
 */
CORBA_ORB
gnome_CORBA_ORB(void)
{
	return _gnorba_gnome_orbit_orb;
}

