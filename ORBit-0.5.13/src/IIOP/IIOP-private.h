#ifndef IIOP_PRIVATE_H
#define IIOP_PRIVATE_H 1


#include "config.h"

#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <glib.h>

typedef struct {
  GList *list;
  gboolean connection_list_changed;
  GPtrArray *fd_to_connection_mapping;
#  ifdef USE_POLL
  GArray *pollset;
#  else
  fd_set selectset_rd, selectset_ex;
#  endif
  int max_fd;
} GIOPConnectionList;

extern GIOPConnectionList giop_connection_list;

/* If you get a buffer that you didn't want, add it to the list! */
void giop_received_list_push(GIOPRecvBuffer *recv_buffer);
GIOPRecvBuffer *giop_received_list_pop(void);


#endif
