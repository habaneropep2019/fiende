#include "config.h"
#if defined (__hpux) && ! defined (_XOPEN_SOURCE_EXTENDED)
#   define _XOPEN_SOURCE_EXTENDED 1
#   define WE_DEFINED_XOPEN_SOURCE_EXTENDED 1
#endif
#include "iiop-endianP.h"
#ifdef WE_DEFINED_XOPEN_SOURCE_EXTENDED
#   undef _XOPEN_SOURCE_EXTENDED
#endif
#include "IIOP.h"
#include "IIOP-private.h"
#include "giop-msg-buffer.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#if defined (__hpux) && ! defined (_XOPEN_SOURCE_EXTENDED)
#   define _XOPEN_SOURCE_EXTENDED 1
#endif
#include <arpa/inet.h>
#include <netdb.h>
#ifdef WE_DEFINED_XOPEN_SOURCE_EXTENDED
#   undef _XOPEN_SOURCE_EXTENDED
#endif
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <syslog.h>

#if defined(HAVE_TCPD_H) && defined(HAVE_HOSTS_ACCESS)
#include <tcpd.h>
#endif

#if 0
#include <malloc.h>

static struct mallinfo mi1, mi2;

#define AM() mi1 = mallinfo();
#define PM(x) mi2 = mallinfo(); printf(x ": used %d, now %d\n", \
mi2.uordblks - mi1.uordblks, mi2.uordblks);
#endif

#if defined(HAVE_POLL) && defined(I_WANT_POLL)
#define USE_POLL
#else
#undef USE_POLL
#endif

#ifdef HAVE_POLL
#include <sys/poll.h>
#endif

#ifndef SUN_LEN
/* This system is not POSIX.1g.  */
#define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)  \
                      + strlen ((ptr)->sun_path))
#endif

void (*IIOPAddConnectionHandler)(GIOPConnection *newcnx) = NULL;
void (*IIOPRemoveConnectionHandler)(GIOPConnection *oldcnx) = NULL;
void (*IIOPIncomingMessageHandler)(GIOPRecvBuffer *recv_buffer) = NULL;

static void giop_connection_add_to_list         (GIOPConnection *cnx);
static void giop_connection_remove_from_list    (GIOPConnection *cnx);

static void iiop_init                           (void);
static void iiop_connection_server_accept       (GIOPConnection *connection);
static void iiop_connection_destroy   		(IIOPConnection *connection);
static IIOPConnection *iiop_connection_new      (const char *host, gushort port);
static IIOPConnection *iiop_connection_unix_new (const char *sockpath);
static void iiop_unlink_unix_sockets            (void);

DEFINE_LOCK(giop_connection_list);
GIOPConnectionList giop_connection_list;
static GSList *iiop_unix_socket_list = NULL;

#if defined(HAVE_HOSTS_ACCESS) && defined (HAVE_TCPD_H)
static const char *argv0_val = NULL;
#endif

/*
 * giop_init
 *
 *    Inputs: None
 *    Outputs: None
 *
 *    Side effects: Initializes giop_connection_list
 *    Global data structures used: giop_connection_list
 *
 *    Description: Initializes giop_connection_list. Calls
 *                 giop_message_buffer_init() to initialize the
 *                 message_buffer subsystem. Calls iiop_init()
 *		   to perform IIOP-specific initialization.
 */

void giop_init(const char *argv0)
{
  struct sigaction mypipe;
  g_assert(sizeof(GIOPMessageHeader) == 12);

#if defined(HAVE_HOSTS_ACCESS) && defined (HAVE_TCPD_H)
  argv0_val = g_strdup(g_basename(argv0)); /* For TCP wrappers */
#endif

  memset(&mypipe, '\0', sizeof(mypipe));
  mypipe.sa_handler = SIG_IGN;

  sigaction(SIGPIPE, &mypipe, NULL);

  giop_message_buffer_init();

  INIT_LOCK(giop_connection_list);

  giop_connection_list.list = NULL;
  giop_connection_list.connection_list_changed = FALSE;

#ifdef USE_POLL
  giop_connection_list.pollset = g_array_new(FALSE, FALSE,
					     sizeof(struct pollfd));
#else
  FD_ZERO(&giop_connection_list.selectset_rd);
  FD_ZERO(&giop_connection_list.selectset_ex);
#endif

  giop_connection_list.fd_to_connection_mapping = g_ptr_array_new();

  /*
   * This also needs to do any transport-specific initialization
   * as appropriate
   */
  iiop_init();
}

/*** giop_connection_init
 *
 *   Inputs: 'giop_connection' - memory region allocated for use as a
 *                               GIOPConnection.
 *           'cnxclass'        - the class of connection that will be stored
 *                               here (SERVER, CLIENT)
 *
 *   Outputs: None
 *
 *   Side effects: Initializes 'giop_connection'.
 *
 *   Description: Basic setup of a GIOPConnection.
 *                Sets is_valid to FALSE because it is the responsibility of
 *	          the transport-specific initialization routine to make
 *	          a connection valid.
 */

static void giop_connection_init(GIOPConnection *giop_connection,
				 GIOPConnectionClass cnxclass)
{
  giop_connection->connection_type = GIOP_CONNECTION_NONE;
  giop_connection->refcount = 0;
  giop_connection->connection_class = cnxclass;
  giop_connection->is_valid = FALSE;
  giop_connection->is_auth = FALSE;
  giop_connection->was_initiated = FALSE;
}

/*
 * giop_connection_free
 *    Inputs: 'connection'
 *    Outputs: None
 *    Side effects: Makes the 'connection' invalid as a GIOPConnection
 *                  and as a gpointer.
 *
 *    Description: Calls giop_connection_remove_from_list() to
 *                 stop the connection from being used for incoming.
 *
 *	           If a transport-specific finalization function has
 *	           been provided, call it.
 *	           
 *	           Free the memory block at '*connection'.
 *
 */
void giop_connection_free(GIOPConnection *connection)
{
  g_return_if_fail(connection != NULL);

  giop_connection_remove_from_list(connection);

  if(connection->is_valid && connection->destroy_func)
    connection->destroy_func(connection);

  connection->is_valid = FALSE;

  if(connection->incoming_msg) {
    GIOPRecvBuffer *buf;

    buf = connection->incoming_msg;
    connection->incoming_msg = NULL;
    giop_recv_buffer_unuse(buf);
  }

  g_free(connection);
}

/*
 * giop_connection_list_recreate
 *
 *    Inputs: None
 *    Outputs: None
 *
 *    Side effects: giop_connection_list changes.
 *
 *    Global data structures used: giop_connection_list
 *
 *    Description:
 *         When new connections are added to giop_connection_list.list,
 *     	   the data structures passed to poll() or select() (OS-dependant)
 *     	   must be recreated to match this list.
 *     	   
 *     	   [We do this at add-connection/remove-connection time
 *     	    instead of every time a poll/select is done in order to
 *     	    speed things up a little]
 *     	   
 *     	   This function reinitializes the OS-specific file
 *     	   descriptor data structure and then adds all the file
 *     	   descriptors in the list to it.
 *     	  
 *     	   It also regenerates the array that maps file descriptors
 *     	   into GIOPConnection*'s
 *
 */
static void
giop_connection_list_recreate(void)
{
  int curfd;
  GList *item;
  GIOPConnection *cnx;
#ifdef USE_POLL
  struct pollfd new_poll;

  new_poll.revents = 0;
#endif

  giop_connection_list.max_fd = 0;
  for(item = giop_connection_list.list; item; item = g_list_next(item))
    {
      cnx = item->data;
      curfd = GIOP_CONNECTION_GET_FD(cnx);

      if(curfd > giop_connection_list.max_fd)
	giop_connection_list.max_fd = curfd;
  }

  g_ptr_array_set_size(giop_connection_list.fd_to_connection_mapping,
		       giop_connection_list.max_fd + 1);

#ifdef USE_POLL
  g_array_set_size(giop_connection_list.pollset, 0);
#else
  FD_ZERO(&giop_connection_list.selectset_rd);
  FD_ZERO(&giop_connection_list.selectset_ex);
#endif

  for(item = giop_connection_list.list; item; item = g_list_next(item))
    {
      cnx = item->data;
      curfd = GIOP_CONNECTION_GET_FD(cnx);

      giop_connection_list.fd_to_connection_mapping->pdata[curfd] = cnx;

#     ifdef USE_POLL
      new_poll.fd = curfd;
      new_poll.events = POLLIN|POLLPRI;
      g_array_append_val(giop_connection_list.pollset, 
			 new_poll);
#     else
      FD_SET(curfd, &giop_connection_list.selectset_rd);
      FD_SET(curfd, &giop_connection_list.selectset_ex);
#     endif
  }
}

/*
 * giop_connection_add_to_list
 *
 *    Inputs: 'cnx' - a GIOPConnection that the user wishes added to the list
 *    Outputs: None
 *
 *    Side effects: Modifies giop_connection_list
 *    Global data structures used: giop_connection_list
 *    Bugs: Does not check for duplicate additions.
 *
 *    Description:
 *         Adds a connection to the list of active connections.
 */
static void
giop_connection_add_to_list(GIOPConnection *cnx)
{
  g_return_if_fail(cnx->is_valid == FALSE);

  cnx->is_valid = TRUE;

  GET_LOCK(giop_connection_list);
  giop_connection_list.list = g_list_prepend(giop_connection_list.list, cnx);

  giop_connection_list_recreate();

  RELEASE_LOCK(giop_connection_list);

  if(IIOPAddConnectionHandler)
    IIOPAddConnectionHandler(cnx);

  giop_connection_ref(cnx);
}

/*
 * giop_connection_remove_from_list
 *
 *    Inputs: 'cnx' - a GIOPConnection that the user wishes
 *    Outputs: None
 *
 *    Side effects: Modifies giop_connection_list
 *    Global data structures used: giop_connection_list
 *
 *    Description:
 *         Removes a connection from the list of active connections.
 *         Calls the library user's "I removed connection" handler if it
 *         exists.
 *
 *    Bugs: Does not check for duplicate removals. This may not be "bad" though.
 */
void
giop_connection_remove_from_list(GIOPConnection *cnx)
{
  GList *link;

  GET_LOCK(giop_connection_list);

  link = g_list_find(giop_connection_list.list, cnx);

  if(!link)
    goto out;

  if(IIOPRemoveConnectionHandler && cnx->is_valid)
    IIOPRemoveConnectionHandler(cnx);

  giop_connection_list.list = g_list_remove_link(giop_connection_list.list,
						 link);
  g_list_free_1(link);

  giop_connection_unref(cnx);

  giop_connection_list_recreate();
 out:
  RELEASE_LOCK(giop_connection_list);
}

/************************************************
 * Routines specific to the IIOP/IPv4 transport *
 ************************************************/

/*
 * iiop_init
 *
 *    Inputs: None
 *    Outputs: None
 *
 *    Side effects: Initializes iiop_unix_socket_list
 *    Global data structures used: iiop_unix_socket_list
 *
 *    Description: Initializes iiop_unix_socket_list.
 *                 Registers Unix domain sockets for
 *                 removal at server termination.
 */
static void
iiop_init(void)
{
  g_atexit(iiop_unlink_unix_sockets);
}

/*
 * iiop_connection_init
 *
 *    Inputs: 'connection' - a memory region that needs to be initialized as
 *                           an 'IIOPConnection'.
 *
 *    Side effects: initializes 'connection'
 *
 *    Description: Performs the IIOP-specific initialization of an
 *                 IIOPConnection. giop_connection_init is called.
 *
 */
void
iiop_connection_init(IIOPConnection *connection,
		     GIOPConnectionClass cnxclass,
		     IIOPConnectionType iioptype)
{
  giop_connection_init(GIOP_CONNECTION(connection), cnxclass);

  GIOP_CONNECTION(connection)->connection_type =
    GIOP_CONNECTION_IIOP;

  GIOP_CONNECTION(connection)->destroy_func =
    (void (*)(GIOPConnection *))iiop_connection_destroy;

  connection->icnxtype = iioptype;
}

/*
 * iiop_connection_from_fd
 *
 *    Inputs: 'fd' - a file descriptor that attention should be paid to
 *    Outputs: 'fd_cnx' - the created connection 
 *
 *    Description: This is intended to be used on a file descriptor
 *	           that has been accept()'d. It creates the connection
 *	           and fills in the connection information, then adds
 *	           it to the active list.
 */
IIOPConnection *
iiop_connection_from_fd(int fd, IIOPConnection *parent)
{
  IIOPConnection *fd_cnx;
  struct hostent *hent;
  socklen_t n;

  g_assert(fd >= 0);

  fd_cnx = g_new0(IIOPConnection, 1);

  iiop_connection_init(fd_cnx, GIOP_CONNECTION_CLIENT, parent->icnxtype);

  GIOP_CONNECTION(fd_cnx)->fd = fd;

  switch(parent->icnxtype) {
  case IIOP_IPV4:
    n = sizeof(struct sockaddr_in);
    if(getpeername(GIOP_CONNECTION_GET_FD(fd_cnx), (struct sockaddr *)&fd_cnx->u.ipv4.location, &n))
      {
	fd_cnx->u.ipv4.hostname = g_strdup("");
      }
    else
      {
	hent = gethostbyaddr((const char *)&fd_cnx->u.ipv4.location.sin_addr.s_addr, 4, AF_INET);
	if(hent)
	  {
	    fd_cnx->u.ipv4.hostname = g_strdup(hent->h_name);
	  }
	else
	  {
	    fd_cnx->u.ipv4.hostname = g_strdup(inet_ntoa(*((struct in_addr *)&fd_cnx->u.ipv4.location.sin_addr)));
	  }
      }
    break;

  case IIOP_USOCK:
    n = sizeof(struct sockaddr_un);
    fd_cnx->u.usock.sun_family = AF_UNIX;
    getpeername(GIOP_CONNECTION_GET_FD(fd_cnx),
	(struct sockaddr *)&fd_cnx->u.usock, &n);
    break;

#ifdef HAVE_IPV6
  case IIOP_IPV6:
    n = sizeof(struct sockaddr_in6);
    getpeername(GIOP_CONNECTION_GET_FD(fd_cnx),
	(struct sockaddr *)&fd_cnx->u.ipv6.location, &n);
    hent = gethostbyaddr((const char *)&fd_cnx->u.ipv6.location.sin6_addr, 
			 sizeof(fd_cnx->u.ipv6.location.sin6_addr), AF_INET6);
    fd_cnx->u.ipv6.hostname = g_strdup(hent->h_name);
    break;
#endif

  default:
    g_error("Unsupported connection type %d", parent->icnxtype);
  }

  fcntl(GIOP_CONNECTION_GET_FD(fd_cnx), F_SETFD,
	fcntl(GIOP_CONNECTION_GET_FD(fd_cnx), F_GETFD, 0)
	| FD_CLOEXEC);
  fcntl(GIOP_CONNECTION_GET_FD(fd_cnx), F_SETFL,
	fcntl(GIOP_CONNECTION_GET_FD(fd_cnx), F_GETFL, 0)
	| O_NONBLOCK);

  ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug,
	      "iiop_connection_from_fd connect [%d]\n",
	      GIOP_CONNECTION_GET_FD(fd_cnx));

  giop_connection_add_to_list(GIOP_CONNECTION(fd_cnx));

  return fd_cnx;
}

/*
 * iiop_connection_server
 *
 *   Outputs: 'server_cnx'
 *
 *    Description: Creates a special IIOPConnection on which incoming
 *                 connections come.
 */
IIOPConnection *
iiop_connection_server(void)
{
  struct hostent *hent;
  char hn_tmp[65];
  socklen_t n;
  IIOPConnection *server_cnx = g_new0(IIOPConnection, 1);

  iiop_connection_init(server_cnx, GIOP_CONNECTION_SERVER, IIOP_IPV4);

  server_cnx->is_serversock = TRUE;
  GIOP_CONNECTION(server_cnx)->fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if(GIOP_CONNECTION_GET_FD(server_cnx) < 0) {
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_server: socket_error: %s\n", strerror(errno));
    goto failed;
  }

  server_cnx->u.ipv4.location.sin_family = AF_INET;
  server_cnx->u.ipv4.location.sin_addr.s_addr = INADDR_ANY;
  bind(GIOP_CONNECTION_GET_FD(server_cnx),
       (struct sockaddr *)&server_cnx->u.ipv4.location,
       sizeof(struct sockaddr_in));

  fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_SETFD,
	fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_GETFD, 0)
	| FD_CLOEXEC);
  fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_SETFL,
	fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_GETFL, 0)
	| O_NONBLOCK);

  n = sizeof(struct sockaddr_in);
  getsockname(GIOP_CONNECTION_GET_FD(server_cnx),
	      (struct sockaddr *)&server_cnx->u.ipv4.location, &n);

  gethostname(hn_tmp, sizeof(hn_tmp) - 1);

  hent = gethostbyname(hn_tmp);
  if(hent) 
    {
      if (strchr (hent->h_name, '.'))
	server_cnx->u.ipv4.hostname = g_strdup(hent->h_name);
      else
	{
	  struct in_addr * addr = (struct in_addr *) hent->h_addr_list[0];
	  g_assert (hent->h_length == sizeof (struct in_addr) && addr);
	  server_cnx->u.ipv4.hostname = g_strdup (inet_ntoa (*addr));
	}
    }
  else
    server_cnx->u.ipv4.hostname = g_strdup(hn_tmp);

  listen(GIOP_CONNECTION_GET_FD(server_cnx), 5);

  giop_connection_add_to_list(GIOP_CONNECTION(server_cnx));

  return server_cnx;

failed:
  close(GIOP_CONNECTION_GET_FD(server_cnx));
  GIOP_CONNECTION(server_cnx)->fd = -1;
  giop_connection_free(GIOP_CONNECTION(server_cnx));
  server_cnx = NULL;
  /*
   * FIXME: GET_LOCK and DEFINE_LOCK never called for server_cnx
  RELEASE_LOCK(server_cnx);
   */
  return NULL;
}

/*
 * iiop_connection_server_ipv6
 *   Outputs: 'server_cnx'
 *
 *    Description: Create a special IIOPConnection on which incoming
 *                 connections come.
 */
IIOPConnection *
iiop_connection_server_ipv6(void)
{
#ifdef HAVE_IPV6
  struct hostent *hent, *hent2;

  char hn_tmp[65];
  int n;
  IIOPConnection *server_cnx;

  g_error("IPv6 support is baroquen! (Actually just never worked)");

  server_cnx = g_new0(IIOPConnection, 1);
  
  iiop_connection_init(server_cnx, GIOP_CONNECTION_SERVER, IIOP_IPV6);
  
  server_cnx->is_serversock = TRUE;
  GIOP_CONNECTION(server_cnx)->fd = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);

  if(GIOP_CONNECTION_GET_FD(server_cnx) < 0) {
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_server_ipv6: socket_error: %s\n", strerror(errno));
    goto failed;
  }

  server_cnx->u.ipv6.location.sin6_family = AF_INET6;
  bind(GIOP_CONNECTION_GET_FD(server_cnx),
       (struct sockaddr *)&server_cnx->u.ipv6.location,
       sizeof(struct sockaddr_in6));
  
  fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_SETFD,
	fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_GETFD, 0)
	| FD_CLOEXEC);
  fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_SETFL,
	fcntl(GIOP_CONNECTION_GET_FD(server_cnx), F_GETFL, 0)
	| O_NONBLOCK);

  n = sizeof(struct sockaddr_in6);
  getsockname(GIOP_CONNECTION_GET_FD(server_cnx), &server_cnx->u.ipv6.location, &n);

  gethostname(hn_tmp, sizeof(hn_tmp) - 1);

  hent = gethostbyname(hn_tmp);
  if(hent) {
    hent2 = gethostbyaddr(hent->h_addr, sizeof(server_cnx->u.ipv6.location.sin6_addr), AF_INET6);
    if(hent2)
      server_cnx->hostname = g_strdup(hent2->h_name);
    else
      server_cnx->hostname = g_strdup(hn_tmp);
  } else
    server_cnx->hostname = g_strdup(hn_tmp);

  listen(GIOP_CONNECTION_GET_FD(server_cnx), 5);

  giop_connection_add_to_list(GIOP_CONNECTION(server_cnx));

  return server_cnx;

failed:
  close(GIOP_CONNECTION_GET_FD(server_cnx));
  GIOP_CONNECTION(server_cnx)->fd = -1;
  giop_connection_free(GIOP_CONNECTION(server_cnx));
  server_cnx = NULL;
  /*
   * FIXME: GET_LOCK and DEFINE_LOCK never called for server_cnx
  RELEASE_LOCK(server_cnx);
   */
#endif
  return NULL;
}

/*
 * iiop_connection_server_unix
 *
 *   Outputs: 'server_cnx_unix'
 *
 *    Side effects: Initializes 'server_cnx_unix' if not initialized.
 *
 *    Description: Return a special IIOPConnection on which incoming connections
 *                 come. If not already initialized, it creates the connection,
 *	           otherwise it returns the existing one.
 *	           This is
 */
IIOPConnection *
iiop_connection_server_unix(const char *sockpath)
{
  IIOPConnection *server_cnx_unix;

  g_assert(sockpath && *sockpath);

  server_cnx_unix = g_new0(IIOPConnection, 1);

  iiop_connection_init(server_cnx_unix, GIOP_CONNECTION_SERVER, IIOP_USOCK);

  server_cnx_unix->is_serversock = TRUE;
  GIOP_CONNECTION(server_cnx_unix)->fd = socket(AF_UNIX, SOCK_STREAM, 0);

  if(GIOP_CONNECTION_GET_FD(server_cnx_unix) < 0) {
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_server_unix: socket_error: %s\n", strerror(errno));
    goto failed;
  }

  strcpy(server_cnx_unix->u.usock.sun_path, sockpath);

  server_cnx_unix->u.usock.sun_family = AF_UNIX;
  if(bind(GIOP_CONNECTION_GET_FD(server_cnx_unix),
	  (struct sockaddr *)&server_cnx_unix->u.usock,
	  SUN_LEN(&server_cnx_unix->u.usock)) != 0) {
    /* see the comment in iiop_connection_destroy switch as to why we
       close it here. bad hack */
    close(GIOP_CONNECTION_GET_FD(server_cnx_unix));
    GIOP_CONNECTION(server_cnx_unix)->fd = -1;
    goto failed;
  }

  fcntl(GIOP_CONNECTION_GET_FD(server_cnx_unix), F_SETFD,
	fcntl(GIOP_CONNECTION_GET_FD(server_cnx_unix), F_GETFD, 0)
	| FD_CLOEXEC);
  fcntl(GIOP_CONNECTION_GET_FD(server_cnx_unix), F_SETFL,
	fcntl(GIOP_CONNECTION_GET_FD(server_cnx_unix), F_GETFL, 0)
	| O_NONBLOCK);

  if(listen(GIOP_CONNECTION_GET_FD(server_cnx_unix), 5) != 0)
    goto failed;

  giop_connection_add_to_list(GIOP_CONNECTION(server_cnx_unix));
  iiop_unix_socket_list = g_slist_prepend(iiop_unix_socket_list,
					  server_cnx_unix);

  /*
   * FIXME: GET_LOCK and DEFINE_LOCK never called for server_cnx_unix
  RELEASE_LOCK(server_cnx_unix);
   */

  return server_cnx_unix;

failed:
  close(GIOP_CONNECTION_GET_FD(server_cnx_unix));
  GIOP_CONNECTION(server_cnx_unix)->fd = -1;
  giop_connection_free(GIOP_CONNECTION(server_cnx_unix));
  server_cnx_unix = NULL;
  /*
   * FIXME: GET_LOCK and DEFINE_LOCK never called for server_cnx_unix
  RELEASE_LOCK(server_cnx_unix);
   */
  return NULL;
}

/*
 * iiop_unlink_unix_sockets(void)
 *
 *    Inputs: None
 *    Outputs: None
 *
 *    Side effects: Modifies iiop_unix_socket_list
 *    Global data structures used: iiop_unix_socket_list
 *
 *    Description:
 *         Unlinks any Unix server sockets created.
 *         Called during program termination.
 */
static void
iiop_unlink_unix_sockets(void)
{
  GSList *item;

  for (item = iiop_unix_socket_list;
       item; item = g_slist_next(item)) {
    GIOPConnection *cnx;

    cnx = GIOP_CONNECTION(item->data);
    if(cnx->connection_class == GIOP_CONNECTION_SERVER)
      unlink(IIOP_CONNECTION(cnx)->u.usock.sun_path);
  }

  if (iiop_unix_socket_list) {
    g_slist_free(iiop_unix_socket_list);
    iiop_unix_socket_list = NULL;
  }
}

/*
 * iiop_connection_get
 *
 * Inputs: 'host' - the hostname (or dotted quad) of the remote host that
 *                  will be connected
 *         'port' - the port number on the above host to connect to.
 *         'existing_only' - don't create a new connection if
 *                           an existing one with the specified host:port
 *                           doesn't exist.
 *
 * Outputs: 'cnx' - the connection to the specified host:port, or
 *                  NULL upon error.
 *
 * Description: Returns an IIOPConnection that is connected to the
 *              specified host:port. If a connection already exists to the
 *	 	host:port, just returns it. Otherwise, calls
 *	 	'iiop_connection_new' to create a new connection
 *	 	to host:port.
 */
IIOPConnection *
iiop_connection_get(const char *host, gushort port, gboolean existing_only)
{
  IIOPConnection *cnx = NULL, *tmp;
  GList *link;

  g_assert(host);
  g_assert(port);

  GET_LOCK(giop_connection_list);
  for(link = giop_connection_list.list; link; link = link->next)
    {
      tmp = IIOP_CONNECTION(link->data);
      if(GIOP_CONNECTION(tmp)->connection_type != GIOP_CONNECTION_IIOP)
	continue;

      if(!GIOP_CONNECTION(tmp)->is_valid)
	continue;

      if(GIOP_CONNECTION(tmp)->connection_class != GIOP_CONNECTION_CLIENT)
	continue;

      if(IIOP_CONNECTION(tmp)->icnxtype != IIOP_IPV4)
	continue;

      if(!strcmp(host, tmp->u.ipv4.hostname)
	 && htons(port) == tmp->u.ipv4.location.sin_port) {
	cnx = tmp;
	break;
      }
    }
  RELEASE_LOCK(giop_connection_list);

  if(!cnx && !existing_only)
    cnx = iiop_connection_new(host, port);

  return cnx;
}


/*
 * iiop_connection_new
 *
 * Inputs: same meanings as in 'iiop_connection_get'
 * Outputs: 'retval' - newly created IIOPConnection
 *
 * Description: Allocates and initializes a new IIOPConnection,
 *              turns 'host' into an IP address, and then makes a TCP
 *              connection to host:port. Adds it to the list of active
 *              connections.
 */
IIOPConnection *
iiop_connection_new(const char *host, gushort port)
{
  IIOPConnection *retval;

  g_return_val_if_fail(host != NULL && port != 0, NULL);

  retval = g_new0(IIOPConnection, 1);

  iiop_connection_init(retval, GIOP_CONNECTION_CLIENT, IIOP_IPV4);

  GIOP_CONNECTION(retval)->fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(GIOP_CONNECTION_GET_FD(retval) < 0) {
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_new: socket_error: %s\n", strerror(errno));
    goto failed;
  }

  retval->u.ipv4.hostname = g_strdup(host);

  retval->u.ipv4.location.sin_port = htons(port);
  retval->u.ipv4.location.sin_family = AF_INET;
  if(!inet_aton(host, &retval->u.ipv4.location.sin_addr))
    {
      struct hostent *hent;
      hent = gethostbyname(host);
      if(!hent) {
	/* a (char *)h_strerror(int) function would be nice here */
	ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_new: gethostbyname error: %d\n", h_errno);
	goto failed;
      }
      memcpy(&retval->u.ipv4.location.sin_addr, hent->h_addr, (size_t) sizeof(retval->u.ipv4.location.sin_addr));
    }
  if(connect(GIOP_CONNECTION_GET_FD(retval), (struct sockaddr *)&retval->u.ipv4.location, sizeof(retval->u.ipv4.location)) < 0) {
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_new: connect error: %s\n", strerror(errno));
    goto failed;
  }

  ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug,
	      "iiop_connection_new connect [%d] to %s:%d\n",
	      GIOP_CONNECTION_GET_FD(retval),
	      host, (guint)port);


  fcntl(GIOP_CONNECTION_GET_FD(retval), F_SETFD, FD_CLOEXEC);
  fcntl(GIOP_CONNECTION_GET_FD(retval), F_SETFL,
	fcntl(GIOP_CONNECTION_GET_FD(retval), F_GETFL, 0)
	| O_NONBLOCK);

  GIOP_CONNECTION(retval)->was_initiated = TRUE;
  GIOP_CONNECTION(retval)->is_auth = TRUE;

  giop_connection_add_to_list(GIOP_CONNECTION(retval));

  return retval;

failed:
  close(GIOP_CONNECTION_GET_FD(retval));
  GIOP_CONNECTION(retval)->fd = -1;
  giop_connection_free(GIOP_CONNECTION(retval));
  return NULL;
}

/*
 * iiop_connection_unix_get
 *
 *   Inputs: 'sockpath' - Of the format 'path'
 *
 *   Outputs: 'cnx' - the connection to the specified path, or
 *             NULL upon error.
 *
 *    Description: Returns an IIOPConnection that is connected to the
 *                 specified UNIX socket, if possible. If a connection
 *                 already exists, just returns it. Otherwise,
 *                 calls 'iiop_connection_unix_new' to create a new
 *                 connection to sockpath.
 */
IIOPConnection *
iiop_connection_unix_get(const char *sockpath, gboolean existing_only)
{
  IIOPConnection *cnx = NULL, *tmp;
  GList *link;

  GET_LOCK(giop_connection_list);
  for(link = giop_connection_list.list; link; link = link->next)
    {
      tmp = IIOP_CONNECTION(link->data);

      if(GIOP_CONNECTION(tmp)->connection_type != GIOP_CONNECTION_IIOP)
	continue;

      if(!GIOP_CONNECTION(tmp)->is_valid)
	continue;

      if(GIOP_CONNECTION(tmp)->connection_class != GIOP_CONNECTION_CLIENT)
	continue;

      if(IIOP_CONNECTION(tmp)->icnxtype != IIOP_USOCK)
	continue;

      if(!strcmp(sockpath, tmp->u.usock.sun_path)) {
	cnx = tmp;
	break;
      }
    }
  RELEASE_LOCK(giop_connection_list);

  if(!cnx && !existing_only)
    cnx = iiop_connection_unix_new(sockpath);

  return cnx;
}

/*
 * iiop_connection_unix_new
 *
 *   Inputs:
 *
 *    Outputs: 'retval' - newly created IIOPConnection, or NULL upon error
 *
 *    Description: Creates a connection to a UNIX socket (if possible)
 *                 Adds it to the list of active connections.
 */
static IIOPConnection *
iiop_connection_unix_new(const char *sockpath)
{
  IIOPConnection *retval;

  retval = g_new0(IIOPConnection, 1);

  retval->u.usock.sun_family = AF_UNIX;

  g_snprintf(retval->u.usock.sun_path,
	     sizeof(retval->u.usock.sun_path), "%s", sockpath);

  iiop_connection_init(retval, GIOP_CONNECTION_CLIENT, IIOP_USOCK);

  GIOP_CONNECTION(retval)->fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(GIOP_CONNECTION_GET_FD(retval) < 0) {
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_new: socket_error: %s\n", strerror(errno));
    goto failed;
  }

  if(connect(GIOP_CONNECTION_GET_FD(retval), (struct sockaddr *)&retval->u.usock, SUN_LEN(&retval->u.usock)) < 0) {
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug, "iiop_connection_new: connect error: %s\n", strerror(errno));
    goto failed;
  }

  GIOP_CONNECTION(retval)->was_initiated = TRUE;
  GIOP_CONNECTION(retval)->is_auth = TRUE;

  fcntl(GIOP_CONNECTION_GET_FD(retval), F_SETFD, FD_CLOEXEC);
  fcntl(GIOP_CONNECTION_GET_FD(retval), F_SETFL,
	fcntl(GIOP_CONNECTION_GET_FD(retval), F_GETFL, 0)
	| O_NONBLOCK);

  giop_connection_add_to_list(GIOP_CONNECTION(retval));

  ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug,
	      "iiop_connection_unix_new connect [%d] to %s\n",
	      GIOP_CONNECTION_GET_FD(retval),
	      sockpath);

  return retval;

failed:
  close(GIOP_CONNECTION_GET_FD(retval));
  GIOP_CONNECTION(retval)->fd = -1;
  giop_connection_free(GIOP_CONNECTION(retval));
  return NULL;
}

/*
 * iiop_connection_server_accept
 *    Inputs: 'connection' - a server IIOPConnection.
 *
 *    Description: Performs accept(), TCP wrapper, access checking and related
 *                 duties on a connection
 */
int allow_severity = LOG_INFO, deny_severity = LOG_NOTICE;

#if defined(HAVE_HOSTS_ACCESS) && defined(HAVE_TCPD_H)
DEFINE_LOCK(tcp_wrappers_usage);

#endif
static void
iiop_connection_server_accept(GIOPConnection *connection)
{
  struct sockaddr sock;
  socklen_t n;
  int newfd;
  GIOPConnection *newcnx;

  n = sizeof(sock);

  switch(IIOP_CONNECTION(connection)->icnxtype) {
  case IIOP_IPV4: sock.sa_family = AF_INET; break;
  case IIOP_USOCK: sock.sa_family = AF_UNIX; break;
  case IIOP_IPV6:
#ifdef HAVE_IPV6
    sock.sa_family = AF_INET6;
#endif
    break;
  }

  newfd = accept(GIOP_CONNECTION_GET_FD(connection), &sock, &n);

#if defined(HAVE_HOSTS_ACCESS) && defined(HAVE_TCPD_H)
  /* tcp wrappers access checking */
  switch(IIOP_CONNECTION(connection)->icnxtype) {
  case IIOP_IPV4:
    {
      struct request_info request;

      GET_LOCK(tcp_wrappers_usage);

      request_init(&request, RQ_DAEMON, argv0_val, RQ_FILE, newfd, 0);

      fromhost(&request);
      if(!hosts_access(&request)) {
	syslog(deny_severity, "[orbit] refused connect from %s", eval_client(&request));
	close(newfd); newfd = -1;
      } else
	syslog(allow_severity, "[orbit] connect from %s", eval_client(&request));

      RELEASE_LOCK(tcp_wrappers_usage);
    }
    break;
  default:
    /* No access controls for these transports */
    break;
  }
#endif

  if(newfd >= 0) {
    newcnx = GIOP_CONNECTION(iiop_connection_from_fd(newfd,
						     IIOP_CONNECTION(connection)));
#ifdef GIOP_INTERNAL_DEBUG
    g_warning ("New connection '%p'", newcnx);
#endif
    GIOP_CONNECTION(newcnx)->orb_data = connection->orb_data;
    switch(IIOP_CONNECTION(connection)->icnxtype) {
    case IIOP_USOCK: newcnx->is_auth = TRUE; break;
    default:
      break;
    }
  }
}

/*
 * iiop_connection_destroy
 *
 *    Inputs: 'iiop_connection' - an IIOPConnection to be finalized
 *
 *    Side effects: invalidates 'iiop_connection' for use as an IIOPConnection
 *
 *    Description: Performs the IIOP-specific parts of connection shutdown,
 *    including sending a CLOSECONNECTION message to the remote side.
 */
static void
iiop_connection_destroy(IIOPConnection *iiop_connection)
{
  const GIOPMessageHeader mh = {"GIOP", {1,0}, FLAG_ENDIANNESS,
				GIOP_CLOSECONNECTION, 0};

  switch(iiop_connection->icnxtype) {
  case IIOP_IPV4:
    ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug,
		"iiop_connection_destroy connect [%d] of %s:%d\n",
		GIOP_CONNECTION_GET_FD(iiop_connection),
		iiop_connection->u.ipv4.hostname,
		ntohs(iiop_connection->u.ipv4.location.sin_port));
    g_free(iiop_connection->u.ipv4.hostname);
    break;
  case IIOP_IPV6:
#ifdef HAVE_IPV6
    g_free(iiop_connection->u.ipv6.hostname);
#else
    g_warning("IPv6 unsupported, can't free it!");
#endif
    break;
  case IIOP_USOCK:
    /* why do we check if fd is > 0 here?
       the orb code tries to reuse existing socket connection points.
       If binding to any of those fails because another process is using it,
       we don't want to unlink the other server's socket!
       if the bind fails, iiop_connection_server_unix closes the fd for us */
    if(GIOP_CONNECTION(iiop_connection)->connection_class == GIOP_CONNECTION_SERVER
       && GIOP_CONNECTION(iiop_connection)->fd >= 0)
      unlink(iiop_connection->u.usock.sun_path);
    break;
  default:
    break;
  }

  if(GIOP_CONNECTION_GET_FD(iiop_connection) >= 0) {
    if(GIOP_CONNECTION(iiop_connection)->is_valid
       && !GIOP_CONNECTION(iiop_connection)->was_initiated)
      {
	write(GIOP_CONNECTION_GET_FD(iiop_connection), &mh, sizeof(mh));
      }

    shutdown(GIOP_CONNECTION_GET_FD(iiop_connection), 2);
    close(GIOP_CONNECTION_GET_FD(iiop_connection));
    GIOP_CONNECTION(iiop_connection)->fd = -1;
  }
}

static int giop_nloops = 0;

void giop_main_quit(void) { giop_nloops--; }

void
giop_main(void)
{
  int looplevel;

  looplevel = ++giop_nloops;

  while(giop_nloops > 0) {

    giop_main_iterate(TRUE);

    if(giop_nloops != looplevel) {
      giop_nloops = --looplevel;
      return;
    }
  }
}

GIOPRecvBuffer *
giop_main_next_message(gboolean blocking)
{
  return giop_main_next_message_2(blocking, NULL);
}

GIOPRecvBuffer *
giop_main_next_message_2(gboolean blocking,
			 GIOPConnection *monitor)
{
    GIOPConnection *connection;
    GIOPRecvBuffer *recv_buffer = NULL;

    do {
      recv_buffer = giop_received_list_pop();
      if(recv_buffer)
	break;

      connection = giop_check_connections(blocking);

      if(!connection)
	return NULL;

      if(GIOP_CONNECTION_GET_FD(connection) < 0) {
	g_assert(!"connection has -ve fd!");
      }

      if(connection->connection_class == GIOP_CONNECTION_SERVER)
	iiop_connection_server_accept(connection);
      else
	recv_buffer = giop_recv_message_buffer_use(connection);

      if(monitor && !monitor->is_valid) return NULL;

    } while(!recv_buffer);

    return recv_buffer;
}

void
giop_main_handle_connection(GIOPConnection *connection)
{
  GIOPRecvBuffer *recv_buffer;

  g_return_if_fail(connection != NULL);
  g_return_if_fail(connection->is_valid);

  if(connection->connection_class == GIOP_CONNECTION_SERVER) {
    iiop_connection_server_accept(connection);
    return;
  } else
    recv_buffer = giop_recv_message_buffer_use(connection);

  if(recv_buffer) {
    if(IIOPIncomingMessageHandler)
      IIOPIncomingMessageHandler(recv_buffer);
    else
      giop_received_list_push(recv_buffer);
  }
}

/*
 * giop_main_handle_connection_exception
 *
 * Input: GIOPConnection *connection
 *
 * Output:
 *
 * Side effects: invalidates connection
 *
 * Description:
 *     When poll() or select() indicates that a file descriptor
 *     has been closed at the remote end, we must invalidate the associated
 *     GIOPConnection structure.
 */
void
giop_main_handle_connection_exception(GIOPConnection *connection)
{
  g_return_if_fail(connection != NULL);
  g_return_if_fail(connection->is_valid);

  giop_connection_ref(connection);

  giop_connection_remove_from_list(connection);

  shutdown(GIOP_CONNECTION_GET_FD(connection), 2);
  close(GIOP_CONNECTION_GET_FD(connection));
  GIOP_CONNECTION(connection)->fd = -1;
  connection->is_valid = FALSE;

  if(connection->incoming_msg) {
    giop_recv_buffer_unuse(connection->incoming_msg);
    connection->incoming_msg = NULL;
  }

  giop_connection_unref(connection);
}

/*
 * giop_main_iterate
 *
 *    Input: 'blocking' - flag to indicate whether to wait for incoming
 *           messages (TRUE), or whether to return immediately if no
 *           incoming messages are available (FALSE).
 *    Output: None
 *    Description:
 *           Gets the next message into recv_buffer (see
 *           giop_main_next_message) If we have a handler for incoming
 *           messages, then pass recv_buffer to the handler (handler
 *           becomes the new owner of recv_buffer's contents). Otherwise,
 *           tosses it onto the list of received-but-unprocessed buffers.
 *
 *    Warnings:
 *           If you don't have an IIOPIncomingMessageHandler set, you're
 *           probably really screwed in the long run.
 */
void
giop_main_iterate(gboolean blocking)
{
  GIOPRecvBuffer *recv_buffer;

  recv_buffer = giop_main_next_message(blocking);

  if(recv_buffer) {
    if(IIOPIncomingMessageHandler)
      IIOPIncomingMessageHandler(recv_buffer);
    else
      giop_received_list_push(recv_buffer);
  }
}

/*
 * giop_check_connections
 *
 *    Inputs: 'block_for_reply' - If no incoming data is immediately available
 *            should this routine wait for incoming data (TRUE) or return
 *            immediately (FALSE).
 *
 *    Outputs: 'connection' - the first connection that has incoming
 *             data available for reading (supposedly a GIOP message, but
 *             could be anything).
 *
 *    Side effects: Removes closed connections from the active list.
 *
 *    Global data structures used: giop_connection_list
 *    
 *    Description: Does a poll or select (OS-dependant) on the list of file
 *                 descriptors in giop_connection_list.
 *
 * 		   If a file descriptor has been closed, call
 * 		   giop_connection_handle_exception() on it and (as
 * 		   appropriated by 'block_for_reply') either return
 * 		   NULL or do another poll/select.
 * 	  
 * 		   If a file descriptor has data available for
 * 		   reading, find the associated GIOPConnection (using
 * 		   giop_connection_list.fd_to_connection_mapping) and
 * 		   return that.
 * 		  
 */
GIOPConnection *
giop_check_connections(gboolean block_for_reply)
{
  GIOPConnection *connection = NULL;
  int pollret;
  int numcnx_checks;
  int i;
  fd_set selectset_rd, selectset_ex;

#ifndef USE_POLL
  struct timeval immediate_timeout = {0,0};
#endif

 do_read_msg:

  if(!giop_connection_list.list)
    return NULL;
#if 0
  giop_connection_list_recreate(); /* easiest way to get valid
				      select sets... */
#endif
  memcpy(&selectset_rd, &giop_connection_list.selectset_rd,
	 sizeof(selectset_rd));
  memcpy(&selectset_ex, &giop_connection_list.selectset_ex,
	 sizeof(selectset_ex));

#ifdef USE_POLL
  numcnx_checks = giop_connection_list.pollset->len;
#else
  numcnx_checks = giop_connection_list.max_fd+1;
#endif

 restart:
#ifdef USE_POLL
  pollret = poll((struct pollfd *)giop_connection_list.pollset->data,
		 giop_connection_list.pollset->len,
		 block_for_reply?-1:0);

#     else /* !USE_POLL */

  {
    pollret = select (giop_connection_list.max_fd + 1,
		      &selectset_rd,
		      NULL, &selectset_ex,
		      block_for_reply?NULL:&immediate_timeout);
  }
#     endif /* !USE_POLL */

  if(pollret <= 0) {
    if(pollret < 0) {
      if(errno == EINTR)
	goto restart;
      else
	g_warning("Error code from select/poll: %s", g_strerror(errno));
    } else
      return NULL;
  }

  /* Check for data to be read on the fd's.
     Note we have to do the hangup/exception checking in a separate loop,
     because there may be data waiting to be read on a connection that the
     other end has closed. */
  for(i = 0; i < numcnx_checks; i++) {
#ifdef USE_POLL
    struct pollfd *p = 
      &g_array_index(giop_connection_list.pollset,
		     struct pollfd, 
		     i);
    g_assert(p->fd <= giop_connection_list.max_fd);
    connection = giop_connection_list.fd_to_connection_mapping->pdata[p->fd];
    if(p->revents & POLLIN)
      goto got_connection;
#else
    connection = giop_connection_list.fd_to_connection_mapping->pdata[i];
    if (FD_ISSET(i, &selectset_rd)) {
      goto got_connection;
    }
#endif    
  }

  /* Handle fd exceptions */
  for(i = 0; i < numcnx_checks; i++)
    {
#ifdef USE_POLL
      struct pollfd *p = 
      &g_array_index(giop_connection_list.pollset,
		     struct pollfd, 
		     i);

      g_assert(p->fd <= giop_connection_list.max_fd);
      if(p->revents & (POLLHUP|POLLNVAL)) {
	connection = giop_connection_list.fd_to_connection_mapping->pdata[p->fd];
	giop_main_handle_connection_exception(connection);
      }
#else /* !USE_POLL */
      if(FD_ISSET(i, &selectset_ex)) {
	connection = giop_connection_list.fd_to_connection_mapping->pdata[i];
	giop_main_handle_connection_exception(connection);
      }
#endif /* !USE_POLL */
    }

  /* Only reached if we didn't find a connection to read data from */
  if(block_for_reply)
    goto do_read_msg;

 got_connection:
  return connection;
}

