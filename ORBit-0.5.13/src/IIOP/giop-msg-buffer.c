/* The big picture:
 * For every outgoing request, we have to have the network-ready data
 * somewhere in memory.
 *
 * Using writev, any pieces that do not need endian conversion can
 * be written in-place.
 *
 * The pieces that do need endian conversion can be put into one or more
 * buffers.
 *
 * WHOA WHOA newsflash
 * Because IIOP lets the message sender specify the endianness,
 * we do not need to do endian conversion _ever_! The receiver can do all
 * conversions if need be, or if they are the same endianness as sender they
 * can just pull it in right off the wire :)
 * 
 */

#include "config.h"
#include "iiop-endianP.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef HAVE_POLL
#  include <sys/poll.h>
#else
#  include <sys/types.h>
#  include <sys/time.h>
#endif
#include "IIOP.h"
#include "IIOP-private.h"

#ifdef HAVE_LIMITED_WRITEV
#define writev g_writev
#endif

/* type defs */

#ifdef        __GNUC__
#define       PACKED __attribute__((packed))
#else
#define       PACKED
#endif

/*
 * Overlaps with struct _GIOPMessageHeader on purpose
 *  - we save time because this stuff never changes
 */
struct _GIOPMessageHeaderConstants {
  GIOP_char magic[4];
  GIOP_char GIOP_version[2];
  GIOP_octet flags;
} PACKED;

#include <stdlib.h>

/* functions */
static gint giop_recv_decode_message(GIOPRecvBuffer *buf);
static gboolean num_on_list(GIOP_unsigned_long num,
			    const GIOP_unsigned_long *request_ids,
			    GIOP_unsigned_long req_count);
static gint giop_recv_reply_decode_message(GIOPRecvBuffer *buf);
static gint giop_recv_request_decode_message(GIOPRecvBuffer *buf);
static gint giop_recv_locate_reply_decode_message(GIOPRecvBuffer *buf);
static gint giop_recv_locate_request_decode_message(GIOPRecvBuffer *buf);
static GIOPRecvBuffer *giop_received_list_check_reply(GIOP_unsigned_long request_id);

#ifdef NOT_REENTRANT
extern DEFINE_LOCK(iiop_connection_list);
#endif
GList *iiop_connection_list = NULL;

/* global variables */
char giop_scratch_space[2048];

static const struct _GIOPMessageHeaderConstants
giop_message_header_constants = {
  "GIOP",
  {1,0},
  FLAG_ENDIANNESS,
};

struct iovec
giop_first_message_vec = {NULL,
			  sizeof(struct _GIOPMessageHeaderConstants)};

DEFINE_LOCK(sendbufferlist);
GSList *sendbufferlist = NULL;

DEFINE_LOCK(recvbufferlist);
GSList *recvbufferlist = NULL;

DEFINE_LOCK(incoming_bufs);
GList *incoming_bufs = NULL; /* List of incoming messages that had to be
				shunted aside */

DEFINE_LOCK(sendbuffers);
DEFINE_LOCK(recvbuffers);
GMemChunk *sendbuffers = NULL, *recvbuffers = NULL;

DEFINE_LOCK(request_id_counter);
GIOP_unsigned_long request_id_counter;

#if 0
inline
void giop_message_buffer_append_iovec(GIOPMessageBuffer *msgbuf,
				      const struct iovec *iovec)
{
  /* g_print("Appending iovec %d bytes @ %p\n", iovec->iov_len, iovec->iov_base); */
  g_array_append_val(msgbuf->iovecs, *iovec);
}
#else
#define giop_message_buffer_append_iovec(msgbuf, iovec) g_array_append_val((msgbuf)->iovecs, *(iovec))
#endif

void
giop_message_buffer_init(void)
{
  giop_first_message_vec.iov_base = (gpointer)&giop_message_header_constants;
  INIT_LOCK(sendbufferlist);
  INIT_LOCK(recvbufferlist);
  request_id_counter = 1;
  INIT_LOCK(request_id_counter);

  INIT_LOCK(sendbuffers);
  sendbuffers = g_mem_chunk_create(GIOPSendBuffer, 2, G_ALLOC_ONLY);
  INIT_LOCK(recvbuffers);
  recvbuffers = g_mem_chunk_create(GIOPRecvBuffer, 2, G_ALLOC_ONLY);
}

static void
giop_message_buffer_new(GIOPMessageBuffer *buf)
{
  buf->iovecs = g_array_new(FALSE, FALSE, sizeof(struct iovec));
}

#define STRUCT_OFFSET(t, f) ((int) ((char*) &((t*) 0)->f))

/* Send buffers only */
static GIOPSendBuffer *
giop_send_buffer_new(void)
{
  GIOPSendBuffer *msgbuf;
  struct iovec firstvec;

  GET_LOCK(sendbuffers);
  msgbuf = g_chunk_new(GIOPSendBuffer, sendbuffers);
  RELEASE_LOCK(sendbuffers);

  giop_message_buffer_new(GIOP_MESSAGE_BUFFER(msgbuf));

  giop_message_buffer_append_iovec(GIOP_MESSAGE_BUFFER(msgbuf),
				   &giop_first_message_vec);

  firstvec.iov_base = &(GIOP_MESSAGE_BUFFER(msgbuf)->message_header.message_type);
  firstvec.iov_len = sizeof(GIOPMessageHeader)
    - STRUCT_OFFSET(GIOPMessageHeader, message_type);
  GIOP_MESSAGE_BUFFER(msgbuf)->message_header.message_size = 0;

  msgbuf->indirects = g_mem_chunk_create(char[GIOP_INDIRECT_CHUNK_SIZE],
					 2, G_ALLOC_ONLY);

  giop_message_buffer_append_iovec(GIOP_MESSAGE_BUFFER(msgbuf), &firstvec);

  return msgbuf;
}

gint
giop_send_buffer_write(GIOPSendBuffer *send_buffer)
{
  gulong nvecs;
  glong res, sum, t;
  struct iovec *curvec;
  int fd;
  GIOPConnection *cnx;
  gint retval = -1;

  cnx = GIOP_MESSAGE_BUFFER(send_buffer)->connection;
  if(!cnx->is_valid)
    return -1;

  fd = GIOP_CONNECTION_GET_FD(cnx);
  nvecs = GIOP_MESSAGE_BUFFER(send_buffer)->iovecs->len;
  curvec = (struct iovec *)GIOP_MESSAGE_BUFFER(send_buffer)->iovecs->data;

#if defined(ORBIT_DEBUG) && 0
  g_print("Message of length %d looks like:\n",
	  GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_size);
  for(sum = 0, t = 2; t < nvecs; t++) {

    sum += curvec[t].iov_len;
    g_print("    [%p, %d]: %d\n", curvec[t].iov_base, curvec[t].iov_len,
	    sum);
  }
#endif
  res = writev(fd, curvec, nvecs);

  sum = (GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_size + sizeof(GIOPMessageHeader));
  if(res < sum) {
    if(res < 0) {
      if(errno != EAGAIN) {
	giop_main_handle_connection_exception(cnx);
	goto out;
      }

      res = 0;
    }

    /* wrote 7, iovecs 3, 2, 2, 4:
       0 + 3 !> 7
       3 + 2 !> 7
       5 + 2 !> 7
     */

    for(t = 0; ; t += curvec->iov_len, curvec++, nvecs--) {
      if((t + curvec->iov_len) > res)
	break;
    }
    if((res - t) > 0) {
      curvec->iov_len -= (res - t);
      curvec->iov_base = (gpointer)((char *)curvec->iov_base + (res - t));
    }

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NONBLOCK);

    t = writev(fd, curvec, nvecs);

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

    if((t < 0) || ((res + t) < sum)) {
	giop_main_handle_connection_exception(cnx);
	goto out;
    }
  }

  retval = 0;

 out:

  return retval;
}

static GIOPSendBuffer *
giop_send_buffer_use(GIOPConnection *connection)
{
  GIOPSendBuffer *retval;

  if(!connection->is_valid)
    return NULL;

  GET_LOCK(sendbufferlist);

  if(sendbufferlist)
    {
      GSList *head;

      retval = sendbufferlist->data;

      head = sendbufferlist;
      sendbufferlist = g_slist_remove_link(sendbufferlist, sendbufferlist);
      g_slist_free_1 (head);

      g_array_set_size(GIOP_MESSAGE_BUFFER(retval)->iovecs, 2);
      GIOP_MESSAGE_BUFFER(retval)->message_header.message_size = 0;
    }
  else
    retval = giop_send_buffer_new();

  RELEASE_LOCK(sendbufferlist);
  
  giop_connection_ref(connection);
  GIOP_MESSAGE_BUFFER(retval)->connection = connection;

  g_mem_chunk_reset(retval->indirects);
  retval->indirect = g_chunk_new(gpointer, retval->indirects);
#ifdef ORBIT_DEBUG
  memset(retval->indirect, '\xFE', GIOP_INDIRECT_CHUNK_SIZE);
#endif
  retval->indirect_used = 0;

  return retval;
}

GIOPSendBuffer *
giop_send_reply_buffer_use(GIOPConnection *connection,
			   const IOP_ServiceContextList *service_context,
			   GIOP_unsigned_long request_id,
			   GIOPReplyStatusType reply_status)
{
  GIOPSendBuffer *send_buffer;

  send_buffer = giop_send_buffer_use(connection);

  if(!send_buffer)
    return NULL;

  GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_type = GIOP_REPLY;

  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   sizeof(GIOP_unsigned_long));
  if(!service_context) {
    static const GIOP_unsigned_long sc_zero_int = 0;
    AP(&sc_zero_int, sizeof(service_context->_length));
  } else {
    int i, n;
    n = service_context->_length;
    AP(&service_context->_length, sizeof(service_context->_length));
    for(i = 0; i < n; i++) {
      int j, o;
      CORBA_sequence_octet *seqo;

      giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				       sizeof(GIOP_unsigned_long));
      AP(&service_context->_buffer[i].context_id,
	 sizeof(service_context->_buffer[i].context_id));
      seqo = &service_context->_buffer[i].context_data;
      o = seqo->_length;
      AP(&seqo->_length, sizeof(GIOP_unsigned_long));
      for(j = 0; j < o; j++)
	AP(seqo->_buffer, seqo->_length);
    }
  }
  send_buffer->message.u.reply.request_id = request_id;
  send_buffer->message.u.reply.reply_status = reply_status;
  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   sizeof(GIOP_unsigned_long));
  AP(&send_buffer->message.u.reply.request_id,
     sizeof(GIOP_unsigned_long));
  AP(&send_buffer->message.u.reply.reply_status,
     sizeof(GIOP_unsigned_long));

  return send_buffer;
}

GIOPSendBuffer *
giop_send_locate_reply_buffer_use(GIOPConnection *connection,
			   GIOP_unsigned_long request_id,
			   GIOPLocateStatusType locate_reply_status)
{
  GIOPSendBuffer *send_buffer;

  send_buffer = giop_send_buffer_use(connection);

  if(!send_buffer)
    return NULL;

  GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_type = GIOP_LOCATEREPLY;

  APIA(&request_id, sizeof(request_id));
  APIA(&locate_reply_status, sizeof(locate_reply_status));

  return send_buffer;
}

GIOPSendBuffer *
giop_send_request_buffer_use(GIOPConnection *connection,
			     const IOP_ServiceContextList *service_context,
			     GIOP_unsigned_long request_id,
			     GIOP_boolean response_expected,
			     const struct iovec *object_key_vec,
			     const struct iovec *operation_vec,
			     const struct iovec *principal_vec)
{
  GIOPSendBuffer *send_buffer;
#if 0
  static const struct {
    CORBA_unsigned_long _length;
    char _buffer[7];
  } default_principal = { sizeof("nobody"), "nobody" };
  static const struct iovec default_principal_vec =
  {(void *)&default_principal,
   sizeof(CORBA_unsigned_long) + sizeof("nobody")};
#endif

  if (!connection)
    return NULL;
  if(!object_key_vec)
    return NULL;
  if(!operation_vec)
    return NULL;

  ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug,
	      "Sending request %s id %d to %s\n",
	      ((guchar *)operation_vec->iov_base) + 4,
	      request_id, ((guchar *)object_key_vec->iov_base) + 4);

  send_buffer = giop_send_buffer_use(connection);

  if (!send_buffer)
    return NULL;

  GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_type = GIOP_REQUEST;

  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   sizeof(GIOP_unsigned_long));
  if(!service_context) {
    static const GIOP_unsigned_long sc_zero_int = 0;
    AP(&sc_zero_int, sizeof(GIOP_unsigned_long));
  } else {
    int i, n;
    n = service_context->_length;
    AP(&service_context->_length, sizeof(service_context->_length));
    for(i = 0; i < n; i++) {
      int j, o;
      CORBA_sequence_octet *seqo;

      giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				       sizeof(GIOP_unsigned_long));
      AP(&service_context->_buffer[i].context_id,
	 sizeof(service_context->_buffer[i].context_id));
      seqo = &service_context->_buffer[i].context_data;
      o = seqo->_length;
      AP(&seqo->_length, sizeof(GIOP_unsigned_long));
      for(j = 0; j < o; j++)
	AP(seqo->_buffer, seqo->_length);
    }
  }
  send_buffer->message.u.request.request_id = request_id;
  send_buffer->message.u.request.response_expected = response_expected;
  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   sizeof(GIOP_unsigned_long));
  AP(&send_buffer->message.u.request.request_id,
     sizeof(GIOP_unsigned_long));
  AP(&send_buffer->message.u.request.response_expected,
     sizeof(GIOP_boolean));
#if 0
  API(&response_expected, 1);
  AP((gpointer)giop_scratch_space, 3);
#endif

  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   sizeof(CORBA_unsigned_long));
  giop_message_buffer_append_iovec(GIOP_MESSAGE_BUFFER(send_buffer),
				   object_key_vec);
  GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_size +=
    object_key_vec->iov_len;

  /*
   * We can know the length at compile time - don't calculate it at runtime
   * if we can help it :)
   */
  /* ENCODER_CALL(CORBA_string, (CORBA_string *)operation); */
  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   sizeof(CORBA_unsigned_long));
  giop_message_buffer_append_iovec(GIOP_MESSAGE_BUFFER(send_buffer),
				   operation_vec);
  GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_size +=
    operation_vec->iov_len;

  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   sizeof(CORBA_unsigned_long));
  giop_message_buffer_append_iovec(GIOP_MESSAGE_BUFFER(send_buffer),
				   principal_vec);
  GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_size +=
    principal_vec->iov_len;

  return send_buffer;
}

GIOPSendBuffer *
giop_send_locate_request_buffer_use(GIOPConnection *connection,
			     GIOP_unsigned_long request_id,
			     const struct iovec *object_key_vec)
{
	GIOPSendBuffer *send_buffer;

	if (!connection)
		return NULL;
	if (!object_key_vec)
		return NULL;

	ORBit_Trace(TraceMod_IIOP, TraceLevel_Debug,
		"Sending locate request id %d to %s\n",
		request_id, ((guchar *)object_key_vec->iov_base) + 4);

	send_buffer = giop_send_buffer_use(connection);

	if (!send_buffer)
		return NULL;

	GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_type = GIOP_LOCATEREQUEST;

	APIA(&request_id, sizeof(request_id));

	giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
		sizeof(CORBA_unsigned_long));
	giop_message_buffer_append_iovec(GIOP_MESSAGE_BUFFER(send_buffer),
		object_key_vec);
	GIOP_MESSAGE_BUFFER(send_buffer)->message_header.message_size +=
		object_key_vec->iov_len;

	return send_buffer;
}

void
giop_send_buffer_unuse(GIOPSendBuffer *send_buffer)
{
  if (send_buffer == NULL)
    return;

  giop_connection_unref(GIOP_MESSAGE_BUFFER(send_buffer)->connection);

  GET_LOCK(sendbufferlist);
  sendbufferlist = g_slist_prepend(sendbufferlist, send_buffer);
  RELEASE_LOCK(sendbufferlist);
}

gulong
giop_message_buffer_do_alignment(GIOPMessageBuffer *buffer,
				 gulong align_for)
{
  struct iovec newvec;
  struct iovec *lastvec;
  guint alignme;
  gulong real_msgsize;
  gulong align_diff;

  if(align_for < 2) return 0;
  if(align_for >
     MAX(sizeof(GIOP_long_long),sizeof(GIOP_long_double)))
    align_for = MAX(sizeof(GIOP_long_long), sizeof(GIOP_long_double));

  real_msgsize = buffer->message_header.message_size+sizeof(GIOPMessageHeader);

  alignme = (gulong)ALIGN_ADDRESS(real_msgsize, align_for);

  align_diff = alignme - real_msgsize;
  if(align_diff > 0)
    {
      lastvec = (struct iovec *)(buffer->iovecs->data) 
				 + buffer->iovecs->len - 1;

      if(lastvec->iov_base == giop_scratch_space)
	{
	  newvec.iov_len = align_diff;
	  lastvec->iov_len += align_diff;
	  buffer->message_header.message_size += align_diff;
	}
      else
	{
	  newvec.iov_base = (gpointer)giop_scratch_space;
	  newvec.iov_len = align_diff;
	  buffer->message_header.message_size += align_diff;
	  giop_message_buffer_append_iovec(buffer, &newvec);
	}
      return newvec.iov_len;
    }
  else
    return 0;
}

void 
giop_message_buffer_append_mem_a(GIOPMessageBuffer *buffer,
				 gconstpointer mem_region,
				 gulong mem_region_length)
{
  struct iovec newvec;
  struct iovec *lastvec;
  gint alignfor;

  alignfor = giop_message_buffer_do_alignment(buffer, mem_region_length);

  lastvec = (struct iovec *)(buffer->iovecs->data) + 
			     + buffer->iovecs->len - 1;

  if((mem_region == giop_scratch_space && lastvec->iov_base == giop_scratch_space)
     || (alignfor == 0 && (((guchar *)lastvec->iov_base) + lastvec->iov_len) == mem_region))
    {
      lastvec->iov_len += mem_region_length;
    }
  else
    {
      newvec.iov_base = (gpointer)mem_region;
      newvec.iov_len = mem_region_length;
      giop_message_buffer_append_iovec(buffer, &newvec);
    }

  buffer->message_header.message_size += mem_region_length;
}

void 
giop_message_buffer_append_mem(GIOPMessageBuffer *buffer,
			       gconstpointer mem_region,
			       gulong mem_region_length)
{
  struct iovec newvec;
  struct iovec *lastvec;

  lastvec = (struct iovec *)(buffer->iovecs->data)
			     + buffer->iovecs->len - 1;

  if((mem_region == giop_scratch_space
      && lastvec->iov_base == giop_scratch_space)
     || ((((guchar *)lastvec->iov_base) + lastvec->iov_len) == mem_region))
    {
      lastvec->iov_len += mem_region_length;
    }
  else
    {
      newvec.iov_base = (gpointer)mem_region;
      newvec.iov_len = mem_region_length;
      giop_message_buffer_append_iovec(buffer, &newvec);
    }

  buffer->message_header.message_size += mem_region_length;
}

/* I think we need a WE_WANT_NEW_CRAPPY_BUGGY_CODE ifdef here - this
   tiny routine seems to be horribly hard to get right.

   Basically we have to paste the whole of 'mem_region' into our
   memory chunks, possibly subdividing it up to fit it into multiple
   1K chunks. Because we have to return the first paste point in case
   the client wants to manipulate it afterwards, if mem_region_length
   >= sizeof(CORBA_unsigned_long), we also have to guarantee that the
   pasted stuff doesn't get divided on a finer boundary than
   sizeof(CORBA_unsigned_long).
*/
gpointer
giop_send_buffer_append_mem_indirect(GIOPSendBuffer *send_buffer,
				     gconstpointer mem_region,
				     gulong mem_region_length)
{
  gulong offset = 0, thisblock_size;
  gpointer blockstart = NULL;

  while(offset < mem_region_length) {
    thisblock_size = MIN(mem_region_length - offset,
			 GIOP_INDIRECT_CHUNK_SIZE - send_buffer->indirect_used);

    if((thisblock_size >= sizeof(CORBA_unsigned_long))
       || (mem_region_length - offset) < sizeof(CORBA_unsigned_long)) {
      if (!blockstart)
	blockstart =
	  ((guchar*) send_buffer->indirect) + send_buffer->indirect_used;

      memcpy((guchar*)send_buffer->indirect + send_buffer->indirect_used,
	     (guchar*)mem_region + offset, thisblock_size);
      giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(send_buffer),
				     (guchar*)send_buffer->indirect +
				     send_buffer->indirect_used,
				     thisblock_size);
      offset += thisblock_size;
      send_buffer->indirect_used += thisblock_size;
    }

    if(send_buffer->indirect_used >= (GIOP_INDIRECT_CHUNK_SIZE - sizeof(CORBA_unsigned_long))) {
#ifdef I_CANT_FIGURE_OUT_WHAT_THIS_LOGIC_WAS_MEANT_TO_DO
       || (thisblock_size >= sizeof(CORBA_unsigned_long)
	   && (mem_region_length - offset) > 0)) {
#endif
	  send_buffer->indirect_used = 0;
	  send_buffer->indirect = g_chunk_new(gpointer,
					      send_buffer->indirects);
    }
  }

  return blockstart;
}

#ifdef WE_WANT_OLD_DEAD_CRAPPY_BUGGY_CODE
gpointer
_giop_send_buffer_append_mem_indirect(GIOPSendBuffer *send_buffer,
				     gconstpointer mem_region,
				     gulong mem_region_length)
{
  gpointer blockstart = NULL;
  gulong offset, new_offset;

  for(offset = new_offset = 0; new_offset < mem_region_length;)
    {
      new_offset =
	MIN(mem_region_length - offset,
	    GIOP_INDIRECT_CHUNK_SIZE - send_buffer->indirect_used);

      if((new_offset - offset) > sizeof(CORBA_unsigned_long)
	 || mem_region_length >= sizeof(CORBA_unsigned_long)) {

	if(!blockstart)
	  blockstart = send_buffer->indirect + send_buffer->indirect_used;
      }
	
      memcpy(send_buffer->indirect + send_buffer->indirect_used,
	     mem_region + offset, new_offset - offset);
	
      giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(send_buffer),
				     send_buffer->indirect + send_buffer->indirect_used,
				     new_offset - offset);

      send_buffer->indirect_used += new_offset - offset;

      offset = new_offset;

      if(new_offset >= GIOP_INDIRECT_CHUNK_SIZE)
	{
	  send_buffer->indirect_used = 0;
	  send_buffer->indirect = g_chunk_new(gpointer,
					      send_buffer->indirects);
#ifdef ORBIT_DEBUG
	  memset(send_buffer->indirect, '\xFE', GIOP_INDIRECT_CHUNK_SIZE);
#endif
	}
    }

  return blockstart;
}
#endif

gpointer
giop_send_buffer_append_mem_indirect_a(GIOPSendBuffer *send_buffer,
				       gconstpointer mem_region,
				       gulong mem_region_length)
{
  giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER(send_buffer),
				   mem_region_length);
  return giop_send_buffer_append_mem_indirect(send_buffer,
					      mem_region, mem_region_length);
}

GIOP_unsigned_long
giop_get_request_id(void)
{
  GIOP_unsigned_long retval;
  GET_LOCK(request_id_counter);
  retval = request_id_counter++;
  RELEASE_LOCK(request_id_counter);
  return retval;
}

/****************************************************
 * GIOPRecvBuffer routines
 ****************************************************/

static GIOPRecvBuffer *
giop_recv_buffer_new(void)
{
  GIOPRecvBuffer *msgbuf;

  GET_LOCK(recvbuffers);
  msgbuf = g_chunk_new(GIOPRecvBuffer, recvbuffers);
  RELEASE_LOCK(recvbuffers);

  giop_message_buffer_new(GIOP_MESSAGE_BUFFER(msgbuf));
  msgbuf->message_body = NULL;

  return msgbuf;
}

void
giop_recv_buffer_unuse(GIOPRecvBuffer *buffer)
{
  if (buffer == NULL)
    return;

  if(buffer->message_body)
    {
      g_free(buffer->message_body);
      buffer->message_body = NULL;
    }

  if(GIOP_MESSAGE_BUFFER(buffer)->connection->incoming_msg == buffer)
    GIOP_MESSAGE_BUFFER(buffer)->connection->incoming_msg = NULL;

  giop_connection_unref(GIOP_MESSAGE_BUFFER(buffer)->connection);

  GET_LOCK(recvbufferlist);
  recvbufferlist = g_slist_prepend(recvbufferlist, buffer);
  RELEASE_LOCK(recvbufferlist);
}

static GIOPRecvBuffer *
giop_recv_buffer_use(GIOPConnection *connection)
{
  GIOPRecvBuffer *retval;

  if(!connection || !connection->is_valid)
    return NULL;

  GET_LOCK(recvbufferlist);

  if(recvbufferlist)
    {
      GSList *head;

      retval = recvbufferlist->data;

      head = recvbufferlist;
      recvbufferlist = g_slist_remove_link(recvbufferlist, recvbufferlist);
      g_slist_free_1 (head);

      GIOP_MESSAGE_BUFFER(retval)->message_header.message_size = 0;
      retval->message_body = NULL;
    }
  else
    retval = giop_recv_buffer_new();

  retval->state = GIOP_MSG_READING_HEADER;
  retval->left_to_read = sizeof(GIOPMessageHeader);

  RELEASE_LOCK(recvbufferlist);
  
  giop_connection_ref(connection);
  GIOP_MESSAGE_BUFFER(retval)->connection = connection;

  return retval;
}

#ifdef GIOP_INTERNAL_DEBUG
static void
dump (FILE *out, guint8 const *ptr, guint32 len, guint32 offset)
{
	guint32 lp,lp2;
	guint32 off;

	for (lp = 0;lp<(len+15)/16;lp++) {
		fprintf (out, "0x%4x ", offset + lp * 16);
		for (lp2=0;lp2<16;lp2++) {
			off = lp2 + (lp<<4);
			off<len?fprintf (out, "%2x ", ptr[off]):fprintf (out, "XX ");
		}
		fprintf (out, "| ");
		for (lp2=0;lp2<16;lp2++) {
			off = lp2 + (lp<<4);
			fprintf (out, "%c", off<len?(ptr[off]>'!'&&ptr[off]<127?ptr[off]:'.'):'*');
		}
		if (lp == 0)
			fprintf (out, " --- \n");
		else
			fprintf (out, "\n");
	}
}

/*
void
giop_dump_send (GIOPSendBuffer *send_buffer)
{
	gulong nvecs;
	struct iovec *curvec;
	guint32 offset = 0;

	g_return_if_fail (send_buffer != NULL);

	nvecs = send_buffer->num_used;
	curvec = (struct iovec *) send_buffer->iovecs;

	fprintf (stderr, "Outgoing IIOP data:\n");
	while (nvecs-- > 0) {
		dump (stderr, curvec->iov_base, curvec->iov_len, offset);
		offset += curvec->iov_len;
		curvec++;
	}
} */

void
giop_dump_recv (GIOPRecvBuffer *recv_buffer)
{
	g_return_if_fail (recv_buffer != NULL);

	fprintf (stderr, "Incoming IIOP data\n");

	dump (stderr, recv_buffer->message_body,
	      GIOP_MESSAGE_BUFFER (recv_buffer)->message_header.message_size +
	      sizeof (GIOPMessageHeader), 0);
}
#endif /* GIOP_INTERNAL_DEBUG */

GIOPRecvBuffer *
giop_recv_message_buffer_use(GIOPConnection *connection)
{
  GIOPRecvBuffer *retval;
  char *bptr;
  int sysret;
  guint message_size;
  
  if(!connection || !connection->is_valid)
    return NULL;

  if(connection->incoming_msg)
    retval = connection->incoming_msg;
  else {
    retval = giop_recv_buffer_use(connection);
    connection->incoming_msg = retval;
  }

  if(!retval) return NULL;

#ifdef GIOP_INTERNAL_DEBUG
  g_warning ("Recv message buffer use");
#endif

  do {
    switch(retval->state) {
    case GIOP_MSG_READING_HEADER:
      bptr = ((char *)&(GIOP_MESSAGE_BUFFER(retval)->message_header));
      bptr += sizeof(GIOP_MESSAGE_BUFFER(retval)->message_header)
	- retval->left_to_read;
      break;
    case GIOP_MSG_READING_BODY:
      bptr = retval->cur; /* Reason for not using retval->message_body:
			     See note XXX1 below */
      bptr += GIOP_MESSAGE_BUFFER(retval)->message_header.message_size;
      bptr -= retval->left_to_read;
      break;
    default:
      bptr = NULL;
    }

    sysret = read(GIOP_CONNECTION_GET_FD(connection), bptr,
		  retval->left_to_read);

    if((sysret == 0)
       || ((sysret < 0) && (errno != EAGAIN)))
      goto errout;

    if(sysret > 0)
      retval->left_to_read -= sysret;

    if(retval->left_to_read == 0) {
      /* we change states here */

      switch(retval->state) {
      case GIOP_MSG_READING_HEADER:
	/* Check the magic stuff */
	if(strncmp(GIOP_MESSAGE_BUFFER(retval)->message_header.magic, "GIOP", 4)
	   || GIOP_MESSAGE_BUFFER(retval)->message_header.GIOP_version[0] != 1)
	  goto errout;

	if(GIOP_MESSAGE_BUFFER(retval)->message_header.message_size == 0
	   && GIOP_MESSAGE_BUFFER(retval)->message_header.message_type != GIOP_CLOSECONNECTION) {
	  g_warning("Unexpected 0-length IIOP message");
	  goto errout;
	}

	if(giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(retval))) {
	  CORBA_unsigned_long t = GIOP_MESSAGE_BUFFER(retval)->message_header.message_size;
	  retval->decoder = (void (*)(gpointer, gpointer, gulong))iiop_byteswap;

	  iiop_byteswap((gpointer)&GIOP_MESSAGE_BUFFER(retval)->message_header.message_size,
			(gpointer)&t, sizeof(t));
	} else {
	  retval->decoder = (void (*)(gpointer,gpointer,gulong))memcpy;
	}

	message_size = GIOP_MESSAGE_BUFFER(retval)->message_header.message_size;
	if(!connection->is_auth
	   && message_size > 131072) {
	  g_warning("message size is bigger than 128k (%d)", message_size);
	  goto errout;
	}

	retval->message_body = g_malloc(message_size+sizeof(GIOPMessageHeader)+4);
	/* XXX1 This is a lame hack to work with the fact that
	   alignment is relative to the MessageHeader, not the RequestHeader */
	retval->cur = (gpointer)(((char *)retval->message_body) + 12);
	retval->state = GIOP_MSG_READING_BODY;
	retval->left_to_read = message_size;
	break;
      case GIOP_MSG_READING_BODY:
	if(giop_recv_decode_message(retval))
	  goto errout;
	connection->incoming_msg = NULL;
	retval->state = GIOP_MSG_READY;
	break;
      default:
        break;
      }
    } else if(retval->left_to_read < 0) {
      g_warning("Whoa, we overstepped the number of bytes we were supposed to read by %d", -retval->left_to_read);
      goto errout;
    } else /* retval->left_to_read > 0 */ {
      /* couldn't read the whole piece, save it */
      retval = NULL;
    }
  } while(retval && retval->state != GIOP_MSG_READY);

#ifdef GIOP_INTERNAL_DEBUG
  g_warning ("Recv message buffer use");
  giop_dump_recv (retval);
#endif /* GIOP_INTERNAL_DEBUG */
  
  return retval;

 errout:
  giop_recv_buffer_unuse(retval);
  giop_main_handle_connection_exception(connection);
  return NULL;
}

void
giop_received_list_push(GIOPRecvBuffer *recv_buffer)
{
  GET_LOCK(incoming_bufs);
  incoming_bufs = g_list_prepend(incoming_bufs, recv_buffer);
  RELEASE_LOCK(incoming_bufs);
}

GIOPRecvBuffer *giop_received_list_pop(void)
{
  GList *head;
  GIOPRecvBuffer *retval;

  GET_LOCK(incoming_bufs);

  head = incoming_bufs;

  if(!head)
    return NULL;

  retval = head->data;
  incoming_bufs = g_list_remove_link(incoming_bufs, head);
  g_list_free_1 (head);

  RELEASE_LOCK(incoming_bufs);

  return retval;
}

static GIOPRecvBuffer *
giop_received_list_check_reply(GIOP_unsigned_long request_id)
{
  GIOPRecvBuffer *retval = NULL;
  GList *item = NULL;

  GET_LOCK(incoming_bufs);

  for(item = incoming_bufs; item; item = g_list_next(item))
    {
      if(GIOP_MESSAGE_BUFFER(item->data)->message_header.message_type == GIOP_REPLY
	 && GIOP_RECV_BUFFER(item->data)->message.u.reply.request_id == request_id) {
	retval = item->data;
	break;
      }
    }

  if(retval)
    incoming_bufs = g_list_remove(incoming_bufs, retval);

  RELEASE_LOCK(incoming_bufs);

  return retval;
}

/** giop_recv_reply_buffer_use_multiple
 */
GIOPRecvBuffer *
giop_recv_reply_buffer_use_multiple(GArray *request_ids,
				    gboolean block_for_reply)
{
  return giop_recv_reply_buffer_use_multiple_2(NULL, request_ids, block_for_reply);
}

/* here is how it will be:
   one routine for getting next message with a specified reply ID.
 */

GIOPRecvBuffer *
giop_recv_reply_buffer_use_multiple_2(GIOPConnection *request_cnx,
				      GArray *request_ids,
				      gboolean block_for_reply)
{
  int i;
  GIOPRecvBuffer *retval = NULL;
  GSList *pushme = NULL;

  do {
    if(request_cnx && !request_cnx->is_valid)
      break;
    /*
     * We _do_ want to put this inside the loop,
     * because we may call ourselves recursively for different request_id's
     */
    for(i = 0; i < request_ids->len && !retval; i++)
      retval = giop_received_list_check_reply(g_array_index(request_ids, GIOP_unsigned_long, i));

    if(retval)
      break;

    retval = giop_main_next_message_2(block_for_reply, request_cnx);

    if(retval) {
      if(GIOP_MESSAGE_BUFFER(retval)->message_header.message_type == GIOP_REPLY) {
	 if(num_on_list(retval->message.u.reply.request_id,
			(GIOP_unsigned_long *)request_ids->data,
			request_ids->len))
	   break;
	 else {
	   pushme = g_slist_prepend(pushme, retval); retval = NULL;
	 }
      } else {
	if(IIOPIncomingMessageHandler)
	  IIOPIncomingMessageHandler(retval);
	else {
	  pushme = g_slist_prepend(pushme, retval); retval = NULL;
	}
	retval = NULL;
      }
    } else
      return NULL;

  } while(!retval && block_for_reply);

  g_slist_foreach(pushme, (GFunc)giop_received_list_push, NULL);
  g_slist_free(pushme);

  return retval;
}

GIOPRecvBuffer *
giop_recv_reply_buffer_use(GIOP_unsigned_long request_id,
			   gboolean block_for_reply)
{
  return giop_recv_reply_buffer_use_2(NULL, request_id, block_for_reply);
}

GIOPRecvBuffer *
giop_recv_reply_buffer_use_2(GIOPConnection *request_cnx,
			     GIOP_unsigned_long request_id,
			     gboolean block_for_reply)
{
  GArray fakeme;

  fakeme.len = 1;
  fakeme.data = (gpointer)&request_id;

  return giop_recv_reply_buffer_use_multiple_2(request_cnx,
					       &fakeme,
					       block_for_reply);
}

GIOPRecvBuffer *
giop_recv_locate_reply_buffer_use(GIOP_unsigned_long request_id,
				  gboolean block_for_reply)
{
  GIOPRecvBuffer *retval = NULL;
  
  do {
    /*
     * We _do_ want to put this inside the loop,
     * because we may call ourselves recursively for different request_id's
     */
    retval = giop_received_list_check_reply(request_id);

    if(retval)
      break;

    retval = giop_main_next_message_2(TRUE, NULL);

    if(retval) {
      if(GIOP_MESSAGE_BUFFER(retval)->message_header.message_type == GIOP_LOCATEREPLY
	 && retval->message.u.locate_reply.request_id == request_id)
	break;
      else {
	if(IIOPIncomingMessageHandler)
	  IIOPIncomingMessageHandler(retval);
	else
	  giop_received_list_push(retval);
	retval = NULL;
      }
    } else
      return NULL;
  } while(!retval && block_for_reply);

  return retval;
}

static gint
giop_recv_decode_message(GIOPRecvBuffer *buf)
{
  switch(GIOP_MESSAGE_BUFFER(buf)->message_header.message_type)
    {
    case GIOP_REPLY:
      return giop_recv_reply_decode_message(buf);
      break;
    case GIOP_REQUEST:
      return giop_recv_request_decode_message(buf);
      break;
    case GIOP_LOCATEREQUEST:
      return(giop_recv_locate_request_decode_message(buf));
      break;
    case GIOP_LOCATEREPLY:
      return(giop_recv_locate_reply_decode_message(buf));
      break;
    case GIOP_CLOSECONNECTION:
      return 0;
      break;
    default:
      g_warning("Don't know how to decode message type %d",
		GIOP_MESSAGE_BUFFER(buf)->message_header.message_type);
      return -1;
    }
}

/* if(currptr+len > end || currptr + len < currptr) */

/* This whole mess needs redoing. */
#define CHECK_NEW_POS(buf, requested_increment) \
if(!( (( ((guchar*)GIOP_RECV_BUFFER(buf)->cur) \
		        + (requested_increment) ) \
		       <= ( ((guchar *)GIOP_RECV_BUFFER(buf)->message_body) \
			   + GIOP_MESSAGE_BUFFER(buf)->message_header.message_size) + 12) \
		      && ( ( ((guchar*)GIOP_RECV_BUFFER(buf)->cur) \
			    + (requested_increment) ) \
			  >= ((guchar*)GIOP_RECV_BUFFER(buf)->cur) ))) goto out;

#define NEW_POS_OUT out: return -1

#define SAFE_ALIGN_ADDRESS(buf, amt) G_STMT_START { \
guchar *newval; \
newval = ALIGN_ADDRESS(GIOP_RECV_BUFFER(buf)->cur, amt); \
CHECK_NEW_POS(buf, newval-((guchar *)GIOP_RECV_BUFFER(buf)->cur)); \
GIOP_RECV_BUFFER(buf)->cur = newval; \
} G_STMT_END

#define GET_ULONG(x) G_STMT_START{					\
                (x) = GUINT32_SWAP_LE_BE((*(CORBA_unsigned_long *)buf->cur)); \
                CHECK_NEW_POS(buf, sizeof(CORBA_unsigned_long));        \
		buf->cur = ((guchar *)buf->cur) + sizeof(CORBA_unsigned_long);	\
	}G_STMT_END

#define GET_ULONG_NC(x) G_STMT_START{ \
			*(x) = (*((CORBA_unsigned_long *)(buf->cur))); \
                        CHECK_NEW_POS(buf, sizeof(CORBA_unsigned_long)); \
			buf->cur = ((guchar *)buf->cur) + sizeof(CORBA_unsigned_long); \
		}G_STMT_END

/* There be dragons in here. */
static gint
giop_recv_reply_decode_message(GIOPRecvBuffer *buf)
{
  /*
	  enum ReplyStatusType {
		NO_EXCEPTION,
		USER_EXCEPTION,
		SYSTEM_EXCEPTION,
		LOCATION_FORWARD
	};

	struct ReplyHeader {
		IOP::ServiceContextList service_context;
		unsigned long request_id;
		ReplyStatusType reply_status;
	};
  */
  int i;

#ifdef GIOP_INTERNAL_DEBUG
  g_warning ("recv_reply_decode_message");
#endif

  buf->message.u.reply.service_context._maximum = 0;
  if(giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(buf))) 
    {
      GET_ULONG(buf->message.u.reply.service_context._length);
/* XXX bad hardcoded hack until someone gives a "right answer" to how to
solve this problem */
      if(buf->message.u.reply.service_context._length > 128) {
	      g_warning ("length '0x%x' > 128", buf->message.u.reply.service_context._length);
	      return -1;
      }
      buf->message.u.reply.service_context._buffer =
	g_new(IOP_ServiceContext, buf->message.u.reply.service_context._length);

      for(i = 0; i < buf->message.u.reply.service_context._length; i++)
	{
	  SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
	  GET_ULONG(buf->message.u.reply.service_context._buffer[i].context_id);
	  GET_ULONG(buf->message.u.reply.service_context._buffer[i].context_data._length);
	  buf->message.u.reply.service_context._buffer[i].context_data._buffer =
	    buf->cur;
	  CHECK_NEW_POS(buf, buf->message.u.reply.service_context._buffer[i].context_data._length);
	  buf->cur = ((guchar *)buf->cur) + buf->message.u.reply.service_context._buffer[i].context_data._length;
	}
      GET_ULONG(buf->message.u.reply.request_id);
      GET_ULONG(buf->message.u.reply.reply_status);
    }
  else
    {

      GET_ULONG_NC(&buf->message.u.reply.service_context._length);
/* XXX bad hardcoded hack until someone gives a "right answer" to how to
solve this problem */
      if(buf->message.u.reply.service_context._length > 128) {
	      g_warning ("length '0x%x' > 128", buf->message.u.reply.service_context._length);
	      return -1;
      }
      buf->message.u.reply.service_context._buffer =
	g_new(IOP_ServiceContext, buf->message.u.reply.service_context._length);

      for(i = 0; i < buf->message.u.reply.service_context._length; i++)
	{
	  SAFE_ALIGN_ADDRESS(buf, sizeof(CORBA_unsigned_long));
	  GET_ULONG_NC(&buf->message.u.reply.service_context._buffer[i].context_id);
	  GET_ULONG_NC(&buf->message.u.reply.service_context._buffer[i].context_data._length);
	  buf->message.u.reply.service_context._buffer[i].context_data._buffer =
	    buf->cur;
	  CHECK_NEW_POS(buf, buf->message.u.reply.service_context._buffer[i].context_data._length);
	  buf->cur = ((guchar *)buf->cur) + buf->message.u.reply.service_context._buffer[i].context_data._length;
	}
      GET_ULONG_NC(&buf->message.u.reply.request_id);
      GET_ULONG_NC(&buf->message.u.reply.reply_status);
    }

#ifdef GIOP_INTERNAL_DEBUG
  g_message("[%d] Received reply %d size %d to request %d",
	    getpid(),
	    buf->message.u.reply.reply_status,
	    GIOP_MESSAGE_BUFFER(buf)->message_header.message_size,
	    buf->message.u.reply.request_id);
#endif

  return 0;

  NEW_POS_OUT;
}

static gint
giop_recv_locate_reply_decode_message(GIOPRecvBuffer *buf)
{
  if(giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(buf))) 
    {
      GET_ULONG(buf->message.u.locate_reply.request_id);
      GET_ULONG(buf->message.u.locate_reply.locate_status);
    }
  else
    {
      GET_ULONG_NC(&buf->message.u.locate_reply.request_id);
      GET_ULONG_NC(&buf->message.u.locate_reply.locate_status);
    }

  return 0;
  NEW_POS_OUT;
}

static gint
giop_recv_request_decode_message(GIOPRecvBuffer *buf)
{
  GIOP_unsigned_long len;
  int i;

  buf->message.u.request.service_context._maximum = 0;
  if(giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(buf))) 
    {
      GET_ULONG(buf->message.u.request.service_context._length);

      /* XXX bad hardcoded hack until someone gives a "right answer"
	 to how to solve this problem */

      if(buf->message.u.request.service_context._length > 128) return -1;
      buf->message.u.request.service_context._buffer =
	g_new(IOP_ServiceContext, buf->message.u.request.service_context._length);

      for(i = 0; i < buf->message.u.request.service_context._length; i++)
	{
	  SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
	  GET_ULONG(buf->message.u.request.service_context._buffer[i].context_id);
	  GET_ULONG(buf->message.u.request.service_context._buffer[i].context_data._length);
	  buf->message.u.request.service_context._buffer[i].context_data._buffer =
	    buf->cur;
	  CHECK_NEW_POS(buf, buf->message.u.request.service_context._buffer[i].context_data._length);
	  buf->cur = ((guchar *)buf->cur) + buf->message.u.request.service_context._buffer[i].context_data._length;
	}

      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG(buf->message.u.request.request_id);
      buf->message.u.request.response_expected = *((GIOP_boolean *)buf->cur);
      CHECK_NEW_POS(buf, sizeof(GIOP_boolean));
      buf->cur = ((guchar *)buf->cur) + sizeof(GIOP_boolean);

      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG(buf->message.u.request.object_key._length);
      buf->message.u.request.object_key._buffer = buf->cur;

      CHECK_NEW_POS(buf, buf->message.u.request.object_key._length);
      buf->cur = ((guchar *)buf->cur) + buf->message.u.request.object_key._length;

      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG(len);
      buf->message.u.request.operation = buf->cur;

      CHECK_NEW_POS(buf, len);
      buf->cur = ((guchar *)buf->cur) + len;

      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG(buf->message.u.request.requesting_principal._length);
      buf->message.u.request.requesting_principal._buffer = buf->cur;

      CHECK_NEW_POS(buf, buf->message.u.request.requesting_principal._length);
      buf->cur = ((guchar *)buf->cur) + buf->message.u.request.requesting_principal._length;
    }
  else
    {
      GET_ULONG_NC(&buf->message.u.request.service_context._length);

      /* XXX bad hardcoded hack until someone gives a "right answer"
	 to how to solve this problem */
      if(buf->message.u.request.service_context._length > 128) return -1;
      buf->message.u.request.service_context._buffer =
	g_new(IOP_ServiceContext, buf->message.u.request.service_context._length);

      for(i = 0; i < buf->message.u.request.service_context._length; i++)
	{
	  SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
	  GET_ULONG_NC(&buf->message.u.request.service_context._buffer[i].context_id);
	  GET_ULONG_NC(&buf->message.u.request.service_context._buffer[i].context_data._length);
	  buf->message.u.request.service_context._buffer[i].context_data._buffer =
	    buf->cur;
	  CHECK_NEW_POS(buf, buf->message.u.request.service_context._buffer[i].context_data._length);
	  buf->cur = ((guchar *)buf->cur) + buf->message.u.request.service_context._buffer[i].context_data._length;
	}
      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG_NC(&buf->message.u.request.request_id);
      buf->message.u.request.response_expected = *((GIOP_boolean *)buf->cur);
      CHECK_NEW_POS(buf, sizeof(GIOP_boolean));
      buf->cur = ((guchar *)buf->cur) + sizeof(GIOP_boolean);
      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG_NC(&buf->message.u.request.object_key._length);
      buf->message.u.request.object_key._buffer = buf->cur;
      CHECK_NEW_POS(buf, buf->message.u.request.object_key._length);
      buf->cur = ((guchar *)buf->cur) + buf->message.u.request.object_key._length;

      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG_NC(&len);
      buf->message.u.request.operation = buf->cur;
      CHECK_NEW_POS(buf, len);
      buf->cur = ((guchar *)buf->cur) + len;

      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG_NC(&buf->message.u.request.requesting_principal._length);
      buf->message.u.request.requesting_principal._buffer = buf->cur;
      CHECK_NEW_POS(buf, buf->message.u.request.requesting_principal._length);
      buf->cur = ((guchar *)buf->cur) + buf->message.u.request.requesting_principal._length;
    }

#ifdef GIOP_INTERNAL_DEBUG
  g_message("[%d] Received request %s size %d ID %d",
	    getpid(),
	    buf->message.u.request.operation,
	    GIOP_MESSAGE_BUFFER(buf)->message_header.message_size,
	    buf->message.u.request.request_id);
#endif

  return 0;

  NEW_POS_OUT;
}

static gint
giop_recv_locate_request_decode_message(GIOPRecvBuffer *buf)
{
  if(giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(buf))) 
    {
      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG(buf->message.u.locate_request.request_id);
      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG(buf->message.u.locate_request.object_key._length);
      buf->message.u.locate_request.object_key._buffer = buf->cur;
      CHECK_NEW_POS(buf, buf->message.u.locate_request.object_key._length);
      buf->cur = ((guchar *)buf->cur) + buf->message.u.locate_request.object_key._length;
    }
  else
    {
      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG_NC(&buf->message.u.locate_request.request_id);
      SAFE_ALIGN_ADDRESS(buf, sizeof(GIOP_unsigned_long));
      GET_ULONG_NC(&buf->message.u.locate_request.object_key._length);
      buf->message.u.locate_request.object_key._buffer = buf->cur;
      CHECK_NEW_POS(buf, buf->message.u.locate_request.object_key._length);
      buf->cur = ((guchar *)buf->cur) + buf->message.u.locate_request.object_key._length;
    }

  return 0;

  NEW_POS_OUT;
}

gboolean
num_on_list(GIOP_unsigned_long num,
	    const GIOP_unsigned_long *request_ids,
	    GIOP_unsigned_long req_count)
{
  int i;
  for(i = 0; i < req_count; i++)
    {
      if(num == request_ids[i])
	return TRUE;
    }

  return FALSE;
}
